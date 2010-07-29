/*________________________phy_procedures_lte.c________________________

Authors : Hicham Anouar, Raymond Knopp, Florian Kaltenberger
Company : EURECOM
Emails  : anouar@eurecom.fr,  knopp@eurecom.fr, kaltenbe@eurecom.fr
________________________________________________________________*/


// This routine is called periodically by macphy_scheduler to analyse the set of PHY_primitives that were
// Scheduled by MAC and on PHY resources at the appropriate time

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

//#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
//#endif

#ifdef USER_MODE
//#define DEBUG_PHY
#endif

#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
#endif

#define DIAG_PHY

#define DLSCH_RB_ALLOC 0x1fbf  // skip DC RB (total 23/25 RBs)
#define DLSCH_RB_ALLOC_12 0x0aaa  // skip DC RB (total 23/25 RBs)

#define NS_PER_SLOT 500000

extern inline unsigned int taus(void);
extern int exit_openair;

unsigned char ulsch_input_buffer[2700] __attribute__ ((aligned(16)));


extern int dlsch_instance_cnt[8];
extern int dlsch_subframe[8];
extern pthread_mutex_t dlsch_mutex[8];
/// Condition variable for dlsch thread
extern pthread_cond_t dlsch_cond[8];

/*
static char dlsch_ue_active = 0;
static char dlsch_ue_cntl_active = 0;
static char dlsch_ue_ra_active = 0;
static char dlsch_ue_1A_active = 0;
static char ulsch_no_allocation_counter = 0;

int dlsch_errors = 0;
int dlsch_errors_last = 0;
int dlsch_received = 0;
int dlsch_received_last = 0;
int dlsch_fer = 0;
int dlsch_cntl_errors = 0;
int dlsch_ra_errors = 0;
int current_dlsch_cqi = 0;

unsigned char  ulsch_ue_rag_active;
unsigned int  ulsch_ue_rag_frame;
unsigned char ulsch_ue_rag_subframe;
unsigned char rag_timer;
*/

DCI_ALLOC_t dci_alloc_rx[8];

#ifdef EMOS
  fifo_dump_emos_UE emos_dump_UE;
#endif

#ifdef DIAG_PHY
extern int rx_sig_fifo;
#endif

unsigned int ue_rag_frame;
unsigned char ue_rag_subframe;

#ifdef USER_MODE

