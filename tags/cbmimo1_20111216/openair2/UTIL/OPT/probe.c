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
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file probe.c
* \brief explain how this block is organized, and how it works 
* \author Ben Romdhanne Bilel
* \date 2010-2011
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

#include "defs.h"
/**
 * Global Variable Bloc
 * */
static unsigned char g_PDUBuffer[1600];
static unsigned int g_PDUOffset;
static unsigned char g_frameBuffer[1600];
static unsigned char g_fileBuffer[1600];
static unsigned int g_frameOffset;
double  timing_perf[250];
clock_t timing_in[250];
clock_t timing_out[250];
int test=0;
int init_value=0;
static int g_sockfd;/* UDP socket used for sending frames */
static struct sockaddr_in g_serv_addr;
/* Remote serveraddress (where Wireshark is running) */
void trace_pdu(int type,unsigned char *pdu_buffer, int pdu_length, int ue_id,int rnti,int subframe)
{	int i,Type_value;
	double *localanalyzer;
	localanalyzer= timing_analyzer(1,0);	
	g_PDUOffset = 0;
	/*
	 * Copy the PDu buffer into local buffer
	 * 
	 * */
	LOG_I(OPT,"Trace_PDU in\n\r");
	for(i=0;i<pdu_length;i++)
	{
	LOG_I(OPT,"VAR i=%d,val=%x\n",i,(unsigned char)pdu_buffer[g_PDUOffset]);
	g_PDUBuffer[g_PDUOffset++]=(unsigned char)pdu_buffer[i];
	}
	Type_value=pdu_format_convert(type);
	dispatcher( type, pdu_buffer,  pdu_length, ue_id, rnti,subframe);
	localanalyzer=timing_analyzer(1,1);	
}
 /******************************************************************/
 
 /*
  * */
 
