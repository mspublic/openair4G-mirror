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
/*! \file eNB_scheduler.c
* \brief procedures related to UE
* \author Raymond Knopp, Navid Nikaein
* \date 2011
* \version 0.5
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
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"

#include "RRC/LITE/extern.h"




#define DEBUG_eNB_SCHEDULER 1
#define DEBUG_HEADER_PARSING 0
//#define DEBUG_PACKET_TRACE 1

//#define ICIC 0

/*
  #ifndef USER_MODE
  #define msg debug_msg
  #endif
*/

extern inline unsigned int taus(void);


void init_ue_sched_info(void){
  int i,j;
  for (i=0;i<NUMBER_OF_eNB_MAX;i++){
    for (j=0;j<NUMBER_OF_UE_MAX;j++){
      // init DL
      eNB_dlsch_info[i][j].weight           = 0;
      eNB_dlsch_info[i][j].subframe         = 0;
      eNB_dlsch_info[i][j].serving_num      = 0;
      eNB_dlsch_info[i][j].status           = S_DL_NONE;
      // init UL
      eNB_ulsch_info[i][j].subframe         = 0;
      eNB_ulsch_info[i][j].serving_num      = 0;
      eNB_ulsch_info[i][j].status           = S_UL_NONE;
    }
  }
}

void add_ue_ulsch_info(unsigned char Mod_id, unsigned char UE_id, unsigned char subframe, UE_ULSCH_STATUS status){

  eNB_ulsch_info[Mod_id][UE_id].rnti             = find_UE_RNTI(Mod_id,UE_id);
  eNB_ulsch_info[Mod_id][UE_id].subframe         = subframe;
  eNB_ulsch_info[Mod_id][UE_id].status           = status;

  eNB_ulsch_info[Mod_id][UE_id].serving_num++;

}
void add_ue_dlsch_info(unsigned char Mod_id, unsigned char UE_id, unsigned char subframe, UE_DLSCH_STATUS status){

  eNB_dlsch_info[Mod_id][UE_id].rnti             = find_UE_RNTI(Mod_id,UE_id);
  //  eNB_dlsch_info[Mod_id][UE_id].weight           = weight;
  eNB_dlsch_info[Mod_id][UE_id].subframe         = subframe;
  eNB_dlsch_info[Mod_id][UE_id].status           = status;

  eNB_dlsch_info[Mod_id][UE_id].serving_num++;

}

unsigned char get_ue_weight(unsigned char Mod_id, unsigned char UE_id){

  return(eNB_dlsch_info[Mod_id][UE_id].weight);

}

// return is rnti ???
unsigned char schedule_next_ulue(unsigned char Mod_id, unsigned char UE_id, unsigned char subframe){

  unsigned char next_ue;

  // first phase: scheduling for ACK
  switch (subframe) {
    // scheduling for subframe 2: for scheduled user during subframe 5 and 6
  case 8:
    if  ((eNB_dlsch_info[Mod_id][UE_id].status == S_DL_SCHEDULED) &&
	 (eNB_dlsch_info[Mod_id][UE_id].subframe == 5 || eNB_dlsch_info[Mod_id][UE_id].subframe == 6)){
      // set the downlink status
      eNB_dlsch_info[Mod_id][UE_id].status = S_DL_BUFFERED;
      return UE_id;
    }
    break;
    // scheduling for subframe 3: for scheduled user during subframe 7 and 8
  case 9:
    if  ((eNB_dlsch_info[Mod_id][UE_id].status == S_DL_SCHEDULED) &&
	 (eNB_dlsch_info[Mod_id][UE_id].subframe == 7 || eNB_dlsch_info[Mod_id][UE_id].subframe == 8)){
      eNB_dlsch_info[Mod_id][UE_id].status = S_DL_BUFFERED;
      return UE_id;
    }
    break;
    // scheduling UL subframe 4: for scheduled user during subframe 9 and 0
  case 0 :
    if  ((eNB_dlsch_info[Mod_id][UE_id].status == S_DL_SCHEDULED) &&
	 (eNB_dlsch_info[Mod_id][UE_id].subframe == 9 || eNB_dlsch_info[Mod_id][UE_id].subframe == 0)){
      eNB_dlsch_info[Mod_id][UE_id].status = S_DL_BUFFERED;
      return UE_id;
    }
    break;
  default:
    break;
  }

  // second phase
  for (next_ue=0; next_ue <NUMBER_OF_UE_MAX; next_ue++ ){

    if  (eNB_ulsch_info[Mod_id][next_ue].status == S_UL_WAITING )
      return next_ue;
    else if (eNB_ulsch_info[Mod_id][next_ue].status == S_UL_SCHEDULED){
      eNB_ulsch_info[Mod_id][next_ue].status = S_UL_BUFFERED;
    }
  }
  for (next_ue=0; next_ue <NUMBER_OF_UE_MAX; next_ue++ ){
    if (eNB_ulsch_info[Mod_id][next_ue].status != S_UL_NONE )// do this just for active UEs
      eNB_ulsch_info[Mod_id][next_ue].status = S_UL_WAITING;
  }
  next_ue = 0;
  return next_ue;

}

unsigned char schedule_next_dlue(unsigned char Mod_id, unsigned char subframe){

  unsigned char next_ue;

  for (next_ue=0; next_ue <NUMBER_OF_UE_MAX; next_ue++ ){
    if  (eNB_dlsch_info[Mod_id][next_ue].status == S_DL_WAITING)
      return next_ue;
  }
  for (next_ue=0; next_ue <NUMBER_OF_UE_MAX; next_ue++ )
    if  (eNB_dlsch_info[Mod_id][next_ue].status == S_DL_BUFFERED) {
      eNB_dlsch_info[Mod_id][next_ue].status = S_DL_WAITING;
    }
  // next_ue = -1;
  return (-1);//next_ue;

}

void initiate_ra_proc(u8 Mod_id, u32 frame, u16 preamble_index,s16 timing_offset,u8 sect_id,u8 subframe,u8 f_id) {

  u8 i;

  LOG_I(MAC,"[eNB %d][RAPROC] Frame %d Initiating RA procedure for preamble index %d\n",Mod_id,frame,preamble_index);

  for (i=0;i<NB_RA_PROC_MAX;i++) {
    if (eNB_mac_inst[Mod_id].RA_template[i].RA_active==0) {
      eNB_mac_inst[Mod_id].RA_template[i].RA_active=1;
      eNB_mac_inst[Mod_id].RA_template[i].generate_rar=1;
      eNB_mac_inst[Mod_id].RA_template[i].generate_Msg4=0;
      eNB_mac_inst[Mod_id].RA_template[i].wait_ack_Msg4=0;
      eNB_mac_inst[Mod_id].RA_template[i].timing_offset=timing_offset;
      // Put in random rnti (to be replaced with proper procedure!!)
      eNB_mac_inst[Mod_id].RA_template[i].rnti = taus();
      eNB_mac_inst[Mod_id].RA_template[i].RA_rnti = 1+subframe+(10*f_id);
      eNB_mac_inst[Mod_id].RA_template[i].preamble_index = preamble_index;
      LOG_D(MAC,"[eNB %d][RAPROC] Frame %d Activating RAR generation for process %d, rnti %x, RA_active %d\n",
	    Mod_id,frame,i,eNB_mac_inst[Mod_id].RA_template[i].rnti,
	    eNB_mac_inst[Mod_id].RA_template[i].RA_active);
      
      return;
    }
  }
}

void cancel_ra_proc(u8 Mod_id, u32 frame, u16 preamble_index) {
  unsigned char i=0;
  LOG_I(MAC,"[eNB %d][RAPROC] Frame %d Cancelling RA procedure for index %d\n",Mod_id,frame, preamble_index);

  //for (i=0;i<NB_RA_PROC_MAX;i++) {
  eNB_mac_inst[Mod_id].RA_template[i].RA_active=0;
  eNB_mac_inst[Mod_id].RA_template[i].generate_rar=0;
  eNB_mac_inst[Mod_id].RA_template[i].generate_Msg4=0;
  eNB_mac_inst[Mod_id].RA_template[i].wait_ack_Msg4=0;
  eNB_mac_inst[Mod_id].RA_template[i].timing_offset=0;
  eNB_mac_inst[Mod_id].RA_template[i].RRC_timer=20;
  eNB_mac_inst[Mod_id].RA_template[i].rnti = 0;
  //}
}

void terminate_ra_proc(u8 Mod_id,u32 frame,u16 rnti,unsigned char *l3msg) {

  unsigned char rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr;
  unsigned char rx_lcids[MAX_NUM_RB];
  u16 rx_lengths[MAX_NUM_RB];
  s8 UE_id;

  LOG_I(MAC,"[eNB %d][RAPROC] Frame %d, Received l3msg %x.%x.%x.%x.%x.%x.%x.%x, Terminating RA procedure for UE rnti %x\n",
	Mod_id,frame,
	l3msg[0],l3msg[1],l3msg[2],l3msg[3],l3msg[4],l3msg[5],l3msg[6],l3msg[7], rnti);

  for (i=0;i<NB_RA_PROC_MAX;i++) {
    LOG_D(MAC,"[RAPROC] Checking proc %d : rnti (%x, %x), active %d\n",i,
	  eNB_mac_inst[Mod_id].RA_template[i].rnti, rnti,
	    eNB_mac_inst[Mod_id].RA_template[i].RA_active);
    if ((eNB_mac_inst[Mod_id].RA_template[i].rnti==rnti) &&
	(eNB_mac_inst[Mod_id].RA_template[i].RA_active==1)) {

      payload_ptr = parse_ulsch_header(l3msg,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths);
	LOG_D(MAC,"[eNB %d][RAPROC] Frame %d Received CCCH: length %d, offset %d\n",
	      Mod_id,frame,rx_lengths[0],payload_ptr-l3msg);
      if ((num_ce == 0) && (num_sdu==1) && (rx_lcids[0] == CCCH)) { // This is an RRCConnectionRequest
	memcpy(&eNB_mac_inst[Mod_id].RA_template[i].cont_res_id[0],payload_ptr,6);
	LOG_D(MAC,"[eNB %d][RAPROC] Frame %d Received CCCH: length %d, offset %d\n",
	      Mod_id,frame,rx_lengths[0],payload_ptr-l3msg);
	UE_id=add_new_ue(Mod_id,eNB_mac_inst[Mod_id].RA_template[i].rnti);
	if (UE_id==-1) {
	  mac_xface->macphy_exit("[MAC][eNB] Max user count reached\n");
	}
	else {
	  LOG_I(MAC,"[eNB %d][RAPROC] Frame %d Added user with rnti %x => UE %d\n",
		Mod_id,frame,eNB_mac_inst[Mod_id].RA_template[i].rnti,UE_id);
	}

	if (Is_rrc_registered == 1)
	  mac_rrc_data_ind(Mod_id,frame,CCCH,(char *)payload_ptr,rx_lengths[0],1,Mod_id);
	  // add_user.  This is needed to have the rnti for configuring UE (PHY). The UE is removed if RRC
	  // doesn't provide a CCCH SDU

      }
      else if (num_ce >0) {  // handle l3msg which is not RRCConnectionRequest
	//	process_ra_message(l3msg,num_ce,rx_lcids,rx_ces);
      }

      eNB_mac_inst[Mod_id].RA_template[i].generate_Msg4 = 1;
      eNB_mac_inst[Mod_id].RA_template[i].wait_ack_Msg4 = 0;

      return;
    } // if process is active

  } // loop on RA processes
}

DCI_PDU *get_dci_sdu(u8 Mod_id, u32 frame, u8 subframe) {

  return(&eNB_mac_inst[Mod_id].DCI_pdu);

}

s8 find_UE_id(unsigned char Mod_id,u16 rnti) {

  unsigned char i;

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    if (eNB_mac_inst[Mod_id].UE_template[i].rnti==rnti) {
      return(i);
    }
  }
  return(-1);

}

s16 find_UE_RNTI(unsigned char Mod_id, unsigned char UE_id) {

  return (eNB_mac_inst[Mod_id].UE_template[UE_id].rnti);

}
u8 is_UE_active(unsigned char Mod_id, unsigned char UE_id ){
  if (eNB_mac_inst[Mod_id].UE_template[UE_id].rnti !=0 )
    return 1;
  else
    return 0 ;
}
s8 find_active_UEs(unsigned char Mod_id){

  unsigned char UE_id;
  u16 rnti;
  unsigned char nb_active_ue=0;

  for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {

    if ((rnti=eNB_mac_inst[Mod_id].UE_template[UE_id].rnti) !=0){

      if (mac_xface->get_eNB_UE_stats(Mod_id,rnti) != NULL){ // check at the phy enb_ue state for this rnti
	nb_active_ue++;
      }
      else { // this ue is removed at the phy => remove it at the mac as well
	mac_remove_ue(Mod_id, UE_id);
      }
    }
  }
  return(nb_active_ue);
}

// function for determining which active ue should be granted resources in uplink based on BSR+QoS
u16 find_ulgranted_UEs(unsigned char Mod_id){

  // all active users should be granted
  return(find_active_UEs(Mod_id));
}

// function for determining which active ue should be granted resources in downlink based on CQI, SI, and BSR
u16 find_dlgranted_UEs(unsigned char Mod_id){

  // all active users should be granted
  return(find_active_UEs(Mod_id));
}
// get aggregatiob form phy for a give UE
unsigned char process_ue_cqi (unsigned char Mod_id, unsigned char UE_id) {

  unsigned char aggregation=2;
  // check the MCS and SNR and set the aggregation accordingly

  return aggregation;
}

s8 add_new_ue(unsigned char Mod_id, u16 rnti) {
  unsigned char i;

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    if (eNB_mac_inst[Mod_id].UE_template[i].rnti==0) {
      eNB_mac_inst[Mod_id].UE_template[i].rnti=rnti;
      eNB_ulsch_info[Mod_id][i].status = S_UL_WAITING;
      eNB_dlsch_info[Mod_id][i].status = S_UL_WAITING;
      LOG_D(MAC,"[eNB] Add UE_id %d : rnti %x\n",i,eNB_mac_inst[Mod_id].UE_template[i].rnti);
      return((s8)i);
    }
  }
  return(-1);
}

s8 mac_remove_ue(unsigned char Mod_id, unsigned char UE_id) {


  eNB_mac_inst[Mod_id].UE_template[UE_id].rnti = 0;
  eNB_ulsch_info[Mod_id][UE_id].rnti          = 0;
  eNB_ulsch_info[Mod_id][UE_id].status        = S_UL_NONE;
  eNB_dlsch_info[Mod_id][UE_id].rnti          = 0;
  eNB_dlsch_info[Mod_id][UE_id].status        = S_DL_NONE;

  return(1);
}

unsigned char *get_dlsch_sdu(u8 Mod_id,u32 frame,u16 rnti,u8 TBindex) {

  unsigned char UE_id;

  if (rnti==SI_RNTI) {
    LOG_D(MAC,"[eNB %d] Frame %d Get DLSCH sdu for BCCH \n",Mod_id,frame);

    return((unsigned char *)&eNB_mac_inst[Mod_id].BCCH_pdu.payload[0]);
  }
  else {

    UE_id = find_UE_id(Mod_id,rnti);
    LOG_D(MAC,"[eNB %d] Frame %d  Get DLSCH sdu for rnti %x => UE_id %d\n",Mod_id,frame,rnti,UE_id);

    return((unsigned char *)&eNB_mac_inst[Mod_id].DLSCH_pdu[UE_id][TBindex].payload[0]);
  }

}

