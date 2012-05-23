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
s16 *primary_synch0_time;
s16 *primary_synch1_time;
s16 *primary_synch2_time;
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
u16 rev256[256],rev512[512],rev1024[1024],rev4096[4096],rev2048[2048];

#ifdef OPENAIR_LTE
char mode_string[4][20] = {"NOT SYNCHED","PRACH","RAR","PUSCH"};
#include "PHY/LTE_TRANSPORT/vars.h"
#endif

#include "PHY/CODING/scrambler.h"

#ifdef USER_MODE
#include "SIMULATION/ETH_TRANSPORT/vars.h"
#endif

#ifndef OPENAIR2
unsigned char NB_eNB_INST=0;
unsigned char NB_UE_INST=0;
unsigned char NB_INST=0;
#endif


//extern  channel_desc_t *eNB2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];
//extern  double ABS_SINR_eff_BLER_table[MCS_COUNT][9][9];
//extern  double ABS_beta[MCS_COUNT];odi
double sinr_bler_map[MCS_COUNT][2][9];

double beta_dlsch[MCS_COUNT] = {1, 1, 1, 1, 1, 0.9459960937499999, 1.2912109374999994, 1.0133789062499998, 1.000390625,
				1.02392578125, 1.8595703124999998, 2.424389648437498, 2.3946533203124982, 2.5790039062499988,
				2.4084960937499984, 2.782617187499999, 2.7868652343749996, 3.92099609375, 4.0392578125,
				4.56109619140625, 5.03338623046875, 5.810888671875, 6.449108886718749};

#endif /*__PHY_VARS_H__ */
