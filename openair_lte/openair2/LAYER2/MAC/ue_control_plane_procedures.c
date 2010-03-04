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
#include "PHY/impl_defs.h"
#endif
#include "PHY_INTERFACE/defs.h"
#include "PHY_INTERFACE/extern.h"
#include "COMMON/mac_rrc_primitives.h"
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif

//#define DEBUG_UE_MAC_CTRL
//#define DEBUG_UE_MAC_RLC
//#define DEBUG_MAC_REPORT
//#define DEBUG_MAC_SCHEDULING
//#define DEBUG_RACH_MAC

char Dummy_buffer[NB_TB_BUFF_MAX * 100];

/****************************************************************************************************************/
void ue_get_chbch(u8 Mod_id, u8 CH_index){
  /****************************************************************************************************************/
  MACPHY_DATA_REQ *Macphy_data_req;


  if ((Macphy_data_req = new_macphy_data_req(Mod_id+NB_CH_INST))==NULL) {
    msg("[MAC][UE] TTI %d: ue_get_chbch() new_macphy_data_req fails\n",Mac_rlc_xface->frame);
    mac_xface->macphy_exit("[ue_get_chbch] new_macphy_data_req fails\n");
    return;
  }
  
#ifdef DEBUG_UE_MAC_CTRL
  msg("[UE %d, TTI %d]Request CHBCH From CH_index %d, Freq_alloc %x\n",NODE_ID[Mod_id+NB_CH_INST],Mac_rlc_xface->frame,CH_index,
      UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Lchan_info.Phy_resources_rx.Freq_alloc);
#endif //DEBUG_UE_MAC_CTRL

  Macphy_data_req->Pdu_type = CHBCH;  
  Macphy_data_req->Direction = RX;  
  Macphy_data_req->Lchan_id.Index = UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Lchan_info.Lchan_id.Index;
  Macphy_data_req->Phy_resources = &UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Lchan_info.Phy_resources_rx;
  Macphy_data_req->Dir.Req_rx.Pdu.Chbch_pdu = &UE_mac_inst[Mod_id].RX_chbch_pdu[CH_index];
  Macphy_data_req->Dir.Req_rx.Meas.DL_meas = &UE_mac_inst[Mod_id].DL_meas[CH_index]; 
  Macphy_data_req->num_tb = 1;
  Macphy_data_req->CH_index = CH_index;
  Macphy_data_req->tb_size_bytes = sizeof(CHBCH_PDU);


  //update queues' state for all active LC
  mac_rlc_status_resp_t rlc_status;
  unsigned short j,G_size,Tb_size;
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  
  if(UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Active == 1){

    Lchan_entry=&UE_mac_inst[Mod_id].Dcch_lchan[CH_index];
    if(NB_TB_BUFF_MAX > Lchan_entry->Lchan_info.W_idx){
      Tb_size=Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size;
      
      rlc_status=mac_rlc_status_ind(Mod_id+NB_CH_INST,Lchan_entry->Lchan_info.Lchan_id.Index,
				    Tb_size,
				    NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx);
      
      G_size=Mac_rlc_xface->mac_rlc_data_req(Mod_id+NB_CH_INST,
					     Lchan_entry->Lchan_info.Lchan_id.Index,
					     &Lchan_entry->Lchan_info.Current_payload_tx[Lchan_entry->Lchan_info.W_idx*Tb_size]);
      
      Lchan_entry->Lchan_info.W_idx+=G_size/Tb_size;

      if(Lchan_entry->Lchan_info.W_idx > NB_TB_BUFF_MAX){
	msg("[MAC][UE] TTI %d: ue_control_plane_procedures.c (ue_get_chbch) RLC RETURNS TOO Many TBs !!!!\n",
	    Mac_rlc_xface->frame);
	mac_xface->macphy_exit("");
	return;
      }
#ifdef DEBUG_UE_MAC_RLC
      msg("[MAC][UE]TTi %d: DATA_REQ on Lchan %d for %d Tbs returns %d TBs, Current MAC BUFFER has %d Tb\n",
	  Mac_rlc_xface->frame,
	  Lchan_entry->Lchan_info.Lchan_id.Index,
	  NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx,
	  G_size/Tb_size,
	  Lchan_entry->Lchan_info.W_idx);
#endif //DEBUG_UE_MAC_RLC
    }
  }
  
  for(j=1;j<NB_RAB_MAX;j++)
    if(UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Active == 1){
      Lchan_entry=&UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index];
      if(NB_TB_BUFF_MAX > Lchan_entry->Lchan_info.W_idx){
	Tb_size=Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size;

#ifdef DEBUG_UE_MAC_RLC
	msg("\n___________________///////////////////MAC UE--------------______________\n ");
	msg("[MAC][UE]TTi %d: STATUS IND on Lchan %d for %d Tbs, Current MAC BUFFER has %d Tb\n",
	    Mac_rlc_xface->frame,
	    Lchan_entry->Lchan_info.Lchan_id.Index,
	    NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx,
	    Lchan_entry->Lchan_info.W_idx);
#endif
	rlc_status=mac_rlc_status_ind(Mod_id+NB_CH_INST,Lchan_entry->Lchan_info.Lchan_id.Index,
				      Tb_size,
				      NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx);
	
	G_size=Mac_rlc_xface->mac_rlc_data_req(Mod_id+NB_CH_INST,
					       Lchan_entry->Lchan_info.Lchan_id.Index,
					       &Lchan_entry->Lchan_info.Current_payload_tx[Lchan_entry->Lchan_info.W_idx*Tb_size]);
	
	Lchan_entry->Lchan_info.W_idx+=G_size/Tb_size;
	
#ifdef DEBUG_UE_MAC_RLC
	msg("[MAC][UE]TTi %d: DATA_REQ on Lchan %d for %d Tbs returns %d TBs, Current MAC BUFFER has %d Tb\n",
	    Mac_rlc_xface->frame,
	    Lchan_entry->Lchan_info.Lchan_id.Index,
	    NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx,
	    G_size/Tb_size,
	    Lchan_entry->Lchan_info.W_idx);
	
	int kk;
	if(G_size>0)
	  for(kk=0;kk<Lchan_entry->Lchan_info.W_idx;kk++)	
	    msg("[MAC][UE]TTI:d , Buffer has %d Tbs, SN[%d]=%d",
		Mac_rlc_xface->frame,
		Lchan_entry->Lchan_info.W_idx,	
		(unsigned int)((Lchan_entry->Lchan_info.Current_payload_tx[kk*Tb_size]>>1)&0x7F));
#endif
	
	if(Lchan_entry->Lchan_info.W_idx > NB_TB_BUFF_MAX){
	  msg("[MAC][UE] RLC RETURNS TOO MUCH TBs !!!!\n");
	  mac_xface->macphy_exit("");
	  return;
	}
      }
    }
  
  
  UE_mac_inst[Mod_id].NB_decoded_chbch=0;
  
}
/********************************************************************************************************************/
void ue_process_DL_meas(unsigned char Mod_id, unsigned short CH_index){
  /********************************************************************************************************************/
  unsigned char i;
  DL_MEAS *DL_meas = &UE_mac_inst[Mod_id].DL_meas[CH_index];
  for(i=0;i<NUMBER_OF_MEASUREMENT_SUBBANDS;i++){
    UE_mac_inst[Mod_id].Def_meas[CH_index].Sinr_meas[0][i]= DL_meas->Sub_band_sinr[i];
    /*
      ((UE_mac_inst[Mod_id].Def_meas[CH_index].Sinr_meas[0][i]*UE_mac_inst[Mod_id].Def_meas[CH_index].Forg_fact)
      + (*(10-UE_mac_inst[Mod_id].Def_meas[CH_index].Forg_fact)))/10;         
      //      msg("Sinr_meas[%d]=%d\n",i,UE_mac_inst[Mod_id].Def_meas[Idx2].Sinr_meas[0][i]);
      */
  }
  UE_mac_inst[Mod_id].Def_meas[CH_index].Wideband_sinr=DL_meas->Wideband_sinr_dB;
  
#ifdef DEBUG_MAC_MEAS  
   if ((Mac_rlc_xface->frame % 100) == 0) {
    
    msg("[MAC][UE] Frame %d: DL_MEAS (%d,%d) -> %d dB\n",
	Mac_rlc_xface->frame,
	DL_meas->Wideband_rssi_dBm,
	DL_meas->Wideband_interference_level_dBm, 
	UE_mac_inst[Mod_id].Def_meas[CH_index].Wideband_sinr);
    }
#endif //DEBUG_MAC_MEAS  

}

