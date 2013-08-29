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
#include "robotc_glue.h"
#include "pidlib.h"
#include "osr.h"

// Digi IO configuration
static  vexDigiCfg  dConfig[kVexDigital_Num] = {
        { kVexDigital_1,    kVexSensorSonarCm,       kVexConfigSonarOut,    kVexSonar_1 },
        { kVexDigital_2,    kVexSensorSonarCm,       kVexConfigSonarIn,     kVexSonar_1 },
        { kVexDigital_3,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_4,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_5,    kVexSensorQuadEncoder,   kVexConfigQuadEnc1,    kVexQuadEncoder_1 },
        { kVexDigital_6,    kVexSensorQuadEncoder,   kVexConfigQuadEnc2,    kVexQuadEncoder_1 },
        { kVexDigital_7,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_8,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_9,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_10,   kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_11,   kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_12,   kVexSensorDigitalOutput, kVexConfigOutput,      0 }
};

static  vexMotorCfg mConfig[kVexMotorNum] = {
        { kVexMotor_1,      kVexMotor393T,           kVexMotorReversed,     kVexSensorNone,       0 },
        { kVexMotor_2,      kVexMotor393T,           kVexMotorNormal,       kVexSensorIME,        kImeChannel_1 },
        { kVexMotor_3,      kVexMotor393T,           kVexMotorNormal,       kVexSensorIME,        kImeChannel_2 },
        { kVexMotor_4,      kVexMotor393T,           kVexMotorReversed,     kVexSensorNone,       0 },
        { kVexMotor_5,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,       0 },
        { kVexMotor_6,      kVexMotorUndefined,      kVexMotorNormal,       kVexSensorNone,       0 },
        { kVexMotor_7,      kVexMotor393T,           kVexMotorNormal,       kVexSensorQuadEncoder,kVexQuadEncoder_1 },
        { kVexMotor_8,      kVexMotor393T,           kVexMotorReversed,     kVexSensorIME,        kImeChannel_3 },
        { kVexMotor_9,      kVexMotor393T,           kVexMotorReversed,     kVexSensorIME,        kImeChannel_4 },
        { kVexMotor_10,     kVexMotor393T,           kVexMotorNormal,       kVexSensorIME,        kImeChannel_5 }
};




pidController *arm_pid;
pidController *intake_pid;

/*-----------------------------------------------------------------------------*/
/*  estop task                                                                 */
/*  Hit the bumper switch to kill all motors and stop all tasks                */
/*-----------------------------------------------------------------------------*/

task eStopTask(void *arg)
{
    //int  i;

    (void) arg;

    // Must call this
    vexTaskRegister("estop task");

    while(!chThdShouldTerminate())
        {
        if( vexSensorValueGet( EStop ) == 0 )
            {
            // Need estop
            vexTaskEmergencyStop();
            }

        wait1Msec(10);
        }
    return (msg_t)0;
}


/*-----------------------------------------------------------------------------*/
/*  Drive control task                                                         */
/*                                                                             */
/*  Uses joystick Ch3, Ch4 and Btn8L, Btn8R                                    */
/*-----------------------------------------------------------------------------*/

