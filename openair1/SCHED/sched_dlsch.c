#ifndef USER_MODE
#define __NO_VERSION__

/*
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
*/

#ifdef RTAI_ISNT_POSIX
#include "rt_compat.h"
#endif /* RTAI_ISNT_POSIX */

#include "MAC_INTERFACE/extern.h"

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#endif // CBMIMO1

#ifdef RTAI_ENABLED
#include <rtai.h>
#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif //

#else
#include <stdio.h>
#include <stdlib.h>
#endif //  /* USER_MODE */


#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "extern.h"

#define DEBUG_PHY

/// Mutex for instance count on dlsch scheduling 
pthread_mutex_t dlsch_mutex[8];
/// Condition variable for dlsch thread
pthread_cond_t dlsch_cond[8];

pthread_t dlsch_threads[8];
pthread_attr_t attr_dlsch_threads;

// activity indicators for harq_pid's
int dlsch_instance_cnt[8];
// process ids for cpu
int dlsch_cpuid[8];
// subframe number for each harq_pid (needed to store ack in right place for UL)
int dlsch_subframe[8];

extern int exit_openair;

/*
extern int dlsch_errors;
extern int dlsch_received;
extern int dlsch_errors_last;
extern int dlsch_received_last;
extern int dlsch_fer;
extern int current_dlsch_cqi;
*/

/** DLSCH Decoding Thread */
static void * dlsch_thread(void *param) {

  unsigned long cpuid = rtai_cpuid();
  unsigned int dlsch_thread_index = 0; //*((int *)param);
  unsigned int ret;
  u8 harq_pid;

  int time_in,time_out;

  int eNB_id = 0, UE_id = 0;
  PHY_VARS_UE *phy_vars_ue = PHY_vars_UE_g[UE_id];

  rt_set_runnable_on_cpuid(pthread_self(),1);

  printk("[openair][SCHED][DLSCH] dlsch_thread for process %d started with id %x, fpu_flag = %x, cpu %d\n",
      dlsch_thread_index,
      (unsigned int)pthread_self(),
      pthread_self()->uses_fpu,
      cpuid);

  if ((dlsch_thread_index <0) || (dlsch_thread_index>7)) {
    msg("[openair][SCHED][DLSCH] Illegal dlsch_thread_index %d!!!!\n",dlsch_thread_index);
    return;
  }

  dlsch_cpuid[dlsch_thread_index] = cpuid;

  while (exit_openair == 0){
    
    pthread_mutex_lock(&dlsch_mutex[dlsch_thread_index]);

    while (dlsch_instance_cnt[dlsch_thread_index] < 0) {
      pthread_cond_wait(&dlsch_cond[dlsch_thread_index],&dlsch_mutex[dlsch_thread_index]);
    }


    dlsch_instance_cnt[dlsch_thread_index]--;
    pthread_mutex_unlock(&dlsch_mutex[dlsch_thread_index]);	

    debug_msg("[openair][SCHED][DLSCH] Frame %d: Calling dlsch_decoding with dlsch_thread_index = %d from cpu %d\n",mac_xface->frame,dlsch_thread_index,rtai_cpuid());

    time_in = openair_get_mbox();

    if (mac_xface->frame < phy_vars_ue->dlsch_errors[eNB_id]) {
      phy_vars_ue->dlsch_errors[eNB_id]=0;
      phy_vars_ue->dlsch_received[eNB_id] = 0;
    }

    if (phy_vars_ue->dlsch_ue[eNB_id][0]) {
      harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid;

      dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
			 phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
			 phy_vars_ue->dlsch_ue[0][0],
			 get_G(&phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->dlsch_ue[eNB_id][0]->nb_rb,
			       phy_vars_ue->dlsch_ue[eNB_id][0]->rb_alloc,
			       get_Qm(phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs),
			       phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
			       dlsch_subframe[dlsch_thread_index]),
			 phy_vars_ue->lte_ue_dlsch_vars[eNB_id]->llr[0],
			 0,
			 dlsch_subframe[dlsch_thread_index]<<1);
      debug_msg("[PHY][UE %d] Calling dlsch_decoding for subframe %d\n",phy_vars_ue->Mod_id,dlsch_subframe[dlsch_thread_index]);
      ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars[eNB_id]->llr[0],
			   &phy_vars_ue->lte_frame_parms,
				 phy_vars_ue->dlsch_ue[eNB_id][0],
			   dlsch_subframe[dlsch_thread_index],
			   phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);
      
      
      
      time_out = openair_get_mbox();
      if (ret == (1+MAX_TURBO_ITERATIONS)) {
	phy_vars_ue->dlsch_errors[eNB_id]++;
	
#ifdef DEBUG_PHY
	msg("DLSCH (rv %d,mcs %d) in error\n",phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
	    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs);
#endif
      }
      else {
	
#ifdef OPENAIR2
	mac_xface->ue_send_sdu(phy_vars_ue->Mod_id,
			       phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->b,
			       0);
#endif
      }
    }
    
    if (mac_xface->frame % 100 == 0) {
      if ((phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]) != 0) 
	phy_vars_ue->dlsch_fer[eNB_id] = (100*(phy_vars_ue->dlsch_errors[eNB_id] - phy_vars_ue->dlsch_errors_last[eNB_id]))/(phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]);
      phy_vars_ue->dlsch_errors_last[eNB_id] = phy_vars_ue->dlsch_errors[eNB_id];
      phy_vars_ue->dlsch_received_last[eNB_id] = phy_vars_ue->dlsch_received[eNB_id];
      
    }
    
    
    debug_msg("[PHY][UE %d] Frame %d, subframe %d: dlsch_decoding ret %d (mcs %d, TBS %d)\n",
	      phy_vars_ue->Mod_id,mac_xface->frame,dlsch_subframe[dlsch_thread_index],ret,
	      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[0]->mcs,
	      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[0]->TBS);
    debug_msg("[PHY][UE %d] Frame %d, subframe %d: dlsch_errors %d, dlsch_received %d, dlsch_fer %d, current_dlsch_cqi %d\n",
	      phy_vars_ue->Mod_id,mac_xface->frame,dlsch_subframe[dlsch_thread_index],
	      phy_vars_ue->dlsch_errors[eNB_id],
	      phy_vars_ue->dlsch_received[eNB_id],
	      phy_vars_ue->dlsch_fer[eNB_id],
	      phy_vars_ue->PHY_measurements.wideband_cqi_tot[eNB_id]);
    
  }
  
  msg("[openair][SCHED][DLSCH] DLSCH thread %d exiting\n",dlsch_thread_index);
  dlsch_instance_cnt[dlsch_thread_index] = 99;
}

