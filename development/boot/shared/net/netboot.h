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

/**************************************************************************
NETBOOT -  BOOTP/TFTP Bootstrap Program

Author: Martin Renters
  Date: Dec/93

**************************************************************************/

#ifndef __GRUB_NET_NETBOOT_H
#define __GRUB_NET_NETBOOT_H

#include "shared.h"

#define __SLOW_DOWN_IO "\noutb %%al,$0x80"

typedef unsigned long Address;

#define bcmp __builtin_memcmp
#define memcpy(d,s,sz) bcopy((s),(d),(sz))

#define ESC		0x1B

#ifndef DEFAULT_BOOTFILE
#define DEFAULT_BOOTFILE	"/kernel"
#endif

#ifndef MAX_TFTP_RETRIES
#define MAX_TFTP_RETRIES	20
#endif

#ifndef MAX_BOOTP_RETRIES
#define MAX_BOOTP_RETRIES	20
#endif

#ifndef MAX_BOOTP_EXTLEN
#define MAX_BOOTP_EXTLEN	1024
#endif

#ifndef MAX_ARP_RETRIES
#define MAX_ARP_RETRIES		20
#endif

#ifndef MAX_RPC_RETRIES
#define MAX_RPC_RETRIES		20
#endif

#ifndef TIMEOUT			/* Inter-packet retry in ticks 18/sec */
#define TIMEOUT			180
#endif

#ifndef NULL
#define NULL	((void *)0)
#endif

#define TRUE		1
#define FALSE		0

#define ETHER_ADDR_SIZE		6	/* Size of Ethernet address */
#define ETHER_HDR_SIZE		14	/* Size of ethernet header */
#define ETH_MIN_PACKET		64
#define ETH_MAX_PACKET		1518

#define VENDOR_NONE	0
#define VENDOR_WD	1
#define VENDOR_NOVELL	2
#define VENDOR_3COM	3
#define VENDOR_3C509	4
#define VENDOR_CS89x0	5

#define FLAG_PIO	0x01
#define FLAG_16BIT	0x02
#define FLAG_790	0x04

#define ARP_CLIENT	0
#define ARP_SERVER	1
#define ARP_GATEWAY	2
#define ARP_ROOTSERVER	3
#define ARP_SWAPSERVER	4
#define MAX_ARP		ARP_SWAPSERVER+1

#define IP		0x0800
#define ARP		0x0806

#define BOOTP_SERVER	67
#define BOOTP_CLIENT	68
#define TFTP		69
#define SUNRPC		111

#define RPC_SOCKET	620			/* Arbitrary */

#define IP_UDP		17
#define IP_BROADCAST	0xFFFFFFFF

#define ARP_REQUEST	1
#define ARP_REPLY	2

#define BOOTP_REQUEST	1
#define BOOTP_REPLY	2

#define TAG_LEN(p)		(*((p)+1))
#define RFC1533_COOKIE		99, 130, 83, 99
#define RFC1533_PAD		0
#define RFC1533_NETMASK		1
#define RFC1533_TIMEOFFSET	2
#define RFC1533_GATEWAY		3
#define RFC1533_TIMESERVER	4
#define RFC1533_IEN116NS	5
#define RFC1533_DNS		6
#define RFC1533_LOGSERVER	7
#define RFC1533_COOKIESERVER	8
#define RFC1533_LPRSERVER	9
#define RFC1533_IMPRESSSERVER	10
#define RFC1533_RESOURCESERVER	11
#define RFC1533_HOSTNAME	12
#define RFC1533_BOOTFILESIZE	13
#define RFC1533_MERITDUMPFILE	14
#define RFC1533_DOMAINNAME	15
#define RFC1533_SWAPSERVER	16
#define RFC1533_ROOTPATH	17
#define RFC1533_EXTENSIONPATH	18
#define RFC1533_IPFORWARDING	19
#define RFC1533_IPSOURCEROUTING	20
#define RFC1533_IPPOLICYFILTER	21
#define RFC1533_IPMAXREASSEMBLY	22
#define RFC1533_IPTTL		23
#define RFC1533_IPMTU		24
#define RFC1533_IPMTUPLATEAU	25
#define RFC1533_INTMTU		26
#define RFC1533_INTLOCALSUBNETS	27
#define RFC1533_INTBROADCAST	28
#define RFC1533_INTICMPDISCOVER	29
#define RFC1533_INTICMPRESPOND	30
#define RFC1533_INTROUTEDISCOVER 31
#define RFC1533_INTROUTESOLICIT	32
#define RFC1533_INTSTATICROUTES	33
#define RFC1533_LLTRAILERENCAP	34
#define RFC1533_LLARPCACHETMO	35
#define RFC1533_LLETHERNETENCAP	36
#define RFC1533_TCPTTL		37
#define RFC1533_TCPKEEPALIVETMO	38
#define RFC1533_TCPKEEPALIVEGB	39
#define RFC1533_NISDOMAIN	40
#define RFC1533_NISSERVER	41
#define RFC1533_NTPSERVER	42
#define RFC1533_VENDOR		43
#define RFC1533_NBNS		44
#define RFC1533_NBDD		45
#define RFC1533_NBNT		46
#define RFC1533_NBSCOPE		47
#define RFC1533_XFS		48
#define RFC1533_XDM		49

#define RFC1533_VENDOR_MAJOR	0
#define RFC1533_VENDOR_MINOR	0

