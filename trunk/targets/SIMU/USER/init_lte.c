/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
    included in this distribution in the file called "COPYING". If not, 
    see <http://www.gnu.org/licenses/>.

   Contact Information
   OpenAirInterface Admin: openair_admin@eurecom.fr
   OpenAirInterface Tech : openair_tech@eurecom.fr
   OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
   Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/

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
			   uint8_t eNB_id,
			   uint8_t Nid_cell,
			   uint8_t cooperation_flag,
			   uint8_t transmission_mode,
			   uint8_t abstraction_flag) {

  int i,j;
  PHY_VARS_eNB* PHY_vars_eNB = malloc(sizeof(PHY_VARS_eNB));
  memset(PHY_vars_eNB,0,sizeof(PHY_VARS_eNB));
  PHY_vars_eNB->Mod_id=eNB_id;
  PHY_vars_eNB->cooperation_flag=cooperation_flag;
  memcpy(&(PHY_vars_eNB->lte_frame_parms), frame_parms, sizeof(LTE_DL_FRAME_PARMS));
  PHY_vars_eNB->lte_frame_parms.Nid_cell = ((Nid_cell/3)*3)+((eNB_id+Nid_cell)%3);
  PHY_vars_eNB->lte_frame_parms.nushift = PHY_vars_eNB->lte_frame_parms.Nid_cell%6;
  phy_init_lte_eNB(PHY_vars_eNB,0,cooperation_flag,abstraction_flag);

  LOG_I(PHY,"init eNB: Nid_cell %d\n", frame_parms->Nid_cell);
  LOG_I(PHY,"init eNB: frame_type %d,tdd_config %d\n", frame_parms->frame_type,frame_parms->tdd_config);
  LOG_I(PHY,"init eNB: number of ue max %d number of enb max %d number of harq pid max %d\n", 
	NUMBER_OF_UE_MAX, NUMBER_OF_eNB_MAX, NUMBER_OF_HARQ_PID_MAX);

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    for (j=0;j<2;j++) {
      PHY_vars_eNB->dlsch_eNB[i][j] = new_eNB_dlsch(1,NUMBER_OF_HARQ_PID_MAX,frame_parms->N_RB_DL,abstraction_flag);
      if (!PHY_vars_eNB->dlsch_eNB[i][j]) {
	LOG_E(PHY,"Can't get eNB dlsch structures for UE %d \n", i);
	exit(-1);
      }
      else {
	LOG_D(PHY,"dlsch_eNB[%d][%d] => %p\n",i,j,PHY_vars_eNB->dlsch_eNB[i][j]);
	PHY_vars_eNB->dlsch_eNB[i][j]->rnti=0;
      }
    }
    PHY_vars_eNB->ulsch_eNB[1+i] = new_eNB_ulsch(NUMBER_OF_HARQ_PID_MAX,MAX_TURBO_ITERATIONS, frame_parms->N_RB_UL, abstraction_flag);
    if (!PHY_vars_eNB->ulsch_eNB[1+i]) {
      LOG_E(PHY,"Can't get eNB ulsch structures\n");
      exit(-1);
    }

        // this is the transmission mode for the signalling channels
    // this will be overwritten with the real transmission mode by the RRC once the UE is connected
    PHY_vars_eNB->transmission_mode[i] = transmission_mode;
    
  }
  
  // ULSCH for RA
  PHY_vars_eNB->ulsch_eNB[0] = new_eNB_ulsch(NUMBER_OF_HARQ_PID_MAX,MAX_TURBO_ITERATIONS, frame_parms->N_RB_UL, abstraction_flag);
  if (!PHY_vars_eNB->ulsch_eNB[0]) {
    LOG_E(PHY,"Can't get eNB ulsch structures\n");
    exit(-1);
  }
  
  PHY_vars_eNB->dlsch_eNB_SI  = new_eNB_dlsch(1,1,frame_parms->N_RB_DL, abstraction_flag);
  LOG_D(PHY,"eNB %d : SI %p\n",eNB_id,PHY_vars_eNB->dlsch_eNB_SI);
  PHY_vars_eNB->dlsch_eNB_ra  = new_eNB_dlsch(1,1,frame_parms->N_RB_DL, abstraction_flag);
  LOG_D(PHY,"eNB %d : RA %p\n",eNB_id,PHY_vars_eNB->dlsch_eNB_ra);
  PHY_vars_eNB->dlsch_eNB_MCH = new_eNB_dlsch(1,NUMBER_OF_HARQ_PID_MAX,frame_parms->N_RB_DL, 0);
  LOG_D(PHY,"eNB %d : MCH %p\n",eNB_id,PHY_vars_eNB->dlsch_eNB_MCH);
    

  PHY_vars_eNB->rx_total_gain_eNB_dB=140;
  
  for(i=0;i<NUMBER_OF_UE_MAX;i++)
    PHY_vars_eNB->mu_mimo_mode[i].dl_pow_off = 2;
  
  PHY_vars_eNB->check_for_total_transmissions = 0;
  
  PHY_vars_eNB->check_for_MUMIMO_transmissions = 0;
  
  PHY_vars_eNB->FULL_MUMIMO_transmissions = 0;
  
  PHY_vars_eNB->check_for_SUMIMO_transmissions = 0;

  PHY_vars_eNB->lte_frame_parms.pucch_config_common.deltaPUCCH_Shift = 1;

  return (PHY_vars_eNB);
}

