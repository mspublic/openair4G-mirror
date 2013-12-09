/*
 * mac_messages_types.h
 *
 *  Created on: Oct 24, 2013
 *      Author: winckel
 */

#ifndef MAC_MESSAGES_TYPES_H_
#define MAC_MESSAGES_TYPES_H_

//-------------------------------------------------------------------------------------------//
// Defines to access message fields.
#define RRC_MAC_IN_SYNC_IND(mSGpTR)             (mSGpTR)->ittiMsg.rrc_mac_in_sync_ind
#define RRC_MAC_OUT_OF_SYNC_IND(mSGpTR)         (mSGpTR)->ittiMsg.rrc_mac_out_of_sync_ind

#define RRC_MAC_BCCH_DATA_REQ(mSGpTR)           (mSGpTR)->ittiMsg.rrc_mac_bcch_data_req
#define RRC_MAC_BCCH_DATA_IND(mSGpTR)           (mSGpTR)->ittiMsg.rrc_mac_bcch_data_ind

#define RRC_MAC_CCCH_DATA_REQ(mSGpTR)           (mSGpTR)->ittiMsg.rrc_mac_ccch_data_req
#define RRC_MAC_CCCH_DATA_CNF(mSGpTR)           (mSGpTR)->ittiMsg.rrc_mac_ccch_data_cnf
#define RRC_MAC_CCCH_DATA_IND(mSGpTR)           (mSGpTR)->ittiMsg.rrc_mac_ccch_data_ind

#define RRC_MAC_MCCH_DATA_REQ(mSGpTR)           (mSGpTR)->ittiMsg.rrc_mac_mcch_data_req
#define RRC_MAC_MCCH_DATA_IND(mSGpTR)           (mSGpTR)->ittiMsg.rrc_mac_mcch_data_ind

// Some constants from "LAYER2/MAC/defs.h"
#define BCCH_SDU_SIZE                           (128)
#define CCCH_SDU_SIZE                           (128)
#define MCCH_SDU_SIZE                           (128)

//-------------------------------------------------------------------------------------------//
// Messages between RRC and MAC layers
typedef struct RrcMacInSyncInd_s {
  uint32_t frame;
  uint16_t enb_index;
} RrcMacInSyncInd;

typedef RrcMacInSyncInd RrcMacOutOfSyncInd;

typedef struct RrcMacBcchDataReq_s {
  uint32_t frame;
  uint32_t sdu_size;
  uint8_t sdu[BCCH_SDU_SIZE];
  uint8_t enb_index;
} RrcMacBcchDataReq;

typedef struct RrcMacBcchDataInd_s {
  uint32_t frame;
  uint32_t sdu_size;
  uint8_t sdu[BCCH_SDU_SIZE];
  uint8_t enb_index;
} RrcMacBcchDataInd;

typedef struct RrcMacCcchDataReq_s {
  uint32_t frame;
  uint32_t sdu_size;
  uint8_t sdu[CCCH_SDU_SIZE];
  uint8_t enb_index;
} RrcMacCcchDataReq;

typedef struct RrcMacCcchDataCnf_s {
  uint8_t enb_index;
} RrcMacCcchDataCnf;

typedef struct RrcMacCcchDataInd_s {
    uint32_t frame;
    uint32_t sdu_size;
    uint8_t sdu[CCCH_SDU_SIZE];
    uint8_t enb_index;
} RrcMacCcchDataInd;

typedef struct RrcMacMcchDataReq_s {
  uint32_t frame;
  uint32_t sdu_size;
  uint8_t sdu[MCCH_SDU_SIZE];
  uint8_t enb_index;
  uint8_t mbsfn_sync_area;
} RrcMacMcchDataReq;

typedef struct RrcMacMcchDataInd_s {
  uint32_t frame;
  uint32_t sdu_size;
  uint8_t sdu[MCCH_SDU_SIZE];
  uint8_t enb_index;
  uint8_t mbsfn_sync_area;
} RrcMacMcchDataInd;

#endif /* MAC_MESSAGES_TYPES_H_ */
