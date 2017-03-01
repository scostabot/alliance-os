/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1996   Erich Boleyn  <erich@uruk.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "shared.h"

struct multiboot_info *mbinf;
int errnum = 0;

char *err_list[] =
{
  0,
  "Selected item won\'t fit into memory",
  "Selected disk doesn\'t exist",
  "Disk read error",
  "Disk write error",
  "Disk geometry error",
  "Attempt to access block outside partition",
  "Partition table invalid or corrupt",
  "No such partition",
  "Bad filename (must be absolute pathname or blocklist)",
  "Bad file or directory type",
  "File not found",
  "Too many symlinks",
  "Cannot mount selected partition",
  "Inconsistent filesystem structure",
  "Filesystem compatibility error, can\'t read whole file",
  "Error while parsing number",
  "Device string unrecognizable",
  "Invalid device requested",
  "Invalid or unsupported executable format",
  "Loading below 1MB is not supported",
  "Unsupported Multiboot features requested",
  "Unknown boot failure",
  "Must load Multiboot kernel before modules",
  "Must load Linux kernel before initrd",
  "Cannot boot without kernel loaded",
  "Unrecognized command",
  "Bad or incompatible header on compressed file",
  "Bad or corrupt data while decompressing file",
  "Bad or corrupt version of stage1/stage2",
  0
};


void __print_error(const char *fl, const long ln)
{
    if (errnum > ERR_NONE && errnum < MAX_ERR_NUM)
        print("libboot: %s in %s line %d\n", err_list[errnum], fl, ln);

    errnum = ERR_NONE;
}


int get_based_digit(int c, int base)
{
    int digit = -1;

    /* make sure letter in the the range we can check! */
    c = tolower(c);

    /*
     *  Is it in the range between zero and nine?
     */
    if(base > 0 && c >= '0' && c <= '9' && c < (base + '0')) {
        digit = c - '0';
    }

    /*
     *  Is it in the range used by a letter?
     */
    if(base > 10 && c >= 'a' && c <= 'z' && c < ((base - 10) + 'a')) {
        digit = c - 'a' + 10;
    }

    return digit;
}


int safe_parse_maxint(char **str_ptr, int *myint_ptr)
{
    register char *ptr = *str_ptr;
    register int myint = 0, digit;
    int mult = 10, found = 0;

    /*
     *  Is this a hex number?
     */
    if(*ptr == '0' && tolower(*(ptr+1)) == 'x') {
        ptr += 2;
        mult = 16;
    }

    while((digit = get_based_digit(*ptr, mult)) != -1) {
        found = 1;
        if (myint > ((MAXINT - digit)/mult)) {
            errnum = ERR_NUMBER_PARSING;
            return 0;
        }
        myint = (myint * mult) + digit;
        ptr++;
    }

    if(!found) {
        errnum = ERR_NUMBER_PARSING;
        return 0;
    }

    *str_ptr = ptr;
    *myint_ptr = myint;

    return 1;
}


int strcmp(char *s1, char *s2)
{
    while(*s1 == *s2) {
        if(!*(s1++))
            return 0;
        s2++;
    }

    if(*s1 == 0)
        return -1;

    return 1;
}


int memcheck(int start, int len)
{
    if((start < 0x1000) ||
       (start < 0x100000 && (mbinf->mem_lower * 1024) < (start+len)) ||
       (start >= 0x100000 && (mbinf->mem_upper * 1024) < ((start-0x100000)+len)))
        errnum = ERR_WONT_FIT;

    return (!errnum);
}


int bcopy(char *from, char *to, int len)
{
    if(memcheck((int)to, len)) {
        if((to >= from+len) || (to <= from)) {
            while(len > 3) {
                len -= 4;
                *(((unsigned long *)to)++) = *(((unsigned long *)from)++);
            }
            while(len-- > 0)
                *(to++) = *(from++);
        } else {
            while(len-- > 0)
            to[len] = from[len];
	}
    }

    return (!errnum);
}


int bzero(char *start, int len)
{
    if(memcheck((int)start, len)) {
        while (len-- > 0)
            *(start++) = 0;
    }

    return (!errnum);
}
