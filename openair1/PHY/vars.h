#ifndef __PHY_VARS_H__
#define __PHY_VARS_H__

#include "PHY/types.h"
#include "PHY/defs.h" 

#ifndef USER_MODE
unsigned int RX_DMA_BUFFER[4][NB_ANTENNAS_RX];
unsigned int TX_DMA_BUFFER[4][NB_ANTENNAS_TX];
#endif

//PHY_CONFIG *PHY_config;

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

//PHY_VARS *PHY_vars;
PHY_VARS_UE **PHY_vars_UE_g;
PHY_VARS_eNB **PHY_vars_eNB_g;
LTE_DL_FRAME_PARMS *lte_frame_parms_g;

short *twiddle_ifft,*twiddle_fft,*twiddle_fft_times4,*twiddle_ifft_times4,*twiddle_fft_half,*twiddle_ifft_half;

#ifndef OPENAIR_LTE
CHBCH_RX_t rx_mode = ML;
#endif //OPENAIR_LTE

unsigned short rev[1024],rev_times4[4096],rev_half[512];

#ifdef OPENAIR_LTE
char mode_string[4][20] = {"NOT SYNCHED","PRACH","RAR","PUSCH"};
#include "PHY/LTE_TRANSPORT/vars.h"
#endif

#include "PHY/CODING/scrambler.h"

#ifdef USER_MODE
#include "SIMULATION/ETH_TRANSPORT/vars.h"
#endif

#ifndef OPENAIR2
unsigned char NB_eNB_INST=1;
unsigned char NB_UE_INST=1;
#endif


#endif /*__PHY_VARS_H__ */
