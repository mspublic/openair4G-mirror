
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
#include "LAYER2/PDCP/pdcp.h"
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif //PHY_EMUL

#ifdef BIGPHYSAREA
extern void *bigphys_malloc(int);
#endif


/***********************************************************************/
void chbch_phy_sync_success(unsigned char Mod_id,unsigned char CH_index){  //init as MR
/***********************************************************************/
  msg("[MAC]Node %d, PHY SYNC to CH_index %d\n",NODE_ID[Mod_id],CH_index);
  if( (layer2_init_mr(Mod_id)==-1) || (Rrc_xface->openair_rrc_mr_init(Mod_id,CH_index)==-1) )
    Mac_rlc_xface->Is_cluster_head[Mod_id]=2;
  
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
  
  if (clear_lchan_table(&CH_mac_inst[Mod_id].Bcch_lchan,NB_SIG_CNX_CH) == -1)
    return(-1);
  if (clear_lchan_table(&CH_mac_inst[Mod_id].Ccch_lchan,NB_SIG_CNX_CH) == -1)
    return(-1);
  if (clear_lchan_table(CH_mac_inst[Mod_id].Dcch_lchan,NB_CNX_CH+1) == -1)
    return(-1);
  for(i=0;i<NB_RAB_MAX;i++){ 
    if (clear_lchan_table(CH_mac_inst[Mod_id].Dtch_lchan[i],NB_CNX_CH+1) == -1)
      return(-1);
    for(j=0;j<(NB_CNX_CH+1);j++)
      for(k=0;k<(NB_CNX_CH-1);k++){
	CH_mac_inst[Mod_id].Dtch_dil_lchan[i][j][k].Active = 0;
	CH_mac_inst[Mod_id].Dtch_dil_lchan[i][j][k].Lchan_info_dil.Meas_entry.Status=IDLE;
      }
  }
  CH_mac_inst[Mod_id].Nb_rx_sched[0]=0;
  CH_mac_inst[Mod_id].Nb_rx_sched[1]=0;
  CH_mac_inst[Mod_id].Nb_rx_sched[2]=0;
  memcpy(&CH_mac_inst[Mod_id].Bcch_lchan.Lchan_info.Phy_resources_tx,
	 (PHY_RESOURCES*)&CHBCH_PHY_RESOURCES[CH_index],sizeof(PHY_RESOURCES));
  //  CH_mac_inst[Mod_id].RX_rach_pdu.Rach_payload=(char*)malloc16(RACH_PAYLOAD_SIZE_MAX);
  CH_mac_inst[Mod_id].Node_id=NODE_ID[Mod_id];
  for(i=0;i<(NB_CNX_CH+1);i++){
    CH_mac_inst[Mod_id].Def_meas[i].Status=IDLE;
	CH_mac_inst[Mod_id].Def_meas[i].Active=0;
  }
  msg("\nMAC: INIT CH %d Successful \n\n",NODE_ID[Mod_id]);
  return 0;
  
}

/***********************************************************************/
char layer2_init_mr(unsigned char Mod_id){
  /***********************************************************************/
  unsigned char  i,j,k,Nb_mod,CH_index;
  Nb_mod=Mod_id-NB_CH_INST;
  Mac_rlc_xface->Is_cluster_head[Mod_id]=0;
  
  for(CH_index =0; CH_index < NB_CNX_UE;CH_index++){
    if (clear_lchan_table(&UE_mac_inst[Nb_mod].Bcch_lchan[CH_index],1) == -1)
      return(-1);
    if (clear_lchan_table(&UE_mac_inst[Nb_mod].Ccch_lchan[CH_index],1) == -1)
      return(-1);
    if (clear_lchan_table(&UE_mac_inst[Nb_mod].Dcch_lchan[CH_index],1) == -1)
      return(-1);
    
    for(i=0;i<NB_RAB_MAX;i++){ 
      if ( clear_lchan_table(&UE_mac_inst[Nb_mod].Dtch_lchan[i][CH_index],1) == -1)
	return(-1);
      for(k=0;k<(NB_CNX_CH-1);k++){
	UE_mac_inst[Nb_mod].Dtch_dil_lchan[i][CH_index][k].Active = 0;
	UE_mac_inst[Nb_mod].Dtch_dil_lchan[i][CH_index][k].Lchan_info.Meas_entry.Status=IDLE;
      }
    }
    UE_mac_inst[Nb_mod].Nb_rx_sched[CH_index][0]=0;
    UE_mac_inst[Nb_mod].Nb_rx_sched[CH_index][1]=0;
    UE_mac_inst[Nb_mod].NB_decoded_chbch =0;
    UE_mac_inst[Nb_mod].CH_ul_freq_map[CH_index]=0;
    for(j=0;j<3;j++){
      UE_mac_inst[Nb_mod].Nb_tx_ops[CH_index][j]=0;
    }
    memcpy(&UE_mac_inst[Nb_mod].Bcch_lchan[CH_index].Lchan_info.Phy_resources_rx,
	   (PHY_RESOURCES*)&CHBCH_PHY_RESOURCES[CH_index],sizeof(PHY_RESOURCES));
    UE_mac_inst[Nb_mod].Bcch_lchan[CH_index].Lchan_info.Lchan_id.Index=(CH_index << RAB_SHIFT2)+BCCH;
    UE_mac_inst[Nb_mod].Ccch_lchan[CH_index].Lchan_info.Lchan_id.Index=(CH_index << RAB_SHIFT2)+CCCH;
    UE_mac_inst[Nb_mod].Node_id=NODE_ID[Nb_mod];
    UE_mac_inst[Nb_mod].Def_meas[CH_index].Status=IDLE;
    UE_mac_inst[Nb_mod].Def_meas[CH_index].Active=0;
  }
  return 0;
}

