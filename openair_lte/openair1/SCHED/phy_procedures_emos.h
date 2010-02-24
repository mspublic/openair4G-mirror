#ifndef __PHY_PROCEDURES_EMOS_H__
#define __PHY_PROCEDURES_EMOS_H__

#include <rtai.h>
#include "PHY/TOOLS/defs.h"
#include "PHY/defs.h"

#define CHANSOUNDER_FIFO_SIZE 20971520  // 20 Mbytes FIFO
#define CHANSOUNDER_FIFO_MINOR 3               // minor of the FIFO device - this is /dev/rtf3

#ifdef OPENAIR_LTE

#define NUMBER_OF_OFDM_CARRIERS_EMOS 512 // the number of OFDM carriers used for channel sounding
#define NUMBER_OF_USEFUL_CARRIERS_EMOS 300    // the number of OFDM carriers that contain data

#define N_RB_DL_EMOS 25
#define N_PILOTS_PER_RB 4  //per tx antenna
#define N_SLOTS_EMOS 2

#define N_RB_UL_EMOS 25
#define N_PILOTS_PER_RB_UL 12
#define N_SRS_SYMBOLS 5

#define MAX_DCI_PER_FRAME 20
#define MAX_UCI_PER_FRAME 20

// This structure hold all the data that is written to FIFO in one frame
// Make sure that this is updated accordingly when new data is written to FIFO
// MAKE SURE THE SIZE OF THIS STRUCTURE IS A MULTIPLE OF 4 (32 bit aligned) 
struct fifo_dump_emos_struct_UE {
  RTIME	           timestamp;              //! Timestamp of the receiver
  unsigned int     frame_tx;               //! Framenumber of the TX (encoded in the BCH)
  unsigned int     frame_rx;               //! Framenumber of the RX 
  PHY_MEASUREMENTS PHY_measurements;       //! Structure holding all PHY measurements
  char             pbch_pdu[NUMBER_OF_eNB_MAX][PBCH_PDU_SIZE];           /// Contents of the PBCH
  unsigned int     pdu_errors[NUMBER_OF_eNB_MAX];                        /// Total number of PDU errors
  unsigned int     pdu_errors_last[NUMBER_OF_eNB_MAX];                   /// Total number of PDU errors 128 frames ago
  unsigned int     pdu_errors_conseq[NUMBER_OF_eNB_MAX];                 /// Total number of consecutive PDU errors
  unsigned int     pdu_fer[NUMBER_OF_eNB_MAX];                           /// FER (in percent) 
  DCI_type_t       DCI_format[MAX_DCI_PER_FRAME];                        /// chosen DL format (DCI)
  char             DCI_pdu[MAX_DCI_PER_FRAME][6];                        /// content of DCI
  UCI_type_t       UCI_format[MAX_UCI_PER_FRAME];                        /// chosen feedback format (UCI)
  char             UCI_pdu[MAX_UCI_PER_FRAME][5];                        /// content of the feedback information
  int              timing_offset;                                        /// Timing offset
  int              freq_offset;                                          /// Frequency offset
  unsigned int     rx_total_gain_dB;                                     /// Total gain
  int              channel[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX*NB_ANTENNAS_TX][N_RB_DL_EMOS*N_PILOTS_PER_RB*N_SLOTS_EMOS];
};


struct fifo_dump_emos_struct_eNb {
  RTIME	           timestamp;              //! Timestamp of the receiver
  unsigned int     frame_tx;               //! Framenumber of the TX (encoded in the BCH)
  PHY_MEASUREMENTS PHY_measurements;
  unsigned int     rx_total_gain_dB;       /// Total gain
  MIMO_mode_t      mimo_mode;              /// Transmission mode
  int              channel[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX][N_RB_UL_EMOS*N_PILOTS_PER_RB_UL*N_SRS_SYMBOLS];
};

 
typedef struct  fifo_dump_emos_struct fifo_dump_emos;

#else //OPENAIR_LTE

//#define EMOS_START_PILOT 16
//#define EMOS_END_PILOT 31

#define NUMBER_OF_OFDM_CARRIERS_EMOS 256 // the number of OFDM carriers used for channel sounding
#define NUMBER_OF_USEFUL_CARRIERS_EMOS 160    // the number of OFDM carriers that contain data

// This structure hold all the data that is written to FIFO in one frame
// Make sure that this is updated accordingly when new data is written to FIFO
// MAKE SURE THE SIZE OF THIS STRUCTURE IS A MULTIPLE OF 4 (32 bit aligned) 
struct fifo_dump_emos_struct {
  RTIME	           timestamp;
  PHY_MEASUREMENTS PHY_measurements;
  char             chbch_pdu[2][CHBCH_PDU_SIZE];                            /// Contents of the CHBCH
  int              pdu_errors[2];
  int              offset;                                               /// Timing offset
  unsigned int     rx_total_gain_dB;                                     /// Total gain
  unsigned int     rx_mode;
};
 
typedef struct  fifo_dump_emos_struct fifo_dump_emos;

struct fifo_read_emos_struct {
  RTIME	           timestamp;
  PHY_MEASUREMENTS PHY_measurements;
  char             chbch_pdu[2][CHBCH_PDU_SIZE];                            /// Contents of the CHBCH
  int              pdu_errors[2];
  int              offset;                                               /// Timing offset
  unsigned int     rx_total_gain_dB;                                     /// Total gain
  unsigned int     rx_mode;
  struct complex16 channel_f_unpacked[2][NB_ANTENNAS_RX][NUMBER_OF_OFDM_CARRIERS_EMOS];          /// Channel Estimate in Freq. Domain (RX)
  struct complex16 perror[2][NB_ANTENNAS_RX][48];                           /// Estimated phase drift
};
 
typedef struct  fifo_read_emos_struct fifo_read_emos;

#endif //OPENAIR_LTE
#endif

