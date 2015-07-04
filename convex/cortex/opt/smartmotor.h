/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     SmartMotorLib.h                                              */
/*    Author:     James Pearman                                                */
/*    Created:    2 Oct 2012                                                   */
/*                V1.10     4 July 2013 - Initial release fopr ChibiOS         */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    This file is part of ConVEX.                                             */
/*                                                                             */
/*    The author is supplying this software for use with the VEX cortex        */
/*    control system. ConVEX is free software; you can redistribute it         */
/*    and/or modify it under the terms of the GNU General Public License       */
/*    as published by the Free Software Foundation; either version 3 of        */
/*    the License, or (at your option) any later version.                      */
/*                                                                             */
/*    ConVEX is distributed in the hope that it will be useful,                */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*    GNU General Public License for more details.                             */
/*                                                                             */
/*    You should have received a copy of the GNU General Public License        */
/*    along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*                                                                             */
/*    A special exception to the GPL can be applied should you wish to         */
/*    distribute a combined work that includes ConVEX, without being obliged   */
/*    to provide the source code for any proprietary components.               */
/*    See the file exception.txt for full details of how and when the          */
/*    exception can be applied.                                                */
/*                                                                             */
/*    The author can be contacted on the vex forums as jpearman                */
/*    or electronic mail using jbpearman_at_mac_dot_com                        */
/*    Mentor for team 8888 RoboLancers, Pasadena CA.                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#ifndef __SMARTMOTORLIB__
#define __SMARTMOTORLIB__

/*-----------------------------------------------------------------------------*/
/** @file    smartmotor.h
  * @brief   A port of the smart motor library for ChibiOS/RT, header
*//*---------------------------------------------------------------------------*/

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"

// Version 1.11
#define kSmartMotorLibVersion   112

// System parameters - don't change
#define SMLIB_R_SYS             0.3
#define SMLIB_PWM_FREQ          1150
#define SMLIB_V_DIODE           0.75

// parameters for vex 393 motor
#define SMLIB_I_FREE_393        0.2
#define SMLIB_I_STALL_393       4.8
#define SMLIB_RPM_FREE_393      110
#define SMLIB_R_393             (7.2/SMLIB_I_STALL_393)
#define SMLIB_L_393             0.000650
#define SMLIB_Ke_393            (7.2*(1-SMLIB_I_FREE_393/SMLIB_I_STALL_393)/SMLIB_RPM_FREE_393)
#define SMLIB_I_SAFE393         0.90

// parameters for vex 269 motor
#define SMLIB_I_FREE_269        0.18
#define SMLIB_I_STALL_269       2.88
#define SMLIB_RPM_FREE_269      120
#define SMLIB_R_269             (7.2/SMLIB_I_STALL_269)
#define SMLIB_L_269             0.000650
#define SMLIB_Ke_269            (7.2*(1-SMLIB_I_FREE_269/SMLIB_I_STALL_269)/SMLIB_RPM_FREE_269)
#define SMLIB_I_SAFE269         0.75

// parameters for cortex and Power expander
// spec says 4A but we have set a little lower here
// may increase in a subsequent release.
#define SMLIB_I_SAFECORTEX      3.0
#define SMLIB_I_SAFEPE          3.0

// encoder counts per revolution depending on motor
#define SMLIB_TPR_269           240.448
#define SMLIB_TPR_393R          261.333
#define SMLIB_TPR_393S          392
#define SMLIB_TPR_393T          627.2
#define SMLIB_TPR_QUAD          360.0
#define SMLIB_TPR_POT           6000.0 // estimate

// Initial ambient temp 72deg F in deg C
#define SMLIB_TEMP_AMBIENT      (( 72.0-32.0) * 5 / 9)
// Trip temperature for PTC 100 deg C, may be a little low
#define SMLIB_TEMP_TRIP         100.0
// Trip hysteresis in deg C, once tripped we need a 10 deg drop to enable normal operation
#define SMLIB_TEMP_HYST         10.0
// Reference temperature for data below, 25 deg C
#define SMLIB_TEMP_REF          25.0

// Hold current is the current where thr PTC should not trip
// Time to trip is the time at 5 x hold current
// K_TAU is a safety factor, probably 0.5 ~ 0.8
// we are being conservative here, it trip occurs too soon then increase
//
// PTC HR16-400 used in cortex and power expander
#define SMLIB_I_HOLD_CORTEX     3.0
#define SMLIB_T_TRIP_CORTEX     1.7
#define SMLIB_K_TAU_CORTEX      0.5
#define SMLIB_TAU_CORTEX        (SMLIB_K_TAU_CORTEX * SMLIB_T_TRIP_CORTEX * 5.0 * 5.0)
#define SMLIB_C1_CORTEX         ( (SMLIB_TEMP_TRIP - SMLIB_TEMP_REF) / (SMLIB_I_HOLD_CORTEX * SMLIB_I_HOLD_CORTEX) )
#define SMLIB_C2_CORTEX         (1.0 / (SMLIB_TAU_CORTEX * 1000.0))

