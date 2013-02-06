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
#include "rrc_proto_intf.h"

extern rlc_info_t Rlc_info_um;
extern rlc_info_t Rlc_info_am_config;

/*  global variables copied from RRC LITE for logical channels */
long logicalChannelGroup0 = 0;
long  logicalChannelSR_Mask_r9=0;

struct LogicalChannelConfig__ul_SpecificParameters LCSRB1 =  {1,
							      LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity,
							      0,
							      &logicalChannelGroup0};

struct LogicalChannelConfig__ul_SpecificParameters LCSRB2 =  {3,
							      LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity,
							      0,
							      &logicalChannelGroup0};

// These are the default SRB configurations from 36.331 (Chapter 9, p. 176-179 in v8.6)

LogicalChannelConfig_t  SRB1_logicalChannelConfig_defaultValue = {&LCSRB1
#ifdef Rel10
								  , &logicalChannelSR_Mask_r9
#endif
                                                                 };

LogicalChannelConfig_t SRB2_logicalChannelConfig_defaultValue = {&LCSRB2
#ifdef Rel10
								 , &logicalChannelSR_Mask_r9
#endif
                                                                 };
/*  END - global variables copied from RRC LITE for logical channels */

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
  return 0;
}

// Unified function to send data 
//-----------------------------------------------------------------------------
int rrc_ue_send_to_srb_rlc (int rb_id, char * data_buffer, int data_length){
//-----------------------------------------------------------------------------
  char tx_data[500];
  int stxtlen = 0;
  int result =0;
  int Mod_id =0;
  int eNB_flag = 0; //1=eNB, 0=UE
  int srb1 =1;
  #ifdef DEBUG_RRC_STATE
  msg ("\n[RRC-UE-FRONTEND] Send Data to RLC, srb %d\n",rb_id);
  rrc_print_buffer (data_buffer, data_length);
  #endif

  // OpenAirInterface, as of 02/01/2013, requires passing all the RRC CELL srb through the DCCH
  // Multiplexing is performed by adding the srb_id as first byte of data buffer
  memset(tx_data,0,500);
  tx_data[0] = rb_id;
  memcpy ((char*)&tx_data[1],data_buffer, data_length);
  data_length = data_length +1;

  switch (rb_id){
    case RRC_SRB0_ID: //CCCH
    case RRC_SRB1_ID: //DCCH-UM
      //result = rrc_rlc_data_req(Mod_id+NB_eNB_INST, protocol_ms->rrc.current_SFN, eNB_flag, srb1, protocol_ms->rrc.next_MUI++, RRC_RLC_CONFIRM_NO, data_length, tx_data);
      if (pdcp_data_req(Mod_id+NB_eNB_INST,protocol_ms->rrc.current_SFN, eNB_flag,srb1,protocol_ms->rrc.next_MUI++,RRC_RLC_CONFIRM_NO,data_length,tx_data,1))
        result = 1;
      break;
    case RRC_SRB2_ID: //DCCH-AM
    case RRC_SRB3_ID: //DCCH-AM - NAS
      //result = rrc_rlc_data_req(Mod_id+NB_eNB_INST,protocol_ms->rrc.current_SFN, eNB_flag, srb1,protocol_ms->rrc.next_MUI++, RRC_RLC_CONFIRM_YES, data_length, tx_data);
      protocol_ms->rrc.rrc_ue_ackSimu_mui = protocol_ms->rrc.next_MUI;
      if (pdcp_data_req(Mod_id+NB_eNB_INST,protocol_ms->rrc.current_SFN, eNB_flag,srb1,protocol_ms->rrc.next_MUI++,RRC_RLC_CONFIRM_YES,data_length,tx_data,1)){
        result = 1;
        protocol_ms->rrc.rrc_ue_ackSimu_flag = 1;
        protocol_ms->rrc.rrc_ue_ackSimu_srbid = rb_id;
      }
      break;
    default:
       msg ("\n[RRC-UE-FRONTEND] ERROR - Unable to send data to RLC, Channel srb %d not supported\n",rb_id);
  }
  if (result !=1)
       msg ("\n[RRC-UE-FRONTEND] ERROR - RLC returned an error code %d\n", result);

  return result;

}

