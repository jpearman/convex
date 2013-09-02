/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexaudio.c                                                   */
/*    Author:     James Pearman                                                */
/*    Created:    25 April 2012                                                */
/*                                                                             */
/*    Revisions:                                                               */
/*                V0.20  EasyC original versionn                               */
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
/*    A reworked version of the code written for EasyC sound.                  */
/*    There is no use of the standard peripheral library calls in this         */
/*    version.  Features of ChibiOS/RT are used instead of repeating timer     */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** @file    vexaudio.c
  * @brief   DAC port driver
*//*---------------------------------------------------------------------------*/


#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

// number of samples we use for the basic sine wave that all sounds are
// created from
// If this changes then change the pre-calculated sin values
#define WAVE_SAMPLES    32

// global storage for sine wave
static  uint16_t Sine12bit[WAVE_SAMPLES];

// dma STREAM FOR dac
static const stm32_dma_stream_t   *vsl_dma = STM32_DMA_STREAM(STM32_DMA_STREAM_ID(2, 3));

// counter for sound output timer
static  volatile uint32_t VSL_Counter = 0;

// the audio task
static  Thread  *vslThread = NULL;

// event source to indicate sound is finished.
static  EVENTSOURCE_DECL(sound_done);

// semaphore used to wake audio thread
static  Semaphore    vslSem;

// working area for background thread
#define AUDIO_WA_SIZE   THD_WA_SIZE(AUDIO_TASK_STACK_SIZE)
/*-----------------------------------------------------------------------------*/
/** @brief      Background audio thread                                        */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This thread is used to stop sounds and start the next if necessary
 *  Unlike the EasyC version, this thread enters a sleep state for the duration
 *  of the sound, if no sound is playing then it sleeps infinitely.
 *  An event is sent to any waiting threads when the sleep time has elapsed.
 *  This is a dynamically created thread, if sound is not used then resources
 *  are not allocated
 */

