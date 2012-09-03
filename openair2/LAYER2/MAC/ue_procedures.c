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
/*! \file ue_procedures.c
* \brief procedures related to UE
* \author Raymond Knopp, Navid Nikaein
* \date 2011
* \version 0.5
* @ingroup _mac

*/

#include "extern.h"
#include "defs.h"
#ifdef PHY_EMUL
#include "SIMULATION/PHY_EMULATION/impl_defs.h"
#else
#include "SCHED/defs.h"
#include "PHY/impl_defs_top.h"
#endif
#include "PHY_INTERFACE/defs.h"
#include "PHY_INTERFACE/extern.h"
#include "COMMON/mac_rrc_primitives.h"
#include "RRC/LITE/extern.h"
#include "UTIL/LOG/log.h"
#include "OCG.h"
#include "OCG_extern.h"
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif

#define DEBUG_HEADER_PARSING 1

/*
#ifndef USER_MODE
#define msg debug_msg
#endif
*/
mapping BSR_names[] = {
    {"NONE", 0},
    {"SHORT BSR", 1},
    {"TRUNCATED BSR", 2},
    {"LONG BSR", 3},
    {"PADDING BSR", 3},
    {NULL, -1}
};

extern inline unsigned int taus(void);


void ue_init_mac(){
  int i,j;
  for (i=0 ; i < NB_UE_INST; i++){
    // default values as deined in 36.331 sec 9.2.2
    LOG_I(MAC,"[UE%d] Applying default macMainConfig\n",i);
    LOG_D(MAC, "[MSC_NEW][FRAME 00000][MAC_UE][MOD %02d][]\n", i+NB_eNB_INST);

    //UE_mac_inst[Mod_id].scheduling_info.macConfig=NULL;
    UE_mac_inst[i].scheduling_info.retxBSR_Timer= MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf2560;
    UE_mac_inst[i].scheduling_info.periodicBSR_Timer=MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_infinity;
    UE_mac_inst[i].scheduling_info.periodicPHR_Timer = MAC_MainConfig__phr_Config__setup__periodicPHR_Timer_sf20;
    UE_mac_inst[i].scheduling_info.prohibitPHR_Timer = MAC_MainConfig__phr_Config__setup__prohibitPHR_Timer_sf20;
    UE_mac_inst[i].scheduling_info.PathlossChange_db = MAC_MainConfig__phr_Config__setup__dl_PathlossChange_dB1;
    UE_mac_inst[i].PHR_state = MAC_MainConfig__phr_Config_PR_setup;
    UE_mac_inst[i].scheduling_info.SR_COUNTER=0;
    UE_mac_inst[i].scheduling_info.sr_ProhibitTimer=0;
    UE_mac_inst[i].scheduling_info.sr_ProhibitTimer_Running=0;
    UE_mac_inst[i].scheduling_info.maxHARQ_Tx=MAC_MainConfig__ul_SCH_Config__maxHARQ_Tx_n5;
    UE_mac_inst[i].scheduling_info.ttiBundling=0;
    UE_mac_inst[i].scheduling_info.drx_config=NULL;
    UE_mac_inst[i].scheduling_info.phr_config=NULL;
    UE_mac_inst[i].scheduling_info.periodicBSR_SF  =  get_sf_periodicBSRTimer(UE_mac_inst[i].scheduling_info.periodicBSR_Timer);
    UE_mac_inst[i].scheduling_info.retxBSR_SF     =  get_sf_retxBSRTimer(UE_mac_inst[i].scheduling_info.retxBSR_Timer);
    UE_mac_inst[i].scheduling_info.periodicPHR_SF =  get_sf_perioidicPHR_Timer(UE_mac_inst[i].scheduling_info.periodicPHR_Timer);
    UE_mac_inst[i].scheduling_info.prohibitPHR_SF =  get_sf_prohibitPHR_Timer(UE_mac_inst[i].scheduling_info.prohibitPHR_Timer);
    UE_mac_inst[i].scheduling_info.PathlossChange_db =  get_db_dl_PathlossChange(UE_mac_inst[i].scheduling_info.PathlossChange);
 
    for (j=0; j < MAX_NUM_LCID; j++){
      LOG_D(MAC,"[UE%d] Applying default logical channel config for LCGID %d\n",i,j);
      UE_mac_inst[i].scheduling_info.Bj[j]=-1;
      UE_mac_inst[i].scheduling_info.bucket_size[j]=-1;
    }
  }
}

