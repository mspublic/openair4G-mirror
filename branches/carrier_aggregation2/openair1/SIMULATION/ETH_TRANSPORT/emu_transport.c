/*! \file phy_emulation.c
* \brief implements the underlying protocol for emulated data exchange over Ethernet using IP multicast
* \author Navid Nikaein
* \date 2011
* \version 1.0 
* \company Eurecom
* \email: navid.nikaein@eurecom.fr
*/ 


#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"
#include "UTIL/OCG/OCG.h"
#include "UTIL/OCG/OCG_extern.h"
#include "UTIL/LOG/log.h"

extern unsigned int   Master_list_rx;
extern unsigned short NODE_ID[1];
extern unsigned char  NB_INST;
//#define DEBUG_CONTROL 1

/******************************************************************************************************/ 
/*
char is_node_local_neighbor(unsigned short Node_id){
  int i;
  for(i=0;i<NB_INST;i++)
    if(NODE_ID[i]==Node_id) return 1;
  return 0; 
}
*/

void emu_transport_sync(void){
 
  if (oai_emulation.info.is_primary_master==0){

    bypass_tx_data(WAIT_SM_TRANSPORT,0,0);
    Master_list_rx=oai_emulation.info.master_list-1; // just wait to recieve the  master 0 msg
    bypass_rx_data(0,0,0);
  }
  else {
    bypass_rx_data(0,0,0);
    bypass_tx_data(WAIT_PM_TRANSPORT,0,0);
  }   

  if (oai_emulation.info.master_list!=0){

    bypass_tx_data(SYNC_TRANSPORT,0,0);	
    bypass_rx_data(0,0,0);

    if (emu_rx_status == SYNCED_TRANSPORT){ // i received the sync from all secondary masters 
      emu_tx_status = SYNCED_TRANSPORT;
    }
    // else  emu_transport_sync(last_slot);
    LOG_D(EMU,"TX secondary master SYNC_TRANSPORT state \n");
  }
  
}

void emu_transport(unsigned int frame, unsigned int last_slot, unsigned int next_slot,lte_subframe_t direction, unsigned char frame_type, int ethernet_flag ){
  
  if (ethernet_flag == 0)
    return;
 
  //DL
  if ( ( (frame_type == 1) &&  (direction == SF_DL )) || (frame_type == 0) ){ 
    emu_transport_DL(frame, last_slot,next_slot);
  }
  // UL
  if ( ((frame_type == 1) &&  (direction == SF_UL)) || (frame_type == 0) ){
    emu_transport_UL(frame, last_slot , next_slot);
  }
}


void emu_transport_DL(unsigned int frame, unsigned int last_slot, unsigned int next_slot) {

   if (oai_emulation.info.is_primary_master==0){
    //  bypass_rx_data(last_slot);
    if (oai_emulation.info.nb_enb_local>0) // send in DL if 
      bypass_tx_data(ENB_TRANSPORT,frame, next_slot);
    else
      bypass_tx_data(WAIT_SM_TRANSPORT,frame,next_slot);

    bypass_rx_data(frame, last_slot, next_slot);
  }
  else { // I am the master
    // bypass_tx_data(WAIT_TRANSPORT,last_slot);
    bypass_rx_data(frame,last_slot, next_slot);
    if (oai_emulation.info.nb_enb_local>0) // send in DL if 
      bypass_tx_data(ENB_TRANSPORT,frame, next_slot);
    else
      bypass_tx_data(WAIT_SM_TRANSPORT,frame, next_slot);
  }   

}

void emu_transport_UL(unsigned int frame, unsigned int last_slot, unsigned int next_slot) {
   
    
  if (oai_emulation.info.is_primary_master==0){
    // bypass_rx_data(last_slot, next_slot);
    if (oai_emulation.info.nb_ue_local>0)
      bypass_tx_data(UE_TRANSPORT,frame, next_slot);
    else
      bypass_tx_data(WAIT_SM_TRANSPORT,frame, next_slot);
    bypass_rx_data(frame,last_slot, next_slot);
  }
  else {  
    // bypass_tx_data(WAIT_TRANSPORT,last_slot);
    bypass_rx_data(frame,last_slot, next_slot);
    if (oai_emulation.info.nb_ue_local>0)
      bypass_tx_data(UE_TRANSPORT,frame, next_slot);
    else
      bypass_tx_data(WAIT_SM_TRANSPORT,frame, next_slot);
  }
  
}

