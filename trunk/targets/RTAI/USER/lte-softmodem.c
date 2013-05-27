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

/*! \file lte-softmodem.c
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

#include "rt_wrapper.h"

#ifdef EMOS
#include <gps.h>
#endif

#include "PHY/types.h"
#include "PHY/defs.h"
#include "openair0_lib.h"

#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#include "SCHED/vars.h"
#include "LAYER2/MAC/vars.h"

#include "../../SIMU/USER/init_lte.h"

#ifdef EMOS
#include "SCHED/phy_procedures_emos.h"
#endif

#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/vars.h"
#include "RRC/LITE/vars.h"
#include "PHY_INTERFACE/vars.h"
#endif

#include "UTIL/LOG/log_extern.h"
#include "UTIL/OTG/otg.h"
#include "UTIL/OTG/otg_vars.h"
#include "UTIL/MATH/oml.h"

#ifdef XFORMS
#include "PHY/TOOLS/lte_phy_scope.h"
#include "stats.h"
// current status is that every UE has a DL scope for a SINGLE eNB (eNB_id=0)
// at eNB 0, an UL scope for every UE 
FD_lte_phy_scope_ue  *form_ue[NUMBER_OF_UE_MAX];
FD_lte_phy_scope_enb *form_enb[NUMBER_OF_UE_MAX];
FD_stats_form *form_stats=NULL;
char title[255];
unsigned char scope_enb_num_ue = 1;
#endif //XFORMS

#define FRAME_PERIOD 100000000ULL
#define DAQ_PERIOD 66667ULL

#undef MALLOC //there are two conflicting definitions, so we better make sure we don't use it at all

#ifdef RTAI
static SEM *mutex;
//static CND *cond;

static int thread0;
static int thread1;
//static int sync_thread;
#else
pthread_t thread0;
pthread_t thread1;
pthread_attr_t attr_dlsch_threads;
struct sched_param p;
#endif

pthread_t  thread2;
pthread_t  thread3;
/*
static int instance_cnt=-1; //0 means worker is busy, -1 means its free
int instance_cnt_ptr_kern,*instance_cnt_ptr_user;
int pci_interface_ptr_kern;
*/
//extern unsigned int bigphys_top;
//extern unsigned int mem_base;

int card = 0;
exmimo_config_t *p_exmimo_config;
exmimo_id_t     *p_exmimo_id;
volatile unsigned int *DAQ_MBOX;

int oai_exit = 0;

//int time_offset[4] = {-138,-138,-138,-138};
int time_offset[4] = {-145,-145,-145,-145};
//int time_offset[4] = {0,0,0,0};

int fs4_test=0;
char UE_flag=0;
u8  eNB_id=0,UE_id=0;

u32 carrier_freq[4]= {1907600000,1907600000,1907600000,1907600000};

struct timing_info_t {
  unsigned int frame, hw_slot, last_slot, next_slot;
  RTIME time0, time1, time2;
  unsigned int mbox0, mbox1, mbox2, mbox_target;
} timing_info[20];

extern s16* sync_corr_ue0;
extern s16 prach_ifft[4][1024*2];


runmode_t mode;
int rx_input_level_dBm;
#ifdef XFORMS
extern int otg_enabled;
#else
int otg_enabled;
#endif
int number_of_cards = 1;

int mbox_bounds[20] = {8,16,24,30,38,46,54,60,68,76,84,90,98,106,114,120,128,136,144, 0}; ///boundaries of slots in terms ob mbox counter rounded up to even numbers

int init_dlsch_threads(void);
void cleanup_dlsch_threads(void);
s32 init_rx_pdsch_thread(void);
void cleanup_rx_pdsch_thread(void);
int init_ulsch_threads(void);
void cleanup_ulsch_threads(void);

LTE_DL_FRAME_PARMS *frame_parms;

void setup_ue_buffers(PHY_VARS_UE *phy_vars_ue, LTE_DL_FRAME_PARMS *frame_parms, int carrier);
void setup_eNB_buffers(PHY_VARS_eNB *phy_vars_eNB, LTE_DL_FRAME_PARMS *frame_parms, int carrier);
void test_config(int card, int ant, unsigned int rf_mode, int UE_flag);

unsigned int build_rflocal(txi, txq, rxi, rxq)
{
    return (txi + (txq<<6) + (rxi<<12) + (rxq<<18));
}
unsigned int build_rfdc(int dcoff_i_rxfe, int dcoff_q_rxfe)
{
    return (dcoff_i_rxfe + (dcoff_q_rxfe<<8));
}

void signal_handler(int sig)
{
  void *array[10];
  size_t size;

  oai_exit=1;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(-1);
}

void exit_fun(const char* s)
{
  void *array[10];
  size_t size;

  printf("Exiting: %s\n",s);

  oai_exit=1;
  rt_sleep_ns(FRAME_PERIOD);

  // cleanup
#ifdef RTAI
  stop_rt_timer();
#endif

  openair0_stop(card);
  openair0_close();

  exit (-1);
}

#ifdef XFORMS
void *scope_thread(void *arg) {
    s16 prach_corr[1024], i;
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
        if (UE_flag==1) {
            len = dump_ue_stats (PHY_vars_UE_g[0], stats_buffer, 0, mode,rx_input_level_dBm);
            fl_set_object_label(form_stats->stats_text, stats_buffer);
            //rewind (UE_stats);
            //fwrite (stats_buffer, 1, len, UE_stats);
            
            phy_scope_UE(form_ue[UE_id], 
                         PHY_vars_UE_g[UE_id],
                         eNB_id,
                         UE_id,7);
            
        } else {
            len = dump_eNB_stats (PHY_vars_eNB_g[0], stats_buffer, 0);
            fl_set_object_label(form_stats->stats_text, stats_buffer);
            //rewind (eNB_stats);
            //fwrite (stats_buffer, 1, len, eNB_stats);
            for(UE_id=0;UE_id<scope_enb_num_ue;UE_id++) {
                phy_scope_eNB(form_enb[UE_id], 
                              PHY_vars_eNB_g[eNB_id],
                              UE_id);
            }
              
        }
        //printf("doing forms\n");
        sleep(0.1);
    }
    
    //fclose (UE_stats);
    //fclose (eNB_stats);
    
    pthread_exit((void*)arg);
}
#endif

int dummy_tx_buffer[3840*4] __attribute__((aligned(16)));

#ifdef EMOS
#define NO_ESTIMATES_DISK 100 //No. of estimates that are aquired before dumped to disk

