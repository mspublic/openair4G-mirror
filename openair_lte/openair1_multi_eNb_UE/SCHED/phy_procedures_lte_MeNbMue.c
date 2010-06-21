/*________________________phy_procedures_lte_MeNbMue.c___________________

Authors : Hicham Anouar, Raymond Knopp, Florian Kaltenberger,Torbjorn Sorby
Company : EURECOM
Emails  : anouar@eurecom.fr,  knopp@eurecom.fr, kaltenbe@eurecom.fr, sorby@eurecom.fr
________________________________________________________________*/


// This routine is called periodically by macphy_scheduler to analyse the set of PHY_primitives that were
// Scheduled by MAC and on PHY resources at the appropriate time

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

#ifdef USER_MODE
#ifndef PBS_SIM 
#define DEBUG_PHY
#endif //PBS_SIM
#define NULL_SHAPE_BF_ENABLED
#endif //USER_MODE

//#define DIAG_PHY

//undef DEBUG_PHY and set debug_msg to option 1 to print only most necessary messages every 100 frames. 
//define DEBUG_PHY and set debug_msg to option 2 to print everything all frames
//use msg for something that should be always printed in any case

//#define debug_msg if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10)) msg
#define debug_msg msg



/*
void debug_msg(unsigned char debug_level, char* format_string) { 

  if (debug_level<=DEBUG_LEVEL)
    if (debug_level == 2)
      msg(format_string);
    else if (debug_level == 1) 
      if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10)) 
	msg(format_string);
*/


#define NS_PER_SLOT 500000

extern inline unsigned int taus(void);
extern int exit_openair;

unsigned char dlsch_input_buffer[2700] __attribute__ ((aligned(16)));
unsigned char ulsch_input_buffer[2700] __attribute__ ((aligned(16)));
int eNb_sync_buffer0[640*6] __attribute__ ((aligned(16)));
int eNb_sync_buffer1[640*6] __attribute__ ((aligned(16)));
int *eNb_sync_buffer[2] = {eNb_sync_buffer0, eNb_sync_buffer1};

extern int dlsch_instance_cnt[8];
extern int dlsch_subframe[8];
extern pthread_mutex_t dlsch_mutex[8];
/// Condition variable for dlsch thread
extern pthread_cond_t dlsch_cond[8];


//static char dlsch_ue_active = 0;
//static char dlsch_ue_cntl_active = 0;
//static char dlsch_eNb_active = 0;
//static char dlsch_eNb_cntl_active = 0;


//int dlsch_errors = 0;
int dlsch_cntl_errors = 0;

DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];

#ifdef EMOS
//  fifo_dump_emos_UE emos_dump_UE;
  fifo_dump_emos_eNb emos_dump_eNb;
#endif

#ifdef DIAG_PHY
extern int rx_sig_fifo;
#endif

unsigned char get_ack(unsigned char tdd_config,harq_status_t *harq_ack,unsigned char subframe,unsigned char *o_ACK) {

  switch (tdd_config) {
  case 3:
    if (subframe == 2) {
      o_ACK[0] = harq_ack[5].ack;
      o_ACK[1] = harq_ack[6].ack;
    }
    else if (subframe == 3) {
      o_ACK[0] = harq_ack[7].ack;
      o_ACK[1] = harq_ack[8].ack;
    }
    else if (subframe == 4) {
      o_ACK[0] = harq_ack[9].ack;
      o_ACK[1] = harq_ack[0].ack;
    }
    else {
      msg("phy_procedures_lte.c: get_ack, illegal subframe %d for tdd_config %d\n",
	  subframe,tdd_config);
      return(0);
    }
    break;
    
  }
  return(0);
}

lte_subframe_t subframe_select_tdd(unsigned char tdd_config,unsigned char subframe) {

  switch (tdd_config) {

  case 3:
    if  ((subframe<1) || (subframe>=5)) 
      return(SF_DL);
    else if ((subframe>1) && (subframe < 5))  
      return(SF_UL);
    else if (subframe==1)
      return (SF_S);
    else  {
      msg("[PHY_PROCEDURES_LTE] Unknown subframe number\n");
      return(255);
    }
    break;
  default:
    msg("[PHY_PROCEDURES_LTE] Unsupported TDD mode\n");
    return(255);
    
  }
}

#ifdef EMOS
void phy_procedures_emos_UE_TX(unsigned char next_slot, PHY_VARS_UE *phy_vars_ue) {
  unsigned char harq_pid;

  if (next_slot%2==0) {      
    // get harq_pid from subframe relationship
    harq_pid = subframe2harq_pid_tdd(phy_vars_ue->lte_frame_parms->tdd_config,(next_slot>>1));    
    if (harq_pid==255) {
      msg("ulsch_decoding.c: FATAL ERROR: illegal harq_pid, returning\n");
      return;
    }

    if (phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->subframe_scheduling_flag == 1) {
      emos_dump_UE.uci_cnt[next_slot>>1] = 1;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o,phy_vars_ue->ulsch_ue[0]->o,MAX_CQI_BITS*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O = phy_vars_ue->ulsch_ue[0]->O;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o_RI,phy_vars_ue->ulsch_ue[0]->o_RI,2*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O_RI = phy_vars_ue->ulsch_ue[0]->O_RI;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o_ACK,phy_vars_ue->ulsch_ue[0]->o_ACK,4*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O_ACK = phy_vars_ue->ulsch_ue[0]->O_ACK;
    }
    else {
      emos_dump_UE.uci_cnt[next_slot>>1] = 0;
    }
  }
}

#endif

void phy_procedures_UE_TX(unsigned char next_slot, PHY_VARS_UE *phy_vars_ue) {
  
  unsigned short first_rb, nb_rb;
  unsigned char harq_pid;
  unsigned int input_buffer_length;
  unsigned int i, aa;

#ifdef EMOS
  phy_procedures_emos_UE_TX(next_slot);
#endif

  if (next_slot%2==0) {      
    for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++){
      //  printf("Clearing TX buffer\n");
#ifdef IFFT_FPGA
      memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
	     0,(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#else
      memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
	     0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif
    }
   
    //Primary transmits SRSs on subframe 2 and 3, whilst secondary transmits SRS on subframe 4
    if ((!phy_vars_ue->is_secondary_ue && (((next_slot>>1)==2) || ((next_slot>>1)==3)))
	|| (phy_vars_ue->is_secondary_ue && ((next_slot>>1)==4))) {
#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generating SRS\n",mac_xface->frame,next_slot);
#endif
      generate_srs_tx(&phy_vars_ue->lte_frame_parms,phy_vars_ue->lte_ue_common_vars.txdataF[0],AMP,next_slot>>1);
#ifdef DEBUG_PHY
      write_output("UE_srs_tx.m","srs_tx",&phy_vars_ue->lte_ue_common_vars.txdataF[0][next_slot*phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1) + phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti-1)],phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
#endif //DEBUG_PHY
    }

    // get harq_pid from subframe relationship
    harq_pid = subframe2harq_pid_tdd(phy_vars_ue->lte_frame_parms.tdd_config,(next_slot>>1));
    if (harq_pid==255) {
      msg("ulsch_decoding.c: FATAL ERROR: illegal harq_pid, returning\n");
      return;
    }

    if (phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->subframe_scheduling_flag == 1) {

      // deactivate service request
      phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;

      get_ack(phy_vars_ue->lte_frame_parms.tdd_config,phy_vars_ue->dlsch_ue[0][0]->harq_ack,(next_slot>>1),phy_vars_ue->ulsch_ue[0]->o_ACK);

      first_rb = phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->first_rb;
      nb_rb = phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->nb_rb;
      
      generate_drs_puch(&phy_vars_ue->lte_frame_parms,phy_vars_ue->lte_ue_common_vars.txdataF[0],AMP,next_slot>>1,first_rb,nb_rb);

      input_buffer_length = phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->TBS/8;
      
      for (i=0;i<input_buffer_length;i++) {
	ulsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      }
#ifdef DEBUG_PHY      
      debug_msg("[PHY_PROCEDURES_LTE][UE_UL] phy_vars_ue->ulsch_ue %p : O %d, O_ACK %d, O_RI %d, TBS %d\n",phy_vars_ue->ulsch_ue[0],phy_vars_ue->ulsch_ue[0]->O,phy_vars_ue->ulsch_ue[0]->O_ACK,phy_vars_ue->ulsch_ue[0]->O_RI,phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->TBS);
#endif

      ulsch_encoding(ulsch_input_buffer,&phy_vars_ue->lte_frame_parms,phy_vars_ue->ulsch_ue[0],harq_pid);
      ulsch_modulation(phy_vars_ue->lte_ue_common_vars.txdataF,AMP,(next_slot>>1),&phy_vars_ue->lte_frame_parms,phy_vars_ue->ulsch_ue[0]);

    }
  }
}

void phy_procedures_UE_S_TX(unsigned char next_slot, PHY_VARS_UE *phy_vars_ue) {

  int aa;

  if (next_slot%2==1) {
    for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++){
      //  printf("Clearing TX buffer\n");
#ifdef IFFT_FPGA
      memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
	     0,(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)*sizeof(mod_sym_t));
#else
      memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
	     0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)*sizeof(mod_sym_t));
#endif
    }

#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generating PSS for UL in symbol %d\n",mac_xface->frame,next_slot,PSS_UL_SYMBOL);
#endif
    generate_pss(phy_vars_ue->lte_ue_common_vars.txdataF,
		 AMP,
		 &phy_vars_ue->lte_frame_parms,
		 phy_vars_ue->lte_ue_common_vars.eNb_id,
		 PSS_UL_SYMBOL,
		 next_slot);
  }

}

