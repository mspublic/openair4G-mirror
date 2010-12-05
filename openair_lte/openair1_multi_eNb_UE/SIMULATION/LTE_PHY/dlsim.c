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

//#define AWGN
//#define NO_DCI

#define BW 7.68

//#define ABSTRACTION

#define RBmask0 0x00fc00fc
#define RBmask1 0x0
#define RBmask2 0x0
#define RBmask3 0x0

unsigned char dlsch_cqi;

PHY_VARS_eNB *PHY_vars_eNb;
PHY_VARS_UE *PHY_vars_UE;


void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,u8 extended_prefix_flag) {

  LTE_DL_FRAME_PARMS *lte_frame_parms;

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
  lte_frame_parms->Ncp                = extended_prefix_flag;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  lte_frame_parms->first_dlsch_symbol = 4;
  lte_frame_parms->num_dlsch_symbols  = (lte_frame_parms->Ncp==0) ? 8: 6;
  lte_frame_parms->Ng_times6          = 1;
  //  lte_frame_parms->Csrs = 2;
  //  lte_frame_parms->Bsrs = 0;
  //  lte_frame_parms->kTC = 0;
  //  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->mode1_flag = (transmission_mode == 1)? 1 : 0;

  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(N_tx,lte_frame_parms); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  PHY_vars_UE->lte_frame_parms = *lte_frame_parms;
  PHY_vars_eNb->lte_frame_parms = *lte_frame_parms;

  /*  
  lte_gold(lte_frame_parms);
  generate_ul_ref_sigs();
  generate_ul_ref_sigs_rx();
  generate_64qam_table();
  generate_16qam_table();
  generate_RIV_tables();

  generate_pcfich_reg_mapping(lte_frame_parms);
  generate_phich_reg_mapping(lte_frame_parms);
  */

  phy_init_lte_top(lte_frame_parms);

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
		   PHY_vars_eNb,
		   0,
		   0);

  
  printf("Done lte_param_init\n");


}

DCI0_5MHz_TDD0_t          UL_alloc_pdu;
DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;

#define UL_RB_ALLOC 0x1ff;
#define CCCH_RB_ALLOC computeRIV(PHY_vars_eNb->lte_frame_parms.N_RB_UL,0,2)
#define DLSCH_RB_ALLOC 0x1fff//0x1fbf // igore DC component,RB13
//#define DLSCH_RB_ALLOC 0x1f0f // igore DC component,RB13



