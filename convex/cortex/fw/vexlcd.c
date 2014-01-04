/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexlcd.c                                                     */
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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexlcd.c
  * @brief   LCD driver
*//*---------------------------------------------------------------------------*/

/** @brief      Number of LCD displays supported */
#define LCD_DISPLAYS        2

// storage for the LCD data
static  LcdData     vexLcdData[LCD_DISPLAYS] = {
        {NULL, 0, VEX_LCD_BACKLIGHT, 0, "","","","" },
        {NULL, 0, VEX_LCD_BACKLIGHT, 0, "","","","" }
};

// flag to indicate of we have already created the lcd thread
static  int16_t     lcd_running = 0;

// static memory buffer for VexLcdPrintf to deal with long format strings
/** @brief  Size of the LCD output buffer */
#define LCD_BUF_MAX     32
static  char        lcd_buffer[LCD_BUF_MAX];

// stack for thread
static WORKING_AREA(waVexLcdUpdate, LCD_TASK_STACK_SIZE);

/*-----------------------------------------------------------------------------*/
/*  LCD update thread                                                          */
/*-----------------------------------------------------------------------------*/

static msg_t
VexLcdUpdate(void *arg) {

  (void)arg;
  chRegSetThreadName("lcd");

  while (TRUE) {
        // send data for line 0
        if( vexLcdData[0].enabled )
            vexLcdSendMessage( &vexLcdData[0], 0 );
        if( vexLcdData[1].enabled )
            vexLcdSendMessage( &vexLcdData[1], 0 );

        chThdSleepMilliseconds(25);

        // check for received message from last time
        if( vexLcdData[0].enabled )
            vexLcdCheckReceiveMessage( &vexLcdData[0] );
        if( vexLcdData[1].enabled )
            vexLcdCheckReceiveMessage( &vexLcdData[1] );

        // send data for line 2
        if( vexLcdData[0].enabled )
            vexLcdSendMessage( &vexLcdData[0], 1 );
        if( vexLcdData[1].enabled )
            vexLcdSendMessage( &vexLcdData[1], 1 );

        chThdSleepMilliseconds(25);

        // check for received message from last time
        if( vexLcdData[0].enabled )
            vexLcdCheckReceiveMessage( &vexLcdData[0] );
        if( vexLcdData[1].enabled )
            vexLcdCheckReceiveMessage( &vexLcdData[1] );
    }

  return (msg_t)0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize a LCD and bind to serial port                       */
/** @param[in]  display The LCD display id, should be 0 or 1                   */
/** @param[in]  sdp A pointer to the serial driver (chibios hal serial driver) */
/*-----------------------------------------------------------------------------*/

void
vexLcdInit( int16_t display, SerialDriver *sdp )
{
    uint16_t    i;

    if( (display < 0) || (display >= LCD_DISPLAYS) )
        return;

    // display can be 0 or 1
    if( sdp != NULL )
        {
        vexLcdData[display].sdp = sdp;

        // clear display lines
        for(i=0;i<sizeof( vexLcdData[display].line1 );i++)
            vexLcdData[display].line1[i] = 0x20;
        vexLcdData[display].line1[16] = 0;

        for(i=0;i<sizeof( vexLcdData[display].line2 );i++)
            vexLcdData[display].line2[i] = 0x20;
        vexLcdData[display].line2[16] = 0;

        // clear buffers
        for(i=0;i<sizeof( vexLcdData[display].txbuf );i++)
            vexLcdData[display].txbuf[i] = 0;
        for(i=0;i<sizeof( vexLcdData[display].rxbuf );i++)
            vexLcdData[display].rxbuf[i] = 0;
        vexLcdData[display].enabled = 1;

        // start task if it is not running
        if(!lcd_running)
            {
            lcd_running = 1;
            chThdCreateStatic(waVexLcdUpdate, sizeof(waVexLcdUpdate), LCD_THREAD_PRIORITY, VexLcdUpdate, NULL);
            }
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Show text from beginning of an LCD line                        */
/** @param[in]  display The LCD display id, 0 or 1                             */
/** @param[in]  line The line to display the text on, 0 or 1                   */
/** @param[in]  buf Pointer to char buffer with test to display                */
/*-----------------------------------------------------------------------------*/

void
vexLcdSet( int16_t display, int16_t line, char *buf )
{
    if( (display < 0) || (display >= LCD_DISPLAYS) )
        return;

    if( !line  )
        strncpy( vexLcdData[display].line1, buf, 16 );
    else
        strncpy( vexLcdData[display].line2, buf, 16 );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Show text on LCD line and column                               */
/** @param[in]  display The LCD display id, 0 or 1                             */
/** @param[in]  line The line to display the text on, 0 or 1                   */
/** @param[in]  col The position at which to start displaying text             */
/** @param[in]  buf Pointer to char buffer with test to display                */
/*-----------------------------------------------------------------------------*/

void
vexLcdSetAt( int16_t display, int16_t line, int16_t col, char *buf )
{
    if( (display < 0) || (display >= LCD_DISPLAYS) )
        return;

    if( (col < 0) || (col > 15 ) )
        return;

    if( !line  )
        strncpy( &vexLcdData[display].line1[col], buf, 16-col );
    else
        strncpy( &vexLcdData[display].line2[col], buf, 16-col );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Formated output to LCD - similar to printf                     */
/** @param[in]  display The LCD display id, 0 or 1                             */
/** @param[in]  line The line to display the text on, 0 or 1                   */
/** @param[in]  fmt A sprintf style format string with optional variables      */
/*-----------------------------------------------------------------------------*/

void
vexLcdPrintf( int16_t display, int16_t line, char *fmt, ... )
{
    va_list args;

    va_start(args, fmt);

    //vsprintf(lcd_buffer, fmt, args );
    vex_vsnprintf(lcd_buffer, LCD_BUF_MAX, fmt, args );

    vexLcdSet( display, line, lcd_buffer );

    va_end(args);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Formated output to LCD - similar to printf                     */
/** @param[in]  display The LCD display id, 0 or 1                             */
/** @param[in]  line The line to display the text on, 0 or 1                   */
/** @param[in]  col The position at which to start displaying text             */
/** @param[in]  fmt A sprintf style format string with optional variables      */
/*-----------------------------------------------------------------------------*/

void
vexLcdPrintfAt( int16_t display, int16_t line, int16_t col, char *fmt, ... )
{
    va_list args;

    va_start(args, fmt);

    //vsprintf(lcd_buffer, fmt, args );
    vex_vsnprintf(lcd_buffer, LCD_BUF_MAX, fmt, args );

    vexLcdSetAt( display, line, col, lcd_buffer );

    va_end(args);
}
/*-----------------------------------------------------------------------------*/
/** @brief      Clear a given lcd display line                                 */
/** @param[in]  display The LCD display id, 0 or 1                             */
/** @param[in]  line The line to display the text on, 0 or 1                   */
/*-----------------------------------------------------------------------------*/

void
vexLcdClearLine( int16_t display, int16_t line )
{
    vexLcdSet( display, line, "                ");
}

/*-----------------------------------------------------------------------------*/
/** @brief      Turn on or off the backlight                                   */
/** @param[in]  display The LCD display id, 0 or 1                             */
/** @param[in]  value 1 for backlight on, 0 for off                            */
/*-----------------------------------------------------------------------------*/

void
vexLcdBacklight( int16_t display, int16_t value )
{
    if( (display < 0) || (display >= LCD_DISPLAYS) )
        return;

    vexLcdData[display].flags = (value == 1) ?  vexLcdData[display].flags | VEX_LCD_BACKLIGHT : vexLcdData[display].flags & ~VEX_LCD_BACKLIGHT;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the current button status                                  */
/** @param[in]  display The LCD display id, 0 or 1                             */
/** @returns    The button (or buttons) pressed                                */
/*-----------------------------------------------------------------------------*/

vexLcdButton
vexLcdButtonGet( int16_t display )
{
    if( (display < 0) || (display >= LCD_DISPLAYS) )
        return( kLcdButtonNone );

    return( (vexLcdButton)vexLcdData[display].buttons );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Form and send message to LCD                                   */
/** @param[in]  lcd Pointer to LcdData structure                               */
/** @param[in]  line The line to display the text on, 0 or 1                   */
/** @note       Internal driver use only                                       */
/*-----------------------------------------------------------------------------*/

void
vexLcdSendMessage( LcdData *lcd, int16_t line )
{
    // create message
    int16_t i, cs;

    // bounds check line variable
    if( ( line < 0 ) || (line > 1) )
        return;

    // Header for LCD communication
    lcd->txbuf[0] = 0xAA;
    lcd->txbuf[1] = 0x55;
    lcd->txbuf[2] = 0x1e;
    lcd->txbuf[3] = 0x12;
    lcd->txbuf[4] = (lcd->flags & VEX_LCD_BACKLIGHT) ? 0x02 + line : line;

    // fill with spaces
    for(i=0;i<16;i++)
        lcd->txbuf[ 5+i ] = 0x20;

    // Copy data transmit buffer
    if(!line) {
        for(i=0;i<16;i++)
            if( lcd->line1[i] != 0)
                lcd->txbuf[ 5+i ] = lcd->line1[i];
        }
    else {
        for(i=0;i<16;i++) {
            if( lcd->line2[i] != 0)
                lcd->txbuf[ 5+i ] = lcd->line2[i];
            }
        }

    // calculate checksum
    cs = 0;
    for(i=4;i<21;i++)
         cs = cs + lcd->txbuf[i];

    lcd->txbuf[21] = 0x100 - cs;

    // send to port
    sdWrite( lcd->sdp, (unsigned char *)lcd->txbuf, 22);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Check for receive data and store                               */
/** @param[in]  lcd Pointer to LcdData structure                               */
/** @note       Internal driver use only                                       */
/*-----------------------------------------------------------------------------*/

void
vexLcdCheckReceiveMessage( LcdData *lcd )
{
    int16_t i;

    // any characters
    if( sdGetWouldBlock(lcd->sdp) )
        return;

    // read up to 16 bytes from serial port
    for(i=0;i<16;i++)
        {
        int16_t c;

        c = sdGetTimeout( lcd->sdp, TIME_IMMEDIATE);
        if( c != Q_TIMEOUT )
            lcd->rxbuf[i] = c;
        else
            break;
        }

    // 6 chars ?
    if( i == 6 )
        {
        // lcd message ?
        if( (lcd->rxbuf[0] == 0xAA) && (lcd->rxbuf[1] == 0x55) && (lcd->rxbuf[2] == 0x16))
            // verify checksum
            if( !((lcd->rxbuf[4] + lcd->rxbuf[5]) & 0xFF) )
                lcd->buttons = lcd->rxbuf[4];
        }

    // flush anything left
    if( !sdGetWouldBlock(lcd->sdp) )
        {
        chSysLock();
        chIQResetI( &(lcd->sdp)->iqueue );
        chSysUnlock();
        }

}

/*-----------------------------------------------------------------------------*/
/** @brief      Debug function to dump LCD buffers                             */
/** @param[in]  chp     A pointer to a vexStream object                      */
/** @param[in]  argc    The number of command line arguments                   */
/** @param[in]  argv    An array of pointers to the command line args          */
/*-----------------------------------------------------------------------------*/

void
vexLcdDebug(vexStream *chp, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    if( vexLcdData[0].enabled){
        vex_chprintf(chp,"\r\n" );
        vex_chprintf(chp,"LCD 0 \r\n" );
        vex_chprintf(chp,"%-16s\r\n", vexLcdData[0].line1 );
        vex_chprintf(chp,"%-16s\r\n", vexLcdData[0].line2 );
        vex_chprintf(chp,"Buttons = %2X\r\n", vexLcdData[0].buttons );
        }

    if( vexLcdData[1].enabled){
        vex_chprintf(chp,"\r\n" );
        vex_chprintf(chp,"LCD 1 \r\n" );
        vex_chprintf(chp,"%-16s\r\n", vexLcdData[1].line1 );
        vex_chprintf(chp,"%-16s\r\n", vexLcdData[1].line2 );
        vex_chprintf(chp,"Buttons = %2X\r\n", vexLcdData[1].buttons );
        }
}

