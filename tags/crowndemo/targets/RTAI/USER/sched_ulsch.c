/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file sched_ulsch.c
* \brief ULSCH decoding thread (RTAI)
* \author R. Knopp, F. Kaltenberger
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr
* \note
* \warning
*/
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#include <rtai_lxrt.h>
#include <rtai_sem.h>
#include <rtai_msg.h>

/*#ifdef RTAI_ENABLED
#include <rtai.h>
#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif */

#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"

#include "MAC_INTERFACE/extern.h"

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#endif // CBMIMO1



#define DEBUG_PHY

/// Mutex for instance count on ulsch scheduling 
pthread_mutex_t ulsch_mutex[8];
/// Condition variable for ulsch thread
pthread_cond_t ulsch_cond[8];

pthread_t ulsch_threads[8];
pthread_attr_t attr_ulsch_threads;

// activity indicators for harq_pid's
int ulsch_instance_cnt[8];
// process ids for cpu
int ulsch_cpuid[8];
// subframe number for each harq_pid (needed to store ack in right place for UL)
int ulsch_subframe[8];

extern int oai_exit;

/*
extern int ulsch_errors;
extern int ulsch_received;
extern int ulsch_errors_last;
extern int ulsch_received_last;
extern int ulsch_fer;
extern int current_ulsch_cqi;
*/

/** ULSCH Decoding Thread */
static void * ulsch_thread(void *param) {

  //unsigned long cpuid;
  unsigned int ulsch_thread_index = (unsigned int)param;

  RTIME time_in,time_out;
  RT_TASK *task;

  int eNB_id = 0, UE_id = 0;
  PHY_VARS_eNB *phy_vars_eNB = PHY_vars_eNB_g[eNB_id];

  char ulsch_thread_name[64];

  sprintf(ulsch_thread_name,"ULSCH_THREAD%d",ulsch_thread_index);

  task = rt_task_init_schmod(nam2num(ulsch_thread_name), 0, 0, 0, SCHED_FIFO, 0xF);
  mlockall(MCL_CURRENT | MCL_FUTURE);

  //rt_set_runnable_on_cpuid(task,1);
  //cpuid = rtai_cpuid();

#ifdef HARD_RT
  rt_make_hard_real_time();
#endif

  LOG_I(PHY,"[SCHED][ULSCH] ulsch_thread for process %d started with id %p\n",
      ulsch_thread_index,
      task);

  if ((ulsch_thread_index <0) || (ulsch_thread_index>NUMBER_OF_UE_MAX)) {
    LOG_E(PHY,"[SCHED][ULSCH] Illegal ulsch_thread_index %d!!!!\n",ulsch_thread_index);
    return 0;
  }

  //ulsch_cpuid[ulsch_thread_index] = cpuid;

  while (!oai_exit){
    
    if (pthread_mutex_lock(&ulsch_mutex[ulsch_thread_index]) != 0) {
      LOG_E(PHY,"[SCHED][ULSCH] error locking mutex.\n");
    }
    else {

      while (ulsch_instance_cnt[ulsch_thread_index] < 0) {
	pthread_cond_wait(&ulsch_cond[ulsch_thread_index],&ulsch_mutex[ulsch_thread_index]);
      }

      if (pthread_mutex_unlock(&ulsch_mutex[ulsch_thread_index]) != 0) {	
	LOG_E(PHY,"[SCHED][ULSCH] error unlocking mutex.\n");
      }
    }

    if (oai_exit) break;

    LOG_D(PHY,"[SCHED][ULSCH] Frame %d: Calling ulsch_decoding with ulsch_thread_index = %d\n",phy_vars_eNB->frame,ulsch_thread_index);

    time_in = rt_get_time();

    ulsch_decoding_procedures(ulsch_subframe[ulsch_thread_index]<<1,ulsch_thread_index,phy_vars_eNB,0);
    
    time_out = rt_get_time();

    if (pthread_mutex_lock(&ulsch_mutex[ulsch_thread_index]) != 0) {
      msg("[openair][SCHED][ULSCH] error locking mutex.\n");
    }
    else {
      ulsch_instance_cnt[ulsch_thread_index]--;
      
      if (pthread_mutex_unlock(&ulsch_mutex[ulsch_thread_index]) != 0) {	
	msg("[openair][SCHED][ULSCH] error unlocking mutex.\n");
      }
    }
  }

#ifdef HARD_RT
  rt_make_soft_real_time();
#endif

  msg("[openair][SCHED][ULSCH] ULSCH thread %d exiting\n",ulsch_thread_index);  

  return 0;
}

int init_ulsch_threads(void) {
  
  int error_code, return_code=0;
  struct sched_param p;
  int ulsch_thread_index;

  // later loop on all harq_pids, do 0 for now
  for (ulsch_thread_index=0;ulsch_thread_index<NUMBER_OF_UE_MAX;ulsch_thread_index++) {

    pthread_mutex_init(&ulsch_mutex[ulsch_thread_index],NULL);
  
    pthread_cond_init(&ulsch_cond[ulsch_thread_index],NULL);

    pthread_attr_init (&attr_ulsch_threads);
    pthread_attr_setstacksize(&attr_ulsch_threads,OPENAIR_THREAD_STACK_SIZE);
  
    //attr_ulsch_threads.priority = 1;

    p.sched_priority = OPENAIR_THREAD_PRIORITY;
    pthread_attr_setschedparam  (&attr_ulsch_threads, &p);
#ifndef RTAI_ISNT_POSIX
    pthread_attr_setschedpolicy (&attr_ulsch_threads, SCHED_FIFO);
#endif
    
    ulsch_instance_cnt[ulsch_thread_index] = -1;
    rt_printk("[openair][SCHED][ULSCH][INIT] Allocating ULSCH thread for ulsch_thread_index %d\n",ulsch_thread_index);
    error_code = pthread_create(&ulsch_threads[ulsch_thread_index],
				&attr_ulsch_threads,
				ulsch_thread,
				(void *)ulsch_thread_index);

    if (error_code!= 0) {
      rt_printk("[openair][SCHED][ULSCH][INIT] Could not allocate ulsch_thread %d, error %d\n",ulsch_thread_index,error_code);
      return_code+=error_code;
      //return(error_code);
    }
    else {
      rt_printk("[openair][SCHED][ULSCH][INIT] Allocate ulsch_thread %d successful\n",ulsch_thread_index);
      //return(0);
    }
  }
  return(return_code);
}

void cleanup_ulsch_threads(void) {

  int ulsch_thread_index;

  for (ulsch_thread_index=0;ulsch_thread_index<NUMBER_OF_UE_MAX;ulsch_thread_index++) {

    //  pthread_exit(&ulsch_threads[ulsch_thread_index]);
    rt_printk("[openair][SCHED][ULSCH] Scheduling ulsch_thread %d to exit\n",ulsch_thread_index);

    ulsch_instance_cnt[ulsch_thread_index] = 0;
    if (pthread_cond_signal(&ulsch_cond[ulsch_thread_index]) != 0)
      rt_printk("[openair][SCHED][ULSCH] ERROR pthread_cond_signal\n");
    else
      rt_printk("[openair][SCHED][ULSCH] Signalled ulsch_thread %d to exit\n",ulsch_thread_index);
    
    rt_printk("[openair][SCHED][ULSCH] Exiting ...\n");
    pthread_cond_destroy(&ulsch_cond[ulsch_thread_index]);
    pthread_mutex_destroy(&ulsch_mutex[ulsch_thread_index]);
  }
}
