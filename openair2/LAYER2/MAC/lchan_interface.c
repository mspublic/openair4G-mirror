/*________________________mac_lchan_interface.c________________________

  Authors : Hicham Anouar, Raymond Knopp
  Company : EURECOM
  Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
  ________________________________________________________________*/

#include "extern.h"
#include "defs.h"
#include "COMMON/mac_rrc_primitives.h"

#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif

#define DEBUG_LCHAN_INTERFACE

int clear_lchan_stats(LCHAN_INFO_TABLE_ENTRY *Lchan_entry) {

  int kk;

  Lchan_entry->Active = 0;
  Lchan_entry->Lchan_info.W_idx=0;
  Lchan_entry->Lchan_info.Bw_req_active=1;
  Lchan_entry->Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth=0;
  Lchan_entry->Lchan_info.Meas_entry.Status=IDLE;
  Lchan_entry->Lchan_info.Nb_sched_tb_ul=0;
  Lchan_entry->Lchan_info.Nb_sched_tb_dl=0;
  Lchan_entry->Lchan_info.Nb_sched_tb_ul_temp=0;
  Lchan_entry->Lchan_info.Nb_sched_tb_dl_temp=0;


  Lchan_entry->Lchan_info.NB_TX=0;
  Lchan_entry->Lchan_info.NB_TX_LAST=0;
  Lchan_entry->Lchan_info.NB_BW_REQ_TX=0;
  Lchan_entry->Lchan_info.NB_BW_REQ_RX=0;
  Lchan_entry->Lchan_info.output_rate=0;
  Lchan_entry->Lchan_info.NB_RX=0;
  Lchan_entry->Lchan_info.NB_RX_ERRORS=0;
  Lchan_entry->Lchan_info.NB_RX_SACH_ERRORS=0;
  Lchan_entry->Lchan_info.NB_RX_SACCH_ERRORS=0;
  Lchan_entry->Lchan_info.NB_RX_SACH_MISSING=0;
  //  Lchan_entry->Lchan_info.Phy_resources_rx_sched.Freq_alloc=0;
  Lchan_entry->Lchan_info.Lchan_status_tx=IDLE;
  Lchan_entry->Lchan_info.Lchan_status_rx=IDLE;



  Lchan_entry->Lchan_info.Nb_tx_last_tti=0;
  Lchan_entry->Lchan_info.Nb_rx_last_tti=0;
  Lchan_entry->Lchan_info.Last_sched_tti=0;
  Lchan_entry->Lchan_info.Last_feedback_tti=0;
  Lchan_entry->Lchan_info.Tx_rate=0;
  Lchan_entry->Lchan_info.Rx_rate=0;
  Lchan_entry->Lchan_info.Rx_rate_temp=0;
  Lchan_entry->Lchan_info.Tx_rate_temp=0;
  Lchan_entry->Lchan_info.Arrival_rate=0;
  Lchan_entry->Lchan_info.Req_rate=0;
  Lchan_entry->Lchan_info.Spec_eff=0;

  for (kk=0;kk<MAX_NUMBER_TB_PER_LCHAN;kk++) {
    Lchan_entry->Lchan_info.NB_TX_TB[kk]=0;    
    Lchan_entry->Lchan_info.NB_RX_TB[kk]=0;    
    Lchan_entry->Lchan_info.NB_RX_ERRORS_TB[kk]=0;  
  }


}

