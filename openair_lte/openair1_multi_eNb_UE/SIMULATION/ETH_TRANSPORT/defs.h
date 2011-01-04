/*________________________bypass_session_layer_defs.h________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/


#ifndef __BYPASS_SESSION_LAYER_DEFS_H__
#    define __BYPASS_SESSION_LAYER_DEFS_H__
//-----------------------------------------------------------------------------
//#include "openair_defs.h"
#define  BYPASS_CHBCH_DATA    0x01
#define  BYPASS_UL_DL_DATA           0x02
#define  BYPASS_MESSAGE_TYPE_CONTROL_BROADCAST           0x03

#define CH_TRAFFIC 0
#define UE_TRAFFIC 1

#define CHBCH_DATA 0
#define UL_DL_DATA 1
#define CH_BYPASS_CONTROL 2

#define NO_WAIT 0
#define WAIT_CHBCH_DATA 1
#define WAIT_UL_DL_DATA 2
#define WAIT_PM_CT 3
#define WAIT_EM_CT 4

#define BYPASS_RX_BUFFER_SIZE 64000
#define BYPASS_TX_BUFFER_SIZE 64000



typedef struct bypass_msg_header {
  unsigned char  Message_type;
  unsigned char  Nb_flows;
  unsigned int   M_id;
}bypass_msg_header_t;

typedef struct bypass_proto2multicast_header_t {
  unsigned int      size;
} bypass_proto2multicast_header_t;




void bypass_init ( int (*tx_handlerP) (unsigned char,char*, unsigned int*, unsigned int*),int (*rx_handlerP) (unsigned char,char*,int));
int bypass_rx_data (void);
void  bypass_signal_mac_phy(void );
#ifndef USER_MODE
int multicast_link_write_sock (int groupP, char *dataP, unsigned int sizeP);
int bypass_tx_handler(unsigned int fifo, int rw);
int bypass_rx_handler(unsigned int fifo, int rw);
#else
void bypass_rx_handler(unsigned int Num_bytes,char *Rx_buffer);
#endif

void bypass_tx_data (char Type);

void emulation_tx_rx(void);

unsigned int emul_tx_handler(unsigned char Mode,char *Tx_buffer,unsigned int* Nbytes,unsigned int *Nb_flows);
unsigned int emul_rx_handler(unsigned char Mode,char *rx_buffer, unsigned int Nbytes);

unsigned int emul_rx_data(void);


/*************************************************************/


typedef struct  {
  u32 pbch_flag:1;
  u32 pss:2;
  u32 sss:8;
  u32 cfi:2;
  u32 phich:19;
  u32 pbch_payload:24;
} eNB_cntl;

typedef struct  {
  u32 pucch_flag:3;  // 0,7 = none, 1 = type 1, 2=type 1a, 3=type 1b, 4=type 2, 5=type 2a, 6=type 2b
  u32 pucch_Ncs1:3;  // physical configuration of pucch, for abstraction purposes
  u32 pucch_uci:13;        // cqi information
  u32 pucch_ack:2;         // ack/nak information
  u32 pusch_flag:1;  // 0=none,1=active
  u32 pusch_uci;     // uci information on pusch
  u32 pusch_ri:2;    // ri information on pusch
  u32 pusch_ack:2;   // ack/nak on pusch
  u32 prach_flag:1;  // 0=none,1=active
  u32 prach_id:6;    // this is the PHY preamble index for the prach
} UE_cntl;

#define MAX_TRANSPORT_BLOCKS_BUFFER_SIZE 16384

typedef struct {
  eNB_cntl cntl;
  u8 num_common_dci;
  u8 num_ue_spec_dci;
  DCI_ALLOC_t dci_alloc;
  u8 transport_blocks[MAX_TRANSPORT_BLOCKS_BUFFER_SIZE];
} eNB_transport_info_t ;

typedef struct {
  UE_cntl cntl;
  u8 transport_blocks[MAX_TRANSPORT_BLOCKS_BUFFER_SIZE];
} UE_transport_info_t ;

void clear_eNB_transport_info(u8);
void clear_UE_transport_info(u8);

#endif //