void dump_dlsch(PHY_VARS_UE *phy_vars_ue) {
  unsigned int coded_bits_per_codeword;

  coded_bits_per_codeword = ( phy_vars_ue->dlsch_ue[0]->nb_rb * (12 * get_Qm(phy_vars_ue->dlsch_ue[0]->harq_processes[0]->mcs)) * (phy_vars_ue->lte_frame_parms.num_dlsch_symbols));
  write_output("rxsigF0.m","rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", phy_vars_ue->lte_ue_dlsch_vars[0]->rxdataF_ext[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext",phy_vars_ue->lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[0],300*12,1,1);
  /*
    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",phy_vars_ue->lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[1],300*12,1,1);
    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",phy_vars_ue->lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[2],300*12,1,1);
    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",phy_vars_ue->lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[3],300*12,1,1);
    write_output("dlsch_rho.m","dl_rho",phy_vars_ue->lte_ue_dlsch_vars[0]->rho[0],300*12,1,1);
  */
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0",phy_vars_ue->lte_ue_dlsch_vars[0]->rxdataF_comp[0],300*12,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr",phy_vars_ue->lte_ue_dlsch_vars[0]->llr[0],coded_bits_per_codeword,1,0);
  
  write_output("dlsch_mag1.m","dlschmag1",phy_vars_ue->lte_ue_dlsch_vars[0]->dl_ch_mag,300*12,1,1);
  write_output("dlsch_mag2.m","dlschmag2",phy_vars_ue->lte_ue_dlsch_vars[0]->dl_ch_magb,300*12,1,1);
}
#endif

#ifdef EMOS
void phy_procedures_emos_UE_TX(unsigned char next_slot,PHY_VARS_UE *phy_vars_ue) {
  unsigned char harq_pid;

  if (next_slot%2==0) {      
    // get harq_pid from subframe relationship
    harq_pid = subframe2harq_pid_tdd(phy_vars_ue->lte_frame_parms.tdd_config,(next_slot>>1));    
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

void phy_procedures_UE_TX(unsigned char next_slot,PHY_VARS_UE *phy_vars_ue) {
  
  unsigned short first_rb, nb_rb;
  unsigned char harq_pid;
  unsigned int input_buffer_length;
  unsigned int i, aa;
  unsigned char eNb_id = 0,rag_flag=0;
  
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
      if (phy_vars_ue->UE_mode != PRACH) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generating SRS\n",mac_xface->frame,next_slot);
#endif
	generate_srs_tx(&phy_vars_ue->lte_frame_parms,phy_vars_ue->lte_ue_common_vars.txdataF[0],AMP,next_slot>>1);
#ifdef DEBUG_PHY
	write_output("UE_srs_tx.m","srs_tx",&phy_vars_ue->lte_ue_common_vars.txdataF[0][next_slot*phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1) + phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti-1)],phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
#endif //DEBUG_PHY
      }
    }

    if ((phy_vars_ue->ulsch_ue_rag_active == 1) && (phy_vars_ue->ulsch_ue_rag_frame == mac_xface->frame) && (phy_vars_ue->ulsch_ue_rag_subframe == (next_slot>>1))) {
      harq_pid = 0;
      phy_vars_ue->ulsch_ue[0]->harq_processes[0]->subframe_scheduling_flag = 1;
      generate_ue_ulsch_params_from_rar(phy_vars_ue->dlsch_ue_ra->harq_processes[0]->b,
					phy_vars_ue->ulsch_ue_rag_subframe,
					phy_vars_ue->ulsch_ue[0],
					&phy_vars_ue->PHY_measurements,
					&phy_vars_ue->lte_frame_parms,
					eNb_id,
					phy_vars_ue->current_dlsch_cqi);
      phy_vars_ue->ulsch_ue[0]->power_offset = 14;
      //      printf("UE: Setting rag_flag\n");
      rag_flag = 1;
      
    }
    else {
      // get harq_pid from subframe relationship
      harq_pid = subframe2harq_pid_tdd(phy_vars_ue->lte_frame_parms.tdd_config,(next_slot>>1));
      if (harq_pid==255) {
	msg("ulsch_decoding.c: FATAL ERROR: illegal harq_pid, returning\n");
	return;
      }
      rag_flag=0;
    }
    if (phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->subframe_scheduling_flag == 1) {

      // deactivate service request
      phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;

      get_ack(phy_vars_ue->lte_frame_parms.tdd_config,phy_vars_ue->dlsch_ue[0]->harq_ack,(next_slot>>1),phy_vars_ue->ulsch_ue[0]->o_ACK);

      first_rb = phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->first_rb;
      nb_rb = phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->nb_rb;
      
      generate_drs_pusch(&phy_vars_ue->lte_frame_parms,phy_vars_ue->lte_ue_common_vars.txdataF[0],AMP,next_slot>>1,first_rb,nb_rb);
      
      input_buffer_length = phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->TBS/8;
      
      for (i=0;i<input_buffer_length;i++) {
	ulsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      }
#ifdef DEBUG_PHY      
      debug_msg("[PHY_PROCEDURES_LTE][UE_UL] ulsch_ue %p : O %d, O_ACK %d, O_RI %d, TBS %d\n",phy_vars_ue->ulsch_ue[0],phy_vars_ue->ulsch_ue[0]->O,phy_vars_ue->ulsch_ue[0]->O_ACK,phy_vars_ue->ulsch_ue[0]->O_RI,phy_vars_ue->ulsch_ue[0]->harq_processes[harq_pid]->TBS);
#endif

      if (rag_flag == 1)
	msg("[PHY_PROCEDURES][UE] Frame %d, Subframe %d Generating RAG (nb_rb %d, first_rb %d)\n",mac_xface->frame,next_slot>>1,phy_vars_ue->ulsch_ue[0]->harq_processes[0]->nb_rb,phy_vars_ue->ulsch_ue[0]->harq_processes[0]->first_rb);
      ulsch_encoding(ulsch_input_buffer,&phy_vars_ue->lte_frame_parms,phy_vars_ue->ulsch_ue[0],harq_pid);
      ulsch_modulation(phy_vars_ue->lte_ue_common_vars.txdataF,AMP,(next_slot>>1),&phy_vars_ue->lte_frame_parms,phy_vars_ue->ulsch_ue[0],rag_flag);

    }
  }
}

void phy_procedures_UE_S_TX(unsigned char next_slot, PHY_VARS_UE *phy_vars_ue) {

  int aa,card_id;

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


    if (phy_vars_ue->UE_mode == PRACH) {

      if ((openair_daq_vars.timing_advance == TIMING_ADVANCE_INIT) ||
	  (openair_daq_vars.manual_timing_advance != 0)) {

	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generating PSS for UL, TX power %d dBm (PL %d dB)\n",
		  mac_xface->frame,next_slot,
		  43-phy_vars_ue->PHY_measurements.rx_rssi_dBm[0]-114,
		  43-phy_vars_ue->PHY_measurements.rx_rssi_dBm[0]);
	
	generate_pss(phy_vars_ue->lte_ue_common_vars.txdataF,
		     AMP,
		     &phy_vars_ue->lte_frame_parms,
		     0, //phy_vars_ue->lte_ue_common_vars.eNb_id,
		     PSS_UL_SYMBOL,
		     next_slot);
	//write_output("ue_txFsig_pss.m","ue_txF_pss",&phy_vars_ue->lte_ue_common_vars.txdataF[0][next_slot*phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1),1,1);
      }
      else {
	openair_daq_vars.timing_advance = TIMING_ADVANCE_INIT;
	
#ifdef CBMIMO1
	for (card_id=0;card_id<number_of_cards;card_id++)
	  pci_interface[card_id]->timing_advance = openair_daq_vars.timing_advance;
#endif
      }
    }
  }
}
  
