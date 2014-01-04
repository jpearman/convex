/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vextest.c                                                    */
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
/*  A place for test code before integration into it's pown modules            */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#include <stdlib.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vextest.c
  * @brief   Placeholder for test code
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/*  Thread used for testing                                                    */
/*  This should probably be avoided now, use vexuser functions instead         */
/*-----------------------------------------------------------------------------*/

static WORKING_AREA(waVexTestThread, TEST_TASK_STACK_SIZE);
static msg_t
vexTestThread( void *arg )
{
    (void)arg;

    chRegSetThreadName("test");

    while (TRUE)
        {
        chThdSleepMilliseconds(25);
        }

    return (msg_t)0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Used for testing new ideas                                     */
/*-----------------------------------------------------------------------------*/

void
vexTest()
{
    // Creates the test thread.
    chThdCreateStatic(waVexTestThread, sizeof(waVexTestThread), NORMALPRIO-1, vexTestThread, NULL);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Dump test data for debug                                       */
/** @param[in]  chp     A pointer to a vexStream object                      */
/** @param[in]  argc    The number of command line arguments                   */
/** @param[in]  argv    An array of pointers to the command line args          */
/*-----------------------------------------------------------------------------*/

void
vexTestDebug(vexStream *chp, int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    (void)chp;
}



