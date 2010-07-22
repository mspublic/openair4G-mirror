/*_______________________nodeb_controle_plane_procedures.c________________________

  Authors : Hicham Anouar, Raymond Knopp
  Company : EURECOM
  Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
  ________________________________________________________________*/

#include "extern.h"
#include "defs.h"

#ifdef PHY_EMUL
#include "SIMULATION/PHY_EMULATION/impl_defs.h"
#else
#include "PHY/impl_defs_top.h"
#include "PHY/LTE_TRANSPORT/defs.h"
#endif //PHY_EMUL

#include "PHY_INTERFACE/defs.h"
#include "PHY_INTERFACE/extern.h"
#include "COMMON/mac_rrc_primitives.h"
#include "PHY/LTE_TRANSPORT/extern.h"
#include "RRC/MESH/defs.h"

#define NDLS (CH_mac_inst[Mod_id].Num_dlsch)
#define NULS (CH_mac_inst[Mod_id].Num_ulsch)
//#define DEBUG_MAC_REPORT
//#define DEBUG_MAC_RLC
//#define DEBUG_MAC_SCHEDULING

//#define DEBUG_MAC_DCI
//#define DEBUG_MAC_CCCH
//#define DEBUG_MAC_BCCH

extern DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
extern DCI1A_5MHz_TDD_1_6_t      BCCH_alloc_pdu;
/*
void ch_fill_dil_map(u8 Mod_id,LCHAN_INFO_DIL_TABLE_ENTRY *Lchan_entry){

  u8 idx=(Mac_rlc_xface->frame+1)%2;
  //ch_fill_ul_map(Mod_id,Lchan_entry);
  CH_mac_inst[Mod_id].DCI_pdu.UL_alloc_pdu[NULS].Lchan_id.Index= Lchan_entry->Lchan_info_dil.Lchan_id.Index;
  //  CH_mac_inst[Mod_id].DCI_pdu.UL_alloc_pdu[NULS].Time_alloc=Lchan_entry->Lchan_info_dil.Phy_resources.Time_alloc;
  CH_mac_inst[Mod_id].DCI_pdu.UL_alloc_pdu[NULS].Freq_alloc=Lchan_entry->Lchan_info_dil.Phy_resources.Freq_alloc;
  CH_mac_inst[Mod_id].Rx_sched[idx][NULS].Phy_resources.Time_alloc=Lchan_entry->Lchan_info_dil.Phy_resources.Time_alloc;
  CH_mac_inst[Mod_id].Rx_sched[idx][NULS].Phy_resources.Freq_alloc=Lchan_entry->Lchan_info_dil.Phy_resources.Freq_alloc;
  CH_mac_inst[Mod_id].Rx_sched[idx][NULS].Lchan_id.Index=CH_mac_inst[Mod_id].DCI_pdu.UL_alloc_pdu[NULS-1].Lchan_id.Index;
  Lchan_entry->Lchan_info_dil.Lchan_status=MAC_RX_READY;  //-->get_sach()  
  NULS++;  //decode UL_SACCH  for measurement & scheduling
}
*/

void ch_fill_dl_map(u8 Mod_id,LCHAN_INFO_TABLE_ENTRY *Lchan_entry){

  /*
  CH_mac_inst[Mod_id].DCI_pdu.DL_sacch_pdu[NDLS].Freq_alloc = Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc;
  CH_mac_inst[Mod_id].DCI_pdu.DL_sacch_pdu[NDLS].Coding_fmt = Lchan_entry->Lchan_info.Phy_resources_tx.Coding_fmt;
  CH_mac_inst[Mod_id].DCI_pdu.DL_sacch_pdu[NDLS].Nb_tb      = Lchan_entry->Lchan_info.Nb_sched_tb_dl;
  CH_mac_inst[Mod_id].DCI_pdu.DL_sacch_pdu[NDLS].Lchan_id.Index = Lchan_entry->Lchan_info.Lchan_id.Index;
  */

  Lchan_entry->Lchan_info.Lchan_status_tx=MAC_TX_READY;  
  NDLS++;
 
#ifdef DEBUG_MAC_DCI  
  msg("[OPENAIR][MAC][NODEB %d] frame %d: DL_MAP %d for LCHAN %d, Tb_size %d, NUM_TB %d\n",
	NODE_ID[Mod_id],
      Mac_rlc_xface->frame,
      NDLS-1,
      Lchan_entry->Lchan_info.Lchan_id.Index,
      Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size);
#endif

}

void ch_fill_ul_map(u8 Mod_id,LCHAN_INFO_TABLE_ENTRY *Lchan_entry){


  u8 idx=(Mac_rlc_xface->frame)%3;

  /*
  CH_mac_inst[Mod_id].DCI_pdu.UL_alloc_pdu[NULS].Freq_alloc     =Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc;
  CH_mac_inst[Mod_id].DCI_pdu.UL_alloc_pdu[NULS].Coding_fmt     =Lchan_entry->Lchan_info.Phy_resources_rx_sched.Coding_fmt;
  CH_mac_inst[Mod_id].DCI_pdu.UL_alloc_pdu[NULS].Lchan_id.Index =Lchan_entry->Lchan_info.Lchan_id.Index;
  CH_mac_inst[Mod_id].DCI_pdu.UL_alloc_pdu[NULS].Nb_tb          =Lchan_entry->Lchan_info.Nb_sched_tb_ul_temp;
  // This corresponds to reception in TTI N+2
  CH_mac_inst[Mod_id].Rx_sched[idx][NULS].Phy_resources.Time_alloc=Lchan_entry->Lchan_info.Phy_resources_rx_sched.Time_alloc;  
  CH_mac_inst[Mod_id].Rx_sched[idx][NULS].Phy_resources.Freq_alloc=Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc;
  CH_mac_inst[Mod_id].Rx_sched[idx][NULS].Phy_resources.Coding_fmt=Lchan_entry->Lchan_info.Phy_resources_rx_sched.Coding_fmt;
  CH_mac_inst[Mod_id].Rx_sched[idx][NULS].Nb_tb = Lchan_entry->Lchan_info.Nb_sched_tb_ul_temp;
  CH_mac_inst[Mod_id].Rx_sched[idx][NULS].Lchan_id.Index=Lchan_entry->Lchan_info.Lchan_id.Index;
  */

  NULS++;
  CH_mac_inst[Mod_id].Nb_rx_sched[idx]++;  
  
  
#ifdef DEBUG_MAC_DCI
  msg("[OPENAIR][MAC][NODEB %d] frame %d: UL_MAP %d for LCHAN %d, NB_Tb %d,MEas_Qdepth %d, on Idx %d (TO BE RECEIVED In frame %d), Nb_ul_idx(%d,%d,%d)\n",
      NODE_ID[Mod_id],
      Mac_rlc_xface->frame);
#endif
}

