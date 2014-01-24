#include "PHY/defs.h"
#include "PHY/extern.h"

#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log_if.h"

#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"

#include "RRC/LITE/extern.h"

#define DLSCH_RB_ALLOC 0x1fbf  // skip DC RB (total 23/25 RBs)
#define DLSCH_RB_ALLOC_12 0x0aaa  // skip DC RB (total 12/25 RBs)
#define DLSCH_RB_ALLOC_6 0x0999  // skip DC RB (total 6/25 RBs)

//static char eNB_generate_rar     = 0;  // flag to indicate start of RA procedure
//static char eNB_generate_rrcconnsetup = 0;  // flag to indicate termination of RA procedure (mirror response)

//#define DEBUG_eNB_SCHEDULER 1
//#define DEBUG_HEADER_PARSING 1
//#define DEBUG_PACKET_TRACE 1

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
void add_ue_ulsch_info(u8 Mod_id, u8 UE_id, u8 subframe, UE_ULSCH_STATUS status){

  eNB_ulsch_info[Mod_id][UE_id].rnti             = find_UE_RNTI(Mod_id,UE_id);
  eNB_ulsch_info[Mod_id][UE_id].subframe         = subframe;
  eNB_ulsch_info[Mod_id][UE_id].status           = status;

  eNB_ulsch_info[Mod_id][UE_id].serving_num++;

}
void add_ue_dlsch_info(u8 Mod_id, u8 UE_id, u8 subframe, UE_DLSCH_STATUS status){

  eNB_dlsch_info[Mod_id][UE_id].rnti             = find_UE_RNTI(Mod_id,UE_id);
  //  eNB_dlsch_info[Mod_id][UE_id].weight           = weight;
  eNB_dlsch_info[Mod_id][UE_id].subframe         = subframe;
  eNB_dlsch_info[Mod_id][UE_id].status           = status;

  eNB_dlsch_info[Mod_id][UE_id].serving_num++;

}

u8 get_ue_weight(u8 Mod_id, u8 UE_id){

  return(eNB_dlsch_info[Mod_id][UE_id].weight);

}

