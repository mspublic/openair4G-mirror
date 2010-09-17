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

  \section _Overview MAC-layer Architecture
 The following figure shows the architecture of the MAC layer for both basestations (Node-B) and
 terminals (UE).
 \image html mac_w3g4f_mac_channels.png "Openair MAC" width=15cm
 \image latex mac_w3g4f_mac_channels.png "Openair MAC" width=15cm

 The MAC layer is responsible for scheduling control plane and user traffic on the physical
 OFDMA resources.  On transmission, the inputs to the MAC layer are connected to data queues originating in
 the RLC layer which form the set of logical channels.  The control plane traffic is
 represented by the following logical channels:

 -# Broadcast Control Channel (BCCH) : This resource is a low bit-rate control channel used by the network 
 (via the basestation) for broadcasting basic information to users the cell served by a particular basestation.
 -# Common Control Channel (CCCH) : This resource is a low bit-rate control channel used both by user terminals and the 
 basestation during the attachment or association phase of a new user terminal.  Requests to join the cell are made by the terminal
 and ackowledgements are given by the basestation.
 -# Multicast Control Channel (MCCH): This resource is a low bit-rate control channel used by the network 
 (via the basestation) to dynamically dimension the resources of the Multicast traffic channels.
 -# Dedicated Control Channel (DCCH): This is a resource used by either the basestation or user terminal to relay access-layer 
 signaling information (link-layer return channels, RF measurement reporting, traffic measurement reporting, power control, etc.) 
 to the correspodent node. 

	 
 The user plane traffic is represented by the following logical channels:
 -# Multicast Traffic Channel (MTCH): This resource is a variable bit-rate traffic channel used by the network to 
 relay multicast information to the groups of users in the cell served by the basestation.
 -# Dedicated Traffic Channel (DTCH): This resource is a variable bit-rate traffic channel with negotiated QoS parameters
 used by the network or user terminals to relay data traffic.



 It is important to note that although dedicated resources are configured at the input of the MAC-layer, the physical resources 
 allocated in the scheduling entities (with exception of the CHBCH) are dynamically allocated with the granularity of the mini-frame 
 (nominally 2ms), and thus all physical resources are shared.  Furthermore, in the case of TDD deployments, the portion of bandwidth allocated 
 to uplink and downlink traffic is also dynamically adjusted at the granularity of the mini-frame.

 \section _CHBCH_SCHED CHBCH Scheduling
 The BCCH and CCCH (downlink) are multiplexed in the scheduling entity resposible for generation of the 
 CHBCH transport channel (\ref _CHBCH). In addition, the SACH allocations for both downlink and uplink traffic in the current mini-frame 
 are signaled in the MAC-layer PDUs DL_SACCH_PDU and UL_ALLOC_PDU.  The contents of a particular 
 DL_SACCH_PDU determine DL SACH allocations in the current mini-frame for a particular logical channel, along with sequencing, coding format 
 parameters and feedback information (PHY and MAC). The UL_ALLOC_PDU 
 contains the time and frequency allocations to be used by a particular logical
 channel during the uplink portion of the mini-frame. 
 The resulting MAC-layer primitive is described by the data structure CHBCH_PDU.

 \section _RACH_SCHED UL CCCH Scheduling
 The CCCH (uplink) is used exclusively during the attachment phase of the user terminal with a particular
 Node-B and corresponds to the only random-access resources allocated by the Node-B in the mini-frame. 
 These are the UL SACH allocations with the CCCH logical channel id.  The number of random-access
 resources scheduled in each mini-frame is dependent on the available resources and higher-layer parametrization.
 During the attachment phase, the UE scheduler randomly selects the resource to be used in the next mini-frame among the 
 set of allocated SACH CCCH resources.  The UE uses the multiuser pilot symbol corresponding to user index 0 and transmits only
 the portion in the sub-band chosen for the CCCH rather than the entire symbol as in the case of a regular UL SACH transmission.

 \section _SACH_SCHED SACH Scheduling
 The MCCH and DCCH (downlink) are multiplexed along with user-plane traffic (MTCH,DTCH) on the available SACH resources.
 The SACH resources are the collection of OFDMA subcarrier groups and symbols during the variable length SACH period.
 The SACH PDUs passed to the PHY layer are described by the structure SACH_PDU. The goal of SACH scheduling is to
 respect the negotiated QoS of each logical channel, while maximizing the aggregate spectral efficiency of the downlink and uplink
 data streams. The actual algorithm used by the Node-B is not specified.  

 As a general rule, MCCH, DCCH and MTCH do not use HARQ, or equivalently at most a single transmission round is used.  DTCH generally
 use HARQ with a maximum number of retransmission rounds determined by higher layer configuration (i.e. the delay class in mac_lchan_desc),
 which is at most 8, corresponding to the number of parallel HARQ processes.

 SACH scheduling makes use of up to 8 parallel HARQ processes per logical channel in order to maximize throughput
 and benefit from superior channel conditions.

 At the start of each mini-frame, the Node-B scheduler determines the physical allocations (OFDM symbol, OFDM sub-bands, transmit antennas)
 for downlink and uplink logical channels and correspondingly parametrizes the DL_SACCH_PDU and UL_ALLOC_PDU data structures.  For logical 
 channels using HARQ (DCCH/DTCH), it manages the HARQ retransmission rounds in conjunction with the PHY
 channel decoder and packet integretity verification algorithm.  At each HARQ round a new coding format and power level 
 can be chosen for the redundancy bits to be transmitted, which are applied uniformly to all HARQ processes. UL power control 
 and HARQ acknowledgements are also computed for corresponding UL flow.

 The DL_SACCH_PDU contains the HARQ sequencing information which indicates the active HARQ processes and their progress indices.

 The UE SACH scheduler parses the UL_ALLOC_PDU to find its allocations and processes the next retransmission round of the HARQ process for
 the allocated logical channels as well as performing DL power control on the correspoding DL flows, acknowledging receipt
 of a HARQ PDU, and relaying the quantized logical channel PDU backlog. The latter are reflected by the UL_SACCH_FB data structure.
 The UE SACH scheduler can select transmit power and coding format with the granularity of the mini-frame.  These allocations are reflected 
 in the UL_SACCH_PDU which precedes the corresponding SACH resources.  The UL_SACCH_PDU must use the lowest spectral-efficiency coding format 
 and is not subject to HARQ since it must be correctly decoded so that the HARQ process of the corresponding SACH can make use of the coded 
 symbols in current mini-frame.  

 Processing of the UL_ALLOC_PDU at the UE must be sufficiently efficient for the UL_SACH to be configured in the same mini-frame. 

 Adjacent cell interference should be managed by the Node-Bs in a given region in a decentralized fashion 
 using DL power control coupled with resource randomization across HARQ retransmission rounds combined with 
 dual-antenna reception at the UE. 
