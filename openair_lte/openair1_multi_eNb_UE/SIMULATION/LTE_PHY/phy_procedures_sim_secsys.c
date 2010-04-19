#include <string.h>
#include <math.h>
#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif
#include "SCHED/defs.h"
#include "SCHED/vars.h"

#include "PHY/LTE_TRANSPORT/mcs_tbs_tools.h"
#define SECONDARY_SYSTEM
#ifdef SECONDARY_SYSTEM
#include "decl_secsys.h"
#endif
//#define SKIP_RF_CHAIN
#define BW 10.0
#define Td 1.0
#define N_TRIALS 1

#define DEBUG_PHY


DCI0_5MHz_TDD0_t          UL_alloc_pdu;
DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;

#define UL_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,24)
#define CCCH_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,2)
#define DLSCH_RB_ALLOC 0x1fff


int main(int argc, char **argv) {

  int i,l,aa,sector,i_max,l_max,aa_max;
  double sigma2, sigma2_dB=0;
  mod_sym_t **txdataF;
#ifdef SECONDARY_SYSTEM
  mod_sym_t **txdataF_secsys;
#endif
#ifdef IFFT_FPGA
  int **txdataF2;
#ifdef SECONDARY_SYSTEM
  int **txdataF2_secsys;
#endif
#endif
   int **txdata,**rxdata;
#ifdef SECONDARY_SYSTEM
   int **txdata_secsys, **rxdata_secsys;
#endif
  double **s_re,**s_im,**r_re,**r_im;
#ifdef SECONDARY_SYSTEM
  double **s_re_secsys,**s_im_secsys,**r_re_secsys,**r_im_secsys;
#endif
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=0.5; //0.0000005;
  int channel_length;
  struct complex **ch;
#ifdef SECONDARY_SYSTEM
  struct complex **ch_secsys;
#endif
  unsigned char pbch_pdu[6];
  int sync_pos, sync_pos_slot;
  FILE *rx_frame_file;
  int result;
  int freq_offset;
  int subframe_offset;
  char fname[40], vname[40];
  int trial, n_errors=0;
  unsigned int nb_rb = 25;
  unsigned int first_rb = 0;
  unsigned int eNb_id = 0;
#ifdef SECONDARY_SYSTEM
  unsigned int eNb_id_secsys = 0;
#endif
  unsigned int slot_offset, slot_offset_time;
  int n_frames;

  int slot,last_slot, next_slot;

  double nf[2] = {3.0,3.0}; //currently unused
  double ip =0.0;
  double N0W, path_loss, path_loss_dB, tx_pwr, rx_pwr;
#ifdef SECONDARY_SYSTEM
  double path_loss_secsys, path_loss_dB_secsys, tx_pwr_secsys, rx_pwr_secsys, SIR;
  int SIRdB = 0;
#endif
  int rx_pwr2;

  unsigned char first_call = 1, first_call_secsys = 1;

  char stats_buffer[4096];
  int len;

#ifdef EMOS
  fifo_dump_emos emos_dump;
#endif

  if (argc>1)
    sigma2_dB = atoi(argv[1]);

  if (argc>2)
    n_frames = atoi(argv[2]);
  else
    n_frames = 2;

  channel_length = (int) 11+2*BW*Td;

  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

#ifndef SECONDARY_SYSTEM
  PHY_VARS_eNB *PHY_vars_eNb[1]; // 1 eNBs
  PHY_vars_eNb[0] = malloc(sizeof(PHY_VARS_eNB));
  PHY_VARS_UE *PHY_vars_UE[1]; // 1 UEs
  PHY_vars_UE[0] = malloc(sizeof(PHY_VARS_UE));
#else //SECONDARY_SYSTEM
  PHY_VARS_eNB *PHY_vars_eNb[2]; // 2 eNBs
  PHY_vars_eNb[0] = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_eNb[1] = malloc(sizeof(PHY_VARS_eNB));
  PHY_VARS_UE *PHY_vars_UE[2]; // 2 UEs
  PHY_vars_UE[0] = malloc(sizeof(PHY_VARS_UE));
  PHY_vars_UE[1] = malloc(sizeof(PHY_VARS_UE));
  //  PHY_config_secsys = malloc(sizeof(PHY_CONFIG));
  //  mac_xfaec_secsys = malloc(sizeof(MAC_xface));
#endif

  //lte_frame_parms = &(PHY_config->lte_frame_parms);
  lte_frame_parms = &(PHY_vars_eNb[0]->lte_frame_parms);
  
  lte_frame_parms->N_RB_DL            = 25;
  lte_frame_parms->N_RB_UL            = 25;
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = 2;
  lte_frame_parms->nb_antennas_rx     = 2;
  lte_frame_parms->first_dlsch_symbol = 4;
  lte_frame_parms->num_dlsch_symbols  = 6;
  lte_frame_parms->Csrs = 2;
  lte_frame_parms->Bsrs = 0;
  lte_frame_parms->kTC = 0;
  lte_frame_parms->n_RRC = 0;

  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(NB_ANTENNAS_TX);
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  lte_gold(lte_frame_parms);
  generate_ul_ref_sigs();
  generate_ul_ref_sigs_rx();
  generate_64qam_table();
  generate_16qam_table();
  generate_RIV_tables();

  msg("[PHY_vars_UE] = %p",PHY_vars_UE);

  lte_sync_time_init(lte_frame_parms);

  //use same frame parameters for UE as for eNb
  PHY_vars_UE[0]->lte_frame_parms = *lte_frame_parms;
#ifdef SECONDARY_SYSTEM
  //use same frame parameters for secondary system
  PHY_vars_eNb[1]->lte_frame_parms = *lte_frame_parms;
  //use same frame parameters for UE as for eNb
  PHY_vars_UE[1]->lte_frame_parms = *lte_frame_parms;
#endif
 
  phy_init_lte_ue(lte_frame_parms,
		  &PHY_vars_UE[0]->lte_ue_common_vars,
		  PHY_vars_UE[0]->lte_ue_dlsch_vars,
		  PHY_vars_UE[0]->lte_ue_dlsch_vars_cntl,
		  PHY_vars_UE[0]->lte_ue_pbch_vars,
		  PHY_vars_UE[0]->lte_ue_pdcch_vars);
  PHY_vars_UE[0]->is_secondary_ue = 0;

  phy_init_lte_eNB(lte_frame_parms,
		   &PHY_vars_eNb[0]->lte_eNB_common_vars,
		   PHY_vars_eNb[0]->lte_eNB_ulsch_vars);
  PHY_vars_eNb[0]->is_secondary_eNb = 0;
  PHY_vars_eNb[0]->is_init_sync = 0; // not used for primary eNb
#ifdef SECONDARY_SYSTEM
  phy_init_lte_ue(lte_frame_parms,
		  &PHY_vars_UE[1]->lte_ue_common_vars,
		  PHY_vars_UE[1]->lte_ue_dlsch_vars,
		  PHY_vars_UE[1]->lte_ue_dlsch_vars_cntl,
		  PHY_vars_UE[1]->lte_ue_pbch_vars,
		  PHY_vars_UE[1]->lte_ue_pdcch_vars);
  PHY_vars_UE[1]->is_secondary_ue = 1;

  phy_init_lte_eNB(lte_frame_parms,
		   &PHY_vars_eNb[1]->lte_eNB_common_vars,
		   PHY_vars_eNb[1]->lte_eNB_ulsch_vars);
  PHY_vars_eNb[1]->is_secondary_eNb = 1;
  PHY_vars_eNb[1]->is_init_sync = 0;
#endif

#ifndef SECONDARY_SYSTEM
  aa_max = 1; //number of eNBs
  l_max = 1; //number of UEs
#else //SECONDARY_SYSTEM
  aa_max = 2; //number of eNBs
  l_max = 2; //number of UEs
#endif

//loop over eNBs
for (aa=0;aa<aa_max;aa++) {
  //loop over UEs
  for (l=0;l<l_max;l++) {
    //loop over transport channels per DLSCH
    for (i=0;i<2;i++) {
      PHY_vars_eNb[aa]->dlsch_eNb[l][i] = new_eNb_dlsch(1,8);
      if (!PHY_vars_eNb[aa]->dlsch_eNb[l][i]) {
	msg("Can't get eNb ulsch structures\n");
	exit(-1);
      } else {
	msg("PHY_vars_eNb[%d]->dlsch_eNb[%d][%d] = %p\n",aa,l,i,PHY_vars_eNb[aa]->dlsch_eNb[l][i]);
      }
      PHY_vars_UE[l]->dlsch_ue[aa][i] = new_ue_dlsch(1,8);
      if (!PHY_vars_UE[l]->dlsch_ue[aa][i]) {
	msg("Can't get ue ulsch structures\n");
	exit(-1);
      } else {
	msg("PHY_vars_UE [%d]->dlsch_ue [%d][%d] = %p\n",l,aa,i,PHY_vars_UE[l]->dlsch_ue[aa][i]);
      }
    }
  }
}

//loop over eNBs
for (aa=0;aa<aa_max;aa++) {
  //loop over UEs
  for (l=0;l<l_max;l++) {
    PHY_vars_eNb[aa]->ulsch_eNb[l] = new_eNb_ulsch(3);
    if (!PHY_vars_eNb[aa]->ulsch_eNb[l]) {
      msg("Can't get eNb ulsch structures\n");
      exit(-1);
    } else {
      msg("PHY_vars_eNb[%d]->ulsch_eNb[%d] = %p\n",aa,l,PHY_vars_eNb[aa]->ulsch_eNb[l]);
    }
    PHY_vars_UE[l]->ulsch_ue[aa] = new_ue_ulsch(3);
    if (!PHY_vars_UE[l]->ulsch_ue[aa]) {
      msg("Can't get ue ulsch structures\n");
      exit(-1);
    } else {
      msg("PHY_vars_UE [%d]->ulsch_ue [%d] = %p\n",l,aa,PHY_vars_UE[l]->ulsch_ue[aa]);
    }
  }
}

  PHY_vars_eNb[0]->dlsch_eNb_cntl = new_eNb_dlsch(1,1);
  PHY_vars_UE[0]->dlsch_ue_cntl  = new_ue_dlsch(1,1);
#ifdef SECONDARY_SYSTEM
  PHY_vars_eNb[1]->dlsch_eNb_cntl = new_eNb_dlsch(1,1);
  PHY_vars_UE[1]->dlsch_ue_cntl  = new_ue_dlsch(1,1);
#endif
 

  unsigned char m_mcs,m_I_tbs;
              //SE = 1
  m_I_tbs = SE2I_TBS(.5, lte_frame_parms->N_RB_DL, lte_frame_parms->num_dlsch_symbols);
  m_mcs = I_TBS2I_MCS(m_I_tbs);

  // init DCI structures for testing
  UL_alloc_pdu.type    = 0;
  UL_alloc_pdu.hopping = 0;
  UL_alloc_pdu.rballoc = UL_RB_ALLOC;
  UL_alloc_pdu.mcs     = 1;
  UL_alloc_pdu.ndi     = 1;
  UL_alloc_pdu.TPC     = 0;
  UL_alloc_pdu.cqi_req = 1;

  CCCH_alloc_pdu.type               = 0;
  CCCH_alloc_pdu.vrb_type           = 0;
  CCCH_alloc_pdu.rballoc            = CCCH_RB_ALLOC;
  CCCH_alloc_pdu.ndi      = 1;
  CCCH_alloc_pdu.rv       = 1;
  CCCH_alloc_pdu.mcs      = 1;
  CCCH_alloc_pdu.harq_pid = 0;

  DLSCH_alloc_pdu2.rah              = 0;
  DLSCH_alloc_pdu2.rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2.TPC              = 0;
  DLSCH_alloc_pdu2.dai              = 0;
  DLSCH_alloc_pdu2.harq_pid         = 1;
  DLSCH_alloc_pdu2.tb_swap          = 0;
  DLSCH_alloc_pdu2.mcs1             = 4; // m_mcs;
  DLSCH_alloc_pdu2.ndi1             = 1;
  DLSCH_alloc_pdu2.rv1              = 0;
  // Forget second codeword
  DLSCH_alloc_pdu2.tpmi             = 0;


#ifdef IFFT_FPGA
  txdata    = (int **)malloc16(2*sizeof(int*));
  txdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);

  bzero(txdata[0],FRAME_LENGTH_BYTES);
  bzero(txdata[1],FRAME_LENGTH_BYTES);

  rxdata    = (int **)malloc16(2*sizeof(int*));
  rxdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  rxdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);

  bzero(rxdata[0],FRAME_LENGTH_BYTES);
  bzero(rxdata[1],FRAME_LENGTH_BYTES);
  txdataF2    = (int **)malloc16(2*sizeof(int*));
  txdataF2[0] = (int *)malloc16(FRAME_LENGTH_BYTES_NO_PREFIX);
  txdataF2[1] = (int *)malloc16(FRAME_LENGTH_BYTES_NO_PREFIX);

  bzero(txdataF2[0],FRAME_LENGTH_BYTES_NO_PREFIX);
  bzero(txdataF2[1],FRAME_LENGTH_BYTES_NO_PREFIX);
