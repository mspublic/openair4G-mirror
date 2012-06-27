/* mac_lte_logger.c
 *
 * Example code for sending MAC LTE frames over UDP
 * Written by Martin Mathieson, with input from Kiran Kumar
 * This header file may also be distributed under
 * the terms of the BSD Licence as follows:
 * 
 * Copyright (C) 2009 Martin Mathieson. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
 */

/*******************************************************************************

  Eurecom OpenAirInterface 2
  Copyright(c) 1999 - 2012 Eurecom

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

/*! \file probe.c
* \brief 
* \author navid nikaein
* \date 2010-2012
* \version 1.0 
* \company Eurecom
* \email: navid.nikaein@eurecom.fr
*/
/** @defgroup _oai System definitions
There is different modules:
- OAI Address
- OAI Components
- \ref _frame   

numbering:
-# OAI Address
-# OAI Components
-# \ref _frame  

The following diagram is based on graphviz (http://www.graphviz.org/), you need to install the package to view the diagram.  
 * 
 * \dot
 * digraph group_frame  {
 *     node [shape=rect, fontname=Helvetica, fontsize=8,style=filled,fillcolor=lightgrey];
 *     a [ label = " Trace_pdu"];  
 *     b [ label = " dispatcher"]; 
 *     c [ label = " send_ul_mac_pdu"]; 
 *	   D [ label = " send_Dl_mac_pdu"];
 * 	   E [ label = " SendFrame"];
 *     F[ label = " _Send_Ra_Mac_Pdu"];
 *		a->b;
 *		b->c;
 *		b->d;
 *	label="Architecture"
 *		
 * }
 * \enddot 
\section _doxy Doxygen Help
You can use the provided Doxyfile as the configuration file or alternatively run "doxygen -g Doxyfile" to generat the file. 
You need at least to set the some variables in the Doxyfile including "PROJECT_NAME","PROJECT_NUMBER","INPUT","IMAGE_PATH".    
Doxygen help and commands can be found at http://www.stack.nl/~dimitri/doxygen/commands.html#cmdprotocol

\section _arch Architecture

You need to set the IMAGE_PATH in your Doxyfile

\image html arch.png "Architecture"
\image latex arch.eps "Architecture" 

\subsection _mac MAC
thisis the mac
\subsection _rlc RLC
this is the rlc
\subsection _impl Implementation
what about the implementation 


*@{*/


#include "opt.h"

typedef unsigned char  guint8;
typedef unsigned short guint16;
typedef unsigned int   guint32;
typedef unsigned char gboolean;
typedef time_t nstime_t;

#include "packet-mac-lte.h"
#include "mac_pcap.h"

//static unsigned char g_PDUBuffer[1600];
//static unsigned int g_PDUOffset;
static unsigned char g_frameBuffer[1600];
//static unsigned char g_fileBuffer[1600];
static unsigned int g_frameOffset;

static char in_ip[40];
static char in_path[100];
static char in_port[40];
FILE *file_fd;
typedef enum trace_mode{
  opt_wireshark,
  opt_pcap,
  opt_tshark,
  opt_none
}trace_mode;
static trace_mode in_trace;
static unsigned int   subframesSinceCaptureStart;
double  timing_perf[250];
clock_t timing_in[250];
clock_t timing_out[250];
int test=0;
int init_value=0;

static int g_sockfd;/* UDP socket used for sending frames */
static struct sockaddr_in g_serv_addr;

static void SendFrame(guint8 radioType, guint8 direction, guint8 rntiType,
               guint16 rnti, guint16 ueid, guint16 sysframeNumber,
               guint8 isPredefinedData, guint8 retx, guint8 crcStatus, 
	       guint8 oob_event, guint8 oob_event_value,
	       char * pdu_buffer, unsigned int pdu_buffer_size);

static int MAC_LTE_PCAP_WritePDU(MAC_Context_Info_t *context,
			  const unsigned char *PDU, unsigned int length);



//struct mac_lte_info * mac_info; 

// #define WIRESHARK_DEV  
/* if you want to define this, then you need to 
 * 1. checkout the wireshark dev at : svn co http://anonsvn.wireshark.org/wireshark/trunk wireshark
 * 2. copy the local packet-mac-lte.h and packet-mac-lte.c into epan/dissectors/
 * 3. install it, read INSTALL
 * 4. run the wireshark and capture packets from lo interface, and filter out icmp packet (!icmp)
 * 5. run ./oasim -a -P -n 30 | grep OPT 
 */
