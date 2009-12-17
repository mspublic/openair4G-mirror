/*________________________phy_procedures_lte.c________________________

 Authors : Hicham Anouar, Raymond Knopp, Florian Kaltenberger
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr, kaltenbe@eurecom.fr
________________________________________________________________*/


// This routine is called periodically by macphy_scheduler to analyse the set of PHY_primitives that were
// Scheduled by MAC and on PHY resources at the appropriate time

/*
#ifndef USER_MODE
//#include "rt_compat.h"

#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/delay.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/mman.h>

#include <linux/slab.h>
//#include <linux/config.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>

#ifdef KERNEL2_6
//#include <linux/config.h>
#include <linux/slab.h>
#endif

#ifdef KERNEL2_4
#include <linux/malloc.h>
#include <linux/wrapper.h>
#endif
#endif //USER_MODE
*/

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif

extern inline unsigned int taus(void);

unsigned char dlsch_input_buffer[2700] __attribute__ ((aligned(16)));

int phy_procedures_lte(unsigned char last_slot, unsigned char next_slot) {

  /*
  int ret[2];
  int time_in,time_out;
  int diff;
  int timing_offset;		
  */
  int i,k,l,m,aa, sector;
  unsigned char pbch_pdu[PBCH_PDU_SIZE];
  int pbch_error;
#ifndef USER_MODE
  RTIME  now;            
#endif

  // DLSCH variables
  unsigned char mod_order[2]={4,4};
  unsigned int rb_alloc[4];
  MIMO_mode_t mimo_mode = SISO; //ALAMOUTI;
  unsigned short input_buffer_length;
  unsigned int coded_bits_per_codeword,nsymb;
  int inv_target_code_rate = 2;
  int subframe_offset;
#ifdef EMOS
  fifo_dump_emos emos_dump;
#endif

  /*
#ifndef OPENAIR2
  static unsigned char dummy_chbch_pdu[CHBCH_PDU_SIZE] __attribute__ ((aligned(16)));
  static unsigned char dummy_chbch_pdu2[CHBCH_PDU_SIZE] __attribute__ ((aligned(16)));
#endif
  */

  if (last_slot<0 || last_slot>=20 || next_slot<0 || next_slot>=20 ) {
    msg("[PHY_PROCEDURES_LTE] Frame %d, Error: last_slot =%d!\n",mac_xface->frame, last_slot);
    return(-1);
  }
  
  rb_alloc[0] = 0x01ffffff;  // RBs 0-31
  rb_alloc[1] = 0x00000000;  // RBs 32-63
  rb_alloc[2] = 0x00000000;  // RBs 64-95
  rb_alloc[3] = 0x00000000;  // RBs 96-109
  
  if (mac_xface->frame%1000 == 0)
    msg("[PHY_PROCEDURES_LTE] Calling phy_procedures for frame %d, slot %d\n",mac_xface->frame, last_slot);

  if (last_slot==SLOTS_PER_FRAME-1) {
    
    PHY_vars->PHY_measurements.rx_avg_power_dB[0] = 0;
    for (i=0;i<lte_frame_parms->nb_antennas_rx; i++) {
      // energy[i] = signal_energy(lte_eNB_common_vars->rxdata[i], FRAME_LENGTH_COMPLEX_SAMPLES);
      PHY_vars->PHY_measurements.rx_power[0][i] = signal_energy(PHY_vars->rx_vars[i].RX_DMA_BUFFER, FRAME_LENGTH_COMPLEX_SAMPLES);
      PHY_vars->PHY_measurements.rx_power_dB[0][i] = dB_fixed(PHY_vars->PHY_measurements.rx_power[0][i]);
      PHY_vars->PHY_measurements.rx_avg_power_dB[0] += PHY_vars->PHY_measurements.rx_power_dB[0][i];
    }
    PHY_vars->PHY_measurements.rx_avg_power_dB[0] /= lte_frame_parms->nb_antennas_rx;
    PHY_vars->PHY_measurements.rx_rssi_dBm[0] = PHY_vars->PHY_measurements.rx_avg_power_dB[0] -  PHY_vars->rx_vars[0].rx_total_gain_dB;
    
    if (mac_xface->frame%100 == 0)
      msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, RX RSSI %d dB, digital (%d, %d) dB, linear (%d, %d), RX gain %d dB\n",
	  mac_xface->frame, next_slot,
	  PHY_vars->PHY_measurements.rx_rssi_dBm[0], 
	  PHY_vars->PHY_measurements.rx_power_dB[0][0],
	  PHY_vars->PHY_measurements.rx_power_dB[0][1],
	  PHY_vars->PHY_measurements.rx_power[0][0],
	  PHY_vars->PHY_measurements.rx_power[0][1],
	  PHY_vars->rx_vars[0].rx_total_gain_dB);

    if (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON) 
      phy_adjust_gain (0,16384,0);
    
  }

  if (mac_xface->is_cluster_head == 0) {

    if (!dlsch_ue) {
      msg("[PHY_PROCEDURES_LTE] Can't get ue dlsch structures\n");
      return(-1);
    }
    for (i=0;i<2;i++) {
      if (!dlsch_ue[i]) {
	    msg("[PHY_PROCEDURES_LTE] Can't get ue dlsch structure %d\n",i);
	    return(-1);
      }
      dlsch_ue[i]->harq_processes[0]->mimo_mode           = mimo_mode;
      dlsch_ue[i]->harq_processes[0]->mod_order           = mod_order[i];
      dlsch_ue[i]->layer_index                            = 0;
      dlsch_ue[i]->harq_processes[0]->active              = 0;
      dlsch_ue[i]->harq_processes[0]->Nl                  = 1;
      dlsch_ue[i]->rvidx                                  = 0;
    }
    
    
    for (l=0;l<lte_frame_parms->symbols_per_tti/2;l++) {
      
      slot_fep(lte_frame_parms,
	       lte_ue_common_vars,
	       l,
	       last_slot,
	       (last_slot>>1)*lte_frame_parms->symbols_per_tti*lte_frame_parms->ofdm_symbol_size,
	       1);

#ifdef EMOS
      if (((last_slot==0) || (last_slot==1)) && ((l==0) || (l==4-lte_frame_parms->Ncp))) {

	for (sector=0; sector<3; sector++) 
	  for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++)
	    lte_dl_channel_estimation_emos(emos_dump.channel[sector],
					   lte_ue_common_vars->rxdataF,
					   lte_frame_parms,
					   last_slot,
					   aa,
					   l,
					   sector);
      }
#endif
      if ((last_slot==0) && (l==4-lte_frame_parms->Ncp)) {
	// Measurements
	/*
	lte_ue_measurements(lte_ue_common_vars,
			    lte_frame_parms,
			    &PHY_vars->PHY_measurements,
			    (last_slot>>1)*lte_frame_parms->symbols_per_tti*lte_frame_parms->ofdm_symbol_size);

	// AGC
	if (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON)
	  if (mac_xface->frame % 100 == 0)
	    phy_adjust_gain (0,16384,0);
	*/

	if (mac_xface->frame%100 == 0)
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, freq_offset_filt = %d \n",mac_xface->frame, last_slot, lte_ue_common_vars->freq_offset);

      }
    
      if ((last_slot==1) && (l==4-lte_frame_parms->Ncp)) {

	lte_adjust_synch(lte_frame_parms,
			 lte_ue_common_vars,
			 1,
			 16384);

	pbch_error = rx_pbch(lte_ue_common_vars,
			     lte_ue_pbch_vars,
			     lte_frame_parms,
			     SISO);
	if (pbch_error) {
	  lte_ue_pbch_vars->pdu_errors_conseq = 0;
#ifdef EMOS
	  PHY_vars->PHY_measurements.frame_tx = *((unsigned int*) lte_ue_pbch_vars->decoded_output);
#endif
	}
	else {
	  lte_ue_pbch_vars->pdu_errors_conseq++;
	  lte_ue_pbch_vars->pdu_errors++;
	}

	if (mac_xface->frame % 100 == 0) {
	  lte_ue_pbch_vars->pdu_fer = lte_ue_pbch_vars->pdu_errors - lte_ue_pbch_vars->pdu_errors_last;
	  lte_ue_pbch_vars->pdu_errors_last = lte_ue_pbch_vars->pdu_errors;
	}

	
	if (mac_xface->frame % 100 == 0) {
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH errors = %d, consecutive errors = %d!\n",
	      mac_xface->frame, last_slot, lte_ue_pbch_vars->pdu_errors, lte_ue_pbch_vars->pdu_errors_conseq);
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH received frame = %d!\n",
	      mac_xface->frame, last_slot,*((unsigned int*) lte_ue_pbch_vars->decoded_output));
	}

	if (lte_ue_pbch_vars->pdu_errors_conseq>20) {
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH consecutive errors > 20, going out of sync!\n",mac_xface->frame, last_slot);
	  openair_daq_vars.mode = openair_NOT_SYNCHED;
	  openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
	  openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
#endif //CBMIMO1
	  mac_xface->frame = -1;
	  openair_daq_vars.synch_wait_cnt=0;
	  openair_daq_vars.sched_cnt=-1;
	  
	  lte_ue_pbch_vars->pdu_errors_conseq=0;
	  lte_ue_pbch_vars->pdu_errors=0;
	  
	}

      }

      if ((last_slot > 1) && (last_slot<19)) {
	if (((last_slot%2)==0) && (l==(4-lte_frame_parms->Ncp))) 
	
	  // process symbols 0,1,2
	  for (m=lte_frame_parms->first_dlsch_symbol;m<(4-lte_frame_parms->Ncp);m++)
	    rx_dlsch(lte_ue_common_vars,
		     lte_ue_dlsch_vars,
		     lte_frame_parms,
		     m,
		     rb_alloc,
		     mod_order,
		     mimo_mode);
	
	if (((last_slot%2)==1) && (l==0)) 

	  // process symbols 3,4,5
	  for (m=4-lte_frame_parms->Ncp+1;m<(lte_frame_parms->symbols_per_tti/2);m++)
	    rx_dlsch(lte_ue_common_vars,
		     lte_ue_dlsch_vars,
		     lte_frame_parms,
		     m,
		     rb_alloc,
		     mod_order,
		     mimo_mode);

	if (((last_slot%2)==1) && (l==(4-lte_frame_parms->Ncp)))

	  // process symbols 6,7,8
	  for (m=(lte_frame_parms->symbols_per_tti/2)+1;m<(11-lte_frame_parms->Ncp*2);m++)
	    rx_dlsch(lte_ue_common_vars,
		     lte_ue_dlsch_vars,
		     lte_frame_parms,
		     m,
		     rb_alloc,
		     mod_order,
		     mimo_mode);
      }

      if ((last_slot > 2) || ((last_slot==0) && (mac_xface->frame>0))) {
	if (((last_slot%2)==0) && (l==0))
	  
	  // process symbols 10,11,12
	  for (m=(11-lte_frame_parms->Ncp*2+1);m<lte_frame_parms->symbols_per_tti;m++)
	    rx_dlsch(lte_ue_common_vars,
		     lte_ue_dlsch_vars,
		     lte_frame_parms,
		     m,
		     rb_alloc,
		     mod_order,
		     mimo_mode);
      }
    }

