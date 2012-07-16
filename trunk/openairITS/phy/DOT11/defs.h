#ifndef __DEFS_H__
#define __DEFS_H__

#define MAX_SDU_SIZE 4095
#define AMP 768

#define ONE_OVER_SQRT_2 23170
#define ONE_OVER_SQRT_10 10362
#define ONE_OVER_SQRT_42 5056

typedef enum {
  BPSK_1_2=0,
  BPSK_3_4=1,
  QPSK_1_2=2,
  QPSK_3_4=3,
  QAM16_1_2=4,
  QAM16_3_4=5,
  QAM64_1_2=6,
  QAM64_3_4=7
} RATE_t;


typedef struct {
  int sdu_length;
  RATE_t rate;
  int service;
} TX_VECTOR_t;

int phy_tx_start(TX_VECTOR_t *tx_vector,uint32_t *tx_frame,uint32_t next_TXop_offset,uint8_t *data_ind);
int phy_tx_start_bot(TX_VECTOR_t *tx_vector,int16_t *output_ptr,uint8_t *data_ind);
#endif
