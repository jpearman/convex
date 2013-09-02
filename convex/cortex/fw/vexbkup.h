/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexbkup.h                                                    */
/*    Author:     James Pearman                                                */
/*    Created:    1 Sept 2013                                                  */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     1 Sept 2013 - Initial release                      */
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

#ifndef __VEXBKUP__
#define __VEXBKUP__

/*-----------------------------------------------------------------------------*/
/** @file    vexbkup.h
  * @brief   Backup register access macros and prototypes
*//*---------------------------------------------------------------------------*/


#ifdef __cplusplus
extern "C" {
#endif

void        vexBkupInit(void);
void        vexBkupRegisterWrite( int16_t index, uint16_t data );
uint16_t    vexBkupRegisterRead( int16_t index );
void        vexBkupDebug(vexStream *chp, int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif  // __VEXBKUP__
