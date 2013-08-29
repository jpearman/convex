/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     apollo.c                                                     */
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
/*                                                                             */
/*    A status display for the vex firmware - an homage to old school          */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#include <string.h>
#include <stdarg.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "chprintf.h"
#include "vex.h"

#include "smartmotor.h"
#include "apollo.h"

/*-----------------------------------------------------------------------------*/
/** @file    apollo.c
  * @brief   Status display for VT100 console
  * @pre     This code need the smartMotor library
  * @details
  *  call apolloInit to start display
  *  call apolloUpdate in a loop until done
  *  call apolloDeinit to clear display before returning to normal cmd line
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/*  Needed to slow down communications from the cortex                         */
/*  serial comms to the master processor is twice as fast as comms out of the  */
/*  joystick programming interface.                                            */
/*  32 chars @ 230000 baud takes 1.4mS                                         */
/*  32 chars @ 115200 baud takes 2.8mS                                         */
/*  There is additional overhead due to the USB or WiFi interface              */
/*  We slow down to the equivalent of about 38400                              */
/*-----------------------------------------------------------------------------*/

static void
apolloPut( uint8_t c )
{
    static  int count = 0;

    // We are on a serial console
    sdPut( SD_CONSOLE, c );

    // add 8mS delay every 32 chars
    if( ++count == 32 )
        {
        count = 0;
        chThdSleepMilliseconds(8);
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Macro so we can use alternatives to apolloPut for testing      */
/*-----------------------------------------------------------------------------*/

#define vt100_putchar( c )              apolloPut((uint8_t)c);

/*-----------------------------------------------------------------------------*/
/*  This could be improved - 64 byte static buffer defined here                */
/*-----------------------------------------------------------------------------*/

static void
vt100_printf( char *fmt, ... )
{
    static  char    buffer[64];
    int     i;

    va_list args;

    va_start(args, fmt);

    vex_vsnprintf(buffer, 64, fmt, args );

    for(i=0;i<64&&buffer[i] != 0;i++ )
        vt100_putchar( buffer[i] );

    va_end(args);
}

/*-----------------------------------------------------------------------------*/
/** @name    vt100 shortcut macros
  * @{
*//*---------------------------------------------------------------------------*/
#define vt100_tl_corner()   vt100_putchar('l')
#define vt100_tr_corner()   vt100_putchar('k')
#define vt100_bl_corner()   vt100_putchar('m')
#define vt100_br_corner()   vt100_putchar('j')
#define vt100_hline()       vt100_putchar('q')
#define vt100_vline()       vt100_putchar('x')
#define vt100_tt()          vt100_putchar('w')
#define vt100_bt()          vt100_putchar('v')
#define vt100_lt()          vt100_putchar('t')
#define vt100_rt()          vt100_putchar('u')
/** @}  */

static inline void vt100_hidecursor(void)
{
    vt100_printf("\033[?25l");
}
static inline void vt100_showcursor(void)
{
    vt100_printf("\033[?25h");
}
static inline void vt100_clearscreen(void)
{
    vt100_printf("\033[2J");
}
static inline void vt100_graphics(void)
{
    vt100_printf("\033(0");
}
static inline void vt100_ascii(void)
{
    vt100_printf("\033(B");
}
static inline void vt100_cursor(int col, int row )
{
    vt100_printf("\033[%d;%dH", row, col);
}
static inline void vt100_videomode(int mode)
{
    vt100_printf("\033[%dm", mode);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Draw a box on the vt100 console                                */
/** @param[in]  col Starting column                                            */
/** @param[in]  row Starting row                                               */
/** @param[in]  width The box width                                            */
/** @param[in]  height The box height                                          */
/*-----------------------------------------------------------------------------*/

void
vt100_drawbox( int col, int row, int width, int height )
{
    int     i;

    // Draw top line and left and right corners
    vt100_cursor(col, row);
    vt100_tl_corner();
    for(i=0;i<width-2;i++)
        vt100_hline();
    vt100_tr_corner();

    // draw vertical lines
    for(i=1;i<(height-1);i++) {
        vt100_cursor( col, row+i );
        vt100_vline();

        vt100_cursor( col+width-1, row+i );
        vt100_vline();
    }

    // Draw bottom line and left and right corners
    vt100_cursor( col,  row+height-1 );
    vt100_bl_corner();
    for(i=0;i<width-2;i++)
        vt100_hline();
    vt100_br_corner();
}


/*-----------------------------------------------------------------------------*/
/*  Setup functions to draw static display items                               */
/*  these are generally called once from the init function                     */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      Draw static lines for the display                              */
/*-----------------------------------------------------------------------------*/

void
apolloScreenSetup()
{
    // outline
    vt100_drawbox( T_X_ORIGIN, T_Y_ORIGIN, T_WIDTH, T_HEIGHT );

    // motors
    vt100_drawbox( M_X_POS, M_Y_POS, M_WIDTH, M_HEIGHT );
    // digital
    vt100_drawbox( D_X_POS, D_Y_POS, D_WIDTH, D_HEIGHT );
    // analog
    vt100_drawbox( A_X_POS, A_Y_POS, A_WIDTH, A_HEIGHT );

    // fix some intersections
    vt100_cursor( M_X_POS, M_Y_POS+M_HEIGHT-1);
    vt100_lt();
    vt100_cursor( T_WIDTH, A_Y_POS+A_HEIGHT-1);
    vt100_rt();

    vt100_cursor( D_X_POS, D_Y_POS);
    vt100_tt();
    vt100_cursor( A_X_POS, A_Y_POS);
    vt100_tt();

    vt100_cursor( D_X_POS, D_Y_POS+D_HEIGHT-1);
    vt100_bt();
    vt100_cursor( A_X_POS, A_Y_POS+A_HEIGHT-1);
    vt100_bt();

}

/*-----------------------------------------------------------------------------*/
/** @brief      Setup motor section of the display                             */
/*-----------------------------------------------------------------------------*/
void
apolloMotorSetup()
{
    int     i;

    // Motor labels
    vt100_cursor( M_X_POS+1, M_Y_POS+1 );

    vt100_videomode(7);
    vt100_printf(" MTR EID  CMD ENCODER  RPM  AMPS  TEMP ");
    vt100_videomode(0);

    for(i=1;i<=10;i++) {
        vt100_cursor( M_X_POS+2, M_Y_POS+1+i);
        vt100_printf("%2d", i);
    }

    vt100_cursor( M_X_POS+2, M_Y_POS+1+11);
    vt100_printf("C 1-5 ");
    vt100_cursor( M_X_POS+2, M_Y_POS+1+12);
    vt100_printf("C 6-10");
}

/*-----------------------------------------------------------------------------*/
/** @brief      Setup digital port status section of the display               */
/*-----------------------------------------------------------------------------*/
void
apolloDigitalSetup()
{
    int     i;

    // Digital labels
    vt100_cursor( D_X_POS+1, D_Y_POS+1 );

    vt100_videomode(7);
    vt100_printf("  DIGITAL          ");
    vt100_videomode(0);

    for(i=1;i<=12;i++) {
        vt100_cursor( D_X_POS+2, D_Y_POS+1+i);
        vt100_printf("%2d %s", i, vexDigitalTypeGetName(i-1) );
    }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Setup analog port status section of the display                */
/*-----------------------------------------------------------------------------*/
void
apolloAnalogSetup()
{
    int     i;

    // Analog labels
    vt100_cursor( A_X_POS+1, A_Y_POS+1 );

    vt100_videomode(7);
    vt100_printf("  ANALOG          ");
    vt100_videomode(0);

    for(i=1;i<=8;i++) {
        vt100_cursor( A_X_POS+2, A_Y_POS+1+i);
        vt100_printf("%2d", i);
    }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Setup joystick status section of the display                   */
/*-----------------------------------------------------------------------------*/
void
apolloJoystickSetup( int joystick )
{
    int offset = ( joystick == 0 ? 0 : 40 );

    vt100_videomode(7);
    vt100_cursor( offset + J_X_POS+7, J_Y_POS+1 );
    vt100_printf("  U  D  L  R ");
    vt100_cursor( offset + J_X_POS+26, J_Y_POS+1 );
    vt100_printf("    H     V ");
    vt100_cursor( offset + J_X_POS+26, J_Y_POS+4 );
    vt100_printf("    X     Y ");

    vt100_cursor( offset + J_X_POS+1, J_Y_POS+2 );
    vt100_printf(" BTN5 ");
    vt100_cursor( offset + J_X_POS+1, J_Y_POS+3 );
    vt100_printf(" BTN6 ");
    vt100_cursor( offset + J_X_POS+1, J_Y_POS+4 );
    vt100_printf(" BTN7 ");
    vt100_cursor( offset + J_X_POS+1, J_Y_POS+5 );
    vt100_printf(" BTN8 ");

    vt100_cursor( offset + J_X_POS+20, J_Y_POS+2 );
    vt100_printf(" JS_1 ");
    vt100_cursor( offset + J_X_POS+20, J_Y_POS+3 );
    vt100_printf(" JS_2 ");
    vt100_cursor( offset + J_X_POS+20, J_Y_POS+5 );
    vt100_printf(" ACCL ");
    vt100_videomode(0);
}

/*-----------------------------------------------------------------------------*/
/*  Update variables                                                           */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      Update motor status                                            */
/** @param[in]  m The smart motor port number (0 - 9)                          */
/*-----------------------------------------------------------------------------*/
void
apolloUpdateMotor( int m )
{
    int      id = vexMotorEncoderIdGet(m);
    int   value = vexMotorGet(m);
    long    enc = vexMotorPositionGet(m);
    int     rpm = SmartMotorGetSpeed(m);
    double amps = (double)SmartMotorGetCurrent(m);
    double temp = SmartMotorGetTemperature(m);
    char   rev;

    vt100_cursor( M_X_POS+6, M_Y_POS+2+m );
    if( vexMotorTypeGet(m) != kVexMotorUndefined )
        {
        if( vexMotorDirectionGet(m) == 1)
            rev = 'R';
        else
            rev = ' ';

        if( id >= 0 )
            vt100_printf("%2d %c%4d %7d %4d %5.2f  %4.1f", id, rev, value, enc, rpm, amps, temp );
        else
            vt100_printf("-- %c%4d %7d %4d %5.2f  %4.1f",     rev, value, enc, rpm, amps, temp );
        }
    else
        vt100_printf("    ----  ------  ---  ----  ----");
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update cortex current and temperature status                   */
/** @param[in]  p The smart motor port number (0 or 1)                         */
/*-----------------------------------------------------------------------------*/
void
apolloUpdateCortex( int p )
{
    double amps = SmartMotorGetControllerCurrent(p);
    double temp = SmartMotorGetControllerTemperature(p);

    vt100_cursor( M_X_POS+28, M_Y_POS+12+p );
    vt100_printf("%5.2f  %4.1f", amps, temp );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update digital port status                                     */
/** @param[in]  pin The digital port number                                    */
/*-----------------------------------------------------------------------------*/
void
apolloUpdateDigital(int pin)
{
    tVexSensorType  type;

    vt100_cursor(D_X_POS+12, D_Y_POS+2+pin);

    type = vexDigitalTypeGet( pin );

    switch(type)
        {
        case    kVexSensorDigitalInput:
            vt100_printf("%7d", vexDigitalPinGet(pin));
            break;
        case    kVexSensorDigitalOutput:
            vt100_printf("%7d", vexDigitalPinGet(pin));
            break;
        case    kVexSensorInterrupt:
            vt100_printf("%7d", vexDigitalIntrCountGet(pin));
            break;
        case    kVexSensorQuadEncoder:
            if( vexDigitalCfgGet(pin) == kVexConfigQuadEnc1 )
                vt100_printf("%7d", vexEncoderGet( vexDigitalChannelGet(pin) ) );
            break;
        case    kVexSensorSonarCm:
            if( vexDigitalCfgGet(pin) == kVexConfigSonarOut )
                vt100_printf("%7d", vexSonarGetCm( vexDigitalChannelGet(pin) ) );
            break;
        case    kVexSensorSonarInch:
            if( vexDigitalCfgGet(pin) == kVexConfigSonarOut )
                vt100_printf("%7d", vexSonarGetInch( vexDigitalChannelGet(pin) ) );
            break;

        default:
            break;
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update analog port status                                      */
/** @param[in]  pin The analog port number                                     */
/*-----------------------------------------------------------------------------*/
void
apolloUpdateAnalog(int pin)
{
    vt100_cursor( A_X_POS+12, A_Y_POS+2+pin );

    vt100_printf("%4d", vexAdcGet(pin));
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update system status                                           */
/*-----------------------------------------------------------------------------*/
void
apolloUpdateSystem()
{
    long    systime = chTimeNow()/1000;
    int     hours, mins, secs;

    vt100_cursor( T_X_ORIGIN+2, T_Y_ORIGIN+T_HEIGHT-2 );

    hours = systime / 3600;
    mins  = (systime - (hours * 3600)) / 60;
    secs = systime % 60;

    vt100_printf("%02d:%02d:%02d  ", hours, mins, secs );
    vt100_printf("Main %4.2fV  ", vexSpiGetMainBattery()/1000.0);
    vt100_printf("Backup %4.2fV", vexSpiGetBackupBattery()/1000.0);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update joystick button status                                  */
/** @param[in]  joystick Which joystick, 0 for main, 1 for partner             */
/** @param[in]  row The row for the data (0 - 4)                               */
/** @param[in]  up Up button status                                            */
/** @param[in]  down Up down status                                            */
/** @param[in]  left Up left status                                            */
/** @param[in]  right Up right status                                          */
/*-----------------------------------------------------------------------------*/
void
apolloUpdateJoystickButton( int joystick, int row, int up, int down, int left, int right )
{
    int offset = ( joystick == 0 ? 0 : 40 );

    vt100_cursor( offset + J_X_POS+9, J_Y_POS+2+row );
    if(left >= 0)
        vt100_printf("%d  %d  %d  %d", up, down, left, right);
    else
        vt100_printf("%d  %d", up, down);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update joystick analog status                                  */
/** @param[in]  joystick Which joystick, 0 for main, 1 for partner             */
/** @param[in]  row The row for the data (0, 1 or 2)                           */
/** @param[in]  h the horizontal joystick data                                 */
/** @param[in]  v the vertical joystick data                                   */
/*-----------------------------------------------------------------------------*/
void
apolloUpdateJoystickAnalog( int joystick, int row, int h, int v )
{
    int offset = ( joystick == 0 ? 0 : 40 );

    vt100_cursor( offset + J_X_POS+27, J_Y_POS+2+row );

    vt100_printf("%4d  %4d", h, v);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Apollo initialization                                          */
/*-----------------------------------------------------------------------------*/
void
apolloInit()
{
    // clear screen
    vt100_clearscreen();
    // hide the cursor (may or may not work)
    vt100_hidecursor();

    // graphics mode
    vt100_graphics();
    // Setup overlay with lines etc.
    apolloScreenSetup();
    
    // normal mode
    vt100_ascii();
    // setup display labels
    apolloMotorSetup();
    apolloDigitalSetup();
    apolloAnalogSetup();
    apolloJoystickSetup(0);
    if( (vexControllerCompetitonState() & kFlagXmit2 ) == kFlagXmit2 )
        apolloJoystickSetup(1);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Apollo de-init.  Cleans up screen when done                    */
/*-----------------------------------------------------------------------------*/
void
apolloDeinit()
{
    vt100_cursor(T_X_ORIGIN, T_HEIGHT);
    // clearscreen
    vt100_clearscreen();
    // re-enable cursor
    vt100_showcursor();
    // normal character set
    vt100_ascii();
}

/*-----------------------------------------------------------------------------*/
/** @brief      Apollo update - update all status                              */
/*-----------------------------------------------------------------------------*/
void
apolloUpdate()
{
    int     i;

    // all motor values
    for(i=0;i<10;i++)
        apolloUpdateMotor( i );

    // all digital values
    for(i=0;i<12;i++)
        apolloUpdateDigital( i );

    // all analog values
    for(i=0;i<8;i++)
        apolloUpdateAnalog( i );

    // update current and temp for cortex
    apolloUpdateCortex(0);
    apolloUpdateCortex(1);
    apolloUpdateSystem();

    // display the joystick data
    apolloUpdateJoystickButton( 0, 0, vexControllerGet(Btn5U), vexControllerGet(Btn5D), -1, -1 );
    apolloUpdateJoystickButton( 0, 1, vexControllerGet(Btn6U), vexControllerGet(Btn6D), -1, -1 );
    apolloUpdateJoystickButton( 0, 2, vexControllerGet(Btn7U), vexControllerGet(Btn7D), vexControllerGet(Btn7L), vexControllerGet(Btn7R) );
    apolloUpdateJoystickButton( 0, 3, vexControllerGet(Btn8U), vexControllerGet(Btn8D), vexControllerGet(Btn8L), vexControllerGet(Btn8R) );
    apolloUpdateJoystickAnalog( 0, 0, vexControllerGet(Ch4), vexControllerGet(Ch3) );
    apolloUpdateJoystickAnalog( 0, 1, vexControllerGet(Ch1), vexControllerGet(Ch2) );
    apolloUpdateJoystickAnalog( 0, 3, vexControllerGet(AcclX), vexControllerGet(AcclY) );

    if( (vexControllerCompetitonState() & kFlagXmit2 ) == kFlagXmit2 )
        {
        // display the joystick data
        apolloUpdateJoystickButton( 1, 0, vexControllerGet(Btn5UXmtr2), vexControllerGet(Btn5DXmtr2), -1, -1 );
        apolloUpdateJoystickButton( 1, 1, vexControllerGet(Btn6UXmtr2), vexControllerGet(Btn6DXmtr2), -1, -1 );
        apolloUpdateJoystickButton( 1, 2, vexControllerGet(Btn7UXmtr2), vexControllerGet(Btn7DXmtr2), vexControllerGet(Btn7LXmtr2), vexControllerGet(Btn7RXmtr2) );
        apolloUpdateJoystickButton( 1, 3, vexControllerGet(Btn8UXmtr2), vexControllerGet(Btn8DXmtr2), vexControllerGet(Btn8LXmtr2), vexControllerGet(Btn8RXmtr2) );
        apolloUpdateJoystickAnalog( 1, 0, vexControllerGet(Ch4Xmtr2), vexControllerGet(Ch3Xmtr2) );
        apolloUpdateJoystickAnalog( 1, 1, vexControllerGet(Ch1Xmtr2), vexControllerGet(Ch2Xmtr2) );
        apolloUpdateJoystickAnalog( 1, 3, vexControllerGet(AcclXXmtr2), vexControllerGet(AcclYXmtr2) );
        }

    // move cursor out of the way
    vt100_cursor(T_WIDTH, T_HEIGHT);
}
