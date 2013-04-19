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

extern rlc_info_t Rlc_info_um, Rlc_info_am_config;

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void rrc_remove_UE(u8 Mod_id,u8 UE_id) {
//-----------------------------------------------------------------------------
  msg ("\n[RRC-RG]call to rrc_remove_UE [empty - to be implemented in CELLULAR mode]\n");
}


// Unified function to send data 
//-----------------------------------------------------------------------------
int rrc_rg_send_to_srb_rlc (int UE_id, int rb_id, char * data_buffer, int data_length){
//-----------------------------------------------------------------------------
  char tx_data[500];
  //int stxtlen = 0;

  int result =0;
  int Mod_id =0;
  int eNB_flag = 1; //1=eNB, 0=UE
  //int srb1 =1;

  #ifdef RRC_DEBUG_DETAILS
  msg ("\n[RRC-RG-FRONTEND] Send  Data to RLC, srb %d\n",rb_id);
  #endif
  // OpenAirInterface, as of 02/01/2013, requires passing all the RRC CELL srb through the DCCH
  // Multiplexing is performed by adding the srb_id as first byte of data buffer
  if ((rb_id != RRC_BCCH_ID)&& (rb_id != RRC_MCCH_ID)){
      memset(tx_data,0,500);
      tx_data[0] = rb_id;
      memcpy ((char*)&tx_data[1],data_buffer, data_length);
      data_length = data_length +1;
  }
  //rrc_print_buffer (tx_data, data_length);
/* RRC LITE
  //rrc_rlc_data_req(Mod_id,frame, 1,(UE_index*NB_RB_MAX)+DCCH,rrc_eNB_mui++,0,size,(char*)buffer);
  pdcp_data_req(Mod_id,frame, 1,(UE_index*NB_RB_MAX)+DCCH,rrc_eNB_mui++,0,size,(char*)buffer,1);
*/

  switch (rb_id){
    case RRC_BCCH_ID: //BCCH
      // send buffer on BCCH - As of 08/07/2010, this call is actually a NOP, since 
      // broadcast is actually retrieved through the rrc_L2_data_req_rx function
      result = 1;
      break;
    case RRC_SRB0_ID: //CCCH
    case RRC_SRB1_ID: //DCCH-UM
      //result = rrc_rlc_data_req(Mod_id,protocol_bs->rrc.current_SFN, eNB_flag,(UE_id*NB_RB_MAX)+DCCH,protocol_bs->rrc.next_MUI++,RRC_RLC_CONFIRM_NO,data_length,tx_data);
     if (pdcp_data_req(Mod_id,protocol_bs->rrc.current_SFN, eNB_flag,(UE_id*NB_RB_MAX)+DCCH,protocol_bs->rrc.next_MUI++,RRC_RLC_CONFIRM_NO,data_length,tx_data,1))
      result = 1;
      break;
    case RRC_SRB2_ID: //DCCH-AM
    case RRC_SRB3_ID: //DCCH-AM - NAS
     if (pdcp_data_req(Mod_id,protocol_bs->rrc.current_SFN, eNB_flag,(UE_id*NB_RB_MAX)+DCCH,protocol_bs->rrc.next_MUI++,RRC_RLC_CONFIRM_YES,data_length,tx_data,1))
      result = 1;
      break;
    case RRC_MCCH_ID: //MCCH
      msg ("\n[RRC-RG-FRONTEND] TEMP - Unable to send data to RLC, Channel srb %d (MCCH) not supported\n",rb_id);
      break;
    default:
       msg ("\n[RRC-RG-FRONTEND] ERROR - Unable to send data to RLC, Channel srb %d not supported\n",rb_id);
  }
  if (result !=1)
       msg ("\n[RRC-RG-FRONTEND] ERROR - RLC returned an error code %d\n", result);

  return result;

}

