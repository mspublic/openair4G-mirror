/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2010 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/
/*! \file def.h
* \brief MAC data structures, constant, and function prototype
* \author Raymond Knopp, Navid Nikaein
* \date 2011
* \version 0.5
* @ingroup _mac

*/

 
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
#include "COMMON/platform_types.h"
#include "COMMON/platform_constants.h"
#include "RadioResourceConfigCommon.h"
#include "RadioResourceConfigDedicated.h"
#include "MeasGapConfig.h"
#include "TDD-Config.h"
#include "RACH-ConfigCommon.h"

//#ifdef PHY_EMUL
//#include "SIMULATION/PHY_EMULATION/impl_defs.h"
//#endif

/** @defgroup _mac_impl_ MAC Layer Reference Implementation
 * @ingroup _ref_implementation_
 * @{
 */

#define BCCH_PAYLOAD_SIZE_MAX 128  
#define CCCH_PAYLOAD_SIZE_MAX 128    
#define SCH_PAYLOAD_SIZE_MAX 1024
/// Logical channel ids from 36-311 (Note BCCH is not specified in 36-311, uses the same as first DRB)
#define BCCH 3  // SI 
#define CCCH 0  // srb0
#define DCCH 1  // srb1
#define DCCH1 2 // srb2
#define DTCH  3 // DTCH + lcid < 11

#ifdef USER_MODE
#define printk printf
#endif //USER_MODE

#define MAX_NUM_LCID 11
#define MAX_NUM_RB 8
#define MAX_NUM_CE 5

#define NB_RA_PROC_MAX 4

#define BSR_TABLE_SIZE 64

typedef enum {
  CONNECTION_OK=0,
  CONNECTION_LOST,
  PHY_RESYNCH
} UE_L2_STATE_t;

typedef struct {
  u8 RAPID:6;
  u8 T:1;
  u8 E:1;
} __attribute__((__packed__))RA_HEADER_RAPID;

typedef struct {
  u8 BI:4;
  u8 R:2;
  u8 T:1;
  u8 E:1;
} __attribute__((__packed__))RA_HEADER_BI;

typedef struct {
  u64 t_crnti:16;
  u64 hopping_flag:1;
  u64 rb_alloc:10;
  u64 mcs:4;
  u64 TPC:3;
  u64 UL_delay:1;
  u64 cqi_req:1;
  u64 Timing_Advance_Command:11;  // first/2nd octet LSB
  u64 R:1;                        // octet MSB
  u64 padding:16;
} __attribute__((__packed__))RAR_PDU;
#define sizeof_RAR_PDU 6

typedef struct {
  u8 LCID:5;  // octet 1 LSB
  u8 E:1;
  u8 R:2;     // octet 1 MSB
  u8 L:7;     // octet 2 LSB
  u8 F:1;     // octet 2 MSB
} __attribute__((__packed__))SCH_SUBHEADER_SHORT;

typedef struct {
  u8 LCID:5;   // octet 1 LSB
  u8 E:1;
  u8 R:2;      // octet 1 MSB
  u16 L:15;      // octet 3/2 LSB
  u16 F:1;      // octet 3 MSB     
} __attribute__((__packed__))SCH_SUBHEADER_LONG;
 
typedef struct {
  u8 LCID:5;
  u8 E:1;
  u8 R:2;
} __attribute__((__packed__))SCH_SUBHEADER_FIXED;

typedef struct {
  u8 Buffer_size:6;  // octet 1 LSB
  u8 LCGID:2;        // octet 1 MSB
} __attribute__((__packed__))BSR_SHORT;

typedef BSR_SHORT BSR_TRUNCATED;

typedef struct {
  u32 Buffer_size3:6;
  u32 Buffer_size2:6;
  u32 Buffer_size1:6;
  u32 Buffer_size0:6;
  u32 padding:8;
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
  u16 weight;
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
  u8 DLSCH_DCI[8][(MAX_DCI_SIZE_BITS>>3)+1];

  /// Number of Allocated RBs after scheduling (priod to frequency allocation)
  u16 nb_rb[8];

  /// DCI buffer for ULSCH
  u8 ULSCH_DCI[8][(MAX_DCI_SIZE_BITS>>3)+1];

  /// DL DAI
  u8 DAI;

  /// UL DAI
  u8 DAI_ul;

  /// UL Scheduling Request Received
  u8 ul_SR;

  //Resource Block indication for each sub-band in MU-MIMO 
  u8 rballoc_sub[8][7];


  // Logical channel info for link with RLC
  
  /// UE BSR info for each logical channel
  u8 bsr_info[MAX_NUM_LCID]; 

} UE_TEMPLATE;

