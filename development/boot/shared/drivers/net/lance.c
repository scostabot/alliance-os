/**************************************************************************
Etherboot -  BOOTP/TFTP Bootstrap Program
LANCE NIC driver for Etherboot
Large portions borrowed from the Linux LANCE driver by Donald Becker
Ken Yap, July 1997
***************************************************************************/

/* to get some global routines like print */
#include "../../net/netboot.h"
/* to get the interface to the body of the program */
#include "../../net/nic.h"
#include "../../net/ip.h"
#include "lio.h"

/* Offsets from base I/O address */
#define	LANCE_DATA	0x10
#define	LANCE_ADDR	0x12
#define	LANCE_RESET	0x14
#define	LANCE_BUS_IF	0x16
#define	LANCE_TOTAL_SIZE	0x18

struct lance_init_block
{
	unsigned short	mode;
	unsigned char	phys_addr[6];
	unsigned long	filter[2];
	Address		rx_ring;
	Address	 	tx_ring;
};

struct lance_rx_head
{
	union {
		Address		base;
		char		addr[4];
	} u;
	short		buf_length;	/* 2s complement */
	short		msg_length;
};

struct lance_tx_head
{
	union {
		Address		base;
		char		addr[4];
	} u;
	short		buf_length;	/* 2s complement */
	short		misc;
};

struct lance_interface
{
	struct lance_init_block	init_block;
	struct lance_rx_head	rx_ring;
	struct lance_tx_head	tx_ring;
	unsigned char		rbuf[ETH_MAX_PACKET];
	unsigned char		tbuf[ETH_MAX_PACKET];
};

#define	LANCE_MUST_PAD		0x00000001
#define	LANCE_ENABLE_AUTOSELECT	0x00000002
#define	LANCE_MUST_REINIT_RING	0x00000004
#define	LANCE_MUST_UNRESET	0x00000008
#define	LANCE_HAS_MISSED_FRAME	0x00000010

/* A mapping from the chip ID number to the part number and features.
   These are from the datasheets -- in real life the '970 version
   reportedly has the same ID as the '965. */
static struct lance_chip_type
{
	int	id_number;
	char	*name;
	int	flags;
} chip_table[] = {
	{0x0000, "LANCE 7990",			/* Ancient lance chip.  */
		LANCE_MUST_PAD + LANCE_MUST_UNRESET},
	{0x0003, "PCnet/ISA 79C960",		/* 79C960 PCnet/ISA.  */
		LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
			LANCE_HAS_MISSED_FRAME},
	{0x2260, "PCnet/ISA+ 79C961",		/* 79C961 PCnet/ISA+, Plug-n-Play.  */
		LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
			LANCE_HAS_MISSED_FRAME},
	{0x2420, "PCnet/PCI 79C970",		/* 79C970 or 79C974 PCnet-SCSI, PCI. */
		LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
			LANCE_HAS_MISSED_FRAME},
	/* Bug: the PCnet/PCI actually uses the PCnet/VLB ID number, so just call
		it the PCnet32. */
	{0x2430, "PCnet32",			/* 79C965 PCnet for VL bus. */
		LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
			LANCE_HAS_MISSED_FRAME},
        {0x2621, "PCnet/PCI-II 79C970A",        /* 79C970A PCInetPCI II. */
                LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
                        LANCE_HAS_MISSED_FRAME},
	{0x0, 	 "PCnet (unknown)",
		LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
			LANCE_HAS_MISSED_FRAME},
};

static int			chip_version;
static unsigned short		ioaddr;
static int			dma;
static struct lance_interface	*lp;
/* additional 8 bytes for 8-byte alignment space */
static char			lance[sizeof(struct lance_interface)+8];

#if	defined(INCLUDE_NE2100)
#define	lance_probe		ne2100_probe
#endif

/* DMA defines and helper routines */

/* DMA controller registers */
#define DMA1_CMD_REG		0x08	/* command register (w) */
#define DMA1_STAT_REG		0x08	/* status register (r) */
#define DMA1_REQ_REG            0x09    /* request register (w) */
#define DMA1_MASK_REG		0x0A	/* single-channel mask (w) */
#define DMA1_MODE_REG		0x0B	/* mode register (w) */
#define DMA1_CLEAR_FF_REG	0x0C	/* clear pointer flip-flop (w) */
#define DMA1_TEMP_REG           0x0D    /* Temporary Register (r) */
#define DMA1_RESET_REG		0x0D	/* Master Clear (w) */
#define DMA1_CLR_MASK_REG       0x0E    /* Clear Mask */
#define DMA1_MASK_ALL_REG       0x0F    /* all-channels mask (w) */