unsigned char *parse_ulsch_header(unsigned char *mac_header,
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
    if (lcid < UE_CONT_RES) { // checkme: this should be  POWER_HEADROOM ???

      if (((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F == 0) {
	length = ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L;
	mac_header_ptr += 2;//sizeof(SCH_SUBHEADER_SHORT);
      }
      else {
	length = ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L;
	mac_header_ptr += 3;//sizeof(SCH_SUBHEADER_LONG);
      }
      rx_lcids[num_sdus] = lcid;
      rx_lengths[num_sdus] = length;
      num_sdus++;
    }
    else {  // This is a control element subheader BSR and CRNTI
      rx_ces[num_ces] = lcid;
      num_ces++;
      //LOG_D(MAC,"[eNB] bsr ce %d lcid %d\n",num_ces,lcid);
      mac_header_ptr++;// sizeof(SCH_SUBHEADER_FIXED);
    }
  }
  *num_ce = num_ces;
  *num_sdu = num_sdus;

  return(mac_header_ptr);
}

void SR_indication(u8 Mod_id,u32 frame, u16 rnti, u8 subframe) {

  u8 UE_id = find_UE_id(Mod_id,rnti);

  LOG_I(MAC,"[eNB %d][SR %x] Frame %d subframe %d Signaling SR\n",Mod_id,rnti,frame,subframe);
  eNB_mac_inst[Mod_id].UE_template[UE_id].ul_SR = 1;

}
void rx_sdu(u8 Mod_id,u32 frame,u16 rnti,u8 *sdu) {

  unsigned char rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr;
  unsigned char rx_lcids[MAX_NUM_RB];
  unsigned short rx_lengths[MAX_NUM_RB];
  unsigned char UE_id = find_UE_id(Mod_id,rnti);
  int ii;
  for(ii=0; ii<MAX_NUM_RB; ii++) rx_lengths[ii] = 0;

  LOG_D(MAC,"[eNB %d] Received ULSCH sdu from PHY (rnti %x, UE_id %d), parsing header\n",Mod_id,rnti,UE_id);
  payload_ptr = parse_ulsch_header(sdu,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths);

#ifdef DEBUG_PACKET_TRACE 
  if((sdu!=NULL)&&(sdu!=0))
    trace_pdu(3,sdu,rx_lengths[1]/*(payload_ptr - sdu )*/, Mod_id, rnti, 8);
#endif
  // control element
  for (i=0;i<num_ce;i++) {

    switch (rx_ces[i]) { // implement and process BSR + CRNTI +
    case POWER_HEADROOM:
      LOG_I(MAC,"[eNB] MAC CE_LCID %d : Received PHR R = %d PH = %d\n", rx_ces[i], (payload_ptr[0]>>6), payload_ptr[0]&0x3f);
      payload_ptr+=sizeof(POWER_HEADROOM_CMD);
      break;
    case CRNTI:
      LOG_I(MAC,"[eNB] MAC CE_LCID %d : Received CRNTI %d \n", rx_ces[i], payload_ptr[0]);
      payload_ptr+=1;
      break;
    case TRUNCATED_BSR:
    case SHORT_BSR :
      LOG_I(MAC,"[eNB] MAC CE_LCID %d : Received short BSR LCGID = %d bsr = %d\n", rx_ces[i],(payload_ptr[0]>>6), payload_ptr[0]&0x3f);
      eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[(payload_ptr[0]>>6)] = (payload_ptr[0]&0x3f);
      payload_ptr+=1;//sizeof(SHORT_BSR); // fixme
      break;
    case LONG_BSR :
      LOG_I(MAC,"[eNB] MAC CE_LCID %d :Received long BSR \n", rx_ces[i]);
      eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[3] = (payload_ptr[0]&0x3f);
      eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[2] = (payload_ptr[0]&0xfc0);
      eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[1] = (payload_ptr[0]&0x3F000);
      eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[0] = (payload_ptr[0]>>18);
      payload_ptr+=(sizeof(LONG_BSR)-1);
      break;
    default:
      break;
    }

  }

  for (i=0;i<num_sdu;i++) {
    LOG_D(MAC,"SDU Number %d MAC Subheader SDU_LCID %d, length %d\n",i,rx_lcids[i],rx_lengths[i]);

    if ((rx_lcids[i] == DCCH)||(rx_lcids[i] == DCCH1)) {
      //      if(eNB_mac_inst[Mod_id].Dcch_lchan[UE_id].Active==1){
      /*
	msg("offset: %d\n",(unsigned char)((unsigned char*)payload_ptr-sdu));
	for (j=0;j<32;j++)
	msg("%x ",payload_ptr[j]);
	msg("\n");
      */
      //  This check is just to make sure we didn't get a bogus SDU length, to be removed ...
      if (rx_lengths[i]<CCCH_PAYLOAD_SIZE_MAX) {
	LOG_D(MAC,"[eNB %d] Frame %d : ULSCH -> UL-DCCH, received %d bytes form UE %d \n",
	      Mod_id,frame, rx_lengths[i], UE_id);

	mac_rlc_data_ind(Mod_id,frame,1,
			 rx_lcids[i]+(UE_id)*MAX_NUM_RB,
			 (char *)payload_ptr,
			 rx_lengths[i],
			 1,
			 NULL);//(unsigned int*)crc_status);
      }
      //      }
    } else if (rx_lcids[i] >= DTCH) {
      //      if(eNB_mac_inst[Mod_id].Dcch_lchan[UE_id].Active==1){
      /*  msg("offset: %d\n",(unsigned char)((unsigned char*)payload_ptr-sdu));
	  for (j=0;j<32;j++)
	  printf("%x ",payload_ptr[j]);
	  printf("\n"); */
      LOG_D(MAC,"[eNB %d] Frame %d : ULSCH -> UL-DTCH, received %d bytes from UE %d \n",
	      Mod_id,frame, rx_lengths[i], UE_id);
      if (rx_lengths[i] <SCH_PAYLOAD_SIZE_MAX) {   // MAX SIZE OF transport block
	mac_rlc_data_ind(Mod_id,frame,1,
			 DTCH+(UE_id)*MAX_NUM_RB,
			 (char *)payload_ptr,
			 rx_lengths[i],
			 1,
			 NULL);//(unsigned int*)crc_status);
      }
      //      }
    }

    payload_ptr+=rx_lengths[i];
  }


}

unsigned char generate_dlsch_header(unsigned char *mac_header,
				    unsigned char num_sdus,
				    unsigned short *sdu_lengths,
				    unsigned char *sdu_lcids,
				    unsigned char drx_cmd,
				    unsigned char timing_advance_cmd,
				    unsigned char *ue_cont_res_id,
				    unsigned char short_padding,
				    unsigned short post_padding) {

  SCH_SUBHEADER_FIXED *mac_header_ptr = (SCH_SUBHEADER_FIXED *)mac_header;
  u8 first_element=0,last_size=0,i;
  u8 mac_header_control_elements[16],*ce_ptr;

  ce_ptr = &mac_header_control_elements[0];

  // compute header components
  
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

  if (drx_cmd != 255) {
    if (first_element>0) {
      mac_header_ptr->E = 1;
      mac_header_ptr++;
    }
    else {
      first_element=1;
    }
    mac_header_ptr->R = 0;
    mac_header_ptr->E    = 0;
    mac_header_ptr->LCID = DRX_CMD;
    last_size=1;
  }

  if (timing_advance_cmd != 0) {
    if (first_element>0) {
      mac_header_ptr->E = 1;
      mac_header_ptr++;
    }
    else {
      first_element=1;
    }
    mac_header_ptr->R = 0;
    mac_header_ptr->E    = 0;
    mac_header_ptr->LCID = TIMING_ADV_CMD;
    last_size=1;
    //    msg("last_size %d,mac_header_ptr %p\n",last_size,mac_header_ptr);
    ((TIMING_ADVANCE_CMD *)ce_ptr)->R=0;
    ((TIMING_ADVANCE_CMD *)ce_ptr)->TA=timing_advance_cmd&0x3f;
    ce_ptr+=sizeof(TIMING_ADVANCE_CMD);
    //msg("offset %d\n",ce_ptr-mac_header_control_elements);
  }

  if (ue_cont_res_id) {
    if (first_element>0) {
      mac_header_ptr->E = 1;
      /*   
      printf("[eNB][MAC] last subheader : %x (R%d,E%d,LCID%d)\n",*(unsigned char*)mac_header_ptr,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->R,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E,
	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID);
      */
      mac_header_ptr++;
    }
    else {
      first_element=1;
    }

    mac_header_ptr->R = 0;
    mac_header_ptr->E    = 0;
    mac_header_ptr->LCID = UE_CONT_RES;
    last_size=1;
    
    LOG_D(MAC,"[eNB ][RAPROC] Generate contention resolution msg: %x.%x.%x.%x.%x.%x\n",
	ue_cont_res_id[0],
	ue_cont_res_id[1],
	ue_cont_res_id[2],
	ue_cont_res_id[3],
	ue_cont_res_id[4],
	ue_cont_res_id[5]);
    
    memcpy(ce_ptr,ue_cont_res_id,6);
    ce_ptr+=6;
    // msg("(cont_res) : offset %d\n",ce_ptr-mac_header_control_elements);
  }

  //msg("last_size %d,mac_header_ptr %p\n",last_size,mac_header_ptr);

  for (i=0;i<num_sdus;i++) {
    //msg("MAC num sdu %d len sdu %d\n",num_sdus, sdu_lengths[i]);
    if (first_element>0) {
      mac_header_ptr->E = 1;
      /*msg("last subheader : %x (R%d,E%d,LCID%d)\n",*(unsigned char*)mac_header_ptr,
	  ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->R,
	  ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E,
	  ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID);
      */
      mac_header_ptr+=last_size;
      //msg("last_size %d,mac_header_ptr %p\n",last_size,mac_header_ptr);
    }
    else {
      first_element=1;
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
  /*

  printf("last_size %d,mac_header_ptr %p\n",last_size,mac_header_ptr);
  
    printf("last subheader : %x (R%d,E%d,LCID%d)\n",*(unsigned char*)mac_header_ptr,
    ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->R,
    ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E,
    ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID);


    if (((SCH_SUBHEADER_FIXED*)mac_header_ptr)->LCID < UE_CONT_RES) {
    if (((SCH_SUBHEADER_SHORT*)mac_header_ptr)->F == 0)
    printf("F = 0, sdu len (L field) %d\n",(((SCH_SUBHEADER_SHORT*)mac_header_ptr)->L));
    else
    printf("F = 1, sdu len (L field) %d\n",(((SCH_SUBHEADER_LONG*)mac_header_ptr)->L));
    }
  */
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

//msg("After subheaders %d\n",(u8*)mac_header_ptr - mac_header);
  
  if ((ce_ptr-mac_header_control_elements) > 0) {
    // printf("Copying %d bytes for control elements\n",ce_ptr-mac_header_control_elements);
    memcpy((void*)mac_header_ptr,mac_header_control_elements,ce_ptr-mac_header_control_elements);
    mac_header_ptr+=(unsigned char)(ce_ptr-mac_header_control_elements);
  }
//msg("After CEs %d\n",(u8*)mac_header_ptr - mac_header);

  return((unsigned char*)mac_header_ptr - mac_header);

}
void add_common_dci(DCI_PDU *DCI_pdu,
		    void *pdu,
		    u16 rnti,
		    unsigned char dci_size_bytes,
		    unsigned char aggregation,
		    unsigned char dci_size_bits,
		    unsigned char dci_fmt,
		    u8 ra_flag) {

  memcpy(&DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].dci_pdu[0],pdu,dci_size_bytes);
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].dci_length = dci_size_bits;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].L          = aggregation;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].rnti       = rnti;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].format     = dci_fmt;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].ra_flag    = ra_flag;


  DCI_pdu->Num_common_dci++;
}

void add_ue_spec_dci(DCI_PDU *DCI_pdu,void *pdu,u16 rnti,unsigned char dci_size_bytes,unsigned char aggregation,unsigned char dci_size_bits,unsigned char dci_fmt,u8 ra_flag) {

  memcpy(&DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].dci_pdu[0],pdu,dci_size_bytes);
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].dci_length = dci_size_bits;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].L          = aggregation;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].rnti       = rnti;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].format     = dci_fmt;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].ra_flag    = ra_flag;

  DCI_pdu->Num_ue_spec_dci++;
}

void schedule_SI(unsigned char Mod_id,u32 frame, unsigned char *nprb,unsigned char *nCCE) {

  unsigned char bcch_sdu_length;

  bcch_sdu_length = mac_rrc_data_req(Mod_id,
				     frame,
				     BCCH,1,
				     (char*)&eNB_mac_inst[Mod_id].BCCH_pdu.payload[0],
				     1,
				     Mod_id);
  if (bcch_sdu_length > 0) {
    LOG_D(MAC,"[eNB %d] Frame %d : BCCH->BCH, Received %d bytes \n",Mod_id,frame,bcch_sdu_length);

    if (bcch_sdu_length <= (mac_xface->get_TBS(0,3)))
      BCCH_alloc_pdu.mcs=0;
    else if (bcch_sdu_length <= (mac_xface->get_TBS(1,3)))
      BCCH_alloc_pdu.mcs=1;
    else if (bcch_sdu_length <= (mac_xface->get_TBS(2,3)))
      BCCH_alloc_pdu.mcs=2;
    else if (bcch_sdu_length <= (mac_xface->get_TBS(3,3)))
      BCCH_alloc_pdu.mcs=3;
    else if (bcch_sdu_length <= (mac_xface->get_TBS(4,3)))
      BCCH_alloc_pdu.mcs=4;
    else if (bcch_sdu_length <= (mac_xface->get_TBS(5,3)))
      BCCH_alloc_pdu.mcs=5;
    else if (bcch_sdu_length <= (mac_xface->get_TBS(6,3)))
      BCCH_alloc_pdu.mcs=6;

    LOG_D(MAC,"[eNB] Frame %d : Scheduling BCCH->BCH for SI %d bytes (mcs %d, TBS %d)\n",
	frame,
	bcch_sdu_length,
	BCCH_alloc_pdu.mcs,
	mac_xface->get_TBS(BCCH_alloc_pdu.mcs,3));

    eNB_mac_inst[Mod_id].bcch_active=1;
    *nprb=3;
    *nCCE=4;
    return;
  }
  eNB_mac_inst[Mod_id].bcch_active=0;
  *nprb=0;
  *nCCE=0;
}

// First stage of Random-Access Scheduling
void schedule_RA(unsigned char Mod_id,u32 frame, unsigned char subframe,unsigned char *nprb,unsigned char *nCCE) {

  RA_TEMPLATE *RA_template = (RA_TEMPLATE *)&eNB_mac_inst[Mod_id].RA_template[0];
  unsigned char i,harq_pid,round;
  u16 rrc_sdu_length;
  unsigned char lcid,offset;
  s8 UE_id;
  unsigned short TBsize,msg4_padding,msg4_post_padding,msg4_header;

  for (i=0;i<NB_RA_PROC_MAX;i++) {

    if (RA_template[i].RA_active == 1) {

      LOG_D(MAC,"[eNB %d][RAPROC] RA %d is active (generate RAR %d, generate_Msg4 %d, wait_ack_Msg4 %d)\n",
	  Mod_id,i,RA_template[i].generate_rar,RA_template[i].generate_Msg4,RA_template[i].wait_ack_Msg4);

      if (RA_template[i].generate_rar == 1) {
	*nprb= (*nprb) + 3;
	*nCCE = (*nCCE) + 4;
      }
      else if (RA_template[i].generate_Msg4 == 1) {

	// check for Msg4 Message
	UE_id = find_UE_id(Mod_id,RA_template[i].rnti);
	if (Is_rrc_registered == 1) {

	  // Get RRCConnectionSetup for Piggyback
	  rrc_sdu_length = mac_rrc_data_req(Mod_id,
					    frame,
					    0,1,
					    (char*)&eNB_mac_inst[Mod_id].CCCH_pdu.payload[0],
					    1,
					    Mod_id);
	  if (rrc_sdu_length == -1)
	    mac_xface->macphy_exit("[MAC][eNB Scheduler] CCCH not allocated\n");
	  else {
	    //msg("[MAC][eNB %d] Frame %d, subframe %d: got %d bytes from RRC\n",Mod_id,frame, subframe,rrc_sdu_length);
	  }
	}

	if (rrc_sdu_length>0) {
	  LOG_D(MAC,"[eNB %d][RAPROC] Frame %d, subframe %d: Generating Msg4 with RRC Piggyback (RA proc %d, RNTI %x)\n",Mod_id,frame, subframe,i,
	      RA_template[i].rnti);

	  //msg("[MAC][eNB %d][RAPROC] Frame %d, subframe %d: Received %d bytes for Msg4: \n",Mod_id,frame,subframe,rrc_sdu_length);
	  //	  for (j=0;j<rrc_sdu_length;j++)
	  //	    msg("%x ",(unsigned char)eNB_mac_inst[Mod_id].CCCH_pdu.payload[j]);
	  //	  msg("\n");
	  //	  msg("[MAC][eNB] Frame %d, subframe %d: Generated DLSCH (Msg4) DCI, format 1A, for UE %d\n",frame, subframe,UE_id);
	  // Schedule Reflection of Connection request

	  ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->ndi=1;

	  // Compute MCS for 3 PRB
	  msg4_header = 1+6+1;  // CR header, CR CE, SDU header
	  if ((rrc_sdu_length+msg4_header) <= 22) {
	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=4;
	    TBsize = 22;
	  }
	  else if ((rrc_sdu_length+msg4_header) <= 28) {
	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=5;
	    TBsize = 28;
	  }
	  else if ((rrc_sdu_length+msg4_header) <= 32) {
	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=6;
	    TBsize = 32;
	  }
	  else if ((rrc_sdu_length+msg4_header) <= 41) {
	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=7;
	    TBsize = 41;
	  }
	  else if ((rrc_sdu_length+msg4_header) <= 49) {
	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=8;
	    TBsize = 49;
	  }
	  else if ((rrc_sdu_length+msg4_header) <= 57) {
	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=9;
	    TBsize = 57;
	  }
	  RA_template[i].generate_Msg4=0;
	  RA_template[i].generate_Msg4_dci=1;
	  //	  RA_template[i].wait_ack_Msg4=1;
	  RA_template[i].RA_active = 0;
	  lcid=0;

	  if ((TBsize - rrc_sdu_length - msg4_header) <= 2) {
	    msg4_padding = TBsize - rrc_sdu_length - msg4_header;
	    msg4_post_padding = 0;
	  }
	  else {
	    msg4_padding = 0;
	    msg4_post_padding = TBsize - rrc_sdu_length - msg4_header-1;
	  }
	  LOG_D(MAC,"[eNB %d][RAPROC] Frame %d subframe %d Msg4 : TBS %d, sdu_len %d, msg4_header %d, msg4_padding %d, msg4_post_padding %d\n",
		Mod_id,frame,subframe,TBsize,rrc_sdu_length,msg4_header,msg4_padding,msg4_post_padding); 
	  offset = generate_dlsch_header((unsigned char*)eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)UE_id][0].payload[0],
					 1,                           //num_sdus
					 &rrc_sdu_length,             //
					 &lcid,                       // sdu_lcid
					 255,                         // no drx
					 0,                           // no timing advance
					 RA_template[i].cont_res_id,  // contention res id
					 msg4_padding,                // no padding
					 msg4_post_padding);

	  memcpy((void*)&eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)UE_id][0].payload[0][(unsigned char)offset],
		 &eNB_mac_inst[Mod_id].CCCH_pdu.payload[0],
		 rrc_sdu_length);
	  *nprb= (*nprb) + 3;
	  *nCCE = (*nCCE) + 4;
	}
	//try here
      } 
      /*
      else if (eNB_mac_inst[Mod_id].RA_template[i].wait_ack_Msg4==1) {
	// check HARQ status and retransmit if necessary
	//msg("[MAC][eNB %d][RAPROC] Frame %d, subframe %d: Checking if Msg4 was acknowledged :\n",Mod_id,frame,subframe);
	// Get candidate harq_pid from PHY
	mac_xface->get_ue_active_harq_pid(Mod_id,eNB_mac_inst[Mod_id].RA_template[i].rnti,subframe,&harq_pid,&round,0);
	if (round>0) {
	  *nprb= (*nprb) + 3;
	  *nCCE = (*nCCE) + 4;
	}
      }
      */
    }
  }
}

// This has to be updated to include BSR information
u8 UE_is_to_be_scheduled(u8 Mod_id,u8 UE_id) {


  //  LOG_D(MAC,"[eNB %d][PUSCH] Frame %d subframe %d Scheduling UE %d\n",Mod_id,rnti,frame,subframe,
  //	UE_id);

  if ((eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DTCH]>0) ||
      (eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH]>0) ||
      (eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH1]>0) ||
      (eNB_mac_inst[Mod_id].UE_template[UE_id].ul_SR>0))
    return(1);
  else return(0);
}

u32 bytes_to_bsr_index(s32 nbytes) {

  u32 i=0;

  if (nbytes<0)
    return(0);

  while ((i<BSR_TABLE_SIZE)&&
	 (BSR_TABLE[i]<=nbytes)){
    i++;
  }
  return(i-1);
}

// This table holds the allowable PRB sizes for ULSCH transmissions
u8 rb_table[33] = {1,2,3,4,5,6,8,9,10,12,15,16,18,20,24,25,27,30,32,36,40,45,48,50,54,60,72,75,80,81,90,96,100};

