/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexdigital.h                                                 */
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

#ifndef __VEXDIGITAL__
#define __VEXDIGITAL__

/*-----------------------------------------------------------------------------*/
/** @file    vexdigital.h
  * @brief   Header for the digital port driver
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief  correlate a port on the cortex with a port/pin combo on the STM32  */
/*-----------------------------------------------------------------------------*/
/** @name Digital IO Pins
 *  The STM32F103 has five 16 bit GPIO ports, A through E.  We define the 12
 *  digital port/pad pairs in the board.h file but then dereference them
 *  once more here.
 *  @{
 */

#define VEX_DIGIO_1_PAD     GPIOE_DIGIO_1
#define VEX_DIGIO_2_PAD     GPIOE_DIGIO_2
#define VEX_DIGIO_3_PAD     GPIOC_DIGIO_3
#define VEX_DIGIO_4_PAD     GPIOC_DIGIO_4
#define VEX_DIGIO_5_PAD     GPIOE_DIGIO_5
#define VEX_DIGIO_6_PAD     GPIOE_DIGIO_6
#define VEX_DIGIO_7_PAD     GPIOE_DIGIO_7
#define VEX_DIGIO_8_PAD     GPIOE_DIGIO_8
#define VEX_DIGIO_9_PAD     GPIOE_DIGIO_9
#define VEX_DIGIO_10_PAD    GPIOE_DIGIO_10
#define VEX_DIGIO_11_PAD    GPIOD_DIGIO_11
#define VEX_DIGIO_12_PAD    GPIOD_DIGIO_12

#define VEX_DIGIO_1_PORT    PORT_DIGIO_1
#define VEX_DIGIO_2_PORT    PORT_DIGIO_2
#define VEX_DIGIO_3_PORT    PORT_DIGIO_3
#define VEX_DIGIO_4_PORT    PORT_DIGIO_4
#define VEX_DIGIO_5_PORT    PORT_DIGIO_5
#define VEX_DIGIO_6_PORT    PORT_DIGIO_6
#define VEX_DIGIO_7_PORT    PORT_DIGIO_7
#define VEX_DIGIO_8_PORT    PORT_DIGIO_8
#define VEX_DIGIO_9_PORT    PORT_DIGIO_9
#define VEX_DIGIO_10_PORT   PORT_DIGIO_10
#define VEX_DIGIO_11_PORT   PORT_DIGIO_11
#define VEX_DIGIO_12_PORT   PORT_DIGIO_12
/** @} */

/*-----------------------------------------------------------------------------*/
/** @name Fast IO pin access
 *  These are used to bypass IO pin boundary checks when maximum speed
 *  is desired.  Normal IO pin access should be done using vexDigitalPinSet
 *  @{
 */
/*-----------------------------------------------------------------------------*/

#define VexDigInput_1()     palReadPad( VEX_DIGIO_1_PORT, VEX_DIGIO_1_PAD )
#define VexDigInput_2()     palReadPad( VEX_DIGIO_2_PORT, VEX_DIGIO_2_PAD )
#define VexDigInput_3()     palReadPad( VEX_DIGIO_3_PORT, VEX_DIGIO_3_PAD )
#define VexDigInput_4()     palReadPad( VEX_DIGIO_4_PORT, VEX_DIGIO_4_PAD )
#define VexDigInput_5()     palReadPad( VEX_DIGIO_5_PORT, VEX_DIGIO_5_PAD )
#define VexDigInput_6()     palReadPad( VEX_DIGIO_6_PORT, VEX_DIGIO_6_PAD )
#define VexDigInput_7()     palReadPad( VEX_DIGIO_7_PORT, VEX_DIGIO_7_PAD )
#define VexDigInput_8()     palReadPad( VEX_DIGIO_8_PORT, VEX_DIGIO_8_PAD )
#define VexDigInput_9()     palReadPad( VEX_DIGIO_9_PORT, VEX_DIGIO_9_PAD )
#define VexDigInput_10()    palReadPad( VEX_DIGIO_10_PORT, VEX_DIGIO_10_PAD )
#define VexDigInput_11()    palReadPad( VEX_DIGIO_11_PORT, VEX_DIGIO_11_PAD )
#define VexDigInput_12()    palReadPad( VEX_DIGIO_12_PORT, VEX_DIGIO_12_PAD )

#define VexPinOutput_1(x)   { (x==0) ? palClearPad( VEX_DIGIO_1_PORT, VEX_DIGIO_1_PAD ) : \
                                       palSetPad(   VEX_DIGIO_1_PORT, VEX_DIGIO_1_PAD ) ; }
#define VexPinOutput_2(x)   { (x==0) ? palClearPad( VEX_DIGIO_2_PORT, VEX_DIGIO_2_PAD ) : \
                                       palSetPad(   VEX_DIGIO_2_PORT, VEX_DIGIO_2_PAD ) ; }
#define VexPinOutput_3(x)   { (x==0) ? palClearPad( VEX_DIGIO_3_PORT, VEX_DIGIO_3_PAD ) : \
                                       palSetPad(   VEX_DIGIO_3_PORT, VEX_DIGIO_3_PAD ) ; }
