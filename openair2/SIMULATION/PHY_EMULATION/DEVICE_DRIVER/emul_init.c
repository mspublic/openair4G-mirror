/*________________________mac_init.c________________________

  Authors : Hicham Anouar, Raymond Knopp
  Company : EURECOM
  Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
  ________________________________________________________________*/

/*!\brief Initilization and reconfiguration routines for generic MAC interface */
#ifndef USER_MODE
#define __NO_VERSION__


#ifdef RTAI_ENABLED
#include <rtai.h>
#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif //

#ifdef RTAI_ISNT_POSIX
#include "SIMULATION/PHY_EMULATION/SCHED/rt_compat.h"
#endif 
#include "SIMULATION/PHY_EMULATION/TRANSPORT/defs.h"

#else  // USER_MODE
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#endif // USER_MODE

//#include "PHY/types.h"
//#include "PHY/extern.h"
//#include "PHY/defs.h"
#include "SIMULATION/PHY_EMULATION/SCHED/defs.h"

#include "defs.h"
#include "extern.h"

#include "PHY_INTERFACE/extern.h"
#include "PHY_INTERFACE/defs.h"
#include "SIMULATION/PHY_EMULATION/impl_defs.h"
#include "SIMULATION/PHY_EMULATION/CONTROL/defs.h"
#include "SIMULATION/phy_extern.h"

#ifdef BIGPHYSAREA
extern void *bigphys_malloc(int);
#endif


#ifdef USER_MODE
void slow_exit(){
  fflush(stdout);
  sleep(10);
  exit(-1);
}
#endif

int mac_init(void)
{


  printk("[DEVICE_DRIVER] EMUL INIT IN...\n");
  unsigned short i,j;
  //Allocate memory for MAC/PHY communication primitives
  //NB_REQ_MAX = 16;
  for(i=0;i<NB_INST;i++){
    Macphy_req_table[i].Macphy_req_table_entry
      = (MACPHY_DATA_REQ_TABLE_ENTRY *)malloc16(NB_REQ_MAX*sizeof(MACPHY_DATA_REQ_TABLE_ENTRY));
    clear_macphy_data_req(i);
    for(j=0;j<NB_CNX_UE;j++){
      Phy_sync_cnt[i][j]=0;
      Phy_sync_status[i][j]=SYNC_WAIT;
      Sync_cnt[i][j]=0;
      Sync_status[i][j]=SYNC_WAIT;
    }
  }

#ifndef USER_MODE
  // mac_xface->macphy_init();
  mac_xface->macphy_exit = emul_sched_exit;
#else
  mac_xface->macphy_exit=slow_exit;//(void (*)(void)) exit;
#endif  


  mac_xface->slots_per_frame = SLOTS_PER_FRAME;
  mac_xface->frame=0;

  //  msg("MAC_INIT: Phy_config....\n");  
  //mac_xface->PHY_cfg=PHY_config;
  //msg("MAC_INIT: Phy_config OK\n");  

  if(Master_list){
#ifndef USER_MODE
    int ret;
    ret=rtf_create(fifo_bypass_phy_kern2user,16384);
   
    if (ret < 0) {
      //msg("[openair][MAC][INIT] Cannot create kern2user bypass fifo %d\n",fifo_bypass_phy_kern2user);     
      return(-1);
    }
    else{
       msg("[openair][MAC][INIT] Created kern2user bypass fifo %d\n",fifo_bypass_phy_kern2user);
      rtf_reset(fifo_bypass_phy_kern2user);
    }
    ret=rtf_create(fifo_bypass_phy_user2kern,16384);
    if (ret < 0) {
      //msg("[openair][MAC][INIT] Cannot create user2kern bypass fifo %d\n",fifo_bypass_phy_user2kern);
      return(-1);
    }
    else{
      rtf_reset(fifo_bypass_phy_user2kern);
      msg("[openair][MAC][INIT] Created user2kern bypass fifo %d\n",fifo_bypass_phy_user2kern);
    }
    ret=rtf_create(fifo_bypass_phy_kern2user_control,16384);
    if (ret < 0) {
      //msg("[openair][MAC][INIT] Cannot create kern2user control bypass fifo %d\n",fifo_bypass_phy_kern2user_control);
      return(-1);
    }
    else{
      msg("[openair][MAC][INIT] Created kern2user control bypass fifo %d\n",fifo_bypass_phy_kern2user_control);
      rtf_reset(fifo_bypass_phy_kern2user_control);
    }
    ret=rtf_create(fifo_mac_bypass,16384);

    if (ret < 0) {
      //msg("[openair][MAC][INIT] Cannot create MAC 2 BYPASS control fifo %d\n",fifo_mac_bypass);   
      return(-1);
    }
    else{
      msg("[openair][MAC][INIT] Created BYPASS 2 MAC FIFO %d\n",fifo_mac_bypass);
      rtf_reset(fifo_mac_bypass);
    }

    rtf_create_handler(fifo_bypass_phy_user2kern,X_FIFO_HANDLER(bypass_rx_handler));
    rtf_create_handler(fifo_bypass_phy_kern2user,X_FIFO_HANDLER(bypass_tx_handler));
  
#endif //USER_MODE   
    msg(" INIT BYPASS\n");  	
    pthread_mutex_init(&Tx_mutex,NULL);
    pthread_cond_init(&Tx_cond,NULL);
    Tx_mutex_var=1; 
    pthread_mutex_init(&Mac_low_mutex,NULL);
    pthread_cond_init(&Mac_low_cond,NULL);
    Mac_low_mutex_var=1; 
    bypass_init(emul_tx_handler,emul_rx_handler);
  } 


 
  /// Maximum number of Data bits (prior to coding) including CRC for different coding formats
  /// This is the raw bit rate assuming all resources are allocated
  msg("CALLING MAC/RLC/RRC init...\n"); 
  mac_xface->macphy_init();
  //  msg("MAC/RLC/RRC init OK\n"); 
  return(1);

}

void mac_cleanup(void)
{


  free16(Macphy_req_table[0].Macphy_req_table_entry,NB_REQ_MAX*sizeof(MACPHY_DATA_REQ_TABLE_ENTRY));


}


void mac_resynch(void) {
  int i;

  for (i=0;i<NB_INST;i++)
    clear_macphy_data_req(i);

}


#ifndef USER_MODE
//#ifdef OPENAIR2
//EXPORT_SYMBOL(NB_REQ_MAX);
//EXPORT_SYMBOL(Macphy_req_table);
//EXPORT_SYMBOL(NODE_ID);
//EXPORT_SYMBOL(NB_UE);
//EXPORT_SYMBOL(NB_INST);
//EXPORT_SYMBOL(UL_meas);
//EXPORT_SYMBOL(frame);
//#endif //USER_MODE
#endif //OPENAIR2
