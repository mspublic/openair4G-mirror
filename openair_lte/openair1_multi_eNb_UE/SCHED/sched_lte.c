 
/*
 // \author R. Knopp
 // \date 02.06.2004   (initial WIDENS version)
 // updated 04.04.2006 (migration to 2.6.x kernels)
 // updated 01.06.2006 (updates by M. Guillaud for MIMO sounder, new HW tracking mechanism)
 // updated 15.01.2007 (RX/TX FIFO debug support, RK)
 // updated 21.05.2007 (structural changes,GET Frame fifo support, RK)
 // Created LTE version 02.10.2009
 *  @{ 
 */

/*
* @addtogroup _physical_layer_ref_implementation_
\section _process_scheduling_ Process Scheduling
This section deals with real-time process scheduling for PHY and synchronization with certain hardware targets (CBMIMO1).
*/

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

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif //CBMIMO1

#ifdef SERIAL_IO
#include "rtai_serial.h"
#endif

#ifdef EMOS
#include "phy_procedures_emos.h"
extern  fifo_dump_emos_UE emos_dump_UE;
#endif

/// Mutex for instance count on MACPHY scheduling 
pthread_mutex_t openair_mutex;
/// Condition variable for MACPHY thread
pthread_cond_t openair_cond;
/// Threads
pthread_t         threads[openair_SCHED_NB_THREADS]={NULL,NULL,NULL};
/// Thread Attributes
pthread_attr_t    attr_threads[openair_SCHED_NB_THREADS];
/// RX Signal FIFO for Testing without hardware
#define RX_SIG_FIFO_NUMBER 59 
int rx_sig_fifo = RX_SIG_FIFO_NUMBER;
/// RX Control FIFO for testing without hardware
#define RF_CNTL_FIFO_NUMBER 60 
int rf_cntl_fifo = RF_CNTL_FIFO_NUMBER;

#ifdef NOCARD_TEST

/// RX Signal FIFO Mutex
pthread_mutex_t openair_rx_fifo_mutex;
/// RX Signal FIFO Condition variable
pthread_cond_t openair_rx_fifo_cond;

/// Packet for RX Signal Control
RF_CNTL_PACKET rf_cntl_packet;
#endif //NOCARD_TEST

/// Global exit variable (exit on error or manual stop via IOCTL)
int exit_openair = 0;

extern int init_dlsch_threads(void);
extern void cleanup_dlsch_threads(void);

extern int dlsch_instance_cnt[8];
extern pthread_mutex_t dlsch_mutex[8];
extern pthread_cond_t dlsch_cond[8];

#define NO_SYNC_TEST 1

#ifdef CBMIMO1
#define NUMBER_OF_CHUNKS_PER_SLOT NUMBER_OF_OFDM_SYMBOLS_PER_SLOT
#define NUMBER_OF_CHUNKS_PER_FRAME (NUMBER_OF_CHUNKS_PER_SLOT * SLOTS_PER_FRAME)
#define SYNCH_WAIT_TIME 4000  // number of symbols between SYNCH retries
#define SYNCH_WAIT_TIME_RUNNING 100  // number of symbols between SYNCH retries
#define DRIFT_OFFSET 300
#define NS_PER_SLOT 500000
#define MAX_DRIFT_COMP 3000
#endif // CBMIMO1
 
#define MAX_SCHED_CNT 50000000


unsigned char first_sync_call;