#define VexPinOutput_4(x)   { (x==0) ? palClearPad( VEX_DIGIO_4_PORT, VEX_DIGIO_4_PAD ) : \
                                       palSetPad(   VEX_DIGIO_4_PORT, VEX_DIGIO_4_PAD ) ; }
#define VexPinOutput_5(x)   { (x==0) ? palClearPad( VEX_DIGIO_5_PORT, VEX_DIGIO_5_PAD ) : \
                                       palSetPad(   VEX_DIGIO_5_PORT, VEX_DIGIO_5_PAD ) ; }
#define VexPinOutput_6(x)   { (x==0) ? palClearPad( VEX_DIGIO_6_PORT, VEX_DIGIO_6_PAD ) : \
                                       palSetPad(   VEX_DIGIO_6_PORT, VEX_DIGIO_6_PAD ) ; }
#define VexPinOutput_7(x)   { (x==0) ? palClearPad( VEX_DIGIO_7_PORT, VEX_DIGIO_7_PAD ) : \
                                       palSetPad(   VEX_DIGIO_7_PORT, VEX_DIGIO_7_PAD ) ; }
#define VexPinOutput_8(x)   { (x==0) ? palClearPad( VEX_DIGIO_8_PORT, VEX_DIGIO_8_PAD ) : \
                                       palSetPad(   VEX_DIGIO_8_PORT, VEX_DIGIO_8_PAD ) ; }
#define VexPinOutput_9(x)   { (x==0) ? palClearPad( VEX_DIGIO_9_PORT, VEX_DIGIO_9_PAD ) : \
                                       palSetPad(   VEX_DIGIO_9_PORT, VEX_DIGIO_9_PAD ) ; }
#define VexPinOutput_10(x)  { (x==0) ? palClearPad( VEX_DIGIO_10_PORT, VEX_DIGIO_10_PAD ) : \
                                       palSetPad(   VEX_DIGIO_10_PORT, VEX_DIGIO_10_PAD ) ; }
#define VexPinOutput_11(x)  { (x==0) ? palClearPad( VEX_DIGIO_11_PORT, VEX_DIGIO_11_PAD ) : \
                                       palSetPad(   VEX_DIGIO_11_PORT, VEX_DIGIO_11_PAD ) ; }
#define VexPinOutput_12(x)  { (x==0) ? palClearPad( VEX_DIGIO_12_PORT, VEX_DIGIO_12_PAD ) : \
                                       palSetPad(   VEX_DIGIO_12_PORT, VEX_DIGIO_12_PAD ) ; }
/** @} */

/*-----------------------------------------------------------------------------*/
/** @brief      Digital pin indexes                                            */
/*-----------------------------------------------------------------------------*/
typedef enum {
    kVexDigital_None = -1,
    kVexDigital_1 = 0,
    kVexDigital_2,
    kVexDigital_3,
    kVexDigital_4,
    kVexDigital_5,
    kVexDigital_6,
    kVexDigital_7,
    kVexDigital_8,
    kVexDigital_9,
    kVexDigital_10,
    kVexDigital_11,
    kVexDigital_12,

    kVexDigital_Num
    } tVexDigitalPin;

/*-----------------------------------------------------------------------------*/
/** @brief      Digital pin types                                              */
/*-----------------------------------------------------------------------------*/
typedef enum {
    kVexDigitalInput  = 0,
    kVexDigitalOutput = 1
    } tVexDigitalMode;

/*-----------------------------------------------------------------------------*/
/** @brief      Digital pin states                                             */
/*-----------------------------------------------------------------------------*/
typedef enum {
    kVexDigitalLow = 0,
    kVexDigitalHigh
    } tVexDigitalState;

/*-----------------------------------------------------------------------------*/
/** @brief      Holds port, pad and mode for a given cortex digital IO pin     */
/*-----------------------------------------------------------------------------*/
typedef struct _ioDef {
    ioportid_t      port;
    int16_t         pad;
    tVexDigitalMode mode;
    int32_t         intrCount;
} ioDef;

#ifdef __cplusplus
extern "C" {
#endif

// vexioDefinition is a global, one of the few
extern  ioDef   vexioDefinition[kVexDigital_Num];

// Safe IO
void                vexDigitalModeSet( tVexDigitalPin pin, tVexDigitalMode mode );
tVexDigitalMode     vexDigitalModeGet( tVexDigitalPin pin );
void                vexDigitalPinSet( tVexDigitalPin pin, tVexDigitalState state );
tVexDigitalState    vexDigitalPinGet( tVexDigitalPin pin );

void                vexDigitalIntrSet( tVexDigitalPin pin );
void                vexDigitalIntrRun(void);
int32_t             vexDigitalIntrCountGet( tVexDigitalPin pin );

// External interrupts
void                vexExtIrqInit(void);
void                vexExtSet( ioportid_t port, uint16_t channel, uint32_t mode, extcallback_t cb );

#ifdef __cplusplus
}
#endif


#endif  // __VEXDIGITAL__
