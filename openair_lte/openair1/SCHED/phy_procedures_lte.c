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

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif

#ifdef USER_MODE
#define DEBUG_PHY
#endif

#define NS_PER_SLOT 500000

extern inline unsigned int taus(void);

unsigned char dlsch_input_buffer[2700] __attribute__ ((aligned(16)));

extern int dlsch_instance_cnt[8];
extern pthread_mutex_t dlsch_mutex[8];
/// Condition variable for dlsch thread
extern pthread_cond_t dlsch_cond[8];

/*
//#define NB_RB 6
//#define RBmask0 0x0001F800
#define NB_RB 12
#define RBmask0 0x00fc00fc
#define RBmask1 0x0
#define RBmask2 0x0
#define RBmask3 0x0
*/

// maybe these definitions should go somewhere else?
extern DCI0_5MHz_TDD0_t          UL_alloc_pdu;
extern DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
extern DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
extern DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;


unsigned int dci_cnt;
int dlsch_errors = 0;


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


int phy_procedures_lte_new(unsigned char last_slot, unsigned char next_slot) {


  if (mac_xface->is_cluster_head == 0) {
    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_UL) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_UE_TX(%d)\n",next_slot);
      //phy_procedures_UE_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_DL) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_UE_RX(%d)\n",last_slot);
      //phy_procedures_UE_RX(last_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_S) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_UE_S_TX(%d)\n",next_slot);
      //phy_procedures_UE_S_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_S) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_UE_S_RX(%d)\n",last_slot);
      //phy_procedures_UE_S_RX(last_slot);
    }
  }
  else { //eNB
    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_DL) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_eNB_TX(%d)\n",next_slot);
      //phy_procedures_eNB_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_UL) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_eNB_RX(%d)\n",last_slot);
      //phy_procedures_eNB_RX(last_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_S) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_eNB_S_TX(%d)\n",next_slot);
      //phy_procedures_eNB_S_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_S) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_eNB_S_RX(%d)\n",last_slot);
      //phy_procedures_eNB_S_RX(last_slot);
    }
  }
}


int phy_procedures_lte(unsigned char last_slot, unsigned char next_slot) {
  /*
  int ret[2];
  int time_in,time_out;
  int diff;
  int timing_offset;		
  */
  int i,k,l,m,aa,aarx; 

#ifndef USER_MODE
  RTIME  now;            
#endif
  int time_in,time_out;

  // PBCH variables
  unsigned char pbch_pdu[PBCH_PDU_SIZE];
  int pbch_error;
  int ret;

  // DLSCH variables
  unsigned char mod_order[2]={2,2};
  unsigned int rb_alloc[4];
  unsigned int N_rb_alloc;
  MIMO_mode_t mimo_mode = ALAMOUTI;
  int eNb_id = 0, eNb_id_i = 1;
  unsigned char dual_stream_UE = 0;

  unsigned short input_buffer_length;
  unsigned int coded_bits_per_codeword,nsymb;
  int rate_num=1,rate_den=3;
  int subframe_offset;
  int rx_power;

  DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];

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

  /*  
  rb_alloc[0] = RBmask0; // RBs 0-31
  rb_alloc[1] = RBmask1;  // RBs 32-63
  rb_alloc[2] = RBmask2;  // RBs 64-95
  rb_alloc[3] = RBmask3;  // RBs 96-109
  */

#ifdef USER_MODE
  if (mac_xface->frame%1000 == 0)
    msg("[PHY_PROCEDURES_LTE] frame %d, last_slot=%d, next_slot=%d: Calling phy_procedures for %s\n",
	mac_xface->frame, last_slot, next_slot, mac_xface->is_cluster_head ? "clusterhead" : "node");
#endif

  if (mac_xface->is_cluster_head == 0) {

    // RX processing of symbols in last_slot
    for (l=0;l<lte_frame_parms->symbols_per_tti/2;l++) {
	
      if (((openair_daq_vars.tdd==1) && (last_slot<10)) || (openair_daq_vars.tdd == 0)) {
	slot_fep(lte_frame_parms,
		 lte_ue_common_vars,
		 l,
		 last_slot,
#ifdef USER_MODE
		 0);
#else
		 1);
#endif
      }
	
#ifdef EMOS
      // first slot in frame is special
      if (((last_slot==0) || (last_slot==1)) && ((l==0) || (l==4-lte_frame_parms->Ncp))) {
	
	for (eNb_id=0; eNb_id<3; eNb_id++) 
	  for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++)
	    lte_dl_channel_estimation_emos(emos_dump.channel[eNb_id],
					   lte_ue_common_vars->rxdataF,
					   lte_frame_parms,
					   last_slot,
					   aa,
					   l,
					   eNb_id);
      }
