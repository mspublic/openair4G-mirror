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
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif

//#define DEBUG_UE_MAC_CTRL
//#define DEBUG_UE_MAC_RLC
//#define DEBUG_MAC_REPORT
//#define DEBUG_MAC_SCHEDULING

//#define DEBUG_RACH_MAC
//#define DEBUG_RACH_RRC
//#define DEBUG_SI_RRC
//#define DEBUG_HEADER_PARSING

/*
#ifndef USER_MODE
#define msg debug_msg
#endif
*/
extern inline unsigned int taus(void);


void ue_init_mac(){
  int i,j;
  for (i=0 ; i < NB_UE_INST; i++){
    // default values as deined in 36.331 sec 9.2.2
    LOG_I(MAC,"[UE%d] Applying default macMainConfig\n",i);
    //UE_mac_inst[Mod_id].scheduling_info.macConfig=NULL;
    UE_mac_inst[i].scheduling_info.retxBSR_Timer= MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf2560;
    UE_mac_inst[i].scheduling_info.periodicBSR_Timer=MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_infinity;
    UE_mac_inst[i].scheduling_info.SR_COUNTER=0;
    UE_mac_inst[i].scheduling_info.sr_ProhibitTimer=0;
    UE_mac_inst[i].scheduling_info.sr_ProhibitTimer_Running=0;
    UE_mac_inst[i].scheduling_info.maxHARQ_Tx=MAC_MainConfig__ul_SCH_Config__maxHARQ_Tx_n5;
    UE_mac_inst[i].scheduling_info.ttiBundling=0;
    UE_mac_inst[i].scheduling_info.drx_config=NULL;
    UE_mac_inst[i].scheduling_info.phr_config=NULL;
    UE_mac_inst[i].scheduling_info.periodicBSR_SF  = get_sf_periodicBSRTimer(UE_mac_inst[i].scheduling_info.periodicBSR_Timer);
    UE_mac_inst[i].scheduling_info.retxBSR_SF     = get_sf_retxBSRTimer(UE_mac_inst[i].scheduling_info.retxBSR_Timer);
    
    for (j=0; j < MAX_NUM_LCID; j++){
      LOG_I(MAC,"[UE%d] Applying default logical channel config for lcid %d\n",i,j);
      UE_mac_inst[i].scheduling_info.Bj[j]=-1; 
      UE_mac_inst[i].scheduling_info.bucket_size[j]=-1;
      UE_mac_inst[i].scheduling_info.new_transmission_flag[j]=0; // only the DTCH chanels are relavent
    }
  }
}

