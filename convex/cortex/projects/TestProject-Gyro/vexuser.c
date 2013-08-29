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
#include "vexgyro.h"

// Digi IO configuration
static	vexDigiCfg	dConfig[kVexDigital_Num] = {
		{ kVexDigital_1,	kVexSensorDigitalOutput, kVexConfigOutput,		0 },
		{ kVexDigital_2,	kVexSensorDigitalOutput, kVexConfigOutput,		0 },
		{ kVexDigital_3,	kVexSensorDigitalInput,  kVexConfigInput,       0 },
		{ kVexDigital_4,	kVexSensorDigitalInput,  kVexConfigInput,		0 },
		{ kVexDigital_5,	kVexSensorDigitalInput,  kVexConfigInput,		0 },
		{ kVexDigital_6,	kVexSensorDigitalInput,  kVexConfigInput,		0 },
		{ kVexDigital_7,	kVexSensorDigitalInput,  kVexConfigInput,		0 },
		{ kVexDigital_8,	kVexSensorDigitalInput,  kVexConfigInput,	    0 },
		{ kVexDigital_9,	kVexSensorDigitalInput,  kVexConfigInput,	    0 },
		{ kVexDigital_10,	kVexSensorDigitalInput,  kVexConfigInput,		0 },
		{ kVexDigital_11,	kVexSensorDigitalInput,  kVexConfigInput,       0 },
		{ kVexDigital_12,	kVexSensorDigitalInput,  kVexConfigInput,       0 }
};

static  vexMotorCfg mConfig[kVexMotorNum] = {
        { kVexMotor_1,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_2,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_3,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_4,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_5,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_6,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_7,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_8,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_9,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_10,     kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 }
};


#define	MotorDriveL		kVexMotor_1
#define	MotorDriveR		kVexMotor_10

// Initialize the digital ports
void
vexUserSetup()
{
	vexDigitalConfigure( dConfig, DIG_CONFIG_SIZE( dConfig ) );
	vexMotorConfigure( mConfig, MOT_CONFIG_SIZE( mConfig ) );
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


// Driver control task
msg_t
vexOperator( void *arg )
{
	int16_t		blink = 0;

	(void)arg;

	// Must call this
	vexTaskRegister("operator");

	vexGyroInit();

	vexLcdClearLine( VEX_LCD_DISPLAY_1, VEX_LCD_LINE_1 );
	vexLcdClearLine( VEX_LCD_DISPLAY_1, VEX_LCD_LINE_2 );

	// Run until asked to terminate
	while(!chThdShouldTerminate())
		{
		// flash led/digi out
		vexDigitalPinSet( kVexDigital_2, blink);
		blink = 1 - blink;

		// status on LCD of encoder and sonar
		vexLcdPrintf( VEX_LCD_DISPLAY_1, VEX_LCD_LINE_1, "%4.2fV G %6.2f", vexSpiGetMainBattery() / 1000.0, vexGyroGet()/10.0 );

		// Don't hog cpu
		vexSleep( 25 );
		}

	return (msg_t)0;
}



