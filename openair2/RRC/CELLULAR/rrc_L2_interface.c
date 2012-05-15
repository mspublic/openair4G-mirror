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
#include "rrc_L2_proto.h"
#ifdef NODE_MT
#include "rrc_ue_vars.h"
#endif
#ifdef NODE_RG
#include "rrc_rg_vars.h"
#endif

//-----------------------------------------------------------------------------
s8 rrc_L2_data_req_rx (unsigned char Mod_id, unsigned short Srb_id, unsigned char Nb_tb,char *Buffer,u8 CH_index){
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
s8 rrc_L2_mac_data_ind_rx (void){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_data_req_rx - begin\n");
#endif
return 0;
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
void rrc_L2_def_meas_ind_rx (unsigned char Mod_id, unsigned char Idx2){
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

//-----------------------------------------------------------------------------
int rrc_L2_get_rrc_status(u8 Mod_id,u8 eNB_flag,u8 index){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_get_rrc_status - begin\n");
#endif
/*
  if(eNB_flag == 1)
    return(eNB_rrc_inst[Mod_id].Info.Status[index]);
  else
    return(UE_rrc_inst[Mod_id].Info[index].Status);
*/
   return 0;
}

//-----------------------------------------------------------------------------
char rrc_L2_ue_init(u8 Mod_id, unsigned char eNB_index){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_ue_init - begin\n");
#endif
#ifdef NODE_MT
    rrc_ue_init (Mod_id);
#endif

#ifdef NODE_RG
    rrc_rg_uelite_init(Mod_id, eNB_index);
#endif
    return 0;
}


//-----------------------------------------------------------------------------
char rrc_L2_eNB_init(u8 Mod_id){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_eNB_init - begin\n");
#endif
#ifdef NODE_MT
    rrc_ue_rglite_init(Mod_id, 0);
#endif

#ifdef NODE_RG
    rrc_rg_init (Mod_id);
#endif
    return 0;
}

//-----------------------------------------------------------------------------
// Out of openair_rrc_L2_interface.c
void openair_rrc_lite_top_init(void){
//-----------------------------------------------------------------------------
  //#ifdef DEBUG_RRC_STATE
   msg ("\n[RRC CELL] [L2_INTF] openair_rrc_lite_top_init - Empty function to keep compatibility with RRC LITE\n\n");
  //#endif
}

//-----------------------------------------------------------------------------
RRC_status_t rrc_rx_tx(u8 Mod_id,u32 frame, u8 eNB_flag,u8 index){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_eNB_init - begin\n");
#endif
#ifdef NODE_MT
    rrc_ue_main_scheduler(Mod_id, frame, 0, index);
#endif

#ifdef NODE_RG
    rrc_rg_main_scheduler(Mod_id, frame, 0, 0);
#endif
    return 0;
}
