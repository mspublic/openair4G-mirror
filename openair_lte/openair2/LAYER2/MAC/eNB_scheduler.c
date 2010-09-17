#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"

#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"

#include "RRC/MESH/extern.h"

#define DLSCH_RB_ALLOC 0x1fbf  // skip DC RB (total 23/25 RBs)
#define DLSCH_RB_ALLOC_12 0x0aaa  // skip DC RB (total 12/25 RBs)
#define DLSCH_RB_ALLOC_6 0x0999  // skip DC RB (total 6/25 RBs)

static char eNB_generate_rar     = 0;  // flag to indicate start of RA procedure
static char eNB_generate_rrcconnsetup = 0;  // flag to indicate termination of RA procedure (mirror response)

//#define DEBUG_eNB_SCHEDULER
#define DEBUG_HEADER_PARSING 1

extern inline unsigned int taus(void);


void initiate_ra_proc(u8 Mod_id, u16 preamble_index) {
  msg("[MAC][eNB Proc] Initiating RA procedure for index %d\n",preamble_index);
  eNB_generate_rar=1;
  eNB_generate_rrcconnsetup=0;
}

void terminate_ra_proc(u8 Mod_id,u16 UE_id,unsigned char *l3msg) {
  msg("[MAC][eNB Proc] Terminating RA procedure for UE index %d, Received RRCConnRequest %x,%x,%x,%x,%x,%x\n",UE_id,
      l3msg[0],l3msg[1],l3msg[2],l3msg[3],l3msg[4],l3msg[5]);
  memcpy(mac_xface->eNB_UE_stats[0][0].cont_res_id,l3msg,6);

  if (Is_rrc_registered == 1)
    Rrc_xface->mac_rrc_data_ind(Mod_id,CH_mac_inst[Mod_id].Ccch_lchan.Lchan_info.Lchan_id.Index,
				l3msg,6,0);

  eNB_generate_rrcconnsetup = 1;
}

DCI_PDU *get_dci_sdu(u8 Mod_id,u8 subframe) {

  return(&CH_mac_inst[Mod_id].DCI_pdu);

}

u8 find_UE_id(u8 Mod_id,u16 rnti) {

  return(0);
}
 