/********************************************************************************************************************/
unsigned int get_chsch_subband_quality(unsigned char Mod_id, unsigned short CH_index) {
  /********************************************************************************************************************/
  int i;
  unsigned int cqi=0;
  char diff;
  
  for (i=0;i<NUMBER_OF_MEASUREMENT_SUBBANDS;i++){
    diff = (char)UE_mac_inst[Mod_id].Def_meas[CH_index].Sinr_meas[0][i] - (char)UE_mac_inst[Mod_id].Def_meas[CH_index].Wideband_sinr;

#ifdef DEBUG_MAC_MEAS  
        msg("%d (%d,%d)",diff,UE_mac_inst[Mod_id].Def_meas[CH_index].Sinr_meas[0][i],UE_mac_inst[Mod_id].Def_meas[CH_index].Wideband_sinr); 
#endif //DEBUG_MAC_MEAS  
    if (diff>SINR_THRES0) {
      if (diff<SINR_THRES1)
	cqi |= (1<<(2*i));
      else if (diff<SINR_THRES2)
	cqi |= (2<<(2*i));
      else
	cqi |= (3<<(2*i));
    }
  }
  return(cqi);
}

/****************************************************************************************************************/
void ue_decode_chbch(u8 Mod_id, CHBCH_PDU *RX_chbch_pdu, DL_MEAS *DL_meas,u16 Index,int crc_status){ 
  /****************************************************************************************************************/
  unsigned char i,idx=(Mac_rlc_xface->frame+1)%2,Activ_tti;
  MACPHY_DATA_REQ *Macphy_data_req;
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;  
  unsigned short Idx1,Idx2,In_idx,Direction;
  unsigned char  CH_index = (Index & RAB_OFFSET2)  >> RAB_SHIFT2;
  mac_rlc_status_resp_t rlc_status;
  unsigned int bitmap,bitmap_cnt;
  unsigned char Tb_size,G_size;
  unsigned int diff;

  if (crc_status < 0) {
    UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Lchan_info.NB_RX++;
    UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.NB_RX++;
    UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Lchan_info.NB_RX_ERRORS++;
    UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.NB_RX_ERRORS++;
    return;
  }
  else {
    UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Lchan_info.NB_RX++;
    UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.NB_RX++;
  }


  if((Mac_rlc_xface->frame%128)==0) {
    
    if (UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Active==1) {
      
      diff = UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.NB_TX - UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.NB_TX_LAST;
      UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.NB_TX_LAST = UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.NB_TX;
      
      UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.output_rate = 
	(8*diff*UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.Lchan_desc[1].transport_block_size)>>7;

    }

    for(i=0;i<NB_RAB_MAX;i++){

      if (UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Active==1) {
	
	diff = UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.NB_TX
	  -  UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.NB_TX_LAST;
	
	UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.NB_TX_LAST = UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.NB_TX;
	
	UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.output_rate = 
	  (8*diff*UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.Lchan_desc[1].transport_block_size)>>7;
	
      }
    }
  }
  
  if (Is_rrc_registered == 1) {
    
    if(RX_chbch_pdu->Num_bytes_bcch >BCCH_PAYLOAD_SIZE_MAX) {
      msg("bcch too big, %d Bytes from CH %d\n",RX_chbch_pdu->Num_bytes_bcch,CH_index);
      
      return;
    }
    
    if(UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Active==1){
      Rrc_xface->mac_rrc_data_ind(Mod_id+NB_CH_INST,UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Lchan_info.Lchan_id.Index,&RX_chbch_pdu->Bcch_payload[0],CH_index);
    }
    
    if(RX_chbch_pdu->Num_bytes_ccch >CCCH_PAYLOAD_SIZE_MAX) {
      msg("ccch too big, %d bytes from CH %d\n",RX_chbch_pdu->Num_bytes_ccch,CH_index);
      return;
    }
    
    if(UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Active==1)
    Rrc_xface->mac_rrc_data_ind(Mod_id+NB_CH_INST,UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Lchan_id.Index,
				&RX_chbch_pdu->Ccch_payload[0],CH_index);

  }

  UE_mac_inst[Mod_id].CH_ul_freq_map[CH_index]=0;
  // Downlink SACH Information
  for(i=0;i<RX_chbch_pdu->Num_dl_sach;i++){
    UE_mac_inst[Mod_id].CH_ul_freq_map[CH_index]+=RX_chbch_pdu->DL_sacch_pdu[i].Freq_alloc;
    // check for UE index
    Idx2=((RX_chbch_pdu->DL_sacch_pdu[i].Lchan_id.Index & RAB_OFFSET2) >> RAB_SHIFT2); 
    In_idx=(RX_chbch_pdu->DL_sacch_pdu[i].Lchan_id.Index & RAB_OFFSET);
    
    if( (Idx2 == Rrc_xface->UE_index[Mod_id+NB_CH_INST][CH_index]) || (In_idx==DTCH_BD) ){//Something I should detect

#ifdef DEBUG_MAC_CHBCH
      msg("[OPENAIR][MAC][UE] Decoding DL_MAP %d from CH_INDEX %d for Lchan_ID %d , IN_idx %d\n",i+1
	  ,CH_index,RX_chbch_pdu->DL_sacch_pdu[i].Lchan_id.Index,In_idx);
#endif //DEBUG_MAC_CHBCH

      if(In_idx ==DCCH)
	Lchan_entry = &UE_mac_inst[Mod_id].Dcch_lchan[CH_index];
      else
	Lchan_entry=&UE_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][CH_index];
      if( Lchan_entry->Active == 1 ) {
	
	if((Macphy_data_req=new_macphy_data_req(Mod_id+NB_CH_INST))==NULL) return;

	Lchan_entry->Lchan_info.Phy_resources_rx.Freq_alloc = RX_chbch_pdu->DL_sacch_pdu[i].Freq_alloc;
	Lchan_entry->Lchan_info.Phy_resources_rx.Coding_fmt = RX_chbch_pdu->DL_sacch_pdu[i].Coding_fmt;
	Lchan_entry->Lchan_info.Nb_sched_tb_dl              = RX_chbch_pdu->DL_sacch_pdu[i].Nb_tb;
	Lchan_entry->Lchan_info.Nb_sched_tb_dl_temp              = Lchan_entry->Lchan_info.Nb_sched_tb_dl;
	Lchan_entry->Lchan_info.Lchan_status_rx=MAC_RX_READY;


	Macphy_data_req->Pdu_type = DL_SACH;
	Macphy_data_req->CH_index = CH_index;
	Macphy_data_req->format_flag = 0;
	Macphy_data_req->Direction=RX;
	Macphy_data_req->Phy_resources=&Lchan_entry->Lchan_info.Phy_resources_rx;
	Macphy_data_req->Lchan_id.Index=Lchan_entry->Lchan_info.Lchan_id.Index;
        Macphy_data_req->num_tb = Lchan_entry->Lchan_info.Nb_sched_tb_dl;
	Macphy_data_req->tb_size_bytes = Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size;

#ifdef DEBUG_MAC_CHBCH
	// Program DL_SACH for reception	
	msg("[MAC][UE][DECODE CHBCH] Frame %d: Programming DL_SACH (LCHAN %d) for RX (Freq %X, Num_tb %d)\n",
	    Mac_rlc_xface->frame,
	    Lchan_entry->Lchan_info.Lchan_id.Index,
	    RX_chbch_pdu->DL_sacch_pdu[i].Freq_alloc,
	    Macphy_data_req->num_tb);
#endif //DEBUG_MAC_CHBCH
       
	bitmap = 0;
	bitmap_cnt = Macphy_data_req->num_tb;
	
	while (bitmap_cnt>0) {
	  bitmap = (bitmap<<1) + 1;
	  bitmap_cnt--;
	}

	Macphy_data_req->Dir.Req_rx.Active_process_map = bitmap;

#ifdef DEBUG_MAC_CHBCH
	msg("[MAC][UE] TTI %d Node %d NODE %d :DL_SACH_DATA_REQ Lchan %d, FreqAlloc %X, num_tb %d \n",
	    Mac_rlc_xface->frame,
	    NODE_ID[Mod_id+NB_CH_INST],
	    NODE_ID[Mod_id+NB_CH_INST],
	    Lchan_entry->Lchan_info.Lchan_id.Index,
	    RX_chbch_pdu->DL_sacch_pdu[i].Freq_alloc,
	    Macphy_data_req->num_tb	    
	    );
#endif //DEBUG_MAC_CHBCH
      }
      else{
	msg("[OPENAIR][MAC] WARNING: Decode CHBCH, Rx DL_MAP for Lchan_id %d which is not yet configured!!!\n",
	    RX_chbch_pdu->DL_sacch_pdu[i].Lchan_id.Index);   
      }
    }
  }
  
