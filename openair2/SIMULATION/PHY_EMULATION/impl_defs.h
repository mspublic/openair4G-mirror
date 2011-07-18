/*________________________phy/impl_defs.h________________________

 Authors : Raymond Knopp, Florian Kaltenberger
 Company : EURECOM
 Emails  : kaltenberg@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/
#ifndef __PHY_IMPLEMENTATION_DEFS_H__
#define __PHY_IMPLEMENTATION_DEFS_H__

//#warning "IN MAC PHY_IMPL_DEFS.h !!!!!"
#include "types.h"
#include "spec_defs.h"


/** @defgroup _physical_layer_ref_implementation_ OpenAirInterface Physical Layer Reference Implementation
* @{
Here we describe a reference implementation in C of the OpenAirInterface System.
* @}
* @defgroup _PHY_STRUCTURES_ Basic Structures and Memory Initialization
* @ingroup _physical_layer_ref_implementation_
* @{
* This module is responsible for defining and initializing the PHY variables during static configuration of OpenAirInterface.
*/

// *** Main definitions for L1 ***
#define ZERO 0
#define ONE 1
#define FALSE 0
#define TRUE 1
#define MINUSONE (-1)
#define WORD_SIZE 32                                          // Size of a word in bit
#define NB_SUBBANDS_IN_dB 12

#define SACH_ERROR 1
#define SACCH_ERROR 2
#define SACH_MISSING 3
#define SACH_PARAM_INVALID 10

//#define NB_ANTENNAS 2

#define NUMBER_OF_SACH_DATA_CARRIERS_PER_GROUP 8

#define NUMBER_OF_OFDM_CARRIERS (PHY_config->PHY_framing.Nd)
#define NUMBER_OF_SYMBOLS_PER_FRAME (PHY_config->PHY_framing.Nsymb)
#define LOG2_NUMBER_OF_OFDM_CARRIERS (PHY_config->PHY_framing.log2Nd)
#define NUMBER_OF_ZERO_CARRIERS (PHY_config->PHY_framing.Nz)
#define NUMBER_OF_USEFUL_CARRIERS (NUMBER_OF_OFDM_CARRIERS-NUMBER_OF_ZERO_CARRIERS)
#define NUMBER_OF_USEFUL_CARRIERS_BYTES (NUMBER_OF_USEFUL_CARRIERS>>2)
#define HALF_NUMBER_OF_USEFUL_CARRIERS (NUMBER_OF_USEFUL_CARRIERS>>1)
#define HALF_NUMBER_OF_USEFUL_CARRIERS_BYTES (HALF_NUMBER_OF_USEFUL_CARRIERS>>2)
#define FIRST_CARRIER_OFFSET (HALF_NUMBER_OF_USEFUL_CARRIERS+NUMBER_OF_ZERO_CARRIERS)
#define NUMBER_OF_OFDM_SYMBOLS_PER_SLOT 16
#define SLOTS_PER_FRAME  (NUMBER_OF_SYMBOLS_PER_FRAME/NUMBER_OF_OFDM_SYMBOLS_PER_SLOT)

#ifdef EMOS
#define CHBCH_SIZE 140	// this is in bytes!
#endif //EMOS
#define MRBCH_SIZE 148  // this is in bytes!
#define MRSCH_INDEX 1

#define EXTENSION_TYPE (PHY_config->PHY_framing.Extension_type)

#define NUMBER_OF_OFDM_CARRIERS_BYTES   NUMBER_OF_OFDM_CARRIERS*4
//#define NUMBER_OF_USEFUL_CARRIERS_BYTES NUMBER_OF_USEFUL_CARRIERS*4
#define HALF_NUMBER_OF_USER_CARRIERS_BYTES NUMBER_OF_USEFUL_CARRIERS/2

#define CYCLIC_PREFIX_LENGTH (PHY_config->PHY_framing.Nc)
#define CYCLIC_PREFIX_LENGTH_SAMPLES (CYCLIC_PREFIX_LENGTH*2)
#define CYCLIC_PREFIX_LENGTH_BYTES (CYCLIC_PREFIX_LENGTH*4)

