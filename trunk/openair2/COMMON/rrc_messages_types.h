/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

*******************************************************************************/

/*
 * rrc_messages_types.h
 *
 *  Created on: Oct 24, 2013
 *      Author: winckel and Navid Nikaein
 */

#ifndef RRC_MESSAGES_TYPES_H_
#define RRC_MESSAGES_TYPES_H_

#include "as_message.h"
#include "rrc_types.h"
#include "s1ap_messages_types.h"
#include "RRC/LITE/MESSAGES/SystemInformationBlockType2.h"

//-------------------------------------------------------------------------------------------//
// Messages for RRC logging
#if defined(DISABLE_ITTI_XER_PRINT)
#include "BCCH-DL-SCH-Message.h"
#include "DL-CCCH-Message.h"
#include "DL-DCCH-Message.h"
#include "UE-EUTRA-Capability.h"
#include "UL-CCCH-Message.h"
#include "UL-DCCH-Message.h"

typedef BCCH_DL_SCH_Message_t   RrcDlBcchMessage;
typedef DL_CCCH_Message_t       RrcDlCcchMessage;
typedef DL_DCCH_Message_t       RrcDlDcchMessage;
typedef UE_EUTRA_Capability_t   RrcUeEutraCapability;
typedef UL_CCCH_Message_t       RrcUlCcchMessage;
typedef UL_DCCH_Message_t       RrcUlDcchMessage;
#endif

//-------------------------------------------------------------------------------------------//
// Defines to access message fields.
#define RRC_STATE_IND(mSGpTR)           (mSGpTR)->ittiMsg.rrc_state_ind

#define RRC_CONFIGURATION_REQ(mSGpTR)   (mSGpTR)->ittiMsg.rrc_configuration_req

#define NAS_CELL_SELECTION_REQ(mSGpTR)  (mSGpTR)->ittiMsg.nas_cell_selection_req
#define NAS_CONN_ESTABLI_REQ(mSGpTR)    (mSGpTR)->ittiMsg.nas_conn_establi_req
#define NAS_UPLINK_DATA_REQ(mSGpTR)     (mSGpTR)->ittiMsg.nas_ul_data_req

#define NAS_RAB_ESTABLI_RSP(mSGpTR)     (mSGpTR)->ittiMsg.nas_rab_est_rsp

#define NAS_CELL_SELECTION_CNF(mSGpTR)  (mSGpTR)->ittiMsg.nas_cell_selection_cnf
#define NAS_CELL_SELECTION_IND(mSGpTR)  (mSGpTR)->ittiMsg.nas_cell_selection_ind
#define NAS_PAGING_IND(mSGpTR)          (mSGpTR)->ittiMsg.nas_paging_ind
#define NAS_CONN_ESTABLI_CNF(mSGpTR)    (mSGpTR)->ittiMsg.nas_conn_establi_cnf
#define NAS_CONN_RELEASE_IND(mSGpTR)    (mSGpTR)->ittiMsg.nas_conn_release_ind
#define NAS_UPLINK_DATA_CNF(mSGpTR)     (mSGpTR)->ittiMsg.nas_ul_data_cnf
#define NAS_DOWNLINK_DATA_IND(mSGpTR)   (mSGpTR)->ittiMsg.nas_dl_data_ind

//-------------------------------------------------------------------------------------------//
typedef struct RrcStateInd_s{
    Rrc_State_t     state;
    Rrc_Sub_State_t sub_state;
} RrcStateInd;

