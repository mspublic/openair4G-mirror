#ifndef __PHY_PROCEDURES_EMOS_H__
#define __PHY_PROCEDURES_EMOS_H__

#ifdef NO_RTAI
typedef long long unsigned int RTIME;
#else
#include <rtai.h>
#include <rtai_fifos.h>
#endif
//#include "PHY/TOOLS/defs.h"
#include "PHY/defs.h"

#define CHANSOUNDER_FIFO_SIZE 20971520  // 20 Mbytes FIFO
#define CHANSOUNDER_FIFO_MINOR 3               // minor of the FIFO device - this is /dev/rtf3

#define NUMBER_OF_OFDM_CARRIERS_EMOS 512 // the number of OFDM carriers used for channel sounding
#define NUMBER_OF_USEFUL_CARRIERS_EMOS 300    // the number of OFDM carriers that contain data

#define N_RB_UL_EMOS 25
#define N_PILOTS_DL_EMOS 2  // ofdm symbols with pilots per slot
#define N_PILOTS_UL_EMOS 2  // ofdm symbols with pilots per subframe
#define N_SLOTS_DL_EMOS 12     // we take slots 0,1,10,11,12,13,14,15,16,17,18,19
#define N_SUBFRAMES_UL_EMOS 3     // we take subframes 2,3,4
#define NB_ANTENNAS_TX_EMOS 2
#define NB_ANTENNAS_RX_EMOS 2

struct fifo_dump_emos_struct_UE {
  // RX
  RTIME            timestamp;              //! Timestamp of the receiver
  unsigned int     frame_tx;               //! Framenumber of the TX (encoded in the BCH)
  unsigned int     frame_rx;               //! Framenumber of the RX 
  UE_MODE_t        UE_mode;
  PHY_MEASUREMENTS PHY_measurements;       //! Structure holding all PHY measurements (one for every slot)
  unsigned int     pbch_errors;                        /// Total number of errors on PBCH
  unsigned int     pbch_errors_last;                   /// Total number of errors on PBCH 100 frames ago
  unsigned int     pbch_errors_conseq;                 /// Total number of consecutive errors on PBCH
  unsigned int     pbch_fer;                           /// PBCH FER (in percent) 
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
  int              channel[NB_ANTENNAS_TX_EMOS][NUMBER_OF_OFDM_CARRIERS_EMOS*N_PILOTS_DL_EMOS*N_SLOTS_DL_EMOS];
};

typedef struct  fifo_dump_emos_struct_UE fifo_dump_emos_UE;


struct fifo_dump_emos_struct_eNB {
  // TX
  RTIME            timestamp;              //! Timestamp of the receiver
  unsigned int     frame_tx;               //! Framenumber of the TX
  unsigned char    mimo_mode;              /// Transmission mode
  // RX
  PHY_MEASUREMENTS_eNB PHY_measurements_eNB;            /// UL measurements
  LTE_eNB_UE_stats eNB_UE_stats[NUMBER_OF_UE_MAX]; /// Contains received feedback
  unsigned int     rx_total_gain_dB;       /// Total gain
  int              channel[NB_ANTENNAS_RX_EMOS][N_RB_UL_EMOS*12*N_PILOTS_UL_EMOS*N_SUBFRAMES_UL_EMOS]; ///UL channel estimate
};

typedef struct  fifo_dump_emos_struct_eNB fifo_dump_emos_eNB;

#endif
