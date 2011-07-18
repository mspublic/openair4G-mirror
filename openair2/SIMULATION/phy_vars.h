#ifndef __PHY_VARS_H__
#define __PHY_VARS_H__

#include "COMMON/openair_types.h"
#include "COMMON/openair_defs.h" 
#include  "SIMULATION/PHY_EMULATION/impl_defs.h" 
PHY_CONFIG *PHY_config;
unsigned int slot_count;
int node_configured=-1,node_running = 0;
unsigned int chbch_error_cnt[2],chbch_running_error_cnt[2];
unsigned int mchrach_error_cnt[2][8],sach_error_cnt=0;



unsigned char synch_source=0;

//PHY_VARS *PHY_vars;
short *twiddle_ifft,*twiddle_fft,*twiddle_fft_times4,*twiddle_ifft_times4;
unsigned int sync_pos;
unsigned short rev[1024],rev_times4[4096];


#endif /*__PHY_VARS_H__ */
