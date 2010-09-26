
/** @defgroup sched_h_ SCHED header file
* @ingroup _PROCESS_SCHEDULING_
                          openair_sched.h  -  description
                             -------------------
  \author Lionel GAUTHIER (wireless3G4Free Version), modified for openair by R. Knopp, modified for LTE by F. Kaltenberger
  \company EURECOM
  \email knopp@eurecom.fr
* @{
*/

#ifndef __openair_SCHED_H__
#define __openair_SCHED_H__

#include "PHY/defs.h"
#ifdef EMOS
#include "phy_procedures_emos.h"
#endif //EMOS

enum THREAD_INDEX { OPENAIR_THREAD_INDEX = 0,
		    TOP_LEVEL_SCHEDULER_THREAD_INDEX,
                    DLC_SCHED_THREAD_INDEX,
                    openair_SCHED_NB_THREADS}; // do not modify this line


#define OPENAIR_THREAD_PRIORITY        255


#define OPENAIR_THREAD_STACK_SIZE    8192 //4096 //RTL_PTHREAD_STACK_MIN*6
//#define DLC_THREAD_STACK_SIZE        4096 //DLC stack size




enum openair_SCHED_STATUS {
      openair_SCHED_STOPPED=1,
      openair_SCHED_STARTING,
      openair_SCHED_STARTED,
      openair_SCHED_STOPPING};

enum openair_ERROR {
  // HARDWARE CAUSES
  openair_ERROR_HARDWARE_CLOCK_STOPPED= 1,

  // SCHEDULER CAUSE
  openair_ERROR_OPENAIR_RUNNING_LATE,
  openair_ERROR_OPENAIR_SCHEDULING_FAILED,

  // OTHERS
  openair_ERROR_OPENAIR_TIMING_OFFSET_OUT_OF_BOUNDS,
};

enum openair_SYNCH_STATUS {
      openair_NOT_SYNCHED=1,
#ifdef OPENAIR_LTE
      openair_SYNCHED,
#else
      openair_SYNCHED_TO_CHSCH,
      openair_SYNCHED_TO_MRSCH,
#endif
      openair_SCHED_EXIT};


#define DAQ_AGC_ON 1
#define DAQ_AGC_OFF 0


typedef struct {
  unsigned char mode;
  unsigned char synch_source;
  unsigned int  slot_count;
  unsigned int  sched_cnt;
  unsigned int  synch_wait_cnt;
  unsigned int  sync_state;
  unsigned int  scheduler_interval_ns;
  unsigned int  last_adac_cnt;
  unsigned char first_sync_call;
  int  instance_cnt;
  unsigned char one_shot_get_frame;
  unsigned char node_configured;  // &1..basic config, &3..ue config &5..eNb config
  unsigned char node_running;
  unsigned char tx_test;
  unsigned char mac_registered;
  unsigned char freq;
  unsigned int  freq_info;
  unsigned int  rx_gain_val;
  unsigned int  rx_gain_mode;
  unsigned int  tcxo_dac;
  int           freq_offset;
  unsigned int  tx_rx_switch_point;
  unsigned int  manual_timing_advance;  /// 1 to override automatic timing advance
  unsigned int  timing_advance;
  unsigned int  dual_tx;                /// 1 for dual-antenna TX, 0 for single-antenna TX
  unsigned int  tdd;                    /// 1 for TDD mode, 0 for FDD mode
  unsigned int  rx_rf_mode;
  unsigned int  node_id;
  unsigned int  rach_detection_count;
  unsigned int  channel_vacant[4];  
  unsigned int  target_ue_dl_mcs;
  unsigned int  target_ue_ul_mcs;
  unsigned int  ue_ul_nb_rb;
  unsigned int  dlsch_rate_adaptation;
  unsigned int  dlsch_transmission_mode;
  unsigned int  ulsch_allocation_mode;
} OPENAIR_DAQ_VARS;

#ifndef USER_MODE
int openair_sched_init(void);
void openair_sched_cleanup(void);
void openair_sched_exit(char *);
void openair1_restart(void);
#endif //USER_MODE

#ifdef OPENAIR_LTE

void phy_procedures_eNb_lte(unsigned char last_slot, unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNb);
void phy_procedures_ue_lte(unsigned char last_slot, unsigned char next_slot,PHY_VARS_UE *phy_vars_ue,u8 eNB_id);

typedef enum {SF_DL, SF_UL, SF_S} lte_subframe_t;

void get_RRCConnReq_alloc(unsigned char tdd_config,
		   unsigned char current_subframe, 
		   unsigned int current_frame,
		   unsigned int *frame,
		   unsigned char *subframe);
unsigned int is_phich_subframe(unsigned char tdd_config,unsigned char subframe);
void phy_procedures_UE_TX(unsigned char next_slot,PHY_VARS_UE *phy_vars_ue,u8 eNb_id);
int phy_procedures_UE_RX(unsigned char last_slot,PHY_VARS_UE *phy_vars_ue,u8 eNb_id);
void phy_procedures_UE_S_TX(unsigned char next_slot,PHY_VARS_UE *phy_vars_ue,u8 eNb_id);
void phy_procedures_UE_S_RX(unsigned char last_slot,PHY_VARS_UE *phy_vars_ue,u8 eNb_id);
void phy_procedures_eNB_TX(unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNb);
void phy_procedures_eNB_RX(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNb);
void phy_procedures_eNB_S_TX(unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNb);
void phy_procedures_eNB_S_RX(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNb);
void get_ue_active_harq_pid(u8 Mod_id,u16 rnti,u8 subframe,u8 *harq_pid,u8 *round);
s8 find_ue(u16 rnti, PHY_VARS_eNB *phy_vars_eNb);

unsigned char get_ack(unsigned char tdd_config,harq_status_t *harq_ack,unsigned char subframe,unsigned char *o_ACK);
lte_subframe_t subframe_select_tdd(unsigned char tdd_config,unsigned char subframe);
unsigned char ul_ACK_subframe2_dl_subframe(unsigned char tdd_config,unsigned char subframe,unsigned char ACK_index);
u8 get_RRCConnReq_harq_pid(unsigned char tdd_config,unsigned char current_subframe);
void process_timing_advance(PHY_VARS_UE *phy_vars_ue, unsigned char timing_advance);
void process_timing_advance_rar(PHY_VARS_UE *phy_vars_ue,unsigned short timing_advance);

#else
#ifdef EMOS
void phy_procedures_emos(unsigned char last_slot);
#else
void phy_procedures(unsigned char last_slot);
#endif //EMOS
#endif //OPENAIR_LTE

#ifndef OPENAIR_LTE
unsigned int find_chbch(void);
unsigned int find_mrbch(void);
#endif

#endif

/*@}*/
