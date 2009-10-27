// This routine is called periodically by macphy_scheduler to analyse the set of PHY_primitives that were
// Scheduled by MAC and on PHY resources at the appropriate time

#ifndef USER_MODE
//#include "rt_compat.h"

#ifdef RTAI_ENABLED
//#include <rtai.h>
//#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif //RTAI_ENABLED

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
#include "SCHED/defs.h"
#include "SCHED/extern.h"
#include "MAC_INTERFACE/extern.h"
#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif

#include "phy_procedures_emos.h"

#define MEASUREMENT_TX_POWER 30

static unsigned char  chbch_pdu[CHBCH_PDU_SIZE] __attribute__ ((aligned(16)));		// Creation of the CHBCH_PDU
static unsigned char  chbch_pdu2[CHBCH_PDU_SIZE] __attribute__ ((aligned(16)));		// Creation of the CHBCH_PDU
static unsigned char  mrbch_pdu[MRBCH_PDU_SIZE] __attribute__ ((aligned(16)));		// Creation of the MRBCH_PDU
static int   channel_f_unpacked[NB_ANTENNAS_RX][NUMBER_OF_OFDM_CARRIERS_EMOS] __attribute__ ((aligned(16))); // Final estimated channel (unpacked)

void phy_procedures(unsigned char last_slot)
{

  int i,aa;
  int time_in,time_out;
  int crc_status,mrbch_crc;
  int error = 0;
  int frame_offset = 0;
  int sch_index,chsch_index;
  int mrbch_tx_power;
  unsigned int frame_tx[3];
  int chbch_index;
#ifdef AGC_TEST	
  static int gain_test = 0;
  static int direction = -1;
#endif
  int ret[2];
  unsigned char chsch_indices[2] = {1, 2};
  unsigned char *chbch_pdu_rx[2];
  int diff;


#ifndef USER_MODE

#ifndef CBMIMO1
  int CHBCH_power,bch_offset;
#endif //CBMIMO1

  fifo_dump_emos emos_dump;


  chbch_pdu_rx[0] = chbch_pdu;
  chbch_pdu_rx[1] = chbch_pdu2;

  /*
  if (mac_xface->frame%100==0)
    msg("[PHY][PROCEDURES][EMOS] frame: %d, slot %d, is_cluster_head = %d, is_primary_cluster_head = %d, is_secondary_cluster_head = %d\n",
	mac_xface->frame,
	last_slot,
	mac_xface->is_cluster_head,
	mac_xface->is_primary_cluster_head,
	mac_xface->is_secondary_cluster_head);
  */

  if (mac_xface->is_cluster_head == 0) {
    if (last_slot == 0) // Channel estimation and decoding of CHBCH
      {

	//    msg("[PHY][PROCEDURES][EMOS] Frame %d: Calling decode_chbch\n", mac_xface->frame);
	time_in = openair_get_mbox();
	
	//phy_decode_chbch_top(0);

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

#ifdef DUALSTREAM
	switch (rx_mode) {

	case MMSE:

	  dual_stream_flag = 1;

	  phy_calc_mmse_filter(PHY_vars->chsch_data[1].channel_f,
			       PHY_vars->chsch_data[2].channel_f,
			       PHY_vars->chsch_data[1].channel_mmse_filter_f,
			       PHY_vars->chsch_data[1].det,
			       PHY_vars->chsch_data[1].idet,
			       PHY_vars->PHY_measurements.n0_power[0]);
	  
	  phy_decode_chbch_2streams(chsch_indices,
				    MMSE,
				    NB_ANTENNAS_RX,
				    NB_ANTENNAS_TX,
				    chbch_pdu_rx,
				    ret,
				    CHBCH_PDU_SIZE);

	  time_out = openair_get_mbox();
	  
#ifdef DEBUG_PHY	
	  if ((mac_xface->frame) % 100 == 0)
	    msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, calc_mmse_filter + decode_2streams time_in %d,time_out %d, scheduler_interval_ns %d\n", 
		mac_xface->frame, last_slot,
		time_in,
		time_out,
		openair_daq_vars.scheduler_interval_ns);
#endif //DEBUG_PHY
	  break;

	case ML: 
	  phy_decode_chbch_2streams_ml(chsch_indices,
				       ML,
				       NB_ANTENNAS_RX,
				       NB_ANTENNAS_TX,
				       chbch_pdu_rx,
				       ret,
				       CHBCH_PDU_SIZE);

#ifdef DEBUG_PHY	
	  if ((mac_xface->frame) % 100 == 0)
	    msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, decode_2streams_ml time_in %d,time_out %d, scheduler_interval_ns %d\n", 
		mac_xface->frame, last_slot,
		time_in,
		time_out,
		openair_daq_vars.scheduler_interval_ns);
#endif //DEBUG_PHY
	  break;


	case SINGLE:

	  if (PHY_vars->PHY_measurements.rx_avg_power_dB[1] > PHY_vars->PHY_measurements.rx_avg_power_dB[2])
	    {
	      dual_stream_flag = 0;
	      openair_daq_vars.synch_source = 1;

	      ret[0] = phy_decode_chbch(1,
#ifdef BIT8_RXDMUX
					1,
#endif 
					NB_ANTENNAS_RX,
					NB_ANTENNAS_TX,
					chbch_pdu_rx[0],
					CHBCH_PDU_SIZE);
	      ret[1] = -1;
	    }
	  else {
	    dual_stream_flag = 0;
	    openair_daq_vars.synch_source = 2;
	    
	    ret[1] = phy_decode_chbch(2,
#ifdef BIT8_RXDMUX
				      1,
#endif 
				      NB_ANTENNAS_RX,
				      NB_ANTENNAS_TX,
				      chbch_pdu_rx[1],
				      CHBCH_PDU_SIZE);
	    ret[0] = -1;
	  }

	  break;

	default:
	  msg("[OPENAIR][PHY_PROCEDURES] rx_mode has to be SINGLE, ML or MMSE\n");
	}

    for (i=0;i<2;i++) {
      PHY_vars->PHY_measurements.crc_status[i] = ret[i];

      if (ret[i] == -1) {
	PHY_vars->chbch_data[chsch_indices[i]].pdu_errors++;
	PHY_vars->chbch_data[chsch_indices[i]].pdu_errors_conseq++;
      }
      else {
	PHY_vars->chbch_data[chsch_indices[i]].pdu_errors_conseq=0;
      
	// we use a 3 time repetition code on the frame number
	// only if all three are the same, we update PHY_vars->PHY_measurements.frame_tx
	// otherwise we just increase the framestamp
	memcpy(frame_tx, chbch_pdu_rx[i]+sizeof(RTIME), sizeof(unsigned int));
	memcpy(frame_tx+1, chbch_pdu_rx[i]+sizeof(RTIME)+4, sizeof(unsigned int));
	memcpy(frame_tx+2, chbch_pdu_rx[i]+sizeof(RTIME)+8, sizeof(unsigned int));
			
	if ((frame_tx[0]==frame_tx[1]) && (frame_tx[1]==frame_tx[2]))
	  PHY_vars->PHY_measurements.frame_tx[i] = frame_tx[1];
	else 
	  PHY_vars->PHY_measurements.frame_tx[i]++;
	
      }

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

    if (mac_xface->frame % 128 == 0)
      for (chsch_index = 0; chsch_index<4; chsch_index++) {
	diff = PHY_vars->chbch_data[chsch_index].pdu_errors - PHY_vars->chbch_data[chsch_index].pdu_errors_last;
	PHY_vars->chbch_data[chsch_index].pdu_fer = (diff*100)>>7;
	PHY_vars->chbch_data[chsch_index].pdu_errors_last = PHY_vars->chbch_data[chsch_index].pdu_errors;
      }

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

#else //DUALSTREAM

    openair_daq_vars.synch_source = 1; 
    
	crc_status = phy_decode_chbch (openair_daq_vars.synch_source, 
#ifdef BIT8_RXDEMUX
				       1,
#endif
				       NB_ANTENNAS_RX,
				       NB_ANTENNAS_TXRX,
				       chbch_pdu, 
				       CHBCH_PDU_SIZE);
      
	PHY_vars->PHY_measurements.crc_status[0]= crc_status;


	if (crc_status == -1)
	  {
	    PHY_vars->chbch_data[1].pdu_errors++;
	    PHY_vars->chbch_data[1].pdu_errors_conseq++;
			
	    PHY_vars->PHY_measurements.frame_tx[0]++;      
	    // PHY_vars->PHY_measurements.frame_tx = mac_xface->frame + frame_offset;
	  }
	else
	  {
	    PHY_vars->chbch_data[1].pdu_errors_conseq=0;
			
	    // we use a 3 time repetition code on the frame number
	    // only if all three are the same, we update PHY_vars->PHY_measurements.frame_tx
	    // otherwise we just increase the framestamp
	    memcpy(frame_tx, chbch_pdu+sizeof(RTIME), sizeof(unsigned int));
	    memcpy(frame_tx+1, chbch_pdu+sizeof(RTIME)+4, sizeof(unsigned int));
	    memcpy(frame_tx+2, chbch_pdu+sizeof(RTIME)+8, sizeof(unsigned int));
			
	    if ((frame_tx[0]==frame_tx[1]) && (frame_tx[1]==frame_tx[2]))
	      PHY_vars->PHY_measurements.frame_tx[0] = frame_tx[1];
	    else 
	      PHY_vars->PHY_measurements.frame_tx[0]++;
      
	    // calculate the offset between the tx framestamp and the rx framestamp 
	    frame_offset = PHY_vars->PHY_measurements.frame_tx[0] - mac_xface->frame;
	  }
    
#ifdef DEBUG_PHY
	if (mac_xface->frame % 100 == 0)
	  {
	    msg("[PHY][PROCEDURES][EMOS] PHY_vars->PHY_measurements = %d\n",PHY_vars->PHY_measurements.frame_tx);
	    // msg("[PHY][PROCEDURES][EMOS] frame_tx(:) = [%d %d %d]\n",frame_tx[0],frame_tx[1],frame_tx[2]);
	  }
#endif
		
	if (PHY_vars->chbch_data[1].pdu_errors_conseq >= MAX_CHBCH_ERRORS)
	  {
	    msg("[PHY][PROCEDURES][EMOS] Frame %d : consecutive error count reached, resynchronizing\n",mac_xface->frame);
	    openair_daq_vars.mode=openair_NOT_SYNCHED;
	    openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
	    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
#endif
	    mac_xface->frame = -1;
	    openair_daq_vars.synch_wait_cnt=0;
	    openair_daq_vars.sched_cnt=-1;
	  }
    
#ifdef DEBUG_PHY
	if (mac_xface->frame % 100 == 0)
	  msg("[PHY][PROCEDURES][EMOS] Frame %d : CHBCH error count = %d\n",
	      mac_xface->frame,
	      PHY_vars->chbch_data[1].pdu_errors);
#endif
    
	//    msg("[PHY][PROCEDURES][EMOS] Frame %d: adjust synch done\n", mac_xface->frame);


#endif //DUALSTREAM

	// Tracking loop
	phy_adjust_synch_multi_CH(0,16384,CHSCH);
		
#ifdef CBMIMO1		
	// Automatic gain control
	if ((mac_xface->frame % 100 == 0) && (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON))
	  {
	    phy_adjust_gain_mesh (0, 16384);
	    // if (clear == 1)
	    // clear = 0;
	  }
#endif
		
	time_out = openair_get_mbox();

#ifdef DEBUG_PHY
	if ((mac_xface->frame%100) == 0)
	  msg("[PHY][PROCEDURES][EMOS] Frame %d: decode_chbch (last_slot %d),  time_in %d,time_out %d, scheduler_interval_ns %d\n",
	      mac_xface->frame, last_slot,
	      time_in,time_out, openair_daq_vars.scheduler_interval_ns);
#endif
    
      }

    else if (last_slot == (SLOTS_PER_FRAME - 3))

      {	// nothing to be done here 
      }

    else if (last_slot == (SLOTS_PER_FRAME - 2)) 
      {
	// do channel estimation on slot 1 

	time_in = openair_get_mbox();

#ifdef HW_PREFIX_REMOVAL
	phy_channel_est_emos(12, 12, 21, 0, TRUE, 1);
#else
	phy_channel_est_emos(12, 12, 21, 0, TRUE, 0);
#endif //HW_PREFIX_REMOVAL

      	// do channel estimation on slot 2

#ifdef HW_PREFIX_REMOVAL
	phy_channel_est_emos(22, 22, 31, 1, TRUE, 1);
#else
	phy_channel_est_emos(22, 22, 31, 1, TRUE, 0);
#endif


	time_out = openair_get_mbox();

#ifdef DEBUG_PHY
	if ((mac_xface->frame % 100) == 0)
	  msg("[PHY][PROCEDURES][EMOS] Frame %d: Channel estimation (last slot %d): time in %d, time out %d [symbols]\n",mac_xface->frame, last_slot, time_in, time_out);
#endif

      }

    else if (last_slot == (SLOTS_PER_FRAME - 1))
      {	//generate TX signal
	RTIME current_time;

	current_time=rt_get_time_ns();                             // relative time since boot in nanoseconds

	time_in = openair_get_mbox();

	for (i=0;i<NB_ANTENNAS_TX;i++) {
	  Zero_Buffer(&PHY_vars->tx_vars[i].TX_DMA_BUFFER[0],
		      FRAME_LENGTH_BYTES_NO_PREFIX);
	}


	// generate MRSCH and MRBCH   
	phy_generate_sch(0,
			 MRSCH_INDEX,
			 SYMBOL_OFFSET_MRSCH,
			 0xffff,
			 0,
			 NB_ANTENNAS_TX);

	for (i=0; i<MRBCH_PDU_SIZE; i++)
	  mrbch_pdu[i]=0;

	memcpy(mrbch_pdu,&current_time,sizeof(RTIME)); //8 byte
	memcpy(mrbch_pdu+sizeof(RTIME),&(PHY_vars->PHY_measurements.frame_tx),sizeof(unsigned int)); //4 byte

#ifdef DEBUG_PHY
	if (mac_xface->frame %100 ==1)
	  msg("[OPENAIR][PHY_PROCEDURES][EMOS] frame %d, frame_tx = %d\n",mac_xface->frame, PHY_vars->PHY_measurements.frame_tx);
#endif

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
					    mrbch_pdu);

	// generate SCH (use different one than MRSCH_INDEX!!!)
#ifdef DEBUG_PHY
	if ((mac_xface->frame % 100) == 0)
	  msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, Generating SCH %d for NODE_ID %d\n", 
	      mac_xface->frame, 
	      last_slot,
	      EMOS_SCH_INDEX,
	      NODE_ID[0]);
