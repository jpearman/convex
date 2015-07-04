/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                 2013-2015                                   */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vex_printf.h                                                 */
/*    Author:     James Pearman                                                */
/*    Created:    7 May 2013                                                   */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     4 July 2013 - Initial release                      */
/*                         12 Oct  2013 - Thread safe version                  */
/*                         13 Mar  2015 - Bug fix in dbl2stri                  */
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
/*    This version for chibios based on the stdarg version by Georges Menie    */
/*    with floating point support from the Daniel D Miller version.            */
/*    termf etc. renamed to be consistent with the vex cortex library          */
/*                                                                             */
/*                                                                             */
/*    This version is not thread safe !                                        */
/*      but neither was the original really.....                               */
/*    anyway, some large arrays are now declared static, they were using       */
/*    too much stack, problem beiung every thread now needs large stack.       */
/*    removed the local buffer from dbl2stri, never was going to be used       */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Copyright 2001, 2002 Georges Menie (www.menie.org)                       */
/*    stdarg version contributed by Christian Ettinger                         */
/*                                                                             */
/*    This program is free software; you can redistribute it and/or modify     */
/*    it under the terms of the GNU Lesser General Public License as published */
/*    by the Free Software Foundation; either version 2 of the License, or     */
/*    (at your option) any later version.                                      */
/*                                                                             */
/*    This program is distributed in the hope that it will be useful,          */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*    GNU Lesser General Public License for more details.                      */
/*                                                                             */
/*    You should have received a copy of the GNU Lesser General Public License */
/*    along with this program; if not, write to the Free Software              */
/*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Updated by Daniel D Miller.  Changes to the original Menie code are      */
/*    Copyright 2009-2013 Daniel D Miller                                      */
/*    All such changes are distributed under the same license as the original, */
/*    as described above.                                                      */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    This version now has a data structure holding what were the              */
/*    globals variables.  Separate data structures are used for vex_printf and */
/*    vex_sprintf. A mutex is used to make access to these thread safe, printf */
/*    was made separate so that it did not block sprintf when the serial       */
/*    output port blocked.                                                     */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#include <string.h>
#include <stdarg.h>

#include "ch.h"         // needs for all ChibiOS programs
#include "hal.h"        // hardware abstraction layer header
#include "vex.h"        // vex library header

#if (CH_KERNEL_MAJOR >= 2) && (CH_KERNEL_MINOR >= 6)
#define      vexStreamPut(stream, c)    chSequentialStreamPut( stream, c );
#else
#define      vexStreamPut(stream, c)    chIOPut( stream, c );
#endif

/*-----------------------------------------------------------------------------*/
/** @file    vexprintf.c
  * @brief   Lightweight printf with float support
*//*---------------------------------------------------------------------------*/

// the following should be enough for 32 bit int
// it's not enough for a 32 bit binary if we add that later
#define PRINT_BUF_LEN 16

typedef struct _pdefs {
    // this need to be the first element so we can easily initialize the structure
    // with one value
    int           init;

    // Handle to output buffer
    char          **out;
    
    // flags that determine formatting
    int           use_leading_plus;
    int           max_output_len;
    int           curr_output_len;

    // buffer to assemble output    
    char          print_buf[PRINT_BUF_LEN];
    char          float_buf[PRINT_BUF_LEN*2];
    
    // mutex or semaphore to protect this structure if more
    // than one thread tries to use it simulateously
#if CH_USE_MUTEXES
    Mutex         mutex;
#elif CH_USE_SEMAPHORES
    Semaphore     semaphore;
#endif
    short         txcount;
    unsigned long lasttime;
    unsigned long delta;

    // vexStream added so we can "printf" to the other serial ports without
    // using sprintf.  This allows all the old chprintf calls to be replaced.
    vexStream     *chp;
} pdefs;

static  pdefs   pdefs1 = {0};
static  pdefs   pdefs2 = {0};

/*-----------------------------------------------------------------------------*/
/*  Constants for floating point rounding                                      */
/*-----------------------------------------------------------------------------*/

static const double round_nums[8] = {
        0.5,
        0.05,
        0.005,
        0.0005,
        0.00005,
        0.000005,
        0.0000005,
        0.00000005
} ;


/*-----------------------------------------------------------------------------*/
/** @brief      acquire lock on print variables                                */
/** @param[in]  p pointer to our working variables, a pdef structure           */
/*-----------------------------------------------------------------------------*/

