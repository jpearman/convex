/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     OpenSourceBot_lcd.c                                          */
/*    Author:     James Pearman                                                */
/*    Created:    31 March 2013                                                */
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
/*    LCD display for the simplified version of the open source robot code     */
/*-----------------------------------------------------------------------------*/

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

#include "robotc_glue.h"
#include "smartmotor.h"
#include "osr.h"

typedef enum {
    kLcdDispPreStart = -1,  // force signed enum
    kLcdDispStart = 0,

    kLcdDispDriveSpeed = 0,
    kLcdDispDriveCurrent,
    kLcdDispDriveTemp,
    kLcdDispDriveMotors,
    kLcdDispDrivePosition,

    kLcdDispArmSpeed,
    kLcdDispArmCurrent,
    kLcdDispArmPosition,

    kLcdDispSonar,
    kLcdDispLineFollow,

    kLcdDispI2CStatus,
    kLcdDispSysStatus,

    kLcdDispNumber
    } kLcdDispType;

kLcdDispType    mode = kLcdDispSysStatus;

#define LCD_DISP    VEX_LCD_DISPLAY_2

/*-----------------------------------------------------------------------------*/
/*  Utility functions to display various robot variables                       */
/*-----------------------------------------------------------------------------*/

void
LcdDisplayMotors( tVexMotor m1, tVexMotor m2, tVexMotor m3, tVexMotor m4 )
{
    vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_T, "%4d  %4d    ", vexMotorGet( m1 ), vexMotorGet( m2 ));
    if(m3!=(-1)) {
        vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_B, "%4d  %4d    ", vexMotorGet( m3 ), vexMotorGet( m4 ));
    }
}

/*-----------------------------------------------------------------------------*/

void
LcdDisplayMotorEncoders( tVexMotor m1, tVexMotor m2, tVexMotor m3, tVexMotor m4 )
{
    if(m2==(-1))
        vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_T, "%7d         ",vexMotorPositionGet( m1 ));
    else
        vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_T, "%7d  %7d",vexMotorPositionGet( m1 ), vexMotorPositionGet( m2 ));


    if(m3!=(-1)) {
        if(m4==(-1))
            vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_B, "%7d         ",vexMotorPositionGet( m3 ));
        else
            vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_B, "%7d  %7d",vexMotorPositionGet( m3 ), vexMotorPositionGet( m4 ));
    }
}

/*-----------------------------------------------------------------------------*/

void
LcdDisplaySpeeds( tVexMotor m1, tVexMotor m2, tVexMotor m3, tVexMotor m4)
{
    vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_T, "%7.2f  %7.2f", SmartMotorGetSpeed( m1 ), SmartMotorGetSpeed( m2 ));

    if(m3!=(-1)) {
        vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_B, "%7.2f  %7.2f", SmartMotorGetSpeed( m3 ), SmartMotorGetSpeed( m4 ));
    }
}

/*-----------------------------------------------------------------------------*/

void
LcdDisplayCurrent( tVexMotor m1, tVexMotor m2, tVexMotor m3, tVexMotor m4)
{
    vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_T, "%7.2f  %7.2f", SmartMotorGetCurrent( m1 ), SmartMotorGetCurrent( m2 ));
    if(m3!=(-1)) {
        vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_B, "%7.2f  %7.2f", SmartMotorGetCurrent( m3 ), SmartMotorGetCurrent( m4 ));
    }
}

/*-----------------------------------------------------------------------------*/

void
LcdDisplayTemperature( tVexMotor m1, tVexMotor m2, tVexMotor m3, tVexMotor m4 )
{
    vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_T, "%7.2f  %7.2f", SmartMotorGetTemperature( m1 ), SmartMotorGetTemperature( m2 ));
    vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_B, "%7.2f  %7.2f", SmartMotorGetTemperature( m3 ), SmartMotorGetTemperature( m4 ));
}

/*-----------------------------------------------------------------------------*/

void
LcdDisplaySysStatus( void )
{
    vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_T, "VBatt %7.2f   ", vexSpiGetMainBattery()/1000.0 );
}

/*-----------------------------------------------------------------------------*/

void
LcdDisplayLineFollowStatus( tVexSensors s1, tVexSensors s2, tVexSensors s3 )
{
    vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_T, "Line Sensors    ");
    vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_B, "%4d %4d %4d ", vexSensorValueGet( s1 ), vexSensorValueGet( s2 ), vexSensorValueGet( s3 ) );
}

/*-----------------------------------------------------------------------------*/

