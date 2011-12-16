/*________________________phy/impl_defs.h________________________

 Authors : Raymond Knopp, Florian Kaltenberger
 Company : EURECOM
 Emails  : kaltenbe@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


#ifndef __PHY_IMPLEMENTATION_DEFS_H__
#define __PHY_IMPLEMENTATION_DEFS_H__

/** @defgroup _ref_implementation_ OpenAirInterface LTE Implementation 
 * @{
 * @defgroup _physical_layer_ref_implementation_ Physical Layer Reference Implementation
 * @ingroup _ref_implementation_  
 * @{


 * @defgroup _PHY_STRUCTURES_ Basic Structures and Memory Initialization
 * @ingroup _physical_layer_ref_implementation_
 * @{
 * This module is responsible for defining and initializing the PHY variables during static configuration of OpenAirInterface.
 * @}

 * @defgroup _PHY_DSP_TOOLS_ DSP Tools
 * @ingroup _physical_layer_ref_implementation_
 * @{
 * This module is responsible for basic signal processing related to inner-MODEM processing.
 * @}

 * @defgroup _PHY_MODULATION_ Modulation and Demodulation
 * @ingroup _physical_layer_ref_implementation_
 * @{
 * This module is responsible for procedures related to OFDMA modulation and demodulation.
 * @}

 * @defgroup _PHY_PARAMETER_ESTIMATION_BLOCKS_ Parameter Estimation
 * @ingroup _physical_layer_ref_implementation_
 * @{
 * This module is responsible for procedures related to OFDMA frequency-domain channel estimation for LTE Downlink Channels.
 * @}

 * @defgroup _PHY_CODING_BLOCKS_ Channel Coding/Decoding Functions
 * @ingroup _physical_layer_ref_implementation_
 * @{
 * This module is responsible for procedures related to channel coding/decoding, rate-matching, segementation and interleaving.
 * @}

 * @defgroup _PHY_TRANSPORT_ Transport/Physical Channel Processing
 * @ingroup _physical_layer_ref_implementation_
 * @{
 * This module is responsible for defining and processing the PHY procedures (TX/RX) related to transport and physical channels.
 * @}
 * @}

 * @defgroup _PHY_PROCEDURES_ Physical Layer Procedures
 * @ingroup _ref_implementation_
 * @{
 * This module is responsible for defining and processing the PHY procedures (TX/RX) related to transport and physical channels.
 * @}

 */

#include "types.h"
#include "spec_defs_top.h"



/**@addtogroup _PHY_STRUCTURES_
 * @{ 
*/
#define NUMBER_OF_OFDM_CARRIERS (frame_parms->ofdm_symbol_size)
#define NUMBER_OF_SYMBOLS_PER_FRAME (frame_parms->symbols_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME)
#define LOG2_NUMBER_OF_OFDM_CARRIERS (frame_parms->log2_symbol_size)
#define NUMBER_OF_USEFUL_CARRIERS (12*frame_parms->N_RB_DL)
#define NUMBER_OF_ZERO_CARRIERS (NUMBER_OF_OFDM_CARRIERS-NUMBER_OF_USEFUL_CARRIERS)
#define NUMBER_OF_USEFUL_CARRIERS_BYTES (NUMBER_OF_USEFUL_CARRIERS>>2)
#define HALF_NUMBER_OF_USEFUL_CARRIERS (NUMBER_OF_USEFUL_CARRIERS>>1)
#define HALF_NUMBER_OF_USEFUL_CARRIERS_BYTES (HALF_NUMBER_OF_USEFUL_CARRIERS>>2)
#define FIRST_CARRIER_OFFSET (HALF_NUMBER_OF_USEFUL_CARRIERS+NUMBER_OF_ZERO_CARRIERS)
#ifdef OPENAIR_LTE
#define NUMBER_OF_OFDM_SYMBOLS_PER_SLOT (NUMBER_OF_SYMBOLS_PER_FRAME/20)
#else
#define NUMBER_OF_OFDM_SYMBOLS_PER_SLOT 16
#endif
#define SLOTS_PER_FRAME  (NUMBER_OF_SYMBOLS_PER_FRAME/NUMBER_OF_OFDM_SYMBOLS_PER_SLOT)

#ifdef EMOS
#define EMOS_SCH_INDEX 1
#endif //EMOS

#define EXTENSION_TYPE (PHY_config->PHY_framing.Extension_type)

