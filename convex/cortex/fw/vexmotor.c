/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexmotor.c                                                   */
/*    Author:     James Pearman                                                */
/*    Created:    7 May 2013                                                   */
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

/*-----------------------------------------------------------------------------*/
/** @file    vexmotor.c
  * @brief   Motor port driver
*//*---------------------------------------------------------------------------*/

#include <stdlib.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

// local variable to store the timer used for pwm motors
// We tested this on a different board without access to timer4
static  TIM_TypeDef *PwmTimer = NULL;

// storage for motor data
static  vexMotor  vexMotors[kVexMotorNum];

// use only in interrupt
// 0 and 9 are really motors 1 and 10
// they are left at 0 and 9 for legacy reasons
static  int16_t   m0_cur_value = 0;
static  int16_t   m0_new_value = 0;
static  int16_t   m9_cur_value = 0;
static  int16_t   m9_new_value = 0;

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the motors                                          */
/*-----------------------------------------------------------------------------*/

void
vexMotorInit()
{
    int16_t     i;

    for(i=kVexMotor_1;i<kVexMotorNum;i++)
        {
        vexMotors[i].value = 0;
        vexMotors[i].type  = kVexMotorUndefined;
        vexMotors[i].reversed = FALSE;
        vexMotors[i].motorPositionGet = NULL;
        vexMotors[i].motorPositionSet = NULL;
        }

    // Initialize the two H-Bridge motor controllers
#ifdef  BOARD_OLIMEX_STM32_P103
    _vexMotorPwmInit( TIM3 );
#else
    _vexMotorPwmInit( TIM4 );
#endif

}

/*-----------------------------------------------------------------------------*/
/** @brief      Set motor to speed given by value                              */
/** @param[in]  index The motor index                                          */
/** @param[in]  value The speed of the motor (-127 to 127)                     */
/*-----------------------------------------------------------------------------*/