u8 *get_dlsch_sdu(u8 Mod_id,u16 rnti,u8 TBindex) {

  u8 UE_id = find_UE_id(Mod_id,rnti);

  return(&CH_mac_inst[Mod_id].DLSCH_pdu[UE_id][TBindex].payload[0]);

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
    else {  // This is a control element subheader
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

void rx_sdu(u8 Mod_id,u16 UE_id,u8 *sdu) {

  unsigned char rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr,j;
  unsigned char rx_lcids[MAX_NUM_RB];
  unsigned short rx_lengths[MAX_NUM_RB];

#ifdef DEBUG_HEADER_PARSING
  msg("[MAC][eNB RX] Received ulsch sdu from L1, parsing header\n");
#endif
  payload_ptr = parse_ulsch_header(sdu,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths);

#ifdef DEBUG_HEADER_PARSING
  msg("Num CE %d, Num SDU %d\n",num_ce,num_sdu);
#endif

  for (i=0;i<num_ce;i++) {

      switch (rx_ces[i]) {

      }  
  }

  for (i=0;i<num_sdu;i++) {
#ifdef DEBUG_HEADER_PARSING
    msg("SDU %d : LCID %d, length %d\n",i,rx_lcids[i],rx_lengths[i]);
#endif
    if (rx_lcids[i] == DCCH) {
      //      if(CH_mac_inst[Mod_id].Dcch_lchan[UE_id].Active==1){
      msg("offset: %d\n",(u8)((u8*)payload_ptr-sdu));
      for (j=0;j<rx_lengths[i];j++)
	printf("%x ",payload_ptr[j]);
      printf("\n");
      Mac_rlc_xface->mac_rlc_data_ind(Mod_id,
				      DCCH,
				      payload_ptr,
				      DCCH_LCHAN_DESC.transport_block_size,
				      rx_lengths[i]/DCCH_LCHAN_DESC.transport_block_size,
				      NULL);//(unsigned int*)crc_status);
      
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

  if (timing_advance_cmd != 255) {
    //    printf("Timing advance : %d (first_element %d)\n",timing_advance_cmd,first_element);
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
    msg("[MAC][eNB Scheduler] UE_CONT_RES Header\n");
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
    //    printf("sdu subheader %d (lcid %d, %d bytes)\n",i,sdu_lcids[i],sdu_lengths[i]);

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
      //      printf("short sdu\n");
    }
    else {
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->R    = 0;
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->E    = 0; 
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->F    = 1;     
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->LCID = sdu_lcids[i];
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L    = sdu_lengths[i]&0x7f;
      ((SCH_SUBHEADER_LONG *)mac_header_ptr)->L2   = (sdu_lengths[i]>>7)&0xff;

      last_size=3;
      //      printf("long sdu\n");
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

  mac_header_ptr+=last_size;
  if (mac_header_control_elements,ce_ptr-mac_header_control_elements > 0) {
    memcpy((void*)mac_header_ptr,mac_header_control_elements,ce_ptr-mac_header_control_elements);
    mac_header_ptr+=(unsigned char)(ce_ptr-mac_header_control_elements);
  }
  return((unsigned char*)mac_header_ptr - mac_header);

}

// loop over users in PUSCH
void eNB_dlsch_ulsch_scheduler(u8 Mod_id,u8 subframe) {

  mac_rlc_status_resp_t rlc_status;
  u8 header_len;
  u8 lcid,sdu_lcids[11],offset,num_sdus=0;
  u16 nb_rb,TBS,payload_size,i,j,sdu_lengths[11],rrc_sdu_length;
  u8 cont_res_id[6];
  u8 dcch_buffer[32];
  u8 current_harq_round;

  CH_mac_inst[0].DCI_pdu.Num_common_dci  = 0;	
  CH_mac_inst[0].DCI_pdu.Num_ue_spec_dci = 0;

	
  for (i=0;i<1;i++) {


    switch (subframe) {
    case 0:
      // Schedule DL control 
      memcpy(&CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_pdu[0],
	     &CCCH_alloc_pdu,
	     sizeof(DCI1A_5MHz_TDD_1_6_t));
      
      CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
      CH_mac_inst[0].DCI_pdu.dci_alloc[0].L          = 3;
      CH_mac_inst[0].DCI_pdu.dci_alloc[0].rnti       = SI_RNTI;
      CH_mac_inst[0].DCI_pdu.dci_alloc[0].format     = format1A;
      
      CH_mac_inst[0].DCI_pdu.Num_common_dci  = 1;	
      
#ifdef DEBUG_eNB_SCHEDULER
      debug_msg("[MAC] Frame %d, subframe %d: Generated CCCH DCI, format 1A\n",mac_xface->frame, subframe);
#endif
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
      break;
      
    case 6:

      //      msg("DLSCH subframe: UE mode %s\n",mode_string[eNB_UE_stats[0][i].mode]);

      if ((mac_xface->eNB_UE_stats[0][i].mode == PUSCH) && (eNB_generate_rrcconnsetup == 0)) {

	current_harq_round = mac_xface->get_ue_harq_round(0,mac_xface->eNB_UE_stats[0][i].crnti,0);

	if (openair_daq_vars.dlsch_rate_adaptation == 0)
	  DLSCH_alloc_pdu2.mcs1   = openair_daq_vars.target_ue_dl_mcs;
	else {

	  // this is the CQI-based rate adaptation
	  // if we got an increase in mcs from UE, make sure we don't increment too much
	  if ((mac_xface->eNB_UE_stats[0][0].DL_cqi[0]<<1)+mac_xface->eNB_UE_stats[0][0].dlsch_mcs_offset > (DLSCH_alloc_pdu2.mcs1+1))
	    mac_xface->eNB_UE_stats[0][0].dlsch_mcs_offset=0;
	  DLSCH_alloc_pdu2.mcs1   = (mac_xface->eNB_UE_stats[0][0].DL_cqi[0]<<1)+mac_xface->eNB_UE_stats[0][0].dlsch_mcs_offset;
	  if (DLSCH_alloc_pdu2.mcs1 == 28)
	    DLSCH_alloc_pdu2.mcs1=0;
	}
	//      msg("mcs: %d, mcs_offset %d\n",DLSCH_alloc_pdu2.mcs1,dlsch_mcs_offset);
	
#ifdef USER_MODE
	DLSCH_alloc_pdu2.rballoc = DLSCH_RB_ALLOC;
	nb_rb=23;
#else
	if (DLSCH_alloc_pdu2.mcs1 > 14)
	  DLSCH_alloc_pdu2.mcs1 = 14;
	
	if (DLSCH_alloc_pdu2.mcs1 > 10) {
	  DLSCH_alloc_pdu2.rballoc = DLSCH_RB_ALLOC_12;
	  nb_rb=12;
	}
	else {
	  DLSCH_alloc_pdu2.rballoc = DLSCH_RB_ALLOC;
	  nb_rb=23;
	}
#endif
	
	if (openair_daq_vars.dlsch_transmission_mode == 6)
	  DLSCH_alloc_pdu2.tpmi   = 5;
	else
	  DLSCH_alloc_pdu2.tpmi   = 0;
	
	if (current_harq_round == 0) {
	  DLSCH_alloc_pdu2.ndi1 = 1;
	}
	else {
	  DLSCH_alloc_pdu2.ndi1 = 0;
	}
	
	mac_xface->eNB_UE_stats[0][0].dlsch_trials[current_harq_round]++;
	
	DLSCH_alloc_pdu2.rv1 = current_harq_round&3;
#ifdef DEBUG_eNB_SCHEDULER
	msg("DLSCH_pdu (rballoc %x,mcs %d, ndi %d, rv %d)\n",DLSCH_alloc_pdu2.rballoc,DLSCH_alloc_pdu2.mcs1,DLSCH_alloc_pdu2.ndi1,DLSCH_alloc_pdu2.rv1);
#endif
	memcpy(&CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
	CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
	CH_mac_inst[0].DCI_pdu.dci_alloc[0].L          = 3;
	CH_mac_inst[0].DCI_pdu.dci_alloc[0].rnti       = mac_xface->eNB_UE_stats[0][0].crnti;
	CH_mac_inst[0].DCI_pdu.dci_alloc[0].format     = format2_2A_M10PRB;
	CH_mac_inst[0].DCI_pdu.Num_ue_spec_dci = 1;	
	
	debug_msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Generated DLSCH DCI, format 2_2A_M10PRB\n",mac_xface->frame,subframe);
 
	// copy MAC header and SDU
#ifdef TBS_FIX
	TBS = 3*dlsch_tbs25[get_I_TBS(DLSCH_alloc_pdu2.mcs1)][nb_rb-1]/4;
	TBS = TBS>>3;
#else
	TBS = dlsch_tbs25[get_I_TBS(DLSCH_alloc_pdu2.mcs1)][nb_rb-1];
	TBS = TBS>>3;
#endif

	// check first for RLC data on DCCH
	header_len = 2+1+1; // 2 bytes DCCH SDU subheader + timing advance subheader + timing advance command 
	rlc_status = mac_rlc_status_ind(0,DCCH,
					(TBS-header_len)/DCCH_LCHAN_DESC.transport_block_size,
					DCCH_LCHAN_DESC.transport_block_size);

	if (rlc_status.bytes_in_buffer>0) {
	  msg("[MAC][eNB] DCCH has %d bytes to send (buffer %d, header %d)\n",rlc_status.bytes_in_buffer,sdu_lengths[0],header_len);

	  sdu_lengths[0] = Mac_rlc_xface->mac_rlc_data_req(0,
							   DCCH,
							   dcch_buffer);
	  sdu_lcids[0] = DCCH;
	  msg("[MAC][eNB] Got %d bytes for DCCH :",sdu_lengths[0]);

	  num_sdus = 1;

	  for (j=0;j<sdu_lengths[0];j++)
	    msg("%x ",dcch_buffer[j]);
	  msg("\n");
	}
	
	// check for DTCH (later) and update header information

	offset = generate_dlsch_header((unsigned char*)CH_mac_inst[0].DLSCH_pdu[0][0].payload[0],
				       num_sdus,              //num_sdus
				       sdu_lengths,  //
				       sdu_lcids,                                 
				       255,                                   // no drx
				       mac_xface->eNB_UE_stats[0][0].UE_timing_offset/4, // timing advance
				       NULL,                                  // contention res id
				       0);                                    // no padding

	msg("[MAC][eNB] Generate header : num_sdus %d, sdu_lengths[0] %d, sdu_lcids[0] %d => payload offset %d\n",
	    num_sdus,sdu_lengths[0],sdu_lcids[0],offset);
 
	// cycle through SDUs and place in dlsch_buffer
	memcpy(&CH_mac_inst[0].DLSCH_pdu[0][0].payload[0][offset],dcch_buffer,sdu_lengths[0]);

	
	msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Generated DLSCH header (mcs %d, TBS %d, nb_rb %d)\n",mac_xface->frame,subframe,DLSCH_alloc_pdu2.mcs1,TBS,nb_rb);
	

	// fill remainder of DLSCH with random data 
	for (j=0;j<(TBS-sdu_lengths[0]-offset);j++)
	  CH_mac_inst[0].DLSCH_pdu[0][0].payload[0][offset+sdu_lengths[0]+j] = (char)(taus()&0xff);

    }      
    break;
      
    case 8:
      
      if (eNB_generate_rar == 1) { 
	msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Generated RAR DCI, format 1A\n",mac_xface->frame, subframe); 
	
	// Schedule Random-Access Response 
	memcpy(&CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_pdu[0],&RA_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
	CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
	CH_mac_inst[0].DCI_pdu.dci_alloc[0].L          = 3;
	CH_mac_inst[0].DCI_pdu.dci_alloc[0].rnti       = RA_RNTI;
	CH_mac_inst[0].DCI_pdu.dci_alloc[0].format     = format1A;


	CH_mac_inst[0].DCI_pdu.Num_common_dci = 1;	
	CH_mac_inst[0].DCI_pdu.Num_ue_spec_dci = 0;	
	eNB_generate_rar=0;
      }

      if (eNB_generate_rrcconnsetup == 1) {

	// check for RRCConnSetup Message

	if (Is_rrc_registered == 1) {
	  rrc_sdu_length = Rrc_xface->mac_rrc_data_req(0,
						     0,1,
						     &CH_mac_inst[0].Ccch_lchan.Lchan_info.Current_payload_tx[0],
						     0);
	}

	if (rrc_sdu_length>0) {
	  msg("[MAC][eNB] Frame %d, subframe %d: Received %d bytes for RRCConnectionSetup: \n",mac_xface->frame,subframe,rrc_sdu_length);
	  for (i=0;i<rrc_sdu_length;i++)
	    msg("%x ",(u8)CH_mac_inst[0].Ccch_lchan.Lchan_info.Current_payload_tx[i]);
	  msg("\n");
	  msg("[MAC][eNB] Frame %d, subframe %d: Generated DLSCH (RRCConnectionSetup) DCI, format 1A\n",mac_xface->frame, subframe); 
	  
	  // Schedule Reflection of Connection request 
	  memcpy(&CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu1A,sizeof(DCI1A_5MHz_TDD_1_6_t));
	  CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
	  CH_mac_inst[0].DCI_pdu.dci_alloc[0].L          = 3;
	  CH_mac_inst[0].DCI_pdu.dci_alloc[0].rnti       = mac_xface->eNB_UE_stats[0][0].crnti;
	  CH_mac_inst[0].DCI_pdu.dci_alloc[0].format     = format1A;
	  //	dlsch_eNb_1A_active = 1;
	  //	nb_dci_ue_spec++;
	  CH_mac_inst[0].DCI_pdu.Num_ue_spec_dci = 1;	
	  
	  
	  eNB_generate_rrcconnsetup=0;
	  lcid=0;
	  offset = generate_dlsch_header((unsigned char*)CH_mac_inst[0].DLSCH_pdu[0][0].payload[0],
					 1,              //num_sdus
					 &rrc_sdu_length,      //
					 &lcid,          // sdu_lcid                   
					 255,                                   // no drx
					 255,                                   // no timing advance
					 mac_xface->eNB_UE_stats[0][0].cont_res_id,        // contention res id
					 0);                                    // no padding
	  msg("offset %d\n",(u8)offset);
	  memcpy((void*)&CH_mac_inst[0].DLSCH_pdu[0][0].payload[0][(u8)offset],
		 &CH_mac_inst[0].Ccch_lchan.Lchan_info.Current_payload_tx[0],
		 rrc_sdu_length);
	}
      }

      break;
      
    case 9:
      
      // Schedule UL subframe
      // get UL harq_pid for subframe+4
      
    if (mac_xface->eNB_UE_stats[0][i].mode == PUSCH) {
      
      
      //if ((mac_xface->frame&1)==0) {
      if (UL_alloc_pdu.ndi == 1)
	UL_alloc_pdu.mcs     = openair_daq_vars.target_ue_ul_mcs;
      UL_alloc_pdu.rballoc = computeRIV(mac_xface->lte_frame_parms->N_RB_UL,9,openair_daq_vars.ue_ul_nb_rb);
      memcpy(&CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
      CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
      CH_mac_inst[0].DCI_pdu.dci_alloc[0].L          = 3;
      CH_mac_inst[0].DCI_pdu.dci_alloc[0].rnti       = mac_xface->eNB_UE_stats[0][0].crnti;
      CH_mac_inst[0].DCI_pdu.dci_alloc[0].format     = format0;

      CH_mac_inst[0].DCI_pdu.Num_ue_spec_dci = 1;
      CH_mac_inst[0].DCI_pdu.Num_common_dci  = 0;	
      
#ifdef DEBUG_eNB_SCHEDULER
      debug_msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Generated ULSCH DCI, format 0\n",mac_xface->frame,subframe);
#endif
      
    }      
    break;
      
    case 7:
      

      break;
      
    }
    
  }
  

}
