/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexadc.h                                                     */
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

#ifndef __VEXADC__
#define __VEXADC__

/*-----------------------------------------------------------------------------*/
/** @file    vexanalog.h
  * @brief   Analog port driver macros and prototypes
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      enumeration for the 8 analog ports                             */
/*-----------------------------------------------------------------------------*/
/** @details
 *  The cortex has 8 analog ports, kVexAnalog_None causes enum to be signed.
 */
typedef enum {
    kVexAnalog_None = -1,
    kVexAnalog_1 = 0,
    kVexAnalog_2,
    kVexAnalog_3,
    kVexAnalog_4,
    kVexAnalog_5,
    kVexAnalog_6,
    kVexAnalog_7,
    kVexAnalog_8,

    kVexAnalog_Num
    } tVexAnalogPin;

#ifdef __cplusplus
extern "C" {
#endif

void        vexAdcInit( void );
int16_t     vexAdcGet( int16_t index );
void        vexAdcDebug( vexStream *chp, int argc, char *argv[] );

#ifdef __cplusplus
}
#endif

#endif  // __VEXADC__