void schedule_ulsch(unsigned char Mod_id,u32 frame,unsigned char cooperation_flag,unsigned char subframe,unsigned char *nCCE) {

  unsigned char UE_id;
  unsigned char next_ue;
  unsigned char granted_UEs;
  unsigned char nCCE_available;
  unsigned char aggregation;
  u16 rnti;
  unsigned char round;
  unsigned char harq_pid;
  DCI0_5MHz_TDD_1_6_t *ULSCH_dci;
  //  DCI0_5MHz_TDD_1_6_t *ULSCH_dci0;
  //  DCI0_5MHz_TDD_1_6_t *ULSCH_dci1;
  LTE_eNB_UE_stats* eNB_UE_stats;
  DCI_PDU *DCI_pdu= &eNB_mac_inst[Mod_id].DCI_pdu;
  u8 status=0;//,status0 = 0,status1 = 0;
  //  u8 k=0;
  u8 rb_table_index;
  u16 TBS,first_rb=0,i;
  u32 buffer_occupancy;
  u32 tmp_bsr;

  granted_UEs = find_ulgranted_UEs(Mod_id);
  nCCE_available = mac_xface->get_nCCE_max(Mod_id) - *nCCE;
  //weight = get_ue_weight(Mod_id,UE_id);
  aggregation = 2; // set to maximum aggregation level

  // allocated UE_ids until nCCE
  for (UE_id=0;UE_id<granted_UEs && (nCCE_available > aggregation);UE_id++) {

    if (UE_is_to_be_scheduled(Mod_id,UE_id)>0) {


      // find next ue to schedule
      //    msg("[MAC][eNB] subframe %d: checking UE_id %d\n",subframe,UE_id);
      next_ue = UE_id;//schedule_next_ulue(Mod_id,UE_id,subframe);
      //    msg("[MAC][eNB] subframe %d: next ue %d\n",subframe,next_ue);
      rnti = find_UE_RNTI(Mod_id,next_ue);

      LOG_D(MAC,"[eNB %d][PUSCH %x] Frame %d subframe %d Scheduling UE (SR %d)\n",Mod_id,rnti,frame,subframe,
	    eNB_mac_inst[Mod_id].UE_template[UE_id].ul_SR);

      eNB_mac_inst[Mod_id].UE_template[UE_id].ul_SR = 0;


      if (rnti==0)
	continue;
      //    msg("[MAC][eNB] subframe %d: rnti %x\n",subframe,rnti);
      aggregation = process_ue_cqi(Mod_id,next_ue);
      //    msg("[MAC][eNB] subframe %d: aggregation %d\n",subframe,aggregation);

      eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);
      if (eNB_UE_stats==NULL)
	mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");

      //msg("[MAC][eNB %d] Scheduler Frame %d, subframe %d, nCCE %d: Checking ULSCH next UE_id %d mode id %d (rnti %x,mode %s), format 0\n",Mod_id,frame,subframe,*nCCE,next_ue,Mod_id, rnti,mode_string[eNB_UE_stats->mode]);

      if (eNB_UE_stats->mode == PUSCH) {

	// Get candidate harq_pid from PHY
	mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,1);

	// Note this code is still for a specific DCI format
	ULSCH_dci = (DCI0_5MHz_TDD_1_6_t *)eNB_mac_inst[Mod_id].UE_template[next_ue].ULSCH_DCI[harq_pid];
	//ULSCH_dci0 = (DCI0_5MHz_TDD_1_6_t *)eNB_mac_inst[Mod_id].UE_template[0].ULSCH_DCI[harq_pid];
	//ULSCH_dci1 = (DCI0_5MHz_TDD_1_6_t *)eNB_mac_inst[Mod_id].UE_template[1].ULSCH_DCI[harq_pid];

	//msg("FAIL\n");
	status = mac_get_rrc_status(Mod_id,1,next_ue);
	//status0 = Rrc_xface->get_rrc_status(Mod_id,1,0);
	//status1 = Rrc_xface->get_rrc_status(Mod_id,1,1);
	/* if((status0 < RRC_CONNECTED) && (status1 < RRC_CONNECTED))
	       ULSCH_dci->cqi_req = 0;
	       else
	       ULSCH_dci->cqi_req = 1;
	*/

	if (status < RRC_CONNECTED)
	  ULSCH_dci->cqi_req = 0;
	else
	  ULSCH_dci->cqi_req = 1;


	ULSCH_dci->type=0;
	if (round > 0) {
	  ULSCH_dci->ndi = 0;
	}
	else {
	  ULSCH_dci->ndi = 1;
	}
	//if ((frame&1)==0) {

	// choose this later based on Power Headroom
	if (ULSCH_dci->ndi == 1) // set mcs for first round
	  ULSCH_dci->mcs     = openair_daq_vars.target_ue_ul_mcs;
	else  // increment RV
	  ULSCH_dci->mcs = round + 28;

	// schedule 4 RBs for UL
	if((cooperation_flag > 0) && (next_ue == 1))// Allocation on same set of RBs
	  {
	    ULSCH_dci->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,
						       ((next_ue-1)*4),//openair_daq_vars.ue_ul_nb_rb),
						       4);//openair_daq_vars.ue_ul_nb_rb);
	  }
	else if (ULSCH_dci->ndi==1) {
	  rb_table_index = 1;
	  TBS = mac_xface->get_TBS(ULSCH_dci->mcs,rb_table[rb_table_index]);
	  buffer_occupancy = ((eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DTCH]  == 0) &&
			      (eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH]  == 0) &&
			      (eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH1] == 0))?
	    BSR_TABLE[1] :   // This is when we've received SR and buffers are fully served
	    BSR_TABLE[eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DTCH]]+
	    BSR_TABLE[eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH]]+
	    BSR_TABLE[eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH1]];  // This is when remaining data in UE buffers (even if SR is triggered)

	  LOG_D(MAC,"[eNB %d][PUSCH %x] Frame %d subframe %d Scheduled UE (DCCH bsr %d, DCCH1 bsr %d, DTCH bsr %d), BO %d\n",
		Mod_id,rnti,frame,subframe,
		eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH] ,
		eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH1],
		eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DTCH] ,buffer_occupancy);

	  while ((TBS < buffer_occupancy) &&
		 rb_table[rb_table_index]<(mac_xface->lte_frame_parms->N_RB_UL-1-first_rb)){
	    // continue until we've exhauster the UEs request or the total number of available PRBs
	    LOG_D(MAC,"[eNB %d][PUSCH %x] Frame %d subframe %d Scheduled UE (rb_table_index %d => TBS %d)\n",
		  Mod_id,rnti,frame,subframe,
		  rb_table_index,TBS);

	    rb_table_index++;
	    TBS = mac_xface->get_TBS(ULSCH_dci->mcs,rb_table[rb_table_index]);
	  }

	  LOG_D(MAC,"[eNB %d][PUSCH %x] Frame %d subframe %d Scheduled UE (mcs %d, first rb %d, nb_rb %d, rb_table_index %d, TBS %d)\n",
		Mod_id,rnti,frame,subframe,ULSCH_dci->mcs,
		first_rb,rb_table[rb_table_index],
		rb_table_index,mac_xface->get_TBS(ULSCH_dci->mcs,rb_table[rb_table_index]));

	  ULSCH_dci->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,
						     first_rb,
						     rb_table[rb_table_index]);//openair_daq_vars.ue_ul_nb_rb);

	  first_rb+=rb_table[rb_table_index];  // increment for next UE allocation
	  buffer_occupancy -= mac_xface->get_TBS(ULSCH_dci->mcs,rb_table[rb_table_index]);

	  i = bytes_to_bsr_index((s32)buffer_occupancy);

	  // Adjust BSR entries for DCCH, DCCH1 and DTCH, add others later
	  if (i>0) {
	    if (eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH] <= i) {
	      tmp_bsr = BSR_TABLE[eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH]];
	      eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH] = 0;
	      if (BSR_TABLE[eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH1]] <= (buffer_occupancy-tmp_bsr)) {
		tmp_bsr += BSR_TABLE[eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH1]];
		eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH1] = 0;
		eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DTCH] = bytes_to_bsr_index((s32)BSR_TABLE[eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DTCH]] - ((s32)buffer_occupancy - (s32)tmp_bsr));
	      }
	      else {
		eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH1] = bytes_to_bsr_index((s32)BSR_TABLE[eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH1]] - ((s32)buffer_occupancy -(s32)tmp_bsr));
	      }
	    }
	    else {
	      eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH] = bytes_to_bsr_index((s32)BSR_TABLE[eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH]] - (s32)buffer_occupancy);
	    }
	  }
	  else {  // we have flushed all buffers so clear bsr
	      eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH] = 0;
	      eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DCCH1] = 0;
	      eNB_mac_inst[Mod_id].UE_template[UE_id].bsr_info[DTCH] = 0;
	  }
	}

	// Cyclic shift for DM RS
	if(cooperation_flag == 2) {
	  if(next_ue == 1)// For Distriibuted Alamouti, cyclic shift applied to 2nd UE
	    ULSCH_dci->cshift = 1;
	  else
	    ULSCH_dci->cshift = 0;
	}
	else
	  ULSCH_dci->cshift = 0;// values from 0 to 7 can be used for mapping the cyclic shift (36.211 , Table 5.5.2.1.1-1)

	ULSCH_dci->TPC = 1;  // Do not adjust power for now

	add_ue_spec_dci(DCI_pdu,
			ULSCH_dci,
			rnti,
			sizeof(DCI0_5MHz_TDD_1_6_t),
			aggregation,
			sizeof_DCI0_5MHz_TDD_1_6_t,
			format0,
			0);
	//#ifdef DEBUG_eNB_SCHEDULER
	//      dump_dci(mac_xface->lte_frame_parms,
	//	       &DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci-1]);
	//#endif
	add_ue_ulsch_info(Mod_id,
			  next_ue,
			  subframe,
			  S_UL_SCHEDULED);

	*nCCE = (*nCCE) - aggregation;

	//msg("[MAC][eNB %d][ULSCH Scheduler] Frame %d, subframe %d: Generated ULSCH DCI for next UE_id %d, format 0\n", Mod_id,frame,subframe,next_ue);


      } // UE is in PUSCH
    } // UE_is_to_be_scheduled
  } // loop over UE_id
}
u32 allocate_prbs(unsigned char UE_id,unsigned char nb_rb, u32 *rballoc) {

  int i;
  u32 rballoc_dci=0;
  unsigned char nb_rb_alloc=0;

  for (i=0;i<(mac_xface->lte_frame_parms->N_RB_DL-2);i+=2) {
    if (((*rballoc>>i)&3)==0) {
      *rballoc |= (3<<i);
      rballoc_dci |= (1<<(i>>1));
      nb_rb_alloc+=2;
    }
    if (nb_rb_alloc==nb_rb)
      return(rballoc_dci);
  }

  if ((mac_xface->lte_frame_parms->N_RB_DL&1)==1) {
    if ((*rballoc>>(mac_xface->lte_frame_parms->N_RB_DL-1)&1)==0) {
      *rballoc |= (1<<(mac_xface->lte_frame_parms->N_RB_DL-1));
      rballoc_dci |= (1<<(mac_xface->lte_frame_parms->N_RB_DL>>1));
    }
  }
  return(rballoc_dci);
}


u32 allocate_prbs_sub(int nb_rb, u8 *rballoc) {

  u8 check1=0,check2=0;
  u16 rballoc_dci=0;

  //msg("*****Check1RBALLOC****: %d%d%d%d\n",rballoc[3],rballoc[2],rballoc[1],rballoc[0]);
  while(nb_rb >0){
    if(rballoc[check2] == 1){
      rballoc_dci |= (1<<(check1>>1));
      nb_rb = nb_rb -2;
    }
    check2 = check2+1;
    check1 = check1+2;
  }
  // rballoc_dci = (rballoc_dci)&(0x1fff);
  //msg("*********RBALLOC : %x\n",rballoc_dci);
  // exit(-1);
  return (rballoc_dci);
}


