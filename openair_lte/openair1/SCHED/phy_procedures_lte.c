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
unsigned char ulsch_input_buffer[2700] __attribute__ ((aligned(16)));

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


static char dlsch_ue_active = 0;
static char dlsch_ue_cntl_active = 0;
static char dlsch_eNb_active = 0;
static char dlsch_eNb_cntl_active = 0;

static char ulsch_ue_active = 0;
static char ulsch_eNb_active = 0;

int dlsch_errors = 0;

DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];

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

void phy_procedures_UE_TX(unsigned char next_slot) {
  
  unsigned short first_rb, nb_rb;
  unsigned char harq_pid;
  unsigned int input_buffer_length;
  unsigned int i;

  if (next_slot%2==0) {      
#ifdef DEBUG_PHY
    if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10))
      msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generating SRS\n",mac_xface->frame,next_slot);
#endif
    
    generate_srs_tx(lte_frame_parms,lte_ue_common_vars->txdataF[0],AMP,next_slot>>1);

    if (ulsch_ue_active == 1) {
      get_ack(lte_frame_parms->tdd_config,dlsch_ue[0]->harq_ack,(next_slot>>1),ulsch_ue[0]->o_ACK);

      printf("[PHY_PROCEDURES_LTE][UE_UL] subframe %d -> ACK %d, %d\n",(next_slot>>1),ulsch_ue[0]->o_ACK[0],ulsch_ue[0]->o_ACK[1]);
      
      harq_pid = subframe2harq_pid_tdd(lte_frame_parms->tdd_config,(next_slot>>1));
      first_rb = ulsch_ue[0]->harq_processes[harq_pid]->first_rb;
      nb_rb = ulsch_ue[0]->harq_processes[harq_pid]->nb_rb;
      
      generate_drs_puch(lte_frame_parms,lte_ue_common_vars->txdataF[0],AMP,next_slot>>1,first_rb,nb_rb);
      
      input_buffer_length = ulsch_ue[0]->harq_processes[harq_pid]->TBS/8;
      
      for (i=0;i<input_buffer_length;i++) {
	ulsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      }
      
      printf("[PHY_PROCEDURES_LTE][UE_UL] ulsch_ue %p : O %d, O_ACK %d, O_RI %d, TBS %d\n",ulsch_ue[0],ulsch_ue[0]->O,ulsch_ue[0]->O_ACK,ulsch_ue[0]->O_RI,ulsch_ue[0]->harq_processes[harq_pid]->TBS);

      ulsch_encoding(ulsch_input_buffer,lte_frame_parms,ulsch_ue[0],harq_pid);
      ulsch_modulation(lte_ue_common_vars->txdataF,AMP,(next_slot>>1),lte_frame_parms,ulsch_ue[0]);
      ulsch_ue_active = 0;
    }
  }
}