typedef struct {
  /// Flag to indicate this process is active
  u8 RA_active;
  /// Size of DCI for RA-Response (bytes)
  u8 RA_dci_size_bytes1;
  /// Size of DCI for RA-Response (bits)
  u8 RA_dci_size_bits1;
  /// Actual DCI to transmit for RA-Response
  u8 RA_alloc_pdu1[(MAX_DCI_SIZE_BITS>>3)+1];
  /// DCI format for RA-Response (should be 1A)
  u8 RA_dci_fmt1;
  /// Size of DCI for Msg4/ContRes (bytes)  
  u8 RA_dci_size_bytes2;
  /// Size of DCI for Msg4/ContRes (bits)  
  u8 RA_dci_size_bits2;
  /// Actual DCI to transmit for Msg4/ContRes
  u8 RA_alloc_pdu2[(MAX_DCI_SIZE_BITS>>3)+1];
  /// DCI format for Msg4/ContRes (should be 1A)
  u8 RA_dci_fmt2;
  /// Flag to indicate the eNB should generate RAR.  This is triggered by detection of PRACH
  u8 generate_rar;
  /// Flag to indicate the eNB should generate Msg4 upon reception of SDU from RRC.  This is triggered by first ULSCH reception at eNB for new user.
  u8 generate_Msg4;
  /// Flag to indicate the eNB should generate the DCI for Msg4, after getting the SDU from RRC.
  u8 generate_Msg4_dci;
  /// Flag to indicate that eNB is waiting for ACK that UE has received Msg3.
  u8 wait_ack_Msg4;
  /// UE RNTI allocated during RAR
  u16 rnti;
  /// RA RNTI allocated from received PRACH
  u16 RA_rnti;
  /// Received preamble_index
  u8 preamble_index;
  /// Received UE Contention Resolution Identifier 
  u8 cont_res_id[6];
  /// Timing offset indicated by PHY
  s16 timing_offset;
  /// Timeout for RRC connection 
  s16 RRC_timer;
} RA_TEMPLATE;


///subband bitmap coniguration (for ALU icic algo purpose), in test phase

typedef struct{
	u8 sbmap[NUMBER_OF_SUBBANDS]; //13 = number of SB MAX for 100 PRB
	u8 periodicity;
	u8 first_subframe;
	u8 sb_size;
	u8 nb_active_sb;

}SBMAP_CONF;

//end ALU's algo

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
  /// DCI template and MAC connection for RA processes
  RA_TEMPLATE RA_template[NB_RA_PROC_MAX];
  /// BCCH active flag
  u8 bcch_active;
  ///subband bitmap configuration
  SBMAP_CONF sbmap_conf;
}eNB_MAC_INST;

typedef struct {
  /// buffer status for each lcid
  u8  BSR[MAX_NUM_LCID]; // should be more for mesh topology
  /// keep the number of bytes in rlc buffer for each lcid
  u16  BSR_bytes[MAX_NUM_LCID];
  /// short bsr lcid
  u8  BSR_short_lcid;
  /// SR pending as defined in 36.321
  u8  SR_pending;
  /// SR_COUNTER as defined in 36.321
  u16 SR_COUNTER;
  /// retxBSR-Timer, default value is sf2560
  u16 retxBSR_Timer;
  /// retxBSR_SF, number of subframe before triggering a regular BSR 
  s16 retxBSR_SF;
  /// periodicBSR-Timer, default to infinity
  u16 periodicBSR_Timer;
  /// periodicBSR_SF, number of subframe before triggering a periodic BSR 
  s16 periodicBSR_SF;  
  /// default value is 0: not configured
  u16 sr_ProhibitTimer;
  /// sr ProhibitTime running
  u8 sr_ProhibitTimer_Running;
  ///  default value to n5
  u16 maxHARQ_Tx; 
  /// default value is false
  u16 ttiBundling;
  /// default value is release 
  struct DRX_Config *drx_config;
  /// default value is release
  struct MAC_MainConfig__phr_Config *phr_config;
  //Bj bucket usage per  lcid
  s16 Bj[MAX_NUM_LCID];
  // Bucket size per lcid
  s16 bucket_size[MAX_NUM_LCID];
} UE_SCHEDULING_INFO;

