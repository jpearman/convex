/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
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

#include <string.h>
#include <stdarg.h>

#include "ch.h"
#include "hal.h"
#include "vex.h"

/*-----------------------------------------------------------------------------*/
/** @file    vexprintf.c
  * @brief   Lightweight printf with float support
*//*---------------------------------------------------------------------------*/

static uint16_t use_leading_plus = 0 ;

static int max_output_len = -1 ;
static int curr_output_len = 0 ;

#define FLOAT_BUF_SIZE      32

//****************************************************************************
static void
vex_printc (char **str, int c)
{
    if (max_output_len >= 0  &&  curr_output_len >= max_output_len)
        return ;
    if (str) {
        **str = (char) c;
        ++(*str);
        curr_output_len++ ;
    }
    else {
        curr_output_len++ ;
        sdPut(SD_CONSOLE, (uint8_t)c);
    }
}

//****************************************************************************
//  This version returns the length of the output string.
//  It is more useful when implementing a walking-string function.
//****************************************************************************
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

static unsigned
dbl2stri(char *outbfr, double dbl, unsigned dec_digits)
{
    char *output = outbfr ;
    static  char tbfr[FLOAT_BUF_SIZE]; // used to be on stack

    // there used to be a local buffer, but what was the point ?
    if( output == NULL )
        return(0);

    //*******************************************
    //  extract negative info
    //*******************************************
    if (dbl < 0.0) {
        *output++ = '-' ;
        dbl *= -1.0 ;
    } else {
        if (use_leading_plus) {
            *output++ = '+' ;
        }
    }

    //  handling rounding by adding .5LSB to the floating-point data
    if (dec_digits < 8) {
        dbl += round_nums[dec_digits] ;
    }

    //**************************************************************************
    //  construct fractional multiplier for specified number of digits.
    //**************************************************************************
    uint16_t mult = 1 ;
    uint16_t idx ;
    for (idx=0; idx < dec_digits; idx++)
        mult *= 10 ;

    uint16_t wholeNum   = (uint16_t) dbl ;
    uint16_t decimalNum = (uint16_t) ((dbl - wholeNum) * mult);

    //*******************************************
    //  convert integer portion
    //*******************************************

    idx = 0 ;
    while (wholeNum != 0) {
        tbfr[idx++] = '0' + (wholeNum % 10) ;
        wholeNum /= 10 ;
    }

    if (idx == 0) {
        *output++ = '0' ;
    } else {
        while (idx > 0) {
            *output++ = tbfr[idx-1] ;  //lint !e771
            idx-- ;
        }
    }
    if (dec_digits > 0) {
        *output++ = '.' ;

        //*******************************************
        //  convert fractional portion
        //*******************************************
        idx = 0 ;
        while (decimalNum != 0) {
            tbfr[idx++] = '0' + (decimalNum % 10) ;
            decimalNum /= 10 ;
        }
        //  pad the decimal portion with 0s as necessary;
        //  We wouldn't want to report 3.093 as 3.93, would we??
        while (idx < dec_digits) {
            tbfr[idx++] = '0' ;
        }

        if (idx == 0) {
            *output++ = '0' ;
        } else {
            while (idx > 0) {
                *output++ = tbfr[idx-1] ;
                idx-- ;
            }
        }
    }
    *output = 0 ;

    //  prepare output
    output = outbfr ;

    return strlen(output) ;
}

//****************************************************************************
#define  PAD_RIGHT   1
#define  PAD_ZERO    2

static int
vex_prints (char **out, const char *string, int width, int pad)
{
    register int pc = 0, padchar = ' ';

    if (width > 0){
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
            vex_printc (out, padchar);
            ++pc;
        }
    }
    for (; *string; ++string) {
        vex_printc (out, *string);
        ++pc;
    }
    for (; width > 0; --width) {
        vex_printc (out, padchar);
        ++pc;
    }
    return pc;
}

//****************************************************************************
// the following should be enough for 32 bit int
#define PRINT_BUF_LEN 16

static int
vex_printi (char **out, int i, uint16_t base, int sign, int width, int pad, int letbase)
{
    static  char print_buf[PRINT_BUF_LEN];
    char *s;
    int t, neg = 0, pc = 0;
    unsigned u = (unsigned) i;

    if (i == 0)
        {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return vex_prints (out, print_buf, width, pad);
        }

    if (sign && base == 10 && i < 0)
        {
        neg = 1;
        u = (unsigned) -i;
        }
    //  make sure print_buf is NULL-term
    s = print_buf + PRINT_BUF_LEN - 1;
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
            vex_printc (out, '-');
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
        if (use_leading_plus) {
            *--s = '+';
        }
    }

    return pc + vex_prints (out, s, width, pad);
}

