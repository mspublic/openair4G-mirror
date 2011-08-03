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
/*____________________________OPT/opt.h___________________________
Authors: Ben Romdhanne Bilel, Navid NIKAIEN
Company: EURECOM
Emails:
*This file include all defined structures & function headers of this module
This header file must be included */ 
/**
 * Include bloc
 * */
 
#ifndef sys_include
#define sys_include
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#endif
#ifndef project_include
#define project_include
#include "packet-mac-lte.h"
#include "UTIL/LOG/log_if.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/impl_defs_lte.h"
#endif

/**
 * function def
*/
void trace_pdu(int type,unsigned char *pdu_buffer, int pdu_length, int ue_id,int rnti,int subframe);
void dispatcher(int type,unsigned char *pdu_buffer,int pdu_length, int ue_id,int rnti,int subframe);
void SendFrame (guint8 radioType, guint8 direction, guint8 rntiType,
	   guint16 rnti, guint16 ueid, guint16 subframeNumber,
	   guint8 isPredefinedData, guint8 retx, guint8 crcStatus);
void _Send_Ra_Mac_Pdu( int PDU_type ,guint8 Extension, guint8 TypeRaPid,
  guint8 RaPid, guint16 TA, guint8 Hopping_flag, guint16 fsrba, 
  guint8 tmcs, guint8 tcsp, guint8 ul_delay, guint8 cqi_request,
 guint8 crnti_temporary, guint8 radioType, guint8 direction, 
  guint8 rntiType, guint16 rnti, guint16 ueid, guint16 subframeNumber,
  guint8 isPredefinedData, guint8 retx, guint8 crcStatus);
  static void g_pdu_construct(
/*PDU info*/  guint8 Ext,guint8 T, guint8 RaPid,/*RAPID header*/
   guint16 TA, guint8 hopping_flag, guint16 fsrba, 
   guint8 tmcs, guint8 tcsp, guint8 ul_delay, guint8 cqi_request,
   guint16 crnti_temporary);
//void _probe_mac_pdu( LTE_DL_FRAME_PARMS *frame_parms, DCI_ALLOC_t *dci,int TA,int Mod_id,u8 lcid);
void test_ra_header(unsigned char *MAC_PDU,u8 length, int ue_id,int rnti,int subframe);
int pdu_format_convert(int type);
int send_dl_mac_pdu( unsigned char *pdu_buffer, int pdu_length, int ue_id,int rnti,int subframe);
int send_ul_mac_pdu( unsigned char *pdu_buffer, int pdu_length, int ue_id,int rnti,int subframe);
static void g_pdu_construct(
/*PDU info*/  guint8 Ext,guint8 T, guint8 RaPid,/*RAPID header*/
   guint16 TA, guint8 hopping_flag, guint16 fsrba, 
   guint8 tmcs, guint8 tcsp, guint8 ul_delay, guint8 cqi_request,
   guint16 crnti_temporary);
   double *timing_analyzer(int index, int direction );
   void Init_OPT(int trace_mode, char *path,char* ip, int port);
