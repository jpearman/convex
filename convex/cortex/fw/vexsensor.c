/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexsensor.c                                                  */
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

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexsensor.c
  * @brief   ROBOTC style sensor interface
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      Get the value of a sensor port                                 */
/** @param[in]  sensor A sensor of type tVexSensors                            */
/** @returns    TRUE if the port is an analog sensor                           */
/*-----------------------------------------------------------------------------*/

bool_t
vexSensorIsAnalog( tVexSensors sensor  )
{
    if( (sensor >= kVexSensorAnalog_1) && (sensor <= kVexSensorAnalog_8))
        return(TRUE);
    else
        return(FALSE);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the value of a sensor port                                 */
/** @param[in]  sensor A sensor of type tVexSensors                            */
/** @returns    The value of the current sensor.                               */
/** @note       A ROBOTC compatibility function                                */
/*-----------------------------------------------------------------------------*/

int32_t
vexSensorValueGet( tVexSensors sensor )
{
    if( sensor <= kVexSensorAnalog_8 )
        return( vexAdcGet( (int16_t)sensor ) );
    else
    if( sensor <= kVexSensorDigital_12 )
        {
        int16_t chan;
        tVexDigitalPin pin  = (tVexDigitalPin)( sensor - kVexSensorDigital_1 );
        tVexSensorType type = vexDigitalTypeGet( pin );

        switch( type )
            {
            case    kVexSensorNone:
                break;
            case    kVexSensorDigitalInput:
                return( vexDigitalPinGet(pin ) );
                break;
            case    kVexSensorDigitalOutput:
                return( vexDigitalPinGet(pin ) );
                break;
            case    kVexSensorInterrupt:
                return( vexDigitalPinGet(pin ) );
                break;
            case    kVexSensorQuadEncoder:
                chan = vexDigitalChannelGet( pin );
                return( vexEncoderGet( chan ) );
                break;
            case    kVexSensorSonarCm:
                chan = vexDigitalChannelGet( pin );
                return( vexSonarGetCm( chan ) );
                break;
            case    kVexSensorSonarInch:
                chan = vexDigitalChannelGet( pin );
                return( vexSonarGetInch( chan ) );
                break;
            default:
                break;
            }
        }
    else
    if( sensor <= kVexSensorIme_8 )
        {
        int16_t chan = sensor - kVexSensorIme_1;
        return( vexImeGetCount( chan) );
        }

    return(0);
}


/*-----------------------------------------------------------------------------*/
/** @brief      Set the value of a sensor port                                 */
/** @param[in]  sensor A sensor of type tVexSensors                            */
/** @param[in]  value  Value to set the sensor to.                             */
/** @note       A ROBOTC compatibility function                                */
/*-----------------------------------------------------------------------------*/

void
vexSensorValueSet( tVexSensors sensor, int32_t value )
{
    if( sensor <= kVexSensorAnalog_8 )
        return;
    else
    if( sensor <= kVexSensorDigital_12 )
        {
        int16_t chan;
        tVexDigitalPin pin  = (tVexDigitalPin)( sensor - kVexSensorDigital_1 );
        tVexSensorType type = vexDigitalTypeGet( pin );

        switch( type )
            {
            case    kVexSensorNone:
                break;
            case    kVexSensorDigitalInput:
                break;
            case    kVexSensorInterrupt:
                break;
            case    kVexSensorDigitalOutput:
                vexDigitalPinSet( pin, value );
                break;
            case    kVexSensorQuadEncoder:
                chan = vexDigitalChannelGet( pin );
                vexEncoderSet( chan, value );
                break;
            case    kVexSensorSonarCm:
                break;
            case    kVexSensorSonarInch:
                break;
            default:
                break;
            }
        }
    else
    if( sensor <= kVexSensorIme_8 )
        {
        int16_t chan = sensor - kVexSensorIme_1;
        vexImeSetCount( chan, value );
        }

    return;
}