void openair1_restart(void) {

  int i;
  
  for (i=0;i<number_of_cards;i++)
    openair_dma(i,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
  //  openair_daq_vars.tx_test=0;
  openair_daq_vars.sync_state = 0;
  mac_xface->frame = 0;

  /*
  if ((mac_xface->is_cluster_head) && (mac_xface->is_primary_cluster_head)) {
    openair_daq_vars.mode = openair_SYNCHED_TO_MRSCH;
  }
  else {
    openair_daq_vars.mode = openair_NOT_SYNCHED;
  }
  */

#ifdef OPENAIR2
  msg("[openair][SCHED][SYNCH] Clearing MAC Interface\n");
  mac_resynch();
#endif //OPENAIR2

}

//-----------------------------------------------------------------------------
/** MACPHY Thread */
static void * openair_thread(void *param) {
  //-----------------------------------------------------------------------------

  //------------------------------
#ifndef USER_MODE
  struct sched_param p;

#endif //  /* USER_MODE */


  u8           next_slot, last_slot;
  unsigned int time_in,time_out,i;
  int diff;


#ifdef SERIAL_IO
  msg("[SCHED][OPENAIR THREAD] Opening RT serial port interface\n");
  rt_spopen(COM1,38400,8,1,RT_SP_PARITY_NONE,RT_SP_NO_HAND_SHAKE,RT_SP_FIFO_SIZE_14);
  rt_msg("[SCHED][OPENAIR THREAD] Opened RT Serial Port Interface\n");
#endif
  
  p.sched_priority = OPENAIR_THREAD_PRIORITY;
  pthread_attr_setschedparam  (&attr_threads[OPENAIR_THREAD_INDEX], &p);
#ifndef RTAI_ISNT_POSIX
  pthread_attr_setschedpolicy (&attr_threads[OPENAIR_THREAD_INDEX], SCHED_FIFO);
#endif
  
  msg("[openair][SCHED][openair_thread] openair_thread started with id %x, fpu_flag = %x, cpuid = %d\n",(unsigned int)pthread_self(),pthread_self()->uses_fpu,rtai_cpuid());

  if (mac_xface->is_primary_cluster_head == 1) {
    msg("[openair][SCHED][openair_thread] Configuring openair_thread for primary clusterhead\n");
  }
  else if (mac_xface->is_secondary_cluster_head == 1) {
    msg("[openair][SCHED][openair_thread] Configuring openair_thread for secondary clusterhead\n");
  }
  else {
    msg("[openair][SCHED][openair_thread] Configuring OPENAIR THREAD for regular node\n");
  }
  
  exit_openair = 0;
  
  if (mac_xface->is_primary_cluster_head ==1) {
    PHY_vars->rx_total_gain_dB = 138;
    PHY_vars->rx_total_gain_eNB_dB = 138;
  }
  else {
    PHY_vars->rx_total_gain_dB = MIN_RF_GAIN;
    PHY_vars->rx_total_gain_eNB_dB = MIN_RF_GAIN;
  }

#ifdef CBMIMO1  
  for (i=0;i<number_of_cards;i++) 
    openair_set_rx_gain_cal_openair(i,PHY_vars->rx_total_gain_dB);
#endif
	
  // turn off AGC by default
  openair_daq_vars.rx_gain_mode = DAQ_AGC_ON;

  openair_daq_vars.synch_source = 1; //by default we sync to CH1

  // Inner thread endless loop
  // exits on error or normally
  
  openair_daq_vars.sched_cnt = 0;
  openair_daq_vars.instance_cnt = -1;

  for (i=0;i<number_of_cards;i++) 
    Zero_Buffer((void*)TX_DMA_BUFFER[i][0],
		4*SLOT_LENGTH_BYTES_NO_PREFIX);

  while (exit_openair == 0){
    
    pthread_mutex_lock(&openair_mutex);
        
    while (openair_daq_vars.instance_cnt < 0) {
      pthread_cond_wait(&openair_cond,&openair_mutex);
    }

    openair_daq_vars.instance_cnt--;
    pthread_mutex_unlock(&openair_mutex);	
    
    next_slot = (openair_daq_vars.slot_count + 1 ) % SLOTS_PER_FRAME;
    if (openair_daq_vars.slot_count==0) 
      last_slot = SLOTS_PER_FRAME-1;
    else
      last_slot = (openair_daq_vars.slot_count - 1 ) % SLOTS_PER_FRAME; 

    // msg("[SCHED][Thread] In, Synched ? %d, %d\n",openair_daq_vars.mode,SYNCHED);	

    //if (mac_xface->frame % 100 == 0)
    //  msg("[SCHED][OPENAIR_THREAD] frame = %d, slot_count %d, last %d, next %d\n", mac_xface->frame, openair_daq_vars.slot_count, last_slot, next_slot);

    if ((openair_daq_vars.mode != openair_NOT_SYNCHED) && (openair_daq_vars.node_running == 1)) {
      time_in = openair_get_mbox();

      //      mac_xface->macphy_scheduler(last_slot); 

#ifdef OPENAIR_LTE
      phy_procedures_lte(last_slot,next_slot);
#else
#ifdef EMOS
      phy_procedures_emos(last_slot);
#else
      phy_procedures(last_slot);
#endif
#endif 

      if (last_slot==SLOTS_PER_FRAME-2)
      	mac_xface->frame++;

      time_out = openair_get_mbox();
      diff = ((int) time_out - (int) time_in) % ((int) (NUMBER_OF_SYMBOLS_PER_FRAME));

      if (diff > (NUMBER_OF_OFDM_SYMBOLS_PER_SLOT+4)) { // we scheduled too late
	msg("[SCHED][OPENAIR_THREAD] Frame %d: last_slot %d, macphy_scheduler time_in %d, time_out %d, diff %d, scheduler_interval_ns %d\n", 
	    mac_xface->frame, last_slot,
	    time_in,time_out,
	    diff,
	    openair_daq_vars.scheduler_interval_ns);
	//openair1_restart();
	exit_openair = 1;
      }

      /*
      if (mac_xface->frame >= 10)
      	exit_openair = 1;
      */

      /*
#ifdef CBMIMO1
      switch (last_slot) {
      case 0:
	if (time_in>22) {
	  msg("[SCHED][OPENAIR_THREAD] Frame %d: last_slot %d, macphy_scheduler time_in %d,time_out %d, scheduler_interval_ns %d\n", mac_xface->frame, last_slot,
	      time_in,time_out,openair_daq_vars.scheduler_interval_ns);
	  //	  exit_openair = 1;
	  openair1_restart();
	}
	
	break;
      case 1:
	if (time_in>38) {
	  msg("[SCHED][OPENAIR_THREAD] Frame %d: last_slot %d, macphy_scheduler time_in %d,time_out %d, scheduler_interval_ns %d\n", mac_xface->frame, last_slot,
	      time_in,time_out,openair_daq_vars.scheduler_interval_ns);
	  //	  exit_openair = 1;
	  openair1_restart();
	}
	break;
      case 2:
	if (time_in>54) {
	  msg("[SCHED][OPENAIR_THREAD] Frame %d: last_slot %d, macphy_scheduler time_in %d,time_out %d, scheduler_interval_ns %d\n", mac_xface->frame, last_slot,
	      time_in,time_out,openair_daq_vars.scheduler_interval_ns);
	  //	  exit_openair = 1;
	  openair1_restart();
	}
	break;
      case 3:
	if (time_in>6) {
	  msg("[SCHED][OPENAIR_THREAD] Frame %d: last_slot %d, macphy_scheduler time_in %d,time_out %d, scheduler_interval_ns %d\n", mac_xface->frame, last_slot,
	      time_in,time_out,openair_daq_vars.scheduler_interval_ns);
	  msg("[SCHED][OPENAIR_THREAD] Frame %d: last_slot %d, NUMBER_OF_SYMBOLS_PER_FRAME = %d\n",mac_xface->frame, last_slot, pci_interface[0]->ofdm_symbols_per_frame);
	  openair1_restart();
	  //	  exit_openair = 1;
	}
	break;
      }
#endif //CBMIMO1
      */

#ifdef CBMIMO1  // Note this code cannot run on PLATON!!!
      if ((mac_xface->is_primary_cluster_head == 0) && (last_slot == 0)) {
	if (openair_daq_vars.first_sync_call == 1)
	  openair_daq_vars.first_sync_call = 0;
      }
#endif // CBMIMO1

    } //  daq_mode != NOT_SYNCHED
  } // end while (1)
  
  // report what happened

  msg ("[SCHED][OPENAIR_THREAD] Exited : openair_daq_vars.slot_count = %d, MODE = %d\n", openair_daq_vars.slot_count, openair_daq_vars.mode);
  
  // rt_task_delete(rt_whoami);
  /*  pthread_exit(NULL); */
#ifdef SERIAL_IO
  rt_spclose(COM1);
#endif
 
  return(0);
}


void openair_sync(void) {
  int i;
  //int size,j;
  int status;
  int length;
  int ret;
  static unsigned char clear = 1;
  int Nsymb, sync_pos, sync_pos_slot;
  int Ns;
  int l;
  int rx_power;
  unsigned int adac_cnt;
  int pbch_decoded = 0;

  RTIME time;

  openair_daq_vars.mode = openair_NOT_SYNCHED;

  //openair_set_lo_freq_openair(openair_daq_vars.freq,openair_daq_vars.freq);	

  for (i=0;i<number_of_cards;i++) {
    ret = setup_regs(i);

    openair_get_frame(i); //received frame is stored in PHY_vars->RX_DMA_BUFFER
  }
  // sleep during acquisition of frame

  time = rt_get_cpu_time_ns();

  for (i=0;i<3*NUMBER_OF_CHUNKS_PER_FRAME;i++) {
    rt_sleep(nano2count(NS_PER_SLOT/NUMBER_OF_OFDM_SYMBOLS_PER_SLOT));
    //adac_cnt      = (*(unsigned int *)mbox)%NUMBER_OF_CHUNKS_PER_FRAME;                 /* counts from 0 to NUMBER_OF_CHUNKS_PER_FRAME-1  */
    //msg("[openair][SCHED][SYNC] adac_cnt for i=%d adac_cnt= %d\n", i,adac_cnt);
  }
  
  time = rt_get_cpu_time_ns();


  if (openair_daq_vars.one_shot_get_frame == 1)  { // we're in a non real-time mode so just grab a frame and write to fifo

    status=rtf_reset(rx_sig_fifo);
    
    for (i=0;i<NB_ANTENNAS_RX;i++) {
      length=rtf_put(rx_sig_fifo,(unsigned char*)RX_DMA_BUFFER[0][i],FRAME_LENGTH_BYTES);
      if (length < FRAME_LENGTH_BYTES) {
	msg("[openair][sched][rx_sig_fifo_handler] Didn't put %d bytes for antenna %d (put %d)\n",FRAME_LENGTH_BYTES,i,length);
      }
      else {
	msg("[openair][sched][rx_sig_fifo_handler] Worte %d bytes for antenna %d to fifo (put %d)\n",FRAME_LENGTH_BYTES,i,length);
      }
    }    

    // signal that acquisition is done in control fifo
    rtf_put(rf_cntl_fifo,&openair_daq_vars.sched_cnt,sizeof(int));
    openair_daq_vars.one_shot_get_frame = 0;
  }

  if (openair_daq_vars.one_shot_get_frame == 0)  { // we're in a real-time mode so do basic decoding
 

    memcpy((void *)(RX_DMA_BUFFER[0][0]+FRAME_LENGTH_COMPLEX_SAMPLES),(void*)RX_DMA_BUFFER[0][0],OFDM_SYMBOL_SIZE_BYTES);
    memcpy((void *)(RX_DMA_BUFFER[0][1]+FRAME_LENGTH_COMPLEX_SAMPLES),(void*)RX_DMA_BUFFER[0][1],OFDM_SYMBOL_SIZE_BYTES);
    
#ifdef DEBUG_PHY
    for (i=0;i<number_of_cards;i++) 
      msg("[openair][SCHED][SYNC] card %d freq %d:%d, RX_DMA ADR 0 %x, RX_DMA ADR 1 %x, OFDM_SPF %d, RX_GAIN_VAL %x, TX_RX_SW %d, TCXO %d, NODE_ID %d\n",
	  (pci_interface[i]->freq_info>>1)&3,
	  (pci_interface[i]->freq_info>>3)&3,
	  pci_interface[i]->adc_head[0],
	  pci_interface[i]->adc_head[1],
	  pci_interface[i]->ofdm_symbols_per_frame,
	  pci_interface[i]->rx_gain_val,
	  pci_interface[i]->tx_rx_switch_point,
	  pci_interface[i]->tcxo_dac,
	  pci_interface[i]->node_id);        

#endif

    if (openair_daq_vars.node_configured&2) { // the node has been cofigured as a UE

      msg("[openair][SCHED][SYNCH] starting sync\n");

      // Do initial timing acquisition
      sync_pos = lte_sync_time(lte_ue_common_vars->rxdata, 
			       lte_frame_parms, 
			       LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*lte_frame_parms->samples_per_tti,
			       &lte_ue_common_vars->eNb_id);
      //sync_pos = 0;

      // this is only for visualization in the scope
      lte_ue_common_vars->sync_corr = sync_corr_ue;
      
      // the sync is in the 3rd (last_ symbol of the special subframe
      // so the position wrt to the start of the frame is 
      sync_pos_slot = OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*2+2) + 10;
      
      PHY_vars->rx_offset = sync_pos - sync_pos_slot;
      
      msg("[openair][SCHED][SYNCH] sync_pos = %d, sync_pos_slot =%d\n", sync_pos, sync_pos_slot);
      
      if (((sync_pos - sync_pos_slot) >=0 ) && 
	  ((sync_pos - sync_pos_slot) < (FRAME_LENGTH_COMPLEX_SAMPLES/2 - lte_frame_parms->samples_per_tti)) ) {
	
	for (l=0;l<lte_frame_parms->symbols_per_tti/2;l++) {
	  
	  slot_fep(lte_frame_parms,
		   lte_ue_common_vars,
		   l,
		   1,
		   sync_pos-sync_pos_slot,
		   0);
	}

	/*
	lte_ue_measurements(lte_ue_common_vars,
			    lte_frame_parms,
			    &PHY_vars->PHY_measurements,
			    sync_pos-sync_pos_slot,
			    0,
			    0);
	*/

	msg("[openair][SCHED][SYNCH] starting PBCH decode!\n");

	pbch_decoded = 0;
	for (frame_mod4=0;frame_mod4<4;frame_mod4++) {
	  pbch_tx_ant = rx_pbch(&PHY_vars_UE->lte_ue_common_vars,
				PHY_vars_UE->lte_ue_pbch_vars[0],
				&PHY_vars_UE->lte_frame_parms,
				0,
				SISO,
				frame_mod4);
	  if ((pbch_tx_ant>0) && (pbch_tx_ant<4)) {
	    PHY_vars_UE->lte_frame_parms.mode1_flag = 1;
	    pbch_decoded = 1;
	    break;
	  }
	  
	  pbch_tx_ant = rx_pbch(&PHY_vars_UE->lte_ue_common_vars,
				PHY_vars_UE->lte_ue_pbch_vars[0],
				&PHY_vars_eNb->lte_frame_parms,
				0,
				ALAMOUTI,
				frame_mod4);
	  if ((pbch_tx_ant>0) && (pbch_tx_ant<4)) {
	    PHY_vars_UE->lte_frame_parms.mode1_flag = 0;
	    pbch_decoded = 1;
	    break;
	  }
	}

	  
	if (pbch_decoded) {
	  msg("[openair][SCHED][SYNCH] pbch decoded sucessfully mode1_flag %d, frame_mod4 %d, tx_ant %d!\n",
	      PHY_vars_UE->lte_frame_parms.mode1_flag,frame_mod4,pbch_tx_ant);
	  lte_frame_parms->nb_antennas_tx = pbch_tx_ant;
	  lte_frame_parms->mode1_flag = (lte_ue_pbch_vars[0]->decoded_output[1] == 1);
	  openair_daq_vars.dlsch_transmission_mode = lte_ue_pbch_vars[0]->decoded_output[1];
	  
	  if (openair_daq_vars.node_running == 1) {
      
	    pci_interface[0]->frame_offset = PHY_vars->rx_offset;
	    
	    openair_daq_vars.mode = openair_SYNCHED;
	    mac_xface->frame = 0;
#ifdef OPENAIR2
	    msg("[openair][SCHED][SYNCH] Clearing MAC Interface\n");
	    mac_resynch();
#endif //OPENAIR2
	    openair_daq_vars.scheduler_interval_ns=NS_PER_SLOT;        // initial guess
	    openair_daq_vars.last_adac_cnt=-1;            

	    UE_mode = PRACH;
#ifdef OPENAIR2
	    mac_xface->chbch_phy_sync_success(0,0);	    
#endif
	  }
	  
	}
	else {
	  msg("[openair][SCHED][SYNCH] PBCH not decoded!\n");
	}
      }
    }

    // Measurements
    rx_power = 0;
    for (i=0;i<NB_ANTENNAS_RX; i++) {
      // energy[i] = signal_energy(lte_eNB_common_vars->rxdata[i], FRAME_LENGTH_COMPLEX_SAMPLES);
      PHY_vars->PHY_measurements.wideband_cqi[0][i] = signal_energy((int*)RX_DMA_BUFFER[0][i],FRAME_LENGTH_COMPLEX_SAMPLES);
      PHY_vars->PHY_measurements.wideband_cqi_dB[0][i] = dB_fixed(PHY_vars->PHY_measurements.wideband_cqi[0][i]);
      rx_power += PHY_vars->PHY_measurements.wideband_cqi[0][i];
    }
    PHY_vars->PHY_measurements.wideband_cqi_tot[0] = dB_fixed(rx_power);
    if (openair_daq_vars.rx_rf_mode == 0)
      PHY_vars->PHY_measurements.rx_rssi_dBm[0] = PHY_vars->PHY_measurements.wideband_cqi_tot[0] -  PHY_vars->rx_total_gain_dB + 25;
    else
      PHY_vars->PHY_measurements.rx_rssi_dBm[0] = PHY_vars->PHY_measurements.wideband_cqi_tot[0] -  PHY_vars->rx_total_gain_dB;

    msg("[openair][SCHED] RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), RX gain %d dB, TDD %d, Dual_tx %d\n",
	PHY_vars->PHY_measurements.rx_rssi_dBm[0], 
	PHY_vars->PHY_measurements.wideband_cqi_dB[0][0],
	PHY_vars->PHY_measurements.wideband_cqi_dB[0][1],
	PHY_vars->PHY_measurements.wideband_cqi[0][0],
	PHY_vars->PHY_measurements.wideband_cqi[0][1],
	PHY_vars->rx_total_gain_dB,
	pci_interface[0]->tdd,
	pci_interface[0]->dual_tx);

#ifdef EMOS
    memcpy(&emos_dump_UE.PHY_measurements[0], &PHY_vars->PHY_measurements, sizeof(PHY_MEASUREMENTS));
    emos_dump_UE.timestamp = rt_get_time_ns();
    emos_dump_UE.UE_mode = UE_mode;
    emos_dump_UE.rx_total_gain_dB = PHY_vars->rx_total_gain_dB;

    debug_msg("[SCHED_LTE] Writing EMOS data to FIFO\n");
    if (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump_UE, sizeof(fifo_dump_emos_UE))!=sizeof(fifo_dump_emos_UE)) {
      msg("[SCHED_LTE] Problem writing EMOS data to FIFO\n");
    }
#endif


    // Do AGC
    if (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON) {
      phy_adjust_gain (clear, 512, 0);
      if (clear == 1)
	clear = 0;
    }
  }

  msg("[openair][SCHED] leaving openair_sync()\n");

}


