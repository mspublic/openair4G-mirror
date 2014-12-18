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

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

*******************************************************************************/

/*! \file eNB_scheduler_primitives.c
 * \brief primitives used by eNB for BCH, RACH, ULSCH, DLSCH scheduling
 * \author  Navid Nikaein and Raymond Knopp
 * \date 2010 - 2014
 * \email: navid.nikaein@eurecom.fr
 * \version 1.0
 * @ingroup _mac

 */

#include "assertions.h"
#include "PHY/defs.h"
#include "PHY/extern.h"

#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"

#include "LAYER2/MAC/proto.h"
#include "UTIL/LOG/log.h"
#include "UTIL/LOG/vcd_signal_dumper.h"
#include "UTIL/OPT/opt.h"
#include "OCG.h"
#include "OCG_extern.h"

#include "RRC/LITE/extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"

//#include "LAYER2/MAC/pre_processor.c"
#include "pdcp.h"

#if defined(ENABLE_ITTI)
# include "intertask_interface.h"
#endif

#define ENABLE_MAC_PAYLOAD_DEBUG
#define DEBUG_eNB_SCHEDULER 1


void init_ue_sched_info(void){
  module_id_t i,j,k;
  for (i=0;i<NUMBER_OF_eNB_MAX;i++){
    for (k=0;i<MAX_NUM_CCs;i++){
      for (j=0;j<NUMBER_OF_UE_MAX;j++){
          // init DL
          eNB_dlsch_info[i][k][j].weight           = 0;
          eNB_dlsch_info[i][k][j].subframe         = 0;
          eNB_dlsch_info[i][k][j].serving_num      = 0;
          eNB_dlsch_info[i][k][j].status           = S_DL_NONE;
          // init UL
          eNB_ulsch_info[i][k][j].subframe         = 0;
          eNB_ulsch_info[i][k][j].serving_num      = 0;
          eNB_ulsch_info[i][k][j].status           = S_UL_NONE;
      }
    }
  }
}



unsigned char get_ue_weight(module_id_t module_idP, int CC_id, int ue_idP){

  return(eNB_dlsch_info[module_idP][CC_id][ue_idP].weight);

}

DCI_PDU *get_dci_sdu(module_id_t module_idP, int CC_id,frame_t frameP, sub_frame_t subframeP) {

  return(&eNB_mac_inst[module_idP].common_channels[CC_id].DCI_pdu);

}

int find_UE_id(module_id_t mod_idP, rnti_t rntiP) {

  int UE_id;
  UE_list_t *UE_list = &eNB_mac_inst[mod_idP].UE_list;
  
  for (UE_id=UE_list->head;UE_id>=0;UE_id=UE_list->next[UE_id]){
    if (UE_list->UE_template[UE_PCCID(mod_idP,UE_id)][UE_id].rnti==rntiP) {
      return(UE_id);
    }
  }
  return(-1);

}

int UE_num_active_CC(UE_list_t *listP,int ue_idP) {
  return(listP->numactiveCCs[ue_idP]);
}

int UE_PCCID(module_id_t mod_idP,int ue_idP) {
  return(eNB_mac_inst[mod_idP].UE_list.pCC_id[ue_idP]);
}

rnti_t UE_RNTI(module_id_t mod_idP, int ue_idP) {

  rnti_t rnti = eNB_mac_inst[mod_idP].UE_list.UE_template[UE_PCCID(mod_idP,ue_idP)][ue_idP].rnti;

  if (rnti>0)
    return (rnti);
  LOG_E(MAC,"[eNB %d] Couldn't find RNTI for UE %d\n",mod_idP,ue_idP);
  mac_xface->macphy_exit("");
  return(0);
}

boolean_t is_UE_active(module_id_t mod_idP, int ue_idP){
  return(eNB_mac_inst[mod_idP].UE_list.active[ue_idP]);
}