#endif
  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
#ifdef SECONDARY_SYSTEM
  s_re_secsys = malloc(2*sizeof(double*));
  s_im_secsys = malloc(2*sizeof(double*));
  r_re_secsys = malloc(2*sizeof(double*));
  r_im_secsys = malloc(2*sizeof(double*));
#endif
  
  for (i=0;i<2;i++) {

    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_re[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_im[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_re[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_im[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));

#ifdef SECONDARY_SYSTEM
    s_re_secsys[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_re_secsys[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im_secsys[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_im_secsys[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re_secsys[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_re_secsys[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im_secsys[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_im_secsys[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
#endif
  }

  ch = (struct complex**) malloc(4 * sizeof(struct complex*));
  for (i = 0; i<4; i++)
    ch[i] = (struct complex*) malloc(channel_length * sizeof(struct complex));
#ifdef SECONDARY_SYSTEM
  ch_secsys = (struct complex**) malloc(4 * sizeof(struct complex*));
  for (i = 0; i<4; i++)
    ch_secsys[i] = (struct complex*) malloc(channel_length * sizeof(struct complex));
#endif

  randominit(0);
  set_taus_seed(0);

  openair_daq_vars.tdd = 1;
  openair_daq_vars.rx_gain_mode = DAQ_AGC_ON;
  PHY_vars_eNb[0]->rx_total_gain_dB = 140;
  PHY_vars_eNb[1]->rx_total_gain_dB = 140;

  for (mac_xface->frame=0; mac_xface->frame<n_frames; mac_xface->frame++) {

    for (slot=0 ; slot<20 ; slot++) {
      last_slot = (slot - 1)%20;
      if (last_slot <0)
	last_slot+=20;
      next_slot = (slot + 1)%20;
      
      /*-------------------------------------------------------------
	                   ALL LTE PROCESSING
      ---------------------------------------------------------------*/

      printf("\n");
      printf("Frame %d, slot %d : eNB procedures\n",mac_xface->frame,slot);
      mac_xface->is_cluster_head = 1;
      phy_procedures_eNb_lte(last_slot,next_slot,PHY_vars_eNb[0]);
#ifdef SECONDARY_SYSTEM
      phy_procedures_eNb_lte(last_slot,next_slot,PHY_vars_eNb[1]);
#endif
      printf("\n\n");
      printf("Frame %d, slot %d : UE procedures\n",mac_xface->frame,slot);
      mac_xface->is_cluster_head = 0;      
      phy_procedures_ue_lte(last_slot,next_slot,PHY_vars_UE[0]);
#ifdef SECONDARY_SYSTEM
      phy_procedures_ue_lte(last_slot,next_slot,PHY_vars_UE[1]);
#endif
      printf("\n");

      
      //      write_output("eNb_txsigF0.m","eNb_txsF0", PHY_vars_eNb->lte_eNB_common_vars->txdataF[eNb_id][0],300*120,1,4);
      //      write_output("eNb_txsigF1.m","eNb_txsF1", PHY_vars_eNb->lte_eNB_common_vars->txdataF[eNb_id][1],300*120,1,4);

      /*-------------------------------------------------------------
	ASSIGN POINTERS TO CORRECT BUFFERS ACCORDING TO TDD-STRUCTURE
	                  -----  TX PART  -----
                 and perform OFDM modulation ifndef IFFT_FPGA
      ---------------------------------------------------------------*/

      if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_DL) {
	txdataF = PHY_vars_eNb[0]->lte_eNB_common_vars.txdataF[eNb_id];
#ifdef SECONDARY_SYSTEM
	txdataF_secsys = PHY_vars_eNb[1]->lte_eNB_common_vars.txdataF[eNb_id_secsys];
#endif
#ifndef IFFT_FPGA
	txdata = PHY_vars_eNb[0]->lte_eNB_common_vars.txdata[eNb_id];
#ifdef SECONDARY_SYSTEM
	txdata_secsys = PHY_vars_eNb[1]->lte_eNB_common_vars.txdata[eNb_id_secsys];
#endif
#endif
      }
      else if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_UL) {
	txdataF = PHY_vars_UE[0]->lte_ue_common_vars.txdataF;
#ifdef SECONDARY_SYSTEM
	txdataF_secsys = PHY_vars_UE[1]->lte_ue_common_vars.txdataF;
#endif
#ifndef IFFT_FPGA
	txdata = PHY_vars_UE[0]->lte_ue_common_vars.txdata;
#ifdef SECONDARY_SYSTEM
	txdata_secsys = PHY_vars_UE[1]->lte_ue_common_vars.txdata;
#endif
#endif
      }
      else //it must be a special subframe
	//which also means that SECONDARY system must listen, and synchronize as an UE, every x(=10) frame(s). PSS located in the 3rd symbol in this slot.
	if (next_slot%2==0) {//DL part
	  txdataF = PHY_vars_eNb[0]->lte_eNB_common_vars.txdataF[eNb_id];
#ifdef SECONDARY_SYSTEM // SEC_SYS should be in Rx-mode here, primary will transmit
	  txdataF_secsys = PHY_vars_eNb[1]->lte_eNB_common_vars.txdataF[eNb_id_secsys]; // should point to NULL (though this will now just point to a lot of zeros, since phy_procedures routine will not generate PSS).
#endif
#ifndef IFFT_FPGA
	  txdata = PHY_vars_eNb[0]->lte_eNB_common_vars.txdata[eNb_id];
#ifdef SECONDARY_SYSTEM // SEC_SYS should be in Rx-mode here, primary will transmit
	  txdata_secsys = PHY_vars_eNb[1]->lte_eNB_common_vars.txdata[eNb_id_secsys]; // should point to NULL (though this will now just point to a lot of zeros, since phy_procedures routine will not generate PSS).
#endif
#endif
	}
	else {// UL part
	  txdataF = PHY_vars_UE[0]->lte_ue_common_vars.txdataF;
#ifdef SECONDARY_SYSTEM // SEC_SYS should be in Rx-mode here
	  txdataF_secsys = PHY_vars_UE[1]->lte_ue_common_vars.txdataF;
#endif
#ifndef IFFT_FPGA
	  txdata = PHY_vars_UE[0]->lte_ue_common_vars.txdata;
#ifdef SECONDARY_SYSTEM // SEC_SYS should be in Rx-mode here
	  txdata_secsys = PHY_vars_UE[1]->lte_ue_common_vars.txdata;
#endif
#endif
	}


#ifdef IFFT_FPGA

      slot_offset = (next_slot)*(lte_frame_parms->N_RB_DL*12)*((lte_frame_parms->Ncp==1) ? 6 : 7);

      //write_output("eNb_txsigF0.m","eNb_txsF0", PHY_vars_eNb->lte_eNB_common_vars->txdataF[eNb_id][0],300*120,1,4);
      //write_output("eNb_txsigF1.m","eNb_txsF1", PHY_vars_eNb->lte_eNB_common_vars->txdataF[eNb_id][1],300*120,1,4);

      
      // do table lookup and write results to txdataF2
      for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {

	l = slot_offset;	
	for (i=0;i<NUMBER_OF_OFDM_CARRIERS*((lte_frame_parms->Ncp==1) ? 6 : 7);i++) 
	  if ((i%512>=1) && (i%512<=150))
	    txdataF2[aa][i] = ((int*)mod_table)[txdataF[aa][l++]];
	  else if (i%512>=362)
	    txdataF2[aa][i] = ((int*)mod_table)[txdataF[aa][l++]];
	  else 
	    txdataF2[aa][i] = 0;
#ifdef SECONDARY_SYSTEM
	  if ((i%512>=1) && (i%512<=150))
	    txdataF2_secsys[aa][i] = ((int*)mod_table)[txdataF_secsys[aa][l++]];
	  else if (i%512>=362)
	    txdataF2_secsys[aa][i] = ((int*)mod_table)[txdataF_secsys[aa][l++]];
	  else 
	    txdataF2_secsys[aa][i] = 0;
#endif
      }
           
#ifdef DEBUG_PHY
      if (next_slot <= 1) {
	sprintf(fname,"eNb_frame%d_slot%d_txsigF20.m",mac_xface->frame,next_slot);
	write_output(fname,"eNb_txsF0",txdataF2[0],512*6,1,1);
	sprintf(fname,"eNb_frame%d_slot%d_txsigF21.m",mac_xface->frame,next_slot);
	write_output(fname,"eNb_txsF1",txdataF2[1],512*6,1,1);
      }
#endif
      
      for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) 
	PHY_ofdm_mod(txdataF2[aa],        // input
		     txdata[aa],         // output
		     lte_frame_parms->log2_symbol_size,                // log2_fft_size
		     (lte_frame_parms->Ncp==1) ? 6 : 7,                 // number of symbols
		     lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		     lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		     lte_frame_parms->rev,           // bit-reversal permutation
		     CYCLIC_PREFIX);
#ifdef SECONDARY_SYSTEM
      for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) 
	PHY_ofdm_mod(txdataF2_secsys[aa],        // input
		     txdata_secsys[aa],         // output
		     lte_frame_parms->log2_symbol_size,
		     (lte_frame_parms->Ncp==1) ? 6 : 7,
		     lte_frame_parms->nb_prefix_samples
		     lte_frame_parms->twiddle_ifft,
		     lte_frame_parms->rev,
		     CYCLIC_PREFIX);
#endif //SECONDARY_SYSTEM
      
#else //IFFT_FPGA

      slot_offset = (next_slot)*(lte_frame_parms->ofdm_symbol_size)*((lte_frame_parms->Ncp==1) ? 6 : 7);
      //      printf("Copying TX buffer for slot %d (%d)\n",next_slot,slot_offset);
      slot_offset_time = (next_slot)*(lte_frame_parms->samples_per_tti>>1);

#ifdef DEBUG_PHY
      if (next_slot == 7 || next_slot == 9) {
	sprintf(fname,"UE_frame%d_slot%d_txsigF0.m",mac_xface->frame,next_slot);
	write_output(fname,"UE_txsF0",&txdataF[0][slot_offset],512*12,1,1);
	sprintf(fname,"UE_frame%d_slot%d_txsigF1.m",mac_xface->frame,next_slot);
	write_output(fname,"UE_txsF1",&txdataF_secsys[0][slot_offset],512*12,1,1); 
      }
#endif
      /*
      if (next_slot == 2) {
	sprintf(fname,"UE_frame%d_txsigF0.m",mac_xface->frame);
	write_output(fname,"UE_txsF0",&txdataF[0][slot_offset],512*12,1,1);
	sprintf(fname,"UE_frame%d_txsigF1.m",mac_xface->frame);
	write_output(fname,"UE_txsF1",&txdataF[1][slot_offset],512*12,1,1);
      }
      */

      for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
	PHY_ofdm_mod(&txdataF[aa][slot_offset],        // input
		     &txdata[aa][slot_offset_time],    // output
		     lte_frame_parms->log2_symbol_size,                // log2_fft_size
		     (lte_frame_parms->Ncp==1) ? 6 : 7,                 // number of symbols
		     lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		     lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		     lte_frame_parms->rev,           // bit-reversal permutation
		     CYCLIC_PREFIX);
      } 
 
#ifdef SECONDARY_SYSTEM
      for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
	PHY_ofdm_mod(&txdataF_secsys[aa][slot_offset],// input
		     &txdata_secsys[aa][slot_offset_time],// output
		     lte_frame_parms->log2_symbol_size,
		     (lte_frame_parms->Ncp==1) ? 6 : 7,
		     lte_frame_parms->nb_prefix_samples,
		     lte_frame_parms->twiddle_ifft,
		     lte_frame_parms->rev, CYCLIC_PREFIX);
      }  
#endif
#endif //IFFT_FPGA

#ifdef DEBUG_PHY
      if (next_slot == 7 || next_slot == 9) {
	sprintf(fname,"UE_frame%d_slot%d_txsig0.m",mac_xface->frame,next_slot);
	write_output(fname,"UE_txs0",&txdata[0][slot_offset_time],640*12,1,1);
	sprintf(fname,"UE_frame%d_slot%d_txsig1.m",mac_xface->frame,next_slot);
	write_output(fname,"UE_txs1",&txdata_secsys[0][slot_offset_time],640*12,1,1);
      }
#endif
 
#ifdef DEBUG_PHY

      if (next_slot == 9) {
	sprintf(fname,"txsig.m");
        write_output(fname,"txs",&(PHY_vars_UE[0]->lte_ue_common_vars.txdata[0][slot_offset_time]),lte_frame_parms->samples_per_tti>>1,1,1);
	sprintf(fname,"txsig_tmp.m");
        write_output(fname,"txs_tmp",&(txdata[0][slot_offset_time]),lte_frame_parms->samples_per_tti>>1,1,1);
      }

      if (next_slot == 11) {
	sprintf(fname,"eNb_frame%d_slot%d_txsig0.m",mac_xface->frame,next_slot);
        write_output(fname,"eNb_txs0",&(PHY_vars_eNb[0]->lte_eNB_common_vars.txdata[eNb_id][0][slot_offset_time]),lte_frame_parms->samples_per_tti,1,1);
	sprintf(fname,"eNb_frame%d_slot%d_txsig0_tmp.m",mac_xface->frame,next_slot);
        write_output(fname,"eNb_txs0_tmp",&(txdata[0][slot_offset_time]),lte_frame_parms->samples_per_tti,1,1);
	sprintf(fname,"eNb_frame%d_slot%d_txsigF0.m",mac_xface->frame,next_slot);
        write_output(fname,"eNb_txsF0",&(txdataF[0][slot_offset-lte_frame_parms->ofdm_symbol_size*((lte_frame_parms->Ncp==1) ? 6 : 7)]),lte_frame_parms->ofdm_symbol_size*((lte_frame_parms->Ncp==1) ? 6 : 7)*2,1,1);
      }
      
      if (next_slot == 4) {
	sprintf(fname,"UE_frame%d_slot%d_txsig0.m",mac_xface->frame,next_slot);
	write_output(fname,"UE_txs0",&(txdata[0][slot_offset_time]),lte_frame_parms->samples_per_tti,1,1);
	sprintf(fname,"UE_frame%d_slot%d_txsig1.m",mac_xface->frame,next_slot);
	write_output(fname,"UE_txs1",&(txdata[1][slot_offset_time]),lte_frame_parms->samples_per_tti,1,1);
      }
#endif
      
      // get pointer to data ready to be transmitted
      for (i=0;i<(lte_frame_parms->samples_per_tti>>1);i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
	  s_re[aa][i] = (double)(((short *)txdata[aa])[(slot_offset_time+i)<<1]);
	  s_im[aa][i] = (double)(((short *)txdata[aa])[((slot_offset_time+i)<<1)+1]);
#ifdef SECONDARY_SYSTEM
	  s_re_secsys[aa][i] = (double)(((short *)txdata_secsys[aa])[(slot_offset_time+i)<<1]);
	  s_im_secsys[aa][i] = (double)(((short *)txdata_secsys[aa])[((slot_offset_time+i)<<1)+1]);
#endif
	}
      }
    
      /*-------------------------------------------------------------
	ASSIGN POINTERS TO CORRECT BUFFERS ACCORDING TO TDD-STRUCTURE
	                  -----  RX PART  -----
      ---------------------------------------------------------------*/
  
  if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_DL)
    rxdata = PHY_vars_UE[0]->lte_ue_common_vars.rxdata;
  else if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_UL)
    rxdata = PHY_vars_eNb[0]->lte_eNB_common_vars.rxdata[eNb_id];
  else //it must be a special subframe
    if (next_slot%2==0) //DL part
      rxdata = PHY_vars_UE[0]->lte_ue_common_vars.rxdata;
    else // UL part
      rxdata = PHY_vars_eNb[0]->lte_eNB_common_vars.rxdata[eNb_id];
 

#ifdef SECONDARY_SYSTEM

  if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_DL)
    rxdata_secsys = PHY_vars_UE[1]->lte_ue_common_vars.rxdata;
  else if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_UL)
    rxdata_secsys = PHY_vars_eNb[1]->lte_eNB_common_vars.rxdata[eNb_id_secsys];
  else //it must be a special subframe
    //which also means that SECONDARY system must listen, and synchronize as an UE, every x(=10) frame(s) or every frame the on power up. PSS is located in the 3rd symbol in this slot.
    if (PHY_vars_eNb[1]->is_init_sync && mac_xface->frame%10>0) {
      if (next_slot%2==0) //DL part
	rxdata_secsys = PHY_vars_UE[1]->lte_ue_common_vars.rxdata;
      else // UL part
	rxdata_secsys = PHY_vars_eNb[1]->lte_eNB_common_vars.rxdata[eNb_id_secsys];
    } 
    else // UL part
      	rxdata_secsys = PHY_vars_eNb[1]->lte_eNB_common_vars.rxdata[eNb_id_secsys];


#endif //SECONDARY_SYSTEM

      /*-------------------------------------------------------------
	                 TRANSMISSION SIMULATION
      ---------------------------------------------------------------*/
  
#ifdef SKIP_RF_CHAIN
#ifdef SECONDARY_SYSTEM
  SIR = pow(10,(double)(SIRdB)/10);
#endif
      for (i=0;i<(lte_frame_parms->samples_per_tti>>1);i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
	  r_re[aa][i] = s_re[aa][i];
	  r_im[aa][i] = s_im[aa][i];
#ifdef SECONDARY_SYSTEM
	  r_re_secsys[aa][i] = s_re_secsys[aa][i] +s_re[aa][i]*sqrt(1.0/(2*SIR));
	  r_im_secsys[aa][i] = s_im_secsys[aa][i] +s_im[aa][i]*sqrt(1.0/(2*SIR));
#endif
	}
      }

  slot_offset_time = next_slot*(lte_frame_parms->samples_per_tti>>1);
      for (i=0;i<(lte_frame_parms->samples_per_tti>>1);i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
	  ((short *)rxdata[aa])[((i+slot_offset_time)<<1)]   = (short)(r_re[aa][i+0]*1);
	  ((short *)rxdata[aa])[1+((i+slot_offset_time)<<1)] = (short)(r_im[aa][i+0]*1);
#ifdef SECONDARY_SYSTEM
	  ((short *)rxdata_secsys[aa])[((i+slot_offset_time)<<1)]   = (short)(r_re_secsys[aa][i+0]*1);
	  ((short *)rxdata_secsys[aa])[1+((i+slot_offset_time)<<1)] = (short)(r_im_secsys[aa][i+0]*1);
#endif
	}
      }

