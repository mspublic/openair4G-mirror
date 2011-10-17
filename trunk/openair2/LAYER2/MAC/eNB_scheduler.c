#include "PHY/defs.h"
#include "PHY/extern.h"

#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log.h"

#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"

#include "RRC/LITE/extern.h"

#define DLSCH_RB_ALLOC 0x1fbf  // skip DC RB (total 23/25 RBs)
#define DLSCH_RB_ALLOC_12 0x0aaa  // skip DC RB (total 12/25 RBs)
#define DLSCH_RB_ALLOC_6 0x0999  // skip DC RB (total 6/25 RBs)



//#define Pre_Processing 1   /// Pre processing for MU-MIMO

//#define FULL_BUFFER 1      /// Fill BUffer for UEs



//static char eNB_generate_rar     = 0;  // flag to indicate start of RA procedure
//static char eNB_generate_rrcconnsetup = 0;  // flag to indicate termination of RA procedure (mirror response)

//#define DEBUG_eNB_SCHEDULER 0
//#define DEBUG_HEADER_PARSING 0
//#define DEBUG_PACKET_TRACE 0

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

void initiate_ra_proc(unsigned char Mod_id, u16 preamble_index,s16 timing_offset,unsigned char sect_id) {
  unsigned char i;
  msg("[MAC][eNB %d] Initiating RA procedure for index %d\n",Mod_id,preamble_index);

  for (i=0;i<NB_RA_PROC_MAX;i++) {
    if (eNB_mac_inst[Mod_id].RA_template[i].RA_active==0) {
      eNB_mac_inst[Mod_id].RA_template[i].RA_active=1;
      eNB_mac_inst[Mod_id].RA_template[i].generate_rar=1;
      eNB_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup=0;
      eNB_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup=0;
      eNB_mac_inst[Mod_id].RA_template[i].timing_offset=timing_offset;
      // Put in random rnti (to be replaced with proper procedure!!)
      eNB_mac_inst[Mod_id].RA_template[i].rnti = taus();
      msg("[MAC][eNB %d] Activating RAR generation for process %d, rnti %x\n",
	  Mod_id,i,eNB_mac_inst[Mod_id].RA_template[i].rnti);

      return;
    }
  }
}

void cancel_ra_proc(unsigned char Mod_id, u16 preamble_index) {
  unsigned char i=0;
  msg("[MAC][eNB Proc] Cancelling RA procedure for index %d\n",preamble_index);

  //for (i=0;i<NB_RA_PROC_MAX;i++) {
  eNB_mac_inst[Mod_id].RA_template[i].RA_active=0;
  eNB_mac_inst[Mod_id].RA_template[i].generate_rar=0;
  eNB_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup=0;
  eNB_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup=0;
  eNB_mac_inst[Mod_id].RA_template[i].timing_offset=0;
  eNB_mac_inst[Mod_id].RA_template[i].RRC_timer=20;
  eNB_mac_inst[Mod_id].RA_template[i].rnti = 0;
  //}
}

void terminate_ra_proc(unsigned char Mod_id,u16 rnti,unsigned char *l3msg) {

  unsigned char rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr,j;
  unsigned char rx_lcids[MAX_NUM_RB];
  u16 rx_lengths[MAX_NUM_RB];

  msg("[MAC][eNB %d] Terminating RA procedure for UE rnti %x, Received l3msg %x,%x,%x,%x,%x,%x\n",Mod_id,rnti,
      l3msg[0],l3msg[1],l3msg[2],l3msg[3],l3msg[4],l3msg[5]);

  for (i=0;i<NB_RA_PROC_MAX;i++) {
    msg("Checking proc %d (%x) : rnti %x, active %d\n",i,eNB_mac_inst[Mod_id].RA_template[i].rnti,eNB_mac_inst[Mod_id].RA_template[i].rnti,eNB_mac_inst[Mod_id].RA_template[i].RA_active);
    if ((eNB_mac_inst[Mod_id].RA_template[i].rnti==rnti) &&
	(eNB_mac_inst[Mod_id].RA_template[i].RA_active==1)) {

      payload_ptr = parse_ulsch_header(l3msg,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths);

      if ((num_ce == 0) && (num_sdu==1) && (rx_lcids[0] == CCCH)) { // This is an RRCConnectionRequest
	msg("[MAC][eNB] : Received CCCH: length %d, offset %d\n",rx_lengths[0],payload_ptr-l3msg);
	memcpy(&eNB_mac_inst[Mod_id].RA_template[i].cont_res_id[0],payload_ptr,6);

	if (Is_rrc_registered == 1)
	  Rrc_xface->mac_rrc_data_ind(Mod_id,CCCH,(char *)payload_ptr,rx_lengths[0],1,Mod_id);
      }
      else if (num_ce >0) {  // handle l3msg which is not RRCConnectionRequest
	//	process_ra_message(l3msg,num_ce,rx_lcids,rx_ces);
      }

      eNB_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup = 1;
      eNB_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup = 0;

      return;
    } // if process is active

  } // loop on RA processes
}

DCI_PDU *get_dci_sdu(unsigned char Mod_id,unsigned char subframe) {

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
      msg("[MAC][UE] Add UE_id %d : rnti %x\n",i,eNB_mac_inst[Mod_id].UE_template[i].rnti);
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

unsigned char *get_dlsch_sdu(unsigned char Mod_id,u16 rnti,unsigned char TBindex) {

  unsigned char UE_id;

  if (rnti==SI_RNTI) {
    msg("[MAC][eNB] Mod_id %d : Get DLSCH sdu for BCCH \n",Mod_id);

    return((unsigned char *)&eNB_mac_inst[Mod_id].BCCH_pdu.payload[0]);
  }
  else {

    UE_id = find_UE_id(Mod_id,rnti);
    msg("[MAC][eNB] Mod_id %d : Get DLSCH sdu for rnti %x => UE_id %d\n",Mod_id,rnti,UE_id);

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

void rx_sdu(unsigned char Mod_id,u16 rnti,unsigned char *sdu) {

  unsigned char rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr,j;
  unsigned char rx_lcids[MAX_NUM_RB];
  unsigned short rx_lengths[MAX_NUM_RB];
  unsigned char UE_id = find_UE_id(Mod_id,rnti);
  int rack,ii;
  for(ii=0; ii<MAX_NUM_RB; ii++) rx_lengths[ii] = 0;

#ifdef DEBUG_HEADER_PARSING
  msg("[MAC][eNB RX] Received ulsch sdu from L1 (rnti %x, UE_id %d), parsing header\n",rnti,UE_id);
#endif
  payload_ptr = parse_ulsch_header(sdu,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths);
#ifdef DEBUG_PACKET_TRACE
  if((sdu!=NULL)&&(sdu!=0))
    trace_pdu(3,sdu,rx_lengths[1]/*(payload_ptr - sdu )*/, Mod_id, rnti, 8);
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
	Mac_rlc_xface->mac_rlc_data_ind(Mod_id,
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
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L    = sdu_lengths[i]&0x7fff;

      last_size=3;
      msg("long sdu\n");
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
void add_common_dci(DCI_PDU *DCI_pdu,void *pdu,u16 rnti,unsigned char dci_size_bytes,unsigned char aggregation,unsigned char dci_size_bits,unsigned char dci_fmt) {

  memcpy(&DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].dci_pdu[0],pdu,dci_size_bytes);
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].dci_length = dci_size_bits;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].L          = aggregation;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].rnti       = rnti;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci].format     = dci_fmt;


  DCI_pdu->Num_common_dci++;
}

