/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file otg_rx_socket.c
* \brief function containing the OTG RX traffic generation functions with sockets
* \author A. Hafsaoui
* \date 2012
* \version 0.1
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning
*/








 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

#include "otg_config.h"
#include "otg_rx_socket.h"


 
 


/*
void packet_send(int src, int dst, int state){


	if (g_otg->ip_v[1]==0) { 
		if (g_otg->trans_proto[1]==0)
			server_socket_tcp_ip4();
		else
			server_socket_tcp_ip6();

	}
	else {	
		if (g_otg->trans_proto[1]==0)
			server_socket_udp_ip4();
		else
			server_socket_udp_ip6();
	}

}
 
*/

void server_socket_tcp_ip4()
{
#if defined (WIN32)
    #include <winsock2.h>
    typedef int socklen_t;
#elif defined (linux)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
#endif
#define PORT 7777


#if defined (WIN32)
        WSADATA WSAData;
        int erreur = WSAStartup(MAKEWORD(2,2), &WSAData);
    #else
        int erreur = 0;
    #endif

    SOCKADDR_IN sin;
    SOCKET sock;
    int recsize = sizeof sin;

    int sock_err, sock_rcv;
    char *buffer; /* a 1K buffer */


    if(!erreur)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);

        if(sock != INVALID_SOCKET)
        {
            printf("Socket number= %d  is opend using TCP and IPv4\n", sock);

            sin.sin_addr.s_addr = htonl(INADDR_ANY);
            sin.sin_family = AF_INET;
            sin.sin_port = htons(PORT);
            sock_err = bind(sock, (SOCKADDR*) &sin, recsize);

            if(sock_err != SOCKET_ERROR)
            {
                sock_err = listen(sock, 5);
                printf("Port Open %d...\n", PORT);

                if(sock_err != SOCKET_ERROR)
                { 
			int cmpt_cl=1;

                    /* Création de l'ensemble de lecture */
                    fd_set readfs;

                    while(1)
                    {
                        /* On vide l'ensemble de lecture et on lui ajoute 
                        la socket serveur */
                        FD_ZERO(&readfs);
                        FD_SET(sock, &readfs);

                        /* Si une erreur est survenue au niveau du select */
                        if(select(sock + 1, &readfs, NULL, NULL, NULL) < 0)
                        {
                            perror("select()");
                            exit(errno);
                        }

                        /* On regarde si la socket serveur contient des 
                        informations à lire */
                        if(FD_ISSET(sock, &readfs))
                        {
                            /* Ici comme c'est la socket du serveur cela signifie 
                            forcement qu'un client veut se connecter au serveur. 
                            Dans le cas d'une socket cliente c'est juste des 
                            données qui serons reçues ici*/

                            SOCKADDR_IN csin;
                            int crecsize = sizeof csin;

                            /* Juste pour l'exemple nous acceptons le client puis 
                            nous refermons immédiatement après la connexion */
                            SOCKET csock = accept(sock, (SOCKADDR *) &csin, &crecsize);

//
		do{     buffer=malloc(MAXSIZE);
			sock_rcv=recv(csock, buffer, MAXSIZE, 0);
	            /* Si l'on reçoit des informations : on les affiche à l'écran */
	            if(sock_rcv != SOCKET_ERROR)
	              printf("Received Payload: %s\n", buffer);      
          		free(buffer);	
		}while(sock_rcv>0 );

//
                            closesocket(csock);

                            printf("Client n=%d finish transmission\n", cmpt_cl);
				cmpt_cl+=1;
                        }
                    }
                }
            }
        }
    }

    #if defined (WIN32)
        WSACleanup();
    #endif

}


void server_socket_udp_ip4()
{


int sockfd, cc, addr_in_size;
  u_short portnum = 12345;
  struct sockaddr_in *my_addr, *from;
  char *msg;
  u_long fromaddr;

  addr_in_size = sizeof(struct sockaddr_in);

  msg = (char *)malloc(MAXSIZE);
  from = (struct sockaddr_in *)malloc(addr_in_size);
  my_addr = (struct sockaddr_in *)malloc(addr_in_size);

  memset((char *)my_addr,(char)0,addr_in_size);
  my_addr->sin_family = AF_INET;
  my_addr->sin_addr.s_addr = htonl(INADDR_ANY);
  my_addr->sin_port = portnum;

  if((sockfd = socket (PF_INET, SOCK_DGRAM, 0)) < 0){
    fprintf(stderr,"Error %d in socket: %s\n",errno,sys_errlist[errno]);
    exit(errno);
  };

  if(bind(sockfd, (struct sockaddr *)my_addr, addr_in_size) < 0){
    fprintf(stderr,"Error %d in bind: %s\n",errno,sys_errlist[errno]);
    if(errno != EADDRINUSE) exit(errno);
  };

  fprintf(stdout,"Ready to receive UDP traffic\n");

  while(1){
    if((cc = recvfrom (sockfd,msg,MAXSIZE,0,(struct sockaddr *)from,
           &addr_in_size)) == -1){
      fprintf(stderr,"Error %d in recvfrom: %s\n",
        errno,sys_errlist[errno]);
      exit(errno);
    };
    fromaddr = from->sin_addr.s_addr;
    msg[cc] = '\0';
    fprintf(stdout,"From %s port %d: %s\n",
      (gethostbyaddr((char *)&fromaddr,
         sizeof(fromaddr),
         AF_INET))->h_name,
       from->sin_port,msg);

  }


}

int main()
{

//server_socket_tcp_ip4();
server_socket_udp_ip4();

return 0;

} 