//------------------------------------------------------------------------------------------------------------------//
int clear_lchan_table(LCHAN_INFO_TABLE_ENTRY *Table, u8 Dim) {
  //------------------------------------------------------------------------------------------------------------------//
  unsigned char i,kk;
  if (Table) {
    for (i=0;i<Dim;i++){
      Table[i].Active = 0;

      Table[i].Lchan_info.W_idx = 0;
      Table[i].Lchan_info.Bw_req_active=1;
      Table[i].Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth=0;

      Table[i].Lchan_info.Meas_entry.Status=IDLE;
      Table[i].Lchan_info.Nb_sched_tb_ul=0;
      Table[i].Lchan_info.Nb_sched_tb_dl=0;
      Table[i].Lchan_info.Nb_sched_tb_ul_temp=0;
      Table[i].Lchan_info.Nb_sched_tb_dl_temp=0;

      Table[i].Lchan_info.NB_TX=0;
      Table[i].Lchan_info.NB_TX_LAST=0;
      Table[i].Lchan_info.NB_BW_REQ_TX=0;
      Table[i].Lchan_info.NB_BW_REQ_RX=0;
      Table[i].Lchan_info.output_rate=0;
      Table[i].Lchan_info.NB_RX=0;

      Table[i].Lchan_info.NB_RX_ERRORS=0;
      Table[i].Lchan_info.NB_RX_SACH_ERRORS=0;
      Table[i].Lchan_info.NB_RX_SACCH_ERRORS=0;
      Table[i].Lchan_info.NB_RX_SACH_MISSING=0;
      //      Table[i].Lchan_info.Phy_resources_rx_sched.Freq_alloc=0;


      Table[i].Lchan_info.Lchan_status_tx=IDLE;
      Table[i].Lchan_info.Lchan_status_rx=IDLE;

      Table[i].Lchan_info.Nb_tx_last_tti=0;
      Table[i].Lchan_info.Nb_rx_last_tti=0;
      Table[i].Lchan_info.Last_sched_tti=0;
      Table[i].Lchan_info.Last_feedback_tti=0;
      Table[i].Lchan_info.Tx_rate=0;
      Table[i].Lchan_info.Rx_rate=0;
      Table[i].Lchan_info.Rx_rate_temp=0;
      Table[i].Lchan_info.Tx_rate_temp=0;
      Table[i].Lchan_info.Arrival_rate=0;
      Table[i].Lchan_info.Req_rate=0;
      Table[i].Lchan_info.Spec_eff=0;

      for (kk=0;kk<16;kk++) {
	Table[i].Lchan_info.NB_TX_TB[kk]=0;    
	Table[i].Lchan_info.NB_RX_TB[kk]=0;    
	Table[i].Lchan_info.NB_RX_ERRORS_TB[kk]=0;  
      }  
    }
  }
  else {
    msg("[OPENAIR][MAC] clear_lchan_table: Table is null\n");
    mac_xface->macphy_exit("");
    return(-1);
  }
  return(0);
}
//------------------------------------------------------------------------------------------------------------------//
u16 mac_config_req(u8 Mod_id,u8 Action,MAC_CONFIG_REQ *Req){
  //------------------------------------------------------------------------------------------------------------------//

  if(Mac_rlc_xface->Is_cluster_head[Mod_id])
    return(ch_mac_config_req(Mod_id,Action,Req));
  else
    return(ue_mac_config_req(Mod_id-NB_CH_INST,Action,Req));
  
}
//------------------------------------------------------------------------------------------------------------------//
u16 ch_mac_config_req(u8 Mod_id,u8 Action,MAC_CONFIG_REQ *Req){
  //------------------------------------------------------------------------------------------------------------------//

  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  LCHAN_INFO_DIL_TABLE_ENTRY *Lchan_entry_dil;
  unsigned short i,In_idx,Idx1,Idx2;

#ifdef DEBUG_LCHAN_INTERFACE
  msg("[MAC][LCHAN INTERFACE] Received config request from RRC for lchan %d\n",Req->Lchan_id.Index);
#endif
  switch(Req->Lchan_type){
  case BCCH: 
    Lchan_entry=&CH_mac_inst[Mod_id].Bcch_lchan;
    break;
  case CCCH: 
    Lchan_entry=&CH_mac_inst[Mod_id].Ccch_lchan;
    break;
  case DCCH: 
    Lchan_entry=&CH_mac_inst[Mod_id].Dcch_lchan[Req->UE_CH_index];
    Lchan_entry->Next_sched_limit=Mac_rlc_xface->frame + DCCH_SCHED_PERIOD;
    //    Lchan_entry->Lchan_info.Phy_resources_rx.Time_alloc=UL_TIME_ALLOC;
    //    Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc=DL_TIME_ALLOC;
    msg("\n[LCHAN XFACE][CH %d]TTI %d: DCCH %d config,UE_index %d\n", 
	NODE_ID[Mod_id],
	Mac_rlc_xface->frame,
	Req->Lchan_id.Index,
	Req->UE_CH_index);
    break;
  case DTCH:
  case DTCH_BD:
    i=(Req->Lchan_id.Index & RAB_OFFSET) - DTCH_BD;
    Lchan_entry=&CH_mac_inst[Mod_id].Dtch_lchan[i][Req->UE_CH_index];
    Lchan_entry->Lchan_info.Last_feedback_tti=Mac_rlc_xface->frame;
    //    Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc=DL_TIME_ALLOC;
    //    Lchan_entry->Lchan_info.Phy_resources_rx.Time_alloc=UL_TIME_ALLOC;
    msg("\n[LCHAN XFACE][CH %d] TTI %d: DTCH %d, UE_INDEX=%d,\n",
	NODE_ID[Mod_id],
	Mac_rlc_xface->frame,
	Req->Lchan_id.Index,
	Req->UE_CH_index,
	i);
    break;
  case DTCH_DIL:
    Idx1=( ( Req->Lchan_id.Index & RAB_OFFSET1 ) >> RAB_SHIFT1 );
    Idx2=( ( Req->Lchan_id.Index & RAB_OFFSET2 ) >> RAB_SHIFT2 );
    In_idx = (Req->Lchan_id.Index & RAB_OFFSET);
    if(CH_mac_inst[Mod_id].Dtch_dil_lchan[In_idx][Idx1][Idx2].Active==1  && Action==ADD_LC){
#ifdef DEBUG_LC_CFG 
      msg("[LCHAN XFACE] Lchan already configured DTCH: UE_CH_INDEX=%d, Lchan_id=%d,Abort!!!\n",Req->UE_CH_index
	  ,CH_mac_inst[Mod_id].Dtch_lchan[i][Req->UE_CH_index].Lchan_info.Lchan_id.Index);
#endif //DEBUG_LC_CFG 
      return 0;
    }
    Lchan_entry_dil=&CH_mac_inst[Mod_id].Dtch_dil_lchan[In_idx][Idx1][Idx2];
    if (Action == ADD_LC){  
      Lchan_entry_dil->Active=1;
      Lchan_entry_dil->Lchan_info_dil.Lchan_id.Index=Req->Lchan_id.Index;  
      Lchan_entry_dil->Lchan_info_dil.Lchan_status = LCHAN_IDLE;
      Lchan_entry_dil->Lchan_info_dil.Lchan_type = Req->Lchan_type;
      memcpy(&Lchan_entry_dil->Lchan_info_dil.Lchan_desc,(LCHAN_DESC*)&Req->Lchan_desc[0],LCHAN_DESC_SIZE);
    } 
    else  Lchan_entry_dil->Active=0;
    return Lchan_entry_dil->Lchan_info_dil.Lchan_id.Index;
    break;
  }
  if (Action == ADD_LC){
    if(Req->Lchan_type <=DTCH_BD)  
      Lchan_entry->Active=1;
    Lchan_entry->Lchan_info.Lchan_id.Index=Req->Lchan_id.Index;  
    Lchan_entry->Lchan_info.Lchan_status_tx = LCHAN_IDLE;
    Lchan_entry->Lchan_info.Lchan_status_rx = LCHAN_IDLE;
    Lchan_entry->Lchan_info.UE_CH_index = Req->UE_CH_index;
    Lchan_entry->Lchan_info.Lchan_type = Req->Lchan_type;
    memcpy(&Lchan_entry->Lchan_info.Lchan_desc[0],(LCHAN_DESC*)&Req->Lchan_desc[0],LCHAN_DESC_SIZE);
    memcpy(&Lchan_entry->Lchan_info.Lchan_desc[1],(LCHAN_DESC*)&Req->Lchan_desc[1],LCHAN_DESC_SIZE);
  }
  else {
    Lchan_entry->Active=0;
    clear_lchan_stats(Lchan_entry);
  }
    return Lchan_entry->Lchan_info.Lchan_id.Index;
  
}