typedef struct{
  u16 Node_id;
  /// pointer to RRC PHY configuration 
  RadioResourceConfigCommonSIB_t *radioResourceConfigCommon;
  /// pointer to RRC PHY configuration 
  struct PhysicalConfigDedicated *physicalConfigDedicated;
  /// pointer to TDD Configuration (NULL for FDD)
  TDD_Config_t *tdd_Config;
  /// Pointer to RRC MAC configuration
  MAC_MainConfig_t *macConfig;
  /// Pointer to RRC Measurement gap configuration
  MeasGapConfig_t  *measGapConfig;
  /// Pointers to LogicalChannelConfig indexed by LogicalChannelIdentity. Note NULL means LCHAN is inactive.
  LogicalChannelConfig_t *logicalChannelConfig[MAX_NUM_LCID];
  /// Scheduling Information 
  UE_SCHEDULING_INFO scheduling_info;
  /// Outgoing CCCH pdu for PHY
  CCCH_PDU CCCH_pdu;
  /// Incoming DLSCH pdu for PHY
  DLSCH_PDU DLSCH_pdu[NB_CNX_UE][2];
  //ULSCH_PDU ULSCH_pdu[NB_CNX_UE][2];
  /// Random-access procedure flag
  u8 RA_active;
  /// Random-access window counter
  s8 RA_window_cnt;
  /// Random-access Msg3 size in bytes
  u8 RA_Msg3_size;
  /// Random-access prachMaskIndex
  u8 RA_prachMaskIndex;
  /// Flag indicating Preamble set (A,B) used for first Msg3 transmission
  u8 RA_usedGroupA;
  /// Random-access Resources
  PRACH_RESOURCES_t RA_prach_resources;
  /// Random-access PREAMBLE_TRANSMISSION_COUNTER
  u8 RA_PREAMBLE_TRANSMISSION_COUNTER;
  /// Random-access backoff counter
  s16 RA_backoff_cnt;
  /// Random-access variable for window calculation (frame of last change in window counter)
  u32 RA_tx_frame;
  /// Random-access variable for window calculation (subframe of last change in window counter)
  u8 RA_tx_subframe;
  /// Random-access Group B maximum path-loss
  /// Random-access variable for backoff (frame of last change in backoff counter)
  u32 RA_backoff_frame;
  /// Random-access variable for backoff (subframe of last change in backoff counter)
  u8 RA_backoff_subframe;
  /// Random-access Group B maximum path-loss
  u16 RA_maxPL;
  /// Random-access Contention Resolution Timer active flag
  u8 RA_contention_resolution_timer_active;
  /// Random-access Contention Resolution Timer count value
  u8 RA_contention_resolution_cnt;
}UE_MAC_INST;





