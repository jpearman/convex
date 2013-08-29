/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexdefault.c                                                 */
/*    Author:     James Pearman                                                */
/*    Created:    8 July 2013                                                  */
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

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexdefault.c
  * @brief   Implementation of the cortex default code
  * @details
  * <pre>
  * Jumpers and ports are used as follows
  * 
  * Motors by default: + CW, - CCW
  * Jumper IN  = 0
  * Jumper OUT = 1
  * Analog IN  = 5
  * Analog OUT = 249

  * jmp1  = IN: motor 1  reversed
  * jmp2  = IN: motor 2  reversed
  * jmp3  = IN: motor 3  reversed
  * jmp4  = IN: motor 4  reversed
  * jmp5  = IN: motor 5  reversed
  * jmp6  = IN: motor 6  reversed
  * jmp7  = IN: motor 7  reversed
  * jmp8  = IN: motor 8  reversed
  * jmp9  = IN: motor 9  reversed
  * jmp10 = IN: motor 10 reversed
  *
  * jmp11 = OUT && jmp12 = OUT: Single Driver TANK
  * jmp11 =  IN && jmp12 = OUT: Dual Driver   TANK
  * jmp11 = OUT && jmp12 =  IN: Single Driver ARCADE
  * jmp11 =  IN && jmp12 =  IN: Dual Driver   ARCADE
  *
  * ana1  < 200: motor 6 IGNORE CCW
  * ana2  < 200: motor 6 IGNORE CW
  * ana3  < 200: motor 7 IGNORE CCW
  * ana4  < 200: motor 7 IGNORE CW
  * ana5  < 200: motor 8 IGNORE CCW
  * ana6  < 200: motor 8 IGNORE CW
  * ana7  < 200: motor 9 IGNORE CCW
  * ana8  < 200: motor 9 IGNORE CW
  * </pre>
*//*---------------------------------------------------------------------------*/

