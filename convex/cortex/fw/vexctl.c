/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexctl.c                                                     */
/*    Author:     James Pearman                                                */
/*    Created:    9 May 2013                                                   */
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
/*	A place for test code before integration into it's pown modules			   */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#include "ch.h"  		// needs for all ChibiOS programs
#include "hal.h" 		// hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexctl.c
  * @brief   Extract data from SPI data received from master processor
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      Get competition state word (and controller status)             */
/** @returns    status word from SPI data                                      */
/*-----------------------------------------------------------------------------*/

uint16_t
vexControllerCompetitonState()
{
	return( vexSpiGetControl() );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get controller data                                            */
/** @param[in]  index The required controller variable eg. Btn8U               */
/** @returns    The requested controller data                                  */
/*-----------------------------------------------------------------------------*/

int16_t
vexControllerGet( tCtlIndex index )
{
	jsdata *js;
	int16_t analog;

	// Get pointer to raw joystick data
	if(index < Ch1Xmtr2)
		js = vexSpiGetJoystickDataPtr( 1 );
	else
		js = vexSpiGetJoystickDataPtr( 2 );

	// decode data, needs transmitter 2 to be defined as index + 0x80
	switch( index & 0x7F )
		{
	    // clip analog joystick at +127 (max would be +128)
		case	Ch1:
		    analog = ( js->Ch1 == 0xFF ) ? 127 : js->Ch1 - 127;
		    return( analog );
			break;
		case	Ch2:
            analog = ( js->Ch2 == 0xFF ) ? 127 : js->Ch2 - 127;
			return( -analog ); // flip vertical axis
			break;
		case	Ch3:
            analog = ( js->Ch3 == 0xFF ) ? 127 : js->Ch3 - 127;
			return( -analog ); // flip vertical axis
			break;
		case	Ch4:
            analog = ( js->Ch4 == 0xFF ) ? 127 : js->Ch4 - 127;
			return( analog );
			break;

		case	Btn8D:
			return( ( js->btns[1] & 0x01 ) ? 1 : 0 );
			break;
		case	Btn8L:
			return( ( js->btns[1] & 0x02 ) ? 1 : 0 );
			break;
		case	Btn8U:
			return( ( js->btns[1] & 0x04 ) ? 1 : 0 );
			break;
		case	Btn8R:
			return( ( js->btns[1] & 0x08 ) ? 1 : 0 );
			break;

		case	Btn7D:
			return( ( js->btns[1] & 0x10 ) ? 1 : 0 );
			break;
		case	Btn7L:
			return( ( js->btns[1] & 0x20 ) ? 1 : 0 );
			break;
		case	Btn7U:
			return( ( js->btns[1] & 0x40 ) ? 1 : 0 );
			break;
		case	Btn7R:
			return( ( js->btns[1] & 0x80 ) ? 1 : 0 );
			break;

		case	Btn5D:
			return( ( js->btns[0] & 0x01 ) ? 1 : 0 );
			break;
		case	Btn5U:
			return( ( js->btns[0] & 0x02 ) ? 1 : 0 );
			break;

		case	Btn6D:
			return( ( js->btns[0] & 0x04 ) ? 1 : 0 );
			break;
		case	Btn6U:
			return( ( js->btns[0] & 0x08 ) ? 1 : 0 );
			break;

		// Accelerometers (there is no Z !)
		case	AcclX:
			return( js->acc_x - 127 );
			break;
		case	AcclY:
			return( js->acc_y - 127 );
			break;
		case	AcclZ:
			return( js->acc_z - 127 );
			break;

		// Check for any button in the group
        case    Btn5:
            return( ( js->btns[0] & 0x03 ) ? 1 : 0 );
            break;
        case    Btn6:
            return( ( js->btns[0] & 0x0C ) ? 1 : 0 );
            break;
        case    Btn7:
            return( ( js->btns[1] & 0xF0 ) ? 1 : 0 );
            break;
        case    Btn8:
            return( ( js->btns[1] & 0x0F ) ? 1 : 0 );
            break;

        // Any button on the controller
        case    BtnAny:
            return( ( js->btns[0] || js->btns[1] ) ? 1 : 0 );
            break;

		default:
			return 0;
		}
}
