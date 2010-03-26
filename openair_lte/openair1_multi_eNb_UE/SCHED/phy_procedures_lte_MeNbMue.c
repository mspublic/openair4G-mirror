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
#define DEBUG_PHY
#endif

//#define DIAG_PHY

//undef DEBUG_PHY and set debug_msg to option 1 to print only most necessary messages every 100 frames. 
//define DEBUG_PHY and set debug_msg to option 2 to print everything all frames
//use msg for something that should be always printed in any case

#define debug_msg if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10)) msg
//#define debug_msg msg



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


static char dlsch_ue_active = 0;
static char dlsch_ue_cntl_active = 0;
static char dlsch_eNb_active = 0;
static char dlsch_eNb_cntl_active = 0;


int dlsch_errors = 0;
int dlsch_cntl_errors = 0;

DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];

#ifdef EMOS
//  fifo_dump_emos_UE emos_dump_UE;
  fifo_dump_emos_eNb emos_dump_eNb;
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
    
#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generating SRS\n",mac_xface->frame,next_slot);
#endif
    
    generate_srs_tx(&phy_vars_ue->lte_frame_parms,phy_vars_ue->lte_ue_common_vars.txdataF[0],AMP,next_slot>>1);

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
    
    lte_ue_measurements(&phy_vars_ue->lte_ue_common_vars,
			&phy_vars_ue->lte_frame_parms,
			&PHY_vars->PHY_measurements,
			(last_slot>>1)*phy_vars_ue->lte_frame_parms.symbols_per_tti*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,
			(last_slot == 2) ? 1 : 2,
			1);
    
    if (last_slot == 0) {
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, freq_offset_filt = %d \n",mac_xface->frame, last_slot, phy_vars_ue->lte_ue_common_vars.freq_offset);
      
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), RX gain %d dB\n",
	  mac_xface->frame, last_slot,
	  PHY_vars->PHY_measurements.rx_rssi_dBm[0] - ((phy_vars_ue->lte_frame_parms.nb_antennas_rx==2) ? 3 : 0), 
	  PHY_vars->PHY_measurements.wideband_cqi_dB[0][0],
	  PHY_vars->PHY_measurements.wideband_cqi_dB[0][1],
	  PHY_vars->PHY_measurements.wideband_cqi[0][0],
	  PHY_vars->PHY_measurements.wideband_cqi[0][1],
	  PHY_vars->rx_vars[0].rx_total_gain_dB);
      
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, N0 %d dBm digital (%d, %d) dB, linear (%d, %d)\n",
	  mac_xface->frame, last_slot,
	  dB_fixed(PHY_vars->PHY_measurements.n0_power_tot/phy_vars_ue->lte_frame_parms.nb_antennas_rx) - (int)PHY_vars->rx_vars[0].rx_total_gain_dB,
	  PHY_vars->PHY_measurements.n0_power_dB[0],
	  PHY_vars->PHY_measurements.n0_power_dB[1],
	  PHY_vars->PHY_measurements.n0_power[0],
	  PHY_vars->PHY_measurements.n0_power[1]);
    }

  }
  

  
  if ((last_slot==1) && (l==(4-phy_vars_ue->lte_frame_parms.Ncp))) {
    
    // AGC
    if (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON)
      //      if (mac_xface->frame % 10 == 0)
	phy_adjust_gain (0,512,0);
    
    eNb_id = 0;
    lte_adjust_synch(&phy_vars_ue->lte_frame_parms,
		     &phy_vars_ue->lte_ue_common_vars,
		     eNb_id,
		     1,
		     16384);
    // write frequency offset to pci interface
  }
}

