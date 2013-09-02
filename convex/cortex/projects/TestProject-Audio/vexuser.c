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
#include "smartmotor.h"

// Digi IO configuration
static	vexDigiCfg	dConfig[kVexDigital_Num] = {
		{ kVexDigital_1,	kVexSensorDigitalOutput, kVexConfigOutput,		0 },
		{ kVexDigital_2,	kVexSensorDigitalOutput, kVexConfigOutput,		0 },
		{ kVexDigital_3,	kVexSensorSonarCm,       kVexConfigSonarOut,    kVexSonar_1 },
		{ kVexDigital_4,	kVexSensorSonarCm,       kVexConfigSonarIn,		kVexSonar_1 },
		{ kVexDigital_5,	kVexSensorDigitalInput,  kVexConfigInput,		0 },
		{ kVexDigital_6,	kVexSensorDigitalInput,  kVexConfigInput,		0 },
		{ kVexDigital_7,	kVexSensorDigitalInput,  kVexConfigInput,		0 },
		{ kVexDigital_8,	kVexSensorQuadEncoder,   kVexConfigQuadEnc1,	kVexQuadEncoder_2 },
		{ kVexDigital_9,	kVexSensorQuadEncoder,   kVexConfigQuadEnc2,	kVexQuadEncoder_2 },
		{ kVexDigital_10,	kVexSensorDigitalInput,  kVexConfigInput,		0 },
		{ kVexDigital_11,	kVexSensorQuadEncoder,   kVexConfigQuadEnc1,    kVexQuadEncoder_1 },
		{ kVexDigital_12,	kVexSensorQuadEncoder,   kVexConfigQuadEnc2,    kVexQuadEncoder_1 }
};

static  vexMotorCfg mConfig[10] = {
        { kVexMotor_1,      kVexMotor393T,           kVexMotorNormal,       kVexSensorIME,         kImeChannel_1 },
        { kVexMotor_2,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_3,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_4,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_5,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_6,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_7,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_8,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_9,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_10,     kVexMotor393T,           kVexMotorNormal,       kVexSensorIME,         kImeChannel_2 }
};


#define	MotorDriveL		kVexMotor_1
#define	MotorDriveR		kVexMotor_10

// A Selection of RTTTL tunes
#define NUM_TUNES   7

char tune1[] = "Mission Impossible:d=4,o=6,b=100:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,16g,8p,16g,8p,16a#,16p,16c,16p,16g,8p,16g,8p,16f,16p,16f#,16p,16g,8p,16g,8p,16a#,16p,16c,16p,16g,8p,16g,8p,16f,16p,16f#,16p,16a#,16g,2d,32p,16a#,16g,2c#,32p,16a#,16g,2c,16p,16a#5,16c";
char tune2[] = "Pacman:d=4,o=5,b=112:32b,32p,32b6,32p,32f#6,32p,32d#6,32p,32b6,32f#6,16p,16d#6,16p,32c6,32p,32c7,32p,32g6,32p,32e6,32p,32c7,32g6,16p,16e6,16p,32b,32p,32b6,32p,32f#6,32p,32d#6,32p,32b6,32f#6,16p,16d#6,16p,32d#6,32e6,32f6,32p,32f6,32f#6,32g6,32p,32g6,32g#6,32a6,32p,32b.6";
char tune3[] = "Super Mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
char tune4[] = "Beethoven:d=4,o=6,b=125:8e,8d,8e,8d,8e,8b5,8d,8c,a5,8p,8c5,8e5,8a5,b5,8p,8e5,8g5,8b5,c,8p,8e5,8e,8d,8e,8d,8e,8b5,8d,8c,a5,8p,8c5,8e5,8a5,b5,8p,8e5,8c,8b5,a5";
char tune5[] = "AxelF:d=4,o=5,b=125:32p,8g,8p,16a#.,8p,16g,16p,16g,8c6, 8g,8f,8g,8p,16d.6,8p,16g,16p,16g,8d#6,8d6,8a#,8g,8d6,8g6, 16g,16f,16p,16f,8d,8a#,2g,p,16f6,8d6,8c6,8a#,g,8a#.,16g, 16p,16g,8c6,8g,8f,g,8d.6,16g,16p,16g,8d#6,8d6,8a#,8g,8d6, 8g6,16g,16f,16p,16f,8d,8a#,2g";
char tune6[] = "National Anthem:d=8,o=5,b=120:e.,d,4c,4e,4g,4c6.,p,e6.,d6,4c6,4e,4f#,4g.,p,4g,4e6.,d6,4c6,2b,a,4b,c6.,16p,4c6,4g,4e,32p,4c";
char tune7[] = "Thunder Birds:d=8,o=4,b=125:g#5,16f5,16g#5,4a#5,p,16d#5,16f5,g#5,a#5,d#6,16f6,16c6,d#6,f6,2a#5,g#5,16f5,16g#5,4a#5,p,16d#5,16f5,g#5,a#5,d#6,16f6,16c6,d#6,f6,2g6,g6,16a6,16e6,4g6,p,16e6,16d6,c6,b5,a.5,16b5,c6,e6,2d6,d#6,16f6,16c6,4d#6,p,16c6,16a#5,g#5,g5,f.5,16g5,g#5,a#5,c6,a#5,g5,d#5";
char *MyTunes[NUM_TUNES] = {tune1,tune2,tune3,tune4,tune5,tune6,tune7};