/** \brief RRC Configuration primitive for PHY/MAC.  Allows configuration of PHY/MAC resources based on System Information (SI), RRCConnectionSetup and RRCConnectionReconfiguration messages.
@param Mod_id Instance ID of eNB
@param CH_flag Indicates if this is a eNB or UE configuration
@param UE_id Index of UE if this is an eNB configuration
@param eNB_id Index of eNB if this is a UE configuration
@param radioResourceConfigCommon Structure from SIB2 for common radio parameters (if NULL keep existing configuration)
@param physcialConfigDedicated Structure from RRCConnectionSetup or RRCConnectionReconfiguration for dedicated PHY parameters (if NULL keep existing configuration)
@param mac_MainConfig Structure from RRCConnectionSetup or RRCConnectionReconfiguration for dedicated MAC parameters (if NULL keep existing configuration)
@param logicalChannelIdentity Logical channel identity index of corresponding logical channel config 
@param logicalChannelConfig Pointer to logical channel configuration
@param measGapConfig Measurement Gap configuration for MAC (if NULL keep existing configuration)
@param tdd_Config TDD Configuration from SIB1 (if NULL keep existing configuration)
@param SIwindowsize SI Windowsize from SIB1 (if NULL keep existing configuration)
@param SIperiod SI Period from SIB1 (if NULL keep existing configuration)
*/
int rrc_mac_config_req(u8 Mod_id,u8 CH_flag,u8 UE_id,u8 eNB_id, 
		       RadioResourceConfigCommonSIB_t *radioResourceConfigCommon,
		       struct PhysicalConfigDedicated *physicalConfigDedicated,
		       MAC_MainConfig_t *mac_MainConfig,
		       long logicalChannelIdentity,
		       LogicalChannelConfig_t *logicalChannelConfig,
		       MeasGapConfig_t *measGapConfig,
		       TDD_Config_t *tdd_Config,
		       u8 *SIwindowsize,
		       u16 *SIperiod);

/** \brief First stage of Random-Access Scheduling. Loops over the RA_templates and checks if RAR, Msg3 or its retransmission are to be scheduled in the subframe.  It returns the total number of PRB used for RA SDUs.  For Msg3 it retrieves the L3msg from RRC and fills the appropriate buffers.  For the others it just computes the number of PRBs. Each DCI uses 3 PRBs (format 1A) 
for the message.
@param Mod_id Instance ID of eNB
@param frame Frame index
@param subframe Subframe number on which to act
@param nprb Pointer to current PRB count
@param nCCE Pointer to current nCCE count
*/
void schedule_RA(u8 Mod_id,u32 frame,u8 subframe,u8 *nprb,u8 *nCCE);

/** \brief First stage of SI Scheduling. Gets a SI SDU from RRC if available and computes the MCS required to transport it as a function of the SDU length.  It assumes a length less than or equal to 64 bytes (MCS 6, 3 PRBs).
@param Mod_id Instance ID of eNB
@param frame Frame index
@param subframe Subframe number on which to act
@param nprb Pointer to current PRB count
@param nCCE Pointer to current nCCE count
*/
void schedule_SI(u8 Mod_id,u32 frame,u8 *nprb,u8 *nCCE);

/** \brief ULSCH Scheduling.
@param Mod_id Instance ID of eNB
@param frame Frame index
@param subframe Subframe number on which to act
@param nCCE Pointer to current nCCE count
*/
void schedule_ulsch(u8 Mod_id,u32 frame,u8 cooperation_flag, u8 subframe,u8 *nCCE);

/** \brief Second stage of DLSCH scheduling, after schedule_SI, schedule_RA and schedule_dlsch have been called.  This routine first allocates random frequency assignments for SI and RA SDUs using distributed VRB allocations and adds the corresponding DCI SDU to the DCI buffer for PHY.  It then loops over the UE specific DCIs previously allocated and fills in the remaining DCI fields related to frequency allocation.  It assumes localized allocation of type 0 (DCI.rah=0).  The allocation is done for tranmission modes 1,2,4. 
@param Mod_id Instance of eNB
@param frame Frame index
@param subframe Index of subframe
@param rballoc Bitmask for allowable subband allocations
@param RA_scheduled RA was scheduled in this subframe
*/
void fill_DLSCH_dci(u8 Mod_id,u32 frame,u8 subframe,u32 rballoc,u8 RA_scheduled);

/** \brief UE specific DLSCH scheduling. Retrieves next ue to be schduled from round-robin scheduler and gets the appropriate harq_pid for the subframe from PHY. If the process is active and requires a retransmission, it schedules the retransmission with the same PRB count and MCS as the first transmission. Otherwise it consults RLC for DCCH/DTCH SDUs (status with maximum number of available PRBS), builds the MAC header (timing advance sent by default) and copies 
@param Mod_id Instance ID of eNB
@param frame Frame index
@param subframe Subframe on which to act
@param nb_rb_used0 Number of PRB used by SI/RA
@param nCCE_used Number of CCE used by SI/RA
*/
void schedule_ue_spec(u8 Mod_id,u32 frame,u8 subframe,u16 nb_rb_used0,u8 nCCE_used);