#ifdef PHY_EMUL
  Activ_tti = (Mac_rlc_xface->frame%3);
#else
  Activ_tti = ((Mac_rlc_xface->frame  )%2);
#endif //PHY_EMUL  
  

  // Uplink SACH Information
  for(i=0;i<RX_chbch_pdu->Num_ul_sach;i++){ //Extract TxOps  //
    
    Idx2 = (RX_chbch_pdu->UL_alloc_pdu[i].Lchan_id.Index & RAB_OFFSET2 )>> RAB_SHIFT2; 
    Idx1 = (RX_chbch_pdu->UL_alloc_pdu[i].Lchan_id.Index & RAB_OFFSET1 )>> RAB_SHIFT1;
    In_idx=(RX_chbch_pdu->UL_alloc_pdu[i].Lchan_id.Index & RAB_OFFSET ); 	
    
#ifdef DEBUG_MAC_CHBCH
    msg("[OPENAIR][MAC][UE %d] Decoding UL_MAP %d: Idx1 %d,Idx2 %d,In_idx %d, Ch_index %d\n",
	NODE_ID[Mod_id+NB_CH_INST],i+1,Idx1,Idx2,In_idx,CH_index);  
#endif //DEBUG_MAC_CHBCH
    
    Lchan_entry=NULL;
    
    
    if(Idx1 == 0){ // towards CH
      if( Idx2 == Rrc_xface->UE_index[Mod_id+NB_CH_INST][CH_index]){//Some UL ressources for me
	
#ifdef DEBUG_MAC_CHBCH
	msg("[OPENAIR][MAC][UE %d] Decoding UL_MAP %d for Lchan_ID %d\n",NODE_ID[Mod_id+NB_CH_INST],
	    i+1,RX_chbch_pdu->UL_alloc_pdu[i].Lchan_id.Index);   
#endif //DEBUG_MAC_CHBCH
	if(In_idx == DCCH){
	  Lchan_entry = &UE_mac_inst[Mod_id].Dcch_lchan[CH_index];
	}	
	else {
	  Lchan_entry=&UE_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][CH_index];
	  if(Lchan_entry->Active==0)  {
	    msg("[MAC][UE %d] TTI %d: ue_decode_chbch UL_TX_OP for not yet active LCHAN %d\n",
		NODE_ID[Mod_id+NB_CH_INST],Mac_rlc_xface->frame,RX_chbch_pdu->UL_alloc_pdu[i].Lchan_id.Index);
	    continue;
	  }
	}	
	if( Lchan_entry->Active == 1 ){

	  if(Lchan_entry->Lchan_info.W_idx==0){
	    
	    if(In_idx == DCCH){
		
	      Rrc_xface->def_meas_ind(Mod_id,CH_index);
	      
	      Tb_size=Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size;
		
	      rlc_status=mac_rlc_status_ind(Mod_id+NB_CH_INST,Lchan_entry->Lchan_info.Lchan_id.Index,
					    Tb_size,
					    NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx);
	      
	      G_size=Mac_rlc_xface->mac_rlc_data_req(Mod_id+NB_CH_INST,
						       Lchan_entry->Lchan_info.Lchan_id.Index,
						       &Lchan_entry->Lchan_info.Current_payload_tx[Lchan_entry->Lchan_info.W_idx*Tb_size]);
	      
	      if(G_size==0) {
		
#ifdef DEBUG_MAC_CHBCH
		msg("[MAC]UE %d: rlc_data_req on dcch_lchan %d return 0 bytes!!!\n",NODE_ID[Mod_id+NB_CH_INST],
		    Lchan_entry->Lchan_info.Lchan_id.Index);
#endif //DEBUG_MAC_CHBCH
		
		Rrc_xface->def_meas_ind(Mod_id,CH_index);
		G_size=Mac_rlc_xface->mac_rlc_data_req(Mod_id+NB_CH_INST,
						       Lchan_entry->Lchan_info.Lchan_id.Index,
						       &Lchan_entry->Lchan_info.Current_payload_tx[Lchan_entry->Lchan_info.W_idx*Tb_size]);
		
		if(G_size==0){
		  msg("[OPEMAIR][MAC] No DEFAULT MEASUREMENT ON DCCH FROM RRC\n");
		  mac_xface->macphy_exit("");
		  continue;
		}
	      }
	      
	      Lchan_entry->Lchan_info.W_idx+=G_size/Tb_size;
	      if(Lchan_entry->Lchan_info.W_idx > NB_TB_BUFF_MAX){
		msg("[MAC][UE] TTI %d: ue_control_plane_procedures.c (ue_get_chbch) RLC RETURNS TOO Many TBs !!!!\n",
		    Mac_rlc_xface->frame);
		mac_xface->macphy_exit("");
	      }
	    }
	    else{
	      msg("[MAC]UE %d: Get Tx Op for Lchan %d with No Data to Tx!!!\n",NODE_ID[Mod_id+NB_CH_INST],
		  Lchan_entry->Lchan_info.Lchan_id.Index);
	      //  mac_xface->macphy_exit("");
	      continue;
	    }
	  }

	  #ifdef DEBUG_MAC_CHBCH
	  msg("[MAC][UE] Frame %d: NODE %d, TTI %d : Registering_Tx_ops for LCHAN_ID %d(%d)from CH_INDEX %d,FREQ_ALLOC %x,Nb_TB %d\n",
	      Mac_rlc_xface->frame,
	      NODE_ID[Mod_id+NB_CH_INST],
	      Mac_rlc_xface->frame,
	      Lchan_entry->Lchan_info.Lchan_id.Index,RX_chbch_pdu->UL_alloc_pdu[i].Lchan_id.Index,
	      CH_index,
	      RX_chbch_pdu->UL_alloc_pdu[i].Freq_alloc,
	      RX_chbch_pdu->UL_alloc_pdu[i].Nb_tb);
	  #endif //DEBUG_MAC_CHBCH
	  
	  memcpy(&UE_mac_inst[Mod_id].Tx_ops[CH_index][Activ_tti][UE_mac_inst[Mod_id].Nb_tx_ops[CH_index][Activ_tti]].UL_alloc_pdu,
		 (UL_ALLOC_PDU*)&RX_chbch_pdu->UL_alloc_pdu[i],
		 sizeof(UL_ALLOC_PDU));

	  Lchan_entry->Lchan_info.Bw_req_active=0;
	  UE_mac_inst[Mod_id].Tx_ops[CH_index][Activ_tti][UE_mac_inst[Mod_id].Nb_tx_ops[CH_index][Activ_tti]].Lchan_entry = Lchan_entry;
	  UE_mac_inst[Mod_id].Nb_tx_ops[CH_index][Activ_tti]++;

	}
      }
    }
    else{ // DIL TX/RX
      Direction=(RX_chbch_pdu->UL_alloc_pdu[i].Lchan_id.Index & DIR_OFFSET);	
      if (Idx2 == Rrc_xface->UE_index[Mod_id][CH_index])     
	Lchan_entry=&UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx][CH_index][Idx1];
      else if(Idx1 == Rrc_xface->UE_index[Mod_id][CH_index])  
	Lchan_entry=&UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx][CH_index][Idx2];
      if(Lchan_entry){
	if( (Direction == TX && Idx2 == Rrc_xface->UE_index[Mod_id][CH_index]) || 
	    (Direction == RX && Idx1 == Rrc_xface->UE_index[Mod_id][CH_index])	   ){
	  //	  UE_mac_inst[Mod_id].Rx_sched[idx][UE_mac_inst[Mod_id].Nb_rx_sched[idx]].Lchan_id.Index
	  // = Lchan_entry->Lchan_info.Lchan_id.Index + ( CH_index << CH_SHIFT );

	  // UE_mac_inst[Mod_id].Rx_sched[idx][UE_mac_inst[Mod_id].Nb_rx_sched[idx]].Phy_resources.Time_alloc
	  // = RX_chbch_pdu->UL_alloc_pdu[i].Time_alloc;
	  //UE_mac_inst[Mod_id].Rx_sched[idx][UE_mac_inst[Mod_id].Nb_rx_sched[idx]++].Phy_resources.Freq_alloc
	  //  = RX_chbch_pdu->UL_alloc_pdu[i].Freq_alloc;	 

	}
	else if( (Direction == RX && Idx2 == Rrc_xface->UE_index[Mod_id+NB_CH_INST][CH_index]) || 
	         (Direction == TX && Idx1 == Rrc_xface->UE_index[Mod_id+NB_CH_INST][CH_index])){
	  //	  memcpy(&UE_mac_inst[Mod_id].Tx_ops[UE_mac_inst[Mod_id].Nb_tx_ops].UL_alloc_pdu,
	  //	  (UL_ALLOC_PDU*)&RX_chbch_pdu->UL_alloc_pdu[i],sizeof(UL_ALLOC_PDU));
	  //UE_mac_inst[Mod_id].Tx_ops[UE_mac_inst[Mod_id].Nb_tx_ops].Lchan_entry = Lchan_entry;
	  // UE_mac_inst[Mod_id].Nb_tx_ops++;	
	}	

      } 		
      else{ //DECODE UL_SACH PILOT FOR MEASUREMENT
	//	UE_mac_inst[Mod_id].Rx_sched[idx][UE_mac_inst[Mod_id].Nb_rx_sched[idx]].Lchan_id.Index=  Lchan_entry->Lchan_info.Lchan_id.Index +  ( CH_index << CH_SHIFT );
	//UE_mac_inst[Mod_id].Rx_sched[idx][UE_mac_inst[Mod_id].Nb_rx_sched[idx]].Phy_resources.Time_alloc
	// = RX_chbch_pdu->UL_alloc_pdu[i].Time_alloc;
	//	UE_mac_inst[Mod_id].Rx_sched[idx][UE_mac_inst[Mod_id].Nb_rx_sched[idx]++].Phy_resources.Freq_alloc = 0;
	
      } 
    }
  } 

  // Process DL Measurement information
  
  ue_process_DL_meas(Mod_id,CH_index);
 
  if(++UE_mac_inst[Mod_id].NB_decoded_chbch == NB_SIG_CNX_UE)
    ue_complete_dl_data_req(Mod_id); 
  
}

