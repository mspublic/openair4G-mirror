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

//#define DEBUG_PHY

#ifdef USER_MODE
//#define DEBUG_PHY
#endif

#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
#endif

#define DIAG_PHY

#define DLSCH_RB_ALLOC 0x1fbf  // skip DC RB (total 23/25 RBs)
#define DLSCH_RB_ALLOC_12 0x0aaa  // skip DC RB (total 12/25 RBs)
#define DLSCH_RB_ALLOC_6 0x0999  // skip DC RB (total 6/25 RBs)

#define NS_PER_SLOT 500000

extern inline unsigned int taus(void);
extern int exit_openair;

unsigned char dlsch_input_buffer[2700] __attribute__ ((aligned(16)));
int eNb_sync_buffer0[640*6] __attribute__ ((aligned(16)));
int eNb_sync_buffer1[640*6] __attribute__ ((aligned(16)));
int *eNb_sync_buffer[2] = {eNb_sync_buffer0, eNb_sync_buffer1};

static char dlsch_eNb_active = 0;
static char dlsch_eNb_cntl_active = 0;
static char dlsch_eNb_ra_active = 0;
static char dlsch_eNb_1A_active = 0;
static char eNb_generate_rar = 0;
static char eNb_generate_rag_ack = 0;

int ulsch_errors[3]={0,0,0},ulsch_consecutive_errors[3]={0,0,0},ulsch_decoding_attempts[3]={0,0,0},dlsch_NAK=0;
unsigned int max_peak_val; 

int max_eNb_id, max_sync_pos;

DCI_ALLOC_t dci_alloc[8];

#ifdef EMOS
  fifo_dump_emos_eNb emos_dump_eNb;
#endif

#ifdef DIAG_PHY
extern int rx_sig_fifo;
#endif

static unsigned char I0_clear = 1;


#ifdef USER_MODE
void dump_ulsch() {
 
    write_output("rxsigF0.m","rxsF0", &lte_eNB_common_vars->rxdataF[0][0][0],512*12*2,2,1);
  write_output("rxsigF1.m","rxsF1", &lte_eNB_common_vars->rxdataF[0][1][0],512*12*2,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", &lte_eNB_ulsch_vars[0]->rxdataF_ext[0][0],300*12*2,2,1);
  write_output("rxsigF1_ext.m","rxsF1_ext", &lte_eNB_ulsch_vars[0]->rxdataF_ext[1][0],300*12*2,2,1);
  write_output("srs_seq.m","srs",lte_eNB_common_vars->srs,2*lte_frame_parms->ofdm_symbol_size,2,1);
  write_output("srs_est0.m","srsest0",lte_eNB_common_vars->srs_ch_estimates[0][0],512,1,1);
  write_output("srs_est1.m","srsest1",lte_eNB_common_vars->srs_ch_estimates[0][1],512,1,1);
  write_output("drs_est0.m","drsest0",lte_eNB_ulsch_vars[0]->drs_ch_estimates[0][0],300*12,1,1);
  write_output("drs_est1.m","drsest1",lte_eNB_ulsch_vars[0]->drs_ch_estimates[0][1],300*12,1,1);
  write_output("ulsch_rxF_comp0.m","ulsch0_rxF_comp0",&lte_eNB_ulsch_vars[0]->rxdataF_comp[0][0][0],300*12,1,1);
  write_output("ulsch_rxF_llr.m","ulsch_llr",lte_eNB_ulsch_vars[0]->llr,ulsch_ue[0]->harq_processes[0]->nb_rb*12*2*9,1,0);	
  write_output("ulsch_ch_mag.m","ulsch_ch_mag",&lte_eNB_ulsch_vars[0]->ul_ch_mag[0][0][0],300*12,1,1);	  

  // UE TX sig in subframe 3
  write_output("txsigF0.m","txsF0", &lte_ue_common_vars->txdataF[0][512*12*3],512*12,1,1);


}
#endif

#ifdef EMOS
void phy_procedures_emos_eNB_TX(unsigned char next_slot) {

  unsigned char eNb_id,i;

  if (next_slot==1) {
      emos_dump_eNb.timestamp = rt_get_time_ns();
      emos_dump_eNb.frame_tx = mac_xface->frame;
  }
  if (next_slot%2==0) {
    for (i=0; i<2; i++) 
      memcpy(&emos_dump_eNb.DCI_alloc[i][next_slot>>1], &dci_alloc[i], sizeof(DCI_ALLOC_t));
    }
  if (next_slot==19) {
    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Writing EMOS data to FIFO\n",mac_xface->frame, next_slot);
    if (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump_eNb, sizeof(fifo_dump_emos_eNb))!=sizeof(fifo_dump_emos_eNb)) {
      debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, Problem writing EMOS data to FIFO\n",mac_xface->frame, next_slot);
      return;
    }
  }
}
#endif

