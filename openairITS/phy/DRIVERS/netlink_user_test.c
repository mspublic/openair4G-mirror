#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "netlink.h"

#include "pcie_defs.h"

/*
 * efone - Distributed internet phone system.
 * (c) 1999,2000 Krzysztof Dabrowski
 * (c) 1999,2000 ElysiuM deeZine
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
/* based on implementation by Finn Yannick Jacobs */
#include <stdio.h>
#include <stdlib.h>
u_int32_t crc_tab[256];
u_int32_t chksum_crc32 (unsigned char *block, unsigned int length) {
   register unsigned long crc;
   unsigned long i;

   crc = 0xFFFFFFFF;
   for (i = 0; i < length; i++)   {
      crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_tab[(crc ^ *block++) & 0xFF];
   }
   return (crc ^ 0xFFFFFFFF);
}
void chksum_crc32gentab () {
   unsigned long crc, poly;
   int i, j;

   poly = 0xEDB88320L;
   for (i = 0; i < 256; i++) {
      crc = i;
      for (j = 8; j > 0; j--)  {
	 if (crc & 1) {
	    crc = (crc >> 1) ^ poly;
	 } 	 else	 {
	    crc >>= 1;
	 } }
     crc_tab[i] = crc;
   }
}


int main(int argc, char *argv[])
{
	char *txmsg, *rxmsg;
	
	unsigned int cmd, len, msgsize;
	unsigned long mycrc32;
	
	int fd, i;
	netlink_packet_t rxpkt;

	chksum_crc32gentab();

	if (argc != 2) {
		printf ("ERROR: Missing argument: message size <bytes>\n");
		return 0;
	}
	
	msgsize = atol( argv[1] );
	
	
	txmsg = malloc( PCIE_MAXPAYLOADSIZE );
	rxmsg = malloc( PCIE_MAXPAYLOADSIZE );
	
	if (txmsg == NULL || rxmsg == NULL) {
		printf ("ERROR: Couldn't reserve enough memory.\n");
		return;
	}
	
	// Prepare RX structure (reserve memory)
	rxpkt.data = rxmsg;

	if (msgsize > PCIE_MAXPAYLOADSIZE-4) {
		printf ("WARNING: msg too large. size truncated to: %u bytes\n", PCIE_MAXPAYLOADSIZE-4);
		msgsize = PCIE_MAXPAYLOADSIZE-4;
	}
	
	fd = netlink_init();
	if (fd < 0) {
		printf("ERROR: netlink_init() failed\n");
		return 0;
	}
	
	srand ( time(NULL) );
	for (i=4; i<msgsize; i++)
		txmsg[i] = rand() % 256;
	
	mycrc32 = chksum_crc32( &txmsg[4], msgsize-4);
	* ((unsigned long*) txmsg) = mycrc32;
	
	printf ("TX: CRC32 over %u bytes: %lX\n", msgsize-4, mycrc32);
	
	printf("Sending message...\n");
	
	netlink_send(fd,  IRQ_FROM_PCI_NETLINK_EXMIMO_ECHO, msgsize, txmsg );
	//netlink_send(fd,  IRQ_FROM_PCI_KERNEL_ECHO, msgsize, txmsg );
	
	printf("Waiting for echo...\n");
	
	i = netlink_recv(fd, &rxpkt, NETLINK_BLOCKING);
	printf ("main(): Received Msg, ret=%i, (cmd=%i, len=%i)\n",  i, rxpkt.command, rxpkt.len );
	
	mycrc32 = chksum_crc32( &rxmsg[4], rxpkt.len-4);
	printf ("RX: CRC32 of received packet = %lX\n", mycrc32);

	netlink_close(fd);
	
	free( rxmsg );
	free( txmsg );
	
	return;
}