//------------------------------------------------------------------------------------------------------------------//
u16 ue_mac_config_req(u8 Mod_id,u8 Action,MAC_CONFIG_REQ *Req){
  //------------------------------------------------------------------------------------------------------------------//
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  u16 i,In_idx,UE_index12[2],Idx2;

  switch(Req->Lchan_type){

  case BCCH: 
    Lchan_entry=&UE_mac_inst[Mod_id].Bcch_lchan[Req->UE_CH_index];
    break;

  case CCCH: 
    Lchan_entry=&UE_mac_inst[Mod_id].Ccch_lchan[Req->UE_CH_index];
    break;

  case DCCH: 
    if(UE_mac_inst[Mod_id].Dcch_lchan[Req->UE_CH_index].Active==1 && Action==ADD_LC){
#ifdef DEBUG_LC_CFG 
      msg("[LCHAN XFACE] Lchan already configured DCCH: UE_CH_INDEX=%d, Lchan_id=%d,Abort!!!\n",Req->UE_CH_index,
	  UE_mac_inst[Mod_id].Dcch_lchan[Req->UE_CH_index].Lchan_info.Lchan_id.Index);
#endif //DEBUG_LC_CFG 
      return 0;
    }		
    Lchan_entry=&UE_mac_inst[Mod_id].Dcch_lchan[Req->UE_CH_index];
    //    Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc=UL_TIME_ALLOC;
    //    Lchan_entry->Lchan_info.Phy_resources_rx.Time_alloc=DL_TIME_ALLOC;
    break;

  case DTCH_BD:
  case DTCH:
    i=(Req->Lchan_id.Index & RAB_OFFSET) - DTCH_BD;
    if(UE_mac_inst[Mod_id].Dtch_lchan[i][Req->UE_CH_index].Active==1  && Action==ADD_LC){
#ifdef DEBUG_LC_CFG 
      msg("[LCHAN XFACE] Lchan already configured DTCH: UE_CH_INDEX=%d, Lchan_id=%d,Abort!!!\n",Req->UE_CH_index,
	  UE_mac_inst[Mod_id].Dtch_lchan[i][Req->UE_CH_index].Lchan_info.Lchan_id.Index);
#endif //DEBUG_LC_CFG 
      return 0;
    }
    Lchan_entry=&UE_mac_inst[Mod_id].Dtch_lchan[i][Req->UE_CH_index];
    //    Lchan_entry->Lchan_info.Phy_resources_tx.Time_alloc=UL_TIME_ALLOC;
    //    Lchan_entry->Lchan_info.Phy_resources_rx.Time_alloc=DL_TIME_ALLOC;
    break;
    
  case DTCH_DIL:
    UE_index12[0]=( ( Req->Lchan_id.Index & RAB_OFFSET1 ) >> RAB_SHIFT1 );
    UE_index12[1]=( ( Req->Lchan_id.Index & RAB_OFFSET2 ) >> RAB_SHIFT2 );
    In_idx = (Req->Lchan_id.Index & RAB_OFFSET);
    if(UE_index12[0] == Rrc_xface->UE_index[Mod_id+NB_CH_INST][Req->UE_CH_index]) Idx2=UE_index12[1];
    if(UE_index12[1] == Rrc_xface->UE_index[Mod_id+NB_CH_INST][Req->UE_CH_index]) Idx2=UE_index12[0];
    if(UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx][Req->UE_CH_index][Idx2].Active==1  && Action==ADD_LC){
#ifdef DEBUG_LC_CFG 
      msg("[LCHAN XFACE] Lchan already configured DTCH: UE_CH_INDEX=%d, Lchan_id=%d,Abort!!!\n",Req->UE_CH_index,
	  UE_mac_inst[Mod_id].Dtch_lchan[i][Req->UE_CH_index].Lchan_info.Lchan_id.Index);
#endif //DEBUG_LC_CFG 
      return 0;
    }
    Lchan_entry=&UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx][Req->UE_CH_index][Idx2];
    break;

  default:
    Lchan_entry = NULL;
    msg("[MAC][CONFIG] lchan_interface.c : Unknown LCHAN id, exiting\n");
    mac_xface->macphy_exit("");
    return 0;
  }

  if (Action == ADD_LC){  
    Lchan_entry->Active=1;
    Lchan_entry->Lchan_info.Lchan_id.Index=Req->Lchan_id.Index;  
    Lchan_entry->Lchan_info.Lchan_status_tx = LCHAN_IDLE;
    Lchan_entry->Lchan_info.Lchan_status_rx = LCHAN_IDLE;
    Lchan_entry->Lchan_info.UE_CH_index = Req->UE_CH_index;
    Lchan_entry->Lchan_info.Lchan_type = Req->Lchan_type;
    memcpy(&Lchan_entry->Lchan_info.Lchan_desc[0],(LCHAN_DESC*)&Req->Lchan_desc[0],LCHAN_DESC_SIZE);
    memcpy(&Lchan_entry->Lchan_info.Lchan_desc[1],(LCHAN_DESC*)&Req->Lchan_desc[1],LCHAN_DESC_SIZE);
    msg("\n[LCHAN_XFACE][UE %d]TTI %d: Lchan_config for Lchan %d of Type %d of sizes (RX %d, TX %d)\n",
	NODE_ID[Mod_id+NB_CH_INST],
	Mac_rlc_xface->frame,
	Lchan_entry->Lchan_info.Lchan_id.Index,
	Lchan_entry->Lchan_info.Lchan_type,
	Lchan_entry->Lchan_info.Lchan_desc[0].transport_block_size,
	Lchan_entry->Lchan_info.Lchan_desc[1].transport_block_size);
  }
  else {
    Lchan_entry->Active=0;
    clear_lchan_stats(Lchan_entry);
  }
  return Lchan_entry->Lchan_info.Lchan_id.Index;
}