void *emos_thread (void *arg)
{
  char c;
  char *fifo2file_buffer, *fifo2file_ptr;

  int fifo, counter=0, bytes;

  FILE  *dumpfile_id;
  char  dumpfile_name[1024];
  time_t starttime_tmp;
  struct tm starttime;
  
  int channel_buffer_size;
  
  time_t timer;
  struct tm *now;

  struct gps_data_t *gps_data = NULL;
  struct gps_fix_t dummy_gps_data;
 
  timer = time(NULL);
  now = localtime(&timer);

  memset(&dummy_gps_data,1,sizeof(struct gps_fix_t));
  
  gps_data = gps_open("127.0.0.1","2947");
  if (gps_data == NULL) 
    {
      printf("[EMOS] Could not open GPS\n");
      //exit(-1);
    }
#if GPSD_API_MAJOR_VERSION>=4
  else if (gps_stream(gps_data, WATCH_ENABLE,NULL) != 0)
#else
  else if (gps_query(gps_data, "w+x") != 0)
#endif
    {
      //sprintf(tmptxt,"Error sending command to GPS, gps_data = %x", gps_data);
      printf("[EMOS] Error sending command to GPS\n");
      //exit(-1);
    }
  
  if (UE_flag==0)
    channel_buffer_size = sizeof(fifo_dump_emos_eNB);
  else
    channel_buffer_size = sizeof(fifo_dump_emos_UE);

  // allocate memory for NO_FRAMES_DISK channes estimations
  fifo2file_buffer = malloc(NO_ESTIMATES_DISK*channel_buffer_size);
  fifo2file_ptr = fifo2file_buffer;

  if (fifo2file_buffer == NULL)
    {
      printf("[EMOS] Cound not allocate memory for fifo2file_buffer\n");
      exit(EXIT_FAILURE);
    }

  if ((fifo = open(CHANSOUNDER_FIFO_DEV, O_RDONLY)) < 0)
    {
      fprintf(stderr, "[EMOS] Error opening the fifo\n");
      exit(EXIT_FAILURE);
    }


  time(&starttime_tmp);
  localtime_r(&starttime_tmp,&starttime);
  snprintf(dumpfile_name,1024,"/tmp/%s_data_%d%02d%02d_%02d%02d%02d.EMOS",
	   (UE_flag==0) ? "eNB" : "UE",
	   1900+starttime.tm_year, starttime.tm_mon+1, starttime.tm_mday, starttime.tm_hour, starttime.tm_min, starttime.tm_sec);

  dumpfile_id = fopen(dumpfile_name,"w");
  if (dumpfile_id == NULL)
    {
      fprintf(stderr, "[EMOS] Error opening dumpfile %s\n",dumpfile_name);
      exit(EXIT_FAILURE);
    }


  printf("[EMOS] starting dump, channel_buffer_size=%d ...\n",channel_buffer_size);
  while (!oai_exit)
    {
      bytes = rtf_read_all_at_once(fifo, fifo2file_ptr, channel_buffer_size);
      /*
      if (eNB_flag==1)
	printf("eNB: count %d, frame %d, read: %d bytes from the fifo\n",counter, ((fifo_dump_emos_eNB*)fifo2file_ptr)->frame_tx,bytes);
      else
	printf("UE: count %d, frame %d, read: %d bytes from the fifo\n",counter, ((fifo_dump_emos_UE*)fifo2file_ptr)->frame_rx,bytes);
      */

      fifo2file_ptr += channel_buffer_size;
      counter ++;

      if (counter == NO_ESTIMATES_DISK)
        {
          //reset stuff
          fifo2file_ptr = fifo2file_buffer;
          counter = 0;

          //flush buffer to disk
	  if (UE_flag==0)
	    printf("[EMOS] eNB: count %d, frame %d, flushing buffer to disk\n",
		   counter, ((fifo_dump_emos_eNB*)fifo2file_ptr)->frame_tx);
	  else
	    printf("[EMOS] UE: count %d, frame %d, flushing buffer to disk\n",
		   counter, ((fifo_dump_emos_UE*)fifo2file_ptr)->frame_rx);


          if (fwrite(fifo2file_buffer, sizeof(char), NO_ESTIMATES_DISK*channel_buffer_size, dumpfile_id) != NO_ESTIMATES_DISK*channel_buffer_size)
            {
              fprintf(stderr, "[EMOS] Error writing to dumpfile\n");
              exit(EXIT_FAILURE);
            }
	  if (gps_data)
	    {
	      if (gps_poll(gps_data) != 0) {
		printf("[EMOS] problem polling data from gps\n");
	      }
	      else {
		printf("[EMOS] lat %g, lon %g\n",gps_data->fix.latitude,gps_data->fix.longitude);
	      }
	      if (fwrite(&(gps_data->fix), sizeof(char), sizeof(struct gps_fix_t), dumpfile_id) != sizeof(struct gps_fix_t))
		{
		  printf("[EMOS] Error writing to dumpfile, stopping recording\n");
		  exit(EXIT_FAILURE);
		}
	    }
	  else
	    {
	      printf("[EMOS] WARNING: No GPS data available, storing dummy packet\n");
	      if (fwrite(&(dummy_gps_data), sizeof(char), sizeof(struct gps_fix_t), dumpfile_id) != sizeof(struct gps_fix_t))
		{
		  printf("[EMOS] Error writing to dumpfile, stopping recording\n");
		  exit(EXIT_FAILURE);
		}
	    } 
        }
    }
  
  free(fifo2file_buffer);
  fclose(dumpfile_id);
  close(fifo);
  
  pthread_exit((void*) arg);

}
#endif

/* This is the main eNB thread. It gets woken up by the kernel driver using the RTAI message mechanism (rt_send and rt_receive). */
static void *eNB_thread(void *arg)
{
#ifdef RTAI
  RT_TASK *task;
#endif
  unsigned char slot=0,last_slot, next_slot;
  int hw_slot,frame=0;
  unsigned int msg1;
  unsigned int aa,slot_offset, slot_offset_F;
  int diff;
  int delay_cnt;
  RTIME time_in;
  int mbox_target=0,mbox_current=0;
  int i,ret;
  int tx_offset;

#ifdef RTAI
  task = rt_task_init_schmod(nam2num("TASK0"), 0, 0, 0, SCHED_FIFO, 0xF);
  LOG_D(HW,"Started eNB thread (id %p)\n",task);
#endif

#ifdef HARD_RT
  rt_make_hard_real_time();
#endif

  mlockall(MCL_CURRENT | MCL_FUTURE);

  while (!oai_exit)
    {
      hw_slot = (((((volatile unsigned int *)DAQ_MBOX)[0]+1)%150)<<1)/15;
      //LOG_D(HW,"eNB frame %d, time %llu: slot %d, hw_slot %d (mbox %d)\n",frame,rt_get_time_ns(),slot,hw_slot,((unsigned int *)DAQ_MBOX)[0]);
      //this is the mbox counter where we should be 
      //mbox_target = ((((slot+1)%20)*15+1)>>1)%150;
      mbox_target = mbox_bounds[slot];
      //this is the mbox counter where we are
      mbox_current = ((volatile unsigned int *)DAQ_MBOX)[0];
      //this is the time we need to sleep in order to synchronize with the hw (in multiples of DAQ_PERIOD)
      if ((mbox_current>=135) && (mbox_target<15)) //handle the frame wrap-arround
          diff = 150-mbox_current+mbox_target;
      else if ((mbox_current<15) && (mbox_target>=135))
          diff = -150+mbox_target-mbox_current;
      else
          diff = mbox_target - mbox_current;
      
      if (diff < (-5)) {
          LOG_D(HW,"eNB Frame %d, time %llu: missed slot, proceeding with next one (slot %d, hw_slot %d, diff %d)\n",frame, rt_get_time_ns(), slot, hw_slot, diff);
          slot++;
          if (slot==20){
              slot=0;
              frame++;
          }
          continue;
      }
      if (diff>8) 
          LOG_D(HW,"eNB Frame %d, time %llu: skipped slot, waiting for hw to catch up (slot %d, hw_slot %d, mbox_current %d, mbox_target %d, diff %d)\n",frame, rt_get_time_ns(), slot, hw_slot, mbox_current, mbox_target, diff);

      delay_cnt = 0;
      while ((diff>0) && (!oai_exit))
        {
	  time_in = rt_get_time_ns();
	  //LOG_D(HW,"eNB Frame %d delaycnt %d : hw_slot %d (%d), slot %d, (slot+1)*15=%d, diff %d, time %llu\n",frame,delay_cnt,hw_slot,((unsigned int *)DAQ_MBOX)[0],slot,(((slot+1)*15)>>1),diff,time_in);
	  //LOG_D(HW,"eNB Frame %d, time %llu: sleeping for %llu (slot %d, hw_slot %d, diff %d, mbox %d, delay_cnt %d)\n", frame, time_in, diff*DAQ_PERIOD,slot,hw_slot,diff,((volatile unsigned int *)DAQ_MBOX)[0],delay_cnt);
          ret = rt_sleep_ns(diff*DAQ_PERIOD);
	  if (ret)
	    LOG_D(HW,"eNB Frame %d, time %llu: rt_sleep_ns returned %d\n",frame, time_in);
	  hw_slot = (((((volatile unsigned int *)DAQ_MBOX)[0]+1)%150)<<1)/15;
          //LOG_D(HW,"eNB Frame %d : hw_slot %d, time %llu\n",frame,hw_slot,rt_get_time_ns());
          delay_cnt++;
          if (delay_cnt == 10)
            {
              oai_exit = 1;
              LOG_D(HW,"eNB Frame %d: HW stopped ... \n",frame);
            }
	  mbox_current = ((volatile unsigned int *)DAQ_MBOX)[0];
	  if ((mbox_current>=135) && (mbox_target<15)) //handle the frame wrap-arround
	    diff = 150-mbox_current+mbox_target;
	  else if ((mbox_current<15) && (mbox_target>=135))
	    diff = -150+mbox_target-mbox_current;
	  else
	    diff = mbox_target - mbox_current;
        }

      last_slot = (slot)%LTE_SLOTS_PER_FRAME;
      if (last_slot <0)
        last_slot+=20;
      next_slot = (slot+3)%LTE_SLOTS_PER_FRAME;

      //PHY_vars_eNB_g[0]->frame = frame;
      if (frame>5)
        {
	  /*
          if (frame%100==0)
            LOG_D(HW,"frame %d (%d), slot %d, hw_slot %d, next_slot %d (before): DAQ_MBOX %d\n",frame, PHY_vars_eNB_g[0]->frame, slot, hw_slot,next_slot,DAQ_MBOX[0]);
	  */
          if (fs4_test==0)
            {
              phy_procedures_eNB_lte (last_slot, next_slot, PHY_vars_eNB_g[0], 0);
#ifndef IFFT_FPGA
              slot_offset_F = (next_slot)*
                              (PHY_vars_eNB_g[0]->lte_frame_parms.ofdm_symbol_size)*
                              ((PHY_vars_eNB_g[0]->lte_frame_parms.Ncp==1) ? 6 : 7);
              slot_offset = (next_slot)*
                            (PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti>>1);
              if ((subframe_select(&PHY_vars_eNB_g[0]->lte_frame_parms,next_slot>>1)==SF_DL)||
                  ((subframe_select(&PHY_vars_eNB_g[0]->lte_frame_parms,next_slot>>1)==SF_S)&&((next_slot&1)==0)))
                {
                  //	  LOG_D(HW,"Frame %d: Generating slot %d\n",frame,next_slot);

                  for (aa=0; aa<PHY_vars_eNB_g[0]->lte_frame_parms.nb_antennas_tx; aa++)
                    {
                      if (PHY_vars_eNB_g[0]->lte_frame_parms.Ncp == 1)
                        {
                          PHY_ofdm_mod(&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][aa][slot_offset_F],
#ifdef BIT8_TX
                                       &PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][slot_offset>>1],
#else
                                       dummy_tx_buffer,//&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][slot_offset],
#endif
                                       PHY_vars_eNB_g[0]->lte_frame_parms.log2_symbol_size,
                                       6,
                                       PHY_vars_eNB_g[0]->lte_frame_parms.nb_prefix_samples,
                                       PHY_vars_eNB_g[0]->lte_frame_parms.twiddle_ifft,
                                       PHY_vars_eNB_g[0]->lte_frame_parms.rev,
                                       CYCLIC_PREFIX);
                        }
                      else
                        {
                          normal_prefix_mod(&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][aa][slot_offset_F],
#ifdef BIT8_TX
                                            &PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][slot_offset>>1],
#else
                                            dummy_tx_buffer,//&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][slot_offset],
#endif
                                            7,
                                            &(PHY_vars_eNB_g[0]->lte_frame_parms));
                        }
#ifdef EXMIMO
		      for (i=0; i<PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti/2; i++) 
			{
			  tx_offset = (int)slot_offset+time_offset[aa]+i;
			  if (tx_offset<0)
			    tx_offset += LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti;
			  if (tx_offset>=(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti))
			    tx_offset -= LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti;
			  ((short*)&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][tx_offset])[0]=
			    ((short*)dummy_tx_buffer)[2*i]<<4;
			  ((short*)&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][tx_offset])[1]=
			    ((short*)dummy_tx_buffer)[2*i+1]<<4;
			}
