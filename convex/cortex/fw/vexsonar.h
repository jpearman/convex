/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexsonar.h                                                   */
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

#ifndef __VEXSONAR__
#define __VEXSONAR__

/*-----------------------------------------------------------------------------*/
/** @file    vexsonar.h
  * @brief   Ultrasonic distance sensor driver macros and prototypes
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      Sonar channel number                                           */
/*-----------------------------------------------------------------------------*/
typedef enum {
    kVexSonar_1 = 0,
    kVexSonar_2,
    kVexSonar_3,
    kVexSonar_4,
    kVexSonar_5,

    kVexSonar_Num
    } tVexSonarChannel;

/*-----------------------------------------------------------------------------*/
/** @brief      Holds information relating to a sonar                          */
/*-----------------------------------------------------------------------------*/
/** @note
 *  pin refers to the digital pin 1 through 12, port refers to the
 *  STM32 port, GPIOA, GPIOB etc. pad refers to the bit on the digital port
 *  0 through 15
 */

 typedef struct _vexSonar_t {
    uint16_t        time_r;         ///< time of rising edge on receive pulse
    uint16_t        time_f;         ///< time of falling edge on receive pulse
    int32_t         time;           ///< receive pulse time
    int16_t         distance_cm;    ///< distance calculated as cm
    int16_t         distance_inch;  ///< distance calculated as inches
    int16_t         flags;          ///< flags, SONAR_INSTALLED & SONAR_ENABLED
    tVexDigitalPin  pa;             ///< Sonar digital pin a
    tVexDigitalPin  pb;             ///< Sonar digital pin b
    ioportid_t      pa_port;        ///< Sonar GPIO port a
    int16_t         pa_pad;         ///< Sonar GPIO pad a
    ioportid_t      pb_port;        ///< Sonar GPIO port b
    int16_t         pb_pad;         ///< Sonar GPIO pad b
} vexSonar_t;

#ifdef __cplusplus
extern "C" {
#endif

void                vexSonarAdd( tVexSonarChannel channel, tVexDigitalPin pa, tVexDigitalPin pb );
void                vexSonarStart( tVexSonarChannel channel );
void                vexSonarStop( tVexSonarChannel channel );
void                vexSonarStartAll(void);
void                vexSonarPing(tVexSonarChannel channel);
void                vexSonarDebug(vexStream *chp, int argc, char *argv[]);
void                vexSonarRun(void);
int16_t             vexSonarGetCm( tVexSonarChannel channel );
int16_t             vexSonarGetInch( tVexSonarChannel channel );

#ifdef __cplusplus
}
#endif


#endif  // __VEXSONAR__