//------------------------------------------------------------------------------------------------------------------//
MAC_MEAS_REQ_ENTRY* mac_meas_req(u8 Mod_id, MAC_MEAS_REQ *Meas_req){
  //------------------------------------------------------------------------------------------------------------------//

  if(Mac_rlc_xface->Is_cluster_head[Mod_id])
    return(ch_mac_meas_req(Mod_id,Meas_req));
  else
    return(ue_mac_meas_req(Mod_id-NB_CH_INST,Meas_req));
}

//------------------------------------------------------------------------------------------------------------------//
MAC_MEAS_REQ_ENTRY* ch_mac_meas_req(u8 Mod_id,MAC_MEAS_REQ *Meas_req){
  //------------------------------------------------------------------------------------------------------------------//
  unsigned short In_idx,Idx2;
  In_idx = (Meas_req->Lchan_id.Index & RAB_OFFSET);
  Idx2 = (( Meas_req->Lchan_id.Index & RAB_OFFSET2) >> RAB_SHIFT2 );	
  if( In_idx == DCCH ){
    if(CH_mac_inst[Mod_id].Dcch_lchan[Idx2].Lchan_info.Meas_entry.Status!=IDLE){
#ifdef DEBUG_MEAS_CFG 
      msg("Meas Already Configured! Abort...\n");
#endif //DEBUG_MEAS_CFG 
    }
    else{
      memcpy(&CH_mac_inst[Mod_id].Dcch_lchan[Idx2].Lchan_info.Meas_entry.Mac_meas_req,(MAC_MEAS_REQ*)Meas_req,sizeof(MAC_MEAS_REQ));
      CH_mac_inst[Mod_id].Dcch_lchan[Idx2].Lchan_info.Meas_entry.Status=1;
      CH_mac_inst[Mod_id].Dcch_lchan[Idx2].Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth=0;
    }
    return  &CH_mac_inst[Mod_id].Dcch_lchan[Idx2].Lchan_info.Meas_entry;
  }
  else{
    if(CH_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Idx2].Lchan_info.Meas_entry.Status!=IDLE){
#ifdef DEBUG_MEAS_CFG 
      msg("Meas Already Configured! Abort...\n");
#endif //DEBUG_MEAS_CFG 
    }
    else{
      memcpy(&CH_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Idx2].Lchan_info.Meas_entry.Mac_meas_req,Meas_req,sizeof(MAC_MEAS_REQ)); 
      CH_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Idx2].Lchan_info.Meas_entry.Status=1;
      CH_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Idx2].Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth=0;
    }
    return  &CH_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Idx2].Lchan_info.Meas_entry;
  }
  
}
//------------------------------------------------------------------------------------------------------------------//
MAC_MEAS_REQ_ENTRY*  ue_mac_meas_req(u8 Mod_id,MAC_MEAS_REQ *Meas_req){
  //------------------------------------------------------------------------------------------------------------------//
  unsigned short In_idx,Idx2;
  In_idx = (Meas_req->Lchan_id.Index & RAB_OFFSET);
  Idx2 = (( Meas_req->Lchan_id.Index & RAB_OFFSET2) >> RAB_SHIFT2 );	
  if(Idx2 < NB_SIG_CNX_UE){
    if(  In_idx == BCCH ){
      if(UE_mac_inst[Mod_id].Bcch_lchan[Idx2].Lchan_info.Meas_entry.Status!=IDLE){
#ifdef DEBUG_MEAS_CFG 
	msg("Meas Already Configured! Abort...\n");
#endif //DEBUG_MEAS_CFG 
      }
      else{
	memcpy(&UE_mac_inst[Mod_id].Bcch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry.Mac_meas_req,(MAC_MEAS_REQ*)Meas_req,sizeof(MAC_MEAS_REQ));
        UE_mac_inst[Mod_id].Bcch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry.Status=1;
        UE_mac_inst[Mod_id].Bcch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth=0;
      }
      return &UE_mac_inst[Mod_id].Bcch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry;
      
    }
    else   if(  In_idx == CCCH ){
      if(UE_mac_inst[Mod_id].Ccch_lchan[Idx2].Lchan_info.Meas_entry.Status!=IDLE){
#ifdef DEBUG_MEAS_CFG 
	msg("Meas Already Configured! Abort...\n");
#endif //DEBUG_MEAS_CFG 
      }
      else{
	memcpy(&UE_mac_inst[Mod_id].Ccch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry.Mac_meas_req,(MAC_MEAS_REQ*)Meas_req,sizeof(MAC_MEAS_REQ));
        UE_mac_inst[Mod_id].Ccch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry.Status=1;
        UE_mac_inst[Mod_id].Ccch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth=0;
      }
      return &UE_mac_inst[Mod_id].Ccch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry;
      
    }
    
    else  if(  In_idx == DCCH ){
      if(UE_mac_inst[Mod_id].Dcch_lchan[Idx2].Lchan_info.Meas_entry.Status!=IDLE){
#ifdef DEBUG_MEAS_CFG 
	msg("Meas Already Configured! Abort...\n");
#endif //DEBUG_MEAS_CFG 
      }
      else{
	memcpy(&UE_mac_inst[Mod_id].Dcch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry.Mac_meas_req,(MAC_MEAS_REQ*)Meas_req,sizeof(MAC_MEAS_REQ));
        UE_mac_inst[Mod_id].Dcch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry.Status=1;
        UE_mac_inst[Mod_id].Dcch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth=0;
      }
      return &UE_mac_inst[Mod_id].Dcch_lchan[Meas_req->UE_CH_index].Lchan_info.Meas_entry;
    }
    else{
      if(UE_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index].Lchan_info.Meas_entry.Status!=IDLE){
#ifdef DEBUG_MEAS_CFG 
	msg("Meas Already Configured! Abort...\n");
#endif //DEBUG_MEAS_CFG 
      }
      else{
	memcpy(&UE_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index].Lchan_info.Meas_entry.Mac_meas_req,(MAC_MEAS_REQ*)Meas_req,sizeof(MAC_MEAS_REQ)); 
        UE_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index].Lchan_info.Meas_entry.Status=1;
        UE_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index].Lchan_info.Meas_entry.Mac_meas_req.Mac_meas.Qdepth=0;
      }
      return &UE_mac_inst[Mod_id].Dtch_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index].Lchan_info.Meas_entry;
    }
  }
  else{
    if(Idx2 == Rrc_xface->UE_index[Mod_id][Meas_req->UE_CH_index]){
      unsigned short Idx1=(( Meas_req->Lchan_id.Index & RAB_OFFSET1) >> RAB_SHIFT1 );	
      if(UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx][Meas_req->UE_CH_index][Idx1].Lchan_info.Meas_entry.Status!=IDLE){
#ifdef DEBUG_MEAS_CFG 
	msg("Meas Already Configured! Abort...\n");
#endif //DEBUG_MEAS_CFG 
      }
      else{
	memcpy(&UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index][Idx1].Lchan_info.Meas_entry.Mac_meas_req,(MAC_MEAS_REQ*)Meas_req,sizeof(MAC_MEAS_REQ)); 
        UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index][Idx1].Lchan_info.Meas_entry.Status=1;
      }
      return &UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index][Idx1].Lchan_info.Meas_entry;
    }
    else{
      if(UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx][Meas_req->UE_CH_index][Idx2].Lchan_info.Meas_entry.Status!=IDLE){
#ifdef DEBUG_MEAS_CFG 
	msg("Meas Already Configured! Abort...\n");
#endif //DEBUG_MEAS_CFG 
      }
      else{
	memcpy(&UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index][Idx2].Lchan_info.Meas_entry.Mac_meas_req,(MAC_MEAS_REQ*)Meas_req,sizeof(MAC_MEAS_REQ)); 
        UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index][Idx2].Lchan_info.Meas_entry.Status=1;
      }
      return &UE_mac_inst[Mod_id].Dtch_dil_lchan[In_idx-DTCH_BD][Meas_req->UE_CH_index][Idx2].Lchan_info.Meas_entry;
    }
  }
}
/*****************************************************************************************************************
void mac_update_meas(unsigned char Mod_id,MAC_MEAS_REQ_ENTRY *Meas_entry, UL_MEAS *UL_meas){

  MAC_MEAS_IND Meas_ind;
  unsigned char Status,i;
  if(Meas_entry->Status==IDLE ){
    msg("update meas erroooor, Inst %d \n",Mod_id);
    mac_xface->macphy_exit("");
  }
  Meas_entry->Mac_meas_req.Mac_meas.Rssi=
    (Meas_entry->Mac_meas_req.Mac_meas.Rssi*Meas_entry->Mac_meas_req.Mac_avg.Rssi_forgetting_factor)
    +(UL_meas->Wideband_rssi_dBm*(1-Meas_entry->Mac_meas_req.Mac_avg.Rssi_forgetting_factor));
  
  Meas_entry->Rx_activity=1;
  for(i=0;i<NUMBER_OF_MEASUREMENT_SUBBANDS;i++)
    Meas_entry->Mac_meas_req.Mac_meas.Sinr[i]=
      (Meas_entry->Mac_meas_req.Mac_meas.Sinr[i]*Meas_entry->Mac_meas_req.Mac_avg.Sinr_forgetting_factor)
      +(UL_meas->Sub_band_sinr[i]*(1-Meas_entry->Mac_meas_req.Mac_avg.Sinr_forgetting_factor));         
  
  //  Status=mac_check_meas_ind(Meas_entry);
  //  if(Status== MEAS_REPORT){
  //  Meas_ind.Lchan_id.Index=Meas_entry->Mac_meas_req.Lchan_id.Index;
  //  Meas_ind.Process_id=Meas_entry->Mac_meas_req.Process_id;
  //  Meas_ind.Meas_status=Status;
  //  memcpy(&Meas_ind.Meas,(MAC_MEAS_T*)&Meas_entry->Mac_meas_req.Mac_meas,MAC_MEAS_T_SIZE);
  // if (Is_rrc_registered==1){
  //  msg("[MAC][INST %d] MEAS IND TO RRC on Lchan_id %d \n",Mod_id,Meas_ind.Lchan_id.Index);
  //  Rrc_xface->mac_rrc_meas_ind(Mod_id,Meas_ind);
  //  //exit(0);

  //  }
  //  }

}
  */