void emu_transport_release(void){
  bypass_tx_data(RELEASE_TRANSPORT,0,0);
  LOG_E(EMU," tx RELEASE_TRANSPORT  \n");
}
 
unsigned int emul_tx_handler(unsigned char Mode,char *Tx_buffer,unsigned int* Nbytes,unsigned int *Nb_flows){
  unsigned short k,Mod_id;
  
  for(k=0;k<NB_INST;k++){//Nb_out_src[Mode];k++){
    Mod_id=k;//Out_list[Mode][k];
  }  
  return *Nbytes;
}

unsigned int emul_rx_data(void){
  return(0);
}

unsigned int emul_rx_handler(unsigned char Mode,char *rx_buffer, unsigned int Nbytes){
  unsigned short Rx_size=0;
  return (Rx_size+2);  
}

void clear_eNB_transport_info(u8 nb_eNB) {
  u8 eNB_id;
  
  for (eNB_id=0;eNB_id<nb_eNB;eNB_id++) {
    eNB_transport_info_TB_index[eNB_id]=0;
    memset((void *)&eNB_transport_info[eNB_id].cntl,0,sizeof(eNB_cntl));
    eNB_transport_info[eNB_id].num_common_dci=0;
    eNB_transport_info[eNB_id].num_ue_spec_dci=0;
  }
  //  LOG_T(EMU, "EMUL clear_eNB_transport_info\n");
}

void clear_UE_transport_info(u8 nb_UE) {
  u8 UE_id;
  
  for (UE_id=0;UE_id<nb_UE;UE_id++) {
    UE_transport_info_TB_index[UE_id]=0;
    memset((void *)&UE_transport_info[UE_id].cntl,0,sizeof(UE_cntl));
  } 
  //  LOG_T(EMU, "EMUL clear_UE_transport_info\n");
}