/*
uint8_t find_active_UEs(module_id_t module_idP,int CC_id){

  module_id_t        ue_mod_id      = 0;
  rnti_t        rnti         = 0;
  uint8_t            nb_active_ue = 0;

  for (ue_mod_id=0;ue_mod_id<NUMBER_OF_UE_MAX;ue_mod_id++) {

      if (((rnti=eNB_mac_inst[module_idP][CC_id].UE_template[ue_mod_id].rnti) !=0)&&(eNB_mac_inst[module_idP][CC_id].UE_template[ue_mod_id].ul_active==TRUE)){

          if (mac_xface->get_eNB_UE_stats(module_idP,rnti) != NULL){ // check at the phy enb_ue state for this rnti
	    nb_active_ue++;
          }
          else { // this ue is removed at the phy => remove it at the mac as well
	    mac_remove_ue(module_idP, CC_id, ue_mod_id);
          }
      }
  }
  return(nb_active_ue);
}
*/


// get aggregatiob form phy for a give UE
unsigned char process_ue_cqi (module_id_t module_idP, int ue_idP) {
  unsigned char aggregation=2;
  // check the MCS and SNR and set the aggregation accordingly
  return aggregation;
}
#ifdef CBA
uint8_t find_num_active_UEs_in_cbagroup(module_id_t module_idP, int CC_id,unsigned char group_id){

  module_id_t    UE_id;
  rnti_t    rnti;
  unsigned char nb_ue_in_pusch=0;
  LTE_eNB_UE_stats* eNB_UE_stats;

  for (UE_id=group_id;UE_id<NUMBER_OF_UE_MAX;UE_id+=eNB_mac_inst[module_idP][CC_id].num_active_cba_groups) {

      if (((rnti=eNB_mac_inst[module_idP][CC_id].UE_template[UE_id].rnti) !=0) &&
          (eNB_mac_inst[module_idP][CC_id].UE_template[UE_id].ul_active==TRUE)    &&
          (mac_get_rrc_status(module_idP,1,UE_id) > RRC_CONNECTED)){
	//  && (UE_is_to_be_scheduled(module_idP,UE_id)))
	// check at the phy enb_ue state for this rnti
	if ((eNB_UE_stats= mac_xface->get_eNB_UE_stats(module_idP,CC_id,rnti)) != NULL){
	  if ((eNB_UE_stats->mode == PUSCH) && (UE_is_to_be_scheduled(module_idP,UE_id) == 0)){
	    nb_ue_in_pusch++;
	  }
	}
      }
  }
  return(nb_ue_in_pusch);
}
#endif

void dump_ue_list(UE_list_t *listP, int ul_flag) {
  int j;
  if ( ul_flag == 0 ){
    for (j=listP->head;j>=0;j=listP->next[j]) {
      LOG_T(MAC,"node %d => %d\n",j,listP->next[j]);
    }
  } else {
    for (j=listP->head_ul;j>=0;j=listP->next_ul[j]) {
      LOG_T(MAC,"node %d => %d\n",j,listP->next_ul[j]);
    }
  }
}

int add_new_ue(module_id_t mod_idP, int cc_idP, rnti_t rntiP,int harq_pidP) {
  int UE_id;
  int j;

  UE_list_t *UE_list = &eNB_mac_inst[mod_idP].UE_list;
  
  LOG_D(MAC,"[eNB %d, CC_id %d] Adding UE with rnti %x (next avail %d, num_UEs %d)\n",mod_idP,cc_idP,rntiP,UE_list->avail,UE_list->num_UEs);
  dump_ue_list(UE_list,0);

  if (UE_list->avail>=0) {
    UE_id = UE_list->avail;
    UE_list->avail = UE_list->next[UE_list->avail];
    UE_list->next[UE_id] = UE_list->head;
    UE_list->next_ul[UE_id] = UE_list->head;
    UE_list->head = UE_id;
    UE_list->head_ul = UE_id;
    UE_list->UE_template[cc_idP][UE_id].rnti       = rntiP;
    UE_list->UE_template[cc_idP][UE_id].configured = FALSE;
    UE_list->numactiveCCs[UE_id]                   = 1;
    UE_list->numactiveULCCs[UE_id]                 = 1;
    UE_list->pCC_id[UE_id]                         = cc_idP;
    UE_list->ordered_CCids[0][UE_id]               = cc_idP;
    UE_list->ordered_ULCCids[0][UE_id]             = cc_idP;
    UE_list->num_UEs++;
    UE_list->active[UE_id]                         = TRUE;

    for (j=0;j<8;j++) {
      UE_list->UE_template[cc_idP][UE_id].oldNDI[j]    = (j==0)?1:0;   // 1 because first transmission is with format1A (Msg4) for harq_pid 0 
      UE_list->UE_template[cc_idP][UE_id].oldNDI_UL[j] = (j==harq_pidP)?0:1; // 1st transmission is with Msg3;
    }
    eNB_ulsch_info[mod_idP][cc_idP][UE_id].status = S_UL_WAITING;
    eNB_dlsch_info[mod_idP][cc_idP][UE_id].status = S_UL_WAITING;
    LOG_D(MAC,"[eNB %d] Add UE_id %d on Primary CC_id %d: rnti %x\n",mod_idP,UE_id,cc_idP,rntiP);
    dump_ue_list(UE_list,0);
    return(UE_id);
  }

  LOG_E(MAC,"error in add_new_ue(), could not find space in UE_list, Dumping UE list\n");
  dump_ue_list(UE_list,0);
  return(-1);
}