/********************************************************************************************************************/
void ue_complete_dl_data_req(unsigned char Mod_id){
/********************************************************************************************************************/

  unsigned char CH_index,ICH_index,i;
  for(CH_index=0;CH_index<NB_SIG_CNX_UE;CH_index++)
    for(i=0;i<NB_REQ_MAX;i++)
      if(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active == 1)
	if (Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Direction == RX){
	  ICH_index=(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.CH_index+1)%2;
	  Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Ifreq_alloc
	    = UE_mac_inst[Mod_id].CH_ul_freq_map[ICH_index];
	
	}
}

/********************************************************************************************************************/
void ue_decode_sch(u8 Mod_id, UL_MEAS *UL_meas, u16 Idx2){
  /********************************************************************************************************************/

  unsigned char i;
  if(UE_mac_inst[Mod_id].Def_meas[Idx2].Active==1){
    //msg("[MAC][UE %d]Frame %d: GOT DL_SCH from CH %d\n",NODE_ID[Mod_id+NB_CH_INST],Mac_rlc_xface->frame,Idx2);
    for(i=0;i<NUMBER_OF_MEASUREMENT_SUBBANDS;i++){
      UE_mac_inst[Mod_id].Def_meas[Idx2].Sinr_meas[0][i]=
	((UE_mac_inst[Mod_id].Def_meas[Idx2].Sinr_meas[0][i]*UE_mac_inst[Mod_id].Def_meas[Idx2].Forg_fact)
	 + (UL_meas->Sub_band_sinr[i]*(10-UE_mac_inst[Mod_id].Def_meas[Idx2].Forg_fact)))/10;         
      //      msg("Sinr_meas[%d]=%d\n",i,UE_mac_inst[Mod_id].Def_meas[Idx2].Sinr_meas[0][i]);
    }
  }
}

