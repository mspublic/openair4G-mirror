#include <string.h>
#include <math.h>
#include <unistd.h>
#include "SIMULATION/TOOLS/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif

#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"
#include "SCHED/defs.h"
#include "SCHED/vars.h"

#define AWGN
#define NO_DCI

#define BW 7.68
#define Td 1.0

//#define OUTPUT_DEBUG 1

#define RBmask0 0x00fc00fc
#define RBmask1 0x0
#define RBmask2 0x0
#define RBmask3 0x0

/*
  unsigned char dlsch_cqi;

  PHY_VARS_eNB *PHY_vars_eNb;
  PHY_VARS_UE *PHY_vars_UE;

  void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode) {

  unsigned int ind;

  printf("Start lte_param_init\n");
  PHY_vars_eNb = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  randominit(0);
  set_taus_seed(0);
  
  lte_frame_parms = &(PHY_vars_eNb->lte_frame_parms);

  lte_frame_parms->N_RB_DL            = 25;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->N_RB_UL            = 25;   
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  lte_frame_parms->first_dlsch_symbol = 4;
  lte_frame_parms->num_dlsch_symbols  = 6;
  lte_frame_parms->mode1_flag = (transmission_mode == 1)? 1 : 0;

  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(N_tx); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  PHY_vars_UE->lte_frame_parms = *lte_frame_parms;
  
  lte_gold(lte_frame_parms);
  generate_ul_ref_sigs();
  generate_ul_ref_sigs_rx();
  generate_64qam_table();
  generate_16qam_table();
  generate_RIV_tables();

  generate_pcfich_reg_mapping(lte_frame_parms);
  generate_phich_reg_mapping_ext(lte_frame_parms);

  phy_init_lte_ue(&PHY_vars_UE->lte_frame_parms,
  &PHY_vars_UE->lte_ue_common_vars,
  PHY_vars_UE->lte_ue_dlsch_vars,
  PHY_vars_UE->lte_ue_dlsch_vars_SI,
  PHY_vars_UE->lte_ue_dlsch_vars_ra,
  PHY_vars_UE->lte_ue_pbch_vars,
  PHY_vars_UE->lte_ue_pdcch_vars,
  PHY_vars_UE);

  phy_init_lte_eNB(&PHY_vars_eNb->lte_frame_parms,
  &PHY_vars_eNb->lte_eNB_common_vars,
  PHY_vars_eNb->lte_eNB_ulsch_vars,
  0,
  PHY_vars_eNb);

  
  printf("Done lte_param_init\n");


  }
*/

#define UL_RB_ALLOC 0x1ff;
#define CCCH_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,2)
#define DLSCH_RB_ALLOC 0x1fbf // igore DC component,RB13
	     //#define DLSCH_RB_ALLOC 0x1f0f // igore DC component,RB13


int main(int argc, char **argv) {

  char c;
  int i,j,aa,s,ind,Kr,Kr_bytes;;
  double sigma2,sigma2_dB,SNR,snr0,snr1,SNRmeas;
  //int **txdataF, **txdata;
  int **txdata;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  double **s_re,**s_im,**r_re,**r_im;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=1; //0.0000005;
  int channel_length,nb_taps=8;
  double forgetting_factor=0.99,maxDoppler=0;

  int eNb_id = 0, eNb_id_i = 1;
  unsigned char mcs,dual_stream_UE = 0;
  unsigned short NB_RB=conv_nprb(0,DLSCH_RB_ALLOC);
  unsigned char Ns,l,m;

  unsigned char *input_data,*decoded_output;

  unsigned char *input_buffer;
  unsigned short input_buffer_length;
  unsigned int ret;
  unsigned int coded_bits_per_codeword,nsymb,dci_cnt;

  unsigned int tx_lev,tx_lev_dB,trials,errs=0,dci_errors=0,dlsch_active=0;
  unsigned int transmission_mode, num_layers;
  int re_allocated;
  FILE *bler_fd;
  FILE *csv_fd;
  char bler_fname[20];
  char csv_fname[20];

  unsigned char pbch_pdu[6];

  DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];
  //DCI0_5MHz_TDD0_t          UL_alloc_pdu;
  DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
  //DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
  DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2[NUMBER_OF_UE_MAX];
  int num_common_dci=0,num_ue_spec_dci=0;

  FILE *rx_frame_file;
  int result;

  int n_frames;
  int cnt=0; 
  int rx_lev_data_sym;
  int rx_lev_null_sym;
  int rx_snr_dB;
  void *data;
  int ii;
  int bler;
  double blerr;
  int ch_realization;

  int eNB_id,UE_id,NB_UE_INST=2,NB_CH_INST=1;

  channel_desc_t *eNB2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];
  channel_desc_t *UE2eNB[NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX];

  // Init simulation parameters

  transmission_mode = 5;
  num_layers = 1;
  mcs = 0;
  n_frames = 1;
  snr0 = 10;
  //if(snr0>0)
  // snr0 = 0;
  while ((c = getopt (argc, argv, "hm:n:s:")) != -1)
    {
      switch (c)
	{
	case 'h':
	  printf("%s -h(elp) -m mcs -n n_frames -s snr0\n",argv[0]);
	  exit(1);
	case 'm':
	  mcs = atoi(optarg);
	  break;
	case 'n':
	  n_frames = atoi(optarg);
	  break;
	case 's':
	  snr0 = atoi(optarg);
	  break;
	default:
	  printf("%s -h(elp) -m mcs -n n_frames -s snr0\n",argv[0]);
	  exit (-1);
	  break;
	}
    }
  
  printf("Setting mcs = %d\n",mcs);
  printf("NPRB = %d\n",NB_RB);
  printf("n_frames = %d\n",n_frames);

  snr1 = snr0+0.1;
  printf("SNR0 %f, SNR1 %f\n",snr0,snr1);

  // Init PHY parameters
      
  PHY_vars_eNb_g = malloc(NB_CH_INST*sizeof(PHY_VARS_eNB*));
  for (eNB_id=0; eNB_id<NB_CH_INST;eNB_id++){ 
    PHY_vars_eNb_g[eNB_id] = malloc(sizeof(PHY_VARS_eNB));
    PHY_vars_eNb_g[eNB_id]->Mod_id=eNB_id;
  }
  //  PHY_VARS_UE *PHY_vars_UE; 
  PHY_vars_UE_g = malloc(NB_UE_INST*sizeof(PHY_VARS_UE*));
  for (UE_id=0; UE_id<NB_UE_INST;UE_id++){ // begin navid
    PHY_vars_UE_g[UE_id] = malloc(sizeof(PHY_VARS_UE));
    PHY_vars_UE_g[UE_id]->Mod_id=UE_id; 
  }// end navid

  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  lte_frame_parms = malloc(sizeof(LTE_DL_FRAME_PARMS));
  lte_frame_parms->N_RB_DL            = 25;
  lte_frame_parms->N_RB_UL            = 25;
  lte_frame_parms->Ng_times6          = 1;
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = 2;
  lte_frame_parms->nb_antennas_rx     = 2;
  lte_frame_parms->first_dlsch_symbol = 4;
  lte_frame_parms->num_dlsch_symbols  = 6;
  lte_frame_parms->mode1_flag = (transmission_mode == 1) ? 1 : 0;

  init_frame_parms(lte_frame_parms);
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  phy_init_top(NB_ANTENNAS_TX);

  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;

  phy_init_lte_top(lte_frame_parms);

  randominit(0);
  set_taus_seed(0);

  channel_length = (int) 11+2*BW*Td;

  nsymb = (lte_frame_parms->Ncp == 0) ? 14 : 12;
  coded_bits_per_codeword = NB_RB * (12 * get_Qm(mcs)) * (lte_frame_parms->num_dlsch_symbols);
  printf("Rate = %f (mod %d)\n",(((double)dlsch_tbs25[get_I_TBS(mcs)][NB_RB-1])*3/4)/coded_bits_per_codeword,
	 get_Qm(mcs));

  sprintf(bler_fname,"bler_%d.m",mcs);
  bler_fd = fopen(bler_fname,"w");
  fprintf(bler_fd,"bler = [");
  // CSV file 
  sprintf(csv_fname,"data_out%d.m",mcs);
  csv_fd = fopen(csv_fname,"w");
  fprintf(csv_fd,"data_all=[");


