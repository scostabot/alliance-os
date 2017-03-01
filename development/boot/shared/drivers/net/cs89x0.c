/* cs89x0.c: A Crystal Semiconductor CS89[02]0 driver for etherboot. */
/*
  This code is heavily based on the linux driver as written by
  Russell Nelson <nelson@crynwr.com> and Donald Becker
  <becker@cesdis.gsfc.nasa.gov> and modified by Mike Cruse
  <mcruse@cti-ltd.com>. That driver has been released under the
  conditions of the GNU Public License, thus this one is probably to
  be considered "derived work". Therefore, there are some legal
  obstacles in combining this file with etherboot's code, which is
  released under a BSD style license.

  So, if you want to actually use this code, you should make sure that
  you are aware of the legal implications. I release *my* work into
  the PUBLIC DOMAIN which implies that you can use it either under a
  BSD style license, or under the conditions of the GPL, or under any
  other conditions that you like. N.B. this does not apply to the
  parts that originate from other authors. So, you should probably
  contact them first and verify if they agree with your intended use.

  If you contacted all of the above authors and they agreed to give
  special permission for using this code under the conditions of a BSD
  style license, then please do let me know.

  ChangeLog:

  Thu Dec 6 22:40:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * disabled all "advanced" features; this should make the code more reliable

  * reorganized the reset function

  * always reset the address port, so that autoprobing will continue working

  * some cosmetic changes

  * 2.5
  
  Thu Dec 5 21:00:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * tested the code against a CS8900 card

  * lots of minor bug fixes and adjustments

  * this is the first release, that actually works! it still requires some
    changes in order to be more tolerant to different environments

  * 4
  
  Fri Nov 22 23:00:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * read the manuals for the CS89x0 chipsets and took note of all the
    changes that will be neccessary in order to adapt Russel Nelson's code
    to the requirements of a BOOT-Prom

  * 6

  Thu Nov 19 22:00:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * Synched with Russel Nelson's current code (v1.00)

  * 2

  Thu Nov 12 18:00:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * Cleaned up some of the code and tried to optimize the code size.

  * 1.5
  
  Sun Nov 10 16:30:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * First experimental release. This code compiles fine, but I
  have no way of testing whether it actually works.

  * I did not (yet) bother to make the code 16bit aware, so for
  the time being, it will only work for netboot-32.

  * 12
  
  */

#include "../../net/netboot.h"
#include "../../net/nic.h"
#include "../../net/ip.h"
#include "lio.h"
#include "cs89x0.h"

static unsigned short	eth_nic_base;
static unsigned long    eth_mem_start;
static unsigned short   eth_irq;
static unsigned short   eth_cs_type;	 /* one of: CS8900, CS8920, CS8920M  */
static unsigned short   eth_auto_neg_cnf;
static unsigned short   eth_adapter_cnf;
static unsigned short	eth_linectl;
static unsigned char	eth_vendor;

/*************************************************************************
	CS89x0 - specific routines
**************************************************************************/

static int inline readreg(portno)
	int portno;
{
	outw(eth_nic_base + ADD_PORT, portno);
	return inw(eth_nic_base + DATA_PORT);
}

static void inline writereg(portno,value)
	int portno;
	int value;
{
	outw(eth_nic_base + ADD_PORT, portno);
	outw(eth_nic_base + DATA_PORT, value);
	return;
}

/*************************************************************************
EEPROM access
**************************************************************************/

static int wait_eeprom_ready()
{
	long tmo = currticks() + 4*18L;
	
	/* check to see if the EEPROM is ready, a timeout is used -
	   just in case EEPROM is ready when SI_BUSY in the
	   PP_SelfST is clear */
	while(readreg(PP_SelfST) & SI_BUSY) {
		if (currticks() - tmo >= 0)
			return -1; }
	return 0;
}