#define NUMBER_OF_OFDM_CARRIERS_BYTES   NUMBER_OF_OFDM_CARRIERS*4
//#define NUMBER_OF_USEFUL_CARRIERS_BYTES NUMBER_OF_USEFUL_CARRIERS*4
#define HALF_NUMBER_OF_USER_CARRIERS_BYTES NUMBER_OF_USEFUL_CARRIERS/2

#define CYCLIC_PREFIX_LENGTH (frame_parms->nb_prefix_samples)
#define CYCLIC_PREFIX_LENGTH_SAMPLES (CYCLIC_PREFIX_LENGTH*2)
#define CYCLIC_PREFIX_LENGTH_BYTES (CYCLIC_PREFIX_LENGTH*4)
#define CYCLIC_PREFIX_LENGTH0 (frame_parms->nb_prefix_samples0)
#define CYCLIC_PREFIX_LENGTH_SAMPLES0 (CYCLIC_PREFIX_LENGTH0*2)
#define CYCLIC_PREFIX_LENGTH_BYTES0 (CYCLIC_PREFIX_LENGTH0*4)

#define OFDM_SYMBOL_SIZE_SAMPLES ((NUMBER_OF_OFDM_CARRIERS + CYCLIC_PREFIX_LENGTH)*2)   // 16-bit units (i.e. real samples)
#define OFDM_SYMBOL_SIZE_SAMPLES0 ((NUMBER_OF_OFDM_CARRIERS + CYCLIC_PREFIX_LENGTH0)*2)   // 16-bit units (i.e. real samples)
#define OFDM_SYMBOL_SIZE_SAMPLES_MAX 4096   // 16-bit units (i.e. real samples)
#define OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES (OFDM_SYMBOL_SIZE_SAMPLES/2)                   // 32-bit units (i.e. complex samples)
#define OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES0 (OFDM_SYMBOL_SIZE_SAMPLES0/2)                   // 32-bit units (i.e. complex samples)
#define OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX ((NUMBER_OF_OFDM_CARRIERS)*2)
#define OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX (OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX/2)
#define OFDM_SYMBOL_SIZE_BYTES (OFDM_SYMBOL_SIZE_SAMPLES*2)
#define OFDM_SYMBOL_SIZE_BYTES0 (OFDM_SYMBOL_SIZE_SAMPLES0*2)
#define OFDM_SYMBOL_SIZE_BYTES_NO_PREFIX (OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2)

#define SLOT_LENGTH_BYTES (frame_parms->samples_per_tti<<1) // 4 bytes * samples_per_tti/2
#define SLOT_LENGTH_BYTES_NO_PREFIX (OFDM_SYMBOL_SIZE_BYTES_NO_PREFIX * NUMBER_OF_OFDM_SYMBOLS_PER_SLOT)

#define FRAME_LENGTH_COMPLEX_SAMPLES (frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME)
#define FRAME_LENGTH_SAMPLES (FRAME_LENGTH_COMPLEX_SAMPLES*2)
#define FRAME_LENGTH_SAMPLES_NO_PREFIX (NUMBER_OF_SYMBOLS_PER_FRAME*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX)
#define FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX (FRAME_LENGTH_SAMPLES_NO_PREFIX/2)

#define NUMBER_OF_CARRIERS_PER_GROUP (NUMBER_OF_USEFUL_CARRIERS/NUMBER_OF_FREQUENCY_GROUPS)

#define RX_PRECISION (16)
#define LOG2_RX_PRECISION (4)
#define RX_OUTPUT_SHIFT (4)


#define SAMPLE_SIZE_BYTES    2                                           // 2 bytes/real sample

#define FRAME_LENGTH_BYTES   (FRAME_LENGTH_SAMPLES * SAMPLE_SIZE_BYTES)  // frame size in bytes
#define FRAME_LENGTH_BYTES_NO_PREFIX   (FRAME_LENGTH_SAMPLES_NO_PREFIX * SAMPLE_SIZE_BYTES)  // frame size in bytes


#define FFT_SCALE_FACTOR     8                                           // Internal Scaling for FFT
#define DMA_BLKS_PER_SLOT    (SLOT_LENGTH_BYTES/2048)                    // Number of DMA blocks per slot
#define SLOT_TIME_NS         (SLOT_LENGTH_SAMPLES*(1e3)/7.68)            // slot time in ns


