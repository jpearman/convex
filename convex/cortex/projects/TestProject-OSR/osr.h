/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     osr.h                                                        */
/*    Author:     James Pearman                                                */
/*    Created:    18 June 2013                                                 */
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


#define     LineFollowA         kVexSensorAnalog_1
#define     LineFollowB         kVexSensorAnalog_2
#define     LineFollowC         kVexSensorAnalog_3
#define     GyroInput           kVexSensorAnalog_4
#define     CurrentMonitor      kVexSensorAnalog_8

#define     RearSonar           kVexSensorDigital_1
#define     EStop               kVexSensorDigital_3
#define     EncIntakeIndex      kVexSensorDigital_4
#define     EncIntake           kVexSensorDigital_5
#define     ArmRLimitHigh       kVexSensorDigital_7
#define     ArmRLimitLow        kVexSensorDigital_8
#define     ArmRLimitLed        kVexSensorDigital_9
#define     led1                kVexSensorDigital_10
#define     led2                kVexSensorDigital_11
#define     led3                kVexSensorDigital_12

#define     EncLF               kVexSensorIme_1
#define     EncLB               kVexSensorIme_2
#define     EncRB               kVexSensorIme_3
#define     EncRF               kVexSensorIme_4
#define     EncArmR             kVexSensorIme_5

#define     MotorArmL           kVexMotor_1
#define     MotorLF             kVexMotor_2
#define     MotorLB             kVexMotor_3
#define     MotorIL             kVexMotor_4

#define     MotorIR             kVexMotor_7
#define     MotorRB             kVexMotor_8
#define     MotorRF             kVexMotor_9
#define     MotorArmR           kVexMotor_10


#ifdef __cplusplus
extern "C" {
#endif

task    eStopTask( void *arg );
void    DriveSystemMecanumDrive( void );
task    DriveTask(void *arg);
void    ArmSystemCheckHardLimits( void );
void    ArmSystemDoArmControl( void );
void    ArmSystemDoIntakeControl( void );
task    ArmTask( void *arg );

void    LcdDisplayStatus(void);

#ifdef __cplusplus
}
#endif
