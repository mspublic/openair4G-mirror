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

/*! \file PHY/LTE_TRANSPORT/phich.c
* \brief Routines for generation of and computations regarding the uplink control information (UCI) for PUSCH. V8.6 2009-03
* \author R. Knopp, F. Kaltenberger, A. Bhamri
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr, florian.kaltenberger@eurecom.fr, ankit.bhamri@eurecom.fr
* \note
* \warning
*/
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#ifdef DEBUG_UCI_TOOLS
#include "PHY/vars.h"
#endif
#define DEBUG_UCI

unsigned int pmi2hex_2Ar1(unsigned short pmi) {

 return ((pmi&3) + (((pmi>>2)&3)<<4) + (((pmi>>4)&3)<<8) + (((pmi>>6)&3)<<12) + 
          (((pmi>>8)&3)<<16) + (((pmi>>10)&3)<<20) + (((pmi>>12)&3)<<24));
}

unsigned int pmi2hex_2Ar2(unsigned char pmi) {

 return ((pmi&1) + (((pmi>>1)&1)<<4) + (((pmi>>2)&1)<<8) + (((pmi>>3)&3)<<12) + 
          (((pmi>>4)&3)<<16) + (((pmi>>5)&3)<<20) + (((pmi>>6)&3)<<24));
}

unsigned int cqi2hex(unsigned short cqi) {

 return ((cqi&3) + (((cqi>>2)&3)<<4) + (((cqi>>4)&3)<<8) + (((cqi>>6)&3)<<12) + 
          (((cqi>>8)&3)<<16) + (((cqi>>10)&3)<<20) + (((cqi>>12)&3)<<24));
}

//void do_diff_cqi(u8 N_RB_DL,
//		 u8 *DL_subband_cqi,
//		 u8 DL_cqi,
//		 u32 diffcqi1) {
//
//  u8 nb_sb,i,offset;
//
//  // This is table 7.2.1-3 from 36.213 (with k replaced by the number of subbands, nb_sb)
//  switch (N_RB_DL) {
//  case 6:
//    nb_sb=0;
//    break;
//  case 15:
//    nb_sb = 4;
//  case 25:
//    nb_sb = 7;
//    break;
//  case 50:
//    nb_sb = 9;
//    break;
//  case 75:
//    nb_sb = 10;
//    break;
//  case 100:
//    nb_sb = 13;
//    break;
//  default:
//    nb_sb=0;
//    break;
//  }
//
//  memset(DL_subband_cqi,0,13);
//
//  for (i=0;i<nb_sb;i++) {
//    offset = (DL_cqi>>(2*i))&3;
//    if (offset == 3)
//      DL_subband_cqi[i] = DL_cqi - 1;
//    else
//      DL_subband_cqi[i] = DL_cqi + offset;
//  }
//}


void do_diff_cqi(u8 N_RB_DL,
		u8 *DL_subband_cqi,
		 u8 DL_cqi,
		 u32 diffcqi1) {

  u8 nb_sb,i,offset;

  // This is table 7.2.1-3 from 36.213 (with k replaced by the number of subbands, nb_sb)
  switch (N_RB_DL) {
  case 6:
    nb_sb=0;
    break;
  case 15:
    nb_sb = 4;
  case 25:
    nb_sb = 7;
    break;
  case 50:
    nb_sb = 9;
    break;
  case 75:
    nb_sb = 10;
    break;
  case 100:
    nb_sb = 13;
    break;
  default:
    nb_sb=0;
    break;
  }

  memset(DL_subband_cqi,0,13);

  for (i=0;i<nb_sb;i++) {
    offset = (diffcqi1>>(2*i))&3;
    if (offset == 3)
      DL_subband_cqi[i] = DL_cqi - 1;
    else 
      DL_subband_cqi[i] = DL_cqi + offset;
  }
}