void lte_ue_measurement_procedures(unsigned char last_slot, unsigned short l, PHY_VARS_UE *phy_vars_ue) {
  
  unsigned char eNb_id,aa;

#ifdef EMOS
  // first slot in frame is special
  if (((last_slot==0) || (last_slot==1)) && ((l==0) || (l==4-phy_vars_ue->lte_frame_parms.Ncp))) {
    for (eNb_id=0; eNb_id<3; eNb_id++) 
      for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++)
	lte_dl_channel_estimation_emos(emos_dump_UE.channel[eNb_id],
				       phy_vars_ue->lte_ue_common_vars.rxdataF,
				       phy_vars_ue->lte_frame_parms,
				       last_slot,
				       aa,
				       l,
				       eNb_id);
  }
#endif

  if (l==0) {
    // UE measurements 
    
    lte_ue_measurements(phy_vars_ue,
			&phy_vars_ue->lte_frame_parms,
			(last_slot>>1)*phy_vars_ue->lte_frame_parms.symbols_per_tti*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,
			(last_slot == 2) ? 1 : 2,
			1);
    
    if (last_slot%2==0) {
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, freq_offset_filt = %d \n",mac_xface->frame, last_slot, phy_vars_ue->lte_ue_common_vars.freq_offset);
      
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), RX gain %d dB\n",
	  mac_xface->frame, last_slot,
	  phy_vars_ue->PHY_measurements.rx_rssi_dBm[0] - ((phy_vars_ue->lte_frame_parms.nb_antennas_rx==2) ? 3 : 0), 
	  phy_vars_ue->PHY_measurements.wideband_cqi_dB[0][0],
	  phy_vars_ue->PHY_measurements.wideband_cqi_dB[0][1],
	  phy_vars_ue->PHY_measurements.wideband_cqi[0][0],
	  phy_vars_ue->PHY_measurements.wideband_cqi[0][1],
	  phy_vars_ue->rx_total_gain_dB);
      
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, N0 %d dBm digital (%d, %d) dB, linear (%d, %d)\n",
	  mac_xface->frame, last_slot,
	  dB_fixed(phy_vars_ue->PHY_measurements.n0_power_tot/phy_vars_ue->lte_frame_parms.nb_antennas_rx) - (int)phy_vars_ue->rx_total_gain_dB,
	  phy_vars_ue->PHY_measurements.n0_power_dB[0],
	  phy_vars_ue->PHY_measurements.n0_power_dB[1],
	  phy_vars_ue->PHY_measurements.n0_power[0],
	  phy_vars_ue->PHY_measurements.n0_power[1]);
#endif //DEBUG_PHY
    }

  }
  

  
  if ((last_slot==1) && (l==(4-phy_vars_ue->lte_frame_parms.Ncp))) {
    
    // AGC
    if (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON)
      //      if (mac_xface->frame % 10 == 0)
      phy_adjust_gain (0,512,0,phy_vars_ue);
    
    eNb_id = 0;
    lte_adjust_synch(&phy_vars_ue->lte_frame_parms,
		     phy_vars_ue,
		     eNb_id,
		     1,
		     16384);
    // write frequency offset to pci interface
  }
}

#ifdef EMOS
void phy_procedures_emos_UE_RX(unsigned char last_slot, PHY_VARS_UE *phy_vars_ue) {

  unsigned char eNb_id,i;
  memcpy(&emos_dump_UE.PHY_measurements[last_slot],&phy_vars_ue->PHY_measurements,sizeof(PHY_MEASUREMENTS));
  if (last_slot==0) {
      emos_dump_UE.timestamp = rt_get_time_ns();
      emos_dump_UE.frame_rx = mac_xface->frame;
      emos_dump_UE.freq_offset = phy_vars_ue->lte_ue_common_vars.freq_offset;
      emos_dump_UE.timing_advance = openair_daq_vars.timing_advance;
      emos_dump_UE.timing_offset  = PHY_vars->rx_vars[0].offset;
      emos_dump_UE.rx_total_gain_dB = phy_vars_ue->rx_total_gain_dB;
      emos_dump_UE.eNb_id = phy_vars_ue->lte_ue_common_vars.eNb_id;
  }
  if (last_slot==1) {
    for (eNb_id = 0; eNb_id<3; eNb_id++) { 
      memcpy(emos_dump_UE.pbch_pdu[eNb_id],phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output,PBCH_PDU_SIZE);
      emos_dump_UE.pdu_errors[eNb_id] = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors;
      emos_dump_UE.pdu_errors_last[eNb_id] = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_last;
      emos_dump_UE.pdu_errors_conseq[eNb_id] = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq;
      emos_dump_UE.pdu_fer[eNb_id] = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_fer;
      emos_dump_UE.dci_errors = phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors;
      emos_dump_UE.dci_received = phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_received;
    }
  }
  if (last_slot%2==0) {
    for (i=0; i<2; i++) 
      memcpy(&emos_dump_UE.DCI_alloc[i][last_slot>>1], &dci_alloc_rx[i], sizeof(DCI_ALLOC_t));
    }
  if (last_slot==19) {
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Writing EMOS data to FIFO\n",mac_xface->frame, last_slot);
#endif //DEBUG_PHY
    if (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump_UE, sizeof(fifo_dump_emos_UE))!=sizeof(fifo_dump_emos_UE)) {
      msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Problem writing EMOS data to FIFO\n",mac_xface->frame, last_slot);
      return;
    }
  }
}


#endif

void lte_ue_pbch_procedures(int eNb_id,unsigned char last_slot, PHY_VARS_UE *phy_vars_ue) {

  int pbch_error;

  pbch_error = rx_pbch(&phy_vars_ue->lte_ue_common_vars,
		       phy_vars_ue->lte_ue_pbch_vars[eNb_id],
		       &phy_vars_ue->lte_frame_parms,
		       eNb_id,
		       SISO);
  if (pbch_error) {
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq = 0;
#ifdef EMOS
    emos_dump_UE.frame_tx = *((unsigned int*) phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output);
    //emos_dump_UE.mimo_mode = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output[4];
    //phy_vars_eNb->PHY_measurements.frame_tx = *((unsigned int*) phy_vars_ue->lte_ue_pbch_vars->decoded_output);
#endif
  }
  else {
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq++;
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors++;
  }
  
  if (mac_xface->frame % 100 == 0) {
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_fer = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors - phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_last;
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_last = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors;
  }
  

#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH errors = %d, consecutive errors = %d!\n",
	mac_xface->frame, last_slot, phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors, phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq);	
    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH received frame = %d!\n",
	mac_xface->frame, last_slot,*((unsigned int*) phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output));	
#endif //DEBUG_PHY


  if (phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq>20 && !phy_vars_ue->is_secondary_ue) {
    msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH consecutive errors > 20, going out of sync!\n",mac_xface->frame, last_slot);
    openair_daq_vars.mode = openair_NOT_SYNCHED;
    openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
    //rt_sleep(nano2count(NS_PER_SLOT*SLOTS_PER_FRAME));
#endif //CBMIMO1
    mac_xface->frame = -1;
    openair_daq_vars.synch_wait_cnt=0;
    openair_daq_vars.sched_cnt=-1;
    
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq=0;
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors=0;

    phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors = 0;    
    phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_received = 0;    
  }
}