#define DMA2_CMD_REG		0xD0	/* command register (w) */
#define DMA2_STAT_REG		0xD0	/* status register (r) */
#define DMA2_REQ_REG            0xD2    /* request register (w) */
#define DMA2_MASK_REG		0xD4	/* single-channel mask (w) */
#define DMA2_MODE_REG		0xD6	/* mode register (w) */
#define DMA2_CLEAR_FF_REG	0xD8	/* clear pointer flip-flop (w) */
#define DMA2_TEMP_REG           0xDA    /* Temporary Register (r) */
#define DMA2_RESET_REG		0xDA	/* Master Clear (w) */
#define DMA2_CLR_MASK_REG       0xDC    /* Clear Mask */
#define DMA2_MASK_ALL_REG       0xDE    /* all-channels mask (w) */


#define DMA_MODE_READ	0x44	/* I/O to memory, no autoinit, increment, single mode */
#define DMA_MODE_WRITE	0x48	/* memory to I/O, no autoinit, increment, single mode */
#define DMA_MODE_CASCADE 0xC0   /* pass thru DREQ->HRQ, DACK<-HLDA only */

/* enable/disable a specific DMA channel */
static void enable_dma(unsigned int dmanr)
{
	if (dmanr <= 3)
		outb_p(DMA1_MASK_REG, dmanr);
	else
		outb_p(DMA2_MASK_REG, dmanr & 3);
}

static void disable_dma(unsigned int dmanr)
{
	if (dmanr <= 3)
		outb_p(DMA1_MASK_REG, dmanr | 4);
	else
		outb_p(DMA2_MASK_REG, (dmanr & 3) | 4);
}

/* set mode (above) for a specific DMA channel */
static void set_dma_mode(unsigned int dmanr, char mode)
{
	if (dmanr <= 3)
		outb_p(DMA1_MODE_REG, mode | dmanr);
	else
		outb_p(DMA2_MODE_REG, mode | (dmanr&3));
}

/**************************************************************************
RESET - Reset adapter
***************************************************************************/
static void lance_reset(struct nic *nic)
{
	int		i;
	Address		l;

	/* Reset the LANCE */
	(void)inw(ioaddr+LANCE_RESET);
	/* Un-Reset the LANCE, needed only for the NE2100 */
	if (chip_table[chip_version].flags & LANCE_MUST_UNRESET)
		outw(ioaddr+LANCE_RESET, 0);
	if (chip_table[chip_version].flags & LANCE_ENABLE_AUTOSELECT)
	{
		/* This is 79C960 specific; Turn on auto-select of media
		   (AUI, BNC). */
		outw(ioaddr+LANCE_ADDR, 0x2);
		outw(ioaddr+LANCE_BUS_IF, 0x2);
	}
	/* Re-initialise the LANCE, and start it when done. */
	/* Set station address */
	for (i = 0; i < ETHER_ADDR_SIZE; ++i)
		lp->init_block.phys_addr[i] = nic->node_addr[i];
	/* Preset the receive ring header */
	lp->rx_ring.buf_length = -ETH_MAX_PACKET;
	/* OWN */
	lp->rx_ring.u.base = (long) lp->rbuf & 0xffffff;
	/* we set the top byte as the very last thing */
	lp->rx_ring.u.addr[3] = 0x80;
	lp->init_block.mode = 0x0;	/* enable Rx and Tx */
	l = (Address)&lp->init_block;
	outw(ioaddr+LANCE_ADDR, 0x1);
	(void)inw(ioaddr+LANCE_ADDR);
	outw(ioaddr+LANCE_DATA, (short)l);
	outw(ioaddr+LANCE_ADDR, 0x2);
	(void)inw(ioaddr+LANCE_ADDR);
	outw(ioaddr+LANCE_DATA, (short)(l >> 16));
	outw(ioaddr+LANCE_ADDR, 0x4);
	(void)inw(ioaddr+LANCE_ADDR);
	outw(ioaddr+LANCE_DATA, 0x915);
	outw(ioaddr+LANCE_ADDR, 0x0);
	(void)inw(ioaddr+LANCE_ADDR);
	outw(ioaddr+LANCE_DATA, 0x4);		/* stop */
	outw(ioaddr+LANCE_DATA, 0x1);		/* init */
	for (i = 100; i > 0; --i)
		if (inw(ioaddr+LANCE_DATA) & 0x100)
			break;
	/* Apparently clearing the InitDone bit here triggers a bug
	   in the '974. (Mark Stockton) */
	outw(ioaddr+LANCE_DATA, 0x2);		/* start */
}

