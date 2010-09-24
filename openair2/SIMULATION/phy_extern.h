
#ifndef __PHY_EXTERN_H__
#define __PHY_EXTERN_H__

#include "COMMON/openair_types.h"
#include "COMMON/openair_defs.h" 
#include  "SIMULATION/PHY_EMULATION/impl_defs.h" 
//#include  "SIMULATION/PHY_EMULATION/spec_defs.h" 

extern PHY_CONFIG *PHY_config;
extern unsigned char scrambling_sequence[];
extern unsigned int chbch_error_cnt[2],chbch_running_error_cnt[2];
extern unsigned int mchrach_error_cnt[2][8],sach_error_cnt;


extern unsigned char synch_source;


#ifdef PC_TARGET

//extern PHY_VARS *PHY_vars;

//extern PHY_LINKS *PHY_links;

extern short *twiddle_fft,*twiddle_ifft,*twiddle_fft_times4,*twiddle_ifft_times4;
extern unsigned short rev[];

extern unsigned short rev_times4[];

extern unsigned int sync_pos;


#endif //PC_TARGET

#endif /*__PHY_EXTERN_H__ */
 
