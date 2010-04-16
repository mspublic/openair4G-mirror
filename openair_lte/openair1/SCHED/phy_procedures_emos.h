#ifndef __PHY_PROCEDURES_EMOS_H__
#define __PHY_PROCEDURES_EMOS_H__

#ifndef USER_MODE 
#include <rtai.h>
#else
typedef long long unsigned int RTIME;
#endif
//#include "PHY/TOOLS/defs.h"
#include "PHY/defs.h"

#define CHANSOUNDER_FIFO_SIZE 20971520  // 20 Mbytes FIFO
#define CHANSOUNDER_FIFO_MINOR 3               // minor of the FIFO device - this is /dev/rtf3

#ifdef OPENAIR_LTE

#define NUMBER_OF_OFDM_CARRIERS_EMOS 512 // the number of OFDM carriers used for channel sounding
#define NUMBER_OF_USEFUL_CARRIERS_EMOS 300    // the number of OFDM carriers that contain data

#define N_RB_DL_EMOS 25
#define N_PILOTS_PER_RB 4  // per tx antenna
#define N_SLOTS_EMOS 4     // we take slots 0,1,12,13

#define N_RB_UL_EMOS 25
#define N_PILOTS_PER_RB_UL 12
#define N_SRS_SYMBOLS 5

#define MAX_DCI_PER_FRAME 20
#define MAX_UCI_PER_FRAME 20

// This structure hold all the data that is written to FIFO in one frame
// Make sure that this is updated accordingly when new data is written to FIFO
// MAKE SURE THE SIZE OF THIS STRUCTURE IS A MULTIPLE OF 4 (32 bit aligned) 


typedef struct {
  unsigned char o[MAX_CQI_BITS];/// Pointer to CQI data
  unsigned char O;  /// Length of CQI data (bits)
  unsigned char o_RI[2];  /// Rank information 
  unsigned char O_RI;  /// Length of rank information (bits)
  unsigned char o_ACK[4];  /// Pointer to ACK
  unsigned char O_ACK;  /// Length of ACK information (bits)
} UCI_DATA_t;

struct fifo_dump_emos_struct_UE {
  // RX
  RTIME	           timestamp;              //! Timestamp of the receiver
  unsigned int     frame_tx;               //! Framenumber of the TX (encoded in the BCH)
  unsigned int     frame_rx;               //! Framenumber of the RX 
  PHY_MEASUREMENTS PHY_measurements[20];       //! Structure holding all PHY measurements (one for every slot)
  char             pbch_pdu[NUMBER_OF_eNB_MAX][PBCH_PDU_SIZE];           /// Contents of the PBCH
  unsigned int     pbch_errors[NUMBER_OF_eNB_MAX];                        /// Total number of errors on PBCH
  unsigned int     pbch_errors_last[NUMBER_OF_eNB_MAX];                   /// Total number of errors on PBCH 100 frames ago
  unsigned int     pbch_errors_conseq[NUMBER_OF_eNB_MAX];                 /// Total number of consecutive errors on PBCH
  unsigned int     pbch_fer[NUMBER_OF_eNB_MAX];                           /// PBCH FER (in percent) 
  unsigned int     dci_cnt[10];                                          /// Number of DCIs received in subframe
  unsigned int     dci_errors;                                           /// Total number of errors on PDCCH
  unsigned int     dci_received;                                         /// Total number of received PDCCH
  unsigned int     dci_false;                                            /// Total number of falsely received PDCCH (only in DIAG mode)
  unsigned int     dci_missed;                                           /// Total number of missed PDCCH (only in DIAG mode)
  DCI_ALLOC_t      DCI_alloc[2][10];                                     /// DCI for every subframe (received)
  unsigned int     dlsch_errors;                                         /// Total number of error on the DLSCH (data)
  unsigned int     dlsch_errors_last;
  unsigned int     dlsch_received;
  unsigned int     dlsch_received_last;
  unsigned int     dlsch_fer;
  unsigned int     dlsch_cntl_errors;                                    /// Total number of error on the DLSCH (control)
  unsigned int     dlsch_ra_errors;                                      /// Total number of error on the DLSCH (random access)
  int              timing_offset;                                        /// Timing offset
  int              timing_advance;                                       /// Timing advance
  int              freq_offset;                                          /// Frequency offset
  unsigned int     rx_total_gain_dB;                                     /// Total gain
  unsigned char    eNb_id;                                               /// eNb_id UE is synched to
  unsigned char    mimo_mode;                                            /// Transmission mode
  int              channel[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX*NB_ANTENNAS_TX][N_RB_DL_EMOS*N_PILOTS_PER_RB*N_SLOTS_EMOS];
  // TX
  unsigned int     uci_cnt[10];
  UCI_DATA_t       UCI_data[2][10];                                      /// UCI informations for every subframe (transmitted)
};


struct fifo_dump_emos_struct_eNb {
  // TX
  RTIME	           timestamp;              //! Timestamp of the receiver
  unsigned int     frame_tx;               //! Framenumber of the TX
  unsigned int     dci_cnt[10];
  DCI_ALLOC_t      DCI_alloc[2][10];       /// DCI for every subframe (sent)
  unsigned char    mimo_mode;              /// Transmission mode
  // RX
  PHY_MEASUREMENTS_eNB PHY_measurements_eNB[3];            /// UL measurements
  LTE_eNB_UE_stats eNB_UE_stats[NUMBER_OF_eNB_MAX][10]; /// Contains received feedback
  unsigned int     ulsch_errors;
  unsigned int     rx_total_gain_dB;       /// Total gain
  int              channel[N_SRS_SYMBOLS][NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX][N_RB_UL_EMOS*N_PILOTS_PER_RB_UL]; ///UL channel estimate
};

 
typedef struct  fifo_dump_emos_struct_UE fifo_dump_emos_UE;
typedef struct  fifo_dump_emos_struct_eNb fifo_dump_emos_eNb;


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