#ifdef IFFT_FPGA
  txdata    = (int **)malloc16(2*sizeof(int*));
  txdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);

  bzero(txdata[0],FRAME_LENGTH_BYTES);
  bzero(txdata[1],FRAME_LENGTH_BYTES);

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

  for (i=0;i<2;i++) {
    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }

  // init eNB vars

  for (eNB_id=0;eNB_id<NB_CH_INST;eNB_id++) {
    memcpy(&(PHY_vars_eNb_g[eNB_id]->lte_frame_parms), lte_frame_parms, sizeof(LTE_DL_FRAME_PARMS));
    phy_init_lte_eNB(&PHY_vars_eNb_g[eNB_id]->lte_frame_parms,
		     &PHY_vars_eNb_g[eNB_id]->lte_eNB_common_vars,
		     PHY_vars_eNb_g[eNB_id]->lte_eNB_ulsch_vars,
		     0,
		     PHY_vars_eNb_g[eNB_id],
		     0,
		     0);
    
    PHY_vars_eNb_g[eNB_id]->dlsch_eNb[0] = (LTE_eNb_DLSCH_t**) malloc16(NUMBER_OF_UE_MAX*sizeof(LTE_eNb_DLSCH_t*));
    PHY_vars_eNb_g[eNB_id]->dlsch_eNb[1] = (LTE_eNb_DLSCH_t**) malloc16(NUMBER_OF_UE_MAX*sizeof(LTE_eNb_DLSCH_t*));
    PHY_vars_eNb_g[eNB_id]->ulsch_eNb = (LTE_eNb_ULSCH_t**) malloc16(NUMBER_OF_UE_MAX*sizeof(LTE_eNb_ULSCH_t*));

    for (i=0;i<NB_UE_INST;i++) {
      for (j=0;j<2;j++) {
	PHY_vars_eNb_g[eNB_id]->dlsch_eNb[i][j] = new_eNb_dlsch(1,8);
	if (!PHY_vars_eNb_g[eNB_id]->dlsch_eNb[i][j]) {
	  msg("Can't get eNb dlsch structures\n");
	  exit(-1);
	}
	else {
	  msg("dlsch_eNb[%d][%d] => %p\n",i,j,PHY_vars_eNb_g[eNB_id]->dlsch_eNb[i][j]);
	  PHY_vars_eNb_g[eNB_id]->dlsch_eNb[i][j]->rnti=0;
	}
      }
      PHY_vars_eNb_g[eNB_id]->ulsch_eNb[i] = new_eNb_ulsch(3);
      if (!PHY_vars_eNb_g[eNB_id]->ulsch_eNb[i]) {
	msg("Can't get eNb ulsch structures\n");
	exit(-1);
      }

    }

    PHY_vars_eNb_g[eNB_id]->dlsch_eNb_SI  = new_eNb_dlsch(1,1);
    PHY_vars_eNb_g[eNB_id]->dlsch_eNb_ra  = new_eNb_dlsch(1,1);

    PHY_vars_eNb_g[eNB_id]->rx_total_gain_eNB_dB=150;

  }

  for (UE_id=0; UE_id<NB_UE_INST;UE_id++){ 
    memcpy(&(PHY_vars_UE_g[UE_id]->lte_frame_parms), lte_frame_parms, sizeof(LTE_DL_FRAME_PARMS));
    
    phy_init_lte_ue(&PHY_vars_UE_g[UE_id]->lte_frame_parms,
		    &PHY_vars_UE_g[UE_id]->lte_ue_common_vars,
		    PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars,
		    PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars_SI,
		    PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars_ra,
		    PHY_vars_UE_g[UE_id]->lte_ue_pbch_vars,
		    PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars,
		    PHY_vars_UE_g[UE_id]);

    PHY_vars_UE_g[UE_id]->dlsch_ue[0] = (LTE_UE_DLSCH_t**) malloc16(NUMBER_OF_eNB_MAX*sizeof(LTE_UE_DLSCH_t*));
    PHY_vars_UE_g[UE_id]->dlsch_ue[1] = (LTE_UE_DLSCH_t**) malloc16(NUMBER_OF_eNB_MAX*sizeof(LTE_UE_DLSCH_t*));
    
    PHY_vars_UE_g[UE_id]->ulsch_ue = (LTE_UE_ULSCH_t**) malloc16(NUMBER_OF_eNB_MAX*sizeof(LTE_UE_ULSCH_t*));
    
    PHY_vars_UE_g[UE_id]->dlsch_ue_SI = (LTE_UE_DLSCH_t**) malloc16(NUMBER_OF_eNB_MAX*sizeof(LTE_UE_DLSCH_t*));
    PHY_vars_UE_g[UE_id]->dlsch_ue_ra = (LTE_UE_DLSCH_t**) malloc16(NUMBER_OF_eNB_MAX*sizeof(LTE_UE_DLSCH_t*));


    for (i=0;i<NB_CH_INST;i++) {
      for (j=0;j<2;j++) {
	PHY_vars_UE_g[UE_id]->dlsch_ue[i][j]  = new_ue_dlsch(1,8);
	if (!PHY_vars_UE_g[UE_id]->dlsch_ue[i][j]) {
	  msg("Can't get ue dlsch structures\n");
	  exit(-1);
	}
	else
	  msg("dlsch_ue[%d][%d] => %p\n",UE_id,i,PHY_vars_UE_g[UE_id]->dlsch_ue[i][j]);//navid
      }
      
      
      PHY_vars_UE_g[UE_id]->ulsch_ue[i]  = new_ue_ulsch(3);
      if (!PHY_vars_UE_g[UE_id]->ulsch_ue[i]) {
	msg("Can't get ue ulsch structures\n");
	exit(-1);
      }
      
      PHY_vars_UE_g[UE_id]->dlsch_ue_SI[i]  = new_ue_dlsch(1,1);
      PHY_vars_UE_g[UE_id]->dlsch_ue_ra[i]  = new_ue_dlsch(1,1);
    }
  }

  // do the srs init
  for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {

    PHY_vars_UE_g[UE_id]->SRS_parameters.Csrs = 2;
    PHY_vars_UE_g[UE_id]->SRS_parameters.Bsrs = 0;
    PHY_vars_UE_g[UE_id]->SRS_parameters.kTC = 0;
    PHY_vars_UE_g[UE_id]->SRS_parameters.n_RRC = 0;
    if (UE_id>=3) {
      printf("This SRS config will only work for 3 users");
      exit(-1);
    }
    PHY_vars_UE_g[UE_id]->SRS_parameters.Ssrs = UE_id+1;
    
    for (eNB_id=0;eNB_id<NB_CH_INST;eNB_id++) 
      PHY_vars_eNb_g[eNB_id]->eNB_UE_stats[UE_id].SRS_parameters = PHY_vars_UE_g[UE_id]->SRS_parameters;
  }

  // set up the user connections for the link level simulations
  for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {

    PHY_vars_UE_g[UE_id]->UE_mode[0] = PUSCH;
    PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[0]->crnti = 0xBEEF+UE_id;
    
    for (eNB_id=0;eNB_id<NB_CH_INST;eNB_id++) {
      PHY_vars_eNb_g[eNB_id]->eNB_UE_stats[UE_id].mode = PUSCH;
      PHY_vars_eNb_g[eNB_id]->eNB_UE_stats[UE_id].crnti = 0xBEEF+UE_id;
    }
  }


  //init_transport_channels(transmission_mode);
  CCCH_alloc_pdu.type               = 0;
  CCCH_alloc_pdu.vrb_type           = 0;
  CCCH_alloc_pdu.rballoc            = CCCH_RB_ALLOC;
  CCCH_alloc_pdu.ndi      = 1;
  CCCH_alloc_pdu.mcs      = 1;
  CCCH_alloc_pdu.harq_pid = 0;

  for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
    DLSCH_alloc_pdu2[UE_id].rah              = 0;
    DLSCH_alloc_pdu2[UE_id].rballoc          = DLSCH_RB_ALLOC;
    DLSCH_alloc_pdu2[UE_id].TPC              = 0;
    DLSCH_alloc_pdu2[UE_id].dai              = 0;
    DLSCH_alloc_pdu2[UE_id].harq_pid         = 0;
    DLSCH_alloc_pdu2[UE_id].tb_swap          = 0;
    DLSCH_alloc_pdu2[UE_id].mcs1             = mcs;  
    DLSCH_alloc_pdu2[UE_id].ndi1             = 1;
    DLSCH_alloc_pdu2[UE_id].rv1              = 0;
    // Forget second codeword
    DLSCH_alloc_pdu2[UE_id].tpmi             = 5 ;  // precoding
  }

  // initialized channel descriptors
  for (eNB_id=0;eNB_id<NB_CH_INST;eNB_id++) {
    for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {

      printf("[SIM] Initializing channel from eNB %d to UE %d\n",eNB_id,UE_id);
      eNB2UE[eNB_id][UE_id] = new_channel_desc(PHY_vars_eNb_g[eNB_id]->lte_frame_parms.nb_antennas_tx,
					       PHY_vars_UE_g[UE_id]->lte_frame_parms.nb_antennas_rx,
					       nb_taps,
					       channel_length,
					       amps,
					       NULL,
					       Td,
					       BW,
					       ricean_factor,
					       aoa,
					       forgetting_factor,
					       maxDoppler,
					       0,
					       0);

      UE2eNB[UE_id][eNB_id] = new_channel_desc(PHY_vars_UE_g[UE_id]->lte_frame_parms.nb_antennas_tx,
					       PHY_vars_eNb_g[eNB_id]->lte_frame_parms.nb_antennas_rx,
					       nb_taps,
					       channel_length,
					       amps,
					       NULL,
					       Td,
					       BW,
					       ricean_factor,
					       aoa,
					       forgetting_factor,
					       maxDoppler,
					       0,
					       0);

    }
  }


  // start TX

  if (1) {
    for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
      if (DLSCH_alloc_pdu2[UE_id].tpmi == 5) {
	PHY_vars_eNb_g[0]->eNB_UE_stats[UE_id].DL_pmi_single = (unsigned short)(taus()&0xffff);
	//PHY_vars_eNb_g[0]->dlsch_eNb[UE_id][0]->pmi_alloc = PHY_vars_eNb_g[0]->dlsch_eNb[UE_id][0]->pmi_alloc;
      }
      else {
	PHY_vars_eNb_g[0]->eNB_UE_stats[UE_id].DL_pmi_single = 0;
      }

      generate_eNb_dlsch_params_from_dci(0,
					 &DLSCH_alloc_pdu2[UE_id],
					 PHY_vars_eNb_g[0]->eNB_UE_stats[UE_id].crnti,
					 format2_2A_M10PRB,
					 PHY_vars_eNb_g[0]->dlsch_eNb[UE_id],
					 &PHY_vars_eNb_g[0]->lte_frame_parms,
					 SI_RNTI,
					 RA_RNTI,
					 P_RNTI,
					 PHY_vars_eNb_g[0]->eNB_UE_stats[UE_id].DL_pmi_single); //change this later
  
      // DCI
      memcpy(&dci_alloc[UE_id].dci_pdu[0],&DLSCH_alloc_pdu2[UE_id],sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
      dci_alloc[UE_id].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
      dci_alloc[UE_id].L          = 3;
      dci_alloc[UE_id].rnti       = PHY_vars_eNb_g[0]->eNB_UE_stats[UE_id].crnti;
      dci_alloc[UE_id].format     = format2_2A_M10PRB;
      num_ue_spec_dci++;

      // DLSCH
      input_buffer_length = PHY_vars_eNb_g[0]->dlsch_eNb[UE_id][0]->harq_processes[0]->TBS/8;
    
      printf("UE %d: TBS      %d, Input buffer size %d bytes\n",UE_id, 
	     PHY_vars_eNb_g[0]->dlsch_eNb[UE_id][0]->harq_processes[0]->TBS,
	     input_buffer_length);
    
      input_buffer = (unsigned char *)malloc(input_buffer_length+4);
    
      for (i=0;i<input_buffer_length;i++)
	input_buffer[i]= (unsigned char)(taus()&0xff);
    
      dlsch_encoding(input_buffer,
		     lte_frame_parms,
		     PHY_vars_eNb_g[0]->dlsch_eNb[UE_id][0]);
    
#ifdef OUTPUT_DEBUG
      for (s=0;s<PHY_vars_eNb_g[0]->dlsch_eNb[0]->harq_processes[0]->C;s++) {
	if (s<PHY_vars_eNb_g[0]->dlsch_eNb[0]->harq_processes[0]->Cminus)
	  Kr = PHY_vars_eNb_g[0]->dlsch_eNb[0]->harq_processes[0]->Kminus;
	else
	  Kr = PHY_vars_eNb_g[0]->dlsch_eNb[0]->harq_processes[0]->Kplus;
      
	Kr_bytes = Kr>>3;
      
	for (i=0;i<Kr_bytes;i++)
	  printf("%d : (%x)\n",i,PHY_vars_eNb_g[0]->dlsch_eNb[0]->harq_processes[0]->c[s][i]);
      }
    
#endif 

      re_allocated = dlsch_modulation(PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id],
				      1024,
				      0,
				      &PHY_vars_eNb_g[0]->lte_frame_parms,
				      PHY_vars_eNb_g[0]->dlsch_eNb[UE_id][0]);
    
    
      printf("RB count %d (%d,%d)\n",re_allocated,re_allocated/lte_frame_parms->num_dlsch_symbols/12,lte_frame_parms->num_dlsch_symbols);
    
    
      
      if (num_layers>1)
	re_allocated = dlsch_modulation(PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id],
					1024,
					0,
					&PHY_vars_eNb_g[0]->lte_frame_parms,
					PHY_vars_eNb_g[0]->dlsch_eNb[UE_id][1]);
    }
  }
  else {  // PBCH + DLSCH CNTL
    generate_eNb_dlsch_params_from_dci(0,
				       &CCCH_alloc_pdu,
				       SI_RNTI,
				       format1A,
				       &PHY_vars_eNb_g[0]->dlsch_eNb_SI,
				       &PHY_vars_eNb_g[0]->lte_frame_parms,
				       SI_RNTI,
				       RA_RNTI,
				       P_RNTI,
				       0);

    memcpy(&dci_alloc[0].dci_pdu[0],&CCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
    dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
    dci_alloc[0].L          = 3;
    dci_alloc[0].rnti       = SI_RNTI;
    dci_alloc[0].format     = format1A;
    num_common_dci++;
    
    input_buffer_length = PHY_vars_eNb_g[0]->dlsch_eNb_SI->harq_processes[0]->TBS/8;
    printf("Input buffer size %d bytes\n",input_buffer_length);
    
    input_buffer = (unsigned char *)malloc(input_buffer_length+4);
    
    for (i=0;i<input_buffer_length;i++)
      input_buffer[i]= (unsigned char)(taus()&0xff);
    
    dlsch_encoding(input_buffer,
		   &PHY_vars_eNb_g[0]->lte_frame_parms,
		   PHY_vars_eNb_g[0]->dlsch_eNb_SI);
    
#ifdef OUTPUT_DEBUG
    for (s=0;s<PHY_vars_eNb_g[0]->dlsch_eNb_SI->harq_processes[0]->C;s++) {
      if (s<PHY_vars_eNb_g[0]->dlsch_eNb_SI->harq_processes[0]->Cminus)
	Kr = PHY_vars_eNb_g[0]->dlsch_eNb_SI->harq_processes[0]->Kminus;
      else
	Kr = PHY_vars_eNb_g[0]->dlsch_eNb_SI->harq_processes[0]->Kplus;
      
      Kr_bytes = Kr>>3;
      
      for (i=0;i<Kr_bytes;i++)
	printf("%d : (%x)\n",i,PHY_vars_eNb_g[0]->dlsch_eNb_SI->harq_processes[0]->c[s][i]);
    }    
    
#endif 
    
    re_allocated = dlsch_modulation(PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id],
				    1024,
				    0,
				    &PHY_vars_eNb_g[0]->lte_frame_parms,
				    PHY_vars_eNb_g[0]->dlsch_eNb_SI);
    
    
    printf("RB count %d (%d,%d)\n",re_allocated,re_allocated/lte_frame_parms->num_dlsch_symbols/12,lte_frame_parms->num_dlsch_symbols);
    
    if ((re_allocated/(lte_frame_parms->num_dlsch_symbols*12)) != NB_RB)
      printf("Bad RB count %d (%d,%d)\n",re_allocated,re_allocated/lte_frame_parms->num_dlsch_symbols/12,lte_frame_parms->num_dlsch_symbols);
  
  
    
    generate_pss(PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id],
		 1024,
		 &PHY_vars_eNb_g[0]->lte_frame_parms,
		 eNb_id,
		 6-PHY_vars_eNb_g[0]->lte_frame_parms.Ncp,
		 0);
    
    for (i=0;i<6;i++)
      pbch_pdu[i] = 0;
    *((unsigned int*) pbch_pdu) = mac_xface->frame;
    ((unsigned char*) pbch_pdu)[4] = transmission_mode;
    
    generate_pbch(PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id],
		  1024,
		  &PHY_vars_eNb_g[0]->lte_frame_parms,
		  pbch_pdu);

  }

  generate_dci_top(num_ue_spec_dci,
		   num_common_dci,
		   dci_alloc,
		   0,
		   1024,
		   &PHY_vars_eNb_g[0]->lte_frame_parms,
		   PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id],
		   0);

  generate_pilots(PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id],
		  1024,
		  &PHY_vars_eNb_g[0]->lte_frame_parms,
		  eNb_id,
		  LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
  