int mac_remove_ue(module_id_t mod_idP, int ue_idP, int frameP) {

  int prev,i, ret=-1;

  UE_list_t *UE_list = &eNB_mac_inst[mod_idP].UE_list;
  int pCC_id = UE_PCCID(mod_idP,ue_idP);

  LOG_I(MAC,"Removing UE %d from Primary CC_id %d (rnti %x)\n",ue_idP,pCC_id, UE_list->UE_template[pCC_id][ue_idP].rnti);
  dump_ue_list(UE_list,0);

  // clear all remaining pending transmissions
  UE_list->UE_template[pCC_id][ue_idP].bsr_info[LCGID0]  = 0;
  UE_list->UE_template[pCC_id][ue_idP].bsr_info[LCGID1]  = 0;
  UE_list->UE_template[pCC_id][ue_idP].bsr_info[LCGID2]  = 0;
  UE_list->UE_template[pCC_id][ue_idP].bsr_info[LCGID3]  = 0;

  UE_list->UE_template[pCC_id][ue_idP].ul_SR             = 0;
  UE_list->UE_template[pCC_id][ue_idP].rnti              = 0;
  UE_list->UE_template[pCC_id][ue_idP].ul_active         = FALSE;
  eNB_ulsch_info[mod_idP][pCC_id][ue_idP].rnti                        = 0;
  eNB_ulsch_info[mod_idP][pCC_id][ue_idP].status                      = S_UL_NONE;
  eNB_dlsch_info[mod_idP][pCC_id][ue_idP].rnti                        = 0;
  eNB_dlsch_info[mod_idP][pCC_id][ue_idP].status                      = S_DL_NONE;

  rrc_eNB_free_UE_index(mod_idP,ue_idP,frameP);

  prev = UE_list->head;
  for (i=UE_list->head;i>=0;i=UE_list->next[i]) {
    if (i == ue_idP) {
      // link prev to next in Active list
      if (prev==UE_list->head)
	UE_list->head = UE_list->next[i];
      else
	UE_list->next[prev] = UE_list->next[i];
      // add UE id (i)to available 
      UE_list->next[i] = UE_list->avail;
      UE_list->avail = i;
      UE_list->active[i] = FALSE;
      UE_list->num_UEs--;  
      ret=0;
      break;
    }
    prev=i;
  }
  // do the same for UL
  prev = UE_list->head_ul;
  for (i=UE_list->head_ul;i>=0;i=UE_list->next_ul[i]) {
    if (i == ue_idP) {
      // link prev to next in Active list
      if (prev==UE_list->head_ul)
	UE_list->head_ul = UE_list->next_ul[i];
      else
	UE_list->next_ul[prev] = UE_list->next_ul[i];
      // add UE id (i)to available 
      UE_list->next_ul[i] = UE_list->avail;
      ret = 0;
      break;
    }
    prev=i;
  }
  if (ret == 0)
    return (0);
  
  LOG_E(MAC,"error in mac_remove_ue(), could not find previous to %d in UE_list, should never happen, Dumping UE list\n",ue_idP);
  dump_ue_list(UE_list,0);
  mac_xface->macphy_exit("");    
  return(-1);

}



