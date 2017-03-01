/*
 * BitmapAlloc.c:  Functions to manage the memory
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 14/03/99  jens        1.0    Moved from CK to KLlibs
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

#include <typewrappers.h>
#include <klibs.h>

/*
 * If we put this in CK's memory space and give everyone read access then
 * some this will not have to be allocated more than one time,
 * this only need to be created once.
 */

PUBLIC UBYTE FastBitStTable[256];/* used for lookup of first free bit */
PUBLIC UBYTE FastBitStList[256]; /* used for lookup of no. first free bits */
PUBLIC UBYTE FastBitEnList[256]; /* used for lookup of no. last free bits */
PUBLIC UBYTE FastBitList[256];   /* used for getting largest bit block */
PUBLIC UBYTE FastBitListSt[256]; /* used for getting largest bit block start */

PUBLIC VOID KLbitmapCreateLookupTable(VOID)
/*
 * Initialises the bitmap alloc fast lookup table
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    DATA a,b;
    DATA free,maxfree,maxstpos;

    /* setup fast quick lookup table for first free bit */

    for(a=0;a<256;a++) {
    	for(b=0;b<8;b++) {
     	    if(((1<<b)&a)==0) {
      	        FastBitStTable[a]=b;
      	        break;
            }
        }
        free=maxfree=maxstpos=0;
        for(b=0;b<8;b++) {
            if(((1<<b)&a)==0) {
                free++;
            } else {
                if(free>maxfree) {
                    maxfree=free;
                    maxstpos=(b-free)+1;
                }
                free=0;
            }
            if(free>maxfree) {
                maxfree=free;
                maxstpos=(b-free)+1;
            }
        }
        FastBitList[a]=maxfree;
        FastBitListSt[a]=maxstpos;
        for(b=0;b<8;b++) {
            if(((1<<b)&a)!=0) break;
        }
        FastBitEnList[a]=b;
        for(b=7;b>=0;b--) {
            if(((1<<b)&a)!=0) break;
        }
        FastBitStList[a]=7-b;
    }
}

PUBLIC VOID KLbitmapInitAlloc(VOID *bitmap,DATA size)
/*
 * Initialize bitmap
 *
 * INPUT:
 *     bitmap - bitmap to initialize
 *     size   - size of bitmap
 *
 * RETURNS:
 *     bit index
 *     -1 = error no more bits
 */
{
    DATA a;
    KLbitmapStruct *bs=(KLbitmapStruct *)bitmap;
    bs->size=(size+31)>>5;
    bs->firstFree=0;
    /* clear bitmap */
    for(a=0;a<bs->size;a++) bs->bitmap[a]=0;
}

#define stepforward \
    p++; \
    b++; \
    if(b==4) { \
	    b=0; a++; \
        if(a==bs->size) { \
            return -1; \
		} \
    }

PUBLIC DATA KLbitmapAlloc(VOID *bitmap,DATA nobits)
/*
 * allocate a bit in a bitmap
 *
 * INPUT:
 *     bitmap - bitmap to alloc from
 *     nobits - number of bits to allocate
 *
 * RETURNS:
 *     bit index
 *     -1 = error no more bits
 */
{
    KLbitmapStruct *bs=(KLbitmapStruct *)bitmap;

    DATA a,b,p,sa,sb,bitno,count;
    UBYTE *temp,bits,bitval;

    for(a=bs->firstFree;a<bs->size;a++) {
                                            /* Search the bitmap             */
        if(bs->bitmap[a]==0xffffffff)
            continue;                       /* No bits free, try next long   */
        bs->firstFree=a;                    /* This must be the first free   */
        temp=(UBYTE *)(bs->bitmap+a);       /* Convert Ptr to UBYTE          */
        for(b=0;b<4;b++) {                  /* Check the whole UWORD32       */
            bits=temp[b];                   /* Read Byte from bitmap         */
            if(bits==0xff) continue;        /* No bits free in this byte?    */
    	    if(nobits==1) {
                bitval=FastBitStTable[bits];/* Retrieve first free bit       */
                temp[b]|=1<<bitval;         /* Mark it used                  */
                bitno=(((a<<2)+b)<<3)+bitval;/* Calculate the page bit       */
                return bitno;               /* Return the PageMapping        */
            } else {
                if(nobits<8) {
                    bitval=FastBitList[bits];
                    if(bitval>=nobits) {
                        bitno=FastBitListSt[bits];
                        temp[b]|=((1<<nobits)-1)<<bitno;
                        bitno=(((a<<2)+b)<<3)+bitno;/* Calculate the page bit        */
                        return bitno;
                    }
                }
    	        sa=a;
                p=sb=b;
                bitval=FastBitStList[bits];
                count=nobits-bitval;
    	        stepforward
                while(count>8) {
                    stepforward
                    if(temp[p]==0) count-=8; else break;
                }
                if(count<8) {
                    count-=FastBitEnList[temp[p]];
                    if(count<=0) {
                        p=sb;
                        count=nobits-bitval;
                        temp[p++]|=-0x80>>(bitval-1);
                        while(count>8) {
                            temp[p++]=0xff;
                            count-=8;
                        }
                        temp[p]|=(1<<count)-1;
                        bitno=(((sa<<2)+sb)<<3)+(8-bitval);   /* Calculate the page bit        */
                        return bitno;
                    }
                }
            }
        }
    }
    return -1;                              /* No free page mappings         */
}

#undef stepforward

PUBLIC VOID KLbitmapMarkUsed(VOID *bitmap,DATA bit)
{
    KLbitmapStruct *bs=(KLbitmapStruct *)bitmap;
    UBYTE *temp=(UBYTE *)bs->bitmap;
    DATA a=bit>>3;                          /* Calculate byte position       */
    DATA b=bit&7;                           /* Calculate bit position        */
    temp[a]|=(1<<b);                        /* Clear bit in bitmap           */
    if(bs->firstFree>a>>2)                  /* If first position bigger      */
        bs->firstFree=a>>2;                 /* set freed as first position   */
}

PUBLIC VOID KLbitmapFree(VOID *bitmap,DATA bit)
/*
 * free a bit from bitmap
 *
 * INPUT:
 *     bitmap - bitmap to free from
 *     bit - bit to free
 *
 * RETURNS:
 *     none
 */
{
    KLbitmapStruct *bs=(KLbitmapStruct *)bitmap;
    UBYTE *temp=(UBYTE *)bs->bitmap;
    DATA a=bit>>3;                          /* Calculate byte position       */
    DATA b=bit&7;                           /* Calculate bit position        */
    temp[a]&=~(1<<b);                       /* Clear bit in bitmap           */
    if(bs->firstFree>a>>2)                  /* If first position bigger      */
        bs->firstFree=a>>2;                 /* set freed as first position   */
}
