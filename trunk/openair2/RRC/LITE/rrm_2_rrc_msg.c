/*!
*******************************************************************************

\file    	emul_interface.c

\brief   	Emulation des interfaces du RRM (Radio Ressource Manager )

			Cette application d'envoyer des stimuli sur les interfaces RRM:
			    - RRC -> RRM
			    - CMM -> RRM

\author  	BURLOT Pascal

\date    	10/07/08

   
\par     Historique:
			$Author$  $Date$  $Revision$
			$Id$
			$Log$

*******************************************************************************
*/

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



/******************************************************************************/
void  fn_rrc (void){
/******************************************************************************/

  msg_head_t *Header ;
#ifdef USER_MODE
  char *Data;
#else
 int bytes_read;
#endif
 
         L2_ID Mac_id;

  while(1){	

#ifdef USER_MODE
    Header = (msg_head_t *) recv_msg(&S_rrc) ;
    if(Header==NULL) break;
    Data_to_read=Header->size;
#else
    if(Header_read_idx < Header_size){
      bytes_read = rtf_get (RRM2RRC_FIFO,&Header_buf[Header_read_idx],Header_size-Header_read_idx);
      if(bytes_read >0) 
	msg("RRC: GET FIFOS RETURNS %d bytes, header %d\n",bytes_read,Header_read_idx);
      Header_read_idx+=bytes_read;
      if(Header_read_idx == Header_size){
	Header = (msg_head_t *) Header_buf;
	Data_to_read=Header->size;
	msg("RRC: Header read completed, data size %d\n",Data_to_read);
      }
      //msg("[fn_rrc]TTI %d: rcv_msg return Null\n",Rrc_xface->Frame_index);
      else
	break;
    }
#endif

    if (Data_to_read > 0 ){
#ifdef USER_MODE
      Data = (char *) (Header +1) ;
#else
      bytes_read = rtf_get (RRM2RRC_FIFO,&Data[Data_read_idx],Data_to_read);
      if(bytes_read >0) 
	msg("RRC: GET FIFOS RETURNS %d bytes, Data_to_read %d\n",bytes_read,Data_to_read);
      Data_to_read-=bytes_read;
      Data_read_idx+=bytes_read;
      if(Data_to_read > 0 ) break;
      msg("RRC: DATA read completed, data size %d\n",Data_to_read);
      Header_read_idx=0;
      Data_read_idx=0;
      Data_to_read=0;
#endif 
    }

    msg("Got MSG of Type %d on Inst %d\n",Header->msg_type,Header->inst);
    switch ( Header->msg_type )
      { 
      case RRM_INIT_CH_REQ:{
	rrm_init_ch_req_t *p = (rrm_init_ch_req_t *) Data;
	msg( "[RRM]>[RRC][Inst %d]:RRM_INIT_CH_REQ\n",Header->inst);
	rrc_init_ch_req(Header->inst,p);
	break;
      }
      case RRCI_INIT_MR_REQ:{
	  rrci_init_mr_req_t *p = (rrci_init_mr_req_t *) Data;
	  msg( "[RRM]>[RRC][Inst %d]:RRCI_INIT_MR_REQ\n",Header->inst);
	  rrc_init_mr_req(Header->inst,p);
	  break;
      }
      case RRM_RB_ESTABLISH_REQ:{
	send_msg((void *)&S_rrc,msg_rrc_rb_establish_resp(Header->inst,Header->Trans_id));
	msg( "[RRM]>[RRC][Inst %d]:RRM_RB_ESTABLISH_REQ, size %d\n",Header->inst,sizeof(rrm_rb_establish_req_t));
	rrc_config_req(Header->inst,(void*)Data,Header->msg_type,Header->Trans_id);
	break ;
      }
      case RRM_RB_MODIFY_REQ:{
	send_msg((void *)&S_rrc,msg_rrc_rb_modify_resp(Header->inst,Header->Trans_id));
	msg( "[RRM]>[RRC][Inst %d]:RRM_RB_MODIFY_REQ\n",Header->inst);
	rrc_config_req(Header->inst,Data,Header->msg_type,Header->Trans_id);
      }
      case RRM_RB_RELEASE_REQ:{
	send_msg((void *)&S_rrc,msg_rrc_rb_release_resp(Header->inst,Header->Trans_id));
	msg( "[RRM]>[RRC][Inst %d]:RRM_RB_RELEASE_REQ\n",Header->inst);
	rrc_config_req(Header->inst,Data,Header->msg_type,Header->Trans_id);
      }
	  	
      case RRCI_CX_ESTABLISH_RESP:
	{
	  rrci_cx_establish_resp_t *p = (rrci_cx_establish_resp_t *) Data;
	  unsigned char CH_index,i;
	  msg( "[RRCI]>[RRC][Inst %d]:RRCI_CX_ESTABLISH_RESP\n",Header->inst);
	  for(i=0;i<NB_SIG_CNX_UE;i++)
	    if(!bcmp(&UE_rrc_inst[Header->inst-NB_CH_INST].Info[i].CH_mac_id,&p->L2_id,sizeof(L2_ID))){
	      CH_index=i;
	      break;
	    }

	  if(i==NB_SIG_CNX_UE){
	    msg("[RRC] FATAL: CH_INDEX NOT FOUND\n");
	      return;
	  }

	    UE_rrc_inst[Header->inst-NB_CH_INST].Srb2[CH_index].Srb_info.IP_addr_type=p->L3_info_t;
	  if(p->L3_info_t == IPv4_ADDR)
	    memcpy(&UE_rrc_inst[Header->inst-NB_CH_INST].Srb2[CH_index].Srb_info.IP_addr,p->L3_info,4);
	  else
	    memcpy(&UE_rrc_inst[Header->inst-NB_CH_INST].Srb2[CH_index].Srb_info.IP_addr,p->L3_info,16);
	}
	break ;
	
      case RRM_SENSING_MEAS_REQ:
	{
	  //	  rrm_sensing_meas_req_t *p = (rrm_sensing_meas_req_t *) sock_msg ;
	  send_msg((void *)&S_rrc,msg_rrc_sensing_meas_resp(Header->inst,Header->Trans_id));
	  msg( "[RRM]>[RRC][Inst %d]:RRM_SENSING_MEAS_REQ\n",Header->inst);
	  // rrc_meas_req(header->inst,p,RRC_MEAS_ADD);
	}
	break ;
	

      case RRM_SENSING_MEAS_RESP:
	{
	    msg( "[RRM]>[RRC][Inst %d]:RRM_SENSING_MEAS_RESP\n",Header->inst);
	    //rrm_rrc_meas_resp(header->inst,header->Trans_id);
	}
	break ;


      case RRM_SCAN_ORD:
	    msg( "[RRM]>[RRC][Inst %d]:RRM_SCAN_ORD\n",Header->inst);
	    //memcpy(&CH_rrc_inst[0].Rrm_init_scan_req,(rrm_init_scan_req_t *) Data,sizeof(rrm_init_scan_req_t));
	    //CH_rrc_inst[0].Last_scan_req=Rrc_xface->Frame_index;
	    ///send over air

	    break; 
      case RRM_INIT_SCAN_REQ:
	    msg( "[RRM]>[RRC][Inst %d]:RRM_INIT_SCAN_REQ\n",Header->inst);
	    memcpy(&CH_rrc_inst[0].Rrm_init_scan_req,(rrm_init_scan_req_t *) Data,sizeof(rrm_init_scan_req_t));
	    CH_rrc_inst[0].Last_scan_req=Rrc_xface->Frame_index;
	    ///send over air

	    break;      
      case RRM_END_SCAN_REQ:

	msg( "[RRM]>[RRC][Inst %d]:RRM_END_SCAN_REQ\n",Header->inst);
	memcpy(&Mac_id.L2_id[0],Data,sizeof(L2_ID));
	unsigned char UE_index=Mac_id.L2_id[0]-NB_CH_MAX+1;
	

	    UE_rrc_inst[0].Srb2[UE_index].Srb_info.Tx_buffer.Payload[0]=100;
	msg("SRB_ID %d\n",CH_rrc_inst[0].Srb2[UE_index].Srb_info.Srb_id);
	Mac_rlc_xface->rrc_rlc_data_req(0,CH_rrc_inst[0].Srb2[UE_index].Srb_info.Srb_id,0,0,1,CH_rrc_inst[0].Srb2[UE_index].Srb_info.Tx_buffer.Payload);
	    //CH_rrc_inst[0].Last_scan_req=Rrc_xface->Frame_index;
	    ///send over air

	break; 

      default :
	msg("[L3_xface]WARNING: msg unknown %d\n",Header->msg_type) ;
      } 
	
  }
}



