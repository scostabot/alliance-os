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


#include "netboot.h"
#include "nic.h"
#include "ip.h"
#include "config.h"


#if     defined(INCLUDE_NEPCI) || defined(INCLUDE_EEPRO100)

#include "pci.h"

static unsigned short   pci_addrs[16];
struct pci_device       pci_nic_list[] = {
#ifdef  INCLUDE_NEPCI
    { PCI_VENDOR_ID_REALTEK, PCI_DEVICE_ID_REALTEK_8029, "Realtek 8029"},
    { PCI_VENDOR_ID_WINBOND2, PCI_DEVICE_ID_WINBOND2_89C940, "Winbond NE2000-PCI"},
    { PCI_VENDOR_ID_COMPEX, PCI_DEVICE_ID_COMPEX_RL2000, "Compex ReadyLink 2000"},
    { PCI_VENDOR_ID_KTI, PCI_DEVICE_ID_KTI_ET32P2, "KTI ET32P2"},
#endif
#ifdef  INCLUDE_EEPRO100
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82557, "Intel EtherExpressPro100"},
#endif
    {0,}
};

#endif  /* INCLUDE_*PCI */


char packet[ETH_MAX_PACKET];


/*
 *  NIC probing is in order of appearance in this table.
 *  If for some reason you want to change the order,
 *  just rearrange the entries (bracketed by the #ifdef/#endif)
 */
    
struct dispatch_table {
    char *nic_name;
    struct nic *(*eth_probe)(struct nic *, unsigned short *);
    unsigned short *probe_addrs;
};

#define NIC_TABLE_SIZE  (sizeof(NIC)/sizeof(NIC[0]))

static struct dispatch_table NIC[] = {
    #ifdef INCLUDE_WD
        { "WD", wd_probe, 0 },
    #endif
    #ifdef INCLUDE_T503
        { "3C503", t503_probe, 0 },
    #endif
    #ifdef INCLUDE_NE
        { "NE*000", ne_probe, 0 },
    #endif
    #ifdef INCLUDE_T509
        { "3C5x9", t509_probe, 0 },
    #endif
    #ifdef INCLUDE_EEPRO100
        { "EEPRO100", eepro100_probe, 0 },
    #endif
    #ifdef INCLUDE_CS
        { "CS89x0", cs89x0_probe, 0 },
    #endif
    #ifdef INCLUDE_NE2100
        { "NE2100", ne2100_probe, 0 },
    #endif
    #ifdef INCLUDE_NEPCI
        { "NE*000/PCI", ne_probe, pci_addrs },
    #endif
    /* this entry must always be last to mark the end of list */
        { 0, 0, 0 }
};


static int eth_dummy(struct nic *nic)
{
    return (0);
}

struct nic nic = {
    (void (*)(struct nic *))eth_dummy,              /* reset      */
    eth_dummy,                                      /* poll       */
    (void (*)(struct nic *, char *, unsigned int,
              unsigned int, char *))eth_dummy,      /* transmit   */
    (void (*)(struct nic *))eth_dummy,              /* disable    */
    1,                                              /* aui        */
    arptable[ARP_CLIENT].node,                      /* node_addr  */
    packet,                                         /* packet     */
    0,                                              /* packetlen  */
    0                                               /* priv_data  */
};

void eth_reset(void)
{
    (*nic.reset)(&nic);
}

int eth_poll(void)
{
    return ((*nic.poll)(&nic));
}

void eth_transmit(char *d, unsigned int t, unsigned int s, char *p)
{
    (*nic.transmit)(&nic, d, t, s, p);
    twiddle();
}

void eth_disable(void)
{
    (*nic.disable)(&nic);
}

int eth_probe(void)
{
    struct dispatch_table *t;

#ifdef INCLUDE_PCI
    pci_addrs[0] = 0;
    eth_pci_init(pci_nic_list, pci_addrs);
    pci_addrs[1] = 0;
#endif

    print("Probing network interfaces... ");
    for (t = NIC; t->nic_name != 0; ++t) {
        print("[%s]", t->nic_name);
        if ((*t->eth_probe)(&nic, t->probe_addrs))
            return (1);
    }
    print("\n");
    return (0);
}