#define OFDM_SYMBOL_SIZE_SAMPLES ((NUMBER_OF_OFDM_CARRIERS + CYCLIC_PREFIX_LENGTH)*2)   // 16-bit units (i.e. real samples)
#define OFDM_SYMBOL_SIZE_SAMPLES_MAX 4096   // 16-bit units (i.e. real samples)
#define OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES (OFDM_SYMBOL_SIZE_SAMPLES/2)                   // 32-bit units (i.e. complex samples)
#define OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX ((NUMBER_OF_OFDM_CARRIERS)*2)
#define OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX (OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX/2)
#define OFDM_SYMBOL_SIZE_BYTES (OFDM_SYMBOL_SIZE_SAMPLES*2)
#define OFDM_SYMBOL_SIZE_BYTES_NO_PREFIX (OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2)

#define SLOT_LENGTH_BYTES (OFDM_SYMBOL_SIZE_BYTES * NUMBER_OF_OFDM_SYMBOLS_PER_SLOT)
#define SLOT_LENGTH_BYTES_NO_PREFIX (OFDM_SYMBOL_SIZE_BYTES_NO_PREFIX * NUMBER_OF_OFDM_SYMBOLS_PER_SLOT)

#define FRAME_LENGTH_SAMPLES (NUMBER_OF_SYMBOLS_PER_FRAME*OFDM_SYMBOL_SIZE_SAMPLES)
#define FRAME_LENGTH_COMPLEX_SAMPLES (FRAME_LENGTH_SAMPLES/2)
#define FRAME_LENGTH_SAMPLES_NO_PREFIX (NUMBER_OF_SYMBOLS_PER_FRAME*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX)
#define FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX (FRAME_LENGTH_SAMPLES_NO_PREFIX/2)

#define NB_RACH          1//(0x04)   // Number of RACH per mini-frame
#define RACH_TIME_ALLOC  (0x01)   // Length 1 (2 without guard and pilot) groups, position 0 (after TX/RX SW)
#define RACH0_FREQ_ALLOC (0xffff)   // 8 Frequency groups of 10 carriers
#define SYMBOLS_PER_TIME_ALLOC 4     //
#define TIME_ALLOC_TO_SLOT_SHIFT 6   // SLOTS are 16 symbols, time alloc are 4 symbols

#define NUMBER_OF_SACH (8)

#define NUMBER_OF_FREQUENCY_GROUPS 16
#define NUMBER_OF_RACH_FREQUENCY_GROUPS 16
#define NUMBER_OF_RACH_SYMBOLS 1

#define NUMBER_OF_CARRIERS_PER_GROUP (NUMBER_OF_USEFUL_CARRIERS/NUMBER_OF_FREQUENCY_GROUPS)
                                       
#define NUMBER_OF_CLUSTERS 2
#define NUMBER_OF_TERMINALS_PER_CLUSTER  7 // make me reconfigurable 

#define NUMBER_OF_CHSCH_SYMBOLS (PHY_config->PHY_chsch[0].Nsymb)
#define NUMBER_OF_CHSCH (4)
#define NUMBER_OF_CHSCH_SYMBOLS_MAX (4)
#define NUMBER_OF_SCH_SYMBOLS (PHY_config->PHY_sch[0].Nsymb)
#define NUMBER_OF_SCH_SYMBOLS_MAX (4)
#define NUMBER_OF_MRSCH_SYMBOLS (PHY_config->PHY_sch[1].Nsymb)
#define NUMBER_OF_CHBCH_SYMBOLS (PHY_config->PHY_chbch.Nsymb)
#define NUMBER_OF_CHBCH_PILOTS  (PHY_config->PHY_chbch.Npilot)
#define NUMBER_OF_MRBCH_SYMBOLS (PHY_config->PHY_mrbch.Nsymb)
#define NUMBER_OF_MRBCH_PILOTS  (PHY_config->PHY_mrbch.Npilot)
#define CHBCH_FREQUENCY_REUSE_FACTOR (PHY_config->PHY_chbch.FreqReuse)
#define CHBCH_FREQUENCY_REUSE_IND (PHY_config->PHY_chbch.FreqReuse_Ind)
#define LOG2_NUMBER_OF_CHBCH_PILOTS 3   // in config later!!!
#define LOG2_NUMBER_OF_MRBCH_PILOTS 3   // in config later!!!
#define INTDEPTH_CHBCH          (PHY_config->PHY_chbch.IntDepth)
#define CHBCH_PILOT_SPACING     (NUMBER_OF_USEFUL_CARRIERS/NUMBER_OF_CHBCH_PILOTS/CHBCH_FREQUENCY_REUSE_FACTOR)
#define FIRST_CHBCH_PILOT_OFFSET (FIRST_CARRIER_OFFSET + (CHBCH_PILOT_SPACING>>1))

