#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "init_lte.h"

#include "PHY/extern.h"
#include "MAC_INTERFACE/extern.h"

#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log_if.h"
#include "PHY_INTERFACE/extern.h"
#endif



PHY_VARS_eNB* init_lte_eNB(LTE_DL_FRAME_PARMS *frame_parms, 
			   u8 eNB_id,
			   u8 Nid_cell,
			   u8 cooperation_flag,
			   u8 transmission_mode,
			   u8 abstraction_flag) {

  int i,j;
  PHY_VARS_eNB* PHY_vars_eNB = malloc(sizeof(PHY_VARS_eNB));

  PHY_vars_eNB->Mod_id=eNB_id;
  PHY_vars_eNB->cooperation_flag=cooperation_flag;
  memcpy(&(PHY_vars_eNB->lte_frame_parms), frame_parms, sizeof(LTE_DL_FRAME_PARMS));
  PHY_vars_eNB->lte_frame_parms.Nid_cell = ((Nid_cell/3)*3)+((eNB_id+Nid_cell)%3);
  PHY_vars_eNB->lte_frame_parms.nushift = PHY_vars_eNB->lte_frame_parms.Nid_cell%6;
  phy_init_lte_eNB(PHY_vars_eNB,0,cooperation_flag,abstraction_flag);

  printf ("init eNB: Nid_cell %d\n", frame_parms->Nid_cell);
  printf ("init eNB: frame_type %d,tdd_config %d\n", frame_parms->frame_type,frame_parms->tdd_config);

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    for (j=0;j<2;j++) {
      PHY_vars_eNB->dlsch_eNB[i][j] = new_eNB_dlsch(1,8,abstraction_flag);
      if (!PHY_vars_eNB->dlsch_eNB[i][j]) {
	LOG_E(PHY,"Can't get eNB dlsch structures\n");
	exit(-1);
      }
      else {
	LOG_E(PHY,"dlsch_eNB[%d][%d] => %p\n",i,j,PHY_vars_eNB->dlsch_eNB[i][j]);
	PHY_vars_eNB->dlsch_eNB[i][j]->rnti=0;
      }
    }
    PHY_vars_eNB->ulsch_eNB[1+i] = new_eNB_ulsch(8,abstraction_flag);
    if (!PHY_vars_eNB->ulsch_eNB[1+i]) {
      LOG_E(PHY,"Can't get eNB ulsch structures\n");
      exit(-1);
    }
    
    // this is the transmission mode for the signalling channels
    // this will be overwritten with the real transmission mode by the RRC once the UE is connected
    PHY_vars_eNB->transmission_mode[i] = (transmission_mode==1?1:2);
    
  }
  
  // ULSCH for RA
  PHY_vars_eNB->ulsch_eNB[0] = new_eNB_ulsch(8,abstraction_flag);
  if (!PHY_vars_eNB->ulsch_eNB[0]) {
    LOG_E(PHY,"Can't get eNB ulsch structures\n");
    exit(-1);
  }
  
  PHY_vars_eNB->dlsch_eNB_SI  = new_eNB_dlsch(1,1,abstraction_flag);
  printf("eNB %d : SI %p\n",eNB_id,PHY_vars_eNB->dlsch_eNB_SI);
  PHY_vars_eNB->dlsch_eNB_ra  = new_eNB_dlsch(1,1,abstraction_flag);
  printf("eNB %d : RA %p\n",eNB_id,PHY_vars_eNB->dlsch_eNB_ra);
  
  PHY_vars_eNB->rx_total_gain_eNB_dB=150;
  
  for(i=0;i<NUMBER_OF_UE_MAX;i++)
    PHY_vars_eNB->mu_mimo_mode[i].dl_pow_off = 2;
  
  PHY_vars_eNB->check_for_total_transmissions = 0;
  
  PHY_vars_eNB->check_for_MUMIMO_transmissions = 0;
  
  PHY_vars_eNB->FULL_MUMIMO_transmissions = 0;
  
  PHY_vars_eNB->check_for_SUMIMO_transmissions = 0;

  return (PHY_vars_eNB);
}

PHY_VARS_UE* init_lte_UE(LTE_DL_FRAME_PARMS *frame_parms, 
			 u8 UE_id,
			 u8 abstraction_flag,
			 u8 transmission_mode) {

  int i,j;
  PHY_VARS_UE* PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  PHY_vars_UE->Mod_id=UE_id; 
  memcpy(&(PHY_vars_UE->lte_frame_parms), frame_parms, sizeof(LTE_DL_FRAME_PARMS));
  phy_init_lte_ue(PHY_vars_UE,1,abstraction_flag);
  for (i=0;i<NUMBER_OF_CONNECTED_eNB_MAX;i++) {
    for (j=0;j<2;j++) {
      PHY_vars_UE->dlsch_ue[i][j]  = new_ue_dlsch(1,8,abstraction_flag);
      if (!PHY_vars_UE->dlsch_ue[i][j]) {
	LOG_E(PHY,"Can't get ue dlsch structures\n");
	exit(-1);
      }
      else
	LOG_D(PHY,"dlsch_ue[%d][%d] => %p\n",UE_id,i,PHY_vars_UE->dlsch_ue[i][j]);//navid
    }

    
    
    PHY_vars_UE->ulsch_ue[i]  = new_ue_ulsch(8,abstraction_flag);
    if (!PHY_vars_UE->ulsch_ue[i]) {
      LOG_E(PHY,"Can't get ue ulsch structures\n");
      exit(-1);
      }
    
    PHY_vars_UE->dlsch_ue_SI[i]  = new_ue_dlsch(1,1,abstraction_flag);
    PHY_vars_UE->dlsch_ue_ra[i]  = new_ue_dlsch(1,1,abstraction_flag);
        
    PHY_vars_UE->transmission_mode[i] = transmission_mode;
  }
  return (PHY_vars_UE);
}

