/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2012                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     SmartMotorLib.c                                              */
/*    Author:     James Pearman                                                */
/*    Created:    2 Oct 2012                                                   */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00  21 Oct 2012 - Initial release                         */
/*                V1.01   7 Dec 2012                                           */
/*                       small bug in SmartMotorLinkMotors                     */
/*                       fix for High Speed 393 Ke constant                    */
/*                       kNumbOfTotalMotors replaced with kVexMotorNum    */
/*                       _Target_Emulator_ defined for versions of ROBOTC      */
/*                       prior to 3.55                                         */
/*                       change to motor enums for V3.60 ROBOTC compatibility  */
/*               V1.02  27 Jan 2013                                            */
/*                      Linking an encoded and non-encoded motor was not       */
/*                      working correctly, added new field to the structure    */
/*                      eport to allow one motor to access the encoder for     */
/*                      another correctly.                                     */
/*               V1.03  10 March 2013                                          */
/*                      Due to new version of ROBOTC (V3.60) detection of PID  */
/*                      version changed. V3.60 was originally planned to have  */
/*                      different motor definitions.                           */
/*                      Added the ability to assign any sensor to be used      */
/*                      for rpm calculation, a bit of a kludge as I didn't     */
/*                      want to add a new variable so reused encoder_id        */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*               V1.10  4 July 2013 - Initial release for ChibiOS              */
/*               V1.11  11 Nov 2013                                            */
/*                      Fix bug when speed limited and changing directions     */
/*                      quickly.                                               */
/*               V1.12  Turbo gear support                                     */
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
/*    Portions of this code are based on work by Chris Siegert aka vamfun on   */
/*    the Vex forums.                                                          */
/*    blog:  vamfun.wordpress.com for model details and vex forum threads      */
/*    email: vamfun@yahoo.com                                                  */
/*    Mentor for team 599 Robodox and team 1508 Lancer Bots                    */
/*                                                                             */
/*    The author can be contacted on the vex forums as jpearman                */
/*    or electronic mail using jbpearman_at_mac_dot_com                        */
/*    Mentor for team 8888 RoboLancers, Pasadena CA.                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Description:                                                             */
/*                                                                             */
/*    This library is designed to work with encoded motors and provides        */
/*    functions to obtain velocity, instantaneous current and estimated PTC    */
/*    temperature.  This data is then used to allow fixed threshold or         */
/*    temperature based current limiting.                                      */
/*                                                                             */
/*    The algorithms used are based on experiments and work done during the    */
/*    summer of 2012 by Chris Siegert and James Pearman.                       */
/*                                                                             */
/*    This library makes extensive use of pointers and therefore needs ROBOTC  */
/*    V3.51 or later.                                                          */
/*                                                                             */
/*    The following vexforum threads have much of the background information   */
/*    used in this library.                                                    */
/*    http://www.vexforum.com/showthread.php?t=72100                           */
/*    http://www.vexforum.com/showthread.php?t=73318                           */
/*    http://www.vexforum.com/showthread.php?t=73960                           */
/*    http://www.vexforum.com/showthread.php?t=74594                           */
/*                                                                             */
/*    Global Memory use for V1.02 is 1404 bytes                                */
/*      1240 for motor data                                                    */
/*       156 for controller data                                               */
/*         8 misc                                                              */
/*                                                                             */
/*    CPU time for SmartMotorTask                                              */
/*    Motor calculations ~ 530uS,  approx 5% cpu bandwidth                     */
/*    Controller calculations with LED status ~ 1.25mS                         */
/*    Worse case is therefore about 1.8mS which occurs every 100mS             */
/*                                                                             */
/*    CPU time for SmartMotorSlewRateTask                                      */
/*    approx 400uS per 15mS loop, about 3% cpu bandwidth                       */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include "smartmotor.h"
#include "robotc_glue.h"
#include "fastmath.c"

/*-----------------------------------------------------------------------------*/
/** @file    smartmotor.c
  * @brief   A port of the smart motor library for ChibiOS/RT
*//*---------------------------------------------------------------------------*/

// storage for all motors
static smartMotor      sMotors[ kVexMotorNum ];
static smartController sPorts[SMLIB_TOTAL_NUM_CONTROL_BANKS];

/*-----------------------------------------------------------------------------*/
/*  Flags to determine behavior of the current limiting                        */
/*-----------------------------------------------------------------------------*/

// flag to hold global status to enable or disable the current limiter
// based on PTC temperature - defaults to off
static short    PtcLimitEnabled = FALSE;

// flag to hold global status to enable or disable the current limiter
// based on preset threshold - defaults to on
static short    CurrentLimitEnabled = FALSE;

static inline float
sgn(float x)
{
    if( x == 0 ) return 0;
    if( x > 0 ) return 1; else return (-1);
}

/*-----------------------------------------------------------------------------*/
/*  External debug function called once per loop with smartMotor ptr           */
/*-----------------------------------------------------------------------------*/
#ifdef  __SMARTMOTORLIBDEBUG__
void    SmartMotorUserDebug( smartMotor *m );
#endif

/*******************************************************************************/
/*******************************************************************************/
/*  PUBLIC FUNCTIONS                                                           */
/*******************************************************************************/
/*******************************************************************************/

/*-----------------------------------------------------------------------------*/
/** @brief      Get pointer to smartMotor structure - not used locally         */
/** @param[in]  index The motor index                                          */
/*              use the _SmartMotorGetPtr instead for functions in this file   */
/*-----------------------------------------------------------------------------*/

