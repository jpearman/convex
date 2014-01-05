/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexsonar.c                                                   */
/*    Author:     James Pearman                                                */
/*    Created:    10 May 2013                                                  */
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
/*                                                                             */
/*    Sonar handling                                                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#include <stdlib.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexsonar.c
  * @brief   Ultrasonic distance sensor driver
*//*---------------------------------------------------------------------------*/

/** @brief  Internal sonar driver states    */
typedef enum {
    kSonarStatePing = 0,
    kSonarStateWait,
    kSonarStateDone,
    kSonarStateError
} tVexSonnarState;;

static  vexSonar_t  vexSonars[kVexSonar_Num];

#ifdef BOARD_OLIMEX_STM32_P103
static  GPTDriver          *sonarGpt = &GPTD4;
#else
static  GPTDriver          *sonarGpt = &GPTD5;
#endif
static  tVexSonarChannel    nextSonar = kVexSonar_1;
static  tVexSonnarState     nextState = kSonarStatePing;
static  Thread             *vexSonarThread = NULL;

// flags
#define SONAR_ENABLED       0x01    ///< flag to indicate sonar is enabled
#define SONAR_INSTALLED     0x02    ///< flag to indicate sonar is installed

/*-----------------------------------------------------------------------------*/
/*  Callback for timer expired                                                 */
/*  We use this to                                                             */
/*  1. time the 10uS pulse to stat the sonar ping                              */
/*  2. as a 40mS timeout if the received echo is not received                  */
/*-----------------------------------------------------------------------------*/

#define SONAR_TIMEOUT   40000       ///< Default timeout for sonar, 40mS

