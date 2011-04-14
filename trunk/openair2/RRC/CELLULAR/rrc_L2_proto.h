/***************************************************************************
                          rrc_L2_proto.h  -  description
                             -------------------
    begin                : Sept 9, 2008
    copyright            : (C) 2008 by Eurecom
    email                : Michelle.Wetterwald@eurecom.fr
 **************************************************************************
  Prototypes related to L2 interface functions
 ***************************************************************************/
#ifndef __RRC_L2_PROTO_H__
#define __RRC_L2_PROTO_H__

// rrc_L2_interfaces.c
unsigned char rrc_L2_data_req_rx (unsigned char Mod_id, unsigned short Srb_id, unsigned char Nb_tb,char *Buffer,u8 CH_index);
void rrc_L2_mac_data_ind_rx (void);
void rrc_L2_rlc_data_ind_rx (void);
void rrc_L2_mac_meas_ind_rx (void);
void rrc_L2_def_meas_ind_rx (unsigned char Mod_id, unsigned char Idx2);
void rrc_L2_sync_ind_rx (void);
void rrc_L2_out_sync_ind_rx (void);

#ifdef NODE_MT
//rrc_ue_init.c
void rrc_ue_init (u8 Mod_id);

//rrc_ue_main.c
void rrc_ue_main_scheduler (u8 Mod_id);
#endif

#ifdef NODE_RG
//rrc_rg_init.c
void rrc_rg_init (u8 Mod_id);

//rrc_rg_main.c
void rrc_rg_main_scheduler (u8 Mod_id);
#endif

#endif

