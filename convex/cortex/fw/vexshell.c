/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexshell.c                                                   */
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
/** @file    vexshell.c
  * @brief   Simple Command line interface shell
*//*---------------------------------------------------------------------------*/

#include <string.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

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

#define xstr(a) str(a)
#define str(a) #a

// Shell termination event source.
EventSource shell_terminated;

// local shell flags
static  bool_t  shell_echo = true;
static  bool_t  shell_bufr = true;

// String to token
static char *_strtok(char *str, const char *delim, char **saveptr)
{
    char *token;
    if (str)
        *saveptr = str;
    token = *saveptr;

    if (!token)
        return NULL;

    token += strspn(token, delim);
    *saveptr = strpbrk(token, delim);
    if (*saveptr)
        *(*saveptr)++ = '\0';

    return( *token ? token : NULL );
}

/*-----------------------------------------------------------------------------*/

static void
usage(vexStream *chp, char *p)
{
    vex_chprintf(chp, "Usage: %s\r\n", p);
}

/*-----------------------------------------------------------------------------*/

static void
list_commands(vexStream *chp, const ShellCommand *scp)
{
    while (scp->sc_name != NULL)
        {
        vex_chprintf(chp, "%s ", scp->sc_name);
        scp++;
        }
}

/*-----------------------------------------------------------------------------*/
/*  print a bunch of useful info about the platform                            */
/*-----------------------------------------------------------------------------*/

static void
cmd_info(vexStream *chp, int argc, char *argv[])
{
    (void)argv;
    if (argc > 0) {
        usage(chp, "info");
        return;
    }

    vex_chprintf(chp, "Kernel:       %s\r\n", CH_KERNEL_VERSION);
#ifdef CH_COMPILER_NAME
    vex_chprintf(chp, "Compiler:     %s\r\n", CH_COMPILER_NAME);
#endif
    vex_chprintf(chp, "Architecture: %s\r\n", CH_ARCHITECTURE_NAME);
#ifdef CH_CORE_VARIANT_NAME
    vex_chprintf(chp, "Core Variant: %s\r\n", CH_CORE_VARIANT_NAME);
#endif
#ifdef CH_PORT_INFO
    vex_chprintf(chp, "Port Info:    %s\r\n", CH_PORT_INFO);
#endif
#ifdef PLATFORM_NAME
    vex_chprintf(chp, "Platform:     %s\r\n", PLATFORM_NAME);
#endif
#ifdef BOARD_NAME
    vex_chprintf(chp, "Board:        %s\r\n", BOARD_NAME);
#endif
#ifdef CONVEX_VERSION
    vex_chprintf(chp, "ConVEX:       %s\r\n", CONVEX_VERSION);
#endif
#ifdef __DATE__
#ifdef __TIME__
    vex_chprintf(chp, "Build time:   %s%s%s\r\n", __DATE__, " - ", __TIME__);
#endif
#endif
#ifdef PROJECT
    vex_chprintf(chp, "Project:      %s\r\n",xstr(PROJECT));
#endif
#ifdef GIT_SHORT_HASH
    vex_chprintf(chp, "Git Hash:     %s\r\n",xstr(GIT_SHORT_HASH));
#endif
}

/*-----------------------------------------------------------------------------*/

static void
cmd_systime(vexStream *chp, int argc, char *argv[])
{
    (void)argv;
    if (argc > 0) {
        usage(chp, "systime");
        return;
    }
    vex_chprintf(chp, "%lu\r\n", (unsigned long)chTimeNow());
}

/*-----------------------------------------------------------------------------*/
// external symbols used to calculate flash usage
#define SYMVAL(sym) (uint32_t)(((uint8_t *)&(sym)) - ((uint8_t *)0))

extern uint32_t _textdata;
extern uint32_t _data;
extern uint32_t _edata;

static void
cmd_mem(vexStream *chp, int argc, char *argv[])
{
    size_t n, size;
    uint32_t flash;

    (void)argv;
    if (argc > 0)
        {
        vex_chprintf(chp, "Usage: mem\r\n");
        return;
        }

    n = chHeapStatus(NULL, &size);
    vex_chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
    vex_chprintf(chp, "heap fragments   : %u\r\n", n);
    vex_chprintf(chp, "heap free total  : %u bytes\r\n", size);

    flash = (SYMVAL(_textdata) - 0x08000000) + (SYMVAL(_edata) - SYMVAL(_data));
    vex_chprintf(chp, "flash used       : %lu (0x%05X) bytes\r\n", flash, flash);
}