void add_ue_spec_dci(DCI_PDU *DCI_pdu,void *pdu,u16 rnti,unsigned char dci_size_bytes,unsigned char aggregation,unsigned char dci_size_bits,unsigned char dci_fmt) {

  memcpy(&DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].dci_pdu[0],pdu,dci_size_bytes);
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].dci_length = dci_size_bits;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].L          = aggregation;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].rnti       = rnti;
  DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci].format     = dci_fmt;


  DCI_pdu->Num_ue_spec_dci++;
}

void schedule_SI(unsigned char Mod_id,unsigned char *nprb,unsigned char *nCCE) {

  unsigned char bcch_sdu_length;

#ifdef DEBUG_eNB_SCHEDULER
  msg("[MAC][eNB %d] Frame %d : ************SCHEDULE SI***************\n",Mod_id,mac_xface->frame);
#endif

  bcch_sdu_length = Rrc_xface->mac_rrc_data_req(Mod_id,
						BCCH,1,
						(char*)&eNB_mac_inst[Mod_id].BCCH_pdu.payload[0],
						1,
						Mod_id);
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
void schedule_RA(unsigned char Mod_id,unsigned char subframe,unsigned char *nprb,unsigned char *nCCE) {

  RA_TEMPLATE *RA_template = (RA_TEMPLATE *)&eNB_mac_inst[Mod_id].RA_template[0];
  unsigned char i,j,harq_pid,round;
  u16 rrc_sdu_length;
  unsigned char lcid,offset;
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
	  rrc_sdu_length = Rrc_xface->mac_rrc_data_req(Mod_id,
						       0,1,
						       (char*)&eNB_mac_inst[Mod_id].CCCH_pdu.payload[0],
						       1,
						       Mod_id);
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
	    msg("%x ",(unsigned char)eNB_mac_inst[Mod_id].CCCH_pdu.payload[j]);
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
	  offset = generate_dlsch_header((unsigned char*)eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)UE_id][0].payload[0],
					 1,              //num_sdus
					 &rrc_sdu_length,      //
					 &lcid,          // sdu_lcid
					 255,                                   // no drx
					 0,                                   // no timing advance
					 RA_template[i].cont_res_id,        // contention res id
					 0);                                    // no padding

	  memcpy((void*)&eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)UE_id][0].payload[0][(unsigned char)offset],
		 &eNB_mac_inst[Mod_id].CCCH_pdu.payload[0],
		 rrc_sdu_length);
	  *nprb= (*nprb) + 3;
	  *nCCE = (*nCCE) + 4;
	}
	//try here

	//	_Send_Ra_Mac_Pdu ( 1/*PDU TYPE*/,0/*Extension*/, 1 /*TypeRaPid*/,
	//			   00/* RaPid*/, eNB_mac_inst[Mod_id].RA_template[0].timing_offset /*TA*/,
	//		   0/*Hopping_flag*/,25 /* rar->rb_allocmac_xface->computeRIV(100,0,2) fsrba*/,
	//		   2 /*tmcs*/, 0 /*tcsp*/, 0 /*ul_delay*/, 1/* cqi_request*/,
	//		   eNB_mac_inst[Mod_id].RA_template[0].rnti/* crnti_temporary*/,
	//		   1 /*radioType=TDD_RADIO*/, 0 /*direction=DIRECTION_DOWNLINK*/,
	//		   2/* rntiType=WS_RA_RNTI*/,  eNB_mac_inst[Mod_id].RA_template[0].rnti /*rnti*/,
	//		   UE_id/*UE_id*/,0/* subframeNumber*/,
	//		   0 /*isPredefinedData*/, 0 /*retx*/, 1 /*crcStatus*/);
      } // rrcconnectionsetup=1

      else if (eNB_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup==1) {
	// check HARQ status and retransmit if necessary
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB] Frame %d, subframe %d: Checking if RRCConnectionSetup was acknowledged :",mac_xface->frame,subframe);
#endif
	// Get candidate harq_pid from PHY
	mac_xface->get_ue_active_harq_pid(Mod_id,eNB_mac_inst[Mod_id].RA_template[i].rnti,subframe,&harq_pid,&round,0);
	if (round>0) {
	  *nprb= (*nprb) + 3;
	  *nCCE = (*nCCE) + 4;
	}
      }
    }
  }
}

void schedule_ulsch(unsigned char Mod_id,unsigned char cooperation_flag,unsigned char subframe,unsigned char *nCCE) {

  unsigned char UE_id;
  unsigned char next_ue;
  unsigned char granted_UEs;
  unsigned char nCCE_available;
  unsigned char aggregation;
  u16 rnti;
  unsigned char round;
  unsigned char harq_pid;
  DCI0_5MHz_TDD_1_6_t *ULSCH_dci;
  LTE_eNB_UE_stats* eNB_UE_stats;
  DCI_PDU *DCI_pdu= &eNB_mac_inst[Mod_id].DCI_pdu;
  u8 status=0,status0=0,status1=0;

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
      ULSCH_dci = (DCI0_5MHz_TDD_1_6_t *)eNB_mac_inst[Mod_id].UE_template[next_ue].ULSCH_DCI[harq_pid];


      //msg("FAIL\n");
      status = Rrc_xface->get_rrc_status(Mod_id,1,next_ue);
      //status0 = Rrc_xface->get_rrc_status(Mod_id,1,0);//next_ue);
      //status1 = Rrc_xface->get_rrc_status(Mod_id,1,1);//next_ue+1);

      //msg("status of RRC %d\n",status);
      //exit(0);

      //if((status0 >= RRC_CONNECTED) && (status1 >= RRC_CONNECTED))
      //if(next_ue == 0){
	if (status < RRC_CONNECTED)
	  ULSCH_dci->cqi_req = 0;
	else
	  ULSCH_dci->cqi_req = 1;
	//}
	//else
	//ULSCH_dci->cqi_req = 1;


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

      // schedule 4 RBs for UL
      if((cooperation_flag > 0) && (next_ue == 1))// Allocation on same set of RBs
	{
	  ULSCH_dci->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,
						     ((next_ue-1)*4),//openair_daq_vars.ue_ul_nb_rb),
						     4);//openair_daq_vars.ue_ul_nb_rb);
	}
      else
	{
	  ULSCH_dci->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,
						     (next_ue*4),//openair_daq_vars.ue_ul_nb_rb),
						     4);//openair_daq_vars.ue_ul_nb_rb);
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