#endif //EXMIMO
                    }
                }
            }

#endif //IFFT_FPGA
	  /*
          if (frame%100==0)
            LOG_D(HW,"hw_slot %d (after): DAQ_MBOX %d\n",hw_slot,DAQ_MBOX[0]);
	  */
        }

      /*
      if ((slot%2000)<10)
      LOG_D(HW,"fun0: doing very hard work\n");
      */

      slot++;
      if (slot==20) {
        slot=0;
        frame++;
      }
    }

  LOG_D(HW,"eNB_thread: finished, ran %d times.\n",frame);

#ifdef HARD_RT
  rt_make_soft_real_time();
#endif

  // clean task
#ifdef RTAI
  rt_task_delete(task);
#endif
  LOG_D(HW,"Task deleted. returning\n");
  return 0;
}

/* This is the main UE thread. Initially it is doing a periodic get_frame. One synchronized it gets woken up by the kernel driver using the RTAI message mechanism (rt_send and rt_receive). */
static void *UE_thread(void *arg)
{
#ifdef RTAI
  RT_TASK *task;
#endif
  RTIME in, out, diff;
  int slot=0,frame=0,hw_slot,last_slot, next_slot;
  unsigned int msg1;
  unsigned int aa,slot_offset, slot_offset_F;
  static int is_synchronized = 0;
  static int received_slots = 0;
  static int slot0 = 0;
  int delay_cnt;
  RTIME time_in;
  int hw_slot_offset=0,rx_offset_mbox=0,mbox_target=0,mbox_current=0;
  int diff2;
  static int first_run=1;
  int i, ret;

#ifdef RTAI
  task = rt_task_init_schmod(nam2num("TASK0"), 0, 0, 0, SCHED_FIFO, 0xF);
  LOG_D(HW,"Started UE thread (id %p)\n",task);
#endif

#ifdef HARD_RT
  rt_make_hard_real_time();
#endif

  mlockall(MCL_CURRENT | MCL_FUTURE);

  openair_daq_vars.freq_offset = 0; //-7500;
  /*
  if (mode == rx_calib_ue) {
    openair_daq_vars.freq_offset = -7500;
    for (i=0; i<4; i++) {
        p_exmimo_config->rf.rf_freq_rx[i] = p_exmimo_config->rf.rf_freq_rx[i]+openair_daq_vars.freq_offset;
        p_exmimo_config->rf.rf_freq_tx[i] = p_exmimo_config->rf.rf_freq_rx[i]+openair_daq_vars.freq_offset;
    }
    openair0_dump_config(card);
  }
  */
  while (!oai_exit)
    {
      hw_slot = (((((volatile unsigned int *)DAQ_MBOX)[0]+1)%150)<<1)/15; //the slot the hw is about to store
      
      if (is_synchronized) {
      //this is the mbox counter that indicates the start of the frame
      rx_offset_mbox = (PHY_vars_UE_g[0]->rx_offset * 150) / (10*PHY_vars_UE_g[0]->lte_frame_parms.samples_per_tti); 
      //this is the mbox counter where we should be 
      mbox_target = (((((slot+1)%20)*15+1)>>1) + rx_offset_mbox + 1)%150;
      // round up to the next multiple of two (mbox counter from express MIMO gives only even numbers)
      mbox_target = ((mbox_target+1)-((mbox_target-1)%2))%150;
      //this is the mbox counter where we are
      mbox_current = ((volatile unsigned int *)DAQ_MBOX)[0];
      //this is the time we need to sleep in order to synchronize with the hw (in multiples of DAQ_PERIOD)
      if ((mbox_current>=120) && (mbox_target<30)) //handle the frame wrap-arround
	diff2 = 150-mbox_current+mbox_target;
      else if ((mbox_current<30) && (mbox_target>=120))
	diff2 = -150+mbox_target-mbox_current;
      else
        diff2 = mbox_target - mbox_current;

      if (diff2 <(-5)) {
	LOG_D(HW,"UE Frame %d: missed slot, proceeding with next one (slot %d, hw_slot %d, diff %d)\n",frame, slot, hw_slot, diff2);
	slot++;
	if (slot==20) {
          slot=0;
	  frame++;
	}
	continue;
      }
      if (diff2>8) 
	LOG_D(HW,"UE Frame %d: skipped slot, waiting for hw to catch up (slot %d, hw_slot %d, mbox_current %d, mbox_target %d, diff %d)\n",frame, slot, hw_slot, mbox_current, mbox_target, diff2);

      /*
      if (frame%100==0)
	LOG_D(HW,"frame %d (%d), slot %d, hw_slot %d, rx_offset_mbox %d, mbox_target %d, mbox_current %d, diff %d\n",frame, PHY_vars_UE_g[0]->frame, slot,hw_slot,rx_offset_mbox,mbox_target,mbox_current,diff2);
      */
      timing_info[slot].time0 = rt_get_time_ns();
      timing_info[slot].mbox0 = ((volatile unsigned int *)DAQ_MBOX)[0];

      delay_cnt = 0;
      while ((diff2>0) && (!oai_exit) && (is_synchronized) )
        {
	  time_in = rt_get_time_ns();
	  //LOG_D(HW,"eNB Frame %d delaycnt %d : hw_slot %d (%d), slot %d (%d), diff %d, time %llu\n",frame,delay_cnt,hw_slot,((volatile unsigned int *)DAQ_MBOX)[0],slot,mbox_target,diff2,time_in);
          ret = rt_sleep_ns(diff2*DAQ_PERIOD); 
	  if (ret)
	    LOG_D(HW,"eNB Frame %d, time %llu: rt_sleep_ns returned %d\n",frame, time_in);

          hw_slot = (((((volatile unsigned int *)DAQ_MBOX)[0]+1)%150)<<1)/15;
          //LOG_D(HW,"eNB Frame %d : hw_slot %d, time %llu\n",frame,hw_slot,rt_get_time_ns());
          delay_cnt++;
          if (delay_cnt == 30)
            {
              oai_exit = 1;
              LOG_D(HW,"UE frame %d: HW stopped ... \n",frame);
            }
	  mbox_current = ((volatile unsigned int *)DAQ_MBOX)[0];
	  if ((mbox_current>=135) && (mbox_target<15)) //handle the frame wrap-arround
	    diff2 = 150-mbox_current+mbox_target;
	  else if ((mbox_current<15) && (mbox_target>=135))
	    diff2 = -150+mbox_target-mbox_current;
	  else
	    diff2 = mbox_target - mbox_current;
        }

      timing_info[slot].time1 = rt_get_time_ns();
      timing_info[slot].mbox1 = ((volatile unsigned int *)DAQ_MBOX)[0];
      timing_info[slot].mbox_target = mbox_target;
      }

      last_slot = (slot)%LTE_SLOTS_PER_FRAME;
      if (last_slot <0)
        last_slot+=LTE_SLOTS_PER_FRAME;
      next_slot = (slot+3)%LTE_SLOTS_PER_FRAME;

      timing_info[slot].frame = PHY_vars_UE_g[0]->frame;
      timing_info[slot].hw_slot = hw_slot;
      timing_info[slot].last_slot = last_slot;
      timing_info[slot].next_slot = next_slot;

      if (is_synchronized)
        {

	  /*
          if (frame%100==0)
            LOG_D(HW,"frame %d (%d), slot %d, hw_slot %d, last_slot %d (before): DAQ_MBOX %d\n",frame, PHY_vars_UE_g[0]->frame, slot,hw_slot,last_slot,DAQ_MBOX[0]);
	  */

          in = rt_get_time_ns();
          phy_procedures_UE_lte (last_slot, next_slot, PHY_vars_UE_g[0], 0, 0,mode);
          out = rt_get_time_ns();
          diff = out-in;

	  /*
          if (frame % 100 == 0)
            LOG_D(HW,"hw_slot %d (after): DAQ_MBOX %d\n",hw_slot,DAQ_MBOX[0]);
	  
            LOG_D(HW,"Frame %d: last_slot %d, phy_procedures_lte_ue time_in %llu, time_out %llu, diff %llu\n",
                      frame, last_slot,in,out,diff);
	  */

	  timing_info[slot].time2 = rt_get_time_ns();
	  timing_info[slot].mbox2 = ((volatile unsigned int *)DAQ_MBOX)[0];

        }
      else   // we are not yet synchronized
        {
          hw_slot_offset = 0;

          slot = 0;
          openair0_get_frame(card);
          //          LOG_D(HW,"after get_frame\n");
          //          rt_sleep_ns(FRAME_PERIOD);
          //          LOG_D(HW,"after sleep\n");

          if (initial_sync(PHY_vars_UE_g[0],mode)==0) {
              /*
              lte_adjust_synch(&PHY_vars_UE_g[0]->lte_frame_parms,
                   PHY_vars_UE_g[0],
                   0,
                   1,
                   16384);
              */
              //for better visualization afterwards
              /*
              for (aa=0; aa<PHY_vars_UE_g[0]->lte_frame_parms.nb_antennas_rx; aa++)
              memset(PHY_vars_UE_g[0]->lte_ue_common_vars.rxdata[aa],0,
                 PHY_vars_UE_g[0]->lte_frame_parms.samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*sizeof(int));
              */
	      if (mode == rx_calib_ue) {
		oai_exit=1;
	      }
	      else {
		is_synchronized = 1;
		//start the DMA transfers
		//LOG_D(HW,"Before openair0_start_rt_acquisition \n");
		openair0_start_rt_acquisition(card);
		
		hw_slot_offset = (PHY_vars_UE_g[0]->rx_offset<<1) / PHY_vars_UE_g[0]->lte_frame_parms.samples_per_tti;
		LOG_D(HW,"Got synch: hw_slot_offset %d\n",hw_slot_offset);
	      }
	  }
	  else {
	    if (openair_daq_vars.freq_offset >= 0) {
	      openair_daq_vars.freq_offset += 100;
	      openair_daq_vars.freq_offset *= -1;
	    }
	    else {
	      openair_daq_vars.freq_offset *= -1;
	    }	      
	    if (abs(openair_daq_vars.freq_offset) > 7500) {
	      LOG_I(PHY,"[initial_sync] No cell synchronization found, abondoning\n");
	      mac_xface->macphy_exit("");
	    }
	    else {
	      LOG_I(PHY,"[initial_sync] trying carrier off %d Hz\n",openair_daq_vars.freq_offset);
	      for (i=1; i<4; i++) {
		p_exmimo_config->rf.rf_freq_rx[i] = carrier_freq[i]+openair_daq_vars.freq_offset;
		p_exmimo_config->rf.rf_freq_tx[i] = carrier_freq[i]+openair_daq_vars.freq_offset;
	      }
	      openair0_dump_config(card);
	      
	    }
	  }
        }

      /*
      if ((slot%2000)<10)
        LOG_D(HW,"fun0: doing very hard work\n");
      */
      slot++;
      if (slot==20) {
          slot=0;
          frame++;
      }
    }
  LOG_D(HW,"UE_thread: finished, ran %d times.\n",frame);

#ifdef HARD_RT
  rt_make_soft_real_time();
#endif

  // clean task
#ifdef RTAI
  rt_task_delete(task);
#endif
  LOG_D(HW,"Task deleted. returning\n");
  return 0;
}





