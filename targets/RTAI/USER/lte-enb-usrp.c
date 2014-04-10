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

/*! \file lte-enb
* \brief main program to control HW and scheduling
* \author R. Knopp, F. Kaltenberger
* \date 2012
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr
* \note
* \warning
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>

#include <signal.h>
#include <execinfo.h>
#include <getopt.h>


#ifdef EMOS
#include <gps.h>
#endif

#include "rt_wrapper.h"

#include "PHY/types.h"
#include "PHY/defs.h"

#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#include "SCHED/vars.h"
#include "LAYER2/MAC/vars.h"

#include "../../SIMU/USER/init_lte.h"

#include "../../ARCH/USRP/USERSPACE/LIB/usrp_lib.h"

#ifdef EMOS
#include "SCHED/phy_procedures_emos.h"
#endif

#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/vars.h"
#ifndef CELLULAR
#include "RRC/LITE/vars.h"
#endif
#include "PHY_INTERFACE/vars.h"
#endif

#include "UTIL/LOG/log_extern.h"
#include "UTIL/OTG/otg.h"
#include "UTIL/OTG/otg_vars.h"
#include "UTIL/MATH/oml.h"
#include "UTIL/LOG/vcd_signal_dumper.h"


#ifdef XFORMS
#include "PHY/TOOLS/lte_phy_scope.h"
#include "stats.h"
// current status is that every UE has a DL scope for a SINGLE eNB (eNB_id=0)
// at eNB 0, an UL scope for every UE
FD_lte_phy_scope_ue  *form_ue[NUMBER_OF_UE_MAX];
FD_lte_phy_scope_enb *form_enb[NUMBER_OF_UE_MAX];
FD_stats_form *form_stats=NULL;
char title[255];
int UE_id;
unsigned char scope_enb_num_ue = 1;
#endif //XFORMS

char *mme_ip;
#define FRAME_PERIOD 100000000ULL
#define DAQ_PERIOD 66667ULL

#undef MALLOC //there are two conflicting definitions, so we better make sure we don't use it at all

static int hw_subframe;

#ifdef RTAI
static SEM *tti_sem; // used to wakeup eNB_TX every 1ms
static SEM *sync_sem; // to sync rx & tx streaming

static pthread_t enb_thread_tx;
static pthread_t enb_thread_rx;
static pthread_t thread_trx;
//static int sync_thread;
#else
pthread_cond_t tti_cond;
pthread_mutex_t tti_mutex;
pthread_cond_t sync_cond;
pthread_mutex_t sync_mutex;

pthread_t enb_thread_tx;
pthread_t enb_thread_rx;
pthread_t thread_trx;
pthread_attr_t attr_dlsch_threads;
struct sched_param sched_param_dlsch;
#endif

openair0_device openair0;
openair0_timestamp timestamp;
pthread_t  thread_scope;
pthread_t  thread3;

int samples_per_frame = 307200;
int samples_per_packets = 2048; // samples got every recv or send
int tx_forward_nsamps;

int sf_bounds_5[10] = {8, 15, 23, 30, 38, 45, 53, 60, 68, 75};
int sf_bounds_10[10] = {8, 15, 23, 30, 38, 45, 53, 60, 68, 75};
int sf_bounds_20[10] = {15, 30, 45, 60, 75, 90, 105, 120, 135, 150};
int *sf_bounds;
int max_cnt;
int tx_delay;

int oai_exit = 0;
int oai_flag = 0;


u8 eNB_id=0;

u32 carrier_freq[4];

struct timing_info_t {
  //unsigned int frame, hw_slot, last_slot, next_slot;
  RTIME time_min, time_max, time_avg, time_last, time_now;
  //unsigned int mbox0, mbox1, mbox2, mbox_target;
  unsigned int n_samples;
} timing_info;

int rx_input_level_dBm;
int number_of_cards = 1;
int card;


int init_dlsch_threads(void);
void cleanup_dlsch_threads(void);
s32 init_rx_pdsch_thread(void);
void cleanup_rx_pdsch_thread(void);
int init_ulsch_threads(void);
void cleanup_ulsch_threads(void);

LTE_DL_FRAME_PARMS *frame_parms;

s32 *rxdata;
s32 *txdata;
void setup_eNB_buffers(PHY_VARS_eNB *phy_vars_eNB, LTE_DL_FRAME_PARMS *frame_parms, int carrier);

void signal_handler(int sig)
{
  void *array[10];
  size_t size;

  if (sig == SIGINT) {
    printf("Ctrl-C pressed, Existing.....\n");
    oai_exit = 1;
  }
  if (sig==SIGSEGV) {
    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, 2);
    exit(-1);
  }
  else {
    oai_exit=1;
  }
}

void exit_fun(const char* s)
{

  printf("Exiting: %s\n",s);

  oai_exit=1;
  rt_sleep_ns(FRAME_PERIOD);

  //exit (-1);
}

#ifdef XFORMS
void *scope_thread(void *arg) {
  s16 i;
  char stats_buffer[16384];
  //FILE *UE_stats, *eNB_stats;
  int len=0;

  /*
    if (UE_flag==1)
    UE_stats  = fopen("UE_stats.txt", "w");
    else
    eNB_stats = fopen("eNB_stats.txt", "w");
  */

  while (!oai_exit) {
    len = dump_eNB_stats (PHY_vars_eNB_g[0], stats_buffer, 0);
    fl_set_object_label(form_stats->stats_text, stats_buffer);
    //rewind (eNB_stats);
    //fwrite (stats_buffer, 1, len, eNB_stats);
    for(UE_id=0;UE_id<scope_enb_num_ue;UE_id++) {
        phy_scope_eNB(form_enb[UE_id],
                      PHY_vars_eNB_g[eNB_id],
                      UE_id);

      }
      //printf("doing forms\n");
    sleep(1);
  }

  //fclose (UE_stats);
  //fclose (eNB_stats);

  pthread_exit((void*)arg);
}
#endif

