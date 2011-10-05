
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
#include "UTIL/LOG/log.h"
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif //PHY_EMUL

#include "SCHED/defs.h"

//#ifdef BIGPHYSAREA
//extern void *bigphys_malloc(int);
//#endif


/***********************************************************************/
void chbch_phy_sync_success(unsigned char Mod_id,unsigned char eNB_index){  //init as MR
/***********************************************************************/
  // msg("[MAC]Node %d, PHY SYNC to eNB_index %d\n",NODE_ID[Mod_id],eNB_index);
  if( (layer2_init_UE(Mod_id)==-1) || (Rrc_xface->openair_rrc_UE_init(Mod_id,eNB_index)==-1) )
    Mac_rlc_xface->Is_cluster_head[Mod_id]=2;

}

/***********************************************************************/
void mrbch_phy_sync_failure(unsigned char Mod_id, unsigned char Free_ch_index){//init as CH 
  /***********************************************************************/
  LOG_I(MAC,"FRAME %d: Node %d, NO PHY SYNC to master\n",mac_xface->frame,Mod_id);
  if((layer2_init_eNB(Mod_id, Free_ch_index)==-1) || ( Rrc_xface->openair_rrc_eNB_init(Mod_id)==-1))
    Mac_rlc_xface->Is_cluster_head[Mod_id]=2;
  
}

/***********************************************************************/
char layer2_init_eNB(unsigned char Mod_id, unsigned char eNB_index){
/***********************************************************************/

  Mac_rlc_xface->Is_cluster_head[Mod_id]=1;
  
  //  msg("\nMAC: INIT eNB %d Successful \n\n",Mod_id);

  return 0;
  
}

/***********************************************************************/
char layer2_init_UE(unsigned char Mod_id){
  /***********************************************************************/
  Mac_rlc_xface->Is_cluster_head[NB_eNB_INST + Mod_id]=0;
  
  return 0;
}

/***********************************************************************/
void mac_UE_out_of_sync_ind(unsigned char Mod_id, unsigned short eNB_index){
/***********************************************************************/

  Mac_rlc_xface->mac_out_of_sync_ind(Mod_id,eNB_index);
}
 