unsigned char *parse_header(unsigned char *mac_header,
			    unsigned char *num_ce,
			    unsigned char *num_sdu,
			    unsigned char *rx_ces,
			    unsigned char *rx_lcids,
			    unsigned short *rx_lengths,
			    unsigned short tb_length) {

  unsigned char not_done=1,num_ces=0,num_sdus=0,lcid;
  unsigned char *mac_header_ptr = mac_header;
  unsigned short length,ce_len=0;

  while (not_done==1) {

    if (((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E == 0) {
      //      printf("E=0\n");
      not_done = 0;
    }
    lcid = ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID;
    if (lcid < UE_CONT_RES) {
      //printf("[MAC][UE] header %x.%x.%x\n",mac_header_ptr[0],mac_header_ptr[1],mac_header_ptr[2]);
      if (not_done==0) {// last MAC SDU, length is implicit
	mac_header_ptr++;
	length = tb_length-(mac_header_ptr-mac_header)-ce_len;
      }
      else {
	if (((SCH_SUBHEADER_LONG *)mac_header_ptr)->F == 1) {
	  length = ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L;
	  mac_header_ptr += 3;
	}  else {	//if (((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F == 0) {
	  length = ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L;
	  mac_header_ptr += 2;
	}
      }
#ifdef DEBUG_HEADER_PARSING
      LOG_D(MAC,"[UE] sdu %d lcid %d length %d (offset now %d)\n",num_sdus,lcid,length,mac_header_ptr-mac_header);
#endif
      rx_lcids[num_sdus] = lcid;
      rx_lengths[num_sdus] = length;
      num_sdus++;
    }
    else {  // This is a control element subheader
      if (lcid == SHORT_PADDING) {
	mac_header_ptr++;
      }
      else {
	rx_ces[num_ces] = lcid;
	num_ces++;
	mac_header_ptr ++;
	if (lcid==TIMING_ADV_CMD)
	  ce_len++;
	else if (lcid==UE_CONT_RES)
	  ce_len+=6;
      }
#ifdef DEBUG_HEADER_PARSING
      LOG_D(MAC,"[UE] ce %d lcid %d (offset now %d)\n",num_ces,lcid,mac_header_ptr-mac_header);
#endif
    }
  }
  *num_ce = num_ces;
  *num_sdu = num_sdus;

  return(mac_header_ptr);
}

u32 ue_get_SR(u8 Mod_id,u32 frame,u8 eNB_id,u16 rnti, u8 subframe) {

  // no UL-SCH resources available for this tti && UE has a valid PUCCH resources for SR configuration for this tti
  //  int MGL=6;// measurement gap length in ms
  int MGRP=0; // measurement gap repition period in ms
  int gapOffset=-1;
  int T=0;
  //  int sfn=0;

  // determin the measurement gap
  /*LOG_D(MAC,"[UE %d][SR %x] Frame %d subframe %d PHY asks for SR (SR_COUNTER/dsr_TransMax %d/%d), SR_pending %d\n",
	Mod_id,rnti,frame,subframe,
	UE_mac_inst[Mod_id].scheduling_info.SR_COUNTER,
	(1<<(2+UE_mac_inst[Mod_id].physicalConfigDedicated->schedulingRequestConfig->choice.setup.dsr_TransMax)),
	UE_mac_inst[Mod_id].scheduling_info.SR_pending);
  */
  if (UE_mac_inst[Mod_id].measGapConfig !=NULL){
    if (UE_mac_inst[Mod_id].measGapConfig->choice.setup.gapOffset.present == MeasGapConfig__setup__gapOffset_PR_gp0){
      MGRP= 40;
      gapOffset= UE_mac_inst[Mod_id].measGapConfig->choice.setup.gapOffset.choice.gp0;
    }else if (UE_mac_inst[Mod_id].measGapConfig->choice.setup.gapOffset.present == MeasGapConfig__setup__gapOffset_PR_gp1){
      MGRP= 80;
      gapOffset= UE_mac_inst[Mod_id].measGapConfig->choice.setup.gapOffset.choice.gp1;
    }else{
      LOG_W(MAC, "Measurement GAP offset is unknown\n");
    }
    T=MGRP/10;
    //check the measurement gap and sr prohibit timer
    if ((subframe ==  gapOffset %10) && ((frame %T) == (floor(gapOffset/10)))
	&& (UE_mac_inst[Mod_id].scheduling_info.sr_ProhibitTimer_Running =0)){
      UE_mac_inst[Mod_id].scheduling_info.SR_pending=1;
      return(0);
    }
  }
  if ((UE_mac_inst[Mod_id].scheduling_info.SR_pending==1) &&
      (UE_mac_inst[Mod_id].scheduling_info.SR_COUNTER <
       (1<<(2+UE_mac_inst[Mod_id].physicalConfigDedicated->schedulingRequestConfig->choice.setup.dsr_TransMax)))
      ){
    UE_mac_inst[Mod_id].scheduling_info.SR_COUNTER++;
    // start the sr-prohibittimer : rel 9 and above
    if (UE_mac_inst[Mod_id].scheduling_info.sr_ProhibitTimer > 0) { // timer configured
      UE_mac_inst[Mod_id].scheduling_info.sr_ProhibitTimer--;
      UE_mac_inst[Mod_id].scheduling_info.sr_ProhibitTimer_Running=1;
    } else
      UE_mac_inst[Mod_id].scheduling_info.sr_ProhibitTimer_Running=0;
    LOG_D(MAC,"[UE %d][SR %x] Frame %d subframe %d sned SR_indication (SR_COUNTER/dsr_TransMax %d/%d), SR_pending %d\n",
	Mod_id,rnti,frame,subframe,
	UE_mac_inst[Mod_id].scheduling_info.SR_COUNTER,
	(1<<(2+UE_mac_inst[Mod_id].physicalConfigDedicated->schedulingRequestConfig->choice.setup.dsr_TransMax)),
	UE_mac_inst[Mod_id].scheduling_info.SR_pending);
    return(1); //instruct phy to signal SR
  }
  else{
    // notify RRC to relase PUCCH/SRS
    // clear any configured dl/ul
    // initiate RA
    UE_mac_inst[Mod_id].scheduling_info.SR_pending=0;
    UE_mac_inst[Mod_id].scheduling_info.SR_COUNTER=0;
    return(0);
  }
}

void ue_send_sdu(u8 Mod_id,u32 frame,u8 *sdu,u16 sdu_len,u8 eNB_index) {

  unsigned char rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr;
  unsigned char rx_lcids[MAX_NUM_RB];
  unsigned short rx_lengths[MAX_NUM_RB];
  unsigned char *tx_sdu;

  printf("sdu: %x.%x.%x\n",sdu[0],sdu[1],sdu[2]);
  payload_ptr = parse_header(sdu,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths,sdu_len);

#ifdef DEBUG_HEADER_PARSING
  LOG_D(MAC,"[UE %d] ue_send_sdu : Frame %d eNB_index %d : num_ce %d num_sdu %d\n",Mod_id,
	frame,eNB_index,num_ce,num_sdu);
#endif
  /*
  msg("[MAC][eNB %d] First 32 bytes of DLSCH : \n");
  for (i=0;i<32;i++)
    msg("%x.",sdu[i]);
  msg("\n");  
  */
  for (i=0;i<num_ce;i++) {
    //    printf("ce %d : %d\n",i,rx_ces[i]);
      switch (rx_ces[i]) {
      case UE_CONT_RES:

	LOG_D(MAC,"[UE %d][RAPROC] Frame %d : received contention resolution msg: %x.%x.%x.%x.%x.%x, Terminating RA procedure\n",
	      Mod_id,frame,payload_ptr[0],payload_ptr[1],payload_ptr[2],payload_ptr[3],payload_ptr[4],payload_ptr[5]);
	if (UE_mac_inst[Mod_id].RA_active == 1) {
	  UE_mac_inst[Mod_id].RA_active=0;
	  // check if RA procedure has finished completely (no contention)
	  tx_sdu = &UE_mac_inst[Mod_id].CCCH_pdu.payload[1];//1=sizeof(SCH_SUBHEADER_FIXED);
	  for (i=0;i<6;i++)
	    if (tx_sdu[i] != payload_ptr[i]) {
	      LOG_D(MAC,"[UE %d][RAPROC] Contention detected, RA failed\n",Mod_id);
	      mac_xface->ra_failed(Mod_id,eNB_index);
	      return;
	    }
	  UE_mac_inst[Mod_id].RA_contention_resolution_timer_active = 0;
	}
	payload_ptr+=6;
	break;
      case TIMING_ADV_CMD:
#ifdef DEBUG_HEADER_PARSING
	LOG_D(MAC,"[UE] CE %d : UE Timing Advance : %d",i,payload_ptr[0]);
#endif
	process_timing_advance(Mod_id,payload_ptr[0]);
	payload_ptr++;
	break;
      case DRX_CMD:
#ifdef DEBUG_HEADER_PARSING
	LOG_D(MAC,"[UE] CE %d : UE DRX :",i);
#endif
	payload_ptr++;
	break;
      }
  }
  for (i=0;i<num_sdu;i++) {
#ifdef DEBUG_HEADER_PARSING
    LOG_D(MAC,"[UE] SDU %d : LCID %d, length %d\n",i,rx_lcids[i],rx_lengths[i]);
#endif
    if (rx_lcids[i] == CCCH) {

      LOG_D(MAC,"[UE %d] Frame %d : DLSCH -> DL-CCCH, RRC message (eNB %d, %d bytes)\n",Mod_id,frame, eNB_index, rx_lengths[i]);
      int j;
      for (j=0;j<rx_lengths[i];j++)
	msg("%x.",(unsigned char)payload_ptr[j]);
      msg("\n");

      mac_rrc_data_ind(Mod_id,
		       frame,
		       CCCH,
		       (char *)payload_ptr,rx_lengths[i],0,eNB_index);

    }
    else if (rx_lcids[i] == DCCH) {
      LOG_D(MAC,"[UE %d] Frame %d : DLSCH -> DL-DCCH%d, RRC message (eNB %d, %d bytes)\n", Mod_id, frame, rx_lcids[i],eNB_index,rx_lengths[i]);
      mac_rlc_data_ind(Mod_id+NB_eNB_INST,
		       frame,
		       0,
		       DCCH,
		       (char *)payload_ptr,
		       rx_lengths[i],
		       1,
		       NULL);
    }
    else if (rx_lcids[i] == DCCH1) {
      LOG_D(MAC,"[UE %d] Frame %d : DLSCH -> DL-DCCH%d, RRC message (eNB %d, %d bytes)\n", Mod_id, frame, rx_lcids[i], eNB_index,rx_lengths[i]);
	mac_rlc_data_ind(Mod_id+NB_eNB_INST,
		       frame,
		       0,
		       DCCH1,
		       (char *)payload_ptr,
		       rx_lengths[i],
		       1,
		       NULL);
    }
    else if (rx_lcids[i] == DTCH) {
      LOG_D(MAC,"[UE %d] Frame %d : DLSCH -> DL-DTCH%d (eNB %d, %d bytes)\n", Mod_id, frame,rx_lcids[i], eNB_index,rx_lengths[i]);
      mac_rlc_data_ind(Mod_id+NB_eNB_INST,
		       frame,
		       0,
		       DTCH,
		       (char *)payload_ptr,
		       rx_lengths[i],
		       1,
		       NULL);
    }

  }
}

void ue_decode_si(u8 Mod_id,u32 frame, u8 eNB_index, void *pdu,u16 len) {

  int i;
  
  //LOG_D(MAC,"[UE %d] Frame %d Sending SI to RRC (LCID Id %d)\n",Mod_id,frame,BCCH);

  mac_rrc_data_ind(Mod_id,
		   frame,
		   BCCH,
		   (char *)pdu,
		   len,
		   0,
		   eNB_index);

}


unsigned char generate_ulsch_header(u8 *mac_header,
				    u8 num_sdus,
				    u8 short_padding,
				    u16 *sdu_lengths,
				    u8 *sdu_lcids,
				    POWER_HEADROOM_CMD *power_headroom,
				    u16 *crnti,
				    BSR_SHORT *truncated_bsr,
				    BSR_SHORT *short_bsr,
				    BSR_LONG *long_bsr,
				    unsigned short post_padding) {

  SCH_SUBHEADER_FIXED *mac_header_ptr = (SCH_SUBHEADER_FIXED *)mac_header;
  unsigned char first_element=0,last_size=0,i;
  unsigned char mac_header_control_elements[16],*ce_ptr;

  LOG_D(MAC,"[UE] Generate ULSCH : num_sdus %d\n",num_sdus);
#ifdef DEBUG_HEADER_PARSING
  for (i=0;i<num_sdus;i++)
    LOG_T(MAC,"[UE] sdu %d : lcid %d length %d",i,sdu_lcids[i],sdu_lengths[i]);
  LOG_T(MAC,"\n");
#endif
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

  if (power_headroom) {
    if (first_element>0) {
      mac_header_ptr->E = 1;
      mac_header_ptr++;
    }
    else {
      first_element=1;
    }
    mac_header_ptr->R = 0;
    mac_header_ptr->E    = 0;
    mac_header_ptr->LCID = POWER_HEADROOM;
    last_size=1;
    *((POWER_HEADROOM_CMD *)ce_ptr)=(*power_headroom);
    ce_ptr+=sizeof(POWER_HEADROOM_CMD);
    //LOG_D(MAC, "phr header size %d\n",sizeof(POWER_HEADROOM_CMD));
  }

  if (crnti) {
#ifdef DEBUG_HEADER_PARSING
    LOG_D(MAC,"[MAC][UE] CRNTI : %x (first_element %d)\n",*crnti,first_element);
#endif
    if (first_element>0) {
      mac_header_ptr->E = 1;
      mac_header_ptr++;
    }
    else {
      first_element=1;
    }
    mac_header_ptr->R    = 0;
    mac_header_ptr->E    = 0;
    mac_header_ptr->LCID = CRNTI;
    last_size=1;
    *((u16 *)ce_ptr)=(*crnti);
    ce_ptr+=sizeof(u16);
    //    printf("offset %d\n",ce_ptr-mac_header_control_elements);
  }

  if (truncated_bsr) {
    if (first_element>0) {
      mac_header_ptr->E = 1;
      /*
      printf("last subheader : %x (R%d,E%d,LCID%d)\n",*(unsigned char*)mac_header_ptr,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->R,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID);
      */
      mac_header_ptr++;
    }
    else {
      first_element=1;
    }
#ifdef DEBUG_HEADER_PARSING
    LOG_D(MAC,"[UE] Scheduler Truncated BSR Header\n");
#endif
    mac_header_ptr->R = 0;
    mac_header_ptr->E    = 0;
    mac_header_ptr->LCID = TRUNCATED_BSR;
    last_size=1;
    *((BSR_TRUNCATED *)ce_ptr)=(*truncated_bsr);
    ce_ptr+=sizeof(BSR_TRUNCATED);

    //    printf("(cont_res) : offset %d\n",ce_ptr-mac_header_control_elements);
  }
  else if (short_bsr) {
    if (first_element>0) {
      mac_header_ptr->E = 1;
      /*
      printf("last subheader : %x (R%d,E%d,LCID%d)\n",*(unsigned char*)mac_header_ptr,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->R,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID);
      */
      mac_header_ptr++;
    }
    else {
      first_element=1;
    }
#ifdef DEBUG_HEADER_PARSING
    LOG_D(MAC,"[UE] Scheduler SHORT BSR Header\n");
#endif
    mac_header_ptr->R = 0;
    mac_header_ptr->E    = 0;
    mac_header_ptr->LCID = SHORT_BSR;
    last_size=1;
    *((BSR_SHORT *)ce_ptr)=(*short_bsr);
    ce_ptr+=sizeof(BSR_SHORT);

    //    printf("(cont_res) : offset %d\n",ce_ptr-mac_header_control_elements);
  }
  else if (long_bsr) {
    if (first_element>0) {
      mac_header_ptr->E = 1;
      /*
      printf("last subheader : %x (R%d,E%d,LCID%d)\n",*(unsigned char*)mac_header_ptr,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->R,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID);
      */
      mac_header_ptr++;
    }
    else {
      first_element=1;
    }
#ifdef DEBUG_HEADER_PARSING
    LOG_D(MAC,"[UE] Scheduler Long BSR Header\n");
#endif
    mac_header_ptr->R = 0;
    mac_header_ptr->E    = 0;
    mac_header_ptr->LCID = LONG_BSR;
    last_size=1;
    *((BSR_LONG *)ce_ptr)=(*long_bsr);
    ce_ptr+=sizeof(BSR_LONG);

    //    printf("(cont_res) : offset %d\n",ce_ptr-mac_header_control_elements);
  }
  //  printf("last_size %d,mac_header_ptr %p\n",last_size,mac_header_ptr);

  for (i=0;i<num_sdus;i++) {
#ifdef DEBUG_HEADER_PARSING
    LOG_T(MAC,"[UE] sdu subheader %d (lcid %d, %d bytes)\n",i,sdu_lcids[i],sdu_lengths[i]);
#endif
    if ((first_element>0)) {
      mac_header_ptr->E = 1;
#ifdef DEBUG_HEADER_PARSING
      LOG_D(MAC,"[UE] last subheader : %x (R%d,E%d,LCID%d)\n",*(unsigned char*)mac_header_ptr,
	  ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->R,
	  ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E,
	  ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID);
#endif
      mac_header_ptr+=last_size;
      //      printf("last_size %d,mac_header_ptr %p\n",last_size,mac_header_ptr);
    }
    else {
      first_element=1;

    }
     if  (sdu_lcids[i] == CCCH){
      ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->R    = 0; 
      ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E    = 0;
      ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID = sdu_lcids[i];
      last_size=1;
      }
      else if (sdu_lengths[i] < 128) {
      ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->R    = 0; // 3
      ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->E    = 0;
      ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F    = 0;
      ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->LCID = sdu_lcids[i];
      ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L    = (unsigned char)sdu_lengths[i];
      last_size=2;
#ifdef DEBUG_HEADER_PARSING
      LOG_D(MAC,"[UE] short sdu\n");
      LOG_T(MAC,"[UE] last subheader : %x (R%d,E%d,LCID%d,F%d,L%d)\n",
	  ((u16*)mac_header_ptr)[0],
	  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->R,
	  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->E,
	  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->LCID,
	  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F,
	  ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L);
#endif
    }
    else {
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->R    = 0;
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->E    = 0;
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->F    = 1;
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->LCID = sdu_lcids[i];
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L    = sdu_lengths[i]&0x7fff;

      last_size=3;
#ifdef DEBUG_HEADER_PARSING
      LOG_D(MAC,"[UE] long sdu\n");
#endif
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
    //mac_header_ptr=last_size; // FIXME: should be ++
  }
  
  if ((ce_ptr-mac_header_control_elements) > 0) {
    memcpy((void*)mac_header_ptr,mac_header_control_elements,ce_ptr-mac_header_control_elements);
    mac_header_ptr+=(unsigned char)(ce_ptr-mac_header_control_elements);
  }
#ifdef DEBUG_HEADER_PARSING
  LOG_T(MAC," [UE %d] header : ", crnti);
  for (i=0;i<((unsigned char*)mac_header_ptr - mac_header);i++)
    LOG_T(MAC,"%2x.",mac_header[i]);
  LOG_T(MAC,"\n");
#endif
  return((unsigned char*)mac_header_ptr - mac_header);

}

void ue_get_sdu(u8 Mod_id,u32 frame,u8 eNB_index,u8 *ulsch_buffer,u16 buflen) {

  mac_rlc_status_resp_t rlc_status;
  u8 dcch_header_len=0,dcch1_header_len=0,dtch_header_len=0;
  u8 dcch_header_len_tmp=0, dtch_header_len_tmp=0;
  u8 bsr_header_len=0, bsr_ce_len=0, bsr_len=0; 
  u8 phr_header_len=0, phr_ce_len=0,phr_len=0;
  u16 sdu_lengths[8];
  u8 sdu_lcids[8],payload_offset=0,num_sdus=0;
  u8 ulsch_buff[MAX_ULSCH_PAYLOAD_BYTES];
  u16 sdu_length_total=0;
  BSR_SHORT bsr_short;
  BSR_LONG bsr_long;
  BSR_SHORT *bsr_s=&bsr_short;
  BSR_LONG  *bsr_l=&bsr_long;
  POWER_HEADROOM_CMD phr;
  POWER_HEADROOM_CMD *phr_p=&phr;
  unsigned short short_padding=0, post_padding=0;
  int lcid;
  int j; // used for padding
  // Compute header length

  dcch_header_len=2;//sizeof(SCH_SUBHEADER_SHORT);
  dcch1_header_len=2;//sizeof(SCH_SUBHEADER_SHORT);
  // hypo length,in case of long header skip the padding byte
  dtch_header_len=(buflen > 128 ) ? 3 : 2 ; //sizeof(SCH_SUBHEADER_LONG)-1 : sizeof(SCH_SUBHEADER_SHORT);
  bsr_header_len = 1;//sizeof(SCH_SUBHEADER_FIXED);
  phr_header_len = 1;//sizeof(SCH_SUBHEADER_FIXED);
  bsr_ce_len = get_bsr_len (Mod_id, buflen);
  if (bsr_ce_len > 0 ){
    bsr_len = bsr_ce_len + bsr_header_len;
    LOG_D(MAC,"[UE %d] header size info: dcch %d, dcch1 %d, dtch %d, bsr (ce%d,hdr%d) buff_len %d\n",Mod_id, dcch_header_len,dcch1_header_len,dtch_header_len, bsr_ce_len, bsr_header_len, buflen);
  } else
    bsr_len = 0;
  phr_ce_len = (UE_mac_inst[Mod_id].PHR_reporting_active == 1) ? 1 /* sizeof(POWER_HEADROOM_CMD)*/: 0;
  if (phr_ce_len > 0){
    phr_len = phr_ce_len + phr_header_len;
    LOG_D(MAC,"[UE %d] header size info: PHR len %d (ce%d,hdr%d) buff_len %d\n",Mod_id, phr_len, phr_ce_len, phr_header_len, buflen);
  }else
    phr_len=0;
  
    // check for UL bandwidth requests and add SR control element

  // Check for DCCH first
  sdu_lengths[0]=0;
  if (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DCCH] > 0) {
    
    rlc_status = mac_rlc_status_ind(Mod_id+NB_eNB_INST,frame,
				    DCCH,
				    (buflen-dcch_header_len-bsr_len-phr_len));
    LOG_D(MAC,"[UE %d] Frame %d : UL-DCCH -> ULSCH, RRC message has %d bytes to send (Transport Block size %d, mac header len %d)\n",
	  Mod_id,frame, rlc_status.bytes_in_buffer,buflen,dcch_header_len);

    sdu_lengths[0] += mac_rlc_data_req(Mod_id+NB_eNB_INST,frame,
				       DCCH,
				       (char *)&ulsch_buff[sdu_lengths[0]]);

    sdu_length_total += sdu_lengths[0];
    sdu_lcids[0] = DCCH;
    LOG_D(MAC,"[UE %d] TX Got %d bytes for DCCH\n",Mod_id,sdu_lengths[0]);
    num_sdus = 1;
    update_bsr(Mod_id, frame, DCCH);
    //header_len +=2;
  }
  else {
    dcch_header_len=0;
    num_sdus = 0;
  }

  // DCCH1
  if (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DCCH1] > 0) {

    rlc_status = mac_rlc_status_ind(Mod_id+NB_eNB_INST,frame,
				    DCCH1,
				    (buflen-bsr_len-phr_len-dcch_header_len-dcch1_header_len-sdu_length_total));

    LOG_D(MAC,"[UE %d] Frame %d : UL-DCCH1 -> ULSCH, RRC message has %d bytes to send (Transport Block size %d, mac header len %d)\n",
	  Mod_id,frame, rlc_status.bytes_in_buffer,buflen,dcch1_header_len);

    sdu_lengths[num_sdus] = mac_rlc_data_req(Mod_id+NB_eNB_INST,frame,
					     DCCH1,
					     (char *)&ulsch_buff[sdu_lengths[0]]);
    sdu_length_total += sdu_lengths[num_sdus];
    sdu_lcids[num_sdus] = DCCH1;
    LOG_D(MAC,"[UE %d] TX Got %d bytes for DCCH1\n",Mod_id,sdu_lengths[num_sdus]);
    num_sdus++;
    update_bsr(Mod_id, frame, DCCH1);
    //dcch_header_len +=2; // include dcch1
  }
  else {
    dcch1_header_len =0;
  }

  // now check for other logical channels
  if ((UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DTCH] > 0) &&
      ((bsr_len+phr_len+dcch_header_len+dcch1_header_len+dtch_header_len+sdu_length_total) <= buflen)){

    // optimize the dtch header lenght
    //if ((UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DTCH] > 128) &&   
    /*   if (((UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DTCH] >= 128) &&
	((UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DTCH]+bsr_len+phr_len+dcch_header_len+dcch1_header_len+dtch_header_len) > buflen)&&
	 buflen >=128 ))
      dtch_header_len = 3;//sizeof(SCH_SUBHEADER_LONG);
    else 
      dtch_header_len = 2;//sizeof(SCH_SUBHEADER_SHORT);
    */
    rlc_status = mac_rlc_status_ind(Mod_id+NB_eNB_INST,frame,
				    DTCH,
				    buflen-bsr_len-phr_len-dcch_header_len-dcch1_header_len-dtch_header_len-sdu_length_total);

     LOG_D(MAC,"[UE %d] Frame %d : UL-DTCH -> ULSCH, %d bytes to send (Transport Block size %d, mac header len %d, BSR byte[DTCH] %d)\n",
	   Mod_id,frame, rlc_status.bytes_in_buffer,buflen,dtch_header_len,UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DTCH]);

    sdu_lengths[num_sdus] = mac_rlc_data_req(Mod_id+NB_eNB_INST,frame,
					     DTCH,
					     (char *)&ulsch_buff[sdu_length_total]);

    //adjust dtch header
    dtch_header_len = (sdu_lengths[num_sdus] >= 128) ? 3 : 2;
    LOG_D(MAC,"[UE %d] TX Got %d bytes for DTCH\n",Mod_id,sdu_lengths[num_sdus]);
    sdu_lcids[num_sdus] = DTCH;
    sdu_length_total += sdu_lengths[num_sdus];
    num_sdus++;
    update_bsr(Mod_id, frame, DTCH);
  }
  else { // no rlc pdu : generate the dummy header
    dtch_header_len = 0;
  }
  // regular BSR :  build bsr
  if (bsr_ce_len == sizeof(BSR_SHORT)) {
    bsr_l = NULL;
    lcid = UE_mac_inst[Mod_id].scheduling_info.BSR_short_lcid;
    bsr_s->LCGID = lcid;
    bsr_s->Buffer_size = UE_mac_inst[Mod_id].scheduling_info.BSR[lcid];
    LOG_D(MAC,"[UE %d] Frame %d report SHORT BSR with level %d for LCGID %d\n", 
	  Mod_id, frame, UE_mac_inst[Mod_id].scheduling_info.BSR[lcid],lcid);
  } else if (bsr_ce_len == sizeof(BSR_LONG))  {
    bsr_s = NULL;
    bsr_l->Buffer_size0 = UE_mac_inst[Mod_id].scheduling_info.BSR[CCCH];
    bsr_l->Buffer_size1 = UE_mac_inst[Mod_id].scheduling_info.BSR[DCCH];
    bsr_l->Buffer_size2 = UE_mac_inst[Mod_id].scheduling_info.BSR[DCCH1];
    bsr_l->Buffer_size3 = UE_mac_inst[Mod_id].scheduling_info.BSR[DTCH];
    LOG_D(MAC, "[UE %d] Frame %d report long BSR (level LCGID0 %d,level LCGID1 %d,level LCGID2 %d,level LCGID3 %d)\n", Mod_id,frame, 
	  UE_mac_inst[Mod_id].scheduling_info.BSR[CCCH],
	  UE_mac_inst[Mod_id].scheduling_info.BSR[DCCH],
	  UE_mac_inst[Mod_id].scheduling_info.BSR[DCCH1],
	  UE_mac_inst[Mod_id].scheduling_info.BSR[DTCH]);
  } else {
    bsr_s = NULL;
    bsr_l = NULL ;
  }
  // build PHR and update the timers 
  if (phr_ce_len == sizeof(POWER_HEADROOM_CMD)){
    phr_p->PH = get_phr_mapping(Mod_id,eNB_index);
    phr_p->R  = 0;
     LOG_D(MAC,"[UE %d] Frame %d report PHR with mapping (%d->%d) for LCGID %d\n", 
	   Mod_id,frame, mac_xface->get_PHR(Mod_id,eNB_index), phr_p->PH,POWER_HEADROOM);
     update_phr(Mod_id);
  }else
    phr_p=NULL;
 
  // adjust the header length 
  dcch_header_len_tmp = dcch_header_len;
  dtch_header_len_tmp = dtch_header_len;
  if (dtch_header_len==0)
    dcch_header_len = (dcch_header_len>0)? 1: dcch_header_len;  
  else 
    dtch_header_len= (dtch_header_len >0)? 1: dtch_header_len;   // for short and long, cut the length+F fields  
  
  if ((buflen-bsr_len-phr_len-dcch_header_len-dcch1_header_len-dtch_header_len-sdu_length_total) <= 2) {
    short_padding = buflen-bsr_len-phr_len-dcch_header_len-dcch1_header_len-dtch_header_len-sdu_length_total;
    post_padding = 0;
  }
  else {
    short_padding = 0;
    if (dtch_header_len==0)
      dcch_header_len = dcch_header_len_tmp;
    else 
      dtch_header_len= dtch_header_len_tmp;
    
    post_padding = buflen-bsr_len-phr_len-dcch_header_len-dcch1_header_len-dtch_header_len-sdu_length_total-1;
  }
 
  // Generate header
  // if (num_sdus>0) {

    payload_offset = generate_ulsch_header(ulsch_buffer,  // mac header
					   num_sdus,      // num sdus
					   short_padding,            // short pading
					   sdu_lengths,  // sdu length
					   sdu_lcids,    // sdu lcid
					   phr_p,  // power headroom
					   NULL,  // crnti
					   NULL,  // truncated bsr
					   bsr_s, // short bsr
					   bsr_l,
					   post_padding); // long_bsr
    LOG_D(MAC,"[UE %d] Generate header :bufflen %d  sdu_length_total %d, num_sdus %d, sdu_lengths[0] %d, sdu_lcids[0] %d => payload offset %d,  dcch_header_len %d, dtch_header_len %d, padding %d,post_padding %d, bsr len %d, phr len %d, reminder %d \n",
	  Mod_id,buflen, sdu_length_total,num_sdus,sdu_lengths[0],sdu_lcids[0],payload_offset, dcch_header_len,  dtch_header_len,
	  short_padding,post_padding, bsr_len, phr_len,buflen-sdu_length_total-payload_offset);
    // cycle through SDUs and place in ulsch_buffer
    memcpy(&ulsch_buffer[payload_offset],ulsch_buff,sdu_length_total);
    // fill remainder of DLSCH with random data
    for (j=0;j<(buflen-sdu_length_total-payload_offset);j++)
      ulsch_buffer[payload_offset+sdu_length_total+j] = (char)(taus()&0xff);
    /*  }
  else { // send BSR
    // bsr.LCGID = 0x0;
    //bsr.Buffer_size = 0x3f;

    payload_offset = generate_ulsch_header(ulsch_buffer,  // mac header
					   num_sdus,      // num sdus
					   short_padding,            // short pading
					   sdu_lengths,  // sdu length
					   sdu_lcids,    // sdu lcid
					   phr_p,  // power headroom
					   NULL,  // crnti
					   NULL,  // truncated bsr
					   bsr_s,
					   bsr_l,
					   post_padding);

    LOG_D(MAC,"[UE %d] Generate header : bufflen %d sdu_length_total %d, num_sdus %d, sdu_lengths[0] %d, sdu_lcids[0] %d => payload offset %d,padding %d,post_padding %d,  bsr len %d, phr len %d, sdu reminder %d bytes\n",
	  Mod_id,buflen, sdu_length_total,num_sdus,sdu_lengths[0],sdu_lcids[0],payload_offset,
	  short_padding,post_padding, bsr_len, phr_len, buflen-sdu_length_total-payload_offset);
    // cycle through SDUs and place in ulsch_buffer
    memcpy(&ulsch_buffer[payload_offset],ulsch_buff,sdu_length_total);
    // fill remainder of DLSCH with random data
    for (j=0;j<(buflen-sdu_length_total-payload_offset);j++)
      ulsch_buffer[payload_offset+sdu_length_total+j] = (char)(taus()&0xff);
  }
    */