int prev(UE_list_t *listP, int nodeP, int ul_flag) {
  int j;
  
  if (ul_flag == 0 ) {
    if (nodeP==listP->head)
      return(nodeP);
    for (j=listP->head;j>=0;j=listP->next[j]) {
      if (listP->next[j]==nodeP)
	return(j);
    }
  } else {
    if (nodeP==listP->head_ul)
      return(nodeP);
    for (j=listP->head_ul;j>=0;j=listP->next_ul[j]) {
      if (listP->next_ul[j]==nodeP)
	return(j);
    }
  }


  
  LOG_E(MAC,"error in prev(), could not find previous to %d in UE_list %s, should never happen, Dumping UE list\n",
	nodeP, (ul_flag == 0)? "DL" : "UL");
  dump_ue_list(listP, ul_flag);


  return(-1);
}

void swap_UEs(UE_list_t *listP,int nodeiP, int nodejP, int ul_flag) {

  int prev_i,prev_j,next_i,next_j;

  LOG_T(MAC,"Swapping UE %d,%d\n",nodeiP,nodejP);
  dump_ue_list(listP,ul_flag);

  prev_i = prev(listP,nodeiP,ul_flag);
  prev_j = prev(listP,nodejP,ul_flag);
  if ((prev_i<0) || (prev_j<0))
    mac_xface->macphy_exit("");    

  if (ul_flag == 0){
    next_i = listP->next[nodeiP];
    next_j = listP->next[nodejP];
  } else {
    next_i = listP->next_ul[nodeiP];
    next_j = listP->next_ul[nodejP];
  }

  LOG_T(MAC,"[%s] next_i %d, next_i, next_j %d, head %d \n",
	(ul_flag == 0)? "DL" : "UL",
	next_i,next_j,listP->head);

  if (ul_flag == 0 ) {

    if (next_i == nodejP) {   // case ... p(i) i j n(j) ... => ... p(j) j i n(i) ...
      LOG_T(MAC,"Case ... p(i) i j n(j) ... => ... p(j) j i n(i) ...\n");
      
      listP->next[nodeiP] = next_j;
      listP->next[nodejP] = nodeiP;
      if (nodeiP==listP->head)  // case i j n(j)
	listP->head = nodejP;
      else
	listP->next[prev_i] = nodejP;
    }
    else if (next_j == nodeiP) {    // case ... p(j) j i n(i) ... => ... p(i) i j n(j) ...
      LOG_T(MAC,"Case ... p(j) j i n(i) ... => ... p(i) i j n(j) ...\n");
      listP->next[nodejP] = next_i;
      listP->next[nodeiP] = nodejP;
      if (nodejP==listP->head)  // case j i n(i)
	listP->head = nodeiP;
      else
	listP->next[prev_j] = nodeiP;
    }
    else {    // case ...  p(i) i n(i) ... p(j) j n(j) ...
      
      listP->next[nodejP] = next_i;
      listP->next[nodeiP] = next_j;
      
      
      if (nodeiP==listP->head) {
	LOG_T(MAC,"changing head to %d\n",nodejP);
	listP->head=nodejP;
	listP->next[prev_j] = nodeiP;
      }
      else if (nodejP==listP->head){
	LOG_D(MAC,"changing head to %d\n",nodeiP);
	listP->head=nodeiP;
	listP->next[prev_i] = nodejP;
      }
      else {
	listP->next[prev_i] = nodejP;
	listP->next[prev_j] = nodeiP;
      }
    }
  } else { // ul_flag

    if (next_i == nodejP) {   // case ... p(i) i j n(j) ... => ... p(j) j i n(i) ...
      LOG_T(MAC,"[UL] Case ... p(i) i j n(j) ... => ... p(j) j i n(i) ...\n");
      
      listP->next_ul[nodeiP] = next_j;
      listP->next_ul[nodejP] = nodeiP;
      if (nodeiP==listP->head_ul)  // case i j n(j)
	listP->head_ul = nodejP;
      else
	listP->next_ul[prev_i] = nodejP;
    }
    else if (next_j == nodeiP) {    // case ... p(j) j i n(i) ... => ... p(i) i j n(j) ...
      LOG_T(MAC,"[UL]Case ... p(j) j i n(i) ... => ... p(i) i j n(j) ...\n");
      listP->next_ul[nodejP] = next_i;
      listP->next_ul[nodeiP] = nodejP;
      if (nodejP==listP->head_ul)  // case j i n(i)
	listP->head_ul = nodeiP;
      else
	listP->next_ul[prev_j] = nodeiP;
    }
    else {    // case ...  p(i) i n(i) ... p(j) j n(j) ...
      
      listP->next_ul[nodejP] = next_i;
      listP->next_ul[nodeiP] = next_j;
      
      
      if (nodeiP==listP->head_ul) {
	LOG_T(MAC,"[UL]changing head to %d\n",nodejP);
	listP->head_ul=nodejP;
	listP->next_ul[prev_j] = nodeiP;
      }
      else if (nodejP==listP->head_ul){
	LOG_T(MAC,"[UL]changing head to %d\n",nodeiP);
	listP->head_ul=nodeiP;
	listP->next_ul[prev_i] = nodejP;
      }
      else {
	listP->next_ul[prev_i] = nodejP;
	listP->next_ul[prev_j] = nodeiP;
      }
    }
  }
  LOG_T(MAC,"After swap\n");
  dump_ue_list(listP,ul_flag);
}

