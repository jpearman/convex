/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexlcd.h                                                     */
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

#ifndef __VEXLCD__
#define __VEXLCD__

/*-----------------------------------------------------------------------------*/
/** @file    vexlcd.h
  * @brief   LCD driver macros and prototypes
*//*---------------------------------------------------------------------------*/

#define VEX_LCD_BACKLIGHT   0x01    ///< bit for turning backlight on


#define VEX_LCD_DISPLAY_1   0       ///< LCD display on UART1
#define VEX_LCD_DISPLAY_2   1       ///< LCD display on UART2

#define VEX_LCD_LINE_1      0       ///< Upper LCD display line
#define VEX_LCD_LINE_2      1       ///< Lower LCD display line
#define VEX_LCD_LINE_T      0       ///< Upper LCD display line
#define VEX_LCD_LINE_B      1       ///< Lower LCD display line

/*-----------------------------------------------------------------------------*/
/** @brief      Holds all data for a single LCD display                        */
/*-----------------------------------------------------------------------------*/
typedef struct _LcdData {
    SerialDriver *sdp;
    short         enabled;
    unsigned char flags;
    unsigned char buttons;
    char          line1[20];
    char          line2[20];
    char          txbuf[32];
    char          rxbuf[16];
} LcdData;

typedef enum _vexLcdButton {
    kLcdButtonNone            = 0,
    kLcdButtonLeft            = 1,
    kLcdButtonCenter          = 2,
    kLcdButtonLeftCenter      = 3,
    kLcdButtonRight           = 4,
    kLcdButtonLeftRight       = 5,
    kLcdButtonCenterRight     = 6,
    kLcdButtonLeftCenterRight = 7
} vexLcdButton;

#ifdef __cplusplus
extern "C" {
#endif

// LCD interface
void        vexLcdInit( int16_t display, SerialDriver *sdp );
void        vexLcdSet( int16_t display, int16_t line, char *buf );
void        vexLcdSetAt( int16_t display, int16_t line, int16_t col, char *buf );
void        vexLcdPrintf( int16_t display, int16_t line, char *fmt, ... );
void        vexLcdPrintfAt( int16_t display, int16_t line, int16_t col, char *fmt, ... );

void        vexLcdClearLine( int16_t display, int16_t line );
void        vexLcdBacklight( int16_t display, int16_t value );

vexLcdButton vexLcdButtonGet( int16_t display );

// private
void        vexLcdCheckReceiveMessage( LcdData *lcd );
void        vexLcdSendMessage( LcdData *lcd, int16_t line );
void        vexLcdDebug(vexStream *chp, int argc, char *argv[]);


#ifdef __cplusplus
}
#endif

#endif  // __VEXLCD__