void init_lte_vars(LTE_DL_FRAME_PARMS **frame_parms,
		   u8 frame_type,
		   u8 tdd_config,
		   u8 tdd_config_S,
		   u8 extended_prefix_flag, 
		   u8 N_RB_DL,
		   u16 Nid_cell,
		   u8 cooperation_flag,u8 transmission_mode,u8 abstraction_flag) {

  u8 eNB_id,UE_id;

  mac_xface = malloc(sizeof(MAC_xface));

  LOG_I(PHY,"init lte parms: Nid_cell %d\n",Nid_cell);

  *frame_parms = malloc(sizeof(LTE_DL_FRAME_PARMS));
  (*frame_parms)->frame_type         = frame_type;
  (*frame_parms)->tdd_config         = tdd_config;
  (*frame_parms)->tdd_config_S       = tdd_config_S;
  (*frame_parms)->N_RB_DL            = N_RB_DL;
  (*frame_parms)->N_RB_UL            = (*frame_parms)->N_RB_DL;
  (*frame_parms)->phich_config_common.phich_resource = oneSixth;
  (*frame_parms)->phich_config_common.phich_duration = normal;
  (*frame_parms)->Ncp                = extended_prefix_flag;
  (*frame_parms)->Nid_cell           = Nid_cell;
  (*frame_parms)->nushift            = (Nid_cell%6);
  (*frame_parms)->nb_antennas_tx     = (transmission_mode == 1) ? 1 : 2;
  (*frame_parms)->nb_antennas_tx_eNB = (transmission_mode == 1) ? 1 : 2;
  (*frame_parms)->nb_antennas_rx     = 1;
  (*frame_parms)->mode1_flag = (transmission_mode == 1) ? 1 : 0;
  (*frame_parms)->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift = 0;//n_DMRS1 set to 0

  init_frame_parms(*frame_parms,1);

  phy_init_top(*frame_parms);

  phy_init_lte_top(*frame_parms);

  PHY_vars_eNB_g = malloc(NB_eNB_INST*sizeof(PHY_VARS_eNB*));
  for (eNB_id=0; eNB_id<NB_eNB_INST;eNB_id++){ 
    PHY_vars_eNB_g[eNB_id] = init_lte_eNB(*frame_parms,eNB_id,Nid_cell,cooperation_flag,transmission_mode,abstraction_flag);
  }



  // init all UE vars
  /*
  for (UE_id=0; UE_id<NB_UE_INST;UE_id++){ 


    memcpy(&(PHY_vars_UE_g[UE_id]->lte_frame_parms), *frame_parms, sizeof(LTE_DL_FRAME_PARMS));
    // Do this until SSS detection is finished
    if (NB_eNB_INST>0) {
      PHY_vars_UE_g[UE_id]->lte_frame_parms.Nid_cell = PHY_vars_eNB_g[UE_id%NB_eNB_INST]->lte_frame_parms.Nid_cell;
      PHY_vars_UE_g[UE_id]->lte_frame_parms.nushift = PHY_vars_eNB_g[UE_id%NB_eNB_INST]->lte_frame_parms.nushift;
    }

    phy_init_lte_ue(PHY_vars_UE_g[UE_id],abstraction_flag);

    for (i=0;i<NUMBER_OF_CONNECTED_eNB_MAX;i++) {
      for (j=0;j<2;j++) {
	PHY_vars_UE_g[UE_id]->dlsch_ue[i][j]  = new_ue_dlsch(1,8,abstraction_flag);
	if (!PHY_vars_UE_g[UE_id]->dlsch_ue[i][j]) {
	  msg("Can't get ue dlsch structures\n");
	  exit(-1);
	}
	else
	  msg("dlsch_ue[%d][%d] => %p\n",UE_id,i,PHY_vars_UE_g[UE_id]->dlsch_ue[i][j]);//navid
      }
      
      
      PHY_vars_UE_g[UE_id]->ulsch_ue[i]  = new_ue_ulsch(8,abstraction_flag);
      if (!PHY_vars_UE_g[UE_id]->ulsch_ue[i]) {
	msg("Can't get ue ulsch structures\n");
	exit(-1);
      }
      
      PHY_vars_UE_g[UE_id]->dlsch_ue_SI[i]  = new_ue_dlsch(1,1,abstraction_flag);
      PHY_vars_UE_g[UE_id]->dlsch_ue_ra[i]  = new_ue_dlsch(1,1,abstraction_flag);

      PHY_vars_UE_g[UE_id]->transmission_mode[i] = transmission_mode;
    }
*/

  PHY_vars_UE_g = malloc(NB_UE_INST*sizeof(PHY_VARS_UE*));
  for (UE_id=0; UE_id<NB_UE_INST;UE_id++){ // begin navid
    PHY_vars_UE_g[UE_id] = init_lte_UE(*frame_parms, UE_id,abstraction_flag,transmission_mode);

  }

}