/*-----------------------------------------------------------------------------*/

static void
cmd_threads(vexStream *chp, int argc, char *argv[])
{
    static const char *states[] = {THD_STATE_NAMES};
    Thread *tp;

    (void)argv;
    if (argc > 0)
        {
        vex_chprintf(chp, "Usage: threads\r\n");
        return;
        }

    vex_chprintf(chp, "            name addr     stack    prio refs     state time\r\n");
    tp = chRegFirstThread();

    do
        {
        if( tp->p_name != NULL )
            vex_chprintf(chp, "%16s ", tp->p_name);
        else
            vex_chprintf(chp,"                 ");

        vex_chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
                (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
                (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
                states[tp->p_state], (uint32_t)tp->p_time);

        tp = chRegNextThread(tp);
        chThdSleepMilliseconds(20);
        } while (tp != NULL);
}
/*-----------------------------------------------------------------------------*/

static void
cmd_reboot(vexStream *chp, int argc, char *argv[])
{
    (void)argv;
    if (argc > 0) {
        vex_chprintf(chp, "Usage: reboot\r\n");
        return;
    }
    vex_chprintf(chp, "rebooting...\r\n");
    chThdSleepMilliseconds(100);
    NVIC_SystemReset();
}

/*-----------------------------------------------------------------------------*/
static void
cmd_flags(vexStream *chp, int argc, char *argv[])
{
    if (argc < 2) {
        vex_chprintf(chp, "Usage:flags [flag] [on|off]\r\n");
        vex_chprintf(chp, "      example\r\n");
        vex_chprintf(chp, "      flags echo on\r\n");
        return;
    }

    if(strcasecmp(argv[0],"echo")==0) {
        if(strcasecmp(argv[1],"on")==0) {
            shell_echo = true;
            vex_chprintf(chp, "echo is on\r\n");
        }
        if(strcasecmp(argv[1],"off")==0) {
            shell_echo = false;
            vex_chprintf(chp, "echo is off\r\n");
        }
    }
    if(strcasecmp(argv[0],"bufr")==0) {
        if(strcasecmp(argv[1],"on")==0) {
            shell_bufr = true;
            vex_chprintf(chp, "buffering is on\r\n");
        }
        if(strcasecmp(argv[1],"off")==0) {
            shell_bufr = false;
            vex_chprintf(chp, "buffering is off\r\n");
        }
    }
}

/*-----------------------------------------------------------------------------*/

//Array of the default commands.
static ShellCommand local_commands[] = {
  {"info",    cmd_info},
  {"systime", cmd_systime},
  {"mem",     cmd_mem},
  {"threads", cmd_threads},
  {"reboot",  cmd_reboot},
  {"flags",   cmd_flags},
  {NULL, NULL}
};

/*-----------------------------------------------------------------------------*/
/*  Execute command                                                            */
/*-----------------------------------------------------------------------------*/

static bool_t
cmdexec(const ShellCommand *scp, vexStream *chp, char *name, int argc, char *argv[])
{
    while (scp->sc_name != NULL) {
        if (strcasecmp(scp->sc_name, name) == 0) {
            scp->sc_function(chp, argc, argv);
            return FALSE;
        }
        scp++;
    }

  return TRUE;
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*  Shell thread function.                                                     */
/*                                                                             */
/*  p - pointer to a vexStream object                                          */
/*  Termination reason.                                                        */
/*    terminated by command.                                                   */
/*    terminated by reset condition on the I/O channel.                        */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

static msg_t
shell_thread(void *p)
{
    int n;
    msg_t msg = RDY_OK;
    vexStream *chp = ((ShellConfig *)p)->sc_channel;
    const ShellCommand *scp = ((ShellConfig *)p)->sc_commands;
    char *lp, *cmd, *tokp, line[SHELL_MAX_LINE_LENGTH];
    char *args[SHELL_MAX_ARGUMENTS + 1];

    chRegSetThreadName("shell");
    vex_chprintf(chp, "\033[2J"); // clearscreen
    vex_chprintf(chp, "\r\nVEX-Cortex Shell V1.00\r\n");
    vex_chprintf(chp, "Kernel:%s  ConVEX:%s  ", CH_KERNEL_VERSION, CONVEX_VERSION);
#ifdef __DATE__
#ifdef __TIME__
    vex_chprintf(chp, "Build time:%s%s%s\r\n", __DATE__, " - ", __TIME__);
#endif
#endif

    while (TRUE)
        {
        vex_chprintf(chp, "ch> ");
        if (shellGetLine(chp, line, sizeof(line)))
            {
            vex_chprintf(chp, "\r\nlogout");
            break;
            }

        lp = _strtok(line, " \009", &tokp);
        cmd = lp;
        n = 0;

        // Break line up into arguments
        while ((lp = _strtok(NULL, " \009", &tokp)) != NULL)
            {
            if (n >= SHELL_MAX_ARGUMENTS)
                {
                vex_chprintf(chp, "too many arguments\r\n");
                cmd = NULL;
                break;
                }
            args[n++] = lp;
            }
        args[n] = NULL;

        if (cmd != NULL)
            {
            if (strcasecmp(cmd, "exit") == 0)
                {
                if (n > 0)
                    {
                    usage(chp, "exit");
                    continue;
                    }
                break;
                }
            else
            if (strcasecmp(cmd, "help") == 0)
                {
                if (n > 0)
                    {
                    usage(chp, "help");
                    continue;
                    }

                vex_chprintf(chp, "Commands: help exit ");
                list_commands(chp, local_commands);
                if (scp != NULL)
                    list_commands(chp, scp);
                vex_chprintf(chp, "\r\n");
                }
            else
            if (cmdexec(local_commands, chp, cmd, n, args) && ((scp == NULL) || cmdexec(scp, chp, cmd, n, args)))
                {
                vex_chprintf(chp, "%s", cmd);
                vex_chprintf(chp, " ?\r\n");
                }
            }
        }

    // Atomically broadcasting the event source and terminating the thread,
    // there is not a chSysUnlock() because the thread terminates upon return.
    chSysLock();
    chEvtBroadcastI(&shell_terminated);
    chThdExitS(msg);

    return 0; // Never executed.
}

/*-----------------------------------------------------------------------------*/
/** @brief  Shell manager initialization.                                      */
/*-----------------------------------------------------------------------------*/

void shellInit(void)
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
Thread *shellCreate(const ShellConfig *scp, size_t size, tprio_t prio)
{
    return chThdCreateFromHeap(NULL, size, prio, shell_thread, (void *)scp);
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

Thread *shellCreateStatic(const ShellConfig *scp, void *wsp, size_t size, tprio_t prio)
{
    return chThdCreateStatic(wsp, size, prio, shell_thread, (void *)scp);
}

/*-----------------------------------------------------------------------------*/
/** @private                                                                   */
/** @brief Gets a single line of input for the shell                           */
/** @param[in]  chp     A pointer to a vexStream object                        */
/** @param[in]  line    A pointer the inpout buffer                            */
/** @param[in]  size    size of the inpout buffer                              */
/** @returns    True if shell is to be terminated else false                   */
/*-----------------------------------------------------------------------------*/

bool_t shellGetLine( vexStream *chp, char *line, unsigned size )
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
        if( (c = (short)chIOGet(chp)) < 0 )
            return TRUE;
#endif

        // In escape sequence part 1 ?
        if( escSeq == 1 )
            {
            if( c == '[' )
                {
                escSeq = 2;
                continue;
                }
            else
                escSeq = 0;
            }

        // In escape sequence part 2 ?
        if( escSeq == 2 )
            {
            switch(c)
                {
                case 'A':   // Cursor Up
                    if( shell_echo )
                        vex_chprintf(chp, "%s", history);

                    // copy history buffer to line
                    q = history;
                    while((p < line + size - 1) && (*q != 0))
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


        switch(c)
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
                    if( shell_echo ) {
                        // print any buffered characters
                        while( r != p )
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

                if(shell_echo) {
                    // print any remaining chars that came in quickly
                    if( r != p )
                        vex_chprintf(chp, "%s", r);

                    // copy to history buffer
                    for( q=history, p=line;*p!=0;q++,p++)
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

                        if( shell_echo ) {
                            if( !shell_bufr || (chTimeNow() - lasttime) > 50 ) {
                                while( r != p )
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


