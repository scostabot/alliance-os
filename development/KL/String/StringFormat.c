/*
 * StringFormat.c:  Functions to manipulate formatted strings
 *
 * Modified from code by Lars Wirzenius & Linus Torvalds
 * (C) 1991, 1992 Linus Torvalds
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 15/11/98 ramon     1.0    First release
 * 16/11/98 ramon     1.1    Lots of comments
 * 02/05/99 scosta    1.2    Taken away typewrappers.h 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <stdarg.h>             /* Comes with egcs */
#include <klibs.h>             /* Typewrappers    */

/* These defines are the flags that a number can have      */
/* They control what it's string representation looks like */

#define ZEROPAD 1               /* pad with zero */
#define SIGN    2               /* unsigned/signed long */
#define PLUS    4               /* show plus */
#define SPACE   8               /* space if plus */
#define LEFT    16              /* left justified */
#define SPECIAL 32              /* 0x */
#define LARGE   64              /* use 'ABCDEF' instead of 'abcdef' */


/* KLisDigit() returns TRUE is the parameter is an ASCII digit */

#define KLisDigit(c) ((c) >= '0' && (c) <= '9')


/* DIVIDE(number,base) will divide number by base, and return what */
/* is left over                                                    */

#define DIVIDE(n,base) ({                            \
    DATA __res;                                      \
    __res = ((UDATA) n) % (UDATA) base;              \
    n = ((UDATA) n) / (UDATA) base;                  \
    __res;                                           \
 })


LOCAL inline DATA KLprecisionStrLength(CONST STRING *string, DATA precision)
/*
 * Returns the length of a string, with a maximum precision
 * So, if strlen(string) < precision return strlen(string), else return
 * precision.
 *
 * INPUT:
 *     string:    string to determine length of
 *     precision: precision to use
 *
 * RETURNS:
 *     KLprecisionStrLength():  String length
 */
{
    CONST STRING *stringPtr;

    for (stringPtr = string; precision-- && *stringPtr != '\0'; ++stringPtr);

    return stringPtr - string;
}


LOCAL inline STRING *KLintToAscii(STRING *string, DATA number, DATA base,
                                  DATA size, DATA precision, DATA type)
/*
 * Convert an integer to a string, with lots of formatting options
 *
 * INPUT:
 *     string:    String to store result in
 *     number:    Integer to convert
 *     base:      Base of conversion
 *     size:      Total width of the field (number of characters)
 *     precision: Precision of the number itself (size - padding)
 *     type:      All those flags (or'ed) that define the appearance 
 *
 * RETURNS:
 *     KLintToAscii():  Pointer to the first character after the number
 */
{
    BYTE pad = ' ', sign = 0;
    STRING temp[66];
    CONST STRING *digits="0123456789abcdefghijklmnopqrstuvwxyz";
    DATA i = 0;

    if (type & LARGE) digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (type & LEFT) type &= ~ZEROPAD;
    if (base < 2 || base > 36) return 0;

    if(type & ZEROPAD) pad = '0';         /* The padding character     */

    if (type & SIGN) {                    /* Is it signed ?            */
        if (number < 0) {                 /* Is it actually negative ? */
            sign = '-';                   /* Sign char is a minus (-)  */
            number = -number;
            size--;
        } else if (type & PLUS) {         /* Actually spec positive ?  */
            sign = '+';                   /* Sign char is a plus (+)   */
            size--;
        } else if (type & SPACE) {        /* Leave sign space empty ?  */
            sign = ' ';                   /* Sign char is a space ( )  */
            size--;
        }
    }

    if (type & SPECIAL) {                 /* Adjust size for different */
        if (base == 16)                   /* base (non-decimal)        */
            size -= 2;
        else if (base == 8)
            size--;
    }

    if (number == 0)                      /* Print the number          */
        temp[i++]='0';
    else
        while (number != 0) temp[i++] = digits[DIVIDE(number, base)];

    if (i > precision) precision = i;    /* Align right, pad spaces */
    size -= precision;
    if (!(type & (ZEROPAD+LEFT)))
        while(size-->0) *string++ = ' ';

    if (sign) *string++ = sign;          /* Print the sign          */

    if (type & SPECIAL) {                /* Print the base ident    */
        if (base==8) *string++ = '0';    /* (0 = octal && 0x = hex) */
        else if (base==16) {
            *string++ = '0';
            *string++ = digits[33];
        }
    }

    if (!(type & LEFT)) while (size-- > 0) *string++ = pad;  /* Pad */

    while (i < precision--) *string++ = '0';  /* Print the number   */
    while (i-- > 0) *string++ = temp[i];      /* to the output str  */
    while (size-- > 0) *string++ = ' ';

    return string;
}


