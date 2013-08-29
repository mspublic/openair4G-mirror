#ifndef __PHY_VARS_H__
#define __PHY_VARS_H__

#include "PHY/types.h"
#include "PHY/defs.h" 

#ifndef USER_MODE
unsigned int RX_DMA_BUFFER[4][NB_ANTENNAS_RX];
unsigned int TX_DMA_BUFFER[4][NB_ANTENNAS_TX];
unsigned int DAQ_MBOX;
#endif
char* namepointer_chMag ;
char fmageren_name2[512];
char* namepointer_log2;
//PHY_CONFIG *PHY_config;

#include "PHY/TOOLS/twiddle64.h"
#include "PHY/TOOLS/twiddle128.h"
#include "PHY/TOOLS/twiddle256.h"
#include "PHY/TOOLS/twiddle512.h"
#include "PHY/TOOLS/twiddle1024.h"
#include "PHY/TOOLS/twiddle2048.h"
#include "PHY/TOOLS/twiddle4096.h"
#include "PHY/TOOLS/twiddle8192.h"
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

unsigned short rev[2048],rev_times4[8192],rev_half[1024];
unsigned short rev256[256],rev512[512],rev1024[1024],rev4096[4096],rev2048[2048],rev8192[8192];

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

int flag_LA=0;
int flagMag;
//extern  channel_desc_t *eNB2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];
//extern  double ABS_SINR_eff_BLER_table[MCS_COUNT][9][9];
//extern  double ABS_beta[MCS_COUNT];odi
double sinr_bler_map[MCS_COUNT][2][20];
int table_length[MCS_COUNT];
//double sinr_bler_map_up[MCS_COUNT][2][16];

//for MU-MIMO abstraction using MIESM
//this 2D arrarays contains SINR, MI and RBIR in rows 1, 2, and 3 respectively
double MI_map_4qam[3][162];
double MI_map_16qam[3][197];
double MI_map_64qam[3][227];

//for SNR to MI conversion 7 th order Polynomial coeff
double q_qam16[8]={3.21151853033897e-10,5.55435952230651e-09,-2.30760065362117e-07,-6.25587743817859e-06,4.62251036452795e-06,0.00224150813158937,0.0393723140344367,0.245486379182639};
double q_qpsk[8]={1.94491167814437e-09,8.40494123817774e-08,4.75527131198034e-07,-2.48946285301621e-05,-0.000347614016158364,0.00209252225437100,0.0742986115462510,0.488297879889425};
double q_qam64[8]={2.25934026232206e-11,-1.45992206328306e-10,-3.70861183071900e-08,-1.22206071019319e-06,6.49115500399637e-06,0.00129828997837433,0.0259669554914859,0.166602901214898};

//for MI to SNR conversion 7 th order Polynomial coeff
double p_qpsk[8]= {5982.42405670359,-21568.1135917693,31293.9987036905,-23394.6795043871,9608.34750585489,-2158.15802349899,267.731968719036,-20.6145324295965};
double p_qam16[8]={7862.12690694170,-28510.3207048338,41542.2150287122,-31088.3036957379,12690.1982361016,-2785.66604739984,326.595462489375,-18.9911849872089};
double p_qam64[8]={8832.57933013696,-32119.1802555952,46914.2578990397,-35163.8150557183,14343.7419388853,-3126.61025510092,360.954930562237,-18.0358548533343};