#else //SKIP_RF_CHAIN

      /*-------------------------------------------------------------
                                   D/A
      ---------------------------------------------------------------*/

      // convert to floating point
      tx_pwr = dac_fixed_gain(s_re,
			      s_im,
			      txdata,
			      slot_offset_time,
			      2,
			      lte_frame_parms->samples_per_tti>>1,
			      14,
			      0);
      printf("tx_pwr %f dB for slot %d (subframe %d)\n",10*log10(tx_pwr),next_slot,next_slot>>1);

#ifdef SECONDARY_SYSTEM
   
      // convert to floating point
      tx_pwr = dac_fixed_gain(s_re_secsys,
			      s_im_secsys,
			      txdata_secsys,
			      slot_offset_time,
			      2,
			      lte_frame_parms->samples_per_tti>>1,
			      14,
			      0);
      printf("tx_pwr_secsys %f dB for slot %d (subframe %d)\n",10*log10(tx_pwr),next_slot,next_slot>>1);

#endif

     
      /*-------------------------------------------------------------
	                     CHANNEL MODEL 
                         ANTENNA(s) TO ANTENNA(s)
      ---------------------------------------------------------------*/

//      printf("channel for slot %d (subframe %d)\n",next_slot,next_slot>>1);
      multipath_channel(ch,s_re,s_im,r_re,r_im,
			amps,Td,BW,ricean_factor,aoa,
			lte_frame_parms->nb_antennas_tx,
			lte_frame_parms->nb_antennas_rx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call == 1) ? 1 : 0));
      if (first_call == 1)
	first_call = 0;

      //write_output("channel0.m","chan0",ch[0],channel_length,1,8);


