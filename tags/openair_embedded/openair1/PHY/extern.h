
#ifndef __PHY_EXTERN_H__
#define __PHY_EXTERN_H__

#include "PHY/defs.h"
#include "PHY/TOOLS/twiddle_extern.h"
#include "MAC_INTERFACE/defs.h"

extern PHY_CONFIG *PHY_config;
extern unsigned char scrambling_sequence[];
extern unsigned int chbch_error_cnt[2],chbch_running_error_cnt[2];
extern unsigned int mchrach_error_cnt[2][8],sach_error_cnt;


//extern unsigned char synch_source;
extern unsigned char dual_stream_flag;
extern unsigned int sync_pos;
extern CHBCH_RX_t rx_mode;

extern PHY_VARS *PHY_vars;

//extern PHY_LINKS *PHY_links;

extern short *twiddle_fft,*twiddle_ifft,*twiddle_fft_times4,*twiddle_ifft_times4,*twiddle_fft_half,*twiddle_ifft_half;
extern unsigned short rev[1024],rev_times4[4096],rev_half[512];




#endif /*__PHY_EXTERN_H__ */
 
