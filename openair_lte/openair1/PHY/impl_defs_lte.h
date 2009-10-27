/*________________________phy/impl_defs_lte.h________________________

 Authors : Raymond Knopp, Florian Kaltenberger
 Company : EURECOM
 Emails  : kaltenbe@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


#ifndef __PHY_IMPLEMENTATION_DEFS_LTE_H__
#define __PHY_IMPLEMENTATION_DEFS_LTE_H__


#include "types.h"
#include "spec_defs_top.h"

#define LTE_NUMBER_OF_SUBFRAMES_PER_FRAME 10

typedef struct {
  unsigned char N_RB_DL;                ///Number of resource blocks (RB) in DL
  unsigned char Nid_cell;               ///Cell ID 
  unsigned char Ncp;                    ///Cyclic Prefix (0=Normal CP, 1=Extended CP)
  unsigned char nushift;                ///shift of pilot position in one RB
  unsigned short ofdm_symbol_size;
  unsigned char log2_symbol_size;
  unsigned short nb_prefix_samples;
  unsigned short first_carrier_offset;
  unsigned int samples_per_tti;
  unsigned char nb_antennas_tx;
  unsigned char nb_antennas_rx;
  unsigned char first_dlsch_symbol;   
  short *twiddle_fft;                  ///pointer to twiddle factors for FFT
  short *twiddle_ifft;                  ///pointer to twiddle factors for IFFT
  unsigned short *rev;                 ///pointer to FFT permutation
} LTE_DL_FRAME_PARMS;

typedef enum {
  SISO,
  ALAMOUTI,
  ANTCYCLING,
  DUALSTREAM
} MIMO_mode_t;

#endif