void do_OFDM_mod(mod_sym_t **txdataF, s32 **txdata, u16 next_slot, LTE_DL_FRAME_PARMS *frame_parms)
{
  int aa, slot_offset, slot_offset_F;

  slot_offset_F = (next_slot)*(frame_parms->ofdm_symbol_size)*((frame_parms->Ncp == EXTENDED) ? 6 : 7);
  slot_offset   = (next_slot)*(frame_parms->samples_per_tti>>1);

  for (aa = 0; aa < frame_parms->nb_antennas_tx; aa++) {
    if (frame_parms->Ncp == EXTENDED) {
      PHY_ofdm_mod(&txdataF[aa][slot_offset_F],
                   &txdata[aa][slot_offset],
                   frame_parms->log2_symbol_size,
                   6,
                   frame_parms->nb_prefix_samples,
                   frame_parms->twiddle_fft,
                   frame_parms->rev,
                   CYCLIC_PREFIX);
    }
    else {
      normal_prefix_mod(&txdataF[aa][slot_offset_F],
                        &txdata[aa][slot_offset],
                        7,
                        frame_parms);
    }
  }
}


/* This is the main eNB thread. It gets woken up by the kernel driver using the RTAI message mechanism (rt_send and rt_receive). */
static void eNB_TX(void *arg)
{
  //return 0;
#ifdef RTAI
  RT_TASK *task;
#endif
  unsigned char next_slot;
  int frame=0;
  RTIME time_in;
  int subframe;

  printf("eNB TX thread created.\n");
#ifdef RTAI
  task = rt_task_init_schmod(nam2num("ENBTX"), 0, 0, 0, SCHED_FIFO, 1<<3);
  LOG_D(HW,"Started eNB TX thread (id %p)\n",task);
#endif

#ifdef HARD_RT
  rt_make_hard_real_time();
#endif

  mlockall(MCL_CURRENT | MCL_FUTURE);

  //rt_set_runnable_on_cpuid(task, 1);

  timing_info.time_min = 100000000ULL;
  timing_info.time_max = 0;
  timing_info.time_avg = 0;
  timing_info.n_samples = 0;


  time_in = rt_get_time_ns();

  while (!oai_exit)
  {

#ifdef RTAI
    rt_sem_wait(tti_sem);

    subframe = hw_subframe + 2;
#else
    //pthread_mutex_lock(&tti_mutex);
    pthread_cond_wait(&tti_cond, &tti_mutex);

    subframe = hw_subframe + 1;
    //pthread_mutex_unlock(&tti_mutex);
#endif

    if(subframe >= 10)
      subframe -= 10;
    LOG_D(HW,"Time: %.3f: Frame %d, subframe %d TX processing time %0.3f\n",(float)(rt_get_time_ns()-time_in)/1e6,
        frame,subframe,(timing_info.time_now-timing_info.time_last)/1e6);

    next_slot = ((subframe<<1))%LTE_SLOTS_PER_FRAME;

    if (1)
    {

      timing_info.time_last = rt_get_time_ns();

      if ((subframe_select(&PHY_vars_eNB_g[0]->lte_frame_parms,next_slot>>1)==SF_DL)||
          ((subframe_select(&PHY_vars_eNB_g[0]->lte_frame_parms,next_slot>>1)==SF_S)&&((next_slot&1)==0)))
      {

        phy_procedures_eNB_TX(next_slot,PHY_vars_eNB_g[0],0);
        do_OFDM_mod(PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0],
                    PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0],
                    next_slot,
                    &PHY_vars_eNB_g[0]->lte_frame_parms);

        do_OFDM_mod(PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0],
                    PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0],
                    next_slot+1,
                    &PHY_vars_eNB_g[0]->lte_frame_parms);

      }

      timing_info.time_now = rt_get_time_ns();
    }

    if(subframe==9)
      frame++;

    if(frame == 1024) {
      frame = 0;
      time_in = rt_get_time_ns();
    }

    PHY_vars_eNB_g[0]->frame = frame;
  }

  LOG_D(HW,"eNB_TX: finished, ran %d times.\n",frame);

