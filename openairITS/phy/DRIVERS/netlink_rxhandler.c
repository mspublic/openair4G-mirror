/** netlink_rxhandler.c: 
      - RX Handler in Kernel: Receive Data from Userspace to Kernel: netlink_rx_handler( char *msg, int len)
      - sends received messages through PCIe to Leon
  */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/pci.h>
#include "netlink.h"
#include "extern.h"
#include "proto.h"

void netlink_rx_handler( char *msg, int len)
{
	unsigned int *p32;
	netlink_packet_t nlp;
	
	p32 = (unsigned int *)msg;

	nlp.command = p32[0];
	nlp.len     = p32[1];	// cannot use (packet) len, as len is always multiple of 4
	nlp.data    = msg + 8;
	
	printk( "[EXMIMO]: netlink_rx_handler(): netlink packet received(len=%i), pktlen=l-8=%i, cmd=%i: nlp.data = _%s_\n", len, nlp.len, nlp.command, ( (nlp.len>0 && nlp.len<50) ? nlp.data : "<empty>"));
	if (len < nlp.len + 8)
		printk( "[EXMIMO]: netlink_rx_handler(): ERROR: received packet is too short!\n");

	if ( nlp.command == IRQ_FROM_PCI_KERNEL_ECHO )
	{
		netlink_send( msg, nlp.len +8/*+cmd+len*/);
	}
	else if ( nlp.command == IRQ_FROM_PCI_KERNEL_DUMP_PCIVARS )
	{
		volatile unsigned int *p32 = (volatile unsigned int *)pcie_vars_interface;
		volatile unsigned char *p8 = (volatile unsigned char *)pcie_vars_interface;
		int i;

		pci_dma_sync_single_for_cpu(pdev[0], pcie_vars_interface_dma_handle, sizeof(pcie_vars_interface_t), PCI_DMA_FROMDEVICE);
		//pci_unmap_single(pdev[0],pcie_vars_interface_dma_handle,sizeof(pcie_buf_pcie2exmimo_t), PCI_DMA_BIDIRECTIONAL);
		printk( "[EXMIMO]: pcie_vars->addr_buf_pcie2exmimo = 0x%08X     pcie_vars->addr_buf_exmimo2pcie = 0x%08X\n", p32[0], p32[1]);
		printk(   "[EXMIMO]: dummy_p2e[0..39]: ");    for (i=0; i<40; i++) printk("%02X ", p8[2*4+i]);
		printk( "\n[EXMIMO]: dummy_e2p[0..39]: before: "); for (i=0; i<40; i++) printk("%02X ", 0xDD); printk("\n");
		printk( "\n[EXMIMO]: dummy_e2p[0..39]: now:    "); for (i=0; i<40; i++) printk("%02X ", p8[(2+10)*4+i]); printk("\n");
		printk( "\n[EXMIMO]: dummy_e2p[0..39]: expectd:"); for (i=0; i<40; i++) printk("%02X ", i+17 ); printk("\n");
	}
	else if ( IRQ_FROM_PCI_IS_NETLINKCMD( nlp.command ) ) 
	{
		volatile unsigned char *p8 = (volatile unsigned char *) pcie_buf_pcie2exmimo;
		volatile unsigned int *p32 = (volatile unsigned int *)  pcie_buf_pcie2exmimo;
		if (len > PCIE_PCIE2EXMIMO_MAXLEN) {
			printk("[EXMIMO]: Warning: Length of RX Netlink packet too long (%i > %i). Will be truncated!\n", len, PCIE_PCIE2EXMIMO_MAXLEN);
			len = PCIE_PCIE2EXMIMO_MAXLEN;
		}
		pci_dma_sync_single_for_device(pdev[0], pcie_buf_pcie2exmimo_dma_handle,sizeof(pcie_buf_pcie2exmimo_t), PCI_DMA_TODEVICE);
		//pci_map_single(pdev[0],pcie_buf_pcie2exmimo_dma_handle,sizeof(pcie_buf_pcie2exmimo_t), PCI_DMA_TODEVICE);
#ifndef NOCARD_TEST
		pcie_buf_pcie2exmimo->counter++;
		memcpy( (void*) &(pcie_buf_pcie2exmimo->command) /*cmd-len-data*/, msg, nlp.len +8/*+cmd+len*/ );
		exmimo_reorder_buffer( pcie_buf_pcie2exmimo->len, pcie_buf_pcie2exmimo->data);
		printk ("[EXMIMO]: Will send netlink packet to Exmimo: cnt:%08X cmd:%08X len:%08X data (bytewise):%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n", p32[0], p32[1], p32[2], p8[12+0], p8[12+1], p8[12+2], p8[12+3], p8[12+4], p8[12+5], p8[12+6], p8[12+7], p8[12+8], p8[12+9] );
#endif
		//exmimo_sendirqcmd( 0, nlp.command );
	} else {
		printk ("[EXIMO]: Unknown command %i. Will send it to exmimo\n", nlp.command);
		//exmimo_sendirqcmd( 0, nlp.command );
	}
}