void dispatcher(int type,unsigned char *pdu_buffer, int pdu_length,  int ue_id, int rnti,int subframe)
{

if(output_type==socket_output)
 {
  //* in this case we fwd the pdu to the correct destination function of pdu_type
  switch(type)
  	{
  	case 1:		//default case of mac 
  	break;
  	case 2:		//MAC_RA_PDU
  	break;		
  	case 3:		//MAC_UL_PDU
  	send_ul_mac_pdu( pdu_buffer,  pdu_length, ue_id ,rnti,subframe);
  	break;
  	case 4:		//MAC_DL_PDU
  	send_dl_mac_pdu( pdu_buffer,  pdu_length, ue_id ,rnti,subframe);
  	break;
  	default:
  	break; 
  	}
 }
 else
 {
   //* in this case we dump into file 
 }
}
/******************************************************************/
void SendFrame (guint8 radioType, guint8 direction, guint8 rntiType,
	   guint16 rnti, guint16 ueid, guint16 subframeNumber,
	   guint8 isPredefinedData, guint8 retx, guint8 crcStatus)
	   {
	   	
  ssize_t bytesSent;
  g_frameOffset = 0;
  unsigned short tmp16;

    /********************************************************************/
  /* Fixed start to each frame (allowing heuristic dissector to work) */
  /* Not NULL terminated */
  memcpy (g_frameBuffer + g_frameOffset, MAC_LTE_START_STRING,
	  strlen (MAC_LTE_START_STRING));
  g_frameOffset += strlen (MAC_LTE_START_STRING);

    /******************************************************************************/
  /* Now write out fixed fields (the mandatory elements of struct mac_lte_info) */
  g_frameBuffer[g_frameOffset++] = radioType;
  g_frameBuffer[g_frameOffset++] = direction;
  g_frameBuffer[g_frameOffset++] = rntiType;

    /*************************************/
  /* Now optional fields               */

  /* RNTI */
  g_frameBuffer[g_frameOffset++] = MAC_LTE_RNTI_TAG;
  tmp16 = htons (rnti);
  memcpy (g_frameBuffer + g_frameOffset, &tmp16, 2);
  g_frameOffset += 2;

  /* UEId */
 g_frameBuffer[g_frameOffset++] = MAC_LTE_UEID_TAG;
  tmp16 = htons (ueid);
  memcpy (g_frameBuffer + g_frameOffset, &tmp16, 2);
  g_frameOffset += 2;

  /* Subframe number */
 g_frameBuffer[g_frameOffset++] = MAC_LTE_SUBFRAME_TAG;
  tmp16 = htons (ueid);
  memcpy (g_frameBuffer + g_frameOffset, &tmp16, 2);
  g_frameOffset += 2;

  g_frameBuffer[g_frameOffset++] = MAC_LTE_CRC_STATUS_TAG;
  g_frameBuffer[g_frameOffset++] = crcStatus;


    /***********************************************************/
  /* For these optional fields, no need to encode if value is default */
  if (!isPredefinedData)
    {
      g_frameBuffer[g_frameOffset++] = MAC_LTE_PREDFINED_DATA_TAG;
      g_frameBuffer[g_frameOffset++] = isPredefinedData;
    }

  if (retx != 0)
    {
      g_frameBuffer[g_frameOffset++] = MAC_LTE_RETX_TAG;
      g_frameBuffer[g_frameOffset++] = retx;
    }
  /* Now write the MAC PDU               */
  g_frameBuffer[g_frameOffset++] = MAC_LTE_PAYLOAD_TAG;

  /* Append actual PDU  */
  memcpy (g_frameBuffer + g_frameOffset, g_PDUBuffer, g_PDUOffset);
  g_frameOffset += g_PDUOffset;
/*  write data in a file*/
//fp=fopen("filetest.dump","r+b");
//LOG_I(MAC_RA, "bilel file log: %d  %d, %s\n",sizeof(unsigned char),g_frameOffset,g_frameBuffer);

//fread(g_fileBuffer,sizeof(unsigned char),file_index,fp);
//file_index +=g_frameOffset;
//fwrite(g_frameBuffer,sizeof(unsigned char),g_frameOffset,fp);
//fclose(fp);

  /* Send out the data over the UDP socket */
  bytesSent = sendto (g_sockfd, g_frameBuffer, g_frameOffset, 0,
		      (const struct sockaddr *) &g_serv_addr,
		      sizeof (g_serv_addr));
  if (bytesSent != g_frameOffset)
    {
      fprintf (stderr,
	       "sendto() failed - expected %d bytes, got %d (errno=%d)\n",
	       g_frameOffset, bytesSent, errno);
      exit (1);
    }
	   
	   }
	   /**********************************************************/