int main(int argc, char **argv) {

  char c;
  int i,aa,aarx;

  int s,Kr,Kr_bytes;

  double sigma2, sigma2_dB=10,SNR,snr0=-2.0,snr1,rate;
  double snr_step=1, snr_int=20;
  //int **txdataF, **txdata;
  int **txdata;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  LTE_DL_FRAME_PARMS *frame_parms;
  double **s_re,**s_im,**r_re,**r_im;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03;
  double ricean_factor=0.0000005;
  double forgetting_factor=0;
  double Td=0.8;
  double iqim=0.0;
  u8 channel_length,nb_taps=8;
  u8 extended_prefix_flag=0,transmission_mode=1,n_tx=1,n_rx=1;

  int eNb_id = 0, eNb_id_i = 1;
  unsigned char mcs,dual_stream_UE = 0,awgn_flag=0,round,dci_flag=0;
  unsigned short NB_RB=conv_nprb(0,DLSCH_RB_ALLOC);
  unsigned char Ns,l,m;

  SCM_t channel_model=custom;
  //  unsigned char *input_data,*decoded_output;

  unsigned char *input_buffer;
  unsigned short input_buffer_length;
  unsigned int ret;
  unsigned int coded_bits_per_codeword,nsymb,dci_cnt;
 
  unsigned int tx_lev,tx_lev_dB,trials,errs[4]={0,0,0,0},round_trials[4]={0,0,0,0},dci_errors=0,dlsch_active=0,num_layers;
  int re_allocated;
  FILE *bler_fd;
  char bler_fname[20];

  FILE *input_trch_fd;
  unsigned char input_trch_file=0;
  char input_trch_val[16];
  double pilot_sinr, abs_channel;
  #ifdef ABSTRACTION
   FILE *csv_fd;
   char csv_fname[20];
   int ch_realization;

   void *data;
    int ii;
    // int bler;
  double blerr;
  #endif

  //  unsigned char pbch_pdu[6];

  DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];
  int num_common_dci=0,num_ue_spec_dci=0;

  //  FILE *rx_frame_file;

  int n_frames;

  channel_desc_t *eNB2UE;
  double snr;
  u8 num_pdcch_symbols=3;
  u8 pilot1,pilot2,pilot3;
  u8 rx_sample_offset = 0;
  //char stats_buffer[4096];
  //int len;
  u8 num_rounds = 4,fix_rounds=0;
  u8 subframe=6;
  int u;
  channel_length = (int) 11+2*BW*Td;

  // default parameters
  mcs = 0;
  n_frames = 1000;
  snr0 = 0;
  num_layers = 1;

  while ((c = getopt (argc, argv, "hadpm:n:o:s:f:t:c:g:r:x:y:z:I:R:S:")) != -1) {
    switch (c)
      {
      case 'a':
	awgn_flag = 1;
	break;
      case 'd':
	dci_flag = 1;
	break;
      case 'm':
	mcs = atoi(optarg);
	break;
      case 'n':
	n_frames = atoi(optarg);
	break;
      case 'o':
	rx_sample_offset = atoi(optarg);
	break;
      case 'r':
	ricean_factor = pow(10,-.1*atof(optarg));
	if (ricean_factor>1) {
	  printf("Ricean factor must be between 0 and 1\n");
	  exit(-1);
	}
	break;
      case 's':
	snr0 = atoi(optarg);
	break;
      case 't':
	Td= atof(optarg);
	break;
      case 'f':
	snr_step= atof(optarg);
	break;
      case 'p':
	extended_prefix_flag=1;
	break;
      case 'c':
	num_pdcch_symbols=atoi(optarg);
	break;
      case 'g':
	switch((char)*optarg) {
	case 'A': 
	  channel_model=SCM_A;
	  break;
	case 'B': 
	  channel_model=SCM_B;
	  break;
	case 'C': 
	  channel_model=SCM_C;
	  break;
	case 'D': 
	  channel_model=SCM_D;
	  break;
	default:
	  msg("Unsupported channel model!\n");
	  exit(-1);
	}
	break;
      case 'x':
	transmission_mode=atoi(optarg);
	if ((transmission_mode!=1) &&
	    (transmission_mode!=2) &&
	    (transmission_mode!=6)) {
	  msg("Unsupported transmission mode %d\n",transmission_mode);
	  exit(-1);
	}
	break;
      case 'y':
	n_tx=atoi(optarg);
	if ((n_tx==0) || (n_tx>2)) {
	  msg("Unsupported number of tx antennas %d\n",n_tx);
	  exit(-1);
	}
	break;
      case 'z':
	n_rx=atoi(optarg);
	if ((n_rx==0) || (n_rx>2)) {
	  msg("Unsupported number of rx antennas %d\n",n_rx);
	  exit(-1);
	}
	break;
      case 'I':
	input_trch_fd = fopen(optarg,"r");
	input_trch_file=1;
	break;
      case 'R':
	num_rounds=atoi(optarg);
	fix_rounds=1;
	break;
      case 'S':
	subframe=atoi(optarg);
	break;
      case 'h':
      default:
	printf("%s -h(elp) -a(wgn on) -d(ci decoding on) -p(extended prefix on) -m mcs -n n_frames -s snr0 -t Delayspread -x transmission mode (1,2,6) -y TXant -z RXant\n",argv[0]);
	printf("-h This message\n");
	printf("-a Use AWGN channel and not multipath\n");
	printf("-c Number of PDCCH symbols\n");
	printf("-m MCS\n");
	printf("-d Transmit the DCI and compute its error statistics and the overall throughput\n");
	printf("-p Use extended prefix mode\n");
	printf("-n Number of frames to simulate\n");
	printf("-o Sample offset for receiver\n");
	printf("-s Starting SNR, runs from SNR to SNR+%.1fdB in steps of %.1fdB. If n_frames is 1 then just SNR is simulated and MATLAB/OCTAVE output is generated\n", snr_int, snr_step);
	printf("-f step size of SNR, default value is 1.\n");
	printf("-t Delay spread for multipath channel\n");
	printf("-r Ricean factor (dB, 0 dB = Rayleigh, 100 dB = almost AWGN)\n");
	printf("-g [A,B,C,D] Use 3GPP 25.814 SCM (ignores delay spread and Ricean factor)\n");
	printf("-x Transmission mode (1,2,6 for the moment)\n");
	printf("-y Number of TX antennas used in eNB\n");
	printf("-z Number of RX antennas used in UE\n");
	printf("-R Number of HARQ rounds (fixed)\n");
	exit(1);
	break;
      }
  }

  lte_param_init(n_tx,n_rx,transmission_mode,extended_prefix_flag);  
  printf("Setting mcs = %d\n",mcs);
  printf("NPRB = %d\n",NB_RB);
  printf("n_frames = %d\n",n_frames);
  printf("Transmission mode %d with %dx%d antenna configuration, Extended Prefix %d\n",transmission_mode,n_tx,n_rx,extended_prefix_flag);

  snr1 = snr0+snr_int;
  printf("SNR0 %f, SNR1 %f\n",snr0,snr1);

  /*
  txdataF    = (int **)malloc16(2*sizeof(int*));
  txdataF[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdataF[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  
  txdata    = (int **)malloc16(2*sizeof(int*));
  txdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  */

  frame_parms = &PHY_vars_eNb->lte_frame_parms;

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
#else
  txdata = PHY_vars_eNb->lte_eNB_common_vars.txdata[eNb_id];
#endif

  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
  //  r_re0 = malloc(2*sizeof(double*));
  //  r_im0 = malloc(2*sizeof(double*));

  nsymb = (PHY_vars_eNb->lte_frame_parms.Ncp == 0) ? 14 : 12;

  sprintf(bler_fname,"bler_%d.csv",mcs);
  bler_fd = fopen(bler_fname,"w");
  fprintf(bler_fd,"SNR; MCS; TBS; rate; err0; trials0; err1; trials1; err2; trials2; err3; trials3; dci_err\n");

#ifdef ABSTRACTION
   // CSV file 
  sprintf(csv_fname,"data_out%d.m",mcs);
  csv_fd = fopen(csv_fname,"w");
  fprintf(csv_fd,"data_all%d=[",mcs);