void lte_ue_pdcch_procedures(int eNb_id,unsigned char last_slot, PHY_VARS_UE *phy_vars_ue) {	

  unsigned int dci_cnt, i, length;
  
#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): DCI decoding\n",mac_xface->frame,last_slot,last_slot>>1);
#endif
  
#ifdef DEBUG_PHY
  //  write_output("UE_rxsigF0.m","UE_rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],512*12*2,2,1);
  //  write_output("UE_rxsigF1.m","UE_rxsF1", phy_vars_ue->lte_ue_common_vars.rxdataF[1],512*12*2,2,1);
#endif //DEBUG_PHY
  
  rx_pdcch(&phy_vars_ue->lte_ue_common_vars,
	   phy_vars_ue->lte_ue_pdcch_vars,
	   &phy_vars_ue->lte_frame_parms,
	   eNb_id,
	   2,
	   (phy_vars_ue->lte_frame_parms.nb_antennas_tx == 1) ? SISO : ALAMOUTI, //this needs to be changed
	   phy_vars_ue->is_secondary_ue);
  
  dci_cnt = dci_decoding_procedure(phy_vars_ue->lte_ue_pdcch_vars,dci_alloc_rx,eNb_id,&phy_vars_ue->lte_frame_parms,SI_RNTI,RA_RNTI,
	    phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti);

  phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_received += dci_cnt;
#ifdef DIAG_PHY
  switch (last_slot>>1) {
  case 0:
  case 5:
  case 8:
    if (dci_cnt != 1) {
      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors ++;
      msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: missed DCI (dci_cnt %d)!\n",mac_xface->frame,last_slot>>1,dci_cnt);
      msg("[PHY_PROCEDURES_UE][DIAG] frame %d, slot %d, RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), RX gain %d dB\n",
	  mac_xface->frame, last_slot,
	  phy_vars_ue->PHY_measurements.rx_rssi_dBm[0] - ((phy_vars_ue->lte_frame_parms.nb_antennas_rx==2) ? 3 : 0), 
	  phy_vars_ue->PHY_measurements.wideband_cqi_dB[0][0],
	  phy_vars_ue->PHY_measurements.wideband_cqi_dB[0][1],
	  phy_vars_ue->PHY_measurements.wideband_cqi[0][0],
	  phy_vars_ue->PHY_measurements.wideband_cqi[0][1],
	  phy_vars_ue->rx_total_gain_dB);
    }
    break;
  default:
    break;
  }
  if (last_slot==18)	
#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE][DIAG] Frame %d, slot %d: PDCCH: DCI errors %d, DCI received %d \n",mac_xface->frame,last_slot,phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors,phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_received);	
#endif //DEBUG_PHY

  /*
  if (((last_slot>>1)==8) && (dci_cnt!=1) && (openair_daq_vars.one_shot_get_frame == 1)) {
    rtf_reset(rx_sig_fifo);
    for (i=0;i<NB_ANTENNAS_RX;i++) {
      length=rtf_put(rx_sig_fifo,PHY_vars->rx_vars[i].RX_DMA_BUFFER,FRAME_LENGTH_BYTES);
      if (length < FRAME_LENGTH_BYTES)
	msg("[PHY_PROCEDURES_LTE][DIAG] Didn't put %d bytes for antenna %d (put %d)\n",FRAME_LENGTH_BYTES,i,length);
      else
	msg("[PHY_PROCEDURES_LTE][DIAG] Worte %d bytes for antenna %d to fifo (put %d)\n",FRAME_LENGTH_BYTES,i,length);
    }    
    openair_daq_vars.one_shot_get_frame = 0;
  }
  x*/

#endif

#ifdef EMOS
  emos_dump_UE.dci_cnt[last_slot>>1] = dci_cnt;
#endif

#ifdef DIAG_PHY
  if (dci_cnt > 2) {
    msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: received %d>2 DCI!\n",mac_xface->frame,last_slot>>1,dci_cnt);
    //exit_openair=1;
    return;
  }
#endif

#ifdef DEBUG_PHY
  debug_msg("[PHY PROCEDURES UE] subframe %d: dci_cnt %d\n",last_slot>>1,dci_cnt);
#endif
  for (i=0;i<dci_cnt;i++){
#ifdef DEBUG_PHY
      msg("[PHY PROCEDURES UE] subframe %d: Found rnti %x, format %d\n",last_slot>>1,dci_alloc_rx[i].rnti,
	  dci_alloc_rx[i].format);
#endif
    if ((dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti) && (dci_alloc_rx[i].format == format2_2A_M10PRB)) {

#ifdef DIAG_PHY
      if ((last_slot>>1) != 5) {
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received C_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	//exit_openair=1;
	return;
      }
#endif

      if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					    (DCI2_5MHz_2A_M10PRB_TDD_t *)&dci_alloc_rx[i].dci_pdu,
					    phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti,
					    format2_2A_M10PRB,
					    phy_vars_ue->dlsch_ue[0],
					    &phy_vars_ue->lte_frame_parms,
					    SI_RNTI,
					    RA_RNTI,
					    P_RNTI)==0) {
	phy_vars_ue->dlsch_ue_active = 1;
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Generated UE DLSCH C_RNTI format 2_2A_M10PRB\n");
#endif    
      }
    }
    else if ((dci_alloc_rx[i].rnti == SI_RNTI) && (dci_alloc_rx[i].format == format1A)) {
#ifdef DIAG_PHY
      if ((last_slot>>1) != 0) {
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received SI_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	//exit_openair=1;
	return;
      }
#endif
      if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					    (DCI1A_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					    SI_RNTI,
					    format1A,
					    &phy_vars_ue->dlsch_ue_cntl, 
					    &phy_vars_ue->lte_frame_parms,
					    SI_RNTI,
					    RA_RNTI,
					    P_RNTI)==0) {
	phy_vars_ue->dlsch_ue_cntl_active = 1;
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Generate UE DLSCH SI_RNTI format 1A\n");
#endif
      }
    }
    else if ((dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti) && (dci_alloc_rx[i].format == format0)) {
#ifdef DIAG_PHY
      if ((last_slot>>1) != 8) {
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received C_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	//exit_openair=1;
	return;
      }
#endif
      generate_ue_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti,
					last_slot>>1,
					format0,
					phy_vars_ue->ulsch_ue[eNb_id],
					&phy_vars_ue->PHY_measurements,
					&phy_vars_ue->lte_frame_parms,
					SI_RNTI,
					RA_RNTI,
					P_RNTI,
					eNb_id);
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Generate UE ULSCH C_RNTI format 0 (subframe %d)\n",last_slot>>1);
#endif
    }
    else if ((dci_alloc_rx[i].rnti == RA_RNTI) && (dci_alloc_rx[i].format == format0)) {
#ifdef DIAG_PHY
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received RA_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	//exit_openair=1;
	return;
#endif
      /*
	generate_ue_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
	RA_RNTI,
	last_slot>>1,
	format0,
	phy_vars_ue->ulsch_ue[eNb_id], 
	phy_vars_ue->lte_frame_parms,
	SI_RNTI,
	RA_RNTI,
	P_RNTI);

	printf("[PHY_PROCEDURES_LTE] Generate UE ULSCH C_RNTI format 0 (subframe %d)\n",last_slot>>1);
      */
    }
    else {
      msg("[PHY PROCEDURES UE] Frame %d, subframe %d: received DCI with RNTI=%x and format %d!\n",mac_xface->frame,last_slot>>1,dci_alloc_rx[i].rnti,dci_alloc_rx[i].format);
#ifdef DIAG_PHY
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	//exit_openair=1;
	return;
#endif
    }

  }
}


int phy_procedures_UE_RX(unsigned char last_slot, PHY_VARS_UE *phy_vars_ue) {

  unsigned short l,m,n_symb,aarx,aatx;
  int eNb_id = 0, eNb_id_i = 1;
  unsigned char dual_stream_UE = 0;
  int ret;
  unsigned char harq_pid;

  if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,last_slot>>1) == SF_S) 
    if ((last_slot%2)==0)
      n_symb = 3;
    else
      n_symb = 0;
  else
    n_symb = phy_vars_ue->lte_frame_parms.symbols_per_tti/2;

  // RX processing of symbols in last_slot
  for (l=0;l<n_symb;l++) {
    slot_fep(&phy_vars_ue->lte_frame_parms,
	     &phy_vars_ue->lte_ue_common_vars,
	     l,
	     last_slot,
	     0,
#ifdef USER_MODE
	     0
#else
             1
#endif
	     );

    lte_ue_measurement_procedures(last_slot,l,phy_vars_ue);

    if ((last_slot==1) && (l==4-phy_vars_ue->lte_frame_parms.Ncp)) {
	
      lte_ue_pbch_procedures(eNb_id,last_slot,phy_vars_ue);
    }

	
    // process last DLSCH symbols + invoke decoding
    if (((last_slot%2)==0) && (l==0)) {

      if ( (phy_vars_ue->dlsch_ue_active == 1) && (phy_vars_ue->dlsch_ue_cntl_active == 1))
	msg("[PHY_PROCEDURES_LTE] WARNING: phy_vars_ue->dlsch_ue and phy_vars_ue->dlsch_ue_cntl active, but data structures can only handle one at a time\n");

      if (phy_vars_ue->dlsch_ue_active == 1) {
#ifdef DEBUG_PHY
	  debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 10,11,12\n",mac_xface->frame,last_slot);
#endif
      
	// process symbols 10,11,12 and trigger DLSCH decoding
	for (m=(11-phy_vars_ue->lte_frame_parms.Ncp*2+1);m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   phy_vars_ue->dlsch_ue[0],
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);

#ifdef DEBUG_PHY
	if (phy_vars_ue->is_secondary_ue) {
	  write_output("dlsch_llr.m","llr",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->llr[0],(8*((3*8*6144)+12)),1,0); }
	else {
	  write_output("dlsch_llr_prim.m","llr_p",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->llr[0],(8*((3*8*6144)+12)),1,0);
	  write_output("dlsch_comp_prim.m","comp_p",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->rxdataF_comp[0],phy_vars_ue->lte_frame_parms.N_RB_DL*(12)*phy_vars_ue->lte_frame_parms.symbols_per_tti,1,1);
	  write_output("dlsch_ext_prim.m","ext_p",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->rxdataF_ext[0],phy_vars_ue->lte_frame_parms.N_RB_DL*(12)*phy_vars_ue->lte_frame_parms.symbols_per_tti,1,1);
	  write_output("dlsch_est_ext_prim.m","est_ext_p",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[0],phy_vars_ue->lte_frame_parms.N_RB_DL*(12),1,1);
	}
#endif //DEBUG_PHY

	phy_vars_ue->dlsch_ue_active = 0;
      
#ifndef USER_MODE	
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Scheduling DLSCH decoding\n",mac_xface->frame,last_slot);	
#endif //DEBUG_PHY

	harq_pid = phy_vars_ue->dlsch_ue[0][0]->current_harq_pid;
	if (harq_pid != 0) {
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: No DLSCH decoding thread for harq_pid = %d\n",mac_xface->frame,last_slot,harq_pid);
	  return(-1);
	}
	
	if (pthread_mutex_lock (&dlsch_mutex[harq_pid]) != 0) {               // Signal MAC_PHY Scheduler
	  msg("[PHY_PROCEDURES_LTE] ERROR pthread_mutex_lock\n");     // lock before accessing shared resource
	  return(-1);
	}
	dlsch_instance_cnt[harq_pid]++;
	dlsch_subframe[harq_pid] = ((last_slot>>1)-1)%10;
	pthread_mutex_unlock (&dlsch_mutex[harq_pid]);
	
	if (dlsch_instance_cnt[harq_pid] == 0) {
	  if (pthread_cond_signal(&dlsch_cond[harq_pid]) != 0) {
	    msg("[PHY_PROCEDURES_LTE] ERROR pthread_cond_signal for dlsch_cond[%d]\n",harq_pid);
	    return(-1);
	  }
	}
	else {
	  msg("[PHY_PROCEDURES_LTE] DLSCH thread for harq_pid %d busy!!!\n",harq_pid);
	  return(-1);
	}
	
#else
	if (mac_xface->frame < phy_vars_ue->dlsch_errors)
	  phy_vars_ue->dlsch_errors=0;
	
	if (phy_vars_ue->dlsch_ue[0][0]) {
	  ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->llr[0],
			       &phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->dlsch_ue[0][0],
			       ((last_slot>>1)-1)%10);
	  phy_vars_ue->turbo_iterations += ret;
	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    phy_vars_ue->dlsch_errors++;
	  }
	}   
	
	#ifdef DEBUG_PHY
	if (mac_xface->frame%20==0)
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_decoding ret %d (%d errors) - isSecondary=%d\n",
		  mac_xface->frame,last_slot,ret,phy_vars_ue->dlsch_errors,phy_vars_ue->is_secondary_ue);
	#endif //DEBUG_PHY
