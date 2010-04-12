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



#include "COMMON/openair_defs.h"
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
#define NB_TB_BUFF_MAX  64


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

#define BCCH 0
#define CCCH 1
#define DCCH 2
#define DTCH_BD 3
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


/*! \brief  DCI_PDU Primitive.  This data structure reflects the DL control-plane traffic for the current miniframe.*/
#define NUM_DCI_MAX 32

typedef struct {
  unsigned char E:1;
  unsigned char T:1;
  unsigned char RAPID:6;
} __attribute__((__packed__))RA_HEADER_RAPID;

typedef struct {
  unsigned char E:1;
  unsigned char T:1;
  unsigned char R:2;
  unsigned char BI:4;
} __attribute__((__packed__))RA_HEADER_BI;

typedef struct {
  unsigned char R:1;
  unsigned short Timing_Advance_Command:11;
  unsigned char hopping_flag:1;
  unsigned short rb_alloc:10;
  unsigned char mcs:4;
  unsigned char TPC:3;
  unsigned char UL_delay:1;
  unsigned char cqi_req:1;
  unsigned short t_crnti;
} __attribute__((__packed__))RAR_PDU;

typedef struct {
  unsigned char R:2;
  unsigned char E:1;
  unsigned char LCID:5;
  unsigned char F:1;
  unsigned char L:7;
} __attribute__((__packed__))SCH_SUBHEADER_SHORT;

typedef struct {
  unsigned char R:2;
  unsigned char E:1;
  unsigned char LCID:5;
  unsigned char F:1;
  unsigned short L:15;
} __attribute__((__packed__))SCH_SUBHEADER_LONG;

typedef struct {
  unsigned char R:2;
  unsigned char E:1;
  unsigned char LCID:5;
} __attribute__((__packed__))SCH_SUBHEADER_FIXED;

typedef struct {
  unsigned char LCGID:2;
  unsigned char Buffer_size:6;
} __attribute__((__packed__))BSR_SHORT;

typedef struct {
  unsigned char Buffer_size0:6;
  unsigned char Buffer_size1:6;
  unsigned char Buffer_size2:6;
  unsigned char Buffer_size3:6;
} __attribute__((__packed__))BSR_LONG;

typedef struct {
  unsigned char R:2;
  unsigned char TA:6;
} __attribute__((__packed__))TIMING_ADVANCE_CMD;

typedef struct {
  unsigned char R:2;
  unsigned char PH:6;
} __attribute__((__packed__))POWER_HEADROOM_CMD;

typedef struct {

  unsigned char Num_ue_spec_dci ; /*!< \brief Number of SACH in the current DL_SACCH payload */ 
  unsigned char Num_common_dci  ; /*!< \brief Number of SACH available in the UL_SACH period */
  DCI_ALLOC_t dci_alloc[NUM_DCI_MAX] ;/*!< \brief Collection of DL_SACCH PDUs */
} __attribute__((__packed__))DCI_PDU;

typedef struct {
  char Ccch_payload[CCCH_PAYLOAD_SIZE_MAX] ;/*!< \brief CCCH payload */
} __attribute__((__packed__))CCCH_PDU;

typedef struct {

  unsigned char Num_bytes_bcch ; /*!< \brief Number of bytes contained in the current BCCH payload */
  char Bcch_payload[BCCH_PAYLOAD_SIZE_MAX] ;/*!< \brief CCCH payload */
} __attribute__((__packed__))BCCH_PDU;

// DLSCH LCHAN IDs
#define CCCH_LCHANID 0
#define UE_CONT_RES 28
#define TIMING_ADVANCE 29
#define DRX 30
#define PADDING 31

// ULSCH LCHAN IDs
#define POWER_HEADROOM 26
#define CRNTI 27
#define TRUNCATED_BSR 28
#define SHORT_BSR 29
#define LONG_BSR 30

/*! \brief Downlink SCH PDU Structure
 */
typedef struct {
#ifdef PHY_EMUL
  // unsigned char CH_id;
  //LCHAN_ID Lchan_id;   //H.A
  unsigned short Pdu_size[2];
#endif //PHY_EMUL
  char payload[2][SCH_PAYLOAD_SIZE_MAX];         /*!< \brief SACH payload */
  unsigned short Pdu_size;
} __attribute__ ((__packed__)) DLSCH_PDU;