#endif

  for (i=0;i<2;i++) {
    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    //    r_re0[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    //    bzero(r_re0[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    //    r_im0[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    //    bzero(r_im0[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }


   PHY_vars_UE->lte_ue_pdcch_vars[0]->crnti = 0x1234;

  // Fill in UL_alloc
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
  CCCH_alloc_pdu.mcs      = 1;
  CCCH_alloc_pdu.harq_pid = 0;

  DLSCH_alloc_pdu2.rah              = 0;
  DLSCH_alloc_pdu2.rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2.TPC              = 0;
  DLSCH_alloc_pdu2.dai              = 0;
  DLSCH_alloc_pdu2.harq_pid         = 0;
  DLSCH_alloc_pdu2.tb_swap          = 0;
  DLSCH_alloc_pdu2.mcs1             = mcs;  
  DLSCH_alloc_pdu2.ndi1             = 1;
  DLSCH_alloc_pdu2.rv1              = 0;
  // Forget second codeword
  DLSCH_alloc_pdu2.tpmi             = (transmission_mode==6 ? 4 : 0) ;  // precoding

  // Create transport channel structures for SI pdus
  PHY_vars_eNb->dlsch_eNb_SI   = new_eNb_dlsch(1,1,0);
  PHY_vars_UE->dlsch_ue_SI[0]  = new_ue_dlsch(1,1,0);
  PHY_vars_eNb->dlsch_eNb_SI->rnti  = SI_RNTI;
  PHY_vars_UE->dlsch_ue_SI[0]->rnti = SI_RNTI;

  if (channel_model==custom) {
    msg("[SIM] Using custom channel model\n");
    eNB2UE = new_channel_desc(PHY_vars_eNb->lte_frame_parms.nb_antennas_tx,
			      PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
			      nb_taps,
			      channel_length,
			      amps,
			      NULL,
			      NULL,
			      Td,
			      BW,
			      ricean_factor,
			      aoa,
			      forgetting_factor,
			      0,
			      rx_sample_offset,
			      0);
  }
  else {
    msg("[SIM] Using SCM\n");
    eNB2UE = new_channel_desc_scm(PHY_vars_eNb->lte_frame_parms.nb_antennas_tx,
				  PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				  SCM_C,
				  BW,
				  forgetting_factor,
				  rx_sample_offset,
				  0);
  }

  if (eNB2UE==NULL) {
    msg("Problem generating channel model. Exiting.\n");
    exit(-1);
  }
  
  // Create transport channel structures for 2 transport blocks (MIMO)
  for (i=0;i<2;i++) {
    PHY_vars_eNb->dlsch_eNb[0][i] = new_eNb_dlsch(1,8,0);
    PHY_vars_UE->dlsch_ue[0][i]  = new_ue_dlsch(1,8,0);
  
    if (!PHY_vars_eNb->dlsch_eNb[0][i]) {
      printf("Can't get eNb dlsch structures\n");
      exit(-1);
    }
    
    if (!PHY_vars_UE->dlsch_ue[0][i]) {
      printf("Can't get ue dlsch structures\n");
      exit(-1);
    }
    
    PHY_vars_eNb->dlsch_eNb[0][i]->rnti = 0x1234;
    PHY_vars_UE->dlsch_ue[0][i]->rnti   = 0x1234;

  }
  
  /*
  if (DLSCH_alloc_pdu2.tpmi == 5) 
    PHY_vars_eNb->eNB_UE_stats[0].DL_pmi_single = (unsigned short)(taus()&0xffff);
  else
    PHY_vars_eNb->eNB_UE_stats[0].DL_pmi_single = 0;

  PHY_vars_UE->dlsch_ue[0][0]->pmi_alloc = PHY_vars_eNb->eNB_UE_stats[0].DL_pmi_single;
  PHY_vars_eNb->dlsch_eNb[0][0]->pmi_alloc = PHY_vars_eNb->eNB_UE_stats[0].DL_pmi_single;
  */

  generate_eNb_dlsch_params_from_dci(0,
                                     &DLSCH_alloc_pdu2,
				     0x1234,
				     format2_2A_M10PRB,
				     PHY_vars_eNb->dlsch_eNb[0],
				     &PHY_vars_eNb->lte_frame_parms,
				     SI_RNTI,
				     RA_RNTI,
				     P_RNTI,
				     0); //change this later

				     
  /*
  generate_eNb_dlsch_params_from_dci(0,
                                     &CCCH_alloc_pdu,
				     SI_RNTI,
				     format1A,
				     &dlsch_eNb_cntl,
				     PHY_vars_eNb->lte_frame_parms,
				     SI_RNTI,
				     RA_RNTI,
				     P_RNTI);
  */
  
  
  //  input_data     = (unsigned char*) malloc(block_length/8);
  //  decoded_output = (unsigned char*) malloc(block_length/8);

  // DCI
  
  memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
  dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
  dci_alloc[0].L          = 1;
  dci_alloc[0].rnti       = 0x1234;
  /*
  memcpy(&dci_alloc[0].dci_pdu[0],&CCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
  dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
  dci_alloc[0].L          = 2;
  dci_alloc[0].rnti       = SI_RNTI;
  */
  memcpy(&dci_alloc[1].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
  dci_alloc[1].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
  dci_alloc[1].L          = 3;
  dci_alloc[1].rnti       = 0x1234;

  /*
  memcpy(&dci_alloc[2].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
  dci_alloc[2].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
  dci_alloc[2].L          = 2;
  dci_alloc[2].rnti       = 0x1235;
  */

  num_ue_spec_dci = 1;
  num_common_dci = 0;



  input_buffer_length = PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->TBS/8;
  
  printf("dlsch0: TBS      %d\n",PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->TBS);
  
  printf("Input buffer size %d bytes\n",input_buffer_length);
  
  
  input_buffer = (unsigned char *)malloc(input_buffer_length+4);
  memset(input_buffer,0,input_buffer_length+4);

  if (input_trch_file==0) {
    for (i=0;i<input_buffer_length;i++)
      input_buffer[i]= (unsigned char)(taus()&0xff);
  }
  else {
    i=0;
    while ((!feof(input_trch_fd)) && (i<input_buffer_length<<3)) {
      fscanf(input_trch_fd,"%s",input_trch_val);
      if (input_trch_val[0] == '1')
	input_buffer[i>>3]+=(1<<(7-(i&7)));
      if (i<16)
	printf("input_trch_val %d : %c\n",i,input_trch_val[0]);
      i++;
      if (((i%8) == 0) && (i<17))
	printf("%x\n",input_buffer[(i-1)>>3]);
    }
    printf("Read in %d bits\n",i);
  }
  
   //imran      
#ifdef ABSTRACTION
 for (ch_realization=0;ch_realization<60;ch_realization++){

 printf("**********************Channel Realization Index = %d **************************\n", ch_realization);
#endif
   
  for (SNR=snr0;SNR<snr1;SNR+=snr_step) {
    errs[0]=0;
    errs[1]=0;
    errs[2]=0;
    errs[3]=0;
    round_trials[0] = 0;
    round_trials[1] = 0;
    round_trials[2] = 0;
    round_trials[3] = 0;

    dci_errors=0;

    round=0;

    for (trials = 0;trials<n_frames;trials++) {
      //            printf("Trial %d\n",trials);
      fflush(stdout);
      round=0;
      while (round < num_rounds) {
	//printf("Trial %d : Round %d \n",trials,round);
	round_trials[round]++;
	if (round == 0) {
	  PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->Ndi = 1;
	  PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->rvidx = round>>1;
	  DLSCH_alloc_pdu2.ndi1             = 1;
	  DLSCH_alloc_pdu2.rv1              = 0;
	  memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
	}
	else {
	  PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->Ndi = 0;
	  PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->rvidx = round>>1;
	  DLSCH_alloc_pdu2.ndi1             = 0;
	  DLSCH_alloc_pdu2.rv1              = round>>1;
	  memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
	}
 
	num_pdcch_symbols = generate_dci_top(num_ue_spec_dci,
					     num_common_dci,
					     dci_alloc,
					     0,
					     1024,
					     &PHY_vars_eNb->lte_frame_parms,
					     PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
					     subframe);

	coded_bits_per_codeword = get_G(&PHY_vars_eNb->lte_frame_parms,NB_RB,PHY_vars_eNb->dlsch_eNb[0][0]->rb_alloc,
					get_Qm(mcs),num_pdcch_symbols,subframe);

#ifdef TBS_FIX
	rate = (double)3*dlsch_tbs25[get_I_TBS(mcs)][NB_RB-1]/(4*coded_bits_per_codeword);
#else
	rate = (double)dlsch_tbs25[get_I_TBS(mcs)][NB_RB-1]/(coded_bits_per_codeword);
#endif
	rate*=get_Qm(mcs);

	if (trials==0) 
	  printf("Rate = %f (G %d TBS %d, mod %d, pdcch_sym %d)\n",
		 rate,
		 coded_bits_per_codeword,
		 (int)(rate*coded_bits_per_codeword),
		 get_Qm(mcs),
		 num_pdcch_symbols);

	// use the PMI from previous trial
	if (DLSCH_alloc_pdu2.tpmi == 5) 
	  PHY_vars_eNb->dlsch_eNb[0][0]->pmi_alloc = pmi2hex_2Ar1(quantize_subband_pmi(&PHY_vars_UE->PHY_measurements,0));

	dlsch_encoding(input_buffer,
		       &PHY_vars_eNb->lte_frame_parms,
		       num_pdcch_symbols,
		       PHY_vars_eNb->dlsch_eNb[0][0],
		       subframe);

	dlsch_scrambling(&PHY_vars_eNb->lte_frame_parms,
			 num_pdcch_symbols,
			 PHY_vars_eNb->dlsch_eNb[0][0],
			 coded_bits_per_codeword,
			 0,
			 subframe<<1);

	if (n_frames==1) {
	  for (s=0;s<PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->C;s++) {
	    if (s<PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->Cminus)
	      Kr = PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->Kminus;
	    else
	      Kr = PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->Kplus;
	    
	    Kr_bytes = Kr>>3;
	    
	    for (i=0;i<Kr_bytes;i++)
	      printf("%d : (%x)\n",i,PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->c[s][i]);
	  }
	  
	}
	  
	re_allocated = dlsch_modulation(PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
					1024,
					subframe,
					&PHY_vars_eNb->lte_frame_parms,
					num_pdcch_symbols,
					PHY_vars_eNb->dlsch_eNb[0][0]);
	

	if (n_frames==1)
	  printf("RB count %d (%d,%d)\n",re_allocated,re_allocated/PHY_vars_eNb->lte_frame_parms.num_dlsch_symbols/12,PHY_vars_eNb->lte_frame_parms.num_dlsch_symbols);

	
	
	if (num_layers>1)
	  re_allocated = dlsch_modulation(PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
					  1024,
					  subframe,
					  &PHY_vars_eNb->lte_frame_parms,
					  num_pdcch_symbols,
					  PHY_vars_eNb->dlsch_eNb[0][1]);
	
	generate_pilots(PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
			1024,
			&PHY_vars_eNb->lte_frame_parms,
			eNb_id,
			LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
	
	
#ifdef IFFT_FPGA
	
	if (n_frames==1) {
	  write_output("txsigF0.m","txsF0", PHY_vars_eNb->lte_eNB_common_vars->txdataF[0][0],300*120,1,4);
	  write_output("txsigF1.m","txsF1", PHY_vars_eNb->lte_eNB_common_vars->txdataF[0][1],300*120,1,4);
	}
	
	// do talbe lookup and write results to txdataF2
	for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx;aa++) {
	  ind = 0;
	  //	  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX;i++) 
	  for (i=0;i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX;i++) 
	    if (((i%512)>=1) && ((i%512)<=150))
	      txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][ind++]];
	    else if ((i%512)>=362)
	      txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][ind++]];
	    else 
	      txdataF2[aa][i] = 0;
	  //    printf("ind=%d\n",ind);
	}
	
	tx_lev = 0;
	for (aa=0; aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx; aa++) {
	  if (frame_parms->Ncp == 1)
	    PHY_ofdm_mod(&txdataF2[aa][subframe*nsymb*PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size],        // input
			 &txdata[aa][subframe*PHY_vars_eNb->lte_frame_parms.samples_per_tti],         // output
			 PHY_vars_eNb->lte_frame_parms.log2_symbol_size,                // log2_fft_size
			 2*nsymb,//NUMBER_OF_SYMBOLS_PER_FRAME,                 // number of symbols
			 PHY_vars_eNb->lte_frame_parms.nb_prefix_samples,               // number of prefix samples
			 PHY_vars_eNb->lte_frame_parms.twiddle_ifft,  // IFFT twiddle factors
			 PHY_vars_eNb->lte_frame_parms.rev,           // bit-reversal permutation
			 CYCLIC_PREFIX);
	  else {
	    normal_prefix_mod(&txdataF2[aa][subframe*nsymb*PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size],
			      &txdata[aa][PHY_vars_eNb->lte_frame_parms.samples_per_tti],2*nsymb,frame_parms);
	  }
			      
			      
	  tx_lev += signal_energy(&txdata[aa][(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size+PHY_vars_eNb->lte_frame_parms.nb_prefix_samples0)],
				  OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
	}
	
#else //IFFT_FPGA
      

	if (n_frames==1) {
	  write_output("txsigF0.m","txsF0", PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0],10*PHY_vars_eNb->lte_frame_parms.samples_per_tti,1,1);
	  if (PHY_vars_eNb->lte_frame_parms.nb_antennas_tx>1)
	    write_output("txsigF1.m","txsF1", PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][1],10*PHY_vars_eNb->lte_frame_parms.samples_per_tti,1,1);
	}
	
	tx_lev = 0;
	for (aa=0; aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx; aa++) {
	  if (frame_parms->Ncp == 1) 
	    PHY_ofdm_mod(&PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][subframe*nsymb*PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size],        // input
			 &txdata[aa][subframe*PHY_vars_eNb->lte_frame_parms.samples_per_tti],         // output
			 PHY_vars_eNb->lte_frame_parms.log2_symbol_size,                // log2_fft_size
			 2*nsymb,//NUMBER_OF_SYMBOLS_PER_FRAME,                 // number of symbols
			 PHY_vars_eNb->lte_frame_parms.nb_prefix_samples,               // number of prefix samples
			 PHY_vars_eNb->lte_frame_parms.twiddle_ifft,  // IFFT twiddle factors
			 PHY_vars_eNb->lte_frame_parms.rev,           // bit-reversal permutation
			 CYCLIC_PREFIX);
	  else {
	    normal_prefix_mod(&PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][subframe*nsymb*PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size],
			      &txdata[aa][subframe*PHY_vars_eNb->lte_frame_parms.samples_per_tti],
			      2*nsymb,
			      frame_parms);
	  }
	  tx_lev += signal_energy(&txdata[aa][subframe*PHY_vars_eNb->lte_frame_parms.samples_per_tti],
				  frame_parms->ofdm_symbol_size);
	  
	}  