#define TARGET_RX_POWER 50		// Target digital power for the AGC
#define TARGET_RX_POWER_MAX 65		// Maximum digital power, such that signal does not saturate (value found by simulation)
#define TARGET_RX_POWER_MIN 35		// Minimum digital power, anything below will be discarded (value found by simulation)
//the min and max gains have to match the calibrated gain table
//#define MAX_RF_GAIN 160
//#define MIN_RF_GAIN 96
#define MAX_RF_GAIN 150
#define MIN_RF_GAIN 100

#define PHY_SYNCH_OFFSET ((OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)-1)  // OFFSET of BEACON SYNCH
#define PHY_SYNCH_MIN_POWER 1000
#define PHY_SYNCH_THRESHOLD 100



#define ONE_OVER_SQRT2_Q15 23170


// QAM amplitude definitions

/// First Amplitude for QAM16 (\f$ 2^{15} \times 2/\sqrt{10}\f$)
#define QAM16_n1 20724
/// Second Amplitude for QAM16 (\f$ 2^{15} \times 1/\sqrt{10}\f$)
#define QAM16_n2 10362

///First Amplitude for QAM64 (\f$ 2^{14} \times 4/\sqrt{42}\f$)
#define QAM64_n1 20225
///Second Amplitude for QAM64 (\f$ 2^{14} \times 2/\sqrt{42}\f$)
#define QAM64_n2 10112
///Third Amplitude for QAM64 (\f$ 2^{14} \times 1/\sqrt{42}\f$)
#define QAM64_n3 5056


#ifdef BIT8_RXMUX
#define PERROR_SHIFT 0
#else
#define PERROR_SHIFT 10
#endif

#define BIT8_TX_SHIFT 2
#define BIT8_TX_SHIFT_DB 12

#define CHBCH_RSSI_MIN -75

#ifdef BIT8_TX
#define AMP 128 
#else
#define AMP 1024
#endif


#ifndef OPENAIR_LTE
///
/// PHY-MAC Interface Defs 
///

/// Maximum number of parallel streams per slot
#define NB_STREAMS_MAX 4

/// Maximum number of frequency groups per slot
#define NB_GROUPS_MAX 16

/// Maximum number of control bytes per slot
#define NB_CNTL_BYTES_MAX 8

/// Maximum number of data bytes per slot
#define NB_DATA_BYTES_MAX 256

#define MAX_NUM_TB 32
#define MAX_TB_SIZE_BYTES 128

/// Size of SACCH PDU in Bytes
#define SACCH_SIZE_BYTES (sizeof(UL_SACCH_PDU)+4) 
/// Size of SACCH PDU in Bytes
#define SACCH_SIZE_BITS  (SACCH_SIZE_BYTES<<3)

#define MAX_SACH_SIZE_BYTES 1024


#define SACH_ERROR 1
#define SACCH_ERROR 2
#define SACH_MISSING 3
#define SACH_PARAM_INVALID 10

#endif //OPENAIR_LTE

/*
enum STATUS_RX {STATUS_RX_OFF,
		STATUS_RX_ON,
		STATUS_RX_SYNCING,
		STATUS_RX_CANNOT_SYNC,
		STATUS_RX_DATA_PROBLEM,
		STATUS_RX_LOST_SYNC,
		STATUS_RX_ABORT,
		STATUS_RX_TOO_LATE,
		STATUS_RX_CLOCK_STOPPED};

enum STATUS_TX {
  STATUS_TX_OFF,
  STATUS_TX_ON,
  STATUS_TX_INPUT_CORRUPT,
  STATUS_TX_ABORT,
  STATUS_TX_TOO_LATE,
  STATUS_TX_CLOCK_STOPPED};

enum MODE {
  SYNCHED,
  SYNCHING,
  NOT_SYNCHED};
*/

/// Data structure for transmission.
typedef struct {
  // RAW TX sample buffer
  mod_sym_t *TX_DMA_BUFFER[2];
  /*
  // Total transmit gain
  unsigned int tx_total_gain_dB;
  */
} TX_VARS ;  


/// Data structure for reception.
typedef struct {
  int *RX_DMA_BUFFER;
} RX_VARS;


/// Measurement Variables
#ifndef OPENAIR_LTE

