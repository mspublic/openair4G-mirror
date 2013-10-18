/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file PHY/impl_defs_top.h
* \brief More defines and structure definitions
* \author R. Knopp, F. Kaltenberger
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr
* \note
* \warning
*/

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
#define NUMBER_OF_OFDM_SYMBOLS_PER_SLOT (NUMBER_OF_SYMBOLS_PER_FRAME/LTE_SLOTS_PER_FRAME)
#else
#define NUMBER_OF_OFDM_SYMBOLS_PER_SLOT 16
#endif

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


#ifdef EXMIMO
#define TARGET_RX_POWER 40		// Target digital power for the AGC
#define TARGET_RX_POWER_MAX 65		// Maximum digital power, such that signal does not saturate (value found by simulation)
#define TARGET_RX_POWER_MIN 30		// Minimum digital power, anything below will be discarded (value found by simulation)
#else
#define TARGET_RX_POWER 50		// Target digital power for the AGC
#define TARGET_RX_POWER_MAX 65		// Maximum digital power, such that signal does not saturate (value found by simulation)
#define TARGET_RX_POWER_MIN 35		// Minimum digital power, anything below will be discarded (value found by simulation)
#endif

//the min and max gains have to match the calibrated gain table
//#define MAX_RF_GAIN 160
//#define MIN_RF_GAIN 96
#define MAX_RF_GAIN 160
#define MIN_RF_GAIN 80

#define PHY_SYNCH_OFFSET ((OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)-1)  // OFFSET of BEACON SYNCH
#define PHY_SYNCH_MIN_POWER 1000
#define PHY_SYNCH_THRESHOLD 100



#define ONE_OVER_SQRT2_Q15 23170


// QAM amplitude definitions

/// First Amplitude for QAM16 (\f$ 2^{15} \times 2/\sqrt{10}\f$)
#define QAM16_n1 20724
/// Second Amplitude for QAM16 (\f$ 2^{15} \times 1/\sqrt{10}\f$)
#define QAM16_n2 10362

///First Amplitude for QAM64 (\f$ 2^{15} \times 4/\sqrt{42}\f$)
#define QAM64_n1 20225
///Second Amplitude for QAM64 (\f$ 2^{15} \times 2/\sqrt{42}\f$)
#define QAM64_n2 10112
///Third Amplitude for QAM64 (\f$ 2^{15} \times 1/\sqrt{42}\f$)
#define QAM64_n3 5056

/// First Amplitude for QAM16 for TM5 (\f$ 2^{15} \times 2/sqrt(20)\f$)
#define QAM16_TM5_n1 14654
/// Second Amplitude for QAM16 for TM5 Receiver (\f$ 2^{15} \times 1/\sqrt{20}\f$)
#define QAM16_TM5_n2 7327

///First Amplitude for QAM64 (\f$ 2^{15} \times 4/\sqrt{84}\f$)
#define QAM64_TM5_n1 14301
///Second Amplitude for QAM64 (\f$ 2^{15} \times 2/\sqrt{84}\f$)
#define QAM64_TM5_n2 7150
///Third Amplitude for QAM64 for TM5 Receiver (\f$ 2^{15} \times 1/\sqrt{84}\f$)
#define QAM64_TM5_n3 3575


#ifdef BIT8_RXMUX
#define PERROR_SHIFT 0
#else
#define PERROR_SHIFT 10
#endif

#define BIT8_TX_SHIFT 2
#define BIT8_TX_SHIFT_DB 12

//#define CHBCH_RSSI_MIN -75

#ifdef BIT8_TX
#define AMP 128 
#else
//#define AMP 512 //1024 //4096
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
  /// RAW TX sample buffer
  char *TX_DMA_BUFFER[2];
} TX_VARS ;  

/// Data structure for reception.
typedef struct {
  /// RAW TX sample buffer
  char *TX_DMA_BUFFER[2];
  /// RAW RX sample buffer
  int *RX_DMA_BUFFER[2];
} TX_RX_VARS;

/// Measurement Variables

#define NUMBER_OF_SUBBANDS 7
#define NUMBER_OF_HARQ_PID_MAX 8

#if defined(CBMIMO1) || defined(EXMIMO)
#define NUMBER_OF_eNB_MAX 1
#define NUMBER_OF_UE_MAX 2
#define NUMBER_OF_CONNECTED_eNB_MAX 3
#else
#ifdef LARGE_SCALE
#define NUMBER_OF_eNB_MAX 2
#define NUMBER_OF_UE_MAX 120
#define NUMBER_OF_CONNECTED_eNB_MAX 1 // to save some memory
#else 
#define NUMBER_OF_eNB_MAX 7
#define NUMBER_OF_UE_MAX 16
#define NUMBER_OF_CONNECTED_eNB_MAX 3
#endif 
#endif