#endif

	if (NODE_ID[0]==8) {
	  for (i = 40; i < 50; i++)
	    {
#ifdef CBMIMO1 
	      // cbmimo1 already removes the cyclic prefix
	      phy_generate_sch (0, 0, i, 0xFFFF, 0, NB_ANTENNAS_TX);
#else  // User-space simulation or PLATON
	      phy_generate_sch (0, 0, i, 0xFFFF, 1, NB_ANTENNAS_TX);
#endif
	    }
	}
	else if (NODE_ID[0]==9) {
	  for (i = 50; i < 60; i++)
	    {
#ifdef CBMIMO1 
	      // cbmimo1 already removes the cyclic prefix
	      phy_generate_sch (0, 1, i, 0xFFFF, 0, NB_ANTENNAS_TX);
#else  // User-space simulation or PLATON
	      phy_generate_sch (0, 1, i, 0xFFFF, 1, NB_ANTENNAS_TX);
#endif
	    }
	}


	// Write stuff to FIFO
	emos_dump.timestamp = rt_get_time_ns();                             // relative time since boot in nanoseconds
	memcpy(&(emos_dump.PHY_measurements), &(PHY_vars->PHY_measurements), sizeof(PHY_MEASUREMENTS));

	memcpy(&(emos_dump.chbch_pdu[0]),chbch_pdu,CHBCH_PDU_SIZE);
	emos_dump.pdu_errors[0] = PHY_vars->chbch_data[1].pdu_errors;
	emos_dump.pdu_errors[1] = PHY_vars->chbch_data[2].pdu_errors;

	emos_dump.offset = PHY_vars->rx_vars[0].offset;
	emos_dump.rx_total_gain_dB = PHY_vars->rx_vars[0].rx_total_gain_dB;

	switch (rx_mode) {
	  case SINGLE:
	    emos_dump.rx_mode = 0;
	    break;
	  case MMSE:
	    emos_dump.rx_mode = 1;
	    break;
	  case ML:
	    emos_dump.rx_mode = 2;
	    break;
	  default:
	    emos_dump.rx_mode = 99;
	}


	//msg("[PHY][PROCEDURES][EMOS] writing to FIFO\n");

	// Write things to FIFO
	error = (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump, sizeof(fifo_dump_emos))!=sizeof(fifo_dump_emos));
		
	// Unpack the channel estimate
	for (sch_index=0;sch_index<2;sch_index++) {
	  for (aa=0;aa<NB_ANTENNAS_RX;aa++) {
	    for (i=0;i<NUMBER_OF_OFDM_CARRIERS;i++) {
	      channel_f_unpacked[aa][i] = PHY_vars->sch_data[sch_index].channel_f[aa][i<<1];
	    }
	  }
	  error += (rtf_put(CHANSOUNDER_FIFO_MINOR, channel_f_unpacked, sizeof(channel_f_unpacked))!=sizeof(channel_f_unpacked));
	}

	for (sch_index=0;sch_index<2;sch_index++) {
	  error += (rtf_put(CHANSOUNDER_FIFO_MINOR, PHY_vars->sch_data[sch_index].perror, NB_ANTENNAS_RX*48*4)!=NB_ANTENNAS_RX*48*4);
	}

	if (error && (mac_xface->frame%100) == 0)
	  msg("[PHY][PROCEDURES][EMOS] Error writing to FIFO\n");

	time_out = openair_get_mbox();