#endif //USER_MODE
      }
      if (phy_vars_ue->dlsch_ue_cntl_active == 1) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (cntl) demod symbols 10,11,12\n",mac_xface->frame,last_slot);
#endif
      
	// process symbols 10,11,12 and trigger DLSCH decoding
	for (m=(11-phy_vars_ue->lte_frame_parms.Ncp*2+1);m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_cntl,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_cntl,
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
	
#ifdef DEBUG_PHY
	//write_output("dlsch_cntl_llr.m","llr",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->llr[0],40,1,0);
#endif //DEBUG_PHY

	phy_vars_ue->dlsch_ue_cntl_active = 0;
      
	if (mac_xface->frame < phy_vars_ue->dlsch_cntl_errors)
	  phy_vars_ue->dlsch_cntl_errors=0;
	
	if (phy_vars_ue->dlsch_ue_cntl) {
	  ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars_cntl[eNb_id]->llr[0],
			       &phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->dlsch_ue_cntl,
			       ((last_slot>>1)-1)%10);
	  phy_vars_ue->turbo_cntl_iterations += ret;	  
	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    phy_vars_ue->dlsch_cntl_errors++;
	  }
	}   
	
#ifdef DEBUG_PHY
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_decoding (cntl) ret %d (%d errors)\n",
	      mac_xface->frame,last_slot,ret,dlsch_cntl_errors);
#endif //DEBUG_PHY
	
      }

    }
      
    if (((last_slot%2)==0) && (l==(4-phy_vars_ue->lte_frame_parms.Ncp)))  {
	
      lte_ue_pdcch_procedures(eNb_id,last_slot,phy_vars_ue);
	
      if (phy_vars_ue->dlsch_ue_active == 1) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 0,1,2\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 0,1,2
	for (m=phy_vars_ue->lte_frame_parms.first_dlsch_symbol;m<(4-phy_vars_ue->lte_frame_parms.Ncp);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   phy_vars_ue->dlsch_ue[0],
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
      }
      if (phy_vars_ue->dlsch_ue_cntl_active == 1) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (cntl) demod symbols 0,1,2\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 0,1,2
	for (m=phy_vars_ue->lte_frame_parms.first_dlsch_symbol;m<(4-phy_vars_ue->lte_frame_parms.Ncp);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_cntl,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_cntl,
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
      }
    }
      
    if (((last_slot%2)==1) && (l==0)) {
	
      if (phy_vars_ue->dlsch_ue_active == 1)  {
#ifdef DEBUG_PHY
	  debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 3,4,5\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 3,4,5
	for (m=4-phy_vars_ue->lte_frame_parms.Ncp+1;m<(phy_vars_ue->lte_frame_parms.symbols_per_tti/2);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   phy_vars_ue->dlsch_ue[0],
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
      }
      if (phy_vars_ue->dlsch_ue_cntl_active == 1)  {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (cntl) demod symbols 3,4,5\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 3,4,5
	for (m=4-phy_vars_ue->lte_frame_parms.Ncp+1;m<(phy_vars_ue->lte_frame_parms.symbols_per_tti/2);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_cntl,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_cntl,
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
      }
    }
      
    if (((last_slot%2)==1) && (l==(4-phy_vars_ue->lte_frame_parms.Ncp))) {
	
      if(phy_vars_ue->dlsch_ue_active == 1) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 6,7,8\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 6,7,8
	for (m=(phy_vars_ue->lte_frame_parms.symbols_per_tti/2)+1;m<(11-phy_vars_ue->lte_frame_parms.Ncp*2);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   phy_vars_ue->dlsch_ue[0],
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
      }
      if(phy_vars_ue->dlsch_ue_cntl_active == 1) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (cntl) demod symbols 6,7,8\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 6,7,8
	for (m=(phy_vars_ue->lte_frame_parms.symbols_per_tti/2)+1;m<(11-phy_vars_ue->lte_frame_parms.Ncp*2);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_cntl,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_cntl,
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
      }
    }

#ifdef DEBUG_PHY
    if (last_slot==12 && l==0 && !phy_vars_ue->is_secondary_ue) {
      write_output("UE0_rxsigF0.m","UE0_rxsF0",phy_vars_ue->lte_ue_common_vars.rxdataF[0],phy_vars_ue->lte_frame_parms.ofdm_symbol_size*phy_vars_ue->lte_frame_parms.symbols_per_tti,1,1);
      write_output("UE0_dl_ch_est.m","dl_ce",phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[eNb_id][0],phy_vars_ue->lte_frame_parms.ofdm_symbol_size*phy_vars_ue->lte_frame_parms.symbols_per_tti,1,1);
      write_output("UE0_dl_ch_est_ext.m","dl_ce_ext",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[0],phy_vars_ue->lte_frame_parms.N_RB_DL*(12)*phy_vars_ue->lte_frame_parms.symbols_per_tti,1,1);
      write_output("UE0_compensated.m","UE0_comp",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->rxdataF_comp[0],phy_vars_ue->lte_frame_parms.N_RB_DL*(12)*phy_vars_ue->lte_frame_parms.symbols_per_tti,1,1);
      write_output("UE0_extracted.m","UE0_ext",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->rxdataF_ext[0],phy_vars_ue->lte_frame_parms.N_RB_DL*(12)*phy_vars_ue->lte_frame_parms.symbols_per_tti,1,1);
      write_output("UE0_llr_tl0.m","UE0_llr",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->llr[0],(300*2)*6,1,0);
    }
#endif //DEBUG_PHY
  }

#ifdef EMOS
  phy_procedures_emos_UE_RX(last_slot);
#endif

  return (0);
}

void phy_procedures_eNB_S_TX(unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNb) {

  int eNb_id = 0, aa;

  if (next_slot%2==0) {
#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generating pilots for DL-S\n",mac_xface->frame,next_slot);
#endif
    
    for (aa=0; aa<phy_vars_eNb->lte_frame_parms.nb_antennas_tx; aa++) {
      /*
#ifdef DEBUG_PHY
      printf("Clearing TX buffer %d at %p, length %d \n",aa,
	     &lte_eNB_common_vars->txdataF[eNb_id][aa][next_slot*(lte_frame_parms->N_RB_DL*12)*(lte_frame_parms->symbols_per_tti>>1)],
	     (lte_frame_parms->N_RB_DL*12)*(lte_frame_parms->symbols_per_tti>>1)*sizeof(mod_sym_t));
#endif
      */
#ifdef IFFT_FPGA
      memset(&(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)]),
	     0,(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)*sizeof(mod_sym_t));
#else
      memset(&(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)]),
	     0,phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)*sizeof(mod_sym_t));
#endif
    }

    generate_pilots_slot(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
			 AMP,
			 &phy_vars_eNb->lte_frame_parms,
			 eNb_id,
			 next_slot);

    generate_pss(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
		 AMP,
		 &phy_vars_eNb->lte_frame_parms,
		 eNb_id,
		 2,
		 next_slot);

  }
}

void phy_procedures_eNB_S_RX(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNb) {

  int aa,l,sync_pos,sync_pos_slot;
  unsigned char eNb_id = 0;
  int time_in, time_out;

  if (last_slot%2==1) {
#ifndef USER_MODE
    time_in = openair_get_mbox();
#endif

    // look for PSS in the last 3 symbols of the last slot
    // but before we need to zero pad the gaps that the HW removed
    bzero(eNb_sync_buffer[0],640*6*sizeof(int));
    bzero(eNb_sync_buffer[1],640*6*sizeof(int));

    //    if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10))
    //      msg("[PHY_PROCEDURES_LTE][eNb_UL] Zero padding data for lte_sync_time (%p, %p)\n",eNb_sync_buffer[0],eNb_sync_buffer[1]);

    for (aa=0; aa<phy_vars_eNb->lte_frame_parms.nb_antennas_rx; aa++) {
      for (l=PSS_UL_SYMBOL; l<phy_vars_eNb->lte_frame_parms.symbols_per_tti/2; l++) {
	memcpy(&eNb_sync_buffer[aa][(l-PSS_UL_SYMBOL)*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size+phy_vars_eNb->lte_frame_parms.nb_prefix_samples)+phy_vars_eNb->lte_frame_parms.nb_prefix_samples], 
	       &phy_vars_eNb->lte_eNB_common_vars.rxdata[eNb_id][aa][(last_slot*phy_vars_eNb->lte_frame_parms.symbols_per_tti/2+l)*
#ifdef USER_MODE
							(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size+phy_vars_eNb->lte_frame_parms.nb_prefix_samples)
#else
							phy_vars_eNb->lte_frame_parms.ofdm_symbol_size
#endif
							],
	       phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*sizeof(int));
      }
    }
    sync_pos_slot = phy_vars_eNb->lte_frame_parms.nb_prefix_samples; //this is where the sync pos should be wrt eNb_sync_buffer

    //    if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10))
    //      msg("[PHY_PROCEDURES_LTE][eNb_UL] Entering lte_sync_time\n");

    sync_pos = lte_sync_time_eNb(eNb_sync_buffer, &phy_vars_eNb->lte_frame_parms, eNb_id, phy_vars_eNb->lte_frame_parms.samples_per_tti/2 - PSS_UL_SYMBOL);

#ifndef USER_MODE
    time_out = openair_get_mbox();
#endif

#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d: Peak found at pos %d, offset %d (time_in %d, time_out %d)\n",mac_xface->frame, last_slot, sync_pos, sync_pos - sync_pos_slot, time_in, time_out);
#endif
  }
}


#ifdef EMOS

