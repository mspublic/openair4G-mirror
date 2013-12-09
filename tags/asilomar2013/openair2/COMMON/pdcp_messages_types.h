/*
 * pdcp_messages_types.h
 *
 *  Created on: Oct 24, 2013
 *      Author: winckel
 */

#ifndef PDCP_MESSAGES_TYPES_H_
#define PDCP_MESSAGES_TYPES_H_

//-------------------------------------------------------------------------------------------//
// Defines to access message fields.
#define RRC_DCCH_DATA_REQ(mSGpTR)               (mSGpTR)->ittiMsg.rrc_dcch_data_req
#define RRC_DCCH_DATA_IND(mSGpTR)               (mSGpTR)->ittiMsg.rrc_dcch_data_ind

//-------------------------------------------------------------------------------------------//
// Messages between RRC and PDCP layers
typedef struct RrcDcchDataReq_s {
  uint32_t frame;
  uint8_t enb_flag;
  uint32_t rb_id;
  uint32_t muip;
  uint32_t confirmp;
  uint32_t sdu_size;
  uint8_t *sdu_p;
  uint8_t mode;
} RrcDcchDataReq;

typedef struct RrcDcchDataInd_s {
  uint32_t frame;
  uint8_t dcch_index;
  uint32_t sdu_size;
  uint8_t *sdu_p;
  uint8_t ue_index;
} RrcDcchDataInd;

#endif /* PDCP_MESSAGES_TYPES_H_ */