void fill_DLSCH_dci(unsigned char Mod_id,u32 frame, unsigned char subframe,u32 RBalloc,u8 RA_scheduled) {
  // loop over all allocated UEs and compute frequency allocations for PDSCH

  unsigned char UE_id,first_rb,nb_rb=3;
  u16 rnti;
  unsigned char vrb_map[100];

  unsigned int x,y,z=0;
  u8 rballoc_sub[14];

  u32 rballoc=RBalloc;

  //  u32 test=0;
  unsigned char round;
  unsigned char harq_pid;
  void *DLSCH_dci=NULL;
  DCI_PDU *DCI_pdu= &eNB_mac_inst[Mod_id].DCI_pdu;
  int i;
  //  u8 status=0;
#ifdef ICIC
  FILE *DCIi;
  DCIi = fopen("dci.txt","a");
  int b;

  buff=rballoc;
  fprintf(DCIi,"eNB: %d active user: %d |rballoc init:\t\t\t",Mod_id,find_active_UEs(Mod_id));
  for (b=31;b>=0;b--)
    fprintf(DCIi,"%d",(buff>>b)&1);
  fprintf(DCIi,"\n");

#endif

  // clear vrb_map
  memset(vrb_map,0,100);

  // SI DLSCH
  //  printf("BCCH check\n");
  if (eNB_mac_inst[Mod_id].bcch_active == 1) {
    eNB_mac_inst[Mod_id].bcch_active = 0;

    // randomize frequency allocation for SI
    first_rb = (unsigned char)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
    BCCH_alloc_pdu.rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
    rballoc |= mac_xface->get_rballoc(BCCH_alloc_pdu.vrb_type,BCCH_alloc_pdu.rballoc);


    vrb_map[first_rb] = 1;
    vrb_map[first_rb+1] = 1;
    vrb_map[first_rb+2] = 1;

    add_common_dci(DCI_pdu,
		   &BCCH_alloc_pdu,
		   SI_RNTI,
		   sizeof(DCI1A_5MHz_TDD_1_6_t),
		   2,
		   sizeof_DCI1A_5MHz_TDD_1_6_t,
		   format1A,0);
    LOG_D(MAC,"[eNB %d] Frame %d: Adding common dci for SI\n",Mod_id,frame);

#ifdef    DEBUG_PACKET_TRACE
    if((DCI_pdu!=NULL)&&(DCI_pdu!=0)) {
	LOG_I(OPT,"Trace_PDU_4578\n\r");
	trace_pdu(4,DCI_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t), UE_id, SI_RNTI, subframe);
      }
#endif
  }

  if (RA_scheduled == 1) {

    for (i=0;i<NB_RA_PROC_MAX;i++) {
      if (eNB_mac_inst[Mod_id].RA_template[i].generate_rar == 1) {

	eNB_mac_inst[Mod_id].RA_template[i].generate_rar = 0;
	LOG_D(MAC,"[eNB %d] Frame %d, subframe %d: Generating RAR DCI (proc %d), RA_active %d format 1A (%d,%d))\n",
	      Mod_id,frame, subframe,i,
	      eNB_mac_inst[Mod_id].RA_template[i].RA_active,
	      eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt1,
	      eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits1);
	// randomize frequency allocation for RA
	while (1) {
	  first_rb = (unsigned char)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
	  if ((vrb_map[first_rb] != 1) && (vrb_map[first_rb+2] != 1))
	    break;
	}
	vrb_map[first_rb] = 1;
	vrb_map[first_rb+1] = 1;
	vrb_map[first_rb+2] = 1;

	((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0])->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
	rballoc |= mac_xface->get_rballoc(((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0])->vrb_type,
					  ((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0])->rballoc);


	add_common_dci(DCI_pdu,
		       (void*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0],
		       eNB_mac_inst[Mod_id].RA_template[i].RA_rnti,
		       eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1,
		       2,
		       eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits1,
		       eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt1,
		       1);
	LOG_D(MAC,"[eNB %d] Frame %d: Adding common dci for RA%d (RAR) RA_active %d\n",Mod_id,frame,i,
	      eNB_mac_inst[Mod_id].RA_template[i].RA_active);
      }
      if (eNB_mac_inst[Mod_id].RA_template[i].generate_Msg4_dci == 1) {

	// randomize frequency allocation for RA
	while (1) {
	  first_rb = (unsigned char)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
	  if ((vrb_map[first_rb] != 1) && (vrb_map[first_rb+2] != 1))
	    break;
	}
	vrb_map[first_rb] = 1;
	vrb_map[first_rb+1] = 1;
	vrb_map[first_rb+2] = 1;
	((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc= mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
	rballoc |= mac_xface->get_rballoc(((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->vrb_type,
					  ((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc);


	add_ue_spec_dci(DCI_pdu,
			(void*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0],
			eNB_mac_inst[Mod_id].RA_template[i].rnti,
			eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes2,
			2,
			eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits2,
			eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt2,
			0);
	LOG_D(MAC,"[eNB %d][RAPROC] Frame %d: Adding ue specific dci (rnti %x) for Msg4\n",
	    Mod_id,frame,eNB_mac_inst[Mod_id].RA_template[i].rnti);
	eNB_mac_inst[Mod_id].RA_template[i].generate_Msg4_dci=0;

#ifdef    DEBUG_PACKET_TRACE
	if((DLSCH_dci!=NULL)&&(DLSCH_dci!=0))	{
	    LOG_I(OPT,"Trace_PDU_4\n\r");
	    trace_pdu(4,DLSCH_dci,eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1, UE_id, rnti, subframe);
	  }
#endif
      }
      else if (eNB_mac_inst[Mod_id].RA_template[i].wait_ack_Msg4==1) {
	// check HARQ status and retransmit if necessary
	LOG_D(MAC,"[eNB %d] Frame %d, subframe %d: Checking if Msg4 was acknowledged :",
	    Mod_id,frame,subframe);
	// Get candidate harq_pid from PHY
	mac_xface->get_ue_active_harq_pid(Mod_id,eNB_mac_inst[Mod_id].RA_template[i].rnti,subframe,&harq_pid,&round,0);
	if (round>0) {
	  // we have to schedule a retransmission
	  ((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->ndi=0;
	  // randomize frequency allocation for RA
	  while (1) {
	    first_rb = (unsigned char)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
	    if ((vrb_map[first_rb] != 1) && (vrb_map[first_rb+2] != 1))
	      break;
	  }
	  vrb_map[first_rb] = 1;
	  vrb_map[first_rb+1] = 1;
	  vrb_map[first_rb+2] = 1;
	  ((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
	  rballoc |= mac_xface->get_rballoc(((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->vrb_type,
					    ((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc);


	  add_ue_spec_dci(DCI_pdu,
			  (void*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0],
			  eNB_mac_inst[Mod_id].RA_template[i].rnti,
			  eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes2,
			  2,
			  eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits2,
			  eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt2,
			  0);
	  LOG_D(MAC,"[eNB %d] Frame %d: Adding ue specific dci (rnti %x) for RA (Msg4 Retransmission)\n",
	      Mod_id,frame,eNB_mac_inst[Mod_id].RA_template[i].rnti);
	}
	else {
	  LOG_D(MAC,"[eNB %d] Msg4 acknowledged\n",Mod_id);
	  eNB_mac_inst[Mod_id].RA_template[i].wait_ack_Msg4=0;
	  eNB_mac_inst[Mod_id].RA_template[i].RA_active=0;
	}
#ifdef    DEBUG_PACKET_TRACE
	if((DLSCH_dci!=NULL)&&(DLSCH_dci!=0)) {
	  trace_pdu(4,DLSCH_dci,eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1, UE_id, rnti, subframe);
	  }
#endif
      }
    }
  } // RA is scheduled in this subframe

  // UE specific DCIs
  for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
    //printf("UE_id: %d => status %d\n",UE_id,eNB_dlsch_info[Mod_id][UE_id].status);
    if (eNB_dlsch_info[Mod_id][UE_id].status == S_DL_SCHEDULED) {

      // clear scheduling flag
      eNB_dlsch_info[Mod_id][UE_id].status = S_DL_WAITING;
      rnti = find_UE_RNTI(Mod_id,UE_id);
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);
      nb_rb = eNB_mac_inst[Mod_id].UE_template[UE_id].nb_rb[harq_pid];

      DLSCH_dci = (void *)eNB_mac_inst[Mod_id].UE_template[UE_id].DLSCH_DCI[harq_pid];

      /// Synchronizing rballoc with rballoc_sub
      for(x=0;x<7;x++){
	for(y=0;y<2;y++){
	  z = 2*x + y;
	    if(z < (2*6 + 1)){
	      rballoc_sub[z] = eNB_mac_inst[Mod_id].UE_template[UE_id].rballoc_sub[harq_pid][x];
	    }
	}
      }
      for(i=0;i<13;i++){
	if(rballoc_sub[i] == 1)
	  rballoc |= (0x0001<<i);
      }

      switch(mac_xface->get_transmission_mode(Mod_id,rnti)) {
      default:

      case 1:

      case 2:
	printf("Adding UE spec DCI for %d PRBS (%x) => ",nb_rb,rballoc);
	((DCI1_5MHz_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs(UE_id,nb_rb,&rballoc);
	((DCI1_5MHz_TDD_t*)DLSCH_dci)->rah = 0;
	//	printf("%x\n",((DCI1_5MHz_TDD_t*)DLSCH_dci)->rballoc);
	add_ue_spec_dci(DCI_pdu,
			DLSCH_dci,
			rnti,
			sizeof(DCI1_5MHz_TDD_t),
			2,//aggregation,
			sizeof_DCI1_5MHz_TDD_t,
			format1,
			0);
#ifdef ICIC
	buff=rballoc;
	fprintf(DCIi,"eNB: %d|rballoc DLSCH:\t\t\t\t\t",Mod_id);
	for (b=31;b>=0;b--)
	  fprintf(DCIi,"%d",(buff>>b)&1);
	fprintf(DCIi,"\n");

	buff=test;
	fprintf(DCIi,"eNB: %d|rballoc DLSCH DCI:\t\t\t\t",Mod_id);
	for (b=31;b>=0;b--)
	  fprintf(DCIi,"%d",(buff>>b)&1);
	fprintf(DCIi,"\n");
#endif


	break;
      case 4:

	//if (nb_rb>10) {
	// DCI format 2_2A_M10PRB can also be used for less than 10 PRB (it refers to the system bandwidth)
	((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs(UE_id,nb_rb,&rballoc);
	((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rah = 0;
	add_ue_spec_dci(DCI_pdu,
			DLSCH_dci,
			rnti,
			sizeof(DCI2_5MHz_2A_M10PRB_TDD_t),
			2,//aggregation,
			sizeof_DCI2_5MHz_2A_M10PRB_TDD_t,
			format2_2A_M10PRB,
			0);
	/*}
	  else {
	  ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs(UE_id,nb_rb,&rballoc);
	  add_ue_spec_dci(DCI_pdu,
	  DLSCH_dci,
	  rnti,
	  sizeof(DCI2_5MHz_2A_L10PRB_TDD_t),
	  2,//aggregation,
	  sizeof_DCI2_5MHz_2A_L10PRB_TDD_t,
	  format2_2A_L10PRB);
	  }*/
	break;
      case 5:
	/*	for(x=0;x<7;x++){
	  for(y=0;y<2;y++){
	    if(z < (2*6 + 1)){
	      z = 2*x + y;
	      rballoc_sub[z] = eNB_mac_inst[Mod_id].UE_template[UE_id].rballoc_sub[harq_pid][x];
	    }
	  }
	  }*/
	((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs_sub(nb_rb,rballoc_sub);
	((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rah = 0;

	add_ue_spec_dci(DCI_pdu,
			DLSCH_dci,
			rnti,
			sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t),
			2,//aggregation,
			sizeof_DCI1E_5MHz_2A_M10PRB_TDD_t,
			format1E_2A_M10PRB,
			0);
	break;
      }

#ifdef    DEBUG_PACKET_TRACE

      if((DLSCH_dci!=NULL)&&(DLSCH_dci!=0))
	{
	  LOG_I(OPT,"Trace_PDU_444\n\r");
	  trace_pdu(4,DLSCH_dci,eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1, UE_id, rnti, subframe);
	}
#endif
    }


  }
#ifdef ICIC
  fclose(DCIi);
#endif

}

//***************************PRE_PROCESSOR for MU-MIMO IN TM5*********************************//
/*
This function is specific to TM5, where it compares all the available UEs for orthogonal PMIs and schedules them in MU-MIMO,
if it has more traffic compared to SU-MIMO
1. dl_pow_off gives an indication whether the UE is scheduled in MU-MIMO (0) or SU-MIMO(1) mode
2. pre_nb_available_rbs gives the total number of RBs available for each UE
3. rballoc_sub gives an indicator of which subbands are occupied by a UE
*/
void tm5_pre_processor (unsigned char Mod_id,
			unsigned char subframe,
			u16 nb_rb_used0,
			unsigned char nCCE_used,
			u8 *dl_pow_off,
			u16 *pre_nb_available_rbs,
			unsigned char rballoc_sub[256][7]){

  unsigned char UE_id,UE_id_temp;
  u16 UE_SU_MIMO = 256;
  unsigned char next_ue, next_ue_temp;
  u16 ue[2][7];
  unsigned char granted_UEs;
  u16 nCCE;
  unsigned char aggregation;
  u16 nb_available_rb,j,TBS,rnti,rnti0=0,rnti1=0,rnti_temp,rnti_k[2][7];
  //nb_rb,TBS;
  unsigned char round=0,round_temp=0,round_k=0;
  //round=0;
  unsigned char harq_pid_temp=0,harq_pid_k=0,harq_pid=0;
  //harq_pid=0;
  void *DLSCH_dci;
  LTE_eNB_UE_stats* eNB_UE_stats;
  LTE_eNB_UE_stats* eNB_UE_stats0;
  LTE_eNB_UE_stats* eNB_UE_stats1;
  LTE_eNB_UE_stats* eNB_UE_stats_temp;
  LTE_eNB_UE_stats* eNB_UE_stats_k[2][7];
  unsigned char k0=0,k1=0,k2=0,k3=0,k4=0,k5=0,k6=0;
  unsigned char i0=0,i1=0,i2=0,i3=0,i4=0,i5=0,i6=0;
  //u8 dl_pow_off[256];
  u8 status=0;
  u16 i=0,ii=0,check=0,jj=0,total_rbs=0;
  //unsigned char rballoc_sub[256][7];
  //u16 pre_nb_available_rbs[256];
  u8 MIMO_mode_indicator[7]= {2,2,2,2,2,2,2};
  u8 total_DL_cqi_MUMIMO = 0,total_DL_cqi_SUMIMO = 0;
  u16 total_TBS_SUMIMO = 0,total_TBS_MUMIMO = 0; 

  /// Initialization
  for(i=0;i<256;i++)
    {
      dl_pow_off[i] = 2;
      pre_nb_available_rbs[i] = 0;
      for(ii=0;ii<7;ii++)
	rballoc_sub[i][ii]=0;
    }


  for(i=0;i<2;i++)
    {
      for(ii=0;ii<7;ii++){
	MIMO_mode_indicator[ii] = 2;
	ue[i][ii] = 256;
      }
    }
  granted_UEs = find_dlgranted_UEs(Mod_id);
  //weight = get_ue_weight(Mod_id,UE_id);
  aggregation = 2; // set to the maximum aggregation level

  // set current available nb_rb and nCCE to maximum
  nb_available_rb = mac_xface->lte_frame_parms->N_RB_DL - nb_rb_used0;
  nCCE = mac_xface->get_nCCE_max(Mod_id) - nCCE_used;

  //********************* Pre-processing for Scheduling UEs**************************///////

  for (UE_id=0;UE_id<granted_UEs;UE_id++) {
      if ((nb_available_rb == 0) || (nCCE < aggregation))
	break;
      next_ue = UE_id;

      // If nobody is left, exit while loop and go to next step
      if (next_ue == 255)
	break;

      // This is an allocated UE_id
      rnti = find_UE_RNTI(Mod_id,next_ue);
      eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

      if (eNB_UE_stats==NULL)
	mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");


      // Get candidate harq_pid from PHY
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid_temp,&round_temp,0);


      switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
      case 1:break;
      case 2:break;
      case 4:break;
      case 5:
	for (UE_id_temp = UE_id+1;UE_id_temp < granted_UEs;UE_id_temp++) {


	  next_ue_temp = UE_id_temp;
	  // If nobody is left, exit while loop and go to next step
	  if (next_ue_temp == 255)
	    break;

	  // This is an allocated UE_id
	  rnti_temp = find_UE_RNTI(Mod_id,next_ue_temp);
	  eNB_UE_stats_temp = mac_xface->get_eNB_UE_stats(Mod_id,rnti_temp);

	  if (eNB_UE_stats_temp==NULL)
	    mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");


	  // Get candidate harq_pid from PHY
	  mac_xface->get_ue_active_harq_pid(Mod_id,rnti_temp,subframe,&harq_pid_k,&round_k,0);



	  switch (mac_xface->get_transmission_mode(Mod_id,rnti_temp)) {
	  case 1:break;
	  case 2:break;
	  case 4:break;
	  case 5:
	    if((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)<<14)&0xc000)== 0x4000)
	      {

		if(k0 == 1)
		  {
		    rnti_k[0][0] = find_UE_RNTI(Mod_id,ue[0][0]);
		    rnti_k[1][0] = find_UE_RNTI(Mod_id,ue[1][0]);


		    eNB_UE_stats_k[0][0] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[0][0]);
		    eNB_UE_stats_k[1][0] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[1][0]);

		    if((eNB_UE_stats->DL_cqi[0]+eNB_UE_stats_temp->DL_cqi[0])>(eNB_UE_stats_k[0][0]->DL_cqi[0]+eNB_UE_stats_k[1][0]->DL_cqi[0]))
		      {


			ue[0][0] = next_ue;
			ue[1][0] = next_ue_temp;


		      }
		  }
		else
		  {
		    ue[0][0] = next_ue;
		    ue[1][0] = next_ue_temp;


		    k0 = 1;
		  }
	      }


	    if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>2)<<14)&0xc000)== 0x4000)
	      {


		if(k1 == 1)
		  {
		    rnti_k[0][1] = find_UE_RNTI(Mod_id,ue[0][1]);
		    rnti_k[1][1] = find_UE_RNTI(Mod_id,ue[1][1]);


		    eNB_UE_stats_k[0][1] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[0][1]);
		    eNB_UE_stats_k[1][1] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[1][1]);

		    if((eNB_UE_stats->DL_cqi[0]+eNB_UE_stats_temp->DL_cqi[0])>(eNB_UE_stats_k[0][1]->DL_cqi[0]+eNB_UE_stats_k[1][1]->DL_cqi[0]))
		      {
			ue[0][1] = next_ue;
			ue[1][1] = next_ue_temp;


		      }
		  }
		else
		  {
		    ue[0][1] = next_ue;
		    ue[1][1] = next_ue_temp;



		    k1 = 1;
		  }
	      }



	    if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>4)<<14)&0xc000)== 0x4000)
	      {


		if(k2 == 1)
		  {
		    rnti_k[0][2] = find_UE_RNTI(Mod_id,ue[0][2]);
		    rnti_k[1][2] = find_UE_RNTI(Mod_id,ue[1][2]);


		    eNB_UE_stats_k[0][2] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[0][2]);
		    eNB_UE_stats_k[1][2] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[1][2]);

		    if((eNB_UE_stats->DL_cqi[0]+eNB_UE_stats_temp->DL_cqi[0])>(eNB_UE_stats_k[0][2]->DL_cqi[0]+eNB_UE_stats_k[1][2]->DL_cqi[0]))
		      {
			ue[0][2] = next_ue;
			ue[1][2] = next_ue_temp;

		      }
		  }
		else
		  {
		    ue[0][2] = next_ue;
		    ue[1][2] = next_ue_temp;



		    k2 = 1;
		  }
	      }




	    if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>6)<<14)&0xc000)== 0x4000)
	      {

		if(k3 == 1)
		  {
		    rnti_k[0][3] = find_UE_RNTI(Mod_id,ue[0][3]);
		    rnti_k[1][3] = find_UE_RNTI(Mod_id,ue[1][3]);


		    eNB_UE_stats_k[0][3] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[0][3]);
		    eNB_UE_stats_k[1][3] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[1][3]);

		    if((eNB_UE_stats->DL_cqi[0]+eNB_UE_stats_temp->DL_cqi[0])>(eNB_UE_stats_k[0][3]->DL_cqi[0]+eNB_UE_stats_k[1][3]->DL_cqi[0]))
		      {
			ue[0][3] = next_ue;
			ue[1][3] = next_ue_temp;


		      }
		  }
		else
		  {
		    ue[0][3] = next_ue;
		    ue[1][3] = next_ue_temp;



		    k3 = 1;
		  }
	      }



	    if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>8)<<14)&0xc000)== 0x4000)
	      {


		if(k4 == 1)
		  {
		    rnti_k[0][4] = find_UE_RNTI(Mod_id,ue[0][4]);
		    rnti_k[1][4] = find_UE_RNTI(Mod_id,ue[1][4]);


		    eNB_UE_stats_k[0][4] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[0][4]);
		    eNB_UE_stats_k[1][4] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[1][4]);

		    if((eNB_UE_stats->DL_cqi[0]+eNB_UE_stats_temp->DL_cqi[0])>(eNB_UE_stats_k[0][4]->DL_cqi[0]+eNB_UE_stats_k[1][4]->DL_cqi[0]))
		      {
			ue[0][4] = next_ue;
			ue[1][4]= next_ue_temp;


		      }
		  }
		else
		  {
		    ue[0][4] = next_ue;
		    ue[1][4] = next_ue_temp;



		    k4= 1;
		  }
	      }



	    if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>10)<<14)&0xc000)== 0x4000)
	      {


		if(k5 == 1)
		  {
		    rnti_k[0][5] = find_UE_RNTI(Mod_id,ue[0][5]);
		    rnti_k[1][5] = find_UE_RNTI(Mod_id,ue[1][5]);


		    eNB_UE_stats_k[0][5] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[0][5]);
		    eNB_UE_stats_k[1][5] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[1][5]);

		    if((eNB_UE_stats->DL_cqi[0]+eNB_UE_stats_temp->DL_cqi[0])>(eNB_UE_stats_k[0][5]->DL_cqi[0]+eNB_UE_stats_k[1][5]->DL_cqi[0]))
		      {
			ue[0][5] = next_ue;
			ue[1][5]= next_ue_temp;


		      }
		  }
		else
		  {
		    ue[0][5] = next_ue;
		    ue[1][5] = next_ue_temp;


		    k5= 1;
		  }
	      }



	    if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>12)<<14)&0xc000)== 0x4000)
	      {

		if(k6 == 1)
		  {
		    rnti_k[0][6] = find_UE_RNTI(Mod_id,ue[0][6]);
		    rnti_k[1][6] = find_UE_RNTI(Mod_id,ue[1][6]);


		    eNB_UE_stats_k[0][6] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[0][6]);
		    eNB_UE_stats_k[1][6] = mac_xface->get_eNB_UE_stats(Mod_id,rnti_k[1][6]);

		    if((eNB_UE_stats->DL_cqi[0]+eNB_UE_stats_temp->DL_cqi[0])>(eNB_UE_stats_k[0][6]->DL_cqi[0]+eNB_UE_stats_k[1][6]->DL_cqi[0]))
		      {
			ue[0][6] = next_ue;
			ue[1][6]= next_ue_temp;


		      }
		  }
		else
		  {
		    ue[0][6] = next_ue;
		    ue[1][6] = next_ue_temp;



		    k6= 1;
		  }
	      }
	    break;
	  case 6: break;
	  case 7: break;
	  default: break;
	  }
	}
	break;
      case 6:break;
      case 7:
	break;
      default:
	break;
      }
    }




  if(k0==1)
    {
      dl_pow_off[ue[0][0]] = 0;
      dl_pow_off[ue[1][0]] = 0;
      MIMO_mode_indicator[0] = 0;

      pre_nb_available_rbs[ue[0][0]] = 4;
      pre_nb_available_rbs[ue[1][0]] = 4;
      rballoc_sub[ue[0][0]][0] = 1;
      rballoc_sub[ue[1][0]][0] = 1;

    }

  if(k1==1)
    {
      dl_pow_off[ue[0][1]] = 0;
      dl_pow_off[ue[1][1]] = 0;
      MIMO_mode_indicator[1] = 0;

      if ((ue[0][1] == ue[0][0]) || (ue[0][1] == ue[1][0]))
	pre_nb_available_rbs[ue[0][1]] = 8;
      else
	pre_nb_available_rbs[ue[0][1]] = 4;
      if((ue[1][1] == ue[0][0]) || (ue[1][1] == ue[1][0]))
	pre_nb_available_rbs[ue[1][1]] = 8;
      else
	pre_nb_available_rbs[ue[1][1]] = 4;
      rballoc_sub[ue[0][1]][1] = 1;
      rballoc_sub[ue[1][1]][1] = 1;
    }


  if(k2 == 1)
    {
      dl_pow_off[ue[0][2]] = 0;
      dl_pow_off[ue[1][2]] = 0;
      MIMO_mode_indicator[2] = 0;

      if (((ue[0][2] == ue[0][0])|| (ue[0][2] == ue[1][0]))&&((ue[0][2] == ue[0][1])|| (ue[0][2] == ue[1][1])))
	pre_nb_available_rbs[ue[0][2]] = 12;
      else
	if((ue[0][2] == ue[0][0]) || (ue[0][2] == ue[1][0]) || (ue[0][2] == ue[0][1]) || (ue[0][2] == ue[1][1]))
	  pre_nb_available_rbs[ue[0][2]] = 8;
	else
	  pre_nb_available_rbs[ue[0][2]] = 4;

      if (((ue[1][2] == ue[0][0])|| (ue[1][2] == ue[1][0]))&&((ue[1][2] == ue[0][1])|| (ue[1][2] == ue[1][1])))
	pre_nb_available_rbs[ue[1][2]] = 12;
      else
	if((ue[1][2] == ue[0][0]) || (ue[1][2] == ue[1][0]) || (ue[1][2] == ue[0][1]) || (ue[1][2] == ue[1][1]))
	  pre_nb_available_rbs[ue[1][2]] = 8;
	else
	  pre_nb_available_rbs[ue[1][2]] = 4;
      rballoc_sub[ue[0][2]][2] = 1;
      rballoc_sub[ue[1][2]][2] = 1;
    }

  if(k3 == 1)
    {
      dl_pow_off[ue[0][3]] = 0;
      dl_pow_off[ue[1][3]] = 0;
      MIMO_mode_indicator[3] = 0;

      if(((ue[0][3] == ue[0][0])|| (ue[0][3] == ue[1][0]))&&
	 ((ue[0][3] == ue[0][1])|| (ue[0][3] == ue[1][1]))&&
	 ((ue[0][3] == ue[0][2])|| (ue[0][3] == ue[1][2])))
	pre_nb_available_rbs[ue[0][3]] = 16;
      else
	if(((ue[0][3] == ue[0][0]) || (ue[0][3] == ue[1][0]))&&
	   ((ue[0][3] == ue[0][1])|| (ue[0][3] == ue[1][1])))
	  pre_nb_available_rbs[ue[0][3]] = 12;
	else
	  if(((ue[0][3] == ue[0][0]) || (ue[0][3] == ue[1][0]))&&
	     ((ue[0][3] == ue[0][2])|| (ue[0][3] == ue[1][2])))
	    pre_nb_available_rbs[ue[0][3]] = 12;
	  else
	    if(((ue[0][3] == ue[0][1]) || (ue[0][3] == ue[1][1]))&&
	       ((ue[0][3] == ue[0][2])|| (ue[0][3] == ue[1][2])))
	      pre_nb_available_rbs[ue[0][3]] = 12;
	    else
	      if((ue[0][3] == ue[0][0])|| (ue[0][3] == ue[1][0])||
		 (ue[0][3] == ue[0][1])|| (ue[0][3] == ue[1][1])||
		 (ue[0][3] == ue[0][2])|| (ue[0][3] == ue[1][2]))
		pre_nb_available_rbs[ue[0][3]] = 8;
	      else
		pre_nb_available_rbs[ue[0][3]] = 4;

      if(((ue[1][3] == ue[0][0])|| (ue[1][3] == ue[1][0]))&&
	 ((ue[1][3] == ue[0][1])|| (ue[1][3] == ue[1][1]))&&
	 ((ue[1][3] == ue[0][2])|| (ue[1][3] == ue[1][2])))
	pre_nb_available_rbs[ue[1][3]] = 16;
      else
	if(((ue[1][3] == ue[0][0]) || (ue[1][3] == ue[1][0]))&&
	   ((ue[1][3] == ue[0][1])|| (ue[1][3] == ue[1][1])))
	  pre_nb_available_rbs[ue[1][3]] = 12;
	else
	  if(((ue[1][3] == ue[0][0]) || (ue[1][3] == ue[1][0]))&&
	     ((ue[1][3] == ue[0][2])|| (ue[1][3] == ue[1][2])))
	    pre_nb_available_rbs[ue[1][3]] = 12;
	  else
	    if(((ue[1][3] == ue[0][1]) || (ue[1][3] == ue[1][1]))&&
	       ((ue[1][3] == ue[0][2])|| (ue[1][3] == ue[1][2])))
	      pre_nb_available_rbs[ue[1][3]] = 12;
	    else
	      if((ue[1][3] == ue[0][0])|| (ue[1][3] == ue[1][0])||
		 (ue[1][3] == ue[0][1])|| (ue[1][3] == ue[1][1])||
		 (ue[1][3] == ue[0][2])|| (ue[1][3] == ue[1][2]))
		pre_nb_available_rbs[ue[1][3]] = 8;
	      else
		pre_nb_available_rbs[ue[1][3]] = 4;

      rballoc_sub[ue[0][3]][3] = 1;
      rballoc_sub[ue[1][3]][3] = 1;
    }

  if(k4 == 1)
    {
      dl_pow_off[ue[0][4]] = 0;
      dl_pow_off[ue[1][4]] = 0;
      MIMO_mode_indicator[4] = 0;

      if(((ue[0][4] == ue[0][0])|| (ue[0][4] == ue[1][0]))&&
	 ((ue[0][4] == ue[0][1])|| (ue[0][4] == ue[1][1])) &&
	 ((ue[0][4] == ue[0][2])|| (ue[0][4] == ue[1][2])) &&
	 ((ue[0][4] == ue[0][3])|| (ue[0][4] == ue[1][3])))
	pre_nb_available_rbs[ue[0][4]] = 20;
      else
	if(((ue[0][4] == ue[0][0])|| (ue[0][4] == ue[1][0]))&&
	   ((ue[0][4] == ue[0][1])|| (ue[0][4] == ue[1][1])) &&
	   ((ue[0][4] == ue[0][2])|| (ue[0][4] == ue[1][2])))
	  pre_nb_available_rbs[ue[0][4]] = 16;
	else
	  if(((ue[0][4] == ue[0][0])|| (ue[0][4] == ue[1][0]))&&
	     ((ue[0][4] == ue[0][1])|| (ue[0][4] == ue[1][1])) &&
	     ((ue[0][4] == ue[0][3])|| (ue[0][4] == ue[1][3])))
	    pre_nb_available_rbs[ue[0][4]] = 16;
	  else
	    if(((ue[0][4] == ue[0][0])|| (ue[0][4] == ue[1][0]))&&
	       ((ue[0][4] == ue[0][2])|| (ue[0][4] == ue[1][2])) &&
	       ((ue[0][4] == ue[0][3])|| (ue[0][4] == ue[1][3])))
	      pre_nb_available_rbs[ue[0][4]] = 16;
	    else
	      if(((ue[0][4] == ue[0][1])|| (ue[0][4] == ue[1][1]))&&
		 ((ue[0][4] == ue[0][2])|| (ue[0][4] == ue[1][2])) &&
		 ((ue[0][4] == ue[0][3])|| (ue[0][4] == ue[1][3])))
		pre_nb_available_rbs[ue[0][4]] = 16;
	      else
		if((((ue[0][4] == ue[0][0]) || (ue[0][4] == ue[1][0])) && ((ue[0][4] == ue[0][1]) || (ue[0][4] == ue[1][1]))) ||
		   (((ue[0][4] == ue[0][0]) || (ue[0][4] == ue[1][0])) && ((ue[0][4] == ue[0][2]) || (ue[0][4] == ue[1][2]))) ||
		   (((ue[0][4] == ue[0][0]) || (ue[0][4] == ue[1][0])) && ((ue[0][4] == ue[0][3]) || (ue[0][4] == ue[1][3]))) ||
		   (((ue[0][4] == ue[0][1]) || (ue[0][4] == ue[1][1])) && ((ue[0][4] == ue[0][2]) || (ue[0][4] == ue[1][2]))) ||
		   (((ue[0][4] == ue[0][1]) || (ue[0][4] == ue[1][1])) && ((ue[0][4] == ue[0][3]) || (ue[0][4] == ue[1][3]))) ||
		   (((ue[0][4] == ue[0][2]) || (ue[0][4] == ue[1][2])) && ((ue[0][4] == ue[0][3]) || (ue[0][4] == ue[1][3]))))
		  pre_nb_available_rbs[ue[0][4]] = 12;
		else
		  if((ue[0][4] == ue[0][0]) || (ue[0][4] == ue[1][0]) ||
		     (ue[0][4] == ue[0][1]) || (ue[0][4] == ue[1][1]) ||
		     (ue[0][4] == ue[0][2]) || (ue[0][4] == ue[1][2]) ||
		     (ue[0][4] == ue[0][3]) || (ue[0][4] == ue[1][3]))
		    pre_nb_available_rbs[ue[0][4]] = 8;
		  else
		    pre_nb_available_rbs[ue[0][4]] = 4;

      if(((ue[1][4] == ue[0][0])|| (ue[1][4] == ue[1][0]))&&
	 ((ue[1][4] == ue[0][1])|| (ue[1][4] == ue[1][1])) &&
	 ((ue[1][4] == ue[0][2])|| (ue[1][4] == ue[1][2])) &&
	 ((ue[1][4] == ue[0][3])|| (ue[1][4] == ue[1][3])))
	pre_nb_available_rbs[ue[1][4]] = 20;
      else
	if(((ue[1][4] == ue[0][0])|| (ue[1][4] == ue[1][0]))&&
	   ((ue[1][4] == ue[0][1])|| (ue[1][4] == ue[1][1])) &&
	   ((ue[1][4] == ue[0][2])|| (ue[1][4] == ue[1][2])))
	  pre_nb_available_rbs[ue[1][4]] = 16;
	else
	  if(((ue[1][4] == ue[0][0])|| (ue[1][4] == ue[1][0]))&&
	     ((ue[1][4] == ue[0][1])|| (ue[1][4] == ue[1][1])) &&
	     ((ue[1][4] == ue[0][3])|| (ue[1][4] == ue[1][3])))
	    pre_nb_available_rbs[ue[1][4]] = 16;
	  else
	    if(((ue[1][4] == ue[0][0])|| (ue[1][4] == ue[1][0]))&&
	       ((ue[1][4] == ue[0][2])|| (ue[1][4] == ue[1][2])) &&
	       ((ue[1][4] == ue[0][3])|| (ue[1][4] == ue[1][3])))
	      pre_nb_available_rbs[ue[1][4]] = 16;
	    else
	      if(((ue[1][4] == ue[0][1])|| (ue[1][4] == ue[1][1]))&&
		 ((ue[1][4] == ue[0][2])|| (ue[1][4] == ue[1][2])) &&
		 ((ue[1][4] == ue[0][3])|| (ue[1][4] == ue[1][3])))
		pre_nb_available_rbs[ue[1][4]] = 16;
	      else
		if((((ue[1][4] == ue[0][0]) || (ue[1][4] == ue[1][0])) && ((ue[1][4] == ue[0][1]) || (ue[1][4] == ue[1][1]))) ||
		   (((ue[1][4] == ue[0][0]) || (ue[1][4] == ue[1][0])) && ((ue[1][4] == ue[0][2]) || (ue[1][4] == ue[1][2]))) ||
		   (((ue[1][4] == ue[0][0]) || (ue[1][4] == ue[1][0])) && ((ue[1][4] == ue[0][3]) || (ue[1][4] == ue[1][3]))) ||
		   (((ue[1][4] == ue[0][1]) || (ue[1][4] == ue[1][1])) && ((ue[1][4] == ue[0][2]) || (ue[1][4] == ue[1][2]))) ||
		   (((ue[1][4] == ue[0][1]) || (ue[1][4] == ue[1][1])) && ((ue[1][4] == ue[0][3]) || (ue[1][4] == ue[1][3]))) ||
		   (((ue[1][4] == ue[0][2]) || (ue[1][4] == ue[1][2])) && ((ue[1][4] == ue[0][3]) || (ue[1][4] == ue[1][3]))))
		  pre_nb_available_rbs[ue[1][4]] = 12;
		else
		  if((ue[1][4] == ue[0][0]) || (ue[1][4] == ue[1][0]) ||
		     (ue[1][4] == ue[0][1]) || (ue[1][4] == ue[1][1]) ||
		     (ue[1][4] == ue[0][2]) || (ue[1][4] == ue[1][2]) ||
		     (ue[1][4] == ue[0][3]) || (ue[1][4] == ue[1][3]))
		    pre_nb_available_rbs[ue[1][4]] = 8;
		  else
		    pre_nb_available_rbs[ue[1][4]] = 4;

      rballoc_sub[ue[0][4]][4] = 1;
      rballoc_sub[ue[1][4]][4] = 1;

    }


  if(k5 == 1)
    {
      dl_pow_off[ue[0][5]] = 0;
      dl_pow_off[ue[1][5]] = 0;
      MIMO_mode_indicator[5] = 0;

      if(((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
	 ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	 ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	 ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) &&
	 ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))
	pre_nb_available_rbs[ue[0][5]] = 24;
      else
	if((((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	    ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) && ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])))||
	   (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	    ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
	   (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	    ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
	   (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	    ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
	   (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) && ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	    ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))))
	  pre_nb_available_rbs[ue[0][5]] = 20;
	else
	  if((((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
	      ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	      ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2]))) ||
	     (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
	      ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	      ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3]))) ||
	     (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
	      ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	      ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))) ||
	     (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
	      ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	      ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))) ||
	     (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
	      ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	      ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3]))) ||
	     (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
	      ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) &&
	      ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))) ||
	     (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	      ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	      ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3]))) ||
	     (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	      ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	      ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))) ||
	     (((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	      ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) &&
	      ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))) ||
	     (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	      ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) &&
	      ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))))
	    pre_nb_available_rbs[ue[0][5]] = 16;
	  else
	    if((((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])))||
	       (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])))||
	       (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])))||
	       (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
	       (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) && ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])))||
	       (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) && ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])))||
	       (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
	       (((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) && ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])))||
	       (((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
	       (((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))))
	      pre_nb_available_rbs[ue[0][5]] = 12;
	    else
	      if((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0]) ||
		 (ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1]) ||
		 (ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2]) ||
		 (ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3]) ||
		 (ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))
		pre_nb_available_rbs[ue[0][5]] = 8;
	      else
		pre_nb_available_rbs[ue[0][5]] = 4;



      if(((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) &&
	 ((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) &&
	 ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) &&
	 ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])) &&
	 ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4])))
	pre_nb_available_rbs[ue[1][5]] = 24;
      else
	if((((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) && ((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) &&
	    ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) && ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])))||
	   (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) && ((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) &&
	    ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) && ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4])))||
	   (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) && ((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) &&
	    ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])) && ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4])))||
	   (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) && ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) &&
	    ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])) && ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4])))||
	   (((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) && ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) &&
	    ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])) && ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4]))))
	  pre_nb_available_rbs[ue[1][5]] = 20;
	else
	  if((((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) &&
	      ((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) &&
	      ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2]))) ||
	     (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) &&
	      ((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) &&
	      ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3]))) ||
	     (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) &&
	      ((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) &&
	      ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4]))) ||
	     (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) &&
	      ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) &&
	      ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4]))) ||
	     (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) &&
	      ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) &&
	      ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3]))) ||
	     (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) &&
	      ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])) &&
	      ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4]))) ||
	     (((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) &&
	      ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) &&
	      ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3]))) ||
	     (((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) &&
	      ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) &&
	      ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4]))) ||
	     (((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) &&
	      ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])) &&
	      ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4]))) ||
	     (((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) &&
	      ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])) &&
	      ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4]))))
	    pre_nb_available_rbs[ue[1][5]] = 16;
	  else
	    if((((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) && ((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])))||
	       (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) && ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])))||
	       (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) && ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])))||
	       (((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0])) && ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4])))||
	       (((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) && ((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])))||
	       (((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) && ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])))||
	       (((ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1])) && ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4])))||
	       (((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) && ((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])))||
	       (((ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2])) && ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4])))||
	       (((ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3])) && ((ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4]))))
	      pre_nb_available_rbs[ue[1][5]] = 12;
	    else
	      if((ue[1][5] == ue[0][0]) || (ue[1][5] == ue[1][0]) ||
		 (ue[1][5] == ue[0][1]) || (ue[1][5] == ue[1][1]) ||
		 (ue[1][5] == ue[0][2]) || (ue[1][5] == ue[1][2]) ||
		 (ue[1][5] == ue[0][3]) || (ue[1][5] == ue[1][3]) ||
		 (ue[1][5] == ue[0][4]) || (ue[1][5] == ue[1][4]))
		pre_nb_available_rbs[ue[1][5]] = 8;
	      else
		pre_nb_available_rbs[ue[1][5]] = 4;

      rballoc_sub[ue[0][5]][5] = 1;
      rballoc_sub[ue[1][5]][5] = 1;
    }

  if(k6 == 1)
    {
      dl_pow_off[ue[0][6]] = 0;
      dl_pow_off[ue[1][6]] = 0;
      MIMO_mode_indicator[6] = 0;

      if(((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	 ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	 ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	 ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	 ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
	 ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))
	pre_nb_available_rbs[ue[0][6]] = 25;
      else
	if((((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	    ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	    ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	    ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	    ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])))||
	   (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	    ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	    ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	    ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	    ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	   (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	    ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	    ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	    ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
	    ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	   (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	    ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	    ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	    ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
	    ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	   (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	    ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	    ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	    ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
	    ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	   (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	    ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	    ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	    ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
	    ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))))
	  pre_nb_available_rbs[ue[0][6]] = 21;
	else
	  if((((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))))
	    pre_nb_available_rbs[ue[0][6]] = 17;
	  else
	    if((((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2]))) ||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
		((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
	       (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
	       (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
	       (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
	       (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
	       (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
		((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
	       (((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
		((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
	       (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
	       (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
	       (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
		((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
	       (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))))
	      pre_nb_available_rbs[ue[0][6]] = 13;
	    else
	      if((((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2]))) ||
		 (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
		 (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		 (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
		 (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		 (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		 (((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))))
		pre_nb_available_rbs[ue[0][6]] = 9;
	      else
		if((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0]) ||
		   (ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1]) ||
		   (ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2]) ||
		   (ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]) ||
		   (ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]) ||
		   (ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))
		  pre_nb_available_rbs[ue[0][6]] = 5;
		else
		  pre_nb_available_rbs[ue[0][6]] = 1;



      if(((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) &&
	 ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	 ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	 ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
	 ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) &&
	 ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))
	pre_nb_available_rbs[ue[1][6]] = 25;
      else
	if((((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) &&
	    ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	    ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	    ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
	    ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])))||
	   (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) &&
	    ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	    ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	    ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
	    ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	   (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) &&
	    ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	    ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
	    ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) &&
	    ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	   (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) &&
	    ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	    ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
	    ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) &&
	    ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	   (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	    ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	    ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
	    ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) &&
	    ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	   (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) &&
	    ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	    ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	    ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) &&
	    ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))))
	  pre_nb_available_rbs[ue[1][6]] = 21;
	else
	  if((((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	      ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])))||
	     (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	      ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])))||
	     (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	      ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	     (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	      ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	     (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	      ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])))||
	     (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	      ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])))||
	     (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	      ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	     (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	      ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	     (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	      ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])))||
	     (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
	      ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	     (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
	      ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	     (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	      ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	     (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
	      ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	     (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
	      ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5])))||
	     (((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
	      ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))))
	    pre_nb_available_rbs[ue[1][6]] = 17;
	  else
	    if((((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
		((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2]))) ||
	       (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
		((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3]))) ||
	       (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
		((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]))) ||
	       (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) &&
		((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
	       (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
		((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3]))) ||
	       (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
		((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]))) ||
	       (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
		((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
	       (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
		((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]))) ||
	       (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
		((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
	       (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) &&
		((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
	       (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
		((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3]))) ||
	       (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
		((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]))) ||
	       (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) &&
		((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
	       (((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
		((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]))) ||
	       (((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) &&
		((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
	       (((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) &&
		((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
	       (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
		((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]))) ||
	       (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
		((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
	       (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) &&
		((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
	       (((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) &&
		((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))))
	      pre_nb_available_rbs[ue[1][6]] = 13;
	    else
	      if((((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1]))) ||
		 (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2]))) ||
		 (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3]))) ||
		 (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]))) ||
		 (((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
		 (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2]))) ||
		 (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3]))) ||
		 (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]))) ||
		 (((ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
		 (((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) && ((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3]))) ||
		 (((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]))) ||
		 (((ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
		 (((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) && ((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]))) ||
		 (((ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))) ||
		 (((ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4])) && ((ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))))
		pre_nb_available_rbs[ue[1][6]] = 9;
	      else
		if((ue[1][6] == ue[0][0]) || (ue[1][6] == ue[1][0]) ||
		   (ue[1][6] == ue[0][1]) || (ue[1][6] == ue[1][1]) ||
		   (ue[1][6] == ue[0][2]) || (ue[1][6] == ue[1][2]) ||
		   (ue[1][6] == ue[0][3]) || (ue[1][6] == ue[1][3]) ||
		   (ue[1][6] == ue[0][4]) || (ue[1][6] == ue[1][4]) ||
		   (ue[1][6] == ue[0][5]) || (ue[1][6] == ue[1][5]))
		  pre_nb_available_rbs[ue[1][6]] = 5;
		else
		  pre_nb_available_rbs[ue[1][6]] = 1;

      rballoc_sub[ue[0][6]][6] = 1;
      rballoc_sub[ue[1][6]][6] = 1;

    }




  if (k0!=1)
    {

      for (UE_id=0;UE_id<granted_UEs;UE_id++)
	{
	  if ((nb_available_rb == 0) || (nCCE < aggregation))
	    break;

	  next_ue = UE_id;
	  // If nobody is left, exit while loop and go to next step
	  if (next_ue == 255)
	    break;

	  // This is an allocated UE_id
	  rnti = find_UE_RNTI(Mod_id,next_ue);
	  eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

	  if (eNB_UE_stats==NULL)
	    mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");



	  switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	  case 1:break;
	  case 2:break;
	  case 4:break;
	  case 5:
	    // Get candidate harq_pid from PHY
	    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid_temp,&round_temp,0);

	    // if(round_temp>0)
	    // break;
	    //else
	    // {
	    if(dl_pow_off[next_ue] != 0){
	      if(i0 == 1)
		{
		  rnti_temp = find_UE_RNTI(Mod_id,ue[0][0]);
		  eNB_UE_stats_temp = mac_xface->get_eNB_UE_stats(Mod_id,rnti_temp);
		  if(eNB_UE_stats->DL_cqi[0] > eNB_UE_stats_temp->DL_cqi[0])
		    ue[0][0] = next_ue;
		}
	      else
		{
		  ue[0][0] = next_ue;
		  i0 = 1;
		}
	    }
	    //}
	  }
	}
      if(i0 == 1){
	dl_pow_off[ue[0][0]] = 1;
	pre_nb_available_rbs[ue[0][0]] = 4;
	rballoc_sub[ue[0][0]][0] = 1;
	MIMO_mode_indicator[0] = 1;
      }
    }






  if(k1!=1)
    {

      for (UE_id=0;UE_id<granted_UEs;UE_id++)
	{
	  if ((nb_available_rb == 0) || (nCCE < aggregation))
	    break;

	  next_ue = UE_id;
	  // If nobody is left, exit while loop and go to next step
	  if (next_ue == 255)
	    break;

	  // This is an allocated UE_id
	  rnti = find_UE_RNTI(Mod_id,next_ue);
	  eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

	  if (eNB_UE_stats==NULL)
	    mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");

	  switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	  case 1:break;
	  case 2:break;
	  case 4:break;
	  case 5:

	    // Get candidate harq_pid from PHY
	    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid_temp,&round_temp,0);

	    //   if(round_temp>0)
	    //  break;
	    //else
	    // {
	    if(dl_pow_off[next_ue] != 0){
	      if(i1 == 1)
		{
		  rnti_temp = find_UE_RNTI(Mod_id,ue[0][1]);
		  eNB_UE_stats_temp = mac_xface->get_eNB_UE_stats(Mod_id,rnti_temp);
		  if(eNB_UE_stats->DL_cqi[0] > eNB_UE_stats_temp->DL_cqi[0])
		    ue[0][1] = next_ue;
		}
	      else
		{
		  ue[0][1] = next_ue;
		  i1 = 1;
		}
	    }
	    //}
	  }
	}
      if(i1 == 1){
	if ((ue[0][1] == ue[0][0]) || (ue[0][1] == ue[1][0]))
	  pre_nb_available_rbs[ue[0][1]]  = 8;
	else
	  pre_nb_available_rbs[ue[0][1]] = 4;

	dl_pow_off[ue[0][1]] = 1;
	rballoc_sub[ue[0][1]][1] = 1;
	MIMO_mode_indicator[1] = 1;
      }
    }






  if(k2!=1)
    {
      for (UE_id=0;UE_id<granted_UEs;UE_id++)
	{
	  if ((nb_available_rb == 0) || (nCCE < aggregation))
	    break;

	  next_ue = UE_id;
	  // If nobody is left, exit while loop and go to next step
	  if (next_ue == 255)
	    break;

	  // This is an allocated UE_id
	  rnti = find_UE_RNTI(Mod_id,next_ue);
	  eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

	  if (eNB_UE_stats==NULL)
	    mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");


	  switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	  case 1:break;
	  case 2:break;
	  case 4:break;
	  case 5:

	    // Get candidate harq_pid from PHY
	    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid_temp,&round_temp,0);

	    //    if(round_temp>0)
	    // break;
	    //else
	    // {
	    if(dl_pow_off[next_ue] != 0){
	      if(i2 == 1)
		{
		  rnti_temp = find_UE_RNTI(Mod_id,ue[0][2]);
		  eNB_UE_stats_temp = mac_xface->get_eNB_UE_stats(Mod_id,rnti_temp);
		  if(eNB_UE_stats->DL_cqi[0] > eNB_UE_stats_temp->DL_cqi[0])
		    ue[0][2] = next_ue;
		}
	      else
		{
		  ue[0][2] = next_ue;
		  i2 = 1;
		}
	    }
	    // }
	  }
	}
      if(i2 == 1){
	if (((ue[0][2] == ue[0][0])|| (ue[0][2] == ue[1][0]))&&((ue[0][2] == ue[0][1])|| (ue[0][2] == ue[1][1])))
	  pre_nb_available_rbs[ue[0][2]] = 12;
	else
	  if((ue[0][2] == ue[0][0]) || (ue[0][2] == ue[1][0]) || (ue[0][2] == ue[0][1]) || (ue[0][2] == ue[1][1]))
	    pre_nb_available_rbs[ue[0][2]] = 8;
	  else
	    pre_nb_available_rbs[ue[0][2]] = 4;
	dl_pow_off[ue[0][2]] = 1;
	rballoc_sub[ue[0][2]][2] = 1;
	MIMO_mode_indicator[2] = 1;
      }
    }



  if(k3!=1)
    {
      for (UE_id=0;UE_id<granted_UEs;UE_id++)
	{
	  if ((nb_available_rb == 0) || (nCCE < aggregation))
	    break;

	  next_ue = UE_id;
	  // If nobody is left, exit while loop and go to next step
	  if (next_ue == 255)
	    break;

	  // This is an allocated UE_id
	  rnti = find_UE_RNTI(Mod_id,next_ue);
	  eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

	  if (eNB_UE_stats==NULL)
	    mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");

	  switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	  case 1:break;
	  case 2:break;
	  case 4:break;
	  case 5:

	    // Get candidate harq_pid from PHY
	    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid_temp,&round_temp,0);

	    //    if(round_temp>0)
	    //  break;
	    //else
	    //{
	    if(dl_pow_off[next_ue] != 0){
	      if(i3 == 1)
		{
		  rnti_temp = find_UE_RNTI(Mod_id,ue[0][3]);
		  eNB_UE_stats_temp = mac_xface->get_eNB_UE_stats(Mod_id,rnti_temp);
		  if(eNB_UE_stats->DL_cqi[0] > eNB_UE_stats_temp->DL_cqi[0])
		    ue[0][3] = next_ue;
		}
	      else
		{
		  ue[0][3] = next_ue;
		  i3 = 1;
		}
	    }
	    // }
	  }
	}
      if(i3 == 1){
	if(((ue[0][3] == ue[0][0])|| (ue[0][3] == ue[1][0]))&&
	   ((ue[0][3] == ue[0][1])|| (ue[0][3] == ue[1][1]))&&
	   ((ue[0][3] == ue[0][2])|| (ue[0][3] == ue[1][2])))
	  pre_nb_available_rbs[ue[0][3]] = 16;
	else
	  if(((ue[0][3] == ue[0][0]) || (ue[0][3] == ue[1][0]))&&
	     ((ue[0][3] == ue[0][1])|| (ue[0][3] == ue[1][1])))
	    pre_nb_available_rbs[ue[0][3]] = 12;
	  else
	    if(((ue[0][3] == ue[0][0]) || (ue[0][3] == ue[1][0]))&&
	       ((ue[0][3] == ue[0][2])|| (ue[0][3] == ue[1][2])))
	      pre_nb_available_rbs[ue[0][3]] = 12;
	    else
	      if(((ue[0][3] == ue[0][1]) || (ue[0][3] == ue[1][1]))&&
		 ((ue[0][3] == ue[0][2])|| (ue[0][3] == ue[1][2])))
		pre_nb_available_rbs[ue[0][3]] = 12;
	      else
		if((ue[0][3] == ue[0][0])|| (ue[0][3] == ue[1][0])||
		   (ue[0][3] == ue[0][1])|| (ue[0][3] == ue[1][1])||
		   (ue[0][3] == ue[0][2])|| (ue[0][3] == ue[1][2]))
		  pre_nb_available_rbs[ue[0][3]] = 8;
		else
		  pre_nb_available_rbs[ue[0][3]] = 4;
	dl_pow_off[ue[0][3]] = 1;
	rballoc_sub[ue[0][3]][3] = 1;
	MIMO_mode_indicator[3] = 1;
      }
    }



  if(k4!=1)
    {
      for (UE_id=0;UE_id<granted_UEs;UE_id++)
	{
	  if ((nb_available_rb == 0) || (nCCE < aggregation))
	    break;

	  next_ue = UE_id;
	  // If nobody is left, exit while loop and go to next step
	  if (next_ue == 255)
	    break;

	  // This is an allocated UE_id
	  rnti = find_UE_RNTI(Mod_id,next_ue);
	  eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

	  if (eNB_UE_stats==NULL)
	    mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");

	  switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	  case 1:break;
	  case 2:break;
	  case 4:break;
	  case 5:
	    // Get candidate harq_pid from PHY
	    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid_temp,&round_temp,0);

	    //   if(round_temp>0)
	    // break;
	    //else
	    // {
	    if(dl_pow_off[next_ue] != 0){
	      if(i4 == 1)
		{
		  rnti_temp = find_UE_RNTI(Mod_id,ue[0][4]);
		  eNB_UE_stats_temp = mac_xface->get_eNB_UE_stats(Mod_id,rnti_temp);
		  if(eNB_UE_stats->DL_cqi[0] > eNB_UE_stats_temp->DL_cqi[0])
		    ue[0][4] = next_ue;
		}
	      else
		{
		  ue[0][4] = next_ue;
		  i4 = 1;
		}
	    }
	    // }
	  }
	}
      if(i4 == 1){
	if(((ue[0][4] == ue[0][0])|| (ue[0][4] == ue[1][0]))&&
	   ((ue[0][4] == ue[0][1])|| (ue[0][4] == ue[1][1])) &&
	   ((ue[0][4] == ue[0][2])|| (ue[0][4] == ue[1][2])) &&
	   ((ue[0][4] == ue[0][3]) || (ue[0][4] == ue[1][3])))
	  pre_nb_available_rbs[ue[0][4]] = 20;
	else
	  if(((ue[0][4] == ue[0][0])|| (ue[0][4] == ue[1][0]))&&
	     ((ue[0][4] == ue[0][1])|| (ue[0][4] == ue[1][1])) &&
	     ((ue[0][4] == ue[0][2])|| (ue[0][4] == ue[1][2])))
	    pre_nb_available_rbs[ue[0][4]] = 16;
	  else
	    if(((ue[0][4] == ue[0][0])|| (ue[0][4] == ue[1][0]))&&
	       ((ue[0][4] == ue[0][1])|| (ue[0][4] == ue[1][1])) &&
	       ((ue[0][4] == ue[0][3])|| (ue[0][4] == ue[1][3])))
	      pre_nb_available_rbs[ue[0][4]] = 16;
	    else
	      if(((ue[0][4] == ue[0][0])|| (ue[0][4] == ue[1][0]))&&
		 ((ue[0][4] == ue[0][2])|| (ue[0][4] == ue[1][2])) &&
		 ((ue[0][4] == ue[0][3])|| (ue[0][4] == ue[1][3])))
		pre_nb_available_rbs[ue[0][4]] = 16;
	      else
		if(((ue[0][4] == ue[0][1])|| (ue[0][4] == ue[1][1]))&&
		   ((ue[0][4] == ue[0][2])|| (ue[0][4] == ue[1][2])) &&
		   ((ue[0][4] == ue[0][3])|| (ue[0][4] == ue[1][3])))
		  pre_nb_available_rbs[ue[0][4]] = 16;
		else
		  if((((ue[0][4] == ue[0][0]) || (ue[0][4] == ue[1][0])) && ((ue[0][4] == ue[0][1]) || (ue[0][4] == ue[1][1]))) ||
		     (((ue[0][4] == ue[0][0]) || (ue[0][4] == ue[1][0])) && ((ue[0][4] == ue[0][2]) || (ue[0][4] == ue[1][2]))) ||
		     (((ue[0][4] == ue[0][0]) || (ue[0][4] == ue[1][0])) && ((ue[0][4] == ue[0][3]) || (ue[0][4] == ue[1][3]))) ||
		     (((ue[0][4] == ue[0][1]) || (ue[0][4] == ue[1][1])) && ((ue[0][4] == ue[0][2]) || (ue[0][4] == ue[1][2]))) ||
		     (((ue[0][4] == ue[0][1]) || (ue[0][4] == ue[1][1])) && ((ue[0][4] == ue[0][3]) || (ue[0][4] == ue[1][3]))) ||
		     (((ue[0][4] == ue[0][2]) || (ue[0][4] == ue[1][2])) && ((ue[0][4] == ue[0][3]) || (ue[0][4] == ue[1][3]))))
		    pre_nb_available_rbs[ue[0][4]] = 12;
		  else
		    if((ue[0][4] == ue[0][0]) || (ue[0][4] == ue[1][0]) ||
		       (ue[0][4] == ue[0][1]) || (ue[0][4] == ue[1][1]) ||
		       (ue[0][4] == ue[0][2]) || (ue[0][4] == ue[1][2]) ||
		       (ue[0][4] == ue[0][3]) || (ue[0][4] == ue[1][3]))
		      pre_nb_available_rbs[ue[0][4]] = 8;
		    else
		      pre_nb_available_rbs[ue[0][4]] = 4;
	dl_pow_off[ue[0][4]] = 1;
	rballoc_sub[ue[0][4]][4] = 1;
	MIMO_mode_indicator[4] = 1;
      }
    }




  if(k5!=1)
    {
      for (UE_id=0;UE_id<granted_UEs;UE_id++)
	{
	  if ((nb_available_rb == 0) || (nCCE < aggregation))
	    break;

	  next_ue = UE_id;
	  // If nobody is left, exit while loop and go to next step
	  if (next_ue == 255)
	    break;

	  // This is an allocated UE_id
	  rnti = find_UE_RNTI(Mod_id,next_ue);
	  eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

	  if (eNB_UE_stats==NULL)
	    mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");

	  switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	  case 1:break;
	  case 2:break;
	  case 4:break;
	  case 5:

	    // Get candidate harq_pid from PHY
	    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid_temp,&round_temp,0);

	    //  if(round_temp>0)
	    // break;
	    //else
	    //  {
	    if(dl_pow_off[next_ue] != 0){
	      if(i5 == 1)
		{
		  rnti_temp = find_UE_RNTI(Mod_id,ue[0][5]);
		  eNB_UE_stats_temp = mac_xface->get_eNB_UE_stats(Mod_id,rnti_temp);
		  if(eNB_UE_stats->DL_cqi[0] > eNB_UE_stats_temp->DL_cqi[0])
		    ue[0][5] = next_ue;
		}
	      else
		{
		  ue[0][5] = next_ue;
		  i5 = 1;
		}
	    }
	    //}
	  }
	}
      if(i5 == 1){
	if(((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
	   ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	   ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	   ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) &&
	   ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))
	  pre_nb_available_rbs[ue[0][5]] = 24;
	else
	  if((((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	      ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) && ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])))||
	     (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	      ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
	     (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
	      ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
	     (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	      ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
	     (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) && ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
	      ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))))
	    pre_nb_available_rbs[ue[0][5]] = 20;
	  else
	    if((((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
		((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
		((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2]))) ||
	       (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
		((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
		((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3]))) ||
	       (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
		((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
		((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))) ||
	       (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
		((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
		((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))) ||
	       (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
		((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
		((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3]))) ||
	       (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) &&
		((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) &&
		((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))) ||
	       (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
		((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
		((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3]))) ||
	       (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
		((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
		((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))) ||
	       (((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) &&
		((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) &&
		((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))) ||
	       (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) &&
		((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) &&
		((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))))
	      pre_nb_available_rbs[ue[0][5]] = 16;
	    else
	      if((((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])))||
		 (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])))||
		 (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])))||
		 (((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
		 (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) && ((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])))||
		 (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) && ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])))||
		 (((ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
		 (((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) && ((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])))||
		 (((ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4])))||
		 (((ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3])) && ((ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))))
		pre_nb_available_rbs[ue[0][5]] = 12;
	      else
		if((ue[0][5] == ue[0][0]) || (ue[0][5] == ue[1][0]) ||
		   (ue[0][5] == ue[0][1]) || (ue[0][5] == ue[1][1]) ||
		   (ue[0][5] == ue[0][2]) || (ue[0][5] == ue[1][2]) ||
		   (ue[0][5] == ue[0][3]) || (ue[0][5] == ue[1][3]) ||
		   (ue[0][5] == ue[0][4]) || (ue[0][5] == ue[1][4]))
		  pre_nb_available_rbs[ue[0][5]] = 8;
		else
		  pre_nb_available_rbs[ue[0][5]] = 4;
	dl_pow_off[ue[0][5]] = 1;
	rballoc_sub[ue[0][5]][5] = 1;
	MIMO_mode_indicator[5] = 1;
      }
    }





  if(k6!=1)
    {
      for (UE_id=0;UE_id<granted_UEs;UE_id++)
	{
	  if ((nb_available_rb == 0) || (nCCE < aggregation))
	    break;

	  next_ue = UE_id;
	  // If nobody is left, exit while loop and go to next step
	  if (next_ue == 255)
	    break;

	  // This is an allocated UE_id
	  rnti = find_UE_RNTI(Mod_id,next_ue);
	  eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

	  if (eNB_UE_stats==NULL)
	    mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");

	  switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	  case 1:break;
	  case 2:break;
	  case 4:break;
	  case 5:

	    // Get candidate harq_pid from PHY
	    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid_temp,&round_temp,0);

	    //  if(round_temp>0)
	    //  break;
	    //else
	    // {
	    if(dl_pow_off[next_ue] != 0){
	      if(i6 == 1)
		{
		  rnti_temp = find_UE_RNTI(Mod_id,ue[0][6]);
		  eNB_UE_stats_temp = mac_xface->get_eNB_UE_stats(Mod_id,rnti_temp);
		  if(eNB_UE_stats->DL_cqi[0] > eNB_UE_stats_temp->DL_cqi[0])
		    ue[0][6] = next_ue;
		}
	      else
		{
		  ue[0][6] = next_ue;
		  i6 = 1;
		}
	    }
	    // }
	  }
	}
      if(i6 == 1){
	if(((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	   ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	   ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	   ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	   ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
	   ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))
	  pre_nb_available_rbs[ue[0][6]] = 25;
	else
	  if((((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	      ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	      ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	      ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	      ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
	      ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	      ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
	      ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
	      ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	     (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) &&
	      ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
	      ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
	      ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
	      ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))))
	    pre_nb_available_rbs[ue[0][6]] = 21;
	  else
	    if((((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])))||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])))||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])))||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])))||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	       (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	       (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])))||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	       (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	       (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	       (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5])))||
	       (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))))
	      pre_nb_available_rbs[ue[0][6]] = 17;
	    else
	      if((((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		  ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		  ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		  ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) &&
		  ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		  ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		  ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		  ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		  ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		  ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
		  ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		  ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
		 (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		  ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		 (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) &&
		  ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		  ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		 (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
		  ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
		  ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		  ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		 (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		  ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) &&
		  ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		 (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) &&
		  ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))))
		pre_nb_available_rbs[ue[0][6]] = 13;
	      else
		if((((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1]))) ||
		   (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2]))) ||
		   (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
		   (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		   (((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		   (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2]))) ||
		   (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
		   (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		   (((ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		   (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]))) ||
		   (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		   (((ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		   (((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]))) ||
		   (((ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))) ||
		   (((ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4])) && ((ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))))
		  pre_nb_available_rbs[ue[0][6]] = 9;
		else
		  if((ue[0][6] == ue[0][0]) || (ue[0][6] == ue[1][0]) ||
		     (ue[0][6] == ue[0][1]) || (ue[0][6] == ue[1][1]) ||
		     (ue[0][6] == ue[0][2]) || (ue[0][6] == ue[1][2]) ||
		     (ue[0][6] == ue[0][3]) || (ue[0][6] == ue[1][3]) ||
		     (ue[0][6] == ue[0][4]) || (ue[0][6] == ue[1][4]) ||
		     (ue[0][6] == ue[0][5]) || (ue[0][6] == ue[1][5]))
		    pre_nb_available_rbs[ue[0][6]] = 5;
		  else
		    pre_nb_available_rbs[ue[0][6]] = 1;
	dl_pow_off[ue[0][6]] = 1;
	rballoc_sub[ue[0][6]][6] = 1;
	MIMO_mode_indicator[6] = 1;
      }
    }
  
    
  /*    
  for(i=0;i<7;i++){

    if(MIMO_mode_indicator[i] == 0){
      rnti0 = find_UE_RNTI(Mod_id,ue[0][i]);
      rnti1 = find_UE_RNTI(Mod_id,ue[1][i]);
      eNB_UE_stats0 = mac_xface->get_eNB_UE_stats(Mod_id,rnti0);
      eNB_UE_stats1 = mac_xface->get_eNB_UE_stats(Mod_id,rnti1);
      TBS0 = mac_xface->get_TBS(eNB_UE_stats0->DL_cqi[0],nb_available_rb);
      total_DL_cqi_MUMIMO = total_DL_cqi_MUMIMO + eNB_UE_stats0->DL_cqi[0] + eNB_UE_stats1->DL_cqi[0];
    }
    else if (MIMO_mode_indicator[i] == 1){
      rnti0 = find_UE_RNTI(Mod_id,ue[0][i]);
      eNB_UE_stats0 = mac_xface->get_eNB_UE_stats(Mod_id,rnti0);
      total_DL_cqi_SUMIMO = total_DL_cqi_SUMIMO + eNB_UE_stats0->DL_cqi[0];
    }
  }
  */



  
  if((MIMO_mode_indicator[0] == 0)|| (MIMO_mode_indicator[1] == 0) || (MIMO_mode_indicator[2] == 0) ||  (MIMO_mode_indicator[3] == 0) ||
     (MIMO_mode_indicator[4] == 0)|| (MIMO_mode_indicator[5] == 0) || (MIMO_mode_indicator[6] == 0)){
    
    
    for( UE_id = 0; UE_id < granted_UEs; UE_id++){
      next_ue = UE_id;
      rnti = find_UE_RNTI(Mod_id, next_ue);
      eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);
      TBS = mac_xface->get_TBS(eNB_UE_stats->DL_cqi[0],pre_nb_available_rbs[next_ue]);
      total_TBS_MUMIMO = TBS + total_TBS_MUMIMO;
    }
    

    for (UE_id=0;UE_id<granted_UEs;UE_id++)
      {
	next_ue = UE_id;
	// If nobody is left, exit while loop and go to next step
	if (next_ue == 255)
	  break;
	
	// This is an allocated UE_id
	rnti = find_UE_RNTI(Mod_id,next_ue);
	eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

	if (eNB_UE_stats==NULL)
	  mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");
	
	switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	case 1:break;
	case 2:break;
	case 4:break;
	case 5:
	  if(check == 1)
	    {
	      rnti_temp = find_UE_RNTI(Mod_id,UE_SU_MIMO);
	      eNB_UE_stats_temp = mac_xface->get_eNB_UE_stats(Mod_id,rnti_temp);
	      if(eNB_UE_stats->DL_cqi[0] > eNB_UE_stats_temp->DL_cqi[0]){
		UE_SU_MIMO = next_ue;
	      }
	    }
	  else
	    {
	      UE_SU_MIMO = next_ue;
	      check = 1;
	    }
	  break;
	case 6: break;
	case 7: break;
	default: break;
	}
      }
    
    rnti = find_UE_RNTI(Mod_id,UE_SU_MIMO);
    eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);
    total_TBS_SUMIMO = mac_xface->get_TBS(eNB_UE_stats->DL_cqi[0],nb_available_rb);
    if(total_TBS_SUMIMO >= total_TBS_MUMIMO){
      
      dl_pow_off[UE_SU_MIMO] = 1;
      pre_nb_available_rbs[UE_SU_MIMO] = 25;
      
      for(j=0;j<7;j++){
	rballoc_sub[UE_SU_MIMO][j] = 1;
	ue[0][j] = UE_SU_MIMO;
	MIMO_mode_indicator[j] = 1;
      }

      for(UE_id=0;UE_id<granted_UEs;UE_id++)
	{
	  if(UE_id!= UE_SU_MIMO){
	    dl_pow_off[UE_id] = 2;
	    pre_nb_available_rbs[UE_id] = 0;
	    for(jj=0;jj<7;jj++){
	      rballoc_sub[UE_id][jj]=0;
	      ue[0][jj] = 256;
	      ue[1][jj] = 256;
	    }
	  }
	}
    }
  }

  if((MIMO_mode_indicator[0] == 1)&& (MIMO_mode_indicator[1] == 1) && (MIMO_mode_indicator[2] == 1) && (MIMO_mode_indicator[3] == 1) &&
     (MIMO_mode_indicator[4] == 1)&& (MIMO_mode_indicator[5] == 1) && (MIMO_mode_indicator[6] == 1))
    PHY_vars_eNB_g[Mod_id]->check_for_SUMIMO_transmissions = PHY_vars_eNB_g[Mod_id]->check_for_SUMIMO_transmissions + 1;
  else
    if((MIMO_mode_indicator[0] == 0)|| (MIMO_mode_indicator[1] == 0) || (MIMO_mode_indicator[2] == 0) ||  (MIMO_mode_indicator[3] == 0) ||
       (MIMO_mode_indicator[4] == 0)|| (MIMO_mode_indicator[5] == 0) || (MIMO_mode_indicator[6] == 0))
      PHY_vars_eNB_g[Mod_id]->check_for_MUMIMO_transmissions = PHY_vars_eNB_g[Mod_id]->check_for_MUMIMO_transmissions + 1;

  if((MIMO_mode_indicator[0] == 0)&& (MIMO_mode_indicator[1] == 0) && (MIMO_mode_indicator[2] == 0) &&  (MIMO_mode_indicator[3] == 0) &&
     (MIMO_mode_indicator[4] == 0)&& (MIMO_mode_indicator[5] == 0) && (MIMO_mode_indicator[6] == 0))
    PHY_vars_eNB_g[Mod_id]->FULL_MUMIMO_transmissions = PHY_vars_eNB_g[Mod_id]->FULL_MUMIMO_transmissions + 1;
  
      PHY_vars_eNB_g[Mod_id]->check_for_total_transmissions = PHY_vars_eNB_g[Mod_id]->check_for_total_transmissions + 1;




  for(UE_id=0;UE_id<granted_UEs;UE_id++){
    PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].dl_pow_off = dl_pow_off[UE_id];
    //msg("******************Scheduling Information for UE%d ************************\n",UE_id);
    //msg("dl power offset UE%d = %d \n",UE_id,dl_pow_off[UE_id]);
    //msg("***********RB Alloc for every subband for UE%d ***********\n",UE_id);
    for(i=0;i<7;i++){
      PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].rballoc_sub[i] = rballoc_sub[UE_id][i];
      //msg("RB Alloc for UE%d and Subband%d = %d\n",UE_id,i,rballoc_sub[UE_id][i]);
    }
    PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].pre_nb_available_rbs = pre_nb_available_rbs[UE_id];
    //msg("Total RBs allocated for UE%d = %d\n",UE_id,pre_nb_available_rbs[UE_id]);
  }
}



