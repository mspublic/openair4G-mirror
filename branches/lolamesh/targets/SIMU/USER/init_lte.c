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
  memset(PHY_vars_eNB,0,sizeof(PHY_VARS_eNB));
  PHY_vars_eNB->Mod_id=eNB_id;
  PHY_vars_eNB->cooperation_flag=cooperation_flag;
  memcpy(&(PHY_vars_eNB->lte_frame_parms), frame_parms, sizeof(LTE_DL_FRAME_PARMS));
  PHY_vars_eNB->lte_frame_parms.Nid_cell = ((Nid_cell/3)*3)+((eNB_id+Nid_cell)%3);
  PHY_vars_eNB->lte_frame_parms.nushift = PHY_vars_eNB->lte_frame_parms.Nid_cell%6;
  phy_init_lte_eNB(PHY_vars_eNB,0,cooperation_flag,abstraction_flag);

  printf("init eNB: Nid_cell %d\n", frame_parms->Nid_cell);
  printf ("init eNB: frame_type %d,tdd_config %d\n", frame_parms->frame_type,frame_parms->tdd_config);

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    for (j=0;j<2;j++) {
      PHY_vars_eNB->dlsch_eNB[i][j] = new_eNB_dlsch(1,8,abstraction_flag);
      if (!PHY_vars_eNB->dlsch_eNB[i][j]) {
	LOG_E(PHY,"Can't get eNB dlsch structures\n");
	exit(-1);
      }
      else {
	LOG_D(PHY,"dlsch_eNB[%d][%d] => %p\n",i,j,PHY_vars_eNB->dlsch_eNB[i][j]);
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
    PHY_vars_eNB->transmission_mode[i] = transmission_mode;
    
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
			 u8 nb_connected_eNB,
			 u8 abstraction_flag,
			 u8 transmission_mode) {

  int j;
  int eNB_id =0;
  int vlink_id;
  PHY_VARS_UE* PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  LOG_D(PHY,"[UE %d] PHY_vars_UE %p\n", UE_id,PHY_vars_UE);
  memset(PHY_vars_UE,0,sizeof(PHY_VARS_UE));
  LTE_DL_FRAME_PARMS **lte_frame_parms = PHY_vars_UE->lte_frame_parms;
  PHY_vars_UE->Mod_id=UE_id; 
  PHY_vars_UE->n_connected_eNB = nb_connected_eNB;    
  

  for(eNB_id=0;eNB_id<NB_eNB_INST;eNB_id++){ 
    lte_frame_parms[eNB_id] = (LTE_DL_FRAME_PARMS*)malloc16(sizeof(LTE_DL_FRAME_PARMS));
    //memcpy(&lte_frame_parms[eNB_id],&frame_parms,sizeof(LTE_DL_FRAME_PARMS));
    lte_frame_parms[eNB_id]->frame_type        = frame_parms->frame_type;
    lte_frame_parms[eNB_id]->tdd_config         = frame_parms->tdd_config;
    lte_frame_parms[eNB_id]->tdd_config_S       = frame_parms->tdd_config_S;
    lte_frame_parms[eNB_id]->N_RB_DL            = frame_parms->N_RB_DL;
    lte_frame_parms[eNB_id]->N_RB_UL            = lte_frame_parms[eNB_id]->N_RB_DL;
    lte_frame_parms[eNB_id]->phich_config_common.phich_resource = oneSixth;
    lte_frame_parms[eNB_id]->phich_config_common.phich_duration = normal;
    lte_frame_parms[eNB_id]->Ncp                = frame_parms->Ncp;
    lte_frame_parms[eNB_id]->Nid_cell           = 0;
    lte_frame_parms[eNB_id]->nushift            = 0;
    lte_frame_parms[eNB_id]->nb_antennas_tx     = frame_parms->nb_antennas_tx;
    lte_frame_parms[eNB_id]->nb_antennas_rx     = 1;
    lte_frame_parms[eNB_id]->nb_antennas_tx_eNB = frame_parms->nb_antennas_tx_eNB;
    lte_frame_parms[eNB_id]->mode1_flag         = frame_parms->mode1_flag;
    lte_frame_parms[eNB_id]->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift = 0;//n_DMRS1 set to 0
    LOG_D(PHY,"initial_sync enb %d lte_frame_parms %p %d %d %d \n",
	  eNB_id, lte_frame_parms[eNB_id], lte_frame_parms[eNB_id]->N_RB_DL ,lte_frame_parms[eNB_id]->tdd_config, frame_parms->nb_antennas_tx);
      
    init_frame_parms(lte_frame_parms[eNB_id],1);
    phy_init_top(lte_frame_parms[eNB_id]);
    phy_init_lte_top(lte_frame_parms[eNB_id]);

  }

  PHY_vars_UE->lte_ue_buffer_vars = (LTE_UE_BUFFER*)malloc16(sizeof(LTE_UE_BUFFER));
  phy_init_lte_ue_buffer(PHY_vars_UE, abstraction_flag);
  for(eNB_id = 0; eNB_id < nb_connected_eNB; eNB_id++) {
    PHY_vars_UE->lte_ue_common_vars[eNB_id] =(LTE_UE_COMMON*)malloc16(sizeof(LTE_UE_COMMON));
    LOG_D(PHY,"initial_sync enb %d ue common vars %p\n",eNB_id, PHY_vars_UE->lte_ue_common_vars[eNB_id]);
    phy_init_lte_ue_common(PHY_vars_UE, abstraction_flag, eNB_id); 
  }

  phy_init_lte_ue(PHY_vars_UE,abstraction_flag, nb_connected_eNB); 
  
  for (eNB_id = 0; eNB_id < nb_connected_eNB; eNB_id++) {
    for (j=0; j<2; j++) {
      PHY_vars_UE->dlsch_ue[eNB_id][j] = new_ue_dlsch(1,8,abstraction_flag);
      if (!PHY_vars_UE->dlsch_ue[eNB_id][j]) {
	LOG_E(PHY,"Can't get ue dlsch structures\n");
	exit(-1);
      }
      else
	LOG_D(PHY,"dlsch_ue[%d][%d] => %p\n",UE_id,eNB_id,PHY_vars_UE->dlsch_ue[eNB_id][j]);//navid
    }

    PHY_vars_UE->ulsch_ue[eNB_id]  = new_ue_ulsch(8,abstraction_flag);
    if (!PHY_vars_UE->ulsch_ue[eNB_id]) {
      LOG_E(PHY,"Can't get ue ulsch structures\n");
      exit(-1);
    }
    
    PHY_vars_UE->dlsch_ue_SI[eNB_id]  = new_ue_dlsch(1,1,abstraction_flag);
    PHY_vars_UE->dlsch_ue_ra[eNB_id]  = new_ue_dlsch(1,1,abstraction_flag);
        
    PHY_vars_UE->transmission_mode[eNB_id] = transmission_mode;
    LOG_I(OCM," navid tm is %d for enb %d \n", PHY_vars_UE->transmission_mode[eNB_id] , eNB_id);
  }

  for(vlink_id = 0; vlink_id < MAX_VLINK_PER_MR; vlink_id++) {
    for(j = 0; j < 2; j++) {
      PHY_vars_UE->dlsch_ue_co[vlink_id][j] = new_ue_dlsch(1, 8, abstraction_flag);
      if(!PHY_vars_UE->dlsch_ue_co[vlink_id][j]) {
	LOG_E(PHY, "Can't get ue dlsch_co structure\n");
	exit(-1);
      }
      else
	LOG_D(PHY, "dlsch_ue_co[%d][%d] => %p\n", UE_id, vlink_id, PHY_vars_UE->dlsch_ue[vlink_id][j]);
    }

    PHY_vars_UE->ulsch_ue_co[vlink_id] = new_ue_ulsch(8, abstraction_flag);
    if(!PHY_vars_UE->ulsch_ue_co[vlink_id]) {
      LOG_E(PHY, "Can't get ue ulsch_co structure\n");
      exit(-1);
    }
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
	u8 nb_connected_eNB, 
	u8 cooperation_flag,
	u8 transmission_mode,
	u8 abstraction_flag){



  u8 eNB_id,UE_id;

  mac_xface = malloc(sizeof(MAC_xface));

  LOG_I(PHY,"init lte parms: Nid_cell %d, Frame type %d, N_RB_DL %d\n",Nid_cell,frame_type,N_RB_DL);

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

  init_frame_parms(*frame_parms,1);

  (*frame_parms)->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift = 0;//n_DMRS1 set to 0
  (*frame_parms)->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled = 1;
  (*frame_parms)->pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled = 0;
  (*frame_parms)->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH = 0;
  init_ul_hopping(*frame_parms);

  phy_init_top(*frame_parms);

  phy_init_lte_top(*frame_parms);

  PHY_vars_eNB_g = malloc(NB_eNB_INST*sizeof(PHY_VARS_eNB*));
  for (eNB_id=0; eNB_id<NB_eNB_INST;eNB_id++){ 
    PHY_vars_eNB_g[eNB_id] = init_lte_eNB(*frame_parms,eNB_id,Nid_cell,cooperation_flag,transmission_mode,abstraction_flag);
  }

  PHY_vars_UE_g = malloc(NB_UE_INST*sizeof(PHY_VARS_UE*));
  for (UE_id=0; UE_id<NB_UE_INST;UE_id++){  // begin init_lte_UE for all UEs
    PHY_vars_UE_g[UE_id] = init_lte_UE(*frame_parms, UE_id, nb_connected_eNB, abstraction_flag,transmission_mode);
    /* LOG_D(PHY,"PHY_vars_UE_g %p for UE id %d  tm is %d \n",  
	  PHY_vars_UE_g[UE_id] , UE_id, PHY_vars_UE_g[UE_id]->transmission_mode[0]);   
    */
  }

}
