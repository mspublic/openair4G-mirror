#ifndef __PHY_PROCEDURES_EMOS_H__
#define __PHY_PROCEDURES_EMOS_H__

//#define EMOS_START_PILOT 16
//#define EMOS_END_PILOT 31

#define CHANSOUNDER_FIFO_SIZE 20971520  // 20 Mbytes FIFO
#define CHANSOUNDER_FIFO_MINOR 3               // minor of the FIFO device - this is /dev/rtf3

#define NUMBER_OF_OFDM_CARRIERS_EMOS 256 // the number of OFDM carriers used for channel sounding
#define NUMBER_OF_USEFUL_CARRIERS_EMOS 160    // the number of OFDM carriers that contain data

#include <rtai.h>
#include "PHY/TOOLS/defs.h"
#include "PHY/defs.h"

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

#endif