LOCAL inline DATA KLnextAsciiToInt(CONST STRING **string)
/*
 * Convert the number at a certain position in a string to an integer value
 *
 * INPUT:
 *     string:  Pointer to start of number inside the string
 *
 * RETURNS:
 *     KLnextAsciiToInt():  Converted integer
 */
{
    DATA retValue = 0;

    while (KLisDigit(**string)) retValue = retValue*10 + *((*string)++) - '0';

    return retValue;
}


PUBLIC DATA KLformatToString(STRING *buffer, CONST STRING *formatString, va_list arguments)
/*
 * Converts a printf() style format string to a full string
 *
 * INPUT:
 *     buffer:       place to store formatted string in
 *     formatString: format string to convert
 *     arguments:    arguments of the format string
 *
 * RETURNS:
 *     KLformatToString():  Length of the formatted string
 */
{
    DATA i;                    /* for() counter variable            */

    DATA stringLength;         /* Length of the temporary string    */
    UDATA number;              /* Temporary number                  */
    STRING *string;            /* Pointer to position in the buffer */
    CONST STRING *tempString;  /* Temporary string                  */

    DATA flags;                /* Flags for KLintToAscii()          */

    DATA fieldWidth;           /* Width of output field             */
    DATA precision;            /* Min. # of digits for integers;    */
                               /* max number chars for string       */
    DATA base;                 /* Base of number                    */
    DATA qualifier;            /* Size of int fields ('s','l' etc.) */

    DATA *converted;           /* For %n                            */

    /* Start looping through the format string */
    for (string=buffer ; *formatString ; ++formatString) {

        /* 
         * Unless this is a format identifier (strarting with %), we
         * just copy the character into the destination string and
         * continue with the next char
         */
        if (*formatString != '%') {
            *string++ = *formatString;
            continue;
        }

        /* If we get here, we have to process a format identifier  */

        /* Process flags (if any: left align, stuff like that) */
        flags = 0;
        repeat:
            ++formatString;           /* Next flag; also skips '%' */
            switch (*formatString) {
                case '-': flags |= LEFT; goto repeat;
                case '+': flags |= PLUS; goto repeat;
                case ' ': flags |= SPACE; goto repeat;
                case '#': flags |= SPECIAL; goto repeat;
                case '0': flags |= ZEROPAD; goto repeat;
            }

        /* Get the field width */
        fieldWidth = -1;                  /* If a number is after the '%' */
        if (KLisDigit(*formatString))     /* Then that's the field width  */
            fieldWidth = KLnextAsciiToInt(&formatString);
        else if (*formatString == '*') {  /* Or if it is a '*', get the   */
            ++formatString;               /* width from the next argument */
            fieldWidth = va_arg(arguments, DATA);
            if (fieldWidth < 0) {
                fieldWidth = -fieldWidth;
                flags |= LEFT;
            }
        }

        /* Get the precision */
        precision = -1;                
        if (*formatString == '.') {  /* If next char is '.', next comes */
            ++formatString;          /* the precision                   */
            if (KLisDigit(*formatString))     /* ... as a number        */
                precision = KLnextAsciiToInt(&formatString);
            else if (*formatString == '*') {  /* ... as next argument   */
                ++formatString;
                /* It's the next argument */
                precision = va_arg(arguments, DATA);
            }
            if (precision < 0) precision = 0; /* ... or nothing         */
        }

        /* Get the conversion qualifier */
        qualifier = -1;
        if (*formatString == 'h' || *formatString == 'l' ||
            *formatString == 'L')
            qualifier = *(formatString++);

        /* Default base is base 10 */
        base = 10;

        /* Now we find out what we actually need to print */
        switch (*formatString) {
            case 'c':                             /* It's a byte !!      */
                if (!(flags & LEFT))              /* Align right         */
                    while (--fieldWidth > 0) *string++ = ' ';
                *string++ = (UBYTE) va_arg(arguments, DATA); /* Print it */
                while (--fieldWidth > 0)          /* Align left          */
                    *string++ = ' ';
                continue;

            case 's':                             /* It's a string !!    */
                tempString = va_arg(arguments, STRING *); /* Get the arg */
                if (!tempString) tempString = "<NULL>";   /* ... or null */
                stringLength = KLprecisionStrLength(tempString, precision);

                if (!(flags & LEFT))              /* Align right         */
                    while (stringLength < fieldWidth--) *string++ = ' ';
                for (i = 0; i < stringLength; ++i) *string++ = *tempString++;
                while (stringLength < fieldWidth--) *string++ = ' ';
                continue;

            case 'p':                             /* It's a pointer !!   */
                if (fieldWidth == -1) {           /* If no width specced */
                    fieldWidth = 2*ADDRSIZE;      /* Full address size   */
                    flags |= ZEROPAD;             /* Pad with zeros      */
                }
                string = KLintToAscii(string,     /* Convert to string   */
                                      (UADDR) va_arg(arguments, VOID *),
                                      16, fieldWidth, precision, flags);
                continue;

            case 'n':                             /* How much converted  */
                        converted = va_arg(arguments, DATA *);
                        *converted = (string - buffer);
                        continue;

            /* Integer number formats - set up the flags and break */
            case 'o':                 /* It's octal !!             */
                base = 8;
                break;

            case 'X':                 /* It's hexadecimal !!       */
                flags |= LARGE;
            case 'x':
                base = 16;
                break;

            case 'd':                 /* It's signed !!            */
            case 'i':
                flags |= SIGN;
            case 'u':                 /* ... or unsigned           */
                break;

            default:                  /* It's a '%' character !!   */
                if (*formatString != '%') *string++ = '%';
                if (*formatString) *string++ = *formatString;
                else --formatString;
                continue;
            }

            /* It's an integer */
            if (qualifier == 'l') number = va_arg(arguments, UDATA);
            else if (qualifier == 'h')
                if (flags & SIGN) number = va_arg(arguments, WORD32);
                else number = va_arg(arguments, UWORD32);
            else if (flags & SIGN) number = va_arg(arguments, DATA);
            else number = va_arg(arguments, UDATA);

            /* Print it */
            string = KLintToAscii(string, number, base,
                                  fieldWidth, precision, flags);
    }

    /* We're finished ! */
    *string = '\0';       /* Terminate ASCIIZ string  */
    return string-buffer; /* Return the string length */
}


PUBLIC DATA KLstringFormat(OUTPUT STRING *buffer, CONST STRING *formatString, ...)
/*
 * Format a printf() style format string.  Uses variable
 * number of arguments.
 *
 * INPUT:
 *     formatString:  string to format
 *     ...:           arguments to format string
 *
 * OUTPUT:
 *     buffer:        place to store result in
 *
 * RETURNS:
 *     KLstringFormat():  Length of formatted string
 */
{
    va_list arguments;
    DATA returnValue;

    va_start(arguments, formatString);
    returnValue = KLformatToString(buffer, formatString, arguments);
    va_end(arguments);

    return returnValue;
}


PUBLIC DATA KLasciiToInt(INPUT STRING *string)
/*
 * Convert the number in a string to an integer value (global function)
 *
 * INPUT:
 *     string:  Pointer to start of number inside the string
 *
 * RETURNS:
 *     KLasciiToInt():  Converted integer
 */
{
    return KLnextAsciiToInt(&string);
}