#ifdef SECONDARY_SYSTEM   
//      printf("channel for slot %d (subframe %d)\n",next_slot,next_slot>>1);

      multipath_channel(ch_secsys,s_re_secsys,s_im_secsys,
			r_re_secsys,r_im_secsys,
			amps,Td,BW,ricean_factor,aoa,
			lte_frame_parms->nb_antennas_tx,
			lte_frame_parms->nb_antennas_rx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call_secsys == 1) ? 1 : 0));
      if (first_call_secsys == 1)
	first_call_secsys = 0;
#endif

      path_loss_dB = -70;
      path_loss    = pow(10,path_loss_dB/10);
#ifdef SECONDARY_SYSTEM
      path_loss_dB_secsys = -70;
      path_loss_secsys    = pow(10,path_loss_dB_secsys/10);
#endif //SECONDARY_SYSTEM
      
      for (i=0;i<(lte_frame_parms->samples_per_tti>>1);i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_rx;aa++) {
	  r_re[aa][i]=r_re[aa][i]*sqrt(path_loss/2); 
	  r_im[aa][i]=r_im[aa][i]*sqrt(path_loss/2);
#ifdef SECONDARY_SYSTEM
	  r_re_secsys[aa][i]=r_re_secsys[aa][i]*sqrt(path_loss_secsys/2); 
	  r_im_secsys[aa][i]=r_im_secsys[aa][i]*sqrt(path_loss_secsys/2); 
	  // add interference
	  r_re_secsys[aa][i]=r_re_secsys[aa][i] + r_re[aa][i]; 
	  r_im_secsys[aa][i]=r_im_secsys[aa][i] + r_im[aa][i]; 
#endif //SECONDARY_SYSTEM
	}
      }

      /*-------------------------------------------------------------
	                       RF MODELLING 
                                 RX PART
      ---------------------------------------------------------------*/

      rf_rx(r_re,
	    r_im,
	    NULL,
	    NULL,
	    0,
	    lte_frame_parms->nb_antennas_rx,
	    lte_frame_parms->samples_per_tti>>1,
	    1.0/7.68e6 * 1e9,      // sampling time (ns)
	    500,            // freq offset (Hz) (-20kHz..20kHz)
	    0.0,            // drift (Hz) NOT YET IMPLEMENTED
	    nf,             // noise_figure NOT YET IMPLEMENTED
	    (double)PHY_vars_eNb[0]->rx_total_gain_dB-72.247,            // rx_gain (dB)
	    200,            // IP3_dBm (dBm)
	    &ip,            // initial phase
	    30.0e3,         // pn_cutoff (kHz)
	    -500.0,          // pn_amp (dBc) default: 50
	    0.0,           // IQ imbalance (dB),
	    0.0);           // IQ phase imbalance (rad)

      rx_pwr = signal_energy_fp(r_re,r_im,lte_frame_parms->nb_antennas_rx,lte_frame_parms->samples_per_tti>>1,0);
 
      printf("rx_pwr (ADC in) %f dB for slot %d (subframe %d)\n",10*log10(rx_pwr),next_slot,next_slot>>1); 