static msg_t
vexAudioTask( void *arg )
{
    uint32_t    tmpCounter;

    (void)arg;

    chRegSetThreadName("audio");
    chEvtInit(&sound_done);

    while(!chThdShouldTerminate())
        {
        if(VSL_Counter > 0)
            {
            tmpCounter = VSL_Counter;
            VSL_Counter = 0;
            // wait for semaphore timeout or other thread reseting the semaphore
            chSemWaitTimeout( &vslSem, tmpCounter );

            chSysLock();
            if( chEvtIsListeningI(&sound_done) )
                chEvtBroadcastI(&sound_done);
            chSysUnlock();

            if( VSL_Counter == 0 )
                {
                if( !vexAudioPlayNextChipTone() )
                    VSL_Deinit();
                }
            }
        else
            chSemWait( &vslSem );
        }

    return (msg_t)0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Play frequency at given amplitude                              */
/** @param[in]  freq desired frequency in Hz                                   */
/** @param[in]  amplitude desired amplitude 0 = silence, 128 = max             */
/** @param[in]  timems duration of tone                                        */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This is the base function that all sound playback uses.
 *  The first time this is called all the DAC and DMA initialization is done,
 *  a thread is created to stop the sound after the requested playback time
 */

void
vexAudioPlaySound( int freq, int amplitude, int timems )
{
    static int current_amplitude = DEFAULT_AMPLITUDE;
    int f1, f2;

    // Not available on the Olimex eval card
#if defined (STM32F10X_HD)

    // init first time
    if( vslThread == NULL )
        {
        VSL_Init();
        // init counting semaphore with a value of 0
        chSemInit( &vslSem, 0 );
        vslThread = chThdCreateFromHeap(NULL, AUDIO_WA_SIZE, NORMALPRIO, vexAudioTask, (void *)NULL );
        }

    // try and stop pops
    // a frequency of 0 means silence
    if( freq == 0 )
        {
        freq = 1000;
        amplitude = 0;
        }

    // create waveform
    if( amplitude != current_amplitude )
        {
        VSL_CreateSineWave( amplitude );
        current_amplitude = amplitude;
        }

    // limit range of frequencies 200Hz to 10KHz
    if( freq <   200 ) freq =   200;
    if( freq > 10000 ) freq = 10000;
    
    // calculate prescale and period for the timer
    VSL_Factorize( 72000000L / (32 * freq), &f1, &f2 );

    // ReInit timer
    VSL_InitTimer(f1, f2);

    // Enable DMA for the DAC
    DAC->CR |= (DAC_CR_EN1 << DAC_Channel_1);
    
    /* TIM7 enable counter */
    TIM7->CR1 |= TIM_CR1_CEN;

    // stop after time
    VSL_Counter = timems;
    // this will wake audio thread if necessary
    chSemReset(&vslSem, 0);
#endif
}

/*-----------------------------------------------------------------------------*/
/** @brief      Play frequency at given amplitude and wait for completion      */
/*-----------------------------------------------------------------------------*/

void
vexAudioPlaySoundWait( int freq, int amplitude, int timems )
{
    EventListener el;

    // register event
    chEvtRegisterMask(&sound_done, &el, 1);
    // play sound
    vexAudioPlaySound( freq, amplitude, timems );

    // wait for event, 10 second timeout
    chEvtWaitAnyTimeout( ALL_EVENTS, MS2ST(10000));

    // unregister
    chEvtUnregister( &sound_done, &el );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Stop sound playing                                             */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This clears the counter used for tone duration and resets the semaphore
 *  the audio task is blocking on causing it to wake.
 */
void
vexAudioStopSound()
{
    // clear count
    VSL_Counter = 0;
    // stop chip tone
    vexAudioStopChipToneSong();
    // wake audio thread
    chSemReset(&vslSem, 0);
}

/*-----------------------------------------------------------------------------*/
/* play continuous tunes, just threw this in, not very elegant                 */
/*-----------------------------------------------------------------------------*/

// pointer for chiptone array - array is allocated dynamically
static  vsl_ct  *VSL_ctarray = NULL;

// variables for chiptone music
static  int     ctSetPtr;    // pointer for adding tones
static  int     ctPlayPtr;   // pointer for playback of tones
static  int     ctSize;      // maximum number of tones for this song
static  int     ctRepeat;    // flag indicating that the song is played in a loop
static  int     ctAmplitude; // Global amplitude for song

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize a chiptone song                                     */
/** @param[in]  len number of tones in the song                                */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Initalize a chiptone song, allocate memory and clear all variables
 *  memory will be reused for subsequent songs if there is enough space to
 *  avoid multiple calls to malloc/free
 */

int
vexAudioInitChipToneSong( int len )
{
    int     i;
    vsl_ct  *p;

    // already have an array ?
    if(VSL_ctarray != NULL)
        {
        if( ctSize < len )
            {
            chHeapFree(VSL_ctarray);
            VSL_ctarray = NULL;
            }
        }

    // allocate some storage for the chiptone array if we need to
    if(VSL_ctarray == NULL)
        VSL_ctarray = (vsl_ct *)chHeapAlloc( NULL, sizeof(vsl_ct) * (len + 1) );

    // clear everything
    ctSetPtr  = 0;
    ctSize    = 0;
    ctPlayPtr = 0;
    ctRepeat  = 0;

    // clear array
    if( VSL_ctarray != NULL )
        {
        p = VSL_ctarray;

        for(i=0;i<len;i++,p++)
            {
            p->freq      = 0;
#ifdef VSL_AMP_PER_TONE
            p->amplitude = 0;
#endif
            p->timems    = 0;
            }
        ctSize    = len;
        
        return(1); // success
        }
    else
        return(0); // error
}

/*-----------------------------------------------------------------------------*/
/** @brief      Free memory occupied by a song                                 */
/*-----------------------------------------------------------------------------*/

void
vexAudioDeinitChipToneSong()
{
    // already have an array ?
    if(VSL_ctarray != NULL)
        {
        chHeapFree(VSL_ctarray);
        VSL_ctarray = NULL;
        ctSize = 0;
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start playback of the song                                     */
/*-----------------------------------------------------------------------------*/

void
vexAudioStartChipToneSong( int repeat )
{
    if( ctSize > 0 )
        {
        ctPlayPtr = 0;
        ctRepeat = repeat;
        
        vexAudioPlayNextChipTone();
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Stop playback of the song                                      */
/*-----------------------------------------------------------------------------*/

void
vexAudioStopChipToneSong()
{
    ctPlayPtr = ctSetPtr;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Add a tone to the array for the song                           */
/** @param[in]  freq desired frequency in Hz                                   */
/** @param[in]  amplitude desired amplitude 0 = silence, 128 = max             */
/** @param[in]  timems duration of tone                                        */
/*-----------------------------------------------------------------------------*/

void
vexAudioSetNextChipTone( int freq, int amplitude, int timems )
{
    vsl_ct  *p;

    if( VSL_ctarray != NULL )
        {
        if( ctSetPtr < ctSize )
            {
            p = &(VSL_ctarray[ ctSetPtr ]);
            // amplitude of the first tone is amplitude for whole song
            // unless we are using an amplitude per tone.
            if( ctSetPtr == 0 )
                ctAmplitude = amplitude;
                
            p->freq      = freq;
#ifdef VSL_AMP_PER_TONE
            p->amplitude = amplitude;
#endif
            p->timems    = timems;

            ctSetPtr++;
            }
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Print out the song data for debug                              */
/*-----------------------------------------------------------------------------*/

void
vexAudioDebugChipTone()
{
    int i;
    vsl_ct  *p;

    if( VSL_ctarray != NULL )
        {
        for(i=0;i<ctSize;i++)
            {
            p = &(VSL_ctarray[ i ]);

#ifdef VSL_AMP_PER_TONE
            vex_printf("%d %d %d\r\n", p->freq, p->amplitude, p->timems );
#else
            vex_printf("%d %d %d\r\n", p->freq, ctAmplitude, p->timems );
#endif
            }
        }

    return;
}

/*-----------------------------------------------------------------------------*/
/*  Private functions                                                          */
/*-----------------------------------------------------------------------------*/
/** @cond                                                                      */

int
vexAudioPlayNextChipTone()
{
    vsl_ct  *p;
    int  rv = 0;

    // check for tones
    if( ctSize == 0 )
        return(0);

    // check array is valid
    if( VSL_ctarray != NULL )
        {
        if( ctPlayPtr < ctSetPtr )
            {
            p = &(VSL_ctarray[ ctPlayPtr ]);
            
#ifdef VSL_AMP_PER_TONE
            vexAudioPlaySound( p->freq, p->amplitude, p->timems );
#else
            vexAudioPlaySound( p->freq, ctAmplitude, p->timems );
#endif
                       
            ctPlayPtr++;

            // repeat ?
            if( ctPlayPtr == ctSetPtr )
                {
                if( ctRepeat )
                    ctPlayPtr = 0;
                }

            rv = 1;
            }
        }

    return(rv);
} 

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*  Small function to factorize a given value into two components suitable     */
/*  for the timer, a prescale value and a period value.                        */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

void
VSL_Factorize(int n, int *f1, int *f2)
{
    int d = 2;
  
    *f1 = *f2 = 1;
  
    if(n < 2)
        return;
  
    // while the factor being tested
    // is lower than the number to factorize
    while(d < n)
        {
        // if valid prime factor
        if(n % d == 0)
            {
            if( (*f1)*d < 256 )
                (*f1) *= d;
            else
                (*f2) *= d;
        
            n /= d;
            }
        // else: invalid prime factor
        else
            {
            if(d == 2)
                d = 3;
            else
                d += 2;
            }
        } 
    
    (*f2) *= d;
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*  Create a sine wave at a given amplitude                                    */
/*  amplitude can vary from 0 to 255                                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

static float sinPreCalc[32] = {
        0.00000, 0.19509, 0.38268, 0.55557, 0.70711, 0.83147, 0.92388, 0.98079,
        1.00000, 0.98079, 0.92388, 0.83147, 0.70711, 0.55557, 0.38268, 0.19509,
        0.00000,-0.19509,-0.38268,-0.55557,-0.70711,-0.83147,-0.92388,-0.98079,
       -1.00000,-0.98079,-0.92388,-0.83147,-0.70711,-0.55557,-0.38268,-0.19509
        };

void
VSL_CreateSineWave( int amplitude )
{
    int     i;
    //double  r;
    double  x;

    // check amplitude
    if((amplitude < MIN_AMPLITUDE) || (amplitude > MAX_AMPLITUDE))
        return;

    // Create sine wave with WAVE_SAMPLES points
    for(i=0;i<WAVE_SAMPLES;i++)
        {
        //r = (2 * 3.141592) * ( i / (double)WAVE_SAMPLES );
        //x = (sin(r) * (amplitude<<3)) + (amplitude<<3);

        // Use precalculated value to save using sin each time
        // This assumes that WAVE_SAMPLES is 32
        x = (sinPreCalc[i] * (amplitude<<3)) + (amplitude<<3);
        Sine12bit[i] = x;
        }
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*  Initialize Timer7                                                          */
/*                                                                             */
/*  We are using timer 7 for the DAC with a variable frequency rather than a   */
/*  fixed frequency with variable data                                         */
/*-----------------------------------------------------------------------------*/

void
VSL_InitTimer( int prescale, int period )
{
    // Not available on the Olimex eval card
#if defined (STM32F10X_HD)

    uint16_t tmpcr1 = 0;

    // TIM7 Peripheral clock enable
    rccEnableAPB1(RCC_APB1ENR_TIM7EN, FALSE );

    // disable timer
    TIM7->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));

    // check parameters
    if( (period == 0) || (prescale == 0) )
        return;
  
    // Time base configuration
    tmpcr1 = TIM7->CR1;
    // Set the clock division - happens to be 0
    tmpcr1 &= (uint16_t)(~((uint16_t)TIM_CR1_CKD));
    tmpcr1 |= (uint32_t)0;
    TIM7->CR1 = tmpcr1;

    // Set the Autoreload value
    TIM7->ARR = period - 1;
    // Set the Prescaler value
    TIM7->PSC = prescale - 1;
    // Generate an update event to reload the Prescaler and the Repetition counter
    // values immediately
    TIM7->EGR = TIM_PSCReloadMode_Immediate;

    // TIM7 TRGO selection
    // Reset the MMS Bits
    TIM7->CR2 &= (uint16_t)~((uint16_t)TIM_CR2_MMS);
    // Select the TRGO source
    TIM7->CR2 |=  TIM_TRGOSource_Update;
#endif
}

/*-----------------------------------------------------------------------------*/
/*  Init the DAC                                                               */
/*-----------------------------------------------------------------------------*/

void
VSL_Init()
{
    uint32_t tmpreg1 = 0, tmpreg2 = 0;

    // Not available on the Olimex eval card
#if defined (STM32F10X_HD)

    // DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration
    // Once the DAC channel is enabled, the corresponding GPIO pin is automatically 
    // connected to the DAC converter. In order to avoid parasitic consumption, 
    // the GPIO pin should be configured in analog
    // GPIO config now done elsewhere.

    // create a default waveform
    VSL_CreateSineWave( DEFAULT_AMPLITUDE );
 
    // Init timer for 1KHz
    VSL_InitTimer(9, 250);

    // DAC Peripheral clock enable
    rccEnableAPB1((RCC_APB1ENR_DACEN), FALSE );

    // DAC channel1 Configuration
    // Get the DAC CR value */
    tmpreg1 = DAC->CR;
    // Clear BOFFx, TENx, TSELx, WAVEx and MAMPx bits */
    tmpreg1 &= ~(CR_CLEAR_MASK << DAC_Channel_1);
    // Configure for the selected DAC channel: buffer output, trigger, wave generation,
    tmpreg2 = (DAC_Trigger_T7_TRGO | DAC_WaveGeneration_None | DAC_LFSRUnmask_Bit0 | DAC_OutputBuffer_Disable);
    // Calculate CR register value depending on DAC_Channel */
    tmpreg1 |= tmpreg2 << DAC_Channel_1;
    // Write to DAC CR */
    DAC->CR = tmpreg1;

    // DMA config
    if( dmaStreamAllocate(vsl_dma, 0, NULL, NULL) == FALSE )
        {
        uint32_t tmpreg = 0;

        tmpreg |= STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_CIRC |
            STM32_DMA_CR_PSIZE_HWORD | STM32_DMA_CR_MSIZE_HWORD |
            STM32_DMA_CR_PL(2) | STM32_DMA_CR_MINC;

        dmaStreamSetMode(vsl_dma, tmpreg);

        dmaStreamSetTransactionSize(vsl_dma, WAVE_SAMPLES);
        dmaStreamSetPeripheral(vsl_dma, &DAC->DHR12R1);
        dmaStreamSetMemory0( vsl_dma, (uint32_t)&Sine12bit );

        // Enable DMA2 Channel 3
        dmaStreamEnable(vsl_dma);

        // Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is
        // automatically connected to the DAC converter.
        DAC->CR |= (DAC_CR_EN1 << DAC_Channel_1);

        // Enable DMA for DAC Channel1
        DAC->CR |= (DAC_CR_DMAEN1 << DAC_Channel_1);
        }
#endif
}  

/*-----------------------------------------------------------------------------*/
/*  Stop the DAC, leave the DMA stream for now                                 */
/*-----------------------------------------------------------------------------*/

void
VSL_Deinit()
{
    // disable timer
    TIM7->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
    // disable DAC
    DAC->CR &= ~(DAC_CR_EN1 << DAC_Channel_1);
    // clear output
    DAC->DHR12R1 = 0;
}