static int get_eeprom_data(off,len,buffer)
	int off;
	int len;
	unsigned short *buffer;
{
	int i;

#if defined(EDEBUG)
	print("\r\ncs: EEPROM data from %x for %x:",off,len);
#endif
	for (i = 0; i < len; i++) {
		if (wait_eeprom_ready() < 0)
			return -1;
		/* Now send the EEPROM read command and EEPROM location
		   to read */
		writereg(PP_EECMD, (off + i) | EEPROM_READ_CMD);
		if (wait_eeprom_ready() < 0)
			return -1;
		buffer[i] = readreg(PP_EEData);
#if defined(EDEBUG)
		if (!(i%10))
			print("\r\ncs: ");
		print("%x ", buffer[i]);
#endif
	}
#if defined(EDEBUG)
	print("\r\n");
#endif

	return(0);
}

static int get_eeprom_chksum(off, len, buffer)
	int off;
	int len;
	unsigned short *buffer;
{
	int  i, cksum;

	cksum = 0;
	for (i = 0; i < len; i++)
		cksum += buffer[i];
	cksum &= 0xffff;
	if (cksum == 0)
		return 0;
	return -1;
}

/*************************************************************************
Activate all of the available media and probe for network
**************************************************************************/

static void clrline()
{
	int i;

	putchar('\r');
	for (i = 79; i--; ) putchar(' ');
	print("\rcs: ");
	return;
}

static void control_dc_dc(on_not_off)
	int on_not_off;
{
	unsigned int selfcontrol;
	long tmo = currticks() + 18;

	/* control the DC to DC convertor in the SelfControl register.  */
	selfcontrol = HCB1_ENBL; /* Enable the HCB1 bit as an output */
	if (((eth_adapter_cnf & A_CNF_DC_DC_POLARITY) != 0) ^ on_not_off)
		selfcontrol |= HCB1;
	else
		selfcontrol &= ~HCB1;
	writereg(PP_SelfCTL, selfcontrol);
  
	/* Wait for the DC/DC converter to power up - 1000ms */
	while (currticks() - tmo < 0);

	return;
}

static int detect_tp()
{
	long tmo;

	/* Turn on the chip auto detection of 10BT/ AUI */
  
	clrline(); print("attempting %s:","TP");

        /* If connected to another full duplex capable 10-Base-T card
	   the link pulses seem to be lost when the auto detect bit in
	   the LineCTL is set.  To overcome this the auto detect bit
	   will be cleared whilst testing the 10-Base-T interface.
	   This would not be necessary for the sparrow chip but is
	   simpler to do it anyway. */
	writereg(PP_LineCTL, eth_linectl &~ AUI_ONLY);
	control_dc_dc(0);

        /* Delay for the hardware to work out if the TP cable is
	   present - 150ms */
	for (tmo = currticks() + 4; currticks() - tmo < 0; );
	
	if ((readreg(PP_LineST) & LINK_OK) == 0)
		return 0;

	if (eth_cs_type != CS8900) {

		writereg(PP_AutoNegCTL, eth_auto_neg_cnf & AUTO_NEG_MASK);

		if ((eth_auto_neg_cnf & AUTO_NEG_BITS) == AUTO_NEG_ENABLE) {
			print(" negotiating duplex... ");
			while (readreg(PP_AutoNegST) & AUTO_NEG_BUSY) {
				if (currticks() - tmo > 40*18) {
					print("time out ");
					break;
				}
			}
		}
		if (readreg(PP_AutoNegST) & FDX_ACTIVE)
			print("using full duplex");
		else
			print("using half duplex");
	}

	return A_CNF_MEDIA_10B_T;
}

/* send a test packet - return true if carrier bits are ok */
static int send_test_pkt(struct nic *nic)
{
	static char testpacket[] = { 0,0,0,0,0,0, 0,0,0,0,0,0,
				     0, 46, /*A 46 in network order       */
				     0, 0,  /*DSAP=0 & SSAP=0 fields      */
				     0xf3,0 /*Control (Test Req+P bit set)*/ };
	long tmo;

	writereg(PP_LineCTL, readreg(PP_LineCTL) | SERIAL_TX_ON);

	bcopy(nic->node_addr,testpacket,ETHER_ADDR_SIZE);
	bcopy(nic->node_addr,testpacket+ETHER_ADDR_SIZE,
	      ETHER_ADDR_SIZE);

	outw(eth_nic_base + TX_CMD_PORT, TX_AFTER_ALL);
	outw(eth_nic_base + TX_LEN_PORT, ETH_MIN_PACKET);

	/* Test to see if the chip has allocated memory for the packet */
	for (tmo = currticks() + 2;
	     (readreg(PP_BusST) & READY_FOR_TX_NOW) == 0; )
		if (currticks() - tmo >= 0)
			return(0);

	/* Write the contents of the packet */
	outsw(eth_nic_base + TX_FRAME_PORT, testpacket,
	      (ETH_MIN_PACKET+1)>>1);

	print(" sending test packet ");
	/* wait a couple of timer ticks for packet to be received */
	for (tmo = currticks() + 2; currticks() - tmo < 0; );
	
	if ((readreg(PP_TxEvent) & TX_SEND_OK_BITS) == TX_OK) {
			print("succeeded");
			return 1;
	}
	print("failed");
	return 0;
}


