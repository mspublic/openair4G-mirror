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
  //    msg("filling buffer with size %d, ccch_tx_idx %d\n",Size,Rrc_inst[0].Srb1[0].Tx_buffer.W_idx);
  return Size;
}
//------------------------------------------------------------------------------------------------------------------//
unsigned char mac_rrc_mesh_data_req( unsigned char Mod_id, 
				     unsigned short Srb_id, 
				     unsigned char Nb_tb,
				     char *Buffer,
				     u8 CH_index){
    //------------------------------------------------------------------------------------------------------------------//

#ifdef DEBUG_RRC
  msg("[OPENAIR][RRC]Mod_id=%d: mac_rrc_data_req to SRB ID=%ld, params: \n",Mod_id,Srb_id,CH_index);
#endif
  // msg("%d, %d\n",Mod_id, Srb_id);
  // msg("%d, %p\n ",Nb_tb,&Buffer[0]);
  //   msg("%d\n",CH_index);
  SRB_INFO *Srb_info;
  
  if( Mac_rlc_xface->Is_cluster_head[Mod_id]){
    u8 H_size,i;  
    u16 tmp;
    if((Srb_id & RAB_OFFSET) == BCCH){
#ifdef DEBUG_RRC
      msg("[OPENAIR][RRC] BCCH request\n");
#endif
      H_size=CH_BCCH_HEADER_SIZE;
      if(CH_rrc_inst[Mod_id].Srb0.Active==0) return 0;
      Srb_info=&CH_rrc_inst[Mod_id].Srb0;
      H_size = CH_BCCH_HEADER_SIZE;
      memcpy(&Buffer[0],&Srb_info->Tx_buffer.Header[0],H_size);

      /*
      if(Srb_info->Tx_buffer.R_idx == Srb_info->Tx_buffer.W_idx){//Fill buffer
	Srb_info->Tx_buffer.R_idx=0;
	ch_rrc_generate_bcch(Mod_id);

			
      //	tmp=( (Srb_info->Tx_buffer.W_idx)/(Srb_info->Tx_buffer.Tb_size-CH_BCCH_HEADER_SIZE));
	//if( tmp * (Srb_info->Tx_buffer.Tb_size-H_size) < (Srb_info->Tx_buffer.W_idx)  )
//	  tmp++;  
//	((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx = tmp << 4;
//	if(tmp>0) ((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx--;
	//		msg("Nb_fragment=%d, Rv_tb_index %d, W_idx %d\n",tmp,((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx,Srb_info->Tx_buffer.W_idx);
	
      }
      
      
      //if(Srb_info->Tx_buffer.W_idx != Srb_info->Tx_buffer.R_idx){//Something to send (at least header)
      if((Srb_info->Tx_buffer.W_idx+H_size) > (Srb_info->Tx_buffer.R_idx + Srb_info->Tx_buffer.Tb_size)){
#ifdef DEBUG_RRC
	msg("[eNB RRC] Getting asked data from buffer of size %d (H_size %d)\n", Nb_tb*Srb_info->Tx_buffer.Tb_size,H_size);
#endif
	((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx+=Nb_tb;
	((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Tb_data_size=(Nb_tb*Srb_info->Tx_buffer.Tb_size);
	  memcpy(Buffer,(char *)&Srb_info->Tx_buffer.Header[0],H_size);
	  memcpy(&Buffer[H_size],
		 (char *)&Srb_info->Tx_buffer.Payload[(Srb_info->Tx_buffer.R_idx)],
		 Nb_tb*Srb_info->Tx_buffer.Tb_size-H_size);		    
	  Srb_info->Tx_buffer.R_idx +=(Nb_tb*Srb_info->Tx_buffer.Tb_size)-H_size;
      }
      else{
#ifdef DEBUG_RRC
	msg("Getting part (at least header) of asked data from buffer of size %d, H_size %d\n"
	 , Srb_info->Tx_buffer.W_idx -(Srb_info->Tx_buffer.R_idx-H_size),H_size );
#endif
	tmp = (((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx >> 4);
	if(tmp>0)
	  ((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx 
	    = (((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx & 0xf0) + (--tmp);
	else
	  ((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx 
	    = (((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx & 0xf0) + (tmp);
	
	((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Tb_data_size 
	  =  Srb_info->Tx_buffer.W_idx -(Srb_info->Tx_buffer.R_idx-H_size);
	memcpy(&Buffer[0],&Srb_info->Tx_buffer.Header[0],H_size);
	
//	memcpy(&Buffer[H_size],
//	       &Srb_info->Tx_buffer.Payload[(Srb_info->Tx_buffer.R_idx)],
//	       (Srb_info->Tx_buffer.W_idx - Srb_info->Tx_buffer.R_idx));
	
	//  msg("[CH][BCCH SEND]Rv_tb_idx=%d\n",((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx);
	  Srb_info->Tx_buffer.R_idx = 0;
	  Srb_info->Tx_buffer.W_idx = 0;	  
      }
*/

      /*
       msg("[TX] BCCH_HEADER DUMP\n");
       for(i=0;i<H_size;i++)
	 msg("%d.",Srb_info->Tx_buffer.Header[i]);
       msg("\n");
      */
      //	return (((CH_BCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Tb_data_size);
	return (CH_BCCH_HEADER_SIZE);
    }
	
    
    if( (Srb_id & RAB_OFFSET ) == CCCH){
      msg("[OPENAIR][CCCH] request (Srb_id %d)\n",Srb_id);
      H_size = CH_CCCH_HEADER_SIZE;
      if(CH_rrc_inst[Mod_id].Srb1.Active==0) return 0;
      Srb_info=&CH_rrc_inst[Mod_id].Srb1;
      if(Srb_info->Tx_buffer.R_idx == Srb_info->Tx_buffer.W_idx){//Fill buffer
	Srb_info->Tx_buffer.R_idx=0;
	ch_rrc_generate_ccch(Mod_id);// return 0;
	
	tmp=((Srb_info->Tx_buffer.W_idx)/(Srb_info->Tx_buffer.Tb_size-H_size));
	if(( tmp * (Srb_info->Tx_buffer.Tb_size -H_size)) < (Srb_info->Tx_buffer.W_idx)  )
	  tmp++;  
	if(tmp > 256 )  {msg("RRC: SEGMENT ERROR; tmp %d, W_idx %d\n",tmp,Srb_info->Tx_buffer.W_idx);Mac_rlc_xface->macphy_exit("");}
	((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx = (tmp << 8);
	if(tmp>0) ((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx--;
	//	msg("[RRC]: Nb_fragment=%d, Rv_tb_index %d, W_idx %d\n",tmp,((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx,Srb_info->Tx_buffer.W_idx);
      }
      
      
      //if(Srb_info->Tx_buffer.W_idx != Srb_info->Tx_buffer.R_idx ){//Something to send (at least header), only one TB per req!!!!
      if( (Srb_info->Tx_buffer.W_idx-Srb_info->Tx_buffer.R_idx) > (Srb_info->Tx_buffer.Tb_size-H_size)){
	((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx+=Nb_tb;
	((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Tb_data_size=(Nb_tb*Srb_info->Tx_buffer.Tb_size);
	memcpy(Buffer,(char *)&Srb_info->Tx_buffer.Header[0],H_size);
	memcpy(&Buffer[H_size],
	       (char *)&Srb_info->Tx_buffer.Payload[Srb_info->Tx_buffer.R_idx],
	       (Nb_tb*Srb_info->Tx_buffer.Tb_size)-H_size);//////////////////////////////////////Modify later		    
	Srb_info->Tx_buffer.R_idx +=(Nb_tb*Srb_info->Tx_buffer.Tb_size)-H_size;
#ifdef RRC_DATA_REQ_DEBUG
	msg("[RRC_L2][GEN CCCH]Frame %d, NODE %d: Getting asked data from buffer of size %d from %d Bytes, remaigning Bytes %d\n", 
	    Rrc_xface->Frame_index,
	    NODE_ID[Mod_id],
	    Srb_info->Tx_buffer.Tb_size,
	    Srb_info->Tx_buffer.W_idx,
	    Srb_info->Tx_buffer.W_idx-Srb_info->Tx_buffer.R_idx);
	//      msg("[CH][CCH_SEND] Rv_tb_idx=%d\n",((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx);
#endif
      }
      else{
#ifdef RRC_DATA_REQ_DEBUG
	msg("[RRC] Frame %d, NODE %d: Getting last part of asked data from buffer of size %d\n"
	    ,Rrc_xface->Frame_index,NODE_ID[Mod_id], Srb_info->Tx_buffer.W_idx -Srb_info->Tx_buffer.R_idx+H_size);
#endif
	tmp = (((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx >> 8);
	//msg("[RRC] tmp %d MSB %d\n",tmp,(((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx & 0xf0));
	if(tmp>0)
	  ((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx 
	    = (((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx & 0xff00) + (tmp-1);
	else
	  ((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx 
	    = (((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx & 0xff00) + (tmp);
	
	((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Tb_data_size 
	  =  Srb_info->Tx_buffer.W_idx -(Srb_info->Tx_buffer.R_idx-H_size);
	memcpy(Buffer,&Srb_info->Tx_buffer.Header[0],H_size);
      memcpy(&Buffer[H_size],
	     &Srb_info->Tx_buffer.Payload[(Srb_info->Tx_buffer.R_idx)],
	     (Srb_info->Tx_buffer.W_idx - Srb_info->Tx_buffer.R_idx));
      //            msg("[CH][CCCH SEND]Rv_tb_idx=%d\n",((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Rv_tb_idx);
      Srb_info->Tx_buffer.R_idx = 0;
      Srb_info->Tx_buffer.W_idx = 0;
      }
      
      // else{
  //msg("FATAL_ERROR, EVEN NO CCCH HEADER TO TRANSMIT!!!!\n");
  //return 0;
  // }
    //msg("delivered msg :\n");
    //for ( tmp = H_size ; tmp < ((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Tb_data_size ; tmp ++)
    //	msg("%x.",Buffer[tmp]);
    //msg("\n");
      if(((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Tb_data_size==0){
	msg("[RRC] generate CCCH of SIZE %d!!!!\n",((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Tb_data_size);
	Mac_rlc_xface->macphy_exit("");
	  }
    return ((CH_CCCH_HEADER*)(Srb_info->Tx_buffer.Header))->Tb_data_size;
    }
  
    
  
  }
  

  else{   Mod_id-=NB_CH_INST; //This is an UE
    msg("filling rach,SRB_ID %d\n",Srb_id);
    msg("Buffers status %d,\n",UE_rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.W_idx);
    if( (UE_rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.W_idx != UE_rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.R_idx) ){
      memcpy(&Buffer[0],&UE_rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.Payload[0],UE_rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.W_idx);
      u8 Ret_size=UE_rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.W_idx;
      UE_rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.W_idx=0;
      msg("[LE_XFACE]Frame %d: sending rach from NODE %d\n",Rrc_xface->Frame_index,NODE_ID[Mod_id+NB_CH_INST]);
      return(Ret_size);
    }
    else{
      //      msg("erooooooooooooooooor\n");
      return 0;
    }
  }
  
}

//--------------------------------------------------------------------------------------------//
u8 mac_rrc_mesh_data_ind(u8 Mod_id, u16 Srb_id, char *Sdu,u8 CH_index ){ 
  //------------------------------------------------------------------------------------------//
  // msg("[OPENAIR][RRC]Node =%d: mac_rrc_data_ind to SRB ID=%ld, CH_UE_INDEX %d...\n",NODE_ID[Mod_id],Srb_id,CH_index);
  //SRB_INFO *Srb_info=rrc_find_srb_info(Mod_id,Srb_id);
  SRB_INFO *Srb_info;
  //unsigned short Idx2=(Srb_id & RAB_OFFSET2) >> RAB_SHIFT2;
  //if(Mac_rlc_xface->Is_cluster_head[Mod_id]==1)
	//Srb_info=&CH_rrc_inst[Srb]
  /*  if(Srb_info == NULL) {
    msg("[OPENAIR][RRC] Frame %d: RRC_DATA_RX: FATAL ERROR: NO SRB to Rx\n",Srb_id);
#ifndef USER_MODE
    mac_xface->macphy_exit();
#else
    exit(-1);
#endif
    return 0;
    }*/
  unsigned short Rv_tb_idx_last,Rv_tb_idx,Size,i;
  if(!Mac_rlc_xface->Is_cluster_head[Mod_id]){
    Mod_id-=NB_CH_INST;
    // msg("test %d\n",((((Srb_id >> 1)-Rrc_inst[Mod_id].NODE_ID)& 0xff00)>>8));
    
    if((Srb_id & RAB_OFFSET) == BCCH){
      Srb_info = &UE_rrc_inst[Mod_id].Srb0[CH_index];
      //      Rv_tb_idx_last =((CH_BCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx;
      //  msg("[OPNEAIR][RRC] RX_BCCH_DATA, Last Rv_tb_idx %d...\n",Rv_tb_idx_last);
      //      memcpy(&Srb_info->Rx_buffer.Header[0],(CH_BCCH_HEADER*)Sdu,CH_BCCH_HEADER_SIZE);
      memcpy(&Srb_info->Rx_buffer.Header[0],&Sdu[0],CH_BCCH_HEADER_SIZE);
      /*
      Srb_info->Header_rx=1;
      Size=((CH_BCCH_HEADER*)(Srb_info->Rx_buffer.Header))->Tb_data_size;
      if(Size==0){ msg("BCCH SIZE 0\n");return;}
      Rv_tb_idx=((CH_BCCH_HEADER*)(Srb_info->Rx_buffer.Header))->Rv_tb_idx;
      //    msg("RV_TB_IDX=%d, RX_idx_last %d, data_size(inc H) %d\n",Rv_tb_idx,Rv_tb_idx_last,Size);
      if( (Rv_tb_idx_last && ( (Rv_tb_idx_last+1) != Rv_tb_idx )) || (!Rv_tb_idx_last &&  ((((CH_BCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx) & 0x0f)!=0 )){
	//	msg("[OPENAIR][RRC] DATA_RX: Segmentation Lost!!!, SRB: {%d} , last %x, new %x\n",Srb_id,Rv_tb_idx_last,Rv_tb_idx);
	//	Mac_rlc_xface->macphy_exit("");
	//rrc_reset_buffer(&Srb_info->Rx_buffer);
	((CH_BCCH_HEADER*)(Srb_info->Rx_buffer.Header))->Rv_tb_idx=0;
	Srb_info->Rx_buffer.W_idx=0;
	Srb_info->Rx_buffer.R_idx=0;
	
	return 0;        
      }
      if(((Srb_info->Rx_buffer.Nb_tb_max * Srb_info->Rx_buffer.Tb_size) 
	  - Srb_info->Rx_buffer.W_idx) < Size){
	msg("[OPENAIR][RRC] DATA_RX: Buffer overflow!!!, SRB: {%d}, W_idx %d, Size %d \n",Srb_id,Srb_info->Rx_buffer.W_idx,Size);
	((CH_BCCH_HEADER*)(Srb_info->Rx_buffer.Header))->Rv_tb_idx=0;
	Srb_info->Rx_buffer.W_idx=0;
	Srb_info->Rx_buffer.R_idx=0;
	
	//	Mac_rlc_xface->macphy_exit("");
	//	rrc_reset_buffer(&Srb_info->Rx_buffer);
	return 0;
      }   
      //      msg("Header =%p\n",Header);
      //      Size = CH_BCCH_HEADER_SIZE ;
      if( Size > CH_BCCH_HEADER_SIZE ) {
	//msg("[RRC_IND] Copying data, W_idx=%d, Tb_data_size %d, Rx_size\n",Srb_info->Rx_buffer.W_idx,Size);
	//msg("[RRC]BCCH HEADER DUMP\n");
	//u8 tmp;
	//for(tmp=0;tmp<CH_BCCH_HEADER_SIZE;tmp++)
	//  msg("[RRC] [%x]\t",Srb_info->Rx_buffer.Header[tmp]);
	//msg("\n");
	//
	msg("header :111: NODE %d, CH %d with sie %d (%d)\n",NODE_ID[Mod_id+NB_CH_INST],CH_index,Size,CH_BCCH_HEADER_SIZE);
	Mac_rlc_xface->macphy_exit("");
		//msg("header :111\n");
		memcpy(&Srb_info->Rx_buffer.Payload[Srb_info->Rx_buffer.W_idx],(char *)&Sdu[CH_BCCH_HEADER_SIZE],
	       Size - CH_BCCH_HEADER_SIZE);
		Srb_info->Rx_buffer.W_idx+= Size - CH_BCCH_HEADER_SIZE;
      }
      int tmp;
      //msg("[OPENAIR][RRC]DATA_IND: DATA TO RECEIVE!!!!\n");
      //for(tmp=CH_CCCH_HEADER_SIZE;tmp<Size;tmp++)
      //msg("%x.",Sdu[tmp]);
      //msg("\n");
      //       msg("LSB %d, MSB %d \n", ((((CH_CCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx) & 0x0f) <<4  , ((((CH_CCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx )) & 0xf0  );
      
      
      
      if( (Rv_tb_idx_last) &&(((Rv_tb_idx+1) & 0x0f)<< 4) == (Rv_tb_idx & 0xf0 ) ){
	//	msg("calling decode ccch!!!\n");
	((CH_BCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx=0;
	//  Srb_info->Rx_buffer.decode_fun(Mod_id);
	if(Srb_info->Rx_buffer.W_idx >0)
	  //	  ue_rrc_decode_bcch(Mod_id,Srb_info,CH_index);
	  Srb_info->Rx_buffer.W_idx=0;
	Srb_info->Rx_buffer.R_idx=0;
	//	rrc_reset_buffer(&Srb_info->Rx_buffer);
	
	//msg("[RX] BCCH_HEADER DUMP, H_size %d\n",CH_BCCH_HEADER_SIZE);
	//for(i=0;i<20;i++)
	//  msg("%d.",Srb_info->Rx_buffer.Header[i]);
	//msg("\n");
	

      }
	*/
      //    msg("dont say you did nothing!!!!\n");
    }
    

    
      
  
    if((Srb_id & RAB_OFFSET) == CCCH){
      Srb_info = &UE_rrc_inst[Mod_id].Srb1[CH_index];
      
      //  msg("[OPNEAIR][RRC] RX_CCCH_DATA...\n");
      Rv_tb_idx_last =((CH_CCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx;
      memcpy(&Srb_info->Rx_buffer.Header[0],&Sdu[0],CH_CCCH_HEADER_SIZE);
      Srb_info->Header_rx=1;
      Size=((CH_CCCH_HEADER*)(Srb_info->Rx_buffer.Header))->Tb_data_size;
      if(Size==0){
	//msg("[RRC] RX_CCCH of size %d, !!!",Size);
	return;
	//Mac_rlc_xface->macphy_exit("");
      }

      Rv_tb_idx=((CH_CCCH_HEADER*)(Srb_info->Rx_buffer.Header))->Rv_tb_idx;
      //msg("[RRC] RV_TB_IDX=%d, RX_idx_last %d, data_size(inc H) %d\n",Rv_tb_idx,Rv_tb_idx_last,Size);
      if( (Rv_tb_idx_last && ( (Rv_tb_idx_last+1) != Rv_tb_idx )) || (!Rv_tb_idx_last &&  ((((CH_CCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx) & 0x00ff)!=0 )){
	//	msg("[OPENAIR][RRC UE %d]TTI %d: DATA_RX from: Segmentation Lost!!!, SRB: {%d} , last %x, new %x\n",
	//  NODE_ID[Mod_id+NB_CH_INST],
	//  Rrc_xface->Frame_index,
	//  Srb_id,Rv_tb_idx_last,(((CH_CCCH_HEADER*)(Srb_info->Rx_buffer.Header))->Rv_tb_idx&0x00ff));
		//Mac_rlc_xface->macphy_exit("");
	//rrc_reset_buffer(&Srb_info->Rx_buffer);
	((CH_CCCH_HEADER*)(Srb_info->Rx_buffer.Header))->Rv_tb_idx=0;
	Srb_info->Rx_buffer.W_idx=0;
	Srb_info->Rx_buffer.R_idx=0;
	
      return 0;        
      }
      if(((Srb_info->Rx_buffer.Nb_tb_max * Srb_info->Rx_buffer.Tb_size) 
	  - Srb_info->Rx_buffer.W_idx) < Size){
	msg("[OPENAIR][RRC] DATA_RX: Buffer overflow!!!, SRB: {%d}, W_idx %d, Size %d \n",Srb_id,Srb_info->Rx_buffer.W_idx,Size);
	((CH_CCCH_HEADER*)(Srb_info->Rx_buffer.Header))->Rv_tb_idx=0;
	Srb_info->Rx_buffer.W_idx=0;
	Srb_info->Rx_buffer.R_idx=0;
	
	//	Mac_rlc_xface->macphy_exit("");
	//	rrc_reset_buffer(&Srb_info->Rx_buffer);
	return 0;
      }   
      //      msg("Header =%p\n",Header);
      if( Size > CH_CCCH_HEADER_SIZE ) {
	if((((((CH_CCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx) & 0x00ff) <<8)==0)//First fragment
	   Srb_info->Rx_buffer.W_idx=0;
	//msg("[RRC Inst%d]CH_index %d C0pying data, W_idx=%d, Tb_data_size %d\n",
	//    Mod_id,CH_index,Srb_info->Rx_buffer.W_idx,Size);
	//msg("header :111\n");
	memcpy(&Srb_info->Rx_buffer.Payload[Srb_info->Rx_buffer.W_idx],(char *)&Sdu[CH_CCCH_HEADER_SIZE],
	       Size - CH_CCCH_HEADER_SIZE);
	Srb_info->Rx_buffer.W_idx+= Size - CH_CCCH_HEADER_SIZE;
	//#ifdef RRC_DATA_IND_DEBUG
	//msg("[OPENAIR][RRC]NODE %d: DATA_RX: SRB: {%d} , TB_IDX: last %x, new %x\n",NODE_ID[Mod_id+NB_CH_INST],Srb_id,Rv_tb_idx_last,Rv_tb_idx);
	//#endif
      }
      int tmp;
      if(Srb_info->Rx_buffer.W_idx==4)
	msg("IN\n");

      //for(tmp=CH_CCCH_HEADER_SIZE;tmp<Size;tmp++)
      //msg("%x.",Sdu[tmp]);
      //msg("\n");
      //msg("Inst %d: CH %d:LSB %d, MSB %d \n",Mod_id, CH_index, ((((CH_CCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx) & 0x00ff) <<8  , ((((CH_CCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx )) & 0xff00  );
      
      
      
	if( ((Rv_tb_idx_last) &&((((Rv_tb_idx+1) & 0x00ff)<< 8) == (Rv_tb_idx & 0xff00 ) ))
	    || ((!Rv_tb_idx_last) &&(!Rv_tb_idx))

        ){
	//	msg("calling decode ccch!!!\n");
	((CH_CCCH_HEADER*)(&Srb_info->Rx_buffer.Header[0]))->Rv_tb_idx=0;
	//  Srb_info->Rx_buffer.decode_fun(Mod_id);
	if(Srb_info->Rx_buffer.W_idx >0)
	  ue_rrc_decode_ccch(Mod_id,Srb_info,CH_index);
	Srb_info->Rx_buffer.W_idx=0;
	Srb_info->Rx_buffer.R_idx=0;
	//	rrc_reset_buffer(&Srb_info->Rx_buffer);
      }
    
      //    msg("dont say you did nothing!!!!\n");
    }
  }

  else{

    Srb_info = &CH_rrc_inst[Mod_id].Srb1;
    //    msg("\n***********************************INST %d Srb_info %p, Srb_id=%d**********************************\n\n",Mod_id,Srb_info,Srb_info->Srb_id);
    memcpy(Srb_info->Rx_buffer.Payload,Sdu,11);
    ch_rrc_decode_ccch(Mod_id,Srb_info);
 }
  //  return Nb_tb;
  
}

//-------------------------------------------------------------------------------------------//
void mac_mesh_sync_ind(u8 Mod_id,u8 Status){
//-------------------------------------------------------------------------------------------//
}

//------------------------------------------------------------------------------------------------------------------//
void rlcrrc_mesh_data_ind( unsigned char Mod_id, u32 Srb_id, u32 sdu_size,char *Buffer){
    //------------------------------------------------------------------------------------------------------------------//
  //msg("[OPENAIR][RRC]Mod_id=%d: rlc_rrc_data_ind to SRB ID=%d, size %d,...\n",Mod_id,Srb_id,sdu_size);
  // usleep(1000000);
  unsigned short Idx2=(Srb_id >> RAB_SHIFT2);
/*  SRB_INFO *Srb_info;// = rrc_find_srb_info(Mod_id,Srb_id);

  if(Mac_rlc_xface->Is_cluster_head[Mod_id]==1)
	Srb_info=&CH_rrc_inst[Mod_id].Srb2[Idx2];
  else
	Srb_info=&UE_rrc_inst[Mod_id].Srb2[Idx2];

    if(Srb_info->Status == IDLE) {
      msg("[OPENAIR][RRC] Frame %d: RRC_DATA_RX: FATAL ERROR: NO SRB to Rx}\n",Srb_id);
      mac_xface->macphy_exit();
    }
    int i;
   
    for(i=0;i<sdu_size;i++)
      msg("%d.",Buffer[i]);
	msg("\n");
    */
  //msg("[NODE %d][RRC_MESH_XFACE] Frame %d: RECEIVED MSG ON DCCH %d, Size %d\n",NODE_ID[Mod_id],Rrc_xface->Frame_index,
  //Srb_id,sdu_size);
  if(Mac_rlc_xface->Is_cluster_head[Mod_id]==1)
    ch_rrc_decode_dcch(Mod_id,Buffer);
  else
    ue_rrc_decode_dcch(Mod_id-NB_CH_INST,Buffer,Idx2);
  
}


#define W_IDX UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.W_idx

/*-------------------------------------------------------------------------------------------*/
void def_meas_ind(u8 Mod_id,u8 Idx2){
  /*-----------------------------------------------------------------------------------------*/
  unsigned char i,j,k=0;
DEFAULT_MEAS_IND Def_meas_ind;
mac_rrc_mesh_meas_ind(Mod_id,UE_rrc_inst[Mod_id].Srb0[Idx2].Meas_entry);
mac_rrc_mesh_meas_ind(Mod_id,UE_rrc_inst[Mod_id].Srb1[Idx2].Meas_entry);
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
  
  UE_rrc_inst[Mod_id].Srb1[CH_index].Rx_buffer.R_idx=0;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Rx_buffer.W_idx=0;
  //  UE_rrc_inst[Mod_id].Srb1[CH_index].Rx_buffer.Rv_tb_idx=0;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.R_idx=0;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.W_idx=0;

  //  ((CH_BCCH_HEADER*)(UE_rrc_inst[Mod_id].Srb0[CH_index].Rx_buffer.Header))->Rv_tb_idx=0;
  ((CH_CCCH_HEADER*)(UE_rrc_inst[Mod_id].Srb1[CH_index].Rx_buffer.Header))->Rv_tb_idx=0;
  //((CH_CCCH_HEADER*)(UE_rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.Header))->Rv_tb_idx=0;

  //  Rrc_inst[Mod_id].Srb1[CH_index].Tx_buffer.Rv_tb_idx=0;
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