// return is rnti ???
u8 schedule_next_ulue(u8 Mod_id, u8 UE_id, u8 subframe){

  u8 next_ue;

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
    msg("[MAC][eNB] Frame %d: Subframe %d Schedule ULSCH : ue %d status %d\n",mac_xface->frame,subframe,next_ue,eNB_ulsch_info[Mod_id][next_ue].status);
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

u8 schedule_next_dlue(u8 Mod_id, u8 subframe){

  u8 next_ue;

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

void initiate_ra_proc(u8 Mod_id, u16 preamble_index,s16 timing_offset,u8 sect_id) {
  u8 i;
  msg("[MAC][eNB Proc] Initiating RA procedure for index %d\n",preamble_index);

  for (i=0;i<NB_RA_PROC_MAX;i++) {
    if (CH_mac_inst[Mod_id].RA_template[i].RA_active==0) {
      CH_mac_inst[Mod_id].RA_template[i].RA_active=1;
      CH_mac_inst[Mod_id].RA_template[i].generate_rar=1;
      CH_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup=0;
      CH_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup=0;
      CH_mac_inst[Mod_id].RA_template[i].timing_offset=timing_offset;
      // Put in random rnti (to be replaced with proper procedure!!)
      CH_mac_inst[Mod_id].RA_template[i].rnti = taus();
      return;
    }
  }
}

void cancel_ra_proc(u8 Mod_id, u16 preamble_index) {
  u8 i=0;
  msg("[MAC][eNB Proc] Cancelling RA procedure for index %d\n",preamble_index);

  //for (i=0;i<NB_RA_PROC_MAX;i++) {
  CH_mac_inst[Mod_id].RA_template[i].RA_active=0;
  CH_mac_inst[Mod_id].RA_template[i].generate_rar=0;
  CH_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup=0;
  CH_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup=0;
  CH_mac_inst[Mod_id].RA_template[i].timing_offset=0;
  CH_mac_inst[Mod_id].RA_template[i].RRC_timer=20;
  CH_mac_inst[Mod_id].RA_template[i].rnti = 0;
  //}
}

void terminate_ra_proc(u8 Mod_id,u16 rnti,unsigned char *l3msg) {
  u8 i;

  msg("[MAC][eNB %d] Terminating RA procedure for UE rnti %x, Received RRCConnRequest %x,%x,%x,%x,%x,%x\n",Mod_id,rnti,
      l3msg[0],l3msg[1],l3msg[2],l3msg[3],l3msg[4],l3msg[5]);

  for (i=0;i<NB_RA_PROC_MAX;i++) {
    if ((CH_mac_inst[Mod_id].RA_template[i].rnti==rnti) &&
	(CH_mac_inst[Mod_id].RA_template[i].RA_active==1)) {

      memcpy(&CH_mac_inst[Mod_id].RA_template[i].cont_res_id[0],l3msg,6);

      if (Is_rrc_registered == 1)
	Rrc_xface->mac_rrc_data_ind(Mod_id,CCCH,(char *)l3msg,6,0);

      CH_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup = 1;
      CH_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup = 0;

      return;
    }
  }
}

DCI_PDU *get_dci_sdu(u8 Mod_id,u8 subframe) {

  return(&CH_mac_inst[Mod_id].DCI_pdu);

}

s8 find_UE_id(u8 Mod_id,u16 rnti) {

  u8 i;

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    if (CH_mac_inst[Mod_id].UE_template[i].rnti==rnti) {
      return(i);
    }
  }
  return(-1);

}

s16 find_UE_RNTI(u8 Mod_id, u8 UE_id) {

  return (CH_mac_inst[Mod_id].UE_template[UE_id].rnti);

}

s8 find_active_UEs(u8 Mod_id){

  u8 UE_id;
  u16 rnti;
  u8 nb_active_ue=0;

  for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {

    if ((rnti=CH_mac_inst[Mod_id].UE_template[UE_id].rnti) !=0){

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
u16 find_ulgranted_UEs(u8 Mod_id){

  // all active users should be granted
  return(find_active_UEs(Mod_id));
}

// function for determining which active ue should be granted resources in downlink based on CQI, SI, and BSR
u16 find_dlgranted_UEs(u8 Mod_id){

  // all active users should be granted
  return(find_active_UEs(Mod_id));
}
// get aggregatiob form phy for a give UE
u8 process_ue_cqi (u8 Mod_id, u8 UE_id) {

  u8 aggregation=2;
  // check the MCS and SNR and set the aggregation accordingly

  return aggregation;
}

s8 add_new_ue(u8 Mod_id, u16 rnti) {
  u8 i;

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    if (CH_mac_inst[Mod_id].UE_template[i].rnti==0) {
      CH_mac_inst[Mod_id].UE_template[i].rnti=rnti;
      eNB_ulsch_info[Mod_id][i].status = S_UL_WAITING;
      eNB_dlsch_info[Mod_id][i].status = S_UL_WAITING;
      msg("[MAC][eNB] Add UE_id %d : rnti %x\n",i,CH_mac_inst[Mod_id].UE_template[i].rnti);
      return((s8)i);
    }
  }
  return(-1);
}

s8 mac_remove_ue(u8 Mod_id, u8 UE_id) {

  msg("[MAC][eNB] Remove UE_id %d : rnti %x\n",UE_id, CH_mac_inst[Mod_id].UE_template[UE_id].rnti);
  CH_mac_inst[Mod_id].UE_template[UE_id].rnti = 0;
  eNB_ulsch_info[Mod_id][UE_id].rnti          = 0;
  eNB_ulsch_info[Mod_id][UE_id].status        = S_UL_NONE;
  eNB_dlsch_info[Mod_id][UE_id].rnti          = 0;
  eNB_dlsch_info[Mod_id][UE_id].status        = S_DL_NONE;

  return(1);
}

u8 *get_dlsch_sdu(u8 Mod_id,u16 rnti,u8 TBindex) {

  u8 UE_id;

  if (rnti==SI_RNTI) {

#ifdef DEBUG_eNB_SCHEDULER    
    msg("[MAC][eNB] Mod_id %d : Get DLSCH sdu for BCCH \n",Mod_id);
#endif

    return((u8 *)&CH_mac_inst[Mod_id].BCCH_pdu.payload[0]);
  }
  else {

    UE_id = find_UE_id(Mod_id,rnti);
#ifdef DEBUG_eNB_SCHEDULER    
    msg("[MAC][eNB] Mod_id %d : Get DLSCH sdu for rnti %x => UE_id %d\n",Mod_id,rnti,UE_id);
#endif

    return((u8 *)&CH_mac_inst[Mod_id].DLSCH_pdu[UE_id][TBindex].payload[0]);
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
    if (lcid < UE_CONT_RES) {

      if (((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F == 0) {
	length = ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L;
	mac_header_ptr += sizeof(SCH_SUBHEADER_SHORT);
      }
      else {
	length = ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L + ((((SCH_SUBHEADER_LONG *)mac_header_ptr)->L2)<<7);
	mac_header_ptr += sizeof(SCH_SUBHEADER_LONG);
      }
#ifdef DEBUG_HEADER_PARSING
      msg("sdu %d lcid %d length %d\n",num_sdus,lcid,length);
#endif
      rx_lcids[num_sdus] = lcid;
      rx_lengths[num_sdus] = length;
      num_sdus++;
    }
    else {  // This is a control element subheader BSR and CRNTI
      rx_ces[num_ces] = lcid;
      num_ces++;
#ifdef DEBUG_HEADER_PARSING
      msg("ce %d lcid %d\n",num_ces,lcid);
#endif


      mac_header_ptr += sizeof(SCH_SUBHEADER_FIXED);
    }
  }
  *num_ce = num_ces;
  *num_sdu = num_sdus;

  return(mac_header_ptr);
}

void rx_sdu(u8 Mod_id,u16 rnti,u8 *sdu) {

  unsigned char rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr,j;
  unsigned char rx_lcids[MAX_NUM_RB];
  unsigned short rx_lengths[MAX_NUM_RB];
  u8 UE_id = find_UE_id(Mod_id,rnti);

#ifdef DEBUG_HEADER_PARSING
  msg("[MAC][eNB RX] Received ulsch sdu from L1 (rnti %x, UE_id %d), parsing header\n",rnti,UE_id);
#endif
  payload_ptr = parse_ulsch_header(sdu,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths);
#ifdef DEBUG_PACKET_TRACE
  trace_pdu(3,sdu,(payload_ptr - sdu), Mod_id, rnti, 8);
#endif
#ifdef DEBUG_HEADER_PARSING
  msg("Num CE %d, Num SDU %d\n",num_ce,num_sdu);
#endif
  // control element
  for (i=0;i<num_ce;i++) {

    switch (rx_ces[i]) { // implement and process BSR + CRNTI +

    }
  }

  for (i=0;i<num_sdu;i++) {
#ifdef DEBUG_HEADER_PARSING
    msg("SDU %d : LCID %d, length %d\n",i,rx_lcids[i],rx_lengths[i]);
#endif
    if (rx_lcids[i] == DCCH) {
      //      if(CH_mac_inst[Mod_id].Dcch_lchan[UE_id].Active==1){
#ifdef DEBUG_HEADER_PARSING
      msg("offset: %d\n",(u8)((u8*)payload_ptr-sdu));
      for (j=0;j<32;j++)
	msg("%x ",payload_ptr[j]);
      msg("\n");
#endif
      if (rx_lengths[i]<CCCH_PAYLOAD_SIZE_MAX) {
	Mac_rlc_xface->mac_rlc_data_ind(Mod_id,
					DCCH+(UE_id)*MAX_NUM_RB,
					(char *)payload_ptr,
                      rx_lengths[i],
                      1,
					NULL);//(unsigned int*)crc_status);
      }
      //      }
    } else if (rx_lcids[i] >= DTCH) {
      //      if(CH_mac_inst[Mod_id].Dcch_lchan[UE_id].Active==1){
      /*  msg("offset: %d\n",(u8)((u8*)payload_ptr-sdu));
      for (j=0;j<32;j++)
	printf("%x ",payload_ptr[j]);
	printf("\n"); */
      if (rx_lengths[i] <SCH_PAYLOAD_SIZE_MAX) {   // MAX SIZE OF transport block
	Mac_rlc_xface->mac_rlc_data_ind(Mod_id,
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
				    unsigned char short_padding) {

  SCH_SUBHEADER_FIXED *mac_header_ptr = (SCH_SUBHEADER_FIXED *)mac_header;
  unsigned char first_element=0,last_size=0,i;
  unsigned char mac_header_control_elements[16],*ce_ptr;

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
    //    printf("last_size %d,mac_header_ptr %p\n",last_size,mac_header_ptr);
    ((TIMING_ADVANCE_CMD *)ce_ptr)->R=0;
    ((TIMING_ADVANCE_CMD *)ce_ptr)->TA=timing_advance_cmd&0x3f;
    ce_ptr+=sizeof(TIMING_ADVANCE_CMD);
    //    printf("offset %d\n",ce_ptr-mac_header_control_elements);
  }

  if (ue_cont_res_id) {
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

    mac_header_ptr->R = 0;
    mac_header_ptr->E    = 0;
    mac_header_ptr->LCID = UE_CONT_RES;
    last_size=1;
    memcpy(ce_ptr,ue_cont_res_id,6);
    ce_ptr+=6;
    //    printf("(cont_res) : offset %d\n",ce_ptr-mac_header_control_elements);
  }

  //  printf("last_size %d,mac_header_ptr %p\n",last_size,mac_header_ptr);

  for (i=0;i<num_sdus;i++) {

    if (first_element>0) {
      mac_header_ptr->E = 1;
      //      printf("last subheader : %x (R%d,E%d,LCID%d)\n",*(unsigned char*)mac_header_ptr,
      //	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->R,
      //	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E,
      //	     ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID);
      mac_header_ptr+=last_size;
      //      printf("last_size %d,mac_header_ptr %p\n",last_size,mac_header_ptr);
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
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L    = sdu_lengths[i]&0x7f;
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L2   = (sdu_lengths[i]>>7)&0xff;

      last_size=3;
      //printf("long sdu\n");
    }
  }

  //  printf("last_size %d,mac_header_ptr %p\n",last_size,mac_header_ptr);
  /*
    printf("last subheader : %x (R%d,E%d,LCID%d)\n",*(unsigned char*)mac_header_ptr,
    ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->R,
    ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->E,
    ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID);


    if (((SCH_SUBHEADER_FIXED*)mac_header_ptr)->LCID < UE_CONT_RES) {
    if (((SCH_SUBHEADER_SHORT*)mac_header_ptr)->F == 0)
    printf("F = 0, sdu length %d\n",(((SCH_SUBHEADER_SHORT*)mac_header_ptr)->L));
    else
    printf("F = 1, sdu length %d\n",(((SCH_SUBHEADER_LONG*)mac_header_ptr)->L));
    }
  */

  if ((last_size == 0) && ((ce_ptr-mac_header_control_elements) == 0)) {// we have no header
    // so add a padding element
    mac_header_ptr->R    = 0;
    mac_header_ptr->E    = 0;
    mac_header_ptr->LCID = SHORT_PADDING;
    mac_header_ptr++;
  }
  else {



    mac_header_ptr+=last_size;


    if ((ce_ptr-mac_header_control_elements) > 0) {
      memcpy((void*)mac_header_ptr,mac_header_control_elements,ce_ptr-mac_header_control_elements);
      mac_header_ptr+=(unsigned char)(ce_ptr-mac_header_control_elements);
    }
  }

return((unsigned char*)mac_header_ptr - mac_header);

}
void add_common_dci(DCI_PDU *DCI_pdu,void *pdu,u16 rnti,u8 dci_size_bytes,u8 aggregation,u8 dci_size_bits,u8 dci_fmt) {

  memcpy(&DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].dci_pdu[0],pdu,dci_size_bytes);
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].dci_length = dci_size_bits;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].L          = aggregation;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].rnti       = rnti;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].format     = dci_fmt;


  DCI_pdu->Num_common_dci++;
}

void add_ue_spec_dci(DCI_PDU *DCI_pdu,void *pdu,u16 rnti,u8 dci_size_bytes,u8 aggregation,u8 dci_size_bits,u8 dci_fmt) {

  memcpy(&DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].dci_pdu[0],pdu,dci_size_bytes);
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].dci_length = dci_size_bits;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].L          = aggregation;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].rnti       = rnti;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].format     = dci_fmt;


  DCI_pdu->Num_ue_spec_dci++;
}

