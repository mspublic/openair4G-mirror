
/*________________________mac_main.c________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/

#ifdef USER_MODE
#include "LAYER2/register.h"
#else
#ifdef KERNEL2_6
//#include <linux/config.h>
#include <linux/slab.h>
#endif

#ifdef KERNEL2_4
#include <linux/malloc.h>
#include <linux/wrapper.h>
#endif


#endif //USER_MODE

#include "defs.h"
#include "extern.h"
#include "PHY_INTERFACE/extern.h"
#include "PHY_INTERFACE/defs.h"
#include "PHY/defs.h"
#include "SCHED/defs.h"
#include "LAYER2/PDCP/pdcp.h"
#include "RRC/LITE/defs.h"
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif //PHY_EMUL

#include "SCHED/defs.h"

//#ifdef BIGPHYSAREA
//extern void *bigphys_malloc(int);
//#endif


/***********************************************************************/
void chbch_phy_sync_success(unsigned char Mod_id,unsigned char CH_index){  //init as MR
/***********************************************************************/
  // msg("[MAC]Node %d, PHY SYNC to CH_index %d\n",NODE_ID[Mod_id],CH_index);
  if( (layer2_init_mr(Mod_id)==-1) || (Rrc_xface->openair_rrc_mr_init(Mod_id,CH_index)==-1) )
    Mac_rlc_xface->Is_cluster_head[Mod_id]=2;

  //  UE_mac_inst[Mod_id-NB_CH_INST].Bcch_lchan[CH_index].Active=1;
  //  UE_mac_inst[Mod_id-NB_CH_INST].Bcch_lchan[CH_index].Lchan_info.Lchan_id.Index=BCCH;
  //  UE_mac_inst[Mod_id-NB_CH_INST].Ccch_lchan[CH_index].Active=1;  
  //  UE_mac_inst[Mod_id-NB_CH_INST].Ccch_lchan[CH_index].Lchan_info.Lchan_id.Index=CCCH;
}

/***********************************************************************/
void mrbch_phy_sync_failure(unsigned char Mod_id, unsigned char Free_ch_index){//init as CH 
  /***********************************************************************/
  msg("[MAC]FRAME %d: Node %d, NO PHY SYNC to CH\n",mac_xface->frame,NODE_ID[Mod_id]);
  if((layer2_init_ch(Mod_id, Free_ch_index)==-1) || ( Rrc_xface->openair_rrc_ch_init(Mod_id)==-1))
    Mac_rlc_xface->Is_cluster_head[Mod_id]=2;
  
}

/***********************************************************************/
char layer2_init_ch(unsigned char Mod_id, unsigned char CH_index){
/***********************************************************************/
  unsigned char  i,j,k,Nb_mod;
  Mac_rlc_xface->Is_cluster_head[Mod_id]=1;
  
  msg("\nMAC: INIT CH %d Successful \n\n",NODE_ID[Mod_id]);

  return 0;
  
}

/***********************************************************************/
char layer2_init_mr(unsigned char Mod_id){
  /***********************************************************************/
  unsigned char  i,j,k,Nb_mod,CH_index;
  Nb_mod=Mod_id-NB_CH_INST;
  Mac_rlc_xface->Is_cluster_head[Mod_id]=0;
  
  return 0;
}

/***********************************************************************/
void mac_UE_out_of_sync_ind(unsigned char Mod_id, unsigned short CH_index){
/***********************************************************************/

  unsigned char j;
  Mod_id-=NB_CH_INST;

  Mac_rlc_xface->mac_out_of_sync_ind(Mod_id,CH_index);
}
 