/***********************************************************************/
void mac_UE_out_of_sync_ind(unsigned char Mod_id, unsigned short CH_index){
/***********************************************************************/

  unsigned char j;
  Mod_id-=NB_CH_INST;
  UE_mac_inst[Mod_id].Nb_rx_sched[CH_index][0]=0;
  UE_mac_inst[Mod_id].Nb_rx_sched[CH_index][1]=0;
  UE_mac_inst[Mod_id].NB_decoded_chbch =0;
  UE_mac_inst[Mod_id].CH_ul_freq_map[CH_index]=0;
  for(j=0;j<3;j++){
    UE_mac_inst[Mod_id].Nb_tx_ops[CH_index][j]=0;
  }
  UE_mac_inst[Mod_id].Def_meas[CH_index].Status=IDLE;
  UE_mac_inst[Mod_id].Def_meas[CH_index].Active=0;
  Mac_rlc_xface->mac_out_of_sync_ind(Mod_id,CH_index);
}


/***********************************************************************/
int mac_top_init(){
/***********************************************************************/
  unsigned char  Mod_id,i;  
  printk("[OPENAIR][MAC INIT] Init function start:Nb_INST=%d\n",NB_INST);
#if ((PHY_EMUL==1)||(PHYSIM==1))
  UE_mac_inst = (UE_MAC_INST*)malloc16(NB_UE_INST*sizeof(UE_MAC_INST));
  printk("ALLOCATE %d Bytes for %d UE_MAC_INST @ %p\n",NB_UE_INST*sizeof(UE_MAC_INST),NB_UE_INST,UE_mac_inst);
  CH_mac_inst = (CH_MAC_INST*)malloc16(NB_CH_INST*sizeof(CH_MAC_INST));
  printk("ALLOCATE %d Bytes for CH_MAC_INST @ %p\n",NB_CH_INST*sizeof(CH_MAC_INST),CH_mac_inst);
#else 
  if(NODE_ID[0]<NB_CH_MAX){
    CH_mac_inst = (CH_MAC_INST*)malloc16(sizeof(CH_MAC_INST));
    NB_CH_INST=1;
    NB_UE_INST=0;
  }
  else{
    UE_mac_inst = (UE_MAC_INST*)malloc16(sizeof(UE_MAC_INST));
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
    printk("calling RRC\n");
#ifndef CELLULAR //nothing to be done yet for cellular
    Rrc_xface->openair_rrc_top_init();
#endif 
  }
    else {
      printk("[OPENAIR][MAC] Running without an RRC\n");
    }
#ifndef USER_MODE
#ifndef PHY_EMUL
  printk("add openair2 proc\n");
  add_openair2_stats();
#endif
#endif  
  printk("[OPENAIR][MAC][INIT] Init function finished\n");
  
  return(0);
  
}