/* Add framing header to MAC PDU and send. */
static void SendFrame(guint8 radioType, guint8 direction, guint8 rntiType,
               guint16 rnti, guint16 ueid, guint16 sysframeNumber,
               guint8 isPredefinedData, guint8 retx, guint8 crcStatus, 
	       guint8 oob_event, guint8 oob_event_value,
	       char * pdu_buffer, unsigned int pdu_buffer_size) {
    ssize_t bytesSent;
    g_frameOffset = 0;
    unsigned short tmp16;
    
    /********************************************************************/
    /* Fixed start to each frame (allowing heuristic dissector to work) */
    /* Not NULL terminated */
    memset(g_frameBuffer+g_frameOffset, 0, sizeof(mac_lte_info)+pdu_buffer_size + 8);

    memcpy(g_frameBuffer+g_frameOffset, MAC_LTE_START_STRING,
           strlen(MAC_LTE_START_STRING));
    g_frameOffset += strlen(MAC_LTE_START_STRING);

    /******************************************************************************/
    /* Now write out fixed fields (the mandatory elements of struct mac_lte_info) */
    g_frameBuffer[g_frameOffset++] = radioType;
    g_frameBuffer[g_frameOffset++] = direction;
    g_frameBuffer[g_frameOffset++] = rntiType;

    /*************************************/
    /* Now optional fields               */

    /* RNTI */
    g_frameBuffer[g_frameOffset++] = MAC_LTE_RNTI_TAG;
    tmp16 = htons(rnti);
    memcpy(g_frameBuffer+g_frameOffset, &tmp16, 2);
    g_frameOffset += 2;

    /* UEId */
    g_frameBuffer[g_frameOffset++] = MAC_LTE_UEID_TAG;
    tmp16 = htons(ueid);
    memcpy(g_frameBuffer+g_frameOffset, &tmp16, 2);
    g_frameOffset += 2;

    /* Subframe number */
    g_frameBuffer[g_frameOffset++] = MAC_LTE_SUBFRAME_TAG;
    tmp16 = htons(sysframeNumber); // frame counter : this will give an expert info as wireshark expects SF and not F
    memcpy(g_frameBuffer+g_frameOffset, &tmp16, 2);

#ifdef WIRESHARK_DEV  
  g_frameOffset += 2;
    tmp16 = htons(0); // subframe
    memcpy(g_frameBuffer+g_frameOffset, &tmp16, 2);
    g_frameOffset += 2;
 
#endif
    /***********************************************************/
    /* For these optional fields, no need to encode if value is default */
    if (!isPredefinedData) {
      g_frameBuffer[g_frameOffset++] = MAC_LTE_PREDEFINED_DATA_TAG;
      g_frameBuffer[g_frameOffset++] = isPredefinedData;
    }
    if (retx != 0) {
        g_frameBuffer[g_frameOffset++] = MAC_LTE_RETX_TAG;
        g_frameBuffer[g_frameOffset++] = retx;
    }

    g_frameBuffer[g_frameOffset++] = MAC_LTE_CRC_STATUS_TAG;
    g_frameBuffer[g_frameOffset++] = crcStatus;

#ifdef WIRESHARK_DEV  
     /* Relating to out-of-band events */
    /* N.B. dissector will only look to these fields if length is 0... */
    if (pdu_buffer_size==0) {
      switch (oob_event){
      case ltemac_send_preamble :
	LOG_D(OPT,"oob event %d %d\n",ltemac_send_preamble );
	//g_frameBuffer[g_frameOffset++]=0;
	//g_frameBuffer[g_frameOffset++]=0;
	//g_frameBuffer[g_frameOffset++]=0;
	g_frameBuffer[g_frameOffset++] = MAC_LTE_OOB_EVENT_TAG;
	g_frameBuffer[g_frameOffset++]=ltemac_send_preamble;
	g_frameBuffer[g_frameOffset++]=rnti; // is the preamble 
	g_frameBuffer[g_frameOffset++]=oob_event_value;
		break;
      case ltemac_send_sr:
	g_frameBuffer[g_frameOffset++]=ltemac_send_sr;
	g_frameOffset+=2;
	g_frameBuffer[g_frameOffset++]=oob_event_value;
	break;
      case ltemac_sr_failure:
      default:
	LOG_D(OPT,"not implemeneted yet\n");
	break;
      }
    } 
#endif

    g_frameBuffer[g_frameOffset++] = MAC_LTE_PAYLOAD_TAG;
    /***************************************/
    /* Now write the MAC PDU               */
    


    /* Append actual PDU  */
    //memcpy(g_frameBuffer+g_frameOffset, g_PDUBuffer, g_PDUOffset);
    //g_frameOffset += g_PDUOffset;
    if (pdu_buffer != NULL) {
      memcpy(g_frameBuffer+g_frameOffset, pdu_buffer, pdu_buffer_size);
      g_frameOffset += pdu_buffer_size;
    }

    /* Send out the data over the UDP socket */
    bytesSent = sendto(g_sockfd, g_frameBuffer, g_frameOffset, 0,
                      (const struct sockaddr*)&g_serv_addr, sizeof(g_serv_addr));
    if (bytesSent != g_frameOffset) {
        fprintf(stderr, "sendto() failed - expected %d bytes, got %d (errno=%d)\n",
                g_frameOffset, bytesSent, errno);
        exit(1);
    }
}

