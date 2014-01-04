/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexencoder.c                                                 */
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
/*                                                                             */
/*    Quadrature encoder handling                                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#include <stdlib.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexencoder.c
  * @brief   Quad Encoder driver
*//*---------------------------------------------------------------------------*/

static  vexQuadEncoder_t    vexQuadEncoders[kVexQuadEncoder_Num];

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*  There are 10 interrupt handlers, one for each input on up to 5 encoders    */
/*  (using 6 is not possible due to interrupt conflicts).  Each handler will   */
/*  call either the A or B service routine depending on which of the two       */
/*  encoder inputs is being serviced.                                          */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

static inline void
vexEncoderIrqServiceA( vexQuadEncoder_t *enc )
{
    int16_t pa;
    int16_t pb;

    // Read state of pins
    pa = (int16_t)palReadPad( enc->pa_port,  enc->pa_pad );
    pb = (int16_t)palReadPad( enc->pb_port,  enc->pb_pad );

    // we were interrupted by pa
    if( pa ^ pb )enc->count--; else enc->count++;
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */

static inline void
vexEncoderIrqServiceB( vexQuadEncoder_t *enc )
{
    int16_t pa;
    int16_t pb;

    // Read state of pins
    pa = (int16_t)palReadPad( enc->pa_port,  enc->pa_pad );
    pb = (int16_t)palReadPad( enc->pb_port,  enc->pb_pad );

    // we were interrupted by pb
    if( pa ^ pb )enc->count++; else enc->count--;
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */

static void
_vqe_1_cb_a(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    // service encoder
    vexEncoderIrqServiceA( &vexQuadEncoders[kVexQuadEncoder_1] );

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */

static void
_vqe_1_cb_b(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    // service encoder
    vexEncoderIrqServiceB( &vexQuadEncoders[kVexQuadEncoder_1] );

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */

static void
_vqe_2_cb_a(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    // service encoder
    vexEncoderIrqServiceA( &vexQuadEncoders[kVexQuadEncoder_2] );

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */

static void
_vqe_2_cb_b(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    // service encoder
    vexEncoderIrqServiceB( &vexQuadEncoders[kVexQuadEncoder_2] );

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */

static void
_vqe_3_cb_a(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    // service encoder
    vexEncoderIrqServiceA( &vexQuadEncoders[kVexQuadEncoder_3] );

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */

static void
_vqe_3_cb_b(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    // service encoder
    vexEncoderIrqServiceB( &vexQuadEncoders[kVexQuadEncoder_3] );

    chSysUnlockFromIsr();
}

static void
_vqe_4_cb_a(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    // service encoder
    vexEncoderIrqServiceA( &vexQuadEncoders[kVexQuadEncoder_4] );

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */

static void
_vqe_4_cb_b(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    // service encoder
    vexEncoderIrqServiceB( &vexQuadEncoders[kVexQuadEncoder_4] );

    chSysUnlockFromIsr();
}

static void
_vqe_5_cb_a(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    // service encoder
    vexEncoderIrqServiceA( &vexQuadEncoders[kVexQuadEncoder_5] );

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */

static void
_vqe_5_cb_b(EXTDriver *extp, expchannel_t channel)
{
    (void)extp;
    (void)channel;

    chSysLockFromIsr();

    // service encoder
    vexEncoderIrqServiceB( &vexQuadEncoders[kVexQuadEncoder_5] );

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/*   Associate an interrupt handler with an encoder channel and input          */
/*-----------------------------------------------------------------------------*/

static  extcallback_t   _vqe_cb_a[kVexQuadEncoder_Num] = {
        _vqe_1_cb_a,
        _vqe_2_cb_a,
        _vqe_3_cb_a,
        _vqe_4_cb_a,
        _vqe_5_cb_a
    };
static  extcallback_t   _vqe_cb_b[kVexQuadEncoder_Num] = {
        _vqe_1_cb_b,
        _vqe_2_cb_b,
        _vqe_3_cb_b,
        _vqe_4_cb_b,
        _vqe_5_cb_b
    };

/*-----------------------------------------------------------------------------*/
/** @brief    Initialize all the encoder data, counts to zero etc.             */
/*-----------------------------------------------------------------------------*/

void
vexEncoderInit()
{
    tVexQuadEncoderChannel  c;

    for(c=kVexQuadEncoder_1;c<kVexQuadEncoder_Num;c++)
        {
        vexQuadEncoders[c].state  = 0;
        vexQuadEncoders[c].count  = 0;
        vexQuadEncoders[c].offset = 0;
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Add an encoder to the system                                   */
/** @param[in]  channel The encoder channel                                    */
/** @param[in]  pa The first input pin used by the encoder                     */
/** @param[in]  pb The second input pin used by the encoder                    */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Setup the interrupt handlers for an encoder but do not enable them
 */

void
vexEncoderAdd( tVexQuadEncoderChannel channel, tVexDigitalPin pa, tVexDigitalPin pb )
{
    if( channel >= kVexQuadEncoder_Num )
        return;

    if(pa > kVexDigital_12)
        return;
    if(pb > kVexDigital_12)
        return;

    // zero variables
    vexQuadEncoders[channel].count  = 0;
    vexQuadEncoders[channel].offset = 0;

    // setup first input
    vexDigitalModeSet( pa, kVexDigitalInput);
    vexQuadEncoders[channel].pa = pa;
    vexQuadEncoders[channel].pa_port   = vexioDefinition[pa].port;
    vexQuadEncoders[channel].pa_pad    = vexioDefinition[pa].pad;
    vexExtSet( vexQuadEncoders[channel].pa_port, vexQuadEncoders[channel].pa_pad, EXT_CH_MODE_BOTH_EDGES, _vqe_cb_a[channel] );

    // setup second input
    vexDigitalModeSet( pb, kVexDigitalInput);
    vexQuadEncoders[channel].pb = pb;
    vexQuadEncoders[channel].pb_port   = vexioDefinition[pb].port;
    vexQuadEncoders[channel].pb_pad    = vexioDefinition[pb].pad;
    vexExtSet( vexQuadEncoders[channel].pb_port, vexQuadEncoders[channel].pb_pad, EXT_CH_MODE_BOTH_EDGES, _vqe_cb_b[channel] );

    vexQuadEncoders[channel].state = 1;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start an encoder interrupt handler                             */
/** @param[in]  channel The encoder channel to start the irq handler on        */
/*-----------------------------------------------------------------------------*/

void
vexEncoderStart( tVexQuadEncoderChannel channel )
{
    if( vexQuadEncoders[channel].state == 1) {
        // start interrupts
        extChannelEnable( &EXTD1, vexQuadEncoders[channel].pa_pad );
        extChannelEnable( &EXTD1, vexQuadEncoders[channel].pb_pad );
    }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Stop an encoder interrupt handler                              */
/** @param[in]  channel The encoder channel to stop the irq handler on         */
/*-----------------------------------------------------------------------------*/

void
vexEncoderStop( tVexQuadEncoderChannel channel )
{
    if( vexQuadEncoders[channel].state == 1) {
        // stop interrupts
        extChannelDisable( &EXTD1, vexQuadEncoders[channel].pa_pad );
        extChannelDisable( &EXTD1, vexQuadEncoders[channel].pb_pad );
    }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start all encoder interrupt handlers                           */
/*-----------------------------------------------------------------------------*/

void
vexEncoderStartAll()
{
    tVexQuadEncoderChannel  c;

    for(c=kVexQuadEncoder_1;c<kVexQuadEncoder_Num;c++)
        vexEncoderStart( c );
}


/*-----------------------------------------------------------------------------*/
/** @brief      Get encoder count                                              */
/** @param[in]  channel The encoder channel                                    */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Get an encoder count for a channel, we don't use the enumerated channel type
 *  so this can be used without warnings from other code in a callback. Typically
 *  the motor control code may call this and not know what type of encoder is
 *  being used, a quad encoder or an IME
 */

int32_t
vexEncoderGet( int16_t channel )
{
    if( channel < 0 || channel >= kVexQuadEncoder_Num )
        return(0);

    return( vexQuadEncoders[channel].count - vexQuadEncoders[channel].offset );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get encoder count                                              */
/** @param[in]  channel The encoder channel                                    */
/** @param[in]  value The value to set the encoder to                          */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Set encoder count for a channel, this saves the value as an offset to the
 *  current count
 */

void
vexEncoderSet( int16_t channel, int32_t value )
{
    if( channel < 0 || channel >= kVexQuadEncoder_Num )
        return;

    vexQuadEncoders[channel].offset = vexQuadEncoders[channel].count - value;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get encoder id                                                 */
/** @param[in]  channel The encoder channel                                    */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Get an encoder id for a channel, we don't use the enumerated type so
 *  this can be used without warnings from other code in a callback. Typically
 *  the motor control code may call this and not know what type of encoder is
 *  being used, a quad encoder or an IME
 *  The encoder id is basically the pin number for inputA
 */

int16_t
vexEncoderGetId( int16_t channel )
{
    if( channel >= kVexQuadEncoder_Num )
        return(-1);

    // this helps with some legacy code
    return( vexQuadEncoders[channel].pa );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Send all encoder raw values and offsets to the console         */
/** @param[in]  chp     A pointer to a vexStream object                      */
/** @param[in]  argc    The number of command line arguments                   */
/** @param[in]  argv    An array of pointers to the command line args          */
/*-----------------------------------------------------------------------------*/

void
vexEncoderDebug(vexStream *chp, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    tVexQuadEncoderChannel  c;

    for(c=kVexQuadEncoder_1;c<kVexQuadEncoder_Num;c++)
        vex_chprintf(chp,"E%d %8ld  %8ld\r\n", c, vexQuadEncoders[c].count, vexQuadEncoders[c].offset );
}