/***********************************************************************/
int mac_init_global_param(){
  /***********************************************************************/

  int i; 
  //  printk("[MAC] Init Global Param In, CHBCH_PDU_SIZE %d ...\n",sizeof(CHBCH_PDU));
  //  if(sizeof(CHBCH_PDU) > 140){
  //    printk("Size of CHBCH_PDU= %d, fix this !!!\n",sizeof(CHBCH_PDU));
  //    return -1;
  //  }  

  Is_rrc_registered=0;  
  Mac_rlc_xface = NULL;
  printk("[MAC] CALLING RLC_MODULE_INIT...\n");	

  if (rlc_module_init()!=0)
    return(-1);

  printk("[MAC] RLC_MODULE_INIT OK, malloc16 for mac_rlc_xface...\n");	
  
  Mac_rlc_xface = (MAC_RLC_XFACE*)malloc16(sizeof(MAC_RLC_XFACE));
  
  if(Mac_rlc_xface == NULL){
    printk("[MAC] FATAL EROOR: Could not allocate memory for Mac_rlc_xface !!!\n");
    return (-1);
    
  }	

  printk("[MAC] malloc16 OK, mac_rlc_xface @ %p\n",(void *)Mac_rlc_xface);  

  mac_xface->macphy_data_ind=macphy_data_ind;
  mac_xface->mrbch_phy_sync_failure=mrbch_phy_sync_failure;
  mac_xface->chbch_phy_sync_success=chbch_phy_sync_success;
  Mac_rlc_xface->macphy_exit=  mac_xface->macphy_exit;
  Mac_rlc_xface->frame = -1;
  Mac_rlc_xface->mac_config_req=mac_config_req;
  Mac_rlc_xface->mac_meas_req=mac_meas_req;
  Mac_rlc_xface->rrc_rlc_config_req=rrc_rlc_config_req;
  Mac_rlc_xface->rrc_rlc_data_req=rrc_rlc_data_req;
  Mac_rlc_xface->rrc_rlc_register_rrc=rrc_rlc_register_rrc;
  
  printk("[MAC]INIT_GLOBAL_PARAM: Mac_rlc_xface=%p,rrc_rlc_register_rrc =%p\n",Mac_rlc_xface,Mac_rlc_xface->rrc_rlc_register_rrc); 
  
Mac_rlc_xface->mac_rlc_data_req=mac_rlc_data_req;
  Mac_rlc_xface->mac_rlc_data_ind=mac_rlc_data_ind;
  Mac_rlc_xface->mac_rlc_status_ind=mac_rlc_status_ind;
  Mac_rlc_xface->pdcp_run=pdcp_run;
  Mac_rlc_xface->pdcp_data_req=pdcp_data_req;	
  Mac_rlc_xface->mrbch_phy_sync_failure=mrbch_phy_sync_failure;
  Mac_rlc_xface->chbch_phy_sync_success=chbch_phy_sync_success;
  
  printk("[MAC] Init CHBCH_PHY_RESOURCES\n");

  /*  
  CHBCH_PHY_RESOURCES[0].Time_alloc=CHBCH_TIME_ALLOC;
  CHBCH_PHY_RESOURCES[0].Freq_alloc=0x0f0f;
  CHBCH_PHY_RESOURCES[0].Antenna_alloc=0;
  CHBCH_PHY_RESOURCES[0].Coding_fmt=0;
  CHBCH_PHY_RESOURCES[1].Time_alloc=CHBCH_TIME_ALLOC;;
  CHBCH_PHY_RESOURCES[1].Freq_alloc=0xf0f0;
  CHBCH_PHY_RESOURCES[1].Antenna_alloc=5;
  CHBCH_PHY_RESOURCES[1].Coding_fmt=5;
  */

  for(i=0;i<MAX_NB_SCHED;i++)
    Sorted_index_table[i]=i; 

#ifdef USER_MODE
  msg("[MAC][GLOBAL_INIT] RRC_INIT_GLOBAL\n");
  rrc_init_global_param();
  Is_rrc_registered=1;
#endif //USER_MODE
 
  mac_xface->out_of_sync_ind=mac_UE_out_of_sync_ind;  
  printk("[MAC] Init Global Param Done\n");

  return 0;
}


/***********************************************************************/
void mac_top_cleanup(u8 Mod_id){
/***********************************************************************/
  /*
  free16(Phy_resources_table,NB_PHY_RESOURCES_MAX*sizeof(PHY_RESOURCES_TABLE_ENTRY));
  free16(Macphy_req_table,NB_REQ_MAX*sizeof(MACPHY_DATA_REQ_TABLE_ENTRY));
  free16(Macphy_ind_table,NB_IND_MAX*sizeof(MACPHY_DATA_IND_TABLE_ENTRY));
  */
}

/***********************************************************************/
void emul_phy_sync(unsigned char Mod_id, unsigned char Chbch_index){
/***********************************************************************/

// MACPHY_DATA_REQ *Macphy_data_req_sch;
/*
 if ((Macphy_data_req_sch = new_macphy_data_req(Mod_id))==NULL)
   mac_xface->macphy_exit("[get_chbch_sch] new_macphy_data_req fails\n");
 Macphy_data_req_sch->Pdu_type = CHBCH_SCH;  
 Macphy_data_req_sch->Direction = RX;  
 Macphy_data_req_sch->Lchan_id.Index = Chbch_index;
 Macphy_data_req_sch->CH_index = Chbch_index;
 Macphy_data_req_sch->Phy_resources = &CHBCH_PHY_RESOURCES[Chbch_index];
 Macphy_data_req_sch->num_tb=1;
#ifdef DEBUG_INITIAL_SYNC
     msg("[EMUL_PHY_SYNC]: Node %d to CH index %d, Freq_alloc %x\n",NODE_ID[Mod_id],
	 Chbch_index,Macphy_data_req_sch->Phy_resources->Freq_alloc); 
#endif //DEBUG_INITIAL_SYNC
*/
}


#ifndef USER_MODE
EXPORT_SYMBOL(CH_mac_inst);
EXPORT_SYMBOL(UE_mac_inst);
#ifndef PHY_EMUL
EXPORT_SYMBOL(NB_UE_INST);
EXPORT_SYMBOL(NB_CH_INST);
#endif //PHY_EMUL
#endif //USER_MODE

