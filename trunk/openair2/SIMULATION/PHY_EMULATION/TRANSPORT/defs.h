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

/*************************************************************/

#endif //

