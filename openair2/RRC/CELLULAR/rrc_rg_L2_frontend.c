/***************************************************************************
                          rrc_rg_L2_frontend.c - description
                          -------------------
    begin               : July 2010
    copyright           : (C) 2010 by Eurecom
    created by		: Michelle.Wetterwald@eurecom.fr 
 **************************************************************************
    This file contain front end functions to isolate L2 interface
 **************************************************************************/
//#define RRC_DEBUG_DUMMIES
/********************
//OpenAir definitions
 ********************/
#include "LAYER2/MAC/extern.h"
#include "UTIL/MEM/mem_block.h"

/********************
// RRC definitions
 ********************/
#include "rrc_rg_vars.h"
//-----------------------------------------------------------------------------
#include "rrc_proto_int.h"
#include "rrc_proto_fsm.h"
#include "rrc_proto_intf.h"
#include "rrc_proto_bch.h"
#include "rrc_proto_mbms.h"


// Configuration Functions for L1-L2 layers
//-----------------------------------------------------------------------------
void crb_config_req (int activation_timeP){
//-----------------------------------------------------------------------------
  //#ifdef RRC_DEBUG_DUMMIES
  msg ("\n[RRC][RG-DUMMIES] DUMMY CALL to crb_config_req\n");
  //#endif
}
//-----------------------------------------------------------------------------
void cmac_config_req (int userP, int activation_timeP){
//-----------------------------------------------------------------------------
  //#ifdef RRC_DEBUG_DUMMIES
  msg ("\n[RRC][RG-DUMMIES] DUMMY CALL to cmac_config_req\n");
  //#endif
}
//-----------------------------------------------------------------------------
void CPHY_config_req (void *config, int activation_time, int userP){
//-----------------------------------------------------------------------------
  //#ifdef RRC_DEBUG_DUMMIES
  msg ("\n[RRC][RG-DUMMIES] DUMMY CALL to CPHY_config_req\n");
  //#endif
}

// Measurement functions
// TBD

// Unified function to send data 
//-----------------------------------------------------------------------------
int rrc_rg_send_to_srb_rlc (int UE_id, int rb_id, char * data_buffer, int data_length){
//-----------------------------------------------------------------------------
  char tx_data[500];
  int stxtlen = 0;
  int result;
  #ifdef RRC_DEBUG_DETAILS
  msg ("\n[RRC-RG] Send  Data to RLC, srb %d\n",rb_id);
  #endif

  switch (rb_id){
    case RRC_BCCH_ID: //BCCH
      // send buffer on BCCH - As of 08/07/2010, this call is actually a NOP, since 
      // broadcast is actually retrieved through the rrc_L2_data_req_rx function
      result = 1;
      break;
    case RRC_SRB0_ID: //CCCH
      // RHODOS26:
      // retcode = rb_tx_data_srb_rg (RRC_SRB0_ID, tx_ccch_info, data_length * 8, 0, FALSE);
      // OpenAIR ATTENTION = Only one Mobile supported  TEMP!!!!!
      //retcode = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB0_ID, 0, RRC_RLC_CONFIRM_NO, data_length, tx_ccch_info);
      result = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB0_ID,  0, RRC_RLC_CONFIRM_NO, data_length, data_buffer);
      break;
    case RRC_SRB1_ID: //DCCH-UM
      // multiplex message with MCCH - TEMP OPENAIR
      memset(tx_data,0,500);
      tx_data[0] = RRC_SRB1_ID;
      memcpy ((char*)&tx_data[1],data_buffer, data_length);
      data_length = data_length +1;
      // TEMPComment - OPENAIR
      //retcode = rb_tx_data_srb_rg (RRC_SRB1_ID + (msgId * maxRB), tx_dcch_info, data_length * 8, 0, FALSE);
      //retcode = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB1_ID, 0, RRC_RLC_CONFIRM_NO, data_length, tx_dcch_info);
      result = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB1_ID, 0, RRC_RLC_CONFIRM_NO, data_length, tx_data);
      break;
    case RRC_SRB2_ID: //DCCH-AM
      // multiplex message with SRB3 - TEMP OPENAIR
      memset(tx_data,0,500);
      tx_data[0] = RRC_SRB2_ID;
      memcpy ((char*)&tx_data[1],data_buffer, data_length);
      data_length = data_length +1;
      // TEMPComment - OPENAIR
      //retcode = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB2_ID, protocol_bs->rrc.next_MUI++, RRC_RLC_CONFIRM_YES, data_length, tx_dcch_info);
      result = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB2_ID, protocol_bs->rrc.next_MUI++, RRC_RLC_CONFIRM_YES, data_length, tx_data);
      break;
    case RRC_SRB3_ID: //DCCH-AM - NAS
      // multiplex message with SRB2 - TEMP OPENAIR
      memset(tx_data,0,500);
      tx_data[0] = RRC_SRB3_ID;
      memcpy ((char*)&tx_data[1],data_buffer, data_length);
      data_length = data_length +1;
      // TEMPComment - OPENAIR
      //retcode = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB2_ID, protocol_bs->rrc.next_MUI++, RRC_RLC_CONFIRM_YES, data_length, tx_dcch_info);
      result = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB2_ID, protocol_bs->rrc.next_MUI++, RRC_RLC_CONFIRM_YES, data_length, tx_data);
      break;
    case RRC_MCCH_ID: //MCCH
      // multiplex message with SRB1 - TEMP OPENAIR
      memset(tx_data,0,500);
      tx_data[0] = RRC_MCCH_ID;
      memcpy ((char*)&tx_data[1],data_buffer, data_length);
      data_length = data_length +1;
      result = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB1_ID, 0, RRC_RLC_CONFIRM_NO, data_length, tx_data);
      break;
    default:
       msg ("\n[RRC] ERROR - Unable to send data to RLC, Channel srb %d not supported\n",rb_id);
  }
  if (result !=1)
       msg ("\n[RRC] ERROR - RLC returned an error code %d", result);

  return result;

}