int main(int argc, char **argv) {

#ifdef RTAI
  RT_TASK *task;
#endif
  int i,j,aa;
  void *status;

  /*
  u32 rf_mode_max[4]     = {55759,55759,55759,55759};
  u32 rf_mode_med[4]     = {39375,39375,39375,39375};
  u32 rf_mode_byp[4]     = {22991,22991,22991,22991};
  */
  u32 my_rf_mode = RXEN + TXEN + TXLPFNORM + TXLPFEN + TXLPF25 + RXLPFNORM + RXLPFEN + RXLPF25 + LNA1ON +LNAMax + RFBBNORM + DMAMODE_RX + DMAMODE_TX;
  u32 my_rf_mode2 = RXEN + TXLPFNORM + TXLPFEN + TXLPF25 + RXLPFNORM + RXLPFEN + RXLPF25 + LNA1ON +LNAMax + RFBBNORM + DMAMODE_RX;
  u32 rf_mode[4]     = {my_rf_mode,my_rf_mode2,0,0};
  u32 rf_local[4]    = {8255000,8255000,8255000,8255000}; // UE zepto
    //{8254617, 8254617, 8254617, 8254617}; //eNB khalifa
    //{8255067,8254810,8257340,8257340}; // eNB PETRONAS

  u32 rf_vcocal[4]   = {910,910,910,910};
  u32 rf_vcocal_850[4] = {2015, 2015, 2015, 2015};
  u32 rf_rxdc[4]     = {32896,32896,32896,32896};
  u32 rxgain[4]      = {20,20,20,20};
  u32 txgain[4]      = {25,25,25,25};

  u16 Nid_cell = 0;
  u8  cooperation_flag=0, transmission_mode=1, abstraction_flag=0;
  u8 beta_ACK=0,beta_RI=0,beta_CQI=2;

  int c;
  char do_forms=0;
  unsigned int fd;
  unsigned int tcxo = 114;

  int amp;
  u8 prach_fmt;
  int N_ZC;

  char rxg_fname[100];
  char txg_fname[100];
  char rflo_fname[100];
  char rfdc_fname[100];
  FILE *rxg_fd=NULL;
  FILE *txg_fd=NULL;
  FILE *rflo_fd=NULL;
  FILE *rfdc_fd=NULL;
  unsigned int rxg_max[4]={133,133,133,133}, rxg_med[4]={127,127,127,127}, rxg_byp[4]={120,120,120,120};
  int tx_max_power=0;

  char line[1000];
  int l;
  int ret, ant;

  int error_code;

  const struct option long_options[] = {
    {"calib-ue-rx", required_argument, NULL, 256},
    {"calib-ue-rx-med", required_argument, NULL, 257},
    {"calib-ue-rx-byp", required_argument, NULL, 258},
    {"debug-ue-prach", no_argument, NULL, 259},
    {"no-L2-connect", no_argument, NULL, 260},
    {NULL, 0, NULL, 0}};

  mode = normal_txrx;


  while ((c = getopt_long (argc, argv, "C:ST:UdF:",long_options,NULL)) != -1)
    {
      switch (c)
        {
        case 'd':
          do_forms=1;
          break;
        case 'U':
          UE_flag = 1;
          break;
        case 'C':
          carrier_freq[0] = atoi(optarg);
          carrier_freq[1] = atoi(optarg);
          carrier_freq[2] = atoi(optarg);
          carrier_freq[3] = atoi(optarg);
          break;
        case 'S':
          fs4_test=1;
          break;
        case 'T':
          tcxo=atoi(optarg);
          break;
	case 'F':
	  sprintf(rxg_fname,"%srxg.lime",optarg);
	  rxg_fd = fopen(rxg_fname,"r");
	  if (rxg_fd) {
	    printf("Loading RX Gain parameters from %s\n",rxg_fname);
	    l=0;
	    while (fgets(line, sizeof(line), rxg_fd)) {
	      if ((strlen(line)==0) || (*line == '#')) continue; //ignore empty or comment lines
	      else {
		if (l==0) sscanf(line,"%d %d %d %d",&rxg_max[0],&rxg_max[1],&rxg_max[2],&rxg_max[3]);
		if (l==1) sscanf(line,"%d %d %d %d",&rxg_med[0],&rxg_med[1],&rxg_med[2],&rxg_med[3]);
		if (l==2) sscanf(line,"%d %d %d %d",&rxg_byp[0],&rxg_byp[1],&rxg_byp[2],&rxg_byp[3]);
		l++;
	      }
	    }
	  }
	  else 
	    printf("%s not found, running with defaults\n",rxg_fname);

	  sprintf(txg_fname,"%stxg.lime",optarg);
	  txg_fd = fopen(txg_fname,"r");
	  if (txg_fd) {
	    printf("Loading TX Gain parameters from %s\n",txg_fname);
	    l=0;
	    while (fgets(line, sizeof(line), txg_fd)) {
	      if ((strlen(line)==0) || (*line == '#')) {
		continue; //ignore empty or comment lines
	      }
	      else {
		if (l==0) sscanf(line,"%d %d %d %d",&txgain[0],&txgain[1],&txgain[2],&txgain[3]);
		if (l==1) sscanf(line,"%d",&tx_max_power);
		l++;
	      }
	    }
	  }
	  else 
	    printf("%s not found, running with defaults\n",txg_fname);

	  sprintf(rflo_fname,"%srflo.lime",optarg);
	  rflo_fd = fopen(rflo_fname,"r");
	  if (rflo_fd) {
	    printf("Loading RF LO parameters from %s\n",rflo_fname);
	    fscanf(rflo_fd,"%d %d %d %d",&rf_local[0],&rf_local[1],&rf_local[2],&rf_local[3]);
	  }
	  else 
	    printf("%s not found, running with defaults\n",rflo_fname);

	  sprintf(rfdc_fname,"%srfdc.lime",optarg);
	  rfdc_fd = fopen(rfdc_fname,"r");
	  if (rfdc_fd) {
	    printf("Loading RF DC parameters from %s\n",rfdc_fname);
	    fscanf(rfdc_fd,"%d %d %d %d",&rf_rxdc[0],&rf_rxdc[1],&rf_rxdc[2],&rf_rxdc[3]);
	  }
	  else 
	    printf("%s not found, running with defaults\n",rfdc_fname);

	  break;
	case 256:
	  mode = rx_calib_ue;
	  rx_input_level_dBm = atoi(optarg);
	  printf("Running with UE calibration on (LNA max), input level %d dBm\n",rx_input_level_dBm);
	  break;
	case 257:
	  mode = rx_calib_ue_med;
	  rx_input_level_dBm = atoi(optarg);
	  printf("Running with UE calibration on (LNA med), input level %d dBm\n",rx_input_level_dBm);
	  break;
	case 258:
	  mode = rx_calib_ue_byp;
	  rx_input_level_dBm = atoi(optarg);
	  printf("Running with UE calibration on (LNA byp), input level %d dBm\n",rx_input_level_dBm);
	  break;
	case 259:
	  mode = debug_prach;
	  break;
	case 260:
	  mode = no_L2_connect;
	  break;
        default:
          break;
        }
    }

  if (UE_flag==1)
    printf("configuring for UE\n");
  else
    printf("configuring for eNB\n");

  //randominit (0);
  set_taus_seed (0);

  // initialize the log (see log.h for details)
  logInit();

#ifdef NAS_NETLINK
  netlink_init();
#endif

  // to make a graceful exit when ctrl-c is pressed
  signal(SIGSEGV, signal_handler);

#ifndef RTAI
  check_clock();
#endif

  // init the parameters
  frame_parms = (LTE_DL_FRAME_PARMS*) malloc(sizeof(LTE_DL_FRAME_PARMS));
  frame_parms->N_RB_DL            = 25;
  frame_parms->N_RB_UL            = 25;
  frame_parms->Ncp                = 0;
  frame_parms->Ncp_UL             = 0;
  frame_parms->Nid_cell           = Nid_cell;
  frame_parms->nushift            = 0;
  frame_parms->nb_antennas_tx_eNB = 1; //initial value overwritten by initial sync later
  frame_parms->nb_antennas_tx     = (UE_flag==0) ? 1 : 1;
  frame_parms->nb_antennas_rx     = (UE_flag==0) ? 1 : 1;
  frame_parms->mode1_flag         = (transmission_mode == 1) ? 1 : 0;
  frame_parms->frame_type         = 1;
  frame_parms->tdd_config         = 3;
  frame_parms->tdd_config_S       = 0;
  frame_parms->phich_config_common.phich_resource = oneSixth;
  frame_parms->phich_config_common.phich_duration = normal;
  // UL RS Config
  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift = 0;//n_DMRS1 set to 0
  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled = 0;
  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled = 0;
  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH = 0;
  init_ul_hopping(frame_parms);

  init_frame_parms(frame_parms,1);

  phy_init_top(frame_parms);
  phy_init_lte_top(frame_parms);

  //init prach for openair1 test
  frame_parms->prach_config_common.rootSequenceIndex=22; 
  frame_parms->prach_config_common.prach_ConfigInfo.zeroCorrelationZoneConfig=1;
  frame_parms->prach_config_common.prach_ConfigInfo.prach_ConfigIndex=0; 
  frame_parms->prach_config_common.prach_ConfigInfo.highSpeedFlag=0;
  frame_parms->prach_config_common.prach_ConfigInfo.prach_FreqOffset=0;
  prach_fmt = get_prach_fmt(frame_parms->prach_config_common.prach_ConfigInfo.prach_ConfigIndex,
			    frame_parms->frame_type);
  N_ZC = (prach_fmt <4)?839:139;

  if (UE_flag==1) {
#ifdef OPENAIR2
    g_log->log_component[PHY].level = LOG_INFO;
#else
    g_log->log_component[PHY].level = LOG_INFO;
#endif
    g_log->log_component[PHY].flag  = LOG_HIGH;
    g_log->log_component[MAC].level = LOG_INFO;
    g_log->log_component[MAC].flag  = LOG_HIGH;
    g_log->log_component[RLC].level = LOG_INFO;
    g_log->log_component[RLC].flag  = LOG_HIGH;
    g_log->log_component[PDCP].level = LOG_INFO;
    g_log->log_component[PDCP].flag  = LOG_HIGH;
    g_log->log_component[OTG].level = LOG_INFO;
    g_log->log_component[OTG].flag  = LOG_HIGH;
    g_log->log_component[RRC].level = LOG_DEBUG;
    g_log->log_component[RRC].flag  = LOG_HIGH;

    PHY_vars_UE_g = malloc(sizeof(PHY_VARS_UE*));
    PHY_vars_UE_g[0] = init_lte_UE(frame_parms, UE_id,abstraction_flag,transmission_mode);
    
    for (i=0;i<NUMBER_OF_eNB_MAX;i++) {
      PHY_vars_UE_g[0]->pusch_config_dedicated[i].betaOffset_ACK_Index = beta_ACK;
      PHY_vars_UE_g[0]->pusch_config_dedicated[i].betaOffset_RI_Index  = beta_RI;
      PHY_vars_UE_g[0]->pusch_config_dedicated[i].betaOffset_CQI_Index = beta_CQI;
    }
    
    compute_prach_seq(&PHY_vars_UE_g[0]->lte_frame_parms.prach_config_common,
		      PHY_vars_UE_g[0]->lte_frame_parms.frame_type,
		      PHY_vars_UE_g[0]->X_u);

    PHY_vars_UE_g[0]->lte_ue_pdcch_vars[0]->crnti = 0x1234;
#ifndef OPENAIR2
    PHY_vars_UE_g[0]->lte_ue_pdcch_vars[0]->crnti = 0x1235;
#endif
    NB_UE_INST=1;
    NB_INST=1;
    
    openair_daq_vars.manual_timing_advance = 0;
    //openair_daq_vars.timing_advance = TIMING_ADVANCE_HW;
    openair_daq_vars.rx_gain_mode = DAQ_AGC_ON;
    openair_daq_vars.auto_freq_correction = 0;
    openair_daq_vars.use_ia_receiver = 1;

    // if AGC is off, the following values will be used
    //    for (i=0;i<4;i++) 
    //    rxgain[i] = 20;
    rxgain[0] = 0;
    rxgain[1] = 0;
    rxgain[2] = 0;
    rxgain[3] = 0;

    for (i=0;i<4;i++) {
      PHY_vars_UE_g[0]->rx_gain_max[i] = rxg_max[i];
      PHY_vars_UE_g[0]->rx_gain_med[i] = rxg_med[i];
      PHY_vars_UE_g[0]->rx_gain_byp[i] = rxg_byp[i];
    }
  
    if ((mode == normal_txrx) || (mode == rx_calib_ue) || (mode == no_L2_connect) || (mode == debug_prach)) {
        for (i=0; i<4; i++) {
            PHY_vars_UE_g[0]->rx_gain_mode[i]  = max_gain;
            //            frame_parms->rfmode[i] = rf_mode_max[i];
	    rf_mode[i] = (rf_mode[i] & (~LNAGAINMASK)) | LNAMax;
        }
        PHY_vars_UE_g[0]->rx_total_gain_dB =  PHY_vars_UE_g[0]->rx_gain_max[0] + rxgain[0] - 30; //-30 because it was calibrated with a 30dB gain
    }
    else if ((mode == rx_calib_ue_med)) {
        for (i=0; i<4; i++) {
            PHY_vars_UE_g[0]->rx_gain_mode[i] = med_gain;
            //            frame_parms->rfmode[i] = rf_mode_med[i];
	    rf_mode[i] = (rf_mode[i] & (~LNAGAINMASK)) | LNAMed;
        }
	PHY_vars_UE_g[0]->rx_total_gain_dB =  PHY_vars_UE_g[0]->rx_gain_med[0]  + rxgain[0] - 30; //-30 because it was calibrated with a 30dB gain;
    }
    else if ((mode == rx_calib_ue_byp)) {
        for (i=0; i<4; i++) {
            PHY_vars_UE_g[0]->rx_gain_mode[i] = byp_gain;
            //            frame_parms->rfmode[i] = rf_mode_byp[i];
	    rf_mode[i] = (rf_mode[i] & (~LNAGAINMASK)) | LNAByp;
        }
        PHY_vars_UE_g[0]->rx_total_gain_dB =  PHY_vars_UE_g[0]->rx_gain_byp[0]  + rxgain[0] - 30; //-30 because it was calibrated with a 30dB gain;
    }
    
    PHY_vars_UE_g[0]->tx_power_max_dBm = tx_max_power;
   
    printf("tx_max_power = %d -> amp %d\n",tx_max_power,get_tx_amp(tx_max_power,tx_max_power));
  }
  else { //this is eNB
#ifdef OPENAIR2
    g_log->log_component[PHY].level = LOG_INFO;
#else
    g_log->log_component[PHY].level = LOG_INFO;
#endif
    g_log->log_component[PHY].flag  = LOG_HIGH;
    g_log->log_component[MAC].level = LOG_INFO;
    g_log->log_component[MAC].flag  = LOG_HIGH;
    g_log->log_component[RLC].level = LOG_INFO;
    g_log->log_component[RLC].flag  = LOG_HIGH;
    g_log->log_component[PDCP].level = LOG_INFO;
    g_log->log_component[PDCP].flag  = LOG_HIGH;
    g_log->log_component[OTG].level = LOG_INFO;
    g_log->log_component[OTG].flag  = LOG_HIGH;


    PHY_vars_eNB_g = malloc(sizeof(PHY_VARS_eNB*));
    PHY_vars_eNB_g[0] = init_lte_eNB(frame_parms,eNB_id,Nid_cell,cooperation_flag,transmission_mode,abstraction_flag);
    
    for (i=0;i<NUMBER_OF_UE_MAX;i++) {
      PHY_vars_eNB_g[0]->pusch_config_dedicated[i].betaOffset_ACK_Index = beta_ACK;
      PHY_vars_eNB_g[0]->pusch_config_dedicated[i].betaOffset_RI_Index  = beta_RI;
      PHY_vars_eNB_g[0]->pusch_config_dedicated[i].betaOffset_CQI_Index = beta_CQI;
    }

    compute_prach_seq(&PHY_vars_eNB_g[0]->lte_frame_parms.prach_config_common,
		      PHY_vars_eNB_g[0]->lte_frame_parms.frame_type,
		      PHY_vars_eNB_g[0]->X_u);
    
    NB_eNB_INST=1;
    NB_INST=1;

    openair_daq_vars.ue_dl_rb_alloc=0x1fff;
    openair_daq_vars.target_ue_dl_mcs=0;
    openair_daq_vars.ue_ul_nb_rb=6;
    openair_daq_vars.target_ue_ul_mcs=19;

    // if AGC is off, the following values will be used
    //    for (i=0;i<4;i++) 
    //      rxgain[i]=30;
    rxgain[0] = 10;
    rxgain[1] = 10;
    rxgain[2] = 10;
    rxgain[3] = 10;


    // set eNB to max gain
    PHY_vars_eNB_g[0]->rx_total_gain_eNB_dB =  rxg_max[0] + rxgain[0] - 30; //was measured at rxgain=30;
    for (i=0; i<4; i++) {
      //        frame_parms->rfmode[i] = rf_mode_max[i];
      rf_mode[i] = (rf_mode[i] & (~LNAGAINMASK)) | LNAMax;
    }


  }

  // Initialize card
  ret = openair0_open();
  if ( ret != 0 ) {
          if (ret == -1)
              printf("Error opening /dev/openair0");
          if (ret == -2)
              printf("Error mapping bigshm");
          if (ret == -3)
              printf("Error mapping RX or TX buffer");
          return(ret);
     }

  printf ("Detected %d number of cards, %d number of antennas.\n", openair0_num_detected_cards, openair0_num_antennas[card]);
  
  p_exmimo_config = openair0_exmimo_pci[card].exmimo_config_ptr;
  p_exmimo_id     = openair0_exmimo_pci[card].exmimo_id_ptr;
  
  printf("Card %d: ExpressMIMO %d, HW Rev %d, SW Rev 0x%d\n", card, p_exmimo_id->board_exmimoversion, p_exmimo_id->board_hwrev, p_exmimo_id->board_swrev);

  p_exmimo_config->framing.eNB_flag   = !UE_flag;
  p_exmimo_config->framing.tdd_config = 0;
  carrier_freq[0] = 0; //don't use this LIME for card 1
  carrier_freq[2] = 0; //don't use this LIME for card 1
  carrier_freq[3] = 0; //don't use this LIME for card 1
  for (ant = 0; ant<4; ant++) { 
    p_exmimo_config->rf.do_autocal[ant] = 1;
    p_exmimo_config->rf.rf_freq_rx[ant] = carrier_freq[ant];
    p_exmimo_config->rf.rf_freq_tx[ant] = carrier_freq[ant];
    p_exmimo_config->rf.rx_gain[ant][0] = rxgain[ant];
    p_exmimo_config->rf.tx_gain[ant][0] = txgain[ant];
    
    p_exmimo_config->rf.rf_local[ant]   = rf_local[ant];
    p_exmimo_config->rf.rf_rxdc[ant]    = rf_rxdc[ant];

    if ((carrier_freq[ant] >= 850000000) && (carrier_freq[ant] <= 865000000)) {
      p_exmimo_config->rf.rf_vcocal[ant]  = rf_vcocal_850[ant];
      p_exmimo_config->rf.rffe_band_mode[ant] = DD_TDD;	    
    }
    else {
      p_exmimo_config->rf.rf_vcocal[ant]  = rf_vcocal[ant];
      p_exmimo_config->rf.rffe_band_mode[ant] = B19G_TDD;	    
    }

    p_exmimo_config->rf.rffe_gain_txlow[ant] = 63;
    p_exmimo_config->rf.rffe_gain_txhigh[ant] = 63;
    p_exmimo_config->rf.rffe_gain_rxfinal[ant] = 63;
    p_exmimo_config->rf.rffe_gain_rxlow[ant] = 63;
  }
  if (UE_flag) {
    //p_exmimo_config->rf.rf_mode[0]    = my_rf_mode;
    p_exmimo_config->rf.rf_mode[0]    = 0;
    p_exmimo_config->rf.rf_mode[1]    = my_rf_mode;
  }
  else {
    //p_exmimo_config->rf.rf_mode[0]    = my_rf_mode;
    p_exmimo_config->rf.rf_mode[0]    = 0;
    p_exmimo_config->rf.rf_mode[1]    = my_rf_mode;
  }


  openair0_dump_config(card);

  for (ant=0;ant<4;ant++)
    p_exmimo_config->rf.do_autocal[ant] = 0;

  dump_frame_parms(frame_parms);
  
  mac_xface = malloc(sizeof(MAC_xface));
  
#ifdef OPENAIR2
  int eMBMS_active=0;
  l2_init(frame_parms,eMBMS_active,
	  0); // cba_group_active
  if (UE_flag == 1)
    mac_xface->dl_phy_sync_success (0, 0, 0, 1);
  else
    mac_xface->mrbch_phy_sync_failure (0, 0, 0);
#endif

  mac_xface->macphy_exit = &exit_fun;

#ifdef OPENAIR2
  //if (otg_enabled) {
    init_all_otg(0);
    g_otg->seed = 0;
    init_seeds(g_otg->seed);
    g_otg->num_nodes = 2;
    for (i=0; i<g_otg->num_nodes; i++){
      for (j=0; j<g_otg->num_nodes; j++){ 
	g_otg->application_idx[i][j] = 1;
	//g_otg->packet_gen_type=SUBSTRACT_STRING;
	g_otg->aggregation_level[i][j][0]=1;
	g_otg->application_type[i][j][0] = BCBR; //MCBR, BCBR
      }
    }
    init_predef_traffic();
    //  }
#endif

    number_of_cards = openair0_num_detected_cards;
    if (p_exmimo_id->board_exmimoversion==1) //ExpressMIMO1
      openair_daq_vars.timing_advance = 138;
    else //ExpressMIMO2
      openair_daq_vars.timing_advance = 45;

  // connect the TX/RX buffers
  if (UE_flag==1) {
      setup_ue_buffers(PHY_vars_UE_g[0],frame_parms,1);
      printf("Setting UE buffer to all-RX\n");
      // Set LSBs for antenna switch (ExpressMIMO)
      for (i=0; i<frame_parms->samples_per_tti*10; i++)
	for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
	  PHY_vars_UE_g[0]->lte_ue_common_vars.txdata[aa][i] = 0x00010001;
      
  }
  else {
    setup_eNB_buffers(PHY_vars_eNB_g[0],frame_parms,1);
      if (fs4_test==0)
        {
          printf("Setting eNB buffer to all-RX\n");
          // Set LSBs for antenna switch (ExpressMIMO)
          for (i=0; i<frame_parms->samples_per_tti*10; i++)
            for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
              PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][i] = 0x00010001;

	  // Set the last OFDM symbol of subframe 4 to TX to allow enough time for switch to settle
	  // (that's ok since the last symbol can be configured as SRS)
	  /*
          for (i=frame_parms->samples_per_tti*5-0*(frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples); 
	       i<frame_parms->samples_per_tti*5; i++)
            for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
              PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][i] = 0x0; 
	  */
        }
      else {
          printf("Setting eNB buffer to fs/4 test signal\n");
          for (j=0; j<PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti*10; j+=4)
            for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
              {
                amp = 0x8000;
                ((short*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa])[2*j+1] = 0;
                ((short*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa])[2*j+3] = amp-1;
                ((short*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa])[2*j+5] = 0;
                ((short*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa])[2*j+7] = amp;
                ((short*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa])[2*j] = amp-1;
                ((short*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa])[2*j+2] = 0;
                ((short*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa])[2*j+4] = amp;
                ((short*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa])[2*j+6] = 0;
              }
        }
    }