/*! \brief Uplink SCH PDU Structure
 */
typedef struct {
  char payload[SCH_PAYLOAD_SIZE_MAX];         /*!< \brief SACH payload */
  unsigned short Pdu_size;
} __attribute__ ((__packed__)) ULSCH_PDU;

#ifdef PHY_EMUL
#include "SIMULATION/PHY_EMULATION/impl_defs.h"
#else 
#include "PHY/impl_defs_top.h"
#endif


typedef struct{
  LCHAN_ID   Lchan_id; //only the LChan_id.Index is needed to identify LC in the CH side

  unsigned short UE_CH_index; //ID of CH who configued the LC (Need this to identify LC in the UE side)
  unsigned char Lchan_type; 
  
  unsigned char Qdepth;  
  unsigned char Qdepth_temp; 
  unsigned short W_idx; 
  //unsigned char Qdepth;  //==> Rx arrival rate  //
  
/*  unsigned int NB_txmited;
  unsigned int NB_rxmited;
  unsigned int Tx_start_frame;
  unsigned int Rx_start_frame;
*/
  MAC_MEAS_REQ_ENTRY Meas_entry;

  LCHAN_DESC Lchan_desc[2]; //TX/RX LCHAN DESCRIPTOR
  unsigned char Nb_tb_tx;//active (in Mac buffers)
  unsigned char Nb_tb_rx;//active
  char Current_payload_tx[TB_SIZE_MAX*NB_TB_BUFF_MAX];
  char Current_payload_rx[TB_SIZE_MAX*NB_TB_BUFF_MAX];

  unsigned char Lchan_status_tx,Lchan_status_rx;//???
  PHY_RESOURCES Phy_resources_tx,Phy_resources_rx,Phy_resources_rx_sched;
  unsigned char Target_spec_eff_rx;
  unsigned char Dual_stream_flag_rx;
  unsigned char Target_spec_eff_tx;
  unsigned char Dual_stream_flag_tx;
  // HARQ control
  unsigned char Num_scheduled_tb[2];                            // Number of scheduled TBs in even/odd TTI
  unsigned short Rx_ack_map[2];                                 // Received ACKs from opposite link in even/odd TTI
  unsigned short Active_process_map_tx[2];                      // Active HARQ process map (TX) in even/odd TTI
  unsigned short New_process_map_tx[2];                      // New HARQ process map (TX) in even/odd TTI
  unsigned char Round_indices_tx[2][NUMBER_HARQ_PROCESS_MAX]; // HARQ Round indices (TX) for active processes in even/odd TTI
  unsigned char Round_indices_rx[2][NUMBER_HARQ_PROCESS_MAX]; // HARQ Round indices (TX) for active processes in even/odd TTI
  unsigned int NB_TX;
  unsigned int NB_TX_LAST;
  unsigned int NB_BW_REQ_TX;
  unsigned int NB_BW_REQ_RX;
  unsigned int output_rate;
  unsigned int NB_RX;
  unsigned int NB_RX_ERRORS;
  unsigned int NB_RX_SACCH_ERRORS;
  unsigned int NB_RX_SACH_ERRORS;
  unsigned int NB_RX_SACH_MISSING;
  unsigned char  Nb_sched_tb_ul;
  unsigned char  Nb_sched_tb_ul_temp;
  unsigned char  Nb_sched_tb_dl;
  unsigned char  Nb_sched_tb_dl_temp;
  unsigned int NB_TX_TB[64];
  unsigned int NB_RX_TB[64];
  unsigned int NB_RX_ERRORS_TB[64];
  char Sch_index;                                              //UL_SCH_PILOT Time index
  unsigned int Nb_tx_last_tti;
  unsigned int Nb_rx_last_tti;
  unsigned int Tx_rate;
  unsigned int Rx_rate;
  unsigned int Rx_rate_temp;
  unsigned int Tx_rate_temp;
  unsigned int Arrival_rate;
  unsigned int Req_rate;
  unsigned char Sched_flag;
  unsigned int Last_sched_tti;
  unsigned int Last_feedback_tti;
  unsigned int Spec_eff;
  unsigned char Bw_req_active;
  //  unsigned char Target_spec_eff_tx;
  //unsigned char Target_spec_eff_rx;
  //unsigned char Dual_stream_flag;
  //  unsigned char Direction;
}LCHAN_INFO;
#define LCHAN_INFO_SIZE sizeof(LCHAN_INFO)

