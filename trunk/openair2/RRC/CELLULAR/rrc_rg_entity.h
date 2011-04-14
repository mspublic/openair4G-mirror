/********************************************************************
                          rrc_rg_entity.h  -
                          -------------------
    begin                : Someday 2001
    copyright            : (C) 2001, 2008 by Eurecom
		created by					 : Lionel.Gauthier@eurecom.fr	
    modified by          : Michelle.Wetterwald@eurecom.fr
 ********************************************************************
  Data structure with RRC UE protocol parameters
 *******************************************************************/

#ifndef __RRC_RG_ENTITY_H__
#define __RRC_RG_ENTITY_H__
//-------------------------------------------------------------------
#include "platform_types.h"
#include "rrc_constant.h"
//#include "rrc_rg_bch.h"
//#include "rrm_config_structs.h"
//#include "rrc_rrm_data.h"
//#include "rrc_rg_variables.h"
//#include "rrc_rg_mbms_variables.h"

//-------------------------------------------------------------------

struct rrc_bs_entity {
/********************************************************************
*                        Protocol variables                         *
********************************************************************/
// implementation variables
  int protocol_state;       //values in rrc_constant.h
  int rg_initial_id;
  u32 local_connection_ref;
  u16 cell_id;
  int current_SFN;

//TMP  struct rrc_neighbor_cells rg_ngbr_list;
  char IMEI[14];
  u8  establishment_cause;
  u8  release_cause;
  u8  failure_cause;        // not mw
  u8  failure_indicator;    // boolean - not mw
  u32 next_MUI;     // for RLC AM
  int idata_xfer;   // flag for Initial data transfer

  int last_message_sent;
//Added MSG1
  u8  prot_error_indicator; // boolean
  u16 nextActivationTime;
  int next_state;
//TMP  struct rrc_rg_msg_infos rg_msg_infos;
//TMP  struct rrc_rg_trans accepted_trans[MAXTRANS];
//TMP  struct rrc_rg_trans rejected_trans[MAXTRANS];
//TMP  struct rrc_rg_trans rcved_trans;      //transaction id received in last message
  u16 c_rnti;
  u16 u_rnti;
//Added MSG2
  u8  am_RLC_ErrorIndicationRb2_3or4;
  u8  am_RLC_ErrorIndicationRb5orAbove;
  u8  rlc_Re_establishIndicatorRb2_3or4;
  u8  rlc_Re_establishIndicatorRb5orAbove;
  u8  cellUpdateCause;
  u8  rejectWaitTime;
  u16 ul_nas_message_lgth;
  mem_block_t *ul_nas_message_ptr;
  u16 dl_nas_message_lgth;
  u8 *dl_nas_message_ptr;
  u16 paging_message_lgth;
  u8 *paging_message_ptr;
//TMP  struct rrc_rg_rb_information rg_established_rbs[maxRB];
  u16 num_rb;
//   u8  qos_classes[MAXURAB];
//   u8  dscp_codes[MAXURAB];
  u16 requested_rbId;
  u16 requested_QoSclass;
  u16 requested_dscp;
  u16 requested_sapid;

// platform configuration
  int rrc_currently_updating;
//TMP  RRM_VARS saved_configuration;

// link to L1
  u8  rrc_rg_synch_ind;
  u8  rrc_rg_cctrch_synch[maxCCTrCH];
  u8  rrc_rg_cctrch[maxCCTrCH];
  u8  rrc_rg_outsynch_ind;
  u8  rrc_rg_cctrch_outsynch[maxCCTrCH];

// Control block for Broadcast
//TMP  struct rrc_rg_bch_blocks rg_bch_blocks;

// Control blocks for measures
  int rrc_rg_last_measurement;
  int rrc_rg_meas_to_activate;
//TMP  struct rrc_rg_meas_cmd rg_meas_cmd[MAXMEASTYPES];
//TMP  struct rrc_rg_meas_rep rg_meas_rep[MAXMEASTYPES];

// HNN Control block for MBMS
//TMP  rrc_rg_mbms_variables mbms;

// pointer to next message to transmit to NAS
  //   this is actually the start of a linked list
  mem_block_t *NASMessageToXmit;

// RT-fifo descriptors
  int rrc_rg_GC_fifo;
  int rrc_rg_NT_fifo;
  int rrc_rg_DCIn_fifo;
  int rrc_rg_DCOut_fifo;
//former parameters
  unsigned int rg_broadcast_counter; // temp for sending broadcast to NAS
  int rg_wait_establish_req;
  //1111 ***** dbl_lk_list_up  rrc_timers;
};

#endif
