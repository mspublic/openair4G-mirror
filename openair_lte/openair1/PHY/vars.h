#ifndef __PHY_VARS_H__
#define __PHY_VARS_H__

#include "PHY/types.h"
#include "PHY/defs.h" 

PHY_CONFIG *PHY_config;

unsigned int slot_count;

int node_configured=-1,node_running = 0;
unsigned int chbch_error_cnt[2],chbch_running_error_cnt[2];
unsigned int mchrach_error_cnt[2][8],sach_error_cnt=0;

unsigned int RX_DMA_BUFFER[4][NB_ANTENNAS_RX];
unsigned int TX_DMA_BUFFER[4][NB_ANTENNAS_TX];

//unsigned char synch_source=0;

#include "PHY/TOOLS/twiddle64.h"
#include "PHY/TOOLS/twiddle128.h"
#include "PHY/TOOLS/twiddle256.h"
#include "PHY/TOOLS/twiddle512.h"
#include "PHY/TOOLS/twiddle1024.h"
#include "PHY/TOOLS/twiddle2048.h"
#include "PHY/TOOLS/twiddle4096.h"
#include "PHY/TOOLS/twiddle32768.h"

#ifdef OPENAIR_LTE
#include "PHY/LTE_REFSIG/primary_synch.h"
int *primary_synch0_time;
int *primary_synch1_time;
int *primary_synch2_time;
#endif

#include "PHY/CODING/vars.h"

PHY_VARS *PHY_vars;

short *twiddle_ifft,*twiddle_fft,*twiddle_fft_times4,*twiddle_ifft_times4,*twiddle_fft_half,*twiddle_ifft_half;

unsigned int sync_pos;

unsigned char dual_stream_flag = 0;

#ifndef OPENAIR_LTE
CHBCH_RX_t rx_mode = ML;
#endif //OPENAIR_LTE
unsigned short rev[1024],rev_times4[4096],rev_half[512];

#ifdef OPENAIR_LTE
LTE_DL_FRAME_PARMS *lte_frame_parms;
LTE_UE_COMMON *lte_ue_common_vars;
LTE_UE_DLSCH **lte_ue_dlsch_vars, **lte_ue_dlsch_vars_cntl,**lte_ue_dlsch_vars_ra,**lte_ue_dlsch_vars_1A;
LTE_UE_PDCCH **lte_ue_pdcch_vars;
LTE_UE_PBCH **lte_ue_pbch_vars;
LTE_eNB_COMMON *lte_eNB_common_vars;
LTE_eNB_ULSCH **lte_eNB_ulsch_vars;
LTE_eNb_DLSCH_t **dlsch_eNb,*dlsch_eNb_1A,*dlsch_eNb_cntl,*dlsch_eNb_ra;
LTE_UE_DLSCH_t **dlsch_ue,*dlsch_ue_cntl,*dlsch_ue_ra,*dlsch_ue_1A;
LTE_eNb_ULSCH_t **ulsch_eNb;
LTE_UE_ULSCH_t **ulsch_ue;
LTE_eNB_UE_stats eNB_UE_stats[NUMBER_OF_eNB_MAX];
DCI1A_5MHz_TDD_1_6_t      RA_alloc_pdu;

DCI0_5MHz_TDD0_t          UL_alloc_pdu;
DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
DCI1A_5MHz_TDD_1_6_t      DLSCH_alloc_pdu1A;
DCI1A_5MHz_TDD_1_6_t      BCCH_alloc_pdu;

DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;

UE_MODE_t UE_mode;

char mode_string[4][20] = {"NOT SYNCHED","PRACH","RAR","ULSCH"};


 
#include "PHY/LTE_TRANSPORT/vars.h"

#endif

#include "PHY/CODING/scrambler.h"

#endif /*__PHY_VARS_H__ */
