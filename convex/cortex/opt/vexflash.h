/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexflash.h                                                   */
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

#ifndef VEXFLASH_H_
#define VEXFLASH_H_

/*-----------------------------------------------------------------------------*/
/** @file    vexflash.h
  * @brief   Store user parameters in Flash macros and prototypes
*//*---------------------------------------------------------------------------*/

#include "stm32f10x_flash.h"

// For user functions, these are not in the stm32 library
#define FLASH_SUCCESS               1
#define FLASH_ERROR_WRITE         (-1)
#define FLASH_ERROR_WRITE_LIMIT   (-2)
#define FLASH_ERROR_ERASE         (-3)
#define FLASH_ERROR_ERASE_LIMIT   (-4)
#define FLASH_ERROR               (-5)

// Number of user parameter words
// Do not change !!
#define USER_PARAM_WORDS    8

// Structure to hold user parameters
typedef struct _user_param {
    // storage for the NV data
    unsigned char data[USER_PARAM_WORDS * 4];

    // useful debug data
             int  offset;
    void          *addr;
    } user_param;


#ifdef __cplusplus
extern "C" {
#endif

// public functions
user_param *vexFlashUserParamRead( void );
int16_t     vexFlashUserParamWrite( user_param *u );
int16_t     vexFlashUserParamInit( void );

#ifdef __cplusplus
}
#endif

#endif // VEXFLASH_H_
