/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexbkup.c                                                    */
/*    Author:     James Pearman                                                */
/*    Created:    1 Sept 2013                                                  */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     1 Sept 2013 - Initial release                      */
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
/** @file    vexbkup.c
  * @brief   Backup register access
*//*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/** @brief      Initialize access to backup registers                          */
/*-----------------------------------------------------------------------------*/

void
vexBkupInit()
{
    RCC_TypeDef *p = RCC;
    PWR_TypeDef *q = PWR;

    // Enable backup and power clocks
    p->APB1ENR = p->APB1ENR | (RCC_APB1ENR_BKPEN | RCC_APB1ENR_PWREN);
    // Disable write protection of backup registers
    q->CR = q->CR | PWR_CR_DBP;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Write backup register                                          */
/** @param[in]  index The index of the backup register( 0 to 41 )              */
/** @param[in]  data The data to write to the backup register                  */
/*-----------------------------------------------------------------------------*/

void
vexBkupRegisterWrite( int16_t index, uint16_t data )
{
    BKP_TypeDef *p = BKP;
    __IO uint16_t    *q;

    // index should be 0 through 41
    if((index >= 0) && (index < 10))
        {
        q = &p->DR1;
        q = q + (index<<1);
        //writeDebugStreamLine("addr is %08X", q );
        *q = data;
        }
    else
    if((index >= 10) && (index < 42))
        {
        q = &p->DR11;
        q = q + ((index-10)<<1);
        //writeDebugStreamLine("addr is %08X", q );
        *q = data;
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Read backup register at index and return data                  */
/** @param[in]  index The index of the backup register( 0 to 41 )              */
/** @returns    The value of the indexed backup register                       */
/*-----------------------------------------------------------------------------*/

uint16_t
vexBkupRegisterRead( int16_t index )
{
    BKP_TypeDef *p = BKP;
    __IO uint16_t    *q;

    // index should be 0 through 41
    if((index >= 0) && (index < 10))
        {
        q = &p->DR1;
        q = q + (index<<1);
        return(*q);
        }
    else
    if((index >= 10) && (index < 42))
        {
        q = &p->DR11;
        q = q + ((index-10)<<1);
        return(*q);
        }

    return(0);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Dump all backup registers to console for debug                 */
/** @param[in]  chp     A pointer to a vexStream object                        */
/** @param[in]  argc    The number of command line arguments                   */
/** @param[in]  argv    An array of pointers to the command line args          */
/*-----------------------------------------------------------------------------*/

void
vexBkupDebug(vexStream *chp, int argc, char *argv[])
{
    int i;

    (void)argc;
    (void)argv;

    vex_chprintf( chp, "Backup Registers\r\n" );
    vex_chprintf( chp, "00-15: " );
    for(i=0;i<16;i++)
        vex_chprintf( chp, "%02X ", vexBkupRegisterRead(i) );
    vex_chprintf(chp,"\r\n");
    vex_chprintf( chp, "16-32: " );
    for(i=16;i<32;i++)
        vex_chprintf( chp, "%02X ", vexBkupRegisterRead(i) );
    vex_chprintf(chp,"\r\n");
    vex_chprintf( chp, "32-41: " );
    for(i=32;i<42;i++)
        vex_chprintf( chp, "%02X ", vexBkupRegisterRead(i) );
    vex_chprintf(chp,"\r\n");
}
