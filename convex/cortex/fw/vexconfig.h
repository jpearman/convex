/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexconfig.h                                                  */
/*    Author:     James Pearman                                                */
/*    Created:    10 May 2013                                                  */
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

#ifndef __VEXCONFIG__
#define __VEXCONFIG__

/*-----------------------------------------------------------------------------*/
/** @file    vexconfig.h
  * @brief   Cortex configuration macros and prototypes
*//*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @brief      Sensor types for cortex configuration                          */
/*-----------------------------------------------------------------------------*/

typedef enum {
    kVexSensorNone = 0,
    kVexSensorDigitalInput,
    kVexSensorDigitalOutput,
    kVexSensorQuadEncoder,
    kVexSensorSonarCm,
    kVexSensorSonarInch,
    kVexSensorIME,

    kVexSensorPotentiometer,  // not used yet
    kVexSensorInterrupt
} tVexSensorType;

/*-----------------------------------------------------------------------------*/
/** @brief      A digital pin can be of the following type                     */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Although a digital IO pin can in reality only be configured as either an input
 *  or output, they can also have additional functionality depending on the
 *  intended function, for example, a quad encoder uses two digital pins, both
 *  are inputs but we also define them as QuadEnc1 or QuadEnc2
 */
typedef enum {
    kVexConfigInput    = kVexDigitalInput,
    kVexConfigOutput   = kVexDigitalOutput,
    kVexConfigQuadEnc1 = 10,
    kVexConfigQuadEnc2,
    kVexConfigSonarOut,
    kVexConfigSonarIn,

    kVexConfigInterrupt
    } tVexDigitalConfig;

/*-----------------------------------------------------------------------------*/
/** @brief     Structure holding information to configure a single digital pin */
/*-----------------------------------------------------------------------------*/

typedef struct _vexDigiCfg {
    tVexDigitalPin      pin;    ///< The pin number
    tVexSensorType      type;   ///< Type of pin, quad enc, sonar etc.
    tVexDigitalConfig   cfg;    ///< The config for this pin, this is type dependent
    uint16_t            chan;   ///< The channel for this pin, quad enc & sonars
} vexDigiCfg;

/*-----------------------------------------------------------------------------*/
/** @brief     Macro to calculate the number of entries in digital config      */
/*-----------------------------------------------------------------------------*/
#define DIG_CONFIG_SIZE( x )    sizeof( x ) / sizeof( vexDigiCfg )

/*-----------------------------------------------------------------------------*/
/** @brief     Structure holding information to configure a single motor       */
/*-----------------------------------------------------------------------------*/

typedef struct _vexMotorCfg {
    tVexMotor           port;       ///< The motor port number (0 through 9)
    tVexMotorType       mtype;      ///< Type of motor, 393, 269 etc.
    bool_t              reversed;   ///< Motor is reversed if true
    tVexSensorType      stype;      ///< Sensor type used for motor position
    int16_t             channel;    ///< The above sensor channel
} vexMotorCfg;

/*-----------------------------------------------------------------------------*/
/** @brief     Macro to calculate the number of entries in motor config        */
/*-----------------------------------------------------------------------------*/
#define MOT_CONFIG_SIZE( x )    sizeof( x ) / sizeof( vexMotorCfg )


#ifdef __cplusplus
extern "C" {
#endif

void            vexDigitalConfigure( vexDigiCfg *cfg, int16_t cfg_num );
void            vexMotorConfigure( vexMotorCfg *cfg, int16_t cfg_num );

tVexSensorType  vexDigitalTypeGet( tVexDigitalPin pin );
char           *vexDigitalTypeGetName( tVexDigitalPin pin );
uint16_t        vexDigitalChannelGet( tVexDigitalPin pin );
uint16_t        vexDigitalCfgGet( tVexDigitalPin pin );

#ifdef __cplusplus
}
#endif

#endif // __VEXCONFIG__
