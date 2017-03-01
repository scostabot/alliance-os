/*
 * 8254.h:   Support for 8254 timer
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 *
 * Original version 1.1, Nov 29, 1997 by John S. Fine <johnfine@erols.com>
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

#ifndef __SYS_8254_H
#define __SYS_8254_H 

/*
 * To initialize a counter, you OUT a byte to TMR_CTRL.  You construct that
 * byte by adding:
 *
 *      one of TMR_SC0 .. TMR_SC2 to select the counter
 * plus one of TMR_low .. TMR_both to select which bytes will be active
 * plus one of TMR_MD0 .. TMR_MD5 to select the mode
 * plus (optionally) TMR_BCD to enable BCD (rather than binary) counting.
 *
 * After the OUT to TMR_CTRL, you must do one or two (low, high or both)
 * OUTs to the correct TMR_CNTn to place an initial count in the counter.
 *_____________________________________________________________________________
 *
 * To read a counter using a latch command, OUT a byte to TMR_CTRL which
 * consists of  TMR_LATCH plus one of TMR_SC0 .. TMR_SC2.  Then IN one or
 * two bytes from the correct TMR_CNTn.
 *
 * Each channel has three parts:  A count register, which always holds the last
 * initial count you wrote;  A counter, which does the actual counting;  and A
 * latch.
 *
 * When you do a latch (or read) command, the counter is not affected.  Only
 * the latch is stopped.  If you read a channel without a latch (or read)
 * command, you usually get the right answer, but occasionally catch bits in
 * transition (Two bits changing from 01 to 10 might look like 00 if you
 * catch them at the moment of transition).
 *_____________________________________________________________________________
 *
 * To read multiple counters and/or channel statuses, use a read command.  OUT
 * to TMR_CTRL a value consisting of:
 *
 *                      TMR_READ
 * plus one or more of  TMR_CH0 .. TMR_CH2 selecting all channels to be read
 * minus one or both of TMR_CNT .. TMR_STAT to read count and/or status
 *
 * When reading both count and status, the first IN will be status.
 */

#define TMR_CTRL  0x43	  /* I/O for control                           */
#define TMR_CNT0  0x40	  /* I/O for counter 0                         */
#define TMR_CNT1  0x41	  /* I/O for counter 1                         */
#define TMR_CNT2  0x42	  /* I/O for counter 2                         */

#define TMR_SC0	  0	  /* Select channel 0                          */
#define TMR_SC1	  0x40	  /* Select channel 1                          */
#define TMR_SC2	  0x80	  /* Select channel 2                          */

#define TMR_low	  0x10	  /* RW low byte only                          */
#define TMR_high  0x20	  /* RW high byte only                         */
#define TMR_both  0x30	  /* RW both bytes                             */

#define TMR_MD0	  0	  /* Mode 0                                    */
#define TMR_MD1   0x2	  /* Mode 1                                    */
#define TMR_MD2	  0x4	  /* Mode 2                                    */
#define TMR_MD3	  0x6	  /* Mode 3                                    */
#define TMR_MD4	  0x8	  /* Mode 4                                    */
#define TMR_MD5	  0xA	  /* Mode 5                                    */

#define TMR_BCD	  1	  /* BCD mode                                  */

#define TMR_LATCH 0	  /* Latch command                             */

#define TMR_READ  0xF0	  /* Read command                              */
#define TMR_CNT	  0x20	  /*    CNT bit  (Active low, subtract it)     */
#define TMR_STAT  0x10	  /*    Status bit  (Active low, subtract it)  */
#define TMR_CH2	  0x8	  /*    Channel 2 bit                          */
#define TMR_CH1	  0x4	  /*    Channel 1 bit                          */
#define TMR_CH0	  0x2	  /*    Channel 0 bit                          */

#endif



