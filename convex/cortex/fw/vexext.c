/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexext.c                                                     */
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

#include <stdlib.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"

/*-----------------------------------------------------------------------------*/
/** @file    vexext.c
  * @brief   External interrupt glue code
*//*---------------------------------------------------------------------------*/

// External interrupt control was changed significantly between
// versions 2.4.3 and 2.6.0
#if (CH_KERNEL_MAJOR >= 2) && (CH_KERNEL_MINOR >= 6)
#define EXT_VERSION   1
#else
#define EXT_VERSION   0
#endif

// Old style EXTConfig
#if EXT_VERSION == 0
static  EXTConfig vexExtCfg = {
   {
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOD_DIGIO_11
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOD_DIGIO_12
   {EXT_CH_MODE_DISABLED, NULL},    //
   {EXT_CH_MODE_DISABLED, NULL},    //
   {EXT_CH_MODE_DISABLED, NULL},    //
   {EXT_CH_MODE_DISABLED, NULL},    //
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOC_DIGIO_3
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOC_DIGIO_4 or GPIOE_DIGIO_10
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOE_DIGIO_7
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOE_DIGIO_1
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOE_DIGIO_8
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOE_DIGIO_2
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOE_DIGIO_9
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOE_DIGIO_5
   {EXT_CH_MODE_DISABLED, NULL},    // GPIOE_DIGIO_6
   {EXT_CH_MODE_DISABLED, NULL},    //
   },
#ifndef  BOARD_OLIMEX_STM32_P103
   EXT_MODE_EXTI(EXT_MODE_GPIOD, EXT_MODE_GPIOD,  0,              0,
                 0,              0,               EXT_MODE_GPIOC, EXT_MODE_GPIOC,
                 EXT_MODE_GPIOE, EXT_MODE_GPIOE,  EXT_MODE_GPIOE, EXT_MODE_GPIOE,
                 EXT_MODE_GPIOE, EXT_MODE_GPIOE,  EXT_MODE_GPIOE, 0)
#else
   EXT_MODE_EXTI(EXT_MODE_GPIOB, EXT_MODE_GPIOB,  EXT_MODE_GPIOB, 0,
                 EXT_MODE_GPIOC, EXT_MODE_GPIOB,  0,              0,
                 EXT_MODE_GPIOB, EXT_MODE_GPIOB,  EXT_MODE_GPIOC, 0,
                 EXT_MODE_GPIOC, EXT_MODE_GPIOC,  EXT_MODE_GPIOB, EXT_MODE_GPIOB)
#endif
};

#else

// New style EXTConfig
#ifndef  BOARD_OLIMEX_STM32_P103
static  EXTConfig vexExtCfg = {
   {
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOD, NULL},   // GPIOD_DIGIO_11
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOD, NULL},   // GPIOD_DIGIO_12
   {EXT_CH_MODE_DISABLED                 , NULL},   //
   {EXT_CH_MODE_DISABLED                 , NULL},   //
   {EXT_CH_MODE_DISABLED                 , NULL},   //
   {EXT_CH_MODE_DISABLED                 , NULL},   //
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOC, NULL},   // GPIOC_DIGIO_3
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOC, NULL},   // GPIOC_DIGIO_4 or GPIOE_DIGIO_10
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOE, NULL},   // GPIOE_DIGIO_7
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOE, NULL},   // GPIOE_DIGIO_1
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOE, NULL},   // GPIOE_DIGIO_8
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOE, NULL},   // GPIOE_DIGIO_2
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOE, NULL},   // GPIOE_DIGIO_9
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOE, NULL},   // GPIOE_DIGIO_5
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOE, NULL},   // GPIOE_DIGIO_6
   {EXT_CH_MODE_DISABLED                 , NULL}    //
   }
};
#else
static  EXTConfig vexExtCfg = {
   {
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOB, NULL},   // 
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOB, NULL},   // 
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOB, NULL},   //
   {EXT_CH_MODE_DISABLED                 , NULL},   //
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOC, NULL},   //
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOB, NULL},   //
   {EXT_CH_MODE_DISABLED                 , NULL},   // 
   {EXT_CH_MODE_DISABLED                 , NULL},   //
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOB, NULL},   // 
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOB, NULL},   // 
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOC, NULL},   // 
   {EXT_CH_MODE_DISABLED                 , NULL},   // 
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOC, NULL},   //
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOC, NULL},   // 
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOB, NULL},   // 
   {EXT_CH_MODE_DISABLED | EXT_MODE_GPIOB, NULL}    //
   }
};
#endif
#endif  // EXT_VERSION

/*-----------------------------------------------------------------------------*/
/** @brief      Configure GPIO pin to be external interrupt source             */
/** @param[in]  port The digital port to configure                             */
/** @param[in]  channel The ext irq channel to configure                       */
/** @param[in]  mode The ext irq mode                                          */
/** @param[in]  cb A callback to be called when the irq fires                  */
/*-----------------------------------------------------------------------------*/

#if EXT_VERSION == 0
/*-----------------------------------------------------------------------------*/
/*  Version 2.43 ext interrupt setup                                           */
/*-----------------------------------------------------------------------------*/
void
vexExtSet( ioportid_t port, uint16_t channel, uint32_t mode, extcallback_t cb )
{
    vexExtCfg.channels[channel].mode = mode;
    vexExtCfg.channels[channel].cb   = cb;

    // Only on the real VEX board
#ifndef  BOARD_OLIMEX_STM32_P103
    // Fix the shared interrupt issue with D4 and D10
    if( channel == 7 )
        {
        if( port == PORT_DIGIO_10 )
            vexExtCfg.exti[1] = (EXT_MODE_GPIOC << 8) | (EXT_MODE_GPIOE << 12);
        if( port == PORT_DIGIO_4 )
            vexExtCfg.exti[1] = (EXT_MODE_GPIOC << 8) | (EXT_MODE_GPIOC << 12);
        }
#else
    (void)port;
#endif
}

#else

/*-----------------------------------------------------------------------------*/
/*  Version 2.60 and on changed the way ext interrupts were setup              */
/*-----------------------------------------------------------------------------*/
void
vexExtSet( ioportid_t port, uint16_t channel, uint32_t mode, extcallback_t cb )
{
    vexExtCfg.channels[channel].mode = (vexExtCfg.channels[channel].mode & EXT_MODE_GPIO_MASK) | mode;
    vexExtCfg.channels[channel].cb   = cb;

    // Only on the real VEX board
#ifndef  BOARD_OLIMEX_STM32_P103
    // Fix the shared interrupt issue with D4 and D10
    if( channel == 7 )
        {
        if( port == PORT_DIGIO_10 )
            vexExtCfg.channels[channel].mode = (vexExtCfg.channels[channel].mode & ~EXT_MODE_GPIO_MASK) | EXT_MODE_GPIOE;
        if( port == PORT_DIGIO_4 )
            vexExtCfg.channels[channel].mode = (vexExtCfg.channels[channel].mode & ~EXT_MODE_GPIO_MASK) | EXT_MODE_GPIOC;
        }
#else
    (void)port;
#endif
}

#endif

/*-----------------------------------------------------------------------------*/
/** @brief      Initialise external interrupts                                 */
/** @note       Call this after all setup is done                              */
/*-----------------------------------------------------------------------------*/

void
vexExtIrqInit()
{
    // Activates the EXT driver 1.
    extStart(&EXTD1, &vexExtCfg);
}

