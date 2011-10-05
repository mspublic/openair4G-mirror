/*________________________L2_interface.c________________________

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
extern eNB_MAC_INST *eNB_mac_inst;
extern UE_MAC_INST *UE_mac_inst;
#endif

//#define RRC_DATA_REQ_DEBUG
//#define DEBUG_RRC

u32 mui=0;
//---------------------------------------------------------------------------------------------//

unsigned char mac_rrc_lite_data_req( unsigned char Mod_id, 
				unsigned short Srb_id, 
				unsigned char Nb_tb,
				char *Buffer,
				u8 eNB_flag,
				u8 eNB_index){
    //------------------------------------------------------------------------------------------------------------------//


  SRB_INFO *Srb_info;
  u8 Sdu_size=0;
  u8 H_size,i;  
  u16 tmp;

#ifdef DEBUG_RRC
  msg("[RRC] Mod_id=%d: mac_rrc_data_req to SRB ID=%d\n",Mod_id,Srb_id);
#endif

  if( eNB_flag == 1){

    if((Srb_id & RAB_OFFSET) == BCCH){
      if(eNB_rrc_inst[Mod_id].SI.Active==0) return 0;
      
      // All even frames transmit SIB in SF 5
      if ((Mac_rlc_xface->frame%2) == 0) {
	memcpy(&Buffer[0],eNB_rrc_inst[Mod_id].SIB1,eNB_rrc_inst[Mod_id].sizeof_SIB1);
#ifdef DEBUG_RRC
	msg("[RRC][eNB%d] Frame %d : BCCH request => SIB 1\n",Mod_id,Rrc_xface->Frame_index);
	for (i=0;i<eNB_rrc_inst[Mod_id].sizeof_SIB1;i++)
	  msg("%x.",Buffer[i]);
	msg("\n");
#endif

	return (eNB_rrc_inst[Mod_id].sizeof_SIB1);
      } // All RFN mod 8 transmit SIB2-3 in SF 5
      else if ((Mac_rlc_xface->frame%8) == 1){
	memcpy(&Buffer[0],eNB_rrc_inst[Mod_id].SIB23,eNB_rrc_inst[Mod_id].sizeof_SIB23);
#ifdef DEBUG_RRC
	msg("[RRC][eNB%d] Frame %d : BCCH request => SIB 2-3\n",Mod_id,Rrc_xface->Frame_index);
	for (i=0;i<eNB_rrc_inst[Mod_id].sizeof_SIB23;i++)
	  msg("%x.",Buffer[i]);
	msg("\n");

#endif
	return(eNB_rrc_inst[Mod_id].sizeof_SIB23);
      }
      else
	return(0);
    }
	
    
    if( (Srb_id & RAB_OFFSET ) == CCCH){
      msg("[RRC][eNB%d] CCCH request (Srb_id %d)\n",Mod_id,Srb_id);

      if(eNB_rrc_inst[Mod_id].Srb0.Active==0) {
	msg("[RRC][eNB%d] CCCH Not active\n",Mod_id);
	return -1;
      }
      Srb_info=&eNB_rrc_inst[Mod_id].Srb0;

      // check if data is there for MAC
      if(Srb_info->Tx_buffer.payload_size>0){//Fill buffer
	msg("[RRC][eNB%d] CCCH (%p) has %d bytes (dest: %p, src %p)\n",Mod_id,Srb_info,Srb_info->Tx_buffer.payload_size,Buffer,Srb_info->Tx_buffer.Payload);
	memcpy(Buffer,Srb_info->Tx_buffer.Payload,Srb_info->Tx_buffer.payload_size);
	Sdu_size = Srb_info->Tx_buffer.payload_size;
	Srb_info->Tx_buffer.payload_size=0;
      }
      
      return (Sdu_size);
    }
  
    
  
  }
  

  else{   //This is an UE
#ifdef DEBUG_RRC
    msg("[RRC][UE%d] Filling rach,SRB_ID %d\n",Mod_id,Srb_id);
    msg("Buffers status %d,\n",UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size);
#endif
    if( (UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size > 0) ) {
      memcpy(&Buffer[0],&UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.Payload[0],UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size);
      u8 Ret_size=UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size;
      UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size=0;
      //      msg("[RRC][UE %d] Sending rach\n",Mod_id);
      return(Ret_size);
    }
    else{
      return 0;
    }
  }
  
}

//--------------------------------------------------------------------------------------------//
u8 mac_rrc_lite_data_ind(u8 Mod_id, u16 Srb_id, char *Sdu, unsigned short Sdu_len,u8 eNB_flag,u8 eNB_index ){ 
  //------------------------------------------------------------------------------------------//
  if (Srb_id == 3)
    msg("[RRC]Node =%d: mac_rrc_data_ind to SI, eNB_UE_INDEX %d...\n",Mod_id,eNB_index); 
  else
    msg("[RRC]Node =%d: mac_rrc_data_ind to SRB ID=%d, eNB_UE_INDEX %d...\n",Mod_id,Srb_id,eNB_index); 

  SRB_INFO *Srb_info;
  unsigned short i;
  int si_window;
  if(eNB_flag == 0){

    //msg("[RRC][UE %d] Received SDU for SRB %d\n",Mod_id,Srb_id);

    if(Srb_id == BCCH){
      if ((Mac_rlc_xface->frame %2) == 0) {
	if (UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 0) {
	  msg("[RRC][UE %d] Frame %d : Received SIB1 from eNB %d (%d bytes)\n",Mod_id,Mac_rlc_xface->frame,eNB_index,Sdu_len);
	  if (UE_rrc_inst[Mod_id].SIB1[eNB_index])
	    memcpy(UE_rrc_inst[Mod_id].SIB1[eNB_index],&Sdu[0],Sdu_len);
	  else {
	    msg("[RRC][FATAL ERROR] SIB1 buffer for eNB %d not allocated, exiting ...\n",eNB_index);
	    mac_xface->macphy_exit("");
	    return(-1);
	  }
	  UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status = 1;
	  decode_SIB1(Mod_id,eNB_index);
	}
      }
      else {
	if ((UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 1) && 
	    (UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus == 0)) {
	  si_window = (Mac_rlc_xface->frame%UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod)/Mac_rlc_xface->frame%UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize;
	  msg("[RRC][UE %d] Frame %d : Received SI (%d bytes), in window %d (SIperiod %d, SIwindowsize %d)\n",Mod_id,Mac_rlc_xface->frame,Sdu_len,si_window,UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod,UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize);
	  memcpy(UE_rrc_inst[Mod_id].SI[eNB_index],&Sdu[0],Sdu_len);
	  if (decode_SI(Mod_id,eNB_index,si_window)==0) {
	    msg("[RRC][UE %d] Frame %d :Decoded SI successfully\n",Mod_id,Mac_rlc_xface->frame);
	    UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus = 1;
	  }

	}
      } 

       
      if ((UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 1) &&
	  (UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus == 1)) {
	if (UE_rrc_inst[Mod_id].Info[eNB_index].Status == RRC_IDLE) {
	  msg("[RRC][UE %d] Received First System Info Switching to RRC_PRE_SYNCHRO\n",Mod_id);
	  UE_rrc_inst[Mod_id].Info[eNB_index].Status = RRC_PRE_SYNCHRO;
	}
	rrc_ue_generate_RRCConnectionRequest(Mod_id,eNB_index);
      }
      



      return 0;
    }   


    if((Srb_id & RAB_OFFSET) == CCCH){
      Srb_info = &UE_rrc_inst[Mod_id].Srb0[eNB_index];
      
      //      msg("[RRC] RX_CCeNB_DATA %d bytes: ",Sdu_len);
      if (Sdu_len>0) {
	memcpy(Srb_info->Rx_buffer.Payload,Sdu,Sdu_len);
	Srb_info->Rx_buffer.payload_size = Sdu_len;
	rrc_ue_decode_ccch(Mod_id,Srb_info,eNB_index);

      }

    }
  }

  else{  // This is an eNB
    Srb_info = &eNB_rrc_inst[Mod_id].Srb0;
    //    msg("\n***********************************INST %d Srb_info %p, Srb_id=%d**********************************\n\n",Mod_id,Srb_info,Srb_info->Srb_id);
    memcpy(Srb_info->Rx_buffer.Payload,Sdu,6);
    rrc_eNB_decode_ccch(Mod_id,Srb_info);
 }
  //  return Nb_tb;
  
}

//-------------------------------------------------------------------------------------------//
void mac_lite_sync_ind(u8 Mod_id,u8 Status){
//-------------------------------------------------------------------------------------------//
}

//------------------------------------------------------------------------------------------------------------------//
void rlcrrc_lite_data_ind( unsigned char Mod_id, u32 Srb_id, u32 sdu_size,u8 *Buffer){
    //------------------------------------------------------------------------------------------------------------------//

  u8 UE_index=(Srb_id-1)/MAX_NUM_RB;
  u8 DCCH_index = Srb_id % MAX_NUM_RB;

  msg("[RRC] Frame %d: RECEIVED MSG ON DCCH %d, UE %d, Size %d\n",Rrc_xface->Frame_index,
      DCCH_index,UE_index,sdu_size);
  if(Mac_rlc_xface->Is_cluster_head[Mod_id]==1)
    rrc_eNB_decode_dcch(Mod_id,DCCH_index,UE_index,Buffer,sdu_size);
  else
    rrc_ue_decode_dcch(Mod_id-NB_eNB_INST,DCCH_index,Buffer,UE_index);
  
} 
 

/*-------------------------------------------------------------------------------------------*/
void rrc_lite_out_of_sync_ind(unsigned char Mod_id, unsigned short eNB_index){
/*-------------------------------------------------------------------------------------------*/


  unsigned char i;
  rlc_info_t rlc_infoP;
  rlc_infoP.rlc_mode=RLC_UM;

  msg("______________[NODE %d][RRC] OUT OF SYNC FROM CH %d______________\n ",NODE_ID[Mod_id],eNB_index);
  
  UE_rrc_inst[Mod_id].Info[eNB_index].Status=RRC_IDLE;
  UE_rrc_inst[Mod_id].Info[eNB_index].Rach_tx_cnt=0;	
  UE_rrc_inst[Mod_id].Info[eNB_index].Nb_bcch_wait=0;	
  UE_rrc_inst[Mod_id].Info[eNB_index].UE_index=0xffff;
  
  UE_rrc_inst[Mod_id].Srb0[eNB_index].Rx_buffer.payload_size=0;
  UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size=0;
  
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Rx_buffer.payload_size=0;
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Tx_buffer.payload_size=0;

  if(UE_rrc_inst[Mod_id].Srb2[eNB_index].Active==1){
    msg("[RRC Inst %d] eNB_index %d, Remove RB %d\n ",Mod_id,eNB_index,UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Srb_id);
    Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_eNB_INST,ACTION_REMOVE,UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Srb_id,SIGNALLING_RADIO_BEARER,Rlc_info_um);
    UE_rrc_inst[Mod_id].Srb2[eNB_index].Active=0;
    UE_rrc_inst[Mod_id].Srb2[eNB_index].Status=IDLE;
    UE_rrc_inst[Mod_id].Srb2[eNB_index].Next_check_frame=0;


  }


} 

/*
u8 get_rrc_status(u8 Mod_id,u8 eNB_flag,u8 eNB_index){
  if(eNB_flag == 1)
    return(eNB_rrc_inst[Mod_id].Info.Status);
  else
    return(UE_rrc_inst[Mod_id].Info[eNB_index].Status);
}
*/