void lte_ue_measurement_procedures(unsigned char last_slot, unsigned short l, PHY_VARS_UE *phy_vars_ue) {
  
  unsigned char eNb_id;
#ifdef EMOS
  unsigned char aa;

  // first slot in frame is special
  if (((last_slot==0) || (last_slot==1) || (last_slot==12) || (last_slot==13)) && 
      ((l==0) || (l==4-phy_vars_ue->lte_frame_parms.Ncp))) {
    for (eNb_id=0; eNb_id<3; eNb_id++) 
      for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++)
	lte_dl_channel_estimation_emos(emos_dump_UE.channel[eNb_id],
				       phy_vars_ue->lte_ue_common_vars.rxdataF,
				       &phy_vars_ue->lte_frame_parms,
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
#ifndef USER_MODE
			(last_slot>>1)*phy_vars_ue->lte_frame_parms.symbols_per_tti*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,
#else
			(last_slot>>1)*phy_vars_ue->lte_frame_parms.symbols_per_tti*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size+phy_vars_ue->lte_frame_parms.nb_prefix_samples),
#endif
			(last_slot == 2) ? 1 : 2,
			1);

#ifdef DEBUG_PHY    
    if (last_slot == 0) {

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
    }
#endif
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

    //if (openair_daq_vars.auto_freq_correction == 1) 
    /*
    if (mac_xface->frame % 100 == 0) {
      if ((ue_common_vars->freq_offset>100) && (openair_daq_vars.freq_offset < 500)) {
	openair_daq_vars.freq_offset+=100;
	openair_set_freq_offset(0,penair_daq_vars.freq_offset);
      }
      else if ((ue_common_vars->freq_offset<-100) && (openair_daq_vars.freq_offset > -500)) {
	openair_daq_vars.freq_offset-=100;
	openair_set_freq_offset(0,penair_daq_vars.freq_offset);
      }
    }
    */
  }
}

#ifdef EMOS
void phy_procedures_emos_UE_RX(unsigned char last_slot, PHY_VARS_UE *phy_vars_ue) {

  unsigned char eNb_id,i;
  memcpy(&emos_dump_UE.PHY_measurements[last_slot],&phy_vars_ue->PHY_measurements,sizeof(PHY_MEASUREMENTS));
  if (last_slot==0) {
      emos_dump_UE.timestamp = rt_get_time_ns();
      emos_dump_UE.frame_rx = mac_xface->frame;
      emos_dump_UE.UE_mode = phy_vars_ue->UE_mode;
      emos_dump_UE.freq_offset = phy_vars_ue->lte_ue_common_vars.freq_offset;
      emos_dump_UE.timing_advance = openair_daq_vars.timing_advance;
      emos_dump_UE.timing_offset  = phy_vars_ue->rx_offset;
      emos_dump_UE.rx_total_gain_dB = phy_vars_ue->rx_total_gain_dB;
      emos_dump_UE.eNb_id = phy_vars_ue->lte_ue_common_vars.eNb_id;
  }
  if (last_slot==1) {
    for (eNb_id = 0; eNb_id<3; eNb_id++) { 
      memcpy(emos_dump_UE.pbch_pdu[eNb_id],phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output,PBCH_PDU_SIZE);
      emos_dump_UE.pbch_errors[eNb_id] = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors;
      emos_dump_UE.pbch_errors_last[eNb_id] = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_last;
      emos_dump_UE.pbch_errors_conseq[eNb_id] = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq;
      emos_dump_UE.pbch_fer[eNb_id] = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_fer;
    }
  }
  if (last_slot%2==0) {
    for (i=0; i<2; i++) 
      memcpy(&emos_dump_UE.DCI_alloc[i][last_slot>>1], &dci_alloc_rx[i], sizeof(DCI_ALLOC_t));
    }
  if (last_slot==0) {
    eNb_id = phy_vars_ue->lte_ue_common_vars.eNb_id;
    emos_dump_UE.dci_errors = phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors;
    emos_dump_UE.dci_received = phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_received;
    emos_dump_UE.dci_false = phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_false;
    emos_dump_UE.dci_missed = phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_missed;
    emos_dump_UE.dlsch_errors = phy_vars_ue->dlsch_errors;
    emos_dump_UE.dlsch_errors_last = phy_vars_ue->dlsch_errors_last;
    emos_dump_UE.dlsch_received = phy_vars_ue->dlsch_received;
    emos_dump_UE.dlsch_received_last = phy_vars_ue->dlsch_received_last;
    emos_dump_UE.dlsch_fer = phy_vars_ue->dlsch_fer;
    emos_dump_UE.dlsch_cntl_errors = phy_vars_ue->dlsch_cntl_errors;
    emos_dump_UE.dlsch_ra_errors = phy_vars_ue->dlsch_ra_errors;
  }
  if (last_slot==0) {
    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Writing EMOS data to FIFO\n",mac_xface->frame, last_slot);
    if (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump_UE, sizeof(fifo_dump_emos_UE))!=sizeof(fifo_dump_emos_UE)) {
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Problem writing EMOS data to FIFO\n",mac_xface->frame, last_slot);
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
		       (phy_vars_ue->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI);
  if (pbch_error) {
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq = 0;
#ifdef EMOS
    emos_dump_UE.frame_tx = *((unsigned int*) phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output);
    emos_dump_UE.mimo_mode = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output[4];
    //phy_vars_ue->PHY_measurements.frame_tx = *((unsigned int*) phy_vars_ue->lte_ue_pbch_vars->decoded_output);
#endif
    phy_vars_ue->lte_frame_parms.mode1_flag = (phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output[4] == 1);
    openair_daq_vars.dlsch_transmission_mode = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output[4];
  }
  else {
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq++;
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors++;
  }
  
  if (mac_xface->frame % 100 == 0) {
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_fer = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors - phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_last;
    phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_last = phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors;
  }
  
  
  if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 100)) {
    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH errors = %d, consecutive errors = %d!\n",
	      mac_xface->frame, last_slot, phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors, phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq);
    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH received frame=%d, transmission mode=%d!\n",
	      mac_xface->frame, last_slot,*((unsigned int*) phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output),phy_vars_ue->lte_ue_pbch_vars[eNb_id]->decoded_output[4]);
    
    
    if (phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq>20) {
      msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH consecutive errors > 20, going out of sync!\n",mac_xface->frame, last_slot);
      openair_daq_vars.mode = openair_NOT_SYNCHED;
      phy_vars_ue->UE_mode = NOT_SYNCHED;
      openair_daq_vars.sync_state=0;
#ifdef CBMIMO1

      openair_dma(0,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
#endif //CBMIMO1
      mac_xface->frame = -1;
      openair_daq_vars.synch_wait_cnt=0;
      openair_daq_vars.sched_cnt=-1;
      
      
      phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq=0;
      phy_vars_ue->lte_ue_pbch_vars[eNb_id]->pdu_errors=0;
      
      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors = 0;
      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_missed = 0;
      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_false  = 0;    
      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_received = 0;    
      
      phy_vars_ue->dlsch_errors = 0;
      phy_vars_ue->dlsch_errors_last = 0;
      phy_vars_ue->dlsch_received = 0;
      phy_vars_ue->dlsch_received_last = 0;
      phy_vars_ue->dlsch_fer = 0;
      phy_vars_ue->dlsch_cntl_errors = 0;
      phy_vars_ue->dlsch_ra_errors = 0;

    }
  }
}

