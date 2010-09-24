#include "LAYER2/MAC/defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"

extern unsigned int  localRIV2alloc_LUT25[512];
extern unsigned int  distRIV2alloc_LUT25[512];
extern unsigned short RIV2nb_rb_LUT25[512];
extern unsigned short RIV2first_rb_LUT25[512];
extern unsigned short RIV_max;

//#define DEBUG_RAR

int generate_eNb_ulsch_params_from_rar(unsigned char *rar_pdu,
				       unsigned char subframe,
				       LTE_eNb_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms) {

  

  //  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)rar_pdu;
  RAR_PDU *rar = (RAR_PDU *)(rar_pdu+1);
  
  //  printf("generate_eNb_ulsch_params_from_rar: subframe %d\n",subframe);
  
  ulsch->harq_processes[0]->TPC                = rar->TPC;

  if (rar->rb_alloc>RIV_max) {
    msg("dci_tools.c: ERROR: rb_alloc > RIV_max\n");
    return(-1);
  }

  ulsch->harq_processes[0]->first_rb           = RIV2first_rb_LUT25[rar->rb_alloc];
  ulsch->harq_processes[0]->nb_rb              = RIV2nb_rb_LUT25[rar->rb_alloc];
  ulsch->harq_processes[0]->Ndi                = 1;
  ulsch->Or2                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
  ulsch->Or1                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
  ulsch->O_RI                                  = 1;
  ulsch->O_ACK                                 = 2;
  ulsch->beta_offset_cqi_times8                = 18;
  ulsch->beta_offset_ri_times8                 = 10;
  ulsch->beta_offset_harqack_times8            = 16;

  
  ulsch->Nsymb_pusch                           = 9;
  
  if (ulsch->harq_processes[0]->Ndi == 1) {
    ulsch->harq_processes[0]->status = ACTIVE;
    ulsch->harq_processes[0]->rvidx = 0;
    ulsch->harq_processes[0]->mcs         = rar->mcs;
    ulsch->harq_processes[0]->TBS         = dlsch_tbs25[ulsch->harq_processes[0]->mcs][ulsch->harq_processes[0]->nb_rb-1];
    ulsch->harq_processes[0]->Msc_initial   = 12*ulsch->harq_processes[0]->nb_rb;
    ulsch->harq_processes[0]->Nsymb_initial = 9;
    ulsch->harq_processes[0]->round = 0;
  }
  else {
    ulsch->harq_processes[0]->rvidx = 0;
    ulsch->harq_processes[0]->round++;
  }
#ifdef DEBUG_RAR
  msg("ulsch ra (eNb): NBRB     %d\n",ulsch->harq_processes[0]->nb_rb);
  msg("ulsch ra (eNb): rballoc  %x\n",ulsch->harq_processes[0]->first_rb);
  msg("ulsch ra (eNb): harq_pid %d\n",0);
  msg("ulsch ra (eNb): Ndi      %d\n",ulsch->harq_processes[0]->Ndi);  
  msg("ulsch ra (eNb): TBS      %d\n",ulsch->harq_processes[0]->TBS);
  msg("ulsch ra (eNb): mcs      %d\n",ulsch->harq_processes[0]->mcs);
  msg("ulsch ra (eNb): Or1      %d\n",ulsch->Or1);
#endif
  return(0);
}

int generate_ue_ulsch_params_from_rar(unsigned char *rar_pdu,
				      unsigned char subframe,
				      LTE_UE_ULSCH_t *ulsch,
				      PHY_MEASUREMENTS *meas,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      unsigned char eNb_id) {
  
  //  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)rar_pdu;
  RAR_PDU *rar = (RAR_PDU *)(rar_pdu+1);

#ifdef DEBUG_RAR
  msg("rar_tools.c: Filling ue ulsch params -> ulsch %p : subframe %d\n",ulsch,subframe);
#endif



    ulsch->harq_processes[0]->TPC                                   = rar->TPC;

    if (rar->rb_alloc>RIV_max) {
      msg("rar_tools.c: ERROR: rb_alloc > RIV_max\n");
      return(-1);
    }

    ulsch->harq_processes[0]->first_rb                              = RIV2first_rb_LUT25[rar->rb_alloc];
    ulsch->harq_processes[0]->nb_rb                                 = RIV2nb_rb_LUT25[rar->rb_alloc];
    if (ulsch->harq_processes[0]->nb_rb ==0)
      return(-1);

    ulsch->power_offset = ue_power_offsets[ulsch->harq_processes[0]->nb_rb];

    if (ulsch->harq_processes[0]->nb_rb > 3) {
      msg("rar_tools.c: unlikely rb count for RAR grant : nb_rb > 3\n");
      return(-1);
    }

    ulsch->harq_processes[0]->Ndi                                   = 1;
    if (ulsch->harq_processes[0]->Ndi == 1)
      ulsch->harq_processes[0]->status = ACTIVE;

    ulsch->O_RI                                  = 1;
    if (meas->rank[eNb_id] == 1) {
      ulsch->O                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
      ulsch->o_RI[0]                             = 1;
    }
    else {
      ulsch->O                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
      ulsch->o_RI[0]                             = 0;
    }


    fill_CQI(ulsch->o,wideband_cqi,meas,eNb_id);
    if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10)) 
      print_CQI(ulsch->o,ulsch->o_RI,wideband_cqi,eNb_id);

    ulsch->O_ACK                                  = 2;

    ulsch->beta_offset_cqi_times8                  = 18;
    ulsch->beta_offset_ri_times8                   = 10;
    ulsch->beta_offset_harqack_times8              = 16;
    
    ulsch->Nsymb_pusch                             = 9;
    if (ulsch->harq_processes[0]->Ndi == 1) {
      ulsch->harq_processes[0]->status = ACTIVE;
      ulsch->harq_processes[0]->rvidx = 0;
      ulsch->harq_processes[0]->mcs         = rar->mcs;
      ulsch->harq_processes[0]->TBS         = dlsch_tbs25[ulsch->harq_processes[0]->mcs][ulsch->harq_processes[0]->nb_rb-1];
      ulsch->harq_processes[0]->Msc_initial   = 12*ulsch->harq_processes[0]->nb_rb;
      ulsch->harq_processes[0]->Nsymb_initial = 9;
      ulsch->harq_processes[0]->round = 0;
    }
    else {
      ulsch->harq_processes[0]->rvidx = 0;
      ulsch->harq_processes[0]->round++;
    }
#ifdef DEBUG_RAR
    debug_msg("ulsch (ue,ra): NBRB     %d\n",ulsch->harq_processes[0]->nb_rb);
    debug_msg("ulsch (ue,ra): first_rb %x\n",ulsch->harq_processes[0]->first_rb);
    debug_msg("ulsch (ue,ra): nb_rb    %d\n",ulsch->harq_processes[0]->nb_rb);
    debug_msg("ulsch (ue,ra): Ndi      %d\n",ulsch->harq_processes[0]->Ndi);  
    debug_msg("ulsch (ue,ra): TBS      %d\n",ulsch->harq_processes[0]->TBS);
    debug_msg("ulsch (ue,ra): mcs      %d\n",ulsch->harq_processes[0]->mcs);
#endif
    return(0);
}


