/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     clawbot.c                                                    */
/*    Author:     James Pearman                                                */
/*    Created:    18 June 2013                                                 */
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
/*                                                                             */
/*   Clawbot demo code                                                         */
/*   A port of my default clawbot code                                         */
/*-----------------------------------------------------------------------------*/

#include <stdlib.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header
#include "smartmotor.h"
#include "robotc_glue.h"
#include "clawbot.h"

/*-----------------------------------------------------------------------------*/
/** @file    clawbot.c
  * @brief   Clawbot demo code
*//*----------------------------------------------------------------------------*/

/** @example clawbot.c
 *  An example that controls the VEX clawbot
 */

// Digi IO configuration
static  vexDigiCfg  dConfig[kVexDigital_Num] = {
        { kVexDigital_1,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_2,    kVexSensorDigitalOutput, kVexConfigOutput,      0 },
        { kVexDigital_3,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_4,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_5,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_6,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_7,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_8,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_9,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_10,   kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_11,   kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_12,   kVexSensorDigitalInput,  kVexConfigInput,       0 }
};

static  vexMotorCfg mConfig[10] = {
        { kVexMotor_1,      kVexMotor393T,          kVexMotorNormal,       kVexSensorIME,         kImeChannel_1 },
        { kVexMotor_2,      kVexMotorUndefined,     kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_3,      kVexMotorUndefined,     kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_4,      kVexMotorUndefined,     kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_5,      kVexMotorUndefined,     kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_6,      kVexMotorUndefined,     kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_7,      kVexMotorUndefined,     kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_8,      kVexMotor393T,          kVexMotorReversed,     kVexSensorNone,        0 },
        { kVexMotor_9,      kVexMotor393T,          kVexMotorNormal,       kVexSensorIME,         kImeChannel_3 },
        { kVexMotor_10,     kVexMotor393T,          kVexMotorReversed,     kVexSensorIME,         kImeChannel_2 }
};

#define MotorDriveR     kVexMotor_1
#define MotorArm        kVexMotor_8
#define MotorClaw       kVexMotor_9
#define MotorDriveL     kVexMotor_10
#define armPot          kVexAnalog_1
#define clawPot         kVexAnalog_2

/*-----------------------------------------------------------------------------*/
/*  Drive control task                                                         */
/*                                                                             */
/*  Uses joystick Ch3, Ch4                                                     */
/*-----------------------------------------------------------------------------*/

void
DriveSystemArcadeDrive( short forward, short turn )
{
    long drive_l_motor;
    long drive_r_motor;

    // Set drive
    drive_l_motor = forward + turn;
    drive_r_motor = forward - turn;

    // normalize drive so max is 127 if any drive is over 127
    int max = abs(drive_l_motor);
    if (abs(drive_r_motor)  > max)
        max = abs(drive_r_motor);
    if (max>127) {
        drive_l_motor = 127 * drive_l_motor / max;
        drive_r_motor = 127 * drive_r_motor  / max;
    }

    // Send to motors
    // left drive
    SetMotor( MotorDriveL, drive_l_motor);

    // right drive
    SetMotor( MotorDriveR, drive_r_motor);
}

/*-----------------------------------------------------------------------------*/
/*  Drive control task                                                         */
/*-----------------------------------------------------------------------------*/

task
DriveTask(void *arg)
{
    short   forward, turn;

    (void)arg;
    vexTaskRegister("Drive task");

    while( TRUE )
        {
        // Get controller
        if( abs( vexControllerGet( Ch3 ) ) > 10 )
            forward = vexControllerGet( Ch3 );
        else
            forward = 0;

        if( abs( vexControllerGet( Ch4 ) ) > 10 )
            turn = vexControllerGet( Ch4 );
        else
            turn = 0;

        DriveSystemArcadeDrive( forward, turn );

        wait1Msec(25);
        }
}

