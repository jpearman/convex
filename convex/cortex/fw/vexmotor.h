/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexmotor.h                                                   */
/*    Author:     James Pearman                                                */
/*    Created:    14 May 2013                                                  */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     4 July 2013 - Initial release                      */
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
/*                                                                             */
/*    Description:                                                             */
/*                                                                             */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */

#ifndef  _VEXMOTOR
#define  _VEXMOTOR

/*-----------------------------------------------------------------------------*/
/** @file    vexmotor.h
  * @brief   Motor port driver, macros and prototypes
*//*---------------------------------------------------------------------------*/

#ifdef   __cplusplus
extern "C" {
#endif

/** @cond */
#ifdef  BOARD_OLIMEX_STM32_P103
// Debugging on the eval card
#define VEX_PWM_PORT        GPIOC

#define VEX_EBL_T0_N_PIN    0
#define VEX_EBL_T0_P_PIN    1
#define VEX_EBL_T9_N_PIN    2
#define VEX_EBL_T9_P_PIN    3

#define VEX_PWM_T0_N_PIN    6
#define VEX_PWM_T0_P_PIN    7
#define VEX_PWM_T9_N_PIN    8
#define VEX_PWM_T9_P_PIN    9

#else
// On the real cortex
#define VEX_PWM_PORT        GPIOD

#define VEX_EBL_T0_N_PIN    3
#define VEX_EBL_T0_P_PIN    4
#define VEX_EBL_T9_N_PIN    7
#define VEX_EBL_T9_P_PIN    8

#define VEX_PWM_T0_N_PIN    12
#define VEX_PWM_T0_P_PIN    13
#define VEX_PWM_T9_N_PIN    14
#define VEX_PWM_T9_P_PIN    15
#endif
/** @endcond */


/*-----------------------------------------------------------------------------*/
/** @brief      Motor types                                                    */
/*-----------------------------------------------------------------------------*/
typedef enum {
    kVexMotorUndefined = 0,
    kVexMotor269,
    kVexMotor393T,
    kVexMotor393S,
    kVexMotor393R
} tVexMotorType;

/*-----------------------------------------------------------------------------*/
/** @brief      Motor port numbers                                             */
/*-----------------------------------------------------------------------------*/
typedef enum {
    kVexMotor_None = -1,
    kVexMotor_1  = 0,
    kVexMotor_2  = 1,
    kVexMotor_3  = 2,
    kVexMotor_4  = 3,
    kVexMotor_5  = 4,
    kVexMotor_6  = 5,
    kVexMotor_7  = 6,
    kVexMotor_8  = 7,
    kVexMotor_9  = 8,
    kVexMotor_10 = 9,

    kVexMotorNum
} tVexMotor;


#define kVexMotorNormal     FALSE       ///< Motor command causes normal movement
#define kVexMotorReversed   TRUE        ///< Motor command causes reversed movement

/*-----------------------------------------------------------------------------*/
/** @brief      Holds data for a motor                                         */
/*-----------------------------------------------------------------------------*/
typedef struct _vexMotor {
    volatile int16_t    value;
    tVexMotorType       type;
    bool_t              reversed;
    int32_t            (*motorPositionGet)( int16_t port );
    void               (*motorPositionSet)( int16_t port, int32_t value );
    int16_t            (*getEncoderId)( int16_t port );
    int16_t             port;
    } vexMotor;

/*-----------------------------------------------------------------------------*/

void            vexMotorInit(void);
void            vexMotorSet( int16_t index, int16_t value );
int16_t         vexMotorGet( int16_t index );
void            vexMotorStopAll(void);

void            vexMotorTypeSet( int16_t index, tVexMotorType type );
tVexMotorType   vexMotorTypeGet( int16_t index );
void            vexMotorDirectionSet( int16_t index, bool_t reversed );
bool_t          vexMotorDirectionGet( int16_t index );
void            vexMotorDebug(vexStream *chp, int argc, char *argv[]);
void            vexMotorPositionSet( int16_t index, int32_t value );
int32_t         vexMotorPositionGet( int16_t index );
void            vexMotorPositionGetCallback( int16_t index, int32_t (*cb)(int16_t), int16_t port );
void            vexMotorPositionSetCallback( int16_t index, void    (*cb)(int16_t, int32_t), int16_t port );
void            vexMotorEncoderIdCallback( int16_t index, int16_t (*cb)(int16_t), int16_t port );
int16_t         vexMotorEncoderIdGet( int16_t index );

// do not call these
/** @private                                                                   */
void            _vexMotorPwmInit( TIM_TypeDef *tim );
/** @private                                                                   */
void            _vexMotorPwmSet_0( int16_t value );
/** @private                                                                   */
void            _vexMotorPwmSet_9( int16_t value );

#ifdef   __cplusplus
}  // Close scope of extern "C" declaration
#endif

#endif  /* _VEXMOTOR */

