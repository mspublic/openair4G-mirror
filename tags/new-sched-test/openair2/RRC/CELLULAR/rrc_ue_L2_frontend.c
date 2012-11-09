/***************************************************************************
                          rrc_ue_L2_frontend.c - description
                          -------------------
    begin               : July 2010
    copyright           : (C) 2005, 2010 by Eurecom
    created by		: Michelle.Wetterwald@eurecom.fr 
 **************************************************************************
      This file contain front end functions to isolate L2 interface
 **************************************************************************/
/********************
//OpenAir definitions
 ********************/
#include "LAYER2/MAC/extern.h"
#include "UTIL/MEM/mem_block.h"

/********************
// RRC definitions
 ********************/
#include "rrc_ue_vars.h"

// Configuration Functions for L1-L2 layers
//-----------------------------------------------------------------------------
void crb_config_req (int activation_timeP){
//-----------------------------------------------------------------------------
  #ifdef RRC_DEBUG_DUMMIES
  msg ("\n[RRC][RG-DUMMIES] DUMMY CALL to crb_config_req\n");
  #endif
}
//-----------------------------------------------------------------------------
void cmac_config_req (int userP, int activation_timeP){
//-----------------------------------------------------------------------------
  #ifdef RRC_DEBUG_DUMMIES
  msg ("\n[RRC][RG-DUMMIES] DUMMY CALL to cmac_config_req\n");
  #endif
}
//-----------------------------------------------------------------------------
void CPHY_config_req (void *config, int activation_time, int userP){
//-----------------------------------------------------------------------------
  #ifndef BYPASS_L1
  #ifdef RRC_DEBUG_DUMMIES
  msg ("\n[RRC][RG-DUMMIES] DUMMY CALL to CPHY_config_req\n");
  #endif
  #endif
}

// Functions for data transmission
//-------------------------------------------------------------------
int rrc_ue_test_rlc_intf_xmit_dcch (void){
//-----------------------------------------------------------------------------
  //char *tx_dcch_info = {"This is a sample data to test the interface with the RLC module. Check segmentation and transmission"};
  char *tx_dcch_info = {"DYNAMIC ACCESS REQUEST"};
  int data_length = 0;
  int retcode;

  #ifdef DEBUG_RRC_TEMP_OPENAIR
  msg ("\n[RRC][UE-DUMMIES] TEMP - CALL to rrc_ue_test_rlc_intf_xmit_dcch\n");
  #endif
  data_length=strlen(tx_dcch_info);
    //retcode = rb_tx_data_srb_rg (RRC_SRB2_ID + (msgId * maxRB), tx_dcch_info, data_length * 8, protocol_bs->rrc.next_MUI++, TRUE);

//  Mac_rlc_xface->rrc_rlc_data_req (Mod_id+NB_CH_INST, UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Srb_id, mui++, 0, W_IDX, UE_rrc_inst[Mod_id].Srb2[Idx2].Srb_info.Tx_buffer.Payload);
  retcode = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LTE_DCCH_ID, protocol_ms->rrc.next_MUI++, 0, data_length, tx_dcch_info);
  #ifdef DEBUG_RRC_TEMP_OPENAIR
  msg ("[RRC][UE-DUMMIES] rrc_ue_test_rlc_intf_xmit_dcch -- retcode = %d\n",retcode);  //RC = 1 ==> OK
  #endif
  return 0;
}


//-------------------------------------------------------------------
int rrc_ue_force_uplink (void){
//-----------------------------------------------------------------------------
  //TEST ACCESS communication
  if ((Mac_rlc_xface->frame%5) == 2){
      rrc_ue_test_rlc_intf_xmit_dcch ();
  }
}


// Unified function to send data 
//-----------------------------------------------------------------------------
int rrc_ue_send_to_srb_rlc (int rb_id, char * data_buffer, int data_length){
//-----------------------------------------------------------------------------
  char tx_data[500];
  int stxtlen = 0;
  int result;
  #ifdef RRC_DEBUG_DETAILS
  msg ("\n[RRC-UE] Send  Data to RLC, srb %d\n",rb_id);
  #endif

  switch (rb_id){
    case RRC_SRB0_ID: //CCCH
      //retcode = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB0_ID, 0, RRC_RLC_CONFIRM_NO, data_length, tx_ccch_info);
      result = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB0_ID,  0, RRC_RLC_CONFIRM_NO, data_length, data_buffer);
      break;
    case RRC_SRB1_ID: //DCCH-UM
      //retcode = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB1_ID, 0, RRC_RLC_CONFIRM_NO, data_length, tx_dcch_info);
      result = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB1_ID, 0, RRC_RLC_CONFIRM_NO, data_length, data_buffer);
      break;
    case RRC_SRB2_ID: //DCCH-AM
      // multiplex message with SRB3 - TEMP OPENAIR
      memset(tx_data,0,500);
      tx_data[0] = RRC_SRB2_ID;
      memcpy ((char*)&tx_data[1],data_buffer, data_length);
      data_length = data_length +1;
      //retcode = rb_tx_data_srb_mt (RRC_SRB2_ID, tx_dcch_info, data_length * 8, protocol_ms->rrc.next_MUI++, TRUE);
      result = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB2_ID, protocol_ms->rrc.next_MUI++, RRC_RLC_CONFIRM_YES, data_length, tx_data);
      break;
    case RRC_SRB3_ID: //DCCH-AM - NAS
      // multiplex message with SRB2 - TEMP OPENAIR
      memset(tx_data,0,500);
      tx_data[0] = RRC_SRB3_ID;
      memcpy ((char*)&tx_data[1],data_buffer, data_length);
      data_length = data_length +1;
      //retcode = rb_tx_data_srb_mt (RRC_SRB3_ID, tx_dcch_info, data_length * 8, protocol_ms->rrc.next_MUI++, TRUE);
      result = Mac_rlc_xface->rrc_rlc_data_req (RRC_MODULE_INST_ID, RRC_LCHAN_SRB2_ID, protocol_ms->rrc.next_MUI++, RRC_RLC_CONFIRM_YES, data_length, tx_data);
      break;
    default:
       msg ("\n[RRC] ERROR - Unable to send data to RLC, Channel srb %d not supported\n",rb_id);
  }
  if (result !=1)
       msg ("\n[RRC] ERROR - RLC returned an error code %d", result);

  return result;

}