smartMotor *
SmartMotorGetPtr( tVexMotor index )
{
    // bounds check index
    if((index < 0) || (index >= kVexMotorNum))
        return( NULL );

    return( &sMotors[ index ] );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get pointer to smartController structure - not used locally    */
/** @param[in]  index The motor index                                          */
/*  use the _SmartMotorControllerGetPtr instead for functions in this file     */
/*-----------------------------------------------------------------------------*/

smartController *
SmartMotorControllerGetPtr( short index )
{
    // bounds check index
    if((index < 0) || (index >= kVexMotorNum))
        return( NULL );

    return( &sPorts[ index ] );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get Motor speed                                                */
/** @param[in]  index The motor index                                          */
/** @returns    The motor speed as rpm                                         */
/*-----------------------------------------------------------------------------*/

float
SmartMotorGetSpeed( tVexMotor index )
{
    // bounds check index
    if((index < 0) || (index >= kVexMotorNum))
        return( 0 );

    return( sMotors[ index ].rpm );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get Motor current                                              */
/** @param[in]  index The motor index                                          */
/** @param[in]  s Set as 1 for signed current otherwise absolute is returned   */
/** @returns    The motor current in amps                                      */
/*-----------------------------------------------------------------------------*/

float
//SmartMotorGetCurrent( tVexMotor index, int s = 0 )
_SmartMotorGetCurrent( tVexMotor index, int s, ... )
{
    // bounds check index
    if((index < 0) || (index >= kVexMotorNum))
        return( 0 );

    // normally return absolute current, s != 0 for signed
    if(s)
        return( sMotors[ index ].current );
    else
        return( fabs( sMotors[ index ].current) );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get Motor temperature                                          */
/** @param[in]  index The motor index                                          */
/** @returns    The motor temperature in deg C                                 */
/*-----------------------------------------------------------------------------*/

float
SmartMotorGetTemperature( tVexMotor index )
{
    // bounds check index
    if((index < 0) || (index >= kVexMotorNum))
        return( 0 );

    return( sMotors[ index ].temperature );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get Motor command limit                                        */
/** @param[in]  index The motor index                                          */
/** @returns    The motor maximum command value                                */
/*-----------------------------------------------------------------------------*/

int
SmartMotorGetLimitCmd( tVexMotor index )
{
    // bounds check index
    if((index < 0) || (index >= kVexMotorNum))
        return( 0 );

    return( sMotors[ index ].limit_cmd );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set Motor current limit                                        */
/** @param[in]  index The motor index                                          */
/** @param[in]  current The maximum motor current                              */
/*-----------------------------------------------------------------------------*/
/** @note
 *  Do not use this function directly, instead use the macro
 *  SmartMotorSetLimitCurent which allows a default value for current of 1.0A
 */
void
//SmartMotorSetLimitCurent( tVexMotor index, float current = 1.0 )
_SmartMotorSetLimitCurent( tVexMotor index, float current, ... )
{
    // bounds check index
    if((index < 0) || (index >= kVexMotorNum))
        return;

    sMotors[ index ].limit_current = current;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set Motor maximum rpm                                          */
/** @param[in]  index The motor index                                          */
/** @param[in]  max_rpm The maximum motor rpm                                  */
/*-----------------------------------------------------------------------------*/

void
SmartMotorSetFreeRpm( tVexMotor index, short max_rpm )
{
    // bounds check index
    if((index < 0) || (index >= kVexMotorNum))
        return;

    smartMotor *m = _SmartMotorGetPtr( index );

    // set the max rpm for this motor
    m->rpm_free = max_rpm;

    // recalculate back emf constant
    m->ke_motor = (7.2*(1-m->i_free/m->i_stall)/ m->rpm_free );

    // recalculate maximum theoretical v_bemf
    m->v_bemf_max = m->ke_motor * m->rpm_free;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set Motor slew rate                                            */
/** @param[in]  index The motor index                                          */
/** @param[in]  slew_rate The maximum motor slew_rate                          */
/*-----------------------------------------------------------------------------*/

void
//SmartMotorSetSlewRate( tVexMotor index, int slew_rate = 10 )
SmartMotorSetSlewRate( tVexMotor index, int slew_rate )
{
    // bounds check index
    if((index < 0) || (index >= kVexMotorNum))
        return;

    // negative or 0 is invalid
    if( slew_rate <= 0 )
        return;

    sMotors[ index ].motor_slew = slew_rate;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get Controller current                                         */
/** @param[in]  index The motor controller index (0, 1 or 2)                   */
/** @returns    The motor controller current in amps                           */
/*-----------------------------------------------------------------------------*/

float
SmartMotorGetControllerCurrent( short index )
{
    // bounds check index
    if((index < 0) || (index >= SMLIB_TOTAL_NUM_CONTROL_BANKS))
        return( 0 );

    return( sPorts[ index ].current );
}
/*-----------------------------------------------------------------------------*/
/** @brief      Get Controller temperature                                     */
/** @param[in]  index The motor controller index (0, 1 or 2)                   */
/** @returns    The motor controller temperature in deg C                      */
/*-----------------------------------------------------------------------------*/

float
SmartMotorGetControllerTemperature( short index )
{
    // bounds check index
    if((index < 0) || (index >= SMLIB_TOTAL_NUM_CONTROL_BANKS))
        return( 0 );

    return( sPorts[ index ].temperature );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set Controller status LED                                      */
/** @param[in]  index The motor controller index (0, 1 or 2)                   */
/** @param[in]  port The digital port that the led is connected to             */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function allows an led to be assigned to a controller bank that
 *  is illuminated when any motor on that bank trips the ptc or exceeds the
 *  preset current limit
 */
void
SmartMotorSetControllerStatusLed( int index, tVexDigitalPin port )
{
    // bounds check index
    if((index < 0) || (index >= SMLIB_TOTAL_NUM_CONTROL_BANKS))
        return;

    if( (port < kVexDigital_1) || (port > kVexDigital_12 ))
        return;

    sPorts[ index ].statusLed = port;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set power expander status port                                 */
/** @param[in]  port The analog port for status from the power expander        */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This allows the battery voltage of the power expander to be monitored.
 *  Monitoring occurs after the ptc in the power expander so can be used to
 *  cut power to those motors if necessary.
 */
void
SmartMotorSetPowerExpanderStatusPort( tVexAnalogPin port )
{
    if( (port < kVexAnalog_1) || (port > kVexAnalog_8 ) )
        return;

    sPorts[ SMLIB_PWREXP_PORT_0 ].statusPort = port;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Enable current limit by monitoring the PTC temperatures        */
/*-----------------------------------------------------------------------------*/

void
SmartMotorPtcMonitorEnable()
{
    CurrentLimitEnabled = FALSE;
    PtcLimitEnabled     = TRUE;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Disable current limit by monitoring the PTC temperatures       */
/*-----------------------------------------------------------------------------*/

void
SmartMotorPtcMonitorDisable()
{
    PtcLimitEnabled     = FALSE;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Enable current limit by using a preset threshold               */
/*-----------------------------------------------------------------------------*/

void
SmartMotorCurrentMonitorEnable()
{
    PtcLimitEnabled     = FALSE;
    CurrentLimitEnabled = TRUE;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Disable current limit by using a preset threshold              */
/*-----------------------------------------------------------------------------*/

void
SmartMotorCurrentMonitorDisable()
{
    CurrentLimitEnabled = FALSE;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start the smart motor monitoring                               */
/** After initialization the smart motor tasks need to be started              */
/*-----------------------------------------------------------------------------*/

void
SmartMotorRun()
{
    // Higher priority than slew rate task
    StartTask( SmartMotorTask , NORMALPRIO + 5 );
    // Higher priority than most user tasks
    StartTask( SmartMotorSlewRateTask, NORMALPRIO + 4 );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Stop smart motor tasks                                         */
/*-----------------------------------------------------------------------------*/

void
SmartMotorStop()
{
    SmartMotorPtcMonitorDisable();
    SmartMotorCurrentMonitorDisable();

    StopTask( SmartMotorTask );
    StopTask( SmartMotorSlewRateTask );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Dump controller and motor status to the debug stream           */
/*-----------------------------------------------------------------------------*/

void
SmartMotorDebugStatus()
{
    short   i, j;
    smartMotor      *m;
    smartController *s;

    // Cortex ports 1 - 5

    for(j=0;j<SMLIB_TOTAL_NUM_CONTROL_BANKS;j++)
        {
        s = _SmartMotorControllerGetPtr( j );
        if( j == SMLIB_CORTEX_PORT_0 )
            vex_printf("Cortex ports 1 - 5  - ");
        if( j == SMLIB_CORTEX_PORT_1 )
            vex_printf("Cortex ports 6 - 10 - ");
        if( j == SMLIB_PWREXP_PORT_0 )
            vex_printf("Power Expander      - ");

        vex_printf("Current:%5.2f ", s->current);
        vex_printf("Temp:%6.2f ", s->temperature);
        vex_printf("Status:%2d ", s->ptc_tripped);
        vex_printf("\r\n");

        for(i=0;i<SMLIB_TOTAL_NUM_BANK_MOTORS;i++)
            {
            if( s->motors[i] != NULL )
                {
                m = s->motors[i];
                vex_printf("      Motor Port: %d - ", m->port );
                vex_printf("Current:%5.2f ", m->current);
                vex_printf("Temp:%6.2f ", m->temperature);
                vex_printf("Status:%2d ", m->ptc_tripped + (m->limit_tripped<<1) );
                vex_printf("\r\n");
                }
            }
        vex_printf("\r\n");
        }
}


/*-----------------------------------------------------------------------------*/
/** @brief      Set motor to value                                             */
/** @param[in]  index The motor index                                          */
/** @param[in]  value The motor control value (speed)                          */
/** @param[in]  immediate If TRUE then bypass the slew rate control            */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Set a new control value for the motor.
 *  This is the only call that should be used to change motor speed.
 *  Using vexMotorSet will completely bugger up the code - do not use it
 *
 *  The naming of this function is inconsistent with the rest of the library
 *  to allow backwards compatibility with team 8888's existing library
 */

void
//SetMotor( int index, int value = 0, bool immediate = FALSE )
_SetMotor( int index, int value, bool_t immediate, ...  )
{
    smartMotor  *m;

    // bounds check index
    if((index < 0) || (index >= kVexMotorNum))
        return;

    // get motor
    m = _SmartMotorGetPtr( index );

    // limit value and set into motorReq
    if( value > SMLIB_MOTOR_MAX_CMD )
        m->motor_cmd = SMLIB_MOTOR_MAX_CMD;
    else
    if( value < SMLIB_MOTOR_MIN_CMD )
        m->motor_cmd = SMLIB_MOTOR_MIN_CMD;
    else
    if( abs(value) >= SMLIB_MOTOR_DEADBAND )
        m->motor_cmd = value;
    else
        m->motor_cmd = 0;

    // new - for hard stop
    if(immediate)
        vexMotorSet( index,  value);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the smartMotor library                              */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Initialize the smart motor library - This function must be called once
 *  when the program starts.  Motors are automatically detected and assigned
 *  to the cortex banls, if a power expander is used then it should be added
 *  by using the SmartMotorsAddPowerExtender function after calling
 *  SmartMotorsInit
 */
/*-----------------------------------------------------------------------------*/

void
SmartMotorsInit()
{
    int         i, j;
    smartMotor  *m;

    // clear controllers
    for(j=0;j<SMLIB_TOTAL_NUM_CONTROL_BANKS;j++)
        {
        for(i=0;i<SMLIB_TOTAL_NUM_BANK_MOTORS;i++)
            sPorts[j].motors[i] = NULL;

        sPorts[j].t_const_1    = SMLIB_C1_CORTEX;
        sPorts[j].t_const_2    = SMLIB_C2_CORTEX;
        sPorts[j].t_ambient    = SMLIB_TEMP_AMBIENT;
        sPorts[j].temperature  = sPorts[j].t_ambient;
        sPorts[j].current      = 0;
        sPorts[j].peak_current = 0;
        sPorts[j].safe_current = SMLIB_I_SAFECORTEX; // cortex and PE the same
        sPorts[j].statusLed    = kVexDigital_None;
        sPorts[j].statusPort   = kVexAnalog_None;
        }


    // unfortunate kludge as getEncoderForMotor will not accept a variable yet
    // update for chibiOS we left in the same format
    sMotors[0].encoder_id = vexMotorEncoderIdGet( kVexMotor_1 );
    sMotors[1].encoder_id = vexMotorEncoderIdGet( kVexMotor_2 );
    sMotors[2].encoder_id = vexMotorEncoderIdGet( kVexMotor_3 );
    sMotors[3].encoder_id = vexMotorEncoderIdGet( kVexMotor_4 );
    sMotors[4].encoder_id = vexMotorEncoderIdGet( kVexMotor_5 );
    sMotors[5].encoder_id = vexMotorEncoderIdGet( kVexMotor_6 );
    sMotors[6].encoder_id = vexMotorEncoderIdGet( kVexMotor_7 );
    sMotors[7].encoder_id = vexMotorEncoderIdGet( kVexMotor_8 );
    sMotors[8].encoder_id = vexMotorEncoderIdGet( kVexMotor_9 );
    sMotors[9].encoder_id = vexMotorEncoderIdGet( kVexMotor_10 );

    for(i=0;i<kVexMotorNum;i++)
        {
        m = _SmartMotorGetPtr( i );
        m->port  = (tVexMotor)i;
        m->eport = (tVexMotor)i;
        m->type  = (tVexMotorType)vexMotorTypeGet( m->port );

        switch( m->type )
            {
            // 393 set for high torque
            case    kVexMotor393T:
                m->i_free   = SMLIB_I_FREE_393;
                m->i_stall  = SMLIB_I_STALL_393;
                m->r_motor  = SMLIB_R_393;
                m->l_motor  = SMLIB_L_393;
                m->ke_motor = SMLIB_Ke_393;
                m->rpm_free = SMLIB_RPM_FREE_393;

                m->ticks_per_rev = SMLIB_TPR_393T;

                m->safe_current = SMLIB_I_SAFE393;

                m->t_const_1 = SMLIB_C1_393;
                m->t_const_2 = SMLIB_C2_393;
                break;

            // 393 set for high speed
            case    kVexMotor393S:
                m->i_free   = SMLIB_I_FREE_393;
                m->i_stall  = SMLIB_I_STALL_393;
                m->r_motor  = SMLIB_R_393;
                m->l_motor  = SMLIB_L_393;
                m->ke_motor = SMLIB_Ke_393/1.6;
                m->rpm_free = SMLIB_RPM_FREE_393 * 1.6;

                m->ticks_per_rev = SMLIB_TPR_393S;

                m->safe_current = SMLIB_I_SAFE393;

                m->t_const_1 = SMLIB_C1_393;
                m->t_const_2 = SMLIB_C2_393;
                break;

            // 393 set for Turbo
            case    kVexMotor393R:
                m->i_free   = SMLIB_I_FREE_393;
                m->i_stall  = SMLIB_I_STALL_393;
                m->r_motor  = SMLIB_R_393;
                m->l_motor  = SMLIB_L_393;
                m->ke_motor = SMLIB_Ke_393/2.4;
                m->rpm_free = SMLIB_RPM_FREE_393 * 2.4;

                m->ticks_per_rev = SMLIB_TPR_393R;

                m->safe_current = SMLIB_I_SAFE393;

                m->t_const_1 = SMLIB_C1_393;
                m->t_const_2 = SMLIB_C2_393;
                break;

            // 269 and 3wire set the same
            case    kVexMotor269:
                m->i_free   = SMLIB_I_FREE_269;
                m->i_stall  = SMLIB_I_STALL_269;
                m->r_motor  = SMLIB_R_269;
                m->l_motor  = SMLIB_L_269;
                m->ke_motor = SMLIB_Ke_269;
                m->rpm_free = SMLIB_RPM_FREE_269;

                m->ticks_per_rev = SMLIB_TPR_269;

                m->safe_current = SMLIB_I_SAFE269;

                m->t_const_1 = SMLIB_C1_269;
                m->t_const_2 = SMLIB_C2_269;
                break;

            default:
                // force OFF
                // Servos, flashlights etc. not considered.
                m->type = kVexMotorUndefined;
                break;
            }

        // Override encoder ticks if not an IME
        if( m->encoder_id < 0 )
            {
            // No encoder
            m->ticks_per_rev = -1;
            m->enc    = 0;
            m->oldenc = 0;
            }
        else
        if( m->encoder_id < 20 ) {
            // quad encoder
            m->ticks_per_rev = SMLIB_TPR_QUAD;
            m->enc    = vexMotorPositionGet( m->eport );
            m->oldenc = vexMotorPositionGet( m->eport );
            }
        else
            {
            m->enc    = vexMotorPositionGet( m->eport );
            m->oldenc = vexMotorPositionGet( m->eport );
            }

        // use until overidden by user
        m->t_ambient = SMLIB_TEMP_AMBIENT;
        m->temperature = m->t_ambient;

        // default for target is safe
        m->target_current = m->safe_current;
        // default for limit is safe
        m->limit_current  = m->safe_current;

        // reset stats
        m->peak_current = 0;

        // we are good to go
        m->limit_tripped = FALSE;
        m->ptc_tripped   = FALSE;
        m->limit_cmd     = SMLIB_MOTOR_MAX_CMD_UNDEFINED;

        // maximum theoretical v_bemf
        m->v_bemf_max = m->ke_motor * m->rpm_free;

        // add to controller
        if( m->type != kVexMotorUndefined )
            {
            if( i < SMLIB_TOTAL_NUM_BANK_MOTORS )
                sPorts[SMLIB_CORTEX_PORT_0].motors[i] = m;
            else
                sPorts[SMLIB_CORTEX_PORT_1].motors[i-SMLIB_TOTAL_NUM_BANK_MOTORS] = m;
            }

        // which bank are we on
        if( i < SMLIB_TOTAL_NUM_BANK_MOTORS )
            m->bank = &sPorts[SMLIB_CORTEX_PORT_0];
        else
            m->bank = &sPorts[SMLIB_CORTEX_PORT_1];

        // we have never run
        m->lastPgmTime = -1;
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Link two motors                                                */
/** @param[in]  master The master motor (usually with an encoder)              */
/** @param[in]  slave The slave motor (usually un-encoded)                     */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This lets us use encoder counts from one motor for another motor.
 */

void
SmartMotorLinkMotors( tVexMotor master, tVexMotor slave )
{
    // bounds check master
    if((master < 0) || (master >= kVexMotorNum))
        return;

    // bounds check master
    if((slave  < 0) || (slave  >= kVexMotorNum))
        return;

    // get motor pointers for master ans slave
    smartMotor  *m = _SmartMotorGetPtr( master );
    smartMotor  *s = _SmartMotorGetPtr( slave  );

    // master need to have an encoder
    if( m->encoder_id == (-1) )
        return;

    // encoder port is the master port
    s->eport         = m->port;

    // link, assume 1:1 gearing
    s->ticks_per_rev = m->ticks_per_rev;
    s->encoder_id    = m->encoder_id;
    s->enc           = m->enc;
    s->oldenc        = m->oldenc;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the encoder gearing                                        */
/** @param[in]  index The motor index                                          */
/** @param[in]  ratio The gear ratio                                           */
/*-----------------------------------------------------------------------------*/
/** @details
 *  If using a quad encoder then it may not be directly connected to the motor.
 *  This function allows a change of gearing by changing the ticks per rpm.
 *  Only call this function once per motor after calling init
 *
 *  ratio is encoder revs/motor revs
 */
void
SmartMotorsSetEncoderGearing( tVexMotor index, float ratio )
{
    // bounds check master
    if((index < 0) || (index >= kVexMotorNum))
        return;

    smartMotor  *m = _SmartMotorGetPtr( index );

    // Not for IMEs or non encoded motors
    if( (m->encoder_id >= 0) && (m->encoder_id < 20))
        m->ticks_per_rev = m->ticks_per_rev * ratio;
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/** @brief      Add a power extender to the system                             */
/** @param[in]  p0 The first power expander motor port                         */
/** @param[in]  p1 The second power expander motor port                        */
/** @param[in]  p2 The third power expander motor port                         */
/** @param[in]  p3 The fourth power expander motor port                        */
/*  Move the motors currently assigned to the cortex to the power extender     */
/*  Only call this function once after SmartMotorInit                          */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function reassigns up to four motors to a power expander
 *  rather than ports on the cortex.
 *
 *  There is a macro SmartMotorsAddPowerExtender for ROBOTC compatibility
 *
 *  @code
 *  // the motor on port4 is assigned to the power expander
 *  SmartMotorsAddPowerExtender( kVexMotor_4 );
 *
 *  // three motors are assigned to the power expander
 *  SmartMotorsAddPowerExtender( ArmMotorA, ArmMotorB, ArmMotorC );
 *  @endcode
 */

void
//SmartMotorsAddPowerExtender( int p0, int p1 = (-1), int p2 = (-1), int p3 = (-1) )
_SmartMotorsAddPowerExtender( int p0, int p1, int p2, int p3, ... )
{
    int     i;
    int     p = 0;
    smartMotor  *m;

    for(i=0;i<kVexMotorNum;i++)
        {
        m = _SmartMotorGetPtr( i );

        // has to be a real motor
        // ignore ports 1 & 10 which are 2 wire ports
        if( (m->type != kVexMotorUndefined) && (m->port > kVexMotor_1) && (m->port < kVexMotor_10) )
            {
            // assigning this port to the power expander ?
            if( (i==p0) || (i==p1) || (i==p2) || (i==p3))
                {
                // link motor to power expander bank
                sPorts[SMLIB_PWREXP_PORT_0].motors[p] = m;
                m->bank = &sPorts[SMLIB_PWREXP_PORT_0];
                p++;

                // remove from cortex
                if( i < SMLIB_TOTAL_NUM_BANK_MOTORS )
                    sPorts[SMLIB_CORTEX_PORT_0].motors[i] = NULL;
                else
                    sPorts[SMLIB_CORTEX_PORT_1].motors[i-SMLIB_TOTAL_NUM_BANK_MOTORS] = NULL;
                }
            }
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Assign (non encoder) sensor for rpm measurement                */
/** @param[in]  index The motor index                                          */
/** @param[in]  port The sensor port                                           */
/** @param[in]  ticks_per_rev ticks per rovolution for this sensor             */
/** @param[in]  reversed A bool indicating motor direction is reversed         */
/*  Assign a sensor to be used to calculate rpm for a motor                    */
/*  This is a special case and is used for, example, when a pot is geared      */
/*  to a lift motor                                                            */
/*                                                                             */
/*  ticks_per_rev for a pot geared 1:1 would be about 6000                     */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function can be used to assign any sensor to a motor for calculating rpm,
 *  the most likely situation would be a potentiometer.  The parameter
 *  ticks_per_rev is the sensor change for one revolution of the motor,
 *  for example, a potentiometer with 1:1 gearing has about 6000 ticks_per_rev,
 *  if the gearing was 7:1 then this would be reduced to about 857.
 *  Set reversed to true if the potentiometer value decreases with positive
 *  motor command values.
 */

/** @cond */
#define ENCODER_ID_SENSOR   0x800
/** @endcond */

void
//SmartMotorSetRpmSensor( tVexMotor index, tSensors port, float ticks_per_rev, bool reversed = FALSE )
_SmartMotorSetRpmSensor( tVexMotor index, tVexAnalogPin port, float ticks_per_rev, bool_t reversed, ... )
{
    // bounds check master
    if((index < 0) || (index >= kVexMotorNum))
        return;

    smartMotor  *m = _SmartMotorGetPtr( index );

    // This could be an analog port or (unlikely) an encoder
    // that was not defined in the motors&sensors setup

    if( (port >= kVexAnalog_1) && (port <= kVexAnalog_8 ) )
        {
        m->encoder_id    = ENCODER_ID_SENSOR + (short)port;
        m->ticks_per_rev = ticks_per_rev;
        // use negative ticks per rev if reversed sensor
        if( reversed )
            m->ticks_per_rev = -m->ticks_per_rev;
        }
}

/*******************************************************************************/
/*******************************************************************************/
/*  PRIVATE FUNCTIONS                                                          */
/*******************************************************************************/
/*******************************************************************************/
//
// No bounds checking in the private functions
//

/*-----------------------------------------------------------------------------*/
/** @brief      calculate speed in rpm for the motor                           */
/** @param[in]  m Pointer to smartMotor structure                              */
/** @param[in]  deltaTime The time in mS from the last call to this function   */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/*-----------------------------------------------------------------------------*/

#ifdef _Target_Emulator_
    float   motordrag = 1.0;
#endif

void
SmartMotorSpeed( smartMotor *m, int deltaTime )
{
#ifdef _Target_Emulator_
    // dummy increment based on speed
    int increment = m->ticks_per_rev * 0.2 * motordrag;
    // cap at 100 rpm
    if( abs(increment) > 100)
        increment = sgn(increment) * 100;
    // increase(or decrease) for testing in emulator
    m->enc = m->enc += increment; // debug
#else
    // Get encoder value
    m->enc = vexMotorPositionGet(m->eport);
#endif

    // calculate encoder delta
    m->delta  = m->enc - m->oldenc;
    m->oldenc = m->enc;

    // calculate the rpm for the motor
    m->rpm = (1000.0/deltaTime) * m->delta * 60.0 / m->ticks_per_rev;
}

/*-----------------------------------------------------------------------------*/
/** @brief      calculate a simulated speed in rpm for a motor                 */
/** @param[in]  m Pointer to smartMotor structure                              */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/*-----------------------------------------------------------------------------*/
/** @note
  * We needed to do something for motors which do not have encoders
  * and are also not coupled to another motor and able to share its encoder
  */

void
SmartMotorSimulateSpeed( smartMotor *m )
{
    static  short   spd_table[10] = {0, 26, 58, 77, 85, 92, 95, 98, 100, 100};
            short   cmd;
            short   index;
            float   f, scale, speed;

    // get estimated speed from table using bilinear interpolation
    // and simple speed LUT
    cmd   =  vexMotorGet( m->port );
    // index to tabke
    index =  abs( cmd ) >> 4; // div by 16
    // fractional part indicates where we are between two table values
    f     = (abs( cmd ) - (index << 4)) / 16.0;

    // assume motor is running at 90% of commanded speed
    // we really have no idea and if we set this too slow it will
    // just trip the current limiters.
    scale = m->rpm_free / 100.0 * 0.90;
    speed = spd_table[index] + (spd_table[index+1] - spd_table[index]) * f;

    m->rpm = sgn(cmd) * (speed * scale);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Calculate speed using sensor rather than encoder for motor     */
/** @param[in]  m Pointer to smartMotor structure                              */
/** @param[in]  deltaTime The time in mS from the last call to this function   */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/*-----------------------------------------------------------------------------*/

static void
SmartMotorSensorSpeed( smartMotor *m, int deltaTime )
{
    tVexAnalogPin    port;

    // get port from special encoder_id
    port = (tVexAnalogPin)(m->encoder_id - ENCODER_ID_SENSOR);
    if( (port < kVexAnalog_1) || (port > kVexAnalog_8 ) )
        return;

    // Get sensor value
    m->enc = vexAdcGet(port);

    // calculate encoder delta
    m->delta  = m->enc - m->oldenc;
    m->oldenc = m->enc;

    // calculate the rpm for the motor
    m->rpm = (1000.0/deltaTime) * m->delta * 60.0 / m->ticks_per_rev;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Estimate smart motor current                                   */
/** @param[in]  m Pointer to smartMotor structure                              */
/** @param[in]  v_battery The battery voltage in volts                         */
/** @returns    The calculated current                                         */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Estimate current in Vex motor using vamfun's algorithm.\n
 *  subroutine written by Vamfun...Mentor Vex 1508, 599.\n
 *  7.13.2012  vamfun@yahoo.com... blog info  http://vamfun.wordpress.com\n
 *
 *  Modified by James Pearman 7.28.2012.\n
 *  Modified by James Pearman 10.1.2012 - more generalized code.\n
 *
 *  If cmd is positive then rpm must also be positive for this to work.
 */

float
SmartMotorCurrent( smartMotor *m, float v_battery  )
{
    float   v_bemf;
    float   c1, c2;
    float   lamda;

    float   duty_on, duty_off;

    float   i_max, i_bar, i_0;
    float   i_ss_on, i_ss_off;

    int     dir;

    // get current cmd
    int     cmd = vexMotorGet( m->port );

    // rescale control value
    // ports 2 through 9 behave a little differently
    if( m->port > kVexMotor_1 && m->port < kVexMotor_10 )
        cmd = (cmd * 128) / 90;

    // clip control value to +/- 127
    if( abs(cmd) > 127 )
        cmd = sgn(cmd) * 127;

    // which way are we turning ?
    // modified to use rpm near command value of 0 to reduce transients
    if( abs(cmd) > 10 )
        dir = sgn(cmd);
    else
        dir = sgn(m->rpm);


    duty_on = abs(cmd)/127.0;

    // constants for this pwm cycle
    lamda = m->r_motor/((float)SMLIB_PWM_FREQ * m->l_motor);
    c1    = fastexp( -lamda *    duty_on  );
    c2    = fastexp( -lamda * (1-duty_on) );

    // Calculate back emf voltage
    v_bemf  = m->ke_motor * m->rpm;

    // new - clip v_bemf, stops issues if motor runs faster than rpm_free
    if( fabs(v_bemf) > m->v_bemf_max )
        v_bemf = sgn(v_bemf) * m->v_bemf_max;

    // Calculate staady state current for on and off pwm phases
    i_ss_on  =  ( v_battery * dir - v_bemf ) / (m->r_motor + SMLIB_R_SYS);
    i_ss_off = -( SMLIB_V_DIODE   * dir + v_bemf ) /  m->r_motor;

    // compute trial i_0
    i_0 = (i_ss_on*(1-c1)*c2 + i_ss_off*(1-c2))/(1-c1*c2);

    //check to see if i_0 crosses 0 during off phase if diode were not in circuit
    if(i_0*dir < 0)
        {
        // waveform reaches zero during off phase of pwm cycle hence
        // ON phase will start at 0
        // once waveform reaches 0, diode clamps the current to zero.
        i_0 = 0;

        // peak current
        i_max = i_ss_on*(1-c1);

        //where does the zero crossing occur
        duty_off = -fastlog(-i_ss_off/(i_max-i_ss_off))/lamda ;
        }
    else
        {
        // peak current
        i_max = i_0*c1 + i_ss_on*(1-c1);

        // i_0 is non zero so final value of waveform must occur at end of cycle
        duty_off = 1 - duty_on;
        }

    // Average current for cycle
    i_bar = i_ss_on*duty_on + i_ss_off*duty_off;

    // Save current
    m->current = i_bar;

    // simple iir filter to remove transients
    m->filtered_current = (m->filtered_current * 0.8) + (i_bar * 0.2);

    // peak current - probably not useful
    if( fabs(m->current) > m->peak_current )
        m->peak_current = fabs(m->current);

    return i_bar;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Calculate the current for a controller bank                    */
/** @param[in]  s A pointer to a smartController structure                     */
/** @returns    The calculated current                                         */
/*  @note  A bank is ports 1-5, ports 6-10 on the cortex or a power expander   */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/*-----------------------------------------------------------------------------*/

float
SmartMotorControllerCurrent( smartController *s )
{
    int     i;

    s->current = 0;

    // Controller current wil always be positive
    // always flows from battery :)
    for(i=0;i<SMLIB_TOTAL_NUM_BANK_MOTORS;i++)
        {
        if( s->motors[i] != NULL )
            {
            if( s->motors[i]->type != kVexMotorUndefined )
                s->current += fabs( s->motors[i]->current );
            }
        }

    // peak current - probably no use
    if( s->current > s->peak_current )
        s->peak_current = s->current;

    return( s->current );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Calculate safe current command                                 */
/** @param[in]  m Pointer to smartMotor structure                              */
/** @param[in]  v_battery The battery voltage in volts                         */
/** @returns    The calculated command value                                   */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Calculate a command for the motor which will result in a target
 *  current based on motor speed.
 *
 *  target_current should be set prior to calling the function and must
 *  be POSITIVE.
 *
 *  If command direction and rpm are not of the same polarity then this
 *  function tends to fallover due to back emf being of opposite direction
 *  to the drive direction.  In this situation we have no choice but to
 *  let current go higher as the motor is changing directions and will in
 *  effect stall as it does through zero.
 */

int
SmartMotorSafeCommand( smartMotor *m, float v_battery  )
{
    // get current cmd
    int     cmd = vexMotorGet( m->port );

    // cmd polarity must match rpm polarity
    // broken up a bit to reduce multiplies
    if (cmd >= 0)
        {
        if (m->rpm >= 0)
            cmd = SMLIB_MOTOR_MAX_CMD * ((m->rpm * m->ke_motor) + (m->target_current * (m->r_motor + SMLIB_R_SYS)) + SMLIB_V_DIODE) / ( v_battery + SMLIB_V_DIODE );
        else
            cmd = SMLIB_MOTOR_MAX_CMD;

        // clip
        if( cmd > SMLIB_MOTOR_MAX_CMD )
            cmd = SMLIB_MOTOR_MAX_CMD;
        }
    else
        {
        if(m->rpm <= 0)
            cmd = SMLIB_MOTOR_MAX_CMD * ((m->rpm * m->ke_motor) - (m->target_current * (m->r_motor + SMLIB_R_SYS)) - SMLIB_V_DIODE) / ( v_battery + SMLIB_V_DIODE );
        else
            cmd = SMLIB_MOTOR_MIN_CMD;

        // clip
        if( cmd < SMLIB_MOTOR_MIN_CMD )
            cmd = SMLIB_MOTOR_MIN_CMD;
        }

    // override if current is 0
    if( m->target_current == 0 )
        cmd = 0;

    // ports 2 through 9 behave a little differently
    if( m->port > kVexMotor_1 && m->port < kVexMotor_10 )
        cmd = (cmd * 90) / 128;

    return( cmd );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Calculate the PTC temperature for a motor                      */
/** @param[in]  m Pointer to smartMotor structure                              */
/** @param[in]  deltaTime The time in mS from the last call to this function   */
/** @returns    The calculated ptc temperature                                 */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/** @note       math by vamfun                                                 */
/*-----------------------------------------------------------------------------*/

float
SmartMotorTemperature( smartMotor *m, int deltaTime )
{
    float   rate;

    rate = m->t_const_2 * (m->current * m->current * m->t_const_1 - (m->temperature - m->t_ambient));

    m->temperature = m->temperature + (rate * deltaTime);

    return( m->temperature );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Calculate the PTC temperature for a controller bank            */
/** @param[in]  s Pointer to smartController structure                         */
/** @param[in]  deltaTime The time in mS from the last call to this function   */
/** @returns    The calculated ptc temperature                                 */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/** @note       math by vamfun                                                 */
/*-----------------------------------------------------------------------------*/

float
SmartMotorControllerTemperature( smartController *s, int deltaTime  )
{
    float   rate;

    rate = s->t_const_2 * (s->current * s->current * s->t_const_1 - (s->temperature - s->t_ambient));

    s->temperature = s->temperature + (rate * deltaTime);

    return( s->temperature );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Monitor Motor PTC temperature                                  */
/** @param[in]  m Pointer to smartMotor structure                              */
/** @param[in]  v_battery The battery voltage in volts                         */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/*-----------------------------------------------------------------------------*/
/** @details
 *  If the temperature is above the set point then calculate a command that
 *  will result in a safe current
 */

void
SmartMotorMonitorPtc( smartMotor *m, float v_battery )
{
    if( !m->ptc_tripped ) {
        if( m->temperature > SMLIB_TEMP_TRIP )
            m->ptc_tripped = TRUE;
    }
    else {
        // 10 deg hysterisis
        if( m->temperature < (SMLIB_TEMP_TRIP - SMLIB_TEMP_HYST) )
            m->ptc_tripped = FALSE;
    }

    // Is the bank ptc tripped ?
    // If so then leave limit_cmd alone
    if( m->bank != NULL )
        {
        if( m->bank->ptc_tripped )
            return;
        }

    // Is (or was) the ptc tripped
    if( m->ptc_tripped )
        {
        // we are using target_current as a debugging means
        // it must be positive
        m->target_current = m->safe_current;
        // maximum cmd value
        m->limit_cmd = SmartMotorSafeCommand( m, v_battery);
        }
    else
        {
        // allow max speed
        m->limit_cmd = SMLIB_MOTOR_MAX_CMD_UNDEFINED;
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Monitor controller bank PTC temperature                        */
/** @param[in]  s Pointer to smartController structure                         */
/** @param[in]  v_battery The battery voltage in volts                         */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/*-----------------------------------------------------------------------------*/
/** @details
 *  If the temperature is above the set point then calculate a commands for
 *  each motor that will result in a safe current.
 *
 */

void
SmartMotorControllerMonitorPtc( smartController *s, float v_battery )
{
    smartMotor    *m;
    int            i;

    if( !s->ptc_tripped ) {
        if( s->temperature > SMLIB_TEMP_TRIP )
            s->ptc_tripped = TRUE;
    }
    else {
        // 10 deg hysterisis
        if( s->temperature < (SMLIB_TEMP_TRIP - SMLIB_TEMP_HYST) )
            s->ptc_tripped = FALSE;
    }

    // Is (or was) the PTC tripped
    // this will constantly be recalculated
    if( s->ptc_tripped )
        {
        // now decide how to fix it.
        // divide amongst active motors, same current for each one we are using
        // type does not matter
        int active_motors = 0;
        for(i=0;i<SMLIB_TOTAL_NUM_BANK_MOTORS;i++)
            {
            m = s->motors[i];
            if( m != NULL )
                {
                if( fabs(m->current) > 0.1 )
                    active_motors++;
                }
            }

        // avoid divide by 0
        if( active_motors == 0)
            active_motors = 1;

        // calculate safe current based on number of active motors
        float m_safe_current = s->safe_current / active_motors;

        for(i=0;i<SMLIB_TOTAL_NUM_BANK_MOTORS;i++)
            {
            m = s->motors[i];
            if( m != NULL )
                {
                // using target_current as a debugging means
                // it must be positive
                // see if the motor is tripped as well and use lowest current
                if( m->ptc_tripped && (m->safe_current < m_safe_current) )
                    m->target_current = m->safe_current;
                else
                    m->target_current = m_safe_current;

                m->limit_cmd = SmartMotorSafeCommand( m, v_battery );
                }
            }
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Monitor Motor Current                                          */
/** @param[in]  m Pointer to smartMotor structure                              */
/** @param[in]  v_battery The battery voltage in volts                         */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/*-----------------------------------------------------------------------------*/

void
SmartMotorMonitorCurrent( smartMotor *m, float v_battery )
{
    m->target_current = m->limit_current;

    // The way this is setup is that if the limit is tripped you
    // will probably need to back off the controls or allow the motor to speed up
    // before the limit is cancelled.  This is to stop oscilation.
    // The 0.9 below controls this behavior.
    if( !m->limit_tripped ) {
        if( fabs(m->filtered_current) > m->target_current )
            m->limit_tripped = TRUE;
    }
    else {
        if( fabs(m->filtered_current) < (m->target_current * 0.9) )
            m->limit_tripped = FALSE;
    }

    if( m->limit_tripped )
        // maximum cmd value
        m->limit_cmd = SmartMotorSafeCommand( m, v_battery);
    else
        m->limit_cmd = SMLIB_MOTOR_MAX_CMD_UNDEFINED;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the current monitor LED on or off                          */
/** @param[in]  s Pointer to smartController structure                         */
/** @warning    Internal smartMotorLibrary function, do not call, ref only     */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Turn on status LED if either controller ptc is tripped or any connected
 *  motors are tripped.
 *  LED is only turned on in this function as it may be shared between more
 *  then one controller.  The LED is briefly turned off each loop around the
 *  calling task but only remains off for a very short period which is
 *  inperceptable.
 */

void
SmartMotorControllerSetLed( smartController *s )
{
    int         i;
    smartMotor *m;
    int         status;

    if(s->statusLed < 0)
        return;

    status = s->ptc_tripped;

    for(i=0;i<SMLIB_TOTAL_NUM_BANK_MOTORS;i++)
        {
        m = s->motors[i];

        if( m != NULL )
            status += (m->ptc_tripped + m->limit_tripped);
        }

    if(status)
        vexDigitalPinSet(s->statusLed,  SMLIB_LEDON);
}

/*-----------------------------------------------------------------------------*/
/** @brief      The smart motor task                                           */
/** @param[in]  arg pointer to user data (not used)                            */
/*-----------------------------------------------------------------------------*/
/** @note
 *  Running a little different in this version, instead of a 100mS delay
 *  and then calculations on each motor we do one motor each iteration
 */

msg_t
SmartMotorTask( void *arg )
{
    static  int loopDelay = 10;
    static  int delayTimeMs = kVexMotorNum * 10;
            int i;
            int nextMotor = 0;
            float   v_battery;

    (void)arg;

    // Must call this - but we are not terminated
    vexTaskRegisterPersistant("smartMotor", TRUE);

    while(!chThdShouldTerminate())
        {
#ifdef  _smTestPoint_1
        // debug time spent in this task
        vexDigitalPinSet( _smTestPoint_1, 1);
#endif

        v_battery = vexSpiGetMainBattery()/1000.0;

        smartMotor *m = _SmartMotorGetPtr( nextMotor );

        // may be overkill, could just use default from above
        delayTimeMs = chTimeNow()  - m->lastPgmTime;
        m->lastPgmTime = chTimeNow() ;
        m->delayTimeMs = delayTimeMs; // debug

        // Set current etc. for one motor if it exists and has an encoder
        if( m->type != kVexMotorUndefined )
            {
            if( m->encoder_id >= 0)
                {
                if( m->encoder_id < ENCODER_ID_SENSOR )
                    SmartMotorSpeed( m, delayTimeMs );
                else
                    SmartMotorSensorSpeed( m, delayTimeMs );
                }
            else
                SmartMotorSimulateSpeed( m );

            SmartMotorCurrent( m, v_battery );
            SmartMotorTemperature( m, delayTimeMs );
            if( PtcLimitEnabled )
                SmartMotorMonitorPtc( m, v_battery );
            if( CurrentLimitEnabled )
                SmartMotorMonitorCurrent( m, v_battery );
            }

#ifdef  __SMARTMOTORLIBDEBUG__
        // Call user debug code
        SmartMotorUserDebug( m );
#endif
        // next motor
        if(++nextMotor == kVexMotorNum)
            {
            nextMotor = 0;

            // now set cortext current
            // this is much quicker than setting the motor currents so do all
            // three ports, cortex and power expander.
            for( i=0;i<SMLIB_TOTAL_NUM_CONTROL_BANKS;i++ )
                {
                smartController *s = _SmartMotorControllerGetPtr( i );

                SmartMotorControllerCurrent( s );
                SmartMotorControllerTemperature( s, delayTimeMs );

                if( PtcLimitEnabled )
                    SmartMotorControllerMonitorPtc( s, v_battery );

                // turn off status leds here, more than one controller may
                // share an led so we turn them off each loop
                // and then any tripped controller may turn them on.
                if( s->statusLed >= 0 )
                    vexDigitalPinSet( s->statusLed, SMLIB_LEDOFF);
                }

            // check status LED
            for( i=0;i<SMLIB_TOTAL_NUM_CONTROL_BANKS;i++ )
                {
                smartController *s = _SmartMotorControllerGetPtr( i );
                if( s->statusLed >= 0 )
                    SmartMotorControllerSetLed(s);
                }

            // Monitor power expander status port
            smartController *s = _SmartMotorControllerGetPtr( SMLIB_PWREXP_PORT_0 );
            if( s->statusPort >= 0 )
                {
                // assume A2 power expander
                float pe_battery = vexAdcGet( s->statusPort ) / 270.0;
                // Use 3 volts as threshold, should work for old and new power expanders
                if( pe_battery < 3.0 )
                    {
                    // tripped - bad !
                    s->temperature  = 110;
                    // drop safe current forever to 0
                    s->safe_current = 0;
                    }
                }
            }

#ifdef  _smTestPoint_1
        // debug time spent in this task
        vexDigitalPinSet( _smTestPoint_1, 0);
#endif
        // wait
        vexSleep(loopDelay);
        }

    return (msg_t)0;
}


/*-----------------------------------------------------------------------------*/
/** @brief      The motor slew rate task                                       */
/** @param[in]  arg pointer to user data (not used)                            */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Task  - compares the requested speed of each motor to the actual speed
 *          and increments or decrements to reduce the difference as necessary
 *
 */

msg_t SmartMotorSlewRateTask( void *arg )
{
    static  int delayTimeMs = 15;
    int motorIndex;
    int motorTmp;
    smartMotor  *m;

    (void)arg;

    // Must call this - but we are not terminated
    vexTaskRegisterPersistant("slewMotor", TRUE);

    // Initialize stuff
    for(motorIndex=0;motorIndex<kVexMotorNum;motorIndex++)
        {
        m = _SmartMotorGetPtr( motorIndex );
        m->motor_req  = 0;
        m->motor_cmd  = 0;
        m->motor_slew = SMLIB_MOTOR_DEFAULT_SLEW_RATE;
        }

    // run task until stopped
    while( !chThdShouldTerminate() )
        {
#ifdef  _smTestPoint_2
        // debug time spent in this task
        vexDigitalPinSet( _smTestPoint_2, 1);
#endif
        // run loop for every motor
        for( motorIndex=0; motorIndex<kVexMotorNum; motorIndex++)
            {
            m = _SmartMotorGetPtr( motorIndex );

            // So we don't keep accessing the internal storage
            motorTmp = vexMotorGet( m->port );

            // check for limiting
            if( (PtcLimitEnabled || CurrentLimitEnabled) && (m->limit_cmd != SMLIB_MOTOR_MAX_CMD_UNDEFINED) )
                {
                if( abs(m->motor_cmd) > abs(m->limit_cmd) ) {
                    // don't limit if we are reversing direction
                    if( sgn(m->motor_cmd) == sgn(m->limit_cmd) )
                        m->motor_req = m->limit_cmd;
                    else
                        m->motor_req = m->motor_cmd;
                    }
                else
                    m->motor_req = m->motor_cmd;
                }
            else
                m->motor_req = m->motor_cmd;

            // Do we need to change the motor value ?
            if( motorTmp != m->motor_req )
                {
                // increasing motor value
                if( m->motor_req > motorTmp )
                    {
                    motorTmp += m->motor_slew;
                    // limit
                    if( motorTmp > m->motor_req )
                        motorTmp = m->motor_req;
                    }

                // increasing motor value
                if( m->motor_req < motorTmp )
                    {
                    motorTmp -= m->motor_slew;
                    // limit
                    if( motorTmp < m->motor_req )
                        motorTmp = m->motor_req;
                    }

                // finally set motor
                vexMotorSet( m->port, motorTmp);
                }
            }

#ifdef  _smTestPoint_2
        // debug time spent in this task
        vexDigitalPinSet( _smTestPoint_2, 0);
#endif
        // Wait approx the speed of motor update over the spi
        vexSleep( delayTimeMs );
        }

    return (msg_t)0;
}