inline void
vex_print_acquire( pdefs *p )
{
    if(!p->init)
        {
        p->init = 1;
        
#if CH_USE_MUTEXES
    chMtxInit(&p->mutex);
#elif CH_USE_SEMAPHORES
    chSemInit(&p->semaphore, 0);
#endif
        // we init some other variables here as well
        p->txcount  = 0;
        p->lasttime = 0;
        p->chp      = (vexStream *)SD_CONSOLE;
        }
        
#if CH_USE_MUTEXES
    chMtxLock(&p->mutex);
#elif CH_USE_SEMAPHORES
    chSemWait(&p->semaphore);
#endif
}

/*-----------------------------------------------------------------------------*/
/** @brief      release lock on print variables                                */
/** @param[in]  p pointer to our working variables, a pdef structure           */
/*-----------------------------------------------------------------------------*/

inline void
vex_print_release( pdefs *p )
{
    (void)p;
    
#if CH_USE_MUTEXES
    chMtxUnlock();
#elif CH_USE_SEMAPHORES
    chSemSignal(&p->semaphore);
#endif
}

/*-----------------------------------------------------------------------------*/
/** @brief      move one character to output                                   */
/** @param[in]  p pointer to our working variables, a pdef structure           */
/** @param[in]  c the character to output                                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function moves one character to the output buffer, if the buffer is NULL
 *  then the character is sent to the standard console
 */

// minimum time in which to send 128 characters
#define THROTTLE_DELAY      30
// quarter of above rounded up to nearest integer
#define THROTTLE_DELAY_4    8

static void
vex_printc ( pdefs *p, int c )
{
    // reached maximum length of output buffer ?
    if (p->max_output_len >= 0  &&  p->curr_output_len >= p->max_output_len)
        return ;

    // is output buffer NULL ?
    if (p->out) {
        **p->out = (char) c;
        ++(*p->out);
        p->curr_output_len++ ;
    }
    else {
        // calculate delta, the time since we were last here
        p->delta = chTimeNow() - p->lasttime;

        // If we were here recently, gap is less then THROTTLE_DELAY
        if( p->delta < THROTTLE_DELAY )
            {
            // but did we have a significant gap ?
            if( p->delta > THROTTLE_DELAY_4 )
                {
                // if so reduce accumulated count
                // allow 4 chars per mS
                p->txcount -= (p->delta * 4);
                // limit to 0
                if( p->txcount < 0 )
                    p->txcount = 0;
                }

            // Have we reached the limit for transmission in this period
            if( ++p->txcount == 128 )
                {
                // we know p->delta is less than THROTTLE_DELAY
                // minimum sleep time will be 1mS but most of the
                // time it will be THROTTLE_DELAY mS
                chThdSleepMilliseconds( THROTTLE_DELAY - p->delta );
                p->txcount = 0;
                }
            }
        else
            // Enough time from the last character so start over
            p->txcount = 0;

        // remember time of this transmit
        p->lasttime = chTimeNow();

        // send next character
        p->curr_output_len++ ;
        vexStreamPut( p->chp, (uint8_t)c);
    }
}

/*-----------------------------------------------------------------------------*/
/** @brief      move string to output with padding                             */
/** @param[in]  p pointer to our working variables, a pdef structure           */
/** @param[in]  string the string to output                                    */
/** @param[in]  width required output width                                    */
/** @param[in]  pad type of padding, left, right, zeros                        */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function moves a string into the output buffer with required padding
 */
#define  PAD_NORMAL  0
#define  PAD_RIGHT   1
#define  PAD_ZERO    2

static int
vex_prints ( pdefs *p, const char *string, int width, int pad )
{
    register int pc = 0, padchar = ' ';

    if (width > 0) {
        int len = 0;
        const char *ptr;
        for (ptr = string; *ptr; ++ptr)
            ++len;
        if (len >= width)
            width = 0;
        else
            width -= len;
        if (pad & PAD_ZERO)
            padchar = '0';
    }
    if (!(pad & PAD_RIGHT)) {
        for (; width > 0; --width) {
            vex_printc ( p, padchar );
            ++pc;
        }
    }
    for (; *string; ++string) {
        vex_printc ( p, *string );
        ++pc;
    }
    for (; width > 0; --width) {
        vex_printc ( p, padchar );
        ++pc;
    }
    return pc;
}