//-----------------------------------------------------------------------------
// Unified function to receive data 
int rrc_rg_receive_from_srb_rlc (char* sduP, u8 ch_idP, unsigned int Sdu_size){
//-----------------------------------------------------------------------------
  int srb_id, rb_id=0;
  int UE_Id;
  //int sdu_offset=0;

  #ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC-RG-FRONTEND] CALL to rrc_rg_srb_rx, lchannel %d\n", ch_idP);
  rrc_print_buffer ((char*)&sduP[0], Sdu_size);
  #endif

  // get UE_Id
  rb_id = ch_idP;
  srb_id = rb_id % NB_RB_MAX;
  UE_Id = (rb_id - srb_id) / NB_RB_MAX;
  // get RRC_CELL srb_id
  srb_id = sduP[0];

/*
  //rb_id = ch_idP - 12;
  rb_id = ch_idP;
  srb_id = rb_id % maxRB;
  UE_Id = (rb_id - srb_id) / maxRB;
  //TEMP - if srb_id is SRB2, check if it is not SRB3
  if (srb_id==RRC_SRB2_ID){
    sdu_offset = 1;
    if (sduP[0] == RRC_SRB2_ID)
      srb_id=RRC_SRB2_ID;
    if (sduP[0] == RRC_SRB3_ID)
      srb_id=RRC_SRB3_ID;
  }
  #ifdef DEBUG_RRC_DETAILS
  msg ("[RRC][SRB-RG] LCHAN%d RX in frame %d\n", ch_idP, protocol_bs->rrc.current_SFN);
  #endif
  if (ch_idP!=RRC_LTE_DCCH_ID){
    #ifdef DEBUG_RRC_STATE
    msg ("[RRC][SRB-RG] RB %d, SRB%d received, UE_Id %d\n", rb_id, srb_id, UE_Id);
    //msg ("[RRC][SRB-RG] frame received: %s\n", (char*)&sduP[sdu_offset]);
    #endif
    rrc_rg_srb_rx (Buffer, rb_id, Sdu_size);
  }
*/

  switch (srb_id){
    case RRC_SRB0_ID: //CCCH
    case RRC_SRB1_ID: //DCCH-UM
    case RRC_SRB2_ID: //DCCH-AM
    case RRC_SRB3_ID: //DCCH-AM - NAS
      rrc_rg_srb_rx ((char*)&sduP[1], srb_id, UE_Id);
      break;
    default:
       msg ("\n[RRC-RG-FRONTEND] Invalid Channel srb  number %d\n",srb_id);
  }

  return 0;
}

/*****************************************
 Configuration Functions for L1-L2 layers
 *****************************************/
// This function retrieves the common configuration for lower layers  to be broadcasted in SIB5 (or SIB2 for LTE)
//-----------------------------------------------------------------------------
void rrc_rg_get_common_config_SIB (int *config_length, char* *config_ptr){
//-----------------------------------------------------------------------------
  #ifdef DEBUG_RRC_DETAILS
  msg ("\n\n[RRC-RG-FRONTEND] rrc_rg_get_common_config_SIB\n");
  #endif
  // (RadioResourceConfigCommonSIB_t *)&eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon,

//    protocol_bs->rrc.rg_bch_asn1.sizeof_SIB2 = sizeof(SystemInformationBlockType2_t);
//    *config_length = sizeof(SystemInformationBlockType2_t);
//    config_ptr = (char*)&protocol_bs->rrc.rg_bch_asn1.sib2;
//    *config_length = sizeof(RadioResourceConfigCommonSIB_t *);
//    config_ptr = (char*)&protocol_bs->rrc.rg_bch_asn1.sib2.radioResourceConfigCommon;
    *config_length = protocol_bs->rrc.rg_bch_asn1.sizeof_SIB2;
    *config_ptr = (char*)&protocol_bs->rrc.rg_bch_asn1.SIB23;
  #ifdef DEBUG_RRC_DETAILS
  //msg ("\n[RRC-RG] rrc_rg_get_common_config_SIB config pointer  %p \n", *config_ptr);
  rrc_print_buffer ((char *) &protocol_bs->rrc.rg_bch_asn1.SIB23, protocol_bs->rrc.rg_bch_asn1.sizeof_SIB2);
  #endif

  #ifdef DEBUG_RRC_DETAILS
  msg ("[RRC-RG-FRONTEND] rrc_rg_get_common_config_SIB - END\n\n");
  #endif
}

