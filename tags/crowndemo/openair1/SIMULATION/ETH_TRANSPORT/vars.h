/*! \file vars.h
* \brief specifies the variables for phy emulation
* \author Navid Nikaein and Raymomd Knopp and Hicham Anouar
* \date 2011
* \version 1.0 
* \company Eurecom
* \email: navid.nikaein@eurecom.fr
*/ 

#ifndef USER_MODE
#include <rtai_posix.h>
#else
#include <pthread.h>
#endif
#include "defs.h"

#ifndef __BYPASS_SESSION_LAYER_VARS_H__
#    define __BYPASS_SESSION_LAYER_VARS_H__

unsigned char Emulation_status;
unsigned char emu_tx_status;
unsigned char emu_rx_status;
//unsigned int Master_list=0;
//unsigned short Master_id;
//unsigned int Is_primary_master;

pthread_mutex_t emul_low_mutex;
pthread_cond_t emul_low_cond;
char emul_low_mutex_var;
pthread_mutex_t Tx_mutex;
pthread_cond_t Tx_cond;
char Tx_mutex_var;

int (*rx_handler) (unsigned char,char*,int);
int (*tx_handler) (unsigned char,char*, unsigned int*, unsigned int*);

eNB_transport_info_t eNB_transport_info[NUMBER_OF_eNB_MAX];
u16 eNB_transport_info_TB_index[NUMBER_OF_eNB_MAX];

UE_transport_info_t UE_transport_info[NUMBER_OF_UE_MAX];
u16 UE_transport_info_TB_index[NUMBER_OF_UE_MAX];

UE_cntl ue_cntl_delay[2];

#endif