/*-----------------------------------------------------------------------------*/
/** @brief      convert one integer to ascii representation                    */
/** @param[in]  p pointer to our working variables, a pdef structure           */
/** @param[in]  base number base, 10, 16 etc.                                  */
/** @param[in]  sign is number signed or unsigned, 1 = signed                  */
/** @param[in]  pad type of padding, left, right, zeros                        */
/** @param[in]  letbase base ascii character for conversion, 'a' or 'A'        */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function converts one integer to formatted output
 */
static int
vex_printi ( pdefs *p, int i, uint16_t base, int sign, int width, int pad, int letbase )
{
    register char *s;
    int t, neg = 0, pc = 0;
    unsigned u = (unsigned) i;

    if (i == 0)
        return vex_prints ( p, "0", width, pad );

    if (sign && base == 10 && i < 0)
        {
        neg = 1;
        u = (unsigned) -i;
        }
    //  make sure print_buf is NULL-term
    s = p->print_buf + PRINT_BUF_LEN - 1;
    *s = '\0';

    while (u)
        {
        t = u % base;
        if (t >= 10)
            t += letbase - '0' - 10;
        *--s = (char) t + '0';
        u /= base;
        }

    if (neg)
        {
        if (width && (pad & PAD_ZERO))
            {
            vex_printc ( p, '-' );
            ++pc;
            --width;
            }
        else
            {
            *--s = '-';
            }
        }
    else
        {
        if (p->use_leading_plus) {
            *--s = '+';
        }
    }

    return pc + vex_prints ( p, s, width, pad );
}

/*-----------------------------------------------------------------------------*/
/** @brief      convert one double to ascii representation                     */
/** @param[in]  p pointer to our working variables, a pdef structure           */
/** @param[in]  dbl input value                                                */
/** @param[in]  dec_width required number of fractional digits                 */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function converts one double to a string
 *  it's declared inline as it is only called from vex_printdbl
 */