void phy_procedures_emos_eNB_RX(unsigned char last_slot) {

  unsigned char eNb_id,i;
  for (eNb_id = 0; eNb_id<3; eNb_id++)  
    memcpy(&emos_dump_eNb.eNB_UE_stats[eNb_id][last_slot],&phy_vars_eNb->eNB_UE_stats,sizeof(LTE_eNB_UE_stats));

  if (last_slot==4) {
      emos_dump_UE.timestamp = rt_get_time_ns();
      emos_dump_UE.frame_tx = mac_xface->frame;
      emos_dump_UE.rx_total_gain_dB = phy_vars_ue->rx_total_gain_dB;
  }
  if (last_slot%2==0) {
    for (i=0; i<2; i++) 
      memcpy(&emos_dump_eNb.DCI_alloc[i][last_slot>>1], &dci_alloc[i], sizeof(DCI_ALLOC_t));
    }
  if (last_slot==9) {
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Writing EMOS data to FIFO\n",mac_xface->frame, last_slot);
    if (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump_eNb, sizeof(fifo_dump_emos_eNb))!=sizeof(fifo_dump_emos_eNb)) {
      msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Problem writing EMOS data to FIFO\n",mac_xface->frame, last_slot);
      return;
    }
  }
}

#endif


void phy_procedures_eNB_TX(unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNb) {

  unsigned char pbch_pdu[PBCH_PDU_SIZE];
  unsigned int nb_dci_ue_spec = 0, nb_dci_common = 0;
  unsigned short input_buffer_length, re_allocated;
  int eNb_id = 0,UE_id = 0,i,aa;
  unsigned char harq_pid;


  if (next_slot%2 == 0) {
    for (aa=0; aa<phy_vars_eNb->lte_frame_parms.nb_antennas_tx;aa++) {
      /*
#ifdef DEBUG_PHY
      printf("Clearing TX buffer %d at %p, length %d \n",aa,
	     &lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*(lte_frame_parms.N_RB_DL*12)*(lte_frame_parms.symbols_per_tti>>1)],
	     (lte_frame_parms.N_RB_DL*12)*(lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif
      */
#ifdef IFFT_FPGA
      memset(&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
	     0,(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#else
      memset(&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
	     0,phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif
    }
  }

  generate_pilots_slot(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
		       AMP,
		       &phy_vars_eNb->lte_frame_parms,
		       eNb_id,
		       next_slot);


  /*
  if (next_slot == 0) {
    generate_pss(lte_eNB_common_vars.txdataF[eNb_id],
		 AMP,
		 lte_frame_parms,
		 eNb_id,
		 6-lte_frame_parms.Ncp,
		 next_slot);
  }
  */

  if (next_slot == 1) {
    
#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_pbch\n",mac_xface->frame, next_slot);
#endif
    
    *((unsigned int*) pbch_pdu) = mac_xface->frame;
    
    generate_pbch(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
		  AMP,
		  &phy_vars_eNb->lte_frame_parms,
		  pbch_pdu);
  }
  
  // DCI generation
  if ((next_slot%2 == 0)) { 
    
    
    // Get DCI parameters from MAC
    switch (next_slot>>1) {
    case 0:
      if (1) { //!phy_vars_eNb->is_secondary_eNb) {
	memcpy(&dci_alloc[0].dci_pdu[0],&CCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
	dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
	dci_alloc[0].L          = 3;
	dci_alloc[0].rnti       = SI_RNTI;
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generated CCCH DCI, format 1A\n",mac_xface->frame, next_slot);
#endif
	
	/*
	  memcpy(&dci_alloc[1].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
	  dci_alloc[1].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
	  dci_alloc[1].L          = 3;
	  dci_alloc[1].rnti       = phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti;
	  #ifdef DEBUG_PHY
	  debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): Generated ULSCH DCI, format 0\n",mac_xface->frame,next_slot,next_slot>>1);
	  #endif
	*/
	
	nb_dci_ue_spec = 0;
	nb_dci_common  = 1;
	
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generated CCCH DCI, format 1A\n",mac_xface->frame, next_slot);
#endif
	
	generate_eNb_dlsch_params_from_dci(next_slot>>1,
					   &CCCH_alloc_pdu,
					   SI_RNTI,
					   format1A,
					   &phy_vars_eNb->dlsch_eNb_cntl,
					   &phy_vars_eNb->lte_frame_parms,
					   SI_RNTI,
					   RA_RNTI,
					   P_RNTI);
	phy_vars_eNb->dlsch_eNb_cntl_active = 1;
	
	// Schedule UL subframe
	
	generate_eNb_ulsch_params_from_dci(&UL_alloc_pdu,
					   phy_vars_eNb->eNB_UE_stats[eNb_id].UE_id[UE_id],
					   (next_slot>>1),
					   format0,
					   phy_vars_eNb->ulsch_eNb[0],
					   &phy_vars_eNb->lte_frame_parms,
					   SI_RNTI,
					   RA_RNTI,
					   P_RNTI);
	// get UL harq_pid for subframe+4
	harq_pid = subframe2harq_pid_tdd(3,((next_slot>>1)+4)%10);
#ifdef DEBUG_pHY
	debug_msg("[PHY PROCEDURES eNB] frame %d, subframe %d Setting scheduling flag for ULSCH harq_pid %d\n",
		  mac_xface->frame,next_slot>>1,harq_pid);
#endif
	phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
	
      }
      break;
    case 1:
      nb_dci_ue_spec = 0;
      nb_dci_common  = 0;
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      if (1){//!phy_vars_eNb->is_secondary_eNb) {
	memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
	dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
	dci_alloc[0].L          = 3;
	dci_alloc[0].rnti       = phy_vars_eNb->eNB_UE_stats[eNb_id].UE_id[UE_id];
	nb_dci_common  = 0;
	nb_dci_ue_spec = 1;
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generated DLSCH DCI, format 2_2A_M10PRB\n",mac_xface->frame, next_slot);
#endif
	
	generate_eNb_dlsch_params_from_dci(next_slot>>1,
					   &DLSCH_alloc_pdu2,
					   phy_vars_eNb->eNB_UE_stats[eNb_id].UE_id[UE_id],
					   format2_2A_M10PRB,
					   &phy_vars_eNb->dlsch_eNb[0][0],
					   &phy_vars_eNb->lte_frame_parms,
					   SI_RNTI,
					   RA_RNTI,
					   P_RNTI);
	phy_vars_eNb->dlsch_eNb_active = 1;
      } else {
	nb_dci_ue_spec = 0;
	nb_dci_common  = 0;
	phy_vars_eNb->dlsch_eNb_active = 0;
      }
      break;
 
    case 6:
      if (0){//phy_vars_eNb->is_secondary_eNb) {
	memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
	dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
	dci_alloc[0].L          = 3;
	dci_alloc[0].rnti       = phy_vars_eNb->eNB_UE_stats[eNb_id].UE_id[UE_id];
	nb_dci_common  = 0;
	nb_dci_ue_spec = 1;
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generated DLSCH DCI, format 2_2A_M10PRB\n",mac_xface->frame, next_slot);
#endif
	
	generate_eNb_dlsch_params_from_dci(next_slot>>1,
					   &DLSCH_alloc_pdu2,
					   phy_vars_eNb->eNB_UE_stats[eNb_id].UE_id[UE_id],
					   format2_2A_M10PRB,
					   &phy_vars_eNb->dlsch_eNb[0][0],
					   &phy_vars_eNb->lte_frame_parms,
					   SI_RNTI,
					   RA_RNTI,
					   P_RNTI);
	phy_vars_eNb->dlsch_eNb_active = 1;
      } else {
	nb_dci_ue_spec = 0;
	nb_dci_common  = 0;
	phy_vars_eNb->dlsch_eNb_active = 0;
      }
      break;
    case 7:
      nb_dci_ue_spec = 0;
      nb_dci_common  = 0;
      phy_vars_eNb->dlsch_eNb_active = 0;
      break;
    case 8:

      // Schedule UL subframe
      // get UL harq_pid for subframe+4
      harq_pid = subframe2harq_pid_tdd(3,((next_slot>>1)+4)%10);
      phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;
      nb_dci_ue_spec = 0;
      nb_dci_common  = 0;
      phy_vars_eNb->dlsch_eNb_active = 0;

      //if ((mac_xface->frame&1)==0) {
      if (1){//!phy_vars_eNb->is_secondary_eNb) {
      memcpy(&dci_alloc[0].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
      dci_alloc[0].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = phy_vars_eNb->eNB_UE_stats[eNb_id].UE_id[UE_id];
      nb_dci_ue_spec = 1;
      nb_dci_common  = 0;
      phy_vars_eNb->dlsch_eNb_active = 0;

#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): Generated ULSCH DCI, format 0\n",mac_xface->frame,next_slot,next_slot>>1);
#endif

      generate_eNb_ulsch_params_from_dci(&UL_alloc_pdu,
					 phy_vars_eNb->eNB_UE_stats[eNb_id].UE_id[UE_id],
					 (next_slot>>1),
					 format0,
					 phy_vars_eNb->ulsch_eNb[0],
					 &phy_vars_eNb->lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);

#ifdef DEBUG_PHY
      debug_msg("[PHY PROCEDURES eNB] frame %d, subframe %d Setting scheduling flag for ULSCH harq_pid %d\n",
	  mac_xface->frame,next_slot>>1,harq_pid);
#endif
      phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
      }
      //}
      break;
    case 9:

      // Schedule UL subframe
      // get UL harq_pid for subframe+4
      harq_pid = subframe2harq_pid_tdd(3,((next_slot>>1)+4)%10);
      phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;
      nb_dci_ue_spec = 0;
      nb_dci_common  = 0;
      phy_vars_eNb->dlsch_eNb_active = 0;

      
      //if ((mac_xface->frame&1)==1) {
      if (0){//phy_vars_eNb->is_secondary_eNb) {
      memcpy(&dci_alloc[0].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
      dci_alloc[0].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = phy_vars_eNb->eNB_UE_stats[eNb_id].UE_id[UE_id];
      nb_dci_ue_spec = 1;
      nb_dci_common  = 0;
      phy_vars_eNb->dlsch_eNb_active = 0;

#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): Generated ULSCH DCI, format 0\n",mac_xface->frame,next_slot,next_slot>>1);
#endif //DEBUG_PHY
      generate_eNb_ulsch_params_from_dci(&UL_alloc_pdu,
					 phy_vars_eNb->eNB_UE_stats[eNb_id].UE_id[UE_id],
					 (next_slot>>1),
					 format0,
					 phy_vars_eNb->ulsch_eNb[0],
					 lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);

#ifdef DEBUG_PHY
      debug_msg("[PHY PROCEDURES eNB] frame %d, subframe %d Setting scheduling flag for ULSCH harq_pid %d\n",
	  mac_xface->frame,next_slot>>1,harq_pid);
#endif
      phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
      }
      //}
      
      break;
    }
    // if we have DCI to generate do it now
    if ((nb_dci_common+nb_dci_ue_spec)>0) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dci_top\n",mac_xface->frame, next_slot);
#endif
      generate_dci_top(nb_dci_ue_spec,
		       nb_dci_common,
		       dci_alloc,
		       0,
		       AMP,
		       &phy_vars_eNb->lte_frame_parms,
		       phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
		       next_slot>>1);
    }

  }

  // For even next slots generate dlsch
  if (next_slot%2 == 0) {

    if (phy_vars_eNb->dlsch_eNb_active == 1) {
      harq_pid = phy_vars_eNb->dlsch_eNb[0][0]->current_harq_pid;
      input_buffer_length = phy_vars_eNb->dlsch_eNb[0][0]->harq_processes[harq_pid]->TBS/8;
      for (i=0;i<input_buffer_length;i++)
	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dlsch with input size = %d\n",mac_xface->frame, next_slot, input_buffer_length);
#endif //DEBUG_PHY
      
      dlsch_encoding(dlsch_input_buffer,
		     &phy_vars_eNb->lte_frame_parms,
		     phy_vars_eNb->dlsch_eNb[0][0]);
      
      re_allocated = dlsch_modulation(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
				      AMP,
				      next_slot/2,
				      &phy_vars_eNb->lte_frame_parms,
				      phy_vars_eNb->dlsch_eNb[0][0]);
      /*
	if (mimo_mode == DUALSTREAM) {
	dlsch_encoding(input_buffer,
	lte_frame_parms,
	dlsch_eNb[1]);
	
	re_allocated += dlsch_modulation(lte_eNB_common_vars.txdataF[eNb_id],
	1024,
	next_slot>>1,
	lte_frame_parms,
	dlsch_eNb[1]);
	}
      */

      phy_vars_eNb->dlsch_eNb_active = 0;

#ifdef DEBUG_PHY    
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d, DLSCH re_allocated = %d\n",mac_xface->frame, next_slot, re_allocated);
#endif //DEBUG_PHY

    }

    if (phy_vars_eNb->dlsch_eNb_cntl_active == 1) {
      input_buffer_length = phy_vars_eNb->dlsch_eNb_cntl->harq_processes[0]->TBS/8;
      for (i=0;i<input_buffer_length;i++)
	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dlsch (cntl) with input size = %d\n",mac_xface->frame, next_slot, input_buffer_length);
#endif //DEBUG_PHY
      
      dlsch_encoding(dlsch_input_buffer,
		     &phy_vars_eNb->lte_frame_parms,
		     phy_vars_eNb->dlsch_eNb_cntl);
      
      re_allocated = dlsch_modulation(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
				      AMP,
				      next_slot/2,
				      lte_frame_parms,
				      phy_vars_eNb->dlsch_eNb_cntl);
      phy_vars_eNb->dlsch_eNb_cntl_active = 0;

#ifdef DEBUG_PHY    
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d, DLSCH (cntl) re_allocated = %d\n",mac_xface->frame, next_slot, re_allocated);
#endif //DEBUG_PHY

    }

  }

#ifdef DEBUG_PHY
    write_output("eNb_txsigF0_pilots.m","eNb_txsF0_pilots", &(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0][(next_slot>>1)*phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*12]),phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*12,1,1);
#endif //DEBUG_PHY
}
  


void phy_procedures_eNB_RX(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNb) {
  //RX processing
  unsigned int l, ret;
  unsigned int eNb_id=0,UE_id=0;
  int ulsch_power;
  unsigned char harq_pid;
  
  for (l=0;l<phy_vars_eNb->lte_frame_parms.symbols_per_tti/2;l++) {
    for (eNb_id=0; eNb_id<3; eNb_id++)
      slot_fep_ul(&phy_vars_eNb->lte_frame_parms,
		  &phy_vars_eNb->lte_eNB_common_vars,
		  l,
		  last_slot,
		  eNb_id,
#ifdef USER_MODE
		  0
#else
		  1
#endif
		  );
  }

  eNb_id=0;

#ifdef EMOS
  if (last_slot%2==1) {
    memcpy(&emos_dump_eNb.channel[(last_slot>>1)-2][0][0][0],
	   lte_eNB_common_vars.srs_ch_estimates[0][0],
	   NUMBER_OF_eNB_MAX*NB_ANTENNAS_RX*N_RB_UL_EMOS*N_PILOTS_PER_RB_UL);
  }

#endif //EMOS


  // Check for active processes in current subframe
  harq_pid = subframe2harq_pid_tdd(3,last_slot>>1);
  if ((phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag==1) && ((last_slot%2)==1)) {
#ifdef DEBUG_PHY
    debug_msg("[PHY PROCEDURES_LTE] frame %d, slot %d, subframe %d: Scheduling ULSCH Reception for harq_pid %d\n",mac_xface->frame,last_slot,last_slot>>1,harq_pid);
#endif //DEBUG_PHY

    ulsch_power = rx_ulsch(&phy_vars_eNb->lte_eNB_common_vars,
			   phy_vars_eNb->lte_eNB_ulsch_vars[0],
			   &phy_vars_eNb->lte_frame_parms,
			   last_slot>>1,
			   eNb_id,  // this is the effective sector id
			   UE_id,   // this is the UE instance to act upon
			   phy_vars_eNb->ulsch_eNb);
    phy_vars_eNb->eNB_UE_stats[eNb_id].UL_rssi[UE_id] = dB_fixed(ulsch_power) - phy_vars_eNb->rx_total_gain_dB;

#ifdef DEBUG_PHY
    debug_msg("[PHY PROCEDURES_LTE] frame %d, slot %d, subframe %d: ULSCH RX power %d dB\n",mac_xface->frame,last_slot,last_slot>>1,dB_fixed(ulsch_power));
#endif //DEBUG_PHY

    
    
    ret = ulsch_decoding(phy_vars_eNb->lte_eNB_ulsch_vars[0]->llr,
		   &phy_vars_eNb->lte_frame_parms,
		   phy_vars_eNb->ulsch_eNb[UE_id],
		   last_slot>>1);    
    

    phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag=0;
#ifdef DEBUG_PHY
    debug_msg("[PHY PROCEDURES LTE] frame %d, slot %d, subframe %d, eNB %d: received ULSCH for UE %d, ret = %d, CQI CRC Status %d\n",mac_xface->frame, last_slot, last_slot>>1, eNb_id, UE_id, ret, phy_vars_eNb->ulsch_eNb[UE_id]->cqi_crc_status);  
#endif //DEBUG_PHY
      
    if (phy_vars_eNb->ulsch_eNb[UE_id]->cqi_crc_status == 1) {
      if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10)) 
      	print_CQI(phy_vars_eNb->ulsch_eNb[UE_id]->o,phy_vars_eNb->ulsch_eNb[UE_id]->o_RI,wideband_cqi,eNb_id);
      extract_CQI(phy_vars_eNb->ulsch_eNb[UE_id]->o,phy_vars_eNb->ulsch_eNb[UE_id]->o_RI,wideband_cqi,UE_id,&phy_vars_eNb->eNB_UE_stats[eNb_id]);
      phy_vars_eNb->eNB_UE_stats[eNb_id].rank[UE_id] = phy_vars_eNb->ulsch_eNb[UE_id]->o_RI[0];
    }
  }
    
  /*
  if (last_slot%2 == 1) {
    
    rx_power = 0;
    for (aarx=0; aarx<lte_frame_parms.nb_antennas_rx; aarx++) {
      phy_vars_ue->PHY_measurements.rx_power[eNb_id][aarx] = 
	signal_energy_nodc(lte_eNB_common_vars.rxdataF[eNb_id][aarx],
		      lte_frame_parms.ofdm_symbol_size*lte_frame_parms.symbols_per_tti);
      phy_vars_ue->PHY_measurements.rx_power_dB[eNb_id][aarx] = dB_fixed(phy_vars_ue->PHY_measurements.rx_power[eNb_id][aarx]);
      rx_power +=  phy_vars_ue->PHY_measurements.rx_power[eNb_id][aarx];

    }
    phy_vars_ue->PHY_measurements.rx_avg_power_dB[eNb_id] = dB_fixed(rx_power);

    // AGC
    //if (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON)
    //if (mac_xface->frame % 100 == 0)
    //phy_adjust_gain (0,16384,0);

#ifdef DEBUG_PHY      
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: SRS channel estimation: avg_power_dB = %d\n",mac_xface->frame,last_slot,phy_vars_ue->PHY_measurements.rx_avg_power_dB[eNb_id] );
#endif //DEBUG_PHY
  }
  */

#ifdef EMOS
  phy_procedures_emos_eNB_RX(last_slot);
#endif

}
  
void phy_procedures_eNB_S_RX_secsys(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNb) {

  // goal here is to synchronize (again) on PSS from primary eNb
  // should expect offset to be close to zero (all other than the first time)
  int sync_pos=0, sync_pos_slot;

  if (last_slot%2==1) {
    phy_procedures_eNB_S_RX(last_slot, phy_vars_eNb);
    } 
  else if (mac_xface->frame>0) { // last_slot is 2 (where we`ll find PSS from primary eNb)

  // dump one frame of data (would be from HW in real time mode), then call lte_sync_time.  Here, one frame of data would be captured from phy_vars_eNb->lte_eNb_common_vars->rxdata

#ifdef DEBUG_PHY
  msg("[PHY_PROCEDURES_LTE] Frame% d: slot(%d)\n",mac_xface->frame, last_slot);
#endif //DEBUG_PHY
  sync_pos = lte_sync_time(phy_vars_eNb->lte_eNB_common_vars.rxdata[0],
			   &phy_vars_eNb->lte_frame_parms,
			   LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*phy_vars_eNb->lte_frame_parms.samples_per_tti,
			   &phy_vars_eNb->PeNb_id);
#ifdef DEBUG_PHY
  write_output("eNb_rxsig0_1.m","eNb_rxs0_1", phy_vars_eNb->lte_eNB_common_vars.rxdata[0][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
#endif //DEBUG_PHY
  sync_pos_slot = OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*2+2) + CYCLIC_PREFIX_LENGTH + 10;
  phy_vars_eNb->rx_offset = sync_pos - sync_pos_slot;

  // try decoding PBCH (requires rewriting rx_pbch(...))

  // if success set phy_vars_eNb->is_init_sync = 1;
  }
}

void phy_precode_nullBeam_create(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNb) {

  int aa, i, eNb_id=0,PeNb_id=phy_vars_eNb->PeNb_id;
  short re_next, im_next,re_last, im_last;

  switch (last_slot) {
  case 5:
#ifdef DEBUG_PHY
    write_output("srs_ch_est_0_5.m","srs_ce_0_5",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1);    
    write_output("srs_ch_est_1_5.m","srs_ce_1_5",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1); 
#endif //DEBUG_PHY   
    // interpolate last SRS_ch_estimates in freq (from slot 7)
    
    // then interpolate in time (from slot 5 and 7) and rearrange according to create NULL-beam
    /*
     * [h1 h2] ==> [-h2 h1]
     */
    break;
  case 7: 
#ifdef DEBUG_PHY
    write_output("srs_ch_est_0_7.m","srs_ce_0_7",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1);    
    write_output("srs_ch_est_1_7.m","srs_ce_1_7",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1); 
#endif //DEBUG_PHY   
    for (aa=0; aa<phy_vars_eNb->lte_frame_parms.nb_antennas_rx; aa++) {
      // first half (positive freq), samples on odd
      for (i=2; i<phy_vars_eNb->lte_frame_parms.ofdm_symbol_size>>1; i+=2) {
	
	// interpolate last SRS_ch_estimates in freq (from slot 5)
	re_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][aa]))[((i-1)<<1)];
	im_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][aa]))[((i-1)<<1) + 1];
	re_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][aa]))[((i+1)<<1)];
	im_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][aa]))[((i+1)<<1) + 1];
	
	/*
	re_last = phy_vars_eNb->const_ch[aa][0];
	im_last = phy_vars_eNb->const_ch[aa][1];
	re_next = phy_vars_eNb->const_ch[aa][0];
	im_next = phy_vars_eNb->const_ch[aa][1];
	*/
	// rearrange according to create NULL-beam
	/*
	 * [h1 h2] ==> [-h2 h1]
	 * don't forget format of dl_precoder_SeNb, repeated
	 * ifndef IFFT_FPGA --> NO_PREFIX
	 */
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2)] = (1-2*aa)*re_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 2] = (1-2*aa)*re_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 1] = (1-2*aa)*im_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 3] = (1-2*aa)*im_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2)] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 2] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 1] = (1-2*aa)*(im_last + im_next)>>1;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 3] = (1-2*aa)*(im_last + im_next)>>1;
      }

      // second half (negative freq), samples on even
      for (i=((phy_vars_eNb->lte_frame_parms.ofdm_symbol_size>>1)+1); i<phy_vars_eNb->lte_frame_parms.ofdm_symbol_size; i+=2) {
	
	// interpolate last SRS_ch_estimates in freq (from slot 5)
	re_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((i-1)<<1)];
	im_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((i-1)<<1) + 1];
	re_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((i+1)<<1)];
	im_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((i+1)<<1) + 1];
	
	/*
	re_last = phy_vars_eNb->const_ch[aa][0];
	im_last = phy_vars_eNb->const_ch[aa][1];
	re_next = phy_vars_eNb->const_ch[aa][0];
	im_next = phy_vars_eNb->const_ch[aa][1];
	*/
	// rearrange according to create NULL-beam
	/*
	 * [h1 h2] ==> [-h2 h1]
	 */
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2)] = (1-2*aa)*re_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 2] = (1-2*aa)*re_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 1] = (1-2*aa)*im_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 3] = (1-2*aa)*im_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2)] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 2] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 1] = (1-2*aa)*(im_last + im_next)>>1;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 3] = (1-2*aa)*(im_last + im_next)>>1;
      }

      // near DC special case for interpolation.
      // interpolate last SRS_ch_estimates in freq (from slot 5)
      // interpolate first positive and last negative frequency
      re_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((phy_vars_eNb->lte_frame_parms.ofdm_symbol_size-1)<<1)];
      im_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((phy_vars_eNb->lte_frame_parms.ofdm_symbol_size-1)<<1) + 1];
      re_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[2];
      im_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[3];
	
	// rearrange according to create NULL-beam
	/*
	 * [h1 h2] ==> [-h2 h1]
	 */
	((short *)(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa]))[0] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa]))[2] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa]))[1] = (1-2*aa)*(im_last + im_next)>>1;
	((short *)(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa]))[3] = (1-2*aa)*(im_last + im_next)>>1;

    }