void phy_procedures_eNB_S_TX(unsigned char next_slot) {

  int eNb_id = 0, aa;

  if (next_slot%2==0) {
#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generating pilots for DL-S\n",mac_xface->frame,next_slot);
#endif
    
    for (eNb_id=0;eNb_id<number_of_cards;eNb_id++) {
      for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {

      
      /*
	#ifdef DEBUG_PHY
	printf("Clearing TX buffer %d at %p, length %d \n",aa,
	&lte_eNB_common_vars->txdataF[eNb_id][aa][next_slot*(lte_frame_parms->N_RB_DL*12)*(lte_frame_parms->symbols_per_tti>>1)],
	(lte_frame_parms->N_RB_DL*12)*(lte_frame_parms->symbols_per_tti>>1)*sizeof(mod_sym_t));
	#endif
      */
#ifdef IFFT_FPGA
	memset(&lte_eNB_common_vars->txdataF[eNb_id][aa][next_slot*(lte_frame_parms->N_RB_DL*12)*(lte_frame_parms->symbols_per_tti>>1)],
	       0,(lte_frame_parms->N_RB_DL*12)*(lte_frame_parms->symbols_per_tti>>1)*sizeof(mod_sym_t));
#else
	memset(&lte_eNB_common_vars->txdataF[eNb_id][aa][next_slot*lte_frame_parms->ofdm_symbol_size*(lte_frame_parms->symbols_per_tti>>1)],
	       0,lte_frame_parms->ofdm_symbol_size*(lte_frame_parms->symbols_per_tti>>1)*sizeof(mod_sym_t));
#endif
      }
      
      generate_pilots_slot(lte_eNB_common_vars->txdataF[eNb_id],
			   AMP,
			   lte_frame_parms,
			   eNb_id,
			   next_slot);
      
      generate_pss(lte_eNB_common_vars->txdataF[eNb_id],
		   AMP,
		   lte_frame_parms,
		   eNb_id,
		   2,
		   next_slot);
      
    }
  }
}
 
void phy_procedures_eNB_S_RX(unsigned char last_slot) {

  int aa,l,sync_pos,sync_pos_slot;
  unsigned int sync_val;
  unsigned char eNb_id=0, UE_id=0;
  int time_in, time_out;
  short *x, *y, *z;
  char fname[100],vname[100];


  if (last_slot%2==1) {
    
    /*
    for (eNb_id = 0; eNb_id < number_of_cards; eNb_id++) {
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
    */

#ifndef USER_MODE
    time_in = openair_get_mbox();
#endif


    // we alternately process the signals from the three different sectors
    eNb_id = mac_xface->frame % number_of_cards; 
    //eNb_id = 2;

    if (eNb_id == 0) {
      max_peak_val = 0;
      max_eNb_id = 0;
      max_sync_pos = 0;
    }

    if (eNB_UE_stats[0].mode[0] == PRACH) {
      // look for PSS in the last 3 symbols of the last slot
      // but before we need to zero pad the gaps that the HW removed
      // also add the signals from all antennas of all eNbs
      bzero(eNb_sync_buffer[0],640*6*sizeof(int));
      bzero(eNb_sync_buffer[1],640*6*sizeof(int));
      
	
      for (aa=0; aa<lte_frame_parms->nb_antennas_rx; aa++) {
	for (l=PSS_UL_SYMBOL; l<lte_frame_parms->symbols_per_tti/2; l++) {
	  
	  x = (short*) &eNb_sync_buffer[aa][(l-PSS_UL_SYMBOL)*(lte_frame_parms->ofdm_symbol_size+lte_frame_parms->nb_prefix_samples)+lte_frame_parms->nb_prefix_samples];
#ifdef USER_MODE
	  y = (short*) &lte_eNB_common_vars->rxdata[eNb_id][aa][(last_slot*lte_frame_parms->symbols_per_tti/2+l)*(lte_frame_parms->ofdm_symbol_size+lte_frame_parms->nb_prefix_samples)+lte_frame_parms->nb_prefix_samples];
#else
	  y = (short*) &lte_eNB_common_vars->rxdata[eNb_id][aa][(last_slot*lte_frame_parms->symbols_per_tti/2+l)*lte_frame_parms->ofdm_symbol_size];
#endif
	  //z = x;
	  //add_vector16(x,y,z,lte_frame_parms->ofdm_symbol_size*2);
	  memcpy(x,y,lte_frame_parms->ofdm_symbol_size*sizeof(int));
	}
      }
      
#ifdef USER_MODE
      /*
      write_output("eNb_sync_buffer0.m","eNb_sync_buf0",eNb_sync_buffer[0],(lte_frame_parms->ofdm_symbol_size+lte_frame_parms->nb_prefix_samples)*(lte_frame_parms->symbols_per_tti/2-PSS_UL_SYMBOL),1,1);
      write_output("eNb_sync_buffer1.m","eNb_sync_buf1",eNb_sync_buffer[1],(lte_frame_parms->ofdm_symbol_size+lte_frame_parms->nb_prefix_samples)*(lte_frame_parms->symbols_per_tti/2-PSS_UL_SYMBOL),1,1);
      */
#endif

      sync_pos_slot = 0; //this is where the sync pos should be wrt eNb_sync_buffer

      sync_pos = lte_sync_time_eNb(eNb_sync_buffer, 
				   lte_frame_parms, 
				   0,//eNb_id,
				   (lte_frame_parms->symbols_per_tti/2 - PSS_UL_SYMBOL) * 
				   (lte_frame_parms->ofdm_symbol_size+lte_frame_parms->nb_prefix_samples),
				   &sync_val,
				   lte_eNB_common_vars->sync_corr[eNb_id]);

#ifdef USER_MODE
      
      if (eNb_id==0)
	write_output("sync_corr_eNb.m","synccorr",lte_eNB_common_vars->sync_corr[eNb_id],
		     (lte_frame_parms->symbols_per_tti/2 - PSS_UL_SYMBOL) * 
		     (lte_frame_parms->ofdm_symbol_size+lte_frame_parms->nb_prefix_samples),1,2);
      
#endif

      if ((sync_pos>=0) && (sync_val > max_peak_val)) {
	max_peak_val = sync_val;
	max_eNb_id = eNb_id;
	max_sync_pos = sync_pos;
      }

      debug_msg("[PHY_PROCEDURES_LTE] found eNb_id=%d, sync_pos=%d, sync_val=%u, max_peak_val=%u, max_peak_pos=%d\n",eNb_id,sync_pos,sync_val,max_peak_val,max_sync_pos);


#ifndef USER_MODE
      time_out = openair_get_mbox();
#endif
      
      if (max_peak_val>0) {
	if (eNb_id==number_of_cards-1) {
	  eNB_UE_stats[0].UE_id[UE_id] = 0x1234; 
	  eNB_UE_stats[0].UE_timing_offset[UE_id] = max(max_sync_pos - sync_pos_slot - lte_frame_parms->nb_prefix_samples/8,0);
	  eNB_UE_stats[0].mode[UE_id] = PRACH;
	  eNB_UE_stats[0].sector[UE_id] = max_eNb_id;
	  //#ifdef DEBUG_PHY
	  debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d: Found user %x in sector %d at pos %d val %d , timing_advance %d (time_in %d, time_out %d)\n",
	      mac_xface->frame, last_slot, 
	      UE_id, max_eNb_id,
	      max_sync_pos, 
	      max_peak_val,
	      eNB_UE_stats[0].UE_timing_offset[UE_id],
	      time_in, time_out);
	  //#endif
	  
	  eNb_generate_rar = 1;
	  max_peak_val = 0;
	  max_eNb_id = 0;
	  max_sync_pos = 0;
      
	  }
      }
      else {
	debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d: No user found\n",mac_xface->frame,last_slot);
	eNb_generate_rar = 0;
	max_peak_val = 0;
	max_eNb_id = 0;
	max_sync_pos = 0;

      }
    }
    
    // Get noise levels


    for (eNb_id=0;eNb_id<number_of_cards;eNb_id++) {
      /*
      sprintf(fname,"rxsigF0_%d.m",eNb_id);
      sprintf(vname,"rxsF0_%d",eNb_id);
      write_output(fname,vname, &lte_eNB_common_vars->rxdataF[eNb_id][0][(19*lte_frame_parms->ofdm_symbol_size)<<1],512*2,2,1);
      sprintf(fname,"rxsigF1_%d.m",eNb_id);
      sprintf(vname,"rxsF1_%d",eNb_id);
      write_output(fname,vname, &lte_eNB_common_vars->rxdataF[eNb_id][1][(19*lte_frame_parms->ofdm_symbol_size)<<1],512*2,2,1);
      */

      lte_eNB_I0_measurements(lte_eNB_common_vars,
			      lte_frame_parms,
			      &PHY_vars->PHY_measurements_eNB[eNb_id],
			      eNb_id,
			      I0_clear);


    }

    if (I0_clear == 1)
      I0_clear = 0;
  }
}