#ifdef SECONDARY_SYSTEM
      // RF model
      rf_rx(r_re_secsys,
	    r_im_secsys,
	    NULL,
	    NULL,
	    0,
	    lte_frame_parms->nb_antennas_rx,
	    lte_frame_parms->samples_per_tti>>1,
	    1.0/7.68e6 * 1e9,      // sampling time (ns)
	    500,            // freq offset (Hz) (-20kHz..20kHz)
	    0.0,            // drift (Hz) NOT YET IMPLEMENTED
	    nf,             // noise_figure NOT YET IMPLEMENTED
	    (double)PHY_vars_eNb[1]->rx_total_gain_dB-72.247,            // rx_gain (dB)
	    200,            // IP3_dBm (dBm)
	    &ip,            // initial phase
	    30.0e3,         // pn_cutoff (kHz)
	    -500.0,          // pn_amp (dBc) default: 50
	    0.0,           // IQ imbalance (dB),
	    0.0);           // IQ phase imbalance (rad)
      rx_pwr = signal_energy_fp(r_re_secsys,r_im_secsys,lte_frame_parms->nb_antennas_rx,lte_frame_parms->samples_per_tti>>1,0);
 
      printf("rx_pwr_secsys (ADC in) %f dB for slot %d (subframe %d)\n",10*log10(rx_pwr),next_slot,next_slot>>1); 