static inline void
dbl2stri( pdefs *p, double dbl, unsigned dec_digits )
{
    char *output = p->float_buf;
        
    //  extract negative info
    if (dbl < 0.0) {
        *output++ = '-' ;
        dbl *= -1.0 ;
    } else {
        if (p->use_leading_plus) {
            *output++ = '+' ;
        }
    }

    //  handling rounding by adding .5LSB to the floating-point data
    if (dec_digits < 8) {
        dbl += round_nums[dec_digits] ;
    }

    //  construct fractional multiplier for specified number of digits.
    uint32_t mult = 1 ;
    uint16_t idx ;
    for (idx=0; idx < dec_digits; idx++)
        mult *= 10 ;

    uint32_t wholeNum   = (uint32_t) dbl ;
    uint32_t decimalNum = (uint32_t) ((dbl - wholeNum) * mult);

    // convert integer portion
    // this creates the number in reverse in print_buf
    idx = 0 ;
    while (wholeNum != 0) {
        p->print_buf[idx++] = '0' + (wholeNum % 10) ;
        wholeNum /= 10 ;
    }

    // move integer part to output buffer
    if (idx == 0) {
        *output++ = '0' ;
    } else {
        while (idx > 0) {
            *output++ = p->print_buf[idx-1] ;  //lint !e771
            idx-- ;
        }
    }
    
    // convert fractional part if necessary
    if (dec_digits > 0)
        {
        *output++ = '.' ;

        // convert fractional portion
        // this creates the number in reverse in print_buf
        idx = 0 ;
        while (decimalNum != 0) {
            p->print_buf[idx++] = '0' + (decimalNum % 10) ;
            decimalNum /= 10 ;
        }
        
        //  pad the decimal portion with 0s as necessary;
        //  We wouldn't want to report 3.093 as 3.93, would we??
        while (idx < dec_digits) {
            p->print_buf[idx++] = '0' ;
        }

        // move fractional part to output buffer
        if (idx == 0) {
            *output++ = '0' ;
        } else {
            while (idx > 0) {
                *output++ = p->print_buf[idx-1] ;
                idx-- ;
            }
        }
    }
    
    // terminating null
    *output = 0 ;

    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      convert one double to ascii representation                     */
/** @param[in]  p pointer to our working variables, a pdef structure           */
/** @param[in]  width required output width                                    */
/** @param[in]  dec_digits required number of fractional digits                */
/** @param[in]  pad type of padding, left, right, zeros                        */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function converts one double to formatted output
 *  it's declared inline as it's only called from vex_print
 */
static inline int
vex_printdbl( pdefs *p, double dbl, int width, int dec_digits, int pad )
{
    dbl2stri( p, dbl, dec_digits ) ;
    
    return vex_prints ( p, p->float_buf, width, pad );
}

/*-----------------------------------------------------------------------------*/
/** @brief      created formatted output                                       */
/** @param[in]  p pointer to our working variables, a pdef structure           */
/** @param[in]  format The format string                                       */
/** @param[in]  args variable argument list                                    */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This is the function that provides the formatting for all vex_printf and
 *  related functions
 */
static int
vex_print( pdefs *p, const char *format, va_list args )
{
    int post_decimal ;
    int width, pad ;
    unsigned dec_width = 6 ;
    int pc = 0;
    char scr[2];

    p->use_leading_plus = 0 ; //  start out with this clear
    p->curr_output_len  = 0 ; // No output yet

    for (; *format != 0; ++format)
        {
        if (*format == '%')
            {
            dec_width = 6 ;
            ++format;
            width = pad = 0;

            if (*format == '\0')
                break;
            if (*format == '%')
                goto out_lbl;
            if (*format == '-') {
                ++format;
                pad = PAD_RIGHT;
            }
            if (*format == '+') {
                ++format;
                p->use_leading_plus = 1 ;
            }
            while (*format == '0') {
                ++format;
                pad |= PAD_ZERO;
            }

            post_decimal = 0 ;
            if (*format == '.' || (*format >= '0' &&  *format <= '9'))
                {
                while (1)
                    {
                    if (*format == '.')
                        {
                        post_decimal = 1 ;
                        dec_width = 0 ;
                        format++ ;
                        }
                    else
                    if ((*format >= '0' &&  *format <= '9'))
                        {
                        if (post_decimal)
                            {
                            dec_width *= 10;
                            dec_width += (uint16_t) (uint8_t) (*format - '0');
                            }
                        else
                            {
                            width *= 10;
                            width += *format - '0';
                            }
                        format++ ;
                        }
                    else
                        {
                        break;
                        }
                    }
                }

            if (*format == 'l')
                ++format;
            switch (*format)
                {
                case 's':
                    {
                    char *s = (char *)va_arg( args, int );
                    pc += vex_prints ( p, s ? s : "(null)", width, pad );
                    p->use_leading_plus = 0 ;  //  reset this flag after printing one value
                    }
                    break;
                case 'd':
                    pc += vex_printi ( p, va_arg( args, int ), 10, 1, width, pad, 'a');
                    p->use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;
                case 'x':
                    pc += vex_printi ( p, va_arg( args, int ), 16, 0, width, pad, 'a');
                    p->use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;
                case 'X':
                    pc += vex_printi ( p, va_arg( args, int ), 16, 0, width, pad, 'A');
                    p->use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;
                case 'p':
                case 'u':
                    pc += vex_printi ( p, va_arg( args, int ), 10, 0, width, pad, 'a');
                    p->use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;
                case 'c':
                    // convert char to string/
                    scr[0] = (char)va_arg( args, int );
                    scr[1] = '\0';
                    pc += vex_prints ( p, scr, width, pad );
                    p->use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;

                case 'f':
                    {
                    vex_printdbl(p, va_arg( args, double ), width, dec_width, pad ) ;
                    p->use_leading_plus = 0 ;  //  reset this flag after printing one value
                    }
                    break;

                default:
                    vex_printc ( p, '%' );
                    vex_printc ( p, *format );
                    p->use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;
                }
            }
        else
            {
            out_lbl:
            vex_printc ( p, *format );
            ++pc;
            }
        }  //  for each char in format string

    if (p->out)
        **p->out = '\0';

    // release mutex/semaphore
    vex_print_release(p);

    return pc;
}

/*-----------------------------------------------------------------------------*/
/** @brief      send formated string to the console                            */
/** @param[in]  format The format string                                       */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf
 */
int vex_printf (const char *format, ...)
{
    va_list args;

    vex_print_acquire(&pdefs1);

    pdefs1.out = 0;
    pdefs1.max_output_len = -1 ;
    pdefs1.chp = (vexStream *)SD_CONSOLE;

    va_start( args, format );
    return vex_print( &pdefs1, format, args );
}

/*-----------------------------------------------------------------------------*/
/** @brief      send formated string to a vexStream                            */
/** @param[in]  chp The stream                                                 */
/** @param[in]  format The format string                                       */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function is used as a direct replacement for chprintf. It has similar
 *  but reduced functionality to the standard library function sprintf.
 */
int vex_chprintf ( vexStream *chp, const char *format, ...)
{
    va_list args;

    vex_print_acquire(&pdefs1);

    pdefs1.out = 0;
    pdefs1.max_output_len = -1 ;
    pdefs1.chp = chp;

    va_start( args, format );
    return vex_print( &pdefs1, format, args );
}

/*-----------------------------------------------------------------------------*/
/** @brief      send formated string to a vexStream                            */
/** @param[in]  format The format string                                       */
/** @param[in]  args variable argument list                                    */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf.  This version takes a variable argument list directly.
 */
int vex_vprintf(const char *format, va_list args)
{
    vex_print_acquire(&pdefs1);

    pdefs1.out = 0;
    pdefs1.max_output_len = -1;
    pdefs1.chp = (vexStream *)SD_CONSOLE;

    return vex_print(&pdefs1, format, args);
}

/*-----------------------------------------------------------------------------*/
/** @brief      send formated string to a vexStream                            */
/** @param[in]  chp The stream                                                 */
/** @param[in]  format The format string                                       */
/** @param[in]  args variable argument list                                    */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf.  This version takes a variable argument list directly and can also
 *  output to a stream that is not the CONSOLE.
 */
int vex_vchprintf(vexStream *chp, const char *format, va_list args)
{
    vex_print_acquire(&pdefs1);

    pdefs1.out = 0;
    pdefs1.max_output_len = -1;
    pdefs1.chp = chp;

    return vex_print(&pdefs1, format, args);
}

/*-----------------------------------------------------------------------------*/
/** @brief      create formated string in a buffer                             */
/** @param[out] out The output buffer                                          */
/** @param[in]  format The format string                                       */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf
 */
int vex_sprintf (char *out, const char *format, ...)
{
    va_list args;

    vex_print_acquire(&pdefs2);

    pdefs2.out = &out;
    pdefs2.max_output_len = -1 ;

    va_start( args, format );
    return vex_print( &pdefs2, format, args );
}

/*-----------------------------------------------------------------------------*/
/** @brief      create formated string in a buffer                             */
/** @param[out] out The output buffer                                          */
/** @param[out] max_len The output buffer length                               */
/** @param[in]  format The format string                                       */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf
 */
int vex_snprintf(char *out, uint16_t max_len, const char *format, ...)
{
    int pc;

    va_list args;

    vex_print_acquire(&pdefs2);

    pdefs2.out = &out;
    pdefs2.max_output_len = (int) max_len ;

    va_start( args, format );

    pc = vex_print( &pdefs2, format, args );

    // maximum will have been limited to max)len
    if(pc > max_len) pc = max_len;

    return pc;
}

/*-----------------------------------------------------------------------------*/
/** @brief      create formated string in a buffer from a va_list              */
/** @param[out] out The output buffer                                          */
/** @param[in]  format The format string                                       */
/** @param[in]  args a variable argument list                                  */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf
 */
int vex_vsprintf( char *out, const char *format, va_list args )
{
    vex_print_acquire(&pdefs2);

    pdefs2.out = &out;
    pdefs2.max_output_len = -1 ;

    return vex_print( &pdefs2, format, args );
}

/*-----------------------------------------------------------------------------*/
/** @brief      create formated string in a buffer from a va_list              */
/** @param[out] out The output buffer                                          */
/** @param[out] max_len The output buffer length                               */
/** @param[in]  format The format string                                       */
/** @param[in]  args a variable argument list                                  */
/** @returns    the number of characters that were output                      */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf
 */
int vex_vsnprintf(char *out, uint16_t max_len, const char *format, va_list args )
{
    int pc;

    vex_print_acquire(&pdefs2);

    pdefs2.out = &out;
    pdefs2.max_output_len = (int) max_len ;

    pc = vex_print( &pdefs2, format, args );

    // maximum will have been limited to max)len
    if(pc > max_len) pc = max_len;

    return pc;
}