/***********************************************************************/
int mac_top_init(){
/***********************************************************************/
  unsigned char  Mod_id,i,j;  
  RA_TEMPLATE *RA_template;
  UE_TEMPLATE *UE_template;
  u8 SB_size;

  LOG_I(MAC,"[MAIN] Init function start:Nb_INST=%d, NODE_ID[0]=%d\n",NB_INST,NODE_ID[0]);
  if (NB_UE_INST>0) {
    UE_mac_inst = (UE_MAC_INST*)malloc16(NB_UE_INST*sizeof(UE_MAC_INST));
    LOG_D(MAC,"[MAIN] ALLOCATE %d Bytes for %d UE_MAC_INST @ %p\n",NB_UE_INST*sizeof(UE_MAC_INST),NB_UE_INST,UE_mac_inst);
    bzero(UE_mac_inst,NB_UE_INST*sizeof(UE_MAC_INST));
    ue_init_mac();
  }
  else 
    UE_mac_inst = NULL;
  if (NB_eNB_INST>0) {
    eNB_mac_inst = (eNB_MAC_INST*)malloc16(NB_eNB_INST*sizeof(eNB_MAC_INST));
    LOG_D(MAC,"[MAIN] ALLOCATE %d Bytes for eNB_MAC_INST @ %p\n",NB_eNB_INST*sizeof(eNB_MAC_INST),eNB_mac_inst);
    bzero(eNB_mac_inst,NB_eNB_INST*sizeof(eNB_MAC_INST));
  }
  else
    eNB_mac_inst = NULL;
  
  for(Mod_id=0;Mod_id<NB_INST;Mod_id++){

#ifdef PHY_EMUL
    Mac_rlc_xface->Is_cluster_head[Mod_id]=2;//0: MR, 1: CH, 2: not CH neither MR
#endif
    
    Mac_rlc_xface->Node_id[Mod_id]=NODE_ID[Mod_id];
  }
  Mac_rlc_xface->frame=Mac_rlc_xface->frame;
  
  
  if (Is_rrc_registered == 1){
    LOG_I(MAC,"[MAIN] calling RRC\n");
#ifndef CELLULAR //nothing to be done yet for cellular
    Rrc_xface->openair_rrc_top_init();
#endif 
  }
    else {
      LOG_I(MAC,"[MAIN] Running without an RRC\n");
    }
#ifndef USER_MODE
#ifndef PHY_EMUL
  LOG_I(MAC,"[MAIN] add openair2 proc\n");
////  add_openair2_stats();
#endif
#endif  
 
  init_transport_channels(2); 

  // Set up DCIs for TDD 5MHz Config 1..6
  for (i=0;i<NB_eNB_INST;i++) {
    LOG_D(MAC,"[MAIN][eNB %d] initializing RA_template\n",i);
    RA_template = (RA_TEMPLATE *)&eNB_mac_inst[i].RA_template[0];
    for (j=0;j<NB_RA_PROC_MAX;j++) {
      memcpy((void *)&RA_template[j].RA_alloc_pdu1[0],(void *)&RA_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
      memcpy((void *)&RA_template[j].RA_alloc_pdu2[0],(void *)&DLSCH_alloc_pdu1A,sizeof(DCI1A_5MHz_TDD_1_6_t));
      RA_template[j].RA_dci_size_bytes1 = sizeof(DCI1A_5MHz_TDD_1_6_t);
      RA_template[j].RA_dci_size_bytes2 = sizeof(DCI1A_5MHz_TDD_1_6_t);
      RA_template[j].RA_dci_size_bits1  = sizeof_DCI1A_5MHz_TDD_1_6_t;
      RA_template[j].RA_dci_size_bits2  = sizeof_DCI1A_5MHz_TDD_1_6_t;
      RA_template[j].RA_dci_fmt1        = format1A;
      RA_template[j].RA_dci_fmt2        = format1A;
    }


    UE_template = (UE_TEMPLATE *)&eNB_mac_inst[i].UE_template[0];
    for (j=0;j<NB_CNX_eNB;j++) {
      UE_template->rnti=0;
    }    
  }


 //ICIC init param
#ifdef ICIC
  SB_size=mac_xface->get_SB_size(mac_xface->lte_frame_parms->N_RB_DL);

  srand (time(NULL));

  for(j=0;j<NB_eNB_INST;j++){
	  eNB_mac_inst[j].sbmap_conf.first_subframe=0;
	  eNB_mac_inst[j].sbmap_conf.periodicity=10;
	  eNB_mac_inst[j].sbmap_conf.sb_size=SB_size;
	  eNB_mac_inst[j].sbmap_conf.nb_active_sb=1;
	  for(i=0;i<NUMBER_OF_SUBBANDS;i++)
		  eNB_mac_inst[j].sbmap_conf.sbmap[i]=1;

	  eNB_mac_inst[j].sbmap_conf.sbmap[rand()%NUMBER_OF_SUBBANDS]=0;

  }
#endif
//end ALU's algo

   LOG_I(MAC,"[MAIN][INIT] Init function finished\n");
  
  return(0);
  
}


/***********************************************************************/
int mac_init_global_param(){
  /***********************************************************************/

  Is_rrc_registered=0;  
  Mac_rlc_xface = NULL;
  LOG_I(MAC,"[MAIN] CALLING RLC_MODULE_INIT...\n");	

  if (rlc_module_init()!=0)
    return(-1);

  LOG_I(MAC,"[MAIN] RLC_MODULE_INIT OK, malloc16 for mac_rlc_xface...\n");	
  
  Mac_rlc_xface = (MAC_RLC_XFACE*)malloc16(sizeof(MAC_RLC_XFACE));
  bzero(Mac_rlc_xface,sizeof(MAC_RLC_XFACE));
  
  if(Mac_rlc_xface == NULL){
    LOG_E(MAC,"[MAIN] FATAL EROOR: Could not allocate memory for Mac_rlc_xface !!!\n");
    return (-1);
    
  }	

  LOG_I(MAC,"[MAIN] malloc16 OK, mac_rlc_xface @ %p\n",(void *)Mac_rlc_xface);  

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

  LOG_I(MAC,"[MAIN] INIT_GLOBAL_PARAM: Mac_rlc_xface=%p,rrc_rlc_register_rrc =%p\n",Mac_rlc_xface,Mac_rlc_xface->rrc_rlc_register_rrc); 
  
  Mac_rlc_xface->mac_rlc_data_req=mac_rlc_data_req;
  Mac_rlc_xface->mac_rlc_data_ind=mac_rlc_data_ind;
  Mac_rlc_xface->mac_rlc_status_ind=mac_rlc_status_ind;
  Mac_rlc_xface->pdcp_run=pdcp_run;
  Mac_rlc_xface->pdcp_data_req=pdcp_data_req;	
  Mac_rlc_xface->mrbch_phy_sync_failure=mrbch_phy_sync_failure;
  Mac_rlc_xface->chbch_phy_sync_success=chbch_phy_sync_success;

  LOG_I(MAC,"[MAIN] RLC interface setup and init\n");
  rrc_init_global_param();
  Is_rrc_registered=1;
#ifdef USER_MODE
  pdcp_layer_init ();
#else
  pdcp_module_init ();
#endif
  mac_xface->out_of_sync_ind=mac_UE_out_of_sync_ind;  
  LOG_I(MAC,"[MAIN] Init Global Param Done\n");

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

  LOG_I(MAC,"[MAIN] MAC_INIT_GLOBAL_PARAM IN...\n");
  //    NB_NODE=2; 
  //    NB_INST=2;


  mac_init_global_param(); 
  
  
  mac_xface->macphy_init=(void (*)(void))mac_top_init;
#ifndef USER_MODE
  mac_xface->macphy_exit = openair_sched_exit;
#else
  mac_xface->macphy_exit=(void (*)(const char*)) exit;
#endif
  LOG_I(MAC,"[MAIN] init eNB MAC functions  \n");
  mac_xface->eNB_dlsch_ulsch_scheduler = (void *)eNB_dlsch_ulsch_scheduler;
  mac_xface->get_dci_sdu               = (DCI_PDU* (*)(u8,u8))get_dci_sdu;
  mac_xface->fill_rar                  = fill_rar;
  mac_xface->terminate_ra_proc         = terminate_ra_proc;
  mac_xface->initiate_ra_proc          = initiate_ra_proc;
  mac_xface->cancel_ra_proc            = cancel_ra_proc;
  mac_xface->rx_sdu                    = rx_sdu;
  mac_xface->get_dlsch_sdu             = get_dlsch_sdu;
  mac_xface->get_eNB_UE_stats          = get_eNB_UE_stats;
  mac_xface->get_transmission_mode     = (u8 (*)(u16))get_transmission_mode;
  mac_xface->get_rballoc               = (u32 (*)(u8,u8))get_rballoc;
  mac_xface->get_nb_rb                 = (u16 (*)(u8,u32))conv_nprb;
  mac_xface->get_SB_size	      	   = Get_SB_size;



  LOG_I(MAC,"[MAIN] init UE MAC functions \n");
  mac_xface->ue_decode_si              = ue_decode_si;
  mac_xface->ue_send_sdu               = ue_send_sdu;
  mac_xface->ue_get_SR                 = ue_get_SR;
  mac_xface->ue_get_sdu                = (void *)ue_get_sdu;
  mac_xface->ue_get_rach               = ue_get_rach;
  mac_xface->ue_process_rar            = ue_process_rar;
  mac_xface->ue_scheduler              = ue_scheduler;  

  
  LOG_I(MAC,"[MAIN] PHY Frame configuration \n");
  mac_xface->lte_frame_parms = frame_parms;
  
  mac_xface->get_ue_active_harq_pid = get_ue_active_harq_pid;
  mac_xface->computeRIV             = computeRIV;
  mac_xface->get_TBS                = get_TBS;
  mac_xface->get_nCCE_max           = get_nCCE_max;
  mac_xface->get_ue_mode            = get_ue_mode;
  mac_xface->phy_config_sib1_eNB    = phy_config_sib1_eNB;
  mac_xface->phy_config_sib1_ue    = phy_config_sib1_ue;

  mac_xface->phy_config_sib2_eNB        = phy_config_sib2_eNB;
  mac_xface->phy_config_sib2_ue         = phy_config_sib2_ue;

  mac_xface->phy_config_dedicated_eNB   = phy_config_dedicated_eNB;
  mac_xface->phy_config_dedicated_ue    = phy_config_dedicated_ue;

  LOG_D(MAC,"[MAIN] ALL INIT OK\n");
   
  mac_xface->frame=0;
  
  mac_xface->macphy_init();

  //Mac_rlc_xface->Is_cluster_head[0] = 1;
  //Mac_rlc_xface->Is_cluster_head[1] = 0;

  return(1);
}