/** \brief Function for UE/PHY to compute PUSCH transmit power in power-control procedure.
    @param Mod_id Module id of UE
    @returns Po_NOMINAL_PUSCH (PREAMBLE_RECEIVED_TARGET_POWER+DELTA_PREAMBLE
*/
s8 get_Po_NOMINAL_PUSCH(u8 Mod_id);

/** \brief Function to compute DELTA_PREAMBLE from 36.321 (for RA power ramping procedure and Msg3 PUSCH power control policy) 
    @param Mod_id Module id of UE
    @returns DELTA_PREAMBLE
*/
s8 get_DELTA_PREAMBLE(u8 Mod_id);

/** \brief Function for compute deltaP_rampup from 36.321 (for RA power ramping procedure and Msg3 PUSCH power control policy) 
    @param Mod_id Module id of UE
    @returns deltaP_rampup
*/
s8 get_deltaP_rampup(u8 Mod_id);

//main.c

void chbch_phy_sync_success(u8 Mod_id,u32 frame,u8 CH_index);

void mrbch_phy_sync_failure(u8 Mod_id, u32 frame,u8 Free_ch_index);

int mac_top_init(void);

char layer2_init_UE(u8 Mod_id);

char layer2_init_eNB(u8 Mod_id, u8 Free_ch_index); 

void mac_switch_node_function(u8 Mod_id);

int mac_init_global_param(void);

void mac_top_cleanup(void);

void mac_UE_out_of_sync_ind(u8 Mod_id,u32 frame, u16 CH_index);


// eNB functions
/* \brief Function to trigger the eNB scheduling procedure.  It is called by PHY at the beginning of each subframe, \f$n$\f 
   and generates all DLSCH allocations for subframe \f$n\f$ and ULSCH allocations for subframe \f$n+k$\f. The resultant DCI_PDU is
   ready after returning from this call.
@param Mod_id Instance ID of eNB
@param cooperation_flag Flag to indicated that this cell has cooperating nodes (i.e. that there are collaborative transport channels that
can be scheduled.
@param subframe Index of current subframe
*/
void eNB_dlsch_ulsch_scheduler(u8 Mod_id, u8 cooperation_flag, u32 frame, u8 subframe); 

/* \brief Function to retrieve result of scheduling (DCI) in current subframe.  Can be called an arbitrary numeber of times after eNB_dlsch_ulsch_scheduler
in a given subframe.
@param Mod_id Instance ID of eNB
@param subframe Index of current subframe
@returns Pointer to generated DCI for subframe
*/
DCI_PDU *get_dci_sdu(u8 Mod_id,u32 frame,u8 subframe);

/* \brief Function to indicate a received preamble on PRACH.  It initiates the RA procedure.
@param Mod_id Instance ID of eNB
@param preamble_index index of the received RA request
@param timing_offset Offset in samples of the received PRACH w.r.t. eNB timing. This is used to 
*/
void initiate_ra_proc(u8 Mod_id,u32 frame, u16 preamble_index,s16 timing_offset,u8 sect_id,u8 subframe,u8 f_id);


/* \brief Function in eNB to fill RAR pdu when requested by PHY.  This provides a single RAR SDU for the moment and returns the t-CRNTI.
@param Mod_id Instance ID of eNB
@param dlsch_buffer Pointer to DLSCH input buffer
@param N_RB_UL Number of UL resource blocks
@returns t_CRNTI
*/
u16  fill_rar(u8 Mod_id,u32 frame,
	      u8 *dlsch_buffer,
	      u16 N_RB_UL);

/* \brief This function indicates the end of RA procedure and provides the l3msg received on ULSCH.
@param Mod_id Instance ID of eNB
@param rnti RNTI of UE transmitting l3msg
@param l3msg Pointer to received l3msg
*/
void terminate_ra_proc(u8 Mod_id,u32 frame,u16 rnti, u8 *l3msg);