void schedule_SI(u8 Mod_id,u8 *nprb,u8 *nCCE) {

  u8 bcch_sdu_length;

#ifdef DEBUG_eNB_SCHEDULER
  msg("[MAC][eNB %d] Frame %d : ************SCHEDULE SI***************\n",Mod_id,mac_xface->frame);
#endif

  bcch_sdu_length = Rrc_xface->mac_rrc_data_req(0,
						BCCH,1,
						(char*)&CH_mac_inst[Mod_id].BCCH_pdu.payload[0],
						0);
  if (bcch_sdu_length > 0) {

#ifdef DEBUG_eNB_SCHEDULER
    msg("[MAC][eNB %d] Frame %d : Received %d bytes from BCCH\n",Mod_id,mac_xface->frame,bcch_sdu_length);
#endif

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
#ifdef DEBUG_eNB_SCHEDULER
    msg("[MAC][eNB] Frame %d : Scheduling BCCH for SI %d bytes (mcs %d, TBS %d)\n",
	mac_xface->frame,
	bcch_sdu_length,
	BCCH_alloc_pdu.mcs,
	mac_xface->get_TBS(BCCH_alloc_pdu.mcs,3));
#endif
    CH_mac_inst[Mod_id].bcch_active=1;
    *nprb=3;
    *nCCE=4;
    return;
  }
  CH_mac_inst[Mod_id].bcch_active=0;
  *nprb=0;
  *nCCE=0;
}

