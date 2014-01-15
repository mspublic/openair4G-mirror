/*
 * rrc_messages_def.h
 *
 *  Created on: Oct 24, 2013
 *      Author: winckel
 */

//-------------------------------------------------------------------------------------------//
// Messages for RRC logging
#if defined(DISABLE_ITTI_XER_PRINT)
MESSAGE_DEF(RRC_DL_BCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  RrcDlBcchMessage,           rrc_dl_bcch_message)
MESSAGE_DEF(RRC_DL_CCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  RrcDlCcchMessage,           rrc_dl_ccch_message)
MESSAGE_DEF(RRC_DL_DCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  RrcDlDcchMessage,           rrc_dl_dcch_message)

MESSAGE_DEF(RRC_UE_EUTRA_CAPABILITY,    MESSAGE_PRIORITY_MED_PLUS,  RrcUeEutraCapability,       rrc_ue_eutra_capability)

MESSAGE_DEF(RRC_UL_CCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  RrcUlCcchMessage,           rrc_ul_ccch_message)
MESSAGE_DEF(RRC_UL_DCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  RrcUlDcchMessage,           rrc_ul_dcch_message)
#else
MESSAGE_DEF(RRC_DL_BCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_dl_bcch_message)
MESSAGE_DEF(RRC_DL_CCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_dl_ccch_message)
MESSAGE_DEF(RRC_DL_DCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_dl_dcch_message)
MESSAGE_DEF(RRC_DL_MCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_dl_mcch_message)

MESSAGE_DEF(RRC_UE_EUTRA_CAPABILITY,    MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_ue_eutra_capability)

MESSAGE_DEF(RRC_UL_CCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_ul_ccch_message)
MESSAGE_DEF(RRC_UL_DCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_ul_dcch_message)
#endif

MESSAGE_DEF(RRC_STATE_IND,              MESSAGE_PRIORITY_MED,       RrcStateInd,                rrc_state_ind)

//-------------------------------------------------------------------------------------------//
// eNB: ENB_APP -> RRC messages
MESSAGE_DEF(RRC_CONFIGURATION_REQ,      MESSAGE_PRIORITY_MED,       RrcConfigurationReq,        rrc_configuration_req)

// UE: NAS -> RRC messages
MESSAGE_DEF(NAS_CELL_SELECTION_REQ,     MESSAGE_PRIORITY_MED,       NasCellSelectionReq,        nas_cell_selection_req)
MESSAGE_DEF(NAS_CONN_ESTABLI_REQ,       MESSAGE_PRIORITY_MED,       NasConnEstabliReq,          nas_conn_establi_req)
MESSAGE_DEF(NAS_UPLINK_DATA_REQ,        MESSAGE_PRIORITY_MED,       NasUlDataReq,               nas_ul_data_req)

MESSAGE_DEF(NAS_RAB_ESTABLI_RSP,        MESSAGE_PRIORITY_MED,       NasRabEstRsp,               nas_rab_est_rsp)

// UE: RRC -> NAS messages
MESSAGE_DEF(NAS_CELL_SELECTION_CNF,     MESSAGE_PRIORITY_MED,       NasCellSelectionCnf,        nas_cell_selection_cnf)
MESSAGE_DEF(NAS_CELL_SELECTION_IND,     MESSAGE_PRIORITY_MED,       NasCellSelectionInd,        nas_cell_selection_ind)
MESSAGE_DEF(NAS_PAGING_IND,             MESSAGE_PRIORITY_MED,       NasPagingInd,               nas_paging_ind)
MESSAGE_DEF(NAS_CONN_ESTABLI_CNF,       MESSAGE_PRIORITY_MED,       NasConnEstabCnf,            nas_conn_establi_cnf)
MESSAGE_DEF(NAS_CONN_RELEASE_IND,       MESSAGE_PRIORITY_MED,       NasConnReleaseInd,          nas_conn_release_ind)
MESSAGE_DEF(NAS_UPLINK_DATA_CNF,        MESSAGE_PRIORITY_MED,       NasUlDataCnf,               nas_ul_data_cnf)
MESSAGE_DEF(NAS_DOWNLINK_DATA_IND,      MESSAGE_PRIORITY_MED,       NasDlDataInd,               nas_dl_data_ind)
