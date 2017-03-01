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


#ifndef __GRUB_NET_CONFIG_H
#define __GRUB_NET_CONFIG_H

#include "netboot.h"
#include "nic.h"

extern struct nic nic;

extern void print_config(void);
extern void eth_reset(void);
extern int  eth_probe(void);
extern int  eth_poll(void);
extern void eth_transmit(char *d, unsigned int t, unsigned int s, char *p);
extern void eth_disable(void);


#ifdef INCLUDE_WD
extern struct nic *wd_probe(struct nic *, unsigned short *);
#endif

#ifdef INCLUDE_T503
extern struct nic *t503_probe(struct nic *, unsigned short *);
#endif

#if defined(INCLUDE_NE) || defined(INCLUDE_NEPCI)
extern struct nic *ne_probe(struct nic *, unsigned short *);
#endif

#ifdef INCLUDE_T509
extern struct nic *t509_probe(struct nic *, unsigned short *);
#endif

#ifdef INCLUDE_EEPRO100
extern struct nic *eepro100_probe(struct nic *, unsigned short *);
#endif

#ifdef INCLUDE_CS
extern struct nic *cs89x0_probe(struct nic *, unsigned short *);
#endif

#ifdef INCLUDE_NE2100
extern struct nic *ne2100_probe(struct nic *, unsigned short *);
#endif

#endif