#define RFC1533_VENDOR_MAGIC	128
#define RFC1533_VENDOR_ADDPARM	129
#define RFC1533_VENDOR_MNUOPTS	160
#define RFC1533_VENDOR_SELECTION 176
#define RFC1533_VENDOR_MOTD	184
#define RFC1533_VENDOR_NUMOFMOTD 8
#define RFC1533_VENDOR_IMG	192
#define RFC1533_VENDOR_NUMOFIMG	16

#define RFC1533_END		255
#define BOOTP_VENDOR_LEN	64

#define	TFTP_DEFAULTSIZE_PACKET	512
#define	TFTP_MAX_PACKET		1432 /* 512 */

#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5
#define TFTP_OACK	6

#define TFTP_CODE_EOF	1
#define TFTP_CODE_MORE	2
#define TFTP_CODE_ERROR	3
#define TFTP_CODE_BOOT	4
#define TFTP_CODE_CFG	5

#define PROG_PORTMAP	100000
#define PROG_NFS	100003
#define PROG_MOUNT	100005

#define MSG_CALL	0
#define MSG_REPLY	1

#define PORTMAP_LOOKUP	3

#define MOUNT_ADDENTRY	1
#define MOUNT_UMNTALL   4
#define NFS_LOOKUP	4
#define NFS_READ	6

#define NFS_READ_SIZE	1024


#define AWAIT_ARP	0
#define AWAIT_BOOTP	1
#define AWAIT_TFTP	2
#define AWAIT_RPC	3

struct arptable_t {
	unsigned long ipaddr;
	unsigned char node[6];
};

struct arprequest {
	unsigned short hwtype;
	unsigned short protocol;
	char hwlen;
	char protolen;
	unsigned short opcode;
	char shwaddr[6];
	char sipaddr[4];
	char thwaddr[6];
	char tipaddr[4];
};

struct iphdr {
	char verhdrlen;
	char service;
	unsigned short len;
	unsigned short ident;
	unsigned short frags;
	char ttl;
	char protocol;
	unsigned short chksum;
	char src[4];
	char dest[4];
};

struct udphdr {
	unsigned short src;
	unsigned short dest;
	unsigned short len;
	unsigned short chksum;
};

struct bootp_t {
	struct iphdr ip;
	struct udphdr udp;
	char bp_op;
	char bp_htype;
	char bp_hlen;
	char bp_hops;
	unsigned long bp_xid;
	unsigned short bp_secs;
	unsigned short unused;
	char bp_ciaddr[4];
	char bp_yiaddr[4];
	char bp_siaddr[4];
	char bp_giaddr[4];
	char bp_hwaddr[16];
	char bp_sname[64];
	char bp_file[128];
	char bp_vend[BOOTP_VENDOR_LEN];
};

struct bootpd_t {
	struct bootp_t bootp_reply;
	unsigned char  bootp_extension[MAX_BOOTP_EXTLEN];
};


struct tftp_t {
	struct iphdr ip;
	struct udphdr udp;
	unsigned short opcode;
	union {
		char rrq[TFTP_DEFAULTSIZE_PACKET];
		struct {
			unsigned short block;
			char download[TFTP_MAX_PACKET];
		} data;
		struct {
			unsigned short block;
		} ack;
		struct {
			unsigned short errcode;
			char errmsg[TFTP_DEFAULTSIZE_PACKET];
		} err;
		struct {
			char data[TFTP_DEFAULTSIZE_PACKET+2];
		} oack;
	} u;
};

struct rpc_t {
	struct iphdr	ip;
	struct udphdr	udp;
	union {
		char data[1400];
		struct {
			long id;
			long type;
			long rstatus;
			long verifier;
			long v2;
			long astatus;
			long data[1];
		} reply;
	} u;
};

#define TFTP_MIN_PACKET	(sizeof(struct iphdr) + sizeof(struct udphdr) + 4)

/***************************************************************************
RPC Functions
***************************************************************************/
#define PUTLONG(val) {\
	register int lval = val; \
	*(rpcptr++) = ((lval) >> 24); \
	*(rpcptr++) = ((lval) >> 16); \
	*(rpcptr++) = ((lval) >> 8); \
	*(rpcptr++) = (lval); \
	rpclen+=4; }


/***************************************************************************
External variables
***************************************************************************/
/* main.c */
extern char *kernel;
extern char kernel_buf[];
extern struct nfs_diskless nfsdiskless;
extern int hostnamelen;
extern unsigned long netmask;
extern int    jmp_bootmenu[10];
extern char   kernel_buf[128];
extern struct bootpd_t bootp_data;
extern struct arptable_t arptable[MAX_ARP];
extern char   *imagelist[RFC1533_VENDOR_NUMOFIMG];
extern char   *motd[RFC1533_VENDOR_NUMOFMOTD];
extern int    menutmo,menudefault;
extern unsigned char *end_of_rfc1533;

/* bootmenu.c */

/* linuxloader.c */

/* rpc.c */
extern int rpc_id;

/* created by linker */
extern char _edata[], _end[];

/* Keyboard stuff */
extern int _pending_key;

extern inline int getchar(void) {
	int foo;

	if (_pending_key == -1)
		return getkey();
	foo = _pending_key;
	_pending_key = -1;
	return foo;
}

extern inline void putchar(int c) {
	print("%c",c);
}

extern inline int iskey(void) {
	return _pending_key != -1 ? 1 : ((_pending_key = checkkey()) != -1);
}

#endif