#endif //IFFT_FPGA
	
	
	//	printf("tx_lev = %d (%d)\n",tx_lev,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
	tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
	
	

	if (n_frames==1)
	  write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);




	for (i=0;i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
	  for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx;aa++) {
	    if (awgn_flag == 0) {
	      s_re[aa][i] = ((double)(((short *)txdata[aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) + (i<<1)]);
	      s_im[aa][i] = ((double)(((short *)txdata[aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)+1]);
            }
            else {
	      for (aarx=0;aarx<PHY_vars_UE->lte_frame_parms.nb_antennas_rx;aarx++) {
		if (aa==0) {
		  r_re[aarx][i] = ((double)(((short *)txdata[aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)]);
		  r_im[aarx][i] = ((double)(((short *)txdata[aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)+1]);
		}
		else {
		  r_re[aarx][i] += ((double)(((short *)txdata[aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)]);
		  r_im[aarx][i] += ((double)(((short *)txdata[aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)+1]);
		}
	      }
            }
	  }
	}
	//	n0_pow_dB = tx_lev_dB + 10*log10(512/(NB_RB*12)) + SNR;
#ifdef ABSTRACTION
	if (trials==0 && round==0){
        if (awgn_flag == 0) {	
	  if(SNR==snr0){
	  multipath_channel(eNB2UE,s_re,s_im,r_re,r_im,
			    2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
	  }else{
	    multipath_channel(eNB2UE,s_re,s_im,r_re,r_im,
			    2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,1);
}

	  freq_channel(eNB2UE, 25);
	  snr=pow(10.0,.1*SNR);
	  fprintf(csv_fd,"%f,",SNR);

	  for (u=0;u<50;u++){
	    abs_channel = (eNB2UE->chF[0][u].r*eNB2UE->chF[0][u].r + eNB2UE->chF[0][u].i*eNB2UE->chF[0][u].i);
	    pilot_sinr = 10*log10(snr*abs_channel);
	    fprintf(csv_fd,"%e,",pilot_sinr); 
	   
	  }
	 
	}
	}
	else{
	  if (awgn_flag == 0) {	
	  multipath_channel(eNB2UE,s_re,s_im,r_re,r_im,
			    2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,1);
	  }
        }
#else //ABStraction
 if (awgn_flag == 0) {	
	  multipath_channel(eNB2UE,s_re,s_im,r_re,r_im,
			    2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
 }

#endif //ABStraction
      
	//(double)tx_lev_dB - (SNR+sigma2_dB));
	//		printf("tx_lev_dB %d\n",tx_lev_dB);
	sigma2_dB = 10*log10((double)tx_lev) +10*log10(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size/(NB_RB*12)) - SNR;

	//AWGN
	sigma2 = pow(10,sigma2_dB/10);

	//	n0_pow_dB = tx_lev_dB + 10*log10(512/(NB_RB*12)) + SNR;
	//	printf("Sigma2 %f (sigma2_dB %f)\n",sigma2,sigma2_dB);
	for (i=0; i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES; i++) {
	  for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_rx;aa++) {
	    ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti)+2*i] = (short) (r_re[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	    ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti)+2*i+1] = (short) (r_im[aa][i] + (iqim*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	  }
	}    
	//    lte_sync_time_init(PHY_vars_eNb->lte_frame_parms,lte_ue_common_vars);
	//    lte_sync_time(lte_ue_common_vars->rxdata, PHY_vars_eNb->lte_frame_parms);
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


	if (n_frames==1) {
	  printf("RX level in null symbol %d\n",dB_fixed(signal_energy(&PHY_vars_UE->lte_ue_common_vars.rxdata[0][160+OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	  printf("RX level in data symbol %d\n",dB_fixed(signal_energy(&PHY_vars_UE->lte_ue_common_vars.rxdata[0][160+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	  printf("rx_level Null symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
	  printf("rx_level data symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
	}
	
	if (PHY_vars_eNb->lte_frame_parms.Ncp == 0) {  // normal prefix
	  pilot1 = 4;
	  pilot2 = 7;
	  pilot3 = 11;
	}
	else {  // extended prefix
	  pilot1 = 3;
	  pilot2 = 6;
	  pilot3 = 9;
	}

	// Inner receiver scheduling for 3 slots
	for (Ns=(2*subframe);Ns<((2*subframe)+3);Ns++) {
	  for (l=0;l<pilot2;l++) {
	    //	    	    	    printf("Ns %d, l %d\n",Ns,l);
	    slot_fep(&PHY_vars_UE->lte_frame_parms,
                     &PHY_vars_UE->lte_ue_common_vars,
		     l,
		     Ns%20,
		     0,
		     0);
	    if (l==0) {
	      lte_ue_measurements(PHY_vars_UE,
				  &PHY_vars_UE->lte_frame_parms,
				  subframe*PHY_vars_UE->lte_frame_parms.symbols_per_tti*(PHY_vars_UE->lte_frame_parms.ofdm_symbol_size+PHY_vars_UE->lte_frame_parms.nb_prefix_samples),
				  1,
				  0);
	      
	      //	      	printf("rx_avg_power_dB %d\n",PHY_vars_UE->PHY_measurements.rx_avg_power_dB[0]);
	      //	printf("n0_power_dB %d\n",PHY_vars->PHY_measurements.n0_power_dB[0]);
	      //if (trials%100==0)
	      //	printf("PMI %x\n",pmi2hex_2Ar1(quantize_subband_pmi(&PHY_vars_UE->PHY_measurements,0)));
	    }

	    if ((Ns==(2*subframe)) && (l==pilot1)) {// process symbols 0,1,2
	      PHY_vars_UE->lte_ue_pdcch_vars[0]->crnti = 0x1234;
	      PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols = num_pdcch_symbols;
	      if (dci_flag == 1) {
		rx_pdcch(&PHY_vars_UE->lte_ue_common_vars,
			 PHY_vars_UE->lte_ue_pdcch_vars,
			 &PHY_vars_UE->lte_frame_parms,
			 subframe,
			 eNb_id,
			 (PHY_vars_UE->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI,
			 0);
		

		dci_cnt = dci_decoding_procedure(PHY_vars_UE->lte_ue_pdcch_vars,
						 dci_alloc_rx,
						 eNb_id,
						 &PHY_vars_UE->lte_frame_parms,
						 get_mi(&PHY_vars_UE->lte_frame_parms,0),
						 SI_RNTI,
						 RA_RNTI);
		//printf("dci_cnt %d\n",dci_cnt);
		//write_output("dlsch00_ch0.m","dl00_ch0",&(lte_ue_common_vars->dl_ch_estimates[eNb_id][0][0]),(6*(lte_frame_parms.ofdm_symbol_size)),1,1);
		//write_output("rxsigF0.m","rxsF0", PHY_vars_UE->lte_ue_common_vars.rxdataF[0],2*12*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size,2,1);
		//write_output("pdcch_rxF_llr.m","pdcch_llr",PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->llr,2400,1,4);
		//exit(-1);
		
		
		if (dci_cnt==0) {
		  dlsch_active = 0;
		  if (round==0) {
		    dci_errors++;
		    round=5;
		    errs[0]++;
		    round_trials[0]++;
		    //		  printf("DCI error trial %d errs[0] %d\n",trials,errs[0]);
		  }
		  //		for (i=1;i<=round;i++)
		  //		  round_trials[i]--;
		  //		round=5;
		}
		
		for (i=0;i<dci_cnt;i++) {
		  if ((dci_alloc_rx[i].rnti == C_RNTI) && (dci_alloc_rx[i].format == format2_2A_M10PRB) &&
		      (generate_ue_dlsch_params_from_dci(0,
							 (DCI2_5MHz_2A_M10PRB_TDD_t *)&dci_alloc_rx[i].dci_pdu,
							 C_RNTI,
							 format2_2A_M10PRB,
							 PHY_vars_UE->dlsch_ue[0],
							 &PHY_vars_UE->lte_frame_parms,
							 SI_RNTI,
							 RA_RNTI,
							 P_RNTI)==0)) {
		    dlsch_active = 1;
		  }
		  else {
		    dlsch_active = 0;
		    if (round==0) {
		      dci_errors++;
		      errs[0]++;
		      round_trials[0]++;

		      if (n_frames==1) {
			printf("DCI misdetection trial %d\n",trials);
			round=5;
		      }
		    }
		    //		  for (i=1;i<=round;i++)
		    //		    round_trials[i]--;
		    //		  round=5;
		  }
		}

	      /*
		else if ((dci_alloc_rx[i].rnti == SI_RNTI) && (dci_alloc_rx[i].format == format1A))
		generate_ue_dlsch_params_from_dci(0,
		(DCI1A_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
		SI_RNTI,
		format1A,
		&dlsch_ue_cntl, 
		lte_frame_parms,
		SI_RNTI,
		RA_RNTI,
		P_RNTI);
	      */
	      //	      msg("dci_cnt = %d\n",dci_cnt);
	      }  // if dci_flag==1
	      else { //dci_flag == 0
		generate_ue_dlsch_params_from_dci(0,
						  &DLSCH_alloc_pdu2,
						  C_RNTI,
						  format2_2A_M10PRB,
						  PHY_vars_UE->dlsch_ue[0],
						  &PHY_vars_UE->lte_frame_parms,
						  SI_RNTI,
						  RA_RNTI,
						  P_RNTI);
		dlsch_active = 1;
	      } // if dci_flag == 1
	    }

	    if (dlsch_active == 1) {
	      if ((Ns==(1+(2*subframe))) && (l==0)) {// process symbols 3,4,5

		for (m=num_pdcch_symbols;
		     m<pilot2;
		     m++) {
		  //		  printf("Demodulating DLSCH for symbol %d (pilot 2 %d)\n",m,pilot2);
		  if (rx_dlsch(&PHY_vars_UE->lte_ue_common_vars,
			       PHY_vars_UE->lte_ue_dlsch_vars,
			       &PHY_vars_UE->lte_frame_parms,
			       eNb_id,
			       eNb_id_i,
			       PHY_vars_UE->dlsch_ue[0],
			       subframe,
			       m,
			       (m==num_pdcch_symbols)?1:0,
			       dual_stream_UE,
			       &PHY_vars_UE->PHY_measurements,
			       0)==-1) {
		    dlsch_active = 0;
		    break;
		  }
		}
	       
	      }
	      if ((Ns==(1+(2*subframe))) && (l==pilot1)) {// process symbols 6,7,8
		/*
		  if (rx_pbch(lte_ue_common_vars,
		  lte_ue_pbch_vars[0],
		  lte_frame_parms,
		  0,
		  SISO)) {
		  msg("pbch decoded sucessfully!\n");
		  }
		  else {
		  msg("pbch not decoded!\n");
		  }
		*/
		for (m=pilot2;
		     m<pilot3;
		     m++)
		  if (rx_dlsch(&PHY_vars_UE->lte_ue_common_vars,
			       PHY_vars_UE->lte_ue_dlsch_vars,
			       &PHY_vars_UE->lte_frame_parms,
			       eNb_id,
			       eNb_id_i,
			       PHY_vars_UE->dlsch_ue[0],
			       subframe,
			       m,
			       0,
			       dual_stream_UE,
			       &PHY_vars_UE->PHY_measurements,
			       0)==-1) {
		    dlsch_active=0;
		    break;
		  }
	      }
	      
	      if ((Ns==(2+(2*subframe))) && (l==0))  // process symbols 10,11, do deinterleaving for TTI
		for (m=pilot3;
		     m<PHY_vars_UE->lte_frame_parms.symbols_per_tti;
		     m++)
		  if (rx_dlsch(&PHY_vars_UE->lte_ue_common_vars,
			       PHY_vars_UE->lte_ue_dlsch_vars,
			       &PHY_vars_UE->lte_frame_parms,
			       eNb_id,
			       eNb_id_i,
			       PHY_vars_UE->dlsch_ue[0],
			       subframe,
			       m,
			       0,
			       dual_stream_UE,
			       &PHY_vars_UE->PHY_measurements,
			       0)==-1) {
		    dlsch_active=0;
		    break;
		  }
	    }
	  }
	}
	/*
#ifdef ABSTRACTION
	if(trials==0){
	  //fprintf(csv_fd,"%f,%d,%d,%d",SNR, rx_lev_data_sym , rx_lev_null_sym, rx_snr_dB);
	  fprintf(csv_fd,"%f,%e",SNR,n0_pow_dB);
	  data= &(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNb_id][0][0]);

	  for (ii=10;ii<((1*(PHY_vars_UE->lte_frame_parms.ofdm_symbol_size))<<1)-414;ii+=2) {
	    fprintf(csv_fd,",%d+1i*(%d)",((short *)data)[ii], ((short *)data)[ii+1]);
	    //fprintf(csv_fd,",%e",(double)10*log10(pow(2,((short *)data)[ii])+pow(2,((short *)data)[ii+1])) + 10*log10(2));
	    //printf("%e",(double)10*log10(pow(2,((short *)data)[ii])+pow(2,((short *)data)[ii+1])) + 10*log10(2));
	    //ch_power_dB[ii-10] = (double)10*log10(pow(2,((short *)data)[ii])+pow(2,((short *)data)[ii])) + 10*log10(2); 
	  }
	  fprintf(csv_fd,",");
	}
#endif
	*/
	if (dlsch_active == 1) {

	  if (n_frames==1) {
	    write_output("ch0.m","ch0",eNB2UE->ch[0],eNB2UE->channel_length,1,8);
	    write_output("rxsig0.m","rxs0", &PHY_vars_UE->lte_ue_common_vars.rxdata[0][0],10*PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
	    write_output("dlsch00_ch0.m","dl00_ch0",&(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNb_id][0][0]),(6*(PHY_vars_UE->lte_frame_parms.ofdm_symbol_size)),1,1);

	  /*
	    write_output("dlsch01_ch0.m","dl01_ch0",&(lte_ue_common_vars->dl_ch_estimates[eNb_id][1][0]),(6*(lte_frame_parms.ofdm_symbol_size)),1,1);
	    write_output("dlsch10_ch0.m","dl10_ch0",&(lte_ue_common_vars->dl_ch_estimates[eNb_id][2][0]),(6*(lte_frame_parms.ofdm_symbol_size)),1,1);	    
	    write_output("dlsch11_ch0.m","dl11_ch0",&(lte_ue_common_vars->dl_ch_estimates[eNb_id][3][0]),(6*(lte_frame_parms.ofdm_symbol_size)),1,1);
	  */

	    write_output("rxsigF0.m","rxsF0", &PHY_vars_UE->lte_ue_common_vars.rxdataF[0][2*subframe*nsymb*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size],2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);
	    write_output("rxsigF0_ext.m","rxsF0_ext", PHY_vars_UE->lte_ue_dlsch_vars[eNb_id]->rxdataF_ext[0],2*12*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size,1,1);
	    if (PHY_vars_UE->lte_frame_parms.nb_antennas_rx>1) {
	      write_output("rxsig1.m","rxs1", PHY_vars_UE->lte_ue_common_vars.rxdata[1],PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
	      write_output("rxsigF1.m","rxsF1", PHY_vars_UE->lte_ue_common_vars.rxdataF[1],2*12*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size,2,1);
	      write_output("rxsigF1_ext.m","rxsF1_ext", PHY_vars_UE->lte_ue_dlsch_vars[eNb_id]->rxdataF_ext[1],2*12*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size,1,1);
	      
	    }
	    write_output("dlsch00_ch0_ext.m","dl00_ch0_ext",PHY_vars_UE->lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[0],300*12,1,1);
	    if (PHY_vars_eNb->lte_frame_parms.nb_antennas_tx>1) {
	      write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",PHY_vars_UE->lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[2],300*12,1,1);
	    }
	    write_output("pdcchF0_ext.m","pdcchF_ext", PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->rxdataF_ext[0],2*3*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size,1,1);
	    write_output("pdcch00_ch0_ext.m","pdcch00_ch0_ext",PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->dl_ch_estimates_ext[0],300*3,1,1);
	    /*
	      write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[1],300*12,1,1);
	      write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[2],300*12,1,1);
	      write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[3],300*12,1,1);
	      write_output("dlsch_rho.m","dl_rho",lte_ue_dlsch_vars[eNb_id]->rho[0],300*12,1,1);
	    */
	    write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0",PHY_vars_UE->lte_ue_dlsch_vars[eNb_id]->rxdataF_comp[0],300*(-(PHY_vars_UE->lte_frame_parms.Ncp*2)+14),1,1);
	    write_output("pdcch_rxF_comp0.m","pdcch0_rxF_comp0",PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->rxdataF_comp[0],4*300,1,1);
	    write_output("dlsch_e.m","e",PHY_vars_eNb->dlsch_eNb[0][0]->e,coded_bits_per_codeword,1,4);
	    write_output("dlsch_rxF_llr.m","dlsch_llr",PHY_vars_UE->lte_ue_dlsch_vars[eNb_id]->llr[0],coded_bits_per_codeword,1,0);
	    write_output("pdcch_rxF_llr.m","pdcch_llr",PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->llr,2400,1,4);
	    
	    write_output("dlsch_mag1.m","dlschmag1",PHY_vars_UE->lte_ue_dlsch_vars[eNb_id]->dl_ch_mag,300*12,1,1);
	    write_output("dlsch_mag2.m","dlschmag2",PHY_vars_UE->lte_ue_dlsch_vars[eNb_id]->dl_ch_magb,300*12,1,1);
	  }
	    
	    //	printf("Calling decoding (Ndi %d, harq_pid %d)\n",
	  //       dlsch_ue[0]->harq_processes[0]->Ndi,
	  //       dlsch_ue[0]->current_harq_pid);
	  
	  dlsch_unscrambling(&PHY_vars_UE->lte_frame_parms,
			     num_pdcch_symbols,
			     PHY_vars_UE->dlsch_ue[0][0],
			     coded_bits_per_codeword,
			     PHY_vars_UE->lte_ue_dlsch_vars[eNb_id]->llr[0],
			     0,
			     subframe<<1);
		     
	  ret = dlsch_decoding(PHY_vars_UE->lte_ue_dlsch_vars[eNb_id]->llr[0],		 
			       &PHY_vars_UE->lte_frame_parms,
			       PHY_vars_UE->dlsch_ue[0][0],
			       subframe,
			       num_pdcch_symbols);
	  
	  if (ret <= MAX_TURBO_ITERATIONS) {

	    if (n_frames==1) 
	      printf("No DLSCH errors found\n");
	    //	    exit(-1);
	    if (fix_rounds==0)
	      round=5;
	    else
	      round++;
	  }	
	  else {
	    errs[round]++;

	    if (n_frames==1) {
	      printf("DLSCH errors found\n");
	      for (s=0;s<PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->C;s++) {
		if (s<PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->Cminus)
		  Kr = PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->Kminus;
		else
		  Kr = PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->Kplus;
		
		Kr_bytes = Kr>>3;
		
		printf("Decoded_output (Segment %d):\n",s);
		for (i=0;i<Kr_bytes;i++)
		  printf("%d : %x (%x)\n",i,PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->c[s][i],PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->c[s][i]^PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->c[s][i]);
	      }
	      exit(-1);
	    }
	    //	    printf("round %d errors %d/%d\n",round,errs[round],trials);
	    round++;

	    if (n_frames==1)
	      printf("DLSCH in error in round %d\n",round);

	  }
	}

      }  //round
      //      printf("\n");
      if ((errs[0]>=100) && (trials>(n_frames/2)))
	break;

      //len = chbch_stats_read(stats_buffer,NULL,0,4096);
      //printf("%s\n\n",stats_buffer);

    }   //trials
    printf("\n**********************SNR = %f dB (tx_lev %f, sigma2_dB %f)**************************\n",
	   SNR,
	   (double)tx_lev_dB+10*log10(PHY_vars_UE->lte_frame_parms.ofdm_symbol_size/(NB_RB*12)),
	   sigma2_dB);

    printf("Errors (%d/%d %d/%d %d/%d %d/%d), Pe = (%e,%e,%e,%e), dci_errors %d/%d, Pe = %e => effective rate %f (%f), normalized delay %f (%f)\n",
	   errs[0],
	   round_trials[0],
	   errs[1],
	   round_trials[1],
	   errs[2],
	   round_trials[2],
	   errs[3],
	   round_trials[3],
	   (double)errs[0]/(round_trials[0]),
	   (double)errs[1]/(round_trials[1]),
	   (double)errs[2]/(round_trials[2]),
	   (double)errs[3]/(round_trials[3]),
	   dci_errors,
	   round_trials[0],
	   (double)dci_errors/(round_trials[0]),
	   rate*((double)(round_trials[0]-dci_errors)/((double)round_trials[0] + round_trials[1] + round_trials[2] + round_trials[3])),
	   rate,
	   (1.0*(round_trials[0]-errs[0])+2.0*(round_trials[1]-errs[1])+3.0*(round_trials[2]-errs[2])+4.0*(round_trials[3]-errs[3]))/((double)round_trials[0])/(double)PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->TBS,
	   (1.0*(round_trials[0]-errs[0])+2.0*(round_trials[1]-errs[1])+3.0*(round_trials[2]-errs[2])+4.0*(round_trials[3]-errs[3]))/((double)round_trials[0]));

    fprintf(bler_fd,"%f;%d;%d;%f;%d;%d;%d;%d;%d;%d;%d;%d;%d\n",
	    SNR,
	    mcs,
	    PHY_vars_eNb->dlsch_eNb[0][0]->harq_processes[0]->TBS,
	    rate,
	    errs[0],
	    round_trials[0],
	    errs[1],
	    round_trials[1],
	    errs[2],
	    round_trials[2],
	    errs[3],
	    round_trials[3],
	    dci_errors);

#ifdef ABSTRACTION         
      blerr= (double)errs[0]/(round_trials[0]);
      fprintf(csv_fd,"%e;\n",blerr);
#endif //ABStraction

      if (((double)errs[0]/(round_trials[0]))<1e-2) 
	      break;
  }// SNR
  
#ifdef ABSTRACTION
 } //ch_realization
#endif
 
 fclose(bler_fd);
 if (input_trch_file==1)
   fclose(input_trch_fd);
#ifdef ABSTRACTION
 fprintf(csv_fd,"];");
 fclose(csv_fd);
#endif
   
    
   

  printf("Freeing dlsch structures\n");
  for (i=0;i<2;i++) {
    printf("eNb %d\n",i);
    free_eNb_dlsch(PHY_vars_eNb->dlsch_eNb[0][i]);
    printf("UE %d\n",i);
    free_ue_dlsch(PHY_vars_UE->dlsch_ue[0][i]);
  }


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
  
  //  lte_sync_time_free();

  return(0);
}
   

