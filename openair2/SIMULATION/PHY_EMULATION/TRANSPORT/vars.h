/*________________________bypass_session_layer_vars.h________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/

#ifndef __BYPASS_SESSION_LAYER_VARS_H__
#    define __BYPASS_SESSION_LAYER_VARS_H__


int (*rx_handler) (unsigned char,char*,int);
int (*tx_handler) (unsigned char,char*, unsigned int*, unsigned int*);

#endif