#define SYMBOL_OFFSET_CHBCH           (PHY_config->PHY_chbch.symbol)
#define SYMBOL_OFFSET_CHSCH           (PHY_config->PHY_chsch[0].symbol)
#define SYMBOL_OFFSET_MRBCH           (PHY_vars->tx_rx_switch_point+7)
//#define SYMBOL_OFFSET_SCH             (PHY_vars->tx_rx_switch_point+4)
#define SYMBOL_OFFSET_MRSCH           (PHY_vars->tx_rx_switch_point+6)
#define SYMBOL_OFFSET_RACH_SCH        (NUMBER_OF_SYMBOLS_PER_FRAME-SYMBOLS_PER_TIME_ALLOC-1)

#define SAMPLE_OFFSET_CHBCH           (SYMBOL_OFFSET_CHBCH*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)
#define SAMPLE_OFFSET_CHSCH           (SYMBOL_OFFSET_CHSCH*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)
//#define SAMPLE_OFFSET_SCH             (SYMBOL_OFFSET_SCH*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)
#define SAMPLE_OFFSET_MRBCH           (SYMBOL_OFFSET_MRBCH*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)

#define SAMPLE_OFFSET_CHBCH_NO_PREFIX (SYMBOL_OFFSET_CHBCH*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)
#define SAMPLE_OFFSET_CHSCH_NO_PREFIX (SYMBOL_OFFSET_CHSCH*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)
//#define SAMPLE_OFFSET_SCH_NO_PREFIX   (SYMBOL_OFFSET_SCH*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)
#define SAMPLE_OFFSET_MRBCH_NO_PREFIX (SYMBOL_OFFSET_MRBCH*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)

 
// fix me in case of multiclustering

#define NUMBER_OF_SACH_SYMBOLS_MAX (NUMBER_OF_SYMBOLS_PER_FRAME)
#define NUMBER_OF_SACH_PILOTS (PHY_config->PHY_sach.Npilot)
#define LOG2_NUMBER_OF_SACH_PILOTS 3   // in config later!!!
#define SACH_PILOT_SPACING     (1+(NUMBER_OF_USEFUL_CARRIERS/NUMBER_OF_SACH_PILOTS))
#define FIRST_SACH_PILOT_OFFSET (FIRST_CARRIER_OFFSET + (SACH_PILOT_SPACING>>1))

#define NUMBER_OF_GUARD_RACH_SYMBOLS 4
#define NUMBER_OF_GUARD_SYMBOLS_END 4
#define NUMBER_OF_UL_CONTROL_SYMBOLS (NUMBER_OF_GUARD_RACH_SYMBOLS+NUMBER_OF_MRBCH_SYMBOLS+NUMBER_OF_MRSCH_SYMBOLS)
#define NUMBER_OF_DL_SACH_SYMBOLS (TX_RX_SWITCH_SYMBOL-(NUMBER_OF_CHSCH_SYMBOLS*NUMBER_OF_CHSCH)-NUMBER_OF_CHBCH_SYMBOLS)
#define NUMBER_OF_UL_SACH_SYMBOLS (NUMBER_OF_SYMBOLS_PER_FRAME-TX_RX_SWITCH_SYMBOL-NUMBER_OF_UL_CONTROL_SYMBOLS-NUMBER_OF_GUARD_SYMBOLS_END)

#define FIRST_DL_SACH_SYMBOL (NUMBER_OF_CHBCH_SYMBOLS+(NUMBER_OF_CHSCH_SYMBOLS*NUMBER_OF_CHSCH))
#define FIRST_UL_SACH_SYMBOL (NUMBER_OF_UL_CONTROL_SYMBOLS+TX_RX_SWITCH_SYMBOL)

#define TX_RX_SWITCH_SYMBOL (NUMBER_OF_SYMBOLS_PER_FRAME>>1)
// end navid 
 
#define RX_PRECISION (16)
#define LOG2_RX_PRECISION (4)
#define RX_OUTPUT_SHIFT (4)


#define SAMPLE_SIZE_BYTES    2                                           // 2 bytes/real sample

#define FRAME_LENGTH_BYTES   (FRAME_LENGTH_SAMPLES * SAMPLE_SIZE_BYTES)  // frame size in bytes


#define FFT_SCALE_FACTOR     8                                           // Internal Scaling for FFT
#define DMA_BLKS_PER_SLOT    (SLOT_LENGTH_BYTES/2048)                    // Number of DMA blocks per slot
#define SLOT_TIME_NS         (SLOT_LENGTH_SAMPLES*(1e3)/7.68)            // slot time in ns


