/*________________________openair_rrc_mesh_interface.c________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/





//#include "openair_types.h"
//#include "openair_defs.h"
//#include "openair_proto.h"
#include "defs.h"
#include "extern.h"
//#include "mac_lchan_interface.h"
//#include "openair_rrc_utils.h"
//#include "openair_rrc_main.h"
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
extern EMULATION_VARS *Emul_vars;
extern CH_MAC_INST *CH_mac_inst;
extern UE_MAC_INST *UE_mac_inst;
#endif

//#define RRC_DATA_REQ_DEBUG
#define DEBUG_RRC

u32 mui=0;
//---------------------------------------------------------------------------------------------//

unsigned char mac_rrc_mesh_data_req( unsigned char Mod_id, 
				     unsigned short Srb_id, 
				     unsigned char Nb_tb,
				     char *Buffer,
				     u8 CH_index){
    //------------------------------------------------------------------------------------------------------------------//


  SRB_INFO *Srb_info;
  u8 Sdu_size=0;
  u8 H_size,i;  
  u16 tmp;

#ifdef DEBUG_RRC
  msg("[RRC] Mod_id=%d: mac_rrc_data_req to SRB ID=%ld\n",Mod_id,Srb_id);
#endif

  if( Mac_rlc_xface->Is_cluster_head[Mod_id]){

    if((Srb_id & RAB_OFFSET) == BCCH){
      if(CH_rrc_inst[Mod_id].Srb3.Active==0) return 0;
      
      // All even frames transmit SIB in SF 5
      if ((Mac_rlc_xface->frame%2) == 0) {
	memcpy(&Buffer[0],CH_rrc_inst[Mod_id].SIB1,CH_rrc_inst[Mod_id].sizeof_SIB1);
#ifdef DEBUG_RRC
	msg("[RRC] Frame %d : BCCH request => SIB 1\n",Rrc_xface->Frame_index);
#endif
	for (i=0;i<CH_rrc_inst[Mod_id].sizeof_SIB1;i++)
	  msg("%x.",Buffer[i]);
	msg("\n");

	return (CH_rrc_inst[Mod_id].sizeof_SIB1);
      } // All RFN mod 8 transmit SIB2-3 in SF 5
      else if ((Mac_rlc_xface->frame%8) == 1){
	memcpy(&Buffer[0],CH_rrc_inst[Mod_id].SIB23,CH_rrc_inst[Mod_id].sizeof_SIB23);
#ifdef DEBUG_RRC
	msg("[RRC] Frame %d : BCCH request => SIB 2-3\n",Rrc_xface->Frame_index);
	for (i=0;i<CH_rrc_inst[Mod_id].sizeof_SIB23;i++)
	  msg("%x.",Buffer[i]);
	msg("\n");

#endif
	return(CH_rrc_inst[Mod_id].sizeof_SIB23);
      }
      else
	return(0);
    }
	
    
    if( (Srb_id & RAB_OFFSET ) == CCCH){
      msg("[RRC] CCCH request (Srb_id %d)\n",Srb_id);

      if(CH_rrc_inst[Mod_id].Srb0.Active==0) {
	msg("[RRC] CCCH Not active\n");
	return -1;
      }
      Srb_info=&CH_rrc_inst[Mod_id].Srb0;

      // check if data is there for MAC
      if(Srb_info->Tx_buffer.W_idx>0){//Fill buffer
	msg("[RRC] CCCH (%p) has %d bytes (dest: %p, src %p)\n",Srb_info,Srb_info->Tx_buffer.W_idx,Buffer,Srb_info->Tx_buffer.Payload);
	memcpy(Buffer,Srb_info->Tx_buffer.Payload,Srb_info->Tx_buffer.W_idx);
	Sdu_size = Srb_info->Tx_buffer.W_idx;
	Srb_info->Tx_buffer.W_idx=0;
      }
      
      return (Sdu_size);
    }
  
    
  
  }
  

  else{   Mod_id-=NB_CH_INST; //This is an UE
#ifdef DEBUG_RRC
    msg("filling rach,SRB_ID %d\n",Srb_id);
    msg("Buffers status %d,\n",UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx);
#endif
    if( (UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx != UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.R_idx)) { //&& (RRC_CONNECTION_FLAG==1)){
      memcpy(&Buffer[0],&UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.Payload[0],UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx);
      u8 Ret_size=UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx;
      UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx=0;
      //      msg("[RRC][UE %d] Sending rach\n",Mod_id);
      return(Ret_size);
    }
    else{
      return 0;
    }
  }
  
}

//--------------------------------------------------------------------------------------------//
u8 mac_rrc_mesh_data_ind(u8 Mod_id, u16 Srb_id, char *Sdu, unsigned short Sdu_len,u8 CH_index ){ 
  //------------------------------------------------------------------------------------------//
  //  msg("[OPENAIR][RRC]Node =%d: mac_rrc_data_ind to SRB ID=%ld, CH_UE_INDEX %d...\n",NODE_ID[Mod_id],Srb_id,CH_index); 

  SRB_INFO *Srb_info;
  unsigned short Rv_tb_idx_last,Rv_tb_idx,Size,i;
  int si_window;
  if(!Mac_rlc_xface->Is_cluster_head[Mod_id]){
    Mod_id-=NB_CH_INST;

    //msg("[RRC][UE %d] Received SDU for SRB %d\n",Mod_id,Srb_id);

    if(Srb_id == BCCH){
      if ((Mac_rlc_xface->frame %2) == 0) {
	if (UE_rrc_inst[Mod_id].Info[CH_index].SIB1Status == 0) {
	  msg("[RRC][UE %d] Frame %d : Received SIB1 (%d bytes)\n",Mod_id,Mac_rlc_xface->frame,Sdu_len);
	  memcpy(UE_rrc_inst[Mod_id].SIB1[CH_index],&Sdu[0],Sdu_len);
	  UE_rrc_inst[Mod_id].Info[CH_index].SIB1Status = 1;
	  decode_SIB1(Mod_id,CH_index);
	}
      }
      else {
	if ((UE_rrc_inst[Mod_id].Info[CH_index].SIB1Status == 1) && 
	    (UE_rrc_inst[Mod_id].Info[CH_index].SIStatus == 0)) {
	  si_window = (Mac_rlc_xface->frame%UE_rrc_inst[Mod_id].Info[CH_index].SIperiod)/Mac_rlc_xface->frame%UE_rrc_inst[Mod_id].Info[CH_index].SIwindowsize;
	  msg("[RRC][UE %d] Frame %d : Received SI (%d bytes), in window %d (SIperiod %d, SIwindowsize %d)\n",Mod_id,Mac_rlc_xface->frame,Sdu_len,si_window,UE_rrc_inst[Mod_id].Info[CH_index].SIperiod,UE_rrc_inst[Mod_id].Info[CH_index].SIwindowsize);
	  memcpy(UE_rrc_inst[Mod_id].SI[CH_index],&Sdu[0],Sdu_len);
	  UE_rrc_inst[Mod_id].Info[CH_index].SIStatus = 1;
	  decode_SI(Mod_id,CH_index,si_window);	  
	}
      } 

       
      if ((UE_rrc_inst[Mod_id].Info[CH_index].SIB1Status == 1) &&
	  (UE_rrc_inst[Mod_id].Info[CH_index].SIStatus == 1)) {
	if (UE_rrc_inst[Mod_id].Info[CH_index].Status == RRC_IDLE) {
	  msg("[RRC][UE %d] Received First System Info Switching to RRC_PRE_SYNCHRO\n",Mod_id);
	  UE_rrc_inst[Mod_id].Info[CH_index].Status = RRC_PRE_SYNCHRO;
	}
	rrc_ue_generate_RRCConnectionRequest(Mod_id,CH_index);
      }
      



      return 0;
    }   


    if((Srb_id & RAB_OFFSET) == CCCH){
      Srb_info = &UE_rrc_inst[Mod_id].Srb0[CH_index];
      
      //      msg("[RRC] RX_CCCH_DATA %d bytes: ",Sdu_len);
      if (Sdu_len>0) {
	for (i=0;i<Sdu_len;i++)
	  msg("%x ",(unsigned char)Sdu[i]);
	msg("\n");
	memcpy(Srb_info->Rx_buffer.Payload,Sdu,Sdu_len);
	Srb_info->Rx_buffer.W_idx = Sdu_len;
	rrc_ue_decode_ccch(Mod_id,Srb_info,CH_index);

      }

    }
  }

  else{  // This is a CH
    Srb_info = &CH_rrc_inst[Mod_id].Srb0;
    //    msg("\n***********************************INST %d Srb_info %p, Srb_id=%d**********************************\n\n",Mod_id,Srb_info,Srb_info->Srb_id);
    memcpy(Srb_info->Rx_buffer.Payload,Sdu,6);
    rrc_ch_decode_ccch(Mod_id,Srb_info);
 }
  //  return Nb_tb;
  
}

//-------------------------------------------------------------------------------------------//
void mac_mesh_sync_ind(u8 Mod_id,u8 Status){
//-------------------------------------------------------------------------------------------//
}

//------------------------------------------------------------------------------------------------------------------//
void rlcrrc_mesh_data_ind( unsigned char Mod_id, u32 Srb_id, u32 sdu_size,u8 *Buffer){
    //------------------------------------------------------------------------------------------------------------------//
  //msg("[OPENAIR][RRC]Mod_id=%d: rlc_rrc_data_ind to SRB ID=%d, size %d,...\n",Mod_id,Srb_id,sdu_size);
  // usleep(1000000);
  u8 UE_index=(Srb_id-1)/MAX_NUM_RB;

  //  msg("[RRC] Frame %d: RECEIVED MSG ON DCCH %d, Size %d\n",Rrc_xface->Frame_index,
  //      Srb_id,sdu_size);
  if(Mac_rlc_xface->Is_cluster_head[Mod_id]==1)
    rrc_ch_decode_dcch(Mod_id,UE_index,Buffer,sdu_size);
  else
    rrc_ue_decode_dcch(Mod_id-NB_CH_INST,Buffer,UE_index);
  
} 
 

#define W_IDX UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.W_idx

/*-------------------------------------------------------------------------------------------*/
void rrc_mesh_out_of_sync_ind(unsigned char Mod_id, unsigned short CH_index){
/*-------------------------------------------------------------------------------------------*/


  unsigned char i;
  rlc_info_t rlc_infoP;
  rlc_infoP.rlc_mode=RLC_UM;
  MAC_CONFIG_REQ Mac_config_req; 
  //Mod_id-=NB_CH_INST;
  Mac_config_req.UE_CH_index=CH_index;
  msg("______________[NODE %d][RRC] OUT OF SYNC FROM CH %d______________\n ",NODE_ID[Mod_id],CH_index);
  
  UE_rrc_inst[Mod_id].Info[CH_index].Status=RRC_IDLE;
  UE_rrc_inst[Mod_id].Info[CH_index].Rach_tx_cnt=0;	
  UE_rrc_inst[Mod_id].Info[CH_index].Nb_bcch_wait=0;	
  UE_rrc_inst[Mod_id].Info[CH_index].UE_index=0xffff;
  
  UE_rrc_inst[Mod_id].Srb0[CH_index].Rx_buffer.R_idx=0;
  UE_rrc_inst[Mod_id].Srb0[CH_index].Rx_buffer.W_idx=0;
  //Rrc_inst[Mod_id].Srb0[CH_index].Rx_buffer.Rv_tb_idx=0;
  UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.R_idx=0;
  UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx=0;
  //Rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.Rv_tb_idx=0;
  
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Rx_buffer.R_idx=0;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Rx_buffer.W_idx=0;
  //  UE_rrc_inst[Mod_id].Srb1[CH_index].Rx_buffer.Rv_tb_idx=0;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Tx_buffer.R_idx=0;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Tx_buffer.W_idx=0;

  //  Rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.Rv_tb_idx=0;
  if(UE_rrc_inst[Mod_id].Srb2[CH_index].Active==1){
    msg("[RRC Inst %d] CH_index %d, Remove RB %d\n ",Mod_id,CH_index,UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Srb_id);
    Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_CH_INST,ACTION_REMOVE,UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Srb_id,SIGNALLING_RADIO_BEARER,Rlc_info_um);
    UE_rrc_inst[Mod_id].Srb2[CH_index].Active=0;
    UE_rrc_inst[Mod_id].Srb2[CH_index].Status=IDLE;
    UE_rrc_inst[Mod_id].Srb2[CH_index].Next_check_frame=0;

    Mac_config_req.Lchan_id.Index=UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Srb_id;
    Mac_config_req.Lchan_type=DCCH;
    Mac_rlc_xface->mac_config_req(Mod_id+NB_CH_INST,REMOVE_LC,&Mac_config_req);

  }


}    