typedef struct{
  LCHAN_ID   Lchan_id; //UNIDIRECTIONEL, 
  unsigned char Lchan_type; 
  unsigned char Qdepth;  //==> Tx arrival rate  //==> which rate to achive target rate over SINR :PFS 
  unsigned char Rate;
  MAC_MEAS_REQ_ENTRY Meas_entry;
  LCHAN_DESC Lchan_desc; //TX/RX LCHAN DESCRIPTOR
  unsigned char Nb_tb;
  unsigned char Lchan_status;//???
  PHY_RESOURCES Phy_resources;
  // HARQ control
  unsigned char Num_scheduled_tb[2];                            // Number of scheduled TBs in even/odd TTI
  unsigned short Rx_ack_map[2];                                 // Received ACKs from opposite link in even/odd TTI
  unsigned short Active_process_map_tx[2];                      // Active HARQ process map (TX) in even/odd TTI
  unsigned short New_process_map_tx[2];                      // New HARQ process map (TX) in even/odd TTI
  unsigned char Round_indices_tx[2][NUMBER_HARQ_PROCESS_MAX]; // HARQ Round indices (TX) for active processes in even/odd TTI
  unsigned char Round_indices_rx[2][NUMBER_HARQ_PROCESS_MAX]; // HARQ Round indices (TX) for active processes in even/odd TTI
}LCHAN_INFO_DIL;

#define LCHAN_INFO_DIL_SIZE sizeof(LCHAN_INFO_DIL)

typedef struct LCHAN_INFO_TABLE_ENTRY{
  LCHAN_INFO Lchan_info;
  unsigned char Active;
  //unsigned char Config_status;
  unsigned int Next_sched_limit;
  //  struct LCHAN_INFO_TABLE_ENTRY *Next_entry;
  //struct LCHAN_INFO_TABLE_ENTRY *Prev_entry;
}LCHAN_INFO_TABLE_ENTRY;
#define LCHAN_INFO_TABLE_ENTRY_SIZE sizeof(LCHAN_INFO_TABLE_ENTRY)

typedef struct LCHAN_INFO_DIL_TABLE_ENTRY{
  LCHAN_INFO_DIL Lchan_info_dil;
  unsigned char Active;
  //unsigned char Config_status;
  unsigned int Next_sched_limit;
  //  struct LCHAN_INFO_TABLE_ENTRY *Next_entry;
  //struct LCHAN_INFO_TABLE_ENTRY *Prev_entry;
}LCHAN_INFO_DIL_TABLE_ENTRY;
#define LCHAN_INFO_DIL_TABLE_ENTRY_SIZE sizeof(LCHAN_INFO_DIL_TABLE_ENTRY)


typedef struct {
  LCHAN_ID Lchan_id;
  PHY_RESOURCES Phy_resources;
  unsigned char Nb_tb;
} RX_SCHED;

typedef struct{
  LCHAN_INFO_TABLE_ENTRY *Lchan_entry;
  //  UL_ALLOC_PDU UL_alloc_pdu;
  //  char Activation_tti;
} TX_OPS;


typedef struct{
  unsigned short Node_id;
  unsigned char Num_dlsch;
  unsigned char Num_ulsch;
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
  ULSCH_PDU RX_UL_sach_pdu;
  //DL_MEAS DL_meas;
  RX_SCHED Rx_sched[3][NUMBER_UL_SACH_MAX]; 
  unsigned char Nb_rx_sched[3];
  char Sinr_sorted_table[NB_CNX_CH+1][MAX_NB_SCHED]; 
  char Sinr_sorted_index[NB_CNX_CH+1][MAX_NB_SCHED]; 
  unsigned char Nb_sched;
  //  unsigned char Sched_user[NB_CNX_CH+1];
  //  unsigned short UL_MAP_USE;
  //unsigned short DL_MAP_USE;
}CH_MAC_INST;