// PTC HR30-090 used in 393
//#define SMLIB_I_HOLD_393      0.9
#define SMLIB_I_HOLD_393        1.0 // increased a little to slow down trip
#define SMLIB_T_TRIP_393        7.1
#define SMLIB_K_TAU_393         0.5
#define SMLIB_TAU_393           (SMLIB_K_TAU_393 * SMLIB_T_TRIP_393 * 5.0 * 5.0)
#define SMLIB_C1_393            ( (SMLIB_TEMP_TRIP - SMLIB_TEMP_REF) / (SMLIB_I_HOLD_393 * SMLIB_I_HOLD_393) )
#define SMLIB_C2_393            (1.0 / (SMLIB_TAU_393 * 1000.0))

// PTC HR16-075 used in 269
#define SMLIB_I_HOLD_269        0.75
#define SMLIB_T_TRIP_269        2.0
#define SMLIB_K_TAU_269         0.5
#define SMLIB_TAU_269           (SMLIB_K_TAU_269 * SMLIB_T_TRIP_269 * 5.0 * 5.0)
#define SMLIB_C1_269            ( (SMLIB_TEMP_TRIP - SMLIB_TEMP_REF) / (SMLIB_I_HOLD_269 * SMLIB_I_HOLD_269) )
#define SMLIB_C2_269            (1.0 / (SMLIB_TAU_269 * 1000.0))

// No way to test 3 wire - Vamfun had them more or less the same as the 269
// PTC MINISMDC-075F used in three wire
#define SMLIB_C1_3WIRE          SMLIB_C1_269
#define SMLIB_C2_3WIRE          SMLIB_C2_269

#define SMLIB_LEDON             0
#define SMLIB_LEDOFF            1


/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*  This large structure holds all the needed information for a single motor   */
/*                                                                             */
/*  The constants used in calculations are all set automatically by the init   */
/*  code.  Other variables are used by the different functions to calculate    */
/*  speed, current and temperature.  some of these are stored for debug        */
/*  purposes                                                                   */
/*                                                                             */
/*  V1.02 code uses 124 bytes (a couple are wasted due to word alignment)      */
/*  so 1240 bytes in all for the 10 motors.                                    */
/*-----------------------------------------------------------------------------*/

typedef struct {
    // the motor port
    tVexMotor      port;

    // the motor encoder port
    tVexMotor      eport;

    // copy of the system motor type
    tVexMotorType  type;

    // due to alignment we get a free variable here
    // use for debugging loop delay
    short       delayTimeMs;

    // pointer to our control bank
    struct _smartController *bank;

    // commanded speed comes from the user
    // requested speed is either the commanded speed or limited speed if the PTC
    // is about to trip
    short   motor_cmd;
    short   motor_req;
    short   motor_slew;

    // current limit and max cmd value
    short   limit_tripped;
    short   limit_cmd;
    float   limit_current;

    // the encoder associated with this motor
    short   encoder_id;
    // encoder ticks per rev
    float   ticks_per_rev;

    // variables used by rpm calculation
    long    enc;
    long    oldenc;
    float   delta;
    float   rpm;

    // variables used for current calculation
    float   i_free;
    float   i_stall;
    float   r_motor;
    float   l_motor;
    float   ke_motor;
    float   rpm_free;
    float   v_bemf_max;

    // instantaneous current
    float   current;
    // a filtered version of current to remove some transients
    float   filtered_current;
    // peak measured current
    float   peak_current;

    // holds safe current for this motor
    float   safe_current;
    // target current in limited mode
    float   target_current;

    // PTC monitor variables
    float   temperature;
    float   t_const_1;
    float   t_const_2;
    float   t_ambient;
    short   ptc_tripped;

    // Last program time we ran - may not keep this, bit overkill
    long    lastPgmTime;
    } smartMotor;

/*-----------------------------------------------------------------------------*/
/*  Motor control related definitions                                          */
/*-----------------------------------------------------------------------------*/

#define SMLIB_MOTOR_MAX_CMD             127     // maximum command value to motor
#define SMLIB_MOTOR_MIN_CMD             (-127)  // minimum command value to motor
#define SMLIB_MOTOR_DEFAULT_SLEW_RATE   10      // Default will cause 375mS from full fwd to rev
#define SMLIB_MOTOR_FAST_SLEW_RATE      256     // essentially off
#define SMLIB_MOTOR_DEADBAND            10      // values below this are set to 0

// When current limit is not needed set limit_cmd to this value
#define SMLIB_MOTOR_MAX_CMD_UNDEFINED   255     // special value for limit_motor

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*  This structure holds all the information for a controller bank             */
/*  which is a PTC protected circuit in the cortex or power expander           */
/*                                                                             */
/*  We limit storage to three banks as that is the limit for a VEX competition */
/*  robot although you could have more power expanders in theory               */
/*                                                                             */
/*  V1.00 code uses 52 bytes per bank                                          */
/*-----------------------------------------------------------------------------*/

