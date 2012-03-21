/*________________________openair_rrc_L2_interface.h________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/

#ifndef __OPENAIR_RRC_L2_INTERFACE_H__
#define __OPENAIR_RRC_L2_INTERFACE_H__

#include "COMMON/mac_rrc_primitives.h"
 
s8 mac_rrc_data_req( u8 Mod_id, u32 frame, u16 Srb_id, u8 Nb_tb,s8 *Buffer,u8,u8);
s8 mac_rrc_data_ind( u8 Mod_id,  u32 frame, u16 Srb_id, s8 *Sdu, u16 Sdu_len,u8,u8 Mui);
void mac_lite_sync_ind( u8 Mod_id, u8 status);
void mac_rrc_meas_ind(u8,MAC_MEAS_REQ_ENTRY*);
void rlcrrc_data_ind( u8 Mod_id, u32 frame, unsigned int Rb_id, u32 sdu_size,u8 *Buffer);
void mac_out_of_sync_ind(u8 Mod_id,u32 frame,u16 CH_index);
#endif