void lte_ue_measurement_procedures(unsigned char last_slot, unsigned short l) {

#ifdef EMOS
  unsigned char eNb_id,aa;

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
  if (l==0) {
    // Measurements from first DL reference symbol
    
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
    
    if (mac_xface->frame%100 < 10) {
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
  

  
  if ((last_slot==1) && (l==(4-lte_frame_parms->Ncp))) {
    
    lte_adjust_synch(lte_frame_parms,
		     lte_ue_common_vars,
		     1,
		     16384);
  }
}



void lte_ue_pbch_procedures(int eNb_id,unsigned char last_slot) {

  int pbch_error;

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
  

  if (mac_xface->frame % 100 < 10) {
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


void lte_ue_pdcch_procedures(int eNb_id,unsigned char last_slot) {	

  unsigned int dci_cnt, i;
  
#ifdef DEBUG_PHY
  if ((mac_xface->frame % 100) < 10)
    msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): DCI decoding\n",mac_xface->frame,last_slot,last_slot>>1);
#endif
  
  //  write_output("UE_rxsigF0.m","UE_rxsF0", lte_ue_common_vars->rxdataF[0],512*12*2,2,1);
  //  write_output("UE_rxsigF1.m","UE_rxsF1", lte_ue_common_vars->rxdataF[1],512*12*2,2,1);
  
  rx_pdcch(lte_ue_common_vars,
	   lte_ue_pdcch_vars,
	   lte_frame_parms,
	   eNb_id,
	   2,
	   (lte_frame_parms->nb_antennas_tx == 1) ? SISO : ALAMOUTI); //this needs to be changed
  
  dci_cnt = dci_decoding_procedure(lte_ue_pdcch_vars,dci_alloc_rx,eNb_id,lte_frame_parms,SI_RNTI,RA_RNTI,C_RNTI);
  printf("[PHY PROCEDURES UE] subframe %d: dci_cnt %d\n",last_slot>>1,dci_cnt);
  for (i=0;i<dci_cnt;i++){
    printf("[PHY PROCEDURES UE] subframe %d: Found rnti %x, format %d\n",last_slot>>1,dci_alloc_rx[i].rnti,
	   dci_alloc_rx[i].format);
    if ((dci_alloc_rx[i].rnti == C_RNTI) && (dci_alloc_rx[i].format == format2_2A_M10PRB)) {
      generate_ue_dlsch_params_from_dci(last_slot>>1,
					(DCI2_5MHz_2A_M10PRB_TDD_t *)&dci_alloc_rx[i].dci_pdu,
					C_RNTI,
					format2_2A_M10PRB,
					dlsch_ue,
					lte_frame_parms,
					SI_RNTI,
					RA_RNTI,
					P_RNTI);
      dlsch_ue_active = 1;
      printf("[PHY_PROCEDURES_LTE] Generated UE DLSCH C_RNTI format 2_2A_M10PRB\n");
    }
    else if ((dci_alloc_rx[i].rnti == SI_RNTI) && (dci_alloc_rx[i].format == format1A)) {
      generate_ue_dlsch_params_from_dci(last_slot>>1,
					(DCI1A_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					SI_RNTI,
					format1A,
					&dlsch_ue_cntl, 
					lte_frame_parms,
					SI_RNTI,
					RA_RNTI,
					P_RNTI);
      dlsch_ue_cntl_active = 1;
      printf("[PHY_PROCEDURES_LTE] Generate UE DLSCH SI_RNTI format 1A\n");
    }
    else if ((dci_alloc_rx[i].rnti == C_RNTI) && (dci_alloc_rx[i].format == format0)) {
      generate_ue_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					C_RNTI,
					last_slot>>1,
					format0,
					ulsch_ue[eNb_id], 
					lte_frame_parms,
					SI_RNTI,
					RA_RNTI,
					P_RNTI);
      ulsch_ue_active = 1;
      printf("[PHY_PROCEDURES_LTE] Generate UE ULSCH C_RNTI format 0 (subframe %d)\n",last_slot>>1);
    }
  }
}


void phy_procedures_UE_RX(unsigned char last_slot) {

  unsigned short l,m,n_symb;
  int eNb_id = 0, eNb_id_i = 1;
  unsigned char dual_stream_UE = 0;
  int ret;

  if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1) == SF_S) 
    if ((last_slot%2)==0)
      n_symb = 3;
    else
      n_symb = 0;
  else
    n_symb = lte_frame_parms->symbols_per_tti/2;

  // RX processing of symbols in last_slot
  for (l=0;l<n_symb;l++) {

    slot_fep(lte_frame_parms,
	     lte_ue_common_vars,
	     l,
	     last_slot,
#ifdef USER_MODE
	     0
#else
             1
#endif
	     );
   
    lte_ue_measurement_procedures(last_slot,l);

    //printf("Frame %d, last_slot %d (symbol %d)-> dlsch_ue_active %d\n",mac_xface->frame,last_slot,l,dlsch_ue_active);

    if ((last_slot==1) && (l==4-lte_frame_parms->Ncp)) {
	
      lte_ue_pbch_procedures(eNb_id,last_slot);
    }

	
    // process last DLSCH symbols + invoke decoding
    if (((last_slot%2)==0) && (l==0)) {

      if ( (dlsch_ue_active == 1) && (dlsch_ue_cntl_active == 1))
	msg("[PHY_PROCEDURES_LTE] WARNING: dlsch_ue and dlsch_ue_cntl active, but data structures can only handle one at a time\n");

      if (dlsch_ue_active == 1) {
#ifdef DEBUG_PHY
	if ((mac_xface->frame % 100) < 10)
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
	
	dlsch_ue_active = 0;
      
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
	//if (mac_xface->frame < dlsch_errors)
	//  dlsch_errors=0;
	
	if (dlsch_ue[0]) {
	  ret = dlsch_decoding(lte_ue_dlsch_vars[eNb_id]->llr[0],
			       lte_frame_parms,
			       dlsch_ue[0],
			       ((last_slot>>1)-1)%10);
	  
	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    dlsch_errors++;
	  }
	}   
	
	if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10)) {
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_decoding ret %d (%d errors)\n",
	      mac_xface->frame,last_slot,ret,dlsch_errors);
	}
#endif
      }
      if (dlsch_ue_cntl_active == 1) {
#ifdef DEBUG_PHY
	if ((mac_xface->frame % 100) < 10)
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (cntl) demod symbols 10,11,12\n",mac_xface->frame,last_slot);
#endif
      
	// process symbols 10,11,12 and trigger DLSCH decoding
	for (m=(11-lte_frame_parms->Ncp*2+1);m<lte_frame_parms->symbols_per_tti;m++)
	  rx_dlsch(lte_ue_common_vars,
		   lte_ue_dlsch_vars,
		   lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &dlsch_ue_cntl,
		   m,
		   dual_stream_UE);
	
	write_output("dlsch_cntl_llr.m","llr",lte_ue_dlsch_vars[eNb_id]->llr[0],40,1,0);

	dlsch_ue_cntl_active = 0;
      
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
	//if (mac_xface->frame < dlsch_errors)
	//  dlsch_errors=0;
	
	if (dlsch_ue_cntl) {
	  ret = dlsch_decoding(lte_ue_dlsch_vars[eNb_id]->llr[0],
			       lte_frame_parms,
			       dlsch_ue_cntl,
			       ((last_slot>>1)-1)%10);
	  
	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    dlsch_errors++;
	  }
	}   
	
	if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10)) {
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: dlsch_decoding (cntl) ret %d (%d errors)\n",
	      mac_xface->frame,last_slot,ret,dlsch_errors);
	}