/*-----------------------------------------------------------------------------*/
/*  Arm control                                                                */
/*-----------------------------------------------------------------------------*/

// Global to hold requested (target) arm position
static int  armRequestedValue;

// Arm limits
#define ARM_UPPER_LIMIT     2000
#define ARM_LOWER_LIMIT      600

#define ARM_PRESET_H        2000
#define ARM_PRESET_L         650

/*-----------------------------------------------------------------------------*/
/*  Set requested arm position and clip to limits                              */
/*-----------------------------------------------------------------------------*/

static void
SetArmPosition( int position )
{
    // crude limiting to upper and lower values
    if( position > ARM_UPPER_LIMIT )
        armRequestedValue = ARM_UPPER_LIMIT;
    else
    if( position < ARM_LOWER_LIMIT )
        armRequestedValue = ARM_LOWER_LIMIT;
    else
        armRequestedValue = position;
}

/*-----------------------------------------------------------------------------*/
/*  Get requested arm position                                                 */
/*-----------------------------------------------------------------------------*/

static int
GetArmPosition(void)
{
    return( armRequestedValue );
}

/*-----------------------------------------------------------------------------*/
/*  arm pid (actually just P) control task                                     */
/*-----------------------------------------------------------------------------*/

task ArmPidController(void *arg)
{
           int    armSensorCurrentValue;
           int    armError;
           float  armDrive;
    static float  pid_K = 0.3;

    (void)arg;
    vexTaskRegister("arm pid");

    while( TRUE )
        {
        // Read the sensor value and scale
        armSensorCurrentValue = vexAdcGet( armPot );

        // calculate error
        armError =  armRequestedValue - armSensorCurrentValue;

        // calculate drive
        armDrive = (pid_K * (float)armError);

        // limit drive
        if( armDrive > 127 )
            armDrive = 127;
        else
        if( armDrive < (-127) )
            armDrive = (-127);

        // send to motor
        SetMotor( MotorArm, armDrive);

        // Don't hog cpu
        wait1Msec( 25 );
        }
}

/*-----------------------------------------------------------------------------*/
/*  Claw control code                                                          */
/*-----------------------------------------------------------------------------*/

typedef enum {
    kClawNoCommand = 0,
    kClawOpenCommand,
    kClawCloseCommand,
    kClawStopCommand
    } clawCommand;

clawCommand clawCmd = kClawNoCommand;

#define CLAW_OPEN_HIGH_SPEED      80
#define CLAW_CLOSE_HIGH_SPEED    -80
#define CLAW_OPEN_LOW_SPEED       20
#define CLAW_CLOSE_LOW_SPEED     -15

/*-----------------------------------------------------------------------------*/
/*  Run the claw motor in the close direction                                  */
/*-----------------------------------------------------------------------------*/

static void
ClawClose(void)
{
    clawCmd = kClawCloseCommand;
}

/*-----------------------------------------------------------------------------*/
/*  Run the claw motor in the open direction                                   */
/*-----------------------------------------------------------------------------*/

static void
ClawOpen(void)
{
    clawCmd = kClawOpenCommand;
}

/*-----------------------------------------------------------------------------*/
/*  Stop claw motor                                                            */
/*-----------------------------------------------------------------------------*/

static void
ClawStop(void)
{
    clawCmd = kClawStopCommand;
}

/*-----------------------------------------------------------------------------*/
/*  Task to control the claw motors                                            */
/*-----------------------------------------------------------------------------*/