/* Write an individual PDU (PCAP packet header + mac-context + mac-pdu) */
static int MAC_LTE_PCAP_WritePDU(MAC_Context_Info_t *context,
                        const unsigned char *PDU, unsigned int length) {

    pcaprec_hdr_t packet_header;
    char context_header[256];
    int offset = 0;
    unsigned short tmp16;

    /*****************************************************************/
    /* Context information (same as written by UDP heuristic clients */
    context_header[offset++] = context->radioType;
    context_header[offset++] = context->direction;
    context_header[offset++] = context->rntiType;

    /* RNTI */
    context_header[offset++] = MAC_LTE_RNTI_TAG;
    tmp16 = htons(context->rnti);
    memcpy(context_header+offset, &tmp16, 2);
    offset += 2;

    /* UEId */
    context_header[offset++] = MAC_LTE_UEID_TAG;
    tmp16 = htons(context->ueid);
    memcpy(context_header+offset, &tmp16, 2);
    offset += 2;

    /* Subframe number */
    context_header[offset++] = MAC_LTE_SUBFRAME_TAG;
    tmp16 = htons(context->subFrameNumber);
    memcpy(context_header+offset, &tmp16, 2);
    offset += 2;

    /* CRC Status */
    context_header[offset++] = MAC_LTE_CRC_STATUS_TAG;
    context_header[offset++] = context->crcStatusOK;

    /* Data tag immediately preceding PDU */
    context_header[offset++] = MAC_LTE_PAYLOAD_TAG;


    /****************************************************************/
    /* PCAP Header                                                  */
    /* TODO: Timestamp might want to be relative to a more sensible
       base time... */
    packet_header.ts_sec = context->subframesSinceCaptureStart / 1000;
    packet_header.ts_usec = (context->subframesSinceCaptureStart % 1000) * 1000;
    packet_header.incl_len = offset + length;
    packet_header.orig_len = offset + length;

    /***************************************************************/
    /* Now write everything to the file                            */
    fwrite(&packet_header, sizeof(pcaprec_hdr_t), 1, file_fd);
    fwrite(context_header, 1, offset, file_fd);
    fwrite(PDU, 1, length, file_fd);

    return 1;
}

