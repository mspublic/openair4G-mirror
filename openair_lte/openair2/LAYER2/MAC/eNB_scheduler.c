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

//static char eNB_generate_rar     = 0;  // flag to indicate start of RA procedure
//static char eNB_generate_rrcconnsetup = 0;  // flag to indicate termination of RA procedure (mirror response)

#define DEBUG_eNB_SCHEDULER
#define DEBUG_HEADER_PARSING 1

extern inline unsigned int taus(void);


void initiate_ra_proc(u8 Mod_id, u16 preamble_index,s16 timing_offset,u8 sect_id) {
  msg("[MAC][eNB Proc] Initiating RA procedure for index %d\n",preamble_index);
  CH_mac_inst[Mod_id].RA_template[0].generate_rar=1;
  CH_mac_inst[Mod_id].RA_template[0].generate_rrcconnsetup=0;
  CH_mac_inst[Mod_id].RA_template[0].timing_offset=timing_offset;
  CH_mac_inst[Mod_id].RA_template[0].rnti = taus();

}

void terminate_ra_proc(u8 Mod_id,u16 rnti,unsigned char *l3msg) {
  msg("[MAC][eNB Proc] Terminating RA procedure for UE rnti %x, Received RRCConnRequest %x,%x,%x,%x,%x,%x\n",rnti,
      l3msg[0],l3msg[1],l3msg[2],l3msg[3],l3msg[4],l3msg[5]);
  memcpy(&CH_mac_inst[Mod_id].RA_template[0].cont_res_id[0],l3msg,6);

  if (Is_rrc_registered == 1)
    Rrc_xface->mac_rrc_data_ind(Mod_id,CCCH,l3msg,6,0);

  CH_mac_inst[Mod_id].RA_template[0].generate_rrcconnsetup = 1;
}

DCI_PDU *get_dci_sdu(u8 Mod_id,u8 subframe) {

  return(&CH_mac_inst[Mod_id].DCI_pdu);

}

s8 find_UE_id(u8 Mod_id,u16 rnti) {

  u8 i;

  for (i=0;i<NB_CNX_CH;i++) {
    if (CH_mac_inst[Mod_id].UE_template[i].rnti==rnti) {
      return(i);
    }
  }
  return(-1);

}

s8 add_new_ue(u8 Mod_id, u16 rnti) {
  u8 i;

  for (i=0;i<NB_CNX_CH;i++) {
    msg("UE_id %d : rnti %x\n",i,CH_mac_inst[Mod_id].UE_template[i].rnti);
    if (CH_mac_inst[Mod_id].UE_template[i].rnti==0) {
      CH_mac_inst[Mod_id].UE_template[i].rnti=rnti;
      return((s8)i);
    }
  }
  return(-1);
}