#ifdef EMOS
  error_code = rtf_create(CHANSOUNDER_FIFO_MINOR,CHANSOUNDER_FIFO_SIZE);
  if (error_code==0)
    printf("[OPENAIR][SCHED][INIT] Created EMOS FIFO %d\n",CHANSOUNDER_FIFO_MINOR);
  else if (error_code==ENODEV)
    printf("[OPENAIR][SCHED][INIT] Problem: EMOS FIFO %d is greater than or equal to RTF_NO\n",CHANSOUNDER_FIFO_MINOR);
  else if (error_code==ENOMEM)
    printf("[OPENAIR][SCHED][INIT] Problem: cannot allocate memory for EMOS FIFO %d\n",CHANSOUNDER_FIFO_MINOR);
  else 
    printf("[OPENAIR][SCHED][INIT] Problem creating EMOS FIFO %d, error_code %d\n",CHANSOUNDER_FIFO_MINOR,error_code);
#endif

  mlockall(MCL_CURRENT | MCL_FUTURE);

#ifdef RTAI
  // make main thread LXRT soft realtime
  task = rt_task_init_schmod(nam2num("MYTASK"), 9, 0, 0, SCHED_FIFO, 0xF);

  // start realtime timer and scheduler
  //rt_set_oneshot_mode();
  rt_set_periodic_mode();
  start_rt_timer(0);

  //now = rt_get_time() + 10*PERIOD;
  //rt_task_make_periodic(task, now, PERIOD);

  printf("Init mutex\n");
  //mutex = rt_get_adr(nam2num("MUTEX"));
  mutex = rt_sem_init(nam2num("MUTEX"), 1);
  if (mutex==0)
    {
      printf("Error init mutex\n");
      exit(-1);
    }
  else
    printf("mutex=%p\n",mutex);