// Digi IO configuration
static  vexDigiCfg  dConfig[kVexDigital_Num] = {
        { kVexDigital_1,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
        { kVexDigital_2,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
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

static  vexMotorCfg mConfig[kVexMotorNum] = {
        { kVexMotor_1,      kVexMotor393T,          kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_2,      kVexMotor393T,          kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_3,      kVexMotor393T,          kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_4,      kVexMotor393T,          kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_5,      kVexMotor393T,          kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_6,      kVexMotor393T,          kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_7,      kVexMotor393T,          kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_8,      kVexMotor393T,          kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_9,      kVexMotor393T,          kVexMotorNormal,       kVexSensorNone,        0 },
        { kVexMotor_10,     kVexMotor393T,          kVexMotorNormal,       kVexSensorNone,        0 }
};

/*-----------------------------------------------------------------------------*/
/** @cond      name the motor ports                                            */
/*-----------------------------------------------------------------------------*/
#define LeftDrive1      kVexMotor_1
#define LeftDrive2      kVexMotor_2
#define LeftDrive3      kVexMotor_3
#define RightDrive4     kVexMotor_4
#define RightDrive5     kVexMotor_5
#define Mech1           kVexMotor_6
#define Mech2           kVexMotor_7
#define Mech3           kVexMotor_8
#define Mech4           kVexMotor_9
#define RightDrive10    kVexMotor_10
/** @endcond */

/** Jumper in pulls digital port low
 */
#define JUMPER_IN       0
/** Jumper out allows digital port to be pulled high
 */
#define JUMPER_OUT      1



/*-----------------------------------------------------------------------------*/
/** @brief      Cortex default code initialization                             */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Setup motors and digital ports
 */

void
vexCortexDefaultSetup(void)
{
    vexDigitalConfigure( dConfig, DIG_CONFIG_SIZE( dConfig ) );
    vexMotorConfigure( mConfig, MOT_CONFIG_SIZE( mConfig ) );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Cortex default code                                            */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function simulates the cortex default code, it was based on the ROBOTC
 *  implementation but with some changes in structure.
 */
void
vexCortexDefaultDriver(void)
{
    int16_t mech1_lim_ccw = 0;
    int16_t mech1_lim_cw  = 0;
    int16_t mech2_lim_ccw = 0;
    int16_t mech2_lim_cw  = 0;
    int16_t mech3_lim_ccw = 0;
    int16_t mech3_lim_cw  = 0;
    int16_t mech4_lim_ccw = 0;
    int16_t mech4_lim_cw  = 0;

    int16_t drive_left  = 0;
    int16_t drive_right = 0;
    int16_t drive_mech1 = 0;
    int16_t drive_mech2 = 0;
    int16_t drive_mech3 = 0;
    int16_t drive_mech4 = 0;

    while( !chThdShouldTerminate() )
        {
        // Set motor direction - a jumper in ports 1 through 10 reversed motor
        vexMotorDirectionSet( LeftDrive1,   vexDigitalPinGet( kVexDigital_1  ) == JUMPER_IN ? kVexMotorReversed : kVexMotorNormal );
        vexMotorDirectionSet( LeftDrive2,   vexDigitalPinGet( kVexDigital_2  ) == JUMPER_IN ? kVexMotorReversed : kVexMotorNormal );
        vexMotorDirectionSet( LeftDrive3,   vexDigitalPinGet( kVexDigital_3  ) == JUMPER_IN ? kVexMotorReversed : kVexMotorNormal );
        vexMotorDirectionSet( RightDrive4,  vexDigitalPinGet( kVexDigital_4  ) == JUMPER_IN ? kVexMotorReversed : kVexMotorNormal );
        vexMotorDirectionSet( RightDrive5,  vexDigitalPinGet( kVexDigital_5  ) == JUMPER_IN ? kVexMotorReversed : kVexMotorNormal );
        vexMotorDirectionSet( Mech1,        vexDigitalPinGet( kVexDigital_6  ) == JUMPER_IN ? kVexMotorReversed : kVexMotorNormal );
        vexMotorDirectionSet( Mech2,        vexDigitalPinGet( kVexDigital_7  ) == JUMPER_IN ? kVexMotorReversed : kVexMotorNormal );
        vexMotorDirectionSet( Mech3,        vexDigitalPinGet( kVexDigital_8  ) == JUMPER_IN ? kVexMotorReversed : kVexMotorNormal );
        vexMotorDirectionSet( Mech4,        vexDigitalPinGet( kVexDigital_9  ) == JUMPER_IN ? kVexMotorReversed : kVexMotorNormal );
        vexMotorDirectionSet( RightDrive10, vexDigitalPinGet( kVexDigital_10 ) == JUMPER_IN ? kVexMotorReversed : kVexMotorNormal );

        // Sample analog inputs - these are used as limit switches
        mech1_lim_ccw = vexAdcGet( kVexAnalog_1 ) < 200 ? 0 : 1;
        mech1_lim_cw  = vexAdcGet( kVexAnalog_2 ) < 200 ? 0 : 1;
        mech2_lim_ccw = vexAdcGet( kVexAnalog_3 ) < 200 ? 0 : 1;
        mech2_lim_cw  = vexAdcGet( kVexAnalog_4 ) < 200 ? 0 : 1;
        mech3_lim_ccw = vexAdcGet( kVexAnalog_5 ) < 200 ? 0 : 1;
        mech3_lim_cw  = vexAdcGet( kVexAnalog_6 ) < 200 ? 0 : 1;
        mech4_lim_ccw = vexAdcGet( kVexAnalog_7 ) < 200 ? 0 : 1;
        mech4_lim_cw  = vexAdcGet( kVexAnalog_8 ) < 200 ? 0 : 1;

        // SINGLE DRIVER - TANK
        if( vexDigitalPinGet( kVexDigital_11 ) == JUMPER_OUT && vexDigitalPinGet( kVexDigital_12 ) == JUMPER_OUT )
            {
            drive_left  =  vexControllerGet( Ch3 ); // up = CW
            drive_right = -vexControllerGet( Ch2 ); // up = CCW

            drive_mech1 = (vexControllerGet( Btn5U ) * 127) - (vexControllerGet( Btn5D ) * 127); // U = CW, D = CCW
            drive_mech2 = (vexControllerGet( Btn6U ) * 127) - (vexControllerGet( Btn6D ) * 127); // U = CW, D = CCW
            drive_mech3 = (vexControllerGet( Btn7U ) * 127) - (vexControllerGet( Btn7D ) * 127); // U = CW, D = CCW
            drive_mech4 = (vexControllerGet( Btn8U ) * 127) - (vexControllerGet( Btn8D ) * 127); // U = CW, D = CCW
            }
        else
        // DUAL DRIVER - TANK
        if( vexDigitalPinGet( kVexDigital_11 ) == JUMPER_IN && vexDigitalPinGet( kVexDigital_12 ) == JUMPER_OUT )
            {
            drive_left  =  vexControllerGet( Ch3 ); // up = CW
            drive_right = -vexControllerGet( Ch2 ); // up = CCW

            drive_mech1 = (vexControllerGet( Btn5UXmtr2 ) * 127) - (vexControllerGet( Btn5DXmtr2 ) * 127); // U = CW, D = CCW
            drive_mech2 = (vexControllerGet( Btn6UXmtr2 ) * 127) - (vexControllerGet( Btn6DXmtr2 ) * 127); // U = CW, D = CCW
            drive_mech3 =  vexControllerGet( Ch3Xmtr2 ); // up = CW
            drive_mech4 =  vexControllerGet( Ch2Xmtr2 ); // up = CW
            }
        else
        // SINGLE DRIVER - ARCADE
        if( vexDigitalPinGet( kVexDigital_11 ) == JUMPER_OUT && vexDigitalPinGet( kVexDigital_12 ) == JUMPER_IN )
            {
            drive_left  =  vexControllerGet( Ch1 ) + vexControllerGet( Ch2 ); // up = CW,  right = CW
            drive_right =  vexControllerGet( Ch1 ) - vexControllerGet( Ch2 ); // up = CCW, right = CW

            drive_mech1 = (vexControllerGet( Btn5U ) * 127) - (vexControllerGet( Btn5D ) * 127); // U = CW, D = CCW
            drive_mech2 = (vexControllerGet( Btn6U ) * 127) - (vexControllerGet( Btn6D ) * 127); // U = CW, D = CCW
            drive_mech3 =  vexControllerGet( Ch3 ); // up = CW
            drive_mech4 = -vexControllerGet( Ch4 ); // right = CCW
            }
        else
        // DUAL DRIVER - ARCADE
        if( vexDigitalPinGet( kVexDigital_11 ) == JUMPER_IN && vexDigitalPinGet( kVexDigital_12 ) == JUMPER_IN )
            {
            drive_left  =  vexControllerGet( Ch1 ) + vexControllerGet( Ch2 ); // up = CW,  right = CW
            drive_right =  vexControllerGet( Ch1 ) - vexControllerGet( Ch2 ); // up = CCW, right = CW

            drive_mech1 = (vexControllerGet( Btn5UXmtr2 ) * 127) - (vexControllerGet( Btn5DXmtr2 ) * 127); // U = CW, D = CCW
            drive_mech2 = (vexControllerGet( Btn6UXmtr2 ) * 127) - (vexControllerGet( Btn6DXmtr2 ) * 127); // U = CW, D = CCW
            drive_mech3 =  vexControllerGet( Ch3Xmtr2 ); // up = CW
            drive_mech4 =  vexControllerGet( Ch2Xmtr2 ); // up = CW
            }

        // Left drive motors
        vexMotorSet( LeftDrive1,   drive_left  );
        vexMotorSet( LeftDrive2,   drive_left  );
        vexMotorSet( LeftDrive3,   drive_left  );

        // Right drive motors
        vexMotorSet( RightDrive4,  drive_right );
        vexMotorSet( RightDrive5,  drive_right );
        vexMotorSet( RightDrive10, drive_right );

        // Limit switches
        if( ( !mech1_lim_cw && (drive_mech1 > 0)) || ( !mech1_lim_ccw && (drive_mech1 < 0)) )
            drive_mech1 = 0;

        if( ( !mech2_lim_cw && (drive_mech2 > 0)) || ( !mech2_lim_ccw && (drive_mech2 < 0)) )
            drive_mech2 = 0;

        if( ( !mech3_lim_cw && (drive_mech3 > 0)) || ( !mech3_lim_ccw && (drive_mech3 < 0)) )
            drive_mech3 = 0;

        if( ( !mech4_lim_cw && (drive_mech4 > 0)) || ( !mech4_lim_ccw && (drive_mech4 < 0)) )
            drive_mech4 = 0;

        // Mechanism motors
        vexMotorSet( Mech1, drive_mech1 );
        vexMotorSet( Mech2, drive_mech2 );
        vexMotorSet( Mech3, drive_mech3 );
        vexMotorSet( Mech4, drive_mech4 );

        // small delay
        vexSleep(10);
        }
}