typedef struct
{
  //unsigned int   rx_power[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX];     //! estimated received signal power (linear)
  //unsigned short rx_power_dB[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX];  //! estimated received signal power (dB)
  //unsigned short rx_avg_power_dB[NUMBER_OF_CONNECTED_eNB_MAX];              //! estimated avg received signal power (dB)

  // RRC measurements
  int rssi;
  int n_adj_cells;
  unsigned int adj_cell_id[6];
  int rsrq[7];
  int rsrp[7];
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
  unsigned int   n0_power_avg;     
  //! average estimated noise power (dB)
  unsigned short n0_power_avg_dB;     
  //! total estimated noise power (dBm)                            
  short n0_power_tot_dBm;

  // UE measurements
  //! estimated received spatial signal power (linear)
  int            rx_spatial_power[NUMBER_OF_CONNECTED_eNB_MAX][2][2];       
  //! estimated received spatial signal power (dB) 
  unsigned short rx_spatial_power_dB[NUMBER_OF_CONNECTED_eNB_MAX][2][2];    

  /// estimated received signal power (sum over all TX antennas)
  //int            wideband_cqi[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX];                     
  int            rx_power[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX];                     
  /// estimated received signal power (sum over all TX antennas)
  //int            wideband_cqi_dB[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX];                  
  unsigned short rx_power_dB[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX];                  

  /// estimated received signal power (sum over all TX/RX antennas)
  int            rx_power_tot[NUMBER_OF_CONNECTED_eNB_MAX]; //NEW
  /// estimated received signal power (sum over all TX/RX antennas)
  unsigned short rx_power_tot_dB[NUMBER_OF_CONNECTED_eNB_MAX]; //NEW

  //! estimated received signal power (sum of all TX/RX antennas, time average)
  int            rx_power_avg[NUMBER_OF_CONNECTED_eNB_MAX];                                 
  //! estimated received signal power (sum of all TX/RX antennas, time average, in dB)
  unsigned short rx_power_avg_dB[NUMBER_OF_CONNECTED_eNB_MAX];                                 

  /// SINR (sum of all TX/RX antennas, in dB)
  int            wideband_cqi_tot[NUMBER_OF_CONNECTED_eNB_MAX];                                 
  /// SINR (sum of all TX/RX antennas, time average, in dB)
  int            wideband_cqi_avg[NUMBER_OF_CONNECTED_eNB_MAX];                                 

  //! estimated rssi (dBm)
  short          rx_rssi_dBm[NUMBER_OF_CONNECTED_eNB_MAX];                  
  //! estimated correlation (wideband linear) between spatial channels (computed in dlsch_demodulation)
  int            rx_correlation[NUMBER_OF_CONNECTED_eNB_MAX][2];            
  //! estimated correlation (wideband dB) between spatial channels (computed in dlsch_demodulation)
  int            rx_correlation_dB[NUMBER_OF_CONNECTED_eNB_MAX][2];         

  /// Wideband CQI (sum of all RX antennas, in dB, for precoded transmission modes (4,5,6), up to 4 spatial streams)
  int            precoded_cqi_dB[NUMBER_OF_CONNECTED_eNB_MAX+1][4];                               
  /// Subband CQI per RX antenna (= SINR)
  int            subband_cqi[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX][NUMBER_OF_SUBBANDS];  
  /// Total Subband CQI  (= SINR)
  int            subband_cqi_tot[NUMBER_OF_CONNECTED_eNB_MAX][NUMBER_OF_SUBBANDS];              
  /// Subband CQI in dB (= SINR dB)
  int            subband_cqi_dB[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX][NUMBER_OF_SUBBANDS];
  /// Total Subband CQI   
  int            subband_cqi_tot_dB[NUMBER_OF_CONNECTED_eNB_MAX][NUMBER_OF_SUBBANDS];           
  /// Wideband PMI for each RX antenna
  int            wideband_pmi_re[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX];                  
  /// Wideband PMI for each RX antenna
  int            wideband_pmi_im[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX];                  
  ///Subband PMI for each RX antenna
  int            subband_pmi_re[NUMBER_OF_CONNECTED_eNB_MAX][NUMBER_OF_SUBBANDS][NB_ANTENNAS_RX]; 
  ///Subband PMI for each RX antenna
  int            subband_pmi_im[NUMBER_OF_CONNECTED_eNB_MAX][NUMBER_OF_SUBBANDS][NB_ANTENNAS_RX];
  /// chosen RX antennas (1=Rx antenna 1, 2=Rx antenna 2, 3=both Rx antennas) 
  unsigned char           selected_rx_antennas[NUMBER_OF_CONNECTED_eNB_MAX][NUMBER_OF_SUBBANDS];         
  /// Wideband Rank indication
  unsigned char  rank[NUMBER_OF_CONNECTED_eNB_MAX];   
  /// Number of RX Antennas                                            
  unsigned char  nb_antennas_rx;                                           
  /// DLSCH error counter
  // short          dlsch_errors;
                                                    
} PHY_MEASUREMENTS;

typedef struct
{
  //unsigned int   rx_power[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX];     //! estimated received signal power (linear)
  //unsigned short rx_power_dB[NUMBER_OF_CONNECTED_eNB_MAX][NB_ANTENNAS_RX];  //! estimated received signal power (dB)
  //unsigned short rx_avg_power_dB[NUMBER_OF_CONNECTED_eNB_MAX];              //! estimated avg received signal power (dB)

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

#define MCS_COUNT 24

#endif //__PHY_IMPLEMENTATION_DEFS_H__ 
/**@}
  *@}
*/