#endif

  DAQ_MBOX = (volatile unsigned int *) openair0_exmimo_pci[card].rxcnt_ptr[0];

  // this starts the DMA transfers
  if (UE_flag!=1)
      openair0_start_rt_acquisition(card);


#ifdef XFORMS
  if (do_forms==1) {
      fl_initialize (&argc, argv, NULL, 0, 0);
      form_stats = create_form_stats_form();
      if (UE_flag==1) {
          form_ue[UE_id] = create_lte_phy_scope_ue();
          sprintf (title, "LTE DL SCOPE UE");
          fl_show_form (form_ue[UE_id]->lte_phy_scope_ue, FL_PLACE_HOTSPOT, FL_FULLBORDER, title);
      } else {
            for(UE_id=0;UE_id<scope_enb_num_ue;UE_id++) {
                form_enb[UE_id] = create_lte_phy_scope_enb();
                sprintf (title, "UE%d LTE UL SCOPE eNB",UE_id+1);
                fl_show_form (form_enb[UE_id]->lte_phy_scope_enb, FL_PLACE_HOTSPOT, FL_FULLBORDER, title);
            }
      }
      fl_show_form (form_stats->stats_form, FL_PLACE_HOTSPOT, FL_FULLBORDER, "stats");
      if (UE_flag==0) {
          for (UE_id=0;UE_id<scope_enb_num_ue;UE_id++) {
              if (otg_enabled) {
                  fl_set_button(form_enb[UE_id]->button_0,1);
                  fl_set_object_label(form_enb[UE_id]->button_0,"DL Traffic ON");
              }
              else {
                  fl_set_button(form_enb[UE_id]->button_0,0);
                  fl_set_object_label(form_enb[UE_id]->button_0,"DL Traffic OFF");
              }
          }
      }
      else {
          if (openair_daq_vars.use_ia_receiver) {
              fl_set_button(form_ue[UE_id]->button_0,1);
              fl_set_object_label(form_ue[UE_id]->button_0, "IA Receiver ON");
          }
          else {
              fl_set_button(form_ue[UE_id]->button_0,0);
              fl_set_object_label(form_ue[UE_id]->button_0, "IA Receiver OFF");
          }
      }

      ret = pthread_create(&thread2, NULL, scope_thread, NULL);
      printf("Scope thread created, ret=%d\n",ret);
    }
