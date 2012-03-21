/*******************************************************************************
 
  Eurecom OpenAirInterface 2
  Copyright(c) 1999 - 2010 Eurecom
 
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
  Address      : Eurecom, 2229, route des cr��tes, 06560 Valbonne Sophia Antipolis, France
 
*******************************************************************************/
/*! \file nas_config.c
* \brief Configures the nasmesh interface
* \author Daniel Camara
* \date 2006-2011
* \version 0.1
* \company Eurecom
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>

#include "nas_config.h"
#include "UTIL/LOG/log.h"


//default values according to the examples, 

char *baseNetAddress = "10.0" ;  
char *netMask = "255.255.255.0" ;  
char *broadcastAddr = "10.0.255.255" ;  


void setBaseNetAddress (char* baseAddr){
  strcpy(baseNetAddress,baseAddr);
}

char* getBaseNetAddress (){
  return baseNetAddress;
}

void setNetMask (char* baseAddr){
  strcpy(netMask,baseAddr);
}

char* getNetMask  (){
  return netMask;
}

void setBroadcastAddress (char* baseAddr){
  strcpy(baseAddr, baseAddr);
}

char* getBroadcastAddress (){
  return broadcastAddr;
}

//Add Gateway to the interface
int set_gateway(char *interfaceName, char *gateway)
{
    int sock_fd;
    struct rtentry rt;
    struct sockaddr_in addr;

    if((sock_fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
        perror("socket failed");
        return 1;
    }

    memset (&rt, 0, sizeof (rt));

    addr.sin_family = AF_INET;
    /*set Destination addr*/
    inet_aton("0.0.0.0",&addr.sin_addr);
    memcpy(&rt.rt_dst, &addr, sizeof(struct sockaddr_in));

    /*set gateway addr*/
    inet_aton(gateway,&addr.sin_addr);
    memcpy(&rt.rt_gateway, &addr, sizeof(struct sockaddr_in));

    /*set genmask addr*/
    inet_aton("0.0.0.0",&addr.sin_addr);
    memcpy(&rt.rt_genmask, &addr, sizeof(struct sockaddr_in));

    rt.rt_dev = interfaceName;
    //rt.rt_flags = RTF_UP|RTF_GATEWAY|RTF_DEFAULT;
    rt.rt_flags = RTF_GATEWAY|RTF_DEFAULT;

    if (ioctl(sock_fd, SIOCADDRT, &rt) < 0)
    {
        close(sock_fd);

        if(strstr(strerror(errno),"File exists") == NULL)
        {
	  LOG_E(OIP,"ioctl SIOCADDRT failed : %s\n",strerror(errno));
            return 2;
        }
        else /*if SIOCADDRT error is route exist, retrun success*/
        {
	  LOG_I(OIP,"File Exist ...\n");
	  LOG_I(OIP,"set_gateway OK!\n");
	  return 0;
        }

    }

    close(sock_fd);

    LOG_D(OIP,"Set Gateway OK!\n");
    return 0;
}