#ifdef DEBUG_PHY
    write_output("srs_ch_est_0.m","srs_ce_0",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[0][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1);
    write_output("precoder_1.m","prec_1",phy_vars_eNb->dl_precoder_SeNb[eNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,2,1);
    write_output("srs_ch_est_1.m","srs_ce_1",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[0][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1);
    write_output("precoder_0.m","prec_0",phy_vars_eNb->dl_precoder_SeNb[eNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,2,1);
#endif //DEBUG_PHY

    phy_vars_eNb->has_valid_precoder = 1;
    break;
  default:
    break;
    }
}

void phy_precode_nullBeam_apply(unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNb) {
  
  int i, aa, l, slot_n_symb_offset, eNb_id=0, output_norm;
  
  if (((next_slot < 3) || (next_slot > 9)) && phy_vars_eNb->has_valid_precoder) {
    output_norm = log2_approx(iSqrt(signal_energy_nodc(phy_vars_eNb->dl_precoder_SeNb[eNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)) + iSqrt(signal_energy_nodc(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)));
    
#ifdef DEBUG_PHY
    if (next_slot==11) {
      write_output("precoder_a0.m","prec_a0",phy_vars_eNb->dl_precoder_SeNb[eNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size<<1,2,1);
      write_output("precoder_a1.m","prec_a1",phy_vars_eNb->dl_precoder_SeNb[eNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size<<1,2,1);
      write_output("txdataF_a0_before.m","txF_a0_b",&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0][next_slot*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1),1,1);
      write_output("txdataF_a1_before.m","txF_a1_b",&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][1][next_slot*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1),1,1);
    }
#endif //DEBUG_PHY
    
    for (l=0; l<phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1; l++) {
      slot_n_symb_offset = next_slot*((phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)<<1)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1) + l*((phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)<<1);
      for (aa=0; aa<(phy_vars_eNb->lte_frame_parms.nb_antennas_tx+phy_vars_eNb->nb_virtual_tx); aa++) {
	for (i=0; i<phy_vars_eNb->lte_frame_parms.ofdm_symbol_size; i++) {
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)] = ((short *)(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0]))[slot_n_symb_offset + (i<<1)];          // real part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+1] = -(((short *)(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0]))[slot_n_symb_offset + (i<<1)+1]);   //-imag part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+2] = ((short *)(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0]))[slot_n_symb_offset + (i<<1)+1];      // imag part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+3] = ((short *)(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0]))[slot_n_symb_offset + (i<<1)];        // real part
	} //for(i... OFDM carriers
      } //for(aa... antennas_tx
      
      for (aa=0; aa<(phy_vars_eNb->lte_frame_parms.nb_antennas_tx+phy_vars_eNb->nb_virtual_tx); aa++) {
	// precode. Using mult_cpx_vector_norep
	
	mult_cpx_vector_norep((short *)(phy_vars_eNb->dl_precoder_SeNb[eNb_id][aa]), // input 1
			      (short *)(txdataF_rep_tmp[aa]), // input 2
			      &(((short *)(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa]))[slot_n_symb_offset]), // output
			      phy_vars_eNb->lte_frame_parms.ofdm_symbol_size, // length of vectors (512)
			      output_norm // output_shift
			      );
	
      } //for(aa... antennas_tx
    } //for(l... symbols
    