task ClawController(void *arg)
{
    int     hold_delay = 0;

    (void)arg;
    vexTaskRegister("claw control");

    while( TRUE )
        {
        switch( clawCmd )
            {
            case    kClawOpenCommand:
                // look for fully open (around 500 counts)
                if( vexMotorPositionGet( MotorClaw ) > 400 )
                    SetMotor( MotorClaw ,  CLAW_OPEN_LOW_SPEED);
                else
                    SetMotor( MotorClaw ,  CLAW_OPEN_HIGH_SPEED);

                break;

            case    kClawCloseCommand:
                // look for fully closed (enc == 0)
                if( vexMotorPositionGet( MotorClaw ) < 100 )
                    SetMotor( MotorClaw , CLAW_CLOSE_LOW_SPEED);
                else
                    {
                    // This test checks for slow motor movement when partially closed
                    // half second delay before check starts
                    if(hold_delay++ > 20)
                        {
                        if( SmartMotorGetSpeed( MotorClaw ) > -40 )
                            SetMotor( MotorClaw , CLAW_CLOSE_LOW_SPEED);
                        }
                    else
                        SetMotor( MotorClaw , CLAW_CLOSE_HIGH_SPEED );
                    }

                // learn minimum encoder position
                if( vexMotorPositionGet( MotorClaw ) < 0 )
                    vexMotorPositionSet( MotorClaw, 0);
                break;

            case    kClawStopCommand:
                SetMotor( MotorClaw ,  0 );
                clawCmd = kClawNoCommand;

                break;

            default:
                break;
            }

        // reset hold delay
        if(clawCmd != kClawCloseCommand)
            hold_delay = 0;

        // Don't hog cpu
        wait1Msec(25);
        }
}

/*-----------------------------------------------------------------------------*/
/*  Task to control arm (and claw) movement                                    */
/*-----------------------------------------------------------------------------*/

task
ManualArmClawTask(void *arg)
{
    (void)arg;
    vexTaskRegister("manual arm");

    // initialize
    SetArmPosition( vexAdcGet( armPot ) );

    // use joystick to modify the requested position
    while( TRUE )
        {
        // presets - assume joystick is centered
        if( vexControllerGet( Btn8U ) == 1 )
            SetArmPosition( ARM_PRESET_H );
        else
        if( vexControllerGet( Btn8D ) == 1 )
            SetArmPosition( ARM_PRESET_L );

        // manual control
        if( abs(vexControllerGet( Ch2 )) > 15 )
            SetArmPosition( GetArmPosition() + (vexControllerGet( Ch2 )) );

        // Claw control
        if( vexControllerGet( Btn6D ) == 1 )
            ClawOpen();
        else
        if( vexControllerGet( Btn6U ) == 1 )
            ClawClose();
        else
            ClawStop();

        // don't hog cpu
        wait1Msec(50);
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

    // start the Arm PID task
    StartTask( ArmPidController );
    // start the claw motor controller
    StartTask( ClawController );

    // Arm Up
    SetArmPosition( 2000 );
    // Claw Open
    ClawOpen();
    wait1Msec(2000);

    // Move forward
    DriveSystemArcadeDrive( 100, 0 );
    wait1Msec(1000);
    // Stop drivw
    DriveSystemArcadeDrive( 0, 0 );

    // Arm down
    SetArmPosition( 700 );
    wait1Msec(2000);

    // close claw
    ClawClose();
    wait1Msec(1000);

    // Arm up
    SetArmPosition( 1000 );
    wait1Msec(1000);
    // Turn
    DriveSystemArcadeDrive( 0, 100 );
    wait1Msec(1000);
    // Stop drivw
    DriveSystemArcadeDrive( 0, 0 );

    // Claw Open
    ClawOpen();
    wait1Msec(1000);

    // Stop claw motor
    ClawStop();

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
    (void)arg;

    // Must call this
    vexTaskRegister("operator");

    // Start manual driving
    StartTask( DriveTask );

    // start the Arm PID task
    StartTask( ArmPidController );
    // start the claw motor controller
    StartTask( ClawController );

    // start manual arm/claw control
    StartTask( ManualArmClawTask );

    // Run until asked to terminate
    while(!chThdShouldTerminate())
        {
        // Don't hog cpu
        vexSleep( 25 );
        }

    return (msg_t)0;
}
