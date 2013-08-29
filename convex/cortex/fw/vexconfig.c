/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexconfig.c                                                  */
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

/*-----------------------------------------------------------------------------*/
/** @file    vexconfig.c
  * @brief   Cortex configuration
*//*---------------------------------------------------------------------------*/

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @brief      Default digital IO configuration                               */
/*              All inputs                                                     */
/*-----------------------------------------------------------------------------*/
static  vexDigiCfg  vexDefConfig[kVexDigital_Num] = {
        { kVexDigital_1,    kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_2,    kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_3,    kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_4,    kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_5,    kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_6,    kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_7,    kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_8,    kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_9,    kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_10,   kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_11,   kVexSensorDigitalInput, kVexConfigInput,    0 },
        { kVexDigital_12,   kVexSensorDigitalInput, kVexConfigInput,    0 }
};

/*-----------------------------------------------------------------------------*/
/** @brief      Configure digital ports                                        */
/** @param[in]  cfg pointer to vexDigiCfg structure                            */
/** @param[in]  cfg_num number of entries in the cfg structure                 */
/** @details    Configure digital ports based on a configuration structure     */
/*-----------------------------------------------------------------------------*/

void
vexDigitalConfigure( vexDigiCfg *cfg, int16_t cfg_num )
{
    vexDigiCfg          *_cfg, *_cfg1, *_cfg2;
    tVexDigitalPin      i;
    tVexDigitalPin      j;
    int16_t             len;

    if( cfg == NULL ){
        _cfg = vexDefConfig;
        len = DIG_CONFIG_SIZE( vexDefConfig );
        }
    else {
        _cfg = cfg;
        len = cfg_num;
        }

    for(i=0,_cfg1=_cfg;i<len;i++,_cfg1++)
        {
        // An input ?
        if( _cfg1->cfg == kVexConfigInput )
            vexDigitalModeSet(_cfg1->pin, kVexDigitalInput );
        // An output ?
        if( _cfg1->cfg == kVexConfigOutput )
            vexDigitalModeSet(_cfg1->pin, kVexDigitalOutput );

        // An encoder ?
        if( _cfg1->cfg == kVexConfigQuadEnc1 )
            {
            // find other encoder pin
            for(j=0, _cfg2=_cfg;j<len;j++,_cfg2++)
                {
                if( (_cfg2->cfg == kVexConfigQuadEnc2) && (_cfg1->chan == _cfg2->chan ) )
                    vexEncoderAdd( _cfg1->chan, _cfg1->pin, _cfg2->pin );
                }
            }

        // A sonar ?
        if( _cfg1->cfg == kVexConfigSonarOut )
            {
            // find other encoder pin
            for(j=0, _cfg2=_cfg;j<len;j++,_cfg2++)
                {
                if( (_cfg2->cfg == kVexConfigSonarIn) && (_cfg1->chan == _cfg2->chan ) )
                    vexSonarAdd( _cfg1->chan, _cfg1->pin, _cfg2->pin );
                }
            }

        // Interrupt (experimental)
        if( _cfg1->cfg == kVexConfigInterrupt )
            {
            vexDigitalIntrSet( _cfg1->pin );
            }

        // A bit messy here
        // use the default config to save the user config.
        // obviously if there is no user config then this is superfluous.
        vexDefConfig[ _cfg1->pin ].type = _cfg1->type;
        vexDefConfig[ _cfg1->pin ].cfg  = _cfg1->cfg;
        vexDefConfig[ _cfg1->pin ].chan = _cfg1->chan;
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the sensor type given pin number                           */
/** @param[in]  pin The pin number                                             */
/** @return     The sensor type                                                */
/*-----------------------------------------------------------------------------*/

tVexSensorType
vexDigitalTypeGet( tVexDigitalPin pin )
{
    if( pin > kVexDigital_12 )
        return( kVexSensorNone );

    return( vexDefConfig[ pin ].type );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the sensor name given pin number                           */
/** @param[in]  pin The pin number                                             */
/** @return     A char pointer to the text name of the sensor                  */
/*-----------------------------------------------------------------------------*/

char *
vexDigitalTypeGetName( tVexDigitalPin pin )
{
    char    *p = NULL;

    if( pin > kVexDigital_12 )
        return( kVexSensorNone );

    switch( vexDefConfig[ pin ].type  )
        {
        case    kVexSensorDigitalInput:
            p = "Input";
            break;
        case    kVexSensorDigitalOutput:
            p = "Output";
            break;
        case    kVexSensorInterrupt:
            p = "Intrpt";
            break;
        case    kVexSensorQuadEncoder:
            if( vexDefConfig[ pin ].cfg == kVexConfigQuadEnc1 )
                p = "Encoder";
            else
                p = "";
            break;
        case    kVexSensorSonarCm:
            if( vexDefConfig[ pin ].cfg == kVexConfigSonarOut )
                p = "SonarCm";
            else
                p = "";
            break;

        case    kVexSensorSonarInch:
            if( vexDefConfig[ pin ].cfg == kVexConfigSonarOut )
                p = "SonarIn";
            else
                p = "";
            break;
        default:
            p = "Undefined";
            break;
        }

    return(p);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the sensor channel given pin number                        */
/** @param[in]  pin The pin number                                             */
/** @return     The channel number                                             */
/*-----------------------------------------------------------------------------*/

uint16_t
vexDigitalChannelGet( tVexDigitalPin pin )
{
    if( pin > kVexDigital_12 )
        return( 0 );

    return( vexDefConfig[ pin ].chan );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the sensor config given pin number                         */
/** @param[in]  pin The pin number                                             */
/** @return     The pin configuration                                          */
/*-----------------------------------------------------------------------------*/
/** @details
 *  The configuration is sensor dependent, eg. sonar output or input
 */
uint16_t
vexDigitalCfgGet( tVexDigitalPin pin )
{
    if( pin > kVexDigital_12 )
        return( 0 );

    return( vexDefConfig[ pin ].cfg );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Configure motors based on a configuration structure            */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Configure motors based on a configuration structure.
 *  This is primarily to link a motor with a position sensor such as an IME
 */
void
vexMotorConfigure( vexMotorCfg *cfg, int16_t cfg_num )
{
    vexMotorCfg *_cfg;
    int16_t     i, len;

    if( cfg != NULL ){
         _cfg = cfg;
         len = cfg_num;
         }
    else
        return;

    for(i=0;i<len;i++,_cfg++)
        {
        // save motor type
        vexMotorTypeSet(  _cfg->port,  _cfg->mtype );

        // set reversal if necessary
        vexMotorDirectionSet( _cfg->port, _cfg->reversed );

        switch( _cfg->stype )
            {
            case    kVexSensorIME:
                // link motor to IME
                vexImeSetMotor( _cfg->channel, _cfg->port );
                // set IME/motor type
                vexImeSetType( _cfg->channel, _cfg->mtype );
                // set the get position callback
                vexMotorPositionGetCallback( _cfg->port, vexImeGetCount, _cfg->channel );
                // set the set position callback
                vexMotorPositionSetCallback( _cfg->port, vexImeSetCount, _cfg->channel );
                // set the id request callback
                vexMotorEncoderIdCallback( _cfg->port, vexImeGetId, _cfg->channel );
                break;

            case    kVexSensorQuadEncoder:
                // set the get position callback
                vexMotorPositionGetCallback( _cfg->port, vexEncoderGet, _cfg->channel );
                // set the set position callback
                vexMotorPositionSetCallback( _cfg->port, vexEncoderSet, _cfg->channel );
                // set the id request callback
                vexMotorEncoderIdCallback( _cfg->port, vexEncoderGetId, _cfg->channel );
                break;

            default:
                break;
            }
        }
}