// sets a genneric interface parameter
// (SIOCSIFADDR, SIOCSIFNETMASK, SIOCSIFBRDADDR, SIOCSIFFLAGS)
int setInterfaceParameter(char *interfaceName, char *settingAddress, int operation)
{
    int sock_fd;
    struct ifreq ifr;
    struct sockaddr_in addr;

    if((sock_fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)    {
      LOG_E(OIP,"Setting operation %d, for %s, address, %s : socket failed\n",
               operation, interfaceName, settingAddress);
        return 1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName, sizeof(ifr.ifr_name)-1);

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;

    inet_aton(settingAddress,&addr.sin_addr);
    memcpy(&ifr.ifr_ifru.ifru_addr,&addr,sizeof(struct sockaddr_in));

    if(ioctl(sock_fd,operation,&ifr) < 0)    {
        close(sock_fd);
        LOG_E(OIP,"Setting operation %d, for %s, address, %s : ioctl call failed\n",
               operation, interfaceName, settingAddress);
        return 2;
    }

    close(sock_fd);

//    printf("Set OK!\n");
    return 0;
}

// sets a genneric interface parameter
// (SIOCSIFADDR, SIOCSIFNETMASK, SIOCSIFBRDADDR, SIOCSIFFLAGS)
int bringInterfaceUp(char *interfaceName, int up)
{
    int sock_fd;
    struct ifreq ifr;

    if((sock_fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
      LOG_E(OIP,"Bringing interface UP, for %s, failed creating socket\n", interfaceName);
        return 1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName, sizeof(ifr.ifr_name)-1);

    if(up)
    {
        ifr.ifr_flags |= IFF_UP | IFF_NOARP | IFF_MULTICAST;
      	if (ioctl(sock_fd, SIOCSIFFLAGS, (caddr_t)&ifr) == -1)
        {
            close(sock_fd);
            LOG_E(OIP,"Bringing interface UP, for %s, failed UP ioctl\n", interfaceName);
            return 2;
        }
    }
    else
    {
        //        printf("desactivation de %s\n", interfaceName);
        ifr.ifr_flags &= (~IFF_UP);
        if (ioctl(sock_fd, SIOCSIFFLAGS, (caddr_t)&ifr) == -1)
        {
            close(sock_fd);
            LOG_E(OIP,"Bringing interface UP, for %s, failed UP ioctl\n", interfaceName);
            return 2;
        }
    }

 //   printf("UP/DOWN OK!\n");
    return 0;
}
// non blocking full configuration of the interface (address, net mask, and broadcast mask)
int NAS_config(char *interfaceName, char *ipAddress, char *networkMask, char *broadcastAddress)
{
	bringInterfaceUp(interfaceName, 0);
    // sets the machine address
    int returnValue= setInterfaceParameter(interfaceName, ipAddress,SIOCSIFADDR);

    // sets the machine network mask
    if(!returnValue)
        returnValue= setInterfaceParameter(interfaceName, networkMask,SIOCSIFNETMASK);

    // sets the machine broadcast address
    if(!returnValue)
        returnValue= setInterfaceParameter(interfaceName, broadcastAddress,SIOCSIFBRDADDR);

    //	if(!returnValue)
    //	returnValue=set_gateway(interfaceName, broadcastAddress);

	bringInterfaceUp(interfaceName, 1);

    return returnValue;
}

// non blocking full configuration of the interface (address, and the two lest octets of the address)
int nas_config(int interface_id, int thirdOctet, int fourthOctet){
  //char buf[5];
  char ipAddress[20];
  char broadcastAddress[20];
  char interfaceName[8];
  int returnValue;
  sprintf(ipAddress, "10.0.%d.%d", thirdOctet,fourthOctet);

  sprintf(broadcastAddress, "10.0.%d.255", thirdOctet);
   
  sprintf(interfaceName, "oai%d", interface_id);
  
  bringInterfaceUp(interfaceName, 0);
  // sets the machine address
  returnValue= setInterfaceParameter(interfaceName, ipAddress,SIOCSIFADDR);
  
  // sets the machine network mask
  if(!returnValue)
    returnValue= setInterfaceParameter(interfaceName, netMask,SIOCSIFNETMASK);
  
  // sets the machine broadcast address
  if(!returnValue)
    returnValue= setInterfaceParameter(interfaceName, broadcastAddress,SIOCSIFBRDADDR);
  
  bringInterfaceUp(interfaceName, 1);
  
  return returnValue;
  
}

// Blocking full configuration of the interface (address, net mask, and broadcast mask)
int blocking_NAS_config(char *interfaceName, char *ipAddress, char *networkMask, char *broadcastAddress)
{

    char command[200];
    command[0]='\0';

    strcat(command, "ifconfig ");
    strcat(command, interfaceName);
    strcat(command, " ");
    strcat(command, ipAddress);
    strcat(command, " networkMask ");
    strcat(command, networkMask);
    strcat(command, " broadcast ");
    strcat(command, broadcastAddress);

    // ifconfig nasmesh0 10.0.1.1 networkMask 255.255.255.0 broadcast 10.0.1.255
    int i = system (command);

    return i;
}

// program help
void helpOptions(char **argv)
{
    printf("Help for %s\n",  argv[0]);
    printf("  -i <interfaceName>\n");
    printf("  -a <IP address>\n");
    printf("  -n <Net mask>\n");
    printf("  -b <broadcast address>\n");
    printf("  -h Shows this help\n");
    printf("If no option is passed as parameter the default values are: \n");
    printf("    Interface Name: nasmesh0\n");
    printf("    IP Address: 10.0.1.1\n");
    printf("    Net mask: 255.255.255.0\n");
    printf("    Broadcast address: [Beginning of the IP address].255\n");

    exit(1);
}

// creates the broadcast address if it wasn't set before
void createBroadcast(char *broadcastAddress)
{
    int pos=strlen(broadcastAddress)-1;

    while(broadcastAddress[pos]!='.')
        pos--;
    broadcastAddress[++pos]='2';
    broadcastAddress[++pos]='2';
    broadcastAddress[++pos]='5';
    broadcastAddress[++pos]='\0';
}
#ifdef STANDALONE
// main function
//---------------------------------------------------------------------------
int main(int argc,char **argv)
//---------------------------------------------------------------------------
{
    int c;
    char interfaceName[100];
    char ipAddress[100];
    char networkMask[100];
    char broadcastAddress[100];

    strcpy(interfaceName, "oai0");
    strcpy(ipAddress, "10.0.1.1");
    strcpy(networkMask, "255.255.255.0");
    broadcastAddress[0]='\0';

    while ((c = getopt (argc, argv, "i:a:n:b:h")) != -1)
        switch (c)
        {
        case 'h':
            helpOptions(argv);
            break;
        case 'i':
            strcpy(interfaceName,optarg);
            break;
        case 'a':
            strcpy(ipAddress,optarg);
            break;
        case 'n':
            strcpy(networkMask,optarg);
            break;
        case 'b':
            strcpy(broadcastAddress,optarg);
            break;
        case '?':
            if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
            return 1;
        default:
            abort ();
        }

    if(strlen(broadcastAddress)==0)
    {
        strcpy(broadcastAddress,ipAddress);
        createBroadcast(broadcastAddress);
    }
    printf("Command: ifconfig %s %s networkMask %s broadcast %s\n", interfaceName, ipAddress, networkMask, broadcastAddress);
    NAS_config(interfaceName, ipAddress, networkMask, broadcastAddress);
    //test
//     setBaseNetAddress("11.11");
//     nas_config(interfaceName, 33, 44);
    
}

#endif
