/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexmain.c                                                    */
/*    Author:     James Pearman                                                */
/*    Created:    7 May 2013                                                   */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00  04 July 2013 - Initial release                        */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    The author is supplying this software for use with the VEX cortex        */
/*    control system. This file can be freely distributed and teams are        */
/*    authorized to freely use this program , however, it is requested that    */
/*    improvements or additions be shared with the Vex community via the vex   */
/*    forum.  Please acknowledge the work of the authors when appropriate.     */
/*    Thanks.                                                                  */
/*                                                                             */
/*    THIS SOFTWARE IS PROVIDED "AS IS". NO WARRANTIES, WHETHER EXPRESS,       */
/*    IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES  */
/*    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS    */
/*    SOFTWARE.  THE AUTHORS SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR    */
/*    SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.*/
/*                                                                             */
/*    The author can be contacted on the vex forums as jpearman                */
/*    or electronic mail using jbpearman_at_mac_dot_com                        */
/*    Mentor for team 8888 RoboLancers, Pasadena CA.                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/


#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "vex.h"

#include "smartmotor.h"
#include "apollo.h"

/*-----------------------------------------------------------------------------*/
/* Command line related.                                                       */
/*-----------------------------------------------------------------------------*/

static void
cmd_apollo( vexStream *chp, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    apolloInit();

    // run until any key press
    //while( chIOGetWouldBlock(chp) )
    while( sdGetWouldBlock((SerialDriver *)chp) )
        {
        apolloUpdate();

        //chThdSleepMilliseconds(50);
        }

    apolloDeinit();
}


static void
cmd_sm(vexStream *chp, int argc, char *argv[])
{
    (void)argv;
    (void)chp;
    (void)argc;

    SmartMotorDebugStatus();
}

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)

// Shell command
static const ShellCommand commands[] = {
  {"adc",     vexAdcDebug },
  {"spi",     vexSpiDebug },
  {"motor",   vexMotorDebug},
  {"lcd",     vexLcdDebug},
  {"enc",     vexEncoderDebug},
  {"son",     vexSonarDebug},
  {"ime",     vexIMEDebug},
  {"test",    vexTestDebug},
  {"sm",      cmd_sm },
  {"apollo",  cmd_apollo},
   {NULL, NULL}
};

// configuration for the shell
static const ShellConfig shell_cfg1 = {
  (vexStream *)SD_CONSOLE,
   commands
};

/*-----------------------------------------------------------------------------*/
//  Application entry point.											       */
/*-----------------------------------------------------------------------------*/

int main(void)
{
	Thread *shelltp = NULL;
	short	timeout = 0;

	// System initializations.
    // - HAL initialization, this also initializes the configured device drivers
    //   and performs the board-specific initializations.
    // - Kernel initialization, the main() function becomes a thread and the
    //   RTOS is active.
	halInit();
	chSysInit();

	// Init the serial port associated with the console
	vexConsoleInit();

    // use digital 10 as safety
    //if( palReadPad( VEX_DIGIO_10_PORT, VEX_DIGIO_10_PIN) == 1)
    // init VEX
    vexCortexInit();

    // wait for good spi comms
    while( vexSpiGetOnlineStatus() == 0 )
    	{
        // wait for a while
        chThdSleepMilliseconds(100);
        // dump after 5 seconds
        if(timeout++ == 50)
        	break;
    	}

    // Shell manager initialization.
    shellInit();

    // spin in loop monitoring the shell
    while (TRUE)
    	{
	    if (!shelltp)
	    	shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
	    else
	    if (chThdTerminated(shelltp))
	    	{
	        chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
	        shelltp = NULL;           /* Triggers spawning of a new shell.        */
	        }

	    chThdSleepMilliseconds(50);
    	}
}