#ifdef EMOS
void phy_procedures_emos_UE_RX(unsigned char last_slot, PHY_VARS_UE *phy_vars_ue) {

  unsigned char eNb_id,i;
  memcpy(&emos_dump_UE.PHY_measurements[last_slot],&PHY_vars->PHY_measurements,sizeof(PHY_MEASUREMENTS));
  if (last_slot==0) {
      emos_dump_UE.timestamp = rt_get_time_ns();
      emos_dump_UE.frame_rx = mac_xface->frame;
      emos_dump_UE.freq_offset = phy_vars_ue->lte_ue_common_vars.freq_offset;
      emos_dump_UE.timing_advance = openair_daq_vars.timing_advance;
      emos_dump_UE.timing_offset  = PHY_vars->rx_vars[0].offset;
      emos_dump_UE.rx_total_gain_dB = PHY_vars->rx_vars[0].rx_total_gain_dB;
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
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Writing EMOS data to FIFO\n",mac_xface->frame, last_slot);
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
    //PHY_vars->PHY_measurements.frame_tx = *((unsigned int*) phy_vars_ue->lte_ue_pbch_vars->decoded_output);
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
  

    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH errors = %d, consecutive errors = %d!\n",
	mac_xface->frame, last_slot, phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors, phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq);
    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH received frame = %d!\n",
	mac_xface->frame, last_slot,*((unsigned int*) phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output));


  if (phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq>20) {
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
  
  //  write_output("UE_rxsigF0.m","UE_rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],512*12*2,2,1);
  //  write_output("UE_rxsigF1.m","UE_rxsF1", phy_vars_ue->lte_ue_common_vars.rxdataF[1],512*12*2,2,1);
  
  rx_pdcch(&phy_vars_ue->lte_ue_common_vars,
	   phy_vars_ue->lte_ue_pdcch_vars,
	   &phy_vars_ue->lte_frame_parms,
	   eNb_id,
	   2,
	   (phy_vars_ue->lte_frame_parms.nb_antennas_tx == 1) ? SISO : ALAMOUTI); //this needs to be changed
  
  dci_cnt = dci_decoding_procedure(phy_vars_ue->lte_ue_pdcch_vars,dci_alloc_rx,eNb_id,&phy_vars_ue->lte_frame_parms,SI_RNTI,RA_RNTI,C_RNTI);

  phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_received += dci_cnt;
#ifdef DIAG_PHY
  switch (last_slot>>1) {
  case 0:
  case 5:
  case 8:
    if (dci_cnt != 1) {
      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors ++;
      msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: missed DCI!\n",mac_xface->frame,last_slot>>1,dci_cnt);
      msg("[PHY_PROCEDURES_UE][DIAG] frame %d, slot %d, RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), RX gain %d dB\n",
	  mac_xface->frame, last_slot,
	  PHY_vars->PHY_measurements.rx_rssi_dBm[0] - ((phy_vars_ue->lte_frame_parms.nb_antennas_rx==2) ? 3 : 0), 
	  PHY_vars->PHY_measurements.wideband_cqi_dB[0][0],
	  PHY_vars->PHY_measurements.wideband_cqi_dB[0][1],
	  PHY_vars->PHY_measurements.wideband_cqi[0][0],
	  PHY_vars->PHY_measurements.wideband_cqi[0][1],
	  PHY_vars->rx_vars[0].rx_total_gain_dB);
    }
    break;
  default:
    break;
  }
  if (last_slot==18)
    debug_msg("[PHY_PROCEDURES_LTE][DIAG] Frame %d, slot %d: PDCCH: DCI errors %d, DCI received %d \n",mac_xface->frame,last_slot,phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors,phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_received);


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
    //#ifdef DEBUG_PHY
      msg("[PHY PROCEDURES UE] subframe %d: Found rnti %x, format %d\n",last_slot>>1,dci_alloc_rx[i].rnti,
	  dci_alloc_rx[i].format);
    //#endif
    if ((dci_alloc_rx[i].rnti == C_RNTI) && (dci_alloc_rx[i].format == format2_2A_M10PRB)) {

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
					    C_RNTI,
					    format2_2A_M10PRB,
					    phy_vars_ue->dlsch_ue[0],
					    &phy_vars_ue->lte_frame_parms,
					    SI_RNTI,
					    RA_RNTI,
					    P_RNTI)==0) {
	dlsch_ue_active = 1;
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
					    &dlsch_ue_cntl, 
					    &phy_vars_ue->lte_frame_parms,
					    SI_RNTI,
					    RA_RNTI,
					    P_RNTI)==0) {
	dlsch_ue_cntl_active = 1;
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Generate UE DLSCH SI_RNTI format 1A\n");
#endif
      }
    }
    else if ((dci_alloc_rx[i].rnti == C_RNTI) && (dci_alloc_rx[i].format == format0)) {
#ifdef DIAG_PHY
      if ((last_slot>>1) != 8) {
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received C_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	//exit_openair=1;
	return;
      }
#endif
      generate_ue_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					C_RNTI,
					last_slot>>1,
					format0,
					phy_vars_ue->ulsch_ue[eNb_id],
					&PHY_vars->PHY_measurements,
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
      msg("[PHY PROCEDURES UE] frame %d, subframe %d: received DCI with RNTI=%x and format %d!\n",mac_xface->frame,last_slot>>1,dci_alloc_rx[i].rnti,dci_alloc_rx[i].format);
#ifdef DIAG_PHY
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	//exit_openair=1;
	return;
#endif
    }

  }
}