// sound demo

void
RtttlDemo( int tune )
{
    char    *n;

    if( tune < NUM_TUNES )
        {
        n = vexAudioPlayRtttl( MyTunes[tune], 128, 1 );
        vexLcdPrintf( 1, 1, "%s", n );
        }
}

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
    SmartMotorsInit();
    SmartMotorCurrentMonitorEnable();
    SmartMotorRun();
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


// arcade drive
void
DriveSystemArcadeDrive( int16_t forward, int16_t turn )
{
    int16_t drive_l_motor;
    int16_t drive_r_motor;

    // Set drive
    drive_l_motor = forward + turn;
    drive_r_motor = forward - turn;

    // normalize drive so max is 127 if any drive is over 127
    int16_t max = abs(drive_l_motor);
    if (abs(drive_r_motor)  > max)
        max = abs(drive_r_motor);
    if (max>127) {
        drive_l_motor = 127 * drive_l_motor / max;
        drive_r_motor = 127 * drive_r_motor  / max;
    }

    // Send to motors
    // left drive
    SetMotor( MotorDriveL, drive_l_motor );

    // right drive
    SetMotor( MotorDriveR, drive_r_motor );
}

// Driver control task
msg_t
vexOperator( void *arg )
{
	int16_t		forward;
	int16_t		turn;
	int16_t     nexttune = 0;

	(void)arg;

	// Must call this
	vexTaskRegister("operator");

    RtttlDemo(nexttune);

	// Run until asked to terminate
	while(!chThdShouldTerminate())
		{
	    if( vexControllerGet( Btn7 ) )
	        {
	        if( vexControllerGet( Btn7R ) )
	            {
	            if(++nexttune == NUM_TUNES)
	                nexttune = 0;
	            }
	        if( vexControllerGet( Btn7L ) )
	            {
	            if(--nexttune < 0)
	                nexttune = NUM_TUNES-1;
	            }

	        RtttlDemo(nexttune);

	        // lazy
	        while(vexControllerGet( Btn7 ))
	            vexSleep(25);
	        }

		// status on LCD of encoder and sonar
		vexLcdPrintf( VEX_LCD_DISPLAY_2, VEX_LCD_LINE_1, "Batt %4.2fV", vexSpiGetMainBattery() / 1000.0 );

		// Get controller
		if( abs( vexControllerGet( Ch2 )) > 10 )
			forward = vexControllerGet( Ch2 );
		else
			forward = 0;

		if( abs( vexControllerGet( Ch1 )) > 10 )
			turn = vexControllerGet( Ch1 );
		else
			turn = 0;

        if( abs( vexControllerGet( Ch3 )) > 10 )
            SetMotor( 1, vexControllerGet( Ch3 ) );
        else
            SetMotor( 1, 0 );

		// arcade drive
		DriveSystemArcadeDrive( forward, turn );

		// Don't hog cpu
		vexSleep( 25 );
		}

	return (msg_t)0;
}



