/*
 *  GRUB  --  GRand Unified Bootloader
 *
 *  This network boot code comes from the L4 version of GRUB.
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


#ifndef __GRUB_NET_IP_H
#define __GRUB_NET_IP_H

int ip_init(void);
int tftp(char *name, int (*fnc)(unsigned char *, int, int, int));

extern int ip_abort;
extern int udp_transmit (unsigned long destip, unsigned int srcsock,
                         unsigned int destsock, int len, char *buf);

extern int await_reply (int type, int ival, char *ptr);
extern unsigned short ipchksum (unsigned short *, int len);

extern void rfc951_sleep (int);
extern void twiddle (void);
extern unsigned long currticks(void);


/*
 *  Byte swap:  needed for endianness conversion
 *  Taken from /usr/include/bits/byteswap.h
 */

/* Swap bytes in 16 bit value.  */
#define __bswap_constant_16(x)                                       \
     ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define __bswap_16(x)                                                \
     (__extension__                                                  \
      ({ register unsigned short int __v;                            \
         if (__builtin_constant_p (x))                               \
           __v = __bswap_constant_16 (x);                            \
         else                                                        \
           __asm__ __volatile__ ("rorw $8, %w0"                      \
                                 : "=r" (__v)                        \
                                 : "0" ((unsigned short int) (x))    \
                                 : "cc");                            \
         __v; }))


/* Swap bytes in 32 bit value.  */
/* Do this the slow way that always works.  On 486+ we could use
   bswap, but I'm too lazy to include that right now.  It's just
   for booting anyway, who cares ?  -- Ramon */
#define __bswap_constant_32(x)                                       \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |      \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))
#define __bswap_32(x)                                                \
     (__extension__                                                  \
      ({ register unsigned int __v;                                  \
         if (__builtin_constant_p (x))                               \
           __v = __bswap_constant_32 (x);                            \
         else                                                        \
           __asm__ __volatile__ ("rorw $8, %w0;"                     \
                                 "rorl $16, %0;"                     \
                                 "rorw $8, %w0"                      \
                                 : "=r" (__v)                        \
                                 : "0" ((unsigned int) (x))          \
                                 : "cc");                            \
         __v; }))



/*
 *  Functions to convert between host and network byte order.
 *  Taken from /usr/include/netinet/in.h
 *  NOTE:  if this is ported to non-x86, we need an endianness
 *         check here !  network byte order == big endian, so
 *         big endian machines don't need any conversion
 */

#define ntohl(x)      __bswap_32 (x)
#define ntohs(x)      __bswap_16 (x)
#define htonl(x)      __bswap_32 (x)
#define htons(x)      __bswap_16 (x)

#endif