int lte_ue_pdcch_procedures(int eNb_id,unsigned char last_slot, PHY_VARS_UE *phy_vars_ue) {	

  unsigned int dci_cnt, i;

#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): DCI decoding\n",mac_xface->frame,last_slot,last_slot>>1);
#endif
  
  //  write_output("UE_rxsigF0.m","UE_rxsF0", &phy_vars_ue->lte_ue_common_vars.rxdataF[0],512*12*2,2,1);
  //  write_output("UE_rxsigF1.m","UE_rxsF1", &phy_vars_ue->lte_ue_common_vars.rxdataF[1],512*12*2,2,1);
  
  rx_pdcch(&phy_vars_ue->lte_ue_common_vars,
	   phy_vars_ue->lte_ue_pdcch_vars,
	   &phy_vars_ue->lte_frame_parms,
	   eNb_id,
	   2,
	   (phy_vars_ue->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI,
	   phy_vars_ue->is_secondary_ue); 

  //  debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): DCI decoding\n",mac_xface->frame,last_slot,last_slot>>1);


  dci_cnt = dci_decoding_procedure(phy_vars_ue->lte_ue_pdcch_vars,
				   dci_alloc_rx,
				   eNb_id,
				   &phy_vars_ue->lte_frame_parms,
				   SI_RNTI,RA_RNTI);

  //  debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): DCI decoding\n",mac_xface->frame,last_slot,last_slot>>1);

  phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_received += dci_cnt;

#ifdef DIAG_PHY
  if (last_slot==18)
    debug_msg("[PHY_PROCEDURES_LTE][DIAG] Frame %d, slot %d: PDCCH: DCI errors %d, DCI received %d, DCI missed %d, DCI False Detection %d \n",
	      mac_xface->frame,last_slot,
	      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors,
	      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_received,
	      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_missed,
	      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_false);
#endif // DIAG_PHY
  
#ifdef EMOS
  emos_dump_UE.dci_cnt[last_slot>>1] = dci_cnt;
#endif

#ifdef DIAG_PHY
  if (phy_vars_ue->UE_mode == PUSCH)
    if (dci_cnt > 2) {
      msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: received %d>2 DCI!\n",mac_xface->frame,last_slot>>1,dci_cnt);
      return(-1);
    }
#endif

#ifdef DEBUG_PHY
  debug_msg("[PHY PROCEDURES UE] subframe %d: dci_cnt %d\n",last_slot>>1,dci_cnt);
#endif
  for (i=0;i<dci_cnt;i++){

    if ((phy_vars_ue->UE_mode != PRACH) && (dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti) && (dci_alloc_rx[i].format == format2_2A_M10PRB)) {
#ifdef DEBUG_PHY
    debug_msg("[PHY PROCEDURES UE] subframe %d: Found rnti %x, format %d\n",last_slot>>1,dci_alloc_rx[i].rnti,
	      dci_alloc_rx[i].format);
#endif      
#ifdef DIAG_PHY
      if ((phy_vars_ue->UE_mode == PUSCH) && ((last_slot>>1) != 6)) {
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received C_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_false++;
	return(-1);
	
      }
#endif

      
      if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					    (DCI2_5MHz_2A_M10PRB_TDD_t *)&dci_alloc_rx[i].dci_pdu,
					    phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti,
					    format2_2A_M10PRB,
					    phy_vars_ue->dlsch_ue,
					    &phy_vars_ue->lte_frame_parms,
					    SI_RNTI,
					    RA_RNTI,
					    P_RNTI)==0) {
	phy_vars_ue->dlsch_ue_active = 1;
	phy_vars_ue->dlsch_received++;
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Generated UE DLSCH C_RNTI format 2_2A_M10PRB\n");
#endif    
      }
    }
    else if ((phy_vars_ue->UE_mode != PRACH) && (dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti) && (dci_alloc_rx[i].format == format1A)) {
#ifdef DEBUG_PHY
    debug_msg("[PHY PROCEDURES UE] subframe %d: Found rnti %x, format 1A, dci_cnt %d\n",last_slot>>1,dci_alloc_rx[i].rnti,i);
#endif      
#ifdef DIAG_PHY
      if ((phy_vars_ue->UE_mode == PUSCH) && ((last_slot>>1) != 7)) {
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received C_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_false++;
	return(-1);
      }
#endif

      if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					    (DCI1A_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					    phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti,
					    format1A,
					    &phy_vars_ue->dlsch_ue_1A,
					    &phy_vars_ue->lte_frame_parms,
					    SI_RNTI,
					    RA_RNTI,
					    P_RNTI)==0) {
	phy_vars_ue->dlsch_ue_1A_active = 1;
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Generated UE DLSCH C_RNTI 1A\n");
#endif    
      }
    }
    else if ((dci_alloc_rx[i].rnti == SI_RNTI) && (dci_alloc_rx[i].format == format1A)) {
#ifdef DEBUG_PHY
    debug_msg("[PHY PROCEDURES UE] subframe %d: Found rnti %x, format 1A, dci_cnt %d\n",last_slot>>1,dci_alloc_rx[i].rnti,i);
#endif      
#ifdef DIAG_PHY
      if ((phy_vars_ue->UE_mode == PUSCH) && ((last_slot>>1) != 0)) {
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received SI_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_false++;
	return(-1);
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
    else if ((dci_alloc_rx[i].rnti == RA_RNTI) && (dci_alloc_rx[i].format == format1A)) {
#ifdef DEBUG_PHY
    debug_msg("[PHY PROCEDURES UE] subframe %d: Found rnti %x, format 1A, dci_cnt %d\n",last_slot>>1,dci_alloc_rx[i].rnti,i);
#endif      
#ifdef DIAG_PHY
      if ((phy_vars_ue->UE_mode == PUSCH) && ((last_slot>>1) != 7)) {
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received RA_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	return(-1);
      }
#endif

      if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					    (DCI1A_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					    RA_RNTI,
					    format1A,
					    &phy_vars_ue->dlsch_ue_ra, 
					    &phy_vars_ue->lte_frame_parms,
					    SI_RNTI,
					    RA_RNTI,
					    P_RNTI)==0) {
	phy_vars_ue->dlsch_ue_ra_active = 1;
#ifdef DEBUG_PHY
	msg("[PHY_PROCEDURES_LTE] Generate UE DLSCH RA_RNTI format 1A, rb_alloc %x, phy_vars_ue->dlsch_ue_ra %p\n",
	    phy_vars_ue->dlsch_ue_ra->rb_alloc[0],phy_vars_ue->dlsch_ue_ra);
#endif
      }
    }
    else if ((phy_vars_ue->UE_mode != PRACH) && (dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti) && (dci_alloc_rx[i].format == format0)) {
#ifdef DIAG_PHY
      if ((phy_vars_ue->UE_mode == PUSCH) && ((last_slot>>1) != 9)) {
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received C_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_false++;
	return(-1);
      }
#endif

      phy_vars_ue->ulsch_no_allocation_counter = 0;

      generate_ue_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti,
					last_slot>>1,
					format0,
					phy_vars_ue->ulsch_ue[0],
					phy_vars_ue->dlsch_ue,
					&phy_vars_ue->PHY_measurements,
					&phy_vars_ue->lte_frame_parms,
					SI_RNTI,
					RA_RNTI,
					P_RNTI,
					eNb_id,
					phy_vars_ue->current_dlsch_cqi);
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Generate UE ULSCH C_RNTI format 0 (subframe %d)\n",last_slot>>1);
#endif
    }
    else if ((dci_alloc_rx[i].rnti == RA_RNTI) && (dci_alloc_rx[i].format == format0)) {
#ifdef DEBUG_PHY
    debug_msg("[PHY PROCEDURES UE] subframe %d: Found rnti %x, format 0, dci_cnt %d\n",last_slot>>1,dci_alloc_rx[i].rnti,i);
#endif      
#ifdef DIAG_PHY
      if (phy_vars_ue->UE_mode == PUSCH) {
	msg("[PHY PROCEDURES UE][DIAG] frame %d, subframe %d: should not have received RA_RNTI!\n",mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_false++;
	return(-1);
      }
#endif
      /*
	generate_ue_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
	RA_RNTI,
	last_slot>>1,
	format0,
	ulsch_ue[eNb_id], 
	&phy_vars_ue->lte_frame_parms,
	SI_RNTI,
	RA_RNTI,
	P_RNTI,
	phy_vars_ue->current_dlsch_cqi);

	printf("[PHY_PROCEDURES_LTE] Generate UE ULSCH C_RNTI format 0 (subframe %d)\n",last_slot>>1);
      */
    }
    else {
      msg("[PHY PROCEDURES UE] frame %d, subframe %d: received DCI with RNTI=%x and format %d!\n",mac_xface->frame,last_slot>>1,dci_alloc_rx[i].rnti,dci_alloc_rx[i].format);
#ifdef DIAG_PHY
      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_errors++;
      phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->dci_false++;
      return(-1);
#endif
    }

  }
  return(0);
}


