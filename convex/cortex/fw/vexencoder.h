/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexencoder.h                                                 */
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

#ifndef __VEXENCODER__
#define __VEXENCODER__

/*-----------------------------------------------------------------------------*/
/** @file    vexencoder.h
  * @brief   Header for the encoder driver
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      Encoder channel number                                         */
/*-----------------------------------------------------------------------------*/
typedef enum {
    kVexQuadEncoder_1 = 0,
    kVexQuadEncoder_2,
    kVexQuadEncoder_3,
    kVexQuadEncoder_4,
    kVexQuadEncoder_5,

    kVexQuadEncoder_Num
    } tVexQuadEncoderChannel;

/*-----------------------------------------------------------------------------*/
/** @brief      Holds information relating to an encoder                       */
/*-----------------------------------------------------------------------------*/
/** @note
 *  pin refers to the digital pin 1 through 12, port refers to the
 *  STM32 port, GPIOA, GPIOB etc. pad refers to the bit on the digital port
 *  0 through 15
 */

typedef struct _vexQuadEncoder_t {
    volatile int32_t  count;      ///< current encoder count
    int32_t           offset;     ///< current encoder offset (subtracted from count)
    int16_t           state;      ///< flag iondicating encoder is installed
    tVexDigitalPin    pa;         ///< Encoder digital pin a
    tVexDigitalPin    pb;         ///< Encoder digital pin b
    ioportid_t        pa_port;    ///< Encoder GPIO port a
    int16_t           pa_pad;     ///< Encoder GPIO pad a
    ioportid_t        pb_port;    ///< Encoder GPIO port b
    int16_t           pb_pad;     ///< Encoder GPIO pad b
} vexQuadEncoder_t;


#ifdef __cplusplus
extern "C" {
#endif

void                vexEncoderInit(void);
void                vexEncoderAdd( tVexQuadEncoderChannel channel, tVexDigitalPin pa, tVexDigitalPin pb );
void                vexEncoderStart( tVexQuadEncoderChannel channel );
void                vexEncoderStop( tVexQuadEncoderChannel channel );
void                vexEncoderStartAll(void);
int32_t             vexEncoderGet( int16_t channel );
void                vexEncoderSet( int16_t channel, int32_t value );
int16_t             vexEncoderGetId( int16_t channel );
void                vexEncoderDebug(vexStream *chp, int argc, char *argv[]);

#ifdef __cplusplus
}
#endif


#endif  // __VEXENCODER__