typedef struct{
  unsigned short Node_id;
  LCHAN_INFO_TABLE_ENTRY Bcch_lchan[NB_SIG_CNX_UE];
  LCHAN_INFO_TABLE_ENTRY Ccch_lchan[NB_SIG_CNX_UE];
  LCHAN_INFO_TABLE_ENTRY Dcch_lchan[NB_CNX_UE];
  LCHAN_INFO_TABLE_ENTRY Dtch_lchan[NB_RAB_MAX][NB_CNX_UE];
  LCHAN_INFO_TABLE_ENTRY Dtch_dil_lchan[NB_RAB_MAX][NB_SIG_CNX_UE][NB_CNX_CH-1];
  //MEAS_REQ_TABLE Meas_table;
  DEFAULT_UE_MEAS Def_meas[NB_SIG_CNX_UE];
  //  DCI_PDU RXDCI_pdu[NB_CNX_UE];
  DLSCH_PDU RX_DLSCH_pdu[NB_CNX_UE];
  //  UL_MEAS UL_meas[];
  //  DL_MEAS DL_meas[NB_CNX_UE];
  RX_SCHED Rx_sched[NB_CNX_UE][3][NUMBER_UL_SACH_MAX]; 
  unsigned char Nb_rx_sched[NB_CNX_UE][3];
  TX_OPS Tx_ops[NB_CNX_UE][3][NUMBER_UL_SACH_MAX];
  unsigned char Nb_tx_ops[NB_CNX_UE][3];
  unsigned short CH_ul_freq_map[NB_SIG_CNX_UE];
  unsigned char NB_decoded_chbch;
}UE_MAC_INST;











//main.c

void chbch_phy_sync_success(unsigned char Mod_id,unsigned char CH_index);
void mrbch_phy_sync_failure(unsigned char Mod_id,unsigned char Free_ch_index);
int mac_top_init(void);
char layer2_init_mr(unsigned char Mod_id);
char layer2_init_ch(unsigned char Mod_id, unsigned char Free_ch_index);
void mac_switch_node_function(unsigned char Mod_id);
int mac_init_global_param(void);
void mac_top_cleanup(unsigned char Mod_id);
void mac_UE_out_of_sync_ind(unsigned char Mod_id, unsigned short CH_index);

//nodeb_scheduler.c
void nodeb_mac_scheduler_tx(unsigned char Mod_id,unsigned char subframe) ;
void nodeb_mac_scheduler_rx(unsigned char Mod_id) ;

//ue_scheduler.c
void ue_mac_scheduler_tx(unsigned char Mod_id) ;
void ue_mac_scheduler_rx(unsigned char Mod_id) ;

//nodeb_control_plane_procedures.c

/*!\fn void nodeb_generate_dci(unsigned char Mod_id)
\brief This routine first retrieves the BCCH and CCCH logical channels from RRC.  It then fills the UL and DL allocation
maps as well as feedback channels in a DCI_PDU structure.  Finally it generates a MACPHY_DATA_REQ for the 
PHY CHBCH transmitter.
@param Mod_id The MAC instance on which to act.
*/
void nodeb_generate_dci(unsigned char);

/*!\fn void ch_fill_dil_map(unsigned char Mod_id,LCHAN_INFO_DIL_TABLE_ENTRY *Lchan_entry)
\brief This routine fills the DCI_PDU entries corresponding to a particular direct link logical channel
@param Mod_id       The MAC instance on which to act
@param *Lchan_entry Pointer to the logical channel physical channel allocations
*/
void ch_fill_dil_map(unsigned char Mod_id,LCHAN_INFO_DIL_TABLE_ENTRY *Lchan_entry);

/*!\fn void ch_fill_dl_map(unsigned char Mod_id,LCHAN_INFO_TABLE_ENTRY *Lchan_entry)
\brief This routine fills the DCI_PDU entries corresponding to a particular downlink logical channel
@param Mod_id       The MAC instance on which to act
@param *Lchan_entry Pointer to the logical channel physical channel allocations
*/
void ch_fill_dl_map(unsigned char Mod_id,LCHAN_INFO_TABLE_ENTRY *Lchan_entry);