int phy_procedures_UE_RX(unsigned char last_slot, PHY_VARS_UE *phy_vars_ue) {

  unsigned short l,m,n_symb;
  int eNb_id = 0, eNb_id_i = 1;
  unsigned char dual_stream_UE = 0;
  int ret;
  unsigned char harq_pid;
  int timing_advance;
  unsigned short dummy;
  unsigned char card_id;
  unsigned int dlsch_buffer_length;

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

      phy_vars_ue->ulsch_no_allocation_counter++;

      if (phy_vars_ue->ulsch_no_allocation_counter == 10) {
	phy_vars_ue->UE_mode = PRACH;
	phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti = 0x1234; // not always 1234
      }
       
      lte_ue_pbch_procedures(eNb_id,last_slot,phy_vars_ue);

      if (phy_vars_ue->UE_mode == RA_RESPONSE) {
	phy_vars_ue->rag_timer--;
	//	msg("[UE RAR] frame %d: rag_timer %d\n",mac_xface->frame,phy_vars_ue->rag_timer);

	if (phy_vars_ue->rag_timer == 0) {
	  phy_vars_ue->UE_mode = PRACH;
	  phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti = 0x1234; // not always 1234
	}
      }
    }

	
    // process last DLSCH symbols + invoke decoding
    if (((last_slot%2)==0) && (l==0)) {

      if ( (phy_vars_ue->dlsch_ue_active == 1) && (phy_vars_ue->dlsch_ue_cntl_active == 1))
	msg("[PHY_PROCEDURES_LTE] WARNING: dlsch_ue and dlsch_ue_cntl active, but data structures can only handle one at a time\n");

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
		   phy_vars_ue->dlsch_ue,
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
	
	phy_vars_ue->dlsch_ue_active = 0;
      
#ifndef USER_MODE
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Scheduling DLSCH decoding\n",mac_xface->frame,last_slot);

	harq_pid = phy_vars_ue->dlsch_ue[0]->current_harq_pid;
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
	if (phy_vars_ue->dlsch_ue[0]) {
	  ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->llr[0],
			       &phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->dlsch_ue[0],
			       ((last_slot>>1)-1)%10);
	  
	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    phy_vars_ue->dlsch_errors++;
#ifdef USER_MODE
	    if (mac_xface->frame > 10) {
	      printf("DLSCH in error, dumping\n");
	      //	      dump_dlsch();
	      //	      exit(-1);
	    }
#endif
	  }
	}

	if (mac_xface->frame % 100 == 0) {
	  if ((phy_vars_ue->dlsch_received - phy_vars_ue->dlsch_received_last) != 0) 
	    phy_vars_ue->dlsch_fer = (100*(phy_vars_ue->dlsch_errors - phy_vars_ue->dlsch_errors_last))/(phy_vars_ue->dlsch_received - phy_vars_ue->dlsch_received_last);
	  phy_vars_ue->dlsch_errors_last = phy_vars_ue->dlsch_errors;
	  phy_vars_ue->dlsch_received_last = phy_vars_ue->dlsch_received;
	  if ((phy_vars_ue->dlsch_fer > 10) && (phy_vars_ue->current_dlsch_cqi>0))
	    phy_vars_ue->current_dlsch_cqi--;
#ifndef USER_MODE
	  if ((phy_vars_ue->dlsch_fer < 4) && (phy_vars_ue->current_dlsch_cqi<8))
	    phy_vars_ue->current_dlsch_cqi++;
#else
	  if ((phy_vars_ue->dlsch_fer < 4) && (phy_vars_ue->current_dlsch_cqi<16))
	    phy_vars_ue->current_dlsch_cqi++;
#endif
	}

	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_decoding ret %d (mcs %d, TBS %d)\n",
		  mac_xface->frame,last_slot,ret,
		  phy_vars_ue->dlsch_ue[0]->harq_processes[0]->mcs,
		  phy_vars_ue->dlsch_ue[0]->harq_processes[0]->TBS);
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_errors %d, dlsch_received %d, dlsch_fer %d, current_dlsch_cqi %d\n",
		  mac_xface->frame,last_slot,
		  phy_vars_ue->dlsch_errors,
		  phy_vars_ue->dlsch_received,
		  phy_vars_ue->dlsch_fer,
		  phy_vars_ue->current_dlsch_cqi);
