/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexcortex.c                                                  */
/*    Author:     James Pearman                                                */
/*    Created:    7 May 2013                                                   */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     4 July 2013 - Initial release                      */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    This file is part of ConVEX.                                             */
/*                                                                             */
/*    The author is supplying this software for use with the VEX cortex        */
/*    control system. ConVEX is free software; you can redistribute it         */
/*    and/or modify it under the terms of the GNU General Public License       */
/*    as published by the Free Software Foundation; either version 3 of        */
/*    the License, or (at your option) any later version.                      */
/*                                                                             */
/*    ConVEX is distributed in the hope that it will be useful,                */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*    GNU General Public License for more details.                             */
/*                                                                             */
/*    You should have received a copy of the GNU General Public License        */
/*    along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*                                                                             */
/*    A special exception to the GPL can be applied should you wish to         */
/*    distribute a combined work that includes ConVEX, without being obliged   */
/*    to provide the source code for any proprietary components.               */
/*    See the file exception.txt for full details of how and when the          */
/*    exception can be applied.                                                */
/*                                                                             */
/*    The author can be contacted on the vex forums as jpearman                */
/*    or electronic mail using jbpearman_at_mac_dot_com                        */
/*    Mentor for team 8888 RoboLancers, Pasadena CA.                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/


#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexcortex.c
  * @brief   Cortex initialization and system tasks
*//*---------------------------------------------------------------------------*/

// Configuration of console serial port
// this is the master processor communications on the cortex
static const SerialConfig console_config =
{
#ifdef  BOARD_OLIMEX_STM32_P103
    115200,
#else
    230400,
#endif
    0,
    USART_CR2_STOP1_BITS,
    0
};

// Configuration of lcd serial port
// lcd communications at 19200 baud
static const SerialConfig lcd_config =
{
    19200,
    0,
    USART_CR2_STOP1_BITS,
    0
};

/*-----------------------------------------------------------------------------*/
/** @brief      Link thread with it's event listener                           */
/*-----------------------------------------------------------------------------*/
typedef struct {
    Thread          *tp;
    EventListener   el;
    bool_t          persistent;
} vexThread;

/*-----------------------------------------------------------------------------*/
/** @brief      The terminate event source                                     */
/*-----------------------------------------------------------------------------*/
static EVENTSOURCE_DECL(task_terminate);

/*-----------------------------------------------------------------------------*/
/** @brief      Storage for the user threads                                   */
/*-----------------------------------------------------------------------------*/
#define MAX_THREAD  20
static  vexThread   myThreads[MAX_THREAD];

/*-----------------------------------------------------------------------------*/
/*  @brief      Cleanup ROBOTC style tasks                                     */
/*-----------------------------------------------------------------------------*/
void   CleanupTask( Thread *tp )  __attribute__ ((weak));

/*-----------------------------------------------------------------------------*/
/*  @brief      Flag to cause all user tasks to be stopped                     */
/*-----------------------------------------------------------------------------*/
static  bool_t      vexKillAll = FALSE;

/*-----------------------------------------------------------------------------*/
/** @brief      Register a thread so it can be terminated during vexSleep      */
/** @param[in]  name string describing the thread                              */
/*-----------------------------------------------------------------------------*/