void SR_indication(module_id_t mod_idP, int cc_idP, frame_t frameP, rnti_t rntiP, sub_frame_t subframeP) {

  int UE_id = find_UE_id(mod_idP, rntiP);
  UE_list_t *UE_list = &eNB_mac_inst[mod_idP].UE_list;

  if (UE_id  != UE_INDEX_INVALID ) {
    LOG_D(MAC,"[eNB %d][SR %x] Frame %d subframeP %d Signaling SR for UE %d on CC_id %d\n",mod_idP,rntiP,frameP,subframeP, UE_id,cc_idP);
      UE_list->UE_template[cc_idP][UE_id].ul_SR = 1;
      UE_list->UE_template[cc_idP][UE_id].ul_active = TRUE;
  } else {
    //     AssertFatal(0, "find_UE_id(%u,rnti %d) not found", enb_mod_idP, rntiP);
    //    AssertError(0, 0, "Frame %d: find_UE_id(%u,rnti %d) not found\n", frameP, enb_mod_idP, rntiP);
    LOG_D(MAC,"[eNB %d][SR %x] Frame %d subframeP %d Signaling SR for UE %d (unknown UEid) \n",mod_idP,rntiP,frameP,subframeP, UE_id);
  }
}




/*
  #ifdef Rel10
  unsigned char generate_mch_header( unsigned char *mac_header,
  unsigned char num_sdus,
  unsigned short *sdu_lengths,
  unsigned char *sdu_lcids,
  unsigned char msi,
  unsigned char short_padding,
  unsigned short post_padding) {

  SCH_SUBHEADER_FIXED *mac_header_ptr = (SCH_SUBHEADER_FIXED *)mac_header;
  uint8_t first_element=0,last_size=0,i;
  uint8_t mac_header_control_elements[2*num_sdus],*ce_ptr;

  ce_ptr = &mac_header_control_elements[0];

  if ((short_padding == 1) || (short_padding == 2)) {
  mac_header_ptr->R    = 0;
  mac_header_ptr->E    = 0;
  mac_header_ptr->LCID = SHORT_PADDING;
  first_element=1;
  last_size=1;
  }
  if (short_padding == 2) {
  mac_header_ptr->E = 1;
  mac_header_ptr++;
  mac_header_ptr->R = 0;
  mac_header_ptr->E    = 0;
  mac_header_ptr->LCID = SHORT_PADDING;
  last_size=1;
  }

  // SUBHEADER for MSI CE
  if (msi != 0) {// there is MSI MAC Control Element
  if (first_element>0) {
  mac_header_ptr->E = 1;
  mac_header_ptr+=last_size;
  }
  else {
  first_element = 1;
  }
  if (num_sdus*2 < 128) {
  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->R    = 0;
  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->E    = 0;
  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F    = 0;
  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->LCID = MCH_SCHDL_INFO;
  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L    = num_sdus*2;
  last_size=2;
  }
  else {
  ((SCH_SUBHEADER_LONG *)mac_header_ptr)->R    = 0;
  ((SCH_SUBHEADER_LONG *)mac_header_ptr)->E    = 0;
  ((SCH_SUBHEADER_LONG *)mac_header_ptr)->F    = 1;
  ((SCH_SUBHEADER_LONG *)mac_header_ptr)->LCID = MCH_SCHDL_INFO;
  ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L    = (num_sdus*2)&0x7fff;
  last_size=3;
  }
  // Create the MSI MAC Control Element here
  }

  // SUBHEADER for MAC SDU (MCCH+MTCHs)
  for (i=0;i<num_sdus;i++) {
  if (first_element>0) {
  mac_header_ptr->E = 1;
  mac_header_ptr+=last_size;
  }
  else {
  first_element = 1;
  }
  if (sdu_lengths[i] < 128) {
  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->R    = 0;
  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->E    = 0;
  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F    = 0;
  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->LCID = sdu_lcids[i];
  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L    = (unsigned char)sdu_lengths[i];
  last_size=2;
  }
  else {
  ((SCH_SUBHEADER_LONG *)mac_header_ptr)->R    = 0;
  ((SCH_SUBHEADER_LONG *)mac_header_ptr)->E    = 0;
  ((SCH_SUBHEADER_LONG *)mac_header_ptr)->F    = 1;
  ((SCH_SUBHEADER_LONG *)mac_header_ptr)->LCID = sdu_lcids[i];
  ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L    = (unsigned short) sdu_lengths[i]&0x7fff;
  last_size=3;
  }
  }

  if (post_padding>0) {// we have lots of padding at the end of the packet
  mac_header_ptr->E = 1;
  mac_header_ptr+=last_size;
  // add a padding element
  mac_header_ptr->R    = 0;
  mac_header_ptr->E    = 0;
  mac_header_ptr->LCID = SHORT_PADDING;
  mac_header_ptr++;
  }
  else { // no end of packet padding
  // last SDU subhead is of fixed type (sdu length implicitly to be computed at UE)
  mac_header_ptr++;
  }

  // Copy MSI Control Element to the end of the MAC Header if it presents
  if ((ce_ptr-mac_header_control_elements) > 0) {
  // printf("Copying %d bytes for control elements\n",ce_ptr-mac_header_control_elements);
  memcpy((void*)mac_header_ptr,mac_header_control_elements,ce_ptr-mac_header_control_elements);
  mac_header_ptr+=(unsigned char)(ce_ptr-mac_header_control_elements);
  }

  return((unsigned char*)mac_header_ptr - mac_header);
  }
  #endif
 */