/****************************************************************************************************************/
void mac_check_rlc_queues_status(unsigned char Mod_id, unsigned char CH_index, UL_SACCH_FB *UL_sacch_fb){
  /****************************************************************************************************************/

  mac_rlc_status_resp_t rlc_status;
  unsigned char j,i,k=1;
  if(UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.W_idx > 0)
    UL_sacch_fb->Qdepth += k;
  k*=2;
  for(j=1;j<NB_RAB_MAX;j++){
    if(UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Active ==1){
      if( (UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.W_idx > 0) && (UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.Lchan_status_tx!=MAC_TX_READY)){
	UL_sacch_fb->Qdepth += k;//(UL_sacch_fb->Qdepth << 2) + j+1;  
#ifdef DEBUG_FEEDBACK
	msg("[MAC]UE %d: TTI %d: CHECK_RLC_QUEUES_STATUS: Lchan_id %d has %d tbs, Qdepth=%d \n",
	    NODE_ID[Mod_id+NB_CH_INST],
	    Mac_rlc_xface->frame,
	    UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.Lchan_id.Index,
	    UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.W_idx,
	    UL_sacch_fb->Qdepth);
#endif
      }
    }
    k*=2; 
  }
}

/****************************************************************************************************************/
void ue_fill_macphy_data_req(u8 Mod_id,LCHAN_INFO_TABLE_ENTRY *Lchan_entry,unsigned char CH_index){
  /****************************************************************************************************************/

  MACPHY_DATA_REQ *Macphy_data_req;
  unsigned char Nb_tb;u32 G_size,j; 
  mac_rlc_status_resp_t rlc_status;
  unsigned short In_idx,Idx1,Idx2,Tb_size,i;
  unsigned char Sch_index;
  unsigned int bitmap,bitmap_cnt;
  //  Tx_size =Lchan_info.Lchan_desc[1].transport_block_size * 

  if (Lchan_entry) {

    Tb_size = Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size;
    Nb_tb   = Lchan_entry->Lchan_info.Nb_sched_tb_ul;
    
    if( (Nb_tb == 0) || (Nb_tb > Lchan_entry->Lchan_info.W_idx) ){
      
      msg("[MAC][UE %d][FATAL] TTI %d : (ue_fill_macphy_data_req) :  LCHAN %d, Freq %X, Nb_tb %d, W_idx %d\n",
	  NODE_ID[Mod_id+NB_CH_INST],
	  Mac_rlc_xface->frame,
	  Lchan_entry->Lchan_info.Lchan_id.Index,
	  Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc,
	  Nb_tb,
	  Lchan_entry->Lchan_info.W_idx);
      Lchan_entry->Lchan_info.Lchan_status_tx = LCHAN_IDLE;
      
      //       mac_xface->macphy_exit("");

      return;
    }

#ifdef DEBUG_MAC_UE_TX 
    msg("[MAC][UE %d] TTI %d ue_fill_macphy_data_req :  LCHAN %d, Time %X: Freq %X, Nb_tb %d, W_idx %d\n",
	NODE_ID[Mod_id+NB_CH_INST],
	Mac_rlc_xface->frame,
	Lchan_entry->Lchan_info.Lchan_id.Index,
	Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc,
	Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc,
	Nb_tb,
	Lchan_entry->Lchan_info.W_idx);
#endif //DEBUG_MAC_UE_TX 
    
    if((Macphy_data_req = new_macphy_data_req(Mod_id+NB_CH_INST))==NULL) {
      msg("[MAC][UE][FATAL] : TTI %d: ue_control_plane_procedures.c (ue_fill_macphy_data_req), new_macphy_data_req returns NULL\n",
	  Mac_rlc_xface->frame);
      mac_xface->macphy_exit("");
      return;
    }
    
    memcpy(&Macphy_data_req->Dir.Req_tx.Pdu.UL_sach_pdu.Sach_payload[0], 
     	   &Lchan_entry->Lchan_info.Current_payload_tx[0],
	   Nb_tb * Tb_size );
#ifdef DEBUG_MAC_RLC 
   if ( (Lchan_entry->Lchan_info.Lchan_id.Index & RAB_OFFSET)!=DCCH)
	for(i=0;i<Nb_tb;i++)
	  msg("[MAC][UE]TTI %d: TX TB with SN %d\n",
	      Mac_rlc_xface->frame,
	      (unsigned int)((Lchan_entry->Lchan_info.Current_payload_tx[i*Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size])>>1)&0x7F);	
#endif //DEBUG_MAC_RLC 
   
    int W_idx=Lchan_entry->Lchan_info.W_idx-Nb_tb;
    int R_idx=0;

    while(W_idx >= Nb_tb){     
      memcpy(&Lchan_entry->Lchan_info.Current_payload_tx[R_idx*Tb_size],
	     &Lchan_entry->Lchan_info.Current_payload_tx[(R_idx+Nb_tb) * Tb_size],
	     (Nb_tb)*Tb_size);
      W_idx-=Nb_tb;
      R_idx+=Nb_tb;	
    }    
    if(W_idx >0){
      memcpy(&Lchan_entry->Lchan_info.Current_payload_tx[R_idx*Tb_size],
	     &Lchan_entry->Lchan_info.Current_payload_tx[(R_idx+Nb_tb) * Tb_size],
	     (W_idx)*Tb_size);
    }

    Lchan_entry->Lchan_info.W_idx-=Nb_tb;
    Lchan_entry->Lchan_info.NB_TX+=Nb_tb;
    Lchan_entry->Lchan_info.NB_TX_TB[Nb_tb]++;    
    
#ifdef PHY_EMUL   
    Macphy_data_req->Dir.Req_tx.Pdu.UL_sach_pdu.UL_sacch_pdu.Pdu_size=Nb_tb*Tb_size;
#endif   
    
    Macphy_data_req->Direction = TX;
    Macphy_data_req->Pdu_type  = UL_SACH;
    
    if (Is_rrc_registered == 1) {
      Macphy_data_req->Lchan_id.Index  = 
	(Rrc_xface->UE_index[Mod_id+NB_CH_INST][CH_index] << RAB_SHIFT2 )+(Lchan_entry->Lchan_info.Lchan_id.Index & RAB_OFFSET);
    } 
    else {//RF ???????????????????????????? 
      Macphy_data_req->Lchan_id.Index  = 
	(Lchan_entry->Lchan_info.Lchan_id.Index & RAB_OFFSET);
    }
    
    Macphy_data_req->CH_index        = CH_index;
    Macphy_data_req->num_tb        = Nb_tb;
    Macphy_data_req->tb_size_bytes = Tb_size;
    bitmap = 0;
    bitmap_cnt = Macphy_data_req->num_tb;
  
    while (bitmap_cnt>0) {
      bitmap = (bitmap<<1) + 1;
      bitmap_cnt--;
    }
    
    // Fill HARQ info
    Macphy_data_req->Dir.Req_tx.Active_process_map = bitmap;
    Macphy_data_req->Dir.Req_tx.New_process_map    = bitmap;
    Macphy_data_req->Dir.Req_tx.Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.Qdepth=0;	
    
    Idx2 = (Lchan_entry->Lchan_info.Lchan_id.Index & RAB_OFFSET2 )>> RAB_SHIFT2; 
    Idx1 = (Lchan_entry->Lchan_info.Lchan_id.Index & RAB_OFFSET1 )>> RAB_SHIFT1;
    In_idx=(Lchan_entry->Lchan_info.Lchan_id.Index & RAB_OFFSET );
    
    
    if(Idx2 == CH_index){//LOCAL
      Idx1 = Rrc_xface->UE_index[Mod_id+NB_CH_INST][CH_index];

#ifdef DEBUG_MAC_UE_TX
	  msg("\n______________////////////MAC UE %d GENERATE SACH NB_tb %d--------------__________\n ",NODE_ID[Mod_id+NB_CH_INST],Nb_tb);
	msg("[MAC][UE %d]TTi %d: STATUS IND on Lchan %d for %d Tbs, Current MAC BUFFER has %d Tb\n",
	    NODE_ID[Mod_id+NB_CH_INST],
	    Mac_rlc_xface->frame,
	    Lchan_entry->Lchan_info.Lchan_id.Index,
	    NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx,
	    Lchan_entry->Lchan_info.W_idx);
#endif //DEBUG_MAC_UE_TX

      if(In_idx == DCCH){
	mac_check_rlc_queues_status(Mod_id,CH_index,&Macphy_data_req->Dir.Req_tx.Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb);
	Macphy_data_req->format_flag = 1;
	if (Lchan_entry->Lchan_info.W_idx==0){
	  //  msg("Activate BW REQ on %d\n",Lchan_entry->Lchan_info.Lchan_id.Index);
	  Lchan_entry->Lchan_info.Bw_req_active=1;
	}
      }
      else{ // DTCH 
	Macphy_data_req->format_flag = 1;

	rlc_status=mac_rlc_status_ind(Mod_id+NB_CH_INST,
				      Lchan_entry->Lchan_info.Lchan_id.Index,
				      Tb_size,
				      NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx);
	
	G_size=Mac_rlc_xface->mac_rlc_data_req(Mod_id+NB_CH_INST,
					       Lchan_entry->Lchan_info.Lchan_id.Index,
					       &Lchan_entry->Lchan_info.Current_payload_tx[Lchan_entry->Lchan_info.W_idx*Tb_size]);
	

	Lchan_entry->Lchan_info.W_idx+=G_size/Tb_size;
	if (Lchan_entry->Lchan_info.W_idx==0){
	  //msg("Activate BW REQ on %d\n",Lchan_entry->Lchan_info.Lchan_id.Index);
	  Lchan_entry->Lchan_info.Bw_req_active=1;
	}	

#ifdef DEBUG_MAC_RLC

	msg("[MAC][UE]TTi %d: RLC DATA REQ on Lchan %d for %d Tbs returns %d TBs, Current MAC BUFFER has %d Tb\n",
	    Mac_rlc_xface->frame,
	    Lchan_entry->Lchan_info.Lchan_id.Index,
	    NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx,
	    G_size/Tb_size,
	    Lchan_entry->Lchan_info.W_idx);
#endif
	Macphy_data_req->Dir.Req_tx.Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.Qdepth=Lchan_entry->Lchan_info.W_idx;

	
#ifdef DEBUG_FEEDBACK
	msg("[MAC][UE] TTI %d, INST %d: Qdepth of LC %d is %d Tb (Bytes in buffer %d)\n",
	    Mac_rlc_xface->frame,
	    Mod_id,
	    Lchan_entry->Lchan_info.Lchan_id.Index,
	    Macphy_data_req->Dir.Req_tx.Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.Qdepth,
	    rlc_status.bytes_in_buffer);
#endif //DEBUG_FEEDBACK
	
      }
      
      // Fill CQI information in SACCH_FB based on CHSCH measurements     
      Macphy_data_req->Dir.Req_tx.Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.cqi           = get_chsch_subband_quality(Mod_id,CH_index);      
      Macphy_data_req->Dir.Req_tx.Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.Wideband_sinr=UE_mac_inst[Mod_id].Def_meas[CH_index].Wideband_sinr;
    }

#ifdef DEBUG_FEEDBACK
	msg("[MAC][UE] TTI %d:CH_index=%d, CQI = %s,Wideband SINR %d dB\n",Mac_rlc_xface->frame,
	CH_index,
	print_cqi(Macphy_data_req->Dir.Req_tx.Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.cqi),
	Macphy_data_req->Dir.Req_tx.Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.Wideband_sinr);
#endif //DEBUG_FEEDBACK      
	
    Macphy_data_req->Phy_resources = &Lchan_entry->Lchan_info.Phy_resources_tx;
    Lchan_entry->Lchan_info.Lchan_status_tx = LCHAN_IDLE;
    if (Macphy_data_req->Phy_resources->Coding_fmt > 1)
      msg("[MAC][UE] TTI %d: CH_index%d Illegal Coding fmt %d\n",Mac_rlc_xface->frame,CH_index,Macphy_data_req->Phy_resources->Coding_fmt);


#ifdef DEBUG_MAC_UE_TX
    if(Macphy_data_req->Phy_resources->Ifreq_alloc!=0){
	msg("\n___________________///////////////////MAC UE GENERATE SACH NB_tb %d--------------______________\n ",Nb_tb);
	msg("[MAC][UE %d]TTi %d: GENARATE SACH on Lchan %d for %d Tbs over FREQ ALLOC %x, INTERFERING_CH_FREQ_MAP %x\n",
	    NODE_ID[Mod_id+NB_CH_INST],
	    Mac_rlc_xface->frame,
	    Lchan_entry->Lchan_info.Lchan_id.Index,
	    Nb_tb,
	    Macphy_data_req->Phy_resources->Freq_alloc,
	    Macphy_data_req->Phy_resources->Ifreq_alloc);
    }
#endif //DEBUG_MAC_UE_TX

  }
  else {
    msg("[MAC][UE] TTI %d: ue_control_plane_procedures.c, ue_fill_macphy_data_req : Lchan_entry is Null, exiting\n",
	Mac_rlc_xface->frame);
    mac_xface->macphy_exit("");
  }
  
}