void _Send_Ra_Mac_Pdu( int PDU_type ,guint8 Extension, guint8 TypeRaPid,
  guint8 RaPid, guint16 TA, guint8 Hopping_flag, guint16 fsrba, 
  guint8 tmcs, guint8 tcsp, guint8 ul_delay, guint8 cqi_request,
 guint8 crnti_temporary, guint8 radioType, guint8 direction, 
  guint8 rntiType, guint16 rnti, guint16 ueid, guint16 subframeNumber,
  guint8 isPredefinedData, guint8 retx, guint8 crcStatus)
  
  {
  LOG_I(OPT,"call here for Send_Ra_Mac_Pdu\n");
 
  struct hostent *hp;
    /***********************************/
  /* Create local socket             */
  g_sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (g_sockfd == -1)
    {
      fprintf (stderr, "Error trying to create socket (errno=%d)\n", errno);
     return (1);
    }
    /***************************************************/
  /* Get remote IP address from 1st command-line arg */
  g_serv_addr.sin_family = AF_INET;
  hp = gethostbyname (ip);
  if (hp == (struct hostent *) 0)
    {
      fprintf (stderr, "Unknown host %s (h_errno=%d)\n", ip, h_errno);
      exit (1);
    }
  memcpy ((void *) &g_serv_addr.sin_addr, (void *) hp->h_addr, hp->h_length);

    /****************************************************/
  /* Get remote port number from 2nd command-line arg */
  g_serv_addr.sin_port = htons (port);
  g_pdu_construct(Extension,TypeRaPid,RaPid,TA,Hopping_flag,fsrba,tmcs,tcsp,ul_delay,cqi_request,crnti_temporary);
  SendFrame (TDD_RADIO, DIRECTION_DOWNLINK, WS_RA_RNTI, 
  		rnti /* RNTI */ ,
	     ueid /* UEId */ ,
	    subframeNumber /* Subframe number */ ,
	     isPredefinedData /* isPredefined */ ,
	     retx /* retx */ ,
	     1 /* CRCStatus (i.e. OK) */ );
	     
close (g_sockfd);

  return EXIT_SUCCESS;
  }
  void _probe_mac_pdu( LTE_DL_FRAME_PARMS *frame_parms, DCI_ALLOC_t *dci,int TA,int Mod_id,u8 lcid)
{}
void test_ra_header(unsigned char *MAC_PDU,u8 length, int ue_id,int rnti,int subframe)
{}
/*---------------------------------------------------*/
void Init_OPT(int trace_mode, char *in_path,char* in_ip, int in_port)//, int frame_num)
{
// trace_mode
mode=trace_mode;
ip=in_ip;
port=in_port;
path=in_path;
int i;
int pid;
char *dumpfile= "outfile.dump";
char *argument ="-w";
char *end= NULL;
char *app="tcpdump";
if(trace_mode > 1)// for trace_mode 2& upper we creat a new thread where we ran tcpdump using specified filter
		{

	if ((pid = fork()) == -1)
	        perror("fork error");
	     else if (pid == 0) {
		execlp(app,"tcpdump",argument,dumpfile,end);
	 	printf("Return not expected. Must be an execlp error.n");
	     }



		}
for(i=0;i<250;i++)
{
	timing_perf[i]=0;
}
}
void Set_OPT(int trace_mode, char *in_path,char* in_ip, int in_port)
{
mode=trace_mode;
ip=in_ip;
port=in_port;
path=in_path;
}
int Get_OPT(int trace_mode, char *in_path,char* in_ip, int in_port)
{
//todo
return(trace_mode);
}
/*---------------------------------------------------*/
void Terminate_OPT()
{
/*****************************************************/
}
int pdu_format_convert(int type)
{
	int Type_value=0;
	if(type==1) Type_value=1;
	else
	{
		if(type==1) Type_value=2;
		else
		{
			if(type==1) Type_value=3;
			else
			{
				if(type==1) Type_value=4;
				else Type_value=5;
			}
		}
	}
	return(Type_value);
}
/************************************************************/
 int send_dl_mac_pdu( unsigned char *pdu_buffer, int pdu_length, int ue_id,int rnti,int subframe)
{
	LOG_I(OPT,"call here for Send_Ra_Mac_Pdu\n");
 
  struct hostent *hp;
    /***********************************/
  /* Create local socket             */
  g_sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (g_sockfd == -1)
    {
      fprintf (stderr, "Error trying to create socket (errno=%d)\n", errno);
     return (1);
    }
    /***************************************************/
  /* Get remote IP address from 1st command-line arg */
  g_serv_addr.sin_family = AF_INET;
  hp = gethostbyname (ip);
  if (hp == (struct hostent *) 0)
    {
      fprintf (stderr, "Unknown host %s (h_errno=%d)\n", ip, h_errno);
      exit (1);
    }
  memcpy ((void *) &g_serv_addr.sin_addr, (void *) hp->h_addr, hp->h_length);

    /****************************************************/
  /* Get remote port number from 2nd command-line arg */
  g_serv_addr.sin_port = htons (port);

    /****************************************************/
  /* Encode and send some frames                       */
/*function of PDU type */
	  SendFrame (TDD_RADIO, DIRECTION_DOWNLINK, WS_C_RNTI, 
  		rnti /* RNTI */ ,
	     ue_id /* UEId */ ,
	    subframe /* Subframe number */ ,
	     0 /* isPredefined */ ,
	     0 /* retx */ ,
	     1 /* CRCStatus (i.e. OK) */ );
  /* Close local socket */
  close (g_sockfd);

  return EXIT_SUCCESS;

}
int send_ul_mac_pdu( unsigned char *pdu_buffer, int pdu_length, int ue_id,int rnti,int subframe)
{
	LOG_I(OPT,"call here for Send_Ra_Mac_Pdu\n");
 
  struct hostent *hp;
    /***********************************/
  /* Create local socket             */
  g_sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (g_sockfd == -1)
    {
      fprintf (stderr, "Error trying to create socket (errno=%d)\n", errno);
     return (1);
    }
    /***************************************************/
  /* Get remote IP address from 1st command-line arg */
  g_serv_addr.sin_family = AF_INET;
  hp = gethostbyname (ip);
  if (hp == (struct hostent *) 0)
    {
      fprintf (stderr, "Unknown host %s (h_errno=%d)\n", ip, h_errno);
      exit (1);
    }
  memcpy ((void *) &g_serv_addr.sin_addr, (void *) hp->h_addr, hp->h_length);

    /****************************************************/
  /* Get remote port number from 2nd command-line arg */
  g_serv_addr.sin_port = htons (port);

    /****************************************************/
  /* Encode and send some frames                       */
/*function of PDU type */
	  SendFrame (TDD_RADIO, DIRECTION_UPLINK, WS_C_RNTI, 
  		rnti /* RNTI */ ,
	     ue_id /* UEId */ ,
	    subframe /* Subframe number */ ,
	     0 /* isPredefined */ ,
	     0 /* retx */ ,
	     1 /* CRCStatus (i.e. OK) */ );
  /* Close local socket */
  close (g_sockfd);

  return EXIT_SUCCESS;

}
/* Write an RAR PDU */
static void g_pdu_construct(
/*PDU info*/  guint8 Ext,guint8 T, guint8 RaPid,/*RAPID header*/
   guint16 TA, guint8 hopping_flag, guint16 fsrba, 
   guint8 tmcs, guint8 tcsp, guint8 ul_delay, guint8 cqi_request,
   guint16 crnti_temporary)
{
	int i;
unsigned char buffer[7];
g_PDUOffset = 0;
/**
 * RAPID header
 * */
 //LOG_I(MAC_RA,"Ext=%u\n",Ext);
buffer[0]=Ext<<7;
//LOG_I(MAC_RA,"buff0=%u\n",buffer[0]);

buffer[1]=(T<<6);buffer[1]=buffer[1] & 0x40;
buffer[2]=RaPid & 0x3F;
buffer[0] =buffer[2] | buffer[1] | buffer[0];
//LOG_I(MAC_RA,"buff0=%u\n",buffer[0]);
/**
 * TA first bit is reserved */
 //LOG_I(MAC_RA,"TA=%u\n",TA);
 buffer[1]=(char)((TA>>4))& 0x7F;
 buffer[2]=(char)(TA<<4);
//LOG_I(MAC_RA,"buff1=%u  buff2=%u\n",buffer[1],buffer[2]);
 /**
  * UL grant cnostruction*/
buffer[2]=buffer[2] |(hopping_flag<<3)|((char)(fsrba>>7));//for fsrba bit from 9to11 will be 0-2
 //LOG_I(MAC_RA," fsrba =%u\n",fsrba);
buffer[3]=(char)(fsrba<<1)|(tmcs>>3);
 //LOG_I(MAC_RA," buff3 =%u\n",buffer[3]);
buffer[4]=((tmcs<<5)& 0xE0)|((tcsp<<2)& 0x1C)|((ul_delay<<1) & 0x02)|((cqi_request) & 0x01);
/**
 * Temporary C-RNTI
 * */
 buffer[5]=(char)(crnti_temporary>>8);
 buffer[6]=(char)(crnti_temporary);
 for ( i=0;i<7;i++)
 g_PDUBuffer[g_PDUOffset++]=buffer[i];
}


double *timing_analyzer(int index, int direction )
{
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
#ifdef TEST_OCG 
int main(int argc, char *argv[]) {
	//this 
	
return 0;
}
#endif

