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
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "MAC_INTERFACE/extern.h"

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#endif // CBMIMO1



#define DEBUG_PHY

/// Mutex for instance count on rx_pdsch scheduling 
pthread_mutex_t rx_pdsch_mutex[8];
/// Condition variable for rx_pdsch thread
pthread_cond_t rx_pdsch_cond[8];

pthread_t rx_pdsch_threads[8];
pthread_attr_t attr_rx_pdsch_threads;

// activity indicators for harq_pid's
int rx_pdsch_instance_cnt[8];
// process ids for cpu
int rx_pdsch_cpuid[8];
// subframe number for each harq_pid (needed to store ack in right place for UL)
int rx_pdsch_slot[8];

extern int oai_exit;
extern pthread_mutex_t dlsch_mutex[8];
extern int dlsch_instance_cnt[8];
extern int dlsch_subframe[8];
extern pthread_cond_t dlsch_cond[8];
/*
extern int rx_pdsch_errors;
extern int rx_pdsch_received;
extern int rx_pdsch_errors_last;
extern int rx_pdsch_received_last;
extern int rx_pdsch_fer;
extern int current_rx_pdsch_cqi;
*/

/** RX_PDSCH Decoding Thread */
static void * rx_pdsch_thread(void *param) {

  //unsigned long cpuid;
  u8 rx_pdsch_thread_index = 0;
  u8 dlsch_thread_index = 0;
  u8 pilot1,pilot2,pilot3,harq_pid,subframe;
  u8 last_slot;

  u8 dual_stream_UE = 0;
  u8 i_mod = 0;

  RTIME time_in,time_out;
  RT_TASK *task;

  int m,eNB_id = 0;
  int eNB_id_i = 1;
  PHY_VARS_UE *phy_vars_ue = PHY_vars_UE_g[0];

  if ((rx_pdsch_thread_index <0) || (rx_pdsch_thread_index>7)) {
    LOG_E(PHY,"[SCHED][RX_PDSCH] Illegal rx_pdsch_thread_index %d!!!!\n",rx_pdsch_thread_index);
    return 0;
  }

  task = rt_task_init_schmod(nam2num("RX_PDSCH_THREAD"), 0, 0, 0, SCHED_FIFO, 0xF);

  if (task==NULL) {
    LOG_E(PHY,"[SCHED][RX_PDSCH] Problem starting rx_pdsch_thread_index %d!!!!\n",rx_pdsch_thread_index);
    return 0;
  }
  else {
    LOG_I(PHY,"[SCHED][RX_PDSCH] rx_pdsch_thread for process %d started with id %p\n",
	  rx_pdsch_thread_index,
	  task);
  }

  mlockall(MCL_CURRENT | MCL_FUTURE);

  //rt_set_runnable_on_cpuid(task,1);
  //cpuid = rtai_cpuid();

#ifdef HARD_RT
  rt_make_hard_real_time();
#endif

  //rx_pdsch_cpuid[rx_pdsch_thread_index] = cpuid;

  if (phy_vars_ue->lte_frame_parms.Ncp == 0) {  // normal prefix
    pilot1 = 4;
    pilot2 = 7;
    pilot3 = 11;
  }
  else {  // extended prefix
    pilot1 = 3;
    pilot2 = 6;
    pilot3 = 9;
  }


  while (!oai_exit){
    
    if (pthread_mutex_lock(&rx_pdsch_mutex[rx_pdsch_thread_index]) != 0) {
      LOG_E(PHY,"[SCHED][RX_PDSCH] error locking mutex.\n");
    }
    else {

      while (rx_pdsch_instance_cnt[rx_pdsch_thread_index] < 0) {
	pthread_cond_wait(&rx_pdsch_cond[rx_pdsch_thread_index],&rx_pdsch_mutex[rx_pdsch_thread_index]);
      }

      if (pthread_mutex_unlock(&rx_pdsch_mutex[rx_pdsch_thread_index]) != 0) {	
	LOG_E(PHY,"[SCHED][RX_PDSCH] error unlocking mutex.\n");
      }
    }

    last_slot = rx_pdsch_slot[rx_pdsch_thread_index];
    subframe = last_slot>>1;
    harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid;
    
    if ((phy_vars_ue->transmission_mode[eNB_id] == 5) && 
	(phy_vars_ue->dlsch_ue[eNB_id][0]->dl_power_off==0) &&
	(openair_daq_vars.use_ia_receiver ==1)) {
      dual_stream_UE = 1;
      eNB_id_i = phy_vars_ue->n_connected_eNB;
      i_mod =  get_Qm(phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs);
    }
    else {
      dual_stream_UE = 0;
      eNB_id_i = eNB_id+1;
      i_mod = 0;
    }


    if (oai_exit) break;

    LOG_D(PHY,"[SCHED][RX_PDSCH] Frame %d, slot %d: Calling rx_pdsch_decoding with rx_pdsch_thread_index = %d, harq_pid %d\n",phy_vars_ue->frame,last_slot,rx_pdsch_thread_index,harq_pid);

    time_in = rt_get_time();

    // Check if we are in even or odd slot
    if (last_slot%2) { // odd slots

      for (m=pilot2;m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++) {

	rx_pdsch(phy_vars_ue,
		 PDSCH,
		 eNB_id,
		 eNB_id_i,
		 subframe,
		 m,
		 0,
		 dual_stream_UE,
		 i_mod);

      }
      // trigger DLSCH decoding thread
      //phy_vars_ue->dlsch_ue[eNB_id][0]->active = 0;

      dlsch_thread_index = 0;
	
      if (pthread_mutex_lock (&dlsch_mutex[dlsch_thread_index]) != 0) {               // Signal MAC_PHY Scheduler
	LOG_E(PHY,"[UE  %d] ERROR pthread_mutex_lock\n",phy_vars_ue->Mod_id);     // lock before accessing shared resource
	//	vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
	//return(-1);
      }
      dlsch_instance_cnt[dlsch_thread_index]++;
      //dlsch_subframe[dlsch_thread_index] = (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1));
      dlsch_subframe[dlsch_thread_index] = subframe;
      pthread_mutex_unlock (&dlsch_mutex[dlsch_thread_index]);
	
      if (dlsch_instance_cnt[dlsch_thread_index] == 0) {
	if (pthread_cond_signal(&dlsch_cond[dlsch_thread_index]) != 0) {
	  LOG_E(PHY,"[UE  %d] ERROR pthread_cond_signal for dlsch_cond[%d]\n",phy_vars_ue->Mod_id,dlsch_thread_index);
	  //	  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
	  //return(-1);
	}
      }
      else {
	LOG_W(PHY,"[UE  %d] DLSCH thread for dlsch_thread_index %d busy!!!\n",phy_vars_ue->Mod_id,dlsch_thread_index);
	//	vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
	//return(-1);
      }
      

    
    } else { // even slots

      for (m=phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols;m<pilot2;m++) { 

	rx_pdsch(phy_vars_ue,
		 PDSCH,
		 eNB_id,
		 eNB_id_i,
		 subframe,
		 m,
		 (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,   // first_symbol_flag
		 dual_stream_UE,
		 i_mod);
      }
    }    

    time_out = rt_get_time();
    
    if (pthread_mutex_lock(&rx_pdsch_mutex[rx_pdsch_thread_index]) != 0) {
      msg("[openair][SCHED][RX_PDSCH] error locking mutex.\n");
    }
    else {
      rx_pdsch_instance_cnt[rx_pdsch_thread_index]--;
      
      if (pthread_mutex_unlock(&rx_pdsch_mutex[rx_pdsch_thread_index]) != 0) {	
	msg("[openair][SCHED][RX_PDSCH] error unlocking mutex.\n");
      }
    }

  }

#ifdef HARD_RT
  rt_make_soft_real_time();
#endif

  msg("[openair][SCHED][RX_PDSCH] RX_PDSCH thread %d exiting\n",rx_pdsch_thread_index);  

  return 0;
}

int init_rx_pdsch_threads(void) {
  
  int error_code;
  struct sched_param p;
  int rx_pdsch_thread_index;

  // later loop on all harq_pids, do 0 for now
  rx_pdsch_thread_index=0;

  pthread_mutex_init(&rx_pdsch_mutex[rx_pdsch_thread_index],NULL);
  
  pthread_cond_init(&rx_pdsch_cond[rx_pdsch_thread_index],NULL);

  pthread_attr_init (&attr_rx_pdsch_threads);
  pthread_attr_setstacksize(&attr_rx_pdsch_threads,OPENAIR_THREAD_STACK_SIZE);
  
  //attr_rx_pdsch_threads.priority = 1;

  p.sched_priority = OPENAIR_THREAD_PRIORITY;
  pthread_attr_setschedparam  (&attr_rx_pdsch_threads, &p);
#ifndef RTAI_ISNT_POSIX
  pthread_attr_setschedpolicy (&attr_rx_pdsch_threads, SCHED_FIFO);
#endif

  rx_pdsch_instance_cnt[rx_pdsch_thread_index] = -1;
  rt_printk("[openair][SCHED][RX_PDSCH][INIT] Allocating RX_PDSCH thread for rx_pdsch_thread_index %d\n",rx_pdsch_thread_index);
  error_code = pthread_create(&rx_pdsch_threads[rx_pdsch_thread_index],
  			      &attr_rx_pdsch_threads,
  			      rx_pdsch_thread,
  			      (void *)&rx_pdsch_thread_index);

  if (error_code!= 0) {
    rt_printk("[openair][SCHED][RX_PDSCH][INIT] Could not allocate rx_pdsch_thread %d, error %d\n",rx_pdsch_thread_index,error_code);
    return(error_code);
  }
  else {
    rt_printk("[openair][SCHED][RX_PDSCH][INIT] Allocate rx_pdsch_thread %d successful\n",rx_pdsch_thread_index);
    return(0);
  }
   
}

void cleanup_rx_pdsch_threads(void) {

  int rx_pdsch_thread_index = 0;

  // later loop on all harq_pid's

  //  pthread_exit(&rx_pdsch_threads[rx_pdsch_thread_index]);
  rt_printk("[openair][SCHED][RX_PDSCH] Scheduling rx_pdsch_thread %d to exit\n",rx_pdsch_thread_index);

  rx_pdsch_instance_cnt[rx_pdsch_thread_index] = 0;
  if (pthread_cond_signal(&rx_pdsch_cond[rx_pdsch_thread_index]) != 0)
    rt_printk("[openair][SCHED][RX_PDSCH] ERROR pthread_cond_signal\n");
  else
    rt_printk("[openair][SCHED][RX_PDSCH] Signalled rx_pdsch_thread %d to exit\n",rx_pdsch_thread_index);
    
  rt_printk("[openair][SCHED][RX_PDSCH] Exiting ...\n");
  pthread_cond_destroy(&rx_pdsch_cond[rx_pdsch_thread_index]);
  pthread_mutex_destroy(&rx_pdsch_mutex[rx_pdsch_thread_index]);
}