u8 *get_dlsch_sdu(u8 Mod_id,u16 rnti,u8 TBindex) {

  u8 UE_id = find_UE_id(Mod_id,rnti);

  msg("[MAC][eNB] Mod_id %d : Get DLSCH sdu for rnti %x => UE_id %d\n",Mod_id,rnti,UE_id);

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

void rx_sdu(u8 Mod_id,u16 rnti,u8 *sdu) {

  unsigned char rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr,j;
  unsigned char rx_lcids[MAX_NUM_RB];
  unsigned short rx_lengths[MAX_NUM_RB];
  u8 UE_id = find_UE_id(Mod_id,rnti);

#ifdef DEBUG_HEADER_PARSING
  msg("[MAC][eNB RX] Received ulsch sdu from L1 (rnti %x, UE_id %d), parsing header\n",rnti,UE_id);
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
      for (j=0;j<32;j++)
	printf("%x ",payload_ptr[j]);
      printf("\n");
      if (rx_lengths[i]<32) {
	Mac_rlc_xface->mac_rlc_data_ind(Mod_id,
					DCCH+(UE_id)*MAX_NUM_RB,
					payload_ptr,
					DCCH_LCHAN_DESC.transport_block_size,
					rx_lengths[i]/DCCH_LCHAN_DESC.transport_block_size,
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
  if ((ce_ptr-mac_header_control_elements) > 0) {
    memcpy((void*)mac_header_ptr,mac_header_control_elements,ce_ptr-mac_header_control_elements);
    mac_header_ptr+=(unsigned char)(ce_ptr-mac_header_control_elements);
  }
  return((unsigned char*)mac_header_ptr - mac_header);

}
void add_common_dci(DCI_PDU *DCI_pdu,void *pdu,u16 rnti,u8 dci_size_bytes,u8 aggregation,u8 dci_size_bits,u8 dci_fmt) {

  msg("[MAC][eNB] Adding Common DCI for RNTI %x, format %d\n",rnti,dci_fmt);
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

void schedule_RA(u8 Mod_id,u8 subframe) {

  RA_TEMPLATE *RA_template = (RA_TEMPLATE *)&CH_mac_inst[Mod_id].RA_template[0];
  u8 i,j;
  DCI_PDU *DCI_pdu= &CH_mac_inst[Mod_id].DCI_pdu;
  u16 rrc_sdu_length;
  u8 lcid,offset;
  s8 UE_id;

  for (i=0;i<NB_RA_PROC_MAX;i++) {

    if (RA_template[i].generate_rar == 1) {
      msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Generating RAR DCI (proc %d), format 1A\n",mac_xface->frame, subframe,i);
      add_common_dci(DCI_pdu,
		     (void*)&RA_template[i].RA_alloc_pdu1[0],
		     RA_RNTI,
		     RA_template[i].RA_dci_size_bytes1,
		     3,
		     RA_template[i].RA_dci_size_bits1,
		     RA_template[i].RA_dci_fmt1);


  /*
      add_common_dci(DCI_pdu,
		     (void*)&RA_alloc_pdu,
		     RA_RNTI,
		     sizeof(DCI1A_5MHz_TDD_1_6_t),
		     3,
		     sizeof_DCI1A_5MHz_TDD_1_6_t,
		     format1A);
  */
      // Schedule Random-Access Response

      RA_template[i].generate_rar=0;
    }

    if (RA_template[i].generate_rrcconnsetup == 1) {

      // check for RRCConnSetup Message

      if (Is_rrc_registered == 1) {
	rrc_sdu_length = Rrc_xface->mac_rrc_data_req(0,
						     0,1,
						     &CH_mac_inst[Mod_id].CCCH_pdu.payload[0],
						     0);
      }

      if (rrc_sdu_length>0) {
	msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Generating RRCConnectionSetup (RA proc %d, RNTI %x)\n",mac_xface->frame, subframe,i,
	    RA_template[i].rnti);
	// add_user
	UE_id=add_new_ue(Mod_id,RA_template[i].rnti);
	if (UE_id==-1) {
	  msg("[MAC][eNB] Max user count reached\n");
	  exit(-1);
	}
	else {
	  msg("[MAC][eNB] Added user with rnti %x => UE %d\n",RA_template[i].rnti,UE_id);
	}
	msg("[MAC][eNB] Frame %d, subframe %d: Received %d bytes for RRCConnectionSetup: \n",mac_xface->frame,subframe,rrc_sdu_length);
	for (j=0;j<rrc_sdu_length;j++)
	  msg("%x ",(u8)CH_mac_inst[Mod_id].CCCH_pdu.payload[j]);
	msg("\n");
	msg("[MAC][eNB] Frame %d, subframe %d: Generated DLSCH (RRCConnectionSetup) DCI, format 1A, for UE %d\n",mac_xface->frame, subframe,UE_id);

	// Schedule Reflection of Connection request
	/*
      add_common_dci(DCI_pdu,
		     (void*)&DLSCH_alloc_pdu1A,
		     RA_template[i].rnti,
		     sizeof(DCI1A_5MHz_TDD_1_6_t),
		     3,
		     sizeof_DCI1A_5MHz_TDD_1_6_t,
		     format1A);
	*/

	add_ue_spec_dci(DCI_pdu,
		       (void*)&RA_template[i].RA_alloc_pdu2[0],
		       RA_template[i].rnti,
		       RA_template[i].RA_dci_size_bytes2,
		       3,
		       RA_template[i].RA_dci_size_bits2,
		       RA_template[i].RA_dci_fmt2);

	RA_template[i].generate_rrcconnsetup=0;
	lcid=0;
	offset = generate_dlsch_header((unsigned char*)CH_mac_inst[Mod_id].DLSCH_pdu[(u8)UE_id][0].payload[0],
				       1,              //num_sdus
				       &rrc_sdu_length,      //
				       &lcid,          // sdu_lcid
				       255,                                   // no drx
				       255,                                   // no timing advance
				       RA_template[i].cont_res_id,        // contention res id
				       0);                                    // no padding
	msg("offset %d\n",(u8)offset);
	memcpy((void*)&CH_mac_inst[Mod_id].DLSCH_pdu[(u8)UE_id][0].payload[0][(u8)offset],
	       &CH_mac_inst[Mod_id].CCCH_pdu.payload[0],
	       rrc_sdu_length);
      }
    }
  }
}

void schedule_ulsch(u8 Mod_id,u8 subframe) {

  u8 UE_id;
  u16 rnti;
  u8 round;
  u8 harq_pid;
  DCI0_5MHz_TDD_1_6_t *ULSCH_dci;
  DCI_PDU *DCI_pdu= &CH_mac_inst[Mod_id].DCI_pdu;

  for (UE_id=0;UE_id<NB_CNX_CH;UE_id++) {
    rnti = CH_mac_inst[Mod_id].UE_template[UE_id].rnti;
    if (rnti>0) {
      // This is an allocated UE_id
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Checking ULSCH UE_id %d (rnti %x,mode %s), format 0\n",mac_xface->frame,subframe,UE_id,rnti,mode_string[mac_xface->eNB_UE_stats[Mod_id][UE_id].mode]);
#endif
      if (mac_xface->eNB_UE_stats[Mod_id][UE_id].mode == PUSCH) {

	// Get candidate harq_pid from PHY
	mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,1);

	// Note this code is for a specific DCI format
	ULSCH_dci = (DCI0_5MHz_TDD_1_6_t *)&CH_mac_inst[Mod_id].UE_template[UE_id].ULSCH_DCI[0];
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

	ULSCH_dci->rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,
						   6 + (UE_id*openair_daq_vars.ue_ul_nb_rb),
						   openair_daq_vars.ue_ul_nb_rb);
	add_ue_spec_dci(DCI_pdu,
			ULSCH_dci,
			rnti,
			sizeof(DCI0_5MHz_TDD_1_6_t),
			3,
			sizeof_DCI0_5MHz_TDD_1_6_t,
			format0);


#ifdef DEBUG_eNB_SCHEDULER
	msg("[MAC][eNB Scheduler] Frame %d, subframe %d: Generated ULSCH DCI for UE_id %d, format 0\n",mac_xface->frame,subframe,UE_id);
#endif

      }

    }
  }
}

void schedule_dlsch(u8 Mod_id,u8 subframe) {

  u8 UE_id;
  mac_rlc_status_resp_t rlc_status;
  u8 header_len;
  u8 lcid,sdu_lcids[11],offset,num_sdus=0;
  u16 nb_rb,TBS,payload_size,i,j,sdu_lengths[11],rrc_sdu_length,rnti;
  u8 dlsch_buffer[MAX_DLSCH_PAYLOAD_BYTES];
  u8 round;
  u8 harq_pid;
  DCI2_5MHz_2A_M10PRB_TDD_t *DLSCH_dci;
  DCI_PDU *DCI_pdu= &CH_mac_inst[Mod_id].DCI_pdu;
  u16 sdu_length_total=0;


  for (UE_id=0;UE_id<NB_CNX_CH;UE_id++) {
    sdu_length_total=0;
    num_sdus=0;
    rnti = CH_mac_inst[Mod_id].UE_template[UE_id].rnti;
    if (rnti>0) {
      // This is an allocated UE_id

      // Get candidate harq_pid from PHY
      mac_xface->get_ue_active_harq_pid(Mod_id,rnti,subframe,&harq_pid,&round,0);

      // Note this code is for a specific DCI format
      DLSCH_dci = (DCI2_5MHz_2A_M10PRB_TDD_t *)&CH_mac_inst[Mod_id].UE_template[UE_id].DLSCH_DCI[0];
      if (round > 0) {

	DLSCH_dci->ndi1 = 0;
      }
      else {

	DLSCH_dci->ndi1 = 1;

	if (openair_daq_vars.dlsch_rate_adaptation == 0)
	  DLSCH_dci->mcs1   = openair_daq_vars.target_ue_dl_mcs;
	else {

	  // this is the CQI-based rate adaptation
	  // if we got an increase in mcs from UE, make sure we don't increment too much
	  if ((mac_xface->eNB_UE_stats[Mod_id][UE_id].DL_cqi[0]<<1)+mac_xface->eNB_UE_stats[Mod_id][UE_id].dlsch_mcs_offset > (DLSCH_dci->mcs1+1))
	    mac_xface->eNB_UE_stats[Mod_id][UE_id].dlsch_mcs_offset=0;
	  DLSCH_dci->mcs1   = (mac_xface->eNB_UE_stats[Mod_id][UE_id].DL_cqi[0]<<1)+mac_xface->eNB_UE_stats[Mod_id][UE_id].dlsch_mcs_offset;
	  if (DLSCH_dci->mcs1 == 28)
	    DLSCH_dci->mcs1=0;
	}
	//      msg("mcs: %d, mcs_offset %d\n",DLSCH_dci->mcs1,dlsch_mcs_offset);
      }
#ifdef USER_MODE
      DLSCH_dci->rballoc = DLSCH_RB_ALLOC;
      nb_rb=23;
#else
      if (DLSCH_dci->mcs1 > 14)
	DLSCH_dci->mcs1 = 14;

      if (DLSCH_dci->mcs1 > 10) {
	DLSCH_dci->rballoc = DLSCH_RB_ALLOC_12; // navid should be mcs1 ?
	nb_rb=12;
      }
      else {
	DLSCH_dci->rballoc = DLSCH_RB_ALLOC; // navid : what about here?
	nb_rb=23;
      }
#endif




      if (openair_daq_vars.dlsch_transmission_mode == 6)
	DLSCH_dci->tpmi   = 5;
      else
	DLSCH_dci->tpmi   = 0;

      mac_xface->eNB_UE_stats[Mod_id][UE_id].dlsch_trials[round]++;

      DLSCH_dci->rv1 = round&3;
#ifdef DEBUG_eNB_SCHEDULER
      msg("[MAC] eNB %d Scheduler, generating DLSCH for rnti %x (rballoc %x,mcs %d, ndi %d, rv %d)\n",
	  Mod_id,rnti,DLSCH_dci->rballoc,DLSCH_dci->mcs1,DLSCH_dci->ndi1,DLSCH_dci->rv1);
#endif

      add_ue_spec_dci(DCI_pdu,
		      DLSCH_dci,
		      rnti,
		      sizeof(DCI2_5MHz_2A_M10PRB_TDD_t),
		      3,
		      sizeof_DCI2_5MHz_2A_M10PRB_TDD_t,
		      format2_2A_M10PRB);

      /*
      memcpy(&DCI_pdu->dci_alloc[0].dci_pdu[0],&DLSCH_dci,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
      DCI_pdu->dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
      DCI_pdu->dci_alloc[0].L          = 3;
      DCI_pdu->dci_alloc[0].rnti       = rnti;
      DCI_pdu->dci_alloc[0].format     = format2_2A_M10PRB;
      DCI_pdu->Num_ue_spec_dci = 1;
      */
      // copy MAC header and SDU

      TBS = mac_xface->get_TBS(DLSCH_dci->mcs1,nb_rb);

      // check first for RLC data on DCCH
      header_len = 2+1+1; // 2 bytes DCCH SDU subheader + timing advance subheader + timing advance command
      rlc_status = mac_rlc_status_ind(Mod_id,DCCH+(MAX_NUM_RB*UE_id),
				      (TBS-header_len)/DCCH_LCHAN_DESC.transport_block_size,
				      DCCH_LCHAN_DESC.transport_block_size);

      sdu_lengths[0]=0;
      while (rlc_status.bytes_in_buffer>0) {
	msg("[MAC][eNB] DCCH has %d bytes to send (buffer %d, header %d)\n",rlc_status.bytes_in_buffer,sdu_lengths[0],header_len);

	sdu_lengths[0] += Mac_rlc_xface->mac_rlc_data_req(Mod_id,
							  DCCH+(MAX_NUM_RB*UE_id),
							  &dlsch_buffer[sdu_lengths[0]]);
	// navid : transport_block_size is 4 bytes
	rlc_status = mac_rlc_status_ind(Mod_id,DCCH+(MAX_NUM_RB*UE_id),
					(TBS-header_len)/DCCH_LCHAN_DESC.transport_block_size,
					DCCH_LCHAN_DESC.transport_block_size);
      }
      if (sdu_lengths[0]>0) {
        sdu_length_total += sdu_lengths[0];
	sdu_lcids[0] = DCCH;
	msg("[MAC][eNB] Got %d bytes for DCCH :",sdu_lengths[0]);

	num_sdus = 1;

	for (j=0;j<sdu_lengths[0];j++)
	  msg("%x ",dlsch_buffer[j]);
	msg("\n");
      }

      // check for DTCH (later) and update header information
      // check first for RLC data on DCCH
      header_len = 3; // 2 bytes DTCH SDU subheader

      rlc_status = mac_rlc_status_ind(Mod_id,DTCH+(MAX_NUM_RB*UE_id),
				      0,
				      TBS-header_len-sdu_length_total-header_len);
      if (rlc_status.bytes_in_buffer > 0) {

	sdu_lengths[num_sdus] = Mac_rlc_xface->mac_rlc_data_req(Mod_id,
								DTCH+(MAX_NUM_RB*UE_id),
								&dlsch_buffer[sdu_length_total]);
	msg("[MAC] eNB %d, Got %d bytes for DTCH\n",Mod_id,sdu_lengths[num_sdus]);
	sdu_lcids[num_sdus] = DTCH;
	sdu_length_total += sdu_lengths[num_sdus];
	num_sdus++;
      }
      
      offset = generate_dlsch_header((unsigned char*)CH_mac_inst[Mod_id].DLSCH_pdu[(u8)UE_id][0].payload[0],
      // offset = generate_dlsch_header((unsigned char*)CH_mac_inst[0].DLSCH_pdu[0][0].payload[0],
				     num_sdus,              //num_sdus
				     sdu_lengths,  //
				     sdu_lcids,
				     255,                                   // no drx
				     mac_xface->eNB_UE_stats[Mod_id][UE_id].UE_timing_offset/4, // timing advance
				     NULL,                                  // contention res id
				     0);                                    // no padding

      msg("[MAC][eNB] Generate header : num_sdus %d, sdu_lengths[0] %d, sdu_lcids[0] %d => payload offset %d\n",
	  num_sdus,sdu_lengths[0],sdu_lcids[0],offset);

      // cycle through SDUs and place in dlsch_buffer
      memcpy(&CH_mac_inst[Mod_id].DLSCH_pdu[(u8)UE_id][0].payload[0][offset],dlsch_buffer,sdu_length_total);
      // memcpy(&CH_mac_inst[0].DLSCH_pdu[0][0].payload[0][offset],dcch_buffer,sdu_lengths[0]);


      msg("[MAC]eNB %d Frame %d, subframe %d: Generated DLSCH header (mcs %d, TBS %d, nb_rb %d)\n",
	  Mod_id,mac_xface->frame,subframe,DLSCH_dci->mcs1,TBS,nb_rb);


      // fill remainder of DLSCH with random data
      for (j=0;j<(TBS-sdu_length_total-offset);j++)
	CH_mac_inst[Mod_id].DLSCH_pdu[(u8)UE_id][0].payload[0][offset+sdu_length_total+j] = (char)(taus()&0xff);
      //CH_mac_inst[0].DLSCH_pdu[0][0].payload[0][offset+sdu_lengths[0]+j] = (char)(taus()&0xff);
    }
  }
}

void eNB_dlsch_ulsch_scheduler(u8 Mod_id,u8 subframe) {


  DCI_PDU *DCI_pdu= &CH_mac_inst[Mod_id].DCI_pdu;

  DCI_pdu->Num_common_dci  = 0;
  DCI_pdu->Num_ue_spec_dci = 0;

  //  printf("[MAC] eNB inst %d scheduler subframe %d\n",Mod_id, subframe);

  switch (subframe) {
  case 0:
    // Schedule DL control
    memcpy(&DCI_pdu->dci_alloc[0].dci_pdu[0],
	   &BCCH_alloc_pdu,
	   sizeof(DCI1A_5MHz_TDD_1_6_t));

    DCI_pdu->dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
    DCI_pdu->dci_alloc[0].L          = 3;
    DCI_pdu->dci_alloc[0].rnti       = SI_RNTI;
    DCI_pdu->dci_alloc[0].format     = format1A;

    DCI_pdu->Num_common_dci  = 1;

#ifdef DEBUG_eNB_SCHEDULER
    msg("[MAC] Frame %d, subframe %d: Generated CCCH DCI, format 1A\n",mac_xface->frame, subframe);
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

    break;

  case 7:

    schedule_dlsch(Mod_id,subframe);
    break;



  case 8:

    schedule_RA(Mod_id,subframe);
    // Schedule UL subframe
    schedule_ulsch(Mod_id,subframe);
    break;

  case 9:

    // Schedule UL subframe
    schedule_ulsch(Mod_id,subframe);




    break;

  }




}
