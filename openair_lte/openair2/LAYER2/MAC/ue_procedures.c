/*________________________ue_control_plane_procedures.c________________________

  Authors : Hicham Anouar, Raymond Knopp
  Company : EURECOM
  Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
  ________________________________________________________________*/

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
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif

//#define DEBUG_UE_MAC_CTRL
//#define DEBUG_UE_MAC_RLC
//#define DEBUG_MAC_REPORT
//#define DEBUG_MAC_SCHEDULING

//#define DEBUG_RACH_MAC
//#define DEBUG_RACH_RRC
#define DEBUG_SI_RRC
//#define DEBUG_HEADER_PARSING

/*
#ifndef USER_MODE
#define msg debug_msg
#endif
*/

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




u32 ue_get_SR(u8 Mod_id,u8 eNB_id,u16 rnti) {

  return(0);
}

void ue_send_sdu(u8 Mod_id,u8 *sdu,u8 CH_index) {

  unsigned char rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr;
  unsigned char rx_lcids[MAX_NUM_RB];
  unsigned short rx_lengths[MAX_NUM_RB];

  payload_ptr = parse_header(sdu,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths);

#ifdef DEBUG_HEADER_PARSING
  msg("[MAC][UE %d] ue_send_sdu : CH_index %d : num_ce %d num_sdu %d\n",Mod_id,CH_index,num_ce,num_sdu);
#endif

  for (i=0;i<num_ce;i++) {

      switch (rx_ces[i]) {
      case UE_CONT_RES:
#ifdef DEBUG_HEADER_PARSING
	msg("[MAC][UE %d] CE %d : UE contention resolution for RRC :",Mod_id,i);
	msg("%x,%x,%x,%x,%x,%x\n",payload_ptr[0],payload_ptr[1],payload_ptr[2],payload_ptr[3],payload_ptr[4],payload_ptr[5]);
	// Send to RRC here
#endif
	payload_ptr+=6;
	break;
      case TIMING_ADV_CMD: 
#ifdef DEBUG_HEADER_PARSING
	msg("CE %d : UE Timing Advance :",i);
	msg("%d\n",payload_ptr[0]);
#endif
	process_timing_advance(payload_ptr[0]);
	payload_ptr++;
	break;
      case DRX_CMD:
#ifdef DEBUG_HEADER_PARSING
	msg("CE %d : UE DRX :",i);
#endif
	payload_ptr++;
	break;
      case SHORT_PADDING:
#ifdef DEBUG_HEADER_PARSING
	msg("CE %d : UE 1 byte Padding :",i);
#endif
	payload_ptr++;
	break;
      }
  }
  for (i=0;i<num_sdu;i++) {
#ifdef DEBUG_HEADER_PARSING
    msg("SDU %d : LCID %d, length %d\n",i,rx_lcids[i],rx_lengths[i]);
#endif
    if (rx_lcids[i] == CCCH) {

      msg("[MAC][UE %d] RX CCCH -> RRC (%d bytes)\n",Mod_id,rx_lengths[i]);
      Rrc_xface->mac_rrc_data_ind(Mod_id+NB_CH_INST,
				  CCCH,
				  (char *)payload_ptr,rx_lengths[i],CH_index);
      
    }
    else if (rx_lcids[i] == DCCH) {
      debug_msg("[MAC][UE %d] RX  DCCH \n",Mod_id);
      Mac_rlc_xface->mac_rlc_data_ind(Mod_id+NB_CH_INST,
				      DCCH,
				      (char *)payload_ptr,
				      DCCH_LCHAN_DESC.transport_block_size,
				      rx_lengths[i]/DCCH_LCHAN_DESC.transport_block_size,
				      NULL);
    }else if (rx_lcids[i] == DTCH) {
      debug_msg("[MAC][UE %d] RX %d DTCH -> RLC \n",Mod_id,rx_lengths[i]);
      Mac_rlc_xface->mac_rlc_data_ind(Mod_id+NB_CH_INST,
				      DTCH,
				      (char *)payload_ptr,
				      rx_lengths[i],
				      1,
				      NULL);
    }
    /* else if (rx_lcids[i] == DTCH) {
      Mac_rlc_xface->mac_rlc_data_ind(Mod_id+NB_CH_INST,
				      DTCH,
				      (u8 *)payload_ptr,
				      rx_lengths[i],
				      1,
				      NULL);
				      }*/
    payload_ptr+=rx_lengths[i];
  }
  /*


	*/
}

