#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "netlink.h"

#include "pcie_defs.h"

unsigned int getnum(char *str)
{
	unsigned int ret;
	if (str[0]=='0' && (str[1]=='x' || str[1]=='X') )
		sscanf(str,"%x",&ret);
	else
		ret = atol( str );
	return ret;
}

int main(int argc, char *argv[])
{
	char *str = "Mr. Kernel, Are you ready (len=37) ??";
	char str2[255];
	unsigned int *ptr = (unsigned int *)str2;
	
	unsigned int cmd, len;
	
	int fd, i;
	netlink_packet_t rxpkt;

	// Prepare RX structure (reserve memory)
	rxpkt.data = str2;

	fd = netlink_init();
	if (fd < 0) {
		printf("ERROR: netlink_init() failed.\n");
		return 0;
	}
	
	if (argc == 2) {
		cmd = getnum( argv[1] );
		len = 0;
		str = NULL;
	} else if (argc == 3) {
		cmd = getnum( argv[1] );
		len = strlen(argv[2]) +1;
		str = argv[2];
	} else {
		printf ("\nPossible Arguments:\n");
		printf("  %s  <CONTROL1 command>\n", argv[0]);
		printf("  %s  <CONTROL1 command> <Teststring>\n\n", argv[0]);
		printf("\nsome CONTROL1 commands:\n");
		printf("0x1    : print dummy e2p in kernel log\n");
		printf("0x11   : Trigger COPY_VARS (with DMA in both directions)\n");
		printf("0x1013 <message> : NETLINK_ECHO through User->Kernel->ExMIMO->Kernel->User\n");
		printf("0x12   <message> : Only User->Kernel->User echo\n");
				
		printf("\nSending IRQ_FROM_PCI_KERNEL_ECHO..\n");
		cmd = IRQ_FROM_PCI_KERNEL_ECHO;
		len = strlen(str) +1;
		//netlink_send(fd, IRQ_FROM_PCI_KERNEL_ECHO, /*incl.\0*/, str);
		//netlink_send(fd, IRQ_FROM_PCI_NETLINK_EXMIMO_ECHO, strlen( str )+1 /*incl.\0*/, str);
	}
	
	printf ("Sending: cmd=%u, len=%u, str=_%s_\n", cmd, len, (len!=0? str : "<empty>"));
	netlink_send(fd,  cmd, len, str);

	i = netlink_recv(fd, &rxpkt, NETLINK_BLOCKING);
	if (i>=0)
		printf ("main(): netlink_send(str, len=%i).  Received Msg(pktlen=%i, cmd=%i, len=%i): %s\n",  len, i,  rxpkt.command, rxpkt.len, rxpkt.data );

/*	i = netlink_recv(fd, &rxpkt, NETLINK_NONBLOCKING);
	if (i>=0)
		printf ("main(): Received Msg(pktlen=%i, cmd=%i, len=%i): %s\n",  i,  rxpkt.command, rxpkt.len, rxpkt.data );
*/
	netlink_close(fd);
	return;
}
