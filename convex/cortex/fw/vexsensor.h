/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexsensor.h                                                  */
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

#ifndef __VEXSENSOR__
#define __VEXSENSOR__

/*-----------------------------------------------------------------------------*/
/** @file    vexsensor.h
  * @brief   ROBOTC style sensor interface macros and prototypes
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      sensor types for vexSensorValue                                */
/** @note       A ROBOTC style sensor interface to make porting code easier    */
/*-----------------------------------------------------------------------------*/

typedef enum
    {
    kVexSensorUndefined     = -1,
    //
    // Analog I/O Ports: light, line follower, gyro, accelerometer,
    //
    kVexSensorAnalog_1      =  0,
    kVexSensorAnalog_2      =  1,
    kVexSensorAnalog_3      =  2,
    kVexSensorAnalog_4      =  3,
    kVexSensorAnalog_5      =  4,
    kVexSensorAnalog_6      =  5,
    kVexSensorAnalog_7      =  6,
    kVexSensorAnalog_8      =  7,

    //
    // Digital I/O Ports: Touch sensors, sonar, etc
    //
    kVexSensorDigital_1     = 8,
    kVexSensorDigital_2     = 9,
    kVexSensorDigital_3     = 10,
    kVexSensorDigital_4     = 11,
    kVexSensorDigital_5     = 12,
    kVexSensorDigital_6     = 13,
    kVexSensorDigital_7     = 14,
    kVexSensorDigital_8     = 15,
    kVexSensorDigital_9     = 16,
    kVexSensorDigital_10    = 17,
    kVexSensorDigital_11    = 18,
    kVexSensorDigital_12    = 19,

    //
    // IMEs
    kVexSensorIme_1         = 20,
    kVexSensorIme_2         = 21,
    kVexSensorIme_3         = 22,
    kVexSensorIme_4         = 23,
    kVexSensorIme_5         = 24,
    kVexSensorIme_6         = 25,
    kVexSensorIme_7         = 26,
    kVexSensorIme_8         = 27
    } tVexSensors;

#ifdef __cplusplus
extern "C" {
#endif

bool_t  vexSensorIsAnalog( tVexSensors sensor  );
int32_t vexSensorValueGet( tVexSensors sensor );
void    vexSensorValueSet( tVexSensors sensor, int32_t value );

#ifdef __cplusplus
}
#endif

#endif  // __VEXSENSOR__