//void fill_DLSCH_dci(unsigned char Mod_id,unsigned char subframe) {
//	// loop over all allocated UEs and compute frequency allocations for PDSCH
//
//	unsigned char UE_id,first_rb,nb_rb=3;
//	u16 rnti;
//	unsigned char vrb_map[100];
//	//u32 rballoc=0b11110000111111111111111111111111;
//	u32 rballoc=0b00000000000000000000000000000011;
//	u32 test=0;
//	unsigned char round;
//	unsigned char harq_pid;
//	void *DLSCH_dci=NULL;
//	DCI_PDU *DCI_pdu= &eNB_mac_inst[Mod_id].DCI_pdu;
//	int i;
//	FILE *DCIi;
//	DCIi = fopen("dcilog.txt", "a");
//
//	fprintf(DCIi,"rballoc init %u\n",rballoc);
//
//	// clear vrb_map
//	memset(vrb_map,0,100);
//
//	// SI DLSCH
//	//  printf("BCCH check\n");
//	if (eNB_mac_inst[Mod_id].bcch_active == 1) {
//		eNB_mac_inst[Mod_id].bcch_active = 0;
//
//		// randomize frequency allocation for SI
//		first_rb = (unsigned char)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
//		BCCH_alloc_pdu.rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
//		rballoc |= mac_xface->get_rballoc(BCCH_alloc_pdu.vrb_type,BCCH_alloc_pdu.rballoc);
//		fprintf(DCIi,"rballoc SI1 %u\n",rballoc);
//
//
//		vrb_map[first_rb] = 1;
//		vrb_map[first_rb+1] = 1;
//		vrb_map[first_rb+2] = 1;
//
//		add_common_dci(DCI_pdu,
//				&BCCH_alloc_pdu,
//				SI_RNTI,
//				sizeof(DCI1A_5MHz_TDD_1_6_t),
//				2,
//				sizeof_DCI1A_5MHz_TDD_1_6_t,
//				format1A);
//#ifdef DEBUG_eNB_SCHEDULER
//		msg("[MAC][eNB] Frame %d: Adding common dci for SI\n",mac_xface->frame);
//#endif
//#ifdef    DEBUG_PACKET_TRACE
//		if((DCI_pdu!=NULL)&&(DCI_pdu!=0))
//		{
//			LOG_I(OPT,"Trace_PDU_4578\n\r");
//			trace_pdu(4,DCI_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t), UE_id, SI_RNTI, subframe);
//		}
//#endif
//	}
//
//	for (i=0;i<NB_RA_PROC_MAX;i++) {
//		//    printf("RA %d check\n",i);
//		if (eNB_mac_inst[Mod_id].RA_template[i].generate_rar == 1) {
//#ifdef DEBUG_eNB_SCHEDULER
//			msg("[MAC][eNB %d] Frame %d, subframe %d: Generating RAR DCI (proc %d), format 1A (%d,%d))\n",Mod_id,mac_xface->frame, subframe,i,
//					eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt1,
//					eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits1);
//#endif
//			// randomize frequency allocation for RA
//			while (1) {
//				first_rb = (unsigned char)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
//				if ((vrb_map[first_rb] != 1) && (vrb_map[first_rb+2] != 1))
//					break;
//			}
//			vrb_map[first_rb] = 1;
//			vrb_map[first_rb+1] = 1;
//			vrb_map[first_rb+2] = 1;
//
//			((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0])->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
//			rballoc |= mac_xface->get_rballoc(((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0])->vrb_type,
//					((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0])->rballoc);
//			fprintf(DCIi,"rballoc RA1 %u\n",rballoc);
//			add_common_dci(DCI_pdu,
//					(void*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu1[0],
//					RA_RNTI,
//					eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1,
//					2,
//					eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits1,
//					eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt1);
//#ifdef DEBUG_eNB_SCHEDULER
//			msg("[MAC][eNB %d] Frame %d: Adding common dci for RA%d (RAR)\n",Mod_id,mac_xface->frame,i);
//#endif
//			// Schedule Random-Access Response
//
//			eNB_mac_inst[Mod_id].RA_template[i].generate_rar=0;
//
//		}
//		if (eNB_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup_dci == 1) {
//
//			// randomize frequency allocation for RA
//			while (1) {
//				first_rb = (unsigned char)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
//				if ((vrb_map[first_rb] != 1) && (vrb_map[first_rb+2] != 1))
//					break;
//			}
//			vrb_map[first_rb] = 1;
//			vrb_map[first_rb+1] = 1;
//			vrb_map[first_rb+2] = 1;
//			((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc= mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
//			rballoc |= mac_xface->get_rballoc(((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->vrb_type,
//					((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc);
//			fprintf(DCIi,"rballoc RA2 %u\n",rballoc);
//			add_ue_spec_dci(DCI_pdu,
//					(void*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0],
//					eNB_mac_inst[Mod_id].RA_template[i].rnti,
//					eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes2,
//					2,
//					eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits2,
//					eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt2);
//#ifdef DEBUG_eNB_SCHEDULER
//			msg("[MAC][eNB] Frame %d: Adding ue specific dci (rnti %x) for RA (ConnectionSetup)\n",mac_xface->frame,eNB_mac_inst[Mod_id].RA_template[i].rnti);
//#endif
//			eNB_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup_dci=0;
//#ifdef    DEBUG_PACKET_TRACE
//			if((DLSCH_dci!=NULL)&&(DLSCH_dci!=0))
//			{
//				LOG_I(OPT,"Trace_PDU_4\n\r");
//				trace_pdu(4,DLSCH_dci,eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1, UE_id, rnti, subframe);
//			}
//#endif
//		}
//		else if (eNB_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup==1) {
//			// check HARQ status and retransmit if necessary
//#ifdef DEBUG_eNB_SCHEDULER
//			msg("[MAC][eNB] Frame %d, subframe %d: Checking if RRCConnectionSetup was acknowledged :",mac_xface->frame,subframe);
//#endif
//			// Get candidate harq_pid from PHY
//			mac_xface->get_ue_active_harq_pid(Mod_id,eNB_mac_inst[Mod_id].RA_template[i].rnti,subframe,&harq_pid,&round,0);
//			if (round>0) {
//				// we have to schedule a retransmission
//				((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->ndi=0;
//				// randomize frequency allocation for RA
//				while (1) {
//					first_rb = (unsigned char)(taus()%(mac_xface->lte_frame_parms->N_RB_DL-4));
//					if ((vrb_map[first_rb] != 1) && (vrb_map[first_rb+2] != 1))
//						break;
//				}
//				vrb_map[first_rb] = 1;
//				vrb_map[first_rb+1] = 1;
//				vrb_map[first_rb+2] = 1;
//				((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,first_rb,3);
//				rballoc |= mac_xface->get_rballoc(((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->vrb_type,
//						((DCI1A_5MHz_TDD_1_6_t*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0])->rballoc);
//
//				fprintf(DCIi,"rballoc RA3 %u\n",rballoc);
//				add_ue_spec_dci(DCI_pdu,
//						(void*)&eNB_mac_inst[Mod_id].RA_template[i].RA_alloc_pdu2[0],
//						eNB_mac_inst[Mod_id].RA_template[i].rnti,
//						eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes2,
//						2,
//						eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits2,
//						eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt2);
//#ifdef DEBUG_eNB_SCHEDULER
//				msg("[MAC][eNB] Frame %d: Adding ue specific dci (rnti %x) for RA (ConnectionSetup Retransmission)\n",mac_xface->frame,eNB_mac_inst[Mod_id].RA_template[i].rnti);
//#endif
//			}
//			else {
//#ifdef DEBUG_eNB_SCHEDULER
//				msg("[MAC][eNB] RRCConnectionSetup acknowledged\n");
//#endif
//				eNB_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup=0;
//				eNB_mac_inst[Mod_id].RA_template[i].RA_active=0;
//			}
//#ifdef    DEBUG_PACKET_TRACE
//			if((DLSCH_dci!=NULL)&&(DLSCH_dci!=0))
//			{
//				LOG_I(OPT,"Trace_PDU_456\n\r");
//				trace_pdu(4,DLSCH_dci,eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1, UE_id, rnti, subframe);
//			}
//#endif
//		}
//	}
//
//	// UE specific DCIs
//	for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
//		//printf("UE_id: %d => status %d\n",UE_id,eNB_dlsch_info[Mod_id][UE_id].status);
//		if (eNB_dlsch_info[Mod_id][UE_id].status == S_DL_SCHEDULED) {
//
//			// clear scheduling flag
//			eNB_dlsch_info[Mod_id][UE_id].status = S_DL_WAITING;
//			rnti = find_UE_RNTI(Mod_id,UE_id);
//			mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);
//			nb_rb = eNB_mac_inst[Mod_id].UE_template[UE_id].nb_rb[harq_pid];
//
//			DLSCH_dci = (void *)eNB_mac_inst[Mod_id].UE_template[UE_id].DLSCH_DCI[harq_pid];
//
//			switch(mac_xface->get_transmission_mode(rnti)) {
//			default:
//
//			case 1:
//
//			case 2:
//
//				test=((DCI1_5MHz_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs(UE_id,nb_rb,&rballoc);
//				((DCI1_5MHz_TDD_t*)DLSCH_dci)->rah = 0;
//				add_ue_spec_dci(DCI_pdu,
//						DLSCH_dci,
//						rnti,
//						sizeof(DCI1_5MHz_TDD_t),
//						2,//aggregation,
//						sizeof_DCI1_5MHz_TDD_t,
//						format1);
//				fprintf(DCIi,"rballoc  DLSCH %u\n",rballoc);
//				fprintf(DCIi,"rballoc  DLSCH dci %u\n\n",test);
//
//
//
//				break;
//			case 4:
//
//				if (nb_rb>10) {
//					((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs(UE_id,nb_rb,&rballoc);
//					((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->rah = 0;
//					add_ue_spec_dci(DCI_pdu,
//							DLSCH_dci,
//							rnti,
//							sizeof(DCI2_5MHz_2A_M10PRB_TDD_t),
//							2,//aggregation,
//							sizeof_DCI2_5MHz_2A_M10PRB_TDD_t,
//							format2_2A_M10PRB);
//				}
//				else {
//					((DCI2_5MHz_2A_L10PRB_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs(UE_id,nb_rb,&rballoc);
//					add_ue_spec_dci(DCI_pdu,
//							DLSCH_dci,
//							rnti,
//							sizeof(DCI2_5MHz_2A_L10PRB_TDD_t),
//							2,//aggregation,
//							sizeof_DCI2_5MHz_2A_L10PRB_TDD_t,
//							format2_2A_L10PRB);
//				}
//				break;
//			}
//#ifdef    DEBUG_PACKET_TRACE
//
//			if((DLSCH_dci!=NULL)&&(DLSCH_dci!=0))
//			{
//				LOG_I(OPT,"Trace_PDU_444\n\r");
//				trace_pdu(4,DLSCH_dci,eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1, UE_id, rnti, subframe);
//			}
//#endif
//		}
//
//
//	}
//	fclose(DCIi);
//
//}




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


