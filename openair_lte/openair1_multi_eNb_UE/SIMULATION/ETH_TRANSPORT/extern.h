/*________________________bypass_session_layer_extern.h________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/

#ifndef __BYPASS_SESSION_LAYER_EXTERN_H__
#    define __BYPASS_SESSION_LAYER_EXTERN_H__

#include <pthread.h>

extern unsigned char Emulation_status;
extern unsigned int Master_list;
extern unsigned short Master_id;
extern unsigned int Is_primary_master;

extern pthread_mutex_t emul_low_mutex;
extern pthread_cond_t emul_low_cond;
extern char emul_low_mutex_var;
extern pthread_mutex_t Tx_mutex;
extern pthread_cond_t Tx_cond;
extern char Tx_mutex_var;


extern int (*rx_handler) (unsigned char,char*,int);
extern int (*tx_handler) (unsigned char,char*, unsigned int*, unsigned int*);

#endif