u8 mac_check_meas_trigger(MAC_MEAS_REQ *Meas_req){
  //------------------------------------------------------------------------------------------------------------------//
  /*if(Meas_req->Mac_meas.Rssi < Meas_req->Meas_trigger.Rssi)
    return 1;
    else if(Meas_req->Mac_meas.Sinr < Meas_req->Meas_trigger.Sinr)
    return 1;
    else if(Meas_req->Mac_meas.Harq_delay > Meas_req->Meas_trigger.Harq_delay)
    return 1;
    else if(Meas_req->Mac_meas.Bler > Meas_req->Meas_trigger.Bler)
    return 1;
    else if(Meas_req->Mac_meas.Spec_eff < Meas_req->Meas_trigger.Spec_eff)
    return 1;*/
  return 0;
}
//------------------------------------------------------------------------------------------------------------------//
unsigned char mac_check_meas_ind(MAC_MEAS_REQ_ENTRY *Meas_entry){
  //------------------------------------------------------------------------------------------------------------------//
  unsigned char Status=0; 
  if( (Mac_rlc_xface->frame > Meas_entry->Next_check_frame) && (Mac_rlc_xface->frame - Meas_entry->Last_report_frame) >= Meas_entry->Mac_meas_req.Rep_interval){
    Status=MEAS_REPORT;
    Meas_entry->Next_check_frame+=Meas_entry->Mac_meas_req.Rep_interval;
    Meas_entry->Last_report_frame=Mac_rlc_xface->frame;
  }
  return Status;
}