void fill_DLSCH_dci(unsigned char Mod_id,unsigned char subframe,u32 RBalloc) {
  // loop over all allocated UEs and compute frequency allocations for PDSCH

  unsigned char UE_id,first_rb,nb_rb=3;
  u16 rnti;
  unsigned char vrb_map[100];

  unsigned char x,y,z;
  u8 rballoc_sub[14];

  u32 rballoc=RBalloc,buff;

  u32 test=0;
  unsigned char round;
  unsigned char harq_pid;
  void *DLSCH_dci=NULL;
  DCI_PDU *DCI_pdu= &eNB_mac_inst[Mod_id].DCI_pdu;
  int i;
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
		   format1A);
#ifdef DEBUG_eNB_SCHEDULER
    msg("[MAC][eNB] Frame %d: Adding common dci for SI\n",mac_xface->frame);
#endif
#ifdef    DEBUG_PACKET_TRACE
    if((DCI_pdu!=NULL)&&(DCI_pdu!=0))
      {
	LOG_I(OPT,"Trace_PDU_4578\n\r");
	trace_pdu(4,DCI_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t), UE_id, SI_RNTI, subframe);
      }
#endif
  }

  for (i=0;i<NB_RA_PROC_MAX;i++) {
    //    printf("RA %d check\n",i);
    if (eNB_mac_inst[Mod_id].RA_template[i].generate_rar == 1) {
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB %d] Frame %d, subframe %d: Generating RAR DCI (proc %d), format 1A (%d,%d))\n",Mod_id,mac_xface->frame, subframe,i,
	  eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt1,
	  eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits1);
