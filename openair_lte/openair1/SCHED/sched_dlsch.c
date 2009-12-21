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


/** DLSCH Decoding Thread */
static void * dlsch_thread(void *param) {
#ifndef USER_MODE
  struct sched_param p;

#endif //  /* USER_MODE */


  unsigned int coded_bits_per_codeword,nsymb;
  int inv_target_code_rate = 2;
  unsigned char mod_order[2]={4,4};
  unsigned long cpuid = rtai_cpuid();
  unsigned int harq_pid = *((unsigned *)param);

  p.sched_priority = OPENAIR_THREAD_PRIORITY;
  pthread_attr_setschedparam  (&attr_threads[OPENAIR_THREAD_INDEX], &p);
#ifndef RTAI_ISNT_POSIX
  pthread_attr_setschedpolicy (&attr_threads[OPENAIR_THREAD_INDEX], SCHED_FIFO);
#endif

  dlsch_cpuid[harq_pid] = cpuid;
  msg("[openair][SCHED][openair_thread] dlsch_thread for process %d started with id %x, fpu_flag = %x, cpu %d\n",
      harq_pid,
      (unsigned int)pthread_self(),
      pthread_self()->uses_fpu,
      cpuid);

  while (exit_openair == 0){
    
    pthread_mutex_lock(&dlsch_mutex[harq_pid]);

    while (dlsch_instance_cnt[harq_pid] < 0) {
      pthread_cond_wait(&dlsch_cond[harq_pid],&dlsch_mutex[harq_pid]);
    }

    dlsch_instance_cnt[harq_pid]--;
    pthread_mutex_unlock(&dlsch_mutex[harq_pid]);	

    nsymb = (lte_frame_parms->Ncp == 0) ? 14 : 12;
    coded_bits_per_codeword =( 25 * (12 * mod_order[0]) * (nsymb-lte_frame_parms->first_dlsch_symbol-3));
    input_buffer_length = ((int)(coded_bits_per_codeword/inv_target_code_rate))>>3;
    
    ret = dlsch_decoding(input_buffer_length<<3,
			 lte_ue_dlsch_vars->llr[0],		 
			 lte_frame_parms,
			 dlsch_ue[harq_pid],
			 harq_pid,               //harq_pid
			 NB_RB);             //NB allocated RBs
  }
}


int init_dlsch_threads() {

  int harq_pid;


  // later loop on all harq_pids, do 0 for now
  harq_pid=0;

  pthread_mutex_init(&dlsch_mutex[harq_pid],NULL);
  
  pthread_cond_init(&dlsch_cond[harq_pid],NULL);

  pthread_attr_init (&attr_dlsch_threads);
  pthread_attr_setstacksize(&attr_dlsch_threads,OPENAIR_THREAD_STACK_SIZE);
  
  attr_dlsch_threads.priority = 1;

  dlsch_instance_cnt[harq_pid] = -1;
  error_code = pthread_create(&dlsch_threads[harq_pid],
  			      &attr_dlsch_threads[harq_pid],
  			      dlsch_thread,
  			      (void *)&harq_pid);

  if (error_code!= 0) {
    printk("[SCHED][OPENAIR_THREAD][INIT] Could not allocate dlsch_thread %d, error %d\n",harq_pid,error_code);
    return(error_code);
  }
  else {
    printk("[SCHED][OPENAIR_THREAD][INIT] Allocate dlsch_thread %d successful\n",harq_pid);
    return(0);
  }
   
}

void cleanup_dlsch_threads() {

  int harq_pid = 0;

  // later loop on all harq_pid's
  pthread_exit(dlsch_threads[harq_pid]);
  
  pthread_cond_destroy(&dlsch_cond[harq_pid]);
  pthrad_mutex_destroy(&dlsch_mutex[harq_pid]);
}
