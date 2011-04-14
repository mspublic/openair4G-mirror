/*________________________phy_procedures.c________________________

 Authors : Hicham Anouar, Raymond Knopp, Florian Kaltenberger
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr, kaltenbe@eurecom.fr
________________________________________________________________*/


// This routine is called periodically by macphy_scheduler to analyse the set of PHY_primitives that were
// Scheduled by MAC and on PHY resources at the appropriate time

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

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/ESTIMATION/defs.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif

int k;
#ifndef USER_MODE
RTIME  now;            /* time when we started waiting        */
struct timespec timeout;        /* timeout value for the wait function */
#endif

//CHBCH_RX_t rx_mode = ML; //now defined in PHY/vars.h

void phy_procedures(unsigned char last_slot) {
  unsigned short i;
  int ret[2];
  int time_in,time_out;
  int chbch_tx_power,mrbch_tx_power;
  int chbch_crc[2],mrbch_crc;
  int chbch_index, chsch_index;
  //int chsch_index_max;
  unsigned char chsch_indices[2] = {1, 2};
  unsigned char *chbch_pdu_rx[2];
  int diff;


  // This should be protected when the MAC has knowledge of MRBCH
  static unsigned char dummy_mrbch_pdu[MRBCH_PDU_SIZE] __attribute__ ((aligned(16)));

#ifndef OPENAIR2
  static unsigned char dummy_chbch_pdu[CHBCH_PDU_SIZE] __attribute__ ((aligned(16)));
  static unsigned char dummy_chbch_pdu2[CHBCH_PDU_SIZE] __attribute__ ((aligned(16)));
#endif
  int timing_offset;		


  if ((last_slot == 0) && 
      (mac_xface->is_cluster_head == 0)) {
    
    time_in = openair_get_mbox();

    for (chsch_index = 0;
	 chsch_index < 4;
	 chsch_index++) {

      phy_channel_estimation_top(0,
				 chsch_index,
#ifdef HW_PREFIX_REMOVAL
				 1,
#else
				 0,
#endif //HW_PREFIX_REMOVAL			  
				 chsch_index,
				 NB_ANTENNAS_RX,
				 CHSCH );
    }

#ifdef OPENAIR2
    phy_decode_chbch_top();

    //    if (mac_xface->frame > 100)
    //      mac_xface->macphy_exit("");
#else //OPENAIR2

    if ((PHY_vars->PHY_measurements.rx_avg_power_dB[1] + PHY_vars->rx_vars[0].rx_total_gain_dB > TARGET_RX_POWER_MIN) &&
	(PHY_vars->PHY_measurements.rx_avg_power_dB[2] + PHY_vars->rx_vars[0].rx_total_gain_dB > TARGET_RX_POWER_MIN))
      { //use dual stream receiver

	dual_stream_flag = 1;
	openair_daq_vars.synch_source = 1;

	if (rx_mode == MMSE) {
	  phy_calc_mmse_filter(PHY_vars->chsch_data[1].channel_f,
			       PHY_vars->chsch_data[2].channel_f,
			       PHY_vars->chsch_data[1].channel_mmse_filter_f,
			       PHY_vars->chsch_data[1].det,
			       PHY_vars->chsch_data[1].idet,
			       PHY_vars->PHY_measurements.n0_power[0]);
	  
	  time_out = openair_get_mbox();
	  
#ifdef DEBUG_PHY	
	  if ((mac_xface->frame) % 100 == 0)
	    msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, calc_mmse_filter time_in %d,time_out %d, scheduler_interval_ns %d\n", 
		mac_xface->frame, last_slot,
		time_in,
		time_out,
		openair_daq_vars.scheduler_interval_ns);
#endif //DEBUG_PHY
	}

	chbch_pdu_rx[0] = dummy_chbch_pdu;
	chbch_pdu_rx[1] = dummy_chbch_pdu2;

	if (rx_mode == MMSE) {	
	  phy_decode_chbch_2streams(chsch_indices,
				    MMSE,
				    NB_ANTENNAS_RX,
				    NB_ANTENNAS_TX,
				    chbch_pdu_rx,
				    ret,
				    CHBCH_PDU_SIZE);
	}
	else if (rx_mode == ML) {
	  phy_decode_chbch_2streams_ml(chsch_indices,
				       ML,
				       NB_ANTENNAS_RX,
				       NB_ANTENNAS_TX,
				       chbch_pdu_rx,
				       ret,
				       CHBCH_PDU_SIZE);
	}
	else {
	  msg("[OPENAIR][PHY_PROCEDURES] rx_mode has to be ML or MMSE\n");
	}


      }
    else if (PHY_vars->PHY_measurements.rx_avg_power_dB[1] + PHY_vars->rx_vars[0].rx_total_gain_dB > TARGET_RX_POWER_MIN)
      {
	dual_stream_flag = 0;
	openair_daq_vars.synch_source = 1;

	ret[0] = phy_decode_chbch(1,
#ifdef BIT8_RXDMUX
				  1,
#endif 
				  NB_ANTENNAS_RX,
				  NB_ANTENNAS_TX,
				  dummy_chbch_pdu,
				  CHBCH_PDU_SIZE);
	ret[1] = -1;
      }
    else if (PHY_vars->PHY_measurements.rx_avg_power_dB[2] + PHY_vars->rx_vars[0].rx_total_gain_dB > TARGET_RX_POWER_MIN)
      {
	dual_stream_flag = 0;
	openair_daq_vars.synch_source = 2;

	ret[1] = phy_decode_chbch(2,
#ifdef BIT8_RXDMUX
				  1,
#endif 
				  NB_ANTENNAS_RX,
				  NB_ANTENNAS_TX,
				  dummy_chbch_pdu,
				  CHBCH_PDU_SIZE);
	ret[0] = -1;
      }
      
    for (i=0;i<2;i++) {
      if (ret[i] == -1) {
	PHY_vars->chbch_data[chsch_indices[i]].pdu_errors++;
	PHY_vars->chbch_data[chsch_indices[i]].pdu_errors_conseq++;
      }
      else
	PHY_vars->chbch_data[chsch_indices[i]].pdu_errors_conseq=0;
      
#ifdef DEBUG_PHY
      if ((mac_xface->frame) % 100 == 0)
	msg("[OPENAIR][PHY_PROCEDURES] Frame %d : CHBCH %d (consecutive) error count = %d (%d)\n",
	    mac_xface->frame,
	    chsch_indices[i],
	    PHY_vars->chbch_data[chsch_indices[i]].pdu_errors,
	    PHY_vars->chbch_data[chsch_indices[i]].pdu_errors_conseq);
#endif //DEBUG_PHY
    }

  
    time_out = openair_get_mbox();

#ifdef DEBUG_PHY	
    if ((mac_xface->frame/5) % 200 == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, decode_chbch time_in %d,time_out %d, scheduler_interval_ns %d\n", 
	  mac_xface->frame, last_slot,
	  time_in,
	  time_out,
	  openair_daq_vars.scheduler_interval_ns);
#endif //DEBUG_PHY  

#ifndef USER_MODE
    if (mac_xface->frame % 128 == 0)
      for (chsch_index = 0; chsch_index<4; chsch_index++) {
	diff = PHY_vars->chbch_data[chsch_index].pdu_errors - PHY_vars->chbch_data[chsch_index].pdu_errors_last;
	PHY_vars->chbch_data[chsch_index].pdu_fer = (diff*100)>>7;
	PHY_vars->chbch_data[chsch_index].pdu_errors_last = PHY_vars->chbch_data[chsch_index].pdu_errors;
      }

	   /*
    if (PHY_vars->PHY_measurements.rx_avg_power_dB[1] >= PHY_vars->PHY_measurements.rx_avg_power_dB[2])
      chsch_index_max = 1;
    else 
      chsch_index_max = 2;
	   */

    if ((PHY_vars->chbch_data[1].pdu_errors_conseq >= 100) && (PHY_vars->chbch_data[2].pdu_errors_conseq >= 100) ) {

      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: consecutive error count reached, resynchronizing\n",
	  mac_xface->frame);
      openair_daq_vars.mode=openair_NOT_SYNCHED;
      openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
      openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
#endif //CBMIMO1
      mac_xface->frame = -1;
      openair_daq_vars.synch_wait_cnt=0;
      openair_daq_vars.sched_cnt=-1;
    }


