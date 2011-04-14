/*________________________mac_sim.c________________________

  Authors : Hicham Anouar, Raymond Knopp
  Company : EURECOM
  Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
  ________________________________________________________________*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "LAYER2/MAC/vars.h"
#include "RRC/MESH/vars.h"
#include "SIMULATION/phy_vars.h"
#include "SIMULATION/config_vars.h"
#include "PHY_INTERFACE/vars.h"
#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/vars.h"
#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/defs.h"
#include "SIMULATION/PHY_EMULATION/CONTROL/defs.h"
#include "SIMULATION/PHY_EMULATION/impl_defs.h"
#include "SIMULATION/config_proto.h"
#include "SIMULATION/simulation_defs.h"
#include "LAYER2/MAC/defs.h"

#ifdef LINUX
extern int netlink_init(void);
#endif //LINUX

#define K 2

#define TARGET_TTI_TIME_NS 3100000


#ifdef ECOS
//#define SLOTS_PER_FRAME 4 //.................
#define TARGET_TTI_TICKS 1 //1 tick=10ms for linux synthetic target, this is the min we can simulate

#endif


#define STATS_BUF_LEN 16384
char print_stats_buffer[STATS_BUF_LEN];

#ifndef ECOS
int main(char argc,char **argv) {
#else
#include <cyg/kernel/kapi.h> 
  int main(void){
#endif  
  
  unsigned char last_slot;
  int ret,i,j,k;
  FILE *topology,*CM;
   TOPOLOGY_OK=0;
  struct timespec time_spec ; 
 unsigned long time_last,time_now;
 int td,td_avg,sleep_time_ms,sleep_time_us,sleep_time_ns;
  //char topology_file[256],c;
  char *topology_file=malloc(256),c;
  int len;

  Emul_vars=(EMULATION_VARS *)malloc(sizeof(EMULATION_VARS));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = (MAC_xface*)malloc(sizeof(MAC_xface));
  

  //Master_id = 99;

#ifndef ECOS
  topology_file[0] = '\0';

  while ((c = getopt(argc,argv,"m:t:")) != -1)
    switch (c) {
    case 'm':
      
      Master_id=atoi(optarg);
      break;
    case 't':
      strcpy(topology_file,optarg);
      break;
    }

  if (Master_id == 99) {
    printf("[MAC_SIM] Provide a Machine id\n");
    exit(-1);
  }
  if (topology_file[0] == '\0') {
    printf("[MAC_SIM] Provide a Topology file\n");
    exit(-1);
  }

//Id of local running Machine (between 0 & NB_MASTER-1)// MAchine with Master_id 0 should start first
  printf("[MAC_SIM][START][Topology] Configuring Topology for Master %d\n",Master_id);

  printf("Allocating memory for PHY_VARS\n");
    
 
  if((config = fopen("../../openair_config.cfg","r")) == NULL) // this can be configured
    {
      printf("[Main USER] The openair configuration file <openair_config.cfg> could not be found!");
      exit(0);
    }
  if ((scenario= fopen("../../openair_scenario.scn","r")) ==NULL)
    {
      printf("[Main USER] The openair scenario file <openair_scenario.scn> could not be found!");
      exit(0);
    }
  
  if ((topology= fopen(topology_file,"r")) ==NULL)
    {
      printf("[Main USER] The openair topology file <openair_topology.top> could not be found!");
      exit(0);
    }
  if ((CM= fopen("../../covariance_matrix.dat","r")) ==NULL)
    {
      printf("[Main USER] The openair Covariance matrix file <Covariance_matrix.tex> could not be found!");
      exit(0);
    }

  printf("Opened configuration files\n");


  config_topology(topology);

  Is_primary_master=0;
  if(!Master_id) 
    Is_primary_master=1;
  Master_list=0;
  j=1;
  for(i=0;i<NB_MASTER;i++){
    if(i!=Master_id)
      Master_list=Master_list+j;
    //printf("i=%d, MASTER_LIST %d\n",i,Master_list);
    j*=2;
  }
  printf("Starting Master %d of %d Masters, Master_list= %d, NB_INST %d\n",Master_id,NB_MASTER,Master_list,NB_INST);

  //exit(0);
 
  radio_emulation_load_KH(CM);//--> top_init
  //    exit(0);
  reconfigure_MACPHY(scenario);

  dump_config();

  //emul_check_out_in_traffic();   

#else
  Master_id=0;
  Master_list=0;
  Is_primary_master=1;
  NB_MASTER=0;
  NB_NODE=2; 
  NB_INST=2;
  NB_CH_INST=1;
  NODE_ID[0]=0;
  NB_UE_INST=1;
  NODE_ID[1]=8;
  NODE_LIST[0]=0;
  NODE_LIST[1]=8;
  Emul_idx[0]=0;
  Emul_idx[1]=1;
  RSSI[0][0]=-120;
  RSSI[0][1]=-60;
  RSSI[1][0]=-60;
  RSSI[1][1]=-120;
  PHY_config->PHY_framing.Nd=256;
  PHY_config->PHY_framing.Nsymb=64;
  PHY_config->PHY_framing.log2Nd=8;
  PHY_config->PHY_framing.Nz=64;
  PHY_config->PHY_framing.Extension_type=1;
  PHY_config->PHY_chsch[0].Nsymb=1;
  PHY_config->PHY_sch[1].Nsymb=1;
  PHY_config->PHY_chbch.Nsymb=7;
  PHY_config->PHY_chbch.Npilot=3;


#endif






  msg("[MAIN]MAC_INIT_GLOBAL_PARAM IN...\n");
  mac_init_global_param(); 


  mac_xface->macphy_init=mac_top_init;
 
#ifdef NAS_NETLINK
#ifdef LINUX
  ret = netlink_init();
#endif //LINUX
#endif
  msg("[MAIN]MAC_INIT IN...\n");
  ret = mac_init();
  radio_emulation_init();
 
  //  msg("[MAIN]RRC_INIT_GLOBAL_PARAM IN...\n");
  //rrc_init_global_param();

#ifdef EMULATION_MASTER  
  //rg_bypass_init(emul_tx_handler,emul_rx_handler);
#else
  //mt_bypass_init(emul_tx_handler,emul_rx_handler);

  //  bypass_init(emul_tx_handler,emul_rx_handler);

  //    mt_bypass_receive_from_peer();
#endif //EMULATION_MASTER

  if (ret >= 0) {
    printf("Initialized MAC variables\n");
    
    last_slot = SLOTS_PER_FRAME-1;
    mac_xface->macphy_scheduler = macphy_scheduler;
    printf("Initialized MAC SCHEDULER\n");

    msg("[MAIN]W3G4FREE_MAC_INIT IN...\n");
    //  w3g4free_mac_init();
    msg("ALL INIT OK\n");
    int kk;
 
    //exit(0);

#ifndef ECOS
    clock_gettime(CLOCK_REALTIME,&time_spec);
    time_now = (unsigned long)time_spec.tv_nsec;
    td_avg = 0;
    j=0;
#else
    cyg_tick_count_t tick_last,tick,diff; //u64
    tick=cyg_current_time();
#endif

    Mac_rlc_xface->frame=mac_xface->frame;
    sleep_time_us=10;
    td_avg=TARGET_TTI_TIME_NS;
    while (1) {
      mac_xface->macphy_scheduler(last_slot);      
      if(last_slot==2){
	if(mac_xface->frame % 1024 == 0){
	  len = openair2_stats_read(print_stats_buffer, NULL, 0, STATS_BUF_LEN);
	  print_stats_buffer[len] = '\0';
	  puts(print_stats_buffer);
	}
	/*
	  if ((mac_xface->frame%1024)==0) {
	  printf("time_diff = %d (ns),td_avg = %f (ms),sleep_time_us = %d\n",td,td_avg/1e6,sleep_time_us);
	  if (sleep_time_us == 0)
	  printf("WARNING not quasi-real time\n");
	  }
	*/
	emulation_tx_rx();
	mac_xface->frame=mac_xface->frame+1;
	
	
	