static void
_vs_gpt_cb(GPTDriver *gptp)
{
    (void)gptp;

    chSysLockFromIsr();

    if( nextState == kSonarStatePing )
        {
        nextState = kSonarStateWait;
        vexDigitalPinSet( vexSonars[nextSonar].pa, 0 );
        gptStartOneShotI( sonarGpt, SONAR_TIMEOUT );
        }
    else
    if( nextState == kSonarStateWait )
        {
        vexSonars[nextSonar].time_r = 0;
        vexSonars[nextSonar].time_f = SONAR_TIMEOUT;
        nextState = kSonarStateError;
        }

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/*  Timer config structure                                                     */
/*  1MHz clock                                                                 */
/*-----------------------------------------------------------------------------*/

static const GPTConfig vexSonarGpt = {
    1000000,    /* 1MHz timer clock.*/
    _vs_gpt_cb  /* Timer callback.*/
#if ( CH_KERNEL_VERSION_HEX >= 0x261 )
    ,0          /* DIER = 0, version 2.6.1.and on */
#endif
    };

/*-----------------------------------------------------------------------------*/
/*  Task to poll enabled sonar devices                                         */
/*-----------------------------------------------------------------------------*/

static WORKING_AREA(waVexSonarTask, SONAR_TASK_STACK_SIZE);
static msg_t
VexSonarTask( void *arg )
{
    tVexSonarChannel    c;

    (void)arg;

    chRegSetThreadName("sonar");

    gptStart( sonarGpt, &vexSonarGpt );

    while(!chThdShouldTerminate())
        {
        if( vexSonars[nextSonar].flags == (SONAR_INSTALLED | SONAR_ENABLED) )
            {
            // ping sonar
            vexSonarPing(nextSonar);

            // wait for next time slot
            // the timer is set to timeout in 40mS but we need a 10mS gap before any more
            // pings can be sent
            chThdSleepUntil(chTimeNow() + 50);

            // calculate echo time
            vexSonars[nextSonar].time = vexSonars[nextSonar].time_f - vexSonars[nextSonar].time_r;

            // was the time too great ?
            if( vexSonars[nextSonar].time > 35000 )
                vexSonars[nextSonar].time = -1;

            // if we have a valid time calculate real distance
            if( vexSonars[nextSonar].time != -1 )
                {
                vexSonars[nextSonar].distance_cm = vexSonars[nextSonar].time   / 58;
                vexSonars[nextSonar].distance_inch = vexSonars[nextSonar].time / 148;
                }
            else
                {
                vexSonars[nextSonar].distance_cm = -1;
                vexSonars[nextSonar].distance_inch = -1;
                }

            // look for next sonar
            for(c=kVexSonar_1;c<kVexSonar_Num;c++)
                {
                if( ++nextSonar == kVexSonar_Num )
                    nextSonar = kVexSonar_1;

                // we need sonar to be installed and enabled
                if( vexSonars[nextSonar].flags == (SONAR_INSTALLED | SONAR_ENABLED) )
                    break;
                }
            }
        else
            // Nothing enabled, just wait
            chThdSleepMilliseconds(25);
        }

    return (msg_t)0;
}

/*-----------------------------------------------------------------------------*/
/*  Callback for echo receive pulse                                            */
/*-----------------------------------------------------------------------------*/

static void
_vs_echo_cb(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    if( palReadPad( vexSonars[nextSonar].pb_port,  vexSonars[nextSonar].pb_pad ) )
        vexSonars[nextSonar].time_r = sonarGpt->tim->CNT;
    else
        {
        vexSonars[nextSonar].time_f = sonarGpt->tim->CNT;
        nextState = kSonarStateDone;
        }

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/** @brief      Add a sonar to the array of installed sonars                   */
/** @param[in]  channel The sonar channel                                      */
/** @param[in]  pa The first input pin used by the sonar                       */
/** @param[in]  pb The second input pin used by the sonat                      */
/*-----------------------------------------------------------------------------*/

void
vexSonarAdd( tVexSonarChannel channel, tVexDigitalPin pa, tVexDigitalPin pb )
{
    if( channel >= kVexSonar_Num )
        return;

    if(pa > kVexDigital_12)
        return;
    if(pb > kVexDigital_12)
        return;

    // zero variables

    // setup first pin as output
    vexDigitalModeSet( pa, kVexDigitalOutput);
    vexSonars[channel].pa = pa;
    vexSonars[channel].pa_port   = vexioDefinition[pa].port;
    vexSonars[channel].pa_pad    = vexioDefinition[pa].pad;

    // setup second pin as input
    vexDigitalModeSet( pb, kVexDigitalInput);
    vexSonars[channel].pb = pb;
    vexSonars[channel].pb_port   = vexioDefinition[pb].port;
    vexSonars[channel].pb_pad    = vexioDefinition[pb].pad;
    vexExtSet( vexSonars[channel].pb_port, vexSonars[channel].pb_pad, EXT_CH_MODE_BOTH_EDGES, _vs_echo_cb );

    // installed and enabled
    vexSonars[channel].flags = (SONAR_INSTALLED | SONAR_ENABLED);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start a sonar by enabling the interrupt port                   */
/** @param[in]  channel The sonar channel                                      */
/*-----------------------------------------------------------------------------*/

void
vexSonarStart( tVexSonarChannel channel )
{
    if( vexSonars[channel].flags & SONAR_ENABLED ) {
        // start interrupts
        extChannelEnable( &EXTD1, vexSonars[channel].pb_pad );
    }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Stop a sonar by disabling the interrupt port                   */
/** @param[in]  channel The sonar channel                                      */
/*-----------------------------------------------------------------------------*/

void
vexSonarStop( tVexSonarChannel channel )
{
    if( vexSonars[channel].flags & SONAR_ENABLED ) {
        // start interrupts
        extChannelDisable( &EXTD1, vexSonars[channel].pb_pad );
    }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Enable all installed sonars                                    */
/*-----------------------------------------------------------------------------*/

void
vexSonarStartAll()
{
    tVexSonarChannel    c;

    for(c=kVexSonar_1;c<kVexSonar_Num;c++)
        vexSonarStart( c );
}

/*-----------------------------------------------------------------------------*/
/** @brief     Start the sonar polling task if at least one sonar is installed */
/*-----------------------------------------------------------------------------*/

void
vexSonarRun()
{
    tVexSonarChannel    c;

    // ALready running
    if( vexSonarThread != NULL )
        return;

    // start task if there is a valid sonar installed
    for(c=kVexSonar_1;c<kVexSonar_Num;c++)
        {
        if( vexSonars[c].flags & SONAR_INSTALLED ) {
            // Creates the blinker thread.
            vexSonarThread = chThdCreateStatic(waVexSonarTask, sizeof(waVexSonarTask), SONAR_THREAD_PRIORITY, VexSonarTask, NULL);
            break;
            }
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start 10uS pulse to initiate sonar ping                        */
/** @param[in]  channel The sonar channel                                      */
/** @note       Internal sonar driver function                                 */
/*-----------------------------------------------------------------------------*/

void
vexSonarPing(tVexSonarChannel channel)
{
    if( vexSonars[channel].flags == (SONAR_INSTALLED | SONAR_ENABLED)) {
        vexDigitalPinSet( vexSonars[channel].pa, 1 );
        nextState = kSonarStatePing;
        gptStartOneShot( sonarGpt, 10 );
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get sonar value in cm                                          */
/** @param[in]  channel The sonar channel                                      */
/** @returns    The distance in cm                                             */
/*-----------------------------------------------------------------------------*/

int16_t
vexSonarGetCm( tVexSonarChannel channel )
{
    if( vexSonars[channel].flags & SONAR_INSTALLED ) {
        return( vexSonars[channel].distance_cm );
        }
    else
        return(-1);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get sonar value in inches                                      */
/** @param[in]  channel The sonar channel                                      */
/** @returns    The distance in inches                                         */
/*-----------------------------------------------------------------------------*/

int16_t
vexSonarGetInch( tVexSonarChannel channel )
{
    if( vexSonars[channel].flags & SONAR_INSTALLED ) {
        return( vexSonars[channel].distance_inch );
        }
    else
        return(-1);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Send useful information to debug console                       */
/** @param[in]  chp     A pointer to a vexStream object                      */
/** @param[in]  argc    The number of command line arguments                   */
/** @param[in]  argv    An array of pointers to the command line args          */
/*-----------------------------------------------------------------------------*/

void
vexSonarDebug(vexStream *chp, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    tVexSonarChannel    c;

    for(c=kVexSonar_1;c<kVexSonar_Num;c++)
        {
        vex_chprintf(chp,"S%d %d %5d %5d ", c, vexSonars[c].flags, vexSonars[c].time_r, vexSonars[c].time_f );
        vex_chprintf(chp,"%5d %4d(cm) %3d(inch)\r\n", vexSonars[c].time, vexSonars[c].distance_cm, vexSonars[c].distance_inch );
        }
}


