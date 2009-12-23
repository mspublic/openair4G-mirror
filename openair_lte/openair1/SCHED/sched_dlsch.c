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

extern int exit_openair;

/** DLSCH Decoding Thread */
static void * dlsch_thread(void *param) {
#ifndef USER_MODE


#endif //  /* USER_MODE */


  unsigned int coded_bits_per_codeword,nsymb,input_buffer_length;
  int rate_num=1,rate_den=3;
  unsigned char mod_order[2]={2,2};
  unsigned long cpuid = rtai_cpuid();
  unsigned int harq_pid = *((int *)param);
  unsigned int ret;

  int time_in,time_out;
  unsigned int dlsch_errs=0;

  msg("[openair][SCHED][dlsch_thread] dlsch_thread for process %d started with id %x, fpu_flag = %x, cpu %d\n",
      harq_pid,
      (unsigned int)pthread_self(),
      pthread_self()->uses_fpu,
      cpuid);

  if ((harq_pid <0) || (harq_pid>7)) {
    msg("[openair][SCHED][dlsch_thread] Illegal harq_pid %d!!!!\n",harq_pid);
    return;
  }

  dlsch_cpuid[harq_pid] = cpuid;

  while (exit_openair == 0){
    
    pthread_mutex_lock(&dlsch_mutex[harq_pid]);

    while (dlsch_instance_cnt[harq_pid] < 0) {
      pthread_cond_wait(&dlsch_cond[harq_pid],&dlsch_mutex[harq_pid]);
    }


    dlsch_instance_cnt[harq_pid]--;
    pthread_mutex_unlock(&dlsch_mutex[harq_pid]);	

    nsymb = (lte_frame_parms->Ncp == 0) ? 14 : 12;
    coded_bits_per_codeword =( 12 * (12 * mod_order[0]) * (nsymb-lte_frame_parms->first_dlsch_symbol-3));
    input_buffer_length = ((int)(coded_bits_per_codeword*rate_num/rate_den))>>3;
    if ((mac_xface->frame % 100) == 0)
      msg("Frame %d: Calling dlsch_decoding (%d,%d,%p) from cpu %d\n",mac_xface->frame,coded_bits_per_codeword,input_buffer_length,dlsch_ue[harq_pid],rtai_cpuid());

    time_in = openair_get_mbox();

    if (mac_xface->frame < dlsch_errs)
      dlsch_errs=0;

    if (dlsch_ue[harq_pid]) {
      ret = dlsch_decoding(input_buffer_length<<3,
			   lte_ue_dlsch_vars->llr[0],		 
			   lte_frame_parms,
			   dlsch_ue[harq_pid],
			   harq_pid,               //harq_pid
			   12);
      
    
	//NB allocated RBs
      time_out = openair_get_mbox();
      if (ret == (1+MAX_TURBO_ITERATIONS)) {
	dlsch_errs++;
      }
    
      if ((mac_xface->frame % 100) == 0)
	msg("Frame %d: dlsch_decoding in %d, out %d, ret %d (%d errors)\n",mac_xface->frame,time_in,time_out,ret,dlsch_errs);
    }
  }
  msg("DLSCH thread %d exiting\n",harq_pid);
  dlsch_instance_cnt[harq_pid] = 99;
}

static int harq_pid;

int init_dlsch_threads() {

  int error_code;
  struct sched_param p;

  // later loop on all harq_pids, do 0 for now
  harq_pid=0;

  pthread_mutex_init(&dlsch_mutex[harq_pid],NULL);
  
  pthread_cond_init(&dlsch_cond[harq_pid],NULL);

  pthread_attr_init (&attr_dlsch_threads);
  pthread_attr_setstacksize(&attr_dlsch_threads,OPENAIR_THREAD_STACK_SIZE);
  
  attr_dlsch_threads.priority = 1;

  p.sched_priority = OPENAIR_THREAD_PRIORITY;
  pthread_attr_setschedparam  (&attr_dlsch_threads, &p);
#ifndef RTAI_ISNT_POSIX
  pthread_attr_setschedpolicy (&attr_dlsch_threads, SCHED_FIFO);
#endif

  dlsch_instance_cnt[harq_pid] = -1;
  printk("[SCHED][DLSCH][INIT] Allocating DLSCH thread for harq_pid %d\n",harq_pid);
  error_code = pthread_create(&dlsch_threads[harq_pid],
  			      &attr_dlsch_threads,
  			      dlsch_thread,
  			      (void *)&harq_pid);

  if (error_code!= 0) {
    printk("[SCHED][DLSCH][INIT] Could not allocate dlsch_thread %d, error %d\n",harq_pid,error_code);
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

  //  pthread_exit(&dlsch_threads[harq_pid]);
  printk("[SCHED][DLSCH] Scheduling dlsch_thread %d to exit\n",harq_pid);

  dlsch_instance_cnt[harq_pid] = 0;
  if (pthread_cond_signal(&dlsch_cond[harq_pid]) != 0)
    msg("[DLSCH][SCHED] ERROR pthread_cond_signal\n");
  else
    msg("[DLSCH][SCHED] Signalled dlsch_thread %d to exit\n",harq_pid);
    
  printk("[SCHEd][DLSCH] Exiting ...\n");
  pthread_cond_destroy(&dlsch_cond[harq_pid]);
  pthread_mutex_destroy(&dlsch_mutex[harq_pid]);
}