/**************************************************************************
POLL - Wait for a frame
***************************************************************************/
static int lance_poll(struct nic *nic)
{
	int		status;

	status = lp->rx_ring.u.base >> 24;
	if (status & 0x80)
		return (0);
#ifdef	DEBUG
	print("LANCE packet received rx_ring.u.base %X mcnt %x csr0 %x\r\n",
		lp->rx_ring.u.base, lp->rx_ring.msg_length,
		inw(ioaddr+LANCE_DATA));
#endif
	if (status == 0x3)
		bcopy(lp->rbuf, nic->packet, nic->packetlen = lp->rx_ring.msg_length);
	/* Andrew Boyd of QNX reports that some revs of the 79C765
	   clear the buffer length */
	lp->rx_ring.buf_length = -ETH_MAX_PACKET;
	lp->rx_ring.u.addr[3] |= 0x80;	/* prime for next receive */
	outw(ioaddr+LANCE_ADDR, 0x0);
	(void)inw(ioaddr+LANCE_ADDR);
	outw(ioaddr+LANCE_DATA, 0x500);		/* clear receive + InitDone */
	return (status == 0x3);
}

/**************************************************************************
TRANSMIT - Transmit a frame
***************************************************************************/
static void lance_transmit(
	struct nic *nic,
	char *d,			/* Destination */
	unsigned int t,			/* Type */
	unsigned int s,			/* size */
	char *p)			/* Packet */
{
	unsigned long		time;

	/* copy the packet to ring buffer */
	bcopy(d, lp->tbuf, ETHER_ADDR_SIZE);	/* dst */
	bcopy(nic->node_addr, &lp->tbuf[ETHER_ADDR_SIZE], ETHER_ADDR_SIZE); /* src */
	lp->tbuf[ETHER_ADDR_SIZE+ETHER_ADDR_SIZE] = t >> 8;	/* type */
	lp->tbuf[ETHER_ADDR_SIZE+ETHER_ADDR_SIZE+1] = t;	/* type */
	bcopy(p, &lp->tbuf[ETHER_HDR_SIZE], s);
	s += ETHER_HDR_SIZE;
	if (chip_table[chip_version].flags & LANCE_MUST_PAD)
		while (s < ETH_MIN_PACKET)	/* pad to min length */
			lp->tbuf[s++] = 0;
	lp->tx_ring.buf_length = -s;
	lp->tx_ring.misc = 0x0;
	/* OWN, STP, ENP */
	lp->tx_ring.u.base = (long) lp->tbuf & 0xffffff;
	/* we set the top byte as the very last thing */
	lp->tx_ring.u.addr[3] = 0x83;
	/* Trigger an immediate send poll */
	outw(ioaddr+LANCE_ADDR, 0x0);
	outw(ioaddr+LANCE_DATA, 0x48);
	/* wait for transmit complete */
	time = currticks() + 18;			/* wait one second */
	while (currticks() < time && (lp->tx_ring.u.base & 0x80000000) != 0)
		;
	if ((lp->tx_ring.u.base & 0x80000000) != 0)
		print("LANCE timed out on transmit\r\n");
	(void)inw(ioaddr+LANCE_ADDR);
	outw(ioaddr+LANCE_DATA, 0x200);		/* clear transmit + InitDone */
#ifdef	DEBUG
	print("tx_ring.u.base %X tx_ring.buf_length %x tx_ring.misc %x csr0 %x\r\n",
		lp->tx_ring.u.base, lp->tx_ring.buf_length, lp->tx_ring.misc,
		inw(ioaddr+LANCE_DATA));
#endif
}

static void lance_disable(struct nic *nic)
{
	disable_dma(dma);
}