#if defined(USER_MODE) && defined(OAI_EMU)
  if (oai_emulation.info.opt_enabled)
    trace_pdu(0, ulsch_buffer, buflen, Mod_id, 3, 
	      UE_mac_inst[Mod_id].crnti,frame,0,0);
  LOG_D(OPT,"[UE %d][ULSCH] Frame %d trace pdu for rnti %x  with size %d\n", 
	Mod_id, frame, UE_mac_inst[Mod_id].crnti, buflen);
#endif
  
    LOG_D(MAC,"[UE %d][SR] Gave SDU to PHY, clearing any scheduling request\n",
	  Mod_id,payload_offset, sdu_length_total);
    UE_mac_inst[Mod_id].scheduling_info.SR_pending=0;
    UE_mac_inst[Mod_id].scheduling_info.SR_COUNTER=0;
     
}

// called at each subframe
// Performs :
// 1. Trigger PDCP every 5ms
// 2. Call RRC for link status return to PHY
// 3. Perform SR/BSR procedures for scheduling feedback
// 4. Perform PHR procedures



UE_L2_STATE_t ue_scheduler(u8 Mod_id,u32 frame, u8 subframe, lte_subframe_t direction,u8 eNB_index) {

  int lcid; // lcid index

  int TTI= 1;
  int bucketsizeduration;
  int bucketsizeduration_max;
  mac_rlc_status_resp_t rlc_status[11];
  struct RACH_ConfigCommon *rach_ConfigCommon = (struct RACH_ConfigCommon *)NULL;
  
  //Mac_rlc_xface->frame=frame;
  //Rrc_xface->Frame_index=Mac_rlc_xface->frame;
  //if (subframe%5 == 0)
    pdcp_run(frame, 0, Mod_id, 0);  

#ifdef CELLULAR
  rrc_rx_tx(Mod_id, frame, 0, eNB_index);
#else
  switch (rrc_rx_tx(Mod_id,
		    frame,
		    0,
		    eNB_index)) {
  case RRC_OK:
    break;
  case RRC_ConnSetup_failed:
    LOG_D(MAC,"RRCConnectionSetup failed, returning to IDLE state\n");
    return(CONNECTION_LOST);
    break;
  case RRC_PHY_RESYNCH:
    LOG_D(MAC,"RRC Loss of synch, returning PHY_RESYNCH\n");
    return(PHY_RESYNCH);
  default:
    break;
  }
#endif 

  // Check Contention resolution timer (put in a function later)
  if (UE_mac_inst[Mod_id].RA_contention_resolution_timer_active == 1) {
    if (UE_mac_inst[Mod_id].radioResourceConfigCommon)
      rach_ConfigCommon = &UE_mac_inst[Mod_id].radioResourceConfigCommon->rach_ConfigCommon;
    else {
      LOG_D(MAC,"FATAL: radioResourceConfigCommon is NULL!!!\n");
      mac_xface->macphy_exit("");
      return(RRC_OK); // RRC_OK ???
    }
    UE_mac_inst[Mod_id].RA_contention_resolution_cnt++;
    if (UE_mac_inst[Mod_id].RA_contention_resolution_cnt ==
	((1+rach_ConfigCommon->ra_SupervisionInfo.mac_ContentionResolutionTimer)<<3)) {
      UE_mac_inst[Mod_id].RA_active = 0;
      // Signal PHY to quit RA procedure
      mac_xface->ra_failed(Mod_id,eNB_index);
      LOG_D(MAC,"Counter resolution timer expired, RA failed\n");
    }
  }


  // call SR procedure to generate pending SR and BSR for next PUCCH/PUSCH TxOp.  This should implement the procedures
  // outlined in Sections 5.4.4 an 5.4.5 of 36.321
  // Put this in another function

    // Get RLC status info and update Bj for all lcids that are active
  for (lcid=CCCH; lcid <= DTCH; lcid++ ) {
    if ((lcid == 0) ||
	(UE_mac_inst[Mod_id].logicalChannelConfig[lcid])) {
      // meausre the Bj
      if ((direction == SF_UL)&& (UE_mac_inst[Mod_id].scheduling_info.Bj[lcid] >= 0)){
	if (UE_mac_inst[Mod_id].logicalChannelConfig[lcid]->ul_SpecificParameters) {
	  bucketsizeduration = UE_mac_inst[Mod_id].logicalChannelConfig[lcid]->ul_SpecificParameters->prioritisedBitRate * TTI;
	  bucketsizeduration_max = get_ms_bucketsizeduration(UE_mac_inst[Mod_id].logicalChannelConfig[lcid]->ul_SpecificParameters->bucketSizeDuration);
	}
	else {
	  LOG_E(MAC,"[UE %d] lcid %d, NULL ul_SpecificParameters\n",Mod_id,lcid);
	  mac_xface->macphy_exit("");
	}
	if ( UE_mac_inst[Mod_id].scheduling_info.Bj[lcid] > bucketsizeduration_max )
	  UE_mac_inst[Mod_id].scheduling_info.Bj[lcid] = bucketsizeduration_max;
	else
	  UE_mac_inst[Mod_id].scheduling_info.Bj[lcid] = bucketsizeduration;
      }
      // measure the buffer size
      rlc_status[lcid] = mac_rlc_status_ind(Mod_id+NB_eNB_INST,frame,
					    lcid,
					    0);//tb_size does not reauire when requesting the status
      //      LOG_D(MAC,"[UE %d] frame %d rlc buffer (lcid %d, byte %d)BSR level %d\n",
      //	    Mod_id, frame, lcid, rlc_status[lcid].bytes_in_buffer, UE_mac_inst[Mod_id].scheduling_info.BSR[lcid]);
      if (rlc_status[lcid].bytes_in_buffer > 0 ) {
	//set the bsr for all lcid by searching the table to find the bsr level
	UE_mac_inst[Mod_id].scheduling_info.BSR[lcid] = locate (BSR_TABLE,BSR_TABLE_SIZE, rlc_status[lcid].bytes_in_buffer);
	//	UE_mac_inst[Mod_id].scheduling_info.num_BSR++;
	UE_mac_inst[Mod_id].scheduling_info.SR_pending=1;
	LOG_D(MAC,"[MAC][UE %d][SR] Frame %d subframe %d SR for PUSCH is pending for LCGID %d with BSR level %d (%d bytes in RLC)\n",
	      Mod_id, frame,subframe,lcid,
	      UE_mac_inst[Mod_id].scheduling_info.BSR[lcid],
	      rlc_status[lcid].bytes_in_buffer);
      } else {
	UE_mac_inst[Mod_id].scheduling_info.BSR[lcid]=0;
      }
    }
    update_bsr(Mod_id, frame, lcid);
  }

  // UE has no valid phy config dedicated ||  no valid/released  SR
  if ((UE_mac_inst[Mod_id].physicalConfigDedicated == NULL)) {
    // cancel all pending SRs
    UE_mac_inst[Mod_id].scheduling_info.SR_pending=0;
    //    LOG_D(MAC,"[UE %d] Release all SRs \n", Mod_id);
    return(CONNECTION_OK);
  }

  if ((UE_mac_inst[Mod_id].physicalConfigDedicated->schedulingRequestConfig == NULL) ||
      (UE_mac_inst[Mod_id].physicalConfigDedicated->schedulingRequestConfig->present == SchedulingRequestConfig_PR_release)){

    // initiate RA with CRNTI included in msg3 (no contention) as descibed in 36.321 sec 5.1.5

    // cancel all pending SRs
    UE_mac_inst[Mod_id].scheduling_info.SR_pending=0;
    LOG_D(MAC,"[MAC][UE %d] Release all SRs \n", Mod_id);
  }

  // Put this in a function
  // Call PHR procedure as described in Section 5.4.6 in 36.321 
  if (UE_mac_inst[Mod_id].PHR_state == MAC_MainConfig__phr_Config_PR_setup ){ // normal operation 
    if (UE_mac_inst[Mod_id].PHR_reconfigured == 1){ // upon (re)configuration of the power headroom reporting functionality by upper layers 
      UE_mac_inst[Mod_id].PHR_reporting_active =1;
      UE_mac_inst[Mod_id].PHR_reconfigured = 0;
    } else { 
      //LOG_D(MAC,"PHR normal operation %d active %d \n", UE_mac_inst[Mod_id].scheduling_info.periodicPHR_SF, UE_mac_inst[Mod_id].PHR_reporting_active);
      if ((UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_SF <= 0) && 
	  ((mac_xface->get_PL(Mod_id,eNB_index) <  UE_mac_inst[Mod_id].scheduling_info.PathlossChange_db ) || 
	   (UE_mac_inst[Mod_id].power_backoff_db[eNB_index] > UE_mac_inst[Mod_id].scheduling_info.PathlossChange_db)))
	// trigger PHR and reset the timer later when the PHR report is sent
	UE_mac_inst[Mod_id].PHR_reporting_active =1;
      else if (UE_mac_inst[Mod_id].PHR_reporting_active ==0 )
	UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_SF--;
     
      if (UE_mac_inst[Mod_id].scheduling_info.periodicPHR_SF <= 0 )
	// trigger PHR and reset the timer later when the PHR report is sent
	UE_mac_inst[Mod_id].PHR_reporting_active =1;
      else if (UE_mac_inst[Mod_id].PHR_reporting_active == 0 )
	UE_mac_inst[Mod_id].scheduling_info.periodicPHR_SF--; 
    }
  } else {    // release / nothing
    UE_mac_inst[Mod_id].PHR_reporting_active =0; // release PHR
  }
  //If the UE has UL resources allocated for new transmission for this TTI here:

  return(CONNECTION_OK);
}