#endif

#ifdef EMOS
  ret = pthread_create(&thread3, NULL, emos_thread, NULL);
  printf("EMOS thread created, ret=%d\n",ret);
#endif

  rt_sleep_ns(10*FRAME_PERIOD);

#ifndef RTAI
  pthread_attr_init (&attr_dlsch_threads);
  pthread_attr_setstacksize(&attr_dlsch_threads,OPENAIR_THREAD_STACK_SIZE);
  //attr_dlsch_threads.priority = 1;
  p.sched_priority = sched_get_priority_max(SCHED_FIFO); //OPENAIR_THREAD_PRIORITY;
  pthread_attr_setschedparam  (&attr_dlsch_threads, &p);
  pthread_attr_setschedpolicy (&attr_dlsch_threads, SCHED_FIFO);
#endif

  // start the main thread
  if (UE_flag == 1) {
#ifdef RTAI
    thread1 = rt_thread_create(UE_thread, NULL, 100000000);
#else
    error_code = pthread_create(&thread1, &attr_dlsch_threads, UE_thread, NULL);
    if (error_code!= 0) {
      LOG_D(HW,"[lte-softmodem.c] Could not allocate UE_thread, error %d\n",error_code);
      return(error_code);
    }
    else {
      LOG_D(HW,"[lte-softmodem.c] Allocate UE_thread successful\n");
    }
#endif
#ifdef DLSCH_THREAD
    init_rx_pdsch_thread();
    rt_sleep_ns(FRAME_PERIOD/10);
    init_dlsch_threads();
#endif
    printf("UE threads created\n");
  }
  else {
#ifdef RTAI
    thread0 = rt_thread_create(eNB_thread, NULL, 100000000);
#else
    error_code = pthread_create(&thread0, &attr_dlsch_threads, eNB_thread, NULL);
    if (error_code!= 0) {
      LOG_D(HW,"[lte-softmodem.c] Could not allocate eNB_thread, error %d\n",error_code);
      return(error_code);
    }
    else {
      LOG_D(HW,"[lte-softmodem.c] Allocate eNB_thread successful\n");
    }
#endif
#ifdef ULSCH_THREAD
    init_ulsch_threads();
#endif
    printf("eNB threads created\n");
  }


  // wait for end of program
  printf("TYPE <ENTER> TO TERMINATE\n");
  getchar();

  // stop threads
  oai_exit=1;
  rt_sleep_ns(FRAME_PERIOD);

  printf("stopping threads\n");