void fill_phy_enb_vars(unsigned int enb_id, unsigned int next_slot) {

  int n_dci=0, n_dci_dl;
  int payload_offset = 0;
  unsigned int harq_pid;
  LTE_eNB_DLSCH_t *dlsch_eNB;
  unsigned short ue_id;
  u8 nb_total_dci;
    

  //LOG_I(EMU," pbch fill phy eNB %d vars for slot %d \n",enb_id, next_slot);
   
  // eNB
  // PBCH : copy payload 
 
  //if (next_slot == 2){ 
    *(u32*)PHY_vars_eNB_g[enb_id]->pbch_pdu = eNB_transport_info[enb_id].cntl.pbch_payload;
    /*  LOG_I(EMU," RX slot %d ENB TRANSPORT pbch payload %d pdu[0] %d  pdu[0] %d \n", 
	  next_slot ,
	  eNB_transport_info[enb_id].cntl.pbch_payload,
	  ((u8*)PHY_vars_eNB_g[enb_id]->pbch_pdu)[0],
	  ((u8*)PHY_vars_eNB_g[enb_id]->pbch_pdu)[1]);
    */
    //  }
  //CFI
  // not needed yet
  
  //PHICH
  // to be added later

  //DCI
  nb_total_dci= eNB_transport_info[enb_id].num_ue_spec_dci+ eNB_transport_info[enb_id].num_common_dci;
  PHY_vars_eNB_g[enb_id]->num_ue_spec_dci[(next_slot>>1)&1] = eNB_transport_info[enb_id].num_ue_spec_dci;
  PHY_vars_eNB_g[enb_id]->num_common_dci[(next_slot>>1)&1]  = eNB_transport_info[enb_id].num_common_dci;

  if (nb_total_dci >0) {		 
    
    memcpy(PHY_vars_eNB_g[enb_id]->dci_alloc[(next_slot>>1)&1],  
	   &eNB_transport_info[enb_id].dci_alloc,
	   (nb_total_dci)* sizeof(DCI_ALLOC_t));
  
   
    n_dci_dl=0;
    // fill dlsch_eNB structure from DCI
    for (n_dci =0 ; 
	 n_dci < nb_total_dci;
	 n_dci++) {
      
      if (eNB_transport_info[enb_id].dci_alloc[n_dci_dl].format > 0){ //exclude ul dci
	
	/*	LOG_D(EMU, "dci spec %d common %d tbs is %d payload offset %d\n", 
	      eNB_transport_info[enb_id].num_ue_spec_dci, 
	      eNB_transport_info[enb_id].num_common_dci,
	      eNB_transport_info[enb_id].tbs[n_dci_dl], 
	      payload_offset);
	*/
	switch (eNB_transport_info[enb_id].dlsch_type[n_dci_dl]) {
	  
	case 0: //SI:
	  
	  memcpy(PHY_vars_eNB_g[enb_id]->dlsch_eNB_SI->harq_processes[0]->b,
		 &eNB_transport_info[enb_id].transport_blocks[payload_offset],
		 eNB_transport_info[enb_id].tbs[n_dci_dl]);
	  //  LOG_D(EMU, "SI eNB_transport_info[enb_id].tbs[n_dci_dl]%d \n", eNB_transport_info[enb_id].tbs[n_dci_dl]);
	  break;
	case 1: //RA:
	  
	  memcpy(PHY_vars_eNB_g[enb_id]->dlsch_eNB_ra->harq_processes[0]->b,
		 &eNB_transport_info[enb_id].transport_blocks[payload_offset],
		 eNB_transport_info[enb_id].tbs[n_dci_dl]);
	  //LOG_D(EMU, "RA eNB_transport_info[enb_id].tbs[n_dci_dl]%d \n", eNB_transport_info[enb_id].tbs[n_dci_dl]);
	  break;
  
	case 2://TB0:
	  harq_pid  = eNB_transport_info[enb_id].harq_pid[n_dci_dl];
	  ue_id = eNB_transport_info[enb_id].ue_id[n_dci_dl];
	  PHY_vars_eNB_g[enb_id]->dlsch_eNB[ue_id][0]->rnti= eNB_transport_info[enb_id].dci_alloc[n_dci_dl].rnti;
	  //LOG_D(EMU, " enb_id %d ue id is %d rnti is %x \n", 
	  //		enb_id, ue_id, eNB_transport_info[enb_id].dci_alloc[n_dci_dl].rnti);
	  dlsch_eNB = PHY_vars_eNB_g[enb_id]->dlsch_eNB[ue_id][0];
	  
	  
	  memcpy(dlsch_eNB->harq_processes[harq_pid]->b,
		 &eNB_transport_info[enb_id].transport_blocks[payload_offset],
		 eNB_transport_info[enb_id].tbs[n_dci_dl]);
	  break;
	  
	case 3://TB1:
	  harq_pid = eNB_transport_info[enb_id].harq_pid[n_dci_dl];
	  ue_id = eNB_transport_info[enb_id].ue_id[n_dci_dl];
	  PHY_vars_eNB_g[enb_id]->dlsch_eNB[ue_id][1]->rnti= eNB_transport_info[enb_id].dci_alloc[n_dci_dl].rnti;
	  dlsch_eNB = PHY_vars_eNB_g[enb_id]->dlsch_eNB[ue_id][1];
	  
	  memcpy(dlsch_eNB->harq_processes[harq_pid]->b,
		 &eNB_transport_info[enb_id].transport_blocks[payload_offset],
		 eNB_transport_info[enb_id].tbs[n_dci_dl]);
	  break;
	  
	}
	payload_offset += eNB_transport_info[enb_id].tbs[n_dci_dl];
      }
      n_dci_dl++;
    }
    LOG_I(EMU, "Fill phy eNB vars done !\n");
  }
}