static void * top_level_scheduler(void *param) {
  
#ifdef CBMIMO1
  unsigned int adac_cnt;
#endif // CBMIMO1

  int adac_offset;
  int first_increment = 0;
  int i;
  int ret=0;  
  
  msg("[openair][SCHED][top_level_scheduler] top_level_scheduler started with id %x, MODE %d, cpuid = %d, fpu_flag = %d\n",(unsigned int)pthread_self(),openair_daq_vars.mode,rtai_cpuid(),pthread_self()->uses_fpu);

  openair_daq_vars.sched_cnt = 0;

  msg("[openair][SCHED][top_level_scheduler] SLOTS_PER_FRAME=%d, NUMBER_OF_CHUNKS_PER_SLOT=%d,  NUMBER_OF_CHUNKS_PER_FRAME=%d, PHY_vars->mbox %p\n",
      SLOTS_PER_FRAME,NUMBER_OF_CHUNKS_PER_SLOT, NUMBER_OF_CHUNKS_PER_FRAME, (void*)mbox);



#ifdef RTAI_ENABLED
  //  msg("[OPENAIR][SCHED] Sleeping ... MODE = %d\n",openair_daq_vars.mode);
  rt_sleep(nano2count(2*NS_PER_SLOT));
  //rt_sleep(2*NS_PER_SLOT);
  //  msg("[OPENAIR][SCHED] Awakening ... MODE = %d\n",openair_daq_vars.mode);
#endif //

  openair_daq_vars.sync_state=0;

  while (exit_openair == 0) {

#ifdef CBMIMO1
    adac_cnt      = (*(unsigned int *)mbox)%NUMBER_OF_CHUNKS_PER_FRAME;                 /* counts from 0 to NUMBER_OF_CHUNKS_PER_FRAME-1  */
#endif //

    openair_daq_vars.sched_cnt++;

    if ((openair_daq_vars.mode == openair_NOT_SYNCHED) && (openair_daq_vars.node_id != PRIMARY_CH)) {


      // DO Nothing
      if (openair_daq_vars.synch_wait_cnt <= 0) {


	rt_sleep(nano2count(NS_PER_SLOT));
	//rt_sleep(NS_PER_SLOT);

#ifdef CBMIMO1  // Note this code cannot run on PLATON!!!
	if (openair_daq_vars.tx_test == 0) 
	  openair_sync();
#endif // CBMIMO1

	msg("[openair][SCHED] adac_cnt = %d\n",adac_cnt);

	if ( (openair_daq_vars.mode != openair_NOT_SYNCHED) &&
	     (openair_daq_vars.node_running == 1) ){

	  msg("[openair][SCHED] staring RT acquisition, adac_cnt = %d\n",adac_cnt);
#ifdef CBMIMO1
	  for (i=0;i<number_of_cards;i++)
	    openair_dma(i,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_START_RT_ACQUISITION);
#endif //CBMIMO1
	  openair_daq_vars.sync_state = 1;
	  openair_daq_vars.sched_cnt = 0;
	  /*
	  for (i=0;i<NB_ANTENNAS_RX;i++){
	    bzero((void *)RX_DMA_BUFFER[0][0],FRAME_LENGTH_BYTES);
	  }
	  */

	  for (i=0;i<NUMBER_OF_CHUNKS_PER_FRAME;i++) {
	    rt_sleep(nano2count(NS_PER_SLOT/NUMBER_OF_OFDM_SYMBOLS_PER_SLOT));
	    //adac_cnt      = (*(unsigned int *)mbox)%NUMBER_OF_CHUNKS_PER_FRAME;                 /* counts from 0 to NUMBER_OF_CHUNKS_PER_FRAME-1  */
	    //msg("[openair][SCHED][SYNC] adac_cnt for i=%d adac_cnt= %d\n", i,adac_cnt);
	  }
	  //rt_sleep(nano2count(NS_PER_SLOT*SLOTS_PER_FRAME));
	  //rt_sleep(NS_PER_SLOT*SLOTS_PER_FRAME);
	}
#ifdef DEBUG_PHY
	else {
	  if (((openair_daq_vars.sched_cnt - 1) % (SYNCH_WAIT_TIME<<2) ) == 0)
	    msg("[openair][SCHED][SYNCH] Return MODE : NOT SYNCHED, sched_cnt = %d\n",openair_daq_vars.sched_cnt);
	}
#endif //DEBUG_PHY

	if (openair_daq_vars.node_running == 0){
	  openair_daq_vars.synch_wait_cnt = SYNCH_WAIT_TIME;
	}
	else {
	  openair_daq_vars.synch_wait_cnt = SYNCH_WAIT_TIME_RUNNING;
	}
      }
      openair_daq_vars.synch_wait_cnt--;

      rt_sleep(nano2count(NS_PER_SLOT));
      //rt_sleep(NS_PER_SLOT));

    }

    else {    // We're in synch with the CH or are a 1ary clusterhead
#ifndef NOCARD_TEST

      if (openair_daq_vars.sync_state == 0) { // This means we're a CH, so start RT acquisition!!
	openair_daq_vars.rach_detection_count=0;
	openair_daq_vars.sync_state = 1;
	openair_daq_vars.sched_cnt = 0;
	//openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL;

	msg("[openair][sched][top_level_scheduler] Starting RT acquisition, ofdm_symbols_per_frame %d, log2_symbol_size %d\n",
	    NUMBER_OF_SYMBOLS_PER_FRAME, LOG2_NUMBER_OF_OFDM_CARRIERS);

#ifdef CBMIMO1
	for (i=0;i<number_of_cards;i++) {
	  openair_dma(i,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_START_RT_ACQUISITION);
	  pci_interface[i]->tx_rx_switch_point = openair_daq_vars.tx_rx_switch_point;
	}
#endif //CBMIMO1

	mac_xface->frame = 0;
  
	openair_daq_vars.scheduler_interval_ns=NS_PER_SLOT;        // initial guess
	
	openair_daq_vars.last_adac_cnt=-1;            

      }

      if (openair_daq_vars.sync_state==1)       // waiting for first wrap-around of frame
      	{

	  if (adac_cnt==0)
	    {
	      msg("[openair][SCHED][top_level_scheduler] got adac_cnt=0, starting acquisition\n");
	      openair_daq_vars.sync_state=2; 
	      openair_daq_vars.last_adac_cnt=0; 
	      openair_daq_vars.slot_count=0;
	      openair_daq_vars.sched_cnt = 0;
	      first_increment = 0;
	      rt_sleep(nano2count(openair_daq_vars.scheduler_interval_ns));          /* sleep for one slot  */
	      //rt_sleep(openair_daq_vars.scheduler_interval_ns);          /* sleep for one slot  */
	    }
	  else
	    {
	      //msg("[openair][SCHED][top_level_scheduler] sync startup, current time=%llu, waitig for adac_cnt=0 (current adac_cnt=%d)\n",rt_get_time_ns(),adac_cnt); 
	      //msg("[openair][SCHED][top_level_scheduler] waiting for adac_cnt=0 (current adac_cnt=%d)\n",adac_cnt); 
	      rt_sleep(nano2count(NS_PER_SLOT/NUMBER_OF_OFDM_SYMBOLS_PER_SLOT/2));  /* sleep for half a SYMBOL */
	      //rt_sleep(NS_PER_SLOT/NUMBER_OF_OFDM_SYMBOLS_PER_SLOT/2);  /* sleep for half a SYMBOL */

	      if (openair_daq_vars.sched_cnt > 3*NUMBER_OF_SYMBOLS_PER_FRAME) {
		msg("[openair][SCHED][top_level_scheduler] got sched_cnt =%d, stopping\n",openair_daq_vars.sched_cnt);
		openair_daq_vars.mode = openair_NOT_SYNCHED;
		openair_daq_vars.sync_state=0; 
		openair1_restart();
		//openair_sched_exit("exit");
	      }
	      
	    }
	  
	  
	}
      else if (openair_daq_vars.sync_state==2)                 /* acquisition running, track hardware.... */
	{
	  
	  // if (mac_xface->frame % 100 == 0) 
	  //   msg("[openair][SCHED] frame %d: scheduler interval %d\n",mac_xface->frame,openair_daq_vars.scheduler_interval_ns);
	  
	  adac_offset=((int)adac_cnt-((int)openair_daq_vars.slot_count*NUMBER_OF_CHUNKS_PER_SLOT));
	  if (adac_offset > NUMBER_OF_CHUNKS_PER_FRAME)
	    adac_offset -= NUMBER_OF_CHUNKS_PER_FRAME;
	  else if (adac_offset < 0)
	    adac_offset += NUMBER_OF_CHUNKS_PER_FRAME;
	  
	  // if (mac_xface->frame % 100 == 0)
	  //  msg("[openair][SCHED] frame %d: adac_offset %d\n",mac_xface->frame,adac_offset);
	  
	  if (adac_offset > 20) {
	    msg("[openair][sched][top_level_scheduler] Frame %d : Scheduling too late (adac_offset %d, adac_cnt %d, slot_count %d, tx_test = %d), restarting ...\n",
		mac_xface->frame, adac_offset, adac_cnt, openair_daq_vars.slot_count,openair_daq_vars.tx_test);
	    openair1_restart();
	  }

	  if (adac_offset>=NUMBER_OF_CHUNKS_PER_SLOT)
	    {
	      if (adac_offset>NUMBER_OF_CHUNKS_PER_SLOT)        /* adjust openair_daq_vars.scheduler_interval_ns to track the ADAC counter */
		{
		  openair_daq_vars.scheduler_interval_ns-= DRIFT_OFFSET;
		  if (openair_daq_vars.scheduler_interval_ns < NS_PER_SLOT - MAX_DRIFT_COMP)
		    openair_daq_vars.scheduler_interval_ns = NS_PER_SLOT - MAX_DRIFT_COMP;
		    /*          msg("adac_offset=%d, openair_daq_vars.scheduler_interval_ns=%d\n",adac_offset,openair_daq_vars.scheduler_interval_ns);  */
		}
	      if (pthread_mutex_lock (&openair_mutex) != 0)                // Signal MAC_PHY Scheduler
		msg("[openair][SCHED][SCHED] ERROR pthread_mutex_lock\n");// lock before accessing shared resource
	      openair_daq_vars.instance_cnt++;
	      pthread_mutex_unlock (&openair_mutex);
	      //	      msg("[openair][SCHED][SCHED] Signaling MACPHY scheduler\n");
	      if (openair_daq_vars.instance_cnt == 0)   // PHY must have finished by now
		if (pthread_cond_signal(&openair_cond) != 0)
		  msg("[openair][SCHED][SCHED] ERROR pthread_cond_signal\n");// schedule L2/L1H TX thread
	      
	      openair_daq_vars.slot_count=(openair_daq_vars.slot_count+1) % SLOTS_PER_FRAME;
	      openair_daq_vars.last_adac_cnt=adac_cnt;
	      rt_sleep(nano2count(openair_daq_vars.scheduler_interval_ns));          
	      //rt_sleep(openair_daq_vars.scheduler_interval_ns);          
	      first_increment = 0;
	    }
	  else if (adac_offset<NUMBER_OF_CHUNKS_PER_SLOT)
            {

	      if (first_increment == 0) {
		openair_daq_vars.scheduler_interval_ns+=DRIFT_OFFSET;
		first_increment = 1;
	      }

	      if (openair_daq_vars.scheduler_interval_ns > NS_PER_SLOT + MAX_DRIFT_COMP)
		openair_daq_vars.scheduler_interval_ns = NS_PER_SLOT + MAX_DRIFT_COMP;


	      //	      msg("adac_offset=%d, openair_daq_vars.scheduler_interval_ns=%d, sleeping for 2us\n",adac_offset,openair_daq_vars.scheduler_interval_ns); 
	      rt_sleep(nano2count(2000));
	      //rt_sleep(2000);
	    }
	  
	} // tracking mode
#else //NOCARD_TEST

      if ((openair_daq_vars.slot_count  % SLOTS_PER_FRAME) == 0) {
	msg("[openair][SCHED][top_level_thread] Waiting for frame signal in fifo (instance cnt %d)\n",
	    openair_daq_vars.instance_cnt);
	rf_cntl_packet.frame = mac_xface->frame;
	rf_cntl_packet.rx_offset = PHY_vars->rx_offset;

	rtf_put(rf_cntl_fifo,&rf_cntl_packet,sizeof(RF_CNTL_PACKET));

	pthread_cond_wait(&openair_rx_fifo_cond,&openair_rx_fifo_mutex);
      }
      
      // here, wait for data in the test fifo
      if (pthread_mutex_lock (&openair_mutex) != 0)                // Signal MAC_PHY Scheduler
	msg("[openair][SCHED][SCHED] ERROR pthread_mutex_lock\n");// lock before accessing shared resource
      openair_daq_vars.instance_cnt++;
      pthread_mutex_unlock (&openair_mutex);
      if (openair_daq_vars.instance_cnt == 0)   // PHY must have finished by now
	if (pthread_cond_signal(&openair_cond) != 0)
	  msg("[openair][SCHED][SCHED] ERROR pthread_cond_signal\n");// schedule L2/L1H TX thread
      
      
      openair_daq_vars.slot_count=(openair_daq_vars.slot_count+1) % SLOTS_PER_FRAME;
      rt_sleep(nano2count(openair_daq_vars.scheduler_interval_ns));          /* sleep for one slot  */
      //rt_sleep(openair_daq_vars.scheduler_interval_ns);          /* sleep for one slot  */

      msg("[openair][SCHED][top_level_thread] slot_count =%d, scheduler_interval_ns = %d\n",openair_daq_vars.slot_count, openair_daq_vars.scheduler_interval_ns);

      
#endif //NOCARD_TEST      
    }  // in synch
  }    // exit_openair = 0

  msg("[openair][SCHED][top_level_thread] Exiting ... openair_daq_vars.sched_cnt = %d\n",openair_daq_vars.sched_cnt);
  openair_daq_vars.mode = openair_SCHED_EXIT;
  // schedule openair_thread to exit
  msg("[openair][SCHED][top_level_thread] Scheduling openair_thread to exit ... \n");
  if (pthread_mutex_lock (&openair_mutex) != 0)
    msg("[openair][SCHED][top_level_thread] ERROR pthread_mutex_lock\n");// lock before accessing shared resource
  
  openair_daq_vars.instance_cnt = 10;
  pthread_mutex_unlock (&openair_mutex);
  
  
  if (pthread_cond_signal(&openair_cond) != 0)
    msg("[openair][SCHED][top_level_thread] ERROR pthread_cond_signal\n");// schedule L2/L1H TX thread
  
  openair_daq_vars.node_running = 0;
  //  rt_task_delete(rt_whoami);
  /*  pthread_exit(NULL); */
  msg("[openair][SCHED][top_level_thread] Exiting top_level_scheduler ... \n");

  return(0);
}