void
vexMotorSet( int16_t index, int16_t value )
{
    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
        return;

    // limit
    if( value > 127 )
        value = 127;
    else
    if( value < (-127))
        value = -127;

    // save limited value in array
    vexMotors[ index ].value = value;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the current commanded speed of a motor                     */
/** @param[in]  index The motor index                                          */
/** @returns    The speed of the indexed motor (-127 to 127)                   */
/*-----------------------------------------------------------------------------*/

int16_t
vexMotorGet( int16_t index )
{
    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
        return 0;

    return( vexMotors[ index ].value );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Stop all motors                                                */
/*-----------------------------------------------------------------------------*/

void
vexMotorStopAll()
{
    int16_t i;

    for(i=kVexMotor_1;i<kVexMotorNum;i++)
        vexMotorSet( i, 0);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Save motor type                                                */
/** @param[in]  index The motor index                                          */
/** @param[in]  type The motor type                                            */
/*-----------------------------------------------------------------------------*/

void
vexMotorTypeSet( int16_t index, tVexMotorType type )
{
    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
        return;

    vexMotors[ index ].type = type;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get motor type                                                 */
/** @param[in]  index The motor index                                          */
/** @returns    The motor type                                                 */
/*-----------------------------------------------------------------------------*/

tVexMotorType
vexMotorTypeGet( int16_t index )
{
    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
        return( kVexMotorUndefined );

    return( vexMotors[ index ].type );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Save motor direction (reversed) flag                           */
/** @param[in]  index The motor index                                          */
/** @param[in]  reversed Flag indicating if the motor is reversed              */
/*-----------------------------------------------------------------------------*/

void
vexMotorDirectionSet( int16_t index, bool_t reversed )
{
    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
        return;

    vexMotors[ index ].reversed = reversed;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get motor reversed flag                                        */
/** @param[in]  index The motor index                                          */
/** @returns    bool indicating if the motor is reversed                       */
/*-----------------------------------------------------------------------------*/

bool_t
vexMotorDirectionGet( int16_t index )
{
    if( (index < 0) || (index >= kVexMotorNum))
        return( FALSE );

    return( vexMotors[ index ].reversed );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the current motor position                                 */
/** @param[in]  index The motor index                                          */
/** @param[in]  value The new motor position                                   */
/*-----------------------------------------------------------------------------*/

void
vexMotorPositionSet( int16_t index, int32_t value )
{
    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
        return;

    if( vexMotors[ index ].motorPositionSet != NULL )
        {
        // 269 needs reversing
        if( vexMotors[ index ].type == kVexMotor269 )
            value = -value;

        // reverse flag ?
        if(!vexMotors[ index ].reversed )
            value = -value;

        // Set new position
        vexMotors[ index ].motorPositionSet( vexMotors[ index ].port, value );
        }
    else
        return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the current motor position                                 */
/** @param[in]  index The motor index                                          */
/** @returns    The motor position                                             */
/*-----------------------------------------------------------------------------*/

int32_t
vexMotorPositionGet( int16_t index )
{
    int32_t     position;

    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
        return(0);

    if( vexMotors[ index ].motorPositionGet != NULL )
        {
        position = vexMotors[ index ].motorPositionGet( vexMotors[ index ].port );

        // 269 needs reversing
        if( vexMotors[ index ].type == kVexMotor269 )
            position = -position;

        if(!vexMotors[ index ].reversed )
            return( position );
        else
            return( -position );
        }
    else
        return(0);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the callback used to get motor position                    */
/** @param[in]  index The motor index                                          */
/** @param[in]  cb function used to get motor position                         */
/** @param[in]  port A variable to send to the callback                        */
/*-----------------------------------------------------------------------------*/

void
vexMotorPositionGetCallback( int16_t index, int32_t (*cb)(int16_t), int16_t port )
{
    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
        return;

    vexMotors[ index ].motorPositionGet = cb;
    vexMotors[ index ].port = port;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the callback used to set motor position                    */
/** @param[in]  index The motor index                                          */
/** @param[in]  cb function used to get motor position                         */
/** @param[in]  port A variable to send to the callback                        */
/*-----------------------------------------------------------------------------*/

void
vexMotorPositionSetCallback( int16_t index, void (*cb)(int16_t, int32_t), int16_t port )
{
    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
        return;

    vexMotors[ index ].motorPositionSet = cb;
    vexMotors[ index ].port = port;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the callback used to get motor position                    */
/** @param[in]  index The motor index                                          */
/** @param[in]  cb function used to get motor encoder id                       */
/** @param[in]  port A variable to send to the callback                        */
/*-----------------------------------------------------------------------------*/

void
vexMotorEncoderIdCallback( int16_t index, int16_t (*cb)(int16_t), int16_t port )
{
    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
        return;

    vexMotors[ index ].getEncoderId = cb;
    vexMotors[ index ].port = port;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the motor encoder id                                       */
/** @param[in]  index The motor index                                          */
/** @returns    The motor encoder id                                           */
/** @note       The encoder id is used to differentiate between encoder types  */
/*-----------------------------------------------------------------------------*/

int16_t
vexMotorEncoderIdGet( int16_t index )
{
    if( (index < kVexMotor_1) || (index >= kVexMotorNum))
         return(-1);

    if( vexMotors[ index ].getEncoderId != NULL )
        {
        return( vexMotors[ index ].getEncoderId( vexMotors[ index ].port ) );
        }
    else
        return(-1);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Command line debug of motors                                   */
/** @param[in]  chp     A pointer to a vexStream object                        */
/** @param[in]  argc    The number of command line arguments                   */
/** @param[in]  argv    An array of pointers to the command line args          */
/*-----------------------------------------------------------------------------*/

void
vexMotorDebug(vexStream *chp, int argc, char *argv[])
{
    int16_t index;
    int16_t data;

    if (argc < 2)
        {
        // Status
        vex_chprintf(chp, "Motor  Speed  Position Rev   ID\r\n");
        for(index=0;index<kVexMotorNum;index++)
            {
            vex_chprintf(chp, "M_%d  %4d %7d      ", index, vexMotors[ index ].value, vexMotorPositionGet( index ) );
            if( vexMotors[ index ].reversed )
                vex_chprintf(chp, "true  ");
            else
                vex_chprintf(chp, "false ");
            vex_chprintf(chp,"%d\r\n", vexMotorEncoderIdGet(index));
            }
        }
    else
        {
        index = atoi( argv[0] );
        data  = atoi( argv[1] );
        if( index >= 0 ) {
            vex_chprintf(chp, "set motor %d to %d\r\n", index, data );

            vexMotorSet( index, data );
            }
        }
}


/*-----------------------------------------------------------------------------*/
/** @private                                                                   */
/** @brief      Motor interrupt handler                                        */
/** @note       Internal motor driver use only                                 */
/*-----------------------------------------------------------------------------*/

#ifdef  BOARD_OLIMEX_STM32_P103
CH_IRQ_HANDLER(TIM3_IRQHandler) {
#else
CH_IRQ_HANDLER(TIM4_IRQHandler) {
#endif

    CH_IRQ_PROLOGUE();

    // clear interrupt
    PwmTimer->SR = 0;

    chSysLockFromIsr();

    // check motor 0
    if(!vexMotors[kVexMotor_1].reversed)
        m0_new_value = vexMotors[kVexMotor_1].value;
    else
        m0_new_value = -vexMotors[kVexMotor_1].value;

    if( m0_cur_value != m0_new_value )
        {
        // new value is 0 then just set
        if( m0_new_value == 0 ) {
            _vexMotorPwmSet_0( 0 );
            m0_cur_value = 0;
            }
        else
        // new value is > 0, test if we had < 0 before and transition through 0 if so
        if( m0_new_value > 0 ) {
            if( m0_cur_value >= 0 ) {
                _vexMotorPwmSet_0( m0_new_value );
                m0_cur_value = m0_new_value;
                }
            else {
                // transition via 0
                _vexMotorPwmSet_0( 0 );
                m0_cur_value = 0;
                }
            }
        // new value is < 0, test if we had > 0 before and transition through 0 if so
        else {
            if( m0_cur_value <= 0 ) {
                _vexMotorPwmSet_0( m0_new_value );
                m0_cur_value = m0_new_value;
                }
            else {
                // transition via 0
                _vexMotorPwmSet_0( 0 );
                m0_cur_value = 0;
                }
            }
        }
     // check motor 9
     if(!vexMotors[kVexMotor_10].reversed)
         m9_new_value = vexMotors[kVexMotor_10].value;
     else
         m9_new_value = -vexMotors[kVexMotor_10].value;
     if( m9_cur_value != m9_new_value )
        {
        // new value is 0 then just set
        if( m9_new_value == 0 ) {
            _vexMotorPwmSet_9( 0 );
            m9_cur_value = 0;
            }
        else
        // new value is > 0, test if we had < 0 before and transition through 0 if so
        if( m9_new_value > 0 ) {
            if( m9_cur_value >= 0 ) {
                _vexMotorPwmSet_9( m9_new_value );
                m9_cur_value = m9_new_value;
                }
            else {
                // transition via 0
                _vexMotorPwmSet_9( 0 );
                m9_cur_value = 0;
                }
            }
        // new value is < 0, test if we had > 0 before and transition through 0 if so
        else {
            if( m9_cur_value <= 0 ) {
                _vexMotorPwmSet_9( m9_new_value );
                m9_cur_value = m9_new_value;
                }
            else {
                // transition via 0
                _vexMotorPwmSet_9( 0 );
                m9_cur_value = 0;
                }
            }
        }

    chSysUnlockFromIsr();

    CH_IRQ_EPILOGUE();
}

/*-----------------------------------------------------------------------------*/
/** @private                                                                   */
/** @brief      Initialize the pwm motors @ 1200Hz                             */
/** @param[in]  tim A pointer to a STD32 timer structure                       */
/*-----------------------------------------------------------------------------*/

void
_vexMotorPwmInit( TIM_TypeDef *tim )
{
    // Use HAL for clock enable
    // Cortex uses TIM4, debug uses TIM3
    if( tim == TIM3 )
        {
        AFIO->MAPR |= (AFIO_MAPR_TIM3_REMAP_1 | AFIO_MAPR_TIM3_REMAP_0);
        rccEnableTIM3(FALSE);
        rccResetTIM3();
        nvicEnableVector(TIM3_IRQn, CORTEX_PRIORITY_MASK(STM32_PWM_TIM3_IRQ_PRIORITY));
        }
    else
    if( tim == TIM4 )
        {
        AFIO->MAPR |= (AFIO_MAPR_TIM4_REMAP);
        rccEnableTIM4(FALSE);
        rccResetTIM4();
        nvicEnableVector(TIM4_IRQn, CORTEX_PRIORITY_MASK(STM32_PWM_TIM4_IRQ_PRIORITY));
        }
    else
        return;

    // Store timer
    PwmTimer = tim;

    // Disable
    tim->CR1  = 0;      // Timer disabled.
    tim->DIER = 0;      // All IRQs disabled.
    tim->SR   = 0;      // Clear eventual pending IRQs.

    tim->PSC  = 0x1D8;
    tim->ARR  = 0x7E;   // Interval
    tim->CR2  = 0;

    tim->CCMR1 = TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 |
                 TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
    tim->CCMR2 = TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 |
                 TIM_CCMR2_OC4PE | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;

    tim->CCER  = TIM_CCER_CC1P | TIM_CCER_CC2P | TIM_CCER_CC3P | TIM_CCER_CC4P ;
    tim->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E ;

    // Causes all high
    tim->CCR1  = 0;
    tim->CCR2  = 0;
    tim->CCR3  = 0;
    tim->CCR4  = 0;

    tim->CR1   = TIM_CR1_ARPE | TIM_CR1_CEN;

    // Set enable pins low
    palClearPad( VEX_PWM_PORT, VEX_EBL_T0_N_PIN );
    palClearPad( VEX_PWM_PORT, VEX_EBL_T0_P_PIN );
    palClearPad( VEX_PWM_PORT, VEX_EBL_T9_N_PIN );
    palClearPad( VEX_PWM_PORT, VEX_EBL_T9_P_PIN );
    palSetPadMode( VEX_PWM_PORT, VEX_EBL_T0_N_PIN, PAL_MODE_OUTPUT_PUSHPULL );
    palSetPadMode( VEX_PWM_PORT, VEX_EBL_T0_P_PIN, PAL_MODE_OUTPUT_PUSHPULL );
    palSetPadMode( VEX_PWM_PORT, VEX_EBL_T9_N_PIN, PAL_MODE_OUTPUT_PUSHPULL );
    palSetPadMode( VEX_PWM_PORT, VEX_EBL_T9_P_PIN, PAL_MODE_OUTPUT_PUSHPULL );

    // Switch over output to PWM - should already be high
    palSetPadMode( VEX_PWM_PORT, VEX_PWM_T0_N_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
    palSetPadMode( VEX_PWM_PORT, VEX_PWM_T0_P_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
    palSetPadMode( VEX_PWM_PORT, VEX_PWM_T9_N_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
    palSetPadMode( VEX_PWM_PORT, VEX_PWM_T9_P_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL );

    // enable ints
    //tim->EGR  = TIM_EGR_UG;
    tim->DIER = TIM_DIER_UIE;
    tim->SR   = 0;

}

/*-----------------------------------------------------------------------------*/
/** @private                                                                   */
/** @brief      Control motor on port 0                                        */
/** @param[in]  value The command value for the motor on port 0                */
/** @warning    Do not call this function directly                             */
/** @warning    Do not change from positive values to negative values directly */
/** @note       Internal motor driver use only                                 */
/*-----------------------------------------------------------------------------*/

void
_vexMotorPwmSet_0( int16_t value )
{
    int16_t x;

    if( PwmTimer == NULL )
        return;

    if( value > 0 )
        {
        PwmTimer->CCR2 = 0;
        palClearPad( VEX_PWM_PORT, VEX_EBL_T0_N_PIN );
        x = value & 0x7F;
        palSetPad( VEX_PWM_PORT, VEX_EBL_T0_P_PIN );
        PwmTimer->CCR1 = x;
        }
    else
    if( value < 0 )
        {
        PwmTimer->CCR1 = 0;
        palClearPad( VEX_PWM_PORT, VEX_EBL_T0_P_PIN );
        x = (-value) & 0x7F;
        palSetPad( VEX_PWM_PORT, VEX_EBL_T0_N_PIN );
        PwmTimer->CCR2 = x;
        }
    else
        {
        PwmTimer->CCR1 = 0;
        PwmTimer->CCR2 = 0;
        palClearPad( VEX_PWM_PORT, VEX_EBL_T0_N_PIN );
        palClearPad( VEX_PWM_PORT, VEX_EBL_T0_P_PIN );
        }
}

/*-----------------------------------------------------------------------------*/
/** @private                                                                   */
/** @brief      Control motor on port 9                                        */
/** @param[in]  value The command value for the motor on port 9                */
/** @warning    Do not call this function directly                             */
/** @warning    Do not change from positive values to negative values directly */
/** @note       Internal motor driver use only                                 */
/*-----------------------------------------------------------------------------*/

void
_vexMotorPwmSet_9( int16_t value )
{
    int16_t x;

    if( PwmTimer == NULL )
        return;

    if( value > 0 )
        {
        PwmTimer->CCR4 = 0;
        palClearPad( VEX_PWM_PORT, VEX_EBL_T9_N_PIN );
        x = value & 0x7F;
        palSetPad( VEX_PWM_PORT, VEX_EBL_T9_P_PIN );
        PwmTimer->CCR3 = x;
        }
    else
    if( value < 0 )
        {
        PwmTimer->CCR3 = 0;
        palClearPad( VEX_PWM_PORT, VEX_EBL_T9_P_PIN );
        x = (-value) & 0x7F;
        palSetPad( VEX_PWM_PORT, VEX_EBL_T9_N_PIN );
        PwmTimer->CCR4 = x;
        }
    else
        {
        PwmTimer->CCR3 = 0;
        PwmTimer->CCR4 = 0;
        palClearPad( VEX_PWM_PORT, VEX_EBL_T9_N_PIN );
        palClearPad( VEX_PWM_PORT, VEX_EBL_T9_P_PIN );
        }
}

