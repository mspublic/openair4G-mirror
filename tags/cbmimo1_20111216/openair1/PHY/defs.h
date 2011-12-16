#ifndef __PHY_DEFS__H__
#define __PHY_DEFS__H__

#ifdef USER_MODE
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#define msg printf   
//use msg in the real-time thread context
#define msg_nrt printf   
//use msg_nrt in the non real-time context (for initialization, ...)
#ifdef EXPRESSMIMO_TARGET
#define malloc16(x) malloc(x)
#else //EXPRESSMIMO_TARGET
#define malloc16(x) memalign(16,x)
#endif //EXPRESSMIMO_TARGET
#define free16(y,x) free(y)
#define bigmalloc malloc
#define bigmalloc16 malloc16
#define openair_free(y,x) free((y))
#define PAGE_SIZE 4096

#define PAGE_MASK 0xfffff000
#define virt_to_phys(x) (x)

#define openair_sched_exit() exit(-1)


#define max(a,b)  ((a)>(b) ? (a) : (b))
#define min(a,b)  ((a)<(b) ? (a) : (b))


#else // USER_MODE

#include "ARCH/COMMON/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"

#define msg fifo_printf
//#define msg(x...) rt_printk(KERN_ALERT x)

#define msg_nrt printk(KERN_ALERT x)

#ifdef BIGPHYSAREA

#define bigmalloc(x) (bigphys_malloc(x))
#define bigmalloc16(x) (bigphys_malloc(x))

#define malloc16(x) (bigphys_malloc(x))
#define free16(y,x) 

#define bigfree(y,x) 

#else // BIGPHYSAREA
 
#define bigmalloc(x) (dma_alloc_coherent(pdev[0],(x),&dummy_dma_ptr,0))
#define bigmalloc16(x) (dma_alloc_coherent(pdev[0],(x),&dummy_dma_ptr,0))
#define bigfree(y,x) (dma_free_coherent(pdev[0],(x),(void *)(y),dummy_dma_ptr))
#define malloc16(x) (kmalloc(x,GFP_KERNEL))
#define free16(y,x) (kfree(y))

#endif // BIGPHYSAREA


#ifdef CBMIMO1
#define openair_get_mbox() (*(unsigned int *)mbox)
#else //CBMIMO1
#define openair_get_mbox() (*(unsigned int *)PHY_vars->mbox>>1)
#endif //CBMIMO1

#endif // USERMODE

#define bzero(s,n) (memset((s),0,(n)))

#define cmax(a,b)  ((a>b) ? (a) : (b))
#define cmin(a,b)  ((a<b) ? (a) : (b))


#ifdef EXPRESSMIMO_TARGET
#define Zero_Buffer(x,y) Zero_Buffer_nommx(x,y)
#endif //EXPRESSMiMO_TARGET
 

#include "spec_defs_top.h"
#include "impl_defs_top.h"
#include "impl_defs_lte.h"

#include "PHY/CODING/defs.h"
#include "PHY/TOOLS/defs.h"


//#include "PHY/LTE_ESTIMATION/defs.h"

#include "PHY/LTE_TRANSPORT/defs.h"

#define NUM_DCI_MAX 32