/* \brief Function to indicate a failed RA response.  It removes all temporary variables related to the initial connection of a UE
@param Mod_id Instance ID of eNB
@param preamble_index index of the received RA request.
*/
void cancel_ra_proc(u8 Mod_id,u32 frame, u16 preamble_index);

/* \brief Function to indicate a received SDU on ULSCH.
@param Mod_id Instance ID of eNB
@param rnti RNTI of UE transmitting the SR
@param sdu Pointer to received SDU
*/
void rx_sdu(u8 Mod_id,u32 frame,u16 rnti, u8 *sdu);

/* \brief Function to indicate a scheduled schduling request (SR) was received by eNB.
@param Mod_id Instance ID of eNB
@param rnti RNTI of UE transmitting the SR
@param subframe Index of subframe where SR was received
*/
void SR_indication(u8 Mod_id,u32 frame,u16 rnti, u8 subframe);

u8 *get_dlsch_sdu(u8 Mod_id,u32 frame,u16 rnti,u8 TBindex);
//added for ALU icic purpose
u32 Get_Cell_SBMap(u8 Mod_id);
void UpdateSBnumber(unsigned char Mod_id);
//end ALU's algo




void init_ue_sched_info(void);
void add_ue_ulsch_info(u8 Mod_id, u8 UE_id, u8 subframe,UE_ULSCH_STATUS status);
void add_ue_dlsch_info(u8 Mod_id, u8 UE_id, u8 subframe,UE_DLSCH_STATUS status);
s8 find_UE_id(u8 Mod_id,u16 rnti) ;
s16 find_UE_RNTI(u8 Mod_id, u8 UE_id);
s8 find_active_UEs(u8 Mod_id);
u8 is_UE_active(unsigned char Mod_id, unsigned char UE_id );
u16 find_ulgranted_UEs(u8 Mod_id);
u16 find_dlgranted_UEs(u8 Mod_id);
u8 process_ue_cqi (u8 Mod_id, u8 UE_id);

/** \brief Round-robin scheduler for ULSCH traffic.
@param Mod_id Instance ID for eNB
@param subframe Subframe number on which to act
@returns UE index that is to be scheduled if needed/room
*/
u8 schedule_next_ulue(u8 Mod_id, u8 UE_id,u8 subframe);

/** \brief Round-robin scheduler for DLSCH traffic.
@param Mod_id Instance ID for eNB
@param subframe Subframe number on which to act
@returns UE index that is to be scheduled if needed/room
*/
u8 schedule_next_dlue(u8 Mod_id, u8 subframe);

/* \brief Allocates a set of PRBS for a particular UE.  This is a simple function for the moment, later it should process frequency-domain CQI information and/or PMI information.  Currently it just returns the first PRBS that are available in the subframe based on the number requested.
@param UE_id Index of UE on which to act
@param nb_rb Number of PRBs allocated to UE by scheduler
@param rballoc Pointer to bit-map of current PRB allocation given to previous users/control channels.  This is updated for subsequent calls to the routine.
@returns an rballoc bitmap for resource type 0 allocation (DCI).
*/
u32 allocate_prbs(u8 UE_id,u8 nb_rb, u32 *rballoc);

/* \fn u32 req_new_ulsch(u8 Mod_id)
\brief check for a new transmission in any drb 
@param Mod_id Instance id of UE in machine
@returns 1 for new transmission, 0 for none
*/
u32 req_new_ulsch(u8 Mod_id);

/* \brief Get SR payload (0,1) from UE MAC
@param Mod_id Instance id of UE in machine
@param eNB_id Index of eNB that UE is attached to
@param rnti C_RNTI of UE
@param subframe subframe number
@returns 0 for no SR, 1 for SR
*/
u32 ue_get_SR(u8 Mod_id, u32 frame, u8 eNB_id,u16 rnti,u8 subframe);

u8 get_ue_weight(u8 Mod_id, u8 UE_id);

// UE functions
void out_of_sync_ind(u8 Mod_id, u32 frame, u16);

void ue_decode_si(u8 Mod_id, u32 frame, u8 CH_index, void *pdu, u16 len);


void ue_send_sdu(u8 Mod_id, u32 frame, u8 *sdu,u16 sdu_len,u8 CH_index);