//-----------------------------------------------------------------------------
void rrc_rg_init_mac (unsigned char Mod_id){
//-----------------------------------------------------------------------------
  u8 result;
  //int Mod_id =0;
  int eNB_flag=1;
  int UE_index=0;
  int eNB_index=0;

  u8 SIwindowsize=1;
  u16 SIperiod=8;


  #ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC-RG-FRONTEND] rrc_rg_init_mac %d\n", Mod_id);
  #endif
  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-RG-FRONTEND] do_SIB1_TDD_config_cell\n");
  #endif

  // Initialize TDD_config parameters (TDD_Config_t tdd_Config;) calling asn1_msg.c
  result = do_SIB1_TDD_config_cell (mac_xface->lte_frame_parms, (TDD_Config_t *)&protocol_bs->rrc.rg_bch_asn1.tdd_Config);

  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-RG-FRONTEND] Frame TDD config %d, protocol_bs value %d\n", mac_xface->lte_frame_parms->tdd_config, (int)protocol_bs->rrc.rg_bch_asn1.tdd_Config.subframeAssignment);
  msg ("[RRC-RG-FRONTEND] do_SIB2_cell\n");
  #endif

//uint8_t do_SIB2_cell(uint8_t Mod_id, LTE_DL_FRAME_PARMS *frame_parms, uint8_t *buffer,
//                  BCCH_DL_SCH_Message_t *bcch_message, SystemInformationBlockType2_t **sib2) {
  protocol_bs->rrc.rg_bch_asn1.sizeof_SIB2 = do_SIB2_cell(0, mac_xface->lte_frame_parms, &protocol_bs->rrc.rg_bch_asn1.SIB23,
                    &protocol_bs->rrc.rg_bch_asn1.systemInformation, &protocol_bs->rrc.rg_bch_asn1.sib2);

  #ifdef DEBUG_RRC_DETAILS
  msg ("[RRC-RG] SystemInformation Encoded %d bytes\n", protocol_bs->rrc.rg_bch_asn1.sizeof_SIB2);
  //LOG_D(RRC,"[eNB] SystemInformation Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
  #endif

  if (protocol_bs->rrc.rg_bch_asn1.sizeof_SIB2 == 255)
    mac_xface->macphy_exit("rrc_sizeof_SIB2 = 255");
  else{
    #ifdef DEBUG_RRC_STATE
    msg ("[RRC-RG-FRONTEND] MAC_CONFIG_REQ  (SIB1-SIB2)--->][MAC_eNB]\n");
    //msg ("[RRC-RG-FRONTEND] Frame TDD config %d, protocol_bs value %d\n", mac_xface->lte_frame_parms->tdd_config, protocol_bs->rrc.rg_bch_asn1.tdd_Config.subframeAssignment);
    #endif

/*    rrc_mac_config_req(Mod_id,1,0,0,
		       (RadioResourceConfigCommonSIB_t *)&eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon,
		       (struct PhysicalConfigDedicated *)NULL,
		       (MeasObjectToAddMod_t **)NULL,
		       (MAC_MainConfig_t *)NULL,
		       0,
		       (struct LogicalChannelConfig *)NULL,
		       (MeasGapConfig_t *)NULL,
		       eNB_rrc_inst[Mod_id].sib1->tdd_Config,
		       &SIwindowsize,
		       &SIperiod,
		       eNB_rrc_inst[Mod_id].sib2->freqInfo.ul_CarrierFreq,
		       eNB_rrc_inst[Mod_id].sib2->freqInfo.ul_Bandwidth,
		       &eNB_rrc_inst[Mod_id].sib2->freqInfo.additionalSpectrumEmission,
		       (MBSFN_SubframeConfigList_t *)eNB_rrc_inst[Mod_id].sib2->mbsfn_SubframeConfigList
#ifdef Rel10
		       ,
		       eNB_rrc_inst[Mod_id].MBMS_flag,
		       (MBSFN_AreaInfoList_r9_t *)&eNB_rrc_inst[Mod_id].sib13->mbsfn_AreaInfoList_r9,
		       (PMCH_InfoList_r9_t *)NULL
#endif 
		       );
*/

     //  Apply configuration to local MAC
    rrc_mac_config_req(Mod_id,eNB_flag,UE_index,eNB_index,
       (RadioResourceConfigCommonSIB_t *)&protocol_bs->rrc.rg_bch_asn1.sib2->radioResourceConfigCommon,
       (struct PhysicalConfigDedicated *)NULL,
                       (MeasObjectToAddMod_t **)NULL,
       (MAC_MainConfig_t *)NULL,
       0, //logicalChannelIdentity
       (struct LogicalChannelConfig *)NULL,
       (MeasGapConfig_t *)NULL,
       (TDD_Config_t *)&protocol_bs->rrc.rg_bch_asn1.tdd_Config,
       &SIwindowsize,
       &SIperiod,
       protocol_bs->rrc.rg_bch_asn1.sib2->freqInfo.ul_CarrierFreq,
       protocol_bs->rrc.rg_bch_asn1.sib2->freqInfo.ul_Bandwidth,
       protocol_bs->rrc.rg_bch_asn1.sib2->freqInfo.additionalSpectrumEmission,
       (MBSFN_SubframeConfigList_t *)protocol_bs->rrc.rg_bch_asn1.sib2->mbsfn_SubframeConfigList
       #ifdef Rel10
       , 0, //eNB_rrc_inst[Mod_id].MBMS_flag,
       (MBSFN_AreaInfoList_r9_t *)NULL,
       (PMCH_InfoList_r9_t *)NULL
       #endif
       );
  }

  #ifdef DEBUG_RRC_DETAILS
  msg ("[RRC-RG-FRONTEND] rrc_rg_init_mac %d - END\n", Mod_id);
  #endif
}


