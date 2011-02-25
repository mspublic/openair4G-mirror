/*________________________LAYER2/MAC/defs.h________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/

 
#ifndef __LAYER2_MAC_DEFS_H__
#define __LAYER2_MAC_DEFS_H__



#ifdef USER_MODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

//#include "COMMON/openair_defs.h"

#include "COMMON/platform_constants.h"
#include "COMMON/mac_rrc_primitives.h"
#include "PHY/defs.h"

//#ifdef PHY_EMUL
//#include "SIMULATION/PHY_EMULATION/impl_defs.h"
//#endif

/** @addtogroup _openair_mac_layer_specs_ 
* @{
 This subclause gives an overview of the mechanisms and interfaces provided MAC Layer.

* @}
*/


/** @defgroup _mac_impl_ MAC Layer Reference Implementation
 * @ingroup _ref_implementation_
 * @{
 */

#define BCCH_PAYLOAD_SIZE_MAX 128  
#define CCCH_PAYLOAD_SIZE_MAX 128    
#define SCH_PAYLOAD_SIZE_MAX 1024
/// Logical channel ids from 36-311 (Note BCCH is not specified in 36-311)
#define BCCH 3
#define CCCH 0
#define DCCH 1
#define DTCH_BD 2
#define DTCH    4
#define DTCH_OFFSET DTCH+NB_RAB_MAX 

#ifdef USER_MODE
#define printk printf
#endif //USER_MODE


#define MAX_NUM_RB 8
#define MAX_NUM_CE 5

/*! \brief  DCI_PDU Primitive.  This data structure reflects the DL control-plane traffic for the current miniframe.*/

#define NB_RA_PROC_MAX 4

typedef struct {
  u8 E:1;
  u8 T:1;
  u8 RAPID:6;
} __attribute__((__packed__))RA_HEADER_RAPID;

typedef struct {
  u8 E:1;
  u8 T:1;
  u8 R:2;
  u8 BI:4;
} __attribute__((__packed__))RA_HEADER_BI;

typedef struct {
  u8 R:1;
  u16 Timing_Advance_Command:11;
  u8 hopping_flag:1;
  u16 rb_alloc:10;
  u8 mcs:4;
  u8 TPC:3;
  u8 UL_delay:1;
  u8 cqi_req:1;
  u16 t_crnti;
} __attribute__((__packed__))RAR_PDU;

typedef struct {
  u8 LCID:5;
  u8 E:1;
  u8 R:2;
  u8 L:7;
  u8 F:1;
} __attribute__((__packed__))SCH_SUBHEADER_SHORT;

typedef struct {
  u8 LCID:5;
  u8 E:1;
  u8 R:2;
  u16 L:7;
  u8 F:1;
  u8 L2:8;
} __attribute__((__packed__))SCH_SUBHEADER_LONG;

typedef struct {
  u8 LCID:5;
  u8 E:1;
  u8 R:2;
} __attribute__((__packed__))SCH_SUBHEADER_FIXED;

typedef struct {
  u8 Buffer_size:6;
  u8 LCGID:2;
} __attribute__((__packed__))BSR_SHORT;

typedef BSR_SHORT BSR_TRUNCATED;

typedef struct {
  u8 Buffer_size0:6;
  u8 Buffer_size1:6;
  u8 Buffer_size2:6;
  u8 Buffer_size3:6;
} __attribute__((__packed__))BSR_LONG;

typedef struct {
  u8 TA:6;
  u8 R:2;
} __attribute__((__packed__))TIMING_ADVANCE_CMD;

typedef struct {
  u8 PH:6;
  u8 R:2;
} __attribute__((__packed__))POWER_HEADROOM_CMD;


typedef struct {

  u8 Num_ue_spec_dci ; 
  u8 Num_common_dci  ; 
  DCI_ALLOC_t dci_alloc[NUM_DCI_MAX] ;
} DCI_PDU;

typedef struct {
  u8 payload[CCCH_PAYLOAD_SIZE_MAX] ;/*!< \brief CCCH payload */
} __attribute__((__packed__))CCCH_PDU;

typedef struct {
  u8 payload[BCCH_PAYLOAD_SIZE_MAX] ;/*!< \brief CCCH payload */
} __attribute__((__packed__))BCCH_PDU;