#ifdef EMOS
    // collect all the data for EMOS in emos_dump and write to FIFO
    if (last_slot == SLOTS_PER_FRAME-1) {

      emos_dump.timestamp = rt_get_time_ns();
      memcpy(&emos_dump.PHY_measurements,&PHY_vars->PHY_measurements,sizeof(PHY_MEASUREMENTS));
      memcpy(emos_dump.pbch_pdu[0],pbch_pdu,PBCH_PDU_SIZE);
      emos_dump.pdu_errors[0] = lte_ue_pbch_vars->pdu_errors;
      emos_dump.pdu_errors_last[0] = lte_ue_pbch_vars->pdu_errors_last;
      emos_dump.pdu_errors_conseq[0] = lte_ue_pbch_vars->pdu_errors_conseq;
      emos_dump.pdu_fer[0] = lte_ue_pbch_vars->pdu_fer;
      emos_dump.timing_offset = openair_daq_vars.timing_advance;
      emos_dump.freq_offset = lte_ue_common_vars->freq_offset;
      emos_dump.rx_total_gain_dB = PHY_vars->rx_vars[0].rx_total_gain_dB;
      emos_dump.mimo_mode = mimo_mode;

      if (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump, sizeof(fifo_dump_emos))!=sizeof(fifo_dump_emos)) {
	msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Problem writing EMOS data to FIFO\n",mac_xface->frame, last_slot);
	return(-1);
      }

      if (mac_xface->frame % 100 == 0)
	msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Writing EMOS data to FIFO\n",mac_xface->frame, last_slot);

    }