//-----------------------------------------------------------------------------
void rrc_rg_config_LTE_srb1 (unsigned char Mod_id){
//-----------------------------------------------------------------------------
  int UE_index =0;
  int eNB_index = 0;
  int eNB_flag = 1; //1=eNB, 0=UE
  int srb1 = 1;
  #ifdef DEBUG_RRC_STATE
  msg ("\n[RRC-RG-FRONTEND] rrc_rg_config_LTE_srb1\n");
  #endif

  // get the parameters values SRB1_config, SRB2_config, physicalConfigDedicated
  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-RG-FRONTEND] do_RRCConnectionSetup\n");
  #endif

    //protocol_ms->rrc.ccch_buffer_size = do_RRCConnectionRequest((u8 *)protocol_ms->rrc.ccch_buffer,rv);
//  SRB_ToAddModList_t **SRB_configList = &eNB_rrc_inst[Mod_id].SRB_configList[UE_index];

//     do_RRCConnectionSetup((u8 *)eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.Payload,
// 			  mac_xface->get_transmission_mode(Mod_id,find_UE_RNTI(Mod_id,UE_index)),
// 			  UE_index,0,
// 			  mac_xface->lte_frame_parms,
// 			  SRB_configList,
// 			  &eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);


  protocol_bs->rrc.ccch_buffer_size = do_RRCConnectionSetup((u8 *)protocol_bs->rrc.ccch_buffer,
			  mac_xface->get_transmission_mode(Mod_id,find_UE_RNTI(Mod_id,UE_index)),
			  UE_index,0,
			  mac_xface->lte_frame_parms,
/*			  //&eNB_rrc_inst[Mod_id].SRB1_config[UE_index],
			  &protocol_bs->rrc.rg_rb_asn1.SRB1_config,
			  //&eNB_rrc_inst[Mod_id].SRB2_config[UE_index],
			  &protocol_bs->rrc.rg_rb_asn1.SRB2_config,*/
                          &protocol_bs->rrc.rg_rb_asn1.SRB_configList[UE_index],
			  //&eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);
			  &protocol_bs->rrc.rg_rb_asn1.physicalConfigDedicated);

  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-RG-FRONTEND] SRB1_logicalChannelConfig\n");
  #endif
  // get the parameters values SRB1_logicalChannelConfig SRB2_logicalChannelConfig
  // Default value set as global variable
  protocol_bs->rrc.rg_rb_asn1.SRB1_logicalChannelConfig = &SRB1_logicalChannelConfig_defaultValue;

  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-RG-FRONTEND] MAC_CONFIG_REQ  (SRB1 UE 0)--->][MAC_eNB]\n");
  #endif
  //Apply configurations to MAC and RLC for SRB1 and SRB2
  /*
  rrc_mac_config_req(Mod_id,1,UE_index,0,
      (RadioResourceConfigCommonSIB_t *)NULL,
      eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
      (MeasObjectToAddMod_t **)NULL,
      eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
      1,
      SRB1_logicalChannelConfig,
      eNB_rrc_inst[Mod_id].measGapConfig[UE_index],
      (TDD_Config_t *)NULL,
      (u8 *)NULL,
      (u16 *)NULL,
      NULL,
      NULL,
      NULL,
      (MBSFN_SubframeConfigList_t *)NULL
      #ifdef Rel10
      , 0,
      (MBSFN_AreaInfoList_r9_t *)NULL,
      (PMCH_InfoList_r9_t *)NULL
      #endif
  );

  */
  rrc_mac_config_req(Mod_id,eNB_flag,UE_index,eNB_index,  //OK
       (RadioResourceConfigCommonSIB_t *)NULL,  //OK
       //(PhysicalConfigDedicated_t *)&protocol_bs->rrc.rg_rb_asn1.physicalConfigDedicated,  //OK
       protocol_bs->rrc.rg_rb_asn1.physicalConfigDedicated,  //OK
       (MeasObjectToAddMod_t **)NULL,
       protocol_bs->rrc.rg_rb_asn1.mac_MainConfig,  //OK = NULL
       srb1, //logicalChannelIdentity  //OK
       protocol_bs->rrc.rg_rb_asn1.SRB1_logicalChannelConfig,  //OK
       protocol_bs->rrc.rg_rb_asn1.measGapConfig, //OK = NULL
       (TDD_Config_t *)NULL, //OK
       (u8 *)NULL,  //OK
       (u16 *)NULL, //OK
       NULL,
       NULL,
       NULL,
       (MBSFN_SubframeConfigList_t *)NULL
       #ifdef Rel10
       , 0,
       (MBSFN_AreaInfoList_r9_t *)NULL,
       (PMCH_InfoList_r9_t *)NULL
       #endif
       );

  msg("[eNB %d] CALLING PDCP + RLC CONFIG SRB1 (rbid %d) for UE %d\n", Mod_id,srb1,UE_index);