// eNB: ENB_APP -> RRC messages
typedef struct RrcConfigurationReq_s {
    uint32_t            cell_identity;

    uint16_t            tac;

    uint16_t            mcc;
    uint16_t            mnc;
    uint8_t             mnc_digit_length;

  /*
    paging_drx_t        default_drx;

    lte_frame_type_t    frame_type;
    uint8_t             tdd_config;
    uint8_t             tdd_config_s;
    int16_t             eutra_band;
  */
  int16_t                 nb_cc;
  lte_frame_type_t        frame_type[MAX_NUM_CCs];
  uint8_t                 tdd_config[MAX_NUM_CCs];
  uint8_t                 tdd_config_s[MAX_NUM_CCs];
  lte_prefix_type_t       prefix_type[MAX_NUM_CCs];
  int16_t                 eutra_band[MAX_NUM_CCs];
  uint32_t                downlink_frequency[MAX_NUM_CCs];
  int32_t                 uplink_frequency_offset[MAX_NUM_CCs];

  int16_t                 Nid_cell[MAX_NUM_CCs];// for testing, change later
  int16_t                 N_RB_DL[MAX_NUM_CCs];// for testing, change later
  int	                  nb_antennas_tx[MAX_NUM_CCs];   
  int                     nb_antennas_rx[MAX_NUM_CCs];   
  long                    prach_root[MAX_NUM_CCs];   
  long                    prach_config_index[MAX_NUM_CCs]; 
  BOOLEAN_t               prach_high_speed[MAX_NUM_CCs]; 
  long                    prach_zero_correlation[MAX_NUM_CCs]; 
  long                    prach_freq_offset[MAX_NUM_CCs]; 
  long                    pucch_delta_shift[MAX_NUM_CCs]; 
  long                    pucch_nRB_CQI[MAX_NUM_CCs]; 
  long                    pucch_nCS_AN[MAX_NUM_CCs];
#ifndef Rel10 
  long                    pucch_n1_AN[MAX_NUM_CCs]; 
#endif
  long                    pdsch_referenceSignalPower[MAX_NUM_CCs]; 
  long                    pdsch_p_b[MAX_NUM_CCs]; 
  long                    pusch_n_SB[MAX_NUM_CCs];
  long                    pusch_hoppingMode[MAX_NUM_CCs];
  long                    pusch_hoppingOffset[MAX_NUM_CCs];
  BOOLEAN_t               pusch_enable64QAM[MAX_NUM_CCs]; 
  BOOLEAN_t               pusch_groupHoppingEnabled[MAX_NUM_CCs]; 
  long                    pusch_groupAssignment[MAX_NUM_CCs]; 
  BOOLEAN_t               pusch_sequenceHoppingEnabled[MAX_NUM_CCs]; 
  long                    pusch_nDMRS1[MAX_NUM_CCs]; 
  long                    phich_duration[MAX_NUM_CCs]; 
  long                    phich_resource[MAX_NUM_CCs]; 
  BOOLEAN_t               srs_enable[MAX_NUM_CCs]; 
  long                    srs_BandwidthConfig[MAX_NUM_CCs]; 
  long                    srs_SubframeConfig[MAX_NUM_CCs]; 
  BOOLEAN_t               srs_ackNackST[MAX_NUM_CCs]; 
  BOOLEAN_t               srs_MaxUpPts[MAX_NUM_CCs]; 
  long                    pusch_p0_Nominal[MAX_NUM_CCs]; 
  long                    pusch_alpha[MAX_NUM_CCs]; 
  long                    pucch_p0_Nominal[MAX_NUM_CCs]; 
  long                    msg3_delta_Preamble[MAX_NUM_CCs]; 
  long                    ul_CyclicPrefixLength[MAX_NUM_CCs];
  e_DeltaFList_PUCCH__deltaF_PUCCH_Format1                    pucch_deltaF_Format1[MAX_NUM_CCs]; 
  e_DeltaFList_PUCCH__deltaF_PUCCH_Format1b                   pucch_deltaF_Format1b[MAX_NUM_CCs]; 
  e_DeltaFList_PUCCH__deltaF_PUCCH_Format2                    pucch_deltaF_Format2[MAX_NUM_CCs]; 
  e_DeltaFList_PUCCH__deltaF_PUCCH_Format2a                   pucch_deltaF_Format2a[MAX_NUM_CCs]; 
  e_DeltaFList_PUCCH__deltaF_PUCCH_Format2b                   pucch_deltaF_Format2b[MAX_NUM_CCs]; 
  long                    rach_numberOfRA_Preambles[MAX_NUM_CCs]; 
  BOOLEAN_t               rach_preamblesGroupAConfig[MAX_NUM_CCs];
  long	                  rach_sizeOfRA_PreamblesGroupA[MAX_NUM_CCs];
  long	                  rach_messageSizeGroupA[MAX_NUM_CCs];
  e_RACH_ConfigCommon__preambleInfo__preamblesGroupAConfig__messagePowerOffsetGroupB	                  rach_messagePowerOffsetGroupB[MAX_NUM_CCs];
  long                    rach_powerRampingStep[MAX_NUM_CCs]; 
  long                    rach_preambleInitialReceivedTargetPower[MAX_NUM_CCs]; 
  long                    rach_preambleTransMax[MAX_NUM_CCs]; 
  long                    rach_raResponseWindowSize[MAX_NUM_CCs]; 
  long                    rach_macContentionResolutionTimer[MAX_NUM_CCs];
  long                    rach_maxHARQ_Msg3Tx[MAX_NUM_CCs];
  long                    bcch_modificationPeriodCoeff[MAX_NUM_CCs];
  long                    pcch_defaultPagingCycle[MAX_NUM_CCs];
  long                    pcch_nB[MAX_NUM_CCs];
  long                    ue_TimersAndConstants_t300[MAX_NUM_CCs];
  long                    ue_TimersAndConstants_t301[MAX_NUM_CCs];
  long                    ue_TimersAndConstants_t310[MAX_NUM_CCs];
  long                    ue_TimersAndConstants_t311[MAX_NUM_CCs];
  long                    ue_TimersAndConstants_n310[MAX_NUM_CCs];
  long                    ue_TimersAndConstants_n311[MAX_NUM_CCs];
  
} RrcConfigurationReq;

// UE: NAS -> RRC messages
typedef cell_info_req_t         NasCellSelectionReq;
typedef nas_establish_req_t     NasConnEstabliReq;
typedef ul_info_transfer_req_t  NasUlDataReq;

typedef rab_establish_rsp_t     NasRabEstRsp;

// UE: RRC -> NAS messages
typedef cell_info_cnf_t         NasCellSelectionCnf;
typedef cell_info_ind_t         NasCellSelectionInd;
typedef paging_ind_t            NasPagingInd;
typedef nas_establish_cnf_t     NasConnEstabCnf;
typedef nas_release_ind_t       NasConnReleaseInd;
typedef ul_info_transfer_cnf_t  NasUlDataCnf;
typedef dl_info_transfer_ind_t  NasDlDataInd;

#endif /* RRC_MESSAGES_TYPES_H_ */
