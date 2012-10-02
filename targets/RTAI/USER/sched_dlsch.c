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

/*! \file sched_dlsch.c
* \brief DLSCH decoding thread (RTAI)
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

extern int oai_exit;

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

  //unsigned long cpuid;
  unsigned int dlsch_thread_index = 0; //*((int *)param);
  unsigned int ret;
  u8 harq_pid;

  RTIME time_in,time_out;
  RT_TASK *task;

  int eNB_id = 0, UE_id = 0;
  PHY_VARS_UE *phy_vars_ue = PHY_vars_UE_g[UE_id];

  if ((dlsch_thread_index <0) || (dlsch_thread_index>7)) {
    LOG_E(PHY,"[SCHED][DLSCH] Illegal dlsch_thread_index %d!!!!\n",dlsch_thread_index);
    return 0;
  }

  task = rt_task_init_schmod(nam2num("DLSCH_THREAD"), 0, 0, 0, SCHED_FIFO, 0xF);

  if (task==NULL) {
    LOG_E(PHY,"[SCHED][DLSCH] Problem starting dlsch_thread_index %d!!!!\n",dlsch_thread_index);
    return 0;
  }
  else {
    LOG_I(PHY,"[SCHED][DLSCH] dlsch_thread for process %d started with id %p\n",
	  dlsch_thread_index,
	  task);
  }

  mlockall(MCL_CURRENT | MCL_FUTURE);

  //rt_set_runnable_on_cpuid(task,1);
  //cpuid = rtai_cpuid();

#ifdef HARD_RT
  rt_make_hard_real_time();
#endif

  //dlsch_cpuid[dlsch_thread_index] = cpuid;

  while (!oai_exit){
    
    if (pthread_mutex_lock(&dlsch_mutex[dlsch_thread_index]) != 0) {
      LOG_E(PHY,"[SCHED][DLSCH] error locking mutex.\n");
    }
    else {

      while (dlsch_instance_cnt[dlsch_thread_index] < 0) {
	pthread_cond_wait(&dlsch_cond[dlsch_thread_index],&dlsch_mutex[dlsch_thread_index]);
      }

      if (pthread_mutex_unlock(&dlsch_mutex[dlsch_thread_index]) != 0) {	
	LOG_E(PHY,"[SCHED][DLSCH] error unlocking mutex.\n");
      }
    }

    if (oai_exit) break;

    LOG_D(PHY,"[SCHED][DLSCH] Frame %d: Calling dlsch_decoding with dlsch_thread_index = %d\n",phy_vars_ue->frame,dlsch_thread_index);

    time_in = rt_get_time();

    if (phy_vars_ue->frame < phy_vars_ue->dlsch_errors[eNB_id]) {
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
			 phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->llr[0],
			 0,
			 dlsch_subframe[dlsch_thread_index]<<1);
      LOG_D(PHY,"[UE %d] Calling dlsch_decoding for subframe %d\n",phy_vars_ue->Mod_id,dlsch_subframe[dlsch_thread_index]);
      ret = dlsch_decoding(phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->llr[0],
			   &phy_vars_ue->lte_frame_parms,
				 phy_vars_ue->dlsch_ue[eNB_id][0],
			   dlsch_subframe[dlsch_thread_index],
			   phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);
      

      if (ret == (1+MAX_TURBO_ITERATIONS)) {
	phy_vars_ue->dlsch_errors[eNB_id]++;
	
#ifdef DEBUG_PHY
	LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d DLSCH in error (rv %d,mcs %d)\n",
	      phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
	      harq_pid,phy_vars_ue->frame,dlsch_subframe[dlsch_thread_index],
	      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
	      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs);
#endif
      }
      else {
	LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d: Received DLSCH (rv %d,mcs %d)\n",
	      phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
	      harq_pid,phy_vars_ue->frame,dlsch_subframe[dlsch_thread_index],
	      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
	      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs);
	
#ifdef OPENAIR2
	mac_xface->ue_send_sdu(phy_vars_ue->Mod_id,
			       phy_vars_ue->frame,
			       phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->b,
			       phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->TBS>>3,
			       eNB_id);
#endif
	phy_vars_ue->total_TBS[eNB_id] =  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[0]->TBS + phy_vars_ue->total_TBS[eNB_id];
	phy_vars_ue->total_received_bits[eNB_id] = phy_vars_ue->total_received_bits[eNB_id] + phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[0]->TBS;
      }
    }
    
    if (phy_vars_ue->frame % 100 == 0) {
      if ((phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]) != 0) 
	phy_vars_ue->dlsch_fer[eNB_id] = (100*(phy_vars_ue->dlsch_errors[eNB_id] - phy_vars_ue->dlsch_errors_last[eNB_id]))/(phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]);
      phy_vars_ue->dlsch_errors_last[eNB_id] = phy_vars_ue->dlsch_errors[eNB_id];
      phy_vars_ue->dlsch_received_last[eNB_id] = phy_vars_ue->dlsch_received[eNB_id];
      
    }

    if(phy_vars_ue->frame % 10 == 0){
      phy_vars_ue->bitrate[eNB_id] = (phy_vars_ue->total_TBS[eNB_id] - phy_vars_ue->total_TBS_last[eNB_id])*10;
      phy_vars_ue->total_TBS_last[eNB_id] = phy_vars_ue->total_TBS[eNB_id];
    }
    
    time_out = rt_get_time();

#ifdef DEBUG_PHY
    LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d: PDSCH/DLSCH decoding iter %d (mcs %d, rv %d, TBS %d)\n",
	  phy_vars_ue->Mod_id,
	  phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,harq_pid,
	  phy_vars_ue->frame,dlsch_subframe[dlsch_thread_index],ret,
	  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,
	  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
	  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS);
    if (phy_vars_ue->frame%100==0) {
      LOG_I(PHY,"[UE  %d][PDSCH %x] Frame %d subframe %d dlsch_errors %d, dlsch_received %d, dlsch_fer %d, current_dlsch_cqi %d\n",
	    phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
	    phy_vars_ue->frame,dlsch_subframe[dlsch_thread_index],
	    phy_vars_ue->dlsch_errors[eNB_id],
	    phy_vars_ue->dlsch_received[eNB_id],
	    phy_vars_ue->dlsch_fer[eNB_id],
	    phy_vars_ue->PHY_measurements.wideband_cqi_tot[eNB_id]);
    }
#endif 
    
    if (pthread_mutex_lock(&dlsch_mutex[dlsch_thread_index]) != 0) {
      msg("[openair][SCHED][DLSCH] error locking mutex.\n");
    }
    else {
      dlsch_instance_cnt[dlsch_thread_index]--;
      
      if (pthread_mutex_unlock(&dlsch_mutex[dlsch_thread_index]) != 0) {	
	msg("[openair][SCHED][DLSCH] error unlocking mutex.\n");
      }
    }
  }

#ifdef HARD_RT
  rt_make_soft_real_time();
#endif

  msg("[openair][SCHED][DLSCH] DLSCH thread %d exiting\n",dlsch_thread_index);  

  return 0;
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
  
  //attr_dlsch_threads.priority = 1;

  p.sched_priority = OPENAIR_THREAD_PRIORITY;
  pthread_attr_setschedparam  (&attr_dlsch_threads, &p);
#ifndef RTAI_ISNT_POSIX
  pthread_attr_setschedpolicy (&attr_dlsch_threads, SCHED_FIFO);
#endif

  dlsch_instance_cnt[dlsch_thread_index] = -1;
  rt_printk("[openair][SCHED][DLSCH][INIT] Allocating DLSCH thread for dlsch_thread_index %d\n",dlsch_thread_index);
  error_code = pthread_create(&dlsch_threads[dlsch_thread_index],
  			      &attr_dlsch_threads,
  			      dlsch_thread,
  			      (void *)&dlsch_thread_index);

  if (error_code!= 0) {
    rt_printk("[openair][SCHED][DLSCH][INIT] Could not allocate dlsch_thread %d, error %d\n",dlsch_thread_index,error_code);
    return(error_code);
  }
  else {
    rt_printk("[openair][SCHED][DLSCH][INIT] Allocate dlsch_thread %d successful\n",dlsch_thread_index);
    return(0);
  }
   
}

void cleanup_dlsch_threads(void) {

  int dlsch_thread_index = 0;

  // later loop on all harq_pid's

  //  pthread_exit(&dlsch_threads[dlsch_thread_index]);
  rt_printk("[openair][SCHED][DLSCH] Scheduling dlsch_thread %d to exit\n",dlsch_thread_index);

  dlsch_instance_cnt[dlsch_thread_index] = 0;
  if (pthread_cond_signal(&dlsch_cond[dlsch_thread_index]) != 0)
    rt_printk("[openair][SCHED][DLSCH] ERROR pthread_cond_signal\n");
  else
    rt_printk("[openair][SCHED][DLSCH] Signalled dlsch_thread %d to exit\n",dlsch_thread_index);
    
  rt_printk("[openair][SCHED][DLSCH] Exiting ...\n");
  pthread_cond_destroy(&dlsch_cond[dlsch_thread_index]);
  pthread_mutex_destroy(&dlsch_mutex[dlsch_thread_index]);
}