int phy_procedures_UE_RX(unsigned char last_slot, PHY_VARS_UE *phy_vars_ue) {

  unsigned short l,m,n_symb;
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

      if ( (dlsch_ue_active == 1) && (dlsch_ue_cntl_active == 1))
	msg("[PHY_PROCEDURES_LTE] WARNING: phy_vars_ue->dlsch_ue and dlsch_ue_cntl active, but data structures can only handle one at a time\n");

      if (dlsch_ue_active == 1) {
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
		   dual_stream_UE);
	
	dlsch_ue_active = 0;
      
#ifndef USER_MODE
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Scheduling DLSCH decoding\n",mac_xface->frame,last_slot);

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
	if (mac_xface->frame < dlsch_errors)
	  dlsch_errors=0;
	
	if (phy_vars_ue->dlsch_ue[0][0]) {
	  ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->llr[0],
			       &phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->dlsch_ue[0][0],
			       ((last_slot>>1)-1)%10);
	  
	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    dlsch_errors++;
	  }
	}   
	
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_decoding ret %d (%d errors)\n",
		  mac_xface->frame,last_slot,ret,dlsch_errors);
#endif
      }
      if (dlsch_ue_cntl_active == 1) {
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
		   &dlsch_ue_cntl,
		   m,
		   dual_stream_UE);
	
	//write_output("dlsch_cntl_llr.m","llr",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->llr[0],40,1,0);

	dlsch_ue_cntl_active = 0;
      
	if (mac_xface->frame < dlsch_cntl_errors)
	  dlsch_cntl_errors=0;

	/*
	if (dlsch_ue_cntl) {
	  ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars_cntl[eNb_id]->llr[0],
			       phy_vars_ue->lte_frame_parms,
			       dlsch_ue_cntl,
			       ((last_slot>>1)-1)%10);
	  
	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    dlsch_cntl_errors++;
	  }
	}   
	
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_decoding (cntl) ret %d (%d errors)\n",
	      mac_xface->frame,last_slot,ret,dlsch_cntl_errors);
	*/
      }

    }
      
    if (((last_slot%2)==0) && (l==(4-phy_vars_ue->lte_frame_parms.Ncp)))  {
	
      lte_ue_pdcch_procedures(eNb_id,last_slot,phy_vars_ue);
	
      if (dlsch_ue_active == 1) {
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
		   phy_vars_ue->dlsch_ue[0][0],
		   m,
		   dual_stream_UE);
      }
      if (dlsch_ue_cntl_active == 1) {
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
		   &dlsch_ue_cntl,
		   m,
		   dual_stream_UE);
      }
    }
      
    if (((last_slot%2)==1) && (l==0)) {
	
      if (dlsch_ue_active == 1)  {
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
		   dual_stream_UE);
      }
      if (dlsch_ue_cntl_active == 1)  {
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
		   &dlsch_ue_cntl,
		   m,
		   dual_stream_UE);
      }
    }
      
    if (((last_slot%2)==1) && (l==(4-phy_vars_ue->lte_frame_parms.Ncp))) {
	
      if(dlsch_ue_active == 1) {
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
		   dual_stream_UE);
      }
      if(dlsch_ue_cntl_active == 1) {
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
		   &dlsch_ue_cntl,
		   m,
		   dual_stream_UE);
      }
    }
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

    //#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d: Peak found at pos %d, offset %d (time_in %d, time_out %d)\n",mac_xface->frame, last_slot, sync_pos, sync_pos - sync_pos_slot, time_in, time_out);
    //#endif
  }
}


