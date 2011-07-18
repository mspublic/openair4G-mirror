/*________________________radio_emulation.h________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr , knopp@eurecom.fr
________________________________________________________________*/


#ifndef __RADIO_EMULATION_H__
#define __RADIO_EMULATION_H__
#include "complex.h"
short SAT_ADD_FIX(short A,short B);
short FIX_MPY_SAT(short A,short B);       
void radio_emulation_init();
void SAT_CMPLX_ADD(complex16 *DEST, complex16 A, complex16 B);
void FIX_CMPLX_MPY(complex16 *DEST, complex16 A, complex16 B);
void generate_inst_rssi(unsigned short,unsigned short);
unsigned short rssi_dB_2_fixed(char x );//x between -120dB & -40dB with a step of 0.25 dB
#define N_BIT 16
#define L_DEV 15
#define ALPHA 8 
#define FIX_SCALE_FAC 128*128 //(2^6*2^6)
#define N0_dB -100
#define N0_LN  96
#define ADD_NP_PER_FG 0.5 //ADDITIONAL NOISE ENERGY (dB) PER FREQUENCY GROUP
#define RX_SINR_TRESHOLD 9  //dB 
 //#defne N0_LN 
#endif