#ifdef DEBUG_PHY
    if (next_slot==11) {
      write_output("txdataF_rep_a0.m","txF_rep_a0",txdataF_rep_tmp[0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size<<1,2,1);
      write_output("txdataF_rep_a1.m","txF_rep_a1",txdataF_rep_tmp[1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size<<1,2,1);
      write_output("txdataF_a0.m","txF_a0",&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0][next_slot*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1),1,1);
      write_output("txdataF_a1.m","txF_a1",&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][1][next_slot*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1),1,1);
    }
#endif
  }
  else {
    msg("phy_precode_nullBeam_apply should not have been called, since next_slot is not a DL or the precoder is not ready");
  }
}

void phy_precode_nullBeam_apply_ue(unsigned char next_slot,PHY_VARS_UE *phy_vars_ue) {

  int i, aa, l, slot_n_symb_offset, output_norm;

  if (((next_slot < 10) || (next_slot > 3)) && phy_vars_ue->has_valid_precoder) {
    output_norm = log2_approx(iSqrt(signal_energy_nodc(phy_vars_ue->ul_precoder_S_UE[0],phy_vars_ue->lte_frame_parms.ofdm_symbol_size)) + iSqrt(signal_energy_nodc(phy_vars_ue->ul_precoder_S_UE[1],phy_vars_ue->lte_frame_parms.ofdm_symbol_size)));

    for (l=0; l<phy_vars_ue->lte_frame_parms.symbols_per_tti>>1; l++) {
      slot_n_symb_offset = next_slot*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1) + l*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1);

      for (aa=0; aa<(phy_vars_ue->lte_frame_parms.nb_antennas_tx+phy_vars_ue->nb_virtual_tx); aa++) {
	for (i=0; i<phy_vars_ue->lte_frame_parms.ofdm_symbol_size; i++) {
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)] = ((short *)(phy_vars_ue->lte_ue_common_vars.txdataF[0]))[slot_n_symb_offset + (i<<1)];          // real part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+1] = -(((short *)(phy_vars_ue->lte_ue_common_vars.txdataF[0]))[slot_n_symb_offset + (i<<1)+1]);   //-imag part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+2] = ((short *)(phy_vars_ue->lte_ue_common_vars.txdataF[0]))[slot_n_symb_offset + (i<<1)+1];      // imag part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+3] = ((short *)(phy_vars_ue->lte_ue_common_vars.txdataF[0]))[slot_n_symb_offset + (i<<1)];        // real part
	} //for(i... OFDM carriers
      } //for(aa... antennas_tx

      for (aa=0; aa<(phy_vars_ue->lte_frame_parms.nb_antennas_tx+phy_vars_ue->nb_virtual_tx); aa++) {
      // precode. Using mult_cpx_vector_norep
	
	mult_cpx_vector_norep((short *)(phy_vars_ue->ul_precoder_S_UE[aa]), // input 1
			      (short *)(txdataF_rep_tmp[aa]), // input 2
			      &(((short *)(phy_vars_ue->lte_ue_common_vars.txdataF[aa]))[slot_n_symb_offset]), // output
			      phy_vars_ue->lte_frame_parms.ofdm_symbol_size, // length of vectors (512)
			      output_norm // output_shift
			      );
	
      } //for(aa... antennas_tx
    } //for(l... symbols
  }
}