#ifdef IFFT_FPGA

#ifdef OUTPUT_DEBUG  
  write_output("txsigF0.m","txsF0", PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[0][0],300*120,1,4);
  write_output("txsigF1.m","txsF1", PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[0][1],300*120,1,4);
#endif

  // do talbe lookup and write results to txdataF2
  for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
    ind = 0;
    for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX;i++) 
      if (((i%512)>=1) && ((i%512)<=150))
	txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id][aa][ind++]];
      else if ((i%512)>=362)
	txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id][aa][ind++]];
      else 
	txdataF2[aa][i] = 0;
    //    printf("ind=%d\n",ind);
  }

#ifdef OUTPUT_DEBUG  
  write_output("txsigF20.m","txsF20", txdataF2[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
#endif

  tx_lev = 0;
  for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
    PHY_ofdm_mod(txdataF2[aa],        // input
		 txdata[aa],         // output
		 lte_frame_parms->log2_symbol_size,                // log2_fft_size
		 NUMBER_OF_SYMBOLS_PER_FRAME,                 // number of symbols
		 lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		 lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 lte_frame_parms->rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);

    tx_lev += signal_energy(&txdata[aa][4*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
			    OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
  }
    
#else //IFFT_FPGA

  txdata = PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdata[eNb_id];

#ifdef OUTPUT_DEBUG  
  write_output("txsigF0.m","txsF0", PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id][0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX/5,1,1);
  //write_output("txsigF1.m","txsF1", PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id][1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX/5,1,1);
#endif
  
  tx_lev = 0;
  for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
    PHY_ofdm_mod(PHY_vars_eNb_g[0]->lte_eNB_common_vars.txdataF[eNb_id][aa],        // input
		 txdata[aa],         // output
		 PHY_vars_eNb_g[0]->lte_frame_parms.log2_symbol_size,                // log2_fft_size
		 NUMBER_OF_SYMBOLS_PER_FRAME,                 // number of symbols
		 PHY_vars_eNb_g[0]->lte_frame_parms.nb_prefix_samples,               // number of prefix samples
		 PHY_vars_eNb_g[0]->lte_frame_parms.twiddle_ifft,  // IFFT twiddle factors
		 PHY_vars_eNb_g[0]->lte_frame_parms.rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);

    tx_lev += signal_energy(&txdata[aa][OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*4],
			    OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);

  }  
#endif //IFFT_FPGA


  printf("tx_lev = %d\n",tx_lev);
  tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
  printf("tx_lev_dB = %d\n",tx_lev_dB);


#ifdef OUTPUT_DEBUG  
  write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
#endif


  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++) {
    for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
      s_re[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)]);
      s_im[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)+1]);
    }
  }



  for (ch_realization=0;ch_realization<20;ch_realization++){
      
    printf("**********************Channel Realization Index = %d **************************\n", ch_realization);

    for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {

      PHY_vars_UE_g[UE_id]->UE_mode[0] = PUSCH;
      PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[0]->crnti = 0xBEEF+UE_id;
 
#ifdef AWGN // copy s_re and s_im to r_re and r_im
      for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
	  r_re[aa][i] = s_re[aa][i];
	  r_im[aa][i] = s_im[aa][i];
	}
      }