// DLSCH LCHAN IDs
#define CCCH_LCHANID 0
#define UE_CONT_RES 28
#define TIMING_ADV_CMD 29
#define DRX_CMD 30
#define SHORT_PADDING 31

// ULSCH LCHAN IDs
#define POWER_HEADROOM 26
#define CRNTI 27
#define TRUNCATED_BSR 28
#define SHORT_BSR 29
#define LONG_BSR 30

/*! \brief Downlink SCH PDU Structure
 */
typedef struct {
  s8 payload[8][SCH_PAYLOAD_SIZE_MAX];         /*!< \brief SACH payload */
  u16 Pdu_size[8];
} __attribute__ ((__packed__)) DLSCH_PDU;

/*! \brief Uplink SCH PDU Structure
 */
typedef struct {
  s8 payload[SCH_PAYLOAD_SIZE_MAX];         /*!< \brief SACH payload */
  u16 Pdu_size;
} __attribute__ ((__packed__)) ULSCH_PDU;

#include "PHY/impl_defs_top.h"

typedef enum {
  S_UL_NONE =0,
  S_UL_WAITING,
  S_UL_SCHEDULED, 
  S_UL_BUFFERED,  
  S_UL_NUM_STATUS
} UE_ULSCH_STATUS;

typedef enum {
  S_DL_NONE =0,
  S_DL_WAITING,
  S_DL_SCHEDULED, 
  S_DL_BUFFERED,  
  S_DL_NUM_STATUS
} UE_DLSCH_STATUS;

// temp struct for sched
typedef struct {
  
  u16 rnti;
  u16 subframe;
  u16 serving_num;  
  UE_ULSCH_STATUS status;
} eNB_ULSCH_INFO;
// temp struct for sched
typedef struct {
  
  u16 rnti;
  u16 weigth;
  u16 subframe;
  u16 serving_num;  
  UE_DLSCH_STATUS status;
} eNB_DLSCH_INFO;

typedef struct{
  /// C-RNTI of UE
  u16 rnti;

  // PHY interface info

  /// DCI format for DLSCH
  u16 DLSCH_dci_fmt;
  
  /// Current Aggregation Level for DCI
  u8 DCI_aggregation_min;

  /// 
  u8 DLSCH_dci_size_bits;

  /// DCI buffer for DLSCH
  u8 DLSCH_DCI[(MAX_DCI_SIZE_BITS>>3)+1];

  /// DCI buffer for ULSCH
  u8 ULSCH_DCI[(MAX_DCI_SIZE_BITS>>3)+1];

  // Logical channel info for link with RLC

} UE_TEMPLATE;

typedef struct {
  /// Size of DCI for RA-Response (bytes)
  u8 RA_dci_size_bytes1;
  /// Size of DCI for RA-Response (bits)
  u8 RA_dci_size_bits1;
  /// Actual DCI to transmit for RA-Response
  u8 RA_alloc_pdu1[(MAX_DCI_SIZE_BITS>>3)+1];
  /// DCI format for RA-Response (should be 1A)
  u8 RA_dci_fmt1;
  /// Size of DCI for RRCConnectionSetup/ContRes (bytes)  
  u8 RA_dci_size_bytes2;
  /// Size of DCI for RRCConnectionSetup/ContRes (bits)  
  u8 RA_dci_size_bits2;
  /// Actual DCI to transmit for RRCConnectionSetup/ContRes
  u8 RA_alloc_pdu2[(MAX_DCI_SIZE_BITS>>3)+1];
  /// DCI format for RRCConnectionSetup/ContRes (should be 1A)
  u8 RA_dci_fmt2;
  /// Flag to indicate the eNB should generate RAR.  This is triggered by detection of PRACH
  u8 generate_rar;
  /// Flag to indicate the eNB should generate RRC.  This is triggered by first ULSCH reception at eNB for new user.
  u8 generate_rrcconnsetup;
  /// UE RNTI allocated during RAR
  u16 rnti;
  /// Received UE Contention Resolution Identifier (RRCConnectionRequest)
  u8 cont_res_id[6];
  /// Timing offset indicated by PHY
  s16 timing_offset;
} RA_TEMPLATE;

