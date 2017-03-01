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

#ifndef __GRUB_NET_NIC_H
#define __GRUB_NET_NIC_H

/*
 *	Structure returned from eth_probe and passed to other driver
 *	functions.
 */

struct nic {
    void (*reset)(struct nic *);
    int  (*poll)(struct nic *);
    void (*transmit)(struct nic *, char *d, unsigned int t, unsigned int s, char *p);
    void (*disable)(struct nic *);

    char aui;
    char *node_addr;
    char *packet;
    int  packetlen;
    void *priv_data;	/* driver can hang private data here */
};

#endif