//****************************************************************************
static int
vex_print(char **out, const char *format, va_list args )
{
    int post_decimal ;
    int width, pad ;
    unsigned dec_width = 6 ;
    int pc = 0;
    char scr[2];
    static  char fbuffer[FLOAT_BUF_SIZE] ;

    use_leading_plus = 0 ;  //  start out with this clear

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
                use_leading_plus = 1 ;
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
                    pc += vex_prints (out, s ? s : "(null)", width, pad);
                    use_leading_plus = 0 ;  //  reset this flag after printing one value
                    }
                    break;
                case 'd':
                    pc += vex_printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
                    use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;
                case 'x':
                    pc += vex_printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
                    use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;
                case 'X':
                    pc += vex_printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
                    use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;
                case 'p':
                case 'u':
                    pc += vex_printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
                    use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;
                case 'c':
                    // convert char to string/
                    scr[0] = (char)va_arg( args, int );
                    scr[1] = '\0';
                    pc += vex_prints (out, scr, width, pad);
                    use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;

                case 'f':
                    {
                    double dbl = va_arg( args, double );

                    // convert the double to a string
                    dbl2stri(fbuffer, dbl, dec_width) ;

                    // print the string
                    pc += vex_prints (out, fbuffer, width, pad);
                    use_leading_plus = 0 ;  //  reset this flag after printing one value
                    }
                    break;

                default:
                    vex_printc (out, '%');
                    vex_printc (out, *format);
                    use_leading_plus = 0 ;  //  reset this flag after printing one value
                    break;
                }
            }
        else
            {
            out_lbl:
            vex_printc (out, *format);
            ++pc;
            }
        }  //  for each char in format string

    if (out)
        **out = '\0';

    return pc;
}

/*-----------------------------------------------------------------------------*/
/** @brief      send formated string to the console                            */
/** @param[in]  format The format string                                       */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf
 */
int vex_printf (const char *format, ...)
{
    va_list args;

    max_output_len = -1 ;
    curr_output_len = 0 ;

    va_start( args, format );
    return vex_print( 0, format, args );
}

/*-----------------------------------------------------------------------------*/
/** @brief      create formated string in a buffer                             */
/** @param[out] out The output buffer                                          */
/** @param[in]  format The format string                                       */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf
 */
int vex_sprintf (char *out, const char *format, ...)
{
    va_list args;

    max_output_len = -1 ;
    curr_output_len = 0 ;

    va_start( args, format );
    return vex_print( &out, format, args );
}

/*-----------------------------------------------------------------------------*/
/** @brief      create formated string in a buffer                             */
/** @param[out] out The output buffer                                          */
/** @param[out] max_len The output buffer length                               */
/** @param[in]  format The format string                                       */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf
 */
int vex_snprintf(char *out, uint16_t max_len, const char *format, ...)
{
    va_list args;

    max_output_len = (int) max_len ;
    curr_output_len = 0 ;

    va_start( args, format );
    return vex_print( &out, format, args );
}

/*-----------------------------------------------------------------------------*/
/** @brief      create formated string in a buffer from a va_list              */
/** @param[out] out The output buffer                                          */
/** @param[in]  format The format string                                       */
/** @param[in]  args a variable argument list                                  */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf
 */
int vex_vsprintf( char *out, const char *format, va_list args )
{
    max_output_len = -1 ;
    curr_output_len = 0 ;

    return vex_print( &out, format, args );
}

/*-----------------------------------------------------------------------------*/
/** @brief      create formated string in a buffer from a va_list              */
/** @param[out] out The output buffer                                          */
/** @param[out] max_len The output buffer length                               */
/** @param[in]  format The format string                                       */
/** @param[in]  args a variable argument list                                  */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This has similar but reduced functionality to the standard library function
 *  sprintf
 */
int vex_vsnprintf(char *out, uint16_t max_len, const char *format, va_list args )
{
    max_output_len = (int) max_len ;
    curr_output_len = 0 ;

    return vex_print( &out, format, args );
}