unsigned char *parse_header(unsigned char *mac_header,
			    unsigned char *num_ce,
			    unsigned char *num_sdu,
			    unsigned char *rx_ces,
			    unsigned char *rx_lcids,
			    unsigned short *rx_lengths) {

  unsigned char not_done=1,num_ces=0,num_sdus=0,lcid;
  unsigned char *mac_header_ptr = mac_header;
  unsigned short length;

  while (not_done==1) {

    if (((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E == 0)
      not_done = 0;

    lcid = ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID;
    if (lcid < UE_CONT_RES) {

      if (((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F == 0) {
	length = ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L;
	mac_header_ptr += sizeof(SCH_SUBHEADER_SHORT);
      }
      else {
	length = ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L;
	mac_header_ptr += sizeof(SCH_SUBHEADER_LONG);
      }
#ifdef DEBUG_HEADER_PARSING
      LOG_T(MAC,"[UE] sdu %d lcid %d length %d\n",num_sdus,lcid,length);
#endif
      rx_lcids[num_sdus] = lcid;
      rx_lengths[num_sdus] = length;
      num_sdus++;
    }
    else {  // This is a control element subheader
      rx_ces[num_ces] = lcid;
      num_ces++;
#ifdef DEBUG_HEADER_PARSING
      LOG_T(MAC,"[UE]ce %d lcid %d\n",num_ces,lcid);
#endif


      mac_header_ptr += sizeof(SCH_SUBHEADER_FIXED);
    }
  }
  *num_ce = num_ces;
  *num_sdu = num_sdus;

  return(mac_header_ptr);
}

u32 req_new_ulsch(u8 Mod_id){
 
  int drb;
  for (drb=DTCH; drb < MAX_NUM_LCID; drb++){
    if (UE_mac_inst[Mod_id].scheduling_info.new_transmission_flag[drb] == 1) // new transmission found
      return 1;
  }
  return 0;
}


u32 ue_get_SR(u8 Mod_id,u8 eNB_id,u16 rnti, u8 subframe) {
  
  // no UL-SCH resources available for this tti && UE has a valid PUCCH resources for SR configuration for this tti
  //  int MGL=6;// measurement gap length in ms
  int MGRP=0; // measurement gap repition period in ms
  int gapOffset=-1;
  int T=0; 
  //  int sfn=0;

  // Commented for the moment because this doesn't check the SRBs and so the connection cannot go through, 
  /*  

  if (req_new_ulsch(Mod_id) == 0)
    return 0;
  */

  // determin the measurement gap
  LOG_D(MAC,"[UE %d][SR %x] Frame %d subframe %d PHY asks for SR (SR_COUNTER/dsr_TransMax %d/%d), SR_pending %d\n",Mod_id,rnti,mac_xface->frame,subframe,
	UE_mac_inst[Mod_id].scheduling_info.SR_COUNTER,
	(1<<(2+UE_mac_inst[Mod_id].physicalConfigDedicated->schedulingRequestConfig->choice.setup.dsr_TransMax)),
	UE_mac_inst[Mod_id].scheduling_info.SR_pending);

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
    if ((subframe ==  gapOffset %10) && ((mac_xface->frame %T) == (floor(gapOffset/10))) 
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
    //   LOG_D(MAC,"[UE %d] instruct phy to signal SR navid ", Mod_id);
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

void ue_send_sdu(u8 Mod_id,u8 *sdu,u8 eNB_index) {

  unsigned char rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr;
  unsigned char rx_lcids[MAX_NUM_RB];
  unsigned short rx_lengths[MAX_NUM_RB];

  payload_ptr = parse_header(sdu,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths);

#ifdef DEBUG_HEADER_PARSING
  LOG_D(MAC,"[UE %d] ue_send_sdu : Frame %d eNB_index %d : num_ce %d num_sdu %d\n",Mod_id,
	mac_xface->frame,eNB_index,num_ce,num_sdu);
#endif

  for (i=0;i<num_ce;i++) {

      switch (rx_ces[i]) {
      case UE_CONT_RES:
#ifdef DEBUG_HEADER_PARSING
	LOG_D(MAC,"[UE %d] CE %d : UE contention resolution for RRC :",Mod_id,i);
	LOG_T(MAC, "[UE] %x,%x,%x,%x,%x,%x\n",payload_ptr[0],payload_ptr[1],payload_ptr[2],payload_ptr[3],payload_ptr[4],payload_ptr[5]);
	// Send to RRC here
#endif
	payload_ptr+=6;
	break;
      case TIMING_ADV_CMD:
#ifdef DEBUG_HEADER_PARSING
	LOG_D(MAC,"[UE] CE %d : UE Timing Advance :",i);
	LOG_T(MAC,"[UE] %d\n",payload_ptr[0]);
#endif
	process_timing_advance(payload_ptr[0]);
	payload_ptr++;
	break;
      case DRX_CMD:
#ifdef DEBUG_HEADER_PARSING
	LOG_D(MAC,"[UE] CE %d : UE DRX :",i);
#endif
	payload_ptr++;
	break;
      case SHORT_PADDING:
#ifdef DEBUG_HEADER_PARSING
	LOG_D(MAC,"[UE] CE %d : UE 1 byte Padding :",i);
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

      LOG_D(MAC,"[UE %d] RX CCCH -> RRC (%d bytes)\n",Mod_id,rx_lengths[i]);
      Rrc_xface->mac_rrc_data_ind(Mod_id,
				  CCCH,
				  (char *)payload_ptr,rx_lengths[i],0,eNB_index);

    }
    else if (rx_lcids[i] == DCCH) {
      LOG_D(MAC,"[UE %d] RX  DCCH \n",Mod_id);
      Mac_rlc_xface->mac_rlc_data_ind(Mod_id+NB_eNB_INST,
				      DCCH,
				      (char *)payload_ptr,
				      rx_lengths[i],
				      1,
				      NULL);
    }
    else if (rx_lcids[i] == DCCH1) {
      LOG_D(MAC,"[UE %d] RX  DCCH1 \n",Mod_id);
      Mac_rlc_xface->mac_rlc_data_ind(Mod_id+NB_eNB_INST,
				      DCCH1,
				      (char *)payload_ptr,
				      rx_lengths[i],
				      1,
				      NULL);
    }
    else if (rx_lcids[i] == DTCH) {
      LOG_D(MAC,"[UE %d][PDSCH] RX DTCH%d %d bytes to RLC \n",Mod_id,rx_lcids[i],rx_lengths[i]);
      Mac_rlc_xface->mac_rlc_data_ind(Mod_id+NB_eNB_INST,
				      DTCH,
				      (char *)payload_ptr,
				      rx_lengths[i],
				      1,
				      NULL);
    }

  }
}

void ue_decode_si(u8 Mod_id, u8 eNB_index, void *pdu,u16 len) {

#ifdef DEBUG_SI_RRC
  LOG_D(MAC,"[UE %d] Frame %d Sending SI to RRC (Lchan Id %d)\n",Mod_id,mac_xface->frame,BCCH);
#endif

  Rrc_xface->mac_rrc_data_ind(Mod_id,BCCH,(char *)pdu,len,0,eNB_index);

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
				    BSR_LONG *long_bsr) {

  SCH_SUBHEADER_FIXED *mac_header_ptr = (SCH_SUBHEADER_FIXED *)mac_header;
  unsigned char first_element=0,last_size=0,i;
  unsigned char mac_header_control_elements[16],*ce_ptr;

  //#ifdef DEBUG_HEADER_PARSING
  LOG_D(MAC,"[UE] Generate ULSCH : num_sdus %d\n",num_sdus);
  for (i=0;i<num_sdus;i++)
    LOG_T(MAC,"[UE] sdu %d : lcid %d length %d",i,sdu_lcids[i],sdu_lengths[i]);
  LOG_T(MAC,"\n");
  //#endif
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
    mac_header_ptr->R = 0;
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
    LOG_T(MAC,"[UE]sdu subheader %d (lcid %d, %d bytes)\n",i,sdu_lcids[i],sdu_lengths[i]);
#endif
    if (first_element>0) {
      mac_header_ptr->E = 1;
#ifdef DEBUG_HEADER_PARSING
      LOG_D(MAC,"[UE]last subheader : %x (R%d,E%d,LCID%d)\n",*(unsigned char*)mac_header_ptr,
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
    if (sdu_lengths[i] < 128) {
      ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->R    = 3;
      ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->E    = 0;
      ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F    = 0;
      ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->LCID = sdu_lcids[i];
      ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L    = (unsigned char)sdu_lengths[i];
      last_size=2;
#ifdef DEBUG_HEADER_PARSING
      LOG_D(MAC,"[UE]short sdu\n");
      LOG_T(MAC,"[UE]last subheader : %x (R%d,E%d,LCID%d,F%d,L%d)\n",
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
      LOG_D(MAC,"[UE]long sdu\n");
#endif
    }
  }

  mac_header_ptr+=last_size;
  memcpy((void*)mac_header_ptr,mac_header_control_elements,ce_ptr-mac_header_control_elements);
  mac_header_ptr+=(unsigned char)(ce_ptr-mac_header_control_elements);

  //#ifdef DEBUG_HEADER_PARSING
  LOG_T(MAC," [UE %d] header : ", crnti);
  for (i=0;i<((unsigned char*)mac_header_ptr - mac_header);i++) 
    LOG_T(MAC,"%2x.",mac_header[i]);
  LOG_T(MAC,"\n");
  //#endif
  return((unsigned char*)mac_header_ptr - mac_header);

}

void ue_get_sdu(u8 Mod_id,u8 eNB_index,u8 *ulsch_buffer,u16 buflen) {

  mac_rlc_status_resp_t rlc_status;
  u8 dcch_header_len,dcch1_header_len,dtch_header_len, bsr_header_len, bsr_ce_len=0, bsr_len;
  u16 sdu_lengths[8];
  u8 sdu_lcids[8],payload_offset=0,num_sdus=0;
  u8 ulsch_buff[MAX_ULSCH_PAYLOAD_BYTES];
  u16 sdu_length_total=0;
  u16 pdu_length_total=0;
  BSR_SHORT bsr_short;
  BSR_LONG bsr_long;
  BSR_SHORT *bsr_s=&bsr_short;
  BSR_LONG  *bsr_l=&bsr_long;

  int lcid;  
  // Compute header length
  
  dcch_header_len=sizeof(SCH_SUBHEADER_SHORT);
  dcch1_header_len=sizeof(SCH_SUBHEADER_SHORT);
  // hypo length,in case of long header skip the padding byte
  dtch_header_len=(UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DTCH] > 128 ) ? sizeof(SCH_SUBHEADER_LONG)-1 : sizeof(SCH_SUBHEADER_SHORT);
  bsr_header_len = sizeof(SCH_SUBHEADER_FIXED);
  bsr_ce_len = get_bsr_len (Mod_id, buflen);
  if (bsr_ce_len > 0 ){
    bsr_len = bsr_ce_len + bsr_header_len;
    LOG_D(MAC,"[UE %d] header size info: dcch %d, dcch1 %d, dtch %d, bsr (ce%d,hdr%d) buff_len %d\n",Mod_id, dcch_header_len,dcch1_header_len,dtch_header_len, bsr_ce_len, bsr_header_len, buflen); 
  } else 
    bsr_len = 0;
  
  // check for UL bandwidth requests and add SR control element
  
  // Check for DCCH first
  sdu_lengths[0]=0;
  if (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DCCH] > 0) {
    rlc_status = mac_rlc_status_ind(Mod_id+NB_eNB_INST,
				    DCCH,
				    (buflen-dcch_header_len-bsr_len));
    LOG_D(MAC,"[UE %d] DCCH has %d bytes to send (buffer %d, header %d): Mod_id to RLC %d\n",Mod_id,rlc_status.bytes_in_buffer,buflen,dcch_header_len,Mod_id+NB_eNB_INST);
    
    sdu_lengths[0] += Mac_rlc_xface->mac_rlc_data_req(Mod_id+NB_eNB_INST,
						      DCCH,
						      (char *)&ulsch_buff[sdu_lengths[0]]);
    
    sdu_length_total += sdu_lengths[0];
    sdu_lcids[0] = DCCH;
    LOG_D(MAC,"[UE %d] TX Got %d bytes for DCCH\n",Mod_id,sdu_lengths[0]);
    num_sdus = 1;
    update_bsr(Mod_id, DCCH);
    //header_len +=2;
  }
  else {
    dcch_header_len=0;
    num_sdus = 0;
  }
 
  // DCCH1
  if (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DCCH1] > 0) {
    
    rlc_status = mac_rlc_status_ind(Mod_id+NB_eNB_INST,
				    DCCH1,
				    (buflen-bsr_len-dcch_header_len-dcch1_header_len-sdu_length_total));
    
    LOG_D(MAC,"[UE %d] DCCH1 has %d bytes to send (buffer %d, header %d): Mod_id to RLC %d\n",Mod_id,rlc_status.bytes_in_buffer,buflen,dcch_header_len+dcch1_header_len,Mod_id+NB_eNB_INST);
    
    sdu_lengths[num_sdus] = Mac_rlc_xface->mac_rlc_data_req(Mod_id+NB_eNB_INST,
							    DCCH1,
							    (char *)&ulsch_buff[sdu_lengths[0]]);
    sdu_length_total += sdu_lengths[num_sdus];
    sdu_lcids[num_sdus] = DCCH1;
    LOG_D(MAC,"[UE %d] TX Got %d bytes for DCCH1\n",Mod_id,sdu_lengths[num_sdus]);
    num_sdus++;
    update_bsr(Mod_id, DCCH1);
    //dcch_header_len +=2; // include dcch1
  }
  else {
    dcch1_header_len =0;
  }

  // now check for other logical channels
  if ((UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DTCH] > 0) && 
      ((bsr_len+dcch_header_len+dcch1_header_len+dtch_header_len+sdu_length_total) <= buflen)){
    
    // adjust the dtch header lenght
    if ((UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DTCH] > 128) && 
	((UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[DTCH]+bsr_len+dcch_header_len+dcch1_header_len+dtch_header_len) > buflen)) 
      dtch_header_len = sizeof(SCH_SUBHEADER_LONG);
    
    
    rlc_status = mac_rlc_status_ind(Mod_id+NB_eNB_INST,
				    DTCH,
				    buflen-bsr_len-dcch_header_len-dcch1_header_len-dtch_header_len-sdu_length_total);
    
    LOG_D(MAC,"[UE %d] DTCH has %d bytes to send (buffer %d, header %d))\n",
	  Mod_id,rlc_status.bytes_in_buffer,buflen,dtch_header_len);
    
    sdu_lengths[num_sdus] = Mac_rlc_xface->mac_rlc_data_req(Mod_id+NB_eNB_INST,
							    DTCH,
							    (char *)&ulsch_buff[sdu_length_total]);
    
      LOG_D(MAC,"[UE %d] TX Got %d bytes for DTCH\n",Mod_id,sdu_lengths[num_sdus]);
      sdu_lcids[num_sdus] = DTCH;
      sdu_length_total += sdu_lengths[num_sdus];
      num_sdus++; 
      update_bsr(Mod_id, DTCH);
  }
  else { // no rlc pdu : generate the dummy header
    dtch_header_len = 0; 
  }
  
  // regular BSR
  //  if (UE_mac_inst[Mod_id].scheduling_info.macConfig 
  //   UE_mac_inst[Mod_id].scheduling_info.periodicBSR_Timer 
  // build bsr
  if (bsr_ce_len == sizeof(BSR_SHORT)) {
    bsr_l = NULL;
    lcid = UE_mac_inst[Mod_id].scheduling_info.BSR_short_lcid;
    bsr_s->LCGID = lcid;
    bsr_s->Buffer_size = UE_mac_inst[Mod_id].scheduling_info.BSR[lcid];
    LOG_D(MAC,"[UE %d] report short bsr (lcid %d, buffer size %d)\n", Mod_id, lcid, UE_mac_inst[Mod_id].scheduling_info.BSR[lcid]);
  } else if (bsr_ce_len == sizeof(BSR_LONG))  {
    bsr_s = NULL;
    bsr_l->Buffer_size0 = UE_mac_inst[Mod_id].scheduling_info.BSR[CCCH];
    bsr_l->Buffer_size1 = UE_mac_inst[Mod_id].scheduling_info.BSR[DCCH];
    bsr_l->Buffer_size2 = UE_mac_inst[Mod_id].scheduling_info.BSR[DCCH1];
    bsr_l->Buffer_size3 = UE_mac_inst[Mod_id].scheduling_info.BSR[DTCH];
    LOG_D(MAC, "[UE %d] report long bsr (bufferSize0 %d,bufferSize1 %d,bufferSize2 %d,bufferSize3 %d)\n", Mod_id,
	  UE_mac_inst[Mod_id].scheduling_info.BSR[CCCH],
	  UE_mac_inst[Mod_id].scheduling_info.BSR[DCCH],
	  UE_mac_inst[Mod_id].scheduling_info.BSR[DCCH1],
	  UE_mac_inst[Mod_id].scheduling_info.BSR[DTCH]);
  } else {
    UE_mac_inst[Mod_id].scheduling_info.new_transmission_flag[DTCH]=1;
    bsr_s = NULL;
    bsr_l = NULL;
  }
    // Generate header
  if (num_sdus>0) {
    
    payload_offset = generate_ulsch_header(ulsch_buffer,  // mac header
					   num_sdus,      // num sdus
					   0,            // short pading
					   sdu_lengths,  // sdu length
					   sdu_lcids,    // sdu lcid
					   NULL,  // power headroom
					   NULL,  // crnti
					   NULL,  // truncated bsr
					   bsr_s, // short bsr
					   bsr_l); // long_bsr

    LOG_D(MAC,"[UE %d] Payload offset %d sdu total length %d\n",
	Mod_id,payload_offset, sdu_length_total);

    // cycle through SDUs and place in ulsch_buffer
    memcpy(&ulsch_buffer[payload_offset],ulsch_buff,sdu_length_total);
  }
  else { // send BSR
    // bsr.LCGID = 0x0;
    //bsr.Buffer_size = 0x3f;

    payload_offset = generate_ulsch_header(ulsch_buffer,  // mac header
					   num_sdus,      // num sdus
					   0,            // short pading
					   sdu_lengths,  // sdu length
					   sdu_lcids,    // sdu lcid
					   NULL,  // power headroom
					   NULL,  // crnti
					   NULL,  // truncated bsr
					   bsr_s,
					   bsr_l); 

    LOG_D(MAC,"[UE %d] Payload offset %d sdu total length %d\n",
	Mod_id,payload_offset, sdu_length_total);

    // cycle through SDUs and place in ulsch_buffer
    memcpy(&ulsch_buffer[payload_offset],ulsch_buff,sdu_length_total);


    }
  
    LOG_D(MAC,"[UE %d][SR] Gave SDU to PHY, clearing any scheduling request\n",
	  Mod_id,payload_offset, sdu_length_total);
    UE_mac_inst[Mod_id].scheduling_info.SR_pending=0;
    UE_mac_inst[Mod_id].scheduling_info.SR_COUNTER=0;
    
}

// called at each slot (next_slot%2==0)
void ue_scheduler(u8 Mod_id, u8 subframe, lte_subframe_t direction) {

  int lcid; // lcid index
 
  int TTI= 1;
  int bucketsizeduration;
  int bucketsizeduration_max;
  mac_rlc_status_resp_t rlc_status[11];
 
  Mac_rlc_xface->frame=mac_xface->frame;
  Rrc_xface->Frame_index=Mac_rlc_xface->frame;
  Mac_rlc_xface->pdcp_run();
  
  // call SR procedure to generate pending SR and BSR for next PUCCH/PUSCH TxOp.  This should implement the procedures
  // outlined in Sections 5.4.4 an 5.4.5 of 36.321
  
    // Get RLC status info and update Bj for all lcids that are active 
  for (lcid=CCCH; lcid <= DTCH; lcid++ ) { 
    if ((lcid == 0) ||
	(UE_mac_inst[Mod_id].logicalChannelConfig[lcid])) {
      // meausre the Bj 
      if ((direction == SF_UL)&& (UE_mac_inst[Mod_id].scheduling_info.Bj[lcid] >= 0)){
	bucketsizeduration = UE_mac_inst[Mod_id].logicalChannelConfig[lcid]->ul_SpecificParameters->prioritisedBitRate * TTI;
	bucketsizeduration_max = get_ms_bucketsizeduration(UE_mac_inst[Mod_id].logicalChannelConfig[lcid]->ul_SpecificParameters->bucketSizeDuration);
	if ( UE_mac_inst[Mod_id].scheduling_info.Bj[lcid] > bucketsizeduration_max )
	  UE_mac_inst[Mod_id].scheduling_info.Bj[lcid] = bucketsizeduration_max;
	else
	  UE_mac_inst[Mod_id].scheduling_info.Bj[lcid] = bucketsizeduration;
      }
      // measure the buffer size 
      rlc_status[lcid] = mac_rlc_status_ind(Mod_id+NB_eNB_INST,
					    lcid,
					    0);//tb_size does not reauire when requesting the status
      //    LOG_D(MAC,"[UE %d] rlc buffer (lcid %d, byte %d)BSR level %d\n", 
      //	  Mod_id, lcid, rlc_status[lcid].bytes_in_buffer, UE_mac_inst[Mod_id].scheduling_info.buffer_status[lcid]);
      if (rlc_status[lcid].bytes_in_buffer > 0 ) {
	//set the bsr for all lcid by searching the table to find the bsr level 
	UE_mac_inst[Mod_id].scheduling_info.BSR[lcid] = locate (BSR_TABLE,BSR_TABLE_SIZE, rlc_status[lcid].bytes_in_buffer);
	//	UE_mac_inst[Mod_id].scheduling_info.num_BSR++;
	UE_mac_inst[Mod_id].scheduling_info.SR_pending=1;
	LOG_D(MAC,"[MAC][UE %d][SR] Frame %d subframe %d SR for PUSCH is pending for LCID %d with BSR level %d (%d bytes in RLC)\n", 
	      Mod_id, mac_xface->frame,subframe,lcid,
	      UE_mac_inst[Mod_id].scheduling_info.BSR[lcid],
	      rlc_status[lcid].bytes_in_buffer);
      } else {
	UE_mac_inst[Mod_id].scheduling_info.BSR[lcid]=0;
      }
    }
    update_bsr(Mod_id, lcid);
  }
    
  // UE has no valid phy config dedicated ||  no valid/released  SR 
  if ((UE_mac_inst[Mod_id].physicalConfigDedicated == NULL) || 
      (UE_mac_inst[Mod_id].physicalConfigDedicated->schedulingRequestConfig == NULL) ||
      (UE_mac_inst[Mod_id].physicalConfigDedicated->schedulingRequestConfig->present == SchedulingRequestConfig_PR_release)){
    
    // initiate RA with CRNTI included in msg3 (no contention) as descibed in 36.321 sec 5.1.5
    
    // cancel all pending SRs
    UE_mac_inst[Mod_id].scheduling_info.SR_pending=0;
    LOG_D(MAC,"[MAC][UE %d] Release all SRs \n", Mod_id);
  }
  
  // Call PHR procedure as described in Section 5.4.6 in 36.321
}

u8 get_bsr_len (u8 Mod_id, u16 buflen) {
 
  int lcid;
  u8 bsr_len=0, bsr_channel=0;
  int pdu = 0;
  
  bsr_channel = 0;
  bsr_len =0;
  pdu=0;
  for (lcid=DCCH; lcid <= DTCH; lcid++ ) { // dcch, dcch1, dtch
    if (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid] > 0 )
      pdu += (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid] + sizeof(SCH_SUBHEADER_SHORT) + bsr_len);
    if (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid] > 128 ) // long header size: adjust the header size
      pdu += 1; 
    // current phy buff can not transport all sdu for this lcid -> transmit a bsr for this lcid 
    
    if ( (pdu > buflen) &&  (UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid] > 0 ) ){ 
      bsr_channel+=1;
      bsr_len = ((bsr_channel > 1 ) ? sizeof(BSR_LONG) :  sizeof(BSR_SHORT)) ;
    }
  }
  if ( bsr_len > 0 )
    LOG_D(MAC,"BSR len is %d (ch%d, pdu %d) \n", bsr_len, bsr_channel, pdu);
  return bsr_len;
}


