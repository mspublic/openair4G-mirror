/*________________________bypass_session_layer_extern.h________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/

#ifndef __BYPASS_SESSION_LAYER_EXTERN_H__
#    define __BYPASS_SESSION_LAYER_EXTERN_H__


extern int (*rx_handler) (unsigned char,char*,int);
extern int (*tx_handler) (unsigned char,char*, unsigned int*, unsigned int*);

#endif
