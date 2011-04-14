
/*
 // \author R. Knopp/H. Anouar
 // \date 02.06.2004   (initial WIDENS version)
 *  @{ 
 */

/*
* @addtogroup _emulation_layer_ref_implementation_
\section _process_scheduling_ Process Scheduling
This section deals with real-time process scheduling for PHY Emulation layer.
*/


#define __NO_VERSION__


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/netdevice.h>

#include <asm/io.h>
#include <asm/bitops.h>
 
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>

#ifdef RTAI_ISNT_POSIX
#include "rt_compat.h"
#endif /* RTAI_ISNT_POSIX */

#include <rtai.h>
#include <rtai_posix.h>
#include <rtai_fifos.h>

#ifdef SERIAL_IO
#include "rtai_serial.h"
#endif

#include "defs.h"
#include "extern.h"
#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/extern.h"
#include "SIMULATION/PHY_EMULATION/CONTROL/defs.h"

#define NUMBER_OF_CHUNKS_PER_SLOT 10
#define NUMBER_OF_CHUNKS_PER_FRAME (NUMBER_OF_CHUNKS_PER_SLOT * SLOTS_PER_FRAME)
#define NS_PER_CHUNK 66667 // (7.68 msps, 512 samples per chunk 
#define DRIFT_OFFSET 300
#define NS_PER_SLOT (NS_PER_CHUNK * NUMBER_OF_CHUNKS_PER_SLOT)
//#define US_PER_SLOT (NS_PER_SLOT * 0.001)

#define MAX_DRIFT_COMP 3000
#define MAX_SCHED_CNT 50000000

/// Threads
pthread_t         threads[openair_SCHED_NB_THREADS]={NULL,NULL,NULL};
/// Thread Attributes
pthread_attr_t    attr_threads[openair_SCHED_NB_THREADS];

/// Global exit variable (exit on error or manual stop via IOCTL)
int exit_openair = 0;







static void * top_level_scheduler(void *param) {
  RTIME Start_time,Finish_time;
  long int Deal_time=0,Drift_time=0;
  char k=0,kk;
  u8              next_slot, last_slot=0,i;
  
  radio_emulation_init();


  while (exit_openair == 0) {
    //    msg("[SCHED][TOP] New Round : last_slot %d ..\n",last_slot);
    if (openair_emul_vars.node_running == 1) {
      openair_emul_vars.sched_cnt++;

      
      Start_time=rt_get_time_ns();//*0.001;
      
      next_slot = (openair_emul_vars.slot_count + 1 ) % SLOTS_PER_FRAME;
      last_slot = (openair_emul_vars.slot_count - 1 ) % SLOTS_PER_FRAME;       
	
      //msg("[SCHED][Thread] calling mac_scheduler\n");	
      mac_xface->macphy_scheduler(last_slot);
      //msg("[SCHED][Thread] returning from mac_scheduler\n");	
      
      if(last_slot==2){	
	//	msg("[SCHED][Thread] calling phy_procedures\n");  
	emulation_tx_rx();
	
	mac_xface->frame++;
      }
      
      
      Finish_time=rt_get_time_ns();//*0.001;
      
      // if(openair_emul_vars.sched_cnt>100){//||is_primary_cluster_head){
      Drift_time=Finish_time-Start_time-((RTIME)(NS_PER_SLOT));
      // if ((mac_xface->frame % 1000) == 0)
      //	msg("[SCHED][TTI %d] DRIFT_TIME %ld\n", mac_xface->frame,Drift_time);
      if(Drift_time<0){//we are too fast!!
	if(Deal_time>-Drift_time){  
	  Deal_time+=Drift_time;
	}
	else{
	  //  if ((mac_xface->frame % 10000) == 0)
	  //msg("[SCHED] Sleeping for %ld\n",(1*(-Drift_time-Deal_time)));
	  rt_sleep(nano2count(1*(-Drift_time-Deal_time)));
	  Deal_time=0;
	  }
      }
      else{
	Deal_time+=Drift_time;
      }
      //msg("********[OPENAIR][SHCHED]//// CURRENT_DEAL_TIME=%d************\n",Deal_time);  
      
      
      
      //rt_sleep(nano2count(666666));
      
      if((mac_xface->frame % 20000)==0) 
	msg("[OPENAIR][SCHED] CURRENT_DRIFT_TIME=%ld ns,Current_DEAL_TIME=%ld ns************\n",
	    Drift_time,
	    Deal_time); 
      
	
	
      //pthread_mutex_lock (&openair_mutex);
      
      openair_emul_vars.slot_count=(openair_emul_vars.slot_count+1) % SLOTS_PER_FRAME; //next slot
      
      //      if (openair_emul_vars.sched_cnt == MAX_SCHED_CNT)
      //	exit_openair = 1;
    }
    else {
      rt_sleep(nano2count(666000000));
      // sleep
    }
    //    if(mac_xface->frame == 800) exit_openair=1; 
  }
  msg("[openair][SCHED][top_level_thread] Exiting ... openair_emul_vars.sched_cnt = %d\n",openair_emul_vars.sched_cnt);
  // schedule openair_thread to exit
  msg("[openair][SCHED][top_level_thread] Scheduling openair_thread to exit ... \n");
  openair_emul_vars.node_running = 0;
  //  rt_task_delete(rt_whoami);
  /*  pthread_exit(NULL); */
  msg("[openair][SCHED][top_level_thread] Exiting top_level_scheduler ... \n");
}

int emul_sched_init(void) {
  
  int error_code;
  int ret;
  
  mac_xface->frame = 0;
  
  openair_emul_vars.scheduler_interval_ns=NUMBER_OF_CHUNKS_PER_SLOT*NS_PER_CHUNK;        // initial guess
  

  
  
  
 
  openair_emul_vars.mode = openair_SYNCHED_TO_CHSCH;
   
  pthread_attr_init (&attr_threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX]);
  pthread_attr_setstacksize(&attr_threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],OPENAIR_THREAD_STACK_SIZE);

  attr_threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX].priority = 0;




   
  // Create top_level_scheduler
  error_code = pthread_create(&threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],
  			      &attr_threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],   //default attributes
  			      top_level_scheduler,
  			      (void *)0);

   

  // rt_change_prio(threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],0);
  if (error_code!= 0) {
    printk("[openair][SCHED][INIT] Could not allocate top_level_scheduler, error %d\n",error_code);
    return(error_code);
  }

  // Done by pthread_create in rtai_posix.h
  //rt_task_use_fpu(threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],1);
  

  return(0);
}

void emul_sched_cleanup() {

  exit_openair = 1;

  /*
    #ifndef USER_MODE  
    msg("[openair][openair][SCHED][SCHED][CLEANUP] Joining OPENAIR_THREAD\n");
    if(threads[OPENAIR_THREAD_INDEX]) 
    if(rt_get_task_state(threads[OPENAIR_THREAD_INDEX])!=0)
    pthread_join_rt(threads[OPENAIR_THREAD_INDEX],NULL);
    msg("[openair][openair][SCHED][SCHED][CLEANUP] Joining TOP_LEVEL_SCHEDULER_THREAD\n");
  
    if(threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX])
    if(rt_get_task_state(threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX])!=0)
    pthread_join_rt(threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],NULL);

    #endif // USER_MODE  
  */

 
 
  pthread_exit(&threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX]);//H.A



}


void emul_sched_exit() {

  exit_openair = 1;
}

/*@}*/


