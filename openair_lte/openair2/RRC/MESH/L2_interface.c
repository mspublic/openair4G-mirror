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
u16 rrc_fill_buffer(RRC_BUFFER *Rx_buffer, char *Data, unsigned short Size){
  //----------------------------------------------------------------------------------//

  unsigned char tmp=(((Rx_buffer->W_idx+Size)/(Rx_buffer->Tb_size-CH_CCCH_HEADER_SIZE)) );
  //    msg("FILL BUFFER: W_id %d, Size %d, Header %d, Tb_size %d, Tmp %d\n",Rx_buffer->W_idx,Size,CH_CCCH_HEADER_SIZE,Rx_buffer->Tb_size,tmp);
  if(( tmp * (Rx_buffer->Tb_size -CH_CCCH_HEADER_SIZE)) < (Rx_buffer->W_idx+Size)  )
	  tmp++;  

  if(tmp> 256 ){
    //   msg("buffer overflow\n");
	  //	  exit(-1);
    return 0;
  }
  //if( ((Rx_buffer->Nb_tb_max * Rx_buffer->Tb_size) - (Rx_buffer->W_idx+CH_CCCH_HEADER_SIZE)) < Size)
  // return 0;
  memcpy(&Rx_buffer->Payload[Rx_buffer->W_idx],(char*)Data,Size);  
   Rx_buffer->W_idx+=Size;
  //    msg("filling buffer with size %d, ccch_tx_idx %d\n",Size,Rrc_inst[0].Srb0[0].Tx_buffer.W_idx);
  return Size;
}
//------------------------------------------------------------------------------------------------------------------//
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
  msg("[RRC]Mod_id=%d: mac_rrc_data_req to SRB ID=%ld\n",Mod_id,Srb_id);
