/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2012                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexrtttl.c                                                   */
/*    Author:     James Pearman                                                */
/*    Created:    26 April 2012                                                */
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
/*        Description:                                                         */
/*                                                                             */
/*        RTTTL player based on code by Michael Ringgaard                      */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/* Ringing Tones Text Transfer Language (RTTTL) player                         */
/*                                                                             */
/* Copyright (C) 2002 Michael Ringgaard. All rights reserved.                  */
/*                                                                             */
/* Redistribution and use in source and binary forms, with or without          */
/* modification, are permitted provided that the following conditions          */
/* are met:                                                                    */
/*                                                                             */
/* 1. Redistributions of source code must retain the above copyright           */
/*    notice, this list of conditions and the following disclaimer.            */
/* 2. Redistributions in binary form must reproduce the above copyright        */
/*    notice, this list of conditions and the following disclaimer in the      */
/*    documentation and/or other materials provided with the distribution.     */
/* 3. Neither the name of the project nor the names of its contributors        */
/*    may be used to endorse or promote products derived from this software    */
/*    without specific prior written permission.                               */
/*                                                                             */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" */
/* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  */
/* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   */
/* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         */
/* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        */
/* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    */
/* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     */
/* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     */
/* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF      */
/* THE POSSIBILITY OF SUCH DAMAGE.                                             */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

/*-----------------------------------------------------------------------------*/
/** @file    vexrttl.c
  * @brief   Read rtttl song and setup cortex for background playback
*//*---------------------------------------------------------------------------*/

static int notefreq[12] = {4186, 4434, 4698, 4978, 5274, 5587,
                    5919, 6271, 6644, 7040, 7458, 7902 };

static char name[48];
   
static int
note2freq(int note)
{
  return notefreq[note % 12] / (1 << (9 - (note / 12)));
}

/*-----------------------------------------------------------------------------*/
/** @brief      Play ringtone string using the vsl sound library               */
/** @param[in]  song Pointer to buffer with rtttl sound data                   */
/** @param[in]  amplitude volume to play song at                               */
/** @param[in]  repeat flag indicating song is played continuously in a loop   */
/** @returns    char * to name of song                                         */
/*-----------------------------------------------------------------------------*/

char *
vexAudioPlayRtttl( char *song , int amplitude, int repeat )
{
    char *p = song;
    char *q;
    int  defdur = 4;
    int  defscale = 6;
    int  bpm = 63;
    int  count = 0;
    
    // Skip name - actually save now for return
    q = name;
    while (*p && *p != ':')
        {
        *q++ = *p++;
        }
    *q = 0;

    if (!*p) return("");
    p++;

    // Parse defaults
    while (*p)
        {
        char param;
        int value;

        while (*p == ' ') p++;
        if (!*p) return("");
        if (*p == ':') break;

        param = *p++;
        if (*p != '=') return("");
    
        p++;
        value = 0;
        while (*p >= '0' && *p <= '9') value = value * 10 + (*p++ - '0');

        switch (param)
            {
            case 'd': defdur = 32 / value; break;
            case 'o': defscale = value; break;
            case 'b': bpm = value; break;
            }

        while (*p == ' ') p++;
        if (*p == ',') p++;
        }
    p++;

    // figure out how many notes in the stream
    // we figure on a comma following each notes
    for(q=p, count=1;*q;q++)
        {
        if(*q == ',')
            count++;
        }
    
    if(count > 1)
        vexAudioInitChipToneSong( count+1 );

    // parse notes
    while (*p)
        {
        int note = -1;
        int scale = defscale;
        int dur = defdur;
        int ms;
        int freq;

        // Skip whitespace
        while (*p == ' ') p++;
        if (!*p) return("");

        // Parse duration
        if (*p >= '0' && *p <= '9')
            {
            int value = 0;
            while (*p >= '0' && *p <= '9') value = value * 10 + (*p++ - '0');

            dur = 32 / value;
            }

        // Parse note
        switch (*p)
            {
            case 0: return("");
            case 'C': case 'c': note = 0; break;
            case 'D': case 'd': note = 2; break;
            case 'E': case 'e': note = 4; break;
            case 'F': case 'f': note = 5; break;
            case 'G': case 'g': note = 7; break;
            case 'A': case 'a': note = 9; break;
            case 'H': case 'h': note = 11; break;
            case 'B': case 'b': note = 11; break;
            case 'P': case 'p': note = -1; break;
            }
        p++;
        if (*p == '#')
            {
            note++;
            p++;
            }
        if (*p == 'b')
            {
            note--;
            p++;
            }

         // Parse special duration
        if (*p == '.')
            {
            dur += dur / 2;
            p++;
            }

        // Parse scale
        if (*p >= '0' && *p <= '9') scale = (*p++ - '0');

        // Parse special duration (again...)
        if (*p == '.')
           {
          dur += dur / 2;
          p++;
           }

        // Skip delimiter
        while (*p == ' ') p++;
        if (*p == ',') p++;

        // Play note
        ms = dur * 60000 / (bpm * 8);
        if (note == -1)
          freq = 0;
        else
          freq = note2freq((scale + 1) * 12 + note);

        vexAudioSetNextChipTone( freq,  amplitude, ms);
        }
     
    // 1/2 second pause at end
    vexAudioSetNextChipTone( 0,  0, 500);
   
    // start playback
    vexAudioStartChipToneSong(repeat);

    return(name);
}