void ue_get_sdu(u8 Mod_id, u32 frame, u8 CH_index,u8 *ulsch_buffer,u16 buflen);

/* \brief Function called by PHY to retrieve information to be transmitted using the RA procedure.  If the UE is not in PUSCH mode for a particular eNB index, this is assumed to be an Msg3 and MAC attempts to retrieves the CCCH message from RRC. If the UE is in PUSCH mode for a particular eNB index and PUCCH format 0 (Scheduling Request) is not activated, the MAC may use this resource for random-access to transmit a BSR along with the C-RNTI control element (see 5.1.4 from 36.321)
@param Mod_id Index of UE instance
@param New_Msg3 Flag to indicate this call is for a new Msg3
@param subframe Index of subframe for PRACH transmission (0 ... 9)
@returns A pointer to a PRACH_RESOURCES_t */
PRACH_RESOURCES_t *ue_get_rach(u8 Mod_id,u32 frame,u8 new_Msg3,u8 subframe);

/* \brief Function called by PHY to process the received RAR.  It checks that the preamble matches what was sent by the eNB and provides the timing advance and t-CRNTI.
@param Mod_id Index of UE instance
@param dlsch_buffer  Pointer to dlsch_buffer containing RAR PDU
@param t_crnti Pointer to PHY variable containing the T_CRNTI
@param preamble_index Preamble Index used by PHY to transmit the PRACH.  This should match the received RAR to trigger the rest of 
random-access procedure
@returns timing advance or 0xffff if preamble doesn't match
*/
u16 ue_process_rar(u8 Mod_id,u32 frame,u8 *dlsch_buffer,u16 *t_crnti,u8 preamble_index);


/* \brief Generate header for UL-SCH.  This function parses the desired control elements and sdus and generates the header as described
in 36-321 MAC layer specifications.  It returns the number of bytes used for the header to be used as an offset for the payload 
in the ULSCH buffer.
@param mac_header Pointer to the first byte of the MAC header (UL-SCH buffer)
@param num_sdus Number of SDUs in the payload
@param short_padding Number of bytes for short padding (0,1,2)
@param sdu_lengths Pointer to array of SDU lengths
@param sdu_lcids Pointer to array of LCIDs (the order must be the same as the SDU length array)
@param power_headroom Pointer to power headroom command (NULL means not present in payload)
@param crnti Pointer to CRNTI command (NULL means not present in payload)
@param truncated_bsr Pointer to Truncated BSR command (NULL means not present in payload)
@param short_bsr Pointer to Short BSR command (NULL means not present in payload)
@param long_bsr Pointer to Long BSR command (NULL means not present in payload)
@returns Number of bytes used for header
*/
unsigned char generate_ulsch_header(u8 *mac_header,
				    u8 num_sdus,
				    u8 short_padding,
				    u16 *sdu_lengths,
				    u8 *sdu_lcids,
				    POWER_HEADROOM_CMD *power_headroom,
				    u16 *crnti,
				    BSR_SHORT *truncated_bsr,
				    BSR_SHORT *short_bsr,
				    BSR_LONG *long_bsr);

/* \brief Parse header for UL-SCH.  This function parses the received UL-SCH header as described
in 36-321 MAC layer specifications.  It returns the number of bytes used for the header to be used as an offset for the payload 
in the ULSCH buffer.
@param mac_header Pointer to the first byte of the MAC header (UL-SCH buffer)
@param num_ces Number of SDUs in the payload
@param num_sdu Number of SDUs in the payload
@param rx_ces Pointer to received CEs in the header
@param rx_lcids Pointer to array of LCIDs (the order must be the same as the SDU length array)
@param rx_lengths Pointer to array of SDU lengths
@returns Pointer to payload following header
*/
u8 *parse_ulsch_header(u8 *mac_header,
		       u8 *num_ce,
		       u8 *num_sdu,
		       u8 *rx_ces,
		       u8 *rx_lcids,
		       u16 *rx_lengths);


int l2_init(LTE_DL_FRAME_PARMS *frame_parms);
int mac_init(void);
void ue_init_mac(void);
s8 add_new_ue(u8 Mod_id, u16 rnti);
s8 mac_remove_ue(u8 Mod_id, u8 UE_id);