//-----------------------------------------------------------------------------
int rrc_ue_receive_from_srb_rlc (char* sduP, u8 ch_idP, unsigned int Sdu_size){
//-----------------------------------------------------------------------------
  int srb_id, rb_id;
  int UE_Id;
  int sdu_offset=0;

  #ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC-UE-FRONTEND] Begin rrc_ue_receive_from_srb_rlc, lchannel %d\n", ch_idP);
  rrc_print_buffer ((char*)&sduP[0], Sdu_size);
  #endif
  // get UE_Id
  rb_id = ch_idP;
  srb_id = rb_id % MAX_NUM_RB;
  UE_Id = (rb_id - srb_id) / MAX_NUM_RB;
  // get RRC_CELL srb_id
  srb_id = sduP[0];


  switch (srb_id){
    case RRC_SRB0_ID: //CCCH
    case RRC_SRB1_ID: //DCCH-UM
    case RRC_SRB2_ID: //DCCH-AM
    case RRC_SRB3_ID: //DCCH-AM - NAS
      rrc_ue_srb_rx ((char*)&sduP[1], srb_id, UE_Id);
      break;
    default:
       msg ("\n[RRC-UE-FRONTEND] Invalid Channel srb  number %d\n",srb_id);
  }

  return 0;
}

//-----------------------------------------------------------------------------
void rrc_ue_simu_receive_ack_from_rlc (void){
//-----------------------------------------------------------------------------
  unsigned char Mod_id =0;

  rrc_L2_rlc_confirm_ind_rx (Mod_id, protocol_ms->rrc.rrc_ue_ackSimu_srbid, protocol_ms->rrc.rrc_ue_ackSimu_mui);
  protocol_ms->rrc.rrc_ue_ackSimu_mui = 0;
  protocol_ms->rrc.rrc_ue_ackSimu_flag = 0;
  protocol_ms->rrc.rrc_ue_ackSimu_srbid = 0;
}

/*****************************************
 Configuration Functions for L1-L2 layers
 *****************************************/
//-----------------------------------------------------------------------------
void  rrc_ue_L2_setupFachRach(void){
//-----------------------------------------------------------------------------
  rrc_ue_config_common_channels ();

  rrc_ue_xmit_ccch();
}

//-----------------------------------------------------------------------------
void rrc_ue_config_common_channels (void){
//-----------------------------------------------------------------------------
  int Mod_id =0;
  int eNB_flag=0;
  int UE_index=0;
  int eNB_index=0;
//  int eNB_index = 0;
//  int Mod_id = 0;

  u8 result;
  u8 SIwindowsize=1;
  u16 SIperiod=8;

  int rrc_sizeof_SIB2=0;

// Simulate decoding SIB1
  #ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC-UE-FRONTEND] rrc_ue_config_common_channels \n");
  #endif

  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-UE-FRONTEND] do_SIB1_TDD_config_cell\n");
  #endif
  // Initialize TDD_config parameters (TDD_Config_t tdd_Config;) calling asn1_msg.c
  result = do_SIB1_TDD_config_cell (mac_xface->lte_frame_parms, &protocol_ms->rrc.ue_bch_asn1.tdd_Config);

  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-UE-FRONTEND] Frame TDD config %d, protocol_ms value %d\n", mac_xface->lte_frame_parms->tdd_config, protocol_ms->rrc.ue_bch_asn1.tdd_Config.subframeAssignment);
  msg ("[RRC-UE-FRONTEND] MAC_CONFIG_REQ  (SIB1)--->][MAC_UE]\n");
  #endif

 // After Decoding SIB1 (l. 776)
 /* rrc_mac_config_req(Mod_id,0,0,eNB_index,
	     (RadioResourceConfigCommonSIB_t *)NULL,
	     (struct PhysicalConfigDedicated *)NULL,
	     (MAC_MainConfig_t *)NULL,
	     0,
	     (struct LogicalChannelConfig *)NULL,
	     (MeasGapConfig_t *)NULL,
	     UE_rrc_inst[Mod_id].sib1[eNB_index]->tdd_Config,
	     &UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize,
	     &UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod); */

  rrc_mac_config_req(Mod_id,eNB_flag,UE_index,eNB_index,
		     NULL, //(RadioResourceConfigCommonSIB_t *)
		     NULL, //(struct PhysicalConfigDedicated *)
                     (MeasObjectToAddMod_t **)NULL,
		     NULL, // (MAC_MainConfig_t *)
		     0,
		     NULL, // (struct LogicalChannelConfig *)
		     NULL, //(MeasGapConfig_t *)
		     (TDD_Config_t *)&protocol_ms->rrc.ue_bch_asn1.tdd_Config,
		     &SIwindowsize,
		     &SIperiod);