#ifdef HARD_RT
  rt_make_soft_real_time();
#endif

  // clean task
#ifdef RTAI
  rt_task_delete(task);
#endif
  LOG_D(HW,"eNB TX Task deleted. returning\n");
}

/* This is the main eNB thread. It gets woken up by the kernel driver using the RTAI message mechanism (rt_send and rt_receive). */
static void eNB_RX(void *arg)
{
  //return 0;
#ifdef RTAI
  RT_TASK *task;
#endif
  unsigned char last_slot;
  int frame=0;
  RTIME time_in;
  int subframe;

  printf("eNB RX thread created.\n");
#ifdef RTAI
  task = rt_task_init_schmod(nam2num("ENBRX"), 0, 0, 0, SCHED_FIFO, 1<<1);
  LOG_D(HW,"Started eNB RX thread (id %p)\n",task);
#endif

#ifdef HARD_RT
  rt_make_hard_real_time();
#endif

  mlockall(MCL_CURRENT | MCL_FUTURE);

  //rt_set_runnable_on_cpuid(task, 3);
  timing_info.time_min = 100000000ULL;
  timing_info.time_max = 0;
  timing_info.time_avg = 0;
  timing_info.n_samples = 0;


  time_in = rt_get_time_ns();

  while (!oai_exit)
  {

#ifdef RTAI
    rt_sem_wait(tti_sem);

    subframe = (hw_subframe-1+10)%10;
#else
    //pthread_mutex_lock(&tti_mutex);
    pthread_cond_wait(&tti_cond, &tti_mutex);

    subframe = (hw_subframe-1+10)%10;

    //pthread_mutex_unlock(&tti_mutex);
#endif

    LOG_D(HW,"Time: %.3f: Frame %d, subframe %d RX processing time %0.3f\n",(float)(rt_get_time_ns()-time_in)/1e6,
        frame,subframe,(timing_info.time_now-timing_info.time_last)/1e6);

    last_slot = (subframe<<1)+1;
    if (last_slot <0)
      last_slot+=20;

    if (1)
    {

      timing_info.time_last = rt_get_time_ns();


      if (((PHY_vars_eNB_g[0]->lte_frame_parms.frame_type == TDD)&&(subframe_select(&PHY_vars_eNB_g[0]->lte_frame_parms,last_slot>>1)==SF_UL))||
         (PHY_vars_eNB_g[0]->lte_frame_parms.frame_type == FDD))
      {

        phy_procedures_eNB_RX(last_slot,PHY_vars_eNB_g[0],0);

      }

      timing_info.time_now = rt_get_time_ns();
      /*
      if (frame%100==0)
        LOG_D(HW,"hw_slot %d (after): DAQ_MBOX %d\n",hw_slot,DAQ_MBOX[0]);
      */
    }

    //PHY_vars_eNB_g[0]->frame = frame;
    //if(subframe==9)
    //  frame++;
    //if (frame == 100) {
    //  write_output("/tmp/txsig0.m", "txs0", &PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0][0], PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti*10,1,1);
    //  write_output("/tmp/rxsig0.m", "rxs0", &PHY_vars_eNB_g[0]->lte_eNB_common_vars.rxdata[0][0][0], PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti*10,1,1);
    //  oai_exit = 1;
    //}
    /*
    if ((slot%2000)<10)
    LOG_D(HW,"fun0: doing very hard work\n");
    */


    if (oai_flag == 2) {
      //dump_ulsch(PHY_vars_eNB_g[0], subframe, 0);
      exit(-1);
      //oai_exit=1;
    }

    if (oai_flag == 1)
      oai_flag = 2;

    if(frame == 1024) {
      frame = 0;
      time_in = rt_get_time_ns();
    }

  }

  LOG_D(HW,"eNB_RX: finished, ran %d times.\n",frame);

#ifdef HARD_RT
  rt_make_soft_real_time();
#endif

  // clean task
#ifdef RTAI
  rt_task_delete(task);
#endif
  LOG_D(HW,"eNB TX Task deleted. returning\n");
}
static void trx_thread_func(void *arg)
{

  //return 0;
  RTIME time_in;
  // enter hard realtime mode
  printf("trx thread created.\n");
#ifdef RTAI
  RT_TASK *task;
  task = rt_task_init_schmod(nam2num("TASKRX"), 0, 0, 0, SCHED_FIFO, 1<<4);
  LOG_D(HW,"Started trx thread (id %p)\n",task);
#endif

#ifdef HARD_RT
  rt_make_hard_real_time();
#endif


  mlockall(MCL_CURRENT | MCL_FUTURE);

  //rt_set_runnable_on_cpuid(task, 4);
  int rx_cnt = 0;
  int tx_cnt = tx_delay;
  hw_subframe = 0;
#ifdef RTAI
  rt_sem_wait(sync_sem);
#else
  //pthread_mutex_lock(&sync_mutex);
  pthread_cond_wait(&sync_cond, &sync_mutex);
  //pthread_mutex_unlock(&sync_mutex);
#endif

  time_in = rt_get_time_ns();
  while(!oai_exit) {
    openair0.trx_read_func(&openair0, &timestamp, &rxdata[rx_cnt*samples_per_packets], samples_per_packets);

    openair0.trx_write_func(&openair0, (timestamp+samples_per_packets*tx_delay-tx_forward_nsamps), &txdata[tx_cnt*samples_per_packets], samples_per_packets, 1);

    rx_cnt++;
    tx_cnt++;

    if(rx_cnt == sf_bounds[hw_subframe]) {
#ifndef RTAI
      //pthread_mutex_lock(&tti_mutex);
#endif
      hw_subframe++;
      if(hw_subframe==10)
        hw_subframe = 0;
#ifndef RTAI
      //pthread_mutex_unlock(&tti_mutex);
#endif
#ifdef RTAI
      rt_sem_broadcast(tti_sem);
#else
      pthread_cond_broadcast(&tti_cond);

#endif
    }

    if(rx_cnt == max_cnt) {
      rx_cnt = 0;
    }
    if(tx_cnt == max_cnt)
      tx_cnt = 0;
  }

#ifdef HARD_RT
  rt_make_soft_real_time();
#endif

  // clean task
#ifdef RTAI
  rt_task_delete(task);
#endif
  LOG_D(HW,"RX task deleted. returning\n");
  printf("RX thread stopped.\n");
}

