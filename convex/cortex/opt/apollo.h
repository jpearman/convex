/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     apollo.h                                                     */
/*    Author:     James Pearman                                                */
/*    Created:    5 June 2013                                                  */
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

#ifndef __APOLLO__
#define __APOLLO__

/*-----------------------------------------------------------------------------*/
/** @file    apollo.h
  * @brief   Status display for VT100 console macros and prototypes
*//*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/** @name    terminal origin and size
  * @{
*//*---------------------------------------------------------------------------*/
#define T_X_ORIGIN  1
#define T_Y_ORIGIN  1
#define T_WIDTH     80
#define T_HEIGHT    24
/** @}  */

/*-----------------------------------------------------------------------------*/
/** @name    Motor status origin and size
  * @{
*//*---------------------------------------------------------------------------*/
#define M_X_POS     1
#define M_WIDTH     41
#define M_Y_POS     1
#define M_HEIGHT    15
/** @}  */

/*-----------------------------------------------------------------------------*/
/** @name    Digital ports status origin and size
  * @{
*//*---------------------------------------------------------------------------*/
#define D_X_POS     (M_X_POS+M_WIDTH-1)
#define D_WIDTH     21
#define D_Y_POS     1
#define D_HEIGHT    15
/** @}  */

/*-----------------------------------------------------------------------------*/
/** @name    Analog ports status origin and size
  * @{
*//*---------------------------------------------------------------------------*/
#define A_X_POS     (D_X_POS+D_WIDTH-1)
#define A_WIDTH     20
#define A_Y_POS     1
#define A_HEIGHT    15
/** @}  */

/*-----------------------------------------------------------------------------*/
/** @name    Joystick status origin
  * @{
*//*---------------------------------------------------------------------------*/
#define J_X_POS     1
#define J_Y_POS     15
/** @}  */

#ifdef __cplusplus
extern "C" {
#endif

void        apolloScreenSetup(void);
void        apolloMotorSetup(void);
void        apolloDigitalSetup(void);
void        apolloAnalogSetup(void);
void        apolloJoystickSetup( int joystick );
void        apolloInit(void);
void        apolloDeinit(void);
void        apolloUpdateMotor( int m );
void        apolloUpdateCortex( int p );
void        apolloUpdateDigital( int pin );
void        apolloUpdateAnalog( int pin );
void        apolloUpdateSystem(void);
void        apolloUpdateJoystickButton( int joystick, int row, int up, int down, int left, int right );
void        apolloUpdateJoystickAnalog( int joystick, int row, int h, int v );
void        apolloUpdate( void );

#ifdef __cplusplus
}
#endif

#endif  // __APOLLO__

