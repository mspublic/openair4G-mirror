      
/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2010 Eurecom

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
/*! \file pre_processor.c
 * \brief procedures related to UE
 * \author Ankit Bhamri
 * \date 2013
 * \version 0.1
 * @ingroup _mac

 */

#include "PHY/defs.h"
#include "PHY/extern.h"

#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log.h"
#include "UTIL/OPT/opt.h"
#include "OCG.h"
#include "OCG_extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"

#include "RRC/LITE/extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"


 
#define DEBUG_eNB_SCHEDULER 1
#define DEBUG_HEADER_PARSING 1
//#define DEBUG_PACKET_TRACE 1

//#define ICIC 0

/*
  #ifndef USER_MODE
  #define msg debug_msg
  #endif
*/

extern inline unsigned int taus(void);



// This function stores the downlink buffer for all the logical channels 
void store_dlsch_buffer (unsigned char Mod_id,
			 u32 frame,
			 unsigned char subframe){
  
  
  unsigned char next_ue;
  u16 rnti,i=0;
  LTE_eNB_UE_stats* eNB_UE_stats;
  mac_rlc_status_resp_t rlc_status;
  unsigned char UE_id,granted_UEs;  

  granted_UEs = find_dlgranted_UEs(Mod_id);

  for (UE_id=0;UE_id<granted_UEs;UE_id++){
    eNB_mac_inst[Mod_id].UE_template[UE_id].dl_buffer_total = 0;
    for(i=0;i< MAX_NUM_LCID; i++)
      eNB_mac_inst[Mod_id].UE_template[UE_id].dl_buffer_info[i]=0;
  }



  for (UE_id=0;UE_id<granted_UEs;UE_id++) {
    
    next_ue = UE_id;
    rnti = find_UE_RNTI(Mod_id,next_ue);
    if (rnti == 0)
      continue;
    
    for(i=0;i< MAX_NUM_LCID; i++){ // loop over all the logical channels
      
      rlc_status = mac_rlc_status_ind(Mod_id,frame,1,i+(NB_RB_MAX*next_ue),0 ); 
      eNB_mac_inst[Mod_id].UE_template[next_ue].dl_buffer_info[i] = rlc_status.bytes_in_buffer; //storing the dlsch buffer for each logical channel
      
      eNB_mac_inst[Mod_id].UE_template[next_ue].dl_buffer_total = eNB_mac_inst[Mod_id].UE_template[next_ue].dl_buffer_total + eNB_mac_inst[Mod_id].UE_template[next_ue].dl_buffer_info[i];//storing the total dlsch buffer
      
      
    }
  }
}


