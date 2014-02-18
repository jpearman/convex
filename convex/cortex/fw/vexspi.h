/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexspi.h                                                     */
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

#ifndef __VEXSPI__
#define __VEXSPI__

/*-----------------------------------------------------------------------------*/
/** @file    vexspi.h
  * @brief   SPI communication to the master processor, macros and prototypes
*//*---------------------------------------------------------------------------*/

/** @cond */
#ifdef  BOARD_OLIMEX_STM32_P103
// On the eval card
#define VEX_SPI_ENABLE_PORT GPIOA
#define VEX_SPI_ENABLE_PIN  8
#define VEX_SPI_CS_PORT     GPIOA
#define VEX_SPI_CS_PIN      4

#else
// On the real cortex
#define VEX_SPI_ENABLE_PORT GPIOA
#define VEX_SPI_ENABLE_PIN  11
#define VEX_SPI_CS_PORT     GPIOE
#define VEX_SPI_CS_PIN      0

#endif
/** @endcond */

/*-----------------------------------------------------------------------------*/
/** @brief      default team name                                              */
/*-----------------------------------------------------------------------------*/
#if !defined(CONVEX_TEAM_NAME)
#define CONVEX_TEAM_NAME    "CONVEX  ";
#endif

/*-----------------------------------------------------------------------------*/
/** @brief      joystick data structure                                        */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Format of the joystick component of the SPI receive packet
 *  The reserved bytes are really used for other purposes
 *  but we keep them here in V1.00
 */
typedef struct _jsdata {
        char    Ch1;                ///< Analog control channel 1
        char    Ch2;                ///< Analog control channel 2
        char    Ch3;                ///< Analog control channel 3
        char    Ch4;                ///< Analog control channel 4
        char    acc_y;              ///< Accelerometer y data
        char    acc_x;              ///< Accelerometer x data
        char    acc_z;              ///< Accelerometer z data - not used
        unsigned char btns[2];      ///< data for the 12 buttons
        unsigned char res[2];       ///< reserved
        } jsdata;

/*-----------------------------------------------------------------------------*/
/** @brief      SPI transmit data packet                                       */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Format of a SPI transmit packet - 16 words
 */
typedef union _spiTxPacket {
    struct  _spiTxPak {
        unsigned char h1;           ///< Header byte one
        unsigned char h2;           ///< Header byte two
        unsigned char state;        ///< Control state
        unsigned char reserved[2];  ///< unknown data
        unsigned char type;         ///< data type
        unsigned char motor[8];     ///< motor pwm data
        unsigned char pad[15];      ///< not used
        unsigned char rev_lsb;      ///< revision of user code lsb
        unsigned char rev_msb;      ///< revision of user code msb
        unsigned char id;           ///< message id
    } pak;                          ///< access spiTxPacket as named variables

    unsigned char   data[32];       ///< access spiTxPacket as an array of char
} spiTxPacket;

/*-----------------------------------------------------------------------------*/
/** @brief      SPI receive data packet                                        */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Format of a SPI receive packet - 16 words
 */
typedef union _spiRxPacket {
    struct _spiRxPak {
        unsigned char h1;           ///< Header byte one
        unsigned char h2;           ///< Header byte two
        unsigned char status;       ///< status
        unsigned char ctl;          ///< status and control byte
        unsigned char batt1;        ///< main battery level, 59mV per bit
        unsigned char batt2;        ///< backup battery level, 59mV per bit
        jsdata        js_1;         ///< data for main joystick
        unsigned char pad;          ///< 1 byte padding
        jsdata        js_2;         ///< data for partner joystick
        unsigned char rev_lsb;      ///< revision of master code lsb
        unsigned char rev_msb;      ///< revision of master code msb
        unsigned char id;           ///< message id
    } pak;                          ///< access spiRxPacke as named variables

    unsigned char   data[32];       ///< access spiRxPacket as an array of char
} spiRxPacket;

/*-----------------------------------------------------------------------------*/
/** @brief      SPI data                                                       */
/*-----------------------------------------------------------------------------*/
/** @details
 *  All SPI related data collected in this structure
 */
typedef struct _SpiData {
    spiTxPacket txdata;             ///< tx data packet
    spiRxPacket rxdata;             ///< valid rx data packet
    spiRxPacket rxdata_t;           ///< receive data packet, may have errors
    uint16_t    online;             ///< online status
    uint32_t    errors;             ///< number of packets received with error
} SpiData;


#ifdef __cplusplus
extern "C" {
#endif

#define     vexTeamnameSet(name)    vexSpiTeamnameSet( name )

void        vexSpiInit(void);
void        vexSpiTeamnameSet( char *name );
void        vexSpiModeStandalone(void);
short       vexSpiGetOnlineStatus(void);
void        vexSpiSetMotor( int16_t index, int16_t data, bool_t reversed );
void        vexSpiSend(void);
void        vexSpiTickDelay( int16_t tick);
jsdata     *vexSpiGetJoystickDataPtr( int16_t index );
uint16_t    vexSpiGetControl(void);
uint16_t    vexSpiGetMainBattery(void);
uint16_t    vexSpiGetBackupBattery(void);

void        vexSpiDebug(vexStream *chp, int argc, char *argv[]);


#ifdef __cplusplus
}
#endif

#endif  // __VEXSPI__