//#define GAIN_QPSK 1024              // Amplitude of QPSK transmit modulation
//#define GAIN_QPSK_2ANT 724         // Amplitude of QPSK transmit modulation with 2 TX antennas (GAIN_QPSK/sqrt(2))
//#define GAIN_16QAM 458             // Amplitude of 16QAM transmit modulation (GAIN_QPSK/sqrt(5))
//#define GAIN_16QAM_2ANT 324        // Amplitude of 16QAM transmit modulation with 2 TX antennas (GAIN_16QAM/sqrt(2))    

#define TARGET_RX_POWER 44		// Target digital power for the AGC
#define MAX_RF_GAIN 150
#define MIN_RF_GAIN 96

#define PHY_SYNCH_OFFSET ((OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)-1)  // OFFSET of BEACON SYNCH
#define PHY_SYNCH_MIN_POWER 1000
#define PHY_SYNCH_THRESHOLD 100

#define FIRST_SACH_SLOT 8

#define MAX_CHBCH_ERRORS 100

#define CHSCH_AMP 91// 128/sqrt(2)
#define SCH_AMP   91// 128/sqrt(2)
#define MRSCH_AMP   91// 128/sqrt(2)
#define LOG2_CHSCH_RX_F_AMP 7
#define LOG2_SCH_RX_F_AMP   7

//#define CHSCH_AMP 512
//#define SCH_AMP   512
//#define LOG2_CHSCH_RX_F_AMP 10
//#define LOG2_SCH_RX_F_AMP   10

#define PLATON_TX_SHIFT 4

#ifdef BIT8_RXMUX
#define PERROR_SHIFT 0
#else
#define PERROR_SHIFT 8
#endif
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


// QAM amplitude definitions

/// First Amplitude for QAM16
#define QAM16_n1 14654   
/// Second Amplitude for QAM16
#define QAM16_n2 7327    

/// First Amplitude for QAM16 scaled by \f$1/\sqrt{2}\f$
#define QAM16_n1_RX 10362

///First Amplitude for QAM64
#define QAM64_n1 23984
///Second Amplitude for QAM64
#define QAM64_n2 14390
///Third Amplitude for QAM64
#define QAM64_n3 4797

/// Size of SACCH PDU in Bytes
#define SACCH_SIZE_BYTES (sizeof(UL_SACCH_PDU)+4)//(NUMBER_OF_UL_SACH_SYMBOLS * (NUMBER_OF_CARRIERS_PER_GROUP - (NUMBER_OF_SACH_PILOTS/NUMBER_OF_FREQUENCY_GROUPS)))
/// Size of SACCH PDU in Bytes
#define SACCH_SIZE_BITS  (SACCH_SIZE_BYTES<<3)


/// Physical Resource Descriptor
typedef struct {
  unsigned char  Time_alloc;           /*!<\brief Time allocation vector of DL-SACH reservation*/
  unsigned short Freq_alloc;      /*!< \brief Frequency allocation vector of DL-SACH reservation*/
  unsigned char  Antenna_alloc;   /*!< \brief Antenna allocation vector of DL-SACH reservation*/ 
 unsigned char  Coding_fmt;          /*!< \brief Coding format for this PDU*/
  unsigned short Ifreq_alloc;
} __attribute__((__packed__)) PHY_RESOURCES;




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

/// SCH Channel Enumerator
typedef enum {
  CHSCH,
  SCH
} SCH_t;

#ifndef PHY_EMUL
/// Data structure for transmission.
typedef struct {
/** RAW TX sample buffer */
  int *TX_DMA_BUFFER;
/** Total transmit gain */           
  unsigned int tx_total_gain_dB;
  /** Dummy overlap for multi-antenna operation */                                                            
  unsigned int dummy2[20];
  unsigned char TX_STATUS;
} TX_VARS ;  


/// Data structure for reception.
typedef struct {
  int *RX_DMA_BUFFER;
  int offset;
  unsigned int rx_total_gain_dB;
} RX_VARS;

/// Measurement Variables
typedef struct
{


  unsigned short rx_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS];  // estimated received signal power (dB)
  short rx_avg_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX];               // estimated avg received signal power (dB)
  unsigned short n0_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS];  // estimated noise power (dB)
  short		 rx_rssi_dBm[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS];  // estimated rssi (dBm)
  int            rx_power[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS];// estimated received signal power (linear)
  int            n0_power[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS];// estimated noise power (linear)
