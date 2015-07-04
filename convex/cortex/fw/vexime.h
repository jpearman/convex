/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexime.h                                                     */
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
/*    This IME version for the ChibiOS/RT implementation on the cortex         */
/*    14 May 2013 was adapted from the code I wrote for the vexpro in Sept     */
/*    2012                                                                     */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */

#ifndef  _VEXIME
#define  _VEXIME

/*-----------------------------------------------------------------------------*/
/** @file    vexime.h
  * @brief   IME driver macros and prototypes
*//*---------------------------------------------------------------------------*/

#ifdef   __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------*/
/** @brief      IME channel definitions                                        */
/*-----------------------------------------------------------------------------*/
typedef enum {
    kImeChannel_1 = 0,
    kImeChannel_2 = 1,
    kImeChannel_3 = 2,
    kImeChannel_4 = 3,
    kImeChannel_5 = 4,
    kImeChannel_6 = 5,
    kImeChannel_7 = 6,
    kImeChannel_8 = 7,

    kImeTotal,

    kImeChannelNone = 99
} tVexImeChannels;

#define IME_MAX             kImeTotal   ///< Maximum number of IMEs we search for

#define DEFAULT_DEVICE      0x60        ///< Default IME address
#define IME_START_ADDRESS   0x20        ///< Start address when reassigning

#define IME_VERSION_ADDR    0x00        ///< Version request
#define IME_VENDOR_ADDR     0x08        ///< Vendor request
#define IME_DEVICEID_ADDR   0x10        ///< Device ID request
#define IME_STATUS_ADDR     0x20        ///< Status command
#define IME_DATA_ADDR       0x40        ///< Get data command

#define IME_SCRATCHR_ADDR   0x60        ///< read register offset of scratchpad regs
#define IME_SCRATCHW_ADDR   0xA0        ///< write register offset of scratchpad regs

#define IME_CLEAR_COUNTERS  0x4A        ///< Clear counters command
#define IME_DISABLE_TERM    0x4B        ///< disable termination command
#define IME_ENABLE_TERM     0x4C        ///< enable termination command
#define IME_SETDEV_REG      0x4D        ///< Set device address command
#define IME_RESET_REG       0x4E        ///< Reset command

#define IME_BUF_LEN         16

#define ACTION_POLL         0
#define ACTION_RENEGOTIATE  1

/*-----------------------------------------------------------------------------*/
/** @brief      structure to hold everything for one encoder                   */
/*-----------------------------------------------------------------------------*/
typedef struct _imeData {
    uint8_t     address;        ///< The I2C address of this IME
    uint8_t     type;           ///< The type of motor this IME is attached to
    uint16_t    valid;          ///< The IME has been negotiated with and is valid
    uint16_t    motor_index;    ///< A reference to the motor that this IME is connected to


    uint8_t     version[IME_BUF_LEN];   /**< @brief buffer to store version string
                                             read from this IME */
    uint8_t     vendor[IME_BUF_LEN];    /**< @brief buffer to store vendor string
                                             read from this IME */
    uint8_t     deviceid[IME_BUF_LEN];  /**< @brief buffer to store device id string
                                             read from this IME */
    uint8_t     enc_data[IME_BUF_LEN];  /**< @brief buffer used when reading encoder
                                             count */

    int32_t     count;          ///< last encoder count read from IME
    int32_t     offset;         ///< an offset that id deducted from count
    int32_t     velocity;       ///< velocity data from IME
    int32_t     delta_count;    ///< change in count from last time read
    int32_t     rpm;            ///< calculated rpm (not tested yet)

    int32_t     old_count;      ///< count from last poll

    int32_t     rpm_constant;   ///< constant used to calculate rpm based on IME type

    // track data requests and errors
    uint32_t    data_polls;     ///< number of times this IME was polled
    uint32_t    data_errors;    ///< number of errors in communication with this IME
    } imeData;

/*-----------------------------------------------------------------------------*/
/** @brief      structure holding all IME data                                 */
/*-----------------------------------------------------------------------------*/
typedef struct _vexImeData {
    I2CDriver   *i2cp;          ///< pointer to the I2C driver
    vexStream   *chp;           ///< Pointer to vexStream for debug output
    int16_t     num;            ///< the number of IMEs found on the I2C bus
    uint16_t    nextAddress;    ///< address for the next discovered IME
    uint16_t    error_lockup;   ///< total bus loss errors
    uint16_t    error_ack;      ///< acknowledge errors
    uint16_t    error_bus;      ///< bus errors
    uint16_t    error_arb;      ///< arbitration errors
    uint16_t    error_tim;      ///< timing errors
    uint16_t    error_seq;      ///< sequential errors
    uint16_t    action;         ///< indicates next action the IME thread should take
    uint16_t    debug;          ///< flag indicates verbose debug output
    imeData     imes[ IME_MAX ];///< array with data for each IME
    } vexImeData;

#define IME_COUNT_RESET     0x7F000000      ///< Dummy value used during IME reset


/** @name IME motor types
 * @{
 */
#define     IME_269             0
#define     IME_393T            1
#define     IME_393S            2
#define     IME_393R            3  // Turbo gears
#define     IME_UND             16
/** @} */

/*-----------------------------------------------------------------------------*/
int32_t     vexImeGetCount( int16_t channel );
void        vexImeSetCount( int16_t channel, int32_t value );
int16_t     vexImeGetId( int16_t channel );
int16_t     vexImeGetChannelMax(void);

imeData    *vexImeGetPtr( tVexImeChannels channel );

void        vexImeInit( I2CDriver *i2cp, vexStream *chp );
void        vexImeSetType( tVexImeChannels channel, tVexMotorType type );
void        vexImeSetMotor( tVexImeChannels channel, tVexMotor motor );
int32_t     vexImeMotorGetCount( tVexMotor motor );

// All of these functions are internal driver use
// we expose them so alternative IME drivers could be written
uint16_t    vexIMEFindEncoders( void );
void        vexIMEDataInit( imeData *ime );

msg_t       vexIMEUpdateCounts( imeData *ime );

msg_t       vexIMEGetVersion( uint8_t device, uint8_t *buf );
msg_t       vexIMEGetVendor( uint8_t device, uint8_t *buf );
msg_t       vexIMEGetDeviceId( uint8_t device, uint8_t *buf );
msg_t       vexIMEGetStatus( uint8_t device, uint8_t *buf );
msg_t       vexIMEGetData( uint8_t device, uint8_t *buf );
msg_t       vexIMEGetScratchpad( uint8_t device, uint8_t *buf, uint8_t offset, uint8_t len );
msg_t       vexIMESetScratchpad( uint8_t device, uint8_t *buf, uint8_t offset, uint8_t len );
msg_t       vexIMEClearCounters( uint8_t device );
msg_t       vexIMEDisableTermination( uint8_t device );
msg_t       vexIMEEnableTermination( uint8_t device );
msg_t       vexIMESetAddr( uint8_t *device, uint8_t newDevice );
msg_t       vexIMEResetAll( void );

void        vexIMEHandleErrors( msg_t status, i2cflags_t errors );
void        vexIMEPrintBuf( vexStream *chp, uint8_t *buf, uint8_t len );
void        vexIMEDebug(vexStream *chp, int argc, char *argv[]);

#ifdef   __cplusplus
}  // Close scope of extern "C" declaration
#endif

#endif  /* _VEXIME */