u8 get_bsr_len (u8 Mod_id, u16 buflen) {

  int lcid;
  u8 bsr_len=0, bsr_channel=0;
  int pdu = 0;

  bsr_channel = 0;
  bsr_len =0;
  pdu=0;
  for (lcid=DCCH; lcid <= DTCH; lcid++ ) { // dcch, dcch1, dtch
    //LOG_D(MAC,"BSR Bytes%d for lcid %d\n", UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid], lcid);
    if (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid] > 0 )
      pdu += (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid] +  bsr_len + 2); //2 = sizeof(SCH_SUBHEADER_SHORT)
    if (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid] > 128 ) // long header size: adjust the header size
      pdu += 1;
    // current phy buff can not transport all sdu for this lcid -> transmit a bsr for this lcid

    if ( (pdu > buflen) &&  (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid] > 0 ) ){
      bsr_channel+=1;
      bsr_len = ((bsr_channel > 1 ) ? sizeof(BSR_LONG) :  sizeof(BSR_SHORT)) ;
    }
  }
  if ( bsr_len > 0 )
    LOG_D(MAC,"[UE %d] Prepare a %s (Transport Block Size %d, MAC pdu Size %d) \n", 
	  Mod_id, map_int_to_str(BSR_names, bsr_len), buflen, pdu);
  return bsr_len;
}