/// Top-level PHY Data Structure for eNB 
typedef struct
{
  /// Module ID indicator for this instance
  u8 Mod_id;
  u8 local_flag;
  unsigned int rx_total_gain_eNB_dB;
  LTE_DL_FRAME_PARMS  lte_frame_parms;
  PHY_MEASUREMENTS_eNB PHY_measurements_eNB[NUMBER_OF_eNB_MAX]; /// Measurement variables 
  LTE_eNB_COMMON   lte_eNB_common_vars;
  LTE_eNB_SRS      lte_eNB_srs_vars[NUMBER_OF_UE_MAX];
  LTE_eNB_PBCH     lte_eNB_pbch;
  LTE_eNB_ULSCH    *lte_eNB_ulsch_vars[NUMBER_OF_UE_MAX];
  LTE_eNB_DLSCH_t  *dlsch_eNB[NUMBER_OF_UE_MAX][2];   // Nusers times two spatial streams
  // old: LTE_eNB_DLSCH_t  **dlsch_eNB[2];   // Nusers times two spatial streams
  LTE_eNB_ULSCH_t  *ulsch_eNB[NUMBER_OF_UE_MAX+1];      // Nusers + number of RA
  LTE_eNB_DLSCH_t  *dlsch_eNB_SI,*dlsch_eNB_ra;
  LTE_eNB_UE_stats eNB_UE_stats[NUMBER_OF_UE_MAX];
  LTE_eNB_UE_stats *eNB_UE_stats_ptr[NUMBER_OF_UE_MAX];

  /// cell-specific reference symbols
  unsigned int lte_gold_table[20][2][14];

  u8 pbch_pdu[4]; //PBCH_PDU_SIZE
  char eNB_generate_rar;

  unsigned int max_peak_val; 
  int max_eNB_id, max_sync_pos;


  unsigned char first_run_timing_advance[NUMBER_OF_UE_MAX];
  unsigned char first_run_I0_measurements;

  unsigned char cooperation_flag; // for cooperative communication

  unsigned char    is_secondary_eNB; // primary by default
  unsigned char    is_init_sync;     /// Flag to tell if initial synchronization is performed. This affects how often the secondary eNB will listen to the PSS from the primary system.
  unsigned char    has_valid_precoder; /// Flag to tell if secondary eNB has channel estimates to create NULL-beams from, and this B/F vector is created.
  unsigned char    PeNB_id;          /// id of Primary eNB
  int              rx_offset;        /// Timing offset (used if is_secondary_eNB)

  /// hold the precoder for NULL beam to the primary user
  int              **dl_precoder_SeNB[3];
  char             log2_maxp; /// holds the maximum channel/precoder coefficient

  /// For emulation only (used by UE abstraction to retrieve DCI)
  u8 num_common_dci[2];                         // num_dci in even/odd subframes
  u8 num_ue_spec_dci[2];                         // num_dci in even/odd subframes
  DCI_ALLOC_t dci_alloc[2][NUM_DCI_MAX]; // dci_alloc from even/odd subframes


  // PDSCH Varaibles
  PDSCH_CONFIG_DEDICATED pdsch_config_dedicated[NUMBER_OF_UE_MAX];

  // PUSCH Varaibles
  PUSCH_CONFIG_DEDICATED pusch_config_dedicated[NUMBER_OF_UE_MAX];

  // PUCCH variables
  PUCCH_CONFIG_DEDICATED pucch_config_dedicated[NUMBER_OF_UE_MAX];

  // UL-POWER-Control
  UL_POWER_CONTROL_DEDICATED ul_power_control_dedicated[NUMBER_OF_UE_MAX];

  // TPC
  TPC_PDCCH_CONFIG tpc_pdcch_config_pucch[NUMBER_OF_UE_MAX];
  TPC_PDCCH_CONFIG tpc_pdcch_config_pusch[NUMBER_OF_UE_MAX];

  // CQI reporting
  CQI_REPORT_CONFIG cqi_report_config[NUMBER_OF_UE_MAX];

  // SRS Variables
  SOUNDINGRS_UL_CONFIG_DEDICATED soundingrs_ul_config_dedicated[NUMBER_OF_UE_MAX];
  u8 ncs_cell[20][7];

  // Scheduling Request Config
  SCHEDULING_REQUEST_CONFIG scheduling_request_config[NUMBER_OF_UE_MAX];

  // Transmission mode per UE
  u8 transmission_mode[NUMBER_OF_UE_MAX];


  /// Information regarding TM5
  MU_MIMO_mode mu_mimo_mode[NUMBER_OF_UE_MAX];


  ///check for Total Transmissions
  u8 check_for_total_transmissions;

  ///check for MU-MIMO Transmissions
  u8 check_for_MUMIMO_transmissions;

  ///check for SU-MIMO Transmissions
  u8 check_for_SUMIMO_transmissions;

} PHY_VARS_eNB;

#define debug_msg if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 50)) msg
//#define debug_msg msg