#ifdef EMOS

void phy_procedures_emos_eNB_RX(unsigned char last_slot) {

  unsigned char eNb_id,i;
  for (eNb_id = 0; eNb_id<3; eNb_id++)  
    memcpy(&emos_dump_eNb.eNB_UE_stats[eNb_id][last_slot],&eNB_UE_stats,sizeof(LTE_eNB_UE_stats));

  if (last_slot==4) {
      emos_dump_UE.timestamp = rt_get_time_ns();
      emos_dump_UE.frame_tx = mac_xface->frame;
      emos_dump_UE.rx_total_gain_dB = PHY_vars->rx_vars[0].rx_total_gain_dB;
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
  int eNb_id = 0,i,aa;
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
      dci_alloc[1].rnti       = C_RNTI;
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
					 &dlsch_eNb_cntl,
					 &phy_vars_eNb->lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);
      dlsch_eNb_cntl_active = 1;

      // Schedule UL subframe
      /*
      generate_eNb_ulsch_params_from_dci(&UL_alloc_pdu,
					 C_RNTI,
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
      */

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
      memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
      dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = C_RNTI;
      nb_dci_common  = 0;
      nb_dci_ue_spec = 1;
      //#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generated DLSCH DCI, format 2_2A_M10PRB\n",mac_xface->frame, next_slot);
      //#endif

      generate_eNb_dlsch_params_from_dci(next_slot>>1,
					 &DLSCH_alloc_pdu2,
					 C_RNTI,
					 format2_2A_M10PRB,
					 phy_vars_eNb->dlsch_eNb[0],
					 &phy_vars_eNb->lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);
      dlsch_eNb_active = 1;
      break;
 
    case 6:
      nb_dci_ue_spec = 0;
      nb_dci_common  = 0;
      dlsch_eNb_active = 0;
      break;
    case 7:
      nb_dci_ue_spec = 0;
      nb_dci_common  = 0;
      dlsch_eNb_active = 0;
      break;
    case 8:

      // Schedule UL subframe
      // get UL harq_pid for subframe+4
      harq_pid = subframe2harq_pid_tdd(3,((next_slot>>1)+4)%10);
      phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;
      nb_dci_ue_spec = 0;
      nb_dci_common  = 0;
      dlsch_eNb_active = 0;

      //if ((mac_xface->frame&1)==0) {
      memcpy(&dci_alloc[0].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
      dci_alloc[0].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = C_RNTI;
      nb_dci_ue_spec = 1;
      nb_dci_common  = 0;
      dlsch_eNb_active = 0;

#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): Generated ULSCH DCI, format 0\n",mac_xface->frame,next_slot,next_slot>>1);
#endif

      generate_eNb_ulsch_params_from_dci(&UL_alloc_pdu,
					 C_RNTI,
					 (next_slot>>1),
					 format0,
					 phy_vars_eNb->ulsch_eNb[0],
					 &phy_vars_eNb->lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);

      //#ifdef DEBUG_PHY
      debug_msg("[PHY PROCEDURES eNB] frame %d, subframe %d Setting scheduling flag for ULSCH harq_pid %d\n",
	  mac_xface->frame,next_slot>>1,harq_pid);
      //#endif
      phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
      //}
      break;
    case 9:

      // Schedule UL subframe
      // get UL harq_pid for subframe+4
      harq_pid = subframe2harq_pid_tdd(3,((next_slot>>1)+4)%10);
      phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;
      nb_dci_ue_spec = 0;
      nb_dci_common  = 0;
      dlsch_eNb_active = 0;

      /*
      if ((mac_xface->frame&1)==1) {
      memcpy(&dci_alloc[0].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
      dci_alloc[0].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = C_RNTI;
      nb_dci_ue_spec = 1;
      nb_dci_common  = 0;
      dlsch_eNb_active = 0;

#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): Generated ULSCH DCI, format 0\n",mac_xface->frame,next_slot,next_slot>>1);
#endif
      generate_eNb_ulsch_params_from_dci(&UL_alloc_pdu,
					 C_RNTI,
					 (next_slot>>1),
					 format0,
					 phy_vars_eNb->ulsch_eNb[0],
					 lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);

      //#ifdef DEBUG_PHY
      debug_msg("[PHY PROCEDURES eNB] frame %d, subframe %d Setting scheduling flag for ULSCH harq_pid %d\n",
	  mac_xface->frame,next_slot>>1,harq_pid);
      //#endif
      phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
      }
      */
      break;
    }
    // if we have DCI to generate do it now
    if ((nb_dci_common+nb_dci_ue_spec)>0) {
      //#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dci_top\n",mac_xface->frame, next_slot);
      //#endif
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

    if (dlsch_eNb_active == 1) {
      harq_pid = phy_vars_eNb->dlsch_eNb[0][0]->current_harq_pid;
      input_buffer_length = phy_vars_eNb->dlsch_eNb[0][0]->harq_processes[harq_pid]->TBS/8;
      for (i=0;i<input_buffer_length;i++)
	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      
      //#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dlsch with input size = %d\n",mac_xface->frame, next_slot, input_buffer_length);
      //#endif
      
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
      dlsch_eNb_active = 0;

      //#ifdef DEBUG_PHY    
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d, DLSCH re_allocated = %d\n",mac_xface->frame, next_slot, re_allocated);
      //#endif

    }

    if (dlsch_eNb_cntl_active == 1) {
      input_buffer_length = dlsch_eNb_cntl->harq_processes[0]->TBS/8;
      for (i=0;i<input_buffer_length;i++)
	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dlsch (cntl) with input size = %d\n",mac_xface->frame, next_slot, input_buffer_length);
#endif
      
      dlsch_encoding(dlsch_input_buffer,
		     &phy_vars_eNb->lte_frame_parms,
		     dlsch_eNb_cntl);
      
      re_allocated = dlsch_modulation(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
				      AMP,
				      next_slot/2,
				      lte_frame_parms,
				      dlsch_eNb_cntl);
      dlsch_eNb_cntl_active = 0;

#ifdef DEBUG_PHY    
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d, DLSCH (cntl) re_allocated = %d\n",mac_xface->frame, next_slot, re_allocated);
#endif

    }

  }
}
  