// 3 banks of maximum 5 motors (even though power expander is 4)
#define SMLIB_TOTAL_NUM_CONTROL_BANKS   3
#define SMLIB_TOTAL_NUM_BANK_MOTORS     5
// Index for banks
#define SMLIB_CORTEX_PORT_0             0
#define SMLIB_CORTEX_PORT_1             1
#define SMLIB_PWREXP_PORT_0             2

typedef struct _smartController {
    // array of pointers to the motors
    smartMotor *motors[SMLIB_TOTAL_NUM_BANK_MOTORS];

    // cumulative current
    float  current;
    // peak measured current
    float  peak_current;
    // holds safe current for this motor
    float  safe_current;

    // PTC monitor variables
    float  temperature;
    float  t_const_1;
    float  t_const_2;
    float  t_ambient;

    // flag for ptc status
    short  ptc_tripped;

    // Do we have an led to show tripped status
    tVexDigitalPin statusLed;

    // Power expander may have a status port
    tVexAnalogPin statusPort;
    } smartController;


// We have no inline so use a macro as shortcut to get ptr
#define _SmartMotorGetPtr( index ) ((smartMotor *)&sMotors[ index ])
// We have no inline so use a macro as shortcut to get ptr
#define _SmartMotorControllerGetPtr( index ) ((smartController *)&sPorts[ index ])

// Initialization
void             SmartMotorsInit( void );
void             SmartMotorLinkMotors( tVexMotor master, tVexMotor slave );
void             SmartMotorsSetEncoderGearing( tVexMotor index, float ratio );
#define          SmartMotorsAddPowerExtender( p0, ... ) \
                 _SmartMotorsAddPowerExtender( p0, ##__VA_ARGS__, -1, -1, -1 )
void             _SmartMotorsAddPowerExtender( int p0, int p1, int p2, int p3, ... );

// Status
float            SmartMotorGetSpeed( tVexMotor index );
#define          SmartMotorGetCurrent(index, ... ) \
                 _SmartMotorGetCurrent( index, ##__VA_ARGS__, 0 )
float            _SmartMotorGetCurrent( tVexMotor index, int s, ... );
float            SmartMotorGetTemperature( tVexMotor index );
int              SmartMotorGetLimitCmd( tVexMotor index );

float            SmartMotorGetControllerCurrent( short index );
float            SmartMotorGetControllerTemperature( short index );

// Control
void             SmartMotorPtcMonitorEnable( void );
void             SmartMotorPtcMonitorDisable( void );
void             SmartMotorCurrentMonitorEnable( void );
void             SmartMotorCurrentMonitorDisable( void );
#define          SmartMotorSetLimitCurent(index, ... ) \
                 _SmartMotorSetLimitCurent( index, ##__VA_ARGS__, 1.0 )
void             _SmartMotorSetLimitCurent( tVexMotor index, float current, ... );
void             SmartMotorSetFreeRpm( tVexMotor index, short max_rpm );
void             SmartMotorSetSlewRate( tVexMotor index, int slew_rate );
void             SmartMotorRun( void );
void             SmartMotorStop( void );
void             SmartMotorSetControllerStatusLed( int index, tVexDigitalPin port );
void             SmartMotorSetPowerExpanderStatusPort( tVexAnalogPin port );
void             SmartMotorDebugStatus(void);
#define          SmartMotorSetRpmSensor( index, port, ticks_per_rev, ... ) \
                 _SmartMotorSetRpmSensor( index, port, ticks_per_rev, ##__VA_ARGS__, FALSE )
void             _SmartMotorSetRpmSensor( tVexMotor index, tVexAnalogPin port, float ticks_per_rev, bool_t revesed, ... );

#define          SetMotor( index, value, ... ) \
                 _SetMotor( index, value, ##__VA_ARGS__, FALSE )
void             _SetMotor( int index, int value,  bool_t immediate, ... );

// Access raw data
smartMotor      *SmartMotorGetPtr( tVexMotor index );
smartController *SmartMotorControllerGetPtr( short index );

// Private functions for reference
void             SmartMotorSpeed( smartMotor *m, int deltaTime );
void             SmartMotorSimulateSpeed( smartMotor *m );
float            SmartMotorCurrent( smartMotor *m, float v_battery  );
float            SmartMotorControllerCurrent( smartController *s );
int              SmartMotorSafeCommand( smartMotor *m, float v_battery  );
float            SmartMotorTemperature( smartMotor *m, int deltaTime );
float            SmartMotorControllerTemperature( smartController *s, int deltaTime  );
void             SmartMotorMonitorPtc( smartMotor *m, float v_battery );
void             SmartMotorControllerMonitorPtc( smartController *s, float v_battery );
void             SmartMotorMonitorCurrent( smartMotor *m, float v_battery );
void             SmartMotorControllerSetLed( smartController *s );
msg_t            SmartMotorTask( void *arg );
msg_t            SmartMotorSlewRateTask( void *arg );

#endif  // __SMARTMOTORLIB__