void add_common_dci(DCI_PDU *DCI_pdu,
    void *pdu,
    rnti_t rnti,
    unsigned char dci_size_bytes,
    unsigned char aggregation,
    unsigned char dci_size_bits,
    unsigned char dci_fmt,
    uint8_t ra_flag) {

  memcpy(&DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].dci_pdu[0],pdu,dci_size_bytes);
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].dci_length = dci_size_bits;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].L          = aggregation;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].rnti       = rnti;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].format     = dci_fmt;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].ra_flag    = ra_flag;


  DCI_pdu->Num_common_dci++;
  LOG_D(MAC,"add common dci format %d for rnti %d \n",dci_fmt,rnti);
}

void add_ue_spec_dci(DCI_PDU *DCI_pdu,void *pdu,rnti_t rnti,unsigned char dci_size_bytes,unsigned char aggregation,unsigned char dci_size_bits,unsigned char dci_fmt,uint8_t ra_flag) {

  memcpy(&DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].dci_pdu[0],pdu,dci_size_bytes);
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].dci_length = dci_size_bits;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].L          = aggregation;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].rnti       = rnti;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].format     = dci_fmt;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].ra_flag    = ra_flag;

  DCI_pdu->Num_ue_spec_dci++;

  LOG_D(MAC,"add ue specific dci format %d for rnti %d \n",dci_fmt,rnti);
}





// This has to be updated to include BSR information
uint8_t UE_is_to_be_scheduled(module_id_t module_idP,int CC_id,uint8_t UE_id) {

  UE_TEMPLATE *UE_template = &eNB_mac_inst[module_idP].UE_list.UE_template[CC_id][UE_id];

  //  LOG_D(MAC,"[eNB %d][PUSCH] Frame %d subframeP %d Scheduling UE %d\n",module_idP,rnti,frameP,subframeP,
  //	UE_id);

  if ((UE_template->bsr_info[LCGID0]>0) ||
      (UE_template->bsr_info[LCGID1]>0) ||
      (UE_template->bsr_info[LCGID2]>0) ||
      (UE_template->bsr_info[LCGID3]>0) ||
      (UE_template->ul_SR>0)) // uplink scheduling request
    return(1);
  else
    return(0);
}




