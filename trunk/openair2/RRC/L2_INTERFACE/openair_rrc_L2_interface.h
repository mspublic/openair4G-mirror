/*________________________openair_rrc_L2_interface.h________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/

#ifndef __OPENAIR_RRC_L2_INTERFACE_H__
#define __OPENAIR_RRC_L2_INTERFACE_H__

#include "COMMON/mac_rrc_primitives.h"
 
unsigned char mac_rrc_data_req( unsigned char Mod_id, unsigned short Srb_id, unsigned char Nb_tb,char *Buffer,u8,u8);
unsigned char mac_rrc_data_ind( unsigned char Mod_id,  unsigned short Srb_id, char *Sdu, unsigned short Sdu_len,u8,unsigned char Mui);
void mac_lite_sync_ind( unsigned char Mod_id, unsigned char status);
void mac_rrc_meas_ind(unsigned char,MAC_MEAS_REQ_ENTRY*);
void rlcrrc_data_ind( unsigned char Mod_id, unsigned int Rb_id, unsigned int sdu_size,u8 *Buffer);
void mac_out_of_sync_ind(unsigned char Mod_id,unsigned short CH_index);
#endif