#endif
      }
    }
      
    if (((last_slot%2)==0) && (l==(4-lte_frame_parms->Ncp)))  {
	
      lte_ue_pdcch_procedures(eNb_id,last_slot);
	
      if (dlsch_ue_active == 1) {
#ifdef DEBUG_PHY
	if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10))
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 0,1,2\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 0,1,2
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
      if (dlsch_ue_cntl_active == 1) {
#ifdef DEBUG_PHY
	if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10))
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (cntl) demod symbols 0,1,2\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 0,1,2
	for (m=lte_frame_parms->first_dlsch_symbol;m<(4-lte_frame_parms->Ncp);m++)
	  rx_dlsch(lte_ue_common_vars,
		   lte_ue_dlsch_vars,
		   lte_frame_parms,
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
	if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10))
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 3,4,5\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 3,4,5
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
      if (dlsch_ue_cntl_active == 1)  {
#ifdef DEBUG_PHY
	if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10))
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (cntl) demod symbols 3,4,5\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 3,4,5
	for (m=4-lte_frame_parms->Ncp+1;m<(lte_frame_parms->symbols_per_tti/2);m++)
	  rx_dlsch(lte_ue_common_vars,
		   lte_ue_dlsch_vars,
		   lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &dlsch_ue_cntl,
		   m,
		   dual_stream_UE);
      }
    }
      
    if (((last_slot%2)==1) && (l==(4-lte_frame_parms->Ncp))) {
	
      if(dlsch_ue_active == 1) {
#ifdef DEBUG_PHY
	if (( (mac_xface->frame % 100) == 0) || (mac_xface->frame < 10))
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH demod symbols 6,7,8\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 6,7,8
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
      if(dlsch_ue_cntl_active == 1) {
#ifdef DEBUG_PHY
	if (( (mac_xface->frame % 100) == 0) || (mac_xface->frame < 10))
	  msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: DLSCH (cntl) demod symbols 6,7,8\n",mac_xface->frame,last_slot);
#endif
	
	// process symbols 6,7,8
	for (m=(lte_frame_parms->symbols_per_tti/2)+1;m<(11-lte_frame_parms->Ncp*2);m++)
	  rx_dlsch(lte_ue_common_vars,
		   lte_ue_dlsch_vars,
		   lte_frame_parms,
		   eNb_id,
		   eNb_id_i,
		   &dlsch_ue_cntl,
		   m,
		   dual_stream_UE);
      }
    }
  }
}

