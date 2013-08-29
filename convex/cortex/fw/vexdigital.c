/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexdigital.c                                                 */
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


#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexdigital.c
  * @brief   Digital port driver
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      definition of a digital IO pin                                 */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This data structure is used to correlate a digital port(pin) number on the cortex
 *  with the port/pad pair on the STM32.
 */
ioDef   vexioDefinition[ kVexDigital_Num ] = {
        {VEX_DIGIO_1_PORT,   VEX_DIGIO_1_PAD,  kVexDigitalInput, -1 },
        {VEX_DIGIO_2_PORT,   VEX_DIGIO_2_PAD,  kVexDigitalInput, -1 },
        {VEX_DIGIO_3_PORT,   VEX_DIGIO_3_PAD,  kVexDigitalInput, -1 },
        {VEX_DIGIO_4_PORT,   VEX_DIGIO_4_PAD,  kVexDigitalInput, -1 },
        {VEX_DIGIO_5_PORT,   VEX_DIGIO_5_PAD,  kVexDigitalInput, -1 },
        {VEX_DIGIO_6_PORT,   VEX_DIGIO_6_PAD,  kVexDigitalInput, -1 },
        {VEX_DIGIO_7_PORT,   VEX_DIGIO_7_PAD,  kVexDigitalInput, -1 },
        {VEX_DIGIO_8_PORT,   VEX_DIGIO_8_PAD,  kVexDigitalInput, -1 },
        {VEX_DIGIO_9_PORT,   VEX_DIGIO_9_PAD,  kVexDigitalInput, -1 },
        {VEX_DIGIO_10_PORT,  VEX_DIGIO_10_PAD, kVexDigitalInput, -1 },
        {VEX_DIGIO_11_PORT,  VEX_DIGIO_11_PAD, kVexDigitalInput, -1 },
        {VEX_DIGIO_12_PORT,  VEX_DIGIO_12_PAD, kVexDigitalInput, -1 }
};

/*-----------------------------------------------------------------------------*/
/** @brief      Set digital port mode                                          */
/** @param[in]  pin The pin to set the new mode on                             */
/** @param[in]  mode kVexDigitalInput or kVexDigitalOutput                     */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Change the type of a digital IO port
 *  All ports are initially initialized to be INPUTs unless the board.h file
 *  is changed (not recommended)
 */

void
vexDigitalModeSet( tVexDigitalPin pin, tVexDigitalMode mode )
{
    if( pin > kVexDigital_12 )
        return;

    vexioDefinition[ pin ].mode = mode;

    if( mode == kVexDigitalInput )
        palSetPadMode( vexioDefinition[ pin ].port, vexioDefinition[ pin ].pad , PAL_MODE_INPUT_PULLUP );
    else
    if( mode == kVexDigitalOutput )
        palSetPadMode( vexioDefinition[ pin ].port, vexioDefinition[ pin ].pad , PAL_MODE_OUTPUT_PUSHPULL );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set digital port mode                                          */
/** @param[in]  pin The pin to get the mode for                                */
/** @returns    the mode for the given pin                                     */
/*-----------------------------------------------------------------------------*/

tVexDigitalMode
vexDigitalModeGet( tVexDigitalPin pin )
{
    if( pin > kVexDigital_12 )
        return(-1);

    return( vexioDefinition[ pin ].mode );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set digital pin state                                          */
/** @param[in]  pin The pin to set the new state on                            */
/** @param[in]  state kVexDigitalLow or kVexDigitalHigh                        */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Set a digital pin to a value, high or low.  If the pin is an input then
 *  this enables the pullup or pulldown resistor.
 */

void
vexDigitalPinSet( tVexDigitalPin pin, tVexDigitalState state )
{
    if( pin > kVexDigital_12 )
        return;

    if( state == kVexDigitalLow )
        palClearPad( vexioDefinition[ pin ].port, vexioDefinition[ pin ].pad );
    else
        palSetPad( vexioDefinition[ pin ].port, vexioDefinition[ pin ].pad );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get digital pin state                                          */
/** @param[in]  pin The pin to get the state of                                */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Read the value of a digital pin, if the pin is a digital output then the
 *  output value is returned.
 */

tVexDigitalState
vexDigitalPinGet( tVexDigitalPin pin )
{
    if( pin > kVexDigital_12 )
        return( kVexDigitalLow );

    if( palReadPad( vexioDefinition[ pin ].port, vexioDefinition[ pin ].pad ) )
        return( kVexDigitalHigh );

    return( kVexDigitalLow );
}





/*-----------------------------------------------------------------------------*/
/*  Callback for digital interrupt                                             */
/*-----------------------------------------------------------------------------*/

static void
_vi_cb(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    int pin;
    for(pin=0;pin<kVexDigital_Num;pin++)
        {
        if( vexioDefinition[pin].pad == (int16_t)channel )
            {
            if( vexioDefinition[pin].intrCount >= 0 )
                {
                vexioDefinition[pin].intrCount++;
                break;
                }
            }
        }

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set digital pin to cause interrupt                             */
/** @param[in]  pin The pin                                                    */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This is used for testing at present, the interrupt just increments a counter
 */

void
vexDigitalIntrSet( tVexDigitalPin pin )
{
    vexDigitalModeSet( pin, kVexDigitalInput );
    vexExtSet( vexioDefinition[pin].port, vexioDefinition[pin].pad, EXT_CH_MODE_BOTH_EDGES, _vi_cb );

    // Fixed screwed up port4/port10 conflict
    if( vexioDefinition[pin].pad == 7 )
        {
        if( pin == kVexDigital_4 )
            vexioDefinition[kVexDigital_10].intrCount = -1;
        if( pin == kVexDigital_10 )
            vexioDefinition[kVexDigital_4].intrCount = -1;
        }

    vexioDefinition[pin].intrCount = 0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Enable any digital pin interrupts                              */
/*-----------------------------------------------------------------------------*/

void
vexDigitalIntrRun()
{
    int16_t pin;

    for(pin=0;pin<kVexDigital_Num;pin++)
        {
        if(vexioDefinition[pin].intrCount == 0)
            extChannelEnable( &EXTD1, vexioDefinition[pin].pad );
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the interrupt count for a given digital pin                */
/** @param[in]  pin The pin                                                    */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This is used for testing at present, the interrupt just increments a counter
 */
int32_t
vexDigitalIntrCountGet( tVexDigitalPin pin )
{
    if( pin > kVexDigital_12 )
        return( 0 );

    return( vexioDefinition[pin].intrCount );
}