/// Top-level PHY Data Structure for UE 
typedef struct
{
  /// Module ID indicator for this instance
  u8 Mod_id;
  u8 local_flag;
  unsigned int tx_total_gain_dB;
  unsigned int rx_total_gain_dB;
  PHY_MEASUREMENTS PHY_measurements; /// Measurement variables 
  LTE_DL_FRAME_PARMS  lte_frame_parms;
  LTE_UE_COMMON    lte_ue_common_vars;
  LTE_UE_DLSCH     *lte_ue_dlsch_vars[NUMBER_OF_eNB_MAX+1];
  LTE_UE_DLSCH     *lte_ue_dlsch_vars_SI[NUMBER_OF_eNB_MAX];
  LTE_UE_DLSCH     *lte_ue_dlsch_vars_ra[NUMBER_OF_eNB_MAX];
  LTE_UE_PBCH      *lte_ue_pbch_vars[NUMBER_OF_eNB_MAX];
  LTE_UE_PDCCH     *lte_ue_pdcch_vars[NUMBER_OF_eNB_MAX];
  //  LTE_UE_PRACH_t   *lte_ue_prach_vars[NUMBER_OF_eNB_MAX];
  LTE_UE_DLSCH_t   *dlsch_ue[NUMBER_OF_eNB_MAX][2];
  LTE_UE_ULSCH_t   *ulsch_ue[NUMBER_OF_eNB_MAX];
  LTE_UE_DLSCH_t   *dlsch_ue_SI[NUMBER_OF_eNB_MAX],*dlsch_ue_ra[NUMBER_OF_eNB_MAX];
  u8               sr;
  u8               pucch_payload[22];

  UE_MODE_t        UE_mode[NUMBER_OF_eNB_MAX];

  /// cell-specific reference symbols
  unsigned int lte_gold_table[3][20][2][14];


  char ulsch_no_allocation_counter[NUMBER_OF_eNB_MAX];

  unsigned char ulsch_ue_RRCConnReq_active[NUMBER_OF_eNB_MAX];
  unsigned int  ulsch_ue_RRCConnReq_frame[NUMBER_OF_eNB_MAX];
  unsigned char ulsch_ue_RRCConnReq_subframe[NUMBER_OF_eNB_MAX];
  unsigned char RRCConnReq_timer[NUMBER_OF_eNB_MAX];
  unsigned char *RRCConnectionRequest_ptr[NUMBER_OF_eNB_MAX];
  int turbo_iterations, turbo_cntl_iterations;
  int dlsch_errors[NUMBER_OF_eNB_MAX];
  int dlsch_errors_last[NUMBER_OF_eNB_MAX];
  int dlsch_received[NUMBER_OF_eNB_MAX];
  int dlsch_received_last[NUMBER_OF_eNB_MAX];
  int dlsch_fer[NUMBER_OF_eNB_MAX];
  int dlsch_SI_received[NUMBER_OF_eNB_MAX];
  int dlsch_SI_errors[NUMBER_OF_eNB_MAX];
  int dlsch_ra_received[NUMBER_OF_eNB_MAX];
  int dlsch_ra_errors[NUMBER_OF_eNB_MAX];
  int current_dlsch_cqi[NUMBER_OF_eNB_MAX];
  unsigned char first_run_timing_advance[NUMBER_OF_eNB_MAX];
  u8               generate_prach;
  u8               prach_timer;
  int              rx_offset; // Timing offset

  /// Flag to tell if UE is secondary user (cognitive mode)
  unsigned char    is_secondary_ue; 
  /// Flag to tell if secondary eNB has channel estimates to create NULL-beams from.
  unsigned char    has_valid_precoder; 
  /// hold the precoder for NULL beam to the primary eNB
  int              **ul_precoder_S_UE;
  /// holds the maximum channel/precoder coefficient
  char             log2_maxp; 

  /// Flag to initialize averaging of PHY measurements
  int init_averaging; 

  /// sinr for all subcarriers of the current link (used only for abstraction)
  double *sinr_dB;

  /// N0 (used for abstraction)
  double N0;
  
  /// PDSCH Varaibles
  PDSCH_CONFIG_DEDICATED pdsch_config_dedicated[NUMBER_OF_eNB_MAX];

  /// PUSCH Varaibles
  PUSCH_CONFIG_DEDICATED pusch_config_dedicated[NUMBER_OF_eNB_MAX];

  /// PUCCH variables
  PUCCH_CONFIG_DEDICATED pucch_config_dedicated[NUMBER_OF_eNB_MAX];

  u8 ncs_cell[20][7];

  /// UL-POWER-Control
  UL_POWER_CONTROL_DEDICATED ul_power_control_dedicated[NUMBER_OF_eNB_MAX];

  /// TPC
  TPC_PDCCH_CONFIG tpc_pdcch_config_pucch[NUMBER_OF_eNB_MAX];
  TPC_PDCCH_CONFIG tpc_pdcch_config_pusch[NUMBER_OF_eNB_MAX];

  /// CQI reporting
  CQI_REPORT_CONFIG cqi_report_config[NUMBER_OF_eNB_MAX];

  /// SRS Variables
  SOUNDINGRS_UL_CONFIG_DEDICATED soundingrs_ul_config_dedicated[NUMBER_OF_eNB_MAX];

  /// Scheduling Request Config
  SCHEDULING_REQUEST_CONFIG scheduling_request_config[NUMBER_OF_eNB_MAX];

  /// Transmission mode per eNB
  u8 transmission_mode[NUMBER_OF_eNB_MAX];


} PHY_VARS_UE;

#include "PHY/INIT/defs.h"
#include "PHY/LTE_REFSIG/defs.h"
#include "PHY/MODULATION/defs.h"
#include "PHY/LTE_TRANSPORT/proto.h"

#ifndef OPENAIR_LTE
#include "PHY/TRANSPORT/defs.h"
#include "PHY/ESTIMATION/defs.h"
#else //OPENAIR_LTE
#include "PHY/LTE_ESTIMATION/defs.h"
  //#include "PHY/LTE_REFSIG/defs.h"
  //#include "PHY/LTE_TRANSPORT/defs.h"
#endif //OPENAIR_LTE
//#ifdef USER_MODE
#include "SIMULATION/ETH_TRANSPORT/defs.h"
  //#endif
#endif //  __PHY_DEFS__H__