/***********************************************************************/
int mac_top_init(){
/***********************************************************************/
  unsigned char  Mod_id,i,j;  
  RA_TEMPLATE *RA_template;
  UE_TEMPLATE *UE_template;

  msg("[OPENAIR][MAC INIT] Init function start:Nb_INST=%d, NODE_ID[0]=%d\n",NB_INST,NODE_ID[0]);
#if ((PHY_EMUL==1)||(PHYSIM==1))
  UE_mac_inst = (UE_MAC_INST*)malloc16(NB_UE_INST*sizeof(UE_MAC_INST));
  msg("ALLOCATE %d Bytes for %d UE_MAC_INST @ %p\n",NB_UE_INST*sizeof(UE_MAC_INST),NB_UE_INST,UE_mac_inst);
  bzero(UE_mac_inst,NB_UE_INST*sizeof(UE_MAC_INST));
  CH_mac_inst = (CH_MAC_INST*)malloc16(NB_CH_INST*sizeof(CH_MAC_INST));
  msg("ALLOCATE %d Bytes for CH_MAC_INST @ %p\n",NB_CH_INST*sizeof(CH_MAC_INST),CH_mac_inst);
  bzero(CH_mac_inst,NB_CH_INST*sizeof(CH_MAC_INST));
#else 
  //  if(NODE_ID[0]<NB_CH_MAX){
  if(NODE_ID[0]<NUMBER_OF_eNB_MAX){
    CH_mac_inst = (CH_MAC_INST*)malloc16(sizeof(CH_MAC_INST));
    msg("ALLOCATE %d Bytes for CH_MAC_INST @ %p\n",NB_CH_INST*sizeof(CH_MAC_INST),CH_mac_inst);
    bzero(CH_mac_inst,NB_CH_INST*sizeof(CH_MAC_INST));
    NB_CH_INST=1;
    NB_UE_INST=0;
  }
  else{
    UE_mac_inst = (UE_MAC_INST*)malloc16(sizeof(UE_MAC_INST));
    msg("ALLOCATE %d Bytes for UE_MAC_INST @ %p\n",NB_UE_INST*sizeof(UE_MAC_INST),UE_mac_inst);
    bzero(UE_mac_inst,NB_UE_INST*sizeof(UE_MAC_INST));
    NB_CH_INST=0;
    NB_UE_INST=1;
  }
#endif
  msg("init---------------------\n");
  for(Mod_id=0;Mod_id<NB_INST;Mod_id++){

#ifdef PHY_EMUL
    Mac_rlc_xface->Is_cluster_head[Mod_id]=2;//0: MR, 1: CH, 2: not CH neither MR
#endif
    
    Mac_rlc_xface->Node_id[Mod_id]=NODE_ID[Mod_id];
  }
  Mac_rlc_xface->frame=Mac_rlc_xface->frame;
  
  
  if (Is_rrc_registered == 1){
    msg("calling RRC\n");
#ifndef CELLULAR //nothing to be done yet for cellular
    Rrc_xface->openair_rrc_top_init();
#endif 
  }
    else {
      msg("[OPENAIR][MAC] Running without an RRC\n");
    }
#ifndef USER_MODE
#ifndef PHY_EMUL
  msg("add openair2 proc\n");
////  add_openair2_stats();
#endif
#endif  
 
  init_transport_channels(2); 

  // Set up DCIs for TDD 5MHz Config 1..6
  for (i=0;i<NB_CH_INST;i++) {
    RA_template = (RA_TEMPLATE *)&CH_mac_inst[i].RA_template[0];
    for (j=0;j<NB_RA_PROC_MAX;j++) {
      memcpy((void *)&RA_template[j].RA_alloc_pdu1[0],(void *)&RA_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
      memcpy((void *)&RA_template[j].RA_alloc_pdu2[0],(void *)&DLSCH_alloc_pdu1A,sizeof(DCI1A_5MHz_TDD_1_6_t));
      RA_template[i].RA_dci_size_bytes1 = sizeof(DCI1A_5MHz_TDD_1_6_t);
      RA_template[i].RA_dci_size_bytes2 = sizeof(DCI1A_5MHz_TDD_1_6_t);
      RA_template[i].RA_dci_size_bits1  = sizeof_DCI1A_5MHz_TDD_1_6_t;
      RA_template[i].RA_dci_size_bits2  = sizeof_DCI1A_5MHz_TDD_1_6_t;
      RA_template[i].RA_dci_fmt1        = format1A;
      RA_template[i].RA_dci_fmt2        = format1A;
    }


    UE_template = (UE_TEMPLATE *)&CH_mac_inst[i].UE_template[0];
    for (j=0;j<NB_CNX_CH;j++) {
      UE_template->rnti=0;
    }    
  }
  msg("[OPENAIR][MAC][INIT] Init function finished\n");
  
  return(0);
  
}


/***********************************************************************/
int mac_init_global_param(){
  /***********************************************************************/

  int i; 
  //  msg("[MAC] Init Global Param In, CHBCH_PDU_SIZE %d ...\n",sizeof(CHBCH_PDU));
  //  if(sizeof(CHBCH_PDU) > 140){
  //    msg("Size of CHBCH_PDU= %d, fix this !!!\n",sizeof(CHBCH_PDU));
  //    return -1;
  //  }  

  Is_rrc_registered=0;  
  Mac_rlc_xface = NULL;
  msg("[MAC] CALLING RLC_MODULE_INIT...\n");	

  if (rlc_module_init()!=0)
    return(-1);

  msg("[MAC] RLC_MODULE_INIT OK, malloc16 for mac_rlc_xface...\n");	
  
  Mac_rlc_xface = (MAC_RLC_XFACE*)malloc16(sizeof(MAC_RLC_XFACE));
  
  if(Mac_rlc_xface == NULL){
    msg("[MAC] FATAL EROOR: Could not allocate memory for Mac_rlc_xface !!!\n");
    return (-1);
    
  }	

  msg("[MAC] malloc16 OK, mac_rlc_xface @ %p\n",(void *)Mac_rlc_xface);  

  //  mac_xface->macphy_data_ind=macphy_data_ind;
  mac_xface->mrbch_phy_sync_failure=mrbch_phy_sync_failure;
  mac_xface->chbch_phy_sync_success=chbch_phy_sync_success;
  Mac_rlc_xface->macphy_exit=  mac_xface->macphy_exit;
  Mac_rlc_xface->frame = 0;
  //  Mac_rlc_xface->mac_config_req=mac_config_req;
  //  Mac_rlc_xface->mac_meas_req=mac_meas_req;
  Mac_rlc_xface->rrc_rlc_config_req=rrc_rlc_config_req;
  Mac_rlc_xface->rrc_rlc_data_req=rrc_rlc_data_req;
  Mac_rlc_xface->rrc_rlc_register_rrc=rrc_rlc_register_rrc;

  Mac_rlc_xface->rrc_mac_config_req=rrc_mac_config_req;

  msg("[MAC]INIT_GLOBAL_PARAM: Mac_rlc_xface=%p,rrc_rlc_register_rrc =%p\n",Mac_rlc_xface,Mac_rlc_xface->rrc_rlc_register_rrc); 
  
  Mac_rlc_xface->mac_rlc_data_req=mac_rlc_data_req;
  Mac_rlc_xface->mac_rlc_data_ind=mac_rlc_data_ind;
  Mac_rlc_xface->mac_rlc_status_ind=mac_rlc_status_ind;
  Mac_rlc_xface->pdcp_run=pdcp_run;
  Mac_rlc_xface->pdcp_data_req=pdcp_data_req;	
  Mac_rlc_xface->mrbch_phy_sync_failure=mrbch_phy_sync_failure;
  Mac_rlc_xface->chbch_phy_sync_success=chbch_phy_sync_success;
  msg("[MAC][RLC] interface setup\n");

  msg("[MAC][GLOBAL_INIT] RRC_INIT_GLOBAL\n");
  rrc_init_global_param();
  Is_rrc_registered=1;
#ifdef USER_MODE
  pdcp_layer_init ();
#else
  pdcp_module_init ();
#endif
  mac_xface->out_of_sync_ind=mac_UE_out_of_sync_ind;  
  msg("[MAC] Init Global Param Done\n");

  return 0;
}


/***********************************************************************/
void mac_top_cleanup(void){
/***********************************************************************/
#ifndef USER_MODE
  pdcp_module_cleanup ();
#endif

}

int l2_init(LTE_DL_FRAME_PARMS *frame_parms) {

  s32 ret;
  s32 ue_id;

  msg("[MAIN]MAC_INIT_GLOBAL_PARAM IN...\n");
  //    NB_NODE=2; 
  //    NB_INST=2;


  mac_init_global_param(); 
  
  
  mac_xface->macphy_init=mac_top_init;
#ifndef USER_MODE
  mac_xface->macphy_exit = openair_sched_exit;
#else
  mac_xface->macphy_exit=(void (*)(void)) exit;
#endif
  
  //eNB MAC functions    
  mac_xface->eNB_dlsch_ulsch_scheduler = eNB_dlsch_ulsch_scheduler;
  mac_xface->get_dci_sdu               = get_dci_sdu;
  mac_xface->fill_rar                  = fill_rar;
  mac_xface->terminate_ra_proc         = terminate_ra_proc;
  mac_xface->initiate_ra_proc          = initiate_ra_proc;
  mac_xface->cancel_ra_proc            = cancel_ra_proc;
  mac_xface->rx_sdu                    = rx_sdu;
  mac_xface->get_dlsch_sdu             = get_dlsch_sdu;
  mac_xface->get_eNB_UE_stats          = get_eNB_UE_stats;
  mac_xface->get_transmission_mode     = get_transmission_mode;
  mac_xface->get_rballoc               = get_rballoc;
  mac_xface->get_nb_rb                 = conv_nprb;

  //UE MAC functions    
  mac_xface->ue_decode_si              = ue_decode_si;
  mac_xface->ue_send_sdu               = ue_send_sdu;
  mac_xface->ue_get_sdu                = ue_get_sdu;
  mac_xface->ue_get_rach               = ue_get_rach;
  mac_xface->ue_process_rar            = ue_process_rar;
  mac_xface->ue_scheduler              = ue_scheduler;  

  
  // PHY Frame configuration
  mac_xface->lte_frame_parms = frame_parms;
  
  // PHY Helper functions
  mac_xface->get_ue_active_harq_pid = get_ue_active_harq_pid;
  mac_xface->computeRIV             = computeRIV;
  mac_xface->get_TBS                = get_TBS;
  mac_xface->get_nCCE_max           = get_nCCE_max;

  mac_xface->phy_config_sib1_eNB    = phy_config_sib1_eNB;
  mac_xface->phy_config_sib1_ue    = phy_config_sib1_ue;

  mac_xface->phy_config_sib2_eNB        = phy_config_sib2_eNB;
  mac_xface->phy_config_sib2_ue         = phy_config_sib2_ue;

  mac_xface->phy_config_dedicated_eNB   = phy_config_dedicated_eNB;
  mac_xface->phy_config_dedicated_ue    = phy_config_dedicated_ue;

  msg("ALL INIT OK\n");
   
  mac_xface->frame=0;
  
  mac_xface->macphy_init();

  //Mac_rlc_xface->Is_cluster_head[0] = 1;
  //Mac_rlc_xface->Is_cluster_head[1] = 0;

  return(1);
}