typedef struct
{
  unsigned short rx_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS_RX];  //! estimated received signal power (dB)
  short          rx_avg_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX];              //! estimated avg received signal power (dB)
  unsigned short n0_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS_RX];  //! estimated noise power (dB)
  short		 rx_rssi_dBm[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS_RX];  //! estimated rssi (dBm)
  int            rx_power[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS_RX];     //! estimated received signal power (linear)
  int            rx_spatial_power[NUMBER_OF_CHSCH_SYMBOLS_MAX][2][2];// estimated received spatial signal power (linear)
  unsigned short rx_spatial_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX][2][2];// estimated received spatial signal power (linea  int            n0_power[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS_RX];     //! estimated noise power (linear)
  unsigned int   chbch_search_count;
  unsigned int   chbch_detection_count[4];
  unsigned int   mrbch_search_count; 
  unsigned int   mrbch_detection_count;
#ifdef EMOS
  //  unsigned char  Meas_flag;      	 //! This is used as a signal to start recording in multiuser mode
  unsigned int   frame_tx[2];            //! This is used to set the file index of the measurement file at the terminal
  int            crc_status[2]; 	 //! crc status of the CHBCH
#endif //EMOS
} PHY_MEASUREMENTS;

/// Physical Resource Descriptor
typedef struct {
  unsigned char  Time_alloc;      /*!<\brief Time allocation vector of DL-SACH reservation*/
  unsigned short Freq_alloc;      /*!< \brief Frequency allocation vector of DL-SACH reservation*/
  unsigned short Ifreq_alloc;     /*!< \brief Frequency allocation vector of interference (DL-SACH)*/
  unsigned char  Antenna_alloc;   /*!< \brief Antenna allocation vector of DL-SACH reservation*/ 
  unsigned char  Coding_fmt;      /*!< \brief Coding format for this PDU*/
} __attribute__((__packed__)) PHY_RESOURCES;


#else //OPENAIR_LTE

#define NUMBER_OF_eNB_MAX 3
#define NUMBER_OF_UE_MAX 8
#define NUMBER_OF_SUBBANDS 7

typedef struct
{
  //unsigned int   rx_power[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];     //! estimated received signal power (linear)
  //unsigned short rx_power_dB[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];  //! estimated received signal power (dB)
  //unsigned short rx_avg_power_dB[NUMBER_OF_eNB_MAX];              //! estimated avg received signal power (dB)

  // common measurements
  //! estimated noise power (linear)
  unsigned int   n0_power[NB_ANTENNAS_RX];                        
  //! estimated noise power (dB)
  unsigned short n0_power_dB[NB_ANTENNAS_RX];                     
  //! total estimated noise power (linear)
  unsigned int   n0_power_tot;                                    
  //! total estimated noise power (dB)
  unsigned short n0_power_tot_dB;     
  //! average estimated noise power (linear)
  unsigned short n0_power_avg;     
  //! average estimated noise power (dB)
  unsigned short n0_power_avg_dB;     
  //! total estimated noise power (dBm)                            
  short n0_power_tot_dBm;
  // UE measurements
  //! estimated received spatial signal power (linear)
  unsigned int   rx_spatial_power[NUMBER_OF_eNB_MAX][2][2];       
  //! estimated received spatial signal power (dB) 
  unsigned short rx_spatial_power_dB[NUMBER_OF_eNB_MAX][2][2];    
  //! estimated received signal power (sum of all TX/RX antennas)
  int            rx_power_avg[NUMBER_OF_eNB_MAX];                                 
  //! estimated received signal power (sum of all TX/RX antennas, in dB)
  int            rx_power_avg_dB[NUMBER_OF_eNB_MAX];                                 
  //! estimated rssi (dBm)
  short          rx_rssi_dBm[NUMBER_OF_eNB_MAX];                  
  //! estimated correlation (wideband linear) between spatial channels (computed in dlsch_demodulation)
  int            rx_correlation[NUMBER_OF_eNB_MAX][2];            
  //! estimated correlation (wideband dB) between spatial channels (computed in dlsch_demodulation)
  int            rx_correlation_dB[NUMBER_OF_eNB_MAX][2];         

  /// Wideband CQI (= SINR)
  int            wideband_cqi[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];                     
  /// Wideband CQI in dB (= SINR dB)
  int            wideband_cqi_dB[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];                  
  /// Wideband CQI (sum of all RX antennas, in dB)
  int            wideband_cqi_tot[NUMBER_OF_eNB_MAX];                                 
  /// Wideband CQI (average, in dB)
  int            wideband_cqi_avg[NUMBER_OF_eNB_MAX];                                 
  /// Wideband CQI (sum of all RX antennas, in dB, for precoded transmission modes (4,5,6), up to 4 spatial streams)
  int            precoded_cqi_dB[NUMBER_OF_eNB_MAX+1][4];                               
  /// Subband CQI per RX antenna (= SINR)
  int            subband_cqi[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX][NUMBER_OF_SUBBANDS];  
  /// Total Subband CQI  (= SINR)
  int            subband_cqi_tot[NUMBER_OF_eNB_MAX][NUMBER_OF_SUBBANDS];              
  /// Subband CQI in dB (= SINR dB)
  int            subband_cqi_dB[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX][NUMBER_OF_SUBBANDS];
  /// Total Subband CQI   
  int            subband_cqi_tot_dB[NUMBER_OF_eNB_MAX][NUMBER_OF_SUBBANDS];           
  /// Wideband PMI for each RX antenna
  int            wideband_pmi_re[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];                  
  /// Wideband PMI for each RX antenna
  int            wideband_pmi_im[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];                  
  ///Subband PMI for each RX antenna
  int            subband_pmi_re[NUMBER_OF_eNB_MAX][NUMBER_OF_SUBBANDS][NB_ANTENNAS_RX]; 
  ///Subband PMI for each RX antenna
  int            subband_pmi_im[NUMBER_OF_eNB_MAX][NUMBER_OF_SUBBANDS][NB_ANTENNAS_RX];
  /// chosen RX antennas (1=Rx antenna 1, 2=Rx antenna 2, 3=both Rx antennas) 
  unsigned char           selected_rx_antennas[NUMBER_OF_eNB_MAX][NUMBER_OF_SUBBANDS];         
  /// Wideband Rank indication
  unsigned char  rank[NUMBER_OF_eNB_MAX];                                             
  /// DLSCH error counter
  // short          dlsch_errors;                                                        
} PHY_MEASUREMENTS;

