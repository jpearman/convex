/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman/Greg Herlein             */
/*                                   2016                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexserial.c                                                  */
/*    Author:     Greg Herlein                                                 */
/*    Created:    21 April 2016                                                */
/*                                                                             */
/*    Revisions:                                                               */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    This file is part of ConVEX.                                             */
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
/** @file    vexserial.h
  * @brief   Simple Command line interface for customer serial protocols
*//*---------------------------------------------------------------------------*/

#ifndef _VEXSERIAL_
#define _VEXSERIAL_

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
} SerialCommand;

/*-----------------------------------------------------------------------------*/
/** @brief   SerialProto descriptor type.                                            */
/*-----------------------------------------------------------------------------*/
typedef struct {
  vexStream             *sc_channel;        /**< @brief I/O channel associated
                                                 to the shell.              */
  const SerialCommand   *sc_commands;       /**< @brief Shell extra commands
                                                 table.                     */
  const char            *delim;             /**< @brief  serial field delim */
} SerProtoConfig;


#if !defined(__DOXYGEN__)
extern EventSource shell_terminated;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void        serialInit(void);
Thread     *serialCreate(const SerProtoConfig *scp, size_t size, tprio_t prio);
Thread     *serialCreateStatic(const SerProtoConfig *scp, void *wsp, size_t size, tprio_t prio);
bool_t      shellGetLine(vexStream *chp, char *line, unsigned size);

#ifdef __cplusplus
}
#endif

#endif /* _VEXSERIAL_ */