#endif //USER_MODE
#endif //OPENAIR2

    //phy_adjust_synch(0,openair_daq_vars.synch_source,16384,CHSCH);
    phy_adjust_synch_multi_CH(0,16384,CHSCH);

    if (mac_xface->frame % 100 == 0) {
      //phy_adjust_gain(0,16384,1);
      phy_adjust_gain_mesh (0, 16384);
    }

    time_out = openair_get_mbox();

    /*
    if ((mac_xface->frame/5) % 200 == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, decode_chbch time_in %d,time_out %d, scheduler_interval_ns %d\n", mac_xface->frame, last_slot,
	  time_in,time_out,openair_daq_vars.scheduler_interval_ns);
    */
  }

  else if ((last_slot == 1) &&
	   (mac_xface->is_cluster_head == 0)) {

    time_in = openair_get_mbox();
#ifdef OPENAIR2
    phy_decode_sach_top(last_slot);
#endif
    time_out = openair_get_mbox();

    /*    
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, decode_sach time_in %d,time_out %d, scheduler_interval_ns %d\n", mac_xface->frame, last_slot,
	  time_in,time_out,openair_daq_vars.scheduler_interval_ns);
    */


  }

  else if ((last_slot == SLOTS_PER_FRAME-2) && 
	   (mac_xface->is_cluster_head == 1)) {

    if (mac_xface->is_secondary_cluster_head) {
      chbch_index = 2;
    }
    else {
      chbch_index = 1;
    }
    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, Calling generate_chbch_top(%d), mbox %x\n", mac_xface->frame,last_slot,chbch_index,mbox);
    */

    time_in = openair_get_mbox();

    for (i=0;i<NB_ANTENNAS_TX;i++) {
      Zero_Buffer(&PHY_vars->tx_vars[i].TX_DMA_BUFFER[0],
		  2*SLOT_LENGTH_BYTES_NO_PREFIX);
    }

#ifdef OPENAIR2
    phy_generate_chbch_top(chbch_index);
#else
    for (i=0; i<CHBCH_PDU_SIZE; i++)
      dummy_chbch_pdu[i]=i;
    
    chbch_tx_power = phy_generate_chbch(chbch_index,
					0,
					NB_ANTENNAS_TX,
					dummy_chbch_pdu);

#endif

    time_out = openair_get_mbox();

    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, generate_chbch time_in %d,time_out %d, scheduler_interval_ns %d\n",
	  mac_xface->frame, last_slot,
	  time_in,time_out,openair_daq_vars.scheduler_interval_ns);
    */