static int lance_probe1(struct nic *nic)
{
	int			reset_val, lance_version, i;
	Address			l;
	short			dma_channels;
	static char		dmas[] = { 5, 6, 7, 3 };

	reset_val = inw(ioaddr+LANCE_RESET);
	outw(ioaddr+LANCE_ADDR, 0x0);	/* Switch to window 0 */
	if (inw(ioaddr+LANCE_DATA) != 0x4)
		return (-1);
	outw(ioaddr+LANCE_ADDR, 88);	/* Get the version of the chip */
	if (inw(ioaddr+LANCE_ADDR) != 88)
		lance_version = 0;
	else
	{
		chip_version = inw(ioaddr+LANCE_DATA);
		outw(ioaddr+LANCE_ADDR, 89);
		chip_version |= inw(ioaddr+LANCE_DATA) << 16;
		if ((chip_version & 0xfff) != 0x3)
			return (-1);
		chip_version = (chip_version >> 12) & 0xffff;
		for (lance_version = 1; chip_table[lance_version].id_number != 0; ++lance_version)
			if (chip_table[lance_version].id_number == chip_version)
				break;
	}
	/* make sure data structure is 8-byte aligned */
	l = ((Address)lance + 7) & ~7;
	lp = (struct lance_interface *)l;
	lp->init_block.mode = 0x3;	/* disable Rx and Tx */
	lp->init_block.filter[0] = lp->init_block.filter[1] = 0x0;
	/* top bits are zero, we have only one buffer each for Rx and Tx */
	lp->init_block.rx_ring = (long) &lp->rx_ring & 0xffffff;
	lp->init_block.tx_ring = (long) &lp->tx_ring & 0xffffff;
	l = (long) &lp->init_block;
	outw(ioaddr+LANCE_ADDR, 0x1);
	(void)inw(ioaddr+LANCE_ADDR);
	outw(ioaddr+LANCE_DATA, (unsigned short)l);
	outw(ioaddr+LANCE_ADDR, 0x2);
	(void)inw(ioaddr+LANCE_ADDR);
	outw(ioaddr+LANCE_DATA, (unsigned short)(l >> 16));
	outw(ioaddr+LANCE_ADDR, 0x4);
	(void)inw(ioaddr+LANCE_ADDR);
	outw(ioaddr+LANCE_DATA, 0x915);
	outw(ioaddr+LANCE_ADDR, 0x0);
	(void)inw(ioaddr+LANCE_ADDR);
	/* now probe for DMA channel */
	dma_channels = ((inb(DMA1_STAT_REG) >> 4) & 0xf) |
		(inb(DMA2_STAT_REG) & 0xf0);
	/* need to fix when PCI provides DMA info */
	for (i = 0; i < (sizeof(dmas)/sizeof(dmas[0])); ++i)
	{
		int		j;

		dma = dmas[i];
		/* Don't enable a permanently busy DMA channel,
		   or the machine will hang */
		if (dma_channels & (1 << dma))
			continue;
		outw(ioaddr+LANCE_DATA, 0x7f04);	/* clear memory error bits */
		set_dma_mode(dma, DMA_MODE_CASCADE);
		enable_dma(dma);
		outw(ioaddr+LANCE_DATA, 0x1);		/* init */
		for (j = 100; j > 0; --j)
			if (inw(ioaddr+LANCE_DATA) & 0x900)
				break;
		if (inw(ioaddr+LANCE_DATA) & 0x100)
			break;
		else
			disable_dma(dma);
	}
	if (i >= (sizeof(dmas)/sizeof(dmas[0])))
		dma = 0;
	print("\r\n%s base 0x%x, DMA %d, addr ",
		chip_table[lance_version].name, ioaddr, dma);
	/* Get station address */
	for (i = 0; i < ETHER_ADDR_SIZE; ++i)
	{
		print("%b", nic->node_addr[i] = inb(ioaddr+i));
		if (i < ETHER_ADDR_SIZE -1)
			print(":");
	}
	print("\r\n");
	return (lance_version);
}

/**************************************************************************
PROBE - Look for an adapter, this routine's visible to the outside
***************************************************************************/
struct nic *lance_probe(struct nic *nic, unsigned short *probe_addrs)
{
	unsigned short		*p;
	static unsigned short	io_addrs[] = { 0x300, 0x320, 0x340, 0x360, 0 };

	/* if probe_addrs is 0, then routine can use a hardwired default */
	if (probe_addrs == 0)
		probe_addrs = io_addrs;
	for (p = probe_addrs; (ioaddr = *p) != 0; ++p)
	{
		char		offset15, offset14 = inb(ioaddr + 14);

		if ((offset14 == 0x52 || offset14 == 0x57) &&
		 ((offset15 = inb(ioaddr + 15)) == 0x57 || offset15 == 0x44))
			if (lance_probe1(nic) >= 0)
				break;
	}

	/* if board found */
	if (ioaddr != 0)
	{
		/* point to NIC specific routines */
		lance_reset(nic);
		nic->reset = lance_reset;
		nic->poll = lance_poll;
		nic->transmit = lance_transmit;
		nic->disable = lance_disable;
		return nic;
	}
	/* else */
	{
		return 0;
	}
}