#ifdef EMOS
void phy_procedures_emos_eNB_RX(unsigned char last_slot) {

  unsigned char eNb_id,i,aa;

  if (last_slot%2==1) {
    memcpy(&emos_dump_eNb.eNB_UE_stats[0][(last_slot>>1)-2],&eNB_UE_stats,sizeof(LTE_eNB_UE_stats));
  }

  if (last_slot==4) {
      emos_dump_eNb.rx_total_gain_dB = PHY_vars->rx_total_gain_eNB_dB;
      emos_dump_eNb.mimo_mode = openair_daq_vars.dlsch_transmission_mode;
  }

  if (last_slot==8) {
    emos_dump_eNb.ulsch_errors = ulsch_errors[1];
    for (eNb_id = 0; eNb_id<3; eNb_id++)  
      memcpy(&emos_dump_eNb.PHY_measurements_eNB[eNb_id],
	     &PHY_vars->PHY_measurements_eNB[eNb_id],
	     sizeof(PHY_MEASUREMENTS_eNB));

  }

  if (last_slot%2==1) {
    for (eNb_id = 0; eNb_id<3; eNb_id++)  
      for (aa=0; aa<lte_frame_parms->nb_antennas_rx; aa++) 
	memcpy(&emos_dump_eNb.channel[(last_slot>>1)-2][eNb_id][aa][0],
	       lte_eNB_common_vars->srs_ch_estimates[eNb_id][aa],
	       lte_frame_parms->ofdm_symbol_size*sizeof(int));
  }

}

#endif

