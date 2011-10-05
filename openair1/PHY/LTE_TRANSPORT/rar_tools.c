#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/extern.h"
#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#endif

extern unsigned int  localRIV2alloc_LUT25[512];
extern unsigned int  distRIV2alloc_LUT25[512];
extern unsigned short RIV2nb_rb_LUT25[512];
extern unsigned short RIV2first_rb_LUT25[512];
extern unsigned short RIV_max;

//#define DEBUG_RAR

#ifdef OPENAIR2
int generate_eNB_ulsch_params_from_rar(unsigned char *rar_pdu,
				       unsigned char subframe,
				       LTE_eNB_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms) {

  

  //  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)rar_pdu;
  RAR_PDU *rar = (RAR_PDU *)(rar_pdu+1);
  u8 harq_pid = get_RRCConnReq_harq_pid(frame_parms,subframe);
  
  msg("[PHY][eNB]generate_eNB_ulsch_params_from_rar: subframe %d (harq_pid %d)\n",subframe,harq_pid);
  
  ulsch->harq_processes[harq_pid]->TPC                = rar->TPC;

  if (rar->rb_alloc>RIV_max) {
    msg("dci_tools.c: ERROR: rb_alloc > RIV_max\n");
    return(-1);
  }

  ulsch->harq_processes[harq_pid]->first_rb           = RIV2first_rb_LUT25[rar->rb_alloc];
  ulsch->harq_processes[harq_pid]->nb_rb              = RIV2nb_rb_LUT25[rar->rb_alloc];
  ulsch->harq_processes[harq_pid]->Ndi                = 1;
  ulsch->Or2                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
  ulsch->Or1                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
  ulsch->O_RI                                  = 1;
  ulsch->O_ACK                                 = 2;
  ulsch->beta_offset_cqi_times8                = 18;
  ulsch->beta_offset_ri_times8                 = 10;
  ulsch->beta_offset_harqack_times8            = 16;

  
  ulsch->Nsymb_pusch                           = 12-(frame_parms->Ncp<<1);
  
  if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
    ulsch->harq_processes[harq_pid]->status = ACTIVE;
    ulsch->harq_processes[harq_pid]->rvidx = 0;
    ulsch->harq_processes[harq_pid]->mcs         = rar->mcs;
    ulsch->harq_processes[harq_pid]->TBS         = dlsch_tbs25[ulsch->harq_processes[harq_pid]->mcs][ulsch->harq_processes[harq_pid]->nb_rb-1];
    ulsch->harq_processes[harq_pid]->Msc_initial   = 12*ulsch->harq_processes[harq_pid]->nb_rb;
    ulsch->harq_processes[harq_pid]->Nsymb_initial = 9;
    ulsch->harq_processes[harq_pid]->round = 0;
  }
  else {
    ulsch->harq_processes[harq_pid]->rvidx = 0;
    ulsch->harq_processes[harq_pid]->round++;
  }
#ifdef DEBUG_RAR
  msg("ulsch ra (eNB): harq_pid %d\n",harq_pid);
  msg("ulsch ra (eNB): NBRB     %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
  msg("ulsch ra (eNB): rballoc  %x\n",ulsch->harq_processes[harq_pid]->first_rb);
  msg("ulsch ra (eNB): harq_pid %d\n",0);
  msg("ulsch ra (eNB): Ndi      %d\n",ulsch->harq_processes[harq_pid]->Ndi);  
  msg("ulsch ra (eNB): TBS      %d\n",ulsch->harq_processes[harq_pid]->TBS);
  msg("ulsch ra (eNB): mcs      %d\n",ulsch->harq_processes[harq_pid]->mcs);
  msg("ulsch ra (eNB): Or1      %d\n",ulsch->Or1);
#endif
  return(0);
}

