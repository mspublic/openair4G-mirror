
#ifndef __PHY_EXTERN_H__
#define __PHY_EXTERN_H__

#include "PHY/defs.h"
#include "PHY/TOOLS/twiddle_extern.h"

extern unsigned int RX_DMA_BUFFER[4][NB_ANTENNAS_RX];
extern unsigned int TX_DMA_BUFFER[4][NB_ANTENNAS_TX];
extern short *twiddle_fft,*twiddle_ifft,*twiddle_fft_times4,*twiddle_ifft_times4,*twiddle_fft_half,*twiddle_ifft_half;
extern unsigned short rev[1024],rev_times4[4096],rev_half[512],rev1024[1024],rev256[256],rev512[512],rev2048[2048],rev4096[4096];

#ifdef OPENAIR_LTE
#include "PHY/LTE_TRANSPORT/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SIMULATION/ETH_TRANSPORT/extern.h"


//extern PHY_CONFIG *PHY_config;
//extern PHY_VARS *PHY_vars;

extern PHY_VARS_UE **PHY_vars_UE_g;
extern PHY_VARS_eNB **PHY_vars_eNB_g;
extern LTE_DL_FRAME_PARMS *lte_frame_parms_g;



extern short primary_synch0[144];
extern short primary_synch1[144];
extern short primary_synch2[144];
extern unsigned char primary_synch0_tab[72];
extern unsigned char primary_synch1_tab[72];
extern unsigned char primary_synch2_tab[72];
extern s16 *primary_synch0_time;
extern s16 *primary_synch1_time;
extern s16 *primary_synch2_time;
extern int *sync_corr_ue0;
extern int *sync_corr_ue1;
extern int *sync_corr_ue2;


//extern short **txdataF_rep_tmp;

extern char mode_string[4][20];

#include "PHY/LTE_TRANSPORT/extern.h"

#endif

#ifndef OPENAIR2
extern unsigned char NB_eNB_INST;
extern unsigned char NB_UE_INST;
#endif

extern double sinr_bler_map[MCS_COUNT][2][16];

//for MU-MIMO abstraction using MIESM
//this 2D arrarays contains SINR, MI and RBIR in rows 1, 2, and 3 respectively
extern double MI_map_4qam[3][162];
extern double MI_map_16qam[3][197];
extern double MI_map_64qam[3][227];

extern double beta1_dlsch_MI[6][MCS_COUNT];
extern double beta2_dlsch_MI[6][MCS_COUNT];


extern double beta1_dlsch[6][MCS_COUNT];
extern double beta2_dlsch[6][MCS_COUNT];

#endif /*__PHY_EXTERN_H__ */
 