void fill_phy_ue_vars(unsigned int ue_id, unsigned int last_slot) {

  int n_enb;//index
  int enb_id;
  //  int harq_id;
  //  int payload_offset = 0;
  unsigned short rnti;
  unsigned int harq_pid;
  LTE_UE_ULSCH_t *ulsch;
  PUCCH_FMT_t pucch_format;
  //  u8 ue_transport_info_index[NUMBER_OF_eNB_MAX];
  u8 subframe = last_slot>>1;
 
  memcpy (&ue_cntl_delay[(subframe+1)%2],
  	  &UE_transport_info[ue_id].cntl,
  	  sizeof(UE_cntl));

   
   LOG_D(EMU, "Fill phy UE %d vars PRACH is (%d, %d) preamble (%d,%d)!\n", 
	ue_id,
	UE_transport_info[ue_id].cntl.prach_flag,
	 ue_cntl_delay[subframe%2].prach_flag,
	 UE_transport_info[ue_id].cntl.prach_id,
	 ue_cntl_delay[subframe%2].prach_id);

   //ue_cntl_delay[subframe%2].prach_flag ;
   PHY_vars_UE_g[ue_id]->generate_prach = UE_transport_info[ue_id].cntl.prach_flag; 
   if (PHY_vars_UE_g[ue_id]->generate_prach == 1) {
     //     if (PHY_vars_UE_g[ue_id]->prach_resources[enb_id] == NULL)
     //  PHY_vars_UE_g[ue_id]->prach_resources[enb_id] = malloc(sizeof(PRACH_RESOURCES_t));
     //ue_cntl_delay[subframe%2].prach_id;
     PHY_vars_UE_g[ue_id]->prach_PreambleIndex = UE_transport_info[ue_id].cntl.prach_id; 
   }

   for (n_enb=0; n_enb < UE_transport_info[ue_id].num_eNB; n_enb++){
    
     //LOG_D(EMU,"Setting ulsch vars for ue %d rnti %x \n",ue_id, UE_transport_info[ue_id].rnti[n_enb]);
     
     pucch_format= UE_transport_info[ue_id].cntl.pucch_flag;
     
     PHY_vars_UE_g[ue_id]->sr[subframe] = ue_cntl_delay[subframe%2].sr;// UE_transport_info[ue_id].cntl.sr;
     
     //if (PHY_vars_UE_g[ue_id]->sr) LOG_I(EMU,"SR is %d \n", PHY_vars_UE_g[ue_id]->sr);
     
     if ((pucch_format == pucch_format1a) || (pucch_format == pucch_format1b )){
       PHY_vars_UE_g[ue_id]->pucch_payload[0] = UE_transport_info[ue_id].cntl.pucch_payload;
     }
     
     rnti = UE_transport_info[ue_id].rnti[n_enb];
     enb_id = UE_transport_info[ue_id].eNB_id[n_enb];



     
     PHY_vars_UE_g[ue_id]->lte_ue_pdcch_vars[enb_id]->crnti=rnti;
     
     
     harq_pid = UE_transport_info[ue_id].harq_pid[n_enb];
     
     ulsch = PHY_vars_UE_g[ue_id]->ulsch_ue[enb_id];
     
     ulsch->o_RI[0]                          = ue_cntl_delay[subframe%2].pusch_ri & 0x1;
     ulsch->o_RI[1]                          = (ue_cntl_delay[subframe%2].pusch_ri>>1) & 0x1;
     
     ulsch->o_ACK[0]                          = ue_cntl_delay[subframe%2].pusch_ack & 0x1;
     ulsch->o_ACK[1]                          = (ue_cntl_delay[subframe%2].pusch_ack>>1) & 0x1;
     
     
     memcpy(PHY_vars_UE_g[ue_id]->ulsch_ue[enb_id]->harq_processes[harq_pid]->b,
	    UE_transport_info[ue_id].transport_blocks,
	    UE_transport_info[ue_id].tbs[enb_id]);
     
     //ue_transport_info_index[enb_id]+=UE_transport_info[ue_id].tbs[enb_id];
     
     //UE_transport_info[ue_id].transport_blocks+=ue_transport_info_index[enb_id];
     //LOG_T(EMU,"ulsch tbs is %d\n", UE_transport_info[ue_id].tbs[enb_id]);
  }
}
