/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexctl.h                                                     */
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

#ifndef __VEXCTL__
#define __VEXCTL__

/*-----------------------------------------------------------------------------*/
/** @file    vexctl.h
  * @brief   Extract data from SPI data received from master processor
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief  controller button and analog control definitions                   */
/** @note   loosely follows the ROBOTC definition of the same functions        */
/*-----------------------------------------------------------------------------*/
/** @details
 *  These are the constants that are used in the call to vexControllerGet.
 *  They follow the format that ROBOTC uses in its call to vexRT[], however,
 *  there are a few additions to allow multiple buttons to be checked.
 */
typedef enum {
        Ch1 = 0,        Ch2,            Ch3,            Ch4,
        Btn8D,          Btn8L,          Btn8U,          Btn8R,
        Btn7D,          Btn7L,          Btn7U,          Btn7R,
        Btn5D,          Btn5U,
        Btn6D,          Btn6U,
        AcclX,          AcclY,          AcclZ,
        Btn5,           Btn6,           Btn7,           Btn8,
        BtnAny,

        Ch1Xmtr2 = 128, Ch2Xmtr2,       Ch3Xmtr2,       Ch4Xmtr2,
        Btn8DXmtr2,     Btn8LXmtr2,     Btn8UXmtr2,     Btn8RXmtr2,
        Btn7DXmtr2,     Btn7LXmtr2,     Btn7UXmtr2,     Btn7RXmtr2,
        Btn5DXmtr2,     Btn5UXmtr2,
        Btn6DXmtr2,     Btn6UXmtr2,
        AcclXXmtr2,     AcclYXmtr2,     AcclZXmtr2,
        Btn5Xmtr2,      Btn6Xmtr2,      Btn7Xmtr2,      Btn8mXtr2,
        BtnAnyXmtr2
} tCtlIndex;

/*-----------------------------------------------------------------------------*/
/** @brief  Various competition and controller flags                           */
/*-----------------------------------------------------------------------------*/
typedef enum {
    kFlagNoXmiters           = 0,        // No transmitters connected
    kFlagXmit1               = 0x01,     //                          1 == Transmitter 1 connected
    kFlagXmit2               = 0x02,     //                          1 == Transmitter 2 connected
    kFlagBit2                = 0x04,     // Unused
    kFlagCompetitionSwitch   = 0x08,     // 0 == No Comp Switch      1 == Competition Switch attached.
    kFlagResetSlave          = 0x10,     // Unused
    kFlagGameController      = 0x20,     // 0 == Legacy75MHz,        1 == Game Controller
    kFlagAutonomousMode      = 0x40,     // 0 == Driver Control,     1 == Autonomous Mode
    kFlagDisabled            = 0x80,     // 0 == Enabled             1 == Disabled.
} tVexControlState;


#ifdef __cplusplus
extern "C" {
#endif

int16_t     vexControllerGet( tCtlIndex index );
int16_t     vexControllerCompetitionStateSet( uint16_t ctl, int16_t mask );
uint16_t    vexControllerCompetitonState(void);
void        vexControllerReleaseWait( tCtlIndex index );

#ifdef __cplusplus
}
#endif

#endif  // __VEXCTL__