void
LcdDisplaySonarStatus( tVexSensors s1 )
{
    vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_T,"Rear Sonar      ");
    vexLcdPrintf( LCD_DISP, VEX_LCD_LINE_B,"%4d", vexSensorValueGet( s1 ) );
}

/*-----------------------------------------------------------------------------*/
/*  Call this function to display robot variables and status                   */
/*  Use the LCD left and right buttons to change the displayed information     */
/*  You can also use joystick buttons 7D and 7R for the same function          */
/*-----------------------------------------------------------------------------*/

void
LcdDisplayStatus(void)
{
    vexLcdButton    Buttons;

    // Select display Item
    Buttons = vexLcdButtonGet(LCD_DISP);
    if( vexControllerGet( Btn7D ) )
        Buttons = kLcdButtonLeft;
    if( vexControllerGet( Btn7R ) )
        Buttons = kLcdButtonRight;

    if( (Buttons == kLcdButtonLeft) || (Buttons == kLcdButtonRight) )
        {
        if( Buttons == kLcdButtonRight )
           {
            mode++;
            if(mode >= kLcdDispNumber)
                mode = kLcdDispStart;
            }
        if( Buttons == kLcdButtonLeft )
            {
            mode--;
            if(mode < kLcdDispStart)
                mode = (kLcdDispNumber-1);
            }

        vexLcdClearLine(LCD_DISP, VEX_LCD_LINE_T);
        vexLcdClearLine(LCD_DISP, VEX_LCD_LINE_B);

        switch(mode)
            {
            case    kLcdDispDriveSpeed:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Speed Drive     ");
                break;
            case    kLcdDispDriveCurrent:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Current Drive   ");
                break;
            case    kLcdDispDriveTemp:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Temp Drive      ");
                break;
            case    kLcdDispDriveMotors:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Motor Drive     ");
                break;
            case    kLcdDispDrivePosition:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Position Drive  ");
                break;

            case    kLcdDispArmSpeed:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Speed Arm/Intake");
                break;
            case    kLcdDispArmCurrent:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Current Arm/Intk");
                break;
            case    kLcdDispArmPosition:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Position Arm    ");
                break;

            case    kLcdDispSonar:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Rear Sonar      ");
                break;
            case    kLcdDispLineFollow:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Line Sensors    ");
                break;
            case    kLcdDispI2CStatus:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "I2C Status      ");
                break;
            case    kLcdDispSysStatus:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Status          ");
                break;

            default:
                vexLcdSet( LCD_DISP,VEX_LCD_LINE_T, "Err             ");
                break;
            }

        do {
            Buttons = vexLcdButtonGet(LCD_DISP);
            if( vexControllerGet( Btn7D ) )
                Buttons = kLcdButtonLeft;
            if( vexControllerGet( Btn7R ) )
                Buttons = kLcdButtonRight;
            vexSleep(10);
            } while( Buttons != kLcdButtonNone );


        vexSleep(250);
        }

   switch( mode )
        {
        case    kLcdDispDriveSpeed:
            LcdDisplaySpeeds( MotorLF, MotorRF, MotorLB, MotorRB );
            break;

        case    kLcdDispDriveCurrent:
            LcdDisplayCurrent( MotorLF, MotorRF, MotorLB, MotorRB );
            break;

        case    kLcdDispDriveTemp:
            LcdDisplayTemperature( MotorLF, MotorRF, MotorLB, MotorRB );
            break;

        case    kLcdDispDriveMotors:
            LcdDisplayMotors( MotorLF, MotorRF, MotorLB, MotorRB );
            break;

        case    kLcdDispDrivePosition:
            LcdDisplayMotorEncoders( MotorLF, MotorRF, MotorLB, MotorRB );
            break;


        case    kLcdDispArmSpeed:
            LcdDisplaySpeeds( MotorArmL, MotorArmR, MotorIL, MotorIR );
            break;

        case    kLcdDispArmCurrent:
            LcdDisplayCurrent( MotorArmL, MotorArmR, MotorIL, MotorIR);
            break;

        case    kLcdDispArmPosition:
            LcdDisplayMotorEncoders( MotorArmR, -1, -1, -1 );
            break;

        case    kLcdDispSonar:
            LcdDisplaySonarStatus( RearSonar );
            break;

        case    kLcdDispLineFollow:
            LcdDisplayLineFollowStatus( LineFollowC, LineFollowB, LineFollowA );
            break;

        case    kLcdDispI2CStatus:
            break;

        case    kLcdDispSysStatus:
            LcdDisplaySysStatus();
            break;

       default:
            vexLcdSet( LCD_DISP, VEX_LCD_LINE_T, "error" );
            break;
        }
}