void extract_CQI(void *o,UCI_format_t uci_format,LTE_eNB_UE_stats *stats) {

  //unsigned char rank;
  //UCI_format fmt;
  u8 N_RB_DL = 25;


  switch(uci_format){
  case wideband_cqi_rank1_2A:
    stats->DL_cqi[0]     = (((wideband_cqi_rank1_2A_5MHz *)o)->cqi1);
    if (stats->DL_cqi[0] > 15)
      stats->DL_cqi[0] = 15;
    stats->DL_pmi_single = ((wideband_cqi_rank1_2A_5MHz *)o)->pmi;   
    break;
  case wideband_cqi_rank2_2A:
    stats->DL_cqi[0]     = (((wideband_cqi_rank2_2A_5MHz *)o)->cqi1);
    if (stats->DL_cqi[0] > 15)
      stats->DL_cqi[0] = 15;      
    stats->DL_cqi[1]     = (((wideband_cqi_rank2_2A_5MHz *)o)->cqi2);
    if (stats->DL_cqi[1] > 15)
      stats->DL_cqi[1] = 15;      
    stats->DL_pmi_dual   = ((wideband_cqi_rank2_2A_5MHz *)o)->pmi; 
    break;
  case HLC_subband_cqi_nopmi:
    stats->DL_cqi[0]     = (((HLC_subband_cqi_nopmi_5MHz *)o)->cqi1);
    if (stats->DL_cqi[0] > 15)
      stats->DL_cqi[0] = 15;      
    do_diff_cqi(N_RB_DL,stats->DL_subband_cqi[0],stats->DL_cqi[0],((HLC_subband_cqi_nopmi_5MHz *)o)->diffcqi1);
    break;
  case HLC_subband_cqi_rank1_2A:
    stats->DL_cqi[0]     = (((HLC_subband_cqi_rank1_2A_5MHz *)o)->cqi1);
    if (stats->DL_cqi[0] > 15)
      stats->DL_cqi[0] = 15;     
    do_diff_cqi(N_RB_DL,stats->DL_subband_cqi[0],stats->DL_cqi[0],(((HLC_subband_cqi_rank1_2A_5MHz *)o)->diffcqi1));      
    stats->DL_pmi_single = ((HLC_subband_cqi_rank1_2A_5MHz *)o)->pmi;    
    break;
  case HLC_subband_cqi_rank2_2A:
    stats->DL_cqi[0]     = (((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi1);
    if (stats->DL_cqi[0] > 15)
      stats->DL_cqi[0] = 15;      
    stats->DL_cqi[1]     = (((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi2);
    if (stats->DL_cqi[1] > 15)
      stats->DL_cqi[1] = 15;      
    do_diff_cqi(N_RB_DL,stats->DL_subband_cqi[0],stats->DL_cqi[0],(((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi1));      
    do_diff_cqi(N_RB_DL,stats->DL_subband_cqi[1],stats->DL_cqi[1],(((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi2));      
    stats->DL_pmi_dual   = ((HLC_subband_cqi_rank2_2A_5MHz *)o)->pmi; 
    break;
  default:
    break;
  }

  /* 
  switch (tmode) {

  case 1:
  case 2:
  case 3:
  case 5:
  case 6:
  case 7:
  default:
    fmt = hlc_cqi;
    break;
  case 4:
    fmt = wideband_cqi;
    break;
  }

  rank = o_RI[0];
  //printf("extract_CQI: rank = %d\n",rank);

  switch (fmt) {

  case wideband_cqi: //and subband pmi
    if (rank == 0) {
      stats->DL_cqi[0]     = (((wideband_cqi_rank1_2A_5MHz *)o)->cqi1);
      if (stats->DL_cqi[0] > 15)
	stats->DL_cqi[0] = 15;
      stats->DL_pmi_single = ((wideband_cqi_rank1_2A_5MHz *)o)->pmi;      
    }
    else {
      stats->DL_cqi[0]     = (((wideband_cqi_rank2_2A_5MHz *)o)->cqi1);
      if (stats->DL_cqi[0] > 15)
	stats->DL_cqi[0] = 15;      
      stats->DL_cqi[1]     = (((wideband_cqi_rank2_2A_5MHz *)o)->cqi2);
      if (stats->DL_cqi[1] > 15)
	stats->DL_cqi[1] = 15;      
      stats->DL_pmi_dual   = ((wideband_cqi_rank2_2A_5MHz *)o)->pmi;      
    }
    break;
  case hlc_cqi:
    if (tmode > 2) {
      if (rank == 0) {
	stats->DL_cqi[0]     = (((HLC_subband_cqi_rank1_2A_5MHz *)o)->cqi1);
	if (stats->DL_cqi[0] > 15)
	  stats->DL_cqi[0] = 15;     
	do_diff_cqi(N_RB_DL,stats->DL_subband_cqi[0],stats->DL_cqi[0],(((HLC_subband_cqi_rank1_2A_5MHz *)o)->diffcqi1));      
	stats->DL_pmi_single = ((HLC_subband_cqi_rank1_2A_5MHz *)o)->pmi;      
      }
      else {
	stats->DL_cqi[0]     = (((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi1);
	if (stats->DL_cqi[0] > 15)
	  stats->DL_cqi[0] = 15;      
	stats->DL_cqi[1]     = (((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi2);
	if (stats->DL_cqi[1] > 15)
	  stats->DL_cqi[1] = 15;      
	do_diff_cqi(N_RB_DL,stats->DL_subband_cqi[0],stats->DL_cqi[0],(((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi1));      
	do_diff_cqi(N_RB_DL,stats->DL_subband_cqi[1],stats->DL_cqi[1],(((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi2));      
	stats->DL_pmi_dual   = ((HLC_subband_cqi_rank2_2A_5MHz *)o)->pmi;      
      }
    }
    else {
      stats->DL_cqi[0]     = (((HLC_subband_cqi_nopmi_5MHz *)o)->cqi1);
      if (stats->DL_cqi[0] > 15)
	stats->DL_cqi[0] = 15;      

      do_diff_cqi(N_RB_DL,stats->DL_subband_cqi[0],stats->DL_cqi[0],((HLC_subband_cqi_nopmi_5MHz *)o)->diffcqi1);
      
    }
    break;
  default:
    break;
  }
  */

}
									     

void print_CQI(void *o,UCI_format_t uci_format,unsigned char eNB_id) {


  switch(uci_format){
  case wideband_cqi_rank1_2A:
#ifdef DEBUG_UCI
    msg("[PRINT CQI] wideband_cqi rank 1: eNB %d, cqi %d\n",eNB_id,((wideband_cqi_rank1_2A_5MHz *)o)->cqi1);
    msg("[PRINT CQI] wideband_cqi rank 1: eNB %d, pmi (%x) %8x\n",eNB_id,((wideband_cqi_rank1_2A_5MHz *)o)->pmi,pmi2hex_2Ar1(((wideband_cqi_rank1_2A_5MHz *)o)->pmi));
#endif //DEBUG_UCI
    break;
  case wideband_cqi_rank2_2A:
#ifdef DEBUG_UCI
    msg("[PRINT CQI] wideband_cqi rank 2: eNB %d, cqi1 %d\n",eNB_id,((wideband_cqi_rank2_2A_5MHz *)o)->cqi1);
    msg("[PRINT CQI] wideband_cqi rank 2: eNB %d, cqi2 %d\n",eNB_id,((wideband_cqi_rank2_2A_5MHz *)o)->cqi2);
    msg("[PRINT CQI] wideband_cqi rank 2: eNB %d, pmi %8x\n",eNB_id,pmi2hex_2Ar2(((wideband_cqi_rank2_2A_5MHz *)o)->pmi));
#endif //DEBUG_UCI
    break;
  case HLC_subband_cqi_nopmi:
#ifdef DEBUG_UCI
    msg("[PRINT CQI] hlc_cqi (no pmi) : eNB %d, cqi1 %d\n",eNB_id,((HLC_subband_cqi_rank1_2A_5MHz *)o)->cqi1);
    msg("[PRINT CQI] hlc_cqi (no pmi) : eNB %d, diffcqi1 %8x\n",eNB_id,cqi2hex(((HLC_subband_cqi_rank1_2A_5MHz *)o)->diffcqi1));
#endif //DEBUG_UCI
    break;
  case HLC_subband_cqi_rank1_2A:
#ifdef DEBUG_UCI
    msg("[PRINT CQI] hlc_cqi rank 1: eNB %d, cqi1 %d\n",eNB_id,((HLC_subband_cqi_rank1_2A_5MHz *)o)->cqi1);
    msg("[PRINT CQI] hlc_cqi rank 1: eNB %d, diffcqi1 %8x\n",eNB_id,cqi2hex(((HLC_subband_cqi_rank1_2A_5MHz *)o)->diffcqi1));
    msg("[PRINT CQI] hlc_cqi rank 1: eNB %d, pmi %d\n",eNB_id,((HLC_subband_cqi_rank1_2A_5MHz *)o)->pmi);
#endif //DEBUG_UCI
    break;
  case HLC_subband_cqi_rank2_2A:
#ifdef DEBUG_UCI
    msg("[PRINT CQI] hlc_cqi rank 2: eNB %d, cqi1 %d\n",eNB_id,((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi1);
    msg("[PRINT CQI] hlc_cqi rank 2: eNB %d, cqi2 %d\n",eNB_id,((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi2);
    msg("[PRINT CQI] hlc_cqi rank 2: eNB %d, diffcqi1 %8x\n",eNB_id,cqi2hex(((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi1));
    msg("[PRINT CQI] hlc_cqi rank 2: eNB %d, diffcqi2 %8x\n",eNB_id,cqi2hex(((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi2));
    msg("[PRINT CQI] hlc_cqi rank 2: eNB %d, pmi %d\n",eNB_id,((HLC_subband_cqi_rank2_2A_5MHz *)o)->pmi);
#endif //DEBUG_UCI
    break;
  case ue_selected:
#ifdef DEBUG_UCI
    msg("dci_tools.c: print_CQI ue_selected CQI not supported yet!!!\n");
#endif //DEBUG_UCI
    break;
  default:
#ifdef DEBUG_UCI
    msg("dci_tools.c: print_CQI, unsupported CQI mode (%d)!!!\n",uci_format);
#endif //DEBUG_UCI
    break;
  }

  /*
  switch (tmode) {

  case 1:
  case 2:
  case 3:
  case 5:
  case 6:
  case 7:
  default:
    fmt = hlc_cqi;
    break;
  case 4:
    fmt = wideband_cqi;
    break;
  }

  switch (fmt) {

  case wideband_cqi:
    if (rank == 0) {
#ifdef DEBUG_UCI
      msg("[PRINT CQI] wideband_cqi rank 1: eNB %d, cqi %d\n",eNB_id,((wideband_cqi_rank1_2A_5MHz *)o)->cqi1);
      msg("[PRINT CQI] wideband_cqi rank 1: eNB %d, pmi (%x) %8x\n",eNB_id,((wideband_cqi_rank1_2A_5MHz *)o)->pmi,pmi2hex_2Ar1(((wideband_cqi_rank1_2A_5MHz *)o)->pmi));
#endif //DEBUG_UCI
    }
    else { 
#ifdef DEBUG_UCI
      msg("[PRINT CQI] wideband_cqi rank 2: eNB %d, cqi1 %d\n",eNB_id,((wideband_cqi_rank2_2A_5MHz *)o)->cqi1);
      msg("[PRINT CQI] wideband_cqi rank 2: eNB %d, cqi2 %d\n",eNB_id,((wideband_cqi_rank2_2A_5MHz *)o)->cqi2);
      msg("[PRINT CQI] wideband_cqi rank 2: eNB %d, pmi %8x\n",eNB_id,pmi2hex_2Ar2(((wideband_cqi_rank2_2A_5MHz *)o)->pmi));
#endif //DEBUG_UCI
    }
    break;
  case hlc_cqi:
    if (tmode > 2) {
      if (rank == 0) {
#ifdef DEBUG_UCI
	msg("[PRINT CQI] hlc_cqi rank 1: eNB %d, cqi1 %d\n",eNB_id,((HLC_subband_cqi_rank1_2A_5MHz *)o)->cqi1);
	msg("[PRINT CQI] hlc_cqi rank 1: eNB %d, diffcqi1 %8x\n",eNB_id,cqi2hex(((HLC_subband_cqi_rank1_2A_5MHz *)o)->diffcqi1));
	msg("[PRINT CQI] hlc_cqi rank 1: eNB %d, pmi %d\n",eNB_id,((HLC_subband_cqi_rank1_2A_5MHz *)o)->pmi);
#endif //DEBUG_UCI
      }
      else {
#ifdef DEBUG_UCI
	msg("[PRINT CQI] hlc_cqi rank 2: eNB %d, cqi1 %d\n",eNB_id,((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi1);
	msg("[PRINT CQI] hlc_cqi rank 2: eNB %d, cqi2 %d\n",eNB_id,((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi2);
	msg("[PRINT CQI] hlc_cqi rank 2: eNB %d, diffcqi1 %8x\n",eNB_id,cqi2hex(((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi1));
	msg("[PRINT CQI] hlc_cqi rank 2: eNB %d, diffcqi2 %8x\n",eNB_id,cqi2hex(((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi2));
	msg("[PRINT CQI] hlc_cqi rank 2: eNB %d, pmi %d\n",eNB_id,((HLC_subband_cqi_rank2_2A_5MHz *)o)->pmi);
#endif //DEBUG_UCI
      }
    }
    else {
#ifdef DEBUG_UCI
      msg("[PRINT CQI] hlc_cqi (no pmi) : eNB %d, cqi1 %d\n",eNB_id,((HLC_subband_cqi_rank1_2A_5MHz *)o)->cqi1);
      msg("[PRINT CQI] hlc_cqi (no pmi) : eNB %d, diffcqi1 %8x\n",eNB_id,cqi2hex(((HLC_subband_cqi_rank1_2A_5MHz *)o)->diffcqi1));
#endif //DEBUG_UCI
    }
    break;
  case ue_selected:
#ifdef DEBUG_UCI
    msg("dci_tools.c: print_CQI ue_selected CQI not supported yet!!!\n");
#endif //DEBUG_UCI
    break;
  }
  */

}