// This function returns the estimated number of RBs required by each UE for downlink scheduling
void assign_rbs_required (unsigned char Mod_id,
			  u32 frame,
			  unsigned char subframe,
			  u16 *nb_rbs_required){

 
  unsigned char next_ue,harq_pid=0,round=0;
  u16 rnti,TBS;
  LTE_eNB_UE_stats* eNB_UE_stats;
  mac_rlc_status_resp_t rlc_status;
  unsigned char UE_id,granted_UEs,i=0;  


  granted_UEs = find_dlgranted_UEs(Mod_id);

  for (UE_id=0;UE_id<granted_UEs;UE_id++){
    nb_rbs_required[UE_id] = 0; //initialization
  }



  for (UE_id=0;UE_id<granted_UEs;UE_id++) {
    
    next_ue = UE_id;
    rnti = find_UE_RNTI(Mod_id,next_ue);
    if (rnti == 0)
      continue;
    
    eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);
    //if(eNB_UE_stats == NULL)
    //continue;


    switch(eNB_UE_stats->DL_cqi[0])
      {
      case 0:
	eNB_UE_stats->dlsch_mcs1 = 0;
	break;
      case 1:
	eNB_UE_stats->dlsch_mcs1 = 0;
	break;
      case 2:
	eNB_UE_stats->dlsch_mcs1 = 0;
	break;	
      case 3:
	eNB_UE_stats->dlsch_mcs1 = 2;
	break;
      case 4:
	eNB_UE_stats->dlsch_mcs1 = 4;
	break;
      case 5:
	eNB_UE_stats->dlsch_mcs1 = 6;
	break;
      case 6:
	eNB_UE_stats->dlsch_mcs1 = 8;
	break;
      case 7:
	eNB_UE_stats->dlsch_mcs1 = 11;
	break;
      case 8:
	eNB_UE_stats->dlsch_mcs1 = 13;
	break;
      case 9:
	eNB_UE_stats->dlsch_mcs1 = 16;
	break;
      case 10:
	eNB_UE_stats->dlsch_mcs1 = 18;
	break;
      case 11:
	eNB_UE_stats->dlsch_mcs1 = 20;
	break;
      case 12:
	eNB_UE_stats->dlsch_mcs1 = 22;
	break;
      case 13:
	eNB_UE_stats->dlsch_mcs1 = 22;//25
	break;
      case 14:
	eNB_UE_stats->dlsch_mcs1 = 22;//27
	break;
      case 15:
	eNB_UE_stats->dlsch_mcs1 = 22;//28
	break;
      default:
	printf("Invalid CQI");
	exit(-1);
      }
   
    
    
    if ((mac_get_rrc_status(Mod_id,1,next_ue) < RRC_RECONFIGURED)){
      nb_rbs_required[next_ue] = mac_xface->lte_frame_parms->N_RB_DL;
      continue;
    }
    
    
    if (eNB_mac_inst[Mod_id].UE_template[next_ue].dl_buffer_total> 0) {
      
      if (eNB_UE_stats->dlsch_mcs1==0) nb_rbs_required[next_ue] = 4;  // don't let the TBS get too small
      else nb_rbs_required[next_ue] = 2;
      
      
      TBS = mac_xface->get_TBS(eNB_UE_stats->dlsch_mcs1,nb_rbs_required[next_ue]); 
      
      while (TBS < eNB_mac_inst[Mod_id].UE_template[next_ue].dl_buffer_total)  {
	nb_rbs_required[next_ue] += 2;  
	if (nb_rbs_required[next_ue]>mac_xface->lte_frame_parms->N_RB_DL) { 
	  TBS = mac_xface->get_TBS(eNB_UE_stats->dlsch_mcs1,mac_xface->lte_frame_parms->N_RB_DL);
	  nb_rbs_required[next_ue] = mac_xface->lte_frame_parms->N_RB_DL;// calculating required number of RBs for each UE
	  break;
	}
	TBS = mac_xface->get_TBS(eNB_UE_stats->dlsch_mcs1,nb_rbs_required[next_ue]);
      }
      
    }
    
  }
}



