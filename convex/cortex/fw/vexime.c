/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexime.c                                                     */
/*    Author:     James Pearman                                                */
/*    Created:    13 May 2013                                                  */
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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexime.c
  * @brief   IME driver
*//*---------------------------------------------------------------------------*/

// I2C Bus configuration
static const I2CConfig imeI2cConfig = {
    OPMODE_I2C,
    100000,
    FAST_DUTY_CYCLE_2,
};

// Global to hold errors
static i2cflags_t errors = 0;

// timeout used for all I2C messages
static systime_t tmo = MS2ST(4);

// All our IME data
static  vexImeData  vexImes;

/*-----------------------------------------------------------------------------*/
/** @brief      return encoder count                                           */
/** @param[in]  channel The encoder channel                                    */
/** @returns    The encoder count - the offset                                 */
/** @note  channel is int16_t to allow this function to be used in callbacks   */
/*-----------------------------------------------------------------------------*/

int32_t
vexImeGetCount( int16_t channel )
{
    if( (tVexImeChannels)channel > kImeChannel_8 )
        return(0);

    if( vexImes.imes[channel].valid )
        return( vexImes.imes[channel].count - vexImes.imes[channel].offset );
    else
        return(0);
}

/*-----------------------------------------------------------------------------*/
/** @brief      sets encoder count (stores an offset)                          */
/** @param[in]  channel The encoder channel                                    */
/** @param[in]  value The new encoder count value                              */
/** @note  channel is int16_t to allow this function to be used in callbacks   */
/*-----------------------------------------------------------------------------*/