// Simulate decoding SIB2
  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-UE-FRONTEND] do_SIB2_cell\n");
  #endif

//uint8_t do_SIB2_cell(uint8_t Mod_id, uint8_t *buffer, SystemInformation_t *systemInformation, SystemInformationBlockType2_t **sib2) {
  protocol_ms->rrc.ue_bch_asn1.sizeof_SIB2 = do_SIB2_cell(0, &protocol_ms->rrc.ue_bch_asn1.SIB23, &protocol_ms->rrc.ue_bch_asn1.systemInformation, &protocol_ms->rrc.ue_bch_asn1.sib2);

  #ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC-UE] SystemInformation SIB2 %d bytes\n", protocol_ms->rrc.ue_bch_asn1.sizeof_SIB2);
  //LOG_D(RRC,"[eNB] SystemInformation Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
  #endif

  if (protocol_ms->rrc.ue_bch_asn1.sizeof_SIB2 == 255)
    mac_xface->macphy_exit("rrc_sizeof_SIB2 is 255");
  else{
    //protocol_ms->rrc.ue_bch_asn1.sizeof_SIB2 = (u8)sizeof(SystemInformationBlockType2_t);

    #ifdef DEBUG_RRC_STATE
    msg ("[RRC-UE-FRONTEND] MAC_CONFIG_REQ  (SIB2)--->][MAC_UE]\n");
    #endif
   // After Decoding SIB2 (l. 926)
   /* rrc_mac_config_req(Mod_id,0,0,eNB_index,
		 &UE_rrc_inst[Mod_id].sib2[eNB_index]->radioResourceConfigCommon,
		 (struct PhysicalConfigDedicated *)NULL,
		 (MAC_MainConfig_t *)NULL,
		 0,
		 (struct LogicalChannelConfig *)NULL,
		 (MeasGapConfig_t *)NULL,
		 (TDD_Config_t *)NULL,
		 NULL,
		 NULL); */

    rrc_mac_config_req(Mod_id,eNB_flag,UE_index,eNB_index,
//                         &UE_rrc_inst[Mod_id].sib2[eNB_index]->radioResourceConfigCommon,
                         (RadioResourceConfigCommonSIB_t *)&protocol_ms->rrc.ue_bch_asn1.sib2->radioResourceConfigCommon,
                         NULL, // (struct PhysicalConfigDedicated *)
                         (MeasObjectToAddMod_t **)NULL,
                         NULL, //(MAC_MainConfig_t *)
                         0,
                         NULL, //(struct LogicalChannelConfig *)
                         NULL, // (MeasGapConfig_t *)
                         NULL, //(TDD_Config_t *)
                         NULL,
                         NULL);

  #ifdef DEBUG_RRC_STATE
  msg ("\n[RRC-UE-FRONTEND] rrc_ue_config_common_channels - END\n\n");
  #endif
  }

 // RHODOS26 version