#ifdef NOCARD_TEST
int rx_sig_fifo_handler(unsigned int fifo, int rw) {

  int  status,i;
  int length;
  
  if (rw=='w') {

    for (i=0;i<NB_ANTENNAS_RX;i++) {
      length=rtf_get(fifo,(unsigned char*)RX_DMA_BUFFER[0][0],FRAME_LENGTH_BYTES);
      if (length < FRAME_LENGTH_BYTES)
	msg("[openair][sched][rx_sig_fifo_handler] Didn't get %d bytes for antenna %d (got %d)\n",FRAME_LENGTH_BYTES,i,length);
    }    
    status=rtf_reset(fifo);
    msg("[openair][sched][rx_sig_fifo_handler] fifo reset status=%d\n",status);
    pthread_cond_signal(&openair_rx_fifo_cond);
    return(0);
  }
  else
    return(0);
}


#endif //NOCARD_TEST




int openair_sched_init(void) {
  
  int error_code;
  
  
  mac_xface->frame = 0;
  
  openair_daq_vars.scheduler_interval_ns=NS_PER_SLOT;        // initial guess
  
  openair_daq_vars.last_adac_cnt=-1;            
  
  
  
  pthread_mutex_init(&openair_mutex,NULL);
  
  pthread_cond_init(&openair_cond,NULL);
  
  
  if (mac_xface->is_primary_cluster_head == 1) {
    printk("[openair][SCHED][init] Configuring primary clusterhead\n");
  }
  else if (mac_xface->is_secondary_cluster_head == 1) {
    printk("[openair][SCHED][init] Configuring secondary clusterhead\n");
  }
  else {
    printk("[openair][SCHED][init] Configuring regular node\n");
  }

  openair_daq_vars.mode = openair_NOT_SYNCHED;
  UE_mode = NOT_SYNCHED;
  
  error_code = rtf_create(rx_sig_fifo, NB_ANTENNAS_RX*FRAME_LENGTH_BYTES);
  printk("[openair][SCHED][INIT] Created rx_sig_fifo (%d bytes), error_code %d\n",
      NB_ANTENNAS_RX*FRAME_LENGTH_BYTES,
      error_code); 
  error_code = rtf_create(rf_cntl_fifo, 256);
  printk("[openair][SCHED][INIT] Created rx_cntl_fifo, error_code %d\n",error_code);
#ifdef EMOS
  error_code = rtf_create(CHANSOUNDER_FIFO_MINOR,CHANSOUNDER_FIFO_SIZE);
  printk("[OPENAIR][SCHED][INIT] Created EMOS FIFO, error code %d\n",error_code);
#endif



 
#ifdef NOCARD_TEST
  /*RX Signal FIFOS HANDLER*/
  
  pthread_mutex_init(&openair_rx_fifo_mutex,NULL);
  pthread_cond_init(&openair_rx_fifo_cond,NULL);
  
  
#endif //NOCARD_TEST

  pthread_attr_init (&attr_threads[OPENAIR_THREAD_INDEX]);
  pthread_attr_setstacksize(&attr_threads[OPENAIR_THREAD_INDEX],OPENAIR_THREAD_STACK_SIZE);
  
  attr_threads[OPENAIR_THREAD_INDEX].priority = 1;
  
  openair_daq_vars.instance_cnt = -1;

  // Create openair_thread
  
  error_code = pthread_create(&threads[OPENAIR_THREAD_INDEX],
  			      &attr_threads[OPENAIR_THREAD_INDEX],
  			      openair_thread,
  			      (void *)0);
  

  if (error_code!= 0) {
    printk("[SCHED][OPENAIR_THREAD][INIT] Could not allocate openair_thread, error %d\n",error_code);
    return(error_code);
  }
  else {
    printk("[SCHED][OPENAIR_THREAD][INIT] Allocate openair_thread successful\n");
  }
  
  pthread_attr_init (&attr_threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX]);
  pthread_attr_setstacksize(&attr_threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],OPENAIR_THREAD_STACK_SIZE);
  attr_threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX].priority = 0;
   
  // Create top_level_scheduler
  error_code = pthread_create(&threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],
  			      &attr_threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],   //default attributes
  			      top_level_scheduler,
  			      (void *)0);

   
  
  // rt_change_prio(threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],0);
  if (error_code!= 0) {
    printk("[openair][SCHED][INIT] Could not allocate top_level_scheduler, error %d\n",error_code);
    return(error_code);
  }
  else {
    printk("[openair][SCHED][INIT] Allocate top_level_scheduler successfull\n");
  }

  // Done by pthread_create in rtai_posix.h
  //  rt_task_use_fpu(threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX],1);
  