#ifdef OPENAIR2
    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, calling generate_sach_top%d\n", mac_xface->frame,last_slot);
    */

    time_in = openair_get_mbox();
    phy_generate_sach_top(last_slot,time_in);
    time_out = openair_get_mbox();
    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, generate_sach time_in %d,time_out %d, scheduler_interval_ns %d\n", 
	  mac_xface->frame, last_slot,
	  time_in,time_out,openair_daq_vars.scheduler_interval_ns);
    */
#endif //OPENAIR2    
  }
	
  else if ((last_slot == SLOTS_PER_FRAME-1) && 
	   (mac_xface->is_cluster_head == 1)) {
    
    time_in = openair_get_mbox();


    phy_channel_estimation_top(0,
			       SYMBOL_OFFSET_MRSCH,
			       1,
			       MRSCH_INDEX,
			       NB_ANTENNAS_RX,
			       SCH);
    
    mrbch_crc = phy_decode_mrbch(MRSCH_INDEX,
				 NB_ANTENNAS_RX,
				 NB_ANTENNAS_TXRX,
				 dummy_mrbch_pdu,
				 MRBCH_PDU_SIZE);

    if (mrbch_crc == -1) {
      PHY_vars->mrbch_data[0].pdu_errors++;
      PHY_vars->mrbch_data[0].pdu_errors_conseq++;
    }
    else
      PHY_vars->mrbch_data[0].pdu_errors_conseq=0;

    if (mac_xface->frame % 128 == 0) {
	diff = PHY_vars->mrbch_data[0].pdu_errors - PHY_vars->mrbch_data[0].pdu_errors_last;
	PHY_vars->mrbch_data[0].pdu_fer = (diff*100)>>7;
	PHY_vars->mrbch_data[0].pdu_errors_last = PHY_vars->mrbch_data[0].pdu_errors;
    }


#ifdef DEBUG_PHY
    if ((mac_xface->frame/5) % 200 == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d : MRBCH (consecutive) error count = %d (%d)\n",
	  mac_xface->frame,
	  PHY_vars->mrbch_data[0].pdu_errors,
	  PHY_vars->mrbch_data[0].pdu_errors_conseq);
#endif //DEBUG_PHY

    //#endif //OPENAIR2
  
    time_out = openair_get_mbox();
    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, decode_mrbch time_in %d,time_out %d, scheduler_interval_ns %d\n", 
	  mac_xface->frame, last_slot,
	  time_in,time_out,openair_daq_vars.scheduler_interval_ns);
    */
    //fk 20080801 the timing offset should be calculated at the MR, not here
    //phy_calc_timing_offset(MRSCH_INDEX, 10000, SCH, &timing_offset);
    //
    //if ((mac_xface->frame/5) % 200 ==0) 
    //  msg("[PHY][PROCEDURES] Frame %d, timing_offset = %d\n",mac_xface->frame,timing_offset);


    if (mac_xface->is_secondary_cluster_head == 1) {

      
      if (((mac_xface->frame/5) % 200 == 0) && (PHY_vars->mrbch_data[0].pdu_errors_conseq > 0)) 
	msg("[OPENAIR][PHY_PROCEDURES] Frame %d: WARNING: MRBCH not detected, possible loss of synching\n",mac_xface->frame);
      

      if (PHY_vars->mrbch_data[0].pdu_errors_conseq >= 100) {
	openair_daq_vars.mode=openair_NOT_SYNCHED;
	openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
        openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
#endif //CBMIMO1
	mac_xface->frame = -1;
	openair_daq_vars.synch_wait_cnt=0;
	openair_daq_vars.sched_cnt=-1;
      }


      //if (mrbch_crc == 0) 
      phy_adjust_synch(0,MRSCH_INDEX,16384,SCH);

    }
      
    if (mac_xface->frame % 100 == 0)
      phy_adjust_gain (0, 16384, MRSCH_INDEX);
      
  }
  

  if ((last_slot == (SLOTS_PER_FRAME - 1))&&
      (mac_xface->is_cluster_head == 0)) {

    for (i=0;i<NB_ANTENNAS_TX;i++) {
      Zero_Buffer(&PHY_vars->tx_vars[i].TX_DMA_BUFFER[OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*2],
		  2*SLOT_LENGTH_BYTES_NO_PREFIX);
    }

    
    // generate MRSCH and MRBCH   
    
    time_in = openair_get_mbox();

    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, Generating MRSCH %d in symbol %d\n", 
	  mac_xface->frame, 
	  last_slot,
	  MRSCH_INDEX,
	  SYMBOL_OFFSET_MRSCH
	  );
    */

    phy_generate_sch(0,
		     MRSCH_INDEX,
		     SYMBOL_OFFSET_MRSCH,
		     0xffff,
		     0,
		     NB_ANTENNAS_TX);

    for (i=0; i<MRBCH_PDU_SIZE; i++)
      dummy_mrbch_pdu[i]=i;

    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, Generating MRBCH %d in symbol %d\n", 
	  mac_xface->frame, 
	  last_slot,
	  MRSCH_INDEX,
	  SYMBOL_OFFSET_MRSCH+1
	  );
    */

    mrbch_tx_power = phy_generate_mrbch(MRSCH_INDEX,
					0,
					NB_ANTENNAS_TX,
					dummy_mrbch_pdu);

    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, Generated MRBCH %d in symbol %d, txpower = %d dB\n", 
	  mac_xface->frame, 
	  last_slot,
	  MRSCH_INDEX,
	  SYMBOL_OFFSET_MRSCH+1,
	  mrbch_tx_power
	  );
    */
    time_out = openair_get_mbox();
    /*    
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, phy_generate_mrbch time_in %d,time_out %d, scheduler_interval_ns %d\n", mac_xface->frame, last_slot,
	  time_in,time_out,openair_daq_vars.scheduler_interval_ns);
    
    */


#ifdef OPENAIR2
    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, Calling generate_sach_top, mbox %d\n", mac_xface->frame,last_slot,openair_get_mbox());
    */

    time_in = openair_get_mbox();
    phy_generate_sach_top(last_slot,time_in);
    time_out = openair_get_mbox();
    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, generate_sach time_in %d,time_out %d, scheduler_interval_ns %d\n", mac_xface->frame, last_slot,
	  time_in,time_out,openair_daq_vars.scheduler_interval_ns);
    */

#endif
  }

  else if ((last_slot == SLOTS_PER_FRAME-1) && 
	   (mac_xface->is_cluster_head == 1)) {
    
#ifdef OPENAIR2
    
    //if (((mac_xface->frame/5) % 200) == 0)
    //     msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, Calling decode_sach_top\n",mac_xface->frame,last_slot);
    

    time_in = openair_get_mbox();
    phy_decode_sach_top(last_slot);
    time_out = openair_get_mbox();

    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, decode_sach time_in %d,time_out %d, scheduler_interval_ns %d\n", mac_xface->frame, last_slot,
	  time_in,time_out,openair_daq_vars.scheduler_interval_ns);
    */

#endif
  }
	
}  