#endif //SECONDARY_SYSTEM
     
      /*-------------------------------------------------------------
	                     A/D CONVERSION
			     (QUANTIZATION)
      ---------------------------------------------------------------*/

  adc(r_re,
      r_im,
      0,
      slot_offset_time,
      rxdata,
      2,
      lte_frame_parms->samples_per_tti>>1,
      12);
  
      if (next_slot == 10) {
	sprintf(fname,"eNb_frame%d_slot%d_txsig0.m",mac_xface->frame,next_slot);
        write_output(fname,"eNb_txs0",r_re[0],lte_frame_parms->samples_per_tti,1,7);
	sprintf(fname,"eNb_frame%d_slot%d_txsig0_tmp.m",mac_xface->frame,next_slot);
        write_output(fname,"eNb_txs0_tmp",&(rxdata[0][slot_offset_time]),lte_frame_parms->samples_per_tti,1,1);
      }

  rx_pwr2 = signal_energy(rxdata[0]+slot_offset,lte_frame_parms->samples_per_tti>>1);
  
  printf("rx_pwr (ADC out) %f dB (%d) for slot %d (subframe %d)\n",10*log10((double)rx_pwr2),rx_pwr2,next_slot,next_slot>>1);  
  
#ifdef SECONDARY_SYSTEM

  slot_offset = 2*(next_slot)*(lte_frame_parms->samples_per_tti>>1);

  adc(r_re_secsys,
      r_im_secsys,
      0,
      slot_offset_time,
      rxdata_secsys,
      2,
      lte_frame_parms->samples_per_tti>>1,
      12);
  
  rx_pwr2 = signal_energy(rxdata_secsys[0]+slot_offset,lte_frame_parms->samples_per_tti>>1);
  
  printf("rx_pwr_secsys (ADC out) %f dB (%d) for slot %d (subframe %d)\n",10*log10((double)rx_pwr2),rx_pwr2,next_slot,next_slot>>1); 

   if (last_slot==5)  //phy_vars_eNb->is_secondary_eNb && 
    write_output("srs_received.m","srs_rx", 
		 &rxdata_secsys[0][slot_offset_time],
		 PHY_vars_eNb[1]->lte_frame_parms.samples_per_tti>>1,
		 1,1); //phy_vars_eNb->lte_frame_parms.ofdm_symbol_size