#endif
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
		     RA_RNTI,
		     eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1,
		     2,
		     eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bits1,
		     eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt1);
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB %d] Frame %d: Adding common dci for RA%d (RAR)\n",Mod_id,mac_xface->frame,i);
#endif
      // Schedule Random-Access Response

      eNB_mac_inst[Mod_id].RA_template[i].generate_rar=0;

    }
    if (eNB_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup_dci == 1) {

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
		      eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt2);
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB] Frame %d: Adding ue specific dci (rnti %x) for RA (ConnectionSetup)\n",mac_xface->frame,eNB_mac_inst[Mod_id].RA_template[i].rnti);
#endif
      eNB_mac_inst[Mod_id].RA_template[i].generate_rrcconnsetup_dci=0;
#ifdef    DEBUG_PACKET_TRACE
      if((DLSCH_dci!=NULL)&&(DLSCH_dci!=0))
	{
	  LOG_I(OPT,"Trace_PDU_4\n\r");
	  trace_pdu(4,DLSCH_dci,eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1, UE_id, rnti, subframe);
	}
#endif
    }
    else if (eNB_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup==1) {
      // check HARQ status and retransmit if necessary
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB] Frame %d, subframe %d: Checking if RRCConnectionSetup was acknowledged :",mac_xface->frame,subframe);
#endif
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
			eNB_mac_inst[Mod_id].RA_template[i].RA_dci_fmt2);
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB] Frame %d: Adding ue specific dci (rnti %x) for RA (ConnectionSetup Retransmission)\n",mac_xface->frame,eNB_mac_inst[Mod_id].RA_template[i].rnti);
#endif
      }
      else {
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB] RRCConnectionSetup acknowledged\n");
#endif
	eNB_mac_inst[Mod_id].RA_template[i].wait_ack_rrcconnsetup=0;
	eNB_mac_inst[Mod_id].RA_template[i].RA_active=0;
      }
#ifdef    DEBUG_PACKET_TRACE
      if((DLSCH_dci!=NULL)&&(DLSCH_dci!=0))
	{
	  LOG_I(OPT,"Trace_PDU_456\n\r");
	  trace_pdu(4,DLSCH_dci,eNB_mac_inst[Mod_id].RA_template[i].RA_dci_size_bytes1, UE_id, rnti, subframe);
	}
