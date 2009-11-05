
#ifndef __PHY_EXTERN_H__
#define __PHY_EXTERN_H__

#include "PHY/defs.h"
#include "PHY/TOOLS/twiddle_extern.h"
#include "MAC_INTERFACE/defs.h"

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
extern short primary_synch0[146];
extern short primary_synch1[146];
extern short primary_synch2[146];
extern int *primary_synch0_time;
extern int *primary_synch1_time;
extern int *primary_synch2_time;

extern LTE_DL_FRAME_PARMS *lte_frame_parms;
extern LTE_UE_COMMON *lte_ue_common_vars;
extern LTE_UE_DLSCH *lte_ue_dlsch_vars;
#endif


#endif /*__PHY_EXTERN_H__ */
 