* @}
*/


/** @defgroup _mac_impl_ MAC Layer Reference Implementation
 * @ingroup _ref_implementation_
 * @{
 */

#define TB_SIZE_MAX 52
#define NB_TB_BUFF_MAX  32


#define NUMBER_DL_SACH_MAX 6
#define NUMBER_UL_SACH_MAX 6

#define BCCH_PAYLOAD_SIZE_MAX 20  // BCCH Fragments
#define CCCH_PAYLOAD_SIZE_MAX 28    // CCCH Fragments

#define NB_DL_SCHED_MAX NUMBER_DL_SACH_MAX
#define NB_UL_SCHED_MAX NUMBER_UL_SACH_MAX 


#define SCH_PAYLOAD_SIZE_MAX (TB_SIZE_MAX * NB_TB_BUFF_MAX) //1600
#define RACH_PAYLOAD_SIZE_MAX 20//until solving pb of fifos (24)
#define MRBCH_PAYLOAD_SIZE_MAX 20//until solving pb of fifos (24)
#define NB_RACH_MAX 2

#define DUMMY_BCCH_SIZE_BYTES 40
#define DUMMY_CCCH_SIZE_BYTES 40
#define DUMMY_RACH_SIZE_BYTES 16



#define SCHED_LONG_MAW 32 
#define SCHED_SHORT_MAW 8 
#define DCCH_SCHED_PERIOD 33