int main(int argc, char **argv) {

#ifdef RTAI
  RT_TASK *task;
#endif
  int i,j,aa;
  void *status;

  u8 frame_type = FDD;
  u8 tdd_config = 3;
  u8 tdd_config_S = 0;
  u8 extended_prefix_flag = 0;
  u16 Nid_cell = 0;
  u8 N_RB_DL = 100;
  u8  cooperation_flag = 0;
  u8 transmission_mode = 1;
  u8 abstraction_flag = 0;
  u8 nb_antennas_rx = 1;

  int c;
  char do_forms=0;

  int amp;

  int ret, ant;
  int ant_offset = 0;


  // default RF parameters
  double tx_freq = 2e9;
  double rx_freq= 2e9;
  double sample_rate = 30.72e6;
  double tx_gain = 50;
  double rx_gain = 30;
  double bw = 14e6;
  char ref[128] = "internal";
  char channels[128] = "0";
  //strncpy(uhd_ref, ref, strlen(ref)+1);
  //strncpy(uhd_channels, channels, strlen(channels)+1);


  mme_ip = "146.208.175.6";
  //mme_ip = "10.60.7.1";

  while ((c = getopt(argc, argv, "dC:T:R:i:g:G:s:")) != -1)
  {
    switch (c)
    {
    case 'd':
      do_forms=1;
      break;
    case 'C':
      tx_freq = atof(optarg);
      rx_freq = tx_freq;
      //rx_freq = tx_freq;
      break;
    case 'T':
      frame_type = TDD; // default FDD
      tdd_config = atoi(optarg);
      rx_freq = tx_freq;
      break;
    case 'R':
      N_RB_DL = atoi(optarg);
      if ((N_RB_DL != 6) && (N_RB_DL != 15) && (N_RB_DL != 25) &&
          (N_RB_DL != 50) && (N_RB_DL != 75) && (N_RB_DL != 100)) {
        printf("Illegal N_RB_DL %d (should be one of 6,15,25,50,75,100)\n", N_RB_DL);
        exit(-1);
      }
      break;
    case 'g':
      tx_gain = atof(optarg);
      break;
    case 'G':
      rx_gain = atof(optarg);
      break;
    case 's':
      {
        int clock_src = atoi(optarg);
        if (clock_src == 0) {
          char ref[128] = "internal";
          //strncpy(uhd_ref, ref, strlen(ref)+1);
        }
        else if (clock_src == 1) {
          char ref[128] = "external";
          //strncpy(uhd_ref, ref, strlen(ref)+1);
        }
      }
      break;
    case 'i':
      mme_ip = optarg;
    default:
      break;
    }
  }

  if(N_RB_DL == 100) {
    sample_rate = 30.72e6;
    samples_per_packets = 2048;
    samples_per_frame = 307200;
    // from usrp_time_offset
    tx_forward_nsamps = 175;
    sf_bounds = sf_bounds_20;
    max_cnt = 150;
    tx_delay = 8;
  }
  else if(N_RB_DL == 50){
    sample_rate = 15.36e6;
    samples_per_packets = 2048;
    samples_per_frame = 153600;
    tx_forward_nsamps = 95;
    sf_bounds = sf_bounds_10;
    max_cnt = 75;
    tx_delay = 4;
  }
  else if (N_RB_DL == 25) {
    sample_rate = 7.68e6;
    samples_per_packets = 1024;
    samples_per_frame = 76800;
    tx_forward_nsamps = 70;
    sf_bounds = sf_bounds_5;
    max_cnt = 75;
    tx_delay = 4;
  }
  if(frame_type == FDD)
    rx_freq = tx_freq -120e6;
  else
    rx_freq = tx_freq;
  set_taus_seed (0);

  // initialize the log (see log.h for details)
  logInit();

#ifdef OPENAIR2
#ifdef NAS_NETLINK
  netlink_init();
#endif
#endif

  // to make a graceful exit when ctrl-c is pressed
  signal(SIGSEGV, signal_handler);
  signal(SIGINT, signal_handler);

#ifndef RTAI
  check_clock();
#endif

  init_lte_vars(&frame_parms, frame_type, tdd_config, tdd_config_S, extended_prefix_flag, N_RB_DL,
      Nid_cell, cooperation_flag, transmission_mode, abstraction_flag, nb_antennas_rx);

  g_log->log_component[HW].level = LOG_INFO;
  g_log->log_component[HW].flag  = LOG_HIGH;
  g_log->log_component[PHY].level = LOG_INFO;
  g_log->log_component[PHY].flag  = LOG_HIGH;
  g_log->log_component[MAC].level = LOG_INFO;
  g_log->log_component[MAC].flag  = LOG_HIGH;
  g_log->log_component[RLC].level = LOG_INFO;
  g_log->log_component[RLC].flag  = LOG_HIGH;
  g_log->log_component[PDCP].level = LOG_INFO;
  g_log->log_component[PDCP].flag  = LOG_HIGH;
  g_log->log_component[S1AP].level = LOG_INFO;
  g_log->log_component[S1AP].flag  = LOG_HIGH;
  g_log->log_component[RRC].level = LOG_INFO;
  g_log->log_component[RRC].flag  = LOG_HIGH;
  g_log->log_component[OIP].level = LOG_INFO;
  g_log->log_component[OIP].flag = LOG_HIGH;


  PHY_vars_eNB_g = malloc(sizeof(PHY_VARS_eNB*));
  PHY_vars_eNB_g[0] = init_lte_eNB(frame_parms,eNB_id,Nid_cell,cooperation_flag,transmission_mode,abstraction_flag);


  NB_eNB_INST=1;
  NB_INST=1;

  openair_daq_vars.ue_dl_rb_alloc=0x1fff;
  openair_daq_vars.target_ue_dl_mcs=12;
  openair_daq_vars.ue_ul_nb_rb=6;
  openair_daq_vars.target_ue_ul_mcs=8;

  dump_frame_parms(frame_parms);

  mac_xface = malloc(sizeof(MAC_xface));

  //create a usrp device
  openair0_config_t openair0_cfg;
  openair0_cfg.sample_rate = sample_rate;
  openair0_cfg.tx_freq = tx_freq;
  openair0_cfg.rx_freq = rx_freq;
  openair0_cfg.tx_bw = bw;
  openair0_cfg.rx_bw = bw;
  openair0_cfg.tx_gain = tx_gain;
  openair0_cfg.rx_gain = rx_gain;

  openair0_device_init(&openair0, &openair0_cfg);


#ifdef OPENAIR2
  int eMBMS_active=0;
  l2_init(frame_parms,eMBMS_active,
	  0); // cba_group_active
  mac_xface->mrbch_phy_sync_failure (0, 0, 0);
#endif

  mac_xface->macphy_exit = &exit_fun;

  setup_eNB_buffers(PHY_vars_eNB_g[0],frame_parms,ant_offset);

#ifdef RTAI
  // make main thread LXRT soft realtime
  task = rt_task_init_schmod(nam2num("MYTASK"), 0, 0, 0, SCHED_FIFO, 0xFF);

  // start realtime timer and scheduler
  rt_set_oneshot_mode();
  //rt_set_periodic_mode();
  start_rt_timer(0);

	//mlockall(MCL_CURRENT | MCL_FUTURE);

	//rt_make_hard_real_time();

  tti_sem = rt_typed_sem_init(nam2num("ttisem"), 0, BIN_SEM|FIFO_Q);
  if(tti_sem == 0)
    printf("error init tx semphore\n");

  sync_sem = rt_typed_sem_init(nam2num("syncsem"), 0, BIN_SEM|FIFO_Q);
  if(sync_sem == 0)
    printf("error init sync semphore\n");
#else
  pthread_cond_init(&tti_cond, NULL);
  pthread_mutex_init(&tti_mutex, NULL);
  pthread_cond_init(&sync_cond,NULL);
  pthread_mutex_init(&sync_mutex, NULL);
#endif

#ifdef XFORMS
  if (do_forms==1) {
    fl_initialize (&argc, argv, NULL, 0, 0);
    form_stats = create_form_stats_form();
    for(UE_id=0;UE_id<scope_enb_num_ue;UE_id++) {
        form_enb[UE_id] = create_lte_phy_scope_enb();
        sprintf (title, "UE%d LTE UL SCOPE eNB",UE_id+1);
        fl_show_form (form_enb[UE_id]->lte_phy_scope_enb, FL_PLACE_HOTSPOT, FL_FULLBORDER, title);
    }
    fl_show_form (form_stats->stats_form, FL_PLACE_HOTSPOT, FL_FULLBORDER, "stats");
    for (UE_id=0;UE_id<scope_enb_num_ue;UE_id++) {
      fl_set_button(form_enb[UE_id]->button_0,0);
      fl_set_object_label(form_enb[UE_id]->button_0,"DL Traffic OFF");
    }

    ret = pthread_create(&thread_scope, NULL, scope_thread, NULL);
    printf("Scope thread created, ret=%d\n",ret);
  }
#endif


#ifndef RTAI
  pthread_attr_init (&attr_dlsch_threads);
  pthread_attr_setstacksize(&attr_dlsch_threads,OPENAIR_THREAD_STACK_SIZE);
  //attr_dlsch_threads.priority = 1;
  sched_param_dlsch.sched_priority = sched_get_priority_max(SCHED_FIFO); //OPENAIR_THREAD_PRIORITY;
  pthread_attr_setschedparam  (&attr_dlsch_threads, &sched_param_dlsch);
  pthread_attr_setschedpolicy (&attr_dlsch_threads, SCHED_FIFO);
#endif


  // start the main thread
#ifdef RTAI
  // Transciever thread to streaming IQ data between PC and RF frontend
  thread_trx = rt_thread_create(trx_thread_func, NULL, 100000);
  enb_thread_tx = rt_thread_create(eNB_TX, NULL, 10000000);
  enb_thread_rx = rt_thread_create(eNB_RX, NULL, 10000000);
  rt_sleep_ns(FRAME_PERIOD*10);
#else

  int error_code;
  error_code = pthread_create(&thread_trx, &attr_dlsch_threads, trx_thread_func, NULL);
  if (error_code!= 0) {
    LOG_D(HW,"[lte-enb] Could not allocate trx_thread, error %d\n",error_code);
    return(error_code);
  }
  else {
    LOG_D(HW,"[lte-enb] Allocate trx_thread  successful\n");
  }

  error_code = pthread_create(&enb_thread_tx, &attr_dlsch_threads, eNB_TX, NULL);
  if (error_code!= 0) {
    LOG_D(HW,"[lte-enb] Could not allocate eNB_TX, error %d\n",error_code);
    return(error_code);
  }
  else {
    LOG_D(HW,"[lte-enb] Allocate eNB_TX successful\n");
  }
  error_code = pthread_create(&enb_thread_rx, &attr_dlsch_threads, eNB_RX, NULL);
  if (error_code!= 0) {
    LOG_D(HW,"[lte-enb] Could not allocate eNB_RX, error %d\n",error_code);
    return(error_code);
  }
  else {
    LOG_D(HW,"[lte-enb] Allocate eNB_RX successful\n");
  }
#endif

#ifdef OPENAIR2
    //init_pdcp_thread(1);
#endif

#ifdef ULSCH_THREAD
  //init_ulsch_threads();
#endif

  openair0.trx_start_func(&openair0);

#ifdef RTAI
  rt_sem_signal(sync_sem);
#else
  //pthread_mutex_lock(&sync_mutex);
  pthread_cond_signal(&sync_cond);
  //pthread_mutex_unlock(&sync_mutex);
#endif

  // wait for end of program
  printf("TYPE <CTRL-C> TO TERMINATE\n");
  //getchar();
  while (!oai_exit)
    rt_sleep_ns(FRAME_PERIOD*100);

  openair0.trx_end_func(&openair0);
  rt_sleep_ns(FRAME_PERIOD*10);

  // stop threads
#ifdef XFORMS
  printf("waiting for XFORMS thread\n");
  if (do_forms==1)
  {
    pthread_join(thread_scope,&status);
    fl_hide_form(form_stats->stats_form);
    fl_free_form(form_stats->stats_form);
    for(UE_id=0;UE_id<scope_enb_num_ue;UE_id++) {
        fl_hide_form(form_enb[UE_id]->lte_phy_scope_enb);
        fl_free_form(form_enb[UE_id]->lte_phy_scope_enb);
    }
  }
#endif

  printf("stopping MODEM threads\n");
  // cleanup
#ifdef RTAI
  rt_sem_broadcast(tti_sem);
  rt_thread_join(enb_thread_tx);
  rt_thread_join(enb_thread_rx);
  rt_thread_join(thread_trx);
#else
  pthread_cond_broadcast(&tti_cond);
  pthread_join(thread_trx, &status);
  pthread_join(enb_thread_rx, &status);
  pthread_join(enb_thread_tx,&status);
#endif

#ifdef ULSCH_THREAD
  //cleanup_ulsch_threads();
#endif


#ifdef OPENAIR2
  //cleanup_pdcp_thread();
#endif

#ifdef RTAI
  rt_sem_delete(tti_sem);
  rt_sem_delete(sync_sem);
  stop_rt_timer();
  rt_task_delete(task);
#else
  pthread_cond_destroy(&tti_cond);
  pthread_mutex_destroy(&tti_mutex);
  pthread_cond_destroy(&sync_cond);
  pthread_mutex_destroy(&sync_mutex);
#endif

  logClean();
  free16(txdata, samples_per_frame*sizeof(s32));
  free16(rxdata, samples_per_frame*sizeof(s32));
  return 0;
}

