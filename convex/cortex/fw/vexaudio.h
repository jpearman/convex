/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexaudio.h                                                   */
/*    Author:     James Pearman                                                */
/*    Created:    25 April 2012                                                */
/*                                                                             */
/*    Revisions:                                                               */
/*                V0.20  25 Apr 2012 - Original version for EasyC              */
/*                V1.00  4 July 2013 - Initial release                         */
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
/*        Header file for vex sound functions                                  */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */

#ifndef VEXAUDIO_H_
#define VEXAUDIO_H_

/*-----------------------------------------------------------------------------*/
/** @file    vexaudio.h
  * @brief   DAC port driver macros and prototypes
*//*---------------------------------------------------------------------------*/

// un-comment this to allow per tone amplitude control at the expense of memory
//#define VSL_AMP_PER_TONE 1

// Public functions
void    vexAudioPlaySound( int freq, int amplitude, int timems );
void    vexAudioPlaySoundWait( int freq, int amplitude, int timems );
void    vexAudioStopSound( void );

char   *vexAudioPlayRttl( char *song , int amplitude, int repeat );

int     vexAudioInitChipToneSong( int len );
void    vexAudioDeinitChipToneSong( void );
void    vexAudioStartChipToneSong( int repeat );
void    vexAudioStopChipToneSong( void );
void    vexAudioSetNextChipTone( int freq, int amplitude, int timems );
void    vexAudioDebugChipTone( void );


// private functions
int     vexAudioPlayNextChipTone( void );
void    VSL_Factorize(int n, int *f1, int *f2);
void    VSL_CreateSineWave( int amplitude );
void    VSL_InitTimer( int prescale, int period );
void    VSL_Init( void );
void    VSL_Deinit(void);

#ifdef VSL_AMP_PER_TONE
// version using amplitude per tone - uses more memory
typedef struct _vsl_ct {
    int freq;
    int amplitude;
    int timems;
    } vsl_ct;
#else
// default - single amplitude for the song
typedef struct _vsl_ct {
    int freq;
    int timems;
    } vsl_ct;
#endif

/** @cond  */ // Not for Doxygen
#define DEFAULT_AMPLITUDE   64
#define MIN_AMPLITUDE        0
#define MAX_AMPLITUDE      255

// DAC definitions from standard peripheral library
#define DAC_Channel_1                      ((uint32_t)0x00000000)
#define DAC_Channel_2                      ((uint32_t)0x00000010)
#define DAC_Trigger_T7_TRGO                ((uint32_t)0x00000014) /*!< TIM7 TRGO selected as external conversion trigger for DAC channel */
#define DAC_WaveGeneration_None            ((uint32_t)0x00000000)
#define DAC_LFSRUnmask_Bit0                ((uint32_t)0x00000000) /*!< Unmask DAC channel LFSR bit0 for noise wave generation */
#define DAC_OutputBuffer_Enable            ((uint32_t)0x00000000)
#define DAC_OutputBuffer_Disable           ((uint32_t)0x00000002)
#define CR_CLEAR_MASK                      ((uint32_t)0x00000FFE)

// Timer definitions from standard peripheral library
#define TIM_PSCReloadMode_Immediate        ((uint16_t)0x0001)
#define TIM_TRGOSource_Update              ((uint16_t)0x0020)
/** @endcond  */

#endif // VEXAUDIO_H_