void
DriveSystemMecanumDrive( )
{
    short   forward, turn, right;

    long drive_l_front;
    long drive_l_back;
    long drive_r_front;
    long drive_r_back;

    // Get controller
    if( abs( vexControllerGet( Ch3) ) > 10 )
        forward = vexControllerGet( Ch3 );
    else
        forward = 0;

    if( abs( vexControllerGet( Ch4 ) ) > 10 )
        right   = vexControllerGet( Ch4 );
    else
        right = 0;

    if( vexControllerGet( Btn8R ) == 1 )
        turn = 64;
    else
    if( vexControllerGet( Btn8L ) == 1 )
        turn = -64;
    else
        turn = 0;

    // Set drive
    drive_l_front = forward + turn + right;
    drive_l_back  = forward + turn - right;

    drive_r_front = forward - turn - right;
    drive_r_back  = forward - turn + right;

    // normalize drive so max is 127 if any drive is over 127
    int max = abs(drive_l_front);
    if (abs(drive_l_back)  > max)
        max = abs(drive_l_back);
    if (abs(drive_r_back)  > max)
        max = abs(drive_r_back);
    if (abs(drive_r_front) > max)
        max = abs(drive_r_front);
    if (max>127) {
        drive_l_front = 127 * drive_l_front / max;
        drive_l_back  = 127 * drive_l_back  / max;
        drive_r_back  = 127 * drive_r_back  / max;
        drive_r_front = 127 * drive_r_front / max;
    }

    // Send to motors
    // left drive
    SetMotor( MotorLF, drive_l_front);
    SetMotor( MotorLB, drive_l_back);

    // right drive
    SetMotor( MotorRF, drive_r_front);
    SetMotor( MotorRB, drive_r_back);
}

/*-----------------------------------------------------------------------------*/
/*  Drive control task                                                         */
/*-----------------------------------------------------------------------------*/

task
DriveTask(void *arg)
{
    (void) arg;

    // Must call this
    vexTaskRegister("drive task");

    while(!chThdShouldTerminate())
        {
        DriveSystemMecanumDrive();
        wait1Msec(25);
        }
    return (msg_t)0;
}

/*-----------------------------------------------------------------------------*/
/*  Check the hard limit switches and kill power if triggered                  */
/*-----------------------------------------------------------------------------*/

void
ArmSystemCheckHardLimits()
{
    static  int hardStopL = 0;
    static  int hardStopH = 0;

    // Check low limit switch
    if( vexSensorValueGet( ArmRLimitLow ) == 0 )
        {
        if( arm_pid->drive_cmd < 0 )
            {
            arm_pid->drive_cmd = 0;
            // hard stop
            if( !hardStopL )
                {
                hardStopL = 1;

                // hard stop
                SetMotor( MotorArmL, 20, TRUE);
                SetMotor( MotorArmR, 20, TRUE);
                wait1Msec(100);
                SetMotor( MotorArmL, 0, TRUE);
                SetMotor( MotorArmR, 0, TRUE);

                // zero encoder
                vexSensorValueSet( EncArmR,   0);
               }
            }
        }
    else
        hardStopL = 0;

    if( vexSensorValueGet( ArmRLimitHigh ) == 0 )
        {
        if( arm_pid->drive_cmd > 0 )
            {
            arm_pid->drive_cmd = 0;
            if( !hardStopH )
                {
                hardStopH = 1;

                // hard stop
                SetMotor( MotorArmL, -20, TRUE);
                SetMotor( MotorArmR, -20, TRUE);
                wait1Msec(100);
                SetMotor( MotorArmL, 0, TRUE);
                SetMotor( MotorArmR, 0, TRUE);
                }
            }
        }
    else
        hardStopH = 0;
}

/*-----------------------------------------------------------------------------*/
/*  PID control of the arm                                                     */
/*-----------------------------------------------------------------------------*/

void
ArmSystemDoArmControl( )
{
    // get controller
    if( vexControllerGet( Btn8U ) == 1 )
        arm_pid->target_value = arm_pid->target_value + 20;
    else
    if( vexControllerGet( Btn8D ) == 1 )
        arm_pid->target_value = arm_pid->target_value - 25;

    // If the intake is rotated we need to wait until it is in the
    // correct position to allow the arm to be lowered.
    if( arm_pid->target_value < 400 )
        {
        if( abs(intake_pid->sensor_value) > 45 )
            arm_pid->target_value = 400;
        }

    // clip
    if( arm_pid->target_value < 0 )
        arm_pid->target_value = 0;
    if( arm_pid->target_value > 1500 )
        arm_pid->target_value = 1500;

    // pid control
    PidControllerUpdate( arm_pid );

    // Kill if power is lost
    if( vexSpiGetMainBattery() < 3000 )
        arm_pid->drive_cmd = 0;

    // check limit switches
    ArmSystemCheckHardLimits();

    // send to motors
    SetMotor( MotorArmL, arm_pid->drive_cmd);
    SetMotor( MotorArmR, arm_pid->drive_cmd);
}