#endif //EMOS
  }

  else {

    generate_pilots_slot(lte_eNB_common_vars->txdataF,
			 AMP,
			 lte_frame_parms,
			 next_slot);


    if (next_slot == 0)
      generate_pss(lte_eNB_common_vars->txdataF,
		   AMP,
		   lte_frame_parms,
		   1);

    else if (next_slot == 1) {

#ifdef DEBUG_PHY
      if (mac_xface->frame%100 == 0)
	msg("[PHY_PROCEDURES_LTE] Calling generate_pbch for frame %d, slot %d\n",mac_xface->frame, next_slot);
#endif
      
      *((unsigned int*) pbch_pdu) = mac_xface->frame;
      
      generate_pbch(lte_eNB_common_vars->txdataF,
		    AMP,
		    lte_frame_parms,
		    pbch_pdu);
    }
    
    else if ((next_slot > 1) && (next_slot%2 == 0) && (next_slot<18)) {
      // fill all other frames with DLSCH

      if (!dlsch_eNb) {
	msg("[PHY_PROCEDURES_LTE] Can't get eNb dlsch structures\n");
	return(-1);
      }
      for (i=0;i<2;i++) {
	if (!dlsch_eNb[i]) {
	  msg("[PHY_PROCEDURES_LTE] Can't get eNb dlsch structure %d\n",i);
	  return(-1);
	}
	dlsch_eNb[i]->harq_processes[0]->mimo_mode          = mimo_mode;
	dlsch_eNb[i]->layer_index                           = 0;
	dlsch_eNb[i]->harq_processes[0]->mod_order          = mod_order[i];
	dlsch_eNb[i]->harq_processes[0]->active             = 0;
	dlsch_eNb[i]->harq_processes[0]->Nl                 = 1;
	dlsch_eNb[i]->rvidx                                 = 0;
      }

      nsymb = (lte_frame_parms->Ncp == 0) ? 14 : 12;
      coded_bits_per_codeword =( 25 * (12 * mod_order[0]) * (nsymb-lte_frame_parms->first_dlsch_symbol-3));
      input_buffer_length = ((int)(coded_bits_per_codeword/inv_target_code_rate))>>3;

      for (i=0;i<input_buffer_length;i++)
	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);

      dlsch_encoding(dlsch_input_buffer,
		     (input_buffer_length<<3),
		     lte_frame_parms,
		     dlsch_eNb[0],
		     0,               // harq_pid
		     25); // number of allocated RB

      dlsch_modulation(lte_eNB_common_vars->txdataF,
		       AMP,
		       next_slot/2,
		       lte_frame_parms,
		       dlsch_eNb[0],
		       0,               // harq_pid
		       rb_alloc); // RB allocation vector


#ifdef DEBUG_PHY
      if (mac_xface->frame%1000 == 0) {
	msg("[PHY_PROCEDURES_LTE] Calling generate_dlsch for frame %d, slot %d\n",mac_xface->frame, next_slot);
	for (i=0;i<300;i++)
	  msg("lte_eNB_common_vars->txdataF[0][%d] = %d\n",i+300*12,lte_eNB_common_vars->txdataF[0][i+300*14]);
	for (i=0;i<100;i++)
	  msg("dlsch_input_buffer[[%d] = %d\n",i,dlsch_input_buffer[i]);
      }
#endif

    }

  }
  
  return(0);
  
}