/********************************************************************************************************************/
void nodeb_generate_bcch(u8 Mod_id){
  /********************************************************************************************************************/

  //  if(CH_mac_inst[Mod_id].Bcch_lchan.Active==0)
  //    return;
      
  u16 i=0,j=0,k=0;
  unsigned char idx,Size=0;
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  //  LCHAN_INFO_DIL_TABLE_ENTRY *Lchan_entry_dil;
  MACPHY_DATA_REQ  *Macphy_data_req;
  unsigned short diff = 0;
  
  // Compute LCHAN throughput statistics on 128 TTI intervals
  if((Mac_rlc_xface->frame%128)==0) {
    
    for(i=0;i<=NB_CNX_CH;i++){
      if (CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1) {
	
	diff = CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.NB_TX - CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.NB_TX_LAST;
	CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.NB_TX_LAST = CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.NB_TX;
	
	CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.output_rate = (8*diff*CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Lchan_desc[1].transport_block_size)>>7;

      }
      for(j=0;j<NB_RAB_MAX;j++){
	if (CH_mac_inst[Mod_id].Dtch_lchan[j][i].Active==1) {
	  diff = CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_TX - CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_TX_LAST;
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_TX_LAST = CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_TX;
	  
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.output_rate = (8*diff*CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_desc[1].transport_block_size)>>7;
	}
      }
    }
  }
  
  
  idx=(Mac_rlc_xface->frame+1)%2;

  /// BCCH
  
  if (Is_rrc_registered == 1) {
#ifdef DEBUG_MAC_BCCH
    msg("[MAC eNB] Sending request for BCCH to RRC\n");
#endif
    Size=Rrc_xface->mac_rrc_data_req(Mod_id,
				     CH_mac_inst[Mod_id].Bcch_lchan.Lchan_info.Lchan_id.Index,
				     0,
				     &CH_mac_inst[Mod_id].BCCH_pdu.Bcch_payload[0],
				     0);
  }
  else
    Size=DUMMY_BCCH_SIZE_BYTES;
#ifdef DEBUG_MAC_BCCH  
  msg("[MAC eNB] Got BCCH from RRC of size %d (Header %d)\n",Size,
      CH_BCCH_HEADER_SIZE);

#endif
  CH_mac_inst[Mod_id].BCCH_pdu.Num_bytes_bcch=Size; 
  
  CH_mac_inst[Mod_id].Bcch_lchan.Lchan_info.Lchan_status_tx=MAC_TX_DONE;

  memcpy(&CH_mac_inst[Mod_id].DCI_pdu.dci_alloc[0].dci_pdu[0],&BCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
  CH_mac_inst[Mod_id].DCI_pdu.Num_common_dci = 1;
  CH_mac_inst[Mod_id].DCI_pdu.dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
  CH_mac_inst[Mod_id].DCI_pdu.dci_alloc[0].L          = 3;
  CH_mac_inst[Mod_id].DCI_pdu.dci_alloc[0].rnti       = SI_RNTI;
#ifdef DEBUG_MAC_CCCH    
  msg("[MAC][eNb] Frame %d: Generated BCCH DCI, format 1A\n",mac_xface->frame);
#endif
  // Copy payload
    // generate_dlsch
}


/********************************************************************************************************************/
void nodeb_generate_ccch(u8 Mod_id){
  /********************************************************************************************************************/

  //  if(CH_mac_inst[Mod_id].Bcch_lchan.Active==0)
  //    return;
      
  u16 i=0,j=0,k=0;
  unsigned char idx,Size=0;
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  //  LCHAN_INFO_DIL_TABLE_ENTRY *Lchan_entry_dil;
  MACPHY_DATA_REQ  *Macphy_data_req;
  unsigned short diff = 0;
  
  if (Is_rrc_registered == 1) {
#ifdef DEBUG_MAC_CCCH
    msg("[MAC eNB] Sending request for CCCH to RRC\n");
#endif
    Size=Rrc_xface->mac_rrc_data_req(Mod_id,
				     CH_mac_inst[Mod_id].Ccch_lchan.Lchan_info.Lchan_id.Index,
				     1,
				     &CH_mac_inst[Mod_id].CCCH_pdu.Ccch_payload[0],
				     0);
  }
  else
    Size = DUMMY_CCCH_SIZE_BYTES;
  
  //  CH_mac_inst[Mod_id].CCCH_pdu.Num_bytes_ccch=Size;
  CH_mac_inst[Mod_id].Ccch_lchan.Lchan_info.Lchan_status_tx=MAC_TX_DONE;
  
#ifdef DEBUG_MAC_CCCH
  msg("[MAC]NODE B: generate CCCH with %d bytes (header %d) TBS = % d\n"
      ,Size,CH_CCCH_HEADER_SIZE,dlsch_tbs25[1][2]/8);
#endif //DEBUG_MAC_CCCH

  // pass to PHY here

  if (Size>CH_CCCH_HEADER_SIZE) {
    memcpy(&CH_mac_inst[Mod_id].DCI_pdu.dci_alloc[0].dci_pdu[0],&CCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
    CH_mac_inst[Mod_id].DCI_pdu.Num_common_dci = 1;
    CH_mac_inst[Mod_id].DCI_pdu.dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
    CH_mac_inst[Mod_id].DCI_pdu.dci_alloc[0].L          = 3;
    CH_mac_inst[Mod_id].DCI_pdu.dci_alloc[0].rnti       = SI_RNTI;
#ifdef DEBUG_MAC_CCCH    
    msg("[MAC][eNb] Frame %d: Generated CCCH DCI, format 1A\n",mac_xface->frame);
#endif
    // Copy payload
    // generate_dlsch

  }

}

void nodeb_generate_dci(unsigned char Mod_id) {

  u16 i=0,j=0,k=0;
  unsigned char idx,Size=0;
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  //  LCHAN_INFO_DIL_TABLE_ENTRY *Lchan_entry_dil;
  MACPHY_DATA_REQ  *Macphy_data_req;
  unsigned short diff = 0;
    
  NDLS=0;
  NULS=0;
  
  //Fill DCCH/DTCH  DL/UL MAPING
  for(i=0;i<=NB_CNX_CH;i++){
    if((CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1) && 
       (CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Lchan_status_tx == MAC_SCHED_TX)){
      Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[i];
      ch_fill_dl_map(Mod_id,Lchan_entry);
    }
    if((CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1) && 
       (CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Lchan_status_rx == MAC_SCHED_RX_READY)){
      Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[i];
      ch_fill_ul_map(Mod_id,Lchan_entry);
      CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Lchan_status_rx = MAC_SCHED_RX_OK;
    }
    for(j=0;j<NB_RAB_MAX;j++){
      if ((CH_mac_inst[Mod_id].Dtch_lchan[j][i].Active==1) && 
	  (CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_status_tx == MAC_SCHED_TX)){
	Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[j][i];
	ch_fill_dl_map(Mod_id,Lchan_entry);
      }
      if  ((CH_mac_inst[Mod_id].Dtch_lchan[j][i].Active==1) && 
	   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_status_rx == MAC_SCHED_RX_READY){
	Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[j][i];
	ch_fill_ul_map(Mod_id,Lchan_entry);
	CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_status_rx = MAC_SCHED_RX_OK;
      }
      /*
      for(k=0;k<(NB_CNX_CH-1);k++){
	if ((CH_mac_inst[Mod_id].Dtch_dil_lchan[j][i][k].Active==1) && 
	    (CH_mac_inst[Mod_id].Dtch_dil_lchan[j][i][k].Lchan_info_dil.Lchan_status == MAC_SCHED_TX)){
	  Lchan_entry_dil=&CH_mac_inst[Mod_id].Dtch_dil_lchan[j][i][k];
	  ch_fill_dil_map(Mod_id,Lchan_entry_dil);
	}
      }
      */
    }
  }

  /*  
  if ((Macphy_data_req=new_macphy_data_req(Mod_id))==NULL){
    msg("FATAL: GENERATE CHBCH: NO MORE DATA REQ\n");
    mac_xface->macphy_exit("");
    return;
  }
  
  Macphy_data_req->Direction=TX;
  Macphy_data_req->Pdu_type=DCI;
  //  Macphy_data_req->Lchan_id.Index=65535;//(NODE_ID[Mod_id] << RAB_SHIFT2 );
  Macphy_data_req->CH_index=(NODE_ID[Mod_id] %2 );
  Macphy_data_req->Dir.Req_tx.Pdu.dci_pdu=&CH_mac_inst[Mod_id].DCI_pdu;
  //  Macphy_data_req->Phy_resources = &CH_mac_inst[Mod_id].Bcch_lchan.Lchan_info.Phy_resources_tx;
  */

}


/********************************************************************************************************************/
void nodeb_get_sach(u8 Mod_id){
  /********************************************************************************************************************/

  unsigned char i,idx=(Mac_rlc_xface->frame)%3;
  MACPHY_DATA_REQ *Macphy_data_req;// *Macphy_data_req_sch;   
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  unsigned short Idx2,In_idx,Index; 
  unsigned int bitmap,bitmap_cnt;

  for(i=0;i<CH_mac_inst[Mod_id].Nb_rx_sched[idx];i++){
    Index                                = CH_mac_inst[Mod_id].Rx_sched[idx][i].Lchan_id.Index;
    Idx2 = ( ( Index & RAB_OFFSET2 ) >> RAB_SHIFT2 );
    In_idx = (Index & RAB_OFFSET);
    if(In_idx == DCCH){
      Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[Idx2];
    }
    else{	
      Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Idx2];
    }
    
    if(Lchan_entry->Active==0){//USER HAS BEEN Disconnected between
      continue;
    }
    if(Lchan_entry->Lchan_info.Nb_sched_tb_ul == 0) 
      continue;
    
    if((Macphy_data_req=new_macphy_data_req(Mod_id))==NULL) return;
    
    Macphy_data_req->Direction=RX;
    
    //    Macphy_data_req->Lchan_id.Index      = CH_mac_inst[Mod_id].Rx_sched[idx][i].Lchan_id.Index;
    //    Index                                = Macphy_data_req->Lchan_id.Index;
    
    //Lchan_entry->Lchan_info.Phy_resources_rx.Time_alloc = CH_mac_inst[Mod_id].Rx_sched[idx][i].Phy_resources.Time_alloc;
    //Lchan_entry->Lchan_info.Phy_resources_rx.Freq_alloc = CH_mac_inst[Mod_id].Rx_sched[idx][i].Phy_resources.Freq_alloc;
    //Lchan_entry->Lchan_info.Phy_resources_rx.Coding_fmt = CH_mac_inst[Mod_id].Rx_sched[idx][i].Phy_resources.Coding_fmt;
    //    Macphy_data_req->num_tb                             = CH_mac_inst[Mod_id].Rx_sched[idx][i].Nb_tb;
    
    //    Lchan_entry->Lchan_info.Phy_resources_rx_sched.Time_alloc=0;
    //    Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc=0;
    
    //    Macphy_data_req->Phy_resources       = &Lchan_entry->Lchan_info.Phy_resources_rx;
    /*  
    if( ((Macphy_data_req->Lchan_id.Index & RAB_OFFSET1) >> RAB_SHIFT1 ) == 0 ){
      Macphy_data_req->Pdu_type            = ULSCH;
      //      Macphy_data_req->format_flag         = 1;
      Macphy_data_req->CH_index = NODE_ID[Mod_id]%2;
      //      Lchan_entry->Lchan_info.Nb_sched_tb_ul-=Macphy_data_req->num_tb;
      
#ifdef DEBUG_MAC_CH_RX
      msg("[MAC][NODEB %d] TTI %d: Programming ULSCH for LCHAN %d\n",
	  NODE_ID[Mod_id],
	  Mac_rlc_xface->frame,
	  Index);

      
#endif //DEBUG_MAC_CH_RX
    */

      //      Macphy_data_req->tb_size_bytes = Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size;  
      // Fill active process map (even without HARQ!)
      //      bitmap = 0;
      //      bitmap_cnt = Macphy_data_req->num_tb;
      //      while (bitmap_cnt>0) {
      //	bitmap = (bitmap<<1) + 1;
      //	bitmap_cnt--;
      //      }
      
      //      Macphy_data_req->Dir.Req_rx.Active_process_map = bitmap;
      //    }
    
      //    Macphy_data_req->Dir.Req_rx.Meas.UL_meas = &CH_mac_inst[Mod_id].UL_meas; 
    /*
#ifdef DEBUG_MAC_CH_RX    
    //if(In_idx == DTCH){
    msg("_______________________________________________________________________________________\n");
    msg("[OPENAIR][MAC] TTI %d Inst %d: GET_SACH for LCHAN_ID %d, Idx %d\n",
	Mac_rlc_xface->frame,
	  Mod_id,
	Macphy_data_req->Lchan_id.Index,
	//	Macphy_data_req->num_tb,
	//	Macphy_data_req->Phy_resources->Freq_alloc,
	idx);
#endif //DEBUG_MAC_CH_RX
    */
    Lchan_entry->Lchan_info.Lchan_status_rx=LCHAN_IDLE;
  }
  
  CH_mac_inst[Mod_id].Nb_rx_sched[idx]=0;
}

/******************************************************************************************************************
void nodeb_decode_sch(u8 Mod_id, UL_MEAS* UL_meas, u16 Idx2){

 
  unsigned char i;
  if(CH_mac_inst[Mod_id].Def_meas[Idx2].Active==1){
    //msg("[MAC][NodeB %d]Frame %d: GOT UL_SCH from UE %d\n",NODE_ID[Mod_id],Mac_rlc_xface->frame,Idx2);
    for(i=0;i<NUMBER_OF_MEASUREMENT_SUBBANDS;i++){
      CH_mac_inst[Mod_id].Def_meas[Idx2].Sinr_meas[0][i]=
	((CH_mac_inst[Mod_id].Def_meas[Idx2].Sinr_meas[0][i]*CH_mac_inst[Mod_id].Def_meas[Idx2].Forg_fact)
	 + (UL_meas->Sub_band_sinr[i]*(10-CH_mac_inst[Mod_id].Def_meas[Idx2].Forg_fact)))/10;         
    }
  }
}

*/

/*******************************************************************************************************************/
void nodeb_decode_ulsch(u8 Mod_id,
			ULSCH_PDU* ULSCH_pdu,unsigned short rnti) {

  /********************************************************************************************************************/
  
  u16 Rx_size,j,One=0x01;
  u8 Nb_tb,Nb_tb_err=0,Nb_fg=0,i;	
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  
  unsigned short User_index,Lchan_index;
  
  User_index = 0;//( ( Lchan_id_index & RAB_OFFSET2 ) >> RAB_SHIFT2 );
  Lchan_index = DTCH_BD+1;//(Lchan_id_index & RAB_OFFSET);
  if(Lchan_index == DCCH){
    Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[User_index];
  }
  else{	
    Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[Lchan_index-DTCH_BD][User_index];
  }
  /*
  if(Lchan_entry){
    if (crc_status[0]== -SACCH_ERROR) {
      if (Lchan_index == DCCH) {
	CH_mac_inst[Mod_id].Dcch_lchan[User_index].Lchan_info.NB_RX_ERRORS++;
	CH_mac_inst[Mod_id].Dcch_lchan[User_index].Lchan_info.NB_RX_SACCH_ERRORS++;
      }
      else {
	CH_mac_inst[Mod_id].Dtch_lchan[Lchan_index-DTCH_BD][User_index].Lchan_info.NB_RX_ERRORS++;
	CH_mac_inst[Mod_id].Dtch_lchan[Lchan_index-DTCH_BD][User_index].Lchan_info.NB_RX_SACCH_ERRORS++;
      }
      return;
    }
    
    else if (crc_status[0]== -SACH_MISSING) {
      if (Lchan_index == DCCH) {
	CH_mac_inst[Mod_id].Dcch_lchan[User_index].Lchan_info.NB_RX_ERRORS++;
	CH_mac_inst[Mod_id].Dcch_lchan[User_index].Lchan_info.NB_RX_SACH_MISSING++;
      }
      else {
	CH_mac_inst[Mod_id].Dtch_lchan[Lchan_index-DTCH_BD][User_index].Lchan_info.NB_RX_ERRORS++;
	CH_mac_inst[Mod_id].Dtch_lchan[Lchan_index-DTCH_BD][User_index].Lchan_info.NB_RX_SACH_MISSING++;
      }
      return;
    }
  */
    /*    
    conv_alloc_to_tb2(1,  // ULSCH calculation
		      Lchan_entry->Lchan_info.Phy_resources_rx.Time_alloc,
		      Lchan_entry->Lchan_info.Phy_resources_rx.Freq_alloc,
		      Lchan_entry->Lchan_info.Target_spec_eff_rx,
		      Lchan_entry->Lchan_info.Dual_stream_flag_rx,
		      99,
		      &Lchan_entry->Lchan_info.Phy_resources_rx.Coding_fmt,
		      &Nb_tb,
		      Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size); 
    

    if(Nb_tb==0){
      msg("CH %d: decode sach %d, NB_TB=0, Freq_alloc %x\n",Mod_id,		      
	  Lchan_entry->Lchan_info.Lchan_id.Index,
	  Lchan_entry->Lchan_info.Phy_resources_rx.Freq_alloc);
      mac_xface->macphy_exit("");
      
    }  
    
    // Compute total size of allocation in bytes 
    Rx_size = Nb_tb * Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size; 

    // Check if any TBs are in error, if at least one, return with error (to be changed later)
    */
  /*
    for (i=0;i<Nb_tb;i++)
      if (crc_status[i] == (-SACH_ERROR)) {

#ifdef DEBUG_MAC_CH_RX		
	msg("[OPENAIR][MAC] Frame %d: LCHAN %d UL SACH in error (Num_tb %d, TB %d, status %d)\n",
	    Mac_rlc_xface->frame,
	    Lchan_entry->Lchan_info.Lchan_id.Index,
	    Nb_tb,
	    i,
	    crc_status[i]);
#endif //DEBUG_MAC_CH_RX		
	
	Nb_tb_err++;
	if (Lchan_index == DCCH) {
	  CH_mac_inst[Mod_id].Dcch_lchan[User_index].Lchan_info.NB_RX_ERRORS++;
	  CH_mac_inst[Mod_id].Dcch_lchan[User_index].Lchan_info.NB_RX_SACH_ERRORS++;
	}
	else {
	  CH_mac_inst[Mod_id].Dtch_lchan[Lchan_index-DTCH_BD][User_index].Lchan_info.NB_RX_ERRORS++;
	  CH_mac_inst[Mod_id].Dtch_lchan[Lchan_index-DTCH_BD][User_index].Lchan_info.NB_RX_SACH_ERRORS++;
	  CH_mac_inst[Mod_id].Dtch_lchan[Lchan_index-DTCH_BD][User_index].Lchan_info.NB_RX_ERRORS_TB[Nb_tb]++;
	}	    
      }
  */

    if (Lchan_index != DCCH)
      CH_mac_inst[Mod_id].Dtch_lchan[Lchan_index-DTCH_BD][User_index].Lchan_info.NB_RX_TB[Nb_tb]+=Nb_tb;
    else
      CH_mac_inst[Mod_id].Dcch_lchan[User_index].Lchan_info.NB_RX_TB[Nb_tb]++;
    
    // Copy PHY decoded packets to 
    memcpy(&Lchan_entry->Lchan_info.Current_payload_rx[0],
	   &ULSCH_pdu->payload[0],
	   Rx_size);
    /*    
    for(i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++)
      if(( (One << i) & Lchan_entry->Lchan_info.Phy_resources_rx.Freq_alloc )!=0)
	Nb_fg++;
    */

    Lchan_entry->Lchan_info.NB_RX+=Nb_tb;
 
    if (Is_rrc_registered == 1) {
     
#ifdef DEBUG_MAC_CH_RX
      msg("____________________________________MAC  NODEB  RXXXXXXXXXXXXXXXXXXXXXX_____________________________________________\n");
      msg("[OPENAIR][MAC] TTI %d, INST %d NODE_B %d DECODE UL SACH on LCHAN %d, DATA_IND_TO_RLC , Rx_size %d,Freq %X, NB_tb %d,Qdepth %d\n",
	    Mac_rlc_xface->frame,
	  Mod_id,
	  NODE_ID[Mod_id],
	  Lchan_entry->Lchan_info.Lchan_id.Index, 
	  Rx_size,
	  Lchan_entry->Lchan_info.Phy_resources_rx.Freq_alloc,
	  Nb_tb,
	  Lchan_entry->Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth);
#endif //DEBUG_MAC_CH_RX 
      
	Mac_rlc_xface->mac_rlc_data_ind(Mod_id,
					Lchan_entry->Lchan_info.Lchan_id.Index,
					&Lchan_entry->Lchan_info.Current_payload_rx[0],
					Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size,
					Nb_tb,
					NULL);//(unsigned int*)crc_status);
	
    }
    
    
    Lchan_entry->Lchan_info.Nb_rx_last_tti=Nb_tb*Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size*8;
}



/******************************************************************************************************************
void nodeb_get_rach(u8 Mod_id,u8 nb_rach){

  
  u8 i;
  
  MACPHY_DATA_REQ *Macphy_data_req;
  for( i=0 ; i < 1 ; i++ ){
    if( (Macphy_data_req = new_macphy_data_req(Mod_id))==NULL ) return;
    Macphy_data_req->Direction                        = RX;
    Macphy_data_req->Pdu_type                         = ULSCH;
    Macphy_data_req->Lchan_id.Index                   = CH_mac_inst[Mod_id].Ccch_lchan.Lchan_info.Lchan_id.Index;
    Macphy_data_req->Phy_resources=&CH_mac_inst[Mod_id].Ccch_lchan.Lchan_info.Phy_resources_rx;
    Macphy_data_req->Phy_resources->Time_alloc=RACH_TIME_ALLOC;
    Macphy_data_req->Phy_resources->Freq_alloc=RACH0_FREQ_ALLOC;
    Macphy_data_req->Phy_resources->Coding_fmt=0;
    Macphy_data_req->Dir.Req_rx.Pdu.Rach_pdu = &CH_mac_inst[Mod_id].RX_rach_pdu;

#ifdef PHY_EMUL
    Macphy_data_req->Dir.Req_rx.Pdu.Rach_pdu->Pdu_size = 12;//20*conv_alloc_to_bytes(Macphy_data_req->Phy_resources->Time_alloc,
#endif

    Macphy_data_req->num_tb = 1;
      Macphy_data_req->CH_index = (NODE_ID[Mod_id]%2);
      Macphy_data_req->tb_size_bytes = 12;
      
      Macphy_data_req->Dir.Req_rx.Active_process_map = 1;
  }
}
*/

/********************************************************************************************************************/
void nodeb_decode_rach(u8 Mod_id,ULSCH_PDU *Rach_pdu){ 
  /********************************************************************************************************************/
  unsigned char User_index,i=2,j;
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  if(Rach_pdu->payload[0]!=MAC_RACH_BW_REQ){
    memcpy(&CH_mac_inst[Mod_id].Ccch_lchan.Lchan_info.Current_payload_rx[0],
	   &Rach_pdu->payload[1],
	   11);
    
    if (Is_rrc_registered == 1)
      Rrc_xface->mac_rrc_data_ind(Mod_id,CH_mac_inst[Mod_id].Ccch_lchan.Lchan_info.Lchan_id.Index,
				  &CH_mac_inst[Mod_id].Ccch_lchan.Lchan_info.Current_payload_rx[0],0);
  
  }

  //BW REQUEST
  else{
    User_index=Rach_pdu->payload[1];
    // msg("CH: BW REQ from USER INDEX %d\n",User_index);
    if(Rach_pdu->payload[i]!=0){
      
      Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[User_index];
      if((Lchan_entry->Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth==0)
	 && (Lchan_entry->Lchan_info.Nb_sched_tb_ul==0) ){
	Lchan_entry->Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth=Rach_pdu->payload[i];
	Lchan_entry->Lchan_info.Lchan_status_rx = MAC_SCHED_RX_REQ;        
	Lchan_entry->Lchan_info.NB_BW_REQ_RX++;        
	//msg("BW request for DCCH %d, NB_TB %d\n",Lchan_entry->Lchan_info.Lchan_id.Index,
	//    Rach_pdu->payload[i]);
      }
    }
    i++;
    for(j=0;j<NB_RAB_MAX;j++){
      if(Rach_pdu->payload[i]!=0){
	Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[j][User_index];
	if((Lchan_entry->Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth==0)
	   && (Lchan_entry->Lchan_info.Nb_sched_tb_ul==0) ){
	  Lchan_entry->Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth=Rach_pdu->payload[i];
	  Lchan_entry->Lchan_info.NB_BW_REQ_RX++;        
	  //	  Lchan_entry->Lchan_info.Lchan_status_rx = MAC_SCHED_RX_REQ;        
	  // msg("BW request for DTCH %d, NB_TB %d\n",Lchan_entry->Lchan_info.Lchan_id.Index,
	  //   Rach_pdu->payload[i]);
	}
      }
      i++;
    }
  }
}
#define FFact 1

/****************************************************************************************************************/
void nodeb_generate_dlsch(u8 Mod_id){
  /****************************************************************************************************************/

  u8 i;
  unsigned char Nb_tb=0; 
  unsigned int G_size=0; 
  LCHAN_INFO_TABLE_ENTRY* Lchan_entry;
  MACPHY_DATA_REQ *Macphy_data_req;
  mac_rlc_status_resp_t rlc_status; 
  unsigned short Lchan_index,User_index,Tb_size;
  unsigned int bitmap,bitmap_cnt;

  for( i=0;i<CH_mac_inst[Mod_id].Num_dlsch;i++ ){

    //    Lchan_index = CH_mac_inst[Mod_id].DL_sacch_pdu[i].Lchan_id.Index & RAB_OFFSET;
    //    User_index = (CH_mac_inst[Mod_id].DL_sacch_pdu[i].Lchan_id.Index & RAB_OFFSET2) >> RAB_SHIFT2;
    
    Lchan_index = 1+DTCH_BD;
    User_index = 0;

    if (Lchan_index == DCCH)
      Lchan_entry  = &CH_mac_inst[Mod_id].Dcch_lchan[User_index];
    else
      Lchan_entry  = &CH_mac_inst[Mod_id].Dtch_lchan[Lchan_index-DTCH_BD][User_index];

    Nb_tb = Lchan_entry->Lchan_info.Nb_sched_tb_dl;
    Lchan_entry->Lchan_info.Nb_tx_last_tti=Nb_tb*Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size*8;

    if(Nb_tb==0 || Nb_tb > Lchan_entry->Lchan_info.W_idx){
      msg("[MAC][NODEB %d] ERROR:Nb_tb  NULL: Frame %d: Filling DL MACPHY_DATA_REQ for LCHAN %d, Freq %X, Nb_tb %d, NB_DL_MAP %d, %d\n",
	  NODE_ID[Mod_id],
	  Mac_rlc_xface->frame,
	  Lchan_entry->Lchan_info.Lchan_id.Index,
	  Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc,
	  Nb_tb,
	  CH_mac_inst[0].Num_dlsch,
	  CH_mac_inst[1].Num_dlsch);
      mac_xface->macphy_exit("");
      return;      
    }

    Lchan_entry->Lchan_info.NB_TX+=Nb_tb;
    Lchan_entry->Lchan_info.NB_TX_TB[Nb_tb]++;
    
    if((Macphy_data_req = new_macphy_data_req(Mod_id))==NULL) 
      msg("nodeb_control_plane_procedures.c: nodeb_generate_dlsch: new_macphy_data_req fails\n",mac_xface->frame);
    return;

    Macphy_data_req->Direction = TX;
    //    Macphy_data_req->format_flag = 0;
    Macphy_data_req->Pdu_type = DLSCH;
    //    Macphy_data_req->Lchan_id.Index=Lchan_entry->Lchan_info.Lchan_id.Index;
    //    Macphy_data_req->Phy_resources = &Lchan_entry->Lchan_info.Phy_resources_tx;
    //    Macphy_data_req->CH_index = NODE_ID[Mod_id]%2;
    //   Macphy_data_req->num_tb        = Nb_tb;
    //    Macphy_data_req->tb_size_bytes = Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size;
    //   Tb_size= Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size;
    //    bitmap = 0;
    //      bitmap_cnt = Nb_tb;
    //      while (bitmap_cnt>0) {
    //	bitmap = (bitmap<<1) + 1;
    //	bitmap_cnt--;
    //      }
      // Fill HARQ info
    //      Macphy_data_req->Dir.Req_tx.Active_process_map = bitmap;
    //      Macphy_data_req->Dir.Req_tx.New_process_map    = bitmap;
      
      memcpy(&Macphy_data_req->Dir.Req_tx.Pdu.DLSCH_pdu.payload[0],
	     &Lchan_entry->Lchan_info.Current_payload_tx[0],
	     Nb_tb*Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size);
      
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
  	Lchan_entry->Lchan_info.Nb_sched_tb_dl-=Nb_tb;
      
#ifdef PHY_EMUL  
      Macphy_data_req->Dir.Req_tx.Pdu.DL_sach_pdu.Pdu_size=Nb_tb*Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size;
#endif //  
      Lchan_entry->Lchan_info.Lchan_status_tx=MAC_TX_OK;
  }
}

#define MIN_SCORE (1<<30)
#define MAX_SCORE -MIN_SCORE

/****************************************************************************************************************
void nodeb_scheduler(u8 Mod_id){


  unsigned short i,j,k,Winner=1;
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry=(LCHAN_INFO_TABLE_ENTRY *)NULL;
  MAC_MEAS_T *dtch_meas;
  mac_rlc_status_resp_t rlc_status;
  // unsigned char NB_sched_tb = NB_TB_MAX;//(16 OFDM Symbols, one Frequency group=160 bits (including one octet for pilot symbol))
  unsigned char Max_sched;
  unsigned short Last_freq_map=0xffff,New_freq_map=0,Curr_freq,Cand_freq;
  int Min_score,Max_score,Score;
  unsigned char Active_dcch_user[NB_CNX_CH+1],Nb_active_dcch=0,Wait_for_better_fg;
  int bytes_tmp = 0;
  unsigned short UL_FREQ_MAP = (0x0001); 
  unsigned short DL_FREQ_MAP = (0x0001); 
  unsigned char User_alloc_map[MAX_NB_SCHED];//, Rab_index; 
  unsigned char Sch_ok=0,s;
  int BROADCAST_SCORE=-10000000,DCCH_SCORE=-2000000000;

 #ifdef DEBUG_MAC_SCHEDULING
  msg("____________________________NODEB SCHEDULER @ Frame %d_____________________________\n",Mac_rlc_xface->frame);
#endif
  
  
  
  // Preprocess measurement information
  // by sorting the frequency response for active user connection (i.e. one that is connected, not just in this TTI) 
 
  for(i=1;i<(NB_CNX_CH+1);i++){
    if(CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1){
      memcpy(&CH_mac_inst[Mod_id].Sinr_sorted_table[i][0],CH_mac_inst[Mod_id].Def_meas[i].Sinr_meas[0],NUMBER_OF_FREQUENCY_GROUPS);
      // initialized SINR indeces prior to sorting
      memcpy(&CH_mac_inst[Mod_id].Sinr_sorted_index[i][0],&Sorted_index_table[0],MAX_NB_SCHED);
      quicksort(&CH_mac_inst[Mod_id].Sinr_sorted_table[i][0],
		&CH_mac_inst[Mod_id].Sinr_sorted_index[i][0],
		0,
		MAX_NB_SCHED-1);//MAX_NB_SCHED-1);
    }
  }
  
  LCHAN_INFO_TABLE_ENTRY *Looser_lchan_entry=NULL;
  unsigned short sf,Worst_freq,Temp_freq_alloc,kk,G_size,Tb_size;
  unsigned int Total_tx_rate,Total_rx_rate;
  unsigned char Num_tb,Num_tb_full;
  int tmp,tmp1,tmp2,tmp3;

  
  // Downlink Scheduling
  
  // Loop over all nodes connected to CH and their RABs
  // to collect Queuing information
  
  CH_mac_inst[Mod_id].Nb_sched=0;
  for(i=1;i<(NB_CNX_CH+1);i++){
    if(CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1){
      Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[i];
      Tb_size=Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size;
      //check RLC backlog for active LChan
      rlc_status=mac_rlc_status_ind(Mod_id,Lchan_entry->Lchan_info.Lchan_id.Index,
				    Tb_size,
				    NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx);
      
      G_size=Mac_rlc_xface->mac_rlc_data_req(Mod_id,
					     Lchan_entry->Lchan_info.Lchan_id.Index,
					     &Lchan_entry->Lchan_info.Current_payload_tx[Lchan_entry->Lchan_info.W_idx*Tb_size]);
      
      Lchan_entry->Lchan_info.W_idx+=G_size/Tb_size;
      if(Lchan_entry->Lchan_info.W_idx > NB_TB_BUFF_MAX){
	msg("[MAC][NODEB][SCHED]: RLC RETURNS TOO MUCH TBs !!!!\n");
	mac_xface->macphy_exit("");
      }
      
      Lchan_entry->Lchan_info.Qdepth=Lchan_entry->Lchan_info.W_idx;
      Lchan_entry->Lchan_info.Qdepth_temp=Lchan_entry->Lchan_info.Qdepth;
      Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc=0;
      Lchan_entry->Lchan_info.Phy_resources_tx.Coding_fmt=0;
      Lchan_entry->Lchan_info.Nb_sched_tb_dl=0;
      Lchan_entry->Lchan_info.Nb_sched_tb_dl_temp=0;
      Lchan_entry->Lchan_info.Sched_flag=0;
      Lchan_entry->Lchan_info.Target_spec_eff_tx = 3;
      Lchan_entry->Lchan_info.Dual_stream_flag_tx = 0;

#ifdef DEBUG_MAC_SCHEDULING
      msg("[MAC][NODEB] TTI %d: Scheduler RLC STATUS IND on Lchan %d(%d,%d) RETURN %d Bytes, TB_SIZE %d, Qdepth %d \n",
	  Mac_rlc_xface->frame,
	  Lchan_entry->Lchan_info.Lchan_id.Index,i,j,
	  rlc_status.bytes_in_buffer,
	  Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size,
	  Lchan_entry->Lchan_info.Qdepth);
#endif
    }
  }
  
  for(i=0;i<(NB_CNX_CH+1);i++){
    for(j=0;j<NB_RAB_MAX;j++)      
      if(CH_mac_inst[Mod_id].Dtch_lchan[j][i].Active==1){
	// if LCHAN is active
        Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[j][i];
	Tb_size=Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size;
	
	//check RLC backlog for active LChan
	rlc_status=mac_rlc_status_ind(Mod_id,Lchan_entry->Lchan_info.Lchan_id.Index,
				      Tb_size,
				      NB_TB_BUFF_MAX-Lchan_entry->Lchan_info.W_idx);
	
	G_size=Mac_rlc_xface->mac_rlc_data_req(Mod_id,
					       Lchan_entry->Lchan_info.Lchan_id.Index,
					       &Lchan_entry->Lchan_info.Current_payload_tx[Lchan_entry->Lchan_info.W_idx*Tb_size]);
	
	
	Lchan_entry->Lchan_info.Arrival_rate=(Lchan_entry->Lchan_info.Arrival_rate*(SCHED_LONG_MAW-1)+(G_size*8))/SCHED_LONG_MAW;//bits/TTI(G_size*8);//bits/TTI
	
	Lchan_entry->Lchan_info.Tx_rate=(Lchan_entry->Lchan_info.Tx_rate*(SCHED_LONG_MAW-1)+Lchan_entry->Lchan_info.Nb_tx_last_tti)/SCHED_LONG_MAW;//bits/TTI
	Lchan_entry->Lchan_info.Tx_rate_temp=Lchan_entry->Lchan_info.Tx_rate;
	Total_tx_rate+=Lchan_entry->Lchan_info.Tx_rate;
	
#ifdef DEBUG_SCHED
	if((Mac_rlc_xface->frame %200==0) )
	  msg("[MAC][NODEB %d]Scheduler: Frame %d: LCHAN %d Resulting Qdepth is %d\, tb_size %d, Arrival_rate %d(bits/tti), Service_rate %d(bits/ttii), Nb_tx_last_tti %d, Arr_bits %d \n",
	      Mod_id,
	      Mac_rlc_xface->frame,
	      Lchan_entry->Lchan_info.Lchan_id.Index,
	      Lchan_entry->Lchan_info.Qdepth,
	      Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size,
	      Lchan_entry->Lchan_info.Arrival_rate,
	      Lchan_entry->Lchan_info.Tx_rate,
	      Lchan_entry->Lchan_info.Nb_tx_last_tti,
	      G_size>>3);
#endif
	
	Lchan_entry->Lchan_info.W_idx+=G_size/Tb_size;
	Lchan_entry->Lchan_info.Nb_tx_last_tti=0;

	if(Lchan_entry->Lchan_info.W_idx > NB_TB_BUFF_MAX){
	  msg("[MAC][NODEB][SCHED] RLC RETURNS TOO MUCH TBs !!!!\n");
	  mac_xface->macphy_exit("");
	}
	
	Lchan_entry->Lchan_info.Qdepth=Lchan_entry->Lchan_info.W_idx;
	Lchan_entry->Lchan_info.Qdepth_temp=Lchan_entry->Lchan_info.Qdepth;
	Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc=0;
	Lchan_entry->Lchan_info.Phy_resources_tx.Coding_fmt=0;
	Lchan_entry->Lchan_info.Nb_sched_tb_dl=0;
	Lchan_entry->Lchan_info.Nb_sched_tb_dl_temp=0;
	Lchan_entry->Lchan_info.Sched_flag=0;
	Lchan_entry->Lchan_info.Target_spec_eff_tx = 1;  // Here compute adaptive spectral efficiency based on measurements
	Lchan_entry->Lchan_info.Dual_stream_flag_tx = 0;


      } // if active
  } // CX/RAB
  

  // Now go and do allocation
  
  k=0;
  Last_freq_map=0xffff;
  New_freq_map=0;
  Wait_for_better_fg=0;
  Min_score=MIN_SCORE;

  while(1){
    Curr_freq = ( DL_FREQ_MAP << k );
    
    if ( ((New_freq_map!=0xffff) &&  (Last_freq_map!=New_freq_map)) || (Wait_for_better_fg == 1) ){
      
#ifdef DEBUG_SCHED
      msg("New DL Round, Curr_freq %x, New_freq_map %x, last_freq_map %x, Jumping %d\n",
	  Curr_freq,New_freq_map,Last_freq_map,Wait_for_better_fg);
#endif
      if(!(Curr_freq & New_freq_map)){ //Current Frequency "k" is available

	Min_score=MIN_SCORE;

#ifdef DEBUG_SCHED
	msg("[NODE %d]TTI %d: New DL allocation round for freq %x, New_freq_map %x, last %x, wait %d\n",
	    Mod_id,
	    Mac_rlc_xface->frame,
	    Curr_freq,
	    New_freq_map,
	    Last_freq_map,
	    Wait_for_better_fg);
#endif
	
	Lchan_entry=NULL;
	Wait_for_better_fg=0;
	Last_freq_map=New_freq_map;
	
	// loop over all nodes connected to CH and their RBs and select the one who maximize the score function   
	  
	for(i=1;i<(NB_CNX_CH+1);i++){
	  if( (CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1) && (CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Qdepth_temp>0) ){
	    Score=DCCH_SCORE + CH_mac_inst[Mod_id].Def_meas[i].Sinr_meas[0][k];; 
	    if( (Score < Min_score)
		&& ( (CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Sched_flag==1 ) || (CH_mac_inst[Mod_id].Nb_sched < NB_DL_SCHED_MAX))){
	      
	      // keep the maximizing metric and corresponding LCHAN
	      Min_score=Score;
	      Winner=i;
	      Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[i];
	    }
	  }
	}

	//SCHED ALGO: Rate adaptive fair scheduling
	//OBJECTIVE FUNCTION:
	//max_over_i(i: active lchans): (Arrival_rate(i)-Service_rate(i))/Service_rate(i) * (Sum_i Service_rate(i)/ Service_rate(i))
	//In the fisrt term: the numerator is the gap between the mean requested rate and the actual meanservice rate, 
        //                     the denominator is a normalizing term
	//The second term is a fairness measure 
	// Arrival_rate and service rate are estimated online. 
	//The sched algo is of best effort type. Next: compare to PFS


	//In case of QoS constraints, e.g. max Delay D(i) for flow i
	 //OBJECTIVE FUNCTION:
	//min_over_i(i: active lchans): (D(i) - Backlog(i)/Service_rate(i))/D(i) * (Service_rate(i)/Sum_i Service_rate(i)) 
	//In the fisrt term: the numerator is the gap between the mean actual delay and the requested maximum delay, 
        //                     the denominator is a normalizing term (D(i))
	//The second term is a fairness measure 
	// Service rate is estimated online. 
	
	


	for(i=0;i<(NB_CNX_CH+1);i++){
	  for(j=0;j<NB_RAB_MAX;j++){
	    if(CH_mac_inst[Mod_id].Dtch_lchan[j][i].Active==1){
	      // if DTCH is active
	      if(CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Qdepth_temp > 0
		 &&((CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Sched_flag==1) || (CH_mac_inst[Mod_id].Nb_sched < NB_DL_SCHED_MAX))){
		
		tmp=
		  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Arrival_rate-CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Tx_rate_temp;

		  if(tmp >0)
		    tmp1=(1000*CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Tx_rate_temp)/tmp;
		  else if (tmp <0)
		    tmp1=-(1000*CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Tx_rate_temp)/tmp;		
		  else
		    tmp1=tmp;
		  
		  if(Total_tx_rate>0)
		    Score=(1000*tmp1*(CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Tx_rate_temp))/Total_tx_rate;
		  else
		    Score=1000*tmp1;
		  
		  
		  if( Score < Min_score){
		    
		  // keep the maximizing metric and corresponding LCHAN
#ifdef DEBUG_SCHED	      
		    msg("DTCH %d is DL active with Score %d (Qd_temp %d), Min_score %d, Arrival_rate %d, Service_rate %d,tmp %d, tmp1 %d, tmp2 %d\n",
			CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_id.Index,
			Score,
			CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Qdepth_temp,
			Min_score,
			CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Arrival_rate,
			CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Tx_rate,tmp,tmp1,tmp2);
#endif
		    
		  Min_score=Score;
		  Winner=i;
		  Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[j][i];
		    
		}
	      }
	    }
	  }
	}
	
	// if we've found a candidate to schedule
	
	if(Min_score!=MIN_SCORE){
	  
	  //check if the current frequency "k" is the best one not yet allocated for the winner
	  for(j=0;j<NUMBER_OF_FREQUENCY_GROUPS;j++){
	  
	    Cand_freq= CH_mac_inst[Mod_id].Sinr_sorted_index[Winner][j];
	    if( ((New_freq_map &  (DL_FREQ_MAP << Cand_freq)) ==0 ) && (Cand_freq !=k) &&
		(CH_mac_inst[Mod_id].Def_meas[Winner].Sinr_meas[0][Cand_freq] > CH_mac_inst[Mod_id].Def_meas[Winner].Sinr_meas[0][k]) ) {
	      //the winner has a better FG, Wait until he competes for it
	      Wait_for_better_fg=1;
	      k=Cand_freq;
	      
#ifdef DEBUG_SCHED
	      msg(" DL:TTI %d: USER %d, Lchan %d, jump from Freq %x to Freq %x (k=%d), Score =%d, Min_score=%d \n",
		  Mac_rlc_xface->frame,Winner,Lchan_entry->Lchan_info.Lchan_id.Index,Curr_freq,(DL_FREQ_MAP << Cand_freq),k,Score,Min_score);
#endif
	      break;
	    }
	  }
	    
	  if( ! Wait_for_better_fg ){//This is the best FG for the winner, allocate
	    Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc  = DL_TIME_ALLOC;
	    Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc +=Curr_freq;
	    if( Lchan_entry->Lchan_info.Sched_flag == 0 ){
	      Lchan_entry->Lchan_info.Sched_flag=1;
	      CH_mac_inst[Mod_id].Nb_sched++;
	    }
	    
	    conv_alloc_to_tb2(0,  // DLSCH calculation
			      Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc,
			      Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc,
			      Lchan_entry->Lchan_info.Target_spec_eff_tx,
			      Lchan_entry->Lchan_info.Dual_stream_flag_tx,
			      Lchan_entry->Lchan_info.Qdepth,
			      &Lchan_entry->Lchan_info.Phy_resources_tx.Coding_fmt,
			      &Num_tb,
			      Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size); 
	    
	    if( Lchan_entry->Lchan_info.Qdepth >= Num_tb ){
	      
	      Lchan_entry->Lchan_info.Qdepth_temp = Lchan_entry->Lchan_info.Qdepth-Num_tb;
	      New_freq_map+=Curr_freq;	    
	      Lchan_entry->Lchan_info.Lchan_status_tx = MAC_SCHED_TX;
	      Lchan_entry->Lchan_info.Nb_sched_tb_dl+=(Num_tb-Lchan_entry->Lchan_info.Nb_sched_tb_dl_temp);
	      Lchan_entry->Lchan_info.Tx_rate_temp=(Lchan_entry->Lchan_info.Tx_rate*(SCHED_SHORT_MAW-1)
						    +(Num_tb)
						    *Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size*8)/SCHED_SHORT_MAW;
	      //bits/TTI //16= MOVING AVERAGE WINDOW  	      
	      

	      Lchan_entry->Lchan_info.Nb_sched_tb_dl_temp = Num_tb;

#ifdef DEBUG_SCHED
	      msg("[MAC][NodeB %d] DL: TTI %d : Schedule Lchan %d for TX, Curr_freq %x Added To Freq_alloc %x, DL_FREQ_MAP %x, Remaining %d TB in Buffer, Nb_sched_tb_dl %d, Nb_sched_lchan %d\n",   
		  NODE_ID[Mod_id],
		  Mac_rlc_xface->frame,
		  Lchan_entry->Lchan_info.Lchan_id.Index, 
		  Curr_freq, 
		  Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc-Curr_freq,
		  New_freq_map,Lchan_entry->Lchan_info.Qdepth_temp,
		  Lchan_entry->Lchan_info.Nb_sched_tb_dl,
		  CH_mac_inst[Mod_id].Nb_sched);
	      
#endif //DEBUG_SCHED
	    
	    }
	    
	    else{
	      Lchan_entry->Lchan_info.Qdepth_temp = 0;
	      Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc-=Curr_freq;
	      Wait_for_better_fg=1;
	    }
	  }
	}	
      }
      
      if( Wait_for_better_fg == 0) 
	k=(k+1)%NUMBER_OF_FREQUENCY_GROUPS;

#ifdef DEBUG_SCHED
      msg("end of DL allocation round for freq %x, New_freq_map %x, last %x, wait %d\n",
	  Curr_freq,
	  New_freq_map,
	  Last_freq_map,
	  Wait_for_better_fg);
#endif
      
      
    }//last=new
    
  
    else if(!Wait_for_better_fg){//No new allocation, free useless  allocations
      
#ifdef DEBUG_SCHED
      msg("DL REallocation round for freq %x, New_freq_map %x, last %x, wait %d\n",
	  Curr_freq,
	  New_freq_map,
	  Last_freq_map,
	  Wait_for_better_fg);
#endif
      Min_score=MIN_SCORE;
      Looser_lchan_entry=NULL;
      
      for(i=0;i<(NB_CNX_CH+1);i++){
	for(kk=0;kk<NB_RAB_MAX;kk++)
	  if(CH_mac_inst[Mod_id].Dtch_lchan[kk][i].Active==1){
	    Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[kk][i];
	    if( Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc != 0 ){

	      conv_alloc_to_tb2(0,  
				Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc,
				Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc,
				Lchan_entry->Lchan_info.Target_spec_eff_tx,
				Lchan_entry->Lchan_info.Dual_stream_flag_tx,
				Lchan_entry->Lchan_info.Qdepth,
				&Lchan_entry->Lchan_info.Phy_resources_tx.Coding_fmt,
				&Num_tb_full,
				Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size); 
	      
	      Temp_freq_alloc=0;
	      
	      for(j=0;j<NUMBER_OF_FREQUENCY_GROUPS;j++){
		
		if( ( Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc & 
		      ( DL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][j] )) ){
		
		  Temp_freq_alloc += ( DL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][j] );
		  
		  conv_alloc_to_tb2(0,  
				    Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc,
				    Temp_freq_alloc,
				    Lchan_entry->Lchan_info.Target_spec_eff_tx,
				    Lchan_entry->Lchan_info.Dual_stream_flag_tx,
				    Lchan_entry->Lchan_info.Qdepth,   
				    &Lchan_entry->Lchan_info.Phy_resources_tx.Coding_fmt,
				    &Num_tb,
				    Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size); 

		  if( (Num_tb_full == Num_tb) || (Num_tb_full == 0) ){
		    if(Num_tb_full==0) {
		      j=-1;
		      //msg("\n***************************\n");
		    }
		    
		    for(sf=NUMBER_OF_FREQUENCY_GROUPS-1;sf>=(unsigned short)(j+1);sf--){
		    
		      if( ( Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc & 
			    ( DL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][sf] )) ){
			
			Score=CH_mac_inst[Mod_id].Sinr_sorted_table[i][sf];
			
			if(Score < Min_score){
			  Min_score=Score;
			  Worst_freq=(DL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][sf]);
			  Looser_lchan_entry=Lchan_entry;
			}

			break;//sf loop
		      }
		    }
		    break;//j Frequency loop
		  }
		}
	      }
	      
	    }
	  }
      }//end of users' DTCH lchan loop

      if(Min_score==MIN_SCORE){//No frequncy freed from DTCHs, look in DCCHs allocation

	for(i=1;i<(NB_CNX_CH+1);i++){
	  if(CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1){
	    Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[i];
	    if( Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc != 0 ){

	      conv_alloc_to_tb2(0,  
				Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc,
				Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc,
				Lchan_entry->Lchan_info.Target_spec_eff_tx,
				Lchan_entry->Lchan_info.Dual_stream_flag_tx,
				Lchan_entry->Lchan_info.Qdepth,
				&Lchan_entry->Lchan_info.Phy_resources_tx.Coding_fmt,
				&Num_tb_full,
				Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size); 


	      
	      Temp_freq_alloc=0;

	      for(j=0;j<NUMBER_OF_FREQUENCY_GROUPS;j++){
		
		if( ( Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc & 
		      ( DL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][j] )) ){
		
		  Temp_freq_alloc += ( DL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][j] );
		  conv_alloc_to_tb2(0,  
				    Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc,
				    Temp_freq_alloc,
				    Lchan_entry->Lchan_info.Target_spec_eff_tx,
				    Lchan_entry->Lchan_info.Dual_stream_flag_tx,
				    Lchan_entry->Lchan_info.Qdepth,
				    &Lchan_entry->Lchan_info.Phy_resources_tx.Coding_fmt,
				    &Num_tb,
				    Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size); 


		  if( (Num_tb_full == Num_tb) || ( Num_tb_full == 0 ) ){
		    if(Num_tb_full==0) {
		      j=-1;
		      //msg("\n***************************\n");
		    }
		    
		    for(sf=NUMBER_OF_FREQUENCY_GROUPS-1;sf>=(unsigned short)(j+1);sf--){
		      if( ( Lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc & 
			    ( DL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][sf] )) ){
			Score=CH_mac_inst[Mod_id].Sinr_sorted_table[i][sf];
			if(Score < Min_score){
			  Min_score=Score;
			  Looser_lchan_entry=Lchan_entry;
			  Worst_freq=(DL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][sf]);
			}
			break;//SF LOOP
		      }
		    }
		    break;//j Frequency loop for this user
		  }
		}
	      }
	    }
	  }
	}//end of users' DCCH lchan loop
      }

      if(Looser_lchan_entry!=NULL){

	conv_alloc_to_tb2(0,  
			  Looser_lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc,
			  Looser_lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc,
			  Looser_lchan_entry->Lchan_info.Target_spec_eff_tx,
			  Looser_lchan_entry->Lchan_info.Dual_stream_flag_tx,
			  Looser_lchan_entry->Lchan_info.Qdepth,
			  &Looser_lchan_entry->Lchan_info.Phy_resources_tx.Coding_fmt,
			  &Num_tb_full,
			  Looser_lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size); 

	if(Num_tb_full==0){
	  
	  Looser_lchan_entry->Lchan_info.Lchan_status_tx=LCHAN_IDLE;
	  New_freq_map-=Looser_lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc;
	  Looser_lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc = 0;
	  Looser_lchan_entry->Lchan_info.Nb_sched_tb_dl= 0;
	  if(Looser_lchan_entry->Lchan_info.Sched_flag==0){
	    msg("[MAC]ERROR: Unschedule USER with Shed_flag on false\n");
	    mac_xface->macphy_exit("");
	  }
	  
	  Looser_lchan_entry->Lchan_info.Sched_flag=0;
	  CH_mac_inst[Mod_id].Nb_sched--;
	}
	else{
	  New_freq_map -= Worst_freq;
	  Looser_lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc-=Worst_freq;
	}
	
	Looser_lchan_entry->Lchan_info.Qdepth_temp=0;//finish with this user

#ifdef DEBUG_SCHED
	msg("[NODEB %d]TTI %d: DL: unschedule Freq %x from lchan %d, Freq_alloc %x, Freq_map %x\n",
	    Mod_id,
	    Mac_rlc_xface->frame,
	    Worst_freq,
	    Looser_lchan_entry->Lchan_info.Lchan_id.Index,
	    Looser_lchan_entry->Lchan_info.Phy_resources_tx.Freq_alloc,
	    New_freq_map);
#endif
	
	sf=0;
	while( ((DL_FREQ_MAP << sf ) & Worst_freq) == 0 )
	  sf++;
	k=sf;
      }
      
      else {
	break; //No new allocation neither reallocation are possible
      }
      
#ifdef DEBUG_SCHED      
      msg("end of DL REallocation round for freq %x, New_freq_map %x, last %x, wait %d\n",
	  Curr_freq,
	  New_freq_map,
	  Last_freq_map,
	  Wait_for_better_fg);
#endif
    }
    
  }



  //  nodeb_sched_ul(unsigned char Mod_id)


  Total_rx_rate=0;
  CH_mac_inst[Mod_id].Nb_sched=0;

  // Loop over all UE attached to this CH
  for(i=1;i<(NB_CNX_CH+1);i++){
    
    if(CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1){
      CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Qdepth=0;
      CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Qdepth_temp=0;

      if((CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Lchan_status_rx == MAC_SCHED_RX_REQ)||								    (CH_mac_inst[Mod_id].Dcch_lchan[i].Next_sched_limit < (Mac_rlc_xface->frame + DCCH_SCHED_PERIOD))){ 

	// DCCH for user i is active in this TTI
	CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Phy_resources_rx_sched.Freq_alloc=0;
	CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Phy_resources_rx_sched.Coding_fmt = 0;
	CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Qdepth=1;
	CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Qdepth_temp=1;
	CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Nb_sched_tb_ul_temp = 0;
	CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Sched_flag = 0;
	CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Target_spec_eff_rx =  0;  
	CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Dual_stream_flag_rx = 0;
      }   
    }
    
    for(j=0;j<NB_RAB_MAX;j++){
      if( (CH_mac_inst[Mod_id].Dtch_lchan[j][i].Active == 1)){ 
	CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Qdepth = 0;
	CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Qdepth_temp = 0;	
	
	CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate=
	  (CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate*(SCHED_LONG_MAW-1)
	   +CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Nb_rx_last_tti)/SCHED_LONG_MAW;//bits/TTI
	CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Nb_rx_last_tti=0;
	if (CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth > 0 ){
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Phy_resources_rx_sched.Freq_alloc=0;
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Phy_resources_rx_sched.Coding_fmt = 0;
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Qdepth = 
	    CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth;
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Qdepth_temp = CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Qdepth;
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Nb_sched_tb_ul_temp = 0;
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Sched_flag = 0;
	  // CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate=(CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate*31
	  //						     +CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Nb_rx_last_tti)/32;//bits/TTI
	    
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Nb_rx_last_tti=0;
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate_temp=CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate;
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Target_spec_eff_rx = 3;   // Call spectral efficiency computation here  
	  CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Dual_stream_flag_rx = 0;	    
	  Total_rx_rate+=CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate;
	  
#ifdef DEBUG_SCHED
	  msg("[MAC][NodeB %d] TTi %d, Sched UL: Lchan %d has %d Tb\n",
	      Mod_id,
	      Mac_rlc_xface->frame,
	      CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_id.Index,
	      CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Qdepth_temp);
#endif
	}
      }
    }
  }  // End loop over UE
  
  
  k=0;
  Last_freq_map=0xffff;
  New_freq_map=0;
  Wait_for_better_fg=0;
  Min_score=MIN_SCORE;
  
  while(1){
    Curr_freq = ( UL_FREQ_MAP << k );
    
    if ( ((New_freq_map!=0xffff) &&  (Last_freq_map!=New_freq_map)) || (Wait_for_better_fg == 1) ){
      
#ifdef DEBUG_SCHED
      msg("New Round, Curr_freq %x, New_freq_map %x, last_freq_map %x, Jumping %d\n",
	  Curr_freq,New_freq_map,Last_freq_map,Wait_for_better_fg);
#endif //DEBUG_SCHED

      if(!(Curr_freq & New_freq_map)){ //Current Frequency "k" is available

	Min_score=MIN_SCORE;

#ifdef DEBUG_SCHED
	msg("[NODE %d]TTI %d: New UL allocation round for freq %x, New_freq_map %x, last %x, wait %d\n",
	    Mod_id,
	    Mac_rlc_xface->frame,
	    Curr_freq,
	    New_freq_map,
	    Last_freq_map,
	    Wait_for_better_fg);
#endif
	
	Wait_for_better_fg=0;
	Last_freq_map=New_freq_map;
	
	// loop over all nodes connected to CH and their RBs and select the one who maximize the score function   
	
	for(i=1;i<(NB_CNX_CH+1);i++){
	  // Do DCCH
	  if( (CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1) && (CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Qdepth_temp>0) ){
	    Score=DCCH_SCORE - CH_mac_inst[Mod_id].Def_meas[i].Sinr_meas[0][k];
	    
	    if( (Score < Min_score) 
		&& ( (CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Sched_flag==1 ) || (CH_mac_inst[Mod_id].Nb_sched < NB_UL_SCHED_MAX))){
	      Min_score=Score;
	      Winner=i;
	      Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[i];
	      
	    }
	  }
	}
	
	// Loop over All connected UE
	for(i=1;i<(NB_CNX_CH+1);i++){
	  
	  // Loop over Active DTCH
	  for(j=0;j<NB_RAB_MAX;j++){
	    if(CH_mac_inst[Mod_id].Dtch_lchan[j][i].Active==1){
	      // if DTCH is active
	      if( (CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Qdepth_temp > 0) 
		  &&((CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Sched_flag==1) || (CH_mac_inst[Mod_id].Nb_sched < NB_UL_SCHED_MAX)) ){
		
		tmp=(CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Req_rate-CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate_temp);

		if(tmp >0)
		  tmp1=(1000*CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate_temp)/tmp;
		else if (tmp <0)
		  tmp1=-(1000*CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate_temp)/tmp;		
		else
		  tmp1=tmp;
		
		if(Total_rx_rate>0)
		  Score=(1000*tmp1*(CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate_temp))/Total_rx_rate;
		else
		  Score=1000*tmp1;

		if( Score < Min_score){

#ifdef DEBUG_MAC_SCHED	      
		  msg("TTI %d:DTCH %d is UL active with Score %d (Qd_temp %d), Min_score %d, Arrival_rate %d, Service rate %d, Service_rate_temp %d\n",
		      mac_xface->frame,
		      CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_id.Index,
		      Score,
		      CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Qdepth_temp,
		      Min_score,
		      CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Req_rate,
		      CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate,
		      CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate_temp);
#endif
		  
		  Min_score=Score;
		  Winner=i;
		  Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[j][i];
		}
	      }
	    }
	  }
	}  // end loop over UE

	
	// if we've found a candidate to schedule
	if(Min_score!=MIN_SCORE){
	  //check if the current frequency "k" is the best one not yet allocated for the winner
	  for(j=0;j<NUMBER_OF_FREQUENCY_GROUPS;j++){
	    Cand_freq= CH_mac_inst[Mod_id].Sinr_sorted_index[Winner][j];
	    if( ((New_freq_map &  (UL_FREQ_MAP << Cand_freq)) ==0 ) && (Cand_freq !=k) &&
 		(CH_mac_inst[Mod_id].Def_meas[Winner].Sinr_meas[0][Cand_freq] > CH_mac_inst[Mod_id].Def_meas[Winner].Sinr_meas[0][k]) ) {
	      Wait_for_better_fg=1;
	      k=Cand_freq;//jump directly to this FG

#ifdef DEBUG_SCHED
	      msg(" USER %d jump from Freq %x to Freq %x (k=%d), Score =%d, Max_score=%d \n",
		  Winner,Curr_freq,(UL_FREQ_MAP << Cand_freq),k,Score,Max_score);
#endif
	      break;
	    }
	  }
	  
	  if( ! Wait_for_better_fg ){//This is the best FG for the winner, allocate

	    Lchan_entry->Lchan_info.Phy_resources_rx_sched.Time_alloc = UL_TIME_ALLOC;
	    Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc+=Curr_freq;

	    // Get number of TB for SPEC_EFF
	    conv_alloc_to_tb2(1,  // ULSCH calculation
			      Lchan_entry->Lchan_info.Phy_resources_rx_sched.Time_alloc,
			      Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc,
			      Lchan_entry->Lchan_info.Target_spec_eff_rx,
			      Lchan_entry->Lchan_info.Dual_stream_flag_rx,
			      Lchan_entry->Lchan_info.Qdepth,   // nb_tb_max
			      &Lchan_entry->Lchan_info.Phy_resources_rx_sched.Coding_fmt,
			      &Num_tb,
			      Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size); 

	    if( Lchan_entry->Lchan_info.Qdepth >= Num_tb ){

	      if( Lchan_entry->Lchan_info.Sched_flag == 0 ){
		Lchan_entry->Lchan_info.Sched_flag=1;
		CH_mac_inst[Mod_id].Nb_sched++;
	      }
	      
	      Lchan_entry->Lchan_info.Qdepth_temp = Lchan_entry->Lchan_info.Qdepth-Num_tb;
	      New_freq_map+=Curr_freq;	    

	      if(((Lchan_entry->Lchan_info.Lchan_id.Index & RAB_OFFSET)== DCCH) &&
		 (Num_tb==1))
		  Lchan_entry->Next_sched_limit+=DCCH_SCHED_PERIOD;
	      
	      Lchan_entry->Lchan_info.Lchan_status_rx = MAC_SCHED_RX_READY;
	      
	      Lchan_entry->Lchan_info.Nb_sched_tb_ul+=(Num_tb-Lchan_entry->Lchan_info.Nb_sched_tb_ul_temp);
	      
	      Lchan_entry->Lchan_info.Rx_rate_temp=(Lchan_entry->Lchan_info.Rx_rate*(SCHED_SHORT_MAW-1)
						    +(Num_tb)
						    * Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size*8)/SCHED_SHORT_MAW;//bits/TTI //16FG ??!!!i 	      
	      Lchan_entry->Lchan_info.Nb_sched_tb_ul_temp = Num_tb;
	      // This is the final number of alloacted TB for this LCHAN

	      Lchan_entry->Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth = Lchan_entry->Lchan_info.Qdepth_temp;
#ifdef DEBUG_SCHED
	      msg("[MAC][NodeB %d] TTI %d : Schedule Lchan %d for RX, Curr_freq %x Added To Freq_alloc %x, UL_FREQ_MAP %x, Remaining %d TB in Buffer, Nb_sched_tb_ul %d,Nb_sched_tb_ul_temp %d,Nb_SCHED_LCHAN %d\n",   
		  NODE_ID[Mod_id],
		  Mac_rlc_xface->frame,
		  Lchan_entry->Lchan_info.Lchan_id.Index, 
		  Curr_freq, 
		  Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc-Curr_freq,
		  New_freq_map,Lchan_entry->Lchan_info.Qdepth_temp,
		  Lchan_entry->Lchan_info.Nb_sched_tb_ul,
		  Lchan_entry->Lchan_info.Nb_sched_tb_ul_temp,
		  CH_mac_inst[Mod_id].Nb_sched);
	      
#endif //DEBUG_SCHED


	      
	    }
	    else {
	      Lchan_entry->Lchan_info.Qdepth_temp =  0;
	      Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc-=Curr_freq;

	      //#ifdef DEBUG_SCHED
	      msg("TTI %d: IN unschedule lchan %d: Num_tb %d, FREQ_MAP %x,freq_alloc %x, Qdepth %d\n",
	      Mac_rlc_xface->frame,
		  Lchan_entry->Lchan_info.Lchan_id.Index,
		  Num_tb,
		  New_freq_map,
		  Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc,
		  Lchan_entry->Lchan_info.Qdepth);
	      //#endif
	    }
	  }
	}	
      }
      
      if( Wait_for_better_fg == 0) 
	k=(k+1)%NUMBER_OF_FREQUENCY_GROUPS;

#ifdef DEBUG_SCHED
      msg("end of UL allocation round for freq %x, New_freq_map %x, last %x, wait %d\n",
	  Curr_freq,
	  New_freq_map,
	  Last_freq_map,
	  Wait_for_better_fg);
#endif
      

    }
    
    else if(!Wait_for_better_fg){//No new allocation, free useless  allocations
      
#ifdef DEBUG_SCHED
      msg("UL REallocation round for freq %x, New_freq_map %x, last %x, wait %d\n",
	  Curr_freq,
	  New_freq_map,
	  Last_freq_map,
	  Wait_for_better_fg);
#endif
      
      Min_score=MIN_SCORE;
      Looser_lchan_entry=NULL;
      
      for(i=1;i<(NB_CNX_CH+1);i++){
	for(kk=0;kk<NB_RAB_MAX;kk++)
	  if(CH_mac_inst[Mod_id].Dtch_lchan[kk][i].Active==1){
	    Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[kk][i];
	    if( Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc != 0 ){
	      conv_alloc_to_tb2(1,  // ULSCH calculation
				Lchan_entry->Lchan_info.Phy_resources_rx_sched.Time_alloc,
				Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc,
				Lchan_entry->Lchan_info.Target_spec_eff_rx,
				Lchan_entry->Lchan_info.Dual_stream_flag_rx,
				Lchan_entry->Lchan_info.Qdepth,   // nb_tb_max,
				&Lchan_entry->Lchan_info.Phy_resources_rx_sched.Coding_fmt,
				&Num_tb_full,
				Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size); 
	      
	      Temp_freq_alloc=0;
	      
	      for(j=0;j<NUMBER_OF_FREQUENCY_GROUPS;j++){
		if( ( Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc & 
		      ( UL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][j] )) ){
		  
		  Temp_freq_alloc += ( UL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][j] );
		  conv_alloc_to_tb2(1,  // ULSCH calculation
				    Lchan_entry->Lchan_info.Phy_resources_rx_sched.Time_alloc,
				    Temp_freq_alloc,
				    Lchan_entry->Lchan_info.Target_spec_eff_rx,
				    Lchan_entry->Lchan_info.Dual_stream_flag_rx,
				    Lchan_entry->Lchan_info.Qdepth,   // nb_tb_maxnum_tb_max,
				    &Lchan_entry->Lchan_info.Phy_resources_rx_sched.Coding_fmt,
				    &Num_tb,
				    Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size); 


		  if( (Num_tb_full == Num_tb) || (Num_tb_full == 0) ){
		    if(Num_tb_full==0) 
		      {
#ifdef DEBUG_SCHED
			msg("LCHAN %d has  0 Tb scheduled with freq_allo%x\n",
			    Lchan_entry->Lchan_info.Lchan_id.Index,
			    Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc);
#endif //DEBUG_SCHED
			j=-1;
		      }
		    
		    for(sf=NUMBER_OF_FREQUENCY_GROUPS-1;sf>=(unsigned short)(j+1);sf--){
		      if( ( Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc & 
			    ( UL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][sf] )) ){
			
			Score=CH_mac_inst[Mod_id].Sinr_sorted_table[i][sf];
	
			if(Score < Min_score){
			  Min_score=Score;
			  Worst_freq=(UL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][sf]);
			  Looser_lchan_entry=Lchan_entry;
			
			}
			break;//sf loop
		      }
		    }
		    break;//j Frequency loop
		  }
		}
	      }
	      
	    }
	  }
      }//end of users' DTCH lchan loop

      if(Min_score==MIN_SCORE){//No frequncy freed from DTCHs, look in DCCHs allocation
	for(i=1;i<(NB_CNX_CH+1);i++){
	  if(CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1){
	    Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[i];
	    if( Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc != 0 ){
	      
	      conv_alloc_to_tb2(1,  // ULSCH calculation
				Lchan_entry->Lchan_info.Phy_resources_rx_sched.Time_alloc,
				Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc,
				Lchan_entry->Lchan_info.Target_spec_eff_rx,
				Lchan_entry->Lchan_info.Dual_stream_flag_rx,
				Lchan_entry->Lchan_info.Qdepth,   // nb_tb_maxnum_tb_max,
				&Lchan_entry->Lchan_info.Phy_resources_rx_sched.Coding_fmt,
				&Num_tb_full,
				Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size); 
	      
	      Temp_freq_alloc=0;
	      for(j=0;j<NUMBER_OF_FREQUENCY_GROUPS;j++){
		if( ( Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc & 
		      ( UL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][j] )) ){
		
		  Temp_freq_alloc += ( UL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][j] );
		  conv_alloc_to_tb2(1,  // ULSCH calculation
				    Lchan_entry->Lchan_info.Phy_resources_rx_sched.Time_alloc,
				    Temp_freq_alloc,
				    Lchan_entry->Lchan_info.Target_spec_eff_rx,
				    Lchan_entry->Lchan_info.Dual_stream_flag_rx,
				    Lchan_entry->Lchan_info.Qdepth,   // nb_tb_maxnum_tb_max,
				    &Lchan_entry->Lchan_info.Phy_resources_rx.Coding_fmt,
				    &Num_tb,
				    Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size); 



		  if( (Num_tb_full == Num_tb) || ( Num_tb_full == 0 )  ){
		    if(Num_tb_full==0) {j=-1;
		      //msg("\nTTI %d:**********DCCH*****************\n", Mac_rlc_xface->frame);
		    }
		    for(sf=NUMBER_OF_FREQUENCY_GROUPS-1;sf>=(unsigned short)(j+1);sf--){
		      if( ( Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc & 
			    ( UL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][sf] )) ){
			Score=CH_mac_inst[Mod_id].Sinr_sorted_table[i][sf];
			if(Score < Min_score){
			  //	    msg("\nTTI %d: **********OK*****************\n", Mac_rlc_xface->frame);
			  Min_score=Score;
			  Looser_lchan_entry=Lchan_entry;
			  Worst_freq=(UL_FREQ_MAP <<CH_mac_inst[Mod_id].Sinr_sorted_index[i][sf]);
			  //	  msg("TEMP Looser Lchan is %d\n",Looser_lchan_entry->Lchan_info.Lchan_id.Index);
			}
			break;//sf loop
		      }
		    }
		    break;//j Frequency loop for this user
		  }
		}
	      }
	    }
	  }
	}//end of users' DCCH lchan loop
      }

      if(Looser_lchan_entry!=NULL){

	conv_alloc_to_tb2(1,  // ULSCH calculation
			  Looser_lchan_entry->Lchan_info.Phy_resources_rx_sched.Time_alloc,
			  Looser_lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc,
			  Looser_lchan_entry->Lchan_info.Target_spec_eff_rx,
			  Looser_lchan_entry->Lchan_info.Dual_stream_flag_rx,
			  Looser_lchan_entry->Lchan_info.Qdepth,   // nb_tb_maxnum_tb_max,
			  &Looser_lchan_entry->Lchan_info.Phy_resources_rx_sched.Coding_fmt,
			  &Num_tb_full,
			  Looser_lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size); 


	if(Num_tb_full==0){
       	  Looser_lchan_entry->Lchan_info.Lchan_status_rx=LCHAN_IDLE;
	  New_freq_map-=Looser_lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc;
	  Looser_lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc = 0;
	  if((Looser_lchan_entry->Lchan_info.Lchan_id.Index & RAB_OFFSET)== DCCH)
	      Looser_lchan_entry->Next_sched_limit-=DCCH_SCHED_PERIOD;

	  if(Looser_lchan_entry->Lchan_info.Sched_flag==0){
	    msg("[MAC]ERROR: Unschedule USER with Shed_flag on false\n");
	    mac_xface->macphy_exit("");
	  }
	  CH_mac_inst[Mod_id].Nb_sched--;
	  Looser_lchan_entry->Lchan_info.Sched_flag=0;
	}
	else{
	  New_freq_map -= Worst_freq;
	  Looser_lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc-=Worst_freq;
	}
	
	Looser_lchan_entry->Lchan_info.Qdepth_temp=0;//finish with this user

#ifdef DEBUG_SCHED
	msg("[NODEB %d]TTI %d: UL, unschedule Freq %x from lchan %d, Freq_alloc %x, Freq_map %x\n",
	    Mod_id,
	    Mac_rlc_xface->frame,
	    Worst_freq,
	    Looser_lchan_entry->Lchan_info.Lchan_id.Index,
	    Looser_lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc,
	    New_freq_map);
#endif
	sf=0;
	while( ((UL_FREQ_MAP << sf ) & Worst_freq) == 0 )
	  sf++;
	k=sf;
      }
      else {
	break; //No new allocation neither reallocation are possible
      }
      
#ifdef DEBUG_SCHED      
      msg("end of UL REallocation round for freq %x, New_freq_map %x, last %x, wait %d\n",
	  Curr_freq,
	  New_freq_map,
	  Last_freq_map,
	  Wait_for_better_fg);
#endif
    }
  }
}
*/
