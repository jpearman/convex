/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexuser.c                                                    */
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
/*    Licensed under the Apache License, Version 2.0 (the "License");          */
/*    you may not use this file except in compliance with the License.         */
/*    You may obtain a copy of the License at                                  */
/*                                                                             */
/*      http://www.apache.org/licenses/LICENSE-2.0                             */
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

#include <stdlib.h>

#include "ch.h"  		// needs for all ChibiOS programs
#include "hal.h" 		// hardware abstraction layer header
#include "vex.h"		// vex library header

// Digi IO configuration
static  vexDigiCfg  dConfig[kVexDigital_Num] = {
        { kVexDigital_1,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_2,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_3,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_4,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_5,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_6,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_7,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_8,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_9,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_10,   kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_11,   kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_12,   kVexSensorDigitalOutput, kVexConfigOutput,      0 }
};

// Initialize the digital ports
void
vexUserSetup()
{
    vexDigitalConfigure( dConfig, DIG_CONFIG_SIZE( dConfig ) );
}

// called before either autonomous or user control
void
vexUserInit()
{
}

// Autonomous control task
msg_t
vexAutonomous( void *arg )
{
    (void)arg;

    // Must call this
    vexTaskRegister("auton");

    while(1)
        {
        // Don't hog cpu
        vexSleep( 25 );
        }

    return (msg_t)0;
}

// Various LED patterns
// Each bit corresponds to one LED
// 1 = on, 0 = off
//
short pattern_0[] = {
    0b000000000001,
    0b000000000010,
    0b000000000100,
    0b000000001000,
    0b000000010000,
    0b000000100000,
    0b000001000000,
    0b000010000000,
    0b000100000000,
    0b001000000000,
    0b010000000000,
    0b100000000000,
    0b010000000000,
    0b001000000000,
    0b000100000000,
    0b000100000000,
    0b000010000000,
    0b000001000000,
    0b000000100000,
    0b000000010000,
    0b000000001000,
    0b000000000100,
    0b000000000010
    };

/*-----------------------------------------------------------------------------*/
/*  Output one word to the 12 LEDs in the digital ports                        */
/*-----------------------------------------------------------------------------*/

void
display(short output)
{
    tVexDigitalPin     i;

    for(i=kVexDigital_1;i<kVexDigital_Num;i++)
        {
        vexDigitalPinSet( i, (output & 0x0001) == 1 ? 0 : 1 );
        output >>= 1;
        }
}

/*-----------------------------------------------------------------------------*/
/*  Playback a pattern sequence                                                */
/*-----------------------------------------------------------------------------*/

void
RunPattern( short *p, short len, short delayMs )
{
    int i = 0;

    for(i=0;i<len;i++)
        {
        display( *p++ );
        vexSleep(delayMs);
        }
}

// Driver control task
msg_t
vexOperator( void *arg )
{
	(void)arg;
	int     delay, i;

	// Must call this
	vexTaskRegister("operator");

    while(1)
        {
        // run a pattern with speed 50
        for(delay=100;delay>10;delay-=10)
            RunPattern( &pattern_0[0], sizeof(pattern_0)/sizeof(short), delay );

        for(i=0;i<20;i++)
            RunPattern( &pattern_0[0], sizeof(pattern_0)/sizeof(short), 10 );

        // not really needed but will leave here
        vexSleep(10);
        }

	return (msg_t)0;
}