#ifdef NOCARD_TEST
  /*RX Signal FIFOS HANDLER*/

  pthread_mutex_init(&openair_rx_fifo_mutex,NULL);
  pthread_cond_init(&openair_rx_fifo_cond,NULL);
  error_code = rtf_create(rx_sig_fifo, NB_ANTENNAS_RX*FRAME_LENGTH_BYTES);
  printk("[openair][SCHED][INIT] Created rx_sig_fifo, error_code %d\n",error_code);
  error_code = rtf_create_handler(rx_sig_fifo, X_FIFO_HANDLER(rx_sig_fifo_handler));
  printk("[openair][SCHED][INIT] Created rx_sig_fifo handler, error_code %d\n",error_code);
#endif //NOCARD_TEST

  //if (mac_xface->is_cluster_head == 0) 
  //FK mac_xface->is_cluster_head not initialized at this stage
    error_code = init_dlsch_threads();

  return(error_code);

}

void openair_sched_cleanup() {

  int error_code;

  exit_openair = 1;
  openair_daq_vars.mode = openair_SCHED_EXIT;
  pthread_exit(&threads[TOP_LEVEL_SCHEDULER_THREAD_INDEX]);//H.A

#ifdef NOCARD_TEST
  pthread_cond_destroy(&openair_rx_fifo_cond);
#endif //NOCARD_TEST

  pthread_mutex_destroy(&openair_mutex);
  pthread_cond_destroy(&openair_cond);

  error_code = rtf_destroy(rx_sig_fifo);
  printk("[OPENAIR][SCHED][CLEANUP] rx_sig_fifo closed, error_code %d\n", error_code);
  error_code = rtf_destroy(rf_cntl_fifo);
  printk("[OPENAIR][SCHED][CLEANUP] rf_cntl_fifo closed, error_code %d\n", error_code);
#ifdef EMOS
  error_code = rtf_destroy(CHANSOUNDER_FIFO_MINOR);
  printk("[OPENAIR][SCHED][CLEANUP] EMOS FIFO closed, error_code %d\n", error_code);
#endif

  //if (mac_xface->is_cluster_head == 0)
  //FK: mac_xface->is_cluster_head not initialized at this stage
    cleanup_dlsch_threads();

  printk("[openair][SCHED][CLEANUP] Done!\n");
}


void openair_sched_exit(char *str) {

  msg("%s\n",str);
  msg("[OPENAIR][SCHED] TTI %d: openair_sched_exit() called, preparing to exit ...\n",mac_xface->frame);
  
  exit_openair = 1;
  openair_daq_vars.mode = openair_SCHED_EXIT;
}



/*@}*/


