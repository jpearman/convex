/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexspi.c                                                     */
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

#include <stdlib.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "chprintf.h"
#include "vex.h"

/*-----------------------------------------------------------------------------*/
/** @file    vexspi.c
  * @brief   SPI communication to the master processor
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/*  Storage for our SPI data                                                   */
/*-----------------------------------------------------------------------------*/
static  SpiData             vexSpiData;

static  GPTDriver          *spiGpt    = &GPTD2;
static  Thread             *spiThread = NULL;

static  char                spiTeamName[16] = CONVEX_TEAM_NAME;

/*-----------------------------------------------------------------------------*/
/* SPI configuration structure.                                                */
/* Maximum speed (2.25MHz), CPHA=1, CPOL=0, 16bits frames                      */
/* MSb transmitted first.                                                      */
/*-----------------------------------------------------------------------------*/

static SPIConfig spicfg = {
    NULL,
    /* HW dependent part.*/
    VEX_SPI_CS_PORT, VEX_SPI_CS_PIN,
    SPI_CR1_DFF | SPI_CR1_BR_2 | SPI_CR1_CPHA
};

/*-----------------------------------------------------------------------------*/
/*  This is the initial communications data to the master processor            */
/*  only initialize the txdata                                                 */
/*-----------------------------------------------------------------------------*/

static const unsigned char txInitData[32] =
{0x17, 0xC9, 0x02, 0x00, 0x00, 0x00,
 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x01, 0x00};


/*-----------------------------------------------------------------------------*/
/*  Timer callback                                                             */
/*  We use timer 2 in a one shot mode for the various nasty SPI delays needed  */
/*  rather than spinning in a loop                                             */
/*-----------------------------------------------------------------------------*/

static void
_vspi_gpt_cb(GPTDriver *gptp)
{
    (void)gptp;

    chSysLockFromIsr();

    // wake thread
    if (spiThread != NULL) {
        chSchReadyI(spiThread);
        spiThread = NULL;
      }

    chSysUnlockFromIsr();
}

/*-----------------------------------------------------------------------------*/
/*  Timer config structure                                                     */
/*  1MHz clock                                                                 */
/*-----------------------------------------------------------------------------*/

static const GPTConfig vexSpiGpt = {
    1000000,      /* 1MHz timer clock.*/
    _vspi_gpt_cb  /* Timer callback.*/
#if ( CH_KERNEL_VERSION_HEX >= 0x261 )
    ,0            /* DIER = 0, version 2.6.1.and on */
#endif
    };

/*-----------------------------------------------------------------------------*/
/** @brief      Init the SPI communication to the master processor             */
/*-----------------------------------------------------------------------------*/