#endif
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
	
	//	write_output("dlsch_ra_llr.m","llr",phy_vars_ue->lte_ue_dlsch_vars_ra[eNb_id]->llr[0],40,1,0);

	phy_vars_ue->dlsch_ue_cntl_active = 0;
      
	if (mac_xface->frame < phy_vars_ue->dlsch_cntl_errors)
	  phy_vars_ue->dlsch_cntl_errors=0;

	if (phy_vars_ue->dlsch_ue_cntl) {
	  ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars_cntl[eNb_id]->llr[0],
			       &phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->dlsch_ue_cntl,
			       ((last_slot>>1)-1)%10);
	  
	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    phy_vars_ue->dlsch_cntl_errors++;
	  }
	  else {
	    timing_advance = process_rar(phy_vars_ue->dlsch_ue_cntl->harq_processes[0]->b,&dummy);
	    if ((timing_advance>>10) & 1) //it is negative
	      timing_advance = timing_advance - (1<<11);
	    
	    if (openair_daq_vars.manual_timing_advance == 0) {
	      if ( (mac_xface->frame % 100) == 0) {
		if ((timing_advance > 3) || (timing_advance < -3) )
		  openair_daq_vars.timing_advance = max(0,(int)openair_daq_vars.timing_advance+timing_advance*4);
		
#ifdef CBMIMO1
		for (card_id=0;card_id<number_of_cards;card_id++)
		  pci_interface[card_id]->timing_advance = openair_daq_vars.timing_advance;
#endif
	      
	      }
	    }
	    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, subframe %d, received (from cntl) timing_advance = %d (%d)\n",mac_xface->frame,((last_slot>>1)-1)%10, timing_advance, openair_daq_vars.timing_advance);
	    dlsch_buffer_length = phy_vars_ue->dlsch_ue_cntl->harq_processes[0]->TBS/8;
	    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, subframe %d, received (from cntl) DLSCH PMI %x (saved %x)\n",mac_xface->frame,((last_slot>>1)-1)%10,
		      pmi2hex_2Ar1(*((unsigned short*)&phy_vars_ue->dlsch_ue_cntl->harq_processes[0]->b[dlsch_buffer_length-2])),
		      pmi2hex_2Ar1(phy_vars_ue->dlsch_ue[0]->pmi_alloc));
	  }
	}   
	
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_decoding (cntl) ret %d (%d errors)\n",
		  mac_xface->frame,last_slot,ret,phy_vars_ue->dlsch_cntl_errors);

      }
    

      if (phy_vars_ue->dlsch_ue_ra_active == 1) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (RA) demod symbols 10,11,12\n",mac_xface->frame,last_slot);
