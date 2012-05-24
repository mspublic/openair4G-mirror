/*******************************************************************************

  Eurecom OpenAirInterface 2
  Copyright(c) 1999 - 2010 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file rrc_common.c
* \brief rrc common procedures for eNB and UE
* \author Raymond Knopp and Navid Nikaein
* \date 2011
* \version 1.0 
* \company Eurecom
* \email: raymond.knopp@eurecom.fr and  navid.nikaein@eurecom.fr
*/ 


#include "defs.h"
#include "extern.h"
#include "LAYER2/MAC/extern.h"
#include "COMMON/openair_defs.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "LAYER2/RLC/rlc.h"
#include "COMMON/mac_rrc_primitives.h"
#include "UTIL/LOG/log.h"
#define DEBUG_RRC 1
extern eNB_MAC_INST *eNB_mac_inst;
extern UE_MAC_INST *UE_mac_inst;

//configure  BCCH & CCCH Logical Channels and associated rrc_buffers, configure associated SRBs
void openair_rrc_on(u8 Mod_id,u8 eNB_flag){
  unsigned short i;

  if( eNB_flag == 1){
    LOG_I(RRC,"[eNB %d] OPENAIR RRC IN....\n", Mod_id);

    rrc_config_buffer(&eNB_rrc_inst[Mod_id].SI,BCCH,1);
    eNB_rrc_inst[Mod_id].SI.Active=1;
    rrc_config_buffer(&eNB_rrc_inst[Mod_id].Srb0,CCCH,1);
    eNB_rrc_inst[Mod_id].Srb0.Active=1;
  } else{
    LOG_I(RRC,"[UE %d] OPENAIR RRC IN....\n", Mod_id);
    for(i=0;i<NB_eNB_INST;i++){
      LOG_D(RRC, "[RRC][UE %d] Activating CCCH (eNB %d)\n",Mod_id,i);
      UE_rrc_inst[Mod_id].Srb0[i].Srb_id = CCCH;
      memcpy(&UE_rrc_inst[Mod_id].Srb0[i].Lchan_desc[0],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&UE_rrc_inst[Mod_id].Srb0[i].Lchan_desc[1],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      rrc_config_buffer(&UE_rrc_inst[Mod_id].Srb0[i],CCCH,1);
      UE_rrc_inst[Mod_id].Srb0[i].Active=1;
    }
  }
}

int rrc_init_global_param(void){


  //#ifdef USER_MODE
  //  Rrc_xface = (RRC_XFACE*)malloc16(sizeof(RRC_XFACE));
  //#endif //USRE_MODE

  //  Rrc_xface->openair_rrc_top_init = openair_rrc_top_init;
  //  Rrc_xface->openair_rrc_eNB_init = openair_rrc_eNB_init;
  //  Rrc_xface->openair_rrc_UE_init  = openair_rrc_ue_init;
  //  Rrc_xface->mac_rrc_data_ind     = mac_rrc_data_ind;
  //Rrc_xface->mac_rrc_data_req     = mac_rrc_data_req;
  // Rrc_xface->rrc_data_indP        = (void *)rlcrrc_data_ind;
  //  Rrc_xface->rrc_rx_tx            = rrc_rx_tx;
  //  Rrc_xface->mac_rrc_meas_ind     = mac_rrc_meas_ind;
  //  Rrc_xface->get_rrc_status       = get_rrc_status;

  //Rrc_xface->rrc_get_status = ...

  //  Mac_rlc_xface->mac_out_of_sync_ind=mac_out_of_sync_ind;

#ifndef NO_RRM
  //  Rrc_xface->fn_rrc=fn_rrc;
#endif
  //  LOG_D(RRC, "[RRC]INIT_GLOBAL_PARAM: Mac_rlc_xface %p, rrc_rlc_register %p,rlcrrc_data_ind%p\n",Mac_rlc_xface,Mac_rlc_xface->rrc_rlc_register_rrc,rlcrrc_data_ind);
  /*
  if((Mac_rlc_xface==NULL) || (Mac_rlc_xface->rrc_rlc_register_rrc==NULL) ||
     (rlcrrc_data_ind==NULL)) {
    LOG_E(RRC,"Data structured is not initialized \n");
    return -1;
  }
  */
  rrc_rlc_register_rrc(rlcrrc_data_ind ,NULL); //register with rlc


  DCCH_LCHAN_DESC.transport_block_size=4;
  DCCH_LCHAN_DESC.max_transport_blocks=16;
  DCCH_LCHAN_DESC.Delay_class=1;
  DTCH_DL_LCHAN_DESC.transport_block_size=52;
  DTCH_DL_LCHAN_DESC.max_transport_blocks=20;
  DTCH_DL_LCHAN_DESC.Delay_class=1;
  DTCH_UL_LCHAN_DESC.transport_block_size=52;
  DTCH_UL_LCHAN_DESC.max_transport_blocks=20;
  DTCH_UL_LCHAN_DESC.Delay_class=1;

  Rlc_info_um.rlc_mode=RLC_UM;
  Rlc_info_um.rlc.rlc_um_info.timer_reordering=0;
  Rlc_info_um.rlc.rlc_um_info.sn_field_length=10;
  Rlc_info_um.rlc.rlc_um_info.is_mXch=0;
  //Rlc_info_um.rlc.rlc_um_info.sdu_discard_mode=16;

  Rlc_info_am_config.rlc_mode=RLC_AM;
  Rlc_info_am_config.rlc.rlc_am_info.max_retx_threshold = 255;
  Rlc_info_am_config.rlc.rlc_am_info.poll_pdu           = 8;
  Rlc_info_am_config.rlc.rlc_am_info.poll_byte          = 1000;
  Rlc_info_am_config.rlc.rlc_am_info.t_poll_retransmit  = 15;
  Rlc_info_am_config.rlc.rlc_am_info.t_reordering       = 5000;
  Rlc_info_am_config.rlc.rlc_am_info.t_status_prohibit  = 10;
#ifndef NO_RRM
  if(L3_xface_init())
    return(-1);
#endif

  return 0;
}


#ifndef NO_RRM
/*------------------------------------------------------------------------------*/
int L3_xface_init(void){
/*------------------------------------------------------------------------------*/

  int ret = 0;

#ifdef USER_MODE

  int sock ;
  LOG_D(RRC, "[L3_XFACE] init de l'interface \n");

  if(open_socket(&S_rrc, RRC_RRM_SOCK_PATH, RRM_RRC_SOCK_PATH,0)==-1)
    return (-1);

  if (S_rrc.s  == -1)
    {
      return (-1);
    }

  socket_setnonblocking(S_rrc.s);
  msg("Interface Connected... RRM-RRC\n");
  return 0 ;

#else

  ret=rtf_create(RRC2RRM_FIFO,32768);

  if (ret < 0) {
    msg("[openair][MAC][INIT] Cannot create RRC2RRM fifo %d (ERROR %d)\n",RRC2RRM_FIFO,ret);

    return(-1);
  }
  else{
    msg("[openair][MAC][INIT] Created RRC2RRM fifo %d\n",RRC2RRM_FIFO);
    rtf_reset(RRC2RRM_FIFO);
  }

  ret=rtf_create(RRM2RRC_FIFO,32768);

  if (ret < 0) {
    msg("[openair][MAC][INIT] Cannot create RRM2RRC fifo %d (ERROR %d)\n",RRM2RRC_FIFO,ret);

    return(-1);
  }
  else{
    msg("[openair][MAC][INIT] Created RRC2RRM fifo %d\n",RRM2RRC_FIFO);
    rtf_reset(RRM2RRC_FIFO);
  }

  return(0);

#endif
}
#endif

void rrc_config_buffer(SRB_INFO *Srb_info, u8 Lchan_type, u8 Role){

  Srb_info->Rx_buffer.payload_size = 0;
  Srb_info->Tx_buffer.payload_size = 0;
}


/*------------------------------------------------------------------------------*/
void openair_rrc_top_init(void){
  /*-----------------------------------------------------------------------------*/


  LOG_D(RRC,"[OPENAIR][INIT] Init function start:Nb_INST=%d, NB_UE_INST=%d, NB_eNB_INST=%d\n",NB_INST,NB_UE_INST,NB_eNB_INST);
  LOG_D(RRC,"[OPENAIR][INIT] Init function start:Nb_INST=%d\n",NB_INST);

  UE_rrc_inst = (UE_RRC_INST*)malloc16(NB_UE_INST*sizeof(UE_RRC_INST));
  memset(UE_rrc_inst,0,NB_UE_INST*sizeof(UE_RRC_INST));
  LOG_D(RRC,"ALLOCATE %d Bytes for UE_RRC_INST @ %p\n",(unsigned int)(NB_UE_INST*sizeof(UE_RRC_INST)),UE_rrc_inst);

  eNB_rrc_inst = (eNB_RRC_INST*)malloc16(NB_eNB_INST*sizeof(eNB_RRC_INST));
  memset(eNB_rrc_inst,0,NB_eNB_INST*sizeof(eNB_RRC_INST));
  LOG_D(RRC,"ALLOCATE %d Bytes for eNB_RRC_INST @ %p\n",(unsigned int)(NB_eNB_INST*sizeof(eNB_RRC_INST)),eNB_rrc_inst);

#ifndef NO_RRM
#ifndef USER_MODE

  Header_buf=(char*)malloc16(sizeof(msg_head_t));
  Data=(char*)malloc16(2400);
  Header_read_idx=0;
  Data_read_idx=0;
  Header_size=sizeof(msg_head_t);

#endif //NO_RRM
  Data_to_read=0;
#endif //USER_MODE

}

int mac_get_rrc_lite_status(u8 Mod_id,u8 eNB_flag,u8 index){
  if(eNB_flag == 1)
    return(eNB_rrc_inst[Mod_id].Info.Status[index]);
  else
    return(UE_rrc_inst[Mod_id].Info[index].State);
}

u16 T300[8] = {100,200,300,400,600,1000,1500,2000};
u16 T310[8] = {0,50,100,200,500,1000,2000};
u16 N310[8] = {1,2,3,4,6,8,10,20};
u16 N311[8] = {1,2,3,4,6,8,10,20};

rrc_t310_expiration(u32 frame,u8 Mod_id,u8 eNB_index) {

  if (UE_rrc_inst[Mod_id].Info[eNB_index].State!=RRC_CONNECTED) {
    LOG_D(RRC,"Timer 310 expired, going to RRC_IDLE\n");
    UE_rrc_inst[Mod_id].Info[eNB_index].State=RRC_IDLE;
    UE_rrc_inst[Mod_id].Info[eNB_index].UE_index=0xffff;
    
    UE_rrc_inst[Mod_id].Srb0[eNB_index].Rx_buffer.payload_size=0;
    UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size=0;
    
    UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Rx_buffer.payload_size=0;
    UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Tx_buffer.payload_size=0;
    
    if(UE_rrc_inst[Mod_id].Srb2[eNB_index].Active==1){
      msg("[RRC Inst %d] eNB_index %d, Remove RB %d\n ",Mod_id,eNB_index,UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Srb_id);
      rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_REMOVE,UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Srb_id,SIGNALLING_RADIO_BEARER,Rlc_info_um);
      UE_rrc_inst[Mod_id].Srb2[eNB_index].Active=0;
      UE_rrc_inst[Mod_id].Srb2[eNB_index].Status=IDLE;
      UE_rrc_inst[Mod_id].Srb2[eNB_index].Next_check_frame=0;
    }
  }
  else {  // Restablishment procedure
    LOG_D(RRC,"Timer 310 expired, trying RRCRestablishment ...\n");    
  }
}
    
