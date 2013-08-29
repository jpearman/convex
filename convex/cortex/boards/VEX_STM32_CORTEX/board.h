/*-----------------------------------------------------------------------------*/
/*   ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio                    */
/*-----------------------------------------------------------------------------*/
/*   This file is a modification of a standard board.h file supplied with      */
/*   ChibiOS/RT.  The changes are Copyright (C) 2013 James Pearman             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     board.h                                                      */
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
/*-----------------------------------------------------------------------------*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for the VEX cortex STM32F103 based board
 */

/*
 * Board identifier.
 */
#define BOARD_VEX_STM32_CORTEX
#define BOARD_NAME              "VEX CORTEX"

/*
 * Board frequencies.
 */
#define STM32_LSECLK            32768
#define STM32_HSECLK            8000000

/*
 * MCU type, supported types are defined in ./os/hal/platforms/hal_lld.h.
 */
#define STM32F10X_HD

/*
 * IO pins assignments.
 */
#define GPIOA_ANALOG1           0
#define GPIOA_ANALOG2           1
#define GPIOA_ANALOG3           2
#define GPIOA_ANALOG4           3
#define GPIOC_ANALOG7           0
#define GPIOC_ANALOG8           1
#define GPIOC_ANALOG5           2
#define GPIOC_ANALOG6           3

#define GPIOE_DIGIO_1           9
#define GPIOE_DIGIO_2           11
#define GPIOC_DIGIO_3           6
#define GPIOC_DIGIO_4           7
#define GPIOE_DIGIO_5           13
#define GPIOE_DIGIO_6           14
#define GPIOE_DIGIO_7           8
#define GPIOE_DIGIO_8           10
#define GPIOE_DIGIO_9           12
#define GPIOE_DIGIO_10          7
#define GPIOD_DIGIO_11          0
#define GPIOD_DIGIO_12          1

#define	PORT_DIGIO_1			GPIOE
#define	PORT_DIGIO_2			GPIOE
#define	PORT_DIGIO_3			GPIOC
#define	PORT_DIGIO_4			GPIOC
#define	PORT_DIGIO_5			GPIOE
#define	PORT_DIGIO_6			GPIOE
#define	PORT_DIGIO_7			GPIOE
#define	PORT_DIGIO_8			GPIOE
#define	PORT_DIGIO_9			GPIOE
#define	PORT_DIGIO_10			GPIOE
#define	PORT_DIGIO_11			GPIOD
#define	PORT_DIGIO_12			GPIOD

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 *
 * The digits have the following meaning:
 *   0 - Analog input.
 *   1 - Push Pull output 10MHz.
 *   2 - Push Pull output 2MHz.
 *   3 - Push Pull output 50MHz.
 *   4 - Digital input.
 *   5 - Open Drain output 10MHz.
 *   6 - Open Drain output 2MHz.
 *   7 - Open Drain output 50MHz.
 *   8 - Digital input with PullUp or PullDown resistor depending on ODR.
 *   9 - Alternate Push Pull output 10MHz.
 *   A - Alternate Push Pull output 2MHz.
 *   B - Alternate Push Pull output 50MHz.
 *   C - Reserved.
 *   D - Alternate Open Drain output 10MHz.
 *   E - Alternate Open Drain output 2MHz.
 *   F - Alternate Open Drain output 50MHz.
 * Please refer to the STM32 Reference Manual for details.
 */

/*
 * Port A setup.
 * PA0  - Analog input 1
 * PA1  - Analog input 2
 * PA2  - Analog input 3
 * PA3  - Analog input 4
 * PA5  - Alternate Output - 50MHz - SPI CLK
 * PA6  - Alternate Output - 50MHz - SPI MISO
 * PA7  - Alternate Output - 50MHz - SPI MOSI
 * PA9  - Alternate Output - 50MHz - USART1 TX
 * PA10 - INPUT                    - USART1 RX
 * PA11 - Output - 50MHz           - SPI CSA
 */
#define VAL_GPIOACRL            0xBBB00000      /*  PA7...PA0 */
#define VAL_GPIOACRH            0x000034B0      /* PA15...PA8 */
#define VAL_GPIOAODR            0x00000000

/*
 * Port B setup.
 * PB8  - Alternate output - Open drain - 50 - I2C SDA
 * PB9  - Alternate output - Open drain - 50 - I2C SCL
 * PB10 - INPUT - RX2
 */
#define VAL_GPIOBCRL            0x00000000      /*  PB7...PB0 */
#define VAL_GPIOBCRH            0x000004FF      /* PB15...PB8 */
#define VAL_GPIOBODR            0x00000000

/*
 * Port C setup.
 * Everything input with pull-up except:
 * PC0  - Analog input 7
 * PC1  - Analog input 8
 * PC2  - Analog input 5
 * PC3  - Analog input 6
 * PC6  - Digitl input 3
 * PC7  - Digitl input 4
 * PC8  - INPUT - RX1
 * PC10 - Alternate Output - 50MHz - USART2 TX
 * PC11 - INPUT                    - USART2 RX
 */
#define VAL_GPIOCCRL            0x88000000      /*  PC7...PC0 */
#define VAL_GPIOCCRH            0x00004B04      /* PC15...PC8 */
#define VAL_GPIOCODR            0x000000C0

/*
 * Port D setup.
 * PD0  - Digital input 11
 * PD1  - Digital input 12
 * PD3  - Output - Push Pull - 10 Motor 0 Enable N
 * PD4  - Output - Push Pull - 10 Motor 0 Enable P
 * PD5  - Alternate Output - 50MHz - USART3 TX
 * PD6  - INPUT                    - USART3 RX
 * PD7  - Output - Push Pull - 10 Motor 9 Enable N
 * PD8  - Output - Push Pull - 10 Motor 9 Enable P
 * PD12 - Output - Push Pull - 10 Motor 9 PWM N
 * PD13 - Output - Push Pull - 10 Motor 9 PWM P
 * PD14 - Output - Push Pull - 10 Motor 0 PWM N
 * PD15 - Output - Push Pull - 10 Motor 0 PWM P
 */
#define VAL_GPIODCRL            0x14B11088      /*  PD7...PD0 */
#define VAL_GPIODCRH            0x11110001      /* PD15...PD8 */
#define VAL_GPIODODR            0x0000F003

/*
 * Port E setup.
 * PE1  - Output - 50MHz           - SPI CSB
 * PE7  - Digital input 10
 * PE8  - Digital input 7
 * PE9  - Digital input 1
 * PE10 - Digital input 8
 * PE11 - Digital input 2
 * PE12 - Digital input 9
 * PE13 - Digital input 5
 * PE14 - Digital input 6
 *
 */
#define VAL_GPIOECRL            0x80000003      /*  PE7...PE0 */
#define VAL_GPIOECRH            0x08888888      /* PE15...PE8 */
#define VAL_GPIOEODR            0x00007F81

/*
 * Port F setup.
 *
 */
#define VAL_GPIOFCRL            0x44444444      /*  PE7...PE0 */
#define VAL_GPIOFCRH            0x44444444      /* PE15...PE8 */
#define VAL_GPIOFODR            0x00000000

/*
 * Port G setup.
 *
 */
#define VAL_GPIOGCRL            0x44444444      /*  PE7...PE0 */
#define VAL_GPIOGCRH            0x44444444      /* PE15...PE8 */
#define VAL_GPIOGODR            0x00000000

/*
 * USB bus activation macro, required by the USB driver.
 */
#define usb_lld_connect_bus(usbp)

/*
 * USB bus de-activation macro, required by the USB driver.
 */
#define usb_lld_disconnect_bus(usbp)

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