/*  if ((rrm_config->prach.rach_trch.tf[0].bs == 0) || (rrm_config->sccpch.fach_trch.tf[0].bs == 0)) {
    msg ("[RRC][FSM-OUT] Configure FACH-RACH channels BAD CONFIG\n");
    wcdma_handle_error (WCDMA_ERROR_RRC_NASTY_BCH_CONFIG);
  }
  // if (!(started))  {
  //rrm_ue_simulate_bcch_sib5_6_acquisition();
  #ifndef BYPASS_L1
  msg ("[RRC][FSM-OUT] NOT BYPASS_L1 - Configure CPHY FACH-RACH channels.\n");
  CPHY_config_fach_rach ();
  #endif
  msg ("[RRC][FSM-OUT] CMAC - Configure FACH-RACH channels.\n");
  cmac_fach_rach_setup ();
  //   rrc_NAS_Conn_Est_Req_Rx();
  //  }
  #ifdef ALLOW_MBMS_PROTOCOL
  msg ("[RRC][FSM-OUT] ALLOW_MBMS_PROTOCOL ON - Configure FACH-RACH for MCCH channels.\n");
  rrc_mt_set_mcch_rb();
  #endif*/

}

//-----------------------------------------------------------------------------
void rrc_ue_xmit_ccch (void){
//-----------------------------------------------------------------------------
  u8 i=0,rv[6];
  //char ccch_buffer[100];
  //int ccch_buffer_size;
  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-UE-FRONTEND] rrc_ue_xmit_ccch\n");
  #endif

  if(protocol_ms->rrc.ccch_buffer_size ==0){

    for (i=0;i<6;i++) {
      rv[i]=i;
    }
    protocol_ms->rrc.ccch_buffer_size = do_RRCConnectionRequest((u8 *)protocol_ms->rrc.ccch_buffer,rv);
 }
}