RRC_status_t rrc_rx_tx(u8 Mod_id,u32 frame, u8 eNB_flag,u8 index){
      
  if(eNB_flag == 0) {
    // check timers
    
    if (UE_rrc_inst[Mod_id].Info[index].T300_active==1) {
      if ((UE_rrc_inst[Mod_id].Info[index].T300_cnt % 10) == 0)
	LOG_D(RRC,"[UE %d][RAPROC] Frame %d T300 Count %d ms\n",Mod_id,frame,
	      UE_rrc_inst[Mod_id].Info[index].T300_cnt);
      if (UE_rrc_inst[Mod_id].Info[index].T300_cnt == T300[UE_rrc_inst[Mod_id].sib2[index]->ue_TimersAndConstants.t300]) {
	UE_rrc_inst[Mod_id].Info[index].T300_active = 0;
	// ALLOW CCCH to be used
	UE_rrc_inst[Mod_id].Srb0[index].Tx_buffer.payload_size=0;
	rrc_ue_generate_RRCConnectionRequest(Mod_id,frame,index);
	return(RRC_ConnSetup_failed);
      }
      UE_rrc_inst[Mod_id].Info[index].T300_cnt++;
    }
    if (UE_rrc_inst[Mod_id].sib2[index]) {
      if (UE_rrc_inst[Mod_id].Info[index].N310_cnt==N310[UE_rrc_inst[Mod_id].sib2[index]->ue_TimersAndConstants.n310]) {
	UE_rrc_inst[Mod_id].Info[index].T310_active=1;
      }
    }
    else { // in case we have not received SIB2 yet
      if (UE_rrc_inst[Mod_id].Info[index].N310_cnt==100) {
	UE_rrc_inst[Mod_id].Info[index].N310_cnt = 0;
	return RRC_PHY_RESYNCH;
      }
    }

    if (UE_rrc_inst[Mod_id].Info[index].T310_active==1) {
      if (UE_rrc_inst[Mod_id].Info[index].N311_cnt ==
	  N311[UE_rrc_inst[Mod_id].sib2[index]->ue_TimersAndConstants.n311]) {
	UE_rrc_inst[Mod_id].Info[index].T310_active=0;
	UE_rrc_inst[Mod_id].Info[index].N311_cnt=0;	
      }
      if ((UE_rrc_inst[Mod_id].Info[index].T310_cnt % 10) == 0)
	LOG_D(RRC,"[UE %d] Frame %d T310 Count %d ms\n",Mod_id,frame,
	      UE_rrc_inst[Mod_id].Info[index].T310_cnt);
      if (UE_rrc_inst[Mod_id].Info[index].T310_cnt == T310[UE_rrc_inst[Mod_id].sib2[index]->ue_TimersAndConstants.t310]) {
	UE_rrc_inst[Mod_id].Info[index].T310_active = 0;
	rrc_t310_expiration(frame,Mod_id,index);
	return(RRC_PHY_RESYNCH);
      }
      UE_rrc_inst[Mod_id].Info[index].T310_cnt++;
    }
  }

  return(RRC_OK);
}