typedef struct{
  /// 
  u16 Node_id;
  /// Outgoing DCI for PHY generated by eNB scheduler
  DCI_PDU DCI_pdu;
  /// Outgoing BCCH pdu for PHY
  BCCH_PDU BCCH_pdu;
  /// Outgoing CCCH pdu for PHY
  CCCH_PDU CCCH_pdu;
  /// Outgoing DLSCH pdu for PHY
  DLSCH_PDU DLSCH_pdu[NB_CNX_CH+1][2];
  /// DCI template and MAC connection parameters for UEs
  UE_TEMPLATE UE_template[NB_CNX_CH];
  /// DCI template and MAC connection parameters for RA processes
  RA_TEMPLATE RA_template[NB_RA_PROC_MAX];
}CH_MAC_INST;




typedef struct{
  u16 Node_id;
  CCCH_PDU CCCH_pdu;
  DLSCH_PDU DLSCH_pdu[NB_CNX_UE][2];
  //ULSCH_PDU DLSCH_pdu[NB_CNX_UE][2];
}UE_MAC_INST;











//main.c

void chbch_phy_sync_success(u8 Mod_id,u8 CH_index);
void mrbch_phy_sync_failure(u8 Mod_id,u8 Free_ch_index);
int mac_top_init(void);
char layer2_init_mr(u8 Mod_id);
char layer2_init_ch(u8 Mod_id, u8 Free_ch_index); 
void mac_switch_node_function(u8 Mod_id);
int mac_init_global_param(void);
void mac_top_cleanup();
void mac_UE_out_of_sync_ind(u8 Mod_id, u16 CH_index);


// eNB functions
void eNB_dlsch_ulsch_scheduler(u8 Mod_id, u8); 
u16  fill_rar(u8 Mod_id,u8 *dlsch_buffer,u16 N_RB_UL, u8 input_buffer_length);
void terminate_ra_proc(u8 Mod_id,u16 UE_id, u8 *l3msg);
void initiate_ra_proc(u8 Mod_id, u16 preamble_index,s16 timing_offset,u8 sect_id);
void rx_sdu(u8 Mod_id,u16 rnti, u8 *sdu);
void mrbch_phy_sync_failure(u8 Mod_id,u8 Free_ch_index);
DCI_PDU *get_dci_sdu(u8 Mod_id,u8 subframe);
u8 *get_dlsch_sdu(u8 Mod_id,u16 rnti,u8 TBindex);

void init_ue_sched_info(void);
void add_ue_ulsch_info(u8 Mod_id, u8 UE_id, u8 subframe,UE_ULSCH_STATUS status);
void add_ue_dlsch_info(u8 Mod_id, u8 UE_id, u8 subframe,UE_DLSCH_STATUS status);
s8 find_UE_id(u8 Mod_id,u16 rnti) ;
s16 find_UE_RNTI(u8 Mod_id, u8 UE_id);
s8 find_active_UEs(u8 Mod_id);
u16 find_ulgranted_UEs(u8 Mod_id);
u16 find_dlgranted_UEs(u8 Mod_id);
u8 process_ue_cqi (u8 Mod_id, u8 UE_id);
u8 schedule_next_ulue(u8 Mod_id, u8 UE_id,u8 subframe);
u8 schedule_next_dlue(u8 Mod_id, u8 UE_id,u8 subframe);
u8 get_ue_weight(u8 Mod_id, u8 UE_id);

// UE functions
void out_of_sync_ind(u8 Mod_id,u16);
void ue_decode_si(u8 Mod_id, u8 CH_index, void *pdu, u16 len);
void ue_send_sdu(u8 Mod_id,u8 *sdu,u8 CH_index);
void ue_get_sdu(u8 Mod_id,u8 CH_index,u8 *ulsch_buffer,u16 buflen);
u8* ue_get_rach(u8 Mod_id,u8 CH_index);
u16 ue_process_rar(u8 Mod_id,u8 *dlsch_buffer,u16 *t_crnti);
int l2_init(LTE_DL_FRAME_PARMS *frame_parms);
int mac_init(void);

s8 add_new_ue(u8 Mod_id, u16 rnti);
s8 mac_remove_ue(u8 Mod_id, u8 UE_id);

void ue_scheduler(u8 Mod_id, u8 subframe);

/*@}*/
#endif /*__LAYER2_MAC_DEFS_H__ */ 