static int detect_aui(struct nic *nic)
{
	clrline(); print("attempting %s:","AUI");
	control_dc_dc(0);

	writereg(PP_LineCTL, (eth_linectl & ~AUTO_AUI_10BASET) | AUI_ONLY);

	if (send_test_pkt(nic)) {
		return A_CNF_MEDIA_AUI; }
	else
		return 0;
}

static int detect_bnc(struct nic *nic)
{
	clrline(); print("attempting %s:","BNC");
	control_dc_dc(1);

	writereg(PP_LineCTL, (eth_linectl & ~AUTO_AUI_10BASET) | AUI_ONLY);

	if (send_test_pkt(nic)) {
		return A_CNF_MEDIA_10B_2; }
	else
		return 0;
}

/**************************************************************************
ETH_RESET - Reset adapter
***************************************************************************/

void cs89x0_reset(struct nic *nic)
{
	int  i;
	long reset_tmo;

	if(eth_vendor!=VENDOR_CS89x0)
		return;

	writereg(PP_SelfCTL, readreg(PP_SelfCTL) | POWER_ON_RESET);

	/* wait for two ticks; that is 2*55ms */
	for (reset_tmo = currticks() + 2;
	     reset_tmo > currticks(); );

	if (eth_cs_type != CS8900) {
		/* Hardware problem requires PNP registers to be reconfigured
		   after a reset */
		if (eth_irq != 0xFFFF) {
			outw(eth_nic_base + ADD_PORT, PP_CS8920_ISAINT);
			outb(eth_nic_base + DATA_PORT, eth_irq);
			outb(eth_nic_base + DATA_PORT + 1, 0); }

		if (eth_mem_start) {
			outw(eth_nic_base + ADD_PORT, PP_CS8920_ISAMemB);
			outb(eth_nic_base + DATA_PORT, (eth_mem_start >> 8) & 0xff);
			outb(eth_nic_base + DATA_PORT + 1, (eth_mem_start >> 24) & 0xff); } }
  
	/* Wait until the chip is reset */
	for (reset_tmo = currticks() + 2;
	     (readreg(PP_SelfST) & INIT_DONE) == 0 &&
		     currticks() - reset_tmo < 0; );

	/* disable interrupts and memory accesses */
	writereg(PP_BusCTL, 0);

	/* set the ethernet address */
	for (i=0; i < ETHER_ADDR_SIZE/2; i++)
		writereg(PP_IA+i*2,
			 nic->node_addr[i*2] |
			 (nic->node_addr[i*2+1] << 8));

	/* receive only error free packets addressed to this card */
	writereg(PP_RxCTL, DEF_RX_ACCEPT);

	/* do not generate any interrupts on receive operations */
	writereg(PP_RxCFG, 0);

	/* do not generate any interrupts on transmit operations */
	writereg(PP_TxCFG, 0);

	/* do not generate any interrupts on buffer operations */
	writereg(PP_BufCFG, 0);

	/* reset address port, so that autoprobing will keep working */
	outw(eth_nic_base + ADD_PORT, PP_ChipID);

	return;
}

/**************************************************************************
ETH_TRANSMIT - Transmit a frame
***************************************************************************/