//-----------------------------------------------------------------------------
void rrc_ue_config_LTE_srb1_srb2 (void){
//-----------------------------------------------------------------------------
  char Mod_id = 0;
  int UE_index =0;
  int eNB_index = 0;
  int eNB_flag = 0; //1=eNB, 0=UE
  int srb1 = 1, srb2 = 2;
  char buffer[1024];
  //LogicalChannelConfig_t *SRB1_logicalChannelConfig,*SRB2_logicalChannelConfig;

  #ifdef DEBUG_RRC_STATE
  msg ("\n[RRC-UE-FRONTEND] rrc_ue_config_LTE_srb1_srb2\n");
  #endif

  // get the parameters values SRB1_config, SRB2_config, physicalConfigDedicated
  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-UE-FRONTEND] do_RRCConnectionSetup\n");
  #endif
  do_RRCConnectionSetup(buffer,
			  mac_xface->get_transmission_mode(Mod_id,find_UE_RNTI(Mod_id,UE_index)),
			  UE_index,0,
			  mac_xface->lte_frame_parms,
			  //&eNB_rrc_inst[Mod_id].SRB1_config[UE_index],
			  &protocol_ms->rrc.ue_rb_asn1.SRB1_config,
			  //&eNB_rrc_inst[Mod_id].SRB2_config[UE_index],
			  &protocol_ms->rrc.ue_rb_asn1.SRB2_config,
			  //&eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);
			  &protocol_ms->rrc.ue_rb_asn1.physicalConfigDedicated);

  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-UE-FRONTEND] SRBx_logicalChannelConfig\n");
  #endif
  // get the parameters values SRB1_logicalChannelConfig SRB2_logicalChannelConfig
  // Default value set as global variable
  protocol_ms->rrc.ue_rb_asn1.SRB1_logicalChannelConfig = &SRB1_logicalChannelConfig_defaultValue;
  protocol_ms->rrc.ue_rb_asn1.SRB2_logicalChannelConfig = &SRB2_logicalChannelConfig_defaultValue;

  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-UE-FRONTEND] MAC_CONFIG_REQ  (SRB1 UE 0)--->][MAC_eNB]\n");
  #endif
  //Apply configurations to MAC and RLC for SRB1 and SRB2
  /*
  UE:   rrc_mac_config_req(Mod_id,0,0,eNB_index,
		     (RadioResourceConfigCommonSIB_t *)NULL,
		     UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index],
		     UE_rrc_inst[Mod_id].mac_MainConfig[eNB_index],
		     1,
		     SRB1_logicalChannelConfig,
		     (MeasGapConfig_t *)NULL,
		     NULL,
		     NULL,
		     NULL);
	}
  */

  rrc_mac_config_req(Mod_id,eNB_flag,UE_index,eNB_index,  //OK
		     (RadioResourceConfigCommonSIB_t *)NULL,  //OK
		     //(PhysicalConfigDedicated_t *)&protocol_ms->rrc.ue_rb_asn1.physicalConfigDedicated,  //OK
		     protocol_ms->rrc.ue_rb_asn1.physicalConfigDedicated,  //OK
                     (MeasObjectToAddMod_t **)NULL,
		     //(MAC_MainConfig_t*)&protocol_ms->rrc.ue_rb_asn1.mac_MainConfig,  //OK = NULL
		     (MAC_MainConfig_t*)NULL,
		     srb1, //logicalChannelIdentity  //OK
		     protocol_ms->rrc.ue_rb_asn1.SRB1_logicalChannelConfig,  //OK
		     protocol_ms->rrc.ue_rb_asn1.measGapConfig, //OK = NULL
		     //(MeasGapConfig_t *)NULL, //OK = NULL
		     (TDD_Config_t *)NULL, //OK
		     (u8 *)NULL,  //OK
		     (u16 *)NULL);  //OK

  msg("[UE %d], CONFIG_SRB1 %d corresponding to eNB_index %d\n", Mod_id,srb1,eNB_index);
  rrc_pdcp_config_req (Mod_id+NB_eNB_INST, protocol_ms->rrc.current_SFN, eNB_flag, ACTION_ADD, srb1);
  rrc_rlc_config_req(Mod_id+NB_eNB_INST, protocol_ms->rrc.current_SFN, eNB_flag, ACTION_ADD, srb1, SIGNALLING_RADIO_BEARER, Rlc_info_am_config);

 /*
  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-UE-FRONTEND] MAC_CONFIG_REQ  (SRB2 UE 0)--->][MAC_eNB]\n");
  #endif
  rrc_mac_config_req(Mod_id,eNB_flag,UE_index,eNB_index,  //OK
		     (RadioResourceConfigCommonSIB_t *)NULL,  //OK
		     (PhysicalConfigDedicated_t *)&protocol_ms->rrc.ue_rb_asn1.physicalConfigDedicated,  //OK
                     (MeasObjectToAddMod_t **)NULL,
		     //(MAC_MainConfig_t*)&protocol_ms->rrc.ue_rb_asn1.mac_MainConfig,  //OK = NULL
		     (MAC_MainConfig_t*)NULL,
		     srb2, //logicalChannelIdentity  //OK
		     &protocol_ms->rrc.ue_rb_asn1.SRB2_logicalChannelConfig,  //OK
		     //(MeasGapConfig_t *)&protocol_ms->rrc.ue_rb_asn1.measGapConfig, //OK = NULL
		     (MeasGapConfig_t *)NULL, //OK = NULL
		     (TDD_Config_t *)NULL, //OK
		     (u8 *)NULL,  //OK
		     (u16 *)NULL);  //OK

  msg("[UE %d], CONFIG_SRB2 %d corresponding to eNB_index %d\n", Mod_id,srb2,eNB_index);
  rrc_pdcp_config_req (Mod_id+NB_eNB_INST, protocol_ms->rrc.current_SFN, eNB_flag, ACTION_ADD, srb2);
  rrc_rlc_config_req(Mod_idMod_id+NB_eNB_INST,protocol_ms->rrc.current_SFN,eNB_flag,ACTION_ADD,srb2,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
*/
  #ifdef DEBUG_RRC_STATE
  msg ("\n[RRC-UE-FRONTEND] rrc_ue_config_LTE_srb1_srb2 - END\n\n");
  #endif
}