// First stage of Random-Access Scheduling
void schedule_RA(u8 Mod_id,u8 subframe,u8 *nprb,u8 *nCCE) {

  RA_TEMPLATE *RA_template = (RA_TEMPLATE *)&CH_mac_inst[Mod_id].RA_template[0];
  u8 i,j,harq_pid,round;
  u16 rrc_sdu_length;
  u8 lcid,offset;
  s8 UE_id;

#ifdef DEBUG_eNB_SCHEDULER
  msg("[MAC][eNB] subframe %d: ************SCHEDULE RA***************\n", subframe);
#endif
  for (i=0;i<NB_RA_PROC_MAX;i++) {

    if (RA_template[i].RA_active == 1) {
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB] RA %d is active (generate_rrcconnsetup %d, wait_ack_rrcconnsetup %d)\n",
	  i,RA_template[i].generate_rrcconnsetup,RA_template[i].wait_ack_rrcconnsetup);
#endif
      if (RA_template[i].generate_rar == 1) {
	*nprb= (*nprb) + 3;
	*nCCE = (*nCCE) + 4;
      }
      else if (RA_template[i].generate_rrcconnsetup == 1) {

	// check for RRCConnSetup Message

	if (Is_rrc_registered == 1) {
	  rrc_sdu_length = Rrc_xface->mac_rrc_data_req(0,
						       0,1,
						       (char*)&CH_mac_inst[Mod_id].CCCH_pdu.payload[0],
						       0);
	  if (rrc_sdu_length == -1)
	    mac_xface->macphy_exit("[MAC][eNB Scheduler] CCCH not allocated\n");
	  else {
#ifdef DEBUG_eNB_SCHEDULER
	    msg("[MAC][eNB %d] Frame %d, subframe %d: got %d bytes from RRC\n",Mod_id,mac_xface->frame, subframe,rrc_sdu_length);
#endif
	  }

	}


	if (rrc_sdu_length>0) {
#ifdef DEBUG_eNB_SCHEDULER
	  msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Generating RRCConnectionSetup (RA proc %d, RNTI %x)\n",mac_xface->frame, subframe,i,
	      RA_template[i].rnti);
#endif
	  // add_user
	  UE_id=add_new_ue(Mod_id,RA_template[i].rnti);
	  if (UE_id==-1) {
	    mac_xface->macphy_exit("[MAC][eNB] Max user count reached\n");
	  }
	  else {
#ifdef DEBUG_eNB_SCHEDULER
	    msg("[MAC][eNB] Added user with rnti %x => UE %d\n",RA_template[i].rnti,UE_id);
#endif
	  }
#ifdef DEBUG_eNB_SCHEDULER
	  msg("[MAC][eNB] Frame %d, subframe %d: Received %d bytes for RRCConnectionSetup: \n",mac_xface->frame,subframe,rrc_sdu_length);
	  for (j=0;j<rrc_sdu_length;j++)
	    msg("%x ",(u8)CH_mac_inst[Mod_id].CCCH_pdu.payload[j]);
	  msg("\n");
	  msg("[MAC][eNB] Frame %d, subframe %d: Generated DLSCH (RRCConnectionSetup) DCI, format 1A, for UE %d\n",mac_xface->frame, subframe,UE_id);
#endif
	  // Schedule Reflection of Connection request
	  ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->ndi=1;

	  // Compute MCS for 3 PRB
	  if ((rrc_sdu_length+8) <= 22)
	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=4;
	  else if ((rrc_sdu_length+8) <= 28)
	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=5;
	  else if ((rrc_sdu_length+8) <= 32)
	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=6;
	  else if ((rrc_sdu_length+8) <= 41)
  	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=7;
	  else if ((rrc_sdu_length+8) <= 41)
  	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=7;
	  else if ((rrc_sdu_length+8) <= 49)
  	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=8;
	  else if ((rrc_sdu_length+8) <= 57)
  	    ((DCI1A_5MHz_TDD_1_6_t*)&RA_template[i].RA_alloc_pdu2[0])->mcs=8;

	  RA_template[i].generate_rrcconnsetup=0;
	  RA_template[i].generate_rrcconnsetup_dci=1;
	  RA_template[i].wait_ack_rrcconnsetup=1;
	  lcid=0;
	  offset = generate_dlsch_header((unsigned char*)CH_mac_inst[Mod_id].DLSCH_pdu[(u8)UE_id][0].payload[0],
					 1,              //num_sdus
					 &rrc_sdu_length,      //
					 &lcid,          // sdu_lcid
					 255,                                   // no drx
					 0,                                   // no timing advance
					 RA_template[i].cont_res_id,        // contention res id
					 0);                                    // no padding

	  memcpy((void*)&CH_mac_inst[Mod_id].DLSCH_pdu[(u8)UE_id][0].payload[0][(u8)offset],
		 &CH_mac_inst[Mod_id].CCCH_pdu.payload[0],
		 rrc_sdu_length);
	  *nprb= (*nprb) + 3;
	  *nCCE = (*nCCE) + 4;
	}
      } // rrcconnectionsetup=1

      else if (CH_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup==1) {
	// check HARQ status and retransmit if necessary
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB] Frame %d, subframe %d: Checking if RRCConnectionSetup was acknowledged :",mac_xface->frame,subframe);
#endif
	// Get candidate harq_pid from PHY
	mac_xface->get_ue_active_harq_pid(Mod_id,CH_mac_inst[Mod_id].RA_template[i].rnti,subframe,&harq_pid,&round,0);
	if (round>0) {
	  *nprb= (*nprb) + 3;
	  *nCCE = (*nCCE) + 4;
	}
      }
    }
  }
}

void schedule_ulsch(u8 Mod_id,u8 cooperation_flag,u8 subframe,u8 *nCCE) {

  u8 UE_id;
  u8 next_ue;
  u8 granted_UEs;
  u8 nCCE_available;
  u8 aggregation;
  u16 rnti;
  u8 round;
  u8 harq_pid;
  DCI0_5MHz_TDD_1_6_t *ULSCH_dci;
  LTE_eNB_UE_stats* eNB_UE_stats,eNB_UE_stats2;
  DCI_PDU *DCI_pdu= &CH_mac_inst[Mod_id].DCI_pdu;

  granted_UEs = find_ulgranted_UEs(Mod_id);
  nCCE_available = mac_xface->get_nCCE_max(Mod_id) - *nCCE;
  //weight = get_ue_weight(Mod_id,UE_id);
  aggregation = 2; // set to maximum aggregation level


#ifdef DEBUG_eNB_SCHEDULER
  msg("[MAC][eNB] subframe %d: ************SCHEDULE ULSCH***************\n",subframe);
  msg("[MAC][eNB] subframe %d: granted_UEs %d\n",subframe,granted_UEs);
#endif

  // allocated UE_ids until nCCE
  for (UE_id=0;UE_id<granted_UEs && (nCCE_available > aggregation);UE_id++) {

    // find next ue to schedule
    //    msg("[MAC][eNB] subframe %d: checking UE_id %d\n",subframe,UE_id);
    next_ue = UE_id;//schedule_next_ulue(Mod_id,UE_id,subframe);
    //    msg("[MAC][eNB] subframe %d: next ue %d\n",subframe,next_ue);
    rnti = find_UE_RNTI(Mod_id,next_ue);
    if (rnti==0) 
      continue;
    //    msg("[MAC][eNB] subframe %d: rnti %x\n",subframe,rnti);
    aggregation = process_ue_cqi(Mod_id,next_ue);
    //    msg("[MAC][eNB] subframe %d: aggregation %d\n",subframe,aggregation);

    eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);
    if (eNB_UE_stats==NULL)
      mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");

#ifdef DEBUG_eNB_SCHEDULER
    msg("[MAC][eNB %d] Scheduler Frame %d, subframe %d, nCCE %d: Checking ULSCH next UE_id %d mode id %d (rnti %x,mode %s), format 0\n",Mod_id,mac_xface->frame,subframe,*nCCE,next_ue,Mod_id, rnti,mode_string[eNB_UE_stats->mode]);
#endif

    if (eNB_UE_stats->mode == PUSCH) {

      // Get candidate harq_pid from PHY
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,1);

      // Note this code is still for a specific DCI format
      ULSCH_dci = (DCI0_5MHz_TDD_1_6_t *)CH_mac_inst[Mod_id].UE_template[next_ue].ULSCH_DCI[harq_pid];
      ULSCH_dci->type=0;
      if (round > 0) {
	ULSCH_dci->ndi = 0;
      }
      else {
	ULSCH_dci->ndi = 1;
      }
      //if ((mac_xface->frame&1)==0) {
      if (ULSCH_dci->ndi == 1) // set mcs for first round
	ULSCH_dci->mcs     = openair_daq_vars.target_ue_ul_mcs;
      else  // increment RV
	ULSCH_dci->mcs = round + 28;

#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB Scheduler] UE %d: got harq_pid %d, round %d, ndi %d, mcs %d, using 4 RBs starting at %d\n",UE_id,
	  harq_pid,round,ULSCH_dci->ndi,ULSCH_dci->mcs,next_ue*4);