#ifndef ECOS
	clock_gettime(CLOCK_REALTIME,&time_spec);
	time_last = time_now;
	time_now  = (unsigned long)time_spec.tv_nsec;
	td = (int)(time_now - time_last);
	
	if (td>0)
	  td_avg = (int)(((K*(long)td) + (((1<<3)-K)*((long)td_avg)))>>3);
	//	printf("last %ld,now %ld, CURRENT TIME DIFF %dus, DIFF_avg %dus\n",time_last,time_now,td/1000, (td_avg-TARGET_TTI_TIME_NS)/1000);
	
	if (td_avg<(TARGET_TTI_TIME_NS - 100)){
	    sleep_time_us += 10;
	}
	else if (td_avg > (TARGET_TTI_TIME_NS + 100))
	  sleep_time_us -= 10;

	if(sleep_time_us > 0){
	  //	  printf("sleep %d\n",sleep_time_us);
	  usleep(sleep_time_us);
	}
#else
	tick_last=tick;
	tick=cyg_current_time();
	diff=tick-tick_last;
	if(diff > TARGET_TTI_TICKS){
	  printf("FATAL: NOT REAL TIME, TTI %ld\n",diff);
	  exit(-1);
	}
	else if(diff < TARGET_TTI_TICKS){
	  //if((mac_xface->frame % 500)==0)
	  //    printf("Frame %d,sleep for %ld ticks\n",mac_xface->frame,TARGET_TTI_TICKS-diff);
	  cyg_thread_delay(TARGET_TTI_TICKS-diff);
	}
	tick=cyg_current_time();
#endif
	
      }//if last_slot
      
      // usleep(100000);
      last_slot = (last_slot+1)%SLOTS_PER_FRAME;
      //   if((mac_xface->frame % 500)==0)
      //printf("Frame %d, Slot %d\n",mac_xface->frame,last_slot);
   
    }

  }


  mac_cleanup();

  return(0);


  // */  
}
  
