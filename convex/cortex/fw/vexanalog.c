/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexanalog.c                                                  */
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

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexanalog.c
  * @brief   Analog port driver
*//*---------------------------------------------------------------------------*/

// ADCConfig structure for stm32 MCUs is empty
static ADCConfig adccfg = {0};

// Create buffer to store ADC results
#define ADC_CH_NUM      8
static adcsample_t samples_buf[ ADC_CH_NUM ];

// Fill ADCConversionGroup structure fields
static ADCConversionGroup adccg = {
      // this 3 fields are common for all MCUs
      // set to TRUE if need circular buffer, set FALSE otherwise
      TRUE,
      // number of channels
      (uint16_t)(ADC_CH_NUM),
      // callback function, set to NULL for begin
      NULL,
      // error callback
      NULL,

      // Resent fields are stm32 specific. They contain ADC control registers data.
      // Please, refer to ST manual RM0008.pdf to understand what we do.
      // CR1 register content, set to zero for begin
      0,
      // CR2 register content, set to zero for begin
      0,
      // SMRP1 register content, channels 10, 11, 12, 13 to 71.5 cycles
      0xB6D,
      // SMRP2 register content, channels 0, 1, 2, 3 to 71.5 cycles
      0xB6D,
      // SQR1 register content. Set channel sequence length
      ADC_SQR1_NUM_CH(ADC_CH_NUM),
      // SQR2 register content, set to zero for begin
      (ADC_SQR2_SQ7_N(ADC_CHANNEL_IN10) |
       ADC_SQR2_SQ8_N(ADC_CHANNEL_IN11)),
      // SQR3 register content. First 6 channels
      (ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0 ) |
       ADC_SQR3_SQ2_N(ADC_CHANNEL_IN1 ) |
       ADC_SQR3_SQ3_N(ADC_CHANNEL_IN2 ) |
       ADC_SQR3_SQ4_N(ADC_CHANNEL_IN3 ) |
       ADC_SQR3_SQ5_N(ADC_CHANNEL_IN12) |
       ADC_SQR3_SQ6_N(ADC_CHANNEL_IN13)),
};

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the ADC1 sub system                                 */
/*-----------------------------------------------------------------------------*/

void
vexAdcInit()
{
    // Init and start ADC1
    adcInit();
    adcStart(&ADCD1, &adccfg);

    // Start conversions
    adcStartConversion(&ADCD1, &adccg, &samples_buf[0], 1);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Return ADC sample from sample buffer                           */
/** @param[in]  index The index of the adc channel (0 through 7)               */
/** @return     The ADC value in the range 0 to 4095                           */
/*-----------------------------------------------------------------------------*/

int16_t
vexAdcGet( int16_t index )
{
    if( (index < 0) || (index > 7))
        return(-1);
    else
        return( samples_buf[ index ] );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Dump all analog values to console for debug                    */
/** @param[in]  chp     A pointer to a vexStream object                        */
/** @param[in]  argc    The number of command line arguments                   */
/** @param[in]  argv    An array of pointers to the command line args          */
/*-----------------------------------------------------------------------------*/

void
vexAdcDebug(vexStream *chp, int argc, char *argv[])
{
    int i;

    (void)argc;
    (void)argv;

    for(i=0;i<8;i++)
        vex_chprintf( chp, "channel %d = %4d\r\n", i, vexAdcGet(i) );
}