/****************************************************************************************************************/
void ue_generate_sach(u8 Mod_id){
  /****************************************************************************************************************/

  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  u8 i,j,k;  
  for(i=0;i<NB_CNX_UE;i++){
    if((UE_mac_inst[Mod_id].Dcch_lchan[i].Active == 1) && 
       (UE_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Lchan_status_tx == MAC_TX_READY)){
      Lchan_entry=&UE_mac_inst[Mod_id].Dcch_lchan[i];
      ue_fill_macphy_data_req(Mod_id,Lchan_entry,i);
    }
    for(j=0;j<NB_RAB_MAX;j++){
      if((UE_mac_inst[Mod_id].Dtch_lchan[j][i].Active == 1)&&
	 (UE_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_status_tx == MAC_TX_READY)){
	Lchan_entry=&UE_mac_inst[Mod_id].Dtch_lchan[j][i];
	ue_fill_macphy_data_req(Mod_id,Lchan_entry,i);
      }
    }
  }
}

/****************************************************************************************************************/
void ue_decode_sach(u8 Mod_id, DL_SACH_PDU *Sach_pdu,UL_MEAS*UL_meas,unsigned short Index,int *crc_status) {
  /****************************************************************************************************************/
  u16 Rx_size,i;
  
  unsigned char Idx1= (Index & RAB_OFFSET1) >>RAB_SHIFT1;
  unsigned char Idx2= (Index & RAB_OFFSET2) >>RAB_SHIFT2;
  unsigned char In_idx= (Index & RAB_OFFSET);
  unsigned char Dil_ch_index = (Index & CH_OFFSET) >> CH_SHIFT;

  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  char *Def_sinr_meas;
  unsigned char Nb_tb;
 
  if( Idx1 == 0){
    if(In_idx ==DCCH){
      Lchan_entry = &UE_mac_inst[Mod_id].Dcch_lchan[Idx2];
      Def_sinr_meas = UE_mac_inst[Mod_id].Def_meas[Idx2].Sinr_meas[0];
    }
    else{
      Lchan_entry = &UE_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Idx2];
      Def_sinr_meas = UE_mac_inst[Mod_id].Def_meas[Idx2].Sinr_meas[0];
    }
  }
  else{
    if(Idx1 == Rrc_xface->UE_index[Mod_id+NB_CH_INST][Dil_ch_index]){
      Lchan_entry = &UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx][Dil_ch_index][Idx2];
      Def_sinr_meas = UE_mac_inst[Mod_id].Def_meas[Dil_ch_index].Sinr_meas[Idx2];       
    }
    else if(Idx2 == Rrc_xface->UE_index[Mod_id+NB_CH_INST][Dil_ch_index]){
      Def_sinr_meas = UE_mac_inst[Mod_id].Def_meas[Dil_ch_index].Sinr_meas[Idx1];       
      Lchan_entry = &UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx][Dil_ch_index][Idx1];
    }
    else{ msg("erooorr dil decode sach\n"); 
      mac_xface->macphy_exit(" ");}
  }
  
  if(Lchan_entry->Lchan_info.Lchan_status_rx == MAC_RX_READY){
    
    Nb_tb=Lchan_entry->Lchan_info.Nb_sched_tb_dl;
    Rx_size = Nb_tb * Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size;
    Lchan_entry->Lchan_info.NB_RX+=Nb_tb;
    Lchan_entry->Lchan_info.NB_RX_TB[Nb_tb]+=Nb_tb;
    
    for (i=0;i<Nb_tb;i++)
      if (crc_status[i]<0) {
	Lchan_entry->Lchan_info.Lchan_status_rx = MAC_RX_OK;
	Lchan_entry->Lchan_info.NB_RX_ERRORS++;
	Lchan_entry->Lchan_info.NB_RX_ERRORS_TB[Nb_tb]++;

#ifdef DEBUG_MAC_UE_RX
	msg("[MAC][UE] TTI %d: DL_SACH for LCHAN %d in error (num_tb %d, TB %d)\n",
	    Mac_rlc_xface->frame,
	    Lchan_entry->Lchan_info.Lchan_id.Index,
	    Nb_tb,i);
#endif //DEBUG_MAC_UE_RX
	
      }

    
#ifdef DEBUG_MAC_UE_RX
    msg("____________________________________MAC  UE  RXXXXXXXXXXXXXXXXXXXXXX_____________________________________________\n");
    msg("[MAC][UE] TTI %d NODE %d, Inst %d RX_DL_SACH Channel_id %d (CRC %d), @ DL_PDU %p, Payload %p, Nb_tb %d, freq_alloc %x\n",
	  Mac_rlc_xface->frame,
	  NODE_ID[Mod_id+NB_CH_INST],
	  Mod_id,
	  Lchan_entry->Lchan_info.Lchan_id.Index,
	  crc_status[0],
	  Sach_pdu,
	  Sach_pdu->Sach_payload,
	  Nb_tb,
	  Lchan_entry->Lchan_info.Phy_resources_rx.Freq_alloc);
#endif //DEBUG_MAC_UE_RX

    memcpy(&Lchan_entry->Lchan_info.Current_payload_rx[0],Sach_pdu->Sach_payload,Rx_size);

    if (Is_rrc_registered == 1) {
      Mac_rlc_xface->mac_rlc_data_ind(Mod_id+NB_CH_INST,
				      Lchan_entry->Lchan_info.Lchan_id.Index,
				      &Lchan_entry->Lchan_info.Current_payload_rx[0],
				      Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size,
				      Nb_tb,
				      (unsigned int *)crc_status);
    }
    
    Lchan_entry->Lchan_info.Lchan_status_rx = MAC_RX_OK;
  }
  
  else{
    msg("[OPENAIR][UE]DECODE SACH at Lchan_id %d which was not scheduled!!!\n",Index);
    mac_xface->macphy_exit("");
  }

  if(UL_meas && (In_idx!=DTCH_BD)){
    //mac_update_meas(Mod_id,&Lchan_entry->Lchan_info.Meas_entry,UL_meas);
    //*Def_sinr_meas =  Lchan_entry->Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Sinr;
  }  
}