void update_bsr(u8 Mod_id, u32 frame, u8 lcid){

  mac_rlc_status_resp_t rlc_status;

  rlc_status = mac_rlc_status_ind(Mod_id+NB_eNB_INST,frame,
				  lcid,
				  0);//tb_size does not require when requesting the status
  if (rlc_status.bytes_in_buffer > 0 ) {
    UE_mac_inst[Mod_id].scheduling_info.BSR[lcid] = locate (BSR_TABLE,BSR_TABLE_SIZE, rlc_status.bytes_in_buffer);
    UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid] = rlc_status.bytes_in_buffer;
    UE_mac_inst[Mod_id].scheduling_info.BSR_short_lcid = lcid; // only applicable to short bsr
    LOG_D(MAC,"[UE %d] BSR level %d (LCGID %d, rlc buffer %d byte)\n",
	  Mod_id, UE_mac_inst[Mod_id].scheduling_info.BSR[lcid], lcid, rlc_status.bytes_in_buffer);
  } else {
    //LOG_D(MAC,"[UE %d] clear BSR info for lcid %d\n",Mod_id, lcid);
    UE_mac_inst[Mod_id].scheduling_info.BSR[lcid]=0;
    UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid]=0;
    UE_mac_inst[Mod_id].scheduling_info.BSR_short_lcid = 0;
  }

}

