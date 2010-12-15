/*________________________bypass_session_layer_vars.h________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/
#include <pthread.h>

#ifndef __BYPASS_SESSION_LAYER_VARS_H__
#    define __BYPASS_SESSION_LAYER_VARS_H__

unsigned char Emulation_status;
unsigned int Master_list=0;
unsigned short Master_id;

unsigned int Is_primary_master;
pthread_mutex_t emul_low_mutex;
pthread_cond_t emul_low_cond;
char emul_low_mutex_var;
pthread_mutex_t Tx_mutex;
pthread_cond_t Tx_cond;
char Tx_mutex_var;

int (*rx_handler) (unsigned char,char*,int);
int (*tx_handler) (unsigned char,char*, unsigned int*, unsigned int*);

#endif