/*  rrc_pdcp_config_req (Mod_id, protocol_bs->rrc.current_SFN, eNB_flag, ACTION_ADD, srb1);
  rrc_rlc_config_req(Mod_id,protocol_bs->rrc.current_SFN,eNB_flag,ACTION_ADD,srb1,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);*/
  rrc_pdcp_config_asn1_req(Mod_id,protocol_bs->rrc.current_SFN,eNB_flag,UE_index,
      protocol_bs->rrc.rg_rb_asn1.SRB_configList[UE_index],
      (DRB_ToAddModList_t*)NULL,
      (DRB_ToReleaseList_t*)NULL
      #ifdef Rel10
      ,(MBMS_SessionInfoList_r9_t *)NULL
      #endif
  );

 rrc_rlc_config_asn1_req(Mod_id,protocol_bs->rrc.current_SFN,eNB_flag,UE_index,
      protocol_bs->rrc.rg_rb_asn1.SRB_configList[UE_index],
      (DRB_ToAddModList_t*)NULL, 
      (DRB_ToReleaseList_t*)NULL
      #ifdef Rel10
      ,(MBMS_SessionInfoList_r9_t *)NULL
      #endif
      );
}

//-----------------------------------------------------------------------------
void rrc_rg_rcve_ccch(u8 Mod_id, char *Sdu, u16 Sdu_len){
//-----------------------------------------------------------------------------
  #ifdef DEBUG_RRC_STATE
  msg ("\n[RRC-RG-FRONTEND] rrc_rg_rcve_ccch , ConnReq, length %d\n", Sdu_len);
  #endif
  rrc_rg_config_LTE_srb1((unsigned char)Mod_id);
}