static void cs89x0_transmit(
	struct nic *nic,
	char *d,			/* Destination */
	unsigned int t,			/* Type */
	unsigned int s,			/* size */
	char *p)			/* Packet */
{
	long          tmo;
	int           sr;

	if(eth_vendor!=VENDOR_CS89x0)
		return;

	/* does this size have to be rounded??? please,
	   somebody have a look in the specs */
	if ((sr = ((s + ETHER_HDR_SIZE + 1)&~1)) < ETH_MIN_PACKET)
		sr = ETH_MIN_PACKET;

retry:
	/* initiate a transmit sequence */
	outw(eth_nic_base + TX_CMD_PORT, TX_AFTER_ALL);
	outw(eth_nic_base + TX_LEN_PORT, sr);

	/* Test to see if the chip has allocated memory for the packet */
	if ((readreg(PP_BusST) & READY_FOR_TX_NOW) == 0) {
		/* Oops... this should not happen! */
		print("cs: unable to send packet; retrying...\r\n");
		for (tmo = currticks() + 5*18; currticks() - tmo < 0; );
		cs89x0_reset(nic);
		goto retry; }

	/* Write the contents of the packet */
	outsw(eth_nic_base + TX_FRAME_PORT, d, ETHER_ADDR_SIZE/2);
	outsw(eth_nic_base + TX_FRAME_PORT, nic->node_addr,
	      ETHER_ADDR_SIZE/2);
	outw(eth_nic_base + TX_FRAME_PORT, ((t >> 8)&0xFF)|(t << 8));
	outsw(eth_nic_base + TX_FRAME_PORT, p, (s+1)/2);
	for (sr = sr/2 - (s+1)/2 - ETHER_ADDR_SIZE - 1; sr-- > 0;
	     outw(eth_nic_base + TX_FRAME_PORT, 0));

	/* wait for transfer to succeed */
	for (tmo = currticks()+5*18;
	     (s = readreg(PP_TxEvent)&~0x1F) == 0 && currticks() - tmo < 0;
	     twiddle());
	if ((s & TX_SEND_OK_BITS) != TX_OK) {
		print("\r\ntransmission error 0x%x\r\n", s);
	}

	return;
}

/**************************************************************************
ETH_POLL - Wait for a frame
***************************************************************************/

static int cs89x0_poll(struct nic *nic)
{
	int status;

	if(eth_vendor!=VENDOR_CS89x0)
		return 0;

	status = readreg(PP_RxEvent);

	if ((status & RX_OK) == 0)
		return(0);

	status = inw(eth_nic_base + RX_FRAME_PORT);
	nic->packetlen = inw(eth_nic_base + RX_FRAME_PORT);
	insw(eth_nic_base + RX_FRAME_PORT, nic->packet, nic->packetlen >> 1);
	if (nic->packetlen & 1)
		nic->packet[nic->packetlen-1] = inw(eth_nic_base + RX_FRAME_PORT);
	return 1;
}

static void cs89x0_disable(struct nic *nic)
{
}

/**************************************************************************
ETH_PROBE - Look for an adapter
***************************************************************************/

struct nic *cs89x0_probe(struct nic *nic, unsigned short *probe_addrs)
{
	static const unsigned int netcard_portlist[] = {
#ifdef CS_SCAN
		CS_SCAN,
#else /* use "conservative" default values for autoprobing */
		0x300,0x320,0x340,0x200,0x220,0x240,
		0x260,0x280,0x2a0,0x2c0,0x2e0,
      /* if that did not work, then be more aggressive */
		0x301,0x321,0x341,0x201,0x221,0x241,
		0x261,0x281,0x2a1,0x2c1,0x2e1,
#endif
		0};

	int      i, result = -1;
	unsigned rev_type = 0, ioaddr, ioidx, isa_cnf, cs_revision;
	unsigned short eeprom_buff[CHKSUM_LEN];