//#define UL_FREQ_ALLOC 0x0001; // 2 freq group minimum allocation
//#define DL_FREQ_ALLOC 0x0001; // 2 freq group minimum allocation

 
//#define NB_NODE_MAX 50

#define BCCH 3
#define CCCH 0
#define DCCH 1
#define DTCH_BD 2
#define DTCH    4
#define DTCH_OFFSET DTCH+NB_RAB_MAX 
//#ifdef MESH
#define DTCH_DIL 5
//#endif //MESH
#define RX 0
#define TX 1
#define NB_CH_MAX 8

#define LCHAN_IDLE 0
#define MAC_SCHED_TX 1
#define MAC_TX_READY 2
#define MAC_TX_DONE 3
#define MAC_TX_OK 4

#define MAC_SCHED_RX_REQ 5
#define MAC_SCHED_RX_READY 6
#define MAC_SCHED_RX_OK 7
#define MAC_RX_READY 8
#define MAC_RX_OK 9

#define NUMBER_HARQ_PROCESS_MAX 32
#define MAX_NUMBER_TB_PER_LCHAN 32//NUMBER_HARQ_PROCESS_MAX
#define NUMBER_OF_MEASUREMENT_SUBBANDS 16//NUMBER_OF_FREQUENCY_GROUPS 
#define MAX_NB_SCHED 25//NUMBER_OF_FREQUENCY_GROUPS 



#define USEFUL_CARRIER_OFFSET_dB 2
#ifdef USER_MODE
#define printk printf
#endif //USER_MODE
#define w3g4free_mac_print(level,fmt,args) {if (level > DEBUG_THRESHOLD) msg(fmt,args); }


#define LCHAN_PAYLOAD_MAX (TB_SIZE_MAX * NB_TB_BUFF_MAX) //from rrm




#define NB_REQ_MAX 16



#define SCH_OFFSET 0x3f  //2 bits for UL_SCH Pilot index
#define SCH_SHIFT 2 //2 bits for UL_SCH Pilot index


#define SINR_THRES0 (-3)
#define SINR_THRES1 (0)
#define SINR_THRES2 (3)

#define MAX_NUM_RB 11
#define MAX_NUM_CE 5

/*! \brief  DCI_PDU Primitive.  This data structure reflects the DL control-plane traffic for the current miniframe.*/
#define NUM_DCI_MAX 32

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
  s8 Ccch_payload[CCCH_PAYLOAD_SIZE_MAX] ;/*!< \brief CCCH payload */
} __attribute__((__packed__))CCCH_PDU;