/****************************************************************************************************************/
void ue_scheduler(unsigned char Mod_id, unsigned char CH_index){
  /****************************************************************************************************************/
  unsigned char i,j;

#ifdef PHY_EMUL
  j = (Mac_rlc_xface->frame%3);
#else
  j = (Mac_rlc_xface->frame%2);
#endif //PHY_EMUL
  
  for(i=0;i<UE_mac_inst[Mod_id].Nb_tx_ops[CH_index][j];i++){
#ifdef DEBUG_MAC_SCHEDULING
    msg("[OPENAIR][MAC] TTI %d: UE SCHEDULE LCHAN_ID %d, NB_TB %d\n", 
      	Mac_rlc_xface->frame,
	UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].Lchan_entry->Lchan_info.Lchan_id.Index,
	UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].UL_alloc_pdu.Nb_tb);  
#endif
      UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc=
	UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].UL_alloc_pdu.Freq_alloc;
      UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].Lchan_entry->Lchan_info.Phy_resources_tx.Coding_fmt=
	UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].UL_alloc_pdu.Coding_fmt;
      UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].Lchan_entry->Lchan_info.Nb_sched_tb_ul = 
	UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].UL_alloc_pdu.Nb_tb;
      UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].Lchan_entry->Lchan_info.Nb_sched_tb_ul_temp =
	UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].Lchan_entry->Lchan_info.Nb_sched_tb_ul; 
      UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].Lchan_entry->Lchan_info.Lchan_status_tx = MAC_TX_READY;    
  }
  UE_mac_inst[Mod_id].Nb_tx_ops[CH_index][j]=0;
}  