//-----------------------------------------------------------------------------
void rrc_rg_config_LTE_srb2 (unsigned char Mod_id){
//-----------------------------------------------------------------------------
  int UE_index =0;
  int eNB_index = 0;
  int eNB_flag = 1; //1=eNB, 0=UE
  int srb2 = 2;

  #ifdef DEBUG_RRC_STATE
  msg ("\n[RRC-RG-FRONTEND] rrc_rg_config_LTE_srb2\n");
  #endif


  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-RG-FRONTEND] SRB2_logicalChannelConfig\n");
  #endif
  // get the parameters values SRB2_logicalChannelConfig
  // Default value set as global variable
  protocol_bs->rrc.rg_rb_asn1.SRB2_logicalChannelConfig = &SRB2_logicalChannelConfig_defaultValue;

  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-RG-FRONTEND] MAC_CONFIG_REQ  (SRB2 UE 0)--->][MAC_eNB]\n");
  #endif
  rrc_mac_config_req(Mod_id,eNB_flag,UE_index,eNB_index,  //OK
       (RadioResourceConfigCommonSIB_t *)NULL,  //OK
       protocol_bs->rrc.rg_rb_asn1.physicalConfigDedicated,  //OK
       (MeasObjectToAddMod_t **)NULL,
       //(MAC_MainConfig_t*)&protocol_bs->rrc.rg_rb_asn1.mac_MainConfig,  //OK = NULL
       (MAC_MainConfig_t*)NULL,
       srb2, //logicalChannelIdentity  //OK
       protocol_bs->rrc.rg_rb_asn1.SRB2_logicalChannelConfig,  //OK
       //(MeasGapConfig_t *)&protocol_bs->rrc.rg_rb_asn1.measGapConfig, //OK = NULL
       (MeasGapConfig_t *)NULL, //OK = NULL
       (TDD_Config_t *)NULL, //OK
       (u8 *)NULL,  //OK
       (u16 *)NULL, //OK
       NULL,
       NULL,
       NULL,
       (MBSFN_SubframeConfigList_t *)NULL
       #ifdef Rel10
       , 0,
       (MBSFN_AreaInfoList_r9_t *)NULL,
       (PMCH_InfoList_r9_t *)NULL
       #endif
       );


  msg("[eNB %d] CALLING RLC CONFIG SRB2 (rbid %d) for UE %d\n", Mod_id,srb2,UE_index);
/*  rrc_pdcp_config_req (Mod_id, protocol_bs->rrc.current_SFN, eNB_flag, ACTION_ADD, srb2);
  rrc_rlc_config_req(Mod_id,protocol_bs->rrc.current_SFN,eNB_flag,ACTION_ADD,srb2,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);*/
  rrc_pdcp_config_asn1_req(Mod_id,protocol_bs->rrc.current_SFN,eNB_flag,UE_index,
      protocol_bs->rrc.rg_rb_asn1.SRB_configList[UE_index],
      (DRB_ToAddModList_t*)NULL,
      (DRB_ToReleaseList_t*)NULL
      #ifdef Rel10
      ,(MBMS_SessionInfoList_r9_t *)NULL
      #endif
  );

 rrc_rlc_config_asn1_req(Mod_id,protocol_bs->rrc.current_SFN,eNB_flag,UE_index,
      protocol_bs->rrc.rg_rb_asn1.SRB_configList[UE_index],
      (DRB_ToAddModList_t*)NULL, 
      (DRB_ToReleaseList_t*)NULL
      #ifdef Rel10
      ,(MBMS_SessionInfoList_r9_t *)NULL
      #endif
      );

}

