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

/*! \file pdcp_thread.c
* \brief
* \author F. Kaltenberger
* \date 2013
* \version 0.1
* \company Eurecom
* \email: florian.kaltenberger@eurecom.fr
* \note
* \warning
*/
#include <pthread.h>
//#include <inttypes.h>

#include "pdcp.h"
#include "PHY/extern.h" //for PHY_vars
#include "UTIL/LOG/log.h"
#include "UTIL/LOG/vcd_signal_dumper.h"

#define OPENAIR_THREAD_STACK_SIZE    8192
#define OPENAIR_THREAD_PRIORITY        255

extern int oai_exit;

#ifdef RTAI
static int pdcp_thread;
SEM *pdcp_sem;
#else
pthread_t pdcp_thread;
pthread_attr_t pdcp_thread_attr;
pthread_mutex_t pdcp_mutex;
pthread_cond_t pdcp_cond;
#endif

int pdcp_instance_cnt;

static void *pdcp_thread_main(void* param) {

#ifdef RTAI
  RT_TASK *task;
  task = rt_task_init_schmod(nam2num("TASK_PDCP"), 0, 0, 0, SCHED_FIFO, 0xF);
#endif
  //u8 eNB_flag = *((u8*)param);
  u8 eNB_flag = 1;

  while (!oai_exit) {

#ifdef RTAI
    rt_sem_wait(pdcp_sem);
#else
    if (pthread_mutex_lock(&pdcp_mutex) != 0) {
        LOG_E(PDCP,"Error locking mutex.\n");
    }
    else {
        while (pdcp_instance_cnt < 0) {
            pthread_cond_wait(&pdcp_cond,&pdcp_mutex);
        }
        if (pthread_mutex_unlock(&pdcp_mutex) != 0) {	
            LOG_E(PDCP,"Error unlocking mutex.\n");
        }
    }
#endif
    if (oai_exit) break;

    if (eNB_flag) {
      pdcp_run(PHY_vars_eNB_g[0]->frame, eNB_flag, PHY_vars_eNB_g[0]->Mod_id, 0);
      //LOG_I(PDCP,"Calling pdcp_run (eNB) for frame %d\n",PHY_vars_eNB_g[0]->frame);
    }
    else  {
      pdcp_run(PHY_vars_UE_g[0]->frame, eNB_flag, 0, PHY_vars_UE_g[0]->Mod_id);
      //LOG_I(PDCP,"Calling pdcp_run (UE) for frame %d\n",PHY_vars_UE_g[0]->frame);
    }

#ifdef RTAI
#else
    if (pthread_mutex_lock(&pdcp_mutex) != 0) {
      LOG_E(PDCP,"Error locking mutex.\n");
    }
    else {
        pdcp_instance_cnt--;
        if (pthread_mutex_unlock(&pdcp_mutex) != 0) {	
	  LOG_E(PDCP,"Error unlocking mutex.\n");
        }
    }
#endif
  }

  LOG_I(PDCP, "PDCP thread existing\n");
#ifdef RTAI
  rt_task_delete(task);
#endif
  return(NULL);
}



int init_pdcp_thread(u8 eNB_flag) {

    int error_code;
    struct sched_param p;

#ifdef RTAI
    
    LOG_I(PDCP,"Allocate PDCP thread successful\n");
    pdcp_thread = rt_thread_create(pdcp_thread_main, NULL, 10000);
    pdcp_sem = rt_sem_init(nam2num("PDCP_SEM"), 1);
    if (pdcp_sem == 0){
      LOG_I(PDCP,"Could not allocate PDCP thread, error %d\n",error_code);
    }
    else {
      LOG_I(PDCP,"Allocating PDCP thread\n");
    }
#else
    pthread_attr_init (&pdcp_thread_attr);
    pthread_attr_setstacksize(&pdcp_thread_attr,OPENAIR_THREAD_STACK_SIZE);
    //attr_dlsch_threads.priority = 1;
    
    p.sched_priority = OPENAIR_THREAD_PRIORITY;
    pthread_attr_setschedparam  (&pdcp_thread_attr, &p);
#ifndef RTAI_ISNT_POSIX
    pthread_attr_setschedpolicy (&pdcp_thread_attr, SCHED_FIFO);
#endif
    pthread_mutex_init(&pdcp_mutex,NULL);
    pthread_cond_init(&pdcp_cond,NULL);

    pdcp_instance_cnt = -1;
    LOG_I(PDCP,"Allocating PDCP thread\n");
    error_code = pthread_create(&pdcp_thread,
				&pdcp_thread_attr,
				pdcp_thread_main,
				(void*)(&eNB_flag));
      
    if (error_code!= 0) {
      LOG_I(PDCP,"Could not allocate PDCP thread, error %d\n",error_code);
      return(error_code);
    }
    else {
      LOG_I(PDCP,"Allocate PDCP thread successful\n");
    }
#endif  
    return(0);
}

void cleanup_pdcp_thread(void) {
  void *status;

  LOG_I(PDCP,"Scheduling PDCP thread to exit\n");
  
#ifdef RTAI
  rt_sem_signal(pdcp_sem);
  rt_thread_join(pdcp_thread);
  rt_sem_delete(pdcp_sem);
#else
  pdcp_instance_cnt = 0;
  if (pthread_cond_signal(&pdcp_cond) != 0)
    LOG_I(PDCP,"ERROR pthread_cond_signal\n");
  else
    LOG_I(PDCP,"Signalled PDCP thread to exit\n");

  pthread_join(pdcp_thread,&status);
  LOG_I(PDCP,"PDCP thread exited\n");
  pthread_cond_destroy(&pdcp_cond);
  pthread_mutex_destroy(&pdcp_mutex);
#endif
}