u8 locate (const u32 *table, int size, int value){

  u8 ju, jm, jl;
  int ascend;

  if (value == 0) return 0; //elseif (value > 150000) return 63;

  jl = 0;      // lower bound
  ju = size  ;// upper bound
  ascend = (table[ju] >= table[jl]) ? 1 : 0; // determine the order of the the table:  1 if ascending order of table, 0 otherwise

  while (ju-jl > 1) { //If we are not yet done,
    jm = (ju+jl) >> 1; //compute a midpoint,
    if ((value >= table[jm]) == ascend)
      jl=jm; // replace the lower limit
    else
      ju=jm; //replace the upper limit
    LOG_T(MAC,"[UE] searching BSR index %d for (BSR TABLE %d < value %d)\n", jm, table[jm], value);
  }
  if (value == table[jl]) return jl;
  else                    return jl+1; //equally  ju

}

int get_sf_periodicBSRTimer(u8 sf_offset){

  switch (sf_offset) {
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf5:
    return 5;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf10:
    return 10;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf16:
    return 16;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf20:
    return 20;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf32:
    return 32;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf40:
    return 40;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf64:
    return 64;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf80:
    return 80;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf128:
    return 128;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf160:
    return 160;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf320:
    return 320;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf640:
    return 640;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf1280:
    return 1280;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf2560:
    return 2560;
    break;
  case MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_infinity:
  default:
    return -1;
    break;
  }
}