/*-----------------------------------------------------------------------------*/
/*  PID control of the intake                                                  */
/*-----------------------------------------------------------------------------*/

void
ArmSystemDoIntakeControl( )
{
    // get controller
    if( vexControllerGet( Btn6U ) == 1 )
        intake_pid->target_value = intake_pid->target_value - 5;
    else
    if( vexControllerGet( Btn6D ) == 1 )
        intake_pid->target_value = intake_pid->target_value + 5;

    // limit intake to 180 degrees (quad encoder has 360 ticks per rev)
    if( abs(intake_pid->target_value) > 180 )
        {
        if( intake_pid->target_value > 0 )
            intake_pid->target_value = 180;
        else
            intake_pid->target_value = -180;
        }

    // No operation if arm is low
    if( arm_pid->target_value < 500 )
        intake_pid->target_value = 0;

    // pid control
    PidControllerUpdate( intake_pid );

    // Kill if power is lost
    if( vexSpiGetMainBattery() < 3000 )
        intake_pid->drive_cmd = 0;

    // send to motors
    SetMotor( MotorIL, intake_pid->drive_cmd);
    SetMotor( MotorIR, intake_pid->drive_cmd );
}

/*-----------------------------------------------------------------------------*/
/*  Arm control task                                                           */
/*-----------------------------------------------------------------------------*/

task
ArmTask(void *arg)
{
    (void) arg;

    // Must call this
    vexTaskRegister("arm task");

    // Init - no bias
    arm_pid = PidControllerInit( 0.004, 0.0, 0.01, EncArmR, 0 );
    intake_pid = PidControllerInit( 0.02, 0.00, 0.05, EncIntake, 0 );
    vexSensorValueSet( EncArmR,   0);
    vexSensorValueSet( EncIntake, 0);

    while(!chThdShouldTerminate())
        {
        // control the arm
        ArmSystemDoArmControl();
        // control the scoop
        ArmSystemDoIntakeControl();
        wait1Msec(25);
        }
    return (msg_t)0;
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
    SmartMotorSetControllerStatusLed( SMLIB_CORTEX_PORT_0, kVexDigital_11 );
    SmartMotorSetControllerStatusLed( SMLIB_CORTEX_PORT_1, kVexDigital_12 );

    SmartMotorLinkMotors( MotorArmR, MotorArmL );

    SmartMotorsSetEncoderGearing( MotorIR, 0.33333333 );
    SmartMotorLinkMotors( MotorIR, MotorIL );

    SmartMotorRun();
}

// Autonomous control task
msg_t
vexAutonomous( void *arg )
{
    (void)arg;

    // Must call this
    vexTaskRegister("auton");

    while(!chThdShouldTerminate())
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
	(void)arg;

	// start estop task
    StartTaskWithPriority( eStopTask, NORMALPRIO + 5 );

	// Must call this
	vexTaskRegister("operator");

    // control tasks
    StartTask( DriveTask );
    StartTask( ArmTask );

    // Init LCD
    vexLcdBacklight( VEX_LCD_DISPLAY_2, 1);
    vexLcdClearLine( VEX_LCD_DISPLAY_2, VEX_LCD_LINE_T);
    vexLcdClearLine( VEX_LCD_DISPLAY_2, VEX_LCD_LINE_B);

    // loop  and display status
	// Run until asked to terminate
	while(!chThdShouldTerminate())
		{
	    LcdDisplayStatus();

	    // Don't hog cpu
		vexSleep( 25 );
		}

	return (msg_t)0;
}