int init_dlsch_threads(void) {

  int error_code;
  struct sched_param p;
  int dlsch_thread_index;

  // later loop on all harq_pids, do 0 for now
  dlsch_thread_index=0;

  pthread_mutex_init(&dlsch_mutex[dlsch_thread_index],NULL);
  
  pthread_cond_init(&dlsch_cond[dlsch_thread_index],NULL);

  pthread_attr_init (&attr_dlsch_threads);
  pthread_attr_setstacksize(&attr_dlsch_threads,OPENAIR_THREAD_STACK_SIZE);
  
  attr_dlsch_threads.priority = 1;

  p.sched_priority = OPENAIR_THREAD_PRIORITY;
  pthread_attr_setschedparam  (&attr_dlsch_threads, &p);
#ifndef RTAI_ISNT_POSIX
  pthread_attr_setschedpolicy (&attr_dlsch_threads, SCHED_FIFO);
#endif

  dlsch_instance_cnt[dlsch_thread_index] = -1;
  printk("[openair][SCHED][DLSCH][INIT] Allocating DLSCH thread for dlsch_thread_index %d\n",dlsch_thread_index);
  error_code = pthread_create(&dlsch_threads[dlsch_thread_index],
  			      &attr_dlsch_threads,
  			      dlsch_thread,
  			      (void *)&dlsch_thread_index);

  if (error_code!= 0) {
    printk("[openair][SCHED][DLSCH][INIT] Could not allocate dlsch_thread %d, error %d\n",dlsch_thread_index,error_code);
    return(error_code);
  }
  else {
    printk("[openair][SCHED][DLSCH][INIT] Allocate dlsch_thread %d successful\n",dlsch_thread_index);
    return(0);
  }
   
}

void cleanup_dlsch_threads(void) {

  int dlsch_thread_index = 0;

  // later loop on all harq_pid's

  //  pthread_exit(&dlsch_threads[dlsch_thread_index]);
  printk("[openair][SCHED][DLSCH] Scheduling dlsch_thread %d to exit\n",dlsch_thread_index);

  dlsch_instance_cnt[dlsch_thread_index] = 0;
  if (pthread_cond_signal(&dlsch_cond[dlsch_thread_index]) != 0)
    printk("[openair][SCHED][DLSCH] ERROR pthread_cond_signal\n");
  else
    printk("[openair][SCHED][DLSCH] Signalled dlsch_thread %d to exit\n",dlsch_thread_index);
    
  printk("[openair][SCHED][DLSCH] Exiting ...\n");
  pthread_cond_destroy(&dlsch_cond[dlsch_thread_index]);
  pthread_mutex_destroy(&dlsch_mutex[dlsch_thread_index]);
}