int get_sf_retxBSRTimer(u8 sf_offset){

  switch (sf_offset) {
  case MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf320:
    return 320;
    break;
  case MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf640:
    return 640;
    break;
  case MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf1280:
    return 1280;
    break;
  case MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf2560:
    return 2560;
    break;
  case MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf5120:
    return 5120;
    break;
  case MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf10240:
    return 10240;
    break;
  default:
    return -1;
    break;
  }
}
int get_ms_bucketsizeduration(u8 bucketsizeduration){

  switch (bucketsizeduration) {
  case LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50:
    return 50;
    break;
  case LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms100:
    return 100;
    break;
  case LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms150:
    return 150;
    break;
  case LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms300:
    return 300;
    break;
  case LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms500:
    return 500;
    break;
  case LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms1000:
    return 1000;
    break;
  default:
    return 0;
    break;
  }
}

void update_phr(u8 Mod_id){
  
  UE_mac_inst[Mod_id].PHR_reporting_active =0;
  UE_mac_inst[Mod_id].scheduling_info.periodicPHR_SF =  get_sf_perioidicPHR_Timer(UE_mac_inst[Mod_id].scheduling_info.periodicPHR_Timer);
  UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_SF =  get_sf_prohibitPHR_Timer(UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_Timer);
  // LOG_D(MAC,"phr %d %d\n ",UE_mac_inst[Mod_id].scheduling_info.periodicPHR_SF, UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_SF);
}
u8 get_phr_mapping (u8 Mod_id, u8 eNB_index){

//power headroom reporting range is from -23 ...+40 dB, as described in 36313
//note: mac_xface->get_Po_NOMINAL_PUSCH(Mod_id) is float
  if (mac_xface->get_PHR(Mod_id,eNB_index) < -23)
    return 0;
  else if (mac_xface->get_PHR(Mod_id,eNB_index) >= 40)
    return 63;
    else  // -23 to 40
      return  (u8) mac_xface->get_PHR(Mod_id,eNB_index) + PHR_MAPPING_OFFSET;
  
}
int get_sf_perioidicPHR_Timer(u8 perioidicPHR_Timer){
  return (perioidicPHR_Timer+1)*10;
}


int get_sf_prohibitPHR_Timer(u8 prohibitPHR_Timer){
  return (prohibitPHR_Timer)*10;
}

int get_db_dl_PathlossChange(u8 dl_PathlossChange){
  switch (dl_PathlossChange){
  case MAC_MainConfig__phr_Config__setup__dl_PathlossChange_dB1:
    return 1;
    break;
  case MAC_MainConfig__phr_Config__setup__dl_PathlossChange_dB3:
    return 3;
    break;
  case MAC_MainConfig__phr_Config__setup__dl_PathlossChange_dB6:
    return 6;
    break;
  case MAC_MainConfig__phr_Config__setup__dl_PathlossChange_infinity:
  default:
    return -1;
    break;
  }
}
