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
#define LTE_CE_FILTER_LENGTH 5
#define LTE_CE_OFFSET LTE_CE_FILTER_LENGTH
#define TX_RX_SWITCH_SYMBOL (NUMBER_OF_SYMBOLS_PER_FRAME>>1) 
#define PBCH_PDU_SIZE 6 //bytes

#define NUMBER_OF_FREQUENCY_GROUPS (lte_frame_parms->N_RB_DL)
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
  unsigned short symbols_per_tti;
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

typedef struct{
  int **txdata;
  mod_sym_t **txdataF;
  int **rxdata;           ///holds the received data in time domain (should point to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  int **rxdataF;          ///holds the received data in the frequency domain
} LTE_eNB_COMMON;

//typedef struct{
//} LTE_eNB_DLSCH;

typedef struct {
  int **rxdata;           ///holds the received data in time domain (should point to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  int **rxdataF;          ///holds the received data in the frequency domain
  int **dl_ch_estimates[3];  /// hold the channel estimates in frequency domain
  int **dl_ch_estimates_time;  /// hold the channel estimates in time domain (used for tracking)
  int *sync_corr;         /// holds output of the sync correlator
  int freq_offset;       /// estimated frequency offset (in radians) for all subcarriers
} LTE_UE_COMMON;

typedef struct {
  int **rxdataF_ext;
  int **rxdataF_comp;
  int **dl_ch_estimates_ext;
  int **dl_ch_rho_ext;
  int **dl_ch_mag;
  int **dl_ch_magb;
  int **rho;
  int **rho_i;
  short *llr[2];
} LTE_UE_DLSCH;

typedef struct {
  int **rxdataF_ext;
  int **rxdataF_comp;
  int **dl_ch_estimates_ext;
  short *llr;
  short *channel_output;
  unsigned char *decoded_output;
  unsigned int pdu_errors;          /// Total number of PDU errors
  unsigned int pdu_errors_last;     /// Total number of PDU errors 128 frames ago
  unsigned int pdu_errors_conseq;   /// Total number of consecutive PDU errors
  unsigned int pdu_fer;             /// FER (in percent) 
} LTE_UE_PBCH;


#endif