	for (eth_vendor = VENDOR_NONE, ioidx = 0;
	     eth_vendor == VENDOR_NONE &&
		     (ioaddr=netcard_portlist[ioidx++]) != 0;) {
		/* if they give us an odd I/O address, then do ONE write to
		   the address port, to get it back to address zero, where we
		   expect to find the EISA signature word. */
		if (ioaddr & 1) {
			ioaddr &= ~1;
			if ((inw(ioaddr + ADD_PORT) & ADD_MASK) != ADD_SIG)
				continue;
			outw(ioaddr + ADD_PORT, PP_ChipID);
		}

		if (inw(ioaddr + DATA_PORT) != CHIP_EISA_ID_SIG)
			continue;
		eth_nic_base = ioaddr;

		/* get the chip type */
		rev_type = readreg(PRODUCT_ID_ADD);
		eth_cs_type = rev_type &~ REVISON_BITS;
		cs_revision = ((rev_type & REVISON_BITS) >> 8) + 'A';

		print("\r\ncs: cs89%c0%s rev %c, base 0x%x",
		       eth_cs_type==CS8900?'0':'2',
		       eth_cs_type==CS8920M?"M":"",
		       cs_revision,
		       eth_nic_base);
    
		/* First check to see if an EEPROM is attached*/
		if ((readreg(PP_SelfST) & EEPROM_PRESENT) == 0) {
			print("\r\ncs: no EEPROM...\r\n");
			outw(eth_nic_base + ADD_PORT, PP_ChipID);
			continue; }
		else if (get_eeprom_data(START_EEPROM_DATA,CHKSUM_LEN,
					 eeprom_buff) < 0) {
			print("\r\ncs: EEPROM read failed...\r\n");
			outw(eth_nic_base + ADD_PORT, PP_ChipID);
			continue; }
		else if (get_eeprom_chksum(START_EEPROM_DATA,CHKSUM_LEN,
					   eeprom_buff) < 0) {
			print("\r\ncs: EEPROM checksum bad...\r\n");
			outw(eth_nic_base + ADD_PORT, PP_ChipID);
			continue; }

		/* get transmission control word but keep the
		   autonegotiation bits */
		eth_auto_neg_cnf = eeprom_buff[AUTO_NEG_CNF_OFFSET/2];
		/* Store adapter configuration */
		eth_adapter_cnf = eeprom_buff[ADAPTER_CNF_OFFSET/2];
		/* Store ISA configuration */
		isa_cnf = eeprom_buff[ISA_CNF_OFFSET/2];

		/* store the initial memory base address */
		eth_mem_start = eeprom_buff[PACKET_PAGE_OFFSET/2] << 8;
    
		print("%s%s%s, addr ",
		       (eth_adapter_cnf & A_CNF_10B_T)?", RJ-45":"",
		       (eth_adapter_cnf & A_CNF_AUI)?", AUI":"",
		       (eth_adapter_cnf & A_CNF_10B_2)?", BNC":"");
  
		/* If this is a CS8900 then no pnp soft */
		if (eth_cs_type != CS8900 &&
		    /* Check if the ISA IRQ has been set  */
		    (i = readreg(PP_CS8920_ISAINT) & 0xff,
		     (i != 0 && i < CS8920_NO_INTS)))
			eth_irq = i;
		else {
			i = isa_cnf & INT_NO_MASK;
			if (eth_cs_type == CS8900) {
				/* the table that follows is dependent
				   upon how you wired up your cs8900
				   in your system.  The table is the
				   same as the cs8900 engineering demo
				   board.  irq_map also depends on the
				   contents of the table.  Also see
				   write_irq, which is the reverse
				   mapping of the table below. */
				if (i < 4) i = "\012\013\014\005"[i];
				else print("\r\ncs: BUG: isa_config is %d\r\n", i); }
			eth_irq = i; }

		/* Retrieve and print the ethernet address. */
    		for (i=0; i<ETHER_ADDR_SIZE; i++) {
			print("%b%s",(int)(nic->node_addr[i] =
					  ((unsigned char *)eeprom_buff)[i]),
			       i < ETHER_ADDR_SIZE-1 ? ":" : "\r\n"); }

		/* Set the LineCTL quintuplet based on adapter
		   configuration read from EEPROM */
		if ((eth_adapter_cnf & A_CNF_EXTND_10B_2) &&
		    (eth_adapter_cnf & A_CNF_LOW_RX_SQUELCH))
			eth_linectl = LOW_RX_SQUELCH;
		else
			eth_linectl = 0;

		/* check to make sure that they have the "right"
		   hardware available */
		switch(eth_adapter_cnf & A_CNF_MEDIA_TYPE) {
		case A_CNF_MEDIA_10B_T: result = eth_adapter_cnf & A_CNF_10B_T;
			break;
		case A_CNF_MEDIA_AUI:   result = eth_adapter_cnf & A_CNF_AUI;
			break;
		case A_CNF_MEDIA_10B_2: result = eth_adapter_cnf & A_CNF_10B_2;
			break;
		default: result = eth_adapter_cnf & (A_CNF_10B_T | A_CNF_AUI | 
						     A_CNF_10B_2);
		}
		if (!result) {
			print("cs: EEPROM is configured for unavailable media\r\n");
		error:
			writereg(PP_LineCTL, readreg(PP_LineCTL) & 
				 ~(SERIAL_TX_ON | SERIAL_RX_ON));
			outw(eth_nic_base + ADD_PORT, PP_ChipID);
			continue;
		}

		/* Initialize the card for probing of the attached media */
		eth_vendor = VENDOR_CS89x0;
		cs89x0_reset(nic);
		eth_vendor = VENDOR_NONE;
		
		/* set the hardware to the configured choice */
		switch(eth_adapter_cnf & A_CNF_MEDIA_TYPE) {
		case A_CNF_MEDIA_10B_T:
			result = detect_tp();
			if (!result) {
				clrline();
				print("10Base-T (RJ-45%s",
				       ") has no cable\r\n"); }
			/* check "ignore missing media" bit */
			if (eth_auto_neg_cnf & IMM_BIT)
				/* Yes! I don't care if I see a link pulse */
				result = A_CNF_MEDIA_10B_T;
			break;
		case A_CNF_MEDIA_AUI:
			result = detect_aui(nic);
			if (!result) {
				clrline();
				print("10Base-5 (AUI%s",
				       ") has no cable\r\n"); }
			/* check "ignore missing media" bit */
			if (eth_auto_neg_cnf & IMM_BIT)
				/* Yes! I don't care if I see a carrrier */
				result = A_CNF_MEDIA_AUI;
			break;
		case A_CNF_MEDIA_10B_2:
			result = detect_bnc(nic);
			if (!result) {
				clrline();
				print("10Base-2 (BNC%s",
				       ") has no cable\r\n"); }
			/* check "ignore missing media" bit */
			if (eth_auto_neg_cnf & IMM_BIT)
				/* Yes! I don't care if I can xmit a packet */
				result = A_CNF_MEDIA_10B_2;
			break;
		case A_CNF_MEDIA_AUTO:
			writereg(PP_LineCTL, eth_linectl | AUTO_AUI_10BASET);
			if (eth_adapter_cnf & A_CNF_10B_T)
				if ((result = detect_tp()) != 0)
					break;
			if (eth_adapter_cnf & A_CNF_AUI)
				if ((result = detect_aui(nic)) != 0)
					break;
			if (eth_adapter_cnf & A_CNF_10B_2)
				if ((result = detect_bnc(nic)) != 0)
					break;
			clrline(); print("no media detected\r\n");
			goto error;
		}
		clrline();
		switch(result) {
		case 0:                 print("no network cable attached to configured media\r\n");
			goto error;
		case A_CNF_MEDIA_10B_T: print("using 10Base-T (RJ-45)\r\n");
			break;
		case A_CNF_MEDIA_AUI:   print("using 10Base-5 (AUI)\r\n");
			break;
		case A_CNF_MEDIA_10B_2: print("using 10Base-2 (BNC)\r\n");
			break;
		}

		/* Turn on both receive and transmit operations */
		writereg(PP_LineCTL, readreg(PP_LineCTL) | SERIAL_RX_ON |
			 SERIAL_TX_ON);

		eth_vendor = VENDOR_CS89x0;
	}

        if (eth_vendor == VENDOR_NONE) return(0);

	nic->reset = cs89x0_reset;
	nic->poll = cs89x0_poll;
	nic->transmit = cs89x0_transmit;
	nic->disable = cs89x0_disable;
	return nic;
}

/*
 * Local variables:
 *  c-basic-offset: 8
 * End:
 */