void phy_procedures_eNB_TX(unsigned char next_slot) {

  unsigned char pbch_pdu[PBCH_PDU_SIZE];
  unsigned int nb_dci_ue_spec = 0, nb_dci_common = 0;
  unsigned short input_buffer_length, re_allocated;
  int eNb_id = 0,i,aa;
  unsigned char harq_pid;

  for (eNb_id = 0 ; eNb_id < number_of_cards; eNb_id++) {

    if (next_slot%2 == 0) {
      for (aa=0; aa<lte_frame_parms->nb_antennas_tx;aa++) {

#ifdef IFFT_FPGA
	memset(&lte_eNB_common_vars->txdataF[eNb_id][aa][next_slot*(lte_frame_parms->N_RB_DL*12)*(lte_frame_parms->symbols_per_tti>>1)],
	       0,(lte_frame_parms->N_RB_DL*12)*(lte_frame_parms->symbols_per_tti)*sizeof(mod_sym_t));
#else
	memset(&lte_eNB_common_vars->txdataF[eNb_id][aa][next_slot*lte_frame_parms->ofdm_symbol_size*(lte_frame_parms->symbols_per_tti>>1)],
	       0,lte_frame_parms->ofdm_symbol_size*(lte_frame_parms->symbols_per_tti)*sizeof(mod_sym_t));
#endif
      }
    }
    
    
    generate_pilots_slot(lte_eNB_common_vars->txdataF[eNb_id],
			 AMP,
			 lte_frame_parms,
			 eNb_id,
			 next_slot);
    
    
    
    if (next_slot == 1) {
      
      
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_pbch\n",mac_xface->frame, next_slot);
#endif
      
      *((unsigned int*) pbch_pdu) = mac_xface->frame;
      ((unsigned char*) pbch_pdu)[4] = openair_daq_vars.dlsch_transmission_mode;
      
      generate_pbch(lte_eNB_common_vars->txdataF[eNb_id],
		    AMP,
		    lte_frame_parms,
		    pbch_pdu);
    }
  }

#ifdef OPENAIR2

  /*
  // DCI generation
  if ((next_slot%2 == 0)) { 
    
    for(i=0;i<NB_REQ_MAX;i++) {
      if(Macphy_req_table[0].Macphy_req_table_entry[i].Active){
	if (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type == DCI) {
	  Macphy_req_table[0].Macphy_req_table_entry[i].Active=0;
	  //	Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_req.Phy_Resources_Entry->Active=0;
	  Macphy_req_table[0].Macphy_req_cnt--;
	  //	  debug_msg("[PHY][eNB PROCEDURES] Got DCI_PDU for %d/%d from MAC\n",mac_xface->frame,next_slot>>1);
    nb_dci_ue_spec = 0;
    nb_dci_common  = 0;
    dlsch_eNb_active = 0;
    dlsch_eNb_cntl_active = 0;

    // Get DCI parameters from MAC
	}
      }
    }
  }
  
  */

  eNb_id=0;
  if ((next_slot%2) == 0) {
    //    debug_msg("[PHY Procedures] Frame %d : Checking for DCI in subframe %d, generate_RAR = %d\n",mac_xface->frame,next_slot>>1,eNb_generate_rar);
    nb_dci_ue_spec=0;
    nb_dci_common=0;

    if (CH_mac_inst[0].DCI_pdu.Num_common_dci == 1) {
      
      generate_eNb_dlsch_params_from_dci(next_slot>>1,
					 &CH_mac_inst[0].DCI_pdu.dci_alloc[0].dci_pdu[0],
					 CH_mac_inst[0].DCI_pdu.dci_alloc[0].rnti,
					 format1A,
					 &dlsch_eNb_cntl,
					 lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);
      dlsch_eNb_cntl_active = 1;
      nb_dci_common  = 1;
    }
    else {
      dlsch_eNb_cntl_active = 0;
      nb_dci_common = 0;
      
    }

    // RA  
    if (((next_slot>>1) == 7) && (eNb_generate_rar == 1)) {
      msg("[PHY_PROCEDURES_LTE][eNB] Frame %d, slot %d: Generated RAR DCI, format 1A\n",mac_xface->frame, next_slot); 

      // Schedule Random-Access Response 
      memcpy(&dci_alloc[0].dci_pdu[0],&RA_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
      dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = RA_RNTI;


      generate_eNb_dlsch_params_from_dci(next_slot>>1,
					 &RA_alloc_pdu,
					 RA_RNTI,
					 format1A,
					 &dlsch_eNb_ra,
					 lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);
      dlsch_eNb_ra_active = 1;
      nb_dci_common++;
      eNb_generate_rar=0;
    }

    // RAG_ACK  
    else if (((next_slot>>1) == 7) && (eNb_generate_rag_ack == 1)) {
      msg("[eNB] Frame %d, slot %d: Generated DLSCH (RAG_ACK) DCI, format 1A\n",mac_xface->frame, next_slot); 

      // Schedule Reflection of Connection request 
      memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu1A,sizeof(DCI1A_5MHz_TDD_1_6_t));
      dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = eNB_UE_stats[0].UE_id[0];

      generate_eNb_dlsch_params_from_dci(next_slot>>1,
					 &DLSCH_alloc_pdu1A,
					 eNB_UE_stats[0].UE_id[0],
					 format1A,
					 &dlsch_eNb_1A,
					 lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);
      dlsch_eNb_1A_active = 1;
      nb_dci_ue_spec++;
      eNb_generate_rag_ack=0;
    }

    // RA 


    if (CH_mac_inst[0].DCI_pdu.Num_ue_spec_dci == 1) {
      
      generate_eNb_dlsch_params_from_dci(next_slot>>1,
					 &DLSCH_alloc_pdu2,
					 eNB_UE_stats[0].UE_id[0],
					 format2_2A_M10PRB,
					 dlsch_eNb,
					 lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);
      dlsch_eNb_active = 1;
      nb_dci_ue_spec ++;
    }
    else {
      dlsch_eNb_active = 0;
    }
  }
#endif //OPENAIR2
#ifdef DIAG_PHY

  if (((next_slot % 2)==0) && (eNB_UE_stats[0].mode[0] == PUSCH) ) {
    switch (next_slot>>1) {
    case 0:
      // Schedule DL control 
      memcpy(&dci_alloc[0].dci_pdu[0],&CCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
      dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = SI_RNTI;
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generated CCCH DCI, format 1A\n",mac_xface->frame, next_slot);
#endif
      nb_dci_common  = 1;
      
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
      
      /*
      // Schedule UL subframe
      memcpy(&dci_alloc[1].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
      dci_alloc[1].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
      dci_alloc[1].L          = 3;
      dci_alloc[1].rnti       = C_RNTI;
      #ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): Generated ULSCH DCI, format 0\n",mac_xface->frame,next_slot,next_slot>>1);
      #endif
      nb_dci_ue_spec = 1;
      
      generate_eNb_ulsch_params_from_dci(&UL_alloc_pdu,
      C_RNTI,
      (next_slot>>1),
      format0,
      ulsch_eNb[0],
      lte_frame_parms,
      SI_RNTI,
      RA_RNTI,
      P_RNTI);
      // get UL harq_pid for subframe+4
      harq_pid = subframe2harq_pid_tdd(3,((next_slot>>1)+4)%10);
      #ifdef DEBUG_pHY
      debug_msg("[PHY PROCEDURES eNB] frame %d, subframe %d Setting scheduling flag for ULSCH harq_pid %d\n",
      mac_xface->frame,next_slot>>1,harq_pid);
      #endif
      ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
      */
      
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
      
    case 6:
      
      if (openair_daq_vars.dlsch_rate_adaptation == 0)
	DLSCH_alloc_pdu2.mcs1   = openair_daq_vars.target_ue_dl_mcs;
      else
	DLSCH_alloc_pdu2.mcs1   = (eNB_UE_stats[0].DL_cqi[0][0]<<1);

      if (DLSCH_alloc_pdu2.mcs1 > 14)
	DLSCH_alloc_pdu2.mcs1 = 14;

      if (DLSCH_alloc_pdu2.mcs1 > 10)
	DLSCH_alloc_pdu2.rballoc = DLSCH_RB_ALLOC_12;
      else
	DLSCH_alloc_pdu2.rballoc = DLSCH_RB_ALLOC;

      if (openair_daq_vars.dlsch_transmission_mode == 6)
	DLSCH_alloc_pdu2.tpmi = 5;  // PUSCH precoding

      if (openair_daq_vars.dlsch_transmission_mode == 6)
	DLSCH_alloc_pdu2.tpmi   = 5;
      else
	DLSCH_alloc_pdu2.tpmi   = 0;

      memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
      dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = eNB_UE_stats[0].UE_id[0];
      dlsch_eNb[0]->mode = openair_daq_vars.dlsch_transmission_mode;
      nb_dci_ue_spec = 1;
      //#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generated DLSCH DCI, format 2_2A_M10PRB\n",mac_xface->frame, next_slot);
      //#endif
      
      generate_eNb_dlsch_params_from_dci(next_slot>>1,
					 &DLSCH_alloc_pdu2,
					 eNB_UE_stats[0].UE_id[0],
					 format2_2A_M10PRB,
					 dlsch_eNb,
					 lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Generated DLSCH DCI, format 2_2A_M10PRB\n",mac_xface->frame, next_slot);
      dlsch_eNb_active = 1;
      

      break;
      
    case 7:

      break;
      
    case 9:
      
      // Schedule UL subframe
      // get UL harq_pid for subframe+4
      harq_pid = subframe2harq_pid_tdd(3,((next_slot>>1)+4)%10);
      ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;
      
      
      //if ((mac_xface->frame&1)==0) {
      UL_alloc_pdu.mcs     = openair_daq_vars.target_ue_ul_mcs;
      UL_alloc_pdu.rballoc = computeRIV(lte_frame_parms->N_RB_UL,9,openair_daq_vars.ue_ul_nb_rb);
      memcpy(&dci_alloc[0].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
      dci_alloc[0].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
      dci_alloc[0].L          = 3;
      dci_alloc[0].rnti       = eNB_UE_stats[0].UE_id[0];
      nb_dci_ue_spec = 1;
      
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d (%d): Generated ULSCH DCI, format 0\n",mac_xface->frame,next_slot,next_slot>>1);
#endif
      
      generate_eNb_ulsch_params_from_dci(&UL_alloc_pdu,
					 eNB_UE_stats[0].UE_id[0],
					 (next_slot>>1),
					 format0,
					 ulsch_eNb[0],
					 lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI);
      
      //#ifdef DEBUG_PHY
      debug_msg("[PHY PROCEDURES eNB] frame %d, subframe %d Setting scheduling flag for ULSCH harq_pid %d\n",
		mac_xface->frame,next_slot>>1,harq_pid);
      //#endif
      ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
      //}
      
      break;

    case 8:
      
      // Schedule UL subframe
      // get UL harq_pid for subframe+4
      harq_pid = subframe2harq_pid_tdd(3,((next_slot>>1)+4)%10);
      ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;
      break;

    }

#ifdef EMOS
    emos_dump_eNb.dci_cnt[next_slot>>1] = nb_dci_common+nb_dci_ue_spec;
#endif

  }
#endif //DIAG_PHY


  // if we have PHICH to generate
  if ((next_slot%2)==0) {
    if (is_phich_subframe(lte_frame_parms->tdd_config,next_slot>>1)) {
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_phich_top\n",mac_xface->frame, next_slot);
      generate_phich_top(lte_frame_parms,0,next_slot>>1);
    }
  }

  // if we have DCI to generate do it now
  if (((next_slot%2)==0) && ((nb_dci_common+nb_dci_ue_spec)>0)) {
    //#ifdef DEBUG_PHY
    debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dci_top\n",mac_xface->frame, next_slot);
    //#endif

    for (eNb_id=0;eNb_id<number_of_cards;eNb_id++) 
      generate_dci_top(nb_dci_ue_spec,
		       nb_dci_common,
		       dci_alloc,
		       0,
		       AMP,
		       lte_frame_parms,
		       lte_eNB_common_vars->txdataF[eNb_id],
		       next_slot>>1);
  }

    // For even next slots generate dlsch
  if ((next_slot%2) == 0) {
    
    if (dlsch_eNb_active == 1) {
      harq_pid = dlsch_eNb[0]->current_harq_pid;
      input_buffer_length = dlsch_eNb[0]->harq_processes[harq_pid]->TBS/8;
      for (i=0;i<input_buffer_length;i++)
	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      
      //#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dlsch with input size = %d\n",mac_xface->frame, next_slot, input_buffer_length);
      //#endif
      
      dlsch_encoding(dlsch_input_buffer,
		     lte_frame_parms,
		     dlsch_eNb[0]);
      
      for (eNb_id=0;eNb_id<number_of_cards;eNb_id++)
	
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

      //#ifdef DEBUG_PHY    
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d, DLSCH re_allocated = %d\n",mac_xface->frame, next_slot, re_allocated);
      //#endif

    }

    if (dlsch_eNb_cntl_active == 1) {
      input_buffer_length = dlsch_eNb_cntl->harq_processes[0]->TBS/8;
      fill_rar(dlsch_input_buffer,
	       lte_frame_parms->N_RB_UL,
	       input_buffer_length,
	       eNB_UE_stats[0].UE_timing_offset[0]/4); 

      // place PMI information for DLSCH at end of CNTL buffer for debug
      *(short *)(&dlsch_input_buffer[input_buffer_length-2]) = eNB_UE_stats[0].DL_pmi_single[0];
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Saving DL PMI %x in CNTL \n",mac_xface->frame, next_slot, pmi2hex_2Ar1(eNB_UE_stats[0].DL_pmi_single[0]));
      /*
      for (i=0;i<input_buffer_length;i++)
	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      */
      
#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dlsch (cntl) with input size = %d\n",mac_xface->frame, next_slot, input_buffer_length);
#endif
      
      dlsch_encoding(dlsch_input_buffer,
		     lte_frame_parms,
		     dlsch_eNb_cntl);
      
      for (eNb_id=0;eNb_id<number_of_cards;eNb_id++) 
	re_allocated = dlsch_modulation(lte_eNB_common_vars->txdataF[eNb_id],
					AMP,
					next_slot/2,
					lte_frame_parms,
					dlsch_eNb_cntl);
      dlsch_eNb_cntl_active = 0;

#ifdef DEBUG_PHY    
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d, DLSCH (cntl) re_allocated = %d\n",mac_xface->frame, next_slot, re_allocated);
#endif

    }

#ifdef OPENAIR2
    if (dlsch_eNb_ra_active == 1) {
      input_buffer_length = dlsch_eNb_ra->harq_processes[0]->TBS/8;
      eNB_UE_stats[0].UE_id[0] = fill_rar(dlsch_input_buffer,
					  lte_frame_parms->N_RB_UL,
					  input_buffer_length,
					  eNB_UE_stats[0].UE_timing_offset[0]/4); 
      eNB_UE_stats[0].mode[0] = RA_RESPONSE;
      //msg("Filling eNb_ulsch_params for RAR\n");
      generate_eNb_ulsch_params_from_rar(dlsch_input_buffer,
					 (next_slot>>1),
					 ulsch_eNb[0],
					 lte_frame_parms);
      ulsch_eNb[0]->RAG_active = 1;
      get_rag_alloc(lte_frame_parms->tdd_config,
		    next_slot>>1,
		    mac_xface->frame,
		    &ulsch_eNb[0]->RAG_frame,
		    &ulsch_eNb[0]->RAG_subframe);

      //      for (i=0;i<input_buffer_length;i++)
      //	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      
      //#ifdef DEBUG_PHY
	debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dlsch (RA) with input size = %d\n",mac_xface->frame, next_slot,input_buffer_length);
	//#endif
      
      dlsch_encoding(dlsch_input_buffer,
		     lte_frame_parms,
		     dlsch_eNb_ra);
      for (eNb_id=0;eNb_id<number_of_cards;eNb_id++) 
	re_allocated = dlsch_modulation(lte_eNB_common_vars->txdataF[eNb_id],
					AMP,
					next_slot/2,
					lte_frame_parms,
					dlsch_eNb_ra);
      dlsch_eNb_ra_active = 0;

#ifdef DEBUG_PHY    
      debug_msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d, DLSCH (RA) re_allocated = %d\n",mac_xface->frame, next_slot, re_allocated);
#endif
    }
#endif //OPENAIR2
    
    if (dlsch_eNb_1A_active == 1) {
      input_buffer_length = dlsch_eNb_1A->harq_processes[0]->TBS/8;
      for (i=0;i<input_buffer_length;i++)
	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
      
      //#ifdef DEBUG_PHY
	msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d: Calling generate_dlsch (1A) with input size = %d\n",mac_xface->frame, next_slot, input_buffer_length);
	//#endif
      
      dlsch_encoding(dlsch_input_buffer,
		     lte_frame_parms,
		     dlsch_eNb_1A);
      
      for (eNb_id=0;eNb_id<number_of_cards;eNb_id++) 
	re_allocated = dlsch_modulation(lte_eNB_common_vars->txdataF[eNb_id],
					AMP,
					next_slot/2,
					lte_frame_parms,
					dlsch_eNb_1A);
      dlsch_eNb_1A_active = 0;

      //#ifdef DEBUG_PHY    
      msg("[PHY_PROCEDURES_LTE] Frame %d, slot %d, DLSCH (1A) re_allocated = %d\n",mac_xface->frame, next_slot, re_allocated);
      //#endif

    }

  }



#ifdef EMOS
  phy_procedures_emos_eNB_TX(next_slot);
#endif
}
  


void phy_procedures_eNB_RX(unsigned char last_slot) {
  //RX processing
  unsigned int l, ret,i;
  unsigned int eNb_id=0,UE_id=0;
  int *ulsch_power;
  unsigned char harq_pid,rag_flag;
  static unsigned char first_run = 1;
  int sync_pos;
  
  //  debug_msg("Running phy_procedures_eNb_RX(%d), eNb_mode = %s\n",last_slot,mode_string[eNB_UE_stats[0].mode[UE_id]]);

  for (l=0;l<lte_frame_parms->symbols_per_tti/2;l++) {

    for (eNb_id=0;eNb_id<number_of_cards;eNb_id++) {
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

  
  eNb_id=0;
  if ((eNB_UE_stats[0].mode[UE_id]!=PRACH) && (last_slot%2==1)) {

    for (eNb_id=0;eNb_id<number_of_cards;eNb_id++) {
      debug_msg("srs meas eNb_id %d\n",l,eNb_id);

      lte_eNB_srs_measurements(lte_eNB_common_vars,
			       lte_frame_parms,
			       &eNB_UE_stats[0],
			       &PHY_vars->PHY_measurements_eNB[eNb_id],
			       eNb_id,
			       UE_id,
			       1);
      debug_msg("srs meas eNb_id %d\n",l,eNb_id);
    }

    eNb_id=0;
#ifdef USER_MODE
    /*
    write_output("srs_est0.m","srsest0",lte_eNB_common_vars->srs_ch_estimates[0][0],512,1,1);
    write_output("srs_est1.m","srsest1",lte_eNB_common_vars->srs_ch_estimates[0][1],512,1,1);
    */
#endif
    
    debug_msg("timing advance \n",l,eNb_id);
    sync_pos = lte_est_timing_advance(lte_frame_parms,
				      lte_eNB_common_vars,
				      &eNb_id,
				      first_run,
				      number_of_cards,
				      24576);

    debug_msg("timing advance \n",l,eNb_id);
    first_run = 0;

    eNB_UE_stats[0].UE_timing_offset[UE_id] = sync_pos - lte_frame_parms->nb_prefix_samples/8;
    eNB_UE_stats[0].sector[UE_id] = eNb_id;
    
    debug_msg("[PHY_PROCEDURES_LTE] frame %d, slot %d: user %d in sector %d: timing_advance = %d\n",
	mac_xface->frame, last_slot, 
	UE_id, eNb_id,
	eNB_UE_stats[0].UE_timing_offset[UE_id]);


  }

  debug_msg("Running ulsch reception: eNb_id %d (RAG active %d)\n",eNb_id,ulsch_eNb[0]->RAG_active);

  // Check for active processes in current subframe
  harq_pid = subframe2harq_pid_tdd(3,last_slot>>1);
  rag_flag = 0;
  if ((ulsch_eNb[0]->RAG_active == 1) && 
      ((last_slot%2)==1) && 
      (ulsch_eNb[0]->RAG_subframe == (last_slot>>1)) &&
      (ulsch_eNb[0]->RAG_frame == (mac_xface->frame)))   {
    harq_pid = 0;
    ulsch_eNb[0]->RAG_active = 0;
    ulsch_eNb[0]->harq_processes[0]->subframe_scheduling_flag=1;
    //    msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, subframe %d: Doing RAG for UE 0\n",mac_xface->frame,last_slot,last_slot>>1);
    rag_flag = 1;
  }

  if ((ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag==1) && ((last_slot%2)==1)) {
#ifdef DEBUG_PHY
    msg("[PHY PROCEDURES_LTE] frame %d, slot %d, subframe %d: Scheduling ULSCH Reception for harq_pid %d\n",mac_xface->frame,last_slot,last_slot>>1,harq_pid);
#endif

    if (rag_flag == 1)
      msg("[PHY PROCEDURES_LTE] frame %d, slot %d, subframe %d: Scheduling ULSCH Recption for RAG in Sector %d\n",
	  mac_xface->frame,last_slot,last_slot>>1,eNb_id);

    ulsch_power = rx_ulsch(lte_eNB_common_vars,
			   lte_eNB_ulsch_vars[0],  // this should be UE_id
			   lte_frame_parms,
			   last_slot>>1,
			   eNb_id,  // this is the effective sector id
			   UE_id,   // this is the UE instance to act upon
			   ulsch_eNb,
			   rag_flag);



    for (i=0;i<NB_ANTENNAS_RX;i++)
      eNB_UE_stats[0].UL_rssi[UE_id][i] = dB_fixed(ulsch_power[i]) - PHY_vars->rx_total_gain_eNB_dB;

#ifdef DEBUG_PHY
    msg("[PHY PROCEDURES_LTE] frame %d, slot %d, subframe %d: ULSCH RX power (%d,%d) dB\n",mac_xface->frame,last_slot,last_slot>>1,dB_fixed(ulsch_power[0]),dB_fixed(ulsch_power[1]));
#endif

    
    /*
    printf("ulsch (ue): NBRB     %d\n",ulsch_ue[0]->harq_processes[harq_pid]->nb_rb);
    printf("ulsch (ue): first_rb %x\n",ulsch_ue[0]->harq_processes[harq_pid]->first_rb);
    printf("ulsch (ue): nb_rb    %d\n",ulsch_ue[0]->harq_processes[harq_pid]->nb_rb);
    printf("ulsch (ue): Ndi      %d\n",ulsch_ue[0]->harq_processes[harq_pid]->Ndi);  
    printf("ulsch (ue): TBS      %d\n",ulsch_ue[0]->harq_processes[harq_pid]->TBS);
    printf("ulsch (ue): mcs      %d\n",ulsch_ue[0]->harq_processes[harq_pid]->mcs);
      //write_output("rxsigF0_ext.m","rxsF0_ext", lte_eNB_ulsch_vars[0]->rxdataF_ext[0][0],300*12*2,2,1);
    write_output("ulsch_rxF_comp0.m","ulsch0_rxF_comp0",&lte_eNB_ulsch_vars[0]->rxdataF_comp[0][0][0],300*12,1,1);
    write_output("ulsch_rxF_llr.m","ulsch_llr",lte_eNB_ulsch_vars[0]->llr,ulsch_ue[0]->harq_processes[harq_pid]->nb_rb*12*2*9,1,0);      
    write_output("drs_est0.m","drsest0",lte_eNB_ulsch_vars[0]->drs_ch_estimates[0][0],300*12,1,1);
    write_output("drs_est1.m","drsest1",lte_eNB_ulsch_vars[0]->drs_ch_estimates[0][1],300*12,1,1);
    

    */

    ret = ulsch_decoding(lte_eNB_ulsch_vars[0]->llr,
			 lte_frame_parms,
			 ulsch_eNb[0],
			 last_slot>>1,
			 rag_flag);    
    ulsch_decoding_attempts[harq_pid]++;

    ulsch_eNb[0]->harq_processes[harq_pid]->subframe_scheduling_flag=0;

      
    if (ulsch_eNb[0]->cqi_crc_status == 1) {
#ifdef DEBUG_PHY
      if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10)) 
      	print_CQI(ulsch_eNb[0]->o,ulsch_eNb[0]->o_RI,wideband_cqi,0);
#endif
      extract_CQI(ulsch_eNb[0]->o,ulsch_eNb[0]->o_RI,wideband_cqi,UE_id,&eNB_UE_stats[0]);
      eNB_UE_stats[0].rank[0] = ulsch_eNb[UE_id]->o_RI[0];
    }

    if (ret == (1+MAX_TURBO_ITERATIONS)) {
      ulsch_eNb[0]->harq_processes[harq_pid]->phich_active = 1;
      ulsch_eNb[UE_id]->harq_processes[harq_pid]->phich_ACK = 0;
      if (rag_flag == 1) {
	eNb_generate_rag_ack = 0;
	eNB_UE_stats[0].mode[0] = PRACH;
      }
      ulsch_errors[harq_pid]++;
      ulsch_consecutive_errors[harq_pid]++;

      // If we've dropped the UE, go back to PRACH mode for this UE
      if (ulsch_consecutive_errors[harq_pid] == 20) {
	eNB_UE_stats[0].mode[0] = PRACH;
	ulsch_consecutive_errors[harq_pid]=0;
      }
#ifdef USER_MODE
      if (rag_flag == 1) {
	dump_ulsch();
	exit(-1);
      }
#endif
    }
    else {
      ulsch_eNb[0]->harq_processes[harq_pid]->phich_active = 1;
      ulsch_eNb[0]->harq_processes[harq_pid]->phich_ACK = 1;
      ulsch_consecutive_errors[harq_pid] = 0;

      if (rag_flag == 1) {
	eNb_generate_rag_ack = 1;
	eNB_UE_stats[0].mode[0] = PUSCH;
	//	msg("[eNB Procedures] Frame %d : RX Subframe %d Setting UE mode to PUSCH\n",mac_xface->frame,last_slot>>1);
	//eNb_generate_rag_ack = 0;
	//eNB_UE_stats[0].mode[0] = PRACH;
	dlsch_NAK=0;
      }
    }

    if (rag_flag == 1) {
      msg("[PHY PROCEDURES LTE] frame %d, slot %d, subframe %d, eNB %d: received ULSCH (RAG) for UE %d, ret = %d, CQI CRC Status %d\n",mac_xface->frame, last_slot, last_slot>>1, eNb_id, UE_id, ret, ulsch_eNb[0]->cqi_crc_status);  
    }
    else {
      debug_msg("[PHY PROCEDURES LTE] frame %d, slot %d, subframe %d, eNB %d: received ULSCH harq_pid %d for UE %d, ret = %d, CQI CRC Status %d, ulsch_errors %d/%d\n",mac_xface->frame, last_slot, last_slot>>1, eNb_id, harq_pid, UE_id, ret, ulsch_eNb[0]->cqi_crc_status,ulsch_errors[harq_pid],ulsch_decoding_attempts[harq_pid]);  

      if (ulsch_eNb[0]->o_ACK[0] == 0)
	dlsch_NAK++;
    }
  }
    
#ifdef EMOS
  phy_procedures_emos_eNB_RX(last_slot);
#endif

}

  
