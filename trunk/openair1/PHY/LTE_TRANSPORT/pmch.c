#include "PHY/defs.h"
#include "PHY/extern.h"

// Mask for identifying subframe for MBMS 
#define MBSFN_TDD_SF3 0x80// for TDD
#define MBSFN_TDD_SF4 0x40
#define MBSFN_TDD_SF7 0x20
#define MBSFN_TDD_SF8 0x10
#define MBSFN_TDD_SF9 0x08

#include "PHY/defs.h"

#define MBSFN_FDD_SF1 0x80// for FDD
#define MBSFN_FDD_SF2 0x40
#define MBSFN_FDD_SF3 0x20
#define MBSFN_FDD_SF6 0x10
#define MBSFN_FDD_SF7 0x08
#define MBSFN_FDD_SF8 0x04

int is_pmch_subframe(uint32_t frame, int subframe, LTE_DL_FRAME_PARMS *frame_parms) {

  uint32_t period;

  if (frame_parms->num_MBSFN_config > 0) {  // we have at least one MBSFN configuration

    period = 1<<frame_parms->MBSFN_config[0].radioframeAllocationPeriod;
    if ((frame % period) == frame_parms->MBSFN_config[0].radioframeAllocationOffset) {
      if (frame_parms->MBSFN_config[0].fourFrames_flag == 0) {
	if (frame_parms->frame_type == FDD) {
	  switch (subframe) {
	    
	  case 1:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_FDD_SF1) > 0)
	      return(1);
	    break;
	  case 2:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_FDD_SF2) > 0)
	      return(1);
	    break;
	  case 3:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_FDD_SF3) > 0)
	      return(1);
	    break;
	  case 6:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_FDD_SF6) > 0)
	      return(1);
	    break;
	  case 7:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_FDD_SF7) > 0)
	      return(1);
	    break;
	  case 8:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_FDD_SF8) > 0)
	      return(1);
	    break;
	  }
	}
	else  {
	  switch (subframe) {
	  case 3:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_TDD_SF3) > 0)
	      return(1);
	    break;
	  case 4:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_TDD_SF4) > 0)
	      return(1);
	    break;
	  case 7:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_TDD_SF7) > 0)
	      return(1);
	    break;
	  case 8:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_TDD_SF8) > 0)
	      return(1);
	    break;
	  case 9:
	    if ((frame_parms->MBSFN_config[0].mbsfn_SubframeConfig & MBSFN_TDD_SF9) > 0)
	      return(1);
	    break;
	  }
	}

      }
      else {  // handle 4 frames case

      }
    } 
  }
  return(0);
} 

void fill_eNB_dlsch_MCH(PHY_VARS_eNB *phy_vars_eNB,int mcs) {

  LTE_eNB_DLSCH_t *dlsch = phy_vars_eNB->dlsch_eNB_MCH;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_eNB->lte_frame_parms;

  dlsch->harq_processes[0]->mcs = mcs;
  dlsch->harq_processes[0]->Ndi = 1;
  dlsch->harq_processes[0]->Nl  = 1;
  dlsch->harq_processes[0]->TBS = TBStable[get_I_TBS(dlsch->harq_processes[0]->mcs)][frame_parms->N_RB_DL-1];
  dlsch->current_harq_pid = 0;
  dlsch->nb_rb = frame_parms->N_RB_DL;

  switch(frame_parms->N_RB_DL) {
  case 6:
    dlsch->rb_alloc[0] = 0x3f;
    break;
  case 25:
    dlsch->rb_alloc[0] = 0x1ffffff;
    break;
  case 50:
    dlsch->rb_alloc[0] = 0xffffffff;
    dlsch->rb_alloc[1] = 0x3ffff;
    break;
  case 100:
    dlsch->rb_alloc[0] = 0xffffffff;
    dlsch->rb_alloc[1] = 0xffffffff;
    dlsch->rb_alloc[2] = 0xffffffff;
    dlsch->rb_alloc[3] = 0xf;
    break;
  }
}

void fill_UE_dlsch_MCH(PHY_VARS_UE *phy_vars_ue,int mcs) {

  LTE_UE_DLSCH_t *dlsch = phy_vars_ue->dlsch_ue_MCH;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;

  dlsch->harq_processes[0]->mcs = mcs;
  dlsch->harq_processes[0]->Ndi = 1;
  dlsch->harq_processes[0]->Nl  = 1;
  dlsch->harq_processes[0]->TBS = TBStable[get_I_TBS(dlsch->harq_processes[0]->mcs)][frame_parms->N_RB_DL-1];
  dlsch->current_harq_pid = 0;
  dlsch->nb_rb = frame_parms->N_RB_DL;
  
  switch(frame_parms->N_RB_DL) {
  case 6:
    dlsch->rb_alloc[0] = 0x3f;
    break;
  case 25:
    dlsch->rb_alloc[0] = 0x1ffffff;
    break;
  case 50:
    dlsch->rb_alloc[0] = 0xffffffff;
    dlsch->rb_alloc[1] = 0x3ffff;
    break;
  case 100:
    dlsch->rb_alloc[0] = 0xffffffff;
    dlsch->rb_alloc[1] = 0xffffffff;
    dlsch->rb_alloc[2] = 0xffffffff;
    dlsch->rb_alloc[3] = 0xf;
    break;
  }
}

void generate_mch(PHY_VARS_eNB *phy_vars_eNB,int subframe,uint8_t *a) {


 

  generate_mbsfn_pilot(phy_vars_eNB,
		       phy_vars_eNB->lte_eNB_common_vars.txdataF[0],
		       AMP,
		       subframe);


  if (dlsch_encoding(a,
		     &phy_vars_eNB->lte_frame_parms,
		     1,
		     phy_vars_eNB->dlsch_eNB_MCH,
		     phy_vars_eNB->frame,
		     subframe,
		     &phy_vars_eNB->dlsch_rate_matching_stats,
		     &phy_vars_eNB->dlsch_turbo_encoding_stats,
		     &phy_vars_eNB->dlsch_interleaving_stats
		     )<0)
    exit(-1);

  dlsch_modulation(phy_vars_eNB->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   subframe,
		   &phy_vars_eNB->lte_frame_parms,
		   1,
		   phy_vars_eNB->dlsch_eNB_MCH);
}
