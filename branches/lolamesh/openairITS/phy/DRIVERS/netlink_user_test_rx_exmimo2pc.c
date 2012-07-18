/** Waits for packets from Exmimo over netlink */

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

#include "netlink.h"

#include "pcie_defs.h"

int wanttoexit=0;

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

char *rxmsg;
int fd;

void exit_program(int sig)
{
    printf("\nClosing netlink socket.\n");
    netlink_close(fd);
    free( rxmsg );
    (void) signal(SIGINT, SIG_DFL);
    exit( 0 );
}

int main(int argc, char *argv[])
{
	unsigned int cmd, len, msgsize;
	unsigned long mycrc32;
	unsigned int *ui32ptr;
	
	int i;
	netlink_packet_t rxpkt;

	chksum_crc32gentab();
	
	(void) signal(SIGINT, exit_program);

	printf ("Waiting for Netlink packet... Press Ctrl+C to exit\n");
	
	rxmsg = malloc( PCIE_MAXPAYLOADSIZE );
	
	if (rxmsg == NULL) {
		printf ("ERROR: Couldn't reserve enough memory.\n");
		return;
	}
	
	// Prepare RX structure (reserve memory)
	rxpkt.data = rxmsg;
	ui32ptr    = (unsigned int *) rxmsg;

	fd = netlink_init();
	if (fd < 0) {
		printf("ERROR: netlink_init() failed\n");
		return 0;
	}
	
	while ( 1 )
	{
		i = netlink_recv(fd, &rxpkt, NETLINK_BLOCKING);
		printf ("main(): Received Msg, ret=%i, (cmd=%i, len=%i, data=%08X %08X %08X %08X)\n",  i, rxpkt.command, rxpkt.len, ui32ptr[0], ui32ptr[1], ui32ptr[2], ui32ptr[3] );
		
		if (rxpkt.command == IRQ_FROM_PCI_NETLINK_EXMIMO_ECHO )
		{
			mycrc32 = chksum_crc32( &rxmsg[4], rxpkt.len-4);
			printf ("RX: CRC32 of received packet = received:%08X, calculated:%08lX ==> CRC %s\n", ui32ptr[0], mycrc32, (ui32ptr[0] == mycrc32 ? "FAILED" : "OK") );
		}
	}
	return;
}
