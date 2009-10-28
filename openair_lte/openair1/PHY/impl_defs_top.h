/*________________________phy/impl_defs.h________________________

 Authors : Raymond Knopp, Florian Kaltenberger
 Company : EURECOM
 Emails  : kaltenbe@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


#ifndef __PHY_IMPLEMENTATION_DEFS_H__
#define __PHY_IMPLEMENTATION_DEFS_H__


#include "types.h"
#include "spec_defs_top.h"



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
#define EMOS_SCH_INDEX 1
#endif //EMOS

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


//#define GAIN_QPSK 1024              // Amplitude of QPSK transmit modulation
//#define GAIN_QPSK_2ANT 724         // Amplitude of QPSK transmit modulation with 2 TX antennas (GAIN_QPSK/sqrt(2))
//#define GAIN_16QAM 458             // Amplitude of 16QAM transmit modulation (GAIN_QPSK/sqrt(5))
//#define GAIN_16QAM_2ANT 324        // Amplitude of 16QAM transmit modulation with 2 TX antennas (GAIN_16QAM/sqrt(2))    

#define TARGET_RX_POWER 43		// Target digital power for the AGC
#define TARGET_RX_POWER_MAX 48		// Maximum digital power, such that signal does not saturate (value found by simulation)
#define TARGET_RX_POWER_MIN 36		// Minimum digital power, anything below will be discarded (value found by simulation)
#define MAX_RF_GAIN 150
#define MIN_RF_GAIN 96

#define PHY_SYNCH_OFFSET ((OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)-1)  // OFFSET of BEACON SYNCH
#define PHY_SYNCH_MIN_POWER 1000
#define PHY_SYNCH_THRESHOLD 100



#define ONE_OVER_SQRT2_Q15 23170

#ifdef BIT8_RXMUX
#define PERROR_SHIFT 0
#else
#define PERROR_SHIFT 10
#endif

#define BIT8_TX_SHIFT 2
#define BIT8_TX_SHIFT_DB 12

#define CHBCH_RSSI_MIN -75

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

// QAM amplitude definitions

/// First Amplitude for QAM16 (\f$ 2^15 \times 2/\sqrt{10}\f$)
#define QAM16_n1 20724
/// Second Amplitude for QAM16 (\f$ 2^15 \times 1/\sqrt{10}\f$)
#define QAM16_n2 10362

///First Amplitude for QAM64 (\f$ 2^14 \times 4/\sqrt{42}\f$)
#define QAM64_n1 10112
///Second Amplitude for QAM64 (\f$ 2^14 \times 2/\sqrt{42}\f$)
#define QAM64_n2 5056
///Third Amplitude for QAM64 (\f$ 2^14 \times 1/\sqrt{42}\f$)
#define QAM64_n3 2528

/// Size of SACCH PDU in Bytes
#define SACCH_SIZE_BYTES (sizeof(UL_SACCH_PDU)+4) 
/// Size of SACCH PDU in Bytes
#define SACCH_SIZE_BITS  (SACCH_SIZE_BYTES<<3)

#define MAX_SACH_SIZE_BYTES 1024


#define SACH_ERROR 1
#define SACCH_ERROR 2
#define SACH_MISSING 3
#define SACH_PARAM_INVALID 10

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

/// Data structure for transmission.
typedef struct {
  /* RAW TX sample buffer */
  int *TX_DMA_BUFFER;
  /* Total transmit gain */           
  unsigned int tx_total_gain_dB;
  /* Dummy overlap for multi-antenna operation */                                                            
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
#ifndef OPENAIR_LTE
typedef struct
{


  unsigned short rx_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS_RX];  // estimated received signal power (dB)
  short          rx_avg_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX];               // estimated avg received signal power (dB)
  unsigned short n0_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS_RX];  // estimated noise power (dB)
  short		 rx_rssi_dBm[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS_RX];  // estimated rssi (dBm)
  int            rx_power[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS_RX];// estimated received signal power (linear)
  int            n0_power[NUMBER_OF_CHSCH_SYMBOLS_MAX][NB_ANTENNAS_RX];// estimated noise power (linear)
  unsigned int   chbch_search_count;
  unsigned int   chbch_detection_count[4];
  unsigned int   mrbch_search_count; 
  unsigned int   mrbch_detection_count;
#ifdef EMOS
  //  unsigned char  Meas_flag;      	 // This is used as a signal to start recording in multiuser mode
  unsigned int   frame_tx[2];              // This is used to set the file index of the measurement file at the terminal
  int            crc_status[2]; 		// crc status of the CHBCH
#endif //EMOS
} PHY_MEASUREMENTS;

/// Physical Resource Descriptor
typedef struct {
  unsigned char  Time_alloc;           /*!<\brief Time allocation vector of DL-SACH reservation*/
  unsigned short Freq_alloc;      /*!< \brief Frequency allocation vector of DL-SACH reservation*/
  unsigned short Ifreq_alloc;      /*!< \brief Frequency allocation vector of interference (DL-SACH)*/
  unsigned char  Antenna_alloc;   /*!< \brief Antenna allocation vector of DL-SACH reservation*/ 
  unsigned char  Coding_fmt;          /*!< \brief Coding format for this PDU*/
} __attribute__((__packed__)) PHY_RESOURCES;
#define PHY_RESOURCES_SIZE sizeof(PHY_RESOURCES)
#endif //OPENAIR_LTE


/// Static Configuration Structure
typedef struct {
  PHY_FRAMING         PHY_framing;       /*!<\brief TTI Configuration*/
#ifndef OPENAIR_LTE
  PHY_CHBCH           PHY_chbch[8];      /*!<\brief CHBCH Configuration*/
  PHY_MRBCH           PHY_mrbch;         /*!<\brief MRBCH Configuration*/
  PHY_CHSCH           PHY_chsch[8];      /*!<\brief CHSCH Configuration (up to 8)*/
  PHY_SCH             PHY_sch[8];        /*!<\brief SCH Configuration (up to 8)*/
  PHY_SACH            PHY_sach;          /*!<\brief SACH configuration*/
  int                 total_no_chsch;    /*!<\brief Number of CHSCH*/
  int                 total_no_chbch;    /*!<\brief Number of CHBCH*/
  int                 total_no_sch;      /*!<\brief Number of SCH*/
#else
  LTE_DL_FRAME_PARMS lte_frame_parms;
#endif //OPENAIR_LTE
} PHY_CONFIG;

/// Top-level PHY Data Structure  
typedef struct
{
/// TX variables indexed by antenna
  TX_VARS tx_vars[NB_ANTENNAS_TX];      
/// RX variables indexed by antenna
  RX_VARS rx_vars[NB_ANTENNAS_RX];      
/// ACQ Mailbox for harware synch
  unsigned int *mbox;                
/// TX/RX switch position in symbols (for TDD)
  unsigned int tx_rx_switch_point;   
#ifndef OPENAIR_LTE
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
/// Measurement variables 
  PHY_MEASUREMENTS    PHY_measurements;
  /// Diagnostics for SACH Metering
  SACH_DIAGNOSTICS   Sach_diagnostics[NB_CNX_CH][1+NB_RAB_MAX];
#else
  LTE_UE_COMMON lte_ue_common_vars;
#endif

} PHY_VARS;




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

