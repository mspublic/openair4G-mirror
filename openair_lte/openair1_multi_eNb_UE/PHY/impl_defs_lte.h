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
#define PBCH_PDU_SIZE 3 //bytes
#define TIMING_ADVANCE_INIT 0

#define PRACH_SYMBOL 3 //position of the UL PSS wrt 2nd slot of special subframe

#define NUMBER_OF_FREQUENCY_GROUPS (lte_frame_parms->N_RB_DL)
typedef struct {
  unsigned char N_RB_DL;                /// Number of resource blocks (RB) in DL
  unsigned char N_RB_UL;                /// Number of resource blocks (RB) in UL
  unsigned char Nid_cell;               /// Cell ID 
  unsigned char Ncp;                    /// Cyclic Prefix (0=Normal CP, 1=Extended CP)
  unsigned char Ng_times6;              /// Number of PHICH groups (times 6, 1,3,6,12)
  unsigned char nushift;                /// shift of pilot position in one RB
  unsigned char frame_type;             /// Frame type (0 FDD, 1 TDD)
  unsigned char tdd_config;             /// TDD Configuration Number (0-9) (default = 3)
  unsigned char mode1_flag;             /// flag to indicate SISO transmission
  unsigned short ofdm_symbol_size;
  unsigned char log2_symbol_size;
  unsigned short nb_prefix_samples;
  unsigned short nb_prefix_samples0;
  unsigned short first_carrier_offset;
  unsigned int samples_per_tti;
  unsigned short symbols_per_tti;
  unsigned char nb_antennas_tx;
  unsigned char nb_antennas_rx;
  unsigned char first_dlsch_symbol;   
  unsigned char num_dlsch_symbols;   
  short *twiddle_fft;                  ///pointer to twiddle factors for FFT
  short *twiddle_ifft;                 ///pointer to twiddle factors for IFFT
  unsigned short *rev;                 ///pointer to FFT permutation
} LTE_DL_FRAME_PARMS;

typedef enum {
  SISO=0,
  ALAMOUTI=1,
  ANTCYCLING=2,
  UNIFORM_PRECODING11=3,
  UNIFORM_PRECODING1m1=4,
  UNIFORM_PRECODING1j=5,
  UNIFORM_PRECODING1mj=6,
  PUSCH_PRECODING0=7,
  PUSCH_PRECODING1=8,
  DUALSTREAM_UNIFORM_PRECODING1=9,
  DUALSTREAM_UNIFORM_PRECODINGj=10,
  DUALSTREAM_PUSCH_PRECODING=11
} MIMO_mode_t;

typedef struct{
  int **txdata[3];           ///holds the transmit data in time domain (#ifdef IFFT_FPGA this points to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  mod_sym_t **txdataF[3];    ///holds the transmit data in the frequency domain (#ifdef IFFT_FPGA this points to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  int **rxdata[3];           ///holds the received data in time domain (should point to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  int **rxdataF[3];          ///holds the received data in the frequency domain
  unsigned int *sync_corr[3];         /// holds output of the sync correlator
} LTE_eNB_COMMON;

typedef struct{
  int **srs_ch_estimates[3];   /// hold the channel estimates in frequency domain based on SRS
  int **srs_ch_estimates_time[3];    /// hold the channel estimates in time domain based on SRS
  int *srs;               /// holds the SRS for channel estimation at the RX
} LTE_eNB_SRS;

typedef struct{
  ///holds the received data in the frequency domain for the allocated RBs
  int **rxdataF_ext[3];       
  int **rxdataF_ext2[3];       
  /// hold the channel estimates in frequency domain based on DRS   
  int **drs_ch_estimates[3]; 
  int **drs_ch_estimates_0[3];// hold the channel estimates for UE0 in case of Distributed Alamouti Scheme
  int **drs_ch_estimates_1[3];// hold the channel estimates for UE1 in case of Distributed Almouti Scheme 
  int **rxdataF_comp[3];
  int **rxdataF_comp_0[3];// hold the compensated data (y)*(h0*) in case of Distributed Alamouti Scheme
  int **rxdataF_comp_1[3];// hold the compensated data (y*)*(h1) in case of Distributed Alamouti Scheme
  int **ul_ch_mag[3];
  int **ul_ch_magb[3];
  int **ul_ch_mag_0[3];   // hold the channel mag for UE0 in case of Distributed Alamouti Scheme
  int **ul_ch_magb_0[3];  // hold the channel magb for UE0 in case of Distributed Alamouti Scheme
  int **ul_ch_mag_1[3];   // hold the channel mag for UE1 in case of Distributed Alamouti Scheme
  int **ul_ch_magb_1[3];  // hold the channel magb for UE1 in case of Distributed Alamouti Scheme
  short *llr;
} LTE_eNB_ULSCH;

typedef struct {
  int **txdata;           ///holds the transmit data in time domain (#ifdef IFFT_FPGA this points to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  mod_sym_t **txdataF;    ///holds the transmit data in the frequency domain (#ifdef IFFT_FPGA this points to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  int **rxdata;           ///holds the received data in time domain (should point to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  int **rxdataF;          ///holds the received data in the frequency domain
  int **dl_ch_estimates[3];  /// hold the channel estimates in frequency domain
  int **dl_ch_estimates_time;  /// hold the channel estimates in time domain (used for tracking)
  int *sync_corr;         /// holds output of the sync correlator
  int freq_offset;          /// estimated frequency offset (in radians) for all subcarriers
  unsigned char eNb_id;     /// eNb_id user is synched to
} LTE_UE_COMMON;

typedef struct {
  int **rxdataF_ext;
  int **rxdataF_comp;
  int **dl_ch_estimates_ext;
  int **dl_ch_rho_ext;
  unsigned char *pmi_ext;
  int **dl_ch_mag;
  int **dl_ch_magb;
  int **rho;
  int **rho_i;  //never used... always send dl_ch_rho_ext instead...
  short *llr[2];
  unsigned char log2_maxh;
  short **llr128;  // to be type casted locally
  //unsigned int *rb_alloc;
  //unsigned char Qm[2];
  //MIMO_mode_t mimo_mode;
} LTE_UE_DLSCH;

typedef struct {
  int **rxdataF_ext;
  int **rxdataF_comp;
  int **dl_ch_estimates_ext;
  int **dl_ch_rho_ext;
  int **rho;
  unsigned short *llr;
  unsigned short *llr16;
  unsigned short *wbar;
  char *e_rx;
  u8 num_pdcch_symbols;
  unsigned short crnti;
  unsigned int dci_errors;          /// Total number of PDU errors (diagnostic mode)
  unsigned int dci_received;        /// Total number of PDU received
  unsigned int dci_false;           /// Total number of DCI False detection (diagnostic mode)
  unsigned int dci_missed;          /// Total number of DCI missed (diagnostic mode)
} LTE_UE_PDCCH;

typedef struct {
  int **rxdataF_ext;
  int **rxdataF_comp;
  int **dl_ch_estimates_ext;
  char *llr;
  short *channel_output;
  unsigned char *decoded_output;
  unsigned int pdu_errors;          /// Total number of PDU errors
  unsigned int pdu_errors_last;     /// Total number of PDU errors 128 frames ago
  unsigned int pdu_errors_conseq;   /// Total number of consecutive PDU errors
  unsigned int pdu_fer;             /// FER (in percent) 
} LTE_UE_PBCH;

typedef enum {
  NOT_SYNCHED=0,
  PRACH,
  RA_RESPONSE,
  PUSCH
} UE_MODE_t;


#endif