void schedule_ue_spec(unsigned char Mod_id,u32 frame, unsigned char subframe,u16 nb_rb_used0,unsigned char nCCE_used) {

  unsigned char UE_id;
  unsigned char next_ue;
  unsigned char granted_UEs;
  u16 nCCE;
  unsigned char aggregation;
  mac_rlc_status_resp_t rlc_status;
  unsigned char header_len_dcch,header_len_dtch;
  unsigned char sdu_lcids[11],offset,num_sdus=0;
  u16 nb_rb,nb_available_rb,TBS,j,sdu_lengths[11],rnti,padding,post_padding;
  unsigned char dlsch_buffer[MAX_DLSCH_PAYLOAD_BYTES];
  unsigned char round=0;
  unsigned char harq_pid=0;
  void *DLSCH_dci;
  LTE_eNB_UE_stats* eNB_UE_stats;
  u16 sdu_length_total=0;
  //  unsigned char loop_count;
  unsigned char DAI;
  u16 i=0,ii=0,check=0,jj=0;
  u8 dl_pow_off[256];
  unsigned char rballoc_sub[256][7];
  u16 pre_nb_available_rbs[256];
  //int **rballoc_sub = (int **)malloc(1792*sizeof(int *));
  granted_UEs = find_dlgranted_UEs(Mod_id);
  //weight = get_ue_weight(Mod_id,UE_id);
  aggregation = 2; // set to the maximum aggregation level
  int mcs;

  for(i=0;i<256;i++)
    pre_nb_available_rbs[i] = 0;

  for(ii=0;ii<7;ii++){
    for(i=0;i<256;i++)
      rballoc_sub[i][ii] = 0;
  }
  // while frequency resources left and nCCE available
  //  for (UE_id=0;(UE_id<granted_UEs) && (nCCE > aggregation);UE_id++) {

  // set current available nb_rb and nCCE to maximum
  nb_available_rb = mac_xface->lte_frame_parms->N_RB_DL - nb_rb_used0;
  nCCE = mac_xface->get_nCCE_max(Mod_id) - nCCE_used;


  /// CALLING Pre_Processor for tm5

  tm5_pre_processor(Mod_id,subframe,nb_rb_used0,nCCE_used,dl_pow_off,pre_nb_available_rbs,rballoc_sub);

  for (UE_id=0;UE_id<granted_UEs;UE_id++) {


    rnti = find_UE_RNTI(Mod_id,UE_id);

    eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

    if (eNB_UE_stats==NULL)
      mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");

    // Get candidate harq_pid from PHY
    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);
    //    printf("Got harq_pid %d, round %d\n",harq_pid,round);

    if (mac_xface->get_transmission_mode(Mod_id,rnti)==5)
      nb_available_rb = pre_nb_available_rbs[UE_id];

    if ((nb_available_rb == 0) || (nCCE < aggregation))
      break;
    sdu_length_total=0;
    num_sdus=0;

    // get Round-Robin allocation
    next_ue = UE_id;//schedule_next_dlue(Mod_id,subframe); // next scheduled user
    // If nobody is left, exit while loop and go to next step
    if (next_ue == 255)
      break;

    switch (mac_xface->lte_frame_parms->tdd_config) {
    case 0:
      if ((subframe==0)||(subframe==1)||(subframe==3)||(subframe==5)||(subframe==6)||(subframe==8))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
    case 1:
      if ((subframe==0)||(subframe==4)||(subframe==5)||(subframe==9))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
      break;
    case 2:
      if ((subframe==4)||(subframe==5))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
      break;
    case 3:
      if ((subframe==1)||(subframe==7)||(subframe==9))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
      break;
    case 4:
      if ((subframe==0)||(subframe==6))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
      break;
    case 5:
      if (subframe==9)
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
      break;
    case 6:
      if ((subframe==0)||(subframe==1)||(subframe==5)||(subframe==6)||(subframe==9))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
    default:
      break;
    }

    // This is an allocated UE_id
    rnti = find_UE_RNTI(Mod_id,next_ue);
    if (rnti==0)
      continue;

    eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);

    if (eNB_UE_stats==NULL)
      mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");

    //eNB_UE_stats->dlsch_mcs1 = openair_daq_vars.target_ue_dl_mcs;

    eNB_UE_stats->dlsch_mcs1 = (eNB_UE_stats->DL_cqi[0]<<1);


    ///TM5 only for QPSK-QPSK IA-receiver
    /*
    while(eNB_UE_stats->dlsch_mcs1 > 9){
      //eNB_UE_stats->DL_cqi[0] = eNB_UE_stats->DL_cqi[0]-1;
      //eNB_UE_stats->dlsch_mcs1 = eNB_UE_stats->DL_cqi[0]<<1;
      eNB_UE_stats->dlsch_mcs1 = eNB_UE_stats->dlsch_mcs1 - 1;
    }
    */

    // Get candidate harq_pid from PHY
    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);
    //    printf("Got harq_pid %d, round %d\n",harq_pid,round);

    // Note this code is for a specific DCI format
    DLSCH_dci = (void *)eNB_mac_inst[Mod_id].UE_template[next_ue].DLSCH_DCI[harq_pid];

    for(i=0;i<7;i++){ // for indicating the rballoc for each sub-band
      eNB_mac_inst[Mod_id].UE_template[next_ue].rballoc_sub[harq_pid][i] = rballoc_sub[next_ue][i];
    }

    if (round > 0) {

      eNB_mac_inst[Mod_id].UE_template[next_ue].DAI++;

      // get freq_allocation
      nb_rb = eNB_mac_inst[Mod_id].UE_template[next_ue].nb_rb[harq_pid];
      if (nb_rb <= nb_available_rb) {
	nb_available_rb -= nb_rb;
	aggregation = process_ue_cqi(Mod_id,next_ue);
	nCCE-=aggregation; // adjust the remaining nCCE

	switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	case 1:
	case 2:
	default:
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->ndi      = 0;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->rv       = round&3;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->dai      = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  LOG_D(MAC,"[eNB %d] Retransmission : harq_pid %d, round %d, dai %d, mcs %d\n",Mod_id,harq_pid,round,(eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1),((DCI1_5MHz_TDD_t*)DLSCH_dci)->mcs);
	  break;
	case 4:
	  //	  if (nb_rb>10) {
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->ndi1 = 0;
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->dai = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  // }
	  //else {
	  //  ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->ndi1 = 0;
	  // ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  // ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->rv1 = round&3;
	  // ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->dai = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  // }
	  break;
	case 5:
	  // if(nb_rb>10){
	  //((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->mcs = eNB_UE_stats->DL_cqi[0]<<1;
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->ndi = 0;
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rv = round&3;
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->dai = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  if(dl_pow_off[next_ue] == 2)
	    dl_pow_off[next_ue] = 1;
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->dl_power_off = dl_pow_off[next_ue];
	  // }
	  break;
	case 6:
	  break;
	}

	add_ue_dlsch_info(Mod_id,
			  next_ue,
			  subframe,
			  S_DL_SCHEDULED);

	eNB_UE_stats->dlsch_trials[round]++;

      }
      else { // don't schedule this UE, its retransmission takes more resources than we have

      }
    }
    else {  // This is a potentially new SDU opportunity

      // calculate mcs


      // Now check RLC information to compute number of required RBs
      // get maximum TBS size for RLC request
      //TBS = mac_xface->get_TBS(eNB_UE_stats->DL_cqi[0]<<1,nb_available_rb);
      TBS = mac_xface->get_TBS(eNB_UE_stats->dlsch_mcs1,nb_available_rb);
      // check first for RLC data on DCCH
      header_len_dcch = 2+((eNB_UE_stats->UE_timing_offset>0)?2:0); // 2 bytes DCCH SDU subheader + timing advance subheader + timing advance command


    
      rlc_status = mac_rlc_status_ind(Mod_id,frame,DCCH+(MAX_NUM_RB*next_ue),
				      (TBS-header_len_dcch)); // transport block set size

      sdu_lengths[0]=0;
      if (rlc_status.bytes_in_buffer > 0) {  // There is DCCH to transmit
	LOG_D(MAC,"[eNB %d] Frame %d, DL-DCCH->DLSCH, Requesting %d bytes from RLC (RRC message)\n",Mod_id,frame,TBS-header_len_dcch);
	sdu_lengths[0] += mac_rlc_data_req(Mod_id,frame,
					 DCCH+(MAX_NUM_RB*next_ue),
					 (char *)&dlsch_buffer[sdu_lengths[0]]);

    	LOG_D(MAC,"[eNB %d][DCCH] Got %d bytes from RLC\n",Mod_id,sdu_lengths[0]);
	sdu_length_total = sdu_lengths[0];
	sdu_lcids[0] = DCCH;
	num_sdus = 1;

#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d][DCCH] Got %d bytes :",Mod_id,sdu_lengths[0]);
	for (j=0;j<sdu_lengths[0];j++)
	  msg("%x ",dlsch_buffer[j]);
	msg("\n");
#endif
      }
      else {
	header_len_dcch = 0;
	sdu_length_total = 0;
      }

      // check for DCCH1 and update header information (assume 2 byte sub-header)
      rlc_status = mac_rlc_status_ind(Mod_id,frame,DCCH+1+(MAX_NUM_RB*next_ue),
				      (TBS-header_len_dcch-sdu_length_total)); // transport block set size less allocations for timing advance and
                                                                                 // DCCH SDU


      if (rlc_status.bytes_in_buffer > 0) {
	LOG_D(MAC,"[eNB %d], Frame %d, DCCH1->DLSCH, Requesting %d bytes from RLC (RRC message)\n",
	      Mod_id,frame,TBS-header_len_dcch-sdu_length_total);
	sdu_lengths[num_sdus] += mac_rlc_data_req(Mod_id,frame,
						  DCCH+1+(MAX_NUM_RB*next_ue),
						  (char *)&dlsch_buffer[sdu_lengths[0]]);
	sdu_lcids[num_sdus] = DCCH1;
	sdu_length_total += sdu_lengths[num_sdus];
	header_len_dcch += 2;
	num_sdus++;
	LOG_D(MAC,"[eNB %d] Got %d bytes for DCCH from RLC\n",Mod_id,sdu_lengths[0]);
      }
      // check for DTCH and update header information
      // here we should loop over all possible DTCH

      header_len_dtch = 3; // 3 bytes DTCH SDU subheader

      rlc_status = mac_rlc_status_ind(Mod_id,frame,DTCH+(MAX_NUM_RB*next_ue),
				      TBS-header_len_dcch-sdu_length_total-header_len_dtch);

      if (rlc_status.bytes_in_buffer > 0) {
	
	LOG_D(MAC,"[eNB %d], Frame %d, DTCH->DLSCH, Requesting %d bytes from RLC (hdr len dtch %d)\n",
	      Mod_id,frame,TBS-header_len_dcch-sdu_length_total-header_len_dtch,header_len_dtch);
	sdu_lengths[num_sdus] = mac_rlc_data_req(Mod_id,frame,
						 DTCH+(MAX_NUM_RB*next_ue),
						 (char*)&dlsch_buffer[sdu_length_total]);
	
	LOG_D(MAC,"[eNB %d] Got %d bytes for DTCH %d \n",Mod_id,sdu_lengths[num_sdus],DTCH+(MAX_NUM_RB*next_ue));
	sdu_lcids[num_sdus] = DTCH;
	sdu_length_total += sdu_lengths[num_sdus];
	if (sdu_lengths[num_sdus] < 128)
	  header_len_dtch=2;
	num_sdus++;
      }
      else {
	header_len_dtch = 0;
      }




      if ((sdu_length_total + header_len_dcch + header_len_dtch )> 0) {

	// Now compute number of required RBs for total sdu length
	// Assume RAH format 2


	nb_rb = 2;

	mcs = eNB_UE_stats->dlsch_mcs1;
	TBS = mac_xface->get_TBS(mcs,nb_rb);


	// correct header lengths
	if (header_len_dtch==0)
	  header_len_dcch--;  // remove length field
	  
	
	while (TBS < (sdu_length_total + header_len_dcch + header_len_dtch ))  {
	  nb_rb += 2;  // 
	  if (nb_rb>mac_xface->lte_frame_parms->N_RB_DL) { // if we've gone beyond the maximum number of RBs
	    // (can happen if N_RB_DL is odd)
	    TBS = mac_xface->get_TBS(eNB_UE_stats->dlsch_mcs1,mac_xface->lte_frame_parms->N_RB_DL);
	    nb_rb = mac_xface->lte_frame_parms->N_RB_DL;
	    break;
	  }
	  TBS = mac_xface->get_TBS(eNB_UE_stats->dlsch_mcs1,nb_rb);
	}

	// decrease mcs until TBS falls below required length
	
	while ((TBS > (sdu_length_total + header_len_dcch + header_len_dtch)) && (mcs>0)) {
	  mcs--;
	  TBS = mac_xface->get_TBS(mcs,nb_rb);
	}
	if (TBS < (sdu_length_total + header_len_dcch + header_len_dtch)) {
	  mcs++;
	  TBS = mac_xface->get_TBS(mcs,nb_rb);
	}


#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] Generated DLSCH header (mcs %d, TBS %d, nb_rb %d)\n",
	    Mod_id,mcs,TBS,nb_rb);
	// msg("[MAC][eNB ] Reminder of DLSCH with random data %d %d %d %d \n",
	//	TBS, sdu_length_total, offset, TBS-sdu_length_total-offset);
#endif
	if ((TBS - header_len_dcch - header_len_dtch - sdu_length_total) <= 2) {
	  padding = (TBS - header_len_dcch - header_len_dtch - sdu_length_total);
	  post_padding = 0;
	}
	else {
	  padding = 0;
	  post_padding = TBS - sdu_length_total - header_len_dcch - header_len_dtch - 1;
	}
	offset = generate_dlsch_header((unsigned char*)eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0],
	   // offset = generate_dlsch_header((unsigned char*)eNB_mac_inst[0].DLSCH_pdu[0][0].payload[0],
				       num_sdus,              //num_sdus
				       sdu_lengths,  //
				       sdu_lcids,
				       255,                                   // no drx
				       (header_len_dcch>0)?(eNB_UE_stats->UE_timing_offset/4):0,      // timing advance
				       NULL,                                  // contention res id
				       padding,                        
				       post_padding);
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] Generate header : sdu_length_total %d, num_sdus %d, sdu_lengths[0] %d, sdu_lcids[0] %d => payload offset %d,timing advance : %d, next_ue %d,padding %d,post_padding %d,(mcs %d, TBS %d, nb_rb %d),header_dcch %d, header_dtch %d\n",
	    Mod_id,sdu_length_total,num_sdus,sdu_lengths[0],sdu_lcids[0],offset,
	    eNB_UE_stats->UE_timing_offset/4,
	    next_ue,padding,post_padding,mcs,TBS,nb_rb,header_len_dcch,header_len_dtch);