void setup_eNB_buffers(PHY_VARS_eNB *phy_vars_eNB, LTE_DL_FRAME_PARMS *frame_parms, int carrier) {

  int i,j;
  u16 N_TA_offset = 0;

  if (frame_parms->frame_type == TDD) {
    if (phy_vars_eNB->lte_frame_parms.N_RB_DL == 100)
      N_TA_offset = 624;
    else if (phy_vars_eNB->lte_frame_parms.N_RB_DL == 50)
      N_TA_offset = 624/2;
    else if (phy_vars_eNB->lte_frame_parms.N_RB_DL == 25)
      N_TA_offset = 624/4;
  }

  if (phy_vars_eNB) {
    if ((frame_parms->nb_antennas_rx>1) && (carrier>0)) {
      printf("RX antennas > 1 and carrier > 0 not possible\n");
      exit(-1);
    }

    if ((frame_parms->nb_antennas_tx>1) && (carrier>0)) {
      printf("TX antennas > 1 and carrier > 0 not possible\n");
      exit(-1);
    }

    //carrier = 1;
    // replace RX signal buffers with mmaped HW versions
    for (i=0;i<frame_parms->nb_antennas_rx;i++) {
        free(phy_vars_eNB->lte_eNB_common_vars.rxdata[0][i]);
        rxdata = (s32*)malloc16(samples_per_frame*sizeof(s32));
        phy_vars_eNB->lte_eNB_common_vars.rxdata[0][i] = rxdata-N_TA_offset; // N_TA offset for TDD
        memset(rxdata, 0, samples_per_frame*sizeof(s32));
        printf("rxdata[%d] @ %p\n", i, phy_vars_eNB->lte_eNB_common_vars.rxdata[0][i]);
    }
    for (i=0;i<frame_parms->nb_antennas_tx;i++) {
        free(phy_vars_eNB->lte_eNB_common_vars.txdata[0][i]);
        txdata = (s32*)malloc16(samples_per_frame*sizeof(s32));
        phy_vars_eNB->lte_eNB_common_vars.txdata[0][i] = txdata;
        memset(txdata, 0, samples_per_frame*sizeof(s32));
        printf("txdata[%d] @ %p\n", i, phy_vars_eNB->lte_eNB_common_vars.txdata[0][i]);
    }
  }
}