#endif
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
      nb_rb = eNB_mac_inst[Mod_id].UE_template[UE_id].nb_rb[harq_pid];

      DLSCH_dci = (void *)eNB_mac_inst[Mod_id].UE_template[UE_id].DLSCH_DCI[harq_pid];

      switch(mac_xface->get_transmission_mode(Mod_id,rnti)) {
      default:

      case 1:

      case 2:

	test=((DCI1_5MHz_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs(UE_id,nb_rb,&rballoc);
	((DCI1_5MHz_TDD_t*)DLSCH_dci)->rah = 0;
	add_ue_spec_dci(DCI_pdu,
			DLSCH_dci,
			rnti,
			sizeof(DCI1_5MHz_TDD_t),
			2,//aggregation,
			sizeof_DCI1_5MHz_TDD_t,
			format1);
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
			format2_2A_M10PRB);
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
	for(x=0;x<7;x++){
	  for(y=0;y<2;y++){
	    if(z < (2*6 + 1)){
	      z = 2*x + y;
	      rballoc_sub[z] = eNB_mac_inst[Mod_id].UE_template[UE_id].rballoc_sub[harq_pid][x];
	    }
	  }
	}
	((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->rballoc = allocate_prbs_sub(nb_rb,rballoc_sub);
	((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->rah = 0;
	add_ue_spec_dci(DCI_pdu,
			DLSCH_dci,
			rnti,
			sizeof(DCI2_5MHz_2D_M10PRB_TDD_t),
			2,//aggregation,
			sizeof_DCI2_5MHz_2D_M10PRB_TDD_t,
			format2_2D_M10PRB);
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



void schedule_ue_spec(unsigned char Mod_id,unsigned char subframe,u16 nb_rb_used0,unsigned char nCCE_used) {

  unsigned char UE_id,UE_id_temp;
  u16 UE_SU_MIMO = 256;
  unsigned char next_ue, next_ue_temp;
  u16 ue[2][7];
  unsigned char granted_UEs;
  u16 nCCE;
  unsigned char aggregation;
  mac_rlc_status_resp_t rlc_status;
  unsigned char header_len_dcch,header_len_dtch;
  unsigned char sdu_lcids[11],offset,num_sdus=0;
  u16 nb_rb,nb_available_rb,TBS,j,sdu_lengths[11],rnti,rnti0=0,rnti1=0,rnti_temp,rnti_k[2][7];
  unsigned char dlsch_buffer[MAX_DLSCH_PAYLOAD_BYTES];
  unsigned char round=0,round_temp=0,round_k=0;
  unsigned char harq_pid=0,harq_pid_temp=0,harq_pid_k=0;
  void *DLSCH_dci;
  LTE_eNB_UE_stats* eNB_UE_stats;
  LTE_eNB_UE_stats* eNB_UE_stats0;
  LTE_eNB_UE_stats* eNB_UE_stats1;
  LTE_eNB_UE_stats* eNB_UE_stats_temp;
  LTE_eNB_UE_stats* eNB_UE_stats_k[2][7];
  u16 sdu_length_total=0;
  unsigned char loop_count;
  unsigned char DAI;
  unsigned char k0=0,k1=0,k2=0,k3=0,k4=0,k5=0,k6=0;
  unsigned char i0=0,i1=0,i2=0,i3=0,i4=0,i5=0,i6=0;
  u8 dl_pow_off[256];
  u16 i=0,ii=0,check=0,total_rbs=0,jj=0;
  unsigned char rballoc_sub[256][7];
  u16 pre_nb_available_rbs[256];
  u8 MIMO_mode_indicator[7];
  u8 total_DL_cqi_MUMIMO = 0,total_DL_cqi_SUMIMO = 0;
  

#ifdef Pre_Processing
  
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
  
#endif


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



  //********************* Pre-processing for Scheduling UEs**************************///////
#ifdef Pre_Processing

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
       
      // Get candidate harq_pid from PHY
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid_temp,&round_temp,0);
      
  
      DLSCH_dci = (void *)eNB_mac_inst[Mod_id].UE_template[next_ue].DLSCH_DCI[harq_pid_temp];
     
      //msg ("Transmission Mode %d \n",mac_xface->get_transmission_mode(Mod_id,rnti));
      //msg("mode 5 \n");
      //exit(0);
      
      switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
      case 1:break;
      case 2:break;
      case 4:break;
      case 5:
	//msg("mode 5 \n");
	//exit(0);

	//	if(round_temp > 0)// retransmitting UEs are not selected
	//  break;
	//else
	// {
	    // if(next_ue == 0)// Selecting primary UE with maximum TBS
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
	      
	      //    if(round_k > 0)
	      //	break;
	      //else
	      //	{
		  //msg("entering mode 5\n");
		  //exit(0);
		  
		  if((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)<<14)&0xc000)== 0x4000)
		    {
		      msg("SUCCESS 0 \n");
		      //exit(-1);

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
			      
			      //dl_pow_off[ue[0][0]] = 0;
			      //dl_pow_off[ue[1][0]] = 0;
			    }
			}
		      else
			{
			  ue[0][0] = next_ue;
			  ue[1][0] = next_ue_temp;
			  
			  //dl_pow_off[ue[0][0]] = 0;
			  //dl_pow_off[ue[1][0]] = 0;
			  
			  k0 = 1;
			}
		    }
		  
		  
		  if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>2)<<14)&0xc000)== 0x4000)
		    {
		      msg("SUCCESS 1 \n");
		      //exit(-1);

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
			      
			      // dl_pow_off[ue[0][1]] = 0;
			      //dl_pow_off[ue[1][1]] = 0;
			    }
			}
		      else
			{
			  ue[0][1] = next_ue;
			  ue[1][1] = next_ue_temp;
			  
			  //dl_pow_off[ue[0][1]] = 0;
			  //dl_pow_off[ue[1][1]] = 0;
			  
			  k1 = 1;
			}
		    }
		  
		  
	    
		  if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>4)<<14)&0xc000)== 0x4000)
		    {
		      msg("SUCCESS 2 \n");
		      //exit(-1);

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
			      
			      // dl_pow_off[ue[0][2]] = 0;
			      //dl_pow_off[ue[1][2]] = 0;
			    }
			}
		      else
			{
			  ue[0][2] = next_ue;
			  ue[1][2] = next_ue_temp;
			  
			  //dl_pow_off[ue[0][2]] = 0;
			  //dl_pow_off[ue[1][2]] = 0;
			  
			  k2 = 1;
			}
		    }
		  
		  
		  
		  
		  if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>6)<<14)&0xc000)== 0x4000)
		    {
		      msg("SUCCESS 3\n");
		      //exit(-1);
		      
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
			      
			      //dl_pow_off[ue[0][3]] = 0;
			      //dl_pow_off[ue[1][3]] = 0;
			    }
			}
		      else
			{
			  ue[0][3] = next_ue;
			  ue[1][3] = next_ue_temp;
			  
			  //dl_pow_off[ue[0][3]] = 0;
			  //dl_pow_off[ue[1][3]] = 0;
			  
			  k3 = 1;
			}
		    }
		  
		  
		  
		  if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>8)<<14)&0xc000)== 0x4000)
		    {
		      msg("SUCCESS 4\n");
		      //exit(-1);

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
				  
			      //dl_pow_off[ue[0][4]] = 0;
			      //dl_pow_off[ue[1][4]] = 0;
			    }
			}
		      else
			{
			  ue[0][4] = next_ue;
			  ue[1][4] = next_ue_temp;
			      
			  //dl_pow_off[ue[0][4]] = 0;
			  //dl_pow_off[ue[1][4]] = 0;
			      
			  k4= 1;
			}
		    }
		  
		  
		  
		  if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>10)<<14)&0xc000)== 0x4000)
		    {
		      msg("SUCCESS 5 \n");
		      //exit(-1);

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
				  
			      //dl_pow_off[ue[0][5]] = 0;
			      //dl_pow_off[ue[1][5]] = 0;
			    }
			}
		      else
			{
			  ue[0][5] = next_ue;
			  ue[1][5] = next_ue_temp;
			      
			  // dl_pow_off[ue[0][5]] = 0;
			  // dl_pow_off[ue[1][5]] = 0;
			      
			  k5= 1;
			}
		    }
		  
		  
		  
		  if(((((eNB_UE_stats_temp->DL_pmi_single^eNB_UE_stats->DL_pmi_single)>>12)<<14)&0xc000)== 0x4000)
		    {
		      msg ("SUCCESS 6 \n");
		      //exit(-1);

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
				  
			      //  dl_pow_off[ue[0][6]] = 0;
			      //dl_pow_off[ue[1][6]] = 0;
			    }
			}
		      else
			{
			  ue[0][6] = next_ue;
			  ue[1][6] = next_ue_temp;
			  
			  //dl_pow_off[ue[0][6]] = 0;
			  //dl_pow_off[ue[1][6]] = 0;
			  
			  k6= 1;
			}
		    }
		  //}
		  //}
	    }
	    break;
      case 6:break;
      case 7:
	break;
      default:
	break;
      }
    }
   


  //switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
  //case 1:break;
  //case 2:break;
  //case 4:break;
  //case 5:
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
  
    
    
  for(i=0;i<7;i++){

    if(MIMO_mode_indicator[i] == 0){
      rnti0 = find_UE_RNTI(Mod_id,ue[0][i]);
      rnti1 = find_UE_RNTI(Mod_id,ue[1][i]);
      eNB_UE_stats0 = mac_xface->get_eNB_UE_stats(Mod_id,rnti0);
      eNB_UE_stats1 = mac_xface->get_eNB_UE_stats(Mod_id,rnti1);
      total_DL_cqi_MUMIMO = total_DL_cqi_MUMIMO + eNB_UE_stats0->DL_cqi[0] + eNB_UE_stats1->DL_cqi[0];
    }
    else if (MIMO_mode_indicator[i] == 1){
      rnti0 = find_UE_RNTI(Mod_id,ue[0][i]);
      eNB_UE_stats0 = mac_xface->get_eNB_UE_stats(Mod_id,rnti0);
      total_DL_cqi_SUMIMO = total_DL_cqi_SUMIMO + eNB_UE_stats0->DL_cqi[0];
    }
  }




  if((MIMO_mode_indicator[0] == 0)|| (MIMO_mode_indicator[1] == 0) || (MIMO_mode_indicator[2] == 0) ||  (MIMO_mode_indicator[3] == 0) ||
     (MIMO_mode_indicator[4] == 0)|| (MIMO_mode_indicator[5] == 0) || (MIMO_mode_indicator[6] == 0)){
 

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
	}
      }
    rnti = find_UE_RNTI(Mod_id,UE_SU_MIMO);
    eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);
    if((7*eNB_UE_stats->DL_cqi[0]) > (total_DL_cqi_SUMIMO + total_DL_cqi_MUMIMO)){
    
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
  
  PHY_vars_eNB_g[Mod_id]->check_for_total_transmissions = PHY_vars_eNB_g[Mod_id]->check_for_total_transmissions + 1;

  
  
  
  for(UE_id=0;UE_id<granted_UEs;UE_id++){
    PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].dl_pow_off = dl_pow_off[UE_id];
    msg("******************Scheduling Information for UE%d ************************\n",UE_id);
    msg("dl power offset UE%d = %d \n",UE_id,dl_pow_off[UE_id]);
    msg("***********RB Alloc for every subband for UE%d ***********\n",UE_id);
    for(i=0;i<7;i++){
      PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].rballoc_sub[i] = rballoc_sub[UE_id][i];
      msg("RB Alloc for UE%d and Subband%d = %d\n",UE_id,i,rballoc_sub[UE_id][i]);
    }
    PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].pre_nb_available_rbs = pre_nb_available_rbs[UE_id];
    msg("Total RBs allocated for UE%d = %d\n",UE_id,pre_nb_available_rbs[UE_id]);
  }
  