int generate_ue_ulsch_params_from_rar(unsigned char *rar_pdu,
				      unsigned char subframe,
				      LTE_UE_ULSCH_t *ulsch,
				      PHY_MEASUREMENTS *meas,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      unsigned char eNB_id,
				      int current_dlsch_cqi) {
  
  //  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)rar_pdu;
  RAR_PDU *rar = (RAR_PDU *)(rar_pdu+1);
  u8 harq_pid = subframe2harq_pid(frame_parms,subframe);

#ifdef DEBUG_RAR
  msg("rar_tools.c: Filling ue ulsch params -> ulsch %p : subframe %d\n",ulsch,subframe);
#endif



    ulsch->harq_processes[harq_pid]->TPC                                   = rar->TPC;

    if (rar->rb_alloc>RIV_max) {
      msg("rar_tools.c: ERROR: rb_alloc > RIV_max\n");
      return(-1);
    }

    ulsch->harq_processes[harq_pid]->first_rb                              = RIV2first_rb_LUT25[rar->rb_alloc];
    ulsch->harq_processes[harq_pid]->nb_rb                                 = RIV2nb_rb_LUT25[rar->rb_alloc];
    if (ulsch->harq_processes[harq_pid]->nb_rb ==0)
      return(-1);

    ulsch->power_offset = ue_power_offsets[ulsch->harq_processes[harq_pid]->nb_rb];

    if (ulsch->harq_processes[harq_pid]->nb_rb > 3) {
      msg("rar_tools.c: unlikely rb count for RAR grant : nb_rb > 3\n");
      return(-1);
    }

    ulsch->harq_processes[harq_pid]->Ndi                                   = 1;
    if (ulsch->harq_processes[harq_pid]->Ndi == 1)
      ulsch->harq_processes[harq_pid]->status = ACTIVE;

    ulsch->O_RI                                  = 1;
    if (meas->rank[eNB_id] == 1) {
      ulsch->O                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
      ulsch->o_RI[0]                             = 1;
    }
    else {
      ulsch->O                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
      ulsch->o_RI[0]                             = 0;
    }


    fill_CQI(ulsch->o,ulsch->uci_format,meas,eNB_id);
    if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10)) 
      print_CQI(ulsch->o,ulsch->uci_format,eNB_id);

    ulsch->O_ACK                                  = 2;

    ulsch->beta_offset_cqi_times8                  = 18;
    ulsch->beta_offset_ri_times8                   = 10;
    ulsch->beta_offset_harqack_times8              = 16;
    
    ulsch->Nsymb_pusch                             = 12-(frame_parms->Ncp<<1);
    if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
      ulsch->harq_processes[harq_pid]->status = ACTIVE;
      ulsch->harq_processes[harq_pid]->rvidx = 0;
      ulsch->harq_processes[harq_pid]->mcs         = rar->mcs;
      ulsch->harq_processes[harq_pid]->TBS         = dlsch_tbs25[ulsch->harq_processes[harq_pid]->mcs][ulsch->harq_processes[harq_pid]->nb_rb-1];
      ulsch->harq_processes[harq_pid]->Msc_initial   = 12*ulsch->harq_processes[harq_pid]->nb_rb;
      ulsch->harq_processes[harq_pid]->Nsymb_initial = 9;
      ulsch->harq_processes[harq_pid]->round = 0;
    }
    else {
      ulsch->harq_processes[harq_pid]->rvidx = 0;
      ulsch->harq_processes[harq_pid]->round++;
    }
#ifdef DEBUG_RAR
    debug_msg("ulsch (ue,ra): NBRB     %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
    debug_msg("ulsch (ue,ra): first_rb %x\n",ulsch->harq_processes[harq_pid]->first_rb);
    debug_msg("ulsch (ue,ra): nb_rb    %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
    debug_msg("ulsch (ue,ra): Ndi      %d\n",ulsch->harq_processes[harq_pid]->Ndi);  
    debug_msg("ulsch (ue,ra): TBS      %d\n",ulsch->harq_processes[harq_pid]->TBS);
    debug_msg("ulsch (ue,ra): mcs      %d\n",ulsch->harq_processes[harq_pid]->mcs);
#endif
    return(0);
}
#endif