typedef struct {

  u8 Num_bytes_bcch ; /*!< \brief Number of bytes contained in the current BCCH payload */
  s8 Bcch_payload[BCCH_PAYLOAD_SIZE_MAX] ;/*!< \brief CCCH payload */
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

#ifdef PHY_EMUL
#include "SIMULATION/PHY_EMULATION/impl_defs.h"
#else 
#include "PHY/impl_defs_top.h"
#endif


typedef struct{
  LCHAN_ID   Lchan_id; //only the LChan_id.Index is needed to identify LC in the CH side
  u8 Lchan_type; 
  u16 W_idx; 
  MAC_MEAS_REQ_ENTRY Meas_entry;
  LCHAN_DESC Lchan_desc[2]; //TX/RX LCHAN DESCRIPTOR
  u8 Current_payload_tx[TB_SIZE_MAX*NB_TB_BUFF_MAX];
  u8 Current_payload_rx[TB_SIZE_MAX*NB_TB_BUFF_MAX];
  u8 Lchan_status_tx,Lchan_status_rx;//???
  u32 NB_TX;
  u32 NB_TX_LAST;
  u32 NB_BW_REQ_TX;
  u32 NB_BW_REQ_RX;
  u32 output_rate;
  u32 NB_RX;
  u32 Tx_rate;
  u32 Rx_rate;
  u32 Rx_rate_temp;
  u32 Tx_rate_temp;
  u32 Arrival_rate;
  u32 Req_rate;
  u32 Spec_eff;
}LCHAN_INFO;
#define LCHAN_INFO_SIZE sizeof(LCHAN_INFO)


typedef struct{
  LCHAN_ID   Lchan_id; //UNIDIRECTIONEL, 
  u8 Lchan_type; 
  u8 Qdepth;  //==> Tx arrival rate  //==> which rate to achive target rate over SINR :PFS 
  u8 Rate;
  MAC_MEAS_REQ_ENTRY Meas_entry;
  LCHAN_DESC Lchan_desc; //TX/RX LCHAN DESCRIPTOR
  u8 Nb_tb;
  u8 Lchan_status;//???
  PHY_RESOURCES Phy_resources;
  // HARQ control
  u8 Num_scheduled_tb[2];                            // Number of scheduled TBs in even/odd TTI
  u16 Rx_ack_map[2];                                 // Received ACKs from opposite link in even/odd TTI
  u16 Active_process_map_tx[2];                      // Active HARQ process map (TX) in even/odd TTI
  u16 New_process_map_tx[2];                      // New HARQ process map (TX) in even/odd TTI
  u8 Round_indices_tx[2][NUMBER_HARQ_PROCESS_MAX]; // HARQ Round indices (TX) for active processes in even/odd TTI
  u8 Round_indices_rx[2][NUMBER_HARQ_PROCESS_MAX]; // HARQ Round indices (TX) for active processes in even/odd TTI
}LCHAN_INFO_DIL;

#define LCHAN_INFO_DIL_SIZE sizeof(LCHAN_INFO_DIL)


typedef struct LCHAN_INFO_TABLE_ENTRY{
  LCHAN_INFO Lchan_info;
  u8 Active;
  //u8 Config_status;
  u32 Next_sched_limit;
  //  struct LCHAN_INFO_TABLE_ENTRY *Next_entry;
  //struct LCHAN_INFO_TABLE_ENTRY *Prev_entry;
}LCHAN_INFO_TABLE_ENTRY;
#define LCHAN_INFO_TABLE_ENTRY_SIZE sizeof(LCHAN_INFO_TABLE_ENTRY)

typedef struct LCHAN_INFO_DIL_TABLE_ENTRY{
  LCHAN_INFO_DIL Lchan_info_dil;
  u8 Active;
  //u8 Config_status;
  u32 Next_sched_limit;
  //  struct LCHAN_INFO_TABLE_ENTRY *Next_entry;
  //struct LCHAN_INFO_TABLE_ENTRY *Prev_entry;
}LCHAN_INFO_DIL_TABLE_ENTRY;
#define LCHAN_INFO_DIL_TABLE_ENTRY_SIZE sizeof(LCHAN_INFO_DIL_TABLE_ENTRY)


typedef struct {
  LCHAN_ID Lchan_id;
  PHY_RESOURCES Phy_resources;
  u8 Nb_tb;
} RX_SCHED;

typedef struct{
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  //  UL_ALLOC_PDU UL_alloc_pdu;
  //  s8 Activation_tti;
} TX_OPS;


typedef struct{
  u16 Node_id;
  u8 Num_dlsch;
  u8 Num_ulsch;
  DCI_PDU DCI_pdu;
  BCCH_PDU BCCH_pdu;
  CCCH_PDU CCCH_pdu;
  LCHAN_INFO_TABLE_ENTRY Bcch_lchan;
  LCHAN_INFO_TABLE_ENTRY Ccch_lchan;
  LCHAN_INFO_TABLE_ENTRY Dcch_lchan[NB_CNX_CH+1];
  LCHAN_INFO_TABLE_ENTRY Dtch_lchan[NB_RAB_MAX][NB_CNX_CH+1];
  LCHAN_INFO_DIL_TABLE_ENTRY Dtch_dil_lchan[NB_RAB_MAX][NB_CNX_CH+1][NB_CNX_CH-1];
  //MEAS_REQ_TABLE Meas_table;
  DEFAULT_CH_MEAS Def_meas[NB_CNX_CH+1];
  DLSCH_PDU DLSCH_pdu[NB_CNX_CH+1][2];
  ULSCH_PDU RX_UL_sach_pdu;
  //DL_MEAS DL_meas;
  RX_SCHED Rx_sched[3][NUMBER_UL_SACH_MAX]; 
  u8 Nb_rx_sched[3];
  s8 Sinr_sorted_table[NB_CNX_CH+1][MAX_NB_SCHED]; 
  s8 Sinr_sorted_index[NB_CNX_CH+1][MAX_NB_SCHED]; 
  u8 Nb_sched;
  //  u8 Sched_user[NB_CNX_CH+1];
  //  u16 UL_MAP_USE;
  //u16 DL_MAP_USE;
}CH_MAC_INST;




typedef struct{
  u16 Node_id;
  LCHAN_INFO_TABLE_ENTRY Bcch_lchan[NB_SIG_CNX_UE];
  LCHAN_INFO_TABLE_ENTRY Ccch_lchan[NB_SIG_CNX_UE];
  LCHAN_INFO_TABLE_ENTRY Dcch_lchan[NB_CNX_UE];
  LCHAN_INFO_TABLE_ENTRY Dtch_lchan[NB_RAB_MAX][NB_CNX_UE];
  LCHAN_INFO_TABLE_ENTRY Dtch_dil_lchan[NB_RAB_MAX][NB_SIG_CNX_UE][NB_CNX_CH-1];
  //MEAS_REQ_TABLE Meas_table;
  DEFAULT_UE_MEAS Def_meas[NB_SIG_CNX_UE];
  //  DCI_PDU RXDCI_pdu[NB_CNX_UE];
  DLSCH_PDU DLSCH_pdu[NB_CNX_UE][2];
  //  UL_MEAS UL_meas[];
  //  DL_MEAS DL_meas[NB_CNX_UE];
  RX_SCHED Rx_sched[NB_CNX_UE][3][NUMBER_UL_SACH_MAX]; 
  u8 Nb_rx_sched[NB_CNX_UE][3];
  TX_OPS Tx_ops[NB_CNX_UE][3][NUMBER_UL_SACH_MAX];
  u8 Nb_tx_ops[NB_CNX_UE][3];
  u16 CH_ul_freq_map[NB_SIG_CNX_UE];
  u8 NB_decoded_chbch;
}UE_MAC_INST;











//main.c

void chbch_phy_sync_success(u8 Mod_id,u8 CH_index);
void mrbch_phy_sync_failure(u8 Mod_id,u8 Free_ch_index);
int mac_top_init(void);
s8 layer2_init_mr(u8 Mod_id);
s8 layer2_init_ch(u8 Mod_id, u8 Free_ch_index);
void mac_switch_node_function(u8 Mod_id);
int mac_init_global_param(void);
void mac_top_cleanup(u8 Mod_id);
void mac_UE_out_of_sync_ind(u8 Mod_id, u16 CH_index);


// eNB functions
void eNB_dlsch_ulsch_scheduler(u8 Mod_id, u8); 
u16  fill_rar(u8 Mod_id,u8 *dlsch_buffer,u16 N_RB_UL, u8 input_buffer_length,u16 timing_advance_cmd);
void terminate_ra_proc(u8 Mod_id,u16 UE_id, u8 *l3msg);
void initiate_ra_proc(u8 Mod_id,u16 preamble);
void rx_sdu(u8 Mod_id,u16 UE_id, u8 *sdu);
void mrbch_phy_sync_failure(u8 Mod_id,u8 Free_ch_index);
DCI_PDU *get_dci_sdu(u8 Mod_id,u8 subframe);
u8 *get_dlsch_sdu(u8 Mod_id,u16 rnti,u8 TBindex);

// UE functions
void out_of_sync_ind(u8 Mod_id,u16);
void ue_decode_si(u8 Mod_id, u8 CH_index, void *pdu, u16 len);
void ue_send_sdu(u8 Mod_id,u8 *sdu,u8 CH_index);
void ue_get_sdu(u8 Mod_id,u8 CH_index,u8 *ulsch_buffer,u16 buflen);
u8* ue_get_rach(u8 Mod_id,u8 CH_index);
u16 ue_process_rar(u8 Mod_id,u8 *dlsch_buffer,u16 *t_crnti);

/*@}*/
#endif /*__LAYER2_MAC_DEFS_H__ */ 