PHY_VARS_UE* init_lte_UE(LTE_DL_FRAME_PARMS *frame_parms, 
			 uint8_t UE_id,
			 uint8_t abstraction_flag,
			 uint8_t transmission_mode) {

  int i,j;
  PHY_VARS_UE* PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  memset(PHY_vars_UE,0,sizeof(PHY_VARS_UE));
  PHY_vars_UE->Mod_id=UE_id; 
  memcpy(&(PHY_vars_UE->lte_frame_parms), frame_parms, sizeof(LTE_DL_FRAME_PARMS));
  phy_init_lte_ue(PHY_vars_UE,1,abstraction_flag);
  for (i=0;i<NUMBER_OF_CONNECTED_eNB_MAX;i++) {
    for (j=0;j<2;j++) {
      PHY_vars_UE->dlsch_ue[i][j]  = new_ue_dlsch(1,NUMBER_OF_HARQ_PID_MAX,MAX_TURBO_ITERATIONS,frame_parms->N_RB_DL, abstraction_flag);
      if (!PHY_vars_UE->dlsch_ue[i][j]) {
	LOG_E(PHY,"Can't get ue dlsch structures\n");
	exit(-1);
      }
      else
	LOG_D(PHY,"dlsch_ue[%d][%d] => %p\n",UE_id,i,PHY_vars_UE->dlsch_ue[i][j]);
    }

    
    
    PHY_vars_UE->ulsch_ue[i]  = new_ue_ulsch(NUMBER_OF_HARQ_PID_MAX,frame_parms->N_RB_UL, abstraction_flag);
    if (!PHY_vars_UE->ulsch_ue[i]) {
      LOG_E(PHY,"Can't get ue ulsch structures\n");
      exit(-1);
      }
    
    PHY_vars_UE->dlsch_ue_SI[i]  = new_ue_dlsch(1,1,MAX_TURBO_ITERATIONS,frame_parms->N_RB_DL, abstraction_flag);
    PHY_vars_UE->dlsch_ue_ra[i]  = new_ue_dlsch(1,1,MAX_TURBO_ITERATIONS,frame_parms->N_RB_DL, abstraction_flag);
        
    PHY_vars_UE->transmission_mode[i] = transmission_mode;
  }
  PHY_vars_UE->lte_frame_parms.pucch_config_common.deltaPUCCH_Shift = 1;

  PHY_vars_UE->dlsch_ue_MCH[0]  = new_ue_dlsch(1,NUMBER_OF_HARQ_PID_MAX,MAX_TURBO_ITERATIONS_MBSFN,frame_parms->N_RB_DL,0);

  return (PHY_vars_UE);
}
PHY_VARS_RN* init_lte_RN(LTE_DL_FRAME_PARMS *frame_parms, 
			 uint8_t RN_id,
			 uint8_t eMBMS_active_state) {

  int i;
  PHY_VARS_RN* PHY_vars_RN = malloc(sizeof(PHY_VARS_RN));
  memset(PHY_vars_RN,0,sizeof(PHY_VARS_RN));
  PHY_vars_RN->Mod_id=RN_id;
  
  if (eMBMS_active_state == multicast_relay) {
    for (i=0; i < 10 ; i++){ // num SF in a frame 
      PHY_vars_RN->dlsch_rn_MCH[i] = new_ue_dlsch(1,1,MAX_TURBO_ITERATIONS_MBSFN,frame_parms->N_RB_DL, 0);
      LOG_D(PHY,"eNB %d : MCH[%d] %p\n",RN_id,i,PHY_vars_RN->dlsch_rn_MCH[i]);
    }
  } else {
    PHY_vars_RN->dlsch_rn_MCH[0] = new_ue_dlsch(1,1,MAX_TURBO_ITERATIONS,frame_parms->N_RB_DL, 0);
    LOG_D(PHY,"eNB %d : MCH[0] %p\n",RN_id,PHY_vars_RN->dlsch_rn_MCH[0]);
  }
  return (PHY_vars_RN);
}
void init_lte_vars(LTE_DL_FRAME_PARMS **frame_parms,
		   uint8_t frame_type,
		   uint8_t tdd_config,
		   uint8_t tdd_config_S,
		   uint8_t extended_prefix_flag, 
		   uint8_t N_RB_DL,
		   uint16_t Nid_cell,
		   uint8_t cooperation_flag,uint8_t transmission_mode,uint8_t abstraction_flag,
		   int nb_antennas_rx, uint8_t eMBMS_active_state) {

  uint8_t eNB_id,UE_id,RN_id,CC_id;

  mac_xface = malloc(sizeof(MAC_xface));

  memset(mac_xface, 0, sizeof(MAC_xface));

  LOG_I(PHY,"init lte parms: Nid_cell %d, Frame type %d, N_RB_DL %d\n",Nid_cell,frame_type,N_RB_DL);

  *frame_parms = calloc(1, sizeof(LTE_DL_FRAME_PARMS));
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
  (*frame_parms)->nb_antennas_rx     = (transmission_mode == 1) ? 1 : 2;
  (*frame_parms)->mode1_flag = (transmission_mode == 1) ? 1 : 0;

  init_frame_parms(*frame_parms,1);

  (*frame_parms)->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift = 0;//n_DMRS1 set to 0
  (*frame_parms)->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled = 1;
  (*frame_parms)->pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled = 0;
  (*frame_parms)->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH = 0;
  init_ul_hopping(*frame_parms);

  phy_init_top(*frame_parms);

  phy_init_lte_top(*frame_parms);

  PHY_vars_eNB_g = (PHY_VARS_eNB***)malloc(NB_eNB_INST*sizeof(PHY_VARS_eNB**));
  for (eNB_id=0; eNB_id<NB_eNB_INST;eNB_id++){ 
    PHY_vars_eNB_g[eNB_id] = (PHY_VARS_eNB**) malloc(MAX_NUM_CCs*sizeof(PHY_VARS_eNB*));
    for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
      PHY_vars_eNB_g[eNB_id][CC_id] = init_lte_eNB(*frame_parms,eNB_id,Nid_cell,cooperation_flag,transmission_mode,abstraction_flag);
      PHY_vars_eNB_g[eNB_id][CC_id]->Mod_id=eNB_id;
      PHY_vars_eNB_g[eNB_id][CC_id]->CC_id=CC_id;
    }
  }

  (*frame_parms)->nb_antennas_tx     = 1;
  (*frame_parms)->nb_antennas_rx     = nb_antennas_rx;

  PHY_vars_UE_g = (PHY_VARS_UE***)malloc(NB_UE_INST*sizeof(PHY_VARS_UE**));
  for (UE_id=0; UE_id<NB_UE_INST;UE_id++){
    PHY_vars_UE_g[UE_id] = (PHY_VARS_UE**) malloc(MAX_NUM_CCs*sizeof(PHY_VARS_UE*));
    for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
      PHY_vars_UE_g[UE_id][CC_id] = init_lte_UE(*frame_parms, UE_id,abstraction_flag,transmission_mode);
      PHY_vars_UE_g[UE_id][CC_id]->Mod_id=UE_id;
      PHY_vars_UE_g[UE_id][CC_id]->CC_id=CC_id;
    }
  }

  if (NB_RN_INST > 0) {
    PHY_vars_RN_g = malloc(NB_RN_INST*sizeof(PHY_VARS_RN*));
    for (RN_id=0; RN_id<NB_RN_INST;RN_id++){
      PHY_vars_RN_g[RN_id] = init_lte_RN(*frame_parms,RN_id,eMBMS_active_state);
    }
  }
  
}
