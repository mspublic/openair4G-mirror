#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#ifdef DEBUG_UCI_TOOLS
#include "PHY/vars.h"
#endif
//#define DEBUG_UCI

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

void extract_CQI(void *o,unsigned char *o_RI,UCI_format fmt,LTE_eNB_UE_stats *stats) {

  unsigned char rank;

  rank = o_RI[0];
  //printf("extract_CQI: rank = %d\n",rank);

  switch (fmt) {

  case wideband_cqi: //and subband pmi
    if (rank == 0) {
      stats->DL_cqi[0]     = ((wideband_cqi_rank1_2A_5MHz *)o)->cqi1;
      if (stats->DL_cqi[0] > 28)
	stats->DL_cqi[0] = 28;
      stats->DL_pmi_single = ((wideband_cqi_rank1_2A_5MHz *)o)->pmi;      
    }
    else {
      stats->DL_cqi[0]     = ((wideband_cqi_rank2_2A_5MHz *)o)->cqi1;
      if (stats->DL_cqi[0] > 28)
	stats->DL_cqi[0] = 28;      
      stats->DL_cqi[1]     = ((wideband_cqi_rank2_2A_5MHz *)o)->cqi2;
      if (stats->DL_cqi[1] > 28)
	stats->DL_cqi[1] = 28;      
      stats->DL_pmi_dual   = ((wideband_cqi_rank2_2A_5MHz *)o)->pmi;      
    }
    break;
  case hlc_cqi:
    if (rank == 0) {
      stats->DL_cqi[0]     = ((HLC_subband_cqi_rank1_2A_5MHz *)o)->cqi1;
      if (stats->DL_cqi[0] > 28)
	stats->DL_cqi[0] = 28;      
      stats->DL_diffcqi[0] = ((HLC_subband_cqi_rank1_2A_5MHz *)o)->diffcqi1;      
      stats->DL_pmi_single = ((HLC_subband_cqi_rank1_2A_5MHz *)o)->pmi;      
    }
    else {
      stats->DL_cqi[0]     = ((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi1;
      if (stats->DL_cqi[0] > 28)
	stats->DL_cqi[0] = 28;      
      stats->DL_cqi[1]     = ((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi2;
      if (stats->DL_cqi[1] > 28)
	stats->DL_cqi[1] = 28;      
      stats->DL_diffcqi[0] = ((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi1;      
      stats->DL_diffcqi[1] = ((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi2;      
      stats->DL_pmi_dual   = ((HLC_subband_cqi_rank2_2A_5MHz *)o)->pmi;      
    }
    break;
  default:
    break;
  }

}
									     

void print_CQI(void *o,unsigned char *o_RI,UCI_format fmt,unsigned char eNB_id) {


  unsigned char rank;

  rank = o_RI[0];

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
    break;
  case ue_selected:
#ifdef DEBUG_UCI
    msg("dci_tools.c: print_CQI ue_selected CQI not supported yet!!!\n");
#endif //DEBUG_UCI
    break;
  }
}