#ifdef EMOS
  unsigned char  Meas_flag;      	// set to 1 to enable
  // This is used as a signal to start recording in multiuser mode

  unsigned int   frame_tx; // This is used to set the file index of the measurement file at the terminal
  int            crc_status; 		// crc status of the CHBCH
#endif //EMOS
} PHY_MEASUREMENTS;




//#include "../../../openair1/PHY/TRANSPORT/defs.h"
/// Top-level PHY Data Structure.  Contains TX,RX variables replicated for each transceiving antenna and the Acquisition of Initial Timing Synch variables.  Furthermore provision for measurements in support of higher layers has been added, but still to be defined precisely.  Synchronization variables for real-time operation (tx_mbox and rx_mbox) are also prensent.
typedef struct
{
/// TX variables indexed by antenna
  TX_VARS tx_vars[NB_ANTENNAS];      
/// RX variables indexed by antenna
  RX_VARS rx_vars[NB_ANTENNAS];      
/// ACQ Mailbox for harware synch
  unsigned int *mbox;                
/// TX/RX switch position in symbols (for TDD)
  unsigned int tx_rx_switch_point;   
/// CHSCH variables (up to 8)
/*
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
/// Measurement variables 
  PHY_MEASUREMENTS    PHY_measurements;       
*/
} PHY_VARS;

#endif //PHY_EMUL
/// Static Configuration Structure
typedef struct {
  /// TTI Configuration
  PHY_FRAMING         PHY_framing;
  /// CHBCH Configuration
  PHY_CHBCH           PHY_chbch;
  /// MRBCH Configuration
  PHY_MRBCH           PHY_mrbch;
  /// CHSCH Configuration (up to 8)
  PHY_CHSCH           PHY_chsch[8];
  /// SCH Configuration (up to 8)
  PHY_SCH             PHY_sch[8];
  /// SACH configuration
  PHY_SACH            PHY_sach;
  /// Number of CHSCH in TTI
  int                 total_no_chsch;
  /// Number of SCH in TTI
  int                 total_no_sch;
} PHY_CONFIG;

/** @} */
/** @defgroup _PHY_TRANSPORT_CHANNEL_PROCEDURES_ Transport Channel Procedures
* @ingroup _physical_layer_ref_implementation_
* @{
This section deals with the physical layer procedures for all transport channels and pilot channels, namely
- CHBCH generation/detection
- SACH/SACCH (including RACH as a special case) generation/detection
- MRBCH generation/detection
- CHSCH generation
- SCH generation
*/
/** @} */

/** @defgroup _PHY_PARAMETER_ESTIMATION_BLOCKS_ Parameter Estimation Blocks
* @ingroup _physical_layer_ref_implementation_
* @{
This section deals with the physical layer procedures related to parameter estimation, specifically
- Initial timing/frequency estimation
- Channel Estimation
- Timing drift tracking
- Automatic gain control
*/
/*!\brief Timing drift hysterisis in samples*/
#define SYNCH_HYST 5

/** @} */


/** @defgroup _PHY_MODULATION_BLOCKS_ OFDM Modulation Blocks
* @ingroup _physical_layer_ref_implementation_
* @{
This section deals with the physical layer procedures related to OFDM modulation.
*/

/** @} */

/** @defgroup _PHY_DSP_TOOLS_ DSP Tools
* @ingroup _physical_layer_ref_implementation_
* @{
This section deals with various DSP tools used by various blocks in the PHY. Specifically, 
- Efficient fixed-point DFT/IDFT via the FFT/IFFT
- Componentwise multiplication of complex vectors by complex vectors
- Componentwise multiplication of complex vectors by a complex scalar
*/

/** @} */

/** @defgroup _PHY_CODING_BLOCKS_ Channel Coding Blocks
* @ingroup _physical_layer_ref_implementation_
* @{
This section deals with the physical layer procedures related to channel coding
- Generic non-systematic rate 1/2 convolutional coder
- 3GPP Turbo coder
- Optimized 802.11 (64-state) Viterbi decoder
- 3GPP CRC
*/

/** @} */

/** @defgroup _PHY_CONFIG_BLOCKS_ Static Configuration Procedures
* @ingroup _physical_layer_ref_implementation_
* @{
This section deals with the physical layer procedures related to static configuration of the Framing and Transport Channel parameters.
*/

/** @} */
#endif //__PHY_IMPLEMENTATION_DEFS_H__ 

