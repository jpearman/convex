/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2012, 2013                                */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     pidlib.c                                                     */
/*    Author:     James Pearman                                                */
/*    Created:    24 Oct 2012                                                  */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     4 July 2013 - Initial release for ChibiOS          */
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

#include <stdlib.h>
#include <math.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"

#include "pidlib.h"
#include "fastmath.c"

/*-----------------------------------------------------------------------------*/
/** @file    pidlib.c
  * @brief   A port of the ROBOTC pidlib library
*//*---------------------------------------------------------------------------*/

// static storage - more portable
#ifndef PIDLIB_USE_DYNAMIC
static  pidController   _pidControllers[ MAX_PID ];
#endif

static  int16_t          nextPidControllerPtr = 0;

static  int16_t          PidDriveLut[PIDLIB_LUT_SIZE];

// There is no sgn function in the standard library
static inline float
sgn(float x)
{
    if( x == 0 ) return 0;
    if( x > 0 ) return 1; else return (-1);
}

/** @brief      Macro to optimize LUT access
 */
#define _LinearizeDrive( x )    PidDriveLut[abs(x)] * sgn(x)

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the PID controller                                  */
/*-----------------------------------------------------------------------------*/

pidController *
PidControllerInit( float Kp, float Ki, float Kd, tVexSensors port, int16_t sensor_reverse )
{
    pidController   *p;

    if( nextPidControllerPtr == MAX_PID )
        return(NULL);

#ifndef PIDLIB_USE_DYNAMIC
    p = (pidController *)&_pidControllers[ nextPidControllerPtr++ ];
#else
    p = chHeapAlloc( NULL, sizeof( pidController ) );
#endif

    // pid constants
    p->Kp    = Kp;
    p->Ki    = Ki;
    p->Kd    = Kd;
    p->Kbias = 0.0;

    // zero out working variables
    p->error           = 0;
    p->last_error      = 0;
    p->integral        = 0;
    p->derivative      = 0;
    p->drive           = 0.0;
    p->drive_cmd       = 0;
    if(Ki != 0)
        p->integral_limit  = (PIDLIB_INTEGRAL_DRIVE_MAX / Ki);
    else
        p->integral_limit  = 0;

    p->error_threshold = 10;

    // sensor port
    p->sensor_port     = port;
    p->sensor_reverse  = sensor_reverse;
    p->sensor_value    = 0;

    p->target_value    = 0;

    // We need a valid sensor for pid control, pot or encoder
    // different sensor use in ConVEX, always enable for now
    if( 1 )
        p->enabled    = 1;
    else
        p->enabled    = 0;

    PidControllerMakeLut();

    return(p);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the PID controller - includes bias                  */
/*-----------------------------------------------------------------------------*/

pidController *
PidControllerInitWithBias( float Kp, float Ki, float Kd, float Kbias, tVexSensors port, int16_t sensor_reverse )
{
    pidController   *p;
    p = PidControllerInit( Kp, Ki, Kd, port, sensor_reverse );
    if( p != NULL)
        p->Kbias = Kbias;

    return(p);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update the pid process variables                               */
/*-----------------------------------------------------------------------------*/

int16_t
PidControllerUpdate( pidController *p )
{
    if( p == NULL )
        return(0);

    if( p->enabled )
        {
        // check for sensor port
        // otherwise externally calculated error
        if( p->sensor_port >= 0 )
            {
            // Get raw position value, may be pot or encoder
            p->sensor_value = vexSensorValueGet( p->sensor_port );

            // A reversed sensor ?
            if( p->sensor_reverse )
                {
                if( vexSensorIsAnalog( p->sensor_port) )
                    // reverse pot
                    p->sensor_value = 4095 - p->sensor_value;
                else
                    // reverse encoder
                    p->sensor_value = -p->sensor_value;
                }

            p->error = p->target_value - p->sensor_value;
            }

        // force error to 0 if below threshold
        if( fabs(p->error) < p->error_threshold )
            p->error = 0;

        // integral accumulation
        if( p->Ki != 0 )
            {
            p->integral += p->error;

            // limit to avoid windup
            if( fabs( p->integral ) > p->integral_limit )
                p->integral = sgn(p->integral) * p->integral_limit;
            }
        else
            p->integral = 0;

        // derivative
        p->derivative = p->error - p->last_error;
        p->last_error = p->error;

        // calculate drive - no delta T in this version
        p->drive = (p->Kp * p->error) + (p->Ki * p->integral) + (p->Kd * p->derivative) + p->Kbias;

        // drive should be in the range +/- 1.0
        if( fabs( p->drive ) > 1.0 )
            p->drive = sgn(p->drive);

        // final motor output
        p->drive_raw = p->drive * 127.0;
        }

    else
        {
        // Disabled - all 0
        p->error      = 0;
        p->last_error = 0;
        p->integral   = 0;
        p->derivative = 0;
        p->drive      = 0.0;
        p->drive_raw  = 0;
        }

    // linearize - be careful this is a macro
    p->drive_cmd = _LinearizeDrive( p->drive_raw );

    // return the thing we are really interested in
    return( p->drive_cmd );
}


/*-----------------------------------------------------------------------------*/
/** @brief      Create a power based lut                                       */
/*-----------------------------------------------------------------------------*/

void
PidControllerMakeLut()
{
    int16_t   i;
    float     x;

    for(i=0;i<PIDLIB_LUT_SIZE;i++)
        {
        // check for valid power base
        if( PIDLIB_LUT_FACTOR > 1 )
            {
            x = fastpow( PIDLIB_LUT_FACTOR, (float)i / (float)(PIDLIB_LUT_SIZE-1) );

            if(i >= (PIDLIB_LUT_OFFSET/2))
               PidDriveLut[i] = (((x - 1.0) / (PIDLIB_LUT_FACTOR - 1.0)) * (PIDLIB_LUT_SIZE-1-PIDLIB_LUT_OFFSET)) + PIDLIB_LUT_OFFSET;
            else
               PidDriveLut[i] = i * 2;
            }
        else
            {
            // Linear
            PidDriveLut[i] = i;
            }
        }
}