#ifdef XFORMS
  if (do_forms==1)
    {
      pthread_join(thread2,&status);
        fl_hide_form(form_stats->stats_form);
        fl_free_form(form_stats->stats_form);
        if (UE_flag==1) {
            fl_hide_form(form_ue[UE_id]->lte_phy_scope_ue);
            fl_free_form(form_ue[UE_id]->lte_phy_scope_ue);
        } else {
            for(UE_id=0;UE_id<scope_enb_num_ue;UE_id++) {
                fl_hide_form(form_enb[UE_id]->lte_phy_scope_enb);
                fl_free_form(form_enb[UE_id]->lte_phy_scope_enb);
            }
        }
    }
#endif

#ifdef EMOS
  pthread_join(thread3,&status);
#endif

  // cleanup
  if (UE_flag == 1) {
#ifdef RTAI
    rt_thread_join(thread1); 
#else
    pthread_join(thread1,&status); 
#endif
#ifdef DLSCH_THREAD
    cleanup_dlsch_threads();
    cleanup_rx_pdsch_thread();
#endif
  }
  else {
#ifdef RTAI
    rt_thread_join(thread0); 
#else
    pthread_join(thread0,&status); 
#endif
#ifdef ULSCH_THREAD
    cleanup_ulsch_threads();
#endif
  }
#ifdef RTAI
  stop_rt_timer();
#endif

  printf("stopping card\n");
  openair0_stop(card);
  printf("closing openair0_lib\n");
  openair0_close();

#ifdef EMOS
  error_code = rtf_destroy(CHANSOUNDER_FIFO_MINOR);
  printf("[OPENAIR][SCHED][CLEANUP] EMOS FIFO closed, error_code %d\n", error_code);
#endif

  return 0;
}

void test_config(int card, int ant, unsigned int rf_mode, int UE_flag) {
    p_exmimo_config->framing.eNB_flag   = !UE_flag;
    p_exmimo_config->framing.tdd_config = 0;
    
    p_exmimo_config->rf.rf_freq_rx[ant] = 1907600000;
    p_exmimo_config->rf.rf_freq_tx[ant] = 1907600000;;
    p_exmimo_config->rf.rx_gain[ant][0] = 20;
    p_exmimo_config->rf.tx_gain[ant][0] = 10;
    p_exmimo_config->rf.rf_mode[ant] = rf_mode;
    
    p_exmimo_config->rf.rf_local[ant] = build_rflocal(20,25,26,04);
    p_exmimo_config->rf.rf_rxdc[ant] = build_rfdc(128, 128);
    p_exmimo_config->rf.rf_vcocal[ant] = (0xE<<6) + 0xE;
}

void setup_ue_buffers(PHY_VARS_UE *phy_vars_ue, LTE_DL_FRAME_PARMS *frame_parms, int carrier) {

  int i;
  if (phy_vars_ue) {

    if ((frame_parms->nb_antennas_rx>1) && (carrier>0)) {
      printf("RX antennas > 1 and carrier > 0 not possible\n");
      exit(-1);
    }

    if ((frame_parms->nb_antennas_tx>1) && (carrier>0)) {
      printf("TX antennas > 1 and carrier > 0 not possible\n");
      exit(-1);
    }
    
    // replace RX signal buffers with mmaped HW versions
    for (i=0;i<frame_parms->nb_antennas_rx;i++) {
      free(phy_vars_ue->lte_ue_common_vars.rxdata[i]);
      phy_vars_ue->lte_ue_common_vars.rxdata[i] = (s32*) openair0_exmimo_pci[card].adc_head[i+carrier];


      printf("rxdata[%d] @ %p\n",i,phy_vars_ue->lte_ue_common_vars.rxdata[i]);
    }
    for (i=0;i<frame_parms->nb_antennas_tx;i++) {
      free(phy_vars_ue->lte_ue_common_vars.txdata[i]);
      phy_vars_ue->lte_ue_common_vars.txdata[i] = (s32*) openair0_exmimo_pci[card].dac_head[i+carrier];

      printf("txdata[%d] @ %p\n",i,phy_vars_ue->lte_ue_common_vars.txdata[i]);
    }
  }
}

void setup_eNB_buffers(PHY_VARS_eNB *phy_vars_eNB, LTE_DL_FRAME_PARMS *frame_parms, int carrier) {

  int i,j;

  if (phy_vars_eNB) {
    if ((frame_parms->nb_antennas_rx>1) && (carrier>0)) {
      printf("RX antennas > 1 and carrier > 0 not possible\n");
      exit(-1);
    }

    if ((frame_parms->nb_antennas_tx>1) && (carrier>0)) {
      printf("TX antennas > 1 and carrier > 0 not possible\n");
      exit(-1);
    }
    
    // replace RX signal buffers with mmaped HW versions
    for (i=0;i<frame_parms->nb_antennas_rx;i++) {
        free(phy_vars_eNB->lte_eNB_common_vars.rxdata[0][i]);
        phy_vars_eNB->lte_eNB_common_vars.rxdata[0][i] = (s32*) openair0_exmimo_pci[card].adc_head[i+carrier];
        
        printf("rxdata[%d] @ %p\n",i,phy_vars_eNB->lte_eNB_common_vars.rxdata[0][i]);
        for (j=0;j<16;j++) {
            printf("rxbuffer %d: %x\n",j,phy_vars_eNB->lte_eNB_common_vars.rxdata[0][i][j]);
            phy_vars_eNB->lte_eNB_common_vars.rxdata[0][i][j] = 16-j;
        }
    }
    for (i=0;i<frame_parms->nb_antennas_tx;i++) {
        free(phy_vars_eNB->lte_eNB_common_vars.txdata[0][i]);
        phy_vars_eNB->lte_eNB_common_vars.txdata[0][i] = (s32*) openair0_exmimo_pci[card].dac_head[i+carrier];

        printf("txdata[%d] @ %p\n",i,phy_vars_eNB->lte_eNB_common_vars.txdata[0][i]);
        for (j=0;j<16;j++) {
            printf("txbuffer %d: %x\n",j,phy_vars_eNB->lte_eNB_common_vars.txdata[0][i][j]);
            phy_vars_eNB->lte_eNB_common_vars.txdata[0][i][j] = 16-j;
	}
    }
  }
}