void
vexSpiInit()
{
    uint16_t    i;

    // initialize the tx data
    for(i=0;i<32;i++)
        vexSpiData.txdata.data[i] = txInitData[i];

    vexSpiData.online = 0;

    // Initializes the SPI driver 1.
    spiStart(&SPID1, &spicfg);

    // start timer
    gptStart( spiGpt, &vexSpiGpt );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the team name                                              */
/** @param[in]  name pointer to string                                         */
/*-----------------------------------------------------------------------------*/

void
vexSpiTeamnameSet( char *name )
{
    int16_t i;

    for(i=0;i<8;i++)
        {
        if(*name != 0)
            spiTeamName[i] = *name++;
        else
            spiTeamName[i] = 0;
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Force stand alone operational mode                             */
/*-----------------------------------------------------------------------------*/

void
vexSpiModeStandalone()
{
    // set stand alone mode
    // the autonomous task will execute
    vexSpiData.txdata.data[3] = 0x01;
}

/*-----------------------------------------------------------------------------*/
/** @brief      We use presence of js_1 as indication of good joystick data    */
/** @returns    online status                                                  */
/** @retval     1 = online                                                     */
/** @retval     0 = offline                                                    */
/*-----------------------------------------------------------------------------*/
short
vexSpiGetOnlineStatus()
{
    return( vexSpiData.online );
}

/*-----------------------------------------------------------------------------*/
/** @brief      return a pointer to a raw joystick data structure              */
/** @param[in]  index Which joystick to use, 1 or 2                            */
/** @returns    pointer to _jsdata structure                                   */
/*-----------------------------------------------------------------------------*/

jsdata *
vexSpiGetJoystickDataPtr( int16_t index )
{
    if(index > 1)
        return( &vexSpiData.rxdata.pak.js_2 );
    else
        return( &vexSpiData.rxdata.pak.js_1 );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get competition and status word                                */
/** @returns    The status word from the spi data                              */
/*-----------------------------------------------------------------------------*/

uint16_t
vexSpiGetControl()
{
    return( (uint16_t)vexSpiData.rxdata.pak.ctl );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Scale factor for getting battery level in mV                   */
/*-----------------------------------------------------------------------------*/
#define SPI_BATTERY_SCALE   59

/*-----------------------------------------------------------------------------*/
/** @brief      Get main battery level                                         */
/** @returns    The battery level in mV                                        */
/** @note       Battery level has a resolution of 59mV                         */
/*-----------------------------------------------------------------------------*/

uint16_t
vexSpiGetMainBattery()
{
    // 59 mV * batt1 is battery voltage in mV
    return( (uint16_t)vexSpiData.rxdata.pak.batt1 * SPI_BATTERY_SCALE );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get backup battery level                                       */
/** @returns    The battery level in mV                                        */
/** @note       Battery level has a resolution of 59mV                         */
/*-----------------------------------------------------------------------------*/

uint16_t
vexSpiGetBackupBattery()
{
    // 59 mV * batt1 is battery voltage in mV
    return( (uint16_t)vexSpiData.rxdata.pak.batt2 * SPI_BATTERY_SCALE );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set motor value in the SPI tx data array                       */
/** @param[in]  index The index of the motor, range is 0 through 7             */
/** @param[in]  data The motor command data in the range +/- 127               */
/** @param[in]  reversed A flag indicating the motor data is reversed          */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function is used to set the data for one of the 3 wire motor ports
 *  on the master processor.  These correspond to cortex ports 2 through 9
 *  but are indexed here as ports 0 through 7
 */

void
vexSpiSetMotor( int16_t index, int16_t data, bool_t reversed )
{
    uint8_t m;

    // index should be in the range 0 through 7
    // corresponding to ports 2 through 9 on the cortex
    if( (index < 0) || (index > 7))
        return;

    // Data will be in the range +/- 127 so limit here and
    // convert to 0-255
    if( data > 127)
        m = 255;
    else
    if( data < (-127))
        m = 0;
    else
        m = data + 127;

    // move to buffer
    if( !reversed )
        vexSpiData.txdata.pak.motor[index] = m;
    else
        vexSpiData.txdata.pak.motor[index] = 255 - m;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Pause for exactly tick uS                                      */
/** @param[in]  tick The delay in uS                                           */
/** @note       New version suspends thread, timer callback wakes thread       */
/** @note       Internal SPI driver use only                                   */
/*-----------------------------------------------------------------------------*/

void
vexSpiTickDelay(int16_t tick)
{
   //gptStart(spiGpt, &gpt2cfg);
   //gptPolledDelay(spiGpt, tick);

   chSysLock();
   spiThread = chThdSelf();
   gptStartOneShotI( spiGpt, tick );
   chSchGoSleepS(THD_STATE_SUSPENDED);
   chSysUnlock();
}

/*-----------------------------------------------------------------------------*/
/** @brief      Send/receive one message with the master processor             */
/** @note       This is generally called by the system task                    */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This implementation is based on the EasyC code but with the loop delays/
 *  replaced with the use of a timer so compiler optimization can be used.
 *  Timing was then changed so there is really not much resemblance to the
 *  original code.
 */

void
vexSpiSend()
{
    int16_t      i;

    uint16_t    *txbuf = (uint16_t *)vexSpiData.txdata.data;
    uint16_t    *rxbuf = (uint16_t *)vexSpiData.rxdata_t.data;

    // configure team name if in configuration state
    if(vexSpiData.txdata.pak.state == 0x03)
        {
        char *p = spiTeamName;
        // Set team name data type
        vexSpiData.txdata.pak.type = 0x55;
        // Copy team name into data area
        // This is the same area as occupied normally by motor data
        for(i=0;i<8;i++)
            {
            if(*p != 0)
                vexSpiData.txdata.data[6+i] = *p++;
            else
                vexSpiData.txdata.data[6+i] = ' ';
            }
        }

    // Set handshake to indicate new spi message
    palSetPad( VEX_SPI_ENABLE_PORT, VEX_SPI_ENABLE_PIN );

    for(i=0;i<16;i++)
        {
        spiSelectI(&SPID1);
        rxbuf[i] = spi_lld_polled_exchange( &SPID1, txbuf[i] );
        //spiExchange( &SPID1, 1, &txbuf[i], &rxbuf[i]);
        spiUnselectI(&SPID1);

        if( ((i%4) == 3) && (i != 15) )
            {
            // long delay between each group of 4 words
            vexSpiTickDelay(73);

            // After 4 words negate handshake pin
            palClearPad( VEX_SPI_ENABLE_PORT, VEX_SPI_ENABLE_PIN );
            }
        else
            vexSpiTickDelay(8);
        }

    // increase id for next message
    vexSpiData.txdata.pak.id++;

    // check integrity of received data
    if( (vexSpiData.rxdata_t.data[0] == 0x17 ) && (vexSpiData.rxdata_t.data[1] == 0xC9 ))
        {
        // copy temporary data
        for(i=0;i<32;i++)
            vexSpiData.rxdata.data[i] = vexSpiData.rxdata_t.data[i];

        // Set online status if valid data status set
        if( (vexSpiData.rxdata.pak.status & 0x0F) == 0x08 )
            vexSpiData.online = 1;

        // If in configuration initialize state (0x02 or 0x03)
        if( (vexSpiData.txdata.pak.state & 0x0E) == 0x02 )
            {
            // check for configure request
            if( (vexSpiData.rxdata.pak.status & 0x0F) == 0x02 )
                vexSpiData.txdata.pak.state = 0x03;
            // check for configure and acknowledge
            if( (vexSpiData.rxdata.pak.status & 0x0F) == 0x03 )
                {
                vexSpiData.txdata.pak.state = 0x08;
                vexSpiData.txdata.pak.type  = 0;
                }
            // Either good or bad data force to normal transmission
            // status will either be 0x04 or 0x08
            if( (vexSpiData.rxdata.pak.status & 0x0C) != 0x00 )
                {
                vexSpiData.txdata.pak.state = 0x08;
                vexSpiData.txdata.pak.type  = 0;
                }
            }
        }
    else
        vexSpiData.errors++;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Debug function to dump SPI buffers                             */
/** @param[in]  chp     A pointer to a vexStream object                      */
/** @param[in]  argc    The number of command line arguments                   */
/** @param[in]  argv    An array of pointers to the command line args          */
/*-----------------------------------------------------------------------------*/

void
vexSpiDebug(vexStream *chp, int argc, char *argv[])
{
    int16_t i;
    int16_t index;
    int16_t data;

    (void)argc;
    (void)argv;

    if (argc > 1 )
        {
        index = atoi( argv[0] );
        data  = atoi( argv[1] );
        if(index > 1)
            vexSpiData.txdata.data[index] = data;
        }

    for(i=0 ;i<24;i++)
        chprintf(chp,"%02X ", vexSpiData.txdata.data[i] );
    chprintf(chp,"\r\n");
    for(i=24;i<32;i++)
        chprintf(chp,"%02X ", vexSpiData.txdata.data[i] );
    chprintf(chp,"\r\n");

    for(i=0 ;i<24;i++)
        chprintf(chp,"%02X ", vexSpiData.rxdata.data[i] );
    chprintf(chp,"\r\n");
    for(i=24;i<32;i++)
        chprintf(chp,"%02X ", vexSpiData.rxdata.data[i] );
    chprintf(chp,"\r\n");

    chprintf(chp,"errors %ld\r\n", vexSpiData.errors );

    chprintf(chp,"JS1 - ");
    chprintf(chp,"ch1 %3d ", vexSpiData.rxdata.pak.js_1.Ch1);
    chprintf(chp,"ch2 %3d ", vexSpiData.rxdata.pak.js_1.Ch2);
    chprintf(chp,"ch3 %3d ", vexSpiData.rxdata.pak.js_1.Ch3);
    chprintf(chp,"ch4 %3d ", vexSpiData.rxdata.pak.js_1.Ch4);
    chprintf(chp,"button %2X%2X\r\n", vexSpiData.rxdata.pak.js_1.btns[0],vexSpiData.rxdata.pak.js_1.btns[1]);
    chprintf(chp,"JS2 - ");
    chprintf(chp,"ch1 %3d ", vexSpiData.rxdata.pak.js_2.Ch1);
    chprintf(chp,"ch2 %3d ", vexSpiData.rxdata.pak.js_2.Ch2);
    chprintf(chp,"ch3 %3d ", vexSpiData.rxdata.pak.js_2.Ch3);
    chprintf(chp,"ch4 %3d ", vexSpiData.rxdata.pak.js_2.Ch4);
    chprintf(chp,"button %2X%2X\r\n", vexSpiData.rxdata.pak.js_2.btns[0],vexSpiData.rxdata.pak.js_2.btns[1]);

}