// This fuction sorts the UE in order their dlsch buffer and CQI
void sort_UEs (unsigned char Mod_id,
	       unsigned char subframe,
	       u16 *UE_id_sorted){
  
  unsigned char next_ue1,next_ue2,round1=0,round2=0,harq_pid1=0,harq_pid2=0;
  u16 UE_id,granted_UEs,i=0,ii=0,j=0,rnti1,rnti2;
  LTE_eNB_UE_stats* eNB_UE_stats1;
  LTE_eNB_UE_stats* eNB_UE_stats2;
  
  
  granted_UEs = find_dlgranted_UEs(Mod_id);
  
  
  for (UE_id=0;UE_id<granted_UEs;UE_id++) {
    UE_id_sorted[i] = UE_id;
    i++;
  }
  
  for(i=0; i < granted_UEs;i++){
    
    next_ue1 = UE_id_sorted[i];
    
    rnti1 = find_UE_RNTI(Mod_id,next_ue1);
    if(rnti1 == 0)
      continue;
    
    
    eNB_UE_stats1 = mac_xface->get_eNB_UE_stats(Mod_id,rnti1);
    
    mac_xface->get_ue_active_harq_pid(Mod_id,rnti1,subframe,&harq_pid1,&round1,0);
    
    
    for(ii=i+1;ii<granted_UEs;ii++){
      
      next_ue2 = UE_id_sorted[ii];
      
      rnti2 = find_UE_RNTI(Mod_id,next_ue2);
      if(rnti2 == 0)
	continue;
      
      eNB_UE_stats2 = mac_xface->get_eNB_UE_stats(Mod_id,rnti2);
      
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti2,subframe,&harq_pid2,&round2,0);
      
      if(round2 > round1){ 
	UE_id_sorted[i] = next_ue2;
	UE_id_sorted[ii] = next_ue1;
      }
      else if (round2 == round1){   
	for(j=0;j<MAX_NUM_LCID;j++){
	  
	  if(eNB_mac_inst[Mod_id].UE_template[next_ue1].dl_buffer_info[j] < eNB_mac_inst[Mod_id].UE_template[next_ue2].dl_buffer_info[j]){
	    UE_id_sorted[i] = next_ue2;
	    UE_id_sorted[ii] = next_ue1;
	    break;
	  }
	  else if((j == MAX_NUM_LCID-1))
	    {
	      if(eNB_UE_stats1->DL_cqi[0] < eNB_UE_stats2->DL_cqi[0]){
		UE_id_sorted[i] = next_ue2;
		UE_id_sorted[ii] = next_ue1;
	      }
	    }
	}
      }
    }
  }
}

	
// This function assigns pre-available RBS to each UE in specified sub-bands before scheduling is done
void dlsch_scheduler_pre_processor (unsigned char Mod_id,
				    u32 frame,
				    unsigned char subframe,
				    u8 *dl_pow_off,
				    u16 *pre_nb_available_rbs,
				    unsigned char rballoc_sub_UE[256][mac_xface->lte_frame_parms->N_RBGS]){

  unsigned char next_ue,next_ue1,next_ue2,rballoc_sub[mac_xface->lte_frame_parms->N_RBGS],harq_pid=0,harq_pid1=0,harq_pid2=0,round=0,round1=0,round2=0,total_ue_count=0;
  unsigned char MIMO_mode_indicator[mac_xface->lte_frame_parms->N_RBGS];
  u16 UE_id,UE_id_sorted[256],granted_UEs,i,ii,j,nb_rbs_required[256],nb_rbs_required_remaining[256],nb_rbs_required_remaining_1[256],i1,i2,i3,r1=0,average_rbs_per_user=0;
  u16 rnti,rnti1,rnti2;
  LTE_eNB_UE_stats* eNB_UE_stats1;
  LTE_eNB_UE_stats* eNB_UE_stats2;
  

  granted_UEs = find_dlgranted_UEs(Mod_id);


  for(i=0;i<256;i++){
    nb_rbs_required[i] = 0;
    UE_id_sorted[i] = i;
    dl_pow_off[i]  =2;
    pre_nb_available_rbs[i] = 0;
    nb_rbs_required_remaining[i] = 0;
    for(j=0;j<mac_xface->lte_frame_parms->N_RBGS;j++)
      {
	MIMO_mode_indicator[j] = 2;
	rballoc_sub[j] = 0;
	rballoc_sub_UE[i][j] = 0;
      }
  }
  
  //printf("SUCCESS %d",mac_xface->lte_frame_parms->N_RBGS);
  //exit(-1);
  // Store the DLSCH buffer for each logical channel
  store_dlsch_buffer (Mod_id,frame,subframe);


  // Calculate the number of RBs required by each UE on the basis of logical channel's buffer
  assign_rbs_required (Mod_id,frame,subframe,nb_rbs_required);

  // Sorts the user on the basis of dlsch logical channel buffer and CQI
  sort_UEs (Mod_id,subframe,UE_id_sorted);
  //printf ("Frame:%d,SUCCESS %d[%d] %d[%d]\n",frame,UE_id_sorted[0],nb_rbs_required[UE_id_sorted[0]],UE_id_sorted[1],nb_rbs_required[UE_id_sorted[1]]);
  
  for (i=0;i<granted_UEs;i++){
    rnti = find_UE_RNTI(Mod_id,i);
    if(rnti == 0)
      continue;
    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);
    if(round>0)
      nb_rbs_required[i] = eNB_mac_inst[Mod_id].UE_template[i].nb_rb[harq_pid];
    //nb_rbs_required_remaining[i] = nb_rbs_required[i];
    if(nb_rbs_required[i] > 0)
      total_ue_count = total_ue_count + 1;
  }

  if(total_ue_count > 0)
    average_rbs_per_user = mac_xface->lte_frame_parms->N_RB_DL/total_ue_count;

  for(i=0;i<granted_UEs;i++){
    if (mac_get_rrc_status(Mod_id,1,i) < RRC_RECONFIGURED)
      nb_rbs_required_remaining_1[i] = nb_rbs_required[i];
    else
      nb_rbs_required_remaining_1[i] = cmin(average_rbs_per_user,nb_rbs_required[i]);
  }
  

  

  for(r1=0;r1<2;r1++){ //Allocation to UEs is done in 2 rounds,1st round:average number of RBs allocated to each UE, 2nd round: remaining RBs are allocated to high priority UEs

    for(i=0; i<granted_UEs;i++)
      {
	if(r1 == 0)
	  nb_rbs_required_remaining[i] = nb_rbs_required_remaining_1[i];
	else
	  nb_rbs_required_remaining[i] = nb_rbs_required[i]-nb_rbs_required_remaining_1[i]+nb_rbs_required_remaining[i];
      }
    
    for (i = 0 ;i<granted_UEs; i++){
   
      next_ue = UE_id_sorted[i];
      rnti = find_UE_RNTI(Mod_id,next_ue);
      if(rnti == 0)
	continue;
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);

      if ((mac_get_rrc_status(Mod_id,1,next_ue) < RRC_RECONFIGURED) && (round >0)) {



	for(j=0;j<mac_xface->lte_frame_parms->N_RBGS;j++){

	  if((rballoc_sub[j] == 0) && (rballoc_sub_UE[next_ue][j] == 0) && (nb_rbs_required_remaining[next_ue]>0)){

	    rballoc_sub[j] = 1;
	    rballoc_sub_UE[next_ue][j] = 1;

	    MIMO_mode_indicator[j] = 1;

	    if(mac_xface->get_transmission_mode(Mod_id,rnti)==5)
	      dl_pow_off[next_ue] = 1;

	    if((j == mac_xface->lte_frame_parms->N_RBGS-1) && (mac_xface->lte_frame_parms->N_RB_DL%2 == 1)){
	      nb_rbs_required_remaining[next_ue] = nb_rbs_required_remaining[next_ue] - 1;
	      pre_nb_available_rbs[next_ue] = pre_nb_available_rbs[next_ue] + 1;
	    }
	    else
	      {
		nb_rbs_required_remaining[next_ue] = nb_rbs_required_remaining[next_ue] - 2;
		pre_nb_available_rbs[next_ue] = pre_nb_available_rbs[next_ue] + 2;
	      }
	  }
	}
      }
    }




    for (i = 0 ;i<granted_UEs; i++){
    
      next_ue = UE_id_sorted[i];
      rnti = find_UE_RNTI(Mod_id,next_ue);
      if(rnti == 0)
	continue;
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);

      if ((mac_get_rrc_status(Mod_id,1,next_ue) >= RRC_RECONFIGURED) && (round > 0)) {



	for(j=0;j<mac_xface->lte_frame_parms->N_RBGS;j++){

	  if((rballoc_sub[j] == 0) && (rballoc_sub_UE[next_ue][j] == 0) && (nb_rbs_required_remaining[next_ue]>0)){

	    rballoc_sub[j] = 1;
	    rballoc_sub_UE[next_ue][j] = 1;

	    MIMO_mode_indicator[j] = 1;

	    if(mac_xface->get_transmission_mode(Mod_id,rnti)==5)
	      dl_pow_off[next_ue] = 1;

	    if((j == mac_xface->lte_frame_parms->N_RBGS-1) && (mac_xface->lte_frame_parms->N_RB_DL%2 == 1)){
	      nb_rbs_required_remaining[next_ue] = nb_rbs_required_remaining[next_ue] - 1;
	      pre_nb_available_rbs[next_ue] = pre_nb_available_rbs[next_ue] + 1;
	    }
	    else
	      {
		nb_rbs_required_remaining[next_ue] = nb_rbs_required_remaining[next_ue] - 2;
		pre_nb_available_rbs[next_ue] = pre_nb_available_rbs[next_ue] + 2;
	      }
	  }
	}
      }
    }



    for (i = 0 ;i<granted_UEs; i++){
    
      next_ue = UE_id_sorted[i];
      rnti = find_UE_RNTI(Mod_id,next_ue);
      if(rnti == 0)
	continue;
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);

      if ((mac_get_rrc_status(Mod_id,1,next_ue) < RRC_RECONFIGURED) && (round == 0)) {

      


	for(j=0;j<mac_xface->lte_frame_parms->N_RBGS;j++){

	  if((rballoc_sub[j] == 0) && (rballoc_sub_UE[next_ue][j] == 0) && (nb_rbs_required_remaining[next_ue]>0)){

	    rballoc_sub[j] = 1;
	    rballoc_sub_UE[next_ue][j] = 1;

	    MIMO_mode_indicator[j] = 1;

	    if(mac_xface->get_transmission_mode(Mod_id,rnti)==5)
	      dl_pow_off[next_ue] = 1;

	    if((j == mac_xface->lte_frame_parms->N_RBGS-1) && (mac_xface->lte_frame_parms->N_RB_DL%2 == 1)){
	      nb_rbs_required_remaining[next_ue] = nb_rbs_required_remaining[next_ue] - 1;
	      pre_nb_available_rbs[next_ue] = pre_nb_available_rbs[next_ue] + 1;
	    }
	    else
	      {
		nb_rbs_required_remaining[next_ue] = nb_rbs_required_remaining[next_ue] - 2;
		pre_nb_available_rbs[next_ue] = pre_nb_available_rbs[next_ue] + 2;
	      }
	  }
	}
      }
    }

  


    for (i = 0 ;i<granted_UEs; i++){
    
     
      next_ue1 = UE_id_sorted[i];
      rnti1 = find_UE_RNTI(Mod_id,next_ue1);
      if(rnti1 == 0)
	continue;

      eNB_UE_stats1 = mac_xface->get_eNB_UE_stats(Mod_id,rnti1);

      mac_xface->get_ue_active_harq_pid(Mod_id,rnti1,subframe,&harq_pid1,&round1,0);

      if ((mac_get_rrc_status(Mod_id,1,next_ue1) >= RRC_RECONFIGURED) && (round1==0) && (mac_xface->get_transmission_mode(Mod_id,rnti1)==5) && (dl_pow_off[next_ue1] != 1)) {


	for(j=0;j<mac_xface->lte_frame_parms->N_RBGS;j+=2){
      
	  if((((j == (mac_xface->lte_frame_parms->N_RBGS-1))&& (rballoc_sub[j] == 0) && (rballoc_sub_UE[next_ue1][j] == 0)) || ((j < (mac_xface->lte_frame_parms->N_RBGS-1)) && (rballoc_sub[j+1] == 0) && (rballoc_sub_UE[next_ue1][j+1] == 0))) && (nb_rbs_required_remaining[next_ue1]>0)){

	    for (ii = i+1;ii < granted_UEs;ii++) {
	      
	      next_ue2 = UE_id_sorted[ii];
	      rnti2 = find_UE_RNTI(Mod_id,next_ue2);
	      if(rnti2 == 0)
		continue;

	      eNB_UE_stats2 = mac_xface->get_eNB_UE_stats(Mod_id,rnti2);
	      mac_xface->get_ue_active_harq_pid(Mod_id,rnti2,subframe,&harq_pid2,&round2,0);
	      
	      if ((mac_get_rrc_status(Mod_id,1,next_ue2) >= RRC_RECONFIGURED) && (round2==0) && (mac_xface->get_transmission_mode(Mod_id,rnti2)==5) && (dl_pow_off[next_ue2] != 1)) {

		if((((j == (mac_xface->lte_frame_parms->N_RBGS-1)) && (rballoc_sub_UE[next_ue2][j] == 0)) || ((j < (mac_xface->lte_frame_parms->N_RBGS-1)) && (rballoc_sub_UE[next_ue2][j+1] == 0))) && (nb_rbs_required_remaining[next_ue2]>0)){

		  if((((eNB_UE_stats2->DL_pmi_single^eNB_UE_stats1->DL_pmi_single)<<(14-j))&0xc000)== 0x4000){ //MU-MIMO only for 25 RBs configuration

		    rballoc_sub[j] = 1;
		    rballoc_sub_UE[next_ue1][j] = 1;
		    rballoc_sub_UE[next_ue2][j] = 1;
		    MIMO_mode_indicator[j] = 0;

		    if (j< mac_xface->lte_frame_parms->N_RBGS-1) {
		      rballoc_sub[j+1] = 1;
		      rballoc_sub_UE[next_ue1][j+1] = 1;
		      rballoc_sub_UE[next_ue2][j+1] = 1;
		      MIMO_mode_indicator[j+1] = 0;
		    }
		
		    dl_pow_off[next_ue1] = 0;
		    dl_pow_off[next_ue2] = 0;

		
		

		    if((j == mac_xface->lte_frame_parms->N_RBGS-1) && (mac_xface->lte_frame_parms->N_RB_DL%2 == 1)){
		      nb_rbs_required_remaining[next_ue1] = nb_rbs_required_remaining[next_ue1] - 1;
		      pre_nb_available_rbs[next_ue1] = pre_nb_available_rbs[next_ue1] + 1;
		      nb_rbs_required_remaining[next_ue2] = nb_rbs_required_remaining[next_ue2] - 1;
		      pre_nb_available_rbs[next_ue2] = pre_nb_available_rbs[next_ue2] + 1;
		    }
		    else
		      {
			nb_rbs_required_remaining[next_ue1] = nb_rbs_required_remaining[next_ue1] - 4;
			pre_nb_available_rbs[next_ue1] = pre_nb_available_rbs[next_ue1] + 4;
			nb_rbs_required_remaining[next_ue2] = nb_rbs_required_remaining[next_ue2] - 4;
			pre_nb_available_rbs[next_ue2] = pre_nb_available_rbs[next_ue2] + 4;
		      }
		    break;
		  }
		}
	      }
	    }
	  }
	}
      }
    }

    for (i = 0;i<granted_UEs; i++){
    
      next_ue = UE_id_sorted[i];
      rnti = find_UE_RNTI(Mod_id,next_ue);
      if (rnti == 0)
	continue;
    
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);
    
      if ((mac_get_rrc_status(Mod_id,1,next_ue) >= RRC_RECONFIGURED) && (round==0)) {

      
	for(j=0;j<mac_xface->lte_frame_parms->N_RBGS;j++){
	
	  if((rballoc_sub[j] == 0) && (rballoc_sub_UE[next_ue][j] == 0) && (nb_rbs_required_remaining[next_ue]>0)){	  
	  
	  
	    switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	    case 1:
	    case 2:
	    case 4:
	    case 6:
	      rballoc_sub[j] = 1;
	      rballoc_sub_UE[next_ue][j] = 1;
	    
	      MIMO_mode_indicator[j] = 1;
	    
	      if((j == mac_xface->lte_frame_parms->N_RBGS-1) && (mac_xface->lte_frame_parms->N_RB_DL%2 == 1)){
		nb_rbs_required_remaining[next_ue] = nb_rbs_required_remaining[next_ue] - 1;
		pre_nb_available_rbs[next_ue] = pre_nb_available_rbs[next_ue] + 1;
	      }
	      else
		{
		  nb_rbs_required_remaining[next_ue] = nb_rbs_required_remaining[next_ue] - 2;
		  pre_nb_available_rbs[next_ue] = pre_nb_available_rbs[next_ue] + 2;
		}
	    
	      break;
	    case 5:
	      if (dl_pow_off[next_ue] != 0){
	      
		dl_pow_off[next_ue] = 1;
	      
		rballoc_sub[j] = 1;
		rballoc_sub_UE[next_ue][j] = 1;
	      
		MIMO_mode_indicator[j] = 1;
	      
		if((j == mac_xface->lte_frame_parms->N_RBGS-1) && (mac_xface->lte_frame_parms->N_RB_DL%2 == 1)){
		  nb_rbs_required_remaining[next_ue] = nb_rbs_required_remaining[next_ue] - 1;
		  pre_nb_available_rbs[next_ue] = pre_nb_available_rbs[next_ue] + 1;
		}
		else
		  {
		    nb_rbs_required_remaining[next_ue] = nb_rbs_required_remaining[next_ue] - 2;
		    pre_nb_available_rbs[next_ue] = pre_nb_available_rbs[next_ue] + 2;
		  }
	      }
	      break;
	    default:
	      break;
	    }
	  }
	}
      }
    }
  }
  
  i1=0;
  i2=0;
  i3=0;
  for (j=0;j<mac_xface->lte_frame_parms->N_RBGS;j++){
    if(MIMO_mode_indicator[j] == 2)
      i1 = i1+1;
    else if(MIMO_mode_indicator[j] == 1)
      i2 = i2+1;
    else if(MIMO_mode_indicator[j] == 0)
      i3 = i3+1;
  }


  if((i1 < mac_xface->lte_frame_parms->N_RBGS) && (i2>0) && (i3==0))
    PHY_vars_eNB_g[Mod_id]->check_for_SUMIMO_transmissions = PHY_vars_eNB_g[Mod_id]->check_for_SUMIMO_transmissions + 1;
  
  if(i3 == mac_xface->lte_frame_parms->N_RBGS && i1==0 && i2==0)
    PHY_vars_eNB_g[Mod_id]->FULL_MUMIMO_transmissions = PHY_vars_eNB_g[Mod_id]->FULL_MUMIMO_transmissions + 1;

  if((i1 < mac_xface->lte_frame_parms->N_RBGS) && (i3 > 0))
    PHY_vars_eNB_g[Mod_id]->check_for_MUMIMO_transmissions = PHY_vars_eNB_g[Mod_id]->check_for_MUMIMO_transmissions + 1;

  PHY_vars_eNB_g[Mod_id]->check_for_total_transmissions = PHY_vars_eNB_g[Mod_id]->check_for_total_transmissions + 1;
  
  
  
  
  for(UE_id=0;UE_id<granted_UEs;UE_id++){
    //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].dl_pow_off = dl_pow_off[UE_id];
    LOG_D(PHY,"******************Scheduling Information for UE%d ************************\n",UE_id);
    LOG_D(PHY,"dl power offset UE%d = %d \n",UE_id,dl_pow_off[UE_id]);
    LOG_D(PHY,"***********RB Alloc for every subband for UE%d ***********\n",UE_id);
    for(j=0;j<mac_xface->lte_frame_parms->N_RBGS;j++){
      //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].rballoc_sub[i] = rballoc_sub_UE[UE_id][i];
      LOG_D(PHY,"RB Alloc for UE%d and Subband%d = %d\n",UE_id,j,rballoc_sub_UE[UE_id][j]);
    }
    //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].pre_nb_available_rbs = pre_nb_available_rbs[UE_id];
    LOG_D(PHY,"Total RBs allocated for UE%d = %d\n",UE_id,pre_nb_available_rbs[UE_id]);
  }
}