typedef struct
{
  //unsigned int   rx_power[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];     //! estimated received signal power (linear)
  //unsigned short rx_power_dB[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];  //! estimated received signal power (dB)
  //unsigned short rx_avg_power_dB[NUMBER_OF_eNB_MAX];              //! estimated avg received signal power (dB)

  // common measurements
  //! estimated noise power (linear)
  unsigned int   n0_power[NB_ANTENNAS_RX];                        
  //! estimated noise power (dB)
  unsigned short n0_power_dB[NB_ANTENNAS_RX];                     
  //! total estimated noise power (linear)
  unsigned int   n0_power_tot;                                    
  //! estimated avg noise power (dB)
  unsigned short n0_power_tot_dB;                                 
  //! estimated avg noise power (dB)
  short n0_power_tot_dBm;                                         
  //! estimated avg noise power per RB per RX ant (lin)
  unsigned short n0_subband_power[NB_ANTENNAS_RX][25];            
  //! estimated avg noise power per RB per RX ant (dB)
  unsigned short n0_subband_power_dB[NB_ANTENNAS_RX][25];        
  //! estimated avg noise power per RB (dB)         
  short n0_subband_power_tot_dB[25];                             
  //! estimated avg noise power per RB (dBm)
  short n0_subband_power_tot_dBm[25];                            
  // eNB measurements (per user)
  //! estimated received spatial signal power (linear)
  unsigned int   rx_spatial_power[NUMBER_OF_UE_MAX][2][2];       
  //! estimated received spatial signal power (dB) 
  unsigned short rx_spatial_power_dB[NUMBER_OF_UE_MAX][2][2];    
  //! estimated rssi (dBm)
  short          rx_rssi_dBm[NUMBER_OF_UE_MAX];                  
  //! estimated correlation (wideband linear) between spatial channels (computed in dlsch_demodulation)
  int            rx_correlation[NUMBER_OF_UE_MAX][2];            
  //! estimated correlation (wideband dB) between spatial channels (computed in dlsch_demodulation)
  int            rx_correlation_dB[NUMBER_OF_UE_MAX][2];         

  /// Wideband CQI (= SINR)
  int            wideband_cqi[NUMBER_OF_UE_MAX][NB_ANTENNAS_RX];                     
  /// Wideband CQI in dB (= SINR dB)
  int            wideband_cqi_dB[NUMBER_OF_UE_MAX][NB_ANTENNAS_RX];                  
  /// Wideband CQI (sum of all RX antennas, in dB)
  char           wideband_cqi_tot[NUMBER_OF_UE_MAX];                 
  /// Subband CQI per RX antenna and RB (= SINR)                
  int            subband_cqi[NUMBER_OF_UE_MAX][NB_ANTENNAS_RX][25];  
  /// Total Subband CQI and RB (= SINR)
  int            subband_cqi_tot[NUMBER_OF_UE_MAX][25];              
  /// Subband CQI in dB and RB (= SINR dB)
  int            subband_cqi_dB[NUMBER_OF_UE_MAX][NB_ANTENNAS_RX][25];
  /// Total Subband CQI and RB  
  int            subband_cqi_tot_dB[NUMBER_OF_UE_MAX][25];           

} PHY_MEASUREMENTS_eNB;
#endif //OPENAIR_LTE