#endif
      
	// process symbols 10,11,12 and trigger DLSCH decoding
	for (m=(11-phy_vars_ue->lte_frame_parms.Ncp*2+1);m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_ra,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_ra,
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
	

	phy_vars_ue->dlsch_ue_ra_active = 0;
      
	if (mac_xface->frame < phy_vars_ue->dlsch_ra_errors)
	  phy_vars_ue->dlsch_ra_errors=0;

	if (phy_vars_ue->dlsch_ue_ra) {
	  ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars_ra[eNb_id]->llr[0],
			       &phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->dlsch_ue_ra,
			       ((last_slot>>1)-1)%10);
	  
	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    phy_vars_ue->dlsch_ra_errors++;
	  }
	  else {
	    debug_msg("[PHY_PROCEDURES_LTE] Received RAR in frame %d, subframe %d\n",mac_xface->frame,((last_slot>>1)-1)%10);

	    //#ifdef OPENAIR2
	    if (phy_vars_ue->UE_mode != PUSCH) {
	      timing_advance = process_rar(phy_vars_ue->dlsch_ue_ra->harq_processes[0]->b,&phy_vars_ue->lte_ue_pdcch_vars[eNb_id]->crnti);

	      if ((timing_advance>>10) & 1) //it is negative
		timing_advance = timing_advance - (1<<11);

	      if (openair_daq_vars.manual_timing_advance == 0) {
		openair_daq_vars.timing_advance = max(0,TIMING_ADVANCE_INIT + timing_advance*4);
		
#ifdef CBMIMO1
		for (card_id=0;card_id<number_of_cards;card_id++)
		  pci_interface[card_id]->timing_advance = openair_daq_vars.timing_advance;
#endif
	      }

	      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, subframe %d, received (rar) timing_advance = %d (%d)\n",mac_xface->frame,((last_slot>>1)-1)%10, timing_advance,openair_daq_vars.timing_advance);

	      phy_vars_ue->ulsch_ue_rag_active=1;
	      get_rag_alloc(phy_vars_ue->lte_frame_parms.tdd_config,
			    ((last_slot>>1)-1)%10,
			    mac_xface->frame,
			    &phy_vars_ue->ulsch_ue_rag_frame,
			    &phy_vars_ue->ulsch_ue_rag_subframe);
	      phy_vars_ue->UE_mode = RA_RESPONSE;
	      phy_vars_ue->rag_timer = 100;
	      phy_vars_ue->ulsch_ue[0]->power_offset = 6;
	    }
	    //#endif //OPENAIR2
	  }
	   
	
	  if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 100)) {
	    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_decoding (RA) ret %d (%d errors)\n",
		mac_xface->frame,last_slot,ret,phy_vars_ue->dlsch_ra_errors);
	  }
	}
      }

      if (phy_vars_ue->dlsch_ue_1A_active == 1) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (1A) demod symbols 10,11,12\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 10,11,12 and trigger DLSCH decoding
	for (m=(11-phy_vars_ue->lte_frame_parms.Ncp*2+1);m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_cntl,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_1A,
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
	
	//write_output("dlsch_cntl_llr.m","llr",phy_vars_ue->lte_ue_dlsch_vars[eNb_id]->llr[0],40,1,0);

	phy_vars_ue->dlsch_ue_1A_active = 0;
      
	if (phy_vars_ue->dlsch_ue_1A) {
	  ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars_1A[eNb_id]->llr[0],
			       &phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->dlsch_ue_1A,
			       ((last_slot>>1)-1)%10);
	  
	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    phy_vars_ue->UE_mode = PRACH;
#ifdef DEBUG_PHY
	    debug_msg("[PHY PROCEDURES_LTE] Frame %d, slot %d: Did not decode DLSCH (format 1A) \n",mac_xface->frame,last_slot);
#endif
	  }
	  else {
	    phy_vars_ue->UE_mode = PUSCH;
#ifdef DEBUG_PHY
	    debug_msg("[PHY PROCEDURES_LTE] Frame %d, slot %d: Decoded DLSCH (format 1A) Setting UE mode to ULSCH (%d) RAR (%d) NOT_SYNCHED %d)\n",mac_xface->frame,last_slot,PUSCH,RA_RESPONSE,NOT_SYNCHED);
#endif
	  }
	}   
	
      }
    }

    if (((last_slot%2)==0) && (l==(4-phy_vars_ue->lte_frame_parms.Ncp)))  {

      debug_msg("[PHY PROCEDURES_LTE] Frame %d, slot %d: Calling pdcch procedures\n",mac_xface->frame,last_slot);
      if (lte_ue_pdcch_procedures(eNb_id,last_slot,phy_vars_ue) == -1) {
	msg("[PHY PROCEDURES_LTE] Frame %d, slot %d: Error in pdcch procedures\n",mac_xface->frame,last_slot);
	return(-1);
	//exit_openair=1;
      }

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
		   phy_vars_ue->dlsch_ue,
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
      
      
      if (phy_vars_ue->dlsch_ue_ra_active == 1) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (RA) demod symbols 0,1,2\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 0,1,2
	for (m=phy_vars_ue->lte_frame_parms.first_dlsch_symbol;m<(4-phy_vars_ue->lte_frame_parms.Ncp);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_ra,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_ra,
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
		   phy_vars_ue->dlsch_ue,
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
      if (phy_vars_ue->dlsch_ue_ra_active == 1)  {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (RA) demod symbols 3,4,5\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 3,4,5
	for (m=4-phy_vars_ue->lte_frame_parms.Ncp+1;m<(phy_vars_ue->lte_frame_parms.symbols_per_tti/2);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_ra,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_ra,
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
      }
      
      if (phy_vars_ue->dlsch_ue_1A_active == 1)  {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (1A) demod symbols 3,4,5\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 3,4,5
	for (m=4-phy_vars_ue->lte_frame_parms.Ncp+1;m<(phy_vars_ue->lte_frame_parms.symbols_per_tti/2);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_1A,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_1A,
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
		   phy_vars_ue->dlsch_ue,
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
    
      if(phy_vars_ue->dlsch_ue_ra_active == 1) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (RA) demod symbols 6,7,8\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 6,7,8
	for (m=(phy_vars_ue->lte_frame_parms.symbols_per_tti/2)+1;m<(11-phy_vars_ue->lte_frame_parms.Ncp*2);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_ra,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_ra,
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
      }

      if(phy_vars_ue->dlsch_ue_1A_active == 1) {
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (1A) demod symbols 6,7,8\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 6,7,8
	for (m=(phy_vars_ue->lte_frame_parms.symbols_per_tti/2)+1;m<(11-phy_vars_ue->lte_frame_parms.Ncp*2);m++)
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars_1A,
		   &phy_vars_ue->lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &phy_vars_ue->dlsch_ue_1A,
		   m,
		   dual_stream_UE,
		   &phy_vars_ue->PHY_measurements,
		   phy_vars_ue->is_secondary_ue);
      }
    }
  }
#ifdef EMOS
  phy_procedures_emos_UE_RX(last_slot);
#endif

  return (0);
}