//-----------------------------------------------------------------------------
// This function needs to be finalized. It is delayed to a later version of OAI.
void rrc_ue_config_common_channels_SIB2 (void){
//-----------------------------------------------------------------------------
  asn_dec_rval_t dec_rval;
  BCCH_DL_SCH_Message_t bcch_message;
  BCCH_DL_SCH_Message_t *bcch_message_ptr;
  u8 SIwindowsize;
  u16 SIperiod;
  long si_WindowLength;
  const char siWindowLength_int[7] = {1,2,5,10,15,20,40};
  u32 si_window;

  #ifdef DEBUG_RRC_STATE
  msg ("\n\n[RRC-UE] rrc_ue_config_common_channels\n");
  msg (" >>>> TEMP OPENAIR : COMMENTED\n");
  #endif

  bcch_message_ptr=&bcch_message;
  memset(&bcch_message,0,sizeof(BCCH_DL_SCH_Message_t));

  // Decode the content of SIB2 (cf log_OpenRV2/120523_SIB2EncodeDecode.txt)
  dec_rval = uper_decode_complete(NULL, &asn_DEF_BCCH_DL_SCH_Message, (void **)&bcch_message_ptr,
              (const void *)&protocol_ms->rrc.ue_bch_blocks.currSIB5.prach_sCCPCH_SIList.data, protocol_ms->rrc.ue_bch_blocks.currSIB5.prach_sCCPCH_SIList.numocts);//,0,0);

  if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
     msg ("[RRC-UE] Failed to decode BCCH_DLSCH_MESSAGE (%d bits)\n",dec_rval.consumed);
     mac_xface->macphy_exit("");
  }
  //  xer_fprint(stdout,  &asn_DEF_BCCH_DL_SCH_Message, (void*)&bcch_message);
  si_WindowLength = SystemInformationBlockType1__si_WindowLength_ms20;
  SIperiod        = 8<<((int)si_WindowLength);
  SIwindowsize    = siWindowLength_int[si_WindowLength];
  #ifdef DEBUG_RRC_DETAILS
  msg ("[RRC-UE] rrc_ue_config_common_channels - SI Window period %d size %d\n", SIperiod, SIwindowsize);
  #endif

  if (bcch_message.message.present == BCCH_DL_SCH_MessageType_PR_c1) {
    switch (bcch_message.message.choice.c1.present) {
     /*
    case BCCH_DL_SCH_MessageType__c1_PR_systemInformationBlockType1:
      if ((frame %2) == 0) {
        if (UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 0) {
	  memcpy((void*)*sib1, (void*)&bcch_message.message.choice.c1.choice.systemInformationBlockType1, sizeof(SystemInformationBlockType1_t));
	  LOG_D(RRC,"[UE %d] Decoding First SIB1\n",Mod_id);
	  decode_SIB1(Mod_id,eNB_index);
	}
	break;
      */
      case BCCH_DL_SCH_MessageType__c1_PR_systemInformation:
      //if ((UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 1) && (UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus == 0)) {

//	si_window = (frame%UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod)/frame%UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize;
	si_window = (protocol_ms->rrc.current_SFN % SIperiod) / protocol_ms->rrc.current_SFN %SIwindowsize;
	//TODO memcpy((void*)si[si_window], (void*)&bcch_message.message.choice.c1.choice.systemInformation, sizeof(SystemInformation_t));
	msg("[RRC-UE] Decoding SI for frame %d, si_window %d\n",protocol_ms->rrc.current_SFN,si_window);
	//TODO decode_SI(0,protocol_ms->rrc.current_SFN,0,si_window);
      //}
        break;
      default:
	break;
      }
    }

}