void
vexTaskRegister(char *name)
{
    vexTaskRegisterPersistant( name, FALSE );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Register a thread so it can be terminated during vexSleep      */
/** @param[in]  name string describing the thread                              */
/** @param[in]  p persistent flag                                              */
/*-----------------------------------------------------------------------------*/

void
vexTaskRegisterPersistant(char *name, bool_t p )
{
    uint16_t    i;

    // register name
    chRegSetThreadName(name);

    // check the array to see if this thread was already registered
    for(i=0;i<MAX_THREAD;i++)
        {
        // if we are already in the array
        if( myThreads[ i ].tp == chThdSelf())
            {
            // update persistent flag
            myThreads[ i ].persistent = p;
            return;
            }
        }

    // So we are not registered, look for an empty slot
    for(i=0;i<MAX_THREAD;i++)
        {
        if( myThreads[ i ].tp == 0 )
            {
            myThreads[ i ].tp = chThdSelf();
            myThreads[ i ].persistent = p;
            chEvtRegisterMask(&task_terminate, &myThreads[ i ].el, 1);
            break;
            }
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Check if this thread was already registered                    */
/** @param[in]  tp pointer to the Thread structure                             */
/** @return     bool with registration status                                  */
/*-----------------------------------------------------------------------------*/

bool_t
vexTaskIsRegistered( Thread *tp )
{
    uint16_t    i;
    bool_t  isreg = FALSE;

    for(i=0;i<MAX_THREAD;i++)
        {
        // see if we are already in the array
        if( myThreads[ i ].tp == tp)
            {
            isreg = TRUE;
            break;
            }
        }
    return(isreg);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Check if this thread has persistent flag set                   */
/** @param[in]  tp pointer to the Thread structure                             */
/** @return     bool with persistent status                                    */
/*-----------------------------------------------------------------------------*/

bool_t
vexTaskPersistentGet( Thread *tp )
{
    uint16_t    i;
    bool_t      p = FALSE;

    for(i=0;i<MAX_THREAD;i++)
        {
        // see if we are already in the array
        if( myThreads[ i ].tp == tp)
            {
            p = myThreads[ i ].persistent;
            break;
            }
        }
    return(p);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set persistent flag                                            */
/** @param[in]  tp pointer to the Thread structure                             */
/** @param[in]  p persistent flag                                              */
/*-----------------------------------------------------------------------------*/

void
vexTaskPersistentSet( Thread *tp, bool_t p )
{
    uint16_t    i;

    for(i=0;i<MAX_THREAD;i++)
        {
        // see if we are already in the array
        if( myThreads[ i ].tp == tp)
            {
            myThreads[ i ].persistent = p;
            break;
            }
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Dump the list of registered tasks                              */
/** @param[in]  chp     A pointer to a vexStream object                        */
/** @param[in]  argc    The number of command line arguments                   */
/** @param[in]  argv    An array of pointers to the command line args          */
/*-----------------------------------------------------------------------------*/

void
vexTaskDebug(vexStream *chp, int argc, char *argv[])
{
    uint16_t    i;
    Thread  *tp;

    (void)argc;
    (void)argv;

    for(i=0;i<MAX_THREAD;i++)
        {
        tp = myThreads[ i ].tp;
        if( tp != NULL )
            {
            if( tp->p_name != NULL )
            	vex_chprintf(chp, "%16s ", tp->p_name);
            else
            	vex_chprintf(chp,"                 ");

            vex_chprintf( chp, "%2d: %.8X %d\r\n", i, tp, myThreads[ i ].persistent );
            }
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Stop all tasks                                                 */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Call this to stop all tasks that were registered with vexTaskRegister or
 *  vexTaskRegisterPersistant.  Use as emergency stop, cortex will need rebooting
 */
void
vexTaskEmergencyStop()
{
    uint16_t     i;
    // scrap persistent flag
    for(i=0;i<MAX_THREAD;i++)
        myThreads[ i ].persistent = FALSE;

    vexKillAll = TRUE;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Sleep for given number of ms                                   */
/** @param[in]  msec number of ms to sleep                                     */
/*-----------------------------------------------------------------------------*/
/**
 *  @details
 *  Used as a replacement for chThdSleepMilliseconds so calling thread can
 *  be terminated
 */
void
vexSleep( int32_t msec )
{
    if( (chEvtWaitAnyTimeout( ALL_EVENTS, MS2ST(msec) ) != 0) || chThdShouldTerminate() )
        {
        // We used to lock here, that was incorrect and has been removed
        // we have been asked to terminate either by the THD_TERMINATE flag being set or
        // by an event sent from the task_terminate event source
        uint16_t i;
        for(i=0;i<MAX_THREAD;i++)
            {
            if( myThreads[ i ].tp == chThdSelf() )
                {
                // A persistent thread ?
                if( myThreads[ i ].persistent == TRUE )
                    {
                    // do not terminate unless a real terminate request
                    if(!chThdShouldTerminate())
                        return;
                    }
                // unregister the event listener
                chEvtUnregister( &task_terminate, &myThreads[ i ].el );

                // may have been started by the ROBOTC glue code
                if( CleanupTask )
                    CleanupTask( myThreads[ i ].tp );

                // If terminated rather than event then clear slot
                if(chThdShouldTerminate())
                    myThreads[ i ].tp = (Thread *)0;
                break;
                }
            }

        // terminate ourself
        chThdExit((msg_t)0);
        }
}

/*-----------------------------------------------------------------------------*/
/*  Stack and working area for the user threads, autonomous or drover control  */
/*  only one if these can be active so re-use space                            */
/*-----------------------------------------------------------------------------*/

static  WORKING_AREA(waVexUserTask, USER_TASK_STACK_SIZE);

/*-----------------------------------------------------------------------------*/
/*  Task that runs every 15mS and monitors competition state                   */
/*-----------------------------------------------------------------------------*/

static WORKING_AREA(waVexCortexMonitorTask, MONITOR_TASK_STACK_SIZE);
static msg_t
vexCortexMonitorTask(void *arg)
{
    uint16_t    i;
    Thread  *tp = NULL;
    uint16_t    state;

    (void)arg;

    chRegSetThreadName("monitor");
    chEvtInit(&task_terminate);

    // clear event listeners
    for(i=0;i<MAX_THREAD;i++)
        {
        myThreads[ i ].tp = (Thread *)0;
        myThreads[ i ].persistent = FALSE;
        }

    // wait until all the master cpu resets are done
    // it issues two additional resets after power on
    // at 100mS intervals
    chThdSleepMilliseconds(120);

#ifndef  BOARD_OLIMEX_STM32_P103
    // wait for good spi comms
    while( vexSpiGetOnlineStatus() == 0 )
        {
        // wait for a while
        chThdSleepMilliseconds(100);
        }
#endif

    // another delay
    chThdSleepMilliseconds(2000);

    // pre auton function - may not exist
    if( vexUserInit )
        vexUserInit();

    while (TRUE)
        {
        chThdSleepMilliseconds(16);

        // If enabled
        if( (vexControllerCompetitonState() & kFlagDisabled ) != kFlagDisabled )
            {
            if( (vexControllerCompetitonState() & kFlagAutonomousMode ) != kFlagAutonomousMode )
                {
                // Operator control
                // Start the operator thread at higher than normal priority
                tp = chThdCreateStatic(waVexUserTask, sizeof(waVexUserTask), USER_THREAD_PRIORITY, vexOperator, NULL);
                state = 0;
                }
            else
                {
                // Autonomous
                // Start the operator thread at higher than normal priority
                tp = chThdCreateStatic(waVexUserTask, sizeof(waVexUserTask), USER_THREAD_PRIORITY, vexAutonomous, NULL);
                state = kFlagAutonomousMode;
                }

            // While we are enabled, either auton or operator, wait here unless kill all flag is set
            while( (vexControllerCompetitonState() & (kFlagDisabled | kFlagAutonomousMode)) == state )
               {
               chThdSleepMilliseconds(16);

               // Emergency stop
               if( vexKillAll )
                   break;
               }

            // Broadcast termination event
            chThdSleepMilliseconds(50);
            chSysLock();
            if( chEvtIsListeningI(&task_terminate) )
                chEvtBroadcastI(&task_terminate);
            chSysUnlock();
            chThdSleepMilliseconds(50);

            // wait for termination
            chThdWait( tp );

            // stop all motors
            vexMotorStopAll();

            // wait for all threads to stop
            for(i=0;i<MAX_THREAD;i++)
                {
                if( ( myThreads[ i ].tp != 0 ) && (myThreads[ i ].persistent == FALSE) )
                    {
                    chThdWait( myThreads[ i ].tp );
                    myThreads[ i ].tp = (Thread *)0;
                    }
                }

            // We are done
            while( vexKillAll )
                chThdSleepMilliseconds(50);
            }
        }

    return (msg_t)0;
}


/*-----------------------------------------------------------------------------*/
/*  Task that runs every 15mS that updates the SPI communications buffer and   */
/*  then communicates with the master processor                                */
/*-----------------------------------------------------------------------------*/

static WORKING_AREA(waVexCortexSystemTask, SYSTEM_TASK_STACK_SIZE);
static msg_t
vexCortexSystemTask(void *arg) {
      (void)arg;
      int16_t   m;

      chRegSetThreadName("system");

      // wait until all the master cpu resets are done
      // it issues two additional resets after power on
      // at 100mS intervals
      chThdSleepMilliseconds(120);

      while (TRUE)
          {
          chThdSleepMilliseconds(16);

          // get motor data
          // motor data 1 through 8 goes to spi slots 0 to 7
          for(m=0;m<8;m++)
              vexSpiSetMotor( m, vexMotorGet( m+1 ), vexMotorDirectionGet(m+1) );

          // comms to master
          vexSpiSend();
#ifdef    VEX_WATCHDOG_ENABLE
          vexWatchdogReload();
#endif
          }

      return (msg_t)0;
}

/*-----------------------------------------------------------------------------*/
/**  @brief     Initialize the VEX cortex                                      */
/*-----------------------------------------------------------------------------*/

void
vexCortexInit()
{
    // Init SPI communications
    vexSpiInit();

    // Initialize the motors
    vexMotorInit();

    // Init ADC conversions - No ADC on olimex
#ifndef BOARD_OLIMEX_STM32_P103
    vexAdcInit();
#endif

    // start any test code
    vexTest();

#ifndef USER_UART1_ENABLE
    // Activates the lcd serial driver using custom configuration.
    sdStart(SD_LCD1, &lcd_config);
    vexLcdInit( 0, SD_LCD1 );
    vexLcdPrintf( 0, 0, "ConVEX V%s" , CONVEX_VERSION);
    vexLcdPrintf( 0, 1, "VEX CORTEX LCD1" );
#endif

#ifndef USER_UART2_ENABLE
    // Activates the lcd serial driver using custom configuration.
    sdStart(SD_LCD2, &lcd_config);
    vexLcdInit( 1, SD_LCD2 );
    vexLcdPrintf( 1, 0, "ConVEX V%s" , CONVEX_VERSION);
    vexLcdPrintf( 1, 1, "VEX CORTEX LCD2" );
#endif

    // Init encoder data structures
    vexEncoderInit();

    // Init I2C bus
    i2cInit();
    // Init IMEs
    vexImeInit( &I2CD1, (vexStream *)SD_CONSOLE );

    // call user setup if it has been defined
    if( vexUserSetup )
        vexUserSetup();

    // start interrupts
    vexExtIrqInit();
    // start any encoders
    vexEncoderStartAll();
    // start any sonars
    vexSonarStartAll();
    vexSonarRun();

    // Start any digital pin interrupts
    vexDigitalIntrRun();

#ifdef    VEX_WATCHDOG_ENABLE
    vexWatchdogInit();
#endif

    // Start the system thread at higher than normal priority
    chThdCreateStatic(waVexCortexSystemTask, sizeof(waVexCortexSystemTask), SYSTEM_THREAD_PRIORITY, vexCortexSystemTask, NULL);
    // Start the monitor thread at higher than normal priority
    chThdCreateStatic(waVexCortexMonitorTask, sizeof(waVexCortexMonitorTask), MONITOR_THREAD_PRIORITY, vexCortexMonitorTask, NULL);
}

/*-----------------------------------------------------------------------------*/
/**  @brief     Initialize the console serial port                             */
/*-----------------------------------------------------------------------------*/

void
vexConsoleInit()
{
     // Activates the serial driver using custom configuration.
     sdStart(SD_CONSOLE, &console_config);
}

/*-----------------------------------------------------------------------------*/
/*  Watchdog timer code                                                        */
/** @cond                                                                      */
/*-----------------------------------------------------------------------------*/

// prescaler values for IDWT
#define IWDG_Prescaler_4        (0x00)
#define IWDG_Prescaler_8        (0x01)
#define IWDG_Prescaler_16       (0x02)
#define IWDG_Prescaler_32       (0x03)
#define IWDG_Prescaler_64       (0x04)
#define IWDG_Prescaler_128      (0x05)
#define IWDG_Prescaler_256      (0x06)

/* KR register bit mask */
#define IWDG_WriteAccess_Enable (0x5555)
#define KR_KEY_Reload           (0xAAAA)
#define KR_KEY_Enable           (0xCCCC)

#define IWDT_RESET_MASK         0x20000000
/** @endcond                                                                   */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      Init watchdog timer                                            */
/*-----------------------------------------------------------------------------*/

void
vexWatchdogInit()
{
    IWDG_TypeDef   *p = IWDG;
    // enable register access
    p->KR  = IWDG_WriteAccess_Enable;
    // set prescale to /64
    // clock is 40000/64 = 625Hz
    p->PR  = IWDG_Prescaler_64;
    // set reload counter at 1 second (625/625)
    p->RLR = 625;
    // reload
    p->KR  = KR_KEY_Reload;
    // Start independent watchdog timer
    p->KR  = KR_KEY_Enable;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Reload the IWDG - call several times per second                */
/*-----------------------------------------------------------------------------*/

void
vexWatchdogReload()
{
    IWDG_TypeDef   *p = IWDG;

    // reload watchdog timer
    p->KR = KR_KEY_Reload;
}

/*-----------------------------------------------------------------------------*/
/** @brief     Check to see if the IWDG was the cause of reset                 */
/*  @returns   1 if IWDG caused reset else returns 0                           */
/*-----------------------------------------------------------------------------*/

int16_t
vexWatchdogResetFlagGet()
{
    RCC_TypeDef   *p = RCC;

    // See if we were reset by IWDT
    if( p->CSR &  IWDT_RESET_MASK)
        return(1);
    else
        return(0);
}