#endif

      if ((cooperation_flag > 0) && (next_ue==1)) {
	//Allocation on same set of RBs
	ULSCH_dci->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,
						   ((next_ue-1)*4),//openair_daq_vars.ue_ul_nb_rb),
						   4);//openair_daq_vars.ue_ul_nb_rb);
      }
      else {
	ULSCH_dci->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,
						   (next_ue*4),//openair_daq_vars.ue_ul_nb_rb),
						   4);//openair_daq_vars.ue_ul_nb_rb);
      }
      
      // Cyclic shift for DM RS
      if (cooperation_flag == 2)
	{
	  if(next_ue == 1)// For Distriibuted Alamouti, cyclic shift applied to 2nd UE
	    ULSCH_dci->cshift = 1;
	  else
	    ULSCH_dci->cshift = 0;
	}
      else
	{
	  ULSCH_dci->cshift = 0;// values from 0 to 6 can be used for mapping the cyclic shift (36.211 , Table 5.5.2.1.1-1)
	}

      add_ue_spec_dci(DCI_pdu,
		      ULSCH_dci,
		      rnti,
		      sizeof(DCI0_5MHz_TDD_1_6_t),
		      aggregation,
		      sizeof_DCI0_5MHz_TDD_1_6_t,
		      format0);
      //#ifdef DEBUG_eNB_SCHEDULER
      //      dump_dci(mac_xface->lte_frame_parms,
      //	       &DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci-1]);
      //#endif
      add_ue_ulsch_info(Mod_id,
			next_ue,
			subframe,
			S_UL_SCHEDULED);

      *nCCE = (*nCCE) - aggregation;

#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Generated ULSCH DCI for next UE_id %d, format 0\n",mac_xface->frame,subframe,next_ue);
#endif

    }


  }
}