/*!\fn void ch_fill_ul_map(unsigned char Mod_id,LCHAN_INFO_TABLE_ENTRY *Lchan_entry)
\brief This routine fills the DCI_PDU entries corresponding to a particular uplink logical channel.  It operates in TTI \f$N-1\f$
and prepares a DCI_PDU which will be on-air in TTI \f$N\f$.  Furthermore, the UL_MAP is used to schedule the RX resources (UL-SACH)
for TTI \f$N+1\f$.
@param Mod_id       The MAC instance on which to act
@param *Lchan_entry Pointer to the logical channel physical channel allocations
*/
void ch_fill_ul_map(unsigned char Mod_id,LCHAN_INFO_TABLE_ENTRY *Lchan_entry);

/*!\fn void nodeb_scheduler(unsigned char Mod_id)
\brief This routine is the top-level entry point for NodeB physical resource scheduling.  It performs downlink, uplink and direct link
scheduling for the next TTI based on measurement feedback (RF and traffic) from nodes and local measurements. 
@param Mod_id       The MAC instance on which to act
*/
void nodeb_scheduler(unsigned char Mod_id);

/*!\fn void nodeb_decode_ulsch(unsigned char Mod_id,ULSCH_PDU* ulsch_pdu,int *crc_status)
\brief This routine extracts the UL_SCH information.  
@param Mod_id         The MAC instance on which to act
@param *Sch_pdu      Pointer to an ULSCH_PDU structure containing PHY transport blocks
@param *crc_status    Vector containing crc status of each transport block
*/
void nodeb_decode_ulsch(unsigned char Mod_id,ULSCH_PDU* ulsch_pdu,unsigned short rnti);

/*!\fn void nodeb_generate_sch(unsigned char Mod_id)
\brief This routine first retrieves the maps as well as feedback channels in a DCI_PDU structure.  Finally it generates a MACPHY_DATA_REQ for the PHY DLSCH transmitter.
@param Mod_id The MAC instance on which to act.
*/
void nodeb_generate_dlsch(unsigned char Mod_id);

/*!\fn void schedule_dcch(unsigned char Mod_id,unsigned char User,unsigned short *Freq_alloc_map,unsigned char *User_alloc_map,unsigned short rb_map)
\brief This routine is used by the NodeB scheduler to allocate resources for dcch channels.
@param Mod_id The MAC instance on which to act
@param User User index
@param Freq_alloc_map Current Frequency Allocation 
@param *User_alloc_map Allocation of users to frequency groups (rbs)
@param *rb_map Frequency map pattern. Bitmap depending on number of rb to be allocated.  Actual bitmaps are shifted based on CQI information
*/

void nodeb_generate_bcch(u8 Mod_id);

void nodeb_generate_ccch(u8 Mod_id);


void schedule_dcch(unsigned char Mod_id,unsigned char User,unsigned short *Freq_alloc_map,unsigned char *User_alloc_map,unsigned short rb_map);

void fill_rar(unsigned char *dlsch_buffer,
	      unsigned short N_RB_UL,
	      unsigned char input_buffer_length,
	      unsigned short timing_advance_cmd);

unsigned char process_rar(unsigned char *dlsch_buffer);


//ue_control_plane_procedures



void ue_complete_dl_data_req(unsigned char Mod_id);
void ue_get_dil_sach(u8 Mod_id);
void mac_check_rlc_queues_status(unsigned char, unsigned char,unsigned short *);//, UL_SACCH_FB *);
void ue_fill_macphy_data_req(unsigned char ,LCHAN_INFO_TABLE_ENTRY *,unsigned char);
//void ue_decode_sch(unsigned char Mod_id, UL_MEAS *UL_meas,unsigned short Index);
void ue_decode_dlsch(unsigned char,DLSCH_PDU *,unsigned short);
void ue_generate_rach(unsigned char,unsigned char);
void ue_generate_sch(unsigned char);
void ue_scheduler(unsigned char, unsigned char);

void ue_get_chbch(u8 Mod_id, u8 CH_index);

int is_lchan_ul_scheduled(unsigned char Mod_id, unsigned char CH_index, unsigned short Lchan_index);