#endif
	/*	      
	msg("[MAC][eNB %d] First 16 bytes of DLSCH : \n");
	for (i=0;i<16;i++)
	  msg("%x.",dlsch_buffer[i]);
	msg("\n");
	*/
	// cycle through SDUs and place in dlsch_buffer
	memcpy(&eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0][offset],dlsch_buffer,sdu_length_total);
	// memcpy(&eNB_mac_inst[0].DLSCH_pdu[0][0].payload[0][offset],dcch_buffer,sdu_lengths[0]);

	// fill remainder of DLSCH with random data
	for (j=0;j<(TBS-sdu_length_total-offset);j++)
	  eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0][offset+sdu_length_total+j] = (char)(taus()&0xff);
	//eNB_mac_inst[0].DLSCH_pdu[0][0].payload[0][offset+sdu_lengths[0]+j] = (char)(taus()&0xff);

	aggregation = process_ue_cqi(Mod_id,next_ue);
	nCCE-=aggregation; // adjust the remaining nCCE
	eNB_mac_inst[Mod_id].UE_template[next_ue].nb_rb[harq_pid] = nb_rb;



	add_ue_dlsch_info(Mod_id,
			  next_ue,
			  subframe,
			  S_DL_SCHEDULED);
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI++;
#ifdef    DEBUG_PACKET_TRACE
	if((eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0]!=NULL)&&(eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0]!=0)){
	    trace_pdu(4,eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0],TBS/*sdu_length_total+offset offset*/, next_ue, rnti, subframe);
	  }