double beta1_dlsch_MI[6][MCS_COUNT] = { {1.17, 1.11, 1.04, 1.05, 1.09, 1.09, 1.10, 1.10, 1.09, 1.03, 1.04, 1.04, 1.05, 1.07, 1.07, 1.06, 1, 1,1,1,1,1,1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1},  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1},{1.808065416202085, 1.754544945430673,1.902272019362616, 1.790054645392961, 1.563204092967629, 1.585258289348813, 1.579349443720310, 1.570650121437345, 1.545055626608596, 1.362229442426877, 1.85, 1.79, 1.65, 1.54, 1.46, 1.39, 1.33, 1,1,1,1,1,1,1},{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1}};
//double beta1_dlsch_MI[6][28] = { {1.17, 1.11, 1.04, 1.05, 1.09, 1.09, 1.10, 1.10, 1.09, 1.03, 1.04, 1.04, 1.05, 1.07, 1.07, 1.06, 1, 1,1,1,1,1,1,1,1,1,1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1,1,1,1,1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1,1,1,1,1},  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1,1,1,1,1},{1.808065416202085, 1.754544945430673,1.902272019362616, 1.790054645392961, 1.563204092967629, 1.585258289348813, 1.579349443720310, 1.570650121437345, 1.545055626608596, 1.362229442426877, 1.85, 1.79, 1.65, 1.54, 1.46, 1.39, 1.33, 1,1,1,1,1,1,1,1,1,1,1},{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1,1,1,1,1}};
double beta2_dlsch_MI[6][MCS_COUNT] = { {1.17, 1.11, 1.04, 1.05, 1.09, 1.09, 1.10, 1.10, 1.09, 1.03, 1.04, 1.04, 1.05, 1.07, 1.07, 1.06, 1, 1,1,1,1,1,1,1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1}, {1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1},  {1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1},{1.079518113138858, 1.105622953570353, 1.031337449900606, 1.073342032668810, 1.242636589110353, 1.255054927783647, 1.291463834317768, 1.317048698347491, 1.354485054187984, 0.338534029291017, 1.85, 1.79, 1.65, 1.54, 1.46, 1.39, 1.33,1, 1,1,1,1,1,1},{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1}};
//double beta2_dlsch_MI[6][28] = { {1.17, 1.11, 1.04, 1.05, 1.09, 1.09, 1.10, 1.10, 1.09, 1.03, 1.04, 1.04, 1.05, 1.07, 1.07, 1.06, 1, 1,1,1,1,1,1,1,1,1,1,1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1,1,1,1,1}, {1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1,1,1,1},  {1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1,1,1,1},{1.079518113138858, 1.105622953570353, 1.031337449900606, 1.073342032668810, 1.242636589110353, 1.255054927783647, 1.291463834317768, 1.317048698347491, 1.354485054187984, 0.338534029291017, 1.85, 1.79, 1.65, 1.54, 1.46, 1.39, 1.33,1, 1,1,1,1,1,1,1,1,1,1},{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1,1,1,1,1}};