/****************************************************************************************************************/
void ue_generate_rach(u8 Mod_id,u8 CH_index){
  /****************************************************************************************************************/

  u8 Size=0,W_idx=2,j;
  MACPHY_DATA_REQ *Macphy_data_req;

  if (Is_rrc_registered == 1)
    Size = Rrc_xface->mac_rrc_data_req(Mod_id+NB_CH_INST,
				       UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Lchan_id.Index,1,
				       &UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Current_payload_tx[0],
				       CH_index);
 
 
  if(Size){

#ifdef DEBUG_RACH_RRC
    msg("[OPENAIUR][UE]GENERATE_RACH\n");
    msg("[MAC] UE %d: generate rach for CH_ID(Index) %d, from CCCH_LCHAN ID %d, Size %d\n",NODE_ID[Mod_id+NB_CH_INST],
	CH_index,UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Lchan_id.Index,Size);
#endif //DEBUG_RACH_RRC

    if ((Macphy_data_req = new_macphy_data_req(Mod_id+NB_CH_INST))==NULL)
      return;
    
    Macphy_data_req->Pdu_type=RACH;
    Macphy_data_req->CH_index=CH_index;
    Macphy_data_req->Direction=TX;
    Macphy_data_req->Dir.Req_tx.Pdu.Rach_pdu.Rach_payload= &UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Current_payload_tx[0];
    
#ifdef PHY_EMUL
    Macphy_data_req->Dir.Req_tx.Pdu.Rach_pdu.Pdu_size = Size; 
#endif

    Macphy_data_req->Phy_resources=&UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Phy_resources_tx;
    Macphy_data_req->Phy_resources->Time_alloc=RACH_TIME_ALLOC;
    Macphy_data_req->Phy_resources->Freq_alloc=RACH0_FREQ_ALLOC;
    Macphy_data_req->Phy_resources->Coding_fmt=0;
    Macphy_data_req->num_tb = 1;
    Macphy_data_req->tb_size_bytes = 12;
    Macphy_data_req->Dir.Req_tx.Active_process_map = 1;
    Macphy_data_req->Dir.Req_tx.New_process_map = 1;
  }

  else{   

    UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Current_payload_tx[W_idx]=0;
    if(UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Active == 1)
      if((UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.W_idx != 0) 
	 && (!is_lchan_ul_scheduled(Mod_id,CH_index,UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.Lchan_id.Index))
	 &&(UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.Bw_req_active==1)){
	UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Current_payload_tx[W_idx]
	  = UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.W_idx;
	UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.Bw_req_active=0;
	Size++;
#ifdef DEBUG_RACH_MAC
	msg("[MAC] UE %d: TTI %d: generate RACH for CH %d: Requesting BW for %d TB on Lchan %d\n",NODE_ID[Mod_id+NB_CH_INST],
	    Mac_rlc_xface->frame,CH_index,UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.W_idx,
	    UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.Lchan_id.Index);
#endif //DEBUG_RACH_MAC
	UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.NB_BW_REQ_TX++;
      }
    W_idx++;
    
    for(j=0;j<NB_RAB_MAX;j++){
      UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Current_payload_tx[W_idx]=0;
      if(UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Active == 1)
	if((UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.W_idx > 0) 
	   && (!is_lchan_ul_scheduled(Mod_id,CH_index,UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.Lchan_id.Index))
	   &&(UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.Bw_req_active==1)){
	  UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Current_payload_tx[W_idx]
	    = UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.W_idx;
	    UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.Bw_req_active=0;
	  Size++;
#ifdef DEBUG_RACH_MAC
	  msg("[MAC] UE %d: TTI %d: generate RACH for CH %d: Requesting BW for Lchan %d\n",NODE_ID[Mod_id+NB_CH_INST],
	      Mac_rlc_xface->frame,CH_index,UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.Lchan_id.Index);
#endif //DEBUG_RACH_MAC
	  UE_mac_inst[Mod_id].Dtch_lchan[j][CH_index].Lchan_info.NB_BW_REQ_TX++;
	}
      W_idx++;
    }
    



    if(Size){
      
      if ((Macphy_data_req = new_macphy_data_req(Mod_id+NB_CH_INST))==NULL)
	return;
      
      Macphy_data_req->Pdu_type=RACH;
      Macphy_data_req->CH_index=CH_index;
      Macphy_data_req->Direction=TX;
      
      UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Current_payload_tx[0] = MAC_RACH_BW_REQ;
      UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Current_payload_tx[1]=Rrc_xface->UE_index[Mod_id+NB_CH_INST][CH_index];
      Macphy_data_req->Dir.Req_tx.Pdu.Rach_pdu.Rach_payload= &UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Current_payload_tx[0];
	     
#ifdef PHY_EMUL
      Macphy_data_req->Dir.Req_tx.Pdu.Rach_pdu.Pdu_size = Size; 
#endif
      
      Macphy_data_req->Phy_resources=&UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Phy_resources_tx;
      Macphy_data_req->Phy_resources->Time_alloc=RACH_TIME_ALLOC;
      Macphy_data_req->Phy_resources->Freq_alloc=RACH0_FREQ_ALLOC;
      Macphy_data_req->Phy_resources->Coding_fmt=0;
      Macphy_data_req->num_tb = 1;
      Macphy_data_req->tb_size_bytes = 12;
      Macphy_data_req->Dir.Req_tx.Active_process_map = 1;
      Macphy_data_req->Dir.Req_tx.New_process_map = 1;


    }
    
  }
}


/****************************************************************************************************************/
  int is_lchan_ul_scheduled(unsigned char Mod_id, unsigned char CH_index, unsigned short Lchan_index){
  /****************************************************************************************************************/
    unsigned char i,j;
    
    for(j=0;j<3;j++)
      for(i=0; i< UE_mac_inst[Mod_id].Nb_tx_ops[CH_index][j];i++)
	if(UE_mac_inst[Mod_id].Tx_ops[CH_index][j][i].Lchan_entry->Lchan_info.Lchan_id.Index==Lchan_index)
	  return 1;
    return 0;
  }