#else
      multipath_channel(eNB2UE[0][UE_id],s_re,s_im,r_re,r_im,
			FRAME_LENGTH_COMPLEX_SAMPLES,
			0);
#endif 

#ifdef OUTPUT_DEBUG
      write_output("channel0.m","chan0",eNB2UE[0][UE_id]->ch[0],channel_length,1,8);
#endif
     
      for (SNR=snr0;SNR<snr1;SNR+=.2) {
   
	sigma2_dB = 10*log10(tx_lev) + 10*log10(lte_frame_parms->ofdm_symbol_size/(NB_RB*12)) - SNR;

	printf("**********************SNR = %f dB (tx_lev %f, sigma2_dB %f)**************************\n",
	       SNR,
	       (double)10*log10(tx_lev)+10*log10(lte_frame_parms->ofdm_symbol_size/(NB_RB*12)),
	       sigma2_dB);

	errs=0;
	dci_errors=0;


	for (trials = 0;trials<n_frames;trials++) {
	  mac_xface->frame = trials;

	  //AWGN
	  sigma2 = pow(10,sigma2_dB/10);


	  for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES; i++) {
	    if ((i/640)%3!=0){
	      for (aa=0;aa<lte_frame_parms->nb_antennas_rx;aa++) {
		((short*) PHY_vars_UE_g[UE_id]->lte_ue_common_vars.rxdata[aa])[2*i] = (short) (r_re[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
		((short*) PHY_vars_UE_g[UE_id]->lte_ue_common_vars.rxdata[aa])[2*i+1] = (short) (r_im[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	      }
	    }
	    else{
	      for (aa=0;aa<lte_frame_parms->nb_antennas_rx;aa++) {
		((short*) PHY_vars_UE_g[UE_id]->lte_ue_common_vars.rxdata[aa])[2*i] = (short) (r_re[aa][i]);
		((short*) PHY_vars_UE_g[UE_id]->lte_ue_common_vars.rxdata[aa])[2*i+1] = (short) (r_im[aa][i]);
	      }
	    }
	  }
    
	  //    lte_sync_time_init(lte_frame_parms,lte_ue_common_vars);
	  //    lte_sync_time(lte_ue_common_vars->rxdata, lte_frame_parms);
	  //    lte_sync_time_free();

	  /*
	  // optional: read rx_frame from file
	  if ((rx_frame_file = fopen("rx_frame.dat","r")) == NULL)
	  {
	  printf("Cannot open rx_frame.m data file\n");
	  exit(0);
	  }
  
	  result = fread((void *)PHY_vars->rx_vars[0].RX_DMA_BUFFER,4,FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
	  printf("Read %d bytes\n",result);
	  result = fread((void *)PHY_vars->rx_vars[1].RX_DMA_BUFFER,4,FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
	  printf("Read %d bytes\n",result);
	
	  fclose(rx_frame_file);
	  */

	  rx_lev_data_sym = signal_energy(&PHY_vars_UE_g[UE_id]->lte_ue_common_vars.rxdata[0][OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*4],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
	  rx_lev_null_sym = signal_energy(&PHY_vars_UE_g[UE_id]->lte_ue_common_vars.rxdata[0][13*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
	  rx_snr_dB = dB_fixed(rx_lev_data_sym - rx_lev_null_sym) - dB_fixed(rx_lev_null_sym);
#ifdef OUTPUT_DEBUG
	  printf("RX level in data symbol (lin) %d\n",rx_lev_data_sym);
	  printf("RX level in null symbol (lin) %d\n",rx_lev_null_sym);
	  printf("RX SNR (dB) %d\n",rx_snr_dB);
#endif

	  phy_procedures_UE_RX(0,PHY_vars_UE_g[UE_id],eNb_id);
	  phy_procedures_UE_RX(1,PHY_vars_UE_g[UE_id],eNb_id);
	  phy_procedures_UE_RX(2,PHY_vars_UE_g[UE_id],eNb_id);

	  /*	
	  // Inner receiver scheduling for 3 slots
	  for (Ns=0;Ns<3;Ns++) {
	  for (l=0;l<6;l++) {
	  //	    printf("Ns %d, l %d\n",Ns,l);
	  // channel estimation happens in slot_fep
	  slot_fep(&PHY_vars_UE_g[UE_id]->lte_frame_parms,
	  &PHY_vars_UE_g[UE_id]->lte_ue_common_vars,
	  l,
	  Ns%20,
	  0,
	  0);

	  if (l%6==0) {
	  // fill &(PHY_vars_UE_g[UE_id]->lte_ue_common_vars.dl_ch_estimates[eNb_id][aa][512*3..512*6-1] with Re=ONE_Q15 Im=0
	  }
	  if (l%6==3) {
	  // fill &(PHY_vars_UE_g[UE_id]->lte_ue_common_vars.dl_ch_estimates[eNb_id][aa][0..512*3] with Re=ONE_Q15 Im=0
	  }
	    
	  lte_ue_measurements(PHY_vars_UE_g[UE_id],
	  &PHY_vars_UE_g[UE_id]->lte_frame_parms,
	  0,
	  1,
	  1);
	  //printf("rx_avg_power_dB %d\n",PHY_vars_UE_g[UE_id]->PHY_measurements.wideband_cqi_tot[0]);
	  //printf("n0_power_dB %d\n",PHY_vars_UE_g[UE_id]->PHY_measurements.n0_power_dB[0]);
	
	  if ((Ns==0) && (l==3)) {// process symbols 0,1,2


	  #ifndef NO_DCI  
	  rx_pdcch(&PHY_vars_UE_g[UE_id]->lte_ue_common_vars,
	  PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars,
	  &PHY_vars_UE_g[UE_id]->lte_frame_parms,
	  eNb_id,
	  2,
	  (PHY_vars_UE_g[UE_id]->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI,
	  0);

	  dci_cnt = dci_decoding_procedure(PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars,dci_alloc_rx,eNb_id,&PHY_vars_UE_g[UE_id]->lte_frame_parms,SI_RNTI,RA_RNTI);
	  //              printf("dci_cnt %d\n",dci_cnt);
	  //              write_output("dlsch00_ch0.m","dl00_ch0",&(lte_ue_common_vars->dl_ch_estimates[eNb_id][0][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	  //              exit(-1);


	  for (i=0;i<dci_cnt;i++) {
	  if ((dci_alloc_rx[i].rnti == 0x1234) && (dci_alloc_rx[i].format == format2_2A_M10PRB) &&
	  (generate_ue_dlsch_params_from_dci(0,
	  (DCI2_5MHz_2A_M10PRB_TDD_t *)&dci_alloc_rx[i].dci_pdu,
	  0x1234,
	  format2_2A_M10PRB,
	  PHY_vars_UE_g[UE_id]->dlsch_ue,
	  &PHY_vars_UE_g[UE_id]->lte_frame_parms,
	  SI_RNTI,
	  RA_RNTI,
	  P_RNTI)==0)) {
	  dlsch_active = 1;
	  }
	  else {
	  dlsch_active = 0;
	  dci_errors++;
	  errs++;
	  }
	  }

	  #else
	  generate_ue_dlsch_params_from_dci(0,
	  &DLSCH_alloc_pdu2,
	  0x1234,
	  format2_2A_M10PRB,
	  PHY_vars_UE_g[UE_id]->dlsch_ue[0],
	  &PHY_vars_UE_g[UE_id]->lte_frame_parms,
	  SI_RNTI,
	  RA_RNTI,
	  P_RNTI);
	  dlsch_active = 1;
	  #endif

	  }

	  if (dlsch_active == 1) {
	  if ((Ns==1) && (l==0)) // process symbols 3,4,5
	  for (m=4;m<6;m++)
	  rx_dlsch(&PHY_vars_UE_g[UE_id]->lte_ue_common_vars,
	  PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars,
	  &PHY_vars_UE_g[UE_id]->lte_frame_parms,
	  eNb_id,
	  eNb_id_i,
	  PHY_vars_UE_g[UE_id]->dlsch_ue[0],
	  m,
	  dual_stream_UE,
	  &PHY_vars_UE_g[UE_id]->PHY_measurements,
	  0);
	      
	  if ((Ns==1) && (l==3)) {// process symbols 6,7,8
	  for (m=7;m<9;m++)
	  rx_dlsch(&PHY_vars_UE_g[UE_id]->lte_ue_common_vars,
	  PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars,
	  &PHY_vars_UE_g[UE_id]->lte_frame_parms,
	  eNb_id,
	  eNb_id_i,
	  PHY_vars_UE_g[UE_id]->dlsch_ue,
	  m,
	  dual_stream_UE,
	  &PHY_vars_UE_g[UE_id]->PHY_measurements,
	  0);
	  }
	      
	  if ((Ns==2) && (l==0))  // process symbols 10,11, do deinterleaving for TTI
	  for (m=10;m<12;m++)
	  rx_dlsch(&PHY_vars_UE_g[UE_id]->lte_ue_common_vars,
	  PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars,
	  &PHY_vars_UE_g[UE_id]->lte_frame_parms,
	  eNb_id,
	  eNb_id_i,
	  PHY_vars_UE_g[UE_id]->dlsch_ue[0],
	  m,
	  dual_stream_UE,
	  &PHY_vars_UE_g[UE_id]->PHY_measurements,
	  0);
	      
	  }
	  }
	  }
	  */
      
	  if(trials==0){
	    fprintf(csv_fd,"%f,%d,%d,%d",SNR, rx_lev_data_sym , rx_lev_null_sym, rx_snr_dB);
	    data= &(PHY_vars_UE_g[UE_id]->lte_ue_common_vars.dl_ch_estimates[eNb_id][0][0]);

	    for (ii=10;ii<((1*(PHY_vars_UE_g[UE_id]->lte_frame_parms.ofdm_symbol_size))<<1)-414;ii+=2) {
	      fprintf(csv_fd,",%d+1i*(%d)",((short *)data)[ii], ((short *)data)[ii+1]);
	    }
	    fprintf(csv_fd,",");
	  }

	  /*
	    if (dlsch_active == 1) {
	    #ifdef OUTPUT_DEBUG      
	    write_output("rxsig0.m","rxs0", PHY_vars_UE_g[UE_id]->lte_ue_common_vars.rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
	    write_output("dlsch00_ch0.m","dl00_ch0",&(PHY_vars_UE_g[UE_id]->lte_ue_common_vars.dl_ch_estimates[eNb_id][0][0]),(6*(PHY_vars_UE_g[UE_id]->lte_frame_parms.ofdm_symbol_size)),1,1);
	    write_output("dlsch01_ch0.m","dl01_ch0",&(PHY_vars_UE_g[UE_id]->lte_ue_common_vars.dl_ch_estimates[eNb_id][1][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	    write_output("dlsch10_ch0.m","dl10_ch0",&(PHY_vars_UE_g[UE_id]->lte_ue_common_vars.dl_ch_estimates[eNb_id][2][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	    write_output("dlsch11_ch0.m","dl11_ch0",&(PHY_vars_UE_g[UE_id]->lte_ue_common_vars.dl_ch_estimates[eNb_id][3][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	    write_output("rxsigF0.m","rxsF0", PHY_vars_UE_g[UE_id]->lte_ue_common_vars.rxdataF[0],2*12*PHY_vars_UE_g[UE_id]->lte_frame_parms.ofdm_symbol_size,2,1);
	    write_output("rxsigF0_ext.m","rxsF0_ext", PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars[eNb_id]->rxdataF_ext[0],2*12*PHY_vars_UE_g[UE_id]->lte_frame_parms.ofdm_symbol_size,1,1);
	    write_output("dlsch00_ch0_ext.m","dl00_ch0_ext",PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[0],300*12,1,1);
	    write_output("pdcchF0_ext.m","pdcchF_ext", PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[eNb_id]->rxdataF_ext[0],2*3*PHY_vars_UE_g[UE_id]->lte_frame_parms.ofdm_symbol_size,1,1);
	    write_output("pdcch00_ch0_ext.m","pdcch00_ch0_ext",PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[eNb_id]->dl_ch_estimates_ext[0],300*3,1,1);
	    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[1],300*12,1,1);
	    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[2],300*12,1,1);
	    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[3],300*12,1,1);
	    write_output("dlsch_rho.m","dl_rho",lte_ue_dlsch_vars[eNb_id]->rho[0],300*12,1,1);
	    write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0",PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars[eNb_id]->rxdataF_comp[0],300*12,1,1);
	    write_output("pdcch_rxF_comp0.m","pdcch0_rxF_comp0",PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[eNb_id]->rxdataF_comp[0],4*300,1,1);
	    write_output("dlsch_rxF_llr.m","dlsch_llr",PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars[eNb_id]->llr[0],coded_bits_per_codeword,1,0);
	    write_output("pdcch_rxF_llr.m","pdcch_llr",PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[eNb_id]->llr,2400,1,4);
	  
	    write_output("dlsch_mag1.m","dlschmag1",PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars[eNb_id]->dl_ch_mag,300*12,1,1);
	    write_output("dlsch_mag2.m","dlschmag2",PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars[eNb_id]->dl_ch_magb,300*12,1,1);
	    #endif //OUTPUT_DEBUG
	  
	    //	printf("Calling decoding (Ndi %d, harq_pid %d)\n",
	    //       dlsch_ue[0]->harq_processes[0]->Ndi,
	    //       dlsch_ue[0]->current_harq_pid);
	  
		 
	    ret = dlsch_decoding(PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars[eNb_id]->llr[0],		 
	    &PHY_vars_UE_g[UE_id]->lte_frame_parms,
	    PHY_vars_UE_g[UE_id]->dlsch_ue[0][0],
	    0);
	  
	    if (ret <= MAX_TURBO_ITERATIONS) {
	    #ifdef OUTPUT_DEBUG  
	    printf("No DLSCH errors found\n");
	    #endif
	    }	
	    else {
	    errs++;
	    #ifdef OUTPUT_DEBUG  
	    printf("DLSCH in error\n");
	    #endif
	    }
	
	  
	    #ifdef OUTPUT_DEBUG  
	    for (s=0;s<PHY_vars_UE_g[UE_id]->dlsch_ue[0]->harq_processes[0]->C;s++) {
	    if (s<PHY_vars_UE_g[UE_id]->dlsch_ue[0]->harq_processes[0]->Cminus)
	    Kr = PHY_vars_UE_g[UE_id]->dlsch_ue[0]->harq_processes[0]->Kminus;
	    else
	    Kr = PHY_vars_UE_g[UE_id]->dlsch_ue[0]->harq_processes[0]->Kplus;
	    
	    Kr_bytes = Kr>>3;
	    
	    printf("Decoded_output (Segment %d):\n",s);
	    for (i=0;i<Kr_bytes;i++)
	    printf("%d : %x (%x)\n",i,PHY_vars_UE_g[UE_id]->dlsch_ue[0]->harq_processes[0]->c[s][i],PHY_vars_UE_g[UE_id]->dlsch_ue[0]->harq_processes[0]->c[s][i]^PHY_vars_eNb_g[0]->dlsch_eNb[0]->harq_processes[0]->c[s][i]);
	    }
	    exit(-1);
	    #endif
	    if (errs==1000)
	    break;
	    }
	  */
	}   //trials
	printf("Errors %d/%d, Pe = %e, dci_errors %d/%d, Pe = %e\n",errs,1+trials,(double)errs/(trials+1),dci_errors,1+trials,(double)dci_errors/(trials+1));
	blerr= (double)errs/(trials+1);
	fprintf(bler_fd,"%f,%e;\n",SNR,blerr);
	fprintf(csv_fd,"%e;\n",blerr);

	if(blerr<.001)
	  break;
      } //SNR
    } //UE_id
  } // Channel Realizations
  fprintf(csv_fd,"];");
  fclose(bler_fd);
  fclose(csv_fd);
  
  /*
    printf("Freeing dlsch structures\n");
    for (i=0;i<2;i++) {
    printf("eNb %d\n",i);
    free_eNb_dlsch(PHY_vars_eNb_g[0]->dlsch_eNb[i]);
    printf("UE %d\n",i);
    free_ue_dlsch(PHY_vars_UE->dlsch_ue[i]);
    }
  */


#ifdef IFFT_FPGA
  printf("Freeing transmit signals\n");
  free(txdataF2[0]);
  free(txdataF2[1]);
  free(txdataF2);
  free(txdata[0]);
  free(txdata[1]);
  free(txdata);
#endif

  printf("Freeing channel I/O\n");
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
  // }
  //  lte_sync_time_free();

  return(0);
}
   