//ideal channel estimation values
double beta1_dlsch[6][MCS_COUNT] = { {2.3814, 0.4956, 0.5273, 1.1708, 0.8014, 0.7889, 0.8111, 0.8139, 0.8124, 0.8479, 1.9280, 1.9664, 2.3857, 2.5147, 2.4511, 3.0158, 2.8643, 5.3013, 5.8594, 6.5372, 7.8073, 7.8030, 7.5295, 7.1320}, {0.5146, 0.5549, 0.7405, 0.6913, 0.7349, 0.7000, 0.7539, 0.7955, 0.8074, 0.7760, 1.8150, 1.6561, 1.9280, 2.3563, 2.6699, 2.3086, 3.1601, 4.5316, 5.2870, 6.0983, 6.5635, 7.7024, 9.9592, 6.6173}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1}, {1.79358, 1.17908, 2.02600, 1.72040, 1.58618, 1.59039, 1.68111, 1.67062, 1.64911, 1.33274, 4.87800, 3.58797, 3.72338, 5.35700, 2.81752, 1.93472, 2.23259, 1,1,1,1,1,1,1}, {0.4445, 0.5918, 0.7118, 0.7115, 0.7284, 0.7202, 0.7117, 0.8111, 0.8239, 0.7907, 1.8456, 1.8144, 2.3830, 2.6634, 2.6129, 2.8127, 2.7372, 4.9424, 4.8763, 6.8413, 7.1493, 9.4180, 10.1230, 8.9613}};
double beta2_dlsch[6][MCS_COUNT] = { {2.3639, 0.4952, 0.5207, 1.1572, 0.8026, 0.7864, 0.7996, 0.8034, 0.8200, 0.8367, 1.8701, 1.9212, 2.2947, 2.4472, 2.4091, 2.9479, 2.8973, 5.0591, 5.5134, 6.1483, 7.2166, 7.5177, 7.5704, 7.2248}, {0.5113, 0.5600, 0.7359, 0.6860, 0.7344, 0.6902, 0.7315, 0.7660, 0.7817, 0.7315, 1.7268, 1.5912, 1.8519, 2.2115, 2.4580, 2.1879, 2.9015, 4.1543, 4.6986, 5.3193, 5.6319, 6.5640, 8.2421, 5.6393}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1}, {0.79479, 0.52872, 0.90005, 0.77170, 0.73220, 0.72060, 0.75433, 0.75451, 0.75989, 0.67655, 1.68525, 1.31100, 1.46573, 1.99843, 1.57293, 1.62852, 2.10636, 1,1,1,1,1,1,1}, {0.4398, 0.5823, 0.7094, 0.7043, 0.7282, 0.7041, 0.6979, 0.7762, 0.7871, 0.7469, 1.7752, 1.7443, 2.2266, 2.4767, 2.4146, 2.6040, 2.5708, 4.4488, 4.4944, 5.9630, 6.3740, 8.1097, 8.4210, 7.8027}};
//real channel estimation valus
//double beta1_dlsch[6][MCS_COUNT] = { {2.50200, 0.84047, 0.78195, 1.37929, 1.16871, 1.11906, 1.06303, 1.07447, 1.11403, 1.09223, 2.82502, 2.87556, 3.51254, 3.62920, 3.53638, 2.35980, 3.74126, 8.66532, 7.31772, 9.86882, 10.64939, 6.75208, 9.50664, 13.63057}, {0.92257, 1, 1.80445, 1.43175, 1.42093, 1.37381, 1.45392, 1.47255, 1.47451, 1.41235, 3.9079, 3.38557, 4.13059, 4.93355, 4.97277, 6.04951, 5.88896, 8.68076, 10.23746, 12.37069, 5.50538, 17.29612, 17.95050, 13.27095}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1}, {1.79255, 1.88213, 4.44226, 2.25150, 1.93710, 2.18504, 2.57389, 1.94322, 1.78515, 2.09265, 2.37459, 1.74442, 1.74346, 1.19705, 1.56149, 1.59604, 1.6, 1,1,1,1,1,1,1}, {0.93374, 1.40389, 1.36670, 1.38679, 1.35707, 1.26353, 1.32360, 1.40164, 1.51843, 1.34863, 3.45839, 3.13726, 3.94768, 4.21966, 4.60750, 4.97894, 5.40755, 8.12814, 10.59221, 12.96427, 13.37323, 14.27206, 16.61779, 17.19656}};
//double beta2_dlsch[6][MCS_COUNT] = { {2.52163, 0.83231, 0.77472, 1.36536, 1.16829, 1.11186, 1.06287, 1.07292, 1.09946, 1.10650, 2.79174, 2.75655, 3.36651, 3.49011, 3.60903, 2.73517, 3.84009, 8.20312, 7.41739, 9.64081, 10.40911, 8.11765, 10.41923, 9.34300}, {0.67252, 0.8600, 1.28633, 1.01624, 1.03066, 0.97590, 1.02560, 1.01840, 1.00547, 0.97093, 2.72573, 2.33283, 2.86181, 3.40452, 3.47957, 4.08916, 3.97628, 6.14541, 7.11017, 8.42369, 4.04812, 11.42082, 11.57171, 9.28462}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1,1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1,1,1}, {0.85784, 0.90361, 2.09766, 1.08385, 0.96300, 1.04432, 1.22763, 0.99249, 0.95544, 1.12333, 1.37924, 1.12913, 1.30644, 1.19253, 1.75488, 2.13813, 2.10636, 1,1,1,1,1,1,1}, {0.66288, 0.96402, 0.98545, 0.99386, 0.99981, 0.92678, 0.98978, 0.99600, 1.05538, 0.97777, 2.52504, 2.29338, 2.89631, 3.10812, 3.41916, 3.58671, 3.84166, 6.05254, 7.45821, 9.15812, 9.66330, 10.17852, 11.50519, 11.16299}};



#endif /*__PHY_VARS_H__ */