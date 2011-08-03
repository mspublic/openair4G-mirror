/***************************************************************************
                          rrc_L2_interface.c  -
                          -------------------
    begin                : Sept 9, 2008
    copyright            : (C) 2008 by Eurecom
    created by           : michelle.wetterwald@eurecom.fr
    description
 **************************************************************************
    Entry point for L2 interfaces
 ***************************************************************************/
/********************
//OpenAir definitions
 ********************/
#include "LAYER2/MAC/extern.h"
#include "UTIL/MEM/mem_block.h"
//-----------------------------------------------------------------------------
/********************
// RRC definitions
 ********************/
//#include "rrc_constant.h"
//#include "rrc_ue_vars_ms_extern.h"
#ifdef NODE_MT
#include "rrc_ue_vars.h"
#endif
#ifdef NODE_RG
#include "rrc_rg_vars.h"
#endif

//-----------------------------------------------------------------------------
unsigned char rrc_L2_data_req_rx (unsigned char Mod_id, unsigned short Srb_id, unsigned char Nb_tb,char *Buffer,u8 CH_index){
//-----------------------------------------------------------------------------
  char *rrc_sim_data = "TESTING\0";
  int br_size=0;

#ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC CELL][L2_INTF] rrc_L2_data_req_rx - begin\n");
  msg ("Received parameters Mod_id %d, Srb_id %d, nb_tb %d, CH_index %d\n",Mod_id, Srb_id,Nb_tb,CH_index);
  //msg("[RRC CELL][DEBUG] cell_id %d\n",protocol_ms->rrc.cell_id );
#endif
  br_size = strlen(rrc_sim_data);
  if( Mac_rlc_xface->Is_cluster_head[Mod_id]){
    if((Srb_id & RAB_OFFSET) == BCCH){
       memcpy(Buffer,&(rrc_sim_data[0]),br_size);
#ifdef DEBUG_RRC_DETAILS
       msg ("[RRC CELL][L2_INTF] rrc_L2_data_req_rx - end BCCH,  br_size %d\n",br_size);
#endif
       return br_size;
    }
  }
#ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC CELL][L2_INTF] rrc_L2_data_req_rx - end, br_size %d\n", br_size);
#endif
  return br_size;
}

//-----------------------------------------------------------------------------
void rrc_L2_mac_data_ind_rx (void){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_data_req_rx - begin\n");
#endif
}

//-----------------------------------------------------------------------------
void rrc_L2_rlc_data_ind_rx (void){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_rlc_data_ind_rx - begin\n");
#endif
}

//-----------------------------------------------------------------------------
void rrc_L2_mac_meas_ind_rx (void){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_mac_meas_ind_rx - begin\n");
#endif
}

//-----------------------------------------------------------------------------
void rrc_L2_def_meas_ind_rx (unsigned char Mod_id, unsigned charIdx2){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_def_meas_ind_rx - begin\n");
#endif
}

//-----------------------------------------------------------------------------
void rrc_L2_sync_ind_rx (void){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_sync_ind_rx - begin\n");
#endif
}

//-----------------------------------------------------------------------------
void rrc_L2_out_sync_ind_rx (void){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrrc_L2_out_sync_ind_rx - begin\n");
#endif
}