/* Remote serveraddress (where Wireshark is running) */
void trace_pdu(int direction, char *pdu_buffer, unsigned int pdu_buffer_size, int ueid,int rntiType, int rnti, int sysframe, int oob_event, int oob_event_value){
  
  MAC_Context_Info_t  pdu_context;
  
  switch (in_trace) {
  case opt_wireshark :
    if (g_sockfd == NULL)
      return;
    SendFrame(TDD_RADIO, 
	      (direction == DIRECTION_DOWNLINK) ? DIRECTION_DOWNLINK : DIRECTION_UPLINK, 
	      rntiType, rnti, ueid, sysframe,
	      1, 0, 1,  //guint8 isPredefinedData, guint8 retx, guint8 crcStatus
	      oob_event,oob_event_value,
	      pdu_buffer, pdu_buffer_size); 
  case opt_pcap:
    if (file_fd == NULL)
      return;
    pdu_context.radioType = TDD_RADIO;
    pdu_context.direction = (direction == DIRECTION_DOWNLINK) ? DIRECTION_DOWNLINK : DIRECTION_UPLINK;
    pdu_context.rntiType = rntiType;
    pdu_context.rnti = rnti;
    pdu_context.ueid = ueid;
    pdu_context.isRetx = 0;
    pdu_context.crcStatusOK =1;
    pdu_context.sysFrameNumber = sysframe;
    pdu_context.subFrameNumber = 0;
    pdu_context. subframesSinceCaptureStart= subframesSinceCaptureStart++;
    MAC_LTE_PCAP_WritePDU( &pdu_context, pdu_buffer, pdu_buffer_size);
    break;
  case opt_tshark:
  default:
    break;
  }
}
/*---------------------------------------------------*/
int init_opt(int trace_mode, char *path,char* ip, char* port){
  
  struct hostent *hp;
  subframesSinceCaptureStart=0;
  
  if (ip != NULL)
    strcpy(&in_path[0], path);
  else
    strcpy(&in_path[0], "oai_opt.pcap");
  if (path != NULL)
      strcpy(&in_ip[0], ip);
  else
    strcpy(&in_ip[0], "127.0.0.1");
  if (port != NULL )
    strcpy(&in_port[0], port);
  else
     strcpy(&in_port[0], "1234");
  
   // trace_mode
  switch (trace_mode){
  case opt_wireshark:
    in_trace = opt_wireshark;
    /* Create local socket             */
    g_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sockfd == -1) {
      fprintf(stderr, "Error trying to create socket (errno=%d)\n", errno);
      LOG_E(OPT,"CREATING SOCKET FAILED\n");
      return (-1);
    }
    /* Get remote IP address from the function argument */
    g_serv_addr.sin_family = AF_INET;
    hp = gethostbyname(in_ip);
    if (hp == (struct hostent *)0) {
      fprintf(stderr, "Unknown host %s (h_errno=%d)\n",in_ip, h_errno);
      LOG_E(OPT,"UNKNOWN SOCKET : SOCKET ERROR\n");
      return (-1);
    }
    memcpy((void*)&g_serv_addr.sin_addr, (void*)hp->h_addr, hp->h_length);
    /* Get remote port number from the function argument */
    g_serv_addr.sin_port = htons(in_port);
    break;
  case opt_pcap:
    in_trace = opt_pcap;
    file_fd = fopen(in_path, "w");
    if (file_fd == NULL) {
        printf("Failed to open file \"%s\" for writing\n", in_path);
        return (-1);
    }
    /* Write the file header */
    fwrite(&file_header, sizeof(pcap_hdr_t), 1, file_fd);
    break;
  case opt_tshark:
  default: 
    in_trace = opt_none;
    break;
  
  }
  LOG_D(OPT,"mode %s init ip %s port %s path %s\n", 
	(in_trace ==opt_wireshark)? "wireshark" : "PCAP", in_ip, in_port, in_path )
     
  //  mac_info = (mac_info*)malloc16(sizeof(mac_lte_info));
  // memset(mac_info, 0, sizeof(mac_lte_info)+pdu_buffer_size + 8);
  return (1);
}
void terminate_opt(void) {

 /* Close local socket */
  //  free(mac_info);
  switch (in_trace){
  case opt_wireshark:
    close(g_sockfd);
  case opt_pcap:
    close (file_fd);
    break;
  default:
    break;
  }
}
/*
double *timing_analyzer(int index, int direction ){
//
int i;
if (direction==0)// in
{
	timing_in[index]=clock();
	//if(timing_out[index+100]>timing_in[index+100]);
	//timing_perf[index+100] +=(double)((double)(timing_out[index+100]-timing_in[index+100])/(double)CLOCKS_PER_SEC);
}
else
{   
	timing_out[index]=clock();
	if(index==5)timing_perf[index]=0;
	timing_perf[index] +=(double)((double)(timing_out[index]-timing_in[index])/(((double)CLOCKS_PER_SEC)/1000000));
	
	//LOG_I(OPT,"timing_analyser index %d =%f\n",index,timing_perf[index]);
	init_value++;
	if(init_value==500)
	{
		for(i=0;i<6;i++)
		{
			LOG_I(OPT,"timing_analyser index %d =%f\n",i,timing_perf[i]);
		}
		init_value=0;
	}
	
	return(&timing_perf[0]);

}

}

*/