void phy_procedures_eNB_RX(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNb) {
  //RX processing
  unsigned int l, ret;
  unsigned int eNb_id=0,UE_id=0;
  int ulsch_power;
  unsigned char harq_pid;
  
  for (l=0;l<phy_vars_eNb->lte_frame_parms.symbols_per_tti/2;l++) {
    
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
    //#ifdef DEBUG_PHY
    debug_msg("[PHY PROCEDURES_LTE] frame %d, slot %d, subframe %d: Scheduling ULSCH Reception for harq_pid %d\n",mac_xface->frame,last_slot,last_slot>>1,harq_pid);
    //#endif

    ulsch_power = rx_ulsch(&phy_vars_eNb->lte_eNB_common_vars,
			   phy_vars_eNb->lte_eNB_ulsch_vars[0],
			   &phy_vars_eNb->lte_frame_parms,
			   last_slot>>1,
			   eNb_id,  // this is the effective sector id
			   UE_id,   // this is the UE instance to act upon
			   phy_vars_eNb->ulsch_eNb);
    eNB_UE_stats[eNb_id].UL_rssi[UE_id] = dB_fixed(ulsch_power) - PHY_vars->rx_vars[0].rx_total_gain_dB;

    debug_msg("[PHY PROCEDURES_LTE] frame %d, slot %d, subframe %d: ULSCH RX power %d dB\n",mac_xface->frame,last_slot,last_slot>>1,dB_fixed(ulsch_power));

    
    
    ret = ulsch_decoding(phy_vars_eNb->lte_eNB_ulsch_vars[0]->llr,
		   &phy_vars_eNb->lte_frame_parms,
		   phy_vars_eNb->ulsch_eNb[UE_id],
		   last_slot>>1);    
    

    phy_vars_eNb->ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag=0;

    debug_msg("[PHY PROCEDURES LTE] frame %d, slot %d, subframe %d, eNB %d: received ULSCH for UE %d, ret = %d, CQI CRC Status %d\n",mac_xface->frame, last_slot, last_slot>>1, eNb_id, UE_id, ret, phy_vars_eNb->ulsch_eNb[UE_id]->cqi_crc_status);  
      
    if (phy_vars_eNb->ulsch_eNb[UE_id]->cqi_crc_status == 1) {
      if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10)) 
      	print_CQI(phy_vars_eNb->ulsch_eNb[UE_id]->o,phy_vars_eNb->ulsch_eNb[UE_id]->o_RI,wideband_cqi,eNb_id);
      extract_CQI(phy_vars_eNb->ulsch_eNb[UE_id]->o,phy_vars_eNb->ulsch_eNb[UE_id]->o_RI,wideband_cqi,UE_id,&eNB_UE_stats[eNb_id]);
      eNB_UE_stats[eNb_id].rank[UE_id] = phy_vars_eNb->ulsch_eNb[UE_id]->o_RI[0];
    }
  }
    
  /*
  if (last_slot%2 == 1) {
    
    rx_power = 0;
    for (aarx=0; aarx<lte_frame_parms.nb_antennas_rx; aarx++) {
      PHY_vars->PHY_measurements.rx_power[eNb_id][aarx] = 
	signal_energy_nodc(lte_eNB_common_vars.rxdataF[eNb_id][aarx],
		      lte_frame_parms.ofdm_symbol_size*lte_frame_parms.symbols_per_tti);
      PHY_vars->PHY_measurements.rx_power_dB[eNb_id][aarx] = dB_fixed(PHY_vars->PHY_measurements.rx_power[eNb_id][aarx]);
      rx_power +=  PHY_vars->PHY_measurements.rx_power[eNb_id][aarx];

    }
    PHY_vars->PHY_measurements.rx_avg_power_dB[eNb_id] = dB_fixed(rx_power);

    // AGC
    //if (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON)
    //if (mac_xface->frame % 100 == 0)
    //phy_adjust_gain (0,16384,0);

#ifdef DEBUG_PHY      
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: SRS channel estimation: avg_power_dB = %d\n",mac_xface->frame,last_slot,PHY_vars->PHY_measurements.rx_avg_power_dB[eNb_id] );
#endif
  }
  */

#ifdef EMOS
  phy_procedures_emos_eNB_RX(last_slot);
#endif

}

  
void phy_procedures_eNb_lte(unsigned char last_slot, unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNb) {

  //#define DEBUG_PHY
    if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,next_slot>>1)==SF_DL) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_eNB_TX(next_slot,phy_vars_eNb);
    }
    if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,last_slot>>1)==SF_UL) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_eNB_RX(last_slot,phy_vars_eNb);
    }
    if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,next_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_eNB_S_TX(next_slot,phy_vars_eNb);
    }
    if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,last_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_eNB_S_RX(last_slot,phy_vars_eNb);
    }
}

void phy_procedures_ue_lte(unsigned char last_slot, unsigned char next_slot, PHY_VARS_UE *phy_vars_ue) {

  //#define DEBUG_PHY
    if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,next_slot>>1)==SF_UL) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_UE_TX(next_slot, phy_vars_ue);
    }
    if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,last_slot>>1)==SF_DL) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_UE_RX(last_slot, phy_vars_ue);
    }
    if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,next_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_UE_S_TX(next_slot, phy_vars_ue);
    }
    if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,last_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_UE_RX(last_slot, phy_vars_ue);
    }
}