uint32_t allocate_prbs(int UE_id,unsigned char nb_rb, uint32_t *rballoc) {

  int i;
  uint32_t rballoc_dci=0;
  unsigned char nb_rb_alloc=0;

  for (i=0;i<(mac_xface->lte_frame_parms->N_RB_DL-2);i+=2) {
      if (((*rballoc>>i)&3)==0) {
          *rballoc |= (3<<i);
          rballoc_dci |= (1<<((12-i)>>1));
          nb_rb_alloc+=2;
      }
      if (nb_rb_alloc==nb_rb)
        return(rballoc_dci);
  }

  if ((mac_xface->lte_frame_parms->N_RB_DL&1)==1) {
      if ((*rballoc>>(mac_xface->lte_frame_parms->N_RB_DL-1)&1)==0) {
          *rballoc |= (1<<(mac_xface->lte_frame_parms->N_RB_DL-1));
          rballoc_dci |= 1;//(1<<(mac_xface->lte_frame_parms->N_RB_DL>>1));
      }
  }
  return(rballoc_dci);
}

int get_min_rb_unit(module_id_t module_id, uint8_t CC_id){
  
  int min_rb_unit=0;
  LTE_DL_FRAME_PARMS* frame_parms = mac_xface->get_lte_frame_parms(module_id,CC_id); 
  switch (frame_parms->N_RB_DL) {
  case 6: // 1.4 MHz
    min_rb_unit=1;
    break;
  case 25: // 5HMz
    min_rb_unit=2;
    break;
  case 50: // 10HMz
    min_rb_unit=3;
    break;
  case 100: // 20HMz
    min_rb_unit=4;
    break;
  default:
    min_rb_unit=2;
    LOG_W(MAC,"[eNB %d] N_DL_RB %d unknown for CC_id %d, setting min_rb_unit to 2\n", module_id, CC_id);
    break;
  }
  return min_rb_unit;
}

uint32_t allocate_prbs_sub(int nb_rb, uint8_t *rballoc) {

  int check=0;//check1=0,check2=0;
  uint32_t rballoc_dci=0;
  //uint8_t number_of_subbands=13;

  LOG_T(MAC,"*****Check1RBALLOC****: %d%d%d%d (nb_rb %d,N_RBG %d)\n",
      rballoc[3],rballoc[2],rballoc[1],rballoc[0],nb_rb,mac_xface->lte_frame_parms->N_RBG);
  while((nb_rb >0) && (check < mac_xface->lte_frame_parms->N_RBG)){
    //printf("rballoc[%d] %d\n",check,rballoc[check]);
      if(rballoc[check] == 1){
          rballoc_dci |= (1<<((mac_xface->lte_frame_parms->N_RBG-1)-check));
          switch (mac_xface->lte_frame_parms->N_RB_DL) {
          case 6:
            nb_rb--;
	    break;
          case 25:
            if ((check == mac_xface->lte_frame_parms->N_RBG-1))
              nb_rb--;
            else
              nb_rb-=2;
            break;
          case 50:
            if ((check == mac_xface->lte_frame_parms->N_RBG-1))
              nb_rb-=2;
            else
              nb_rb-=3;
            break;
          case 100:
            nb_rb-=4;
            break;
          }
      }
      //      printf("rb_alloc %x\n",rballoc_dci);
      check = check+1;
      //    check1 = check1+2;
  }
  // rballoc_dci = (rballoc_dci)&(0x1fff);
  LOG_T(MAC,"*********RBALLOC : %x\n",rballoc_dci);
  // exit(-1);
  return (rballoc_dci);
}


int get_nb_subband(void){
 
  int nb_sb=0;

  switch (mac_xface->lte_frame_parms->N_RB_DL) {
  case 6:
    nb_sb=0; 
    break;
  case 15:
    nb_sb = 4;  // sb_size =4
  case 25: 
    nb_sb = 7; // sb_size =4, 1 sb with 1PRB, 6 with 2 RBG, each has 2 PRBs
    break;
  case 50:    // sb_size =6
    nb_sb = 9;
    break;
  case 75:  // sb_size =8
    nb_sb = 10;
    break;
  case 100: // sb_size =8 , 1 sb with 1 RBG + 12 sb with 2RBG, each RBG has 4 PRBs
    nb_sb = 13;
    break;
  default:
    nb_sb=0;
    break;
  }
  return nb_sb;

}