#endif


  for (UE_id=0;UE_id<granted_UEs;UE_id++) {
    	  
    //msg("\n \n \n \n \n \n \n \nAvailable RBs for USER %d = %d\n\n \n \n \n \n \n \n \n",UE_id,pre_nb_available_rbs[UE_id]);
#ifdef Pre_Processing // Number of pre-allocated PRBs allocated to each UE
    rnti = find_UE_RNTI(Mod_id,UE_id);

    eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,rnti);
	  
    if (eNB_UE_stats==NULL)
      mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");
	  
    eNB_UE_stats->dlsch_mcs1 = openair_daq_vars.target_ue_dl_mcs;
	  
    // Get candidate harq_pid from PHY
    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);
    printf("Got harq_pid %d, round %d\n",harq_pid,round);
	  
    // Note this code is for a specific DCI format
    DLSCH_dci = (void *)eNB_mac_inst[Mod_id].UE_template[next_ue].DLSCH_DCI[harq_pid];
	  
    //if (round > 0) 
    // break;
    //else {
      switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
      case 5:
	nb_available_rb = pre_nb_available_rbs[UE_id];
	break;
      default:
	break;
      }
      //}
#endif
    	  
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
	  
    eNB_UE_stats->dlsch_mcs1 = openair_daq_vars.target_ue_dl_mcs;
	  
    // Get candidate harq_pid from PHY
    mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);
    printf("Got harq_pid %d, round %d\n",harq_pid,round);
	  
    // Note this code is for a specific DCI format
    DLSCH_dci = (void *)eNB_mac_inst[Mod_id].UE_template[next_ue].DLSCH_DCI[harq_pid];
	  
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
	  msg("[MAC] Retransmission : harq_pid %d, round %d, dai %d, mcs %d\n",harq_pid,round,(eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1),((DCI1_5MHz_TDD_t*)DLSCH_dci)->mcs);
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
	  ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->ndi1 = 0;
	  ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->dai = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->dl_power_off = dl_pow_off[UE_id];
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
      TBS = mac_xface->get_TBS(eNB_UE_stats->DL_cqi[0],nb_available_rb);
      
      // check first for RLC data on DCCH
      header_len_dcch = 2+1+1; // 2 bytes DCCH SDU subheader + timing advance subheader + timing advance command
      
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB %d] Requesting %d bytes from RLC (mcs %d, nb_available_rb %d)\n",Mod_id,TBS-header_len_dcch,
	  eNB_UE_stats->DL_cqi[0],nb_available_rb);
#endif
      rlc_status = mac_rlc_status_ind(Mod_id,DCCH+(MAX_NUM_RB*next_ue),
				      (TBS-header_len_dcch)); // transport block set size
	    
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
      /*
	if ((sdu_lengths[0] + header_len_dcch )< TBS) {
	// repeat to see if additional data generated due to request
	#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] DCCH has %d bytes to send (buffer %d, header %d)\n",Mod_id,rlc_status.bytes_in_buffer,sdu_lengths[0],header_len_dcch);
	#endif
	rlc_status = mac_rlc_status_ind(Mod_id,DCCH+(MAX_NUM_RB*next_ue),
	(TBS-header_len_dcch-sdu_lengths[0]));
	      
	sdu_lengths[0] += Mac_rlc_xface->mac_rlc_data_req(Mod_id,
	DCCH+(MAX_NUM_RB*next_ue),
	(char*)&dlsch_buffer[sdu_lengths[0]]);
	#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] Got %d bytes from RLC\n",Mod_id,sdu_lengths[0]);
	#endif
	}
      */
	    
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
	    
      // check for DCCH1 and update header information (assume 2 byte sub-header)
      rlc_status = mac_rlc_status_ind(Mod_id,DCCH+1+(MAX_NUM_RB*next_ue),
				      (TBS-header_len_dcch-2)); // transport block set size
	    
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB %d] DCCH1 has %d bytes to send (buffer %d, header %d)\n",Mod_id,rlc_status.bytes_in_buffer,sdu_lengths[0],header_len_dcch+2);
#endif
      if (rlc_status.bytes_in_buffer > 0) {
	      
	sdu_lengths[num_sdus] += Mac_rlc_xface->mac_rlc_data_req(Mod_id,
								 DCCH+1+(MAX_NUM_RB*next_ue),
								 (char *)&dlsch_buffer[sdu_lengths[0]]);
	sdu_lcids[num_sdus] = DCCH1;
	sdu_length_total += sdu_lengths[num_sdus];
	header_len_dcch += 2;
	num_sdus++;
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] Got %d bytes from RLC\n",Mod_id,sdu_lengths[0]);
#endif
      }
      // check for DTCH and update header information
      // here we should loop over all possible DTCH
	    
      header_len_dtch = 3; // 3 bytes DTCH SDU subheader
	    
      rlc_status = mac_rlc_status_ind(Mod_id,DTCH+(MAX_NUM_RB*next_ue),
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
	offset = generate_dlsch_header((unsigned char*)eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0],
				       // offset = generate_dlsch_header((unsigned char*)eNB_mac_inst[0].DLSCH_pdu[0][0].payload[0],
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
	memcpy(&eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0][offset],dlsch_buffer,sdu_length_total);
	// memcpy(&eNB_mac_inst[0].DLSCH_pdu[0][0].payload[0][offset],dcch_buffer,sdu_lengths[0]);
	      
	// Now compute number of required RBs for total sdu length
	// Assume RAH format 2