void update_bsr(u8 Mod_id, u8 lcid){

  mac_rlc_status_resp_t rlc_status;
  
  rlc_status = mac_rlc_status_ind(Mod_id+NB_eNB_INST,
					lcid,
					0);//tb_size does not reauire when requesting the status
  if (rlc_status.bytes_in_buffer > 0 ) {
    UE_mac_inst[Mod_id].scheduling_info.BSR[lcid] = locate (BSR_TABLE,BSR_TABLE_SIZE, rlc_status.bytes_in_buffer);
    UE_mac_inst[Mod_id].scheduling_info.BSR_bytes[lcid] = rlc_status.bytes_in_buffer;
    UE_mac_inst[Mod_id].scheduling_info.BSR_short_lcid = lcid; // only applicable to short bsr 
    if ( UE_mac_inst[Mod_id].scheduling_info.new_transmission_flag[lcid] == 0)
       UE_mac_inst[Mod_id].scheduling_info.new_transmission_flag[lcid]=1;
    LOG_D(MAC,"[UE %d] BSR level %d (lcid %d, rlc buffer %d byte)\n", 
	  Mod_id, UE_mac_inst[Mod_id].scheduling_info.BSR[lcid], lcid, rlc_status.bytes_in_buffer);
  } else {
    UE_mac_inst[Mod_id].scheduling_info.new_transmission_flag[lcid]=0;
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
