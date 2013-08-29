/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     pidlib.h                                                     */
/*    Author:     James Pearman                                                */
/*    Created:    19 June 2013                                                 */
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

#ifndef __PIDLIB__
#define __PIDLIB__

/*-----------------------------------------------------------------------------*/
/** @file    pidlib.h
  * @brief   A port of the ROBOTC pidlib library, macros and prototypes
*//*---------------------------------------------------------------------------*/

/** @brief Current pidlib Version is 1.02
 */
#define kPidLibVersion          102

/** @brief Use heap for pid controller data rather than static data
 */
#define PIDLIB_USE_DYNAMIC      1

/*-----------------------------------------------------------------------------*/
/** @brief Structure to hold all data for one instance of a PID controller     */
/*-----------------------------------------------------------------------------*/
/** @note
 *  Currently at 64 bytes memory usage
 */
typedef struct _pidController {
    // Turn on or off the control loop
    int16_t      enabled;        ///< enable or diable pid calculations
    int16_t      res1;           ///< word alignment of Kp

    // PID constants, Kbias is used to compensate for gravity or similar
    float        Kp;             ///< proportional constant
    float        Ki;             ///< integral constant
    float        Kd;             ///< derivative constant
    float        Kbias;          ///< bias constant

    // working variables
    float        error;          ///< error between actual pisition and target
    float        last_error;     ///< error last time update called
    float        integral;       ///< integrated error
    float        integral_limit; ///< limit for integrated error
    float        derivative;     ///< change in error from last time
    float        error_threshold;///< threshold below which error is ignored

    // output
    float        drive;          ///< calculated motor drive in range +/- 1.0
    int16_t      drive_raw;      ///< motor drive in the range +/- 127
    int16_t      drive_cmd;      ///< linearized motor drive in the range +/- 127

    tVexSensors  sensor_port;    ///< digital or analog port with the position sensor
    int16_t      sensor_reverse; ///< flag indicating the sensor values should be reversed
    int32_t      sensor_value;   ///< current value of the position sensor

    int32_t      target_value;   ///< the target value
    } pidController;


/*-----------------------------------------------------------------------------*/
/** @brief Allow 4 pid controllers                                             */
/*-----------------------------------------------------------------------------*/
#define MAX_PID                     4

// lookup table to linearize control

/** @brief size of linearizing table
 */
#define PIDLIB_LUT_SIZE           128
/** @brief severity pf the linearize power function
 */
#define PIDLIB_LUT_FACTOR        20.0
/** @brief offset below which the linearize function is not used
 */
#define PIDLIB_LUT_OFFSET          10

/** @brief This causes the motor never to be given more than a 0.25 drive
 *  command due to integral
 */
#define PIDLIB_INTEGRAL_DRIVE_MAX   0.25

#ifdef __cplusplus
extern "C" {
#endif

pidController *PidControllerInit( float Kp, float Ki, float Kd, tVexSensors port, int16_t sensor_reverse );
pidController *PidControllerInitWithBias( float Kp, float Ki, float Kd, float Kbias, tVexSensors port, int16_t sensor_reverse );
int16_t        PidControllerUpdate( pidController *p );
void           PidControllerMakeLut(void);

#ifdef __cplusplus
}
#endif

#endif  // __PIDLIB__