#ifdef FULL_BUFFER
	nb_rb = nb_available_rb;
#else
	nb_rb = 2;
#endif
	
	TBS = mac_xface->get_TBS(eNB_UE_stats->DL_cqi[0],nb_rb);
	
#ifndef FULL_BUFFER      
	
	while (TBS < (sdu_length_total + offset))  {
	  nb_rb += 2;  // to be replaced with RA allocation size for other than 25 PRBs!!!!!!!
	  if (nb_rb>mac_xface->lte_frame_parms->N_RB_DL) { // if we've gone beyond the maximum number of RBs
	    // (can happen if N_RB_DL is odd)
	    TBS = mac_xface->get_TBS(eNB_UE_stats->DL_cqi[0],mac_xface->lte_frame_parms->N_RB_DL);
	    nb_rb = mac_xface->lte_frame_parms->N_RB_DL;
	    break;
	  }
	  TBS = mac_xface->get_TBS(eNB_UE_stats->DL_cqi[0],nb_rb);
	}
#endif
	
	msg("CHECK1: RBs for UE%d = %d********************************\n\n",next_ue,nb_available_rb);
	msg("CHECK2: RBs for UE%d = %d********************************\n\n",next_ue,nb_rb);
	      
#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB %d] Generated DLSCH header (mcs %d, TBS %d, nb_rb %d)\n",
	    Mod_id,eNB_UE_stats->DL_cqi[0],TBS,nb_rb);
	// msg("[MAC][eNB ] Reminder of DLSCH with random data %d %d %d %d \n",
	//	TBS, sdu_length_total, offset, TBS-sdu_length_total-offset);
#endif
	      
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
	if((eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0]!=NULL)&&(eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0]!=0))
	  {
	    LOG_I(OPT,"Trace_PDU_474\n\r");
	    trace_pdu(4,eNB_mac_inst[Mod_id].DLSCH_pdu[(unsigned char)next_ue][0].payload[0],TBS/*sdu_length_total+offset offset*/, next_ue, rnti, subframe);
	  }
#endif
	switch (mac_xface->get_transmission_mode(Mod_id,rnti)) {
	case 1:
	case 2:
	default:
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->mcs = eNB_UE_stats->DL_cqi[0];
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->ndi = 1;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->rv = 0;
	  ((DCI1_5MHz_TDD_t*)DLSCH_dci)->dai      = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  break;
	case 4:
	  //  if (nb_rb>10) {
	  ((DCI2_5MHz_2A_M10PRB_TDD_t*)DLSCH_dci)->mcs1 = eNB_UE_stats->DL_cqi[0];
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

	  ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->mcs1 = eNB_UE_stats->DL_cqi[0];
	  if(((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->mcs1 > 9)
	    ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->mcs1 = 9;
	  ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->harq_pid = harq_pid;
	  ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->ndi1 = 1;
	  ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->rv1 = round&3;
	  ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->dai = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
	  ((DCI2_5MHz_2D_M10PRB_TDD_t*)DLSCH_dci)->dl_power_off = dl_pow_off[UE_id];
				    
	  for(i=0;i<7;i++) // for indicating the rballoc for each sub-band
	    eNB_mac_inst[Mod_id].UE_template[next_ue].rballoc_sub[harq_pid][i] = rballoc_sub[next_ue][i];

	  break;
	case 6:
	  break;
	}
				
				
				
      }
	    
      else {  // There is no data from RLC or MAC header, so don't schedule
	      
      }
    }
	  
    DAI = (eNB_mac_inst[Mod_id].UE_template[next_ue].DAI-1)&3;
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

void eNB_dlsch_ulsch_scheduler(unsigned char Mod_id,unsigned char cooperation_flag,unsigned char subframe) {

  unsigned char nprb=0,nCCE=0;
  u32 RBalloc=0;

  DCI_PDU *DCI_pdu= &eNB_mac_inst[Mod_id].DCI_pdu;
#ifdef DEBUG_eNB_SCHEDULER
  msg("[MAC][eNB %d] In MAC scheduler entry point\n",Mod_id);
#endif
  // clear DCI and BCCH contents before scheduling
  DCI_pdu->Num_common_dci  = 0;
  DCI_pdu->Num_ue_spec_dci = 0;
  eNB_mac_inst[Mod_id].bcch_active = 0;

  msg("[MAC][eNB] inst %d scheduler subframe %d\n",Mod_id, subframe);

  //LOG_I (MAC, "eNB inst %d scheduler subframe %d nCCE %d \n",Mod_id, subframe, mac_xface->get_nCCE_max(Mod_id) );
  Mac_rlc_xface->frame= mac_xface->frame;
  Rrc_xface->Frame_index=Mac_rlc_xface->frame;

  Mac_rlc_xface->pdcp_run();
#ifdef ICIC
  // navid: the following 2 functions does not work properly when there is user-plane traffic 
  UpdateSBnumber(Mod_id);
  RBalloc=Get_Cell_SBMap(Mod_id);
#endif 
  switch (subframe) {
  case 0:
    //test navid

    //add_common_dci(DCI_PDU *DCI_pdu,void *pdu,u16 rnti,unsigned char dci_size_bytes,unsigned char aggregation,unsigned char dci_size_bits,unsigned char dci_fmt)


    schedule_ulsch(Mod_id,cooperation_flag,subframe,&nCCE);
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
    fill_DLSCH_dci(Mod_id,subframe,RBalloc);

    break;

  case 6:

    break;

  case 7:

    schedule_ue_spec(Mod_id,subframe,0,0);
    fill_DLSCH_dci(Mod_id,subframe,RBalloc);
    break;



  case 8:

    schedule_RA(Mod_id,subframe,&nprb,&nCCE);
    //    schedule_ue_spec(Mod_id,subframe,nprb,nCCE);
    fill_DLSCH_dci(Mod_id,subframe,RBalloc);
    // Schedule UL subframe
    //schedule_ulsch(Mod_id,subframe,&nCCE);

    break;

  case 9:

    // Schedule UL subframe
    //    schedule_ulsch(Mod_id,subframe,&nCCE);


    break;

  }

}
