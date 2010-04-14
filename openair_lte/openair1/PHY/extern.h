
#ifndef __PHY_EXTERN_H__
#define __PHY_EXTERN_H__

#include "PHY/defs.h"
#include "PHY/TOOLS/twiddle_extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"

extern unsigned int RX_DMA_BUFFER[4][NB_ANTENNAS_RX];
extern unsigned int TX_DMA_BUFFER[4][NB_ANTENNAS_TX];

extern PHY_CONFIG *PHY_config;

#ifndef OPENAIR_LTE
extern unsigned char scrambling_sequence[];
extern unsigned int chbch_error_cnt[2],chbch_running_error_cnt[2];
extern unsigned int mchrach_error_cnt[2][8],sach_error_cnt;
#endif //OPENAIR_LTE

//extern unsigned char synch_source;
extern unsigned char dual_stream_flag;
extern unsigned int sync_pos;
#ifndef OPENAIR_LTE
extern CHBCH_RX_t rx_mode;
#endif //OPENAIR_LTE

extern PHY_VARS *PHY_vars;

//extern PHY_LINKS *PHY_links;

extern short *twiddle_fft,*twiddle_ifft,*twiddle_fft_times4,*twiddle_ifft_times4,*twiddle_fft_half,*twiddle_ifft_half;
extern unsigned short rev[1024],rev_times4[4096],rev_half[512];

#ifdef OPENAIR_LTE
extern short primary_synch0[144];
extern short primary_synch1[144];
extern short primary_synch2[144];
extern unsigned char primary_synch0_tab[72];
extern unsigned char primary_synch1_tab[72];
extern unsigned char primary_synch2_tab[72];
extern int *primary_synch0_time;
extern int *primary_synch1_time;
extern int *primary_synch2_time;
extern int *sync_corr;

extern LTE_DL_FRAME_PARMS *lte_frame_parms;
extern LTE_UE_COMMON *lte_ue_common_vars;
extern LTE_UE_DLSCH **lte_ue_dlsch_vars,**lte_ue_dlsch_vars_cntl,**lte_ue_dlsch_vars_ra,**lte_ue_dlsch_vars_1A;
extern LTE_UE_PDCCH **lte_ue_pdcch_vars;
extern LTE_UE_PBCH **lte_ue_pbch_vars;
extern LTE_eNB_COMMON *lte_eNB_common_vars;
extern LTE_eNb_DLSCH_t **dlsch_eNb,*dlsch_eNb_cntl,*dlsch_eNb_ra,*dlsch_eNb_1A;
extern LTE_eNB_ULSCH **lte_eNB_ulsch_vars;
extern LTE_UE_DLSCH_t **dlsch_ue,*dlsch_ue_cntl,*dlsch_ue_ra,*dlsch_ue_1A;
extern LTE_eNb_ULSCH_t **ulsch_eNb;
extern LTE_UE_ULSCH_t **ulsch_ue;
extern LTE_eNB_UE_stats eNB_UE_stats[NUMBER_OF_eNB_MAX];

extern DCI0_5MHz_TDD0_t          UL_alloc_pdu;
extern DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
extern DCI1A_5MHz_TDD_1_6_t      BCCH_alloc_pdu;
extern DCI1A_5MHz_TDD_1_6_t      DLSCH_alloc_pdu1A;
extern DCI1A_5MHz_TDD_1_6_t      RA_alloc_pdu;
extern DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
extern DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;

extern UE_MODE_t UE_mode;
extern char mode_string[4][20];

extern unsigned short t_crnti;
#include "PHY/LTE_TRANSPORT/extern.h"

#endif


#endif /*__PHY_EXTERN_H__ */
 