u32 allocate_prbs(u8 UE_id,u8 nb_rb, u32 *rballoc) {

  int i;
  u32 rballoc_dci=0;
  u8 nb_rb_alloc=0;

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

void fill_DLSCH_dci(u8 Mod_id,u8 subframe) {
  // loop over all allocated UEs and compute frequency allocations for PDSCH

  u8 UE_id,first_rb,nb_rb;
  u16 rnti;
  u8 vrb_map[100];
  u32 rballoc=0;
  u8 round;
  u8 harq_pid;
  void *DLSCH_dci;
  DCI_PDU *DCI_pdu= &CH_mac_inst[Mod_id].DCI_pdu;
  int i;

  // clear vrb_map
  memset(vrb_map,0,100);

  // SI DLSCH
  //printf("BCCH check\n");
  if (CH_mac_inst[Mod_id].bcch_active == 1) {
    CH_mac_inst[Mod_id].bcch_active = 0;

    // randomize frequency allocation for SI
    first_rb = (u8)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
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
		   format1A);
#ifdef DEBUG_eNB_SCHEDULER
    msg("[MAC][eNB] Frame %d: Adding common dci for SI\n",mac_xface->frame);
#endif
  }

  for (i=0;i<NB_RA_PROC_MAX;i++) {
    //    printf("RA %d check\n",i);
    if (CH_mac_inst[Mod_id].RA_template[i].generate_rar == 1) {
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Generating RAR DCI (proc %d), format 1A\n",mac_xface->frame, subframe,i);
#endif
      // randomize frequency allocation for RA
      while (1) {
	  first_rb = (u8)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
	  if ((vrb_map[first_rb] != 1) && (vrb_map[first_rb+2] != 1))
	    break;
	}
	vrb_map[first_rb] = 1;
	vrb_map[first_rb+1] = 1;
	vrb_map[first_rb+2] = 1;

	((DCI1A_5MHz_TDD_1_6_t*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0])->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
	rballoc |= mac_xface->get_rballoc(((DCI1A_5MHz_TDD_1_6_t*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0])->vrb_type,
					  ((DCI1A_5MHz_TDD_1_6_t*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0])->rballoc);

	add_common_dci(DCI_pdu,
		       (void*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0],
		       RA_RNTI,
		       CH_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1,
		       2,
		       CH_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits1,
		       CH_mac_inst[Mod_id].RA_template[i].RA_dci_fmt1);
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB] Frame %d: Adding common dci for RA (RAR)\n",mac_xface->frame);
#endif
	// Schedule Random-Access Response

	CH_mac_inst[Mod_id].RA_template[i].generate_rar=0;
    }
    if (CH_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup_dci == 1) {

      // randomize frequency allocation for RA
      while (1) {
	first_rb = (u8)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
	if ((vrb_map[first_rb] != 1) && (vrb_map[first_rb+2] != 1))
	  break;
      }
      vrb_map[first_rb] = 1;
      vrb_map[first_rb+1] = 1;
      vrb_map[first_rb+2] = 1;
      ((DCI1A_5MHz_TDD_1_6_t*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc= mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
      rballoc |= mac_xface->get_rballoc(((DCI1A_5MHz_TDD_1_6_t*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->vrb_type,
					((DCI1A_5MHz_TDD_1_6_t*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc);

      add_ue_spec_dci(DCI_pdu,
		      (void*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0],
		      CH_mac_inst[Mod_id].RA_template[i].rnti,
		      CH_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes2,
		      2,
		      CH_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits2,
		      CH_mac_inst[Mod_id].RA_template[i].RA_dci_fmt2);
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB] Frame %d: Adding ue specific dci (rnti %x) for RA (ConnectionSetup)\n",mac_xface->frame,CH_mac_inst[Mod_id].RA_template[i].rnti);
#endif
      CH_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup_dci=0;
    }
    else if (CH_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup==1) {
      // check HARQ status and retransmit if necessary
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB] Frame %d, subframe %d: Checking if RRCConnectionSetup was acknowledged :",mac_xface->frame,subframe);
#endif
      // Get candidate harq_pid from PHY
      mac_xface->get_ue_active_harq_pid(Mod_id,CH_mac_inst[Mod_id].RA_template[i].rnti,subframe,&harq_pid,&round,0);
      if (round>0) {
	// we have to schedule a retransmission
	((DCI1A_5MHz_TDD_1_6_t*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->ndi=0;
      // randomize frequency allocation for RA
      while (1) {
	first_rb = (u8)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
	if ((vrb_map[first_rb] != 1) && (vrb_map[first_rb+2] != 1))
	  break;
      }
      vrb_map[first_rb] = 1;
      vrb_map[first_rb+1] = 1;
      vrb_map[first_rb+2] = 1;
      ((DCI1A_5MHz_TDD_1_6_t*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
      rballoc |= mac_xface->get_rballoc(((DCI1A_5MHz_TDD_1_6_t*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->vrb_type,
					((DCI1A_5MHz_TDD_1_6_t*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc);


      add_ue_spec_dci(DCI_pdu,
		      (void*)&CH_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0],
		      CH_mac_inst[Mod_id].RA_template[i].rnti,
		      CH_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes2,
		      2,
		      CH_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits2,
		      CH_mac_inst[Mod_id].RA_template[i].RA_dci_fmt2);
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB] Frame %d: Adding ue specific dci (rnti %x) for RA (ConnectionSetup Retransmission)\n",mac_xface->frame,CH_mac_inst[Mod_id].RA_template[i].rnti);
#endif
      }
      else {
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB] RRCConnectionSetup acknowledged\n");
#endif
	CH_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup=0;
	CH_mac_inst[Mod_id].RA_template[i].RA_active=0;
      }
    }
  }

  // UE specific DCIs
  for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
    //printf("UE_id: %d => status %d\n",UE_id,eNB_dlsch_info[Mod_id][UE_id].status);
    if (eNB_dlsch_info[Mod_id][UE_id].status == S_DL_SCHEDULED) {

      // clear scheduling flag
      eNB_dlsch_info[Mod_id][UE_id].status = S_DL_WAITING;
      rnti = find_UE_RNTI(Mod_id,UE_id);
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);
      nb_rb = CH_mac_inst[Mod_id].UE_template[UE_id].nb_rb[harq_pid];

      DLSCH_dci = (void *)CH_mac_inst[Mod_id].UE_template[UE_id].DLSCH_DCI[harq_pid];
#ifdef    DEBUG_PACKET_TRACE
    //trace_pdu(4,CH_mac_inst[Mod_id].DLSCH_pdu[(u8)next_ue][0].payload[0],TBS/*sdu_length_total+offset offset*/, UE_id, rnti, subframe);
	//TODO
#endif
      switch(mac_xface->get_transmission_mode(rnti)) {
      default:
      case 1:
      case 2:

	((DCI1_5MHz_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs(UE_id,nb_rb,&rballoc);
	((DCI1_5MHz_TDD_t*)DLSCH_dci)->rah = 0;
	add_ue_spec_dci(DCI_pdu,
			DLSCH_dci,
			rnti,
			sizeof(DCI1_5MHz_TDD_t),
			2,//aggregation,
			sizeof_DCI1_5MHz_TDD_t,
			format1);
	break;
	case 4:
	  if (nb_rb>10) {
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs(UE_id,nb_rb,&rballoc);
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rah = 0;
	  add_ue_spec_dci(DCI_pdu,
			  DLSCH_dci,
			  rnti,
			  sizeof(DCI2_5MHz_2A_M10PRB_TDD_t),
			  2,//aggregation,
			  sizeof_DCI2_5MHz_2A_M10PRB_TDD_t,
			  format2_2A_M10PRB);
	  }
	  else {
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs(UE_id,nb_rb,&rballoc);
	    add_ue_spec_dci(DCI_pdu,
			    DLSCH_dci,
			    rnti,
			    sizeof(DCI2_5MHz_2A_L10PRB_TDD_t),
			    2,//aggregation,
			    sizeof_DCI2_5MHz_2A_L10PRB_TDD_t,
			    format2_2A_L10PRB);
	  }
	  break;
      }
    }
  }
}


 void schedule_ue_spec(u8 Mod_id,u8 subframe,u16 nb_rb_used0,u8 nCCE_used) {

  u8 UE_id;
  u8 next_ue;
  u8 granted_UEs;
  u16 nCCE;
  u8 aggregation;
  mac_rlc_status_resp_t rlc_status;
  u8 header_len_dcch,header_len_dtch;
  u8 sdu_lcids[11],offset,num_sdus=0;
  u16 nb_rb,nb_available_rb,TBS,j,sdu_lengths[11],rnti;
  u8 dlsch_buffer[MAX_DLSCH_PAYLOAD_BYTES];
  u8 round;
  u8 harq_pid;
  void *DLSCH_dci;
  LTE_eNB_UE_stats* eNB_UE_stats;
  u16 sdu_length_total=0;
  u8 loop_count;
  u8 DAI;

  granted_UEs = find_dlgranted_UEs(Mod_id);
  //weight = get_ue_weight(Mod_id,UE_id);
  aggregation = 2; // set to the maximum aggregation level


#ifdef DEBUG_eNB_SCHEDULER
  msg("[MAC][eNB %d] Frame %d subframe %d: ************SCHEDULE DLSCH***************\n",Mod_id,mac_xface->frame,subframe);
#endif
  // while frequency resources left and nCCE available
  //  for (UE_id=0;(UE_id<granted_UEs) && (nCCE > aggregation);UE_id++) {

  // set current available nb_rb and nCCE to maximum
  nb_available_rb = mac_xface->lte_frame_parms->N_RB_DL - nb_rb_used0;
  nCCE = mac_xface->get_nCCE_max(Mod_id) - nCCE_used;

  for (UE_id=0;UE_id<granted_UEs;UE_id++) {
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
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
    case 1:
      if ((subframe==0)||(subframe==4)||(subframe==5)||(subframe==9))
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
      break;
    case 2:
      if ((subframe==4)||(subframe==5))
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
      break;
    case 3:
      if ((subframe==1)||(subframe==7)||(subframe==9))
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
      break;
    case 4:
      if ((subframe==0)||(subframe==6))
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
      break;
    case 5:
      if (subframe==9)
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
      break;
    case 6:
      if ((subframe==0)||(subframe==1)||(subframe==5)||(subframe==6)||(subframe==9))
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI = 0;
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

    eNB_UE_stats->dlsch_mcs1 = openair_daq_vars.target_ue_dl_mcs;

    // Get candidate harq_pid from PHY
    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);
    //printf("Got harq_pid %d, round %d\n",harq_pid,round);

    // Note this code is for a specific DCI format
    DLSCH_dci = (void *)CH_mac_inst[Mod_id].UE_template[next_ue].DLSCH_DCI[harq_pid];

    if (round > 0) {

      CH_mac_inst[Mod_id].UE_template[next_ue].DAI++;

      // get freq_allocation
      nb_rb = CH_mac_inst[Mod_id].UE_template[next_ue].nb_rb[harq_pid];
      if (nb_rb <= nb_available_rb) {
	nb_available_rb -= nb_rb;
	aggregation = process_ue_cqi(Mod_id,next_ue);
	nCCE-=aggregation; // adjust the remaining nCCE

	switch (mac_xface->get_transmission_mode(rnti)) {
	case 1:
	case 2:
	default:
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->ndi      = 0;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->rv       = round&3;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->dai      = (CH_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  msg("[MAC] Retransmission : harq_pid %d, round %d, dai %d, mcs %d\n",harq_pid,round,(CH_mac_inst[Mod_id].UE_template[next_ue].DAI-1),((DCI1_5MHz_TDD_t*)DLSCH_dci)->mcs);
	  break;
	case 4:
	  if (nb_rb>10) {
	    ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->ndi1 = 0;
	    ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	    ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->dai = (CH_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  }
	  else {
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->ndi1 = 0;
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->rv1 = round&3;
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->dai = (CH_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  }
	  break;
	case 5:
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

      // Now check RLC information to compute number of required RBs
      // get maximum TBS size for RLC request
      TBS = mac_xface->get_TBS(eNB_UE_stats->dlsch_mcs1,nb_available_rb);

      // check first for RLC data on DCCH
      header_len_dcch = 2+1+1; // 2 bytes DCCH SDU subheader + timing advance subheader + timing advance command
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB %d] Requesting %d bytes from RLC (mcs %d, nb_available_rb %d)\n",Mod_id,TBS-header_len_dcch,
	  eNB_UE_stats->dlsch_mcs1,nb_available_rb);
#endif
      rlc_status = mac_rlc_status_ind(Mod_id,DCCH+(MAX_NUM_RB*next_ue),
				      (TBS-header_len_dcch)/DCCH_LCHAN_DESC.transport_block_size,
				      DCCH_LCHAN_DESC.transport_block_size); // transport block set size


      sdu_lengths[0]=0;
      loop_count=0;
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB %d] DCCH has %d bytes to send (buffer %d, header %d)\n",Mod_id,rlc_status.bytes_in_buffer,sdu_lengths[0],header_len_dcch);
#endif
      sdu_lengths[0] += Mac_rlc_xface->mac_rlc_data_req(Mod_id,
							DCCH+(MAX_NUM_RB*next_ue),
							(char *)&dlsch_buffer[sdu_lengths[0]]);
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB %d] Got %d bytes from RLC\n",Mod_id,sdu_lengths[0]);
#endif
      if ((sdu_lengths[0] + header_len_dcch )< TBS) {
	// repeat to see if additional data generated due to request
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] DCCH has %d bytes to send (buffer %d, header %d)\n",Mod_id,rlc_status.bytes_in_buffer,sdu_lengths[0],header_len_dcch);
#endif
	rlc_status = mac_rlc_status_ind(Mod_id,DCCH+(MAX_NUM_RB*next_ue),
					(TBS-header_len_dcch-sdu_lengths[0])/DCCH_LCHAN_DESC.transport_block_size,
					DCCH_LCHAN_DESC.transport_block_size);

	sdu_lengths[0] += Mac_rlc_xface->mac_rlc_data_req(Mod_id,
							  DCCH+(MAX_NUM_RB*next_ue),
							  (char*)&dlsch_buffer[sdu_lengths[0]]);
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] Got %d bytes from RLC\n",Mod_id,sdu_lengths[0]);
#endif
      }

      if (sdu_lengths[0]>0) {   // There is DCCH to transmit
	sdu_length_total = sdu_lengths[0];
	sdu_lcids[0] = DCCH;
	num_sdus = 1;

#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] Got %d bytes for DCCH :",Mod_id,sdu_lengths[0]);
	for (j=0;j<sdu_lengths[0];j++)
	  msg("%x ",dlsch_buffer[j]);
	msg("\n");
#endif
      }
      else {
	if (eNB_UE_stats->UE_timing_offset/4 != 0) {
	  header_len_dcch = 1+1;  // Timing advance subheader+cmd
	  sdu_length_total = 0;
	}
	else {
	  header_len_dcch = 0;
	  sdu_length_total = 0;
	}
      }
      // check for DTCH and update header information

      header_len_dtch = 3; // 3 bytes DTCH SDU subheader

      rlc_status = mac_rlc_status_ind(Mod_id,DTCH+(MAX_NUM_RB*next_ue),
				      0,
				      TBS-header_len_dcch-sdu_length_total-header_len_dtch);

      if (rlc_status.bytes_in_buffer > 0) {

	sdu_lengths[num_sdus] = Mac_rlc_xface->mac_rlc_data_req(Mod_id,
								DTCH+(MAX_NUM_RB*next_ue),
								(char*)&dlsch_buffer[sdu_length_total]);
	if (sdu_lengths[num_sdus] < 128)
	  header_len_dtch = 2;

#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] PHY_DATA_REQ Got %d bytes for DTCH\n",Mod_id,sdu_lengths[num_sdus]);
#endif
	sdu_lcids[num_sdus] = DTCH;
	sdu_length_total += sdu_lengths[num_sdus];
	num_sdus++;
      }
      else {
	header_len_dtch = 0;
      }

      if ((sdu_length_total + header_len_dcch + header_len_dtch )> 0) {
	offset = generate_dlsch_header((unsigned char*)CH_mac_inst[Mod_id].DLSCH_pdu[(u8)next_ue][0].payload[0],
				       // offset = generate_dlsch_header((unsigned char*)CH_mac_inst[0].DLSCH_pdu[0][0].payload[0],
				       num_sdus,              //num_sdus
				       sdu_lengths,  //
				       sdu_lcids,
				       255,                                   // no drx
				       eNB_UE_stats->UE_timing_offset/4,      // timing advance
				       NULL,                                  // contention res id
				       0);                                    // no padding
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] Generate header : sdu_length_total %d, num_sdus %d, sdu_lengths[0] %d, sdu_lcids[0] %d => payload offset %d,timing advance : %d, next_ue %d\n",
	    Mod_id,sdu_length_total,num_sdus,sdu_lengths[0],sdu_lcids[0],offset,
	    eNB_UE_stats->UE_timing_offset/4,
	    next_ue);
#endif

	// cycle through SDUs and place in dlsch_buffer
	memcpy(&CH_mac_inst[Mod_id].DLSCH_pdu[(u8)next_ue][0].payload[0][offset],dlsch_buffer,sdu_length_total);
	// memcpy(&CH_mac_inst[0].DLSCH_pdu[0][0].payload[0][offset],dcch_buffer,sdu_lengths[0]);

	// Now compute number of required RBs for total sdu length
	// Assume RAH format 2
	nb_rb = 2;
	TBS = mac_xface->get_TBS(eNB_UE_stats->dlsch_mcs1,nb_rb);

	while (TBS < (sdu_length_total + offset))  {
	  nb_rb += 2;  // to be replaced with RA allocation size for other than 25 PRBs!!!!!!!
	  if (nb_rb>mac_xface->lte_frame_parms->N_RB_DL) { // if we've gone beyond the maximum number of RBs
	    // (can happen if N_RB_DL is odd)
	    TBS = mac_xface->get_TBS(eNB_UE_stats->dlsch_mcs1,mac_xface->lte_frame_parms->N_RB_DL);
	    nb_rb = mac_xface->lte_frame_parms->N_RB_DL;
	    break;
	  }
	  TBS = mac_xface->get_TBS(eNB_UE_stats->dlsch_mcs1,nb_rb);
	}

#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] Generated DLSCH header (mcs %d, TBS %d, nb_rb %d)\n",
	    Mod_id,eNB_UE_stats->dlsch_mcs1,TBS,nb_rb);
	// msg("[MAC][eNB ] Reminder of DLSCH with random data %d %d %d %d \n",
	//	TBS, sdu_length_total, offset, TBS-sdu_length_total-offset);
#endif

	// fill remainder of DLSCH with random data
	for (j=0;j<(TBS-sdu_length_total-offset);j++)
	  CH_mac_inst[Mod_id].DLSCH_pdu[(u8)next_ue][0].payload[0][offset+sdu_length_total+j] = (char)(taus()&0xff);
	//CH_mac_inst[0].DLSCH_pdu[0][0].payload[0][offset+sdu_lengths[0]+j] = (char)(taus()&0xff);

	aggregation = process_ue_cqi(Mod_id,next_ue);
	nCCE-=aggregation; // adjust the remaining nCCE
	CH_mac_inst[Mod_id].UE_template[next_ue].nb_rb[harq_pid] = nb_rb;

	add_ue_dlsch_info(Mod_id,
			  next_ue,
			  subframe,
			  S_DL_SCHEDULED);
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI++;
#ifdef    DEBUG_PACKET_TRACE
    trace_pdu(4,CH_mac_inst[Mod_id].DLSCH_pdu[(u8)next_ue][0].payload[0],TBS/*sdu_length_total+offset offset*/, next_ue, rnti, subframe);
#endif
	switch (mac_xface->get_transmission_mode(rnti)) {
	case 1:
	case 2:
	default:
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->mcs = eNB_UE_stats->dlsch_mcs1;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->ndi = 1;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->rv = 0;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->dai      = (CH_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  break;
	case 4:
	  if (nb_rb>10) {
	    ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->mcs1 = eNB_UE_stats->dlsch_mcs1;
	    ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	    ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->ndi1 = 1;
	    ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rv1 = round&3;
	    ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->dai = (CH_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;

	  }
	  else {
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->mcs1 = eNB_UE_stats->dlsch_mcs1;
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->ndi1 = 1;
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->rv1 = round&3;
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->tpmi = 5;
	    ((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->dai = (CH_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  }
	  break;
	case 5:
	  break;
	case 6:
	  break;
	}



      }

      else {  // There is no data from RLC or MAC header, so don't schedule

      }
    }

    DAI = (CH_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
#ifdef DEBUG_eNB_SCHEDULER
    msg("[MAC][eNB %d] Frame %d: DAI %d for UE %d\n",Mod_id,mac_xface->frame,DAI,next_ue);
#endif

    // Save DAI for Format 0 DCI
    switch (mac_xface->lte_frame_parms->tdd_config) {
    case 0:
      if ((subframe==0)||(subframe==1)||(subframe==5)||(subframe==6))
	break;
    case 1:
      if ((subframe==1)||(subframe==4)||(subframe==6)||(subframe==9))
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    case 2:
      if ((subframe==3)||(subframe==8))
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    case 3:
      if ((subframe==0)||(subframe==8)||(subframe==9))
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    case 4:
      if ((subframe==8)||(subframe==9))
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    case 5:
      if (subframe==8)
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    case 6:
      if ((subframe==1)||(subframe==4)||(subframe==6)||(subframe==9))
	CH_mac_inst[Mod_id].UE_template[next_ue].DAI_ul = DAI;
      break;
    default:
      break;
    }

  }
 }



void eNB_dlsch_ulsch_scheduler(u8 Mod_id,u8 cooperation_flag,u8 subframe) {

  u8 nprb=0,nCCE=0;

  DCI_PDU *DCI_pdu= &CH_mac_inst[Mod_id].DCI_pdu;

  // clear DCI and BCCH contents before scheduling
  DCI_pdu->Num_common_dci  = 0;
  DCI_pdu->Num_ue_spec_dci = 0;
  CH_mac_inst[Mod_id].bcch_active = 0;

#ifdef DEBUG_eNB_SCHEDULER
  msg("[MAC][eNB] inst %d scheduler subframe %d\n",Mod_id, subframe);
#endif

  //LOG_I (MAC, "eNB inst %d scheduler subframe %d nCCE %d \n",Mod_id, subframe, mac_xface->get_nCCE_max(Mod_id) );
  Mac_rlc_xface->frame= mac_xface->frame;
  Rrc_xface->Frame_index=Mac_rlc_xface->frame;

  Mac_rlc_xface->pdcp_run();

  switch (subframe) {

  case 0:
    //schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
    break;

  case 1:
    break;

  case 2:
    break;

  case 3:
    break;

  case 4:
    break;

  case 5:
    schedule_SI(Mod_id,&nprb,&nCCE);
    fill_DLSCH_dci(Mod_id,subframe);
    break;

  case 6:
    break;

  case 7:
    schedule_ue_spec(Mod_id,subframe,0,0);
    fill_DLSCH_dci(Mod_id,subframe);
    break;

  case 8:
    schedule_RA(Mod_id,subframe,&nprb,&nCCE);
    //    schedule_ue_spec(Mod_id,subframe,nprb,nCCE);
    fill_DLSCH_dci(Mod_id,subframe);
    // Schedule UL subframe
    //schedule_ulsch(Mod_id,subframe,&nCCE);
    break;

  case 9:
    // Schedule UL subframe
    schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
    break;
  }

}