#endif


	switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	case 1:
	case 2:
	default:
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->mcs = mcs;
	  //if(((DCI1_5MHz_TDD_t*)DLSCH_dci)->mcs > 9)
	  //((DCI1_5MHz_TDD_t*)DLSCH_dci)->mcs = 9;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->ndi = 1;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->rv = 0;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->dai      = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  break;
	case 4:
	  //  if (nb_rb>10) {
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->mcs1 = mcs;
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->ndi1 = 1;
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rv1 = round&3;
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->dai = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;

	  //}
	  /* else {
	     ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->mcs1 = eNB_UE_stats->DL_cqi[0];
	     ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	     ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->ndi1 = 1;
	     ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->rv1 = round&3;
	     ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->tpmi = 5;
	     ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->dai = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	     }*/
	  break;
	case 5:

	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->mcs = mcs;
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->ndi = 1;
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rv = round&3;
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->dai = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  if(dl_pow_off[next_ue] == 2)
	    dl_pow_off[next_ue] = 1;
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->dl_power_off = dl_pow_off[next_ue];
	  ((DCI1E_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->tpmi = 5;
	  break;
	case 6:
	  break;
	}



      }

      else {  // There is no data from RLC or MAC header, so don't schedule

      }
    }

    DAI = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
    LOG_T(MAC,"[eNB %d] Frame %d: DAI %d for UE %d\n",Mod_id,frame,DAI,next_ue);
    // Save DAI for Format 0 DCI
    switch (mac_xface->lte_frame_parms->tdd_config) {
    case 0:
      if ((subframe==0)||(subframe==1)||(subframe==5)||(subframe==6))
	break;
    case 1:
      if ((subframe==1)||(subframe==4)||(subframe==6)||(subframe==9))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    case 2:
      if ((subframe==3)||(subframe==8))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    case 3:
      if ((subframe==0)||(subframe==8)||(subframe==9))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    case 4:
      if ((subframe==8)||(subframe==9))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    case 5:
      if (subframe==8)
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    case 6:
      if ((subframe==1)||(subframe==4)||(subframe==6)||(subframe==9))
	eNB_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    default:
      break;
    }

  }
}

