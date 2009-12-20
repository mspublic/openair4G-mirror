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


/// Mutex for instance count on MACPHY scheduling 
pthread_mutex_t dlsch_mutex[4];
/// Condition variable for MACPHY thread
pthread_cond_t dlsch_cond[4];

/** DLSCH Decoding Thread */
static void * dlsch_thread(void *param) {
#ifndef USER_MODE
  struct sched_param p;

#endif //  /* USER_MODE */


  unsigned int coded_bits_per_codeword,nsymb;
  int inv_target_code_rate = 2;
  unsigned char mod_order[2]={4,4};



  p.sched_priority = OPENAIR_THREAD_PRIORITY;
  pthread_attr_setschedparam  (&attr_threads[OPENAIR_THREAD_INDEX], &p);
#ifndef RTAI_ISNT_POSIX
  pthread_attr_setschedpolicy (&attr_threads[OPENAIR_THREAD_INDEX], SCHED_FIFO);
#endif
  cpu_id = pthread_self()->uses_cpu;
  msg("[openair][SCHED][openair_thread] openair_thread started with id %x, fpu_flag = %x, cpu %d\n",
      (unsigned int)pthread_self(),
      pthread_self()->uses_fpu,
      cpu_id);

  while (exit_openair == 0){
    
    pthread_mutex_lock(&dlsch_mutex[cpu_id]);

    while (dlsch_instance_cnt[cpu_id] < 0) {
      pthread_cond_wait(&dlsch_cond[cpu_id],&dlsch_mutex[cpu_id]);
    }

    dlsch_instance_cnt[cpu_id]--;
    pthread_mutex_unlock(&dlsch_mutex[cpu_id]);	

    nsymb = (lte_frame_parms->Ncp == 0) ? 14 : 12;
    coded_bits_per_codeword =( 25 * (12 * mod_order[0]) * (nsymb-lte_frame_parms->first_dlsch_symbol-3));
    input_buffer_length = ((int)(coded_bits_per_codeword/inv_target_code_rate))>>3;
    
    ret = dlsch_decoding(input_buffer_length<<3,
			 lte_ue_dlsch_vars->llr[0],		 
			 lte_frame_parms,
			 dlsch_ue[0],
			 0,               //harq_pid
			 NB_RB);             //NB allocated RBs
  }
}
