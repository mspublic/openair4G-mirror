/*________________________phy/impl_defs_lte.h________________________

 Authors : Raymond Knopp, Florian Kaltenberger
 Company : EURECOM
 Emails  : kaltenbe@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


#ifndef __PHY_IMPLEMENTATION_DEFS_LTE_H__
#define __PHY_IMPLEMENTATION_DEFS_LTE_H__


#include "types.h"
#include "spec_defs_top.h"
#include "defs.h"

#define LTE_NUMBER_OF_SUBFRAMES_PER_FRAME 10
#define LTE_CE_FILTER_LENGTH 5
#define LTE_CE_OFFSET LTE_CE_FILTER_LENGTH
#define TX_RX_SWITCH_SYMBOL (NUMBER_OF_SYMBOLS_PER_FRAME>>1) 
#define PBCH_PDU_SIZE 3 //bytes
#define TIMING_ADVANCE_INIT 0

#define PRACH_SYMBOL 3 //position of the UL PSS wrt 2nd slot of special subframe

#define NUMBER_OF_FREQUENCY_GROUPS (lte_frame_parms->N_RB_DL)

typedef enum {
  normal=0,
  extended
} PHICH_DURATION_t;

typedef enum {
  oneSixth=1,
  half=3,
  one=6,
  two=12
} PHICH_RESOURCE_t;

typedef struct {
  /// phich Duration, see 36.211 (Table 6.9.3-1)
  PHICH_DURATION_t phich_duration;
  /// phich_resource, see 36.211 (6.9)
  PHICH_RESOURCE_t phich_resource;
} PHICH_CONFIG;

typedef struct {
  /// Config Index
  u8 prach_ConfigIndex;
  /// High Speed Flag (0,1)
  u8 highSpeedFlag;
  /// Zero correlation zone
  u8 zeroCorrelationZoneConfig;
  /// Frequency offset
  u8 prach_FreqOffset;
} PRACH_CONFIG_INFO;

typedef struct {
  ///Root Sequence Index (0...837)
  u16 RootSequenceIndex;
  ///PRACH Configuration Information
  PRACH_CONFIG_INFO prach_ConfigInfo;
} PRACH_CONFIG;

typedef enum {
  n2=0,
  n4,
  n6
} ACKNAKREP_t;

typedef enum {
  bundling=0,
  multiplexing
} ANFBmode_t;

/// PUCCH-ConfigCommon Structure from 36.331 RRC spec
typedef struct {
  /// Flag to indicate ACK NAK repetition activation, see 36.213 (10.1)
  u8 ackNackRepetition;
  /// NANRep, see 36.213 (10.1)
  ACKNAKREP_t repetitionFactor;
  /// n1PUCCH-AN-Rep, see 36.213 (10.1)
  u16 n1PUCCH_AN_Rep;
  /// Feedback mode, see 36.213 (7.3).  Applied to both PUCCH and PUSCH feedback.  For TDD, should always be set to bundling.
  ANFBmode_t tdd_AckNackFeedbackMode;
} PUCCH_CONFIG_DEDICATED;

/// PUCCH-ConfigCommon from 36.331 RRC spec
typedef struct {
  /// Parameter rom 36.211, 5.4.1, values 1,2,3
  u8 deltaPUCCH_Shift;
  /// NRB2 from 36.211, 5.4
  u8 nRB_CQI;
  /// NCS1 from 36.211, 5.4
  u8 nCS_AN;
  /// N1PUCCH from 36.213, 10.1
  u16 n1PUCCH_AN;
} PUCCH_CONFIG_COMMON;

/// UL-ReferenceSignalsPUSCH from 36.311 RRC spec
typedef struct {
  /// See 36.211 (5.5.1.3) (0,1)
  u8 groupHoppingEnabled;
  ///deltaSS see 36.211 (5.5.1.3)
  u8 groupAssignmentPUSCH;
  /// See 36.211 (5.5.1.4) (0,1)
  u8 sequenceHoppingEnabled;
  /// cyclicShift from 36.211 (see Table 5.5.2.1.1-2) (0...7)
  u8 cyclicShift;
} UL_REFERENCE_SIGNALS_PUSCH_t;
 
typedef enum {
  interSubFrame=0, 
  intraAndInterSubFrame
} PUSCH_HOPPING_t;

/// PUSCH-ConfigCommon from 36.331 RRC spec
typedef struct {
  /// Nsb from 36.211 (5.3.4)
  u8 n_SB;
  /// Hopping mode, see 36.211 (5.3.4)
  PUSCH_HOPPING_t hoppingMode;
  /// NRBHO from 36.211 (5.3.4)
  u8 pusch_HoppingOffset;
  /// 1 indicates 64QAM is allowed, 0 not allowed, see 36.213
  u8 enable64QAM;
  /// Ref signals configuration
  UL_REFERENCE_SIGNALS_PUSCH_t UL_ReferenceSignalsPUSCH;
} PUSCH_CONFIG_COMMON;

typedef struct {
  /// 
  u8 betaOffset_ACK_Index;
  ///
  u8 betaOffset_RI_Index;
  /// 
  u8 betaOffset_CQI_Index;
} PUSCH_CONFIG_DEDICATED;

/// PDSCH-ConfigDedicated from 36.331 RRC spec
typedef struct {
  /// Donwlink Reference Signal EPRE (-60... 50), 36.213 (5.2)
  s8 referenceSignalPower;
  /// Parameter PB, 36.213 (Table 5.2-1)
  u8 p_b;
} PDSCH_CONFIG_DEDICATED;

typedef enum {
  dBm6=0,
  dBm477,
  dBm3,
  dBm177,
  dB0,
  dB1,
  dB2,
  dB3
} PA_t;

/// PDSCH-ConfigCommon from 36.331 RRC spec
typedef struct {
  /// Parameter PA in dB, 36.213 (5.2)
  PA_t p_a;
} PDSCH_CONFIG_COMMON;

/// SoundingRS-UL-ConfigCommon Information Element from 36.331 RRC spec
typedef struct {
  ///SRS BandwidthConfiguration \in {0,1,...,7} see 36.211 (Table 5.5.3.2-1,5.5.3.2-2,5.5.3-2.3 and 5.5.3.2-4). Actual configuration depends on UL bandwidth.
  u8 srs_BandwidthConfig;
  ///SRS Subframe configuration \in {0,...,15} see 36.211 (Table 5.5.3.3-1 FDD, Table 5.5.3.3-2 TDD)
  u8 srs_SubframeConfig;
  ///SRS Simultaneous-AN-and-SRS, see 36.213 (8.2)
  u8 ackNackSRS_SimultaneousTransmission;
  ///srsMaxUpPts \in {0,1}, see 36.211 (5.5.3.2).  If this field is 1, reconfiguration of mmax_SRS0 applies for UpPts, otherwise reconfiguration does not apply
  u8 srs_MaxUpPts;
} SOUNDINGRS_UL_CONFIG_COMMON;

/// SoundingRS-UL-ConfigDedicated Information Element from 36.331 RRC spec
typedef struct {
  ///SRS Bandwidth b \in {0,1,2,3}
  u8 srs_Bandwidth;
  ///SRS Hopping bandwidth bhop \in {0,1,2,3}
  u8 srs_HoppingBandwidth;
  ///SRS n_RRC Frequency Domain Position \in {0,1,...,23}, see 36.211 (5.5.3.2)
  u8 freqDomainPosition;
  ///SRS duration, see 36.213 (8.2), 0 corresponds to "single" and 1 to "indefinite"
  u8 duration;
  ///SRS Transmission comb kTC \in {0,1}, see 36.211 (5.5.3.2)
  u8 transmissionComb;
  ///SRS Config Index (Isrs) \ in {0,1,...,1023}, see 36.213 (8.2)
  u16 srs_ConfigIndex;
  ///cyclicShift, n_SRS \in (0,1,...,7), see 36.211 (5.5.3.1)
  u8 cyclicShift;
} SOUNDINGRS_UL_CONFIG_DEDICATED;

typedef enum {
  al0=0,
  al04,
  al05,
  al06,
  al07,
  al08,
  al09,
  al1
} PUSCH_alpha_t;

typedef enum {
  deltaFm2=0,
  deltaF0,
  deltaF1,
  deltaF2,
  deltaF3,
  deltaF5  
} deltaF_PUCCH_t;

/// UplinkPowerControlCommon Information Element from 36.331 RRC spec
typedef struct {
  /// p0-NominalPUSCH \in (-126,...24), see 36.213 (5.1.1)
  s8 p0_NominalPUSCH;
  /// alpha, See 36.213 (5.1.1.1)
  PUSCH_alpha_t alpha;
  /// p0-NominalPUCCH \in (-127,...,-96), see 36.213 (5.1.1)
  s8 p0_NominalPUCCH;
  /// deltaF-PUCCH-Format1, see 36.213 (5.1.2)
  deltaF_PUCCH_t deltaF_PUCCH_Format1;
  /// deltaF-PUCCH-Format1a, see 36.213 (5.1.2)
  deltaF_PUCCH_t deltaF_PUCCH_Format1a;
  /// deltaF-PUCCH-Format1b, see 36.213 (5.1.2)
  deltaF_PUCCH_t deltaF_PUCCH_Format1b;
  /// deltaF-PUCCH-Format2, see 36.213 (5.1.2)
  deltaF_PUCCH_t deltaF_PUCCH_Format2;
  /// deltaF-PUCCH-Format2a, see 36.213 (5.1.2)
  deltaF_PUCCH_t deltaF_PUCCH_Format2a;
  /// deltaF-PUCCH-Format2b, see 36.213 (5.1.2)
  deltaF_PUCCH_t deltaF_PUCCH_Format2b;
} UPLINK_POWER_CONTROL_DEDICATED;

typedef union {
    /// indexOfFormat3 \in (1,...,15)
    u8 indexOfFormat3;
    /// indexOfFormat3A \in (1,...,31)
    u8 indexOfFormat3A;
} TPC_INDEX_t;

typedef struct
{
  u16 rnti;
  TPC_INDEX_t tpc_Index;
} TPC_PDCCH_CONFIG;

typedef enum {
  rm12=0,
  rm20,
  rm22,
  rm30,
  rm31
} CQI_REPORTMODEAPERIODIC;

typedef enum {
  sr_n4=0,
  sr_n8,
  sr_n16,
  sr_n32,
  sr_n64
} DSR_TRANSMAX_t;
typedef struct {
  u16 sr_PUCCH_ResourceIndex;
  u8 sr_ConfigIndex;
  DSR_TRANSMAX_t dsr_TransMax;
} SCHEDULING_REQUEST_CONFIG;

typedef struct {
  /// Parameter n2pucch, see 36.213 (7.2)
  u16 cqi_PUCCH_ResourceIndex;
  /// Parameter Icqi/pmi, see 36.213 (tables 7.2.2-1A and 7.2.2-1C)
  u16 cqi_PMI_ConfigIndex;
  /// Parameter K from 36.213 (4.2.2)
  u8 K;
  /// Parameter IRI, 36.213 (7.2.2-1B)
  u16 ri_ConfigIndex;
  /// Parameter simultaneousAckNackAndCQI
  u8 simultaneousAckNackAndCQI;
} CQI_REPORTPERIODIC;

 
typedef struct {
  CQI_REPORTMODEAPERIODIC cqi_ReportModeAperiodic;
  s8 nomPDSCH_RS_EPRE_Offset;
  CQI_REPORTPERIODIC CQI_ReportPeriodic;
} CQI_REPORT_CONFIG;

typedef struct {
  /// Number of resource blocks (RB) in DL
  u8 N_RB_DL;                
  /// Number of resource blocks (RB) in UL
  u8 N_RB_UL;
  /// Cell ID                 
  u8 Nid_cell;               
  /// Cyclic Prefix (0=Normal CP, 1=Extended CP)
  u8 Ncp;                   
  /// shift of pilot position in one RB
  u8 nushift;                
/// Frame type (0 FDD, 1 TDD)
  u8 frame_type;
  /// TDD Configuration Number (0-9) (default = 3)             
  u8 tdd_config;
  /// Frequency index of CBMIMO1 card
  u8 freq_idx;               
  /// Turns on second TX of CBMIMO1 card
  u8 dual_tx;                
/// flag to indicate SISO transmission
  u8 mode1_flag;           
/// Size of FFT  
  u16 ofdm_symbol_size;
/// log2(Size of FFT)  
  u8 log2_symbol_size;
  /// Number of prefix samples in all but first symbol of slot
  u16 nb_prefix_samples;
  /// Number of prefix samples in first symbol of slot
  u16 nb_prefix_samples0;
  /// Carrier offset in FFT buffer for first RE in PRB0
  u16 first_carrier_offset;
  /// Number of samples in a subframe
  u32 samples_per_tti;
  /// Number of OFDM/SC-FDMA symbols in one subframe (to be modified to account for potential different in UL/DL)
  u16 symbols_per_tti;
  /// Number of Transmit antennas in node
  u8 nb_antennas_tx;
  /// Number of Receive antennas in node
  u8 nb_antennas_rx;
  /// Pointer to twiddle factors for FFT
  s16 *twiddle_fft;
  ///pointer to twiddle factors for IFFT
  s16 *twiddle_ifft;                 
  ///pointer to FFT permutation vector
  u16 *rev;
  /// PUCCH Config Common (from 36-331 RRC spec)
  PUCCH_CONFIG_COMMON pucch_config_common;
  /// PDSCH Config Common (from 36-331 RRC spec)
  PDSCH_CONFIG_COMMON pdsch_config_common;
  /// PHICH Config (from 36-331 RRC spec)
  PHICH_CONFIG phich_config_common;
  /// SRS Config (from 36-331 RRC spec)
  SOUNDINGRS_UL_CONFIG_COMMON soundingrs_ul_config_common;
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
  s32 **txdata[3];           ///holds the transmit data in time domain (#ifdef IFFT_FPGA this points to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  mod_sym_t **txdataF[3];    ///holds the transmit data in the frequency domain (#ifdef IFFT_FPGA this points to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  s32 **rxdata[3];           ///holds the received data in time domain (should point to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  s32 **rxdataF[3];          ///holds the received data in the frequency domain
  u32 *sync_corr[3];         /// holds output of the sync correlator
} LTE_eNB_COMMON;

typedef struct{
  s32 **srs_ch_estimates[3];   /// hold the channel estimates in frequency domain based on SRS
  s32 **srs_ch_estimates_time[3];    /// hold the channel estimates in time domain based on SRS
  s32 *srs;               /// holds the SRS for channel estimation at the RX
} LTE_eNB_SRS;

typedef struct{
  ///holds the received data in the frequency domain for the allocated RBs
  s32 **rxdataF_ext[3];       
  s32 **rxdataF_ext2[3];       
  /// hold the channel estimates in frequency domain based on DRS   
  s32 **drs_ch_estimates[3]; 
  s32 **drs_ch_estimates_0[3];// hold the channel estimates for UE0 in case of Distributed Alamouti Scheme
  s32 **drs_ch_estimates_1[3];// hold the channel estimates for UE1 in case of Distributed Almouti Scheme 
  s32 **rxdataF_comp[3];
  s32 **rxdataF_comp_0[3];// hold the compensated data (y)*(h0*) in case of Distributed Alamouti Scheme
  s32 **rxdataF_comp_1[3];// hold the compensated data (y*)*(h1) in case of Distributed Alamouti Scheme
  s32 **ul_ch_mag[3];
  s32 **ul_ch_magb[3];
  s32 **ul_ch_mag_0[3];   // hold the channel mag for UE0 in case of Distributed Alamouti Scheme
  s32 **ul_ch_magb_0[3];  // hold the channel magb for UE0 in case of Distributed Alamouti Scheme
  s32 **ul_ch_mag_1[3];   // hold the channel mag for UE1 in case of Distributed Alamouti Scheme
  s32 **ul_ch_magb_1[3];  // hold the channel magb for UE1 in case of Distributed Alamouti Scheme
  s16 *llr;
} LTE_eNB_ULSCH;

typedef struct {
  s32 **txdata;           ///holds the transmit data in time domain (#ifdef IFFT_FPGA this points to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  mod_sym_t **txdataF;    ///holds the transmit data in the frequency domain (#ifdef IFFT_FPGA this points to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  s32 **rxdata;           ///holds the received data in time domain (should point to the same memory as PHY_vars->rx_vars[a].RX_DMA_BUFFER)
  s32 **rxdataF;          ///holds the received data in the frequency domain
  s32 **dl_ch_estimates[3];  /// hold the channel estimates in frequency domain
  s32 **dl_ch_estimates_time;  /// hold the channel estimates in time domain (used for tracking)
  s32 *sync_corr;         /// holds output of the sync correlator
  s32 freq_offset;          /// estimated frequency offset (in radians) for all subcarriers
  u8 eNb_id;     /// eNb_id user is synched to
} LTE_UE_COMMON;

typedef struct {
  s32 **rxdataF_ext;
  s32 **rxdataF_comp;
  s32 **dl_ch_estimates_ext;
  s32 **dl_ch_rho_ext;
  u8 *pmi_ext;
  s32 **dl_ch_mag;
  s32 **dl_ch_magb;
  s32 **rho;
  s32 **rho_i;  //never used... always send dl_ch_rho_ext instead...
  s16 *llr[2];
  u8 log2_maxh;
  s16 **llr128;  // to be type casted locally
  //u32 *rb_alloc;
  //u8 Qm[2];
  //MIMO_mode_t mimo_mode;
} LTE_UE_DLSCH;

typedef struct {
  s32 **rxdataF_ext;
  s32 **rxdataF_comp;
  s32 **dl_ch_estimates_ext;
  s32 **dl_ch_rho_ext;
  s32 **rho;
  u16 *llr;
  u16 *llr16;
  u16 *wbar;
  s8 *e_rx;
  u8 num_pdcch_symbols;
  u16 crnti;
  u32 dci_errors;          /// Total number of PDU errors (diagnostic mode)
  u32 dci_received;        /// Total number of PDU received
  u32 dci_false;           /// Total number of DCI False detection (diagnostic mode)
  u32 dci_missed;          /// Total number of DCI missed (diagnostic mode)
} LTE_UE_PDCCH;

typedef struct {
  s32 **rxdataF_ext;
  s32 **rxdataF_comp;
  s32 **dl_ch_estimates_ext;
  s8 *llr;
  s16 *channel_output;
  u8 *decoded_output;
  u32 pdu_errors;          /// Total number of PDU errors
  u32 pdu_errors_last;     /// Total number of PDU errors 128 frames ago
  u32 pdu_errors_conseq;   /// Total number of consecutive PDU errors
  u32 pdu_fer;             /// FER (in percent) 
} LTE_UE_PBCH;

typedef enum {
  NOT_SYNCHED=0,
  PRACH,
  RA_RESPONSE,
  PUSCH
} UE_MODE_t;


#endif