#endif

  if( Mac_rlc_xface->Is_cluster_head[Mod_id]){

    if((Srb_id & RAB_OFFSET) == BCCH){
#ifdef DEBUG_RRC
      msg("[RRC] BCCH request\n");
#endif
      H_size=CH_BCCH_HEADER_SIZE;
      if(CH_rrc_inst[Mod_id].Srb0.Active==0) return 0;
      Srb_info=&CH_rrc_inst[Mod_id].Srb0;
      H_size = CH_BCCH_HEADER_SIZE;
      memcpy(&Buffer[0],&Srb_info->Tx_buffer.Header[0],H_size);

      return (CH_BCCH_HEADER_SIZE);
    }
	
    
    if( (Srb_id & RAB_OFFSET ) == CCCH){
      msg("[RRC] CCCH request (Srb_id %d)\n",Srb_id);

      if(CH_rrc_inst[Mod_id].Srb0.Active==0) return 0;
      Srb_info=&CH_rrc_inst[Mod_id].Srb0;

      // check if data is there for MAC
      if(Srb_info->Tx_buffer.W_idx>0){//Fill buffer
	msg("[RRC] CCCH (%p) has %d bytes\n",Srb_info,Srb_info->Tx_buffer.W_idx);
	memcpy(Buffer,Srb_info->Tx_buffer.Payload,Srb_info->Tx_buffer.W_idx);
	Sdu_size = Srb_info->Tx_buffer.W_idx;
	Srb_info->Tx_buffer.W_idx=0;
      }
      
      return (Sdu_size);
    }
  
    
  
  }
  

  else{   Mod_id-=NB_CH_INST; //This is an UE
    //      msg("filling rach,SRB_ID %d\n",Srb_id);
    //      msg("Buffers status %d,\n",UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx);
    if( (UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx != UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.R_idx) ){
      memcpy(&Buffer[0],&UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.Payload[0],UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx);
      u8 Ret_size=UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx;
      UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx=0;
      msg("[RRC][UE] Frame %d: sending rach from NODE %d\n",Rrc_xface->Frame_index,NODE_ID[Mod_id+NB_CH_INST]);
      return(Ret_size);
    }
    else{
      //      msg("erooooooooooooooooor\n");
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
  if(!Mac_rlc_xface->Is_cluster_head[Mod_id]){
    Mod_id-=NB_CH_INST;

    msg("[RRC] Received SDU for SRB %d\n",Srb_id);

    if(Srb_id == BCCH){
      Srb_info = &UE_rrc_inst[Mod_id].Srb0[CH_index];
      memcpy(&Srb_info->Rx_buffer.Payload[0],&Sdu[0],Sdu_len);
      if ((UE_rrc_inst[Mod_id].Info[0].Status == RRC_IDLE) || (UE_rrc_inst[Mod_id].Info[0].Status == RRC_PRE_SYNCHRO)) {
	msg("[RRC] Received System Info Switching to RRC_PRE_SYNCHRO\n");
	UE_rrc_inst[Mod_id].Info[0].Status = RRC_PRE_SYNCHRO;
	rrc_ue_generate_RRCConnectionRequest(Mod_id,0);
      }
      



      return 0;
    }   


    if((Srb_id & RAB_OFFSET) == CCCH){
      Srb_info = &UE_rrc_inst[Mod_id].Srb0[CH_index];
      
      msg("[RRC] RX_CCCH_DATA %d bytes: ",Sdu_len);
      if (Sdu_len>0) {
	for (i=0;i<Sdu_len;i++)
	  msg("%x ",(unsigned char)Sdu[i]);
	msg("\n");
	memcpy(Srb_info->Rx_buffer.Payload,Sdu,Sdu_len);
	Srb_info->Rx_buffer.W_idx = Sdu_len;
	ue_rrc_decode_ccch(Mod_id,Srb_info,CH_index);

      }

    }
  }

  else{  // This is a CH
    Srb_info = &CH_rrc_inst[Mod_id].Srb0;
    //    msg("\n***********************************INST %d Srb_info %p, Srb_id=%d**********************************\n\n",Mod_id,Srb_info,Srb_info->Srb_id);
    memcpy(Srb_info->Rx_buffer.Payload,Sdu,6);
    ch_rrc_decode_ccch(Mod_id,Srb_info);
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
  unsigned short Idx2=(Srb_id >> RAB_SHIFT2);

  msg("[RRC] Frame %d: RECEIVED MSG ON DCCH %d, Size %d\n",Rrc_xface->Frame_index,
      Srb_id,sdu_size);
  if(Mac_rlc_xface->Is_cluster_head[Mod_id]==1)
    ch_rrc_decode_dcch(Mod_id,0,Buffer,sdu_size);
  else
    ue_rrc_decode_dcch(Mod_id-NB_CH_INST,Buffer,Idx2);
  
}


#define W_IDX UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.W_idx

/*-------------------------------------------------------------------------------------------*/
void def_meas_ind(u8 Mod_id,u8 Idx2){
  /*-----------------------------------------------------------------------------------------*/
  unsigned char i,j,k=0;
DEFAULT_MEAS_IND Def_meas_ind;
//mac_rrc_mesh_meas_ind(Mod_id,UE_rrc_inst[Mod_id].Srb0[Idx2].Meas_entry);
mac_rrc_mesh_meas_ind(Mod_id,UE_rrc_inst[Mod_id].Srb1[Idx2].Srb_info.Meas_entry);
mac_rrc_mesh_meas_ind(Mod_id,UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Meas_entry);
for (i=0;i<NB_RAB_MAX;i++)
  if(UE_rrc_inst[Mod_id].Rab[i][Idx2].Active==1)
    mac_rrc_mesh_meas_ind(Mod_id,UE_rrc_inst[Mod_id].Rab[i][Idx2].Rb_info.Meas_entry);

 Def_meas_ind.UE_index=UE_rrc_inst[Mod_id].Info[Idx2].UE_index;

 for(j=0;j<NB_RAB_MAX;j++){
   if( UE_rrc_inst[Mod_id].Rab[j][Idx2].Active==1)
     Def_meas_ind.Rb_active[j]=1;
   else
     Def_meas_ind.Rb_active[j]=0;
 }
 
 UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload[k++]=(unsigned char)UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.R_idx;
 
 memcpy(&UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload[1],(char*)&Def_meas_ind,DEFAULT_MEAS_IND_SIZE);
 


 UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload[W_IDX]= 0;
 if((UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.IP_addr_type!=NONE_L3) && ( UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.IP_addr_ok < 20) ){//MR_IP_ADDR sent 20 times
   //   msg("[RRC] UE inst %d, send attach cfm \n",NODE_ID[Mod_id+NB_CH_INST]);

     UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload[W_IDX++]= UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.IP_addr_type;
     memcpy(&UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload[W_IDX],UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.IP_addr,16);
     W_IDX+=16;
     UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.IP_addr_ok++;
 }

Mac_rlc_xface->rrc_rlc_data_req(Mod_id+NB_CH_INST,UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Srb_id,mui++,0,W_IDX,UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload);
//  msg("[NODE %d][RRC_MESH_XFACE] Frame %d: SENT MEASUREMENT REOPORT MSG ON DCCH %d, Size=%d, UE_index %d\n",NODE_ID[Mod_id+NB_CH_INST],Rrc_xface->Frame_index,UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Srb_id,W_IDX,Def_meas_ind.UE_index);

// if(W_IDX > (DEFAULT_MEAS_IND_SIZE+1))

//for(j=0;j<W_IDX;j++)
// msg("%x.",UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload[j]);
//msg("\n");
 W_IDX=DEFAULT_MEAS_IND_SIZE+1;
UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.R_idx=0;
}


/*-------------------------------------------------------------------------------------------*/
void mac_rrc_mesh_meas_ind(u8 Mod_id, MAC_MEAS_REQ_ENTRY *Meas_entry){
  /*-----------------------------------------------------------------------------------------*/
  if(Meas_entry){
 
    if( (Meas_entry-> Rx_activity==1 ) && (Rrc_xface->Frame_index > Meas_entry->Next_check_frame) && (Rrc_xface->Frame_index - Meas_entry->Last_report_frame) >= Meas_entry->Mac_meas_req.Rep_interval){
      MAC_MEAS_IND Meas_ind;
      unsigned short Rb_id=Meas_entry->Mac_meas_req.Lchan_id.Index;
      //local processing ; put data on dcch 
      unsigned short Idx2=(Rb_id & RAB_OFFSET2) >> RAB_SHIFT2;
      unsigned short In_idx=(Rb_id & RAB_OFFSET);
      
      if(In_idx < DCCH)
	Meas_ind.Lchan_id.Index=In_idx;
      else
	Meas_ind.Lchan_id.Index=(Rb_id & RAB_OFFSET) + (Rrc_xface->UE_index[Mod_id+NB_CH_INST][Idx2] << RAB_SHIFT2) ;
       msg("[OPENAIR][RRC] [Node %d] Frame %d: mac_rrc_meas_ind for LC_ID %d\n",NODE_ID[Mod_id+NB_CH_INST],Rrc_xface->Frame_index,
	    Meas_ind.Lchan_id.Index);
      //if(!rrc_fill_buffer(&UE_rrc_inst[Mod_id].Srb2[Idx2].Tx_buffer,(char*)&Def_meas_req,DEFAULT_MEAS_REQ_SIZE))
      
      // UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload[0]=RAB_MEAS_IND;
 	 Meas_ind.Meas_status=MEAS_REPORT;
	 memcpy(&Meas_ind.Meas,(MAC_MEAS_T*)&Meas_entry->Mac_meas_req.Mac_meas,MAC_MEAS_T_SIZE);
	 memcpy(&UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload[W_IDX],(char*)&Meas_ind,MAC_MEAS_IND_SIZE);
	 W_IDX+=MAC_MEAS_IND_SIZE;
	 UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.R_idx++;
	 Meas_entry->Rx_activity=0;
	 Meas_entry->Next_check_frame+=Meas_entry->Mac_meas_req.Rep_interval;
	 Meas_entry->Last_report_frame=Rrc_xface->Frame_index;
	 
	 
	 //Mac_rlc_xface->rrc_rlc_data_req(Mod_id+NB_CH_INST,UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Srb_id,mui++,0,MAC_MEAS_IND_SIZE+1,UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload);
	 //	  msg("[NODE %d][RRC_MESH_XFACE] Meas on LC %d wrote on TX Buffer of SRB %d, W_idx = %d \n",NODE_ID[Mod_id+NB_CH_INST],Meas_ind.Lchan_id.Index,UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Srb_id,W_IDX);
	 
	 /*mac_rlc_status_resp_t rlc_status;
	   rlc_status=mac_rlc_status_ind(Mod_id+NB_CH_INST,UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Srb_id,40,1);
	   msg("RLC_STATUS_IND ON RAB %d return %d Bytes\n",UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Srb_id,rlc_status.bytes_in_buffer);
	   exit(0);
	 */
	 //int i;
	 //   for(i=0;i<MAC_MEAS_IND_SIZE;i++)
   //   msg("%d.",Rrc_inst[Mod_id].Srb2[UE_CH_index].Srb_info.Tx_buffer.Payload[i]);
	 //msg("\n");
	 
	 
	 // (module_id_t module_idP, rb_id_t rb_idP, mui_t muiP, confirm_t confirmP, sdu_size_t sdu_sizeP, char* sduP) {
	 
    }
  }
}
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

  ((CH_CCCH_HEADER*)(UE_rrc_inst[Mod_id].Srb0[CH_index].Rx_buffer.Header))->Rv_tb_idx=0;
  //((CH_CCCH_HEADER*)(UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.Header))->Rv_tb_idx=0;

  //  Rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.Rv_tb_idx=0;
  if(UE_rrc_inst[Mod_id].Srb2[CH_index].Active==1){
    msg("[RRC Inst %d] CH_index %d, Remove RB %d\n ",Mod_id,CH_index,UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Srb_id);
    Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_CH_INST,ACTION_REMOVE,UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Srb_id,SIGNALLING_RADIO_BEARER,Rlc_info_um);
    UE_rrc_inst[Mod_id].Srb2[CH_index].Active=0;
    UE_rrc_inst[Mod_id].Srb2[CH_index].Status=IDLE;
    UE_rrc_inst[Mod_id].Srb2[CH_index].Next_check_frame=0;
    if(UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Meas_entry)
      UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Meas_entry->Status=IDLE;
    Mac_config_req.Lchan_id.Index=UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Srb_id;
    Mac_config_req.Lchan_type=DCCH;
    Mac_rlc_xface->mac_config_req(Mod_id+NB_CH_INST,REMOVE_LC,&Mac_config_req);
    if(UE_rrc_inst[Mod_id].Def_meas[CH_index]!= NULL){
      UE_rrc_inst[Mod_id].Def_meas[CH_index]->Status = IDLE;
      UE_rrc_inst[Mod_id].Def_meas[CH_index]->Active = 0;
      UE_rrc_inst[Mod_id].Def_meas[CH_index]->Last_report_frame = 0;
    }

  }
  // mac_release_req(Mod_id,Rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Srb_id,CH_index);
//for (i=0;i<NB_RAB_MAX;i++)   
  //        msg("[RRC][MOD_ID %d] Remove request for RAB %d on In_idx %d, Active=%d, Status %d (%d)\n",Mod_id,UE_rrc_inst[Mod_id].Rab[i][CH_index].Rb_info.Rb_id,i,UE_rrc_inst[Mod_id].Rab[i][CH_index].Active,UE_rrc_inst[Mod_id].Rab[i][CH_index].Status,IDLE);
      
 for (i=0;i<NB_RAB_MAX;i++)
     if(UE_rrc_inst[Mod_id].Rab[i][CH_index].Active==1 ){
            msg("[RRC][MOD_ID %d] Remove request for RAB %d on In_idx %d\n",Mod_id,UE_rrc_inst[Mod_id].Rab[i][CH_index].Rb_info.Rb_id,i);
      Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_CH_INST,ACTION_REMOVE,UE_rrc_inst[Mod_id].Rab[i][CH_index].Rb_info.Rb_id,RADIO_ACCESS_BEARER,Rlc_info_am_config);
      UE_rrc_inst[Mod_id].Rab[i][CH_index].Active=0;
      UE_rrc_inst[Mod_id].Rab[i][CH_index].Status=IDLE;
      UE_rrc_inst[Mod_id].Rab[i][CH_index].Next_check_frame=0;
      if(UE_rrc_inst[Mod_id].Rab[i][CH_index].Rb_info.Meas_entry)
	UE_rrc_inst[Mod_id].Rab[i][CH_index].Rb_info.Meas_entry->Status=IDLE;
      Mac_config_req.Lchan_id.Index=UE_rrc_inst[Mod_id].Rab[i][CH_index].Rb_info.Rb_id;
      Mac_config_req.Lchan_type=DTCH;
      Mac_rlc_xface->mac_config_req(Mod_id+NB_CH_INST,REMOVE_LC,&Mac_config_req);

      //mac_release_req(Mod_id,Rrc_inst[Mod_id].Rab[i][CH_index].Rb_info.Rb_id,CH_index);
    }
    
    UE_rrc_inst[Mod_id].Nb_rb[CH_index]=0;//DTCH BROADCAST
    //mac_release_all_meas_process(Mod_id,CH_index);
//exit(0);
    //Rrc_inst[Mod_id].Nb_rb[i] = 0;

}    



