/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexshell.h                                                   */
/*    Author:     James Pearman                                                */
/*    Created:    15 May 2013                                                  */
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
/*    Customized version of the ChibiOS/RT shell using the same names          */
/*    Banner message is different                                              */
/*    One level of history implemented using up arrow key                      */
/*    Detection of (127) as backspace from the screen program running on OSX   */
/*    My weird and rather old fashioned formatting                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio                   */
/*                                                                             */
/*    Licensed under the Apache License, Version 2.0 (the "License");          */
/*    you may not use this file except in compliance with the License.         */
/*    You may obtain a copy of the License at                                  */
/*                                                                             */
/*        http://www.apache.org/licenses/LICENSE-2.0                           */
/*                                                                             */
/*    Unless required by applicable law or agreed to in writing, software      */
/*    distributed under the License is distributed on an "AS IS" BASIS,        */
/*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/*    See the License for the specific language governing permissions and      */
/*    limitations under the License.                                           */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @file    vexshell.h
  * @brief   Simple CLI shell header.
*//*---------------------------------------------------------------------------*/

#ifndef _VEXSHELL_
#define _VEXSHELL_

/*-----------------------------------------------------------------------------*/
/** @brief   Shell maximum input line length.                                  */
/*-----------------------------------------------------------------------------*/

#if !defined(SHELL_MAX_LINE_LENGTH) || defined(__DOXYGEN__)
#define SHELL_MAX_LINE_LENGTH       64
#endif

/*-----------------------------------------------------------------------------*/
/** @brief   Shell maximum arguments per command.                              */
/*-----------------------------------------------------------------------------*/
#if !defined(SHELL_MAX_ARGUMENTS) || defined(__DOXYGEN__)
#define SHELL_MAX_ARGUMENTS         4
#endif

/*-----------------------------------------------------------------------------*/
/** @brief   Command handler function type.                                    */
/*-----------------------------------------------------------------------------*/
typedef void (*shellcmd_t)(vexStream *chp, int argc, char *argv[]);

/*-----------------------------------------------------------------------------*/
/** @brief   Custom command entry type.                                        */
/*-----------------------------------------------------------------------------*/
typedef struct {
  const char           *sc_name;            /**< @brief Command name.       */
  shellcmd_t            sc_function;        /**< @brief Command function.   */
} ShellCommand;

/*-----------------------------------------------------------------------------*/
/** @brief   Shell descriptor type.                                            */
/*-----------------------------------------------------------------------------*/
typedef struct {
  vexStream            *sc_channel;        /**< @brief I/O channel associated
                                                 to the shell.              */
  const ShellCommand   *sc_commands;       /**< @brief Shell extra commands
                                                 table.                     */
} ShellConfig;

#if !defined(__DOXYGEN__)
extern EventSource shell_terminated;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void        shellInit(void);
Thread     *shellCreate(const ShellConfig *scp, size_t size, tprio_t prio);
Thread     *shellCreateStatic(const ShellConfig *scp, void *wsp, size_t size, tprio_t prio);
bool_t      shellGetLine(vexStream *chp, char *line, unsigned size);

#ifdef __cplusplus
}
#endif

#endif /* _VEXSHELL_ */