//lchan_interface.h
int clear_lchan_table(LCHAN_INFO_TABLE_ENTRY *Table, unsigned char Dim);
unsigned short mac_config_req(unsigned char Mod_id,unsigned char Action,MAC_CONFIG_REQ *Req);
unsigned short ch_mac_config_req(unsigned char Mod_id,unsigned char Action,MAC_CONFIG_REQ *Req);
unsigned short ue_mac_config_req(unsigned char Mod_id,unsigned char Action,MAC_CONFIG_REQ *Req);
MAC_MEAS_REQ_ENTRY* mac_meas_req(unsigned char Mod_id,MAC_MEAS_REQ *Meas_req);
MAC_MEAS_REQ_ENTRY* ch_mac_meas_req(unsigned char Mod_id,MAC_MEAS_REQ *Meas_req);
MAC_MEAS_REQ_ENTRY* ue_mac_meas_req(unsigned char Mod_id,MAC_MEAS_REQ *Meas_req);
//void mac_update_meas(unsigned char Mod_id,MAC_MEAS_REQ_ENTRY *Meas_entry, UL_MEAS *UL_meas);
unsigned char mac_check_meas_trigger(MAC_MEAS_REQ *Meas_req);
unsigned char mac_check_meas_ind(MAC_MEAS_REQ_ENTRY *Meas_entry);

//openair2_proc.c
/*!\fn int add_openair2_stats()
\brief This routine initialized the openair2 /proc/openair2 entry.
@returns 0 on success
*/
int add_openair2_stats(void);

/*!\fn int openair2_stats_read(char *buffer, char **my_buffer, off_t off, int length)
\brief This routine initialized the openair2 /proc/openair2 entry.
@param buffer Pointer to string
@param my_buffer 
@param off Offset in buffer
@param length Maximum length of buffer
@returns length of string in bytes
*/
#ifdef USER_MODE
int openair2_stats_read(char *buffer, char **my_buffer, off_t off, int length);
#endif
//utils.h
/*!\fn char *print_cqi(unsigned int cqi)
\brief This routine prints the CQI information.
@param cqi 32-bit CQI value
@returns A pointer to a string containing the CQI information
*/
char *print_cqi(unsigned int cqi);

/*!\fn unsigned char conv_alloc_to_tb2(unsigned char node_type,unsigned char time_alloc,unsigned short freq_alloc,unsigned char target_spec_eff,unsigned char dual_stream_flag,unsigned char nb_tb_max,unsigned char *coding_fmt,unsigned char *num_tb,unsigned short tb_size_bytes)
@param node_type Type of node (0 CH, 1 UE)
@param time_alloc Allocated time map
@param freq_alloc Allocated frequency map
@param target_spec_eff Target spectral efficiency for TB allocation
@param dual_stream_flag Flag to indicate dual-stream transmission
@param num_tb_max Number of TB max available in MAC buffer
@param *coding_fmt Pointer to chosen Modulation/Coding format
@param *num_tb Pointer to chosen number of TBs
@param tb_size_bytes  Size of TB in bytes
*/

#ifdef PHY_EMUL
unsigned char conv_alloc_to_tb2(unsigned char node_type,unsigned char time_alloc,unsigned short freq_alloc,unsigned char target_spec_eff,unsigned char dual_stream_flag,unsigned char num_tb_max,unsigned char *coding_fmt,unsigned char *num_tb,unsigned short tb_size_bytes);

char conv_alloc_to_coding_fmt(unsigned char node_type,
				       unsigned char time_alloc,
				       unsigned short freq_alloc,
				       unsigned char target_spec_eff,
				       unsigned char dual_stream_flag,
			      // unsigned char num_tb_max,
				       unsigned char *coding_fmt,
				       unsigned char *num_tb,
				       unsigned short tb_size_bytes);


unsigned char conv_alloc_to_tb(unsigned char node_type,unsigned char time_alloc,unsigned short freq_alloc,unsigned char coding_fmt,unsigned short tb_size_bytes);
#endif


void emul_phy_sync(unsigned char Mod_id, unsigned char Chbch_index);

void copy_phy_resources(PHY_RESOURCES *To,PHY_RESOURCES *From);

//scheduler
void macphy_scheduler(unsigned char last_slot) ;
void swap_oai(char *Array,char a, char b);
char partition( char *a, char *,char low, char high );
void quicksort( char *a, char *b, char low, char high );
void q_sort(char low, char high );
//int SplitArray(int* array, int *indices, int pivot, int startIndex, int endIndex);
//void quicksort(int* array, int * indices, int startIndex, int endIndex);

//#define msg debug_msg
/*@}*/
#endif /*__LAYER2_MAC_DEFS_H__ */ 