#endif //SECONDARY_SYSTEM
#endif //SKIP_RF_CHAIN

#ifdef DEBUG_PHY
      if (next_slot == 7 || next_slot == 9) {
	sprintf(fname,"UE_frame%d_slot%d_rxsig0.m",mac_xface->frame,next_slot);
	write_output(fname,"UE_rxs0",&rxdata[0][slot_offset_time],640*12,1,1);
	sprintf(fname,"UE_frame%d_slot%d_rxsig1.m",mac_xface->frame,next_slot);
	write_output(fname,"UE_rxs1",&rxdata_secsys[0][slot_offset_time],640*12,1,1);
      }
#endif

  if (last_slot == 19) {
    write_output("UE_rxsig0.m","UE_rxs0", PHY_vars_UE[0]->lte_ue_common_vars.rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("UE_rxsig1.m","UE_rxs1", PHY_vars_UE[0]->lte_ue_common_vars.rxdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_rxsig0.m","eNb_rxs0", PHY_vars_eNb[0]->lte_eNB_common_vars.rxdata[eNb_id][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_rxsig1.m","eNb_rxs1", PHY_vars_eNb[0]->lte_eNB_common_vars.rxdata[eNb_id][1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
 
    write_output("UE_txsig0.m","UE_txs0", PHY_vars_UE[0]->lte_ue_common_vars.txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("UE_txsig1.m","UE_txs1", PHY_vars_UE[0]->lte_ue_common_vars.txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_txsig0.m","eNb_txs0", PHY_vars_eNb[0]->lte_eNB_common_vars.txdata[eNb_id][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_txsig1.m","eNb_txs1", PHY_vars_eNb[0]->lte_eNB_common_vars.txdata[eNb_id][1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  }

#ifdef SECONDARY_SYSTEM
if (last_slot == 19) {
    write_output("UE_rxsig0_1.m","UE_rxs0_1", PHY_vars_UE[1]->lte_ue_common_vars.rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("UE_rxsig1_1.m","UE_rxs1_1", PHY_vars_UE[1]->lte_ue_common_vars.rxdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_rxsig0_1.m","eNb_rxs0_1", PHY_vars_eNb[1]->lte_eNB_common_vars.rxdata[eNb_id][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_rxsig1_1.m","eNb_rxs1_1", PHY_vars_eNb[1]->lte_eNB_common_vars.rxdata[eNb_id][1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
 
    write_output("UE_txsig0_1.m","UE_txs0_1", PHY_vars_UE[1]->lte_ue_common_vars.txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("UE_txsig1_1.m","UE_txs1_1", PHY_vars_UE[1]->lte_ue_common_vars.txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_txsig0_1.m","eNb_txs0_1", PHY_vars_eNb[1]->lte_eNB_common_vars.txdata[eNb_id_secsys][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_txsig1_1.m","eNb_txs1_1", PHY_vars_eNb[1]->lte_eNB_common_vars.txdata[eNb_id_secsys][1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  }
#endif //SECONDARY_SYSTEM
  
  /*
  // optional: read rx_frame from file
  if ((rx_frame_file = fopen("rx_frame.dat","r")) == NULL)
    {
      printf("[openair][CHBCH_TEST][INFO] Cannot open rx_frame.m data file\n");
      exit(0);
    }

  fclose(rx_frame_file);
  */
    }
  }

#ifdef IFFT_FPGA
  free(txdataF2[0]);
  free(txdataF2[1]);
  free(txdataF2);
  free(txdata[0]);
  free(txdata[1]);
  free(txdata);
#endif 

  for (i=0;i<2;i++) {
    free(s_re[i]);
    free(s_im[i]);
    free(r_re[i]);
    free(r_im[i]);
  }
  free(s_re);
  free(s_im);
  free(r_re);
  free(r_im);

#ifdef SECONDARY_SYSTEM
for (i=0;i<2;i++) {
    free(s_re_secsys[i]);
    free(s_im_secsys[i]);
    free(r_re_secsys[i]);
    free(r_im_secsys[i]);
  }
  free(s_re_secsys);
  free(s_im_secsys);
  free(r_re_secsys);
  free(r_im_secsys);
#endif //SECONDARY_SYSTEM
  
  lte_sync_time_free();

  return(n_errors);
}