#endif
      if ((last_slot==0) && (l==4-lte_frame_parms->Ncp)) {
	// Measurements

	lte_ue_measurements(lte_ue_common_vars,
			    lte_frame_parms,
			    &PHY_vars->PHY_measurements,
			    (last_slot>>1)*lte_frame_parms->symbols_per_tti*lte_frame_parms->ofdm_symbol_size,
			    0,
			    1);

	// AGC
	if (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON)
	  if (mac_xface->frame % 100 == 0)
	    phy_adjust_gain (0,16384,0);

	if (mac_xface->frame%100 == 0) {
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, freq_offset_filt = %d \n",mac_xface->frame, last_slot, lte_ue_common_vars->freq_offset);

	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), RX gain %d dB\n",
	      mac_xface->frame, last_slot,
	      PHY_vars->PHY_measurements.rx_rssi_dBm[0], 
	      PHY_vars->PHY_measurements.rx_power_dB[0][0],
	      PHY_vars->PHY_measurements.rx_power_dB[0][1],
	      PHY_vars->PHY_measurements.rx_power[0][0],
	      PHY_vars->PHY_measurements.rx_power[0][1],
	      PHY_vars->rx_vars[0].rx_total_gain_dB);

	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, N0 digital (%d, %d) dB, linear (%d, %d)\n",
	      mac_xface->frame, last_slot,
	      PHY_vars->PHY_measurements.n0_power_dB[0],
	      PHY_vars->PHY_measurements.n0_power_dB[1],
	      PHY_vars->PHY_measurements.n0_power[0],
	      PHY_vars->PHY_measurements.n0_power[1]);
	}
      }
    
      if ((last_slot==1) && (l==4-lte_frame_parms->Ncp)) {

	lte_adjust_synch(lte_frame_parms,
			 lte_ue_common_vars,
			 1,
			 16384);

	pbch_error = rx_pbch(lte_ue_common_vars,
			     lte_ue_pbch_vars[eNb_id],
			     lte_frame_parms,
			     eNb_id,
			     SISO);
	if (pbch_error) {
	  lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq = 0;
#ifdef EMOS
	  PHY_vars->PHY_measurements.frame_tx = *((unsigned int*) lte_ue_pbch_vars->decoded_output);
#endif
	}
	else {
	  lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq++;
	  lte_ue_pbch_vars[eNb_id]->pdu_errors++;
	}

	if (mac_xface->frame % 100 == 0) {
	  lte_ue_pbch_vars[eNb_id]->pdu_fer = lte_ue_pbch_vars[eNb_id]->pdu_errors - lte_ue_pbch_vars[eNb_id]->pdu_errors_last;
	  lte_ue_pbch_vars[eNb_id]->pdu_errors_last = lte_ue_pbch_vars[eNb_id]->pdu_errors;
	}

	
	if (mac_xface->frame % 100 == 0) {
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH errors = %d, consecutive errors = %d!\n",
	      mac_xface->frame, last_slot, lte_ue_pbch_vars[eNb_id]->pdu_errors, lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq);
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH received frame = %d!\n",
	      mac_xface->frame, last_slot,*((unsigned int*) lte_ue_pbch_vars[eNb_id]->decoded_output));
	}

	if (lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq>20) {
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH consecutive errors > 20, going out of sync!\n",mac_xface->frame, last_slot);
	  openair_daq_vars.mode = openair_NOT_SYNCHED;
	  openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
	  openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
	  rt_sleep(nano2count(NS_PER_SLOT*SLOTS_PER_FRAME));
#endif //CBMIMO1
	  mac_xface->frame = -1;
	  openair_daq_vars.synch_wait_cnt=0;
	  openair_daq_vars.sched_cnt=-1;
	  
	  lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq=0;
	  lte_ue_pbch_vars[eNb_id]->pdu_errors=0;
	  
	}

      }

      // process DLSCH slots 
      // slots 4 6 8 10 (and 12 14 18 0 if FDD)
      if (((openair_daq_vars.tdd==1) && (last_slot < 10) && (last_slot >= 0)) || 
	  (openair_daq_vars.tdd==0)) {

	if  ((last_slot > 2) || ((last_slot==0) && (mac_xface->frame>0))) {

	  if (((last_slot%2)==0) && (l==0)) {

#ifdef DEBUG_PHY
	    if ((mac_xface->frame % 100) == 0)
	      msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 10,11,12\n",mac_xface->frame,last_slot);
#endif

	    // process symbols 10,11,12 and trigger DLSCH decoding
	    for (m=(11-lte_frame_parms->Ncp*2+1);m<lte_frame_parms->symbols_per_tti;m++)
	      rx_dlsch(lte_ue_common_vars,
		       lte_ue_dlsch_vars,
		       lte_frame_parms,
		       eNb_id,
		       eNb_id_i,
		       dlsch_ue,
		       m,
		       dual_stream_UE);
	    

#ifndef USER_MODE
	      if ((mac_xface->frame % 100) == 0)
		msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Scheduling DLSCH decoding\n",mac_xface->frame,last_slot);
	      
	      if (pthread_mutex_lock (&dlsch_mutex[0]) != 0) {               // Signal MAC_PHY Scheduler
		msg("[PHY_PROCEDURES_LTE] ERROR pthread_mutex_lock\n");     // lock before accessing shared resource
		return(-1);
	      }
	      dlsch_instance_cnt[0]++;
	      pthread_mutex_unlock (&dlsch_mutex[0]);
	      
	      if (dlsch_instance_cnt[0] == 0) {
		if (pthread_cond_signal(&dlsch_cond[0]) != 0) {
		  msg("[PHY_PROCEDURES_LTE] ERROR pthread_cond_signal for dlsch_cond[0]\n");
		  return(-1);
		}
	      }
	      else {
		msg("[PHY_PROCEDURES_LTE] DLSCH thread busy!!!\n");
		return(-1);
	      }

#else
	      //time_in = openair_get_mbox();
	      
	      if (mac_xface->frame < dlsch_errors)
		dlsch_errors=0;
	      
	      if (dlsch_ue[0]) {
		ret = dlsch_decoding(lte_ue_dlsch_vars[eNb_id]->llr[0],
				     lte_frame_parms,
				     dlsch_ue[0]);
		
		if (ret == (1+MAX_TURBO_ITERATIONS)) {
		  dlsch_errors++;
		}
	      } 
	      
	      //time_out = openair_get_mbox();
	      
	      if ((mac_xface->frame % 100) == 0) {
		msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_decoding in %d, out %d, ret %d (%d errors)\n",
		    mac_xface->frame,last_slot,time_in,time_out,ret,dlsch_errors);
	      }
#endif
	    
	  }
	}
	if (((last_slot%2)==0) && (l==(4-lte_frame_parms->Ncp)))  {
	    
#ifdef DEBUG_PHY
	  if ((mac_xface->frame % 100) == 0)
	    msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DCI decoding\n",mac_xface->frame,last_slot);
#endif
	  
	  write_output("UE_rxsigF0.m","UE_rxsF0", lte_ue_common_vars->rxdataF[0],512*12*2,2,1);
	  write_output("UE_rxsigF1.m","UE_rxsF1", lte_ue_common_vars->rxdataF[1],512*12*2,2,1);
	  
	  rx_pdcch(lte_ue_common_vars,
		   lte_ue_pdcch_vars,
		   lte_frame_parms,
		   eNb_id,
		   2,
		   (lte_frame_parms->nb_antennas_tx == 1) ? SISO : ALAMOUTI); //this needs to be changed
	  
	  dci_cnt = dci_decoding_procedure(lte_ue_pdcch_vars,dci_alloc_rx,eNb_id,lte_frame_parms,SI_RNTI,RA_RNTI,C_RNTI);
	  for (i=0;i<dci_cnt;i++)
	    if ((dci_alloc_rx[i].rnti == C_RNTI) && (dci_alloc_rx[i].format == format2_2A_M10PRB))
	      generate_ue_dlsch_params_from_dci((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci_alloc_rx[i].dci_pdu,
						C_RNTI,
						format2_2A_M10PRB,
						dlsch_ue,
						lte_frame_parms,
						SI_RNTI,
						RA_RNTI,
						P_RNTI);
	    else if ((dci_alloc_rx[i].rnti == SI_RNTI) && (dci_alloc_rx[i].format == format1A))
	      generate_ue_dlsch_params_from_dci((DCI1A_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
						SI_RNTI,
						format1A,
						&dlsch_ue_cntl, 
						lte_frame_parms,
						SI_RNTI,
						RA_RNTI,
						P_RNTI);
	  

#ifdef DEBUG_PHY
	  if ((mac_xface->frame % 100) == 0)
	    msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 0,1,2\n",mac_xface->frame,last_slot);
#endif
	  
	  // process symbols 0,1,2
	  if (dci_cnt)
	    for (m=lte_frame_parms->first_dlsch_symbol;m<(4-lte_frame_parms->Ncp);m++)
	      rx_dlsch(lte_ue_common_vars,
		       lte_ue_dlsch_vars,
		       lte_frame_parms,
		       eNb_id,
		       eNb_id_i,
		       dlsch_ue,
		       m,
		       dual_stream_UE);
	}
  
	if (((last_slot%2)==1) && (l==0)) {
	  
#ifdef DEBUG_PHY
	  if ((mac_xface->frame % 100) == 0)
	    msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 3,4,5\n",mac_xface->frame,last_slot);
#endif
	  
	  // process symbols 3,4,5
	  if (dci_cnt)
	    for (m=4-lte_frame_parms->Ncp+1;m<(lte_frame_parms->symbols_per_tti/2);m++)
	      rx_dlsch(lte_ue_common_vars,
		       lte_ue_dlsch_vars,
		       lte_frame_parms,
		       eNb_id,
		       eNb_id_i,
		       dlsch_ue,
		       m,
		       dual_stream_UE);
	  }
	  
	  if (((last_slot%2)==1) && (l==(4-lte_frame_parms->Ncp))) {
	    
#ifdef DEBUG_PHY
	    if ((mac_xface->frame % 100) == 0)
	      msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 6,7,8\n",mac_xface->frame,last_slot);
#endif

	    // process symbols 6,7,8
	    if(dci_cnt)
	      for (m=(lte_frame_parms->symbols_per_tti/2)+1;m<(11-lte_frame_parms->Ncp*2);m++)
		rx_dlsch(lte_ue_common_vars,
			 lte_ue_dlsch_vars,
			 lte_frame_parms,
			 eNb_id,
			 eNb_id_i,
			 dlsch_ue,
			 m,
			 dual_stream_UE);
	  }
      }
  }


    // TX processing
    if (((openair_daq_vars.tdd==1) && (next_slot>=10)) || (openair_daq_vars.tdd == 0)) {

      if (next_slot%2==0) {      
#ifdef DEBUG_PHY
	if ((mac_xface->frame % 100) == 0)
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generating SRS\n",mac_xface->frame,next_slot);
#endif

#ifdef IFFT_FPGA
	subframe_offset = (next_slot>>1)*lte_frame_parms->symbols_per_tti*lte_frame_parms->N_RB_UL*12;
#else
	subframe_offset = (next_slot>>1)*lte_frame_parms->symbols_per_tti*lte_frame_parms->ofdm_symbol_size;
#endif
	generate_srs_tx(lte_frame_parms,lte_ue_common_vars->txdataF[0],AMP,subframe_offset);
	generate_drs_puch(lte_frame_parms,lte_ue_common_vars->txdataF[0],AMP,subframe_offset,0,lte_frame_parms->N_RB_UL);

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

  else { // we are a CH

    // TX processing
    if (((openair_daq_vars.tdd==1) && (next_slot < 10)) || (openair_daq_vars.tdd==0)) {

      generate_pilots_slot(lte_eNB_common_vars->txdataF[eNb_id],
			   AMP,
			   lte_frame_parms,
			   eNb_id,
			   next_slot);
      
      
      if (next_slot == 0) {
	generate_pss(lte_eNB_common_vars->txdataF[eNb_id],
		     AMP,
		     lte_frame_parms,
		     eNb_id,
		     1);
      }

      if (next_slot == 1) {
	
#ifdef DEBUG_PHY
	if (mac_xface->frame%100 == 0)
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d: Calling generate_pbch for\n",mac_xface->frame, next_slot);
#endif
      
	*((unsigned int*) pbch_pdu) = mac_xface->frame;
	
	generate_pbch(lte_eNB_common_vars->txdataF[eNb_id],
		      AMP,
		      lte_frame_parms,
		      pbch_pdu);
      }
      
      if ((next_slot%2 == 0)) { 
	// fill all other frames with DLSCH

	
	///dci_alloc = ...;
	memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
	dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
	dci_alloc[0].L          = 3;
	dci_alloc[0].rnti       = C_RNTI;
	memcpy(&dci_alloc[1].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
	dci_alloc[1].dci_length = sizeof_DCI_0_5MHz_TDD_0_t;
	dci_alloc[1].L          = 3;
	dci_alloc[1].rnti       = C_RNTI;
	
#ifdef DEBUG_PHY
	if (mac_xface->frame%100 == 0)
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d: Calling generate_dci_top\n",mac_xface->frame, next_slot);
#endif

	generate_dci_top(2,
			 0,
			 dci_alloc,
			 0,
			 AMP,
			 lte_frame_parms,
			 lte_eNB_common_vars->txdataF[eNb_id],
			 next_slot/2);

	generate_eNb_dlsch_params_from_dci(&DLSCH_alloc_pdu2,
					   C_RNTI,
					   format2_2A_M10PRB,
					   dlsch_eNb,
					   lte_frame_parms,
					   SI_RNTI,
					   RA_RNTI,
					   P_RNTI);
      }

      if ((next_slot%2 == 0) && (next_slot > 1)) {
	input_buffer_length = dlsch_eNb[0]->harq_processes[0]->TBS/8;
	for (i=0;i<input_buffer_length;i++)
	  dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
	  
#ifdef DEBUG_PHY
	if (mac_xface->frame%100 == 0)
	  msg("[PHY_PROCEDURES_LTE] frame %d, slot %d: Calling generate_dlsch with input size = %d\n",mac_xface->frame, next_slot, input_buffer_length);
#endif
	
	dlsch_encoding(dlsch_input_buffer,
		       lte_frame_parms,
		       dlsch_eNb[0]);
	
	dlsch_modulation(lte_eNB_common_vars->txdataF[eNb_id],
			 AMP,
			 next_slot/2,
			 lte_frame_parms,
			 dlsch_eNb[0]);
	
	
	  /*
	    for (i=0;i<300;i++)
	    msg("lte_eNB_common_vars->txdataF[0][%d] = %d\n",i+300*12,lte_eNB_common_vars->txdataF[0][i+300*14]);
	    for (i=0;i<100;i++)
	    msg("dlsch_input_buffer[[%d] = %d\n",i,dlsch_input_buffer[i]);
	  */
      }
    }

    //RX processing
    if (((openair_daq_vars.tdd==1) && (last_slot>=10)) || (openair_daq_vars.tdd == 0)) {
      
      for (l=0;l<lte_frame_parms->symbols_per_tti/2;l++) {
	
	slot_fep_ul(lte_frame_parms,
		    lte_eNB_common_vars,
		    l,
		    last_slot,
		    eNb_id,
#ifdef USER_MODE
		    0);
#else
		    1);
#endif

      }

    /*
      if ((mac_xface->frame % 100) == 0)
	msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DRS channel estimation: N_rb_alloc = %d\n",mac_xface->frame,last_slot,N_rb_alloc );
    */

      if (last_slot == 11) {

	rx_power = 0;
	for (aarx=0; aarx<lte_frame_parms->nb_antennas_rx; aarx++) {
	  PHY_vars->PHY_measurements.rx_power[eNb_id][aarx] = 
	    signal_energy(lte_eNB_common_vars->rxdataF[eNb_id][aarx],
			  lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti);
	  PHY_vars->PHY_measurements.rx_power_dB[eNb_id][aarx] = dB_fixed(PHY_vars->PHY_measurements.rx_power[eNb_id][aarx]);
	  rx_power +=  PHY_vars->PHY_measurements.rx_power[eNb_id][aarx];
	}
	PHY_vars->PHY_measurements.rx_avg_power_dB[eNb_id] = dB_fixed(rx_power);

	/*
	// AGC
	if (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON)
	  if (mac_xface->frame % 100 == 0)
	    phy_adjust_gain (0,16384,0);
	*/

#ifdef DEBUG_PHY      
	if ((mac_xface->frame % 100) == 0)
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: SRS channel estimation: avg_power_dB = %d\n",mac_xface->frame,last_slot,PHY_vars->PHY_measurements.rx_avg_power_dB[eNb_id] );
#endif
      }
    }

  }
  
  return(0);
  
}