#ifndef OPENAIR_LTE
/// Physical Resource Descriptor
typedef struct {
  unsigned char  Time_alloc;      /*!<\brief Time allocation vector of DL-SACH reservation*/
  unsigned int   Freq_alloc[2];   /*!< \brief Frequency allocation vector of DL-SACH reservation*/
  unsigned short Ifreq_alloc;     /*!< \brief Frequency allocation vector of interference (DL-SACH)*/
  unsigned char  Antenna_alloc;   /*!< \brief Antenna allocation vector of DL-SACH reservation*/ 
  unsigned char  Coding_fmt;      /*!< \brief Coding format for this PDU*/
} __attribute__((__packed__)) PHY_RESOURCES;


#define PHY_RESOURCES_SIZE sizeof(PHY_RESOURCES)

/// Static Configuration Structure
typedef struct {
  PHY_FRAMING         PHY_framing;       /*!<\brief TTI Configuration*/
  PHY_CHBCH           PHY_chbch[8];      /*!<\brief CHBCH Configuration*/
  PHY_MRBCH           PHY_mrbch;         /*!<\brief MRBCH Configuration*/
  PHY_CHSCH           PHY_chsch[8];      /*!<\brief CHSCH Configuration (up to 8)*/
  PHY_SCH             PHY_sch[8];        /*!<\brief SCH Configuration (up to 8)*/
  PHY_SACH            PHY_sach;          /*!<\brief SACH configuration*/
  int                 total_no_chsch;    /*!<\brief Number of CHSCH*/
  int                 total_no_chbch;    /*!<\brief Number of CHBCH*/
  int                 total_no_sch;      /*!<\brief Number of SCH*/
  int                 dual_tx;           /// 1 for two tx antennas, 0 for 1 tx antenna
  int                 tdd;		 /// 1 for TDD, 0 for FDD
} PHY_CONFIG;


/// Top-level PHY Data Structure  
typedef struct
{
  /*
  /// ACQ Mailbox for harware synch
  unsigned int *mbox;                
  /// Total RX gain
  unsigned int rx_total_gain_dB;
  /// Total RX gain
  unsigned int rx_total_gain_eNB_dB;
  /// Timing offset (UE)
  int rx_offset;
  /// TX/RX switch position in symbols (for TDD)
  //unsigned int tx_rx_switch_point;   --> only in openair_daq_vars
  */
  /// TX variables indexed by antenna
  TX_VARS tx_vars[NB_ANTENNAS_TX];      
  /// RX variables indexed by antenna
  RX_VARS rx_vars[NB_ANTENNAS_RX];      
  /// CHSCH variables (up to 8)
  CHSCH_data          chsch_data[8];   
  /// SCH variables (up to 8)
  SCH_data            sch_data[8];     
  /// CHBCH variables (up to 8)
  Transport_data      chbch_data[8];   
  /// MRBCH variables (up to 8)
  Transport_data      mrbch_data[8];   
  /// SACH variables (up to 8)
  Transport_data      sach_data[NUMBER_OF_SACH];  
  /// SACCH variables (up to 8)
  Transport_data      sacch_data[NUMBER_OF_SACH];
  /// Diagnostics for SACH Metering
  SACH_DIAGNOSTICS   Sach_diagnostics[NB_CNX_CH][1+NB_RAB_MAX];
} PHY_VARS;
#endif



#ifdef NOCARD_TEST
// This structure is used for the emulation mode of the MODEM to 
// pass synchronization information to a user-space process via RT-FIFOS
typedef struct{
  unsigned int frame;
  unsigned int rx_offset;
  unsigned int rx_gain;
} RF_CNTL_PACKET;
#endif //NOCARD_TEST

#endif //__PHY_IMPLEMENTATION_DEFS_H__ 
/**@}
  *@}
*/