#ifdef ICIC
//added for ALU icic

u32 Get_Cell_SBMap(unsigned char Mod_id){

#define ALFA_CG 0.25



  u8 SB_id,UE_id;
  s32 data_rate;
  s32 sb_cost[NUMBER_OF_SUBBANDS];
  u8 sb_size = 4;//eNB_mac_inst[Mod_id].sbmap_conf.sb_size;
  u8 nb_of_cell_users = find_active_UEs(Mod_id);
  u8 NB_OF_SB_TOT=NUMBER_OF_SUBBANDS;  //#define somewhere
  u16 rnti;
  u32 rballoc=0xffffffff;

  if(!nb_of_cell_users){

    for(SB_id=0;SB_id<NB_OF_SB_TOT;SB_id++)
      if(eNB_mac_inst[Mod_id].sbmap_conf.sbmap[SB_id]==0)
	switch(SB_id){
	case 0:
	  rballoc &= 0xfffffff0;
	  break;
	case 1:
	  rballoc &= 0xffffff0f;
	  break;
	case 2:
	  rballoc &= 0xfffff0ff;
	  break;
	case 3:
	  rballoc &= 0xffff0fff;
	  break;
	case 4:
	  rballoc &= 0xfff0ffff;
	  break;
	case 5:
	  rballoc &= 0xff0fffff;
	  break;
	case 6:
	  rballoc &= 0xf0ffffff;
	  break;
	}

    return rballoc;
  }

  else{


    /*********************************************************************************************************************
     * 		step1: compute cost function																				 *
     *********************************************************************************************************************/

    for(SB_id=0;SB_id<NB_OF_SB_TOT;SB_id++){
      data_rate=0;
      for(UE_id=0;UE_id<NB_UE_INST;UE_id++)
	if (PHY_vars_eNB_g[Mod_id]->dlsch_eNB[(u8)UE_id][0]->rnti>0)
	  data_rate+=180*sb_size*log2(1+ALFA_CG*pow(10,PHY_vars_UE_g[UE_id]->PHY_measurements.subband_cqi_tot_dB[Mod_id][SB_id]/10));

      sb_cost[SB_id]=data_rate/nb_of_cell_users;
    }

    /*********************************************************************************************************************
     * 		step2: rank subbands    																				     *																									 *
     *********************************************************************************************************************/

    u8 buff=0;
    u8 ranked_sb[NB_OF_SB_TOT];
    u8 t=NB_OF_SB_TOT,tt=0;

    for(SB_id=0;SB_id<NB_OF_SB_TOT;SB_id++)
      ranked_sb[SB_id]=SB_id;

    while(t>0){
      tt=0;
      for(SB_id=0;SB_id<t-1;SB_id++){
	if(sb_cost[SB_id]<sb_cost[SB_id+1]){
	  buff=ranked_sb[SB_id];
	  ranked_sb[SB_id]=ranked_sb[SB_id+1];
	  ranked_sb[SB_id+1]=buff;
	  tt=SB_id+1;
	}
      }
      t=tt;
    }

    /*********************************************************************************************************************
     * 		step3: choose and set "Zl" best subbands    															     *																									 *
     *********************************************************************************************************************/

    u8 nb_of_sb_1 = eNB_mac_inst[Mod_id].sbmap_conf.nb_active_sb;

    for(SB_id=0;SB_id<NB_OF_SB_TOT;SB_id++)
      eNB_mac_inst[Mod_id].sbmap_conf.sbmap[SB_id]=1;

    for(SB_id=0;SB_id<NB_OF_SB_TOT;SB_id++){
      if(nb_of_sb_1){
	eNB_mac_inst[Mod_id].sbmap_conf.sbmap[ranked_sb[SB_id]]=0;
	nb_of_sb_1--;
      }
    }
    for(SB_id=0;SB_id<NB_OF_SB_TOT;SB_id++)
      if(eNB_mac_inst[Mod_id].sbmap_conf.sbmap[SB_id]==0)
	switch(SB_id){
	case 0:
	  rballoc &= 0xfffffff0;
	  break;
	case 1:
	  rballoc &= 0xffffff0f;
	  break;
	case 2:
	  rballoc &= 0xfffff0ff;
	  break;
	case 3:
	  rballoc &= 0xffff0fff;
	  break;
	case 4:
	  rballoc &= 0xfff0ffff;
	  break;
	case 5:
	  rballoc &= 0xff0fffff;
	  break;
	case 6:
	  rballoc &= 0xf0ffffff;
	  break;
	}

    return rballoc;

  }
}

void UpdateSBnumber(unsigned char Mod_id){

#define TH_SINR 10

  /*********************************************************************************************************************
   * 		step 4: Update Zl                           															     *																									 *
   *********************************************************************************************************************/

  u8 SB_id,UE_id;
  u32 sinr=0;
  int sinrDb=20;
  u8 nb_of_sb_1 = eNB_mac_inst[Mod_id].sbmap_conf.nb_active_sb;
  u8 nb_of_cell_users = find_active_UEs(Mod_id);

  if(nb_of_cell_users){

    for(UE_id=0;UE_id<NB_UE_INST;UE_id++)
      if (PHY_vars_eNB_g[Mod_id]->dlsch_eNB[(u8)UE_id][0]->rnti>0)
	for(SB_id=0;SB_id<NUMBER_OF_SUBBANDS;SB_id++)
	  if(eNB_mac_inst[Mod_id].sbmap_conf.sbmap[SB_id]==0)
	    sinr+=pow(10,PHY_vars_UE_g[UE_id]->PHY_measurements.subband_cqi_tot_dB[Mod_id][SB_id]/10);

    sinr= sinr/(nb_of_sb_1*nb_of_cell_users);
    if(sinr)
      sinrDb=10*log10(sinr);

    if((sinrDb>TH_SINR) && (nb_of_sb_1<NUMBER_OF_SUBBANDS))
      eNB_mac_inst[Mod_id].sbmap_conf.nb_active_sb++;
    else
      if(nb_of_sb_1>1)
	eNB_mac_inst[Mod_id].sbmap_conf.nb_active_sb--;

  }
}
#endif
//end ALU's algo

void eNB_dlsch_ulsch_scheduler(u8 Mod_id,u8 cooperation_flag, u32 frame, u8 subframe) {

  unsigned char nprb=0,nCCE=0;
  u32 RBalloc=0;

  DCI_PDU *DCI_pdu= &eNB_mac_inst[Mod_id].DCI_pdu;
  LOG_T(MAC,"[eNB %d] Frame %d, Subframe, entering MAC scheduler t\n",Mod_id, frame, subframe);
  // clear DCI and BCCH contents before scheduling
  DCI_pdu->Num_common_dci  = 0;
  DCI_pdu->Num_ue_spec_dci = 0;
  eNB_mac_inst[Mod_id].bcch_active = 0;
  
  if (subframe%5 == 0)
    pdcp_run(frame, 1, 0, Mod_id);

#ifdef CELLULAR
  rrc_rx_tx(Mod_id, frame, 0, 0);
#endif

#ifdef ICIC
  // navid: the following 2 functions does not work properly when there is user-plane traffic
  UpdateSBnumber(Mod_id);
  RBalloc=Get_Cell_SBMap(Mod_id);
#endif
  switch (subframe) {
  case 0:
    // FDD/TDD Schedule Downlink RA transmissions (RA response, Msg4 Contention resolution)
    // Schedule ULSCH for FDD or subframe 4 (TDD config 0,3,6)
    // Schedule Normal DLSCH

    //    schedule_RA(Mod_id,subframe,&nprb,&nCCE);

    if ((mac_xface->lte_frame_parms->frame_type == 0) ||  //FDD
	(mac_xface->lte_frame_parms->tdd_config == 0) ||
	(mac_xface->lte_frame_parms->tdd_config == 3) ||
	(mac_xface->lte_frame_parms->tdd_config == 6))
      schedule_ulsch(Mod_id,frame,cooperation_flag,subframe,&nCCE);
    
    // schedule_ue_spec(Mod_id,subframe,nprb,nCCE);

    fill_DLSCH_dci(Mod_id,frame,subframe,RBalloc,0);

    break;
  case 1:
    // TDD, schedule UL for subframe 7 (TDD config 0,1) / subframe 8 (TDD Config 6)
    // FDD, schedule normal UL/DLSCH
    if (mac_xface->lte_frame_parms->frame_type == 1) { // TDD
      switch (mac_xface->lte_frame_parms->tdd_config) {
      case 0:
      case 1:
      case 6:
	schedule_ulsch(Mod_id,frame,cooperation_flag,subframe,&nCCE);
	break;
      default:
	break;
      }
    }
    else {  //FDD
	//	schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
	// schedule_ue_spec(Mod_id,subframe,nprb,nCCE);
	// fill_DLSCH_dci(Mod_id,subframe,RBalloc,0);
    }
    break;
  case 2:
    // TDD, nothing : ???
    // FDD, normal UL/DLSCH
    if (mac_xface->lte_frame_parms->frame_type == 0) {  //FDD
      // schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
	// schedule_ue_spec(Mod_id,subframe,nprb,nCCE);
	// fill_DLSCH_dci(Mod_id,subframe,RBalloc,0);
    }
    break;
  case 3:
    // TDD Config 2, ULSCH for subframe 7
    // TDD Config 2/5 normal DLSCH
    // FDD, normal UL/DLSCH
    if (mac_xface->lte_frame_parms->frame_type == 1) {
      switch (mac_xface->lte_frame_parms->tdd_config) {
      case 0:
      case 1:
      case 2: 
      case 6:
	schedule_ulsch(Mod_id,frame,cooperation_flag,subframe,&nCCE);
	break;
      case 3:
      case 4:
      case 5: // 3|4|5
	schedule_ue_spec(Mod_id,frame,subframe,nprb,nCCE);
	fill_DLSCH_dci(Mod_id,frame,subframe,RBalloc,0);
	break;
      default:
	break;
      }
    }
    else { //FDD
	//	schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
	// schedule_ue_spec(Mod_id,subframe,0,0);
	// fill_DLSCH_dci(Mod_id,subframe,RBalloc,0);
    }

    break;
  case 4:
    // TDD Config 1, ULSCH for subframe 8
    // TDD Config 1/2/4/5 DLSCH
    // FDD UL/DLSCH
    if (mac_xface->lte_frame_parms->frame_type == 1) { // TDD
      switch (mac_xface->lte_frame_parms->tdd_config) {
      case 0:
      case 1:
      case 6:
	//schedule_RA(Mod_id,frame,subframe,&nprb,&nCCE);
	schedule_ulsch(Mod_id,frame,cooperation_flag,subframe,&nCCE);
	break;
      case 2:
      case 3:
      case 4:
      case 5: 
	schedule_ue_spec(Mod_id,frame,subframe,nprb,nCCE);
	fill_DLSCH_dci(Mod_id,frame,subframe,RBalloc,1);
	break;
      default:
	break;
      }
    }
    else {
      if (mac_xface->lte_frame_parms->frame_type == 0) {  //FDD
	//	schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
	// schedule_ue_spec(Mod_id,subframe,nprb,nCCE);
	// fill_DLSCH_dci(Mod_id,subframe,RBalloc,0);
      }
    }
    break;
  case 5:
    // TDD/FDD Schedule SI and RA
    // TDD Config 0,6 ULSCH for subframes 9,3 resp.
    // TDD normal DLSCH
    // FDD normal UL/DLSCH
    schedule_SI(Mod_id,frame,&nprb,&nCCE);
    schedule_RA(Mod_id,frame,subframe,&nprb,&nCCE);
    if ((mac_xface->lte_frame_parms->frame_type == 0) || //FDD
	(mac_xface->lte_frame_parms->tdd_config == 0) || // TDD Config 0
	(mac_xface->lte_frame_parms->tdd_config == 6)) { // TDD Config 6

      //	schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);

    }
    //    schedule_ue_spec(Mod_id,subframe,nprb,nCCE);
    fill_DLSCH_dci(Mod_id,frame,subframe,RBalloc,1);

    break;

  case 6:
    // TDD Config 0,1,6 ULSCH for subframes 2,3
    // TDD Config 3,4,5 Normal DLSCH
    // FDD normal ULSCH/DLSCH
    if (mac_xface->lte_frame_parms->frame_type == 1) { // TDD
      switch (mac_xface->lte_frame_parms->tdd_config) {
      case 0:
      case 1:
      case 6:
	schedule_ulsch(Mod_id,cooperation_flag,frame,subframe,&nCCE);
	break;
      case 3:
      case 4:
      case 5:
	schedule_ue_spec(Mod_id,frame,subframe,nprb,nCCE);
	fill_DLSCH_dci(Mod_id,frame,subframe,RBalloc,0);
	break;

      default:
	break;
      }
    }
    else {  //FDD
	//	schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
      schedule_ue_spec(Mod_id,frame,subframe,nprb,nCCE);
      fill_DLSCH_dci(Mod_id,frame,subframe,RBalloc,0);
    }

    break;

  case 7:
    // TDD Config 3,4,5 Normal DLSCH
    // FDD Normal UL/DLSCH
    if (mac_xface->lte_frame_parms->frame_type == 1) { // TDD
      switch (mac_xface->lte_frame_parms->tdd_config) {
      case 3:
      case 4:
      case 5:
	//	schedule_RA(Mod_id,frame,subframe,&nprb,&nCCE);
	schedule_ue_spec(Mod_id,frame,subframe,0,0);
	fill_DLSCH_dci(Mod_id,frame,subframe,RBalloc,1);
	break;
      default:
	break;
      }
    }
    else {  //FDD
	//	schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
	// schedule_ue_spec(Mod_id,subframe,0,0);
	// fill_DLSCH_dci(Mod_id,subframe,RBalloc,0);
    }


    break;



  case 8:
    // TDD Config 2,3,4,5 ULSCH for subframe 2
    //
    // FDD Normal UL/DLSCH
    if (mac_xface->lte_frame_parms->frame_type == 1) { // TDD
      switch (mac_xface->lte_frame_parms->tdd_config) {
      case 2:
      case 3:
      case 4:
      case 5:

	//	schedule_RA(Mod_id,subframe,&nprb,&nCCE);
	//	schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
	// schedule_ue_spec(Mod_id,subframe,nprb,nCCE);
	//fill_DLSCH_dci(Mod_id,subframe,RBalloc,1);
	break;
      default:
	break;
      }
    }
    else {  //FDD
	//	schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
	// schedule_ue_spec(Mod_id,subframe,0,0);
	// fill_DLSCH_dci(Mod_id,subframe,RBalloc,0);
    }
    break;

  case 9:
    // TDD Config 1,3,4,6 ULSCH for subframes 3,3,3,4
    if (mac_xface->lte_frame_parms->frame_type == 1) {
      switch (mac_xface->lte_frame_parms->tdd_config) {
      case 1:
      case 3:
      case 4:
      case 6:
	//schedule_ulsch(Mod_id,frame,cooperation_flag,subframe,&nCCE);
      case 2:
      case 5:
	//	schedule_ue_spec(Mod_id,subframe,0,0);
	//schedule_RA(Mod_id,frame,subframe,&nprb,&nCCE);
	//fill_DLSCH_dci(Mod_id,frame,subframe,RBalloc,1);
	break;
      default:
	break;
      }
    }
    else {  //FDD
	//	schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
	// schedule_ue_spec(Mod_id,subframe,0,0);
	// fill_DLSCH_dci(Mod_id,subframe,RBalloc,0);
    }
    break;

  }

}