void ue_decode_si(u8 Mod_id, u8 CH_index, void *pdu,u16 len) {

#ifdef DEBUG_SI_RRC
  msg("[MAC][UE %d] Frame %d Sending SI to RRC (Lchan Id %d)\n",Mod_id,mac_xface->frame,BCCH);
#endif

  Rrc_xface->mac_rrc_data_ind(Mod_id+NB_CH_INST,BCCH,(char *)pdu,len,0);//CH_index);

}

unsigned char *ue_get_rach(u8 Mod_id,u8 CH_index){


  u8 Size=0;

  if (Is_rrc_registered == 1) {
    Size = Rrc_xface->mac_rrc_data_req(Mod_id+NB_CH_INST,
				       CCCH,1,
				       (char*)&UE_mac_inst[Mod_id].CCCH_pdu.payload[0],
				       CH_index);
    msg("[MAC][UE %d] Frame %d: Requested RRCConnectionRequest, got %d bytes\n",Mod_id,mac_xface->frame,Size);
    if (Size>0)
      return((char*)&UE_mac_inst[Mod_id].CCCH_pdu.payload[0]);
  }
  return(NULL);
 
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
    //    msg("[MAC][UE %d] Scheduler Truncated BSR Header\n",Mod_id);
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
    //    msg("[MAC][UE %d] Scheduler SHORT BSR Header\n",Mod_id);
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
    //    msg("[MAC][UE %d] Scheduler Long BSR Header\n",Mod_id);
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
      //printf("short sdu\n");
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

  mac_header_ptr+=last_size;
  memcpy((void*)mac_header_ptr,mac_header_control_elements,ce_ptr-mac_header_control_elements);
  mac_header_ptr+=(unsigned char)(ce_ptr-mac_header_control_elements);

  return((unsigned char*)mac_header_ptr - mac_header);

}
// generate BSR
void ue_get_sdu(u8 Mod_id,u8 CH_index,u8 *ulsch_buffer,u16 buflen) {

  mac_rlc_status_resp_t rlc_status;
  u8 dcch_header_len,dtch_header_len;
  u16 sdu_size_dcch,sdu_lengths[8],i;
  u8 sdu_lcids[8],payload_offset=0,num_sdus=0;
  u8 DCCH_not_empty;
  
  u8 ulsch_buff[MAX_ULSCH_PAYLOAD_BYTES];
  u16 sdu_length_total=0;

  BSR_SHORT bsr;

  // Compute header length
  // check for UL bandwidth requests and add SR control element
  dcch_header_len = 2;
  
    
  // Check for DCCH first
  DCCH_not_empty=1;
  sdu_lengths[0]=0;
  while (DCCH_not_empty==1) {
    rlc_status = mac_rlc_status_ind(Mod_id+NB_CH_INST,
				    DCCH,
				    (buflen-dcch_header_len-sdu_length_total)/DCCH_LCHAN_DESC.transport_block_size,
				    DCCH_LCHAN_DESC.transport_block_size);
    debug_msg("[MAC][UE %d] RLC status for DCCH : %d\n",
	Mod_id,rlc_status.bytes_in_buffer);

    if (rlc_status.bytes_in_buffer>0) {
      debug_msg("[MAC][UE %d] DCCH has %d bytes to send (buffer %d, header %d, sdu_length_total %d)\n",Mod_id,rlc_status.bytes_in_buffer,buflen,dcch_header_len,sdu_length_total);
      
      sdu_lengths[0] += Mac_rlc_xface->mac_rlc_data_req(Mod_id+NB_CH_INST,
							DCCH,
							&ulsch_buff[sdu_lengths[0]]);
      sdu_length_total += sdu_lengths[0];
      sdu_lcids[0] = DCCH;
      debug_msg("[MAC][UE %d] TX Got %d bytes for DCCH\n",Mod_id,sdu_lengths[0]);
      num_sdus = 1;
      //header_len +=2; 
      DCCH_not_empty=0;
    }
    else {
      DCCH_not_empty=0;
      dcch_header_len=0;
    }
  }

  dtch_header_len = 3;
  if ((sdu_length_total +dcch_header_len + dtch_header_len )< buflen) {
// now check for other logical channels
// check for ulsch   
// rlc UM v 9
    rlc_status = mac_rlc_status_ind(Mod_id+NB_CH_INST,
				    DTCH,
				    0,
				    buflen - dcch_header_len - dtch_header_len - sdu_length_total);
    
    if (rlc_status.bytes_in_buffer > 0 ) { // get rlc pdu 
      debug_msg("[MAC][UE %d] DTCH has %d bytes to send (buffer %d, header %d)\n",
	  Mod_id,rlc_status.bytes_in_buffer,buflen,dtch_header_len);
      
      
      if ( rlc_status.bytes_in_buffer < 128) { // SCH_SUBHEADER_LONG case 
	dtch_header_len=2;
	rlc_status = mac_rlc_status_ind(Mod_id+NB_CH_INST,
					DTCH,
					0,
					buflen - dcch_header_len - dtch_header_len - sdu_length_total); // number of bytes
	
      }
      sdu_lengths[num_sdus] = Mac_rlc_xface->mac_rlc_data_req(Mod_id+NB_CH_INST,
							      DTCH,
							      &ulsch_buff[sdu_length_total]);
      
      debug_msg("[MAC][UE %d] TX Got %d bytes for DTCH\n",Mod_id,sdu_lengths[num_sdus]);
      
      sdu_lcids[num_sdus] = DTCH;
      sdu_length_total += sdu_lengths[num_sdus];
      num_sdus++;
     }
    else { // no rlc pdu : generate the dummy header
      

    }
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
					   NULL, // short bsr
					   NULL); // long_bsr

    debug_msg("[MAC][UE %d] Payload offset %d sdu total length %d\n",
	Mod_id,payload_offset, sdu_length_total);

    // cycle through SDUs and place in ulsch_buffer
    memcpy(&ulsch_buffer[payload_offset],ulsch_buff,sdu_length_total);
  }
  else { // send BSR 
    bsr.LCGID = 0x0;
    bsr.Buffer_size = 0x3f;
    
    payload_offset = generate_ulsch_header(ulsch_buffer,  // mac header
					   num_sdus,      // num sdus
					   0,            // short pading
					   sdu_lengths,  // sdu length
					   sdu_lcids,    // sdu lcid 
					   NULL,  // power headroom
					   NULL,  // crnti
					   NULL,  // truncated bsr
					   &bsr, // short bsr
					   NULL); // long_bsr
    
    debug_msg("[MAC][UE %d] Payload offset %d sdu total length %d\n",
	Mod_id,payload_offset, sdu_length_total);
    
    // cycle through SDUs and place in ulsch_buffer
    memcpy(&ulsch_buffer[payload_offset],ulsch_buff,sdu_length_total);


    }
}

 
void ue_scheduler(u8 Mod_id, u8 subframe) {

  Mac_rlc_xface->frame=mac_xface->frame;
  Rrc_xface->Frame_index=Mac_rlc_xface->frame;
  Mac_rlc_xface->pdcp_run();
  
}