void
vexImeSetCount( int16_t channel, int32_t value )
{
    if( (tVexImeChannels)channel > kImeChannel_8 )
        return;

    if( vexImes.imes[channel].valid )
        vexImes.imes[channel].offset = vexImes.imes[channel].count - value;
    else
        return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      return encoder id                                              */
/** @param[in]  channel The encoder channel                                    */
/** @returns    The channel id (address) for this channel                      */
/*  @note  channel is int16_t to allow this function to be used in callbacks   */
/*-----------------------------------------------------------------------------*/

int16_t
vexImeGetId( int16_t channel )
{
    if( (tVexImeChannels)channel > kImeChannel_8 )
        return(-1);

    // this helps with some legacy code
    return( vexImes.imes[channel].address );
}

/*-----------------------------------------------------------------------------*/
/** @brief      return pointer to IME data structure                           */
/** @param[in]  channel The encoder channel                                    */
/** @returns    A pointer to an imeData structure                              */
/*-----------------------------------------------------------------------------*/

imeData *
vexImeGetPtr( tVexImeChannels channel )
{
    if( channel > kImeChannel_8 )
        return(NULL);

    // return ptr to IME
    return( &vexImes.imes[channel] );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the number of IMEs found                                   */
/** @returns    The number of IMEs that were found during the last init        */
/*-----------------------------------------------------------------------------*/

int16_t
vexImeGetChannelMax()
{
    return( vexImes.num );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set motor type that this IME is attached to                    */
/** @param[in]  channel The encoder channel                                    */
/** @param[in]  type The motor type                                            */
/*-----------------------------------------------------------------------------*/
/** @note The IME motor type is different to tVexMotorType for backwards
 *        compatibility with the legacy vexPRO code.
 */

void
vexImeSetType( tVexImeChannels channel, tVexMotorType type )
{
    if( channel >= kImeTotal )
        return;

    // a bit screwed up but we added tVexMotorType later on
    switch(type)
        {
        case    kVexMotor269:
            vexImes.imes[channel].type = IME_269;
            break;
        case    kVexMotor393T:
            vexImes.imes[channel].type = IME_393T;
            break;
        case    kVexMotor393S:
            vexImes.imes[channel].type = IME_393S;
            break;
        case    kVexMotor393R:
            vexImes.imes[channel].type = IME_393R;
            break;
        default:
            vexImes.imes[channel].type = IME_UND;
            break;
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the motor that this IME is attached to                     */
/** @param[in]  channel The encoder channel                                    */
/** @param[in]  motor The motor index                                          */
/*-----------------------------------------------------------------------------*/

void
vexImeSetMotor( tVexImeChannels channel, tVexMotor motor )
{
    if( channel >= kImeTotal )
        return;
    if( motor > kVexMotor_10 )
        return;

    // save motor index in IME structure
    vexImes.imes[channel].motor_index = motor;
}

/*-----------------------------------------------------------------------------*/
/** @brief      return encoder count based on motor index                      */
/** @param[in]  motor The motor index                                          */
/** @returns    The encoder count for the IME attached to the motor            */
/** @note       This function was deprecated                                   */
/*-----------------------------------------------------------------------------*/

int32_t
vexImeMotorGetCount( tVexMotor motor )
{
    int16_t     channel;

    if( motor > kVexMotor_10 )
        return(0);

    // slow
    for(channel=0;channel<kImeTotal;channel++)
        {
        if( vexImes.imes[channel].motor_index == motor )
            return( vexImeGetCount( channel ) );
        }

    return( 0 );
}

/*-----------------------------------------------------------------------------*/
/*  Task to poll IME devices                                                   */
/*-----------------------------------------------------------------------------*/

static WORKING_AREA(waVexImeTask, IME_TASK_STACK_SIZE);
static msg_t
vexImeTask( void *arg )
{
    int     i;
    (void)arg;

    chRegSetThreadName("ime");

    // try and find IMEs
    vexImes.action = ACTION_RENEGOTIATE;

    while(!chThdShouldTerminate())
        {
        if( vexImes.action == ACTION_RENEGOTIATE )
            {
            // find encoders
            vexIMEFindEncoders();

            // Show debug
            if(vexImes.debug)
                {
                for( i = 0;i<vexImes.num;i++ )
                    {
                    // print data for demo
                    vex_chprintf(vexImes.chp,"Encoder:%d\r\n", i);
                    vex_chprintf(vexImes.chp,"Version:   ");
                    vexIMEPrintBuf( vexImes.chp, vexImes.imes[i].version, 8 );

                    vex_chprintf(vexImes.chp,"Vendor:    ");
                    vexIMEPrintBuf( vexImes.chp, vexImes.imes[i].vendor, 8 );

                    vex_chprintf(vexImes.chp,"Device ID: ");
                    vexIMEPrintBuf( vexImes.chp, vexImes.imes[i].deviceid, 8 );
                    }
                }
            }

        // If we have some IMEs then poll each one in turn
        if( vexImes.num > 0 )
            {
            for( i = 0;i<vexImes.num; i++ )
                {
                // poll next IME
                if( vexIMEUpdateCounts( &vexImes.imes[i] ) == RDY_OK )
                    vexImes.error_seq = 0;

                // sleep
                chThdSleepMilliseconds(1);
                }
            }
        else
            {
            // No IME's, try and find them
            vexImes.action = ACTION_RENEGOTIATE;
            // Try infrequently
            chThdSleepMilliseconds(10);
            }
        }

    return (msg_t)0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize IMEs on the I2C bus                                 */
/** @param[in]  i2cp A pointer to the I2C driver (chibios hal driver)          */
/** @param[in]  chp  A pointer to a vexStream object (used for debug output)   */
/** @pre        The I2C driver should have already been initialized            */
/*-----------------------------------------------------------------------------*/

void
vexImeInit( I2CDriver *i2cp, vexStream *chp )
{
    int16_t     i;

    // Start driver
    // This may need to be moved from here if more I2C devices are added
    i2cStart(i2cp, &imeI2cConfig);

    // Save driver
    vexImes.i2cp = i2cp;
    // Save console driver for debug
    vexImes.chp  = chp;

    // Init errors
    vexImes.error_ack   = 0;
    vexImes.error_arb   = 0;
    vexImes.error_bus   = 0;

    // turn off debug
    vexImes.debug       = 0;

    // Zero statistics for each ime
    for(i=0;i<IME_MAX;i++)
        {
        vexImes.imes[i].address      = 0;
        vexImes.imes[i].data_polls   = 0;
        vexImes.imes[i].data_errors  = 0;
        vexImes.imes[i].motor_index  = -1;
        }

    // Start thread
    chThdCreateStatic(waVexImeTask, sizeof(waVexImeTask), IME_THREAD_PRIORITY, vexImeTask, NULL);
}

/*---------------------------------------------------------------------------*/
/** @brief      Read encoder data and then calculate the rpm                 */
/** @param[in]  ime A pointer to an imeData structure                        */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @retval     0 ime pointer is NULL                                        */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEUpdateCounts( imeData *ime )
{
    msg_t status = RDY_OK;

    if(ime == NULL)
        return(0);

    // one more message
    ime->data_polls++;

    // get new data
    if( (status = vexIMEGetData( ime->address, ime->enc_data )) == RDY_OK )
        {
        // 32 bit counter, 48 seems over the top
        ime->count    =  ((long)ime->enc_data[0] << 8) | ((long)ime->enc_data[1] << 0) | ((long)ime->enc_data[2] << 24) | ((long)ime->enc_data[3] << 16);
        ime->velocity =  ((long)ime->enc_data[4] << 8) + ((long)ime->enc_data[5] << 0);

        if(ime->old_count != IME_COUNT_RESET )
            {
            ime->delta_count =  ime->count - ime->old_count;

            // calculate rpm based on IME velocity data
            if(ime->velocity != 0)
                ime->rpm =  ((int32_t)15625 * (int32_t)(60000/4)) / (ime->velocity * ime->rpm_constant);
            else
                ime->rpm = 0;
            }

        ime->old_count = ime->count;

        }
    else
        {
        ime->data_errors++;
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Init data structure for one IME                              */
/** param[in]   ime A pointer to a imeData structure                         */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

void
vexIMEDataInit( imeData *ime )
{
    ime->address = DEFAULT_DEVICE;
    ime->valid   = 0;

    memset( ime->version,  0, IME_BUF_LEN );
    memset( ime->vendor,   0, IME_BUF_LEN );
    memset( ime->deviceid, 0, IME_BUF_LEN );
    memset( ime->enc_data, 0, IME_BUF_LEN );

    ime->count        = 0;
    ime->offset       = 0;
    ime->old_count    = IME_COUNT_RESET;
    ime->velocity     = 0;
}

/*---------------------------------------------------------------------------*/
/** @brief      Search bus and add all encoders found (up to 8)              */
/** @returns    The number of IMEs found                                     */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

uint16_t
vexIMEFindEncoders()
{
    int16_t i = 0;
    imeData *ime;

    // Try and disable the termination on every IME
    for(i=0;i<IME_MAX+2;i++) {
        vexIMEDisableTermination(0);
        chThdSleepMilliseconds(3);
    }

    // Reset
    vexIMEResetAll();

    // Init data structure
    vexImes.num         = 0;
    vexImes.error_seq   = 0;
    vexImes.nextAddress = IME_START_ADDRESS;
    vexImes.action      = ACTION_POLL;

    // look for up to IME_MAX encoders
    for(i=0;i<IME_MAX;i++)
        {
        // get next ime slot and initialize it's data
        ime = &vexImes.imes[ i ];
        vexIMEDataInit( ime );

        // Look for next encoder
        // seems enabling the termination of the default devices help initialization
        if( vexIMEEnableTermination( ime->address ) == RDY_OK )
            {
            // one more encoder found
            vexImes.num++;

            // Set new address
            vexIMESetAddr( &ime->address, vexImes.nextAddress );
            vexImes.nextAddress += 2;

            // Give a little time to set new address
            chThdSleepMilliseconds(1);

            // Get encoder information
            vexIMEGetVersion( ime->address, ime->version );
            vexIMEGetVendor( ime->address, ime->vendor );
            vexIMEGetDeviceId( ime->address, ime->deviceid );

            // clear encoder counters
            vexIMEClearCounters( ime->address );

            // set a constant for the rpm calculations based on the encoder
            // type, we could do this from the device_id but will not know
            // if a 393 is set for speed or torque.
            // constants reduced by a factor of 4 due to overflow issues
            // take the ticks_per_rev * 125 to get these numbers
            switch( ime->type )
                {
                case    IME_269:
                    ime->rpm_constant = 30056/4;
                    break;
                case    IME_393T:
                    ime->rpm_constant = 78400/4;
                    break;
                case    IME_393S:
                    ime->rpm_constant = 49000/4;
                    break;
                case    IME_393R:
                    ime->rpm_constant = 32668/4;
                    break;
                default:
                    ime->rpm_constant = 39200/4;
                    break;
                }

            // valid channel
            ime->valid   = 1;

            // disable termination so we can fint the next encoder
            vexIMEDisableTermination( ime->address );
            chThdSleepMilliseconds(1);
            }
        else
            {
            // set address as error
            ime->address = 0;
            break;
            }
        }

    // enable termination on last encoder
    if( vexImes.num > 0 )
        vexIMEEnableTermination( vexImes.imes[ vexImes.num-1 ].address );

    return( vexImes.num );
}

/*---------------------------------------------------------------------------*/
/** @brief      Read version from IME                                        */
/** @param[in]  device The IME address                                       */
/** @param[out] buf Pointer to the buffer for reply data                     */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEGetVersion( uint8_t device, uint8_t *buf )
{
    msg_t status = RDY_OK;
    uint8_t txbuf = IME_VERSION_ADDR;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, device>>1, &txbuf, 1, buf, 8, tmo);
    i2cReleaseBus(vexImes.i2cp);

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Read vendor from IME                                         */
/** @param[in]  device The IME address                                       */
/** @param[out] buf Pointer to the buffer for reply data                     */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEGetVendor( uint8_t device, uint8_t *buf )
{
    msg_t status = RDY_OK;
    uint8_t txbuf = IME_VENDOR_ADDR;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, device>>1, &txbuf, 1, buf, 8, tmo);
    i2cReleaseBus(vexImes.i2cp);

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Read device ID from IME                                      */
/** @param[in]  device The IME address                                       */
/** @param[out] buf Pointer to the buffer for reply data                     */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEGetDeviceId( uint8_t device, uint8_t *buf )
{
    msg_t status = RDY_OK;
    uint8_t txbuf = IME_DEVICEID_ADDR;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, device>>1, &txbuf, 1, buf, 8, tmo);
    i2cReleaseBus(vexImes.i2cp);

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Read Status from IME                                         */
/** @param[in]  device The IME address                                       */
/** @param[out] buf Pointer to the buffer for reply data                     */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEGetStatus( uint8_t device, uint8_t *buf )
{
    msg_t status = RDY_OK;
    uint8_t txbuf = IME_STATUS_ADDR;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, device>>1, &txbuf, 1, buf, 4, tmo);
    i2cReleaseBus(vexImes.i2cp);

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Read encoder data from IME                                   */
/** @param[in]  device The IME address                                       */
/** @param[out] buf Pointer to the buffer for reply data                     */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEGetData( uint8_t device, uint8_t *buf )
{
    msg_t status = RDY_OK;
    uint8_t txbuf = IME_DATA_ADDR;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, device>>1, &txbuf, 1, buf, 8, tmo);
    i2cReleaseBus(vexImes.i2cp);

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Read scratchpad data from IME                                */
/** @param[in]  device The IME address                                       */
/** @param[in]  buf Pointer to the scratchpad data                           */
/** @param[in]  offset An offset into the scratchpad area                    */
/** @param[in]  len The length of the scratchpad data to write               */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEGetScratchpad( uint8_t device, uint8_t *buf, uint8_t offset, uint8_t len )
{
    msg_t status = RDY_OK;
    uint8_t txbuf = IME_SCRATCHR_ADDR+offset;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, device>>1, &txbuf, 1, buf, len, tmo);
    i2cReleaseBus(vexImes.i2cp);

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Write scratchpad data to the IME                             */
/** @param[in]  device The IME address                                       */
/** @param[in]  buf Pointer to the scratchpad data                           */
/** @param[in]  offset An offset into the scratchpad area                    */
/** @param[in]  len The length of the scratchpad data to write               */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMESetScratchpad( uint8_t device, uint8_t *buf, uint8_t offset, uint8_t len )
{
    msg_t status = RDY_OK;
    uint8_t *txbuf;
    int i;

    // allocate 64 bytes on heap
    // This used to be on the stack but that was a waste
    // so as this function will be rarely ever called (perhaps never) we use
    // the heap.
    txbuf = chHeapAlloc( NULL, 64 );

    if( txbuf != NULL )
        {
        txbuf[0] = IME_SCRATCHW_ADDR+offset;
        for(i=0;i<len;i++)
            txbuf[i+1] = buf[i];

        i2cAcquireBus(vexImes.i2cp);
        status = i2cMasterTransmitTimeout(vexImes.i2cp, device>>1, txbuf, len+1, NULL, 0, tmo);
        i2cReleaseBus(vexImes.i2cp);

        // free tx buffer
        chHeapFree(txbuf);
        }

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Clear Counters in IME                                        */
/** @param[in]  device The IME address                                       */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEClearCounters( uint8_t device )
{
    msg_t status = RDY_OK;
    uint8_t txbuf = IME_CLEAR_COUNTERS;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, device>>1, &txbuf, 1, NULL, 0, tmo);
    i2cReleaseBus(vexImes.i2cp);

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Disable termination in IME                                   */
/** @param[in]  device The IME address                                       */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEDisableTermination( uint8_t device )
{
    msg_t status = RDY_OK;
    uint8_t txbuf = IME_DISABLE_TERM;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, device>>1, &txbuf, 1, NULL, 0, tmo);
    i2cReleaseBus(vexImes.i2cp);

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Enable termination in IME                                    */
/** @param[in]  device The IME address                                       */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEEnableTermination( uint8_t device )
{
    msg_t status = RDY_OK;
    uint8_t txbuf = IME_ENABLE_TERM;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, device>>1, &txbuf, 1, NULL, 0, tmo);
    i2cReleaseBus(vexImes.i2cp);

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Set new Address in IME                                       */
/** @param[in]  device The IME address                                       */
/** @param[in]  newDevice The new IME address                                */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMESetAddr( uint8_t *device, unsigned char newDevice )
{
    msg_t status = RDY_OK;
    uint8_t buf[4];


    //
    buf[0] = IME_SETDEV_REG;
    buf[1] = newDevice;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, (*device)>>1, buf, 2, NULL, 0, tmo);
    i2cReleaseBus(vexImes.i2cp);

    if (status != RDY_OK){
        errors = i2cGetErrors(vexImes.i2cp);
        vexIMEHandleErrors( status, errors );
        }

    *device = newDevice;

    return(status);
}

/*---------------------------------------------------------------------------*/
/** @brief      Reset an IME                                                 */
/** @returns    The status of the I2C driver                                 */
/** @retval     RDY_OK If the function succeeded                             */
/** @retval     RDY_RESET   if one or more I2C errors occurred.              */
/** @retval     RDY_TIMEOUT if a timeout occurred before operation end.      */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

msg_t
vexIMEResetAll( )
{
    msg_t status = RDY_OK;
    uint8_t buf[4];

    //
    buf[0] = IME_RESET_REG;
    buf[1] = 0xCA;
    buf[2] = 0x03;

    i2cAcquireBus(vexImes.i2cp);
    status = i2cMasterTransmitTimeout(vexImes.i2cp, 0, buf, 3, NULL, 0, tmo);
    i2cReleaseBus(vexImes.i2cp);

    // short delay for reset
    chThdSleepMilliseconds(100);

    return(status);
}


/*---------------------------------------------------------------------------*/
/** @brief      Handle received IME I2C errors                               */
/** @param[in]  status The received message from the I2C low level driver    */
/** @param[in]  errors The received error status                             */
/** @note       Internal IME driver use only                                 */
/*---------------------------------------------------------------------------*/

void
vexIMEHandleErrors( msg_t status, i2cflags_t errors )
{
    // A RDY_TIMEOUT should never occur, means bus was held low for long time
    // reset I2C bus and start again
    if( status == RDY_TIMEOUT )
        {
        // total bus loss
        vexImes.error_lockup++;
        // stop bus
        i2cStop( vexImes.i2cp );
        // wait
        chThdSleepMilliseconds(100);
        // restart
        i2cStart( vexImes.i2cp , &imeI2cConfig);
        // force renegotiation
        vexImes.action = ACTION_RENEGOTIATE;
        }

    // If any error then increase number of
    // sequential errors
    if( errors != I2CD_NO_ERROR )
        {
        vexImes.error_seq++;
        // if we found too many then assume cable pulled
        if( vexImes.error_seq == 5 )
            {
            // force re-negotiate
            vexImes.action = ACTION_RENEGOTIATE;
            }
        }

    // count errors for debug
    if( errors & I2CD_ACK_FAILURE )
        vexImes.error_ack++;
    if( errors & I2CD_ARBITRATION_LOST )
        vexImes.error_arb++;
    if( errors & I2CD_BUS_ERROR )
        vexImes.error_bus++;
    if( errors & I2CD_TIMEOUT )
        vexImes.error_tim++;
}


/*---------------------------------------------------------------------------*/
/** @brief      Print a buffer with IME data                                 */
/** @param[in]  chp     A pointer to a vexStream object                      */
/** @param[in]  buf A pointer to a buffer of uint8_t                         */
/** @param[in]  len The buffer elngth                                        */
/** @note       Used by internal IME debug functions                         */
/*---------------------------------------------------------------------------*/

void
vexIMEPrintBuf( vexStream *chp, uint8_t *buf, uint8_t len )
{
    int i;

    for(i=0;i<len;i++)
        vex_chprintf(chp,"%02X ", buf[i] );

    vex_chprintf(chp,": ");

    for(i=0;i<len;i++)
        {
        if((buf[i] < 0x7f) && (buf[i] > 0x1f))
            vex_chprintf(chp,"%c", buf[i] );
        else
            vex_chprintf(chp,".");
        }

    vex_chprintf(chp,"\r\n");
}

/*-----------------------------------------------------------------------------*/
/** @brief      Debug function to dump IME buffers                             */
/** @param[in]  chp     A pointer to a vexStream object                        */
/** @param[in]  argc    The number of command line arguments                   */
/** @param[in]  argv    An array of pointers to the command line args          */
/*-----------------------------------------------------------------------------*/

void
vexIMEDebug(vexStream *chp, int argc, char *argv[])
{
    int i;

    (void)argc;
    (void)argv;

    vex_chprintf(chp,"%d IME's found\r\n",vexImes.num);
    vex_chprintf(chp,"Errors Lock(%d) Ack(%d) Bus(%d) Arb(%d) Tim(%d)\r\n",
            vexImes.error_lockup, vexImes.error_ack, vexImes.error_bus, vexImes.error_arb, vexImes.error_tim );

    for(i=0;i<vexImes.num;i++)
        {
        vex_chprintf(chp,"IME_%d ", i+1); // want to show 1 through 8 not 0 through 7
                                          // same with motor index below
        switch( vexImes.imes[i].type )
            {
            case    IME_269:
                vex_chprintf(chp,"269  ");
                break;
            case    IME_393T:
                vex_chprintf(chp,"393T ");
                break;
            case    IME_393S:
                vex_chprintf(chp,"393S ");
                break;
            case    IME_393R:
                vex_chprintf(chp,"393R ");
                break;
            default:
                vex_chprintf(chp,"---- ");
                break;
            }

        vex_chprintf(chp,"(%-8s) M_%-2d ",vexImes.imes[i].deviceid, vexImes.imes[i].motor_index+1 );
        vex_chprintf(chp,"%8d/", vexImes.imes[i].data_polls );
        vex_chprintf(chp,"%-5d ", vexImes.imes[i].data_errors );
        vex_chprintf(chp,"count %6d ", vexImes.imes[i].count );
        vex_chprintf(chp,"vel   %5d ", vexImes.imes[i].velocity );
        vex_chprintf(chp,"rpm   %3d ", vexImes.imes[i].rpm );
        vex_chprintf(chp,"\r\n");
        }
}