void phy_procedures_eNB_TX(next_slot) {

  unsigned char pbch_pdu[PBCH_PDU_SIZE];
  unsigned int nb_dci_ue_spec = 0, nb_dci_common = 0;
  unsigned short input_buffer_length, re_allocated;
  int eNb_id = 0,i;

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
      msg("[PHY_PROCEDURES_LTE] frame %d, slot %d: Calling generate_pbch\n",mac_xface->frame, next_slot);
#endif
    
    *((unsigned int*) pbch_pdu) = mac_xface->frame;
    
    generate_pbch(lte_eNB_common_vars->txdataF[eNb_id],
		  AMP,
		  lte_frame_parms,
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

      memcpy(&dci_alloc[1].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
      dci_alloc[1].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
      dci_alloc[1].L          = 3;
      dci_alloc[1].rnti       = C_RNTI;
      nb_dci_ue_spec = 1;
      nb_dci_common  = 1;
      msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): Generated ULSCH DCI, format 0\n",mac_xface->frame,next_slot,next_slot>>1);

      msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generated CCCH DCI, format 1A\n",mac_xface->frame, next_slot);
      generate_eNb_dlsch_params_from_dci(next_slot>>1,
					 &CCCH_alloc_pdu,
					 SI_RNTI,
					 format1A,
					 &dlsch_eNb_cntl,
					 lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);
      dlsch_eNb_cntl_active = 1;
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
      msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generated DLSCH DCI, format 2_2A_M10PRB\n",mac_xface->frame, next_slot);
      generate_eNb_dlsch_params_from_dci(next_slot>>1,
					 &DLSCH_alloc_pdu2,
					 C_RNTI,
					 format2_2A_M10PRB,
					 dlsch_eNb,
					 lte_frame_parms,
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
      memcpy(&dci_alloc[0].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
      dci_alloc[0].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = C_RNTI;
      nb_dci_ue_spec = 1;
      nb_dci_common  = 0;
      dlsch_eNb_active = 0;
      msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): Generated ULSCH DCI, format 0\n",mac_xface->frame,next_slot,next_slot>>1);
      break;
    case 9:
      memcpy(&dci_alloc[0].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
      dci_alloc[0].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = C_RNTI;
      nb_dci_ue_spec = 1;
      nb_dci_common  = 0;
      dlsch_eNb_active = 0;
      msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): Generated ULSCH DCI, format 0\n",mac_xface->frame,next_slot,next_slot>>1);
      break;
    }
    
    if ((nb_dci_common+nb_dci_ue_spec)>0) {
#ifdef DEBUG_PHY
      if (mac_xface->frame%100 == 0)
	msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dci_top\n",mac_xface->frame, next_slot);
#endif
      generate_dci_top(nb_dci_ue_spec,
		       nb_dci_common,
		       dci_alloc,
		       0,
		       AMP,
		       lte_frame_parms,
		       lte_eNB_common_vars->txdataF[eNb_id],
		       next_slot>>1);

    }
  }
  if (next_slot%2 == 0) {

    if (dlsch_eNb_active == 1) {
      input_buffer_length = dlsch_eNb[0]->harq_processes[0]->TBS/8;
      for (i=0;i<input_buffer_length;i++)
	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      
#ifdef DEBUG_PHY
      if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10))
	msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dlsch with input size = %d\n",mac_xface->frame, next_slot, input_buffer_length);
#endif
      
      dlsch_encoding(dlsch_input_buffer,
		     lte_frame_parms,
		     dlsch_eNb[0]);
      
      re_allocated = dlsch_modulation(lte_eNB_common_vars->txdataF[eNb_id],
				      AMP,
				      next_slot/2,
				      lte_frame_parms,
				    dlsch_eNb[0]);
      /*
	if (mimo_mode == DUALSTREAM) {
	dlsch_encoding(input_buffer,
	lte_frame_parms,
	dlsch_eNb[1]);
	
	re_allocated += dlsch_modulation(lte_eNB_common_vars->txdataF[eNb_id],
	1024,
	next_slot>>1,
	lte_frame_parms,
	dlsch_eNb[1]);
	}
      */
      dlsch_eNb_active = 0;

#ifdef DEBUG_PHY    
    msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d, DLSCH re_allocated = %d\n",mac_xface->frame, next_slot, re_allocated);
#endif

    }

    if (dlsch_eNb_cntl_active == 1) {
      input_buffer_length = dlsch_eNb_cntl->harq_processes[0]->TBS/8;
      for (i=0;i<input_buffer_length;i++)
	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      
#ifdef DEBUG_PHY
      if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10))
	msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dlsch (cntl) with input size = %d\n",mac_xface->frame, next_slot, input_buffer_length);
