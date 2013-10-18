/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     robotc_glue.c                                                */
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

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

#include "robotc_glue.h"

/*-----------------------------------------------------------------------------*/
/** @file    robotc_glue.c
  * @brief   Some glue functions to make porting ROBOTC coe easier
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief   Maximum number of ROBOTC style tasks
 */
#define RC_TASKS        10
/*-----------------------------------------------------------------------------*/
/** @brief   Stack size for ROBOTC
 *  each thread also uses 176 bytes (0xB0) for thread overhead and 8 bytes for
 *  heap header
 */
#define TC_THREAD_STACK 584

/*-----------------------------------------------------------------------------*/
/** @brief      Structure that correlates a function name with a thread
 */
typedef struct _rcTask {
    tfunc_t     pf;
    Thread      *tp;
} rcTask;

static  rcTask  rcTasks[RC_TASKS];

/*-----------------------------------------------------------------------------*/
/*  All robotc threads now start here, this then calls the user supplied       */
/*-----------------------------------------------------------------------------*/
static msg_t
vexRobotcTask(void *arg)
{
    // All robotc tasks enter here and then call the user provided function
    if(arg != NULL)
        {
        // register the task
        tfunc_t  func = arg;
        vexTaskRegister( "robotc_task" );

        // call user supplied function, this may not return
        return func(NULL);
        }
    else
        return (msg_t)0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start a task with a given priority                             */
/** @param[in]  pf The thread function                                         */
/** @param[in]  priority The thread priority                                   */
/** @returns    A pointer to the thread                                        */
/*-----------------------------------------------------------------------------*/
Thread *
StartTaskWithPriority(tfunc_t pf, tprio_t priority, ... )
{
    static  bool_t  init = TRUE;
    int16_t     i;
    bool_t      memfree = FALSE;

    // First time initialization
    if(init)
        {
        for(i=0;i<RC_TASKS;i++)
            {
            rcTasks[i].pf = NULL;
            rcTasks[i].tp = NULL;
            }
        // not next time
        init = FALSE;
        }

    // Check to see of this function is already in the robotc task list
    for(i=0;i<RC_TASKS;i++)
        {
        if( rcTasks[i].pf == pf )
            return(NULL);
        }

    // check to see of we have exhausted the task array
    for(i=0;i<RC_TASKS;i++)
            {
            if( rcTasks[i].pf == NULL )
                {
                memfree = TRUE;
                break;
                }
            }
    if( !memfree )
        return(NULL);

    Thread *tp;
    tp = chThdCreateFromHeap(NULL, THD_WA_SIZE(TC_THREAD_STACK), priority, vexRobotcTask, (void *)pf );

    // tp will be NULL if memory is exhausted
    if( tp != NULL )
        {
        // Save association of thread and callback
        for(i=0;i<RC_TASKS;i++)
            {
            if( rcTasks[i].tp == NULL )
                {
                rcTasks[i].tp = tp;
                rcTasks[i].pf = pf;
                break;
                }
            }
        }

    return(tp);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Stop a task                                                    */
/** @param[in]  pf The thread function                                         */
/*-----------------------------------------------------------------------------*/
void
StopTask(tfunc_t pf)
{
    int16_t     i;
    Thread     *tp;

    for(i=0;i<RC_TASKS;i++)
        {
        if( rcTasks[i].pf == pf )
            {
            if( rcTasks[i].tp != NULL )
                {
                tp = rcTasks[i].tp;

                // We cannot stop ourself
                if( tp == chThdSelf() )
                    return;

                // Set terminate flag - the other thread has to see this and exit
                chThdTerminate( tp );
                // this will cause a higher priority task to run immeadiately
                chThdResume( tp );
                // wait for termination
                chThdWait( tp );
                // may already be NULL for high priority task
                rcTasks[i].tp = NULL;
                rcTasks[i].pf = NULL;
                }
            }
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      cleanup a task if it was terminated elsewhere                  */
/** @param[in]  tp A pointer to a thread                                       */
/** @note this will not usually be called by the user by by an internal thread */
/*-----------------------------------------------------------------------------*/

void
CleanupTask( Thread *tp )
{
    int16_t     i;

    for(i=0;i<RC_TASKS;i++)
        {
        if( rcTasks[i].tp == tp )
            {
            rcTasks[i].tp = NULL;
            rcTasks[i].pf = NULL;
            }
        }
}