#ifdef DEBUG_PHY
	if ((mac_xface->frame % 100) == 0) {
	  msg("[PHY][PROCEDURES][EMOS] Frame %d: Generate MRSCH+MRBCH, dump to FIFO (last slot %d): time in %d, time out %d [symbols]\n",mac_xface->frame, last_slot, time_in, time_out);
	}
#endif

      }
  }
  else if  (mac_xface->is_cluster_head == 1) {
    if (last_slot == (SLOTS_PER_FRAME-2)) {
      time_in = openair_get_mbox();

      phy_channel_estimation_top(0,
				 SYMBOL_OFFSET_MRSCH,
				 1,
				 MRSCH_INDEX,
				 NB_ANTENNAS_RX,
				 SCH);

      mrbch_crc = phy_decode_mrbch(MRSCH_INDEX,
#ifdef BIT8_RXDMUX
				   1,
#endif 
				   NB_ANTENNAS_RX,
				   NB_ANTENNAS_TXRX,
				   mrbch_pdu,
				   MRBCH_PDU_SIZE);

      PHY_vars->PHY_measurements.crc_status[0] = mrbch_crc;
      if (mac_xface->is_primary_cluster_head) {
	PHY_vars->PHY_measurements.frame_tx[0] = mac_xface->frame;
      }
      else {
	if (mrbch_crc != -1) {
	  //memcpy(frame_tx, mrbch_pdu+sizeof(RTIME), sizeof(unsigned int));
	  PHY_vars->PHY_measurements.frame_tx[0] = ((unsigned int*) mrbch_pdu)[2];
	}
	else {
	  PHY_vars->PHY_measurements.frame_tx[0]++;
	}
      }

#ifdef DEBUG_PHY
      if (mac_xface->frame % 100 == 0) {
	msg("[OPENAIR][PHY_PROCEDURES][EMOS]  Frame %d: crc=%d, Frame_tx=%d\n",mac_xface->frame,mrbch_crc,PHY_vars->PHY_measurements.frame_tx);
	msg("[OPENAIR][PHY_PROCEDURES][EMOS]  Frame %d:, mrbch_pdu = ");
	for (i=0; i<MRBCH_PDU_SIZE; i++)
	  msg("%d, ",mrbch_pdu[i]);
	msg("\n");
      }
#endif

      if (mrbch_crc == -1) {
	PHY_vars->mrbch_data[0].pdu_errors++;
	PHY_vars->mrbch_data[0].pdu_errors_conseq++;
      }
      else
	PHY_vars->mrbch_data[0].pdu_errors_conseq=0;
      
#ifdef DEBUG_PHY
      if ((mac_xface->frame/5) % 200 == 0)
	msg("[OPENAIR][PHY_PROCEDURES] Frame %d : MRBCH (consecutive) error count = %d (%d)\n",
	    mac_xface->frame,
	    PHY_vars->mrbch_data[0].pdu_errors,
	    PHY_vars->mrbch_data[0].pdu_errors_conseq);
#endif //DEBUG_PHY

      if (mac_xface->is_secondary_cluster_head == 1) {

	if (PHY_vars->mrbch_data[0].pdu_errors_conseq >= 1000) {

	  msg("[OPENAIR][PHY_PROCEDURES] Frame %d: MRBCH consecutive error count reached, resynching\n",mac_xface->frame);

	  
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
      if ((mac_xface->frame % 100 == 0) && (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON))
         phy_adjust_gain (0, 16384, MRSCH_INDEX);
      
	
      time_out = openair_get_mbox();

#ifdef DEBUG_PHY
      if (((mac_xface->frame) % 100) == 0)
	msg("[OPENAIR][PHY_PROCEDURES_EMOS] Frame %d: decode_mrbch (last_slot %d) time_in %d,time_out %d, scheduler_interval_ns %d\n", 
	    mac_xface->frame, last_slot,
	    time_in,time_out,openair_daq_vars.scheduler_interval_ns);
#endif


    }
    else if (last_slot == (SLOTS_PER_FRAME - 3))
      { // generate tx signal
	RTIME current_time;

	current_time=rt_get_time_ns();                             // relative time since boot in nanoseconds

	time_in = openair_get_mbox();

	for (i=0;i<NB_ANTENNAS_TX;i++) {
	  Zero_Buffer(&PHY_vars->tx_vars[i].TX_DMA_BUFFER[0],
		      2*SLOT_LENGTH_BYTES_NO_PREFIX);
	}

	// Initialization of the EMOS TX signal Part II: CHBCH

	// generate chbch_pdu
	// It includes timestamp and a flag for recording which can be set using ioctl
	// the flag should be set by the BS using openair_rf_cbmimo1 2 39 1
	// Zero_Buffer(chbch_pdu, CHBCH_PDU_SIZE);
	memcpy(chbch_pdu,&current_time,sizeof(RTIME)); //8 byte
	// sprintf(chbch_pdu+sizeof(RTIME),"EMOS"); //4 byte
	// we use a 3 time repetition code on the frame number
	memcpy(chbch_pdu+sizeof(RTIME),&(mac_xface->frame),sizeof(unsigned int)); //4 byte
	memcpy(chbch_pdu+sizeof(RTIME)+4,&(mac_xface->frame),sizeof(unsigned int)); //4 byte
	memcpy(chbch_pdu+sizeof(RTIME)+8,&(mac_xface->frame),sizeof(unsigned int)); //4 byte
    
	//chbch_pdu[16] = PHY_vars->PHY_measurements.Meas_flag; //1 byte
		
	// Generation of the CHBCH
	if (mac_xface->is_secondary_cluster_head) {
	  chbch_index = 2;
	}
	else {
	  chbch_index = 1;
	}

#ifdef CBMIMO1
	phy_generate_chbch (chbch_index, 0, NB_ANTENNAS_TX, chbch_pdu);
#else
	CHBCH_power = phy_generate_chbch (chbch_index, 1, NB_ANTENNAS_TX,chbch_pdu);
	bch_offset = MEASUREMENT_TX_POWER - CHBCH_power;        //  target - actual digital power
#ifdef AGC_TEST
	if (mac_xface->frame % 100 == 0) 
	  {
	    if (gain_test >= 0)
	      direction = -1;
	    else if (gain_test <=-20)
	      direction = 1;
	    // else don't change direction
	    gain_test += direction;
	    msg("[PHY][PROCEDURES][EMOS] Set TX gain to %d\n",bch_offset+gain_test);
	  }
	bch_offset += gain_test;				
#endif //AGC_TEST
		
		
	if ((mac_xface->frame % 100) == 0)
	  msg("[PHY][PROCEDURES][EMOS] CHBCH_power %d, bch_offset %d\n",CHBCH_power,bch_offset);

	for (i=0;i<NB_ANTENNAS_RX;i++)
	  openair_set_tx_gain(i,bch_offset);
#endif //CBMIMO1

	// Generation of the SCH
	// Initialization of the TX signal for EMOS Part I: Pilot symbols
	if (mac_xface->is_primary_cluster_head) {
	  for (i = 12; i < 22; i++)
	    {
#ifdef CBMIMO1 
	      // cbmimo1 already removes the cyclic prefix
	      phy_generate_sch (0, 0, i, 0xFFFF, 0, NB_ANTENNAS_TX);
#else  // User-space simulation or PLATON
	      phy_generate_sch (0, 0, i, 0xFFFF, 1, NB_ANTENNAS_TX);
#endif
	    }
	}
	else if (mac_xface->is_secondary_cluster_head) {
	  for (i = 22; i < 32; i++)
	    {
#ifdef CBMIMO1 
	      // cbmimo1 already removes the cyclic prefix
	      phy_generate_sch (0, 1, i, 0xFFFF, 0, NB_ANTENNAS_TX);
#else  // User-space simulation or PLATON
	      phy_generate_sch (0, 1, i, 0xFFFF, 1, NB_ANTENNAS_TX);
#endif
	    }
	}

	// Write stuff to FIFO
	emos_dump.timestamp = rt_get_time_ns();                             // relative time since boot in nanoseconds
	memcpy(&(emos_dump.PHY_measurements), &(PHY_vars->PHY_measurements), sizeof(PHY_MEASUREMENTS));

	memcpy(&(emos_dump.chbch_pdu[0]),mrbch_pdu,MRBCH_PDU_SIZE);
	emos_dump.pdu_errors[0] = PHY_vars->mrbch_data[0].pdu_errors;

	emos_dump.offset = PHY_vars->rx_vars[0].offset;
	emos_dump.rx_total_gain_dB = PHY_vars->rx_vars[0].rx_total_gain_dB;

	//msg("[PHY][PROCEDURES][EMOS] writing to FIFO\n");

	// Write things to FIFO
	error = (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump, sizeof(fifo_dump_emos))!=sizeof(fifo_dump_emos));
		
	// Unpack the channel estimate
	for (sch_index=0;sch_index<2;sch_index++) {
	  for (aa=0;aa<NB_ANTENNAS_RX;aa++) {
	    for (i=0;i<NUMBER_OF_OFDM_CARRIERS;i++) {
	      channel_f_unpacked[aa][i] = PHY_vars->sch_data[sch_index].channel_f[aa][i<<1];
	    }
	  }
	  error += (rtf_put(CHANSOUNDER_FIFO_MINOR, channel_f_unpacked, sizeof(channel_f_unpacked))!=sizeof(channel_f_unpacked));
	}

	for (sch_index=0;sch_index<2;sch_index++) {
	  error += (rtf_put(CHANSOUNDER_FIFO_MINOR, PHY_vars->sch_data[sch_index].perror, NB_ANTENNAS_RX*48*4)!=NB_ANTENNAS_RX*48*4);
	}

	if (error && (mac_xface->frame%100) == 0)
	  msg("[PHY][PROCEDURES][EMOS] Error writing to FIFO\n");

	time_out = openair_get_mbox();

#ifdef DEBUG_PHY
	if ((mac_xface->frame % 100) == 0) {
	  msg("[PHY][PROCEDURES][EMOS] Frame %d: Generate CHSCH+CHBCH+EMOS_SCH, dump to FIFO (last slot %d): time in %d, time out %d [symbols]\n",mac_xface->frame, last_slot, time_in, time_out);
	}
#endif

	/*
	// Write stuff to FIFO
	emos_dump.timestamp = rt_get_time_ns();                             // relative time since boot in nanoseconds
	memcpy(&(emos_dump.PHY_measurements), &(PHY_vars->PHY_measurements), sizeof(PHY_MEASUREMENTS));

	// Unpack the channel estimate
	for (sch_index=0;sch_index<2;sch_index++) {
	  for (aa=0;aa<NB_ANTENNAS_RX;aa++) {
	    for (i=0;i<48;i++)
	      emos_dump.perror[sch_index][aa][i] = PHY_vars->sch_data[sch_index].perror[aa][i];
	    for (i=0;i<NUMBER_OF_OFDM_CARRIERS;i++) {
	      emos_dump.channel_f_unpacked[sch_index][aa][i].r = ((short*) PHY_vars->sch_data[sch_index].channel_f[aa])[i<<2];
	      emos_dump.channel_f_unpacked[sch_index][aa][i].i = ((short*) PHY_vars->sch_data[sch_index].channel_f[aa])[(i<<2)+1];
	    }
	  }
	}

	memcpy(&(emos_dump.chbch_pdu[0]),mrbch_pdu,MRBCH_PDU_SIZE);
	emos_dump.pdu_errors = PHY_vars->mrbch_data[0].pdu_errors;

	emos_dump.offset = PHY_vars->rx_vars[0].offset;
	emos_dump.rx_total_gain_dB = PHY_vars->rx_vars[0].rx_total_gain_dB;

	//msg("[PHY][PROCEDURES][EMOS] writing to FIFO\n");

	// Write things to FIFO
	error = (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump, sizeof(fifo_dump_emos))!=sizeof(fifo_dump_emos));
		
	if (error && (mac_xface->frame%100) == 0)
	  msg("[PHY][PROCEDURES][EMOS] Error writing to FIFO\n");
	*/
 

      }
    else if (last_slot == SLOTS_PER_FRAME - 1) {

      time_in = openair_get_mbox();

      //EMOS channel estimation on the uplink
      //We run the normal channel estimation first to fill the PHY_measurements structure
      //The channel estimate is then overwritten with the better EMOS estimate
#ifdef HW_PREFIX_REMOVAL
      phy_channel_estimation_top(0,40,1,0,NB_ANTENNAS_RX,SCH);
      phy_channel_est_emos(40, 40, 49, 0, TRUE, 1);
#else
      phy_channel_estimation_top(0,40,0,0,NB_ANTENNAS_RX,SCH);
      phy_channel_est_emos(40, 40, 49, 0, TRUE, 0);
#endif //HW_PREFIX_REMOVAL

      	// do channel estimation on slot 2

#ifdef HW_PREFIX_REMOVAL
      phy_channel_estimation_top(0,50,1,1,NB_ANTENNAS_RX,SCH);
      phy_channel_est_emos(50, 50, 59, 1, TRUE, 1);
#else
      phy_channel_estimation_top(0,50,0,1,NB_ANTENNAS_RX,SCH);
      phy_channel_est_emos(50, 50, 59, 1, TRUE, 0);
#endif

    time_out = openair_get_mbox();

#ifdef DEBUG_PHY
    if ((mac_xface->frame % 100) == 0)
      msg("[PHY][PROCEDURES][EMOS] Frame %d: Channel estimation (last slot %d): time in %d, time out %d [symbols]\n",mac_xface->frame, last_slot, time_in, time_out);
#endif
    
    }
  }


#endif //USER_MODE

}