#endif
      
      dlsch_encoding(dlsch_input_buffer,
		     lte_frame_parms,
		     dlsch_eNb_cntl);
      
      re_allocated = dlsch_modulation(lte_eNB_common_vars->txdataF[eNb_id],
				      AMP,
				      next_slot/2,
				      lte_frame_parms,
				      dlsch_eNb_cntl);
      dlsch_eNb_cntl_active = 0;

#ifdef DEBUG_PHY    
    msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d, DLSCH (cntl) re_allocated = %d\n",mac_xface->frame, next_slot, re_allocated);
#endif

    }

  }
}
  


void phy_procedures_eNB_RX(last_slot) {
  //RX processing
  unsigned int l;
  unsigned int eNb_id=0;
  int rx_power,aarx;
  
  if (((openair_daq_vars.tdd==1) && (last_slot>=10)) || (openair_daq_vars.tdd == 0)) {
    
    for (l=0;l<lte_frame_parms->symbols_per_tti/2;l++) {
      
      slot_fep_ul(lte_frame_parms,
		  lte_eNB_common_vars,
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
  }  
  
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

  
void phy_procedures_lte(unsigned char last_slot, unsigned char next_slot) {


  if (mac_xface->is_cluster_head == 0) {
    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_UL) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_UE_TX(%d)\n",next_slot);
      phy_procedures_UE_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_DL) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_UE_RX(%d)\n",last_slot);
      phy_procedures_UE_RX(last_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_S) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_UE_S_TX(%d)\n",next_slot);
      //phy_procedures_UE_S_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_S) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_UE_RX(%d)\n",last_slot);
      phy_procedures_UE_RX(last_slot);
    }
  }
  else { //eNB
    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_DL) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_eNB_TX(%d)\n",next_slot);
      phy_procedures_eNB_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_UL) {
      msg("[PHY_PROCEDURES_LTE] Calling phy_procedures_eNB_RX(%d)\n",last_slot);
      phy_procedures_eNB_RX(last_slot);
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