//-----------------------------------------------------------------------------
void rrc_rg_config_LTE_default_drb (unsigned char Mod_id){
//-----------------------------------------------------------------------------
  int UE_index =0;
  int eNB_index = 0;
  int eNB_flag = 1; //1=eNB, 0=UE
  int drb_ix=0;  // default DRB
  u8 buffer[100];
  u8 size;
  u8 DRB2LCHAN[8];

  #ifdef DEBUG_RRC_STATE
  msg ("\n[RRC-RG-FRONTEND] rrc_rg_config_LTE_default_drb: begin\n");
  #endif
  /*
  uint8_t do_RRCConnectionReconfiguration(uint8_t  Mod_id,
              uint8_t                          *buffer,
              uint8_t                           UE_id,
              uint8_t                           Transaction_id,
              SRB_ToAddModList_t                *SRB_list,
              DRB_ToAddModList_t                *DRB_list,
              DRB_ToReleaseList_t               *DRB_list2,
              struct SPS_Config                 *sps_Config,
              struct PhysicalConfigDedicated    *physicalConfigDedicated,
              MeasObjectToAddModList_t          *MeasObj_list,
              ReportConfigToAddModList_t        *ReportConfig_list,
              QuantityConfig_t                  *QuantityConfig,
              MeasIdToAddModList_t              *MeasId_list,
              MAC_MainConfig_t                  *mac_MainConfig,
              MeasGapConfig_t                   *measGapConfig,
              uint8_t                           *nas_pdu,
              uint32_t                           nas_length
              ) {
  */
  #ifdef DEBUG_RRC_STATE
  msg ("[RRC-RG-FRONTEND] do_RRCConnReconf_defaultCELL\n");
  #endif

//  size = do_RRCConnectionReconfiguration_cell(Mod_id, buffer, UE_index, 0,
  size = do_RRCConnReconf_defaultCELL(Mod_id, buffer, UE_index, 0,
                                         //&eNB_rrc_inst[Mod_id].SRB2_config[UE_index],
                                         //&protocol_bs->rrc.rg_rb_asn1.SRB2_config,
                                         &protocol_bs->rrc.rg_rb_asn1.SRB_configList[UE_index],
                                         //&eNB_rrc_inst[Mod_id].DRB_config[UE_id][0],
                                         &protocol_bs->rrc.rg_rb_asn1.DRB_configList[UE_index],
                                         //&eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);
                                         &protocol_bs->rrc.rg_rb_asn1.physicalConfigDedicated);

  if (protocol_bs->rrc.rg_rb_asn1.DRB_configList[UE_index]->list.array[0]){
    msg("[RRC-RG-FRONTEND] rrc_rg_config_LTE_default_drb: reconfiguring DRB %d/LCID %d\n",
       (int)protocol_bs->rrc.rg_rb_asn1.DRB_configList[UE_index]->list.array[0]->drb_Identity,
       (int)*protocol_bs->rrc.rg_rb_asn1.DRB_configList[UE_index]->list.array[0]->logicalChannelIdentity);

    DRB2LCHAN[drb_ix] = (u8)*protocol_bs->rrc.rg_rb_asn1.DRB_configList[UE_index]->list.array[0]->logicalChannelIdentity;

    if (protocol_bs->rrc.rg_rb_asn1.DRB1_active == 0) {
      msg("[RRC-RG-FRONTEND] rrc_rg_config_LTE_default_drb: Frame %d: Establish PDCP + RLC UM Bidirectional, DRB %d Active\n",
            protocol_bs->rrc.current_SFN, (int)protocol_bs->rrc.rg_rb_asn1.DRB_configList[UE_index]->list.array[0]->drb_Identity);

  //     rrc_pdcp_config_req (Mod_id, protocol_bs->rrc.current_SFN, eNB_flag, ACTION_ADD,
  //                           (UE_index * NB_RB_MAX) + (int)*protocol_bs->rrc.rg_rb_asn1.DRB1_config->logicalChannelIdentity);
  //     rrc_rlc_config_req(Mod_id,protocol_bs->rrc.current_SFN,eNB_flag,ACTION_ADD,
  //                         (UE_index * NB_RB_MAX) + (int)*protocol_bs->rrc.rg_rb_asn1.DRB1_config->logicalChannelIdentity,
  //                         RADIO_ACCESS_BEARER,Rlc_info_um);

      rrc_pdcp_config_asn1_req(Mod_id,protocol_bs->rrc.current_SFN,eNB_flag,UE_index,
          (SRB_ToAddModList_t*)NULL,
          protocol_bs->rrc.rg_rb_asn1.DRB_configList[UE_index],
          (DRB_ToReleaseList_t*)NULL
          #ifdef Rel10
          ,(MBMS_SessionInfoList_r9_t *)NULL
          #endif
      );
      rrc_rlc_config_asn1_req(Mod_id,protocol_bs->rrc.current_SFN,eNB_flag,UE_index,
          (SRB_ToAddModList_t*)NULL,
          protocol_bs->rrc.rg_rb_asn1.DRB_configList[UE_index],
          (DRB_ToReleaseList_t*)NULL
          #ifdef Rel10
          ,(MBMS_SessionInfoList_r9_t *)NULL
          #endif
      );
      protocol_bs->rrc.rg_rb_asn1.DRB1_active = 1;

      msg("[RRC-RG-FRONTEND] rrc_rg_config_LTE_default_drb:[--- MAC_CONFIG_REQ  (DRB UE %d) --->][MAC_eNB]\n", UE_index);


  /*  RRC Lite l 1357
            rrc_mac_config_req(Mod_id,1,UE_index,0,
                              (RadioResourceConfigCommonSIB_t *)NULL,
                              eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
                              (MeasObjectToAddMod_t **)NULL,
                              eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
                              DRB2LCHAN[i],
                              DRB_configList->list.array[i]->logicalChannelConfig,
                              eNB_rrc_inst[Mod_id].measGapConfig[UE_index],
                              (TDD_Config_t *)NULL,
                              (u8 *)NULL,
                              (u16 *)NULL,
                              NULL,
                              NULL,
                              NULL,
                              (MBSFN_SubframeConfigList_t *)NULL
                              #ifdef Rel10
                              , 0,
                              (MBSFN_AreaInfoList_r9_t *)NULL,
                              (PMCH_InfoList_r9_t *)NULL
                                #endif
                              );
  */
  
      rrc_mac_config_req(Mod_id,eNB_flag,UE_index,eNB_index,
          (RadioResourceConfigCommonSIB_t *)NULL,
          protocol_bs->rrc.rg_rb_asn1.physicalConfigDedicated,
          (MeasObjectToAddMod_t **)NULL,
          (MAC_MainConfig_t*)&protocol_bs->rrc.rg_rb_asn1.mac_MainConfig,
          DRB2LCHAN[drb_ix],
          //protocol_bs->rrc.rg_rb_asn1.DRB1_config->logicalChannelConfig,
          protocol_bs->rrc.rg_rb_asn1.DRB_configList[UE_index]->list.array[0]->logicalChannelConfig,
          (MeasGapConfig_t *)&protocol_bs->rrc.rg_rb_asn1.measGapConfig,
          (TDD_Config_t *)NULL,
          (u8 *)NULL,
          (u16 *)NULL, //OK
          NULL,
          NULL,
          NULL,
          (MBSFN_SubframeConfigList_t *)NULL
          #ifdef Rel10
            , 0,
          (MBSFN_AreaInfoList_r9_t *)NULL,
          (PMCH_InfoList_r9_t *)NULL
          #endif
      );
  
    }else{ // remove LCHAN from MAC/PHY
      // Initialized here because it is a local variable (global in RRC Lite)
      //DRB2LCHAN[drb_ix] = (u8)*protocol_bs->rrc.rg_rb_asn1.DRB1_config->logicalChannelIdentity;
      if (protocol_bs->rrc.rg_rb_asn1.DRB1_active ==1) {
        // DRB has just been removed so remove RLC + PDCP for DRB
        msg("[RRC-RG-FRONTEND] rrc_rg_config_LTE_default_drb: Frame %d: Remove PDCP + RLC UM Bidirectional, DRB 0 \n",
//            protocol_bs->rrc.current_SFN, (int)protocol_bs->rrc.rg_rb_asn1.DRB_configList[UE_index]->list.array[0]->drb_Identity);
            protocol_bs->rrc.current_SFN);

/*        rrc_pdcp_config_req (Mod_id, protocol_bs->rrc.current_SFN, eNB_flag, ACTION_REMOVE,
                             (UE_index * NB_RB_MAX) + DRB2LCHAN[drb_ix]);*/
        rrc_rlc_config_req(Mod_id,protocol_bs->rrc.current_SFN,eNB_flag,ACTION_REMOVE,
                          (UE_index * NB_RB_MAX) + DRB2LCHAN[drb_ix],
                          RADIO_ACCESS_BEARER,Rlc_info_um);

      }
      protocol_bs->rrc.rg_rb_asn1.DRB1_active = 0;
      msg("[RRC-RG-FRONTEND] rrc_rg_config_LTE_default_drb:[--- MAC_CONFIG_REQ  (DRB UE %d) --->][MAC_eNB]\n", UE_index);
      rrc_mac_config_req(Mod_id,eNB_flag,UE_index,eNB_index,
          (RadioResourceConfigCommonSIB_t *)NULL,
          protocol_bs->rrc.rg_rb_asn1.physicalConfigDedicated,
          (MeasObjectToAddMod_t **)NULL,
          (MAC_MainConfig_t*)&protocol_bs->rrc.rg_rb_asn1.mac_MainConfig,
          DRB2LCHAN[drb_ix],
          (LogicalChannelConfig_t *)NULL,
          (MeasGapConfig_t *)NULL,
          (TDD_Config_t *)NULL,
          (u8 *)NULL,
          (u16 *)NULL, //OK
          NULL,
          NULL,
          NULL,
          (MBSFN_SubframeConfigList_t *)NULL
          #ifdef Rel10
          , 0,
          (MBSFN_AreaInfoList_r9_t *)NULL,
          (PMCH_InfoList_r9_t *)NULL
          #endif
      );
    }
  }
}