/*! \fn  UE_L2_state_t ue_scheduler(u8 Mod_id,u32 frame, u8 subframe, lte_subframe_t direction,u8 eNB_index)
   \brief UE scehdular where all the ue background tasks are done
\param[in] Mod_id instance of the UE
\param[in] subframe the subframe number
\param[in] direction subframe direction
\param[in] eNB_index instance of eNB
@returns L2 state (CONNETION_OK or CONNECTION_LOST or PHY_RESYNCH)
*/
UE_L2_STATE_t ue_scheduler(u8 Mod_id,u32 frame, u8 subframe, lte_subframe_t direction,u8 eNB_index);


/*! \fn  u8 get_bsr_len (u8 Mod_id, u16 bufflen);
\brief determine whether the bsr is short or long assuming that the MAC pdu is built 
\param[in] Mod_id instance of the UE
\param[in] bufflen size of phy transport block
\param[out] bsr_len size of bsr control element 
*/
u8 get_bsr_len (u8 Mod_id, u16 buflen);

/*! \fn  BSR_SHORT *  get_bsr_short(u8 Mod_id, u8 bsr_len)
\brief get short bsr level
\param[in] Mod_id instance of the UE
\param[in] bsr_len indicator for no, short, or long bsr
\param[out] bsr_s pointer to short bsr
*/
BSR_SHORT *get_bsr_short(u8 Mod_id, u8 bsr_len);

/*! \fn  BSR_LONG * get_bsr_long(u8 Mod_id, u8 bsr_len)
\brief get long bsr level
\param[in] Mod_id instance of the UE
\param[in] bsr_len indicator for no, short, or long bsr
\param[out] bsr_l pointer to long bsr
*/
BSR_LONG * get_bsr_long(u8 Mod_id, u8 bsr_len);

/*! \fn  void update_bsr(u8 Mod_id, u32 frame, u8 lcid)
   \brief get the rlc stats and update the bsr level for each lcid 
\param[in] Mod_id instance of the UE
\param[in] frame Frame index
\param[in] lcid logical channel identifier
*/
void update_bsr(u8 Mod_id, u32 frame, u8 lcid);

/*! \fn  locate (int *table, int size, int value)
   \brief locate the BSR level in the table as defined in 36.321. This function requires that he values in table to be monotonic, either increasing or decreasing. The returned value is not less than 0, nor greater than n-1, where n is the size of table. 
\param[in] *table Pointer to BSR table
\param[in] size Size of the table
\param[in] value Value of the buffer 
\return the index in the BSR_LEVEL table
*/
u8 locate (const u32 *table, int size, int value);


/*! \fn  int get_sf_periodicBSRTimer(u8 periodicBSR_Timer)
   \brief get the number of subframe from the periodic BSR timer configured by the higher layers
\param[in] periodicBSR_Timer timer for periodic BSR
\return the number of subframe
*/
int get_sf_periodicBSRTimer(u8 bucketSize);

/*! \fn  int get_ms_bucketsizeduration(u8 bucketSize)
   \brief get the time in ms form the bucket size duration configured by the higher layer
\param[in]  bucketSize the bucket size duration
\return the time in ms
*/
int get_ms_bucketsizeduration(u8 bucketsizeduration);

/*! \fn  int get_sf_retxBSRTimer(u8 retxBSR_Timer)
   \brief get the number of subframe form the bucket size duration configured by the higher layer
\param[in]  retxBSR_Timer timer for regular BSR
\return the time in ms
*/
int get_sf_retxBSRTimer(u8 retxBSR_Timer);

/*! \brief Function to indicate Msg3 transmission/retransmission which initiates/reset Contention Resolution Timer
\param[in] Mod_id Instance index of UE
\param[in] eNB_id Index of eNB
*/
void Msg3_tx(u8 Mod_id,u32 frame,u8 eNB_id);

void dl_phy_sync_success(unsigned char Mod_id,
			 u32 frame,
			 unsigned char eNB_index,
			 u8 first_sync);

/*@}*/
#endif /*__LAYER2_MAC_DEFS_H__ */ 



