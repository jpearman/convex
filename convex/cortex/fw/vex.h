/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vex.h                                                        */
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

#ifndef __VEX__
#define __VEX__

/*-----------------------------------------------------------------------------*/
/** @file    vex.h
  * @brief   Main vex include file
*//*---------------------------------------------------------------------------*/

#include <stdarg.h>

// Must come first - contains compatibility definitions
#include "vexcompat.h"

#include "vexanalog.h"
#include "vexdigital.h"
#include "vexencoder.h"
#include "vexsonar.h"
#include "vexmotor.h"
#include "vexime.h"
#include "vexconfig.h"
#include "vexspi.h"
#include "vexlcd.h"
#include "vexctl.h"
#include "vexaudio.h"
#include "vexsensor.h"
#include "vexprintf.h"
#include "vexshell.h"
#include "vexbkup.h"

/**
 * @brief   ConVEX version string.
 */
#define CONVEX_VERSION       "1.0.6"

/**
 * @name    ConVEX version
 * @{
 */
/**
 * @brief   ConVEX version major number.
 */
#define CONVEX_MAJOR         1

/**
 * @brief   ConVEX version minor number.
 */
#define CONVEX_MINOR         0

/**
 * @brief   ConVEX version patch number.
 */
#define CONVEX_PATCH         4
/** @} */

// ChibiOS version as a hex number, eg. 2.6.1 becomes 0x261
#define CH_KERNEL_VERSION_HEX    ((CH_KERNEL_MAJOR << 8) | (CH_KERNEL_MINOR << 4) | (CH_KERNEL_PATCH))

/*-----------------------------------------------------------------------------*/
/** @{                                                                         */
/** @name task priorities                                                      */
/*-----------------------------------------------------------------------------*/
#define SYSTEM_THREAD_PRIORITY      NORMALPRIO + 10
#define IME_THREAD_PRIORITY         NORMALPRIO + 9
#define MONITOR_THREAD_PRIORITY     NORMALPRIO + 8
#define SONAR_THREAD_PRIORITY       NORMALPRIO + 8
#define LCD_THREAD_PRIORITY         NORMALPRIO
#define USER_THREAD_PRIORITY        NORMALPRIO + 1
/** @} */

/*-----------------------------------------------------------------------------*/
/** @{                                                                         */
/** @name Stack sizes                                                          */
/*-----------------------------------------------------------------------------*/
#define USER_TASK_STACK_SIZE        512
#define IME_TASK_STACK_SIZE         0x1D0
#define SONAR_TASK_STACK_SIZE       0xD0
#define LCD_TASK_STACK_SIZE         0xD0
#define TEST_TASK_STACK_SIZE        0xD0
#define SYSTEM_TASK_STACK_SIZE      0xD0
#define MONITOR_TASK_STACK_SIZE     0x1D0
#define AUDIO_TASK_STACK_SIZE       0xD0
/** @} */

/*-----------------------------------------------------------------------------*/
// Serial ports swap around depending on the board
#ifdef  BOARD_OLIMEX_STM32_P103
#define SD_CONSOLE     &SD2
#define SD_LCD1        &SD1
#else
#define SD_CONSOLE     &SD1
#define SD_LCD1        &SD2
#endif

#define SD_LCD2        &SD3

/*-----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------*/
// Competition top level tasks and functions
void        vexUserSetup(void)   __attribute__ ((weak));
void        vexUserInit( void )  __attribute__ ((weak));
msg_t       vexAutonomous( void *arg );
msg_t       vexOperator( void *arg );

/*-----------------------------------------------------------------------------*/
// System related functions
void        vexCortexInit( void );
void        vexConsoleInit( void );
void        vexTaskRegister( char *name );
void        vexTaskRegisterPersistant(char *name, bool_t p );
bool_t      vexTaskIsRegistered( Thread *tp );
bool_t      vexTaskPersistentGet( Thread *tp );
void        vexTaskPersistentSet( Thread *tp, bool_t p );

void        vexTaskEmergencyStop( void );
void        vexSleep( int32_t msec );

//#define     VEX_WATCHDOG_ENABLE     1
void        vexWatchdogInit(void);
void        vexWatchdogReload(void);
int16_t     vexWatchdogResetFlagGet(void);

/*-----------------------------------------------------------------------------*/
// used for trying out code
void        vexTest( void );
void        vexTestDebug(vexStream *chp, int argc, char *argv[]);
void        vexServiceEncoder( short source, short pa, short pb, long *encoder );

#ifdef __cplusplus
}
#endif

#endif  // __VEX__
