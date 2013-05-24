/***************************************************************************
                          rrc_rg_init.c
                          -------------------
    begin                : Someday 2008
    copyright            : (C) 2010 by Eurecom
    created by	         : Michelle.Wetterwald@eurecom.fr
 **************************************************************************
  Initialization of RRC protocol entity for Radio Gateway
 ***************************************************************************/
/********************
// OpenAir includes
 ********************/
#include "LAYER2/MAC/extern.h"
#include "COMMON/openair_defs.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "LAYER2/RLC/rlc.h"
#include "COMMON/mac_rrc_primitives.h"

//#include "SIMULATION/simulation_defs.h"
//extern EMULATION_VARS *Emul_vars;
//extern CH_MAC_INST *CH_mac_inst;
//extern UE_MAC_INST *UE_mac_inst;

/********************
// RRC includes
 ********************/
#include "rrc_rg_vars.h"
//-----------------------------------------------------------------------------
#include "rrc_proto_rrm.h"
#include "rrc_proto_int.h"
#include "rrc_L2_proto.h"
//-----------------------------------------------------------------------------
int *pt_nas_rg_irq;
u16 *pt_rg_own_cell_id;
#ifndef  USER_MODE
EXPORT_SYMBOL(pt_nas_rg_irq);
EXPORT_SYMBOL(pt_rg_own_cell_id);
#endif
LCHAN_DESC BCCH_LCHAN_DESC,CCCH_LCHAN_DESC,DCCH_LCHAN_DESC,DTCH_DL_LCHAN_DESC,DTCH_UL_LCHAN_DESC;
rlc_info_t Rlc_info_um;
rlc_info_t Rlc_info_am_config;

//-----------------------------------------------------------------------------
void rrc_rg_init (u8 Mod_id){
//-----------------------------------------------------------------------------
  int user;

  #ifdef DEBUG_RRC_STATE
  printk("\n\n***********************************************\n");
  printk("**************[RRC CELL][RG-INIT]**************\n");
  printk("***********************************************\n");
  #endif
  printk("[RRC CELL][RG-INIT] Init BS function start\n");

 // Initialize the control block for variables
  pool_buffer_init();
  protocol_bs = &prot_pool_bs;
  memset ((char *)protocol_bs, 0, sizeof (struct protocol_pool_bs));

  rrm_config = &rrc_as_config;
  memset ((char *)rrm_config, 0, sizeof (RRM_VARS));


  for (user = 0; user < maxUsers; user++) {
    protocol_bs->rrc.protocol_state[user] = RRC_RG_IDLE;
  }

#ifdef USER_MODE
  protocol_bs->rrc.rg_cell_id = 1;
#else
  protocol_bs->rrc.rg_cell_id = 5;      // Temp
#endif
  protocol_bs->rrc.mod_id =0;

  // initialise NAS global variables
  pt_nas_rg_irq = &(protocol_bs->rrc.ip_rx_irq);
  pt_rg_own_cell_id = &(protocol_bs->rrc.rg_cell_id);

  //rrc_rg_rrm_connected_init(); // Transferred to rrc_rg_rrm_process.c

  // Next is TEMP - Will be removed when RRM interface has dynamic MAC config
  rrc_init_mac_config();

#ifdef USER_MODE
  rrc_rg_sap_init ();      // init FIFOs towards NAS
  rrc_rg_rrm_sap_init ();  // init FIFOs towards RRM
#endif
  // init function pointers for RRM interface
  //init_rrc_handler();
#ifdef DEBUG_RRC_DETAILS
  //rrc_rg_init_check_qos_classes();
#endif

  printk("\n[RRC CELL][RG-INIT] cell_id %d\n",protocol_bs->rrc.rg_cell_id );
  printk("[RRC CELL][RG-INIT] Init RG function completed\n");
  printk("***********************************************\n\n\n");

 }

