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
/** @file    vexserial.c
  * @brief   Simple Command line interface for customer serial protocols
*//*---------------------------------------------------------------------------*/

#include <string.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

// local shell flags
static  bool_t  shell_echo = true;
static  bool_t  shell_bufr = true;

#define CR                    '\r'
#define LF                    '\n'
#define CTRL_C                0x03
#define CTRL_D                0x04
#define CTRL_Q                0x11
#define BKSPACE_CHAR          '\b'
#define ABORT_CHAR            CTRL_C
#define QUIT_APPEND           CTRL_Q
#define HISTORY_CHAR          '!'
#define ESC                   0x1B


bool_t serialGetLine( vexStream *chp, char *line, unsigned size );

/*-----------------------------------------------------------------------------*/

static msg_t
serial_thread(void *p)
{
  msg_t msg = RDY_OK;
  vexStream *chp = ((ShellConfig *)p)->sc_channel;
  char* logoutString = ((SerProtoConfig *)p)->logoutString;
  char line[((SerProtoConfig *)p)->max_len];
  lineHandler_cb cb = ((SerProtoConfig *)p)->cb;

  chRegSetThreadName("vexserial");

  while (TRUE)
  {
//    vex_chprintf(chp, lineStart);
    if (serialGetLine(chp, line, sizeof(line)))
    {
      vex_chprintf(chp, logoutString);
      break;
    }

//    vex_chprintf(chp, "[%s]\r\n",line);
    if (cb) cb(chp, line);
  }

// Atomically broadcasting the event source and terminating the thread,
// there is not a chSysUnlock() because the thread terminates upon return.
  chSysLock();
  chEvtBroadcastI(&shell_terminated);
  chThdExitS(msg);
  return 0; // Never executed.
}

/*-----------------------------------------------------------------------------*/
/** @brief  Serial manager initialization.                                      */
/*-----------------------------------------------------------------------------*/

void serialInit(void)
{
  chEvtInit(&shell_terminated);
}

/*-----------------------------------------------------------------------------*/
/**  @brief  Spawns a new shell using dynamic memory allocation                */
/**  @param[in] scp  pointer to a @p ShellConfig object                        */
/**  @param[in] size size of the shell working area to be allocated            */
/**  @param[in] prio priority level for the new shell                          */
/**  @returns   A pointer to the shell thread.                                 */
/**  @retval NULL  thread creation failed because memory allocation.           */
/**  @note CH_USE_MALLOC_HEAP and @p CH_USE_DYNAMIC must be enabled.           */
/*-----------------------------------------------------------------------------*/

#if (CH_USE_HEAP && CH_USE_DYNAMIC) || defined(__DOXYGEN__)
Thread *serialCreate(const SerProtoConfig *scp, size_t size, tprio_t prio)
{
  return chThdCreateFromHeap(NULL, size, prio, serial_thread, (void *)scp);
}
#endif

/*-----------------------------------------------------------------------------*/
/**  @brief  Create statically allocated shell thread.                         */
/**  @param[in] scp  pointer to a @p ShellConfig object                        */
/**  @param[in] wsp  pointer to a working area dedicated to the shell thread   */
/**                  stack                                                     */
/**  @param[in] size size of the shell working area to be allocated            */
/**  @param[in] prio priority level for the new shell                          */
/**  @returns   A pointer to the shell thread.                                 */
/*-----------------------------------------------------------------------------*/

Thread *serialCreateStatic(const SerProtoConfig *scp, void *wsp, size_t size, tprio_t prio)
{
  return chThdCreateStatic(wsp, size, prio, serial_thread, (void *)scp);
}

/*-----------------------------------------------------------------------------*/
/** @private                                                                   */
/** @brief Gets a single line of input                                         */
/** @param[in]  chp     A pointer to a vexStream object                        */
/** @param[in]  line    A pointer the inpout buffer                            */
/** @param[in]  size    size of the inpout buffer                              */
/** @returns    True if shell is to be terminated else false                   */
/*-----------------------------------------------------------------------------*/

bool_t serialGetLine( vexStream *chp, char *line, unsigned size )
{
  static  char    history[SHELL_MAX_LINE_LENGTH] = {""};

  short escSeq = 0;
  char *p = line;
  char *q;
  char *r = line;
  systime_t   lasttime = chTimeNow();


  // not in escape sequence
  escSeq = 0;

  while (TRUE)
  {
#if (CH_KERNEL_MAJOR >= 2) && (CH_KERNEL_MINOR >= 6)
    char  c;
    if (chSequentialStreamRead(chp, (uint8_t *)&c, 1) == 0)
      return TRUE;
#else
    short c;
    if ( (c = (short)chIOGet(chp)) < 0 )
      return TRUE;
#endif

    // In escape sequence part 1 ?
    if ( escSeq == 1 )
    {
      if ( c == '[' )
      {
        escSeq = 2;
        continue;
      }
      else
        escSeq = 0;
    }

    // In escape sequence part 2 ?
    if ( escSeq == 2 )
    {
      switch (c)
      {
      case 'A':   // Cursor Up
        if ( shell_echo )
          vex_chprintf(chp, "%s", history);

        // copy history buffer to line
        q = history;
        while ((p < line + size - 1) && (*q != 0))
          *p++ = *q++;
        // assume no buffering for history
        r = p;
        break;

      default:
        break;
      }
      escSeq = 0;
      continue;
    }


    switch (c)
    {
    // CTRL D quits shell
    case    CTRL_D:
      vex_chprintf(chp, "^D");
      return TRUE;
      break;

    // Backspace
    case    BKSPACE_CHAR:
    case    0x7F:       // for some reason OSX screen outputs 7F
      if (p != line)
      {
        if ( shell_echo ) {
          // print any buffered characters
          while ( r != p )
            vex_chprintf(chp, "%c", (uint8_t)*r++);
          // print backspace and wipe the previous character
          vex_chprintf(chp, "%c %c", (uint8_t)BKSPACE_CHAR, (uint8_t)BKSPACE_CHAR );
        }

        // one less character
        p--;
        // no more buffering
        r = p;
      }
      break;

    case    CR:
      *p = 0;

      if (shell_echo) {
        // print any remaining chars that came in quickly
        if ( r != p )
          vex_chprintf(chp, "%s", r);

        // copy to history buffer
        for ( q = history, p = line; *p != 0; q++, p++)
          *q = *p;
        *q = 0;
      }

      // send cr/lf
      vex_chprintf(chp, "\r\n");
      return FALSE;
      break;

    case    ESC:
      // escape sequence ?
      escSeq = 1;
      break;

    case    0xC9:
      // start of VEX escape sequence
      // sleep - do not echo
      chThdSleepMilliseconds(500);
      break;

    case    0x21:
      // ROBOTC uses 0x21 command to start user code
      // and does this before downloading its firmware
      // sleep - do not echo
      chThdSleepMilliseconds(100);
      break;

    default:
      if (c >= 0x20) {
        if (p < line + size - 1) {
          *p++ = (char)c;

          if ( shell_echo ) {
            if ( !shell_bufr || (chTimeNow() - lasttime) > 50 ) {
              while ( r != p )
                vex_chprintf(chp, "%c", (uint8_t)*r++);
            }

            lasttime = chTimeNow();
          }
        }
      }
      break;
    }
  }
}