void phy_precode_nullBeam_create_ue(unsigned char last_slot,PHY_VARS_UE *phy_vars_ue) {  

  int aa,i,symb_offset,n_car,PeNb_id=0;

  n_car = phy_vars_ue->lte_frame_parms.N_RB_DL*phy_vars_ue->lte_frame_parms.symbols_per_tti; //300 in the 5MHz case
  symb_offset = phy_vars_ue->lte_frame_parms.ofdm_symbol_size-(n_car>>1);

  if (last_slot==2 || last_slot==10 || 1) { //always allow being called? Why not?
    for (aa=0; aa<phy_vars_ue->lte_frame_parms.nb_antennas_rx; aa++) {
      for (i=0; i<(n_car>>1); i++) {
	
      //positive frequencies
	phy_vars_ue->ul_precoder_S_UE[1-aa][i<<1] = (1-2*aa)*phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][aa][i+(n_car>>1)]; //Re0Im0
	phy_vars_ue->ul_precoder_S_UE[1-aa][(i<<1)+1] = (1-2*aa)*phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][aa][i+(n_car>>1)]; //Re0Im0
	/*
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[i<<2] = (1-2*aa)*phy_vars_ue->const_ch[aa][0]; //Re0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+1] = (1-2*aa)*phy_vars_ue->const_ch[aa][1]; //Im0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+2] = (1-2*aa)*phy_vars_ue->const_ch[aa][0]; //Re0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+3] = (1-2*aa)*phy_vars_ue->const_ch[aa][1]; //Im0
	*/
      //negative frequencies
	
	phy_vars_ue->ul_precoder_S_UE[1-aa][((i+symb_offset)<<1)] = (1-2*aa)*phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][aa][i]; //Re0Im0
	phy_vars_ue->ul_precoder_S_UE[1-aa][((i+symb_offset)<<1)+1] = (1-2*aa)*phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][aa][i]; //Re0Im0
	
	/*
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i+symb_offset)<<2] = (1-2*aa)*phy_vars_ue->const_ch[aa][0]; //Re0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+1] = (1-2*aa)*phy_vars_ue->const_ch[aa][1]; //Im0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+2] = (1-2*aa)*phy_vars_ue->const_ch[aa][0]; //Re0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+3] = (1-2*aa)*phy_vars_ue->const_ch[aa][1]; //Im0
	*/
      }
    }

#ifdef DEBUG_PHY
    write_output("dl_ch_est_0.m","dl_ce_0",phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][0],phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
    write_output("ul_precoder_1.m","ul_prec_1",phy_vars_ue->ul_precoder_S_UE[1],phy_vars_ue->lte_frame_parms.ofdm_symbol_size<<1,2,1);
    write_output("dl_ch_est_1.m","dl_ce_1",phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][1],phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
    write_output("ul_precoder_0.m","ul_prec_0",phy_vars_ue->ul_precoder_S_UE[0],phy_vars_ue->lte_frame_parms.ofdm_symbol_size<<1,2,1);
#endif //DEBUG_PHY
    
    phy_vars_ue->has_valid_precoder = 1;
  }
}

void phy_procedures_eNb_lte(unsigned char last_slot, unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNb) {
  char aa;
  int eNb_id=0;
  
  if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,next_slot>>1)==SF_DL) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_TX(%d)\n",mac_xface->frame, next_slot);
#endif
    if (phy_vars_eNb->is_secondary_eNb) {
      if (phy_vars_eNb->has_valid_precoder) {
	phy_procedures_eNB_TX(next_slot,phy_vars_eNb);
#ifdef NULL_SHAPE_BF_ENABLED
#ifdef DEBUG_PHY
	msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_precode_nullBeam_apply(%d)\n",mac_xface->frame, next_slot);
#endif
	phy_precode_nullBeam_apply(next_slot,phy_vars_eNb);
#endif //NULL_SHAPE_BF_ENABLED
      }
    } 
    else {
      phy_procedures_eNB_TX(next_slot,phy_vars_eNb);
    }
  }
  if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,last_slot>>1)==SF_UL) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_RX(%d)\n",mac_xface->frame, last_slot);
#endif
    phy_procedures_eNB_RX(last_slot,phy_vars_eNb);
    if (phy_vars_eNb->is_secondary_eNb) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_precode_nullBeam_create(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_precode_nullBeam_create(last_slot,phy_vars_eNb);
    }
  }
  if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,next_slot>>1)==SF_S) {
    if (phy_vars_eNb->is_secondary_eNb && next_slot%2==0) {
      if ( mac_xface->frame%10 && phy_vars_eNb->has_valid_precoder) {
#ifdef DEBUG_PHY
	msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
	phy_procedures_eNB_S_TX(next_slot,phy_vars_eNb);
#ifdef NULL_SHAPE_BF_ENABLED
#ifdef DEBUG_PHY
	msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_precode_nullBeam_apply(%d)\n",mac_xface->frame, next_slot);
#endif //DEBUG_PHY
	phy_precode_nullBeam_apply(next_slot,phy_vars_eNb);
#endif //NULL_SHAPE_BF_ENABLED
      } 
      else { //make sure tx-buffer is cleared, in case no transmission.
	for (aa=0; aa<(phy_vars_eNb->lte_frame_parms.nb_antennas_tx+phy_vars_eNb->nb_virtual_tx); aa++) {
#ifdef IFFT_FPGA
	  memset(&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
		 0,(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#else
	  memset(&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
		 0,phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif
	}
      }
    }
    else {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_eNB_S_TX(next_slot,phy_vars_eNb);
    }
  }
  if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,last_slot>>1)==SF_S) {
    if (phy_vars_eNb->is_secondary_eNb) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_RX_secsys(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_eNB_S_RX_secsys(last_slot,phy_vars_eNb);
    }
    else {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_eNB_S_RX(last_slot,phy_vars_eNb);
    }
  }
}


void phy_procedures_ue_lte(unsigned char last_slot, unsigned char next_slot, PHY_VARS_UE *phy_vars_ue) {
  
  if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,next_slot>>1)==SF_UL) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_TX(%d)\n",mac_xface->frame, next_slot);
#endif
    if (phy_vars_ue->is_secondary_ue) {
      if (phy_vars_ue->has_valid_precoder) {
	phy_procedures_UE_TX(next_slot, phy_vars_ue);
#ifdef NULL_SHAPE_BF_ENABLED
	phy_precode_nullBeam_apply_ue(next_slot,phy_vars_ue);
#endif //NULL_SHAPE_BF_ENABLED
      }
    }
    else {
      phy_procedures_UE_TX(next_slot, phy_vars_ue);
    }
  }
  if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,last_slot>>1)==SF_DL) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_RX(%d)\n",mac_xface->frame, last_slot);
#endif
    phy_procedures_UE_RX(last_slot, phy_vars_ue);
    if (phy_vars_ue->is_secondary_ue && last_slot==10 && mac_xface->frame>0) {
      phy_precode_nullBeam_create_ue(last_slot,phy_vars_ue);
    }
  }
  if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,next_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
    if (phy_vars_ue->is_secondary_ue) {
      if (phy_vars_ue->has_valid_precoder) {
	phy_procedures_UE_S_TX(next_slot, phy_vars_ue);
#ifdef NULL_SHAPE_BF_ENABLED
	phy_precode_nullBeam_apply_ue(next_slot,phy_vars_ue);
#endif //NULL_SHAPE_BF_ENABLED
      }
    }
    else {
      phy_procedures_UE_S_TX(next_slot, phy_vars_ue);
    }
  }
  if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,last_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_RX(%d)\n",mac_xface->frame, last_slot);
#endif
    phy_procedures_UE_RX(last_slot, phy_vars_ue);
    if (phy_vars_ue->is_secondary_ue && last_slot==2 && mac_xface->frame>0) {
      phy_precode_nullBeam_create_ue(last_slot,phy_vars_ue);
    }
  }
}
