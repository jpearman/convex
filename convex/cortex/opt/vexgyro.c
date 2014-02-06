/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2012, 2013                                */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexgyro.c                                                    */
/*    Author:     James Pearman                                                */
/*    Created:    9 July 2013                                                  */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     4 July 2013 - Initial release for ChibiOS          */
/*                          28 Jan 2014 - A few small improvements             */
/*                                        to allow different analog ports and  */
/*                                        thread restart.                      */
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

#include <stdlib.h>
#include <math.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header
#include "vexgyro.h"

/*-----------------------------------------------------------------------------*/
/** @file    vexgyro.c
  * @brief   A quick and dirty gyro implementation based on the ROBOTC example
*//*---------------------------------------------------------------------------*/

// going to run the gyro as a user task
static WORKING_AREA(waVexGyroTask, USER_TASK_STACK_SIZE);

// final value in deg * 10
static int32_t     GyroValue = 0;

// filter out noise.
const int GyroJitterRange = 4;

// pointer to our thread so we can kill it
static Thread *gyroThread = NULL;

// the gyro analog port
static  tVexAnalogPin   gyroAnalogPin = kVexAnalog_1;

static msg_t
vexGyroTask(void *arg)
{
    int16_t     i;
    int32_t     GyroBiasAcc = 0;
    int32_t     GyroBias;
    int32_t     GyroSmallBias;
    int32_t     GyroRaw;
    int32_t     GyroRawFiltered = 0;

    int16_t     GyroDelta;
    int32_t     GyroSensorScale = 130;
    int32_t     GyroJitterCycles = 0;

    (void)arg;
    chRegSetThreadName("gyro");

    // find bias
    for(i=0;i<1024;i++)
        {
        GyroBiasAcc = GyroBiasAcc + vexAdcGet( gyroAnalogPin );
        chThdSleepMilliseconds(1);
        }

    GyroBias      = GyroBiasAcc / 1024;
    GyroSmallBias = GyroBiasAcc - (GyroBias * 1024);
    // Ok bias done

    while(!chThdShouldTerminate())
        {
        // Get raw analog value
        GyroRaw   = vexAdcGet( gyroAnalogPin );
        // remove bias
        GyroDelta = GyroRaw - GyroBias;

        // ignore small changes
        if ((GyroDelta < -GyroJitterRange) || (GyroDelta > +GyroJitterRange))
            {
            // integrate angle
            GyroRawFiltered += GyroDelta;

            // compensate for error in bias
            if ((++GyroJitterCycles % 1024) == 0)
                GyroRawFiltered -= GyroSmallBias;
            }

        // calculate angle in deg * 10
        GyroValue = GyroRawFiltered / GyroSensorScale;

        // sleep
        chThdSleepMilliseconds(1);
        }

    return( (msg_t) 0);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the latest gyro value                                      */
/** @returns    The gyro value in degrees * 10                                 */
/*-----------------------------------------------------------------------------*/

int32_t
vexGyroGet()
{
    return( GyroValue );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Init the gyro task                                            */
/*-----------------------------------------------------------------------------*/

void
vexGyroInit( tVexAnalogPin pin )
{
    if( (pin < kVexAnalog_1) || (pin > kVexAnalog_8))
        return;

    gyroAnalogPin = pin;

    gyroThread = chThdCreateStatic(waVexGyroTask, sizeof(waVexGyroTask), USER_THREAD_PRIORITY, vexGyroTask, NULL);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Restart the gyro task                                          */
/*-----------------------------------------------------------------------------*/

void
vexGyroReset()
{
    if( gyroThread != NULL )
        {
        // terminate the gyro task
        chThdTerminate(gyroThread);
        // wait for it to die - 1mS max
        chThdWait(gyroThread);
        
        // restart
        gyroThread = NULL;
        vexGyroInit(gyroAnalogPin);
        }
}
