/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexflash.c                                                   */
/*    Author:     James Pearman                                                */
/*    Created:    3 Sept 2013                                                  */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     3 Sept 2013 - Initial release                      */
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
#include "vexflash.h"

/*-----------------------------------------------------------------------------*/
/** @file    vexflash.c
  * @brief   Store user parameters in Flash
*//*---------------------------------------------------------------------------*/

// page 190 at present
#define USER_PARAM_PAGE_ADDR     0x0805F000
#define USER_PARAM_INDEX         64
#define USER_PARAM_MAX_WRITE     32

// FLASH Keys
#define RDP_Key             ((uint16_t)0x00A5)
#define FLASH_KEY1          ((uint32_t)0x45670123)
#define FLASH_KEY2          ((uint32_t)0xCDEF89AB)


// local storage for user parameters
static  user_param  params;

/*-----------------------------------------------------------------------------*/
/** @brief      Get the offset into the user parameter block                   */
/** @returns    The offset (0 to 55) or -1 indicating no parameters            */
/*-----------------------------------------------------------------------------*/

static int16_t
vexFlashUserParamGetOffset(void)
{
    uint32_t     *p;
    int16_t     offset = -1;
    int16_t     i;

    // the first 64 words (256 bytes) are used as an index
    // calculate user parameter offset
    p = (uint32_t *)USER_PARAM_PAGE_ADDR;

    // look for non zero word in index
    for(i=0;i<USER_PARAM_INDEX;i++)
        {
        if(*p++ != 0xFFFFFFFF)
            offset++;
        else
            break;
        }

    return(offset);
}


/*-----------------------------------------------------------------------------*/
/** @brief      Read user parameters                                           */
/** @returns    A pointer to a user_param structure                            */
/*-----------------------------------------------------------------------------*/

user_param *
vexFlashUserParamRead()
{
    uint32_t    *p = (uint32_t *)USER_PARAM_PAGE_ADDR;
    uint32_t    *q = (uint32_t *)params.data;
    uint16_t     i;

    params.offset = vexFlashUserParamGetOffset();

    if(params.offset == (-1))
        {
        // no user parameters
        for(i=0;i<USER_PARAM_WORDS;i++)
            *q++ = 0xFFFFFFFF;

        // error
        params.addr = (void *)0;
        }
    else
        {
        // Set address ptr
        p = (uint32_t *)(USER_PARAM_PAGE_ADDR  + ((USER_PARAM_INDEX + (params.offset * USER_PARAM_WORDS)) * sizeof(uint32_t)));

        // save address
        params.addr = p;

        // Now read params stored at offset
        for(i=0;i<USER_PARAM_WORDS;i++)
            *q++ = *p++;
        }

    return( &params );
}

/*-----------------------------------------------------------------------------*/
/** @brief      write user parameters                                          */
/** @param[in]  u Pointer to user_param structure                              */
/** @returns    status or error code                                           */
/*-----------------------------------------------------------------------------*/

int16_t
vexFlashUserParamWrite( user_param *u )
{
    uint32_t     p = USER_PARAM_PAGE_ADDR;
    uint32_t    *q;
    uint16_t     i;
    int16_t      ret = FLASH_SUCCESS;

    // limit number of writes per run
    static  int16_t     user_param_write_limit = 0;

    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;


    // check for NULL pointer
    if( u == NULL )
        return( FLASH_ERROR );

    // check write limit
    if( user_param_write_limit >= USER_PARAM_MAX_WRITE )
        return(FLASH_ERROR_WRITE_LIMIT);

    // one more write
    user_param_write_limit++;

    // Unlock the Flash Bank1 Program Erase controller
    FLASH_UnlockBank1();

    // Clear All pending flags
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    // Get current param offset
    u->offset = vexFlashUserParamGetOffset();

    // Next block
    u->offset++;

    // did we fill the page ?
    if( u->offset >= ((512 - USER_PARAM_INDEX) / USER_PARAM_WORDS) )
        {
        // Do erase here
        FLASHStatus = FLASH_ErasePage(USER_PARAM_PAGE_ADDR);

        // check for error
        if( FLASHStatus != FLASH_COMPLETE )
            return(FLASH_ERROR_ERASE);

        // start over
        u->offset = 0;
        }

    // start of area to write params
    p = USER_PARAM_PAGE_ADDR  + ((USER_PARAM_INDEX + (u->offset * USER_PARAM_WORDS)) * sizeof( uint32_t));

    // Save address for debug
    u->addr = (void *)p;

    // Write data
    for(i=0, q=(uint32_t *)u->data;i<USER_PARAM_WORDS;i++)
        {
        // program one word (4 bytes)
        FLASHStatus = FLASH_ProgramWord( p, *q++ );

        // Next word
        p += 4;

        // check for error
        if( FLASHStatus != FLASH_COMPLETE )
            {
            ret = FLASH_ERROR_WRITE;
            break;
            }
        }

    // Update data at offset
    p = USER_PARAM_PAGE_ADDR  + (u->offset * sizeof( uint32_t));
    FLASHStatus = FLASH_ProgramWord( p, 0);

    // check for error
    if( FLASHStatus != FLASH_COMPLETE )
        ret = (FLASH_ERROR_WRITE);

    return( ret );
}

/*-----------------------------------------------------------------------------*/
/** @brief  Init user parameters                                               */
/*@ Returns status or error code                                               */
/*-----------------------------------------------------------------------------*/

int16_t
vexFlashUserParamInit()
{
    static  int16_t erase_done = 0;
    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;

    if( !erase_done )
        {
        // only allow one init per run
        erase_done = 1;

        // Unlock the Flash Bank1 Program Erase controller
        FLASH_UnlockBank1();

        // Erase user parameters
        FLASHStatus = FLASH_ErasePage(USER_PARAM_PAGE_ADDR);

        // check for error
        if( FLASHStatus != FLASH_COMPLETE )
            return(FLASH_ERROR_ERASE);
        }
    else
        {
        return(FLASH_ERROR_ERASE_LIMIT);
        }

    return(FLASH_SUCCESS);
}
