#include <string.h>
#include <math.h>
#include <unistd.h>
#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif
#ifdef EMOS
#include "SCHED/phy_procedures_emos.h"
#endif
#include "SCHED/defs.h"
#include "SCHED/vars.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"
#include "LAYER2/MAC/vars.h"

#define BW 10.0
#define N_TRIALS 100

PHY_VARS_eNB *PHY_vars_eNb,*PHY_vars_eNb1,*PHY_vars_eNb2;;
PHY_VARS_UE *PHY_vars_UE;

#define UL_RB_ALLOC 0x1ff;
#define CCCH_RB_ALLOC computeRIV(PHY_vars_eNb->lte_frame_parms.N_RB_UL,0,2)
#define DLSCH_RB_ALLOC 0x1fbf // igore DC component,RB13


void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,unsigned char extended_prefix_flag,u16 Nid_cell,u8 tdd_config,u8 N_RB_DL,u8 osf) {

  unsigned int ind,i;
  LTE_DL_FRAME_PARMS *lte_frame_parms;

  printf("Start lte_param_init (Nid_cell %d, extended_prefix %d, transmission_mode %d, N_tx %d, N_rx %d)\n",
	 Nid_cell, extended_prefix_flag,transmission_mode,N_tx,N_rx);
  PHY_vars_eNb = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_eNb1 = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_eNb2 = malloc(sizeof(PHY_VARS_eNB));

  PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  //PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  randominit(0);
  set_taus_seed(0);
  
  lte_frame_parms = &(PHY_vars_eNb->lte_frame_parms);

  lte_frame_parms->N_RB_DL            = N_RB_DL;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->N_RB_UL            = N_RB_DL;   
  lte_frame_parms->Ncp                = extended_prefix_flag;
  lte_frame_parms->Nid_cell           = Nid_cell;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  lte_frame_parms->phich_config_common.phich_resource = oneSixth;
  lte_frame_parms->tdd_config         = tdd_config;
  lte_frame_parms->frame_type         = 1;

  //  lte_frame_parms->Csrs = 2;
  //  lte_frame_parms->Bsrs = 0;
  //  lte_frame_parms->kTC = 0;
  //  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->mode1_flag = (transmission_mode == 1)? 1 : 0;

  init_frame_parms(lte_frame_parms,osf);
  
  //copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(lte_frame_parms); //allocation

   
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;

  memcpy(&PHY_vars_UE->lte_frame_parms,lte_frame_parms,sizeof(LTE_DL_FRAME_PARMS));

  
  phy_init_lte_top(lte_frame_parms);

  phy_init_lte_ue(&PHY_vars_UE->lte_frame_parms,
		  &PHY_vars_UE->lte_ue_common_vars,
		  PHY_vars_UE->lte_ue_dlsch_vars,
		  PHY_vars_UE->lte_ue_dlsch_vars_SI,
		  PHY_vars_UE->lte_ue_dlsch_vars_ra,
		  PHY_vars_UE->lte_ue_pbch_vars,
		  PHY_vars_UE->lte_ue_pdcch_vars,
		  PHY_vars_UE,0);

  phy_init_lte_eNB(&PHY_vars_eNb->lte_frame_parms,
		   &PHY_vars_eNb->lte_eNB_common_vars,
		   PHY_vars_eNb->lte_eNB_ulsch_vars,
		   0,
		   PHY_vars_eNb,
		   0,
		   0);


  memcpy((void*)&PHY_vars_eNb1->lte_frame_parms,(void*)&PHY_vars_eNb->lte_frame_parms,sizeof(LTE_DL_FRAME_PARMS));
  PHY_vars_eNb1->lte_frame_parms.nushift=1;
  PHY_vars_eNb1->lte_frame_parms.Nid_cell=2;

  memcpy((void*)&PHY_vars_eNb2->lte_frame_parms,(void*)&PHY_vars_eNb->lte_frame_parms,sizeof(LTE_DL_FRAME_PARMS));
  PHY_vars_eNb2->lte_frame_parms.nushift=2;
  PHY_vars_eNb2->lte_frame_parms.Nid_cell=3;

  phy_init_lte_eNB(&PHY_vars_eNb1->lte_frame_parms,
		   &PHY_vars_eNb1->lte_eNB_common_vars,
		   PHY_vars_eNb1->lte_eNB_ulsch_vars,
		   0,
		   PHY_vars_eNb1,
		   0,
		   0);

  phy_init_lte_eNB(&PHY_vars_eNb2->lte_frame_parms,
		   &PHY_vars_eNb2->lte_eNB_common_vars,
		   PHY_vars_eNb2->lte_eNB_ulsch_vars,
		   0,
		   PHY_vars_eNb2,
		   0,
		   0);

  phy_init_lte_top(lte_frame_parms);

  for (i=0;i<3;i++)
    lte_gold(lte_frame_parms,PHY_vars_UE->lte_gold_table[i],i);    

  printf("Done lte_param_init\n");

  CCCH_alloc_pdu.type               = 1;
  CCCH_alloc_pdu.vrb_type           = 0;
  CCCH_alloc_pdu.rballoc            = CCCH_RB_ALLOC;
  CCCH_alloc_pdu.ndi      = 1;
  CCCH_alloc_pdu.mcs      = 1;
  CCCH_alloc_pdu.harq_pid = 0;

}

int main(int argc, char **argv) {

  char c;

  int i,l,aa,sector;
  double sigma2, sigma2_dB=0,SNR,snr0=-2.0,snr1;
  //mod_sym_t **txdataF;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  int **txdata;
  double **s_re,**s_im,**r_re,**r_im;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=0.0000005,Td=.8,iqim=0.0;
  u8 channel_length,nb_taps=8;
  struct complex **ch;
  unsigned char pbch_pdu[6];
  int sync_pos, sync_pos_slot;
  FILE *rx_frame_file;
  int result;
  int freq_offset;
  int subframe_offset;
  u8 subframe=0;
  char fname[40], vname[40];
  int trial, n_errors_common=0,n_errors_ul=0,n_errors_dl=0,n_errors_cfi;
  unsigned char eNb_id = 0;

  u8 awgn_flag=0;
  double nf[2] = {3.0,3.0}; //currently unused
  double ip =0.0;
  double N0W, path_loss, path_loss_dB;
  int n_frames=1;
  channel_desc_t *eNB2UE;
  u32 nsymb,tx_lev,tx_lev_dB;
  u8 extended_prefix_flag=0,transmission_mode=1,n_tx=1,n_rx=1,num_pdcch_symbols;
  u16 Nid_cell=0;
  s8 interf1=-128,interf2=-128;
  u8 dci_cnt=0;
  LTE_DL_FRAME_PARMS *frame_parms;
  u8 log2L=2, log2Lcommon=2, N=3, format_selector=0;
  u8 dci_length=sizeof_DCI1_5MHz_TDD_t;
  DCI_format_t format=format1;
  u8 dci_length_bytes=sizeof(DCI1_5MHz_TDD_t);
  u8 numCCE,nCCE_max,common_active,ul_active,dl_active,num_dci,num_common_dci,num_ue_spec_dci;
  u32 rv;

  u32 n_trials_common=0,n_trials_ul=0,n_trials_dl=0,false_detection_cnt=0;
  u8 common_rx,ul_rx,dl_rx;
  u8 tdd_config=3;

  FILE *input_fd=NULL;
  char input_val_str[50],input_val_str2[50];
  double input_val1,input_val2;
  u16 n_rnti=0x1234;
  u8 osf=1,N_RB_DL=25;

  SCM_t channel_model=custom;

  DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];

  void* dlsch_pdu = NULL;

  channel_length = (int) 11+2*BW*Td;

  number_of_cards = 1;
  openair_daq_vars.rx_rf_mode = 1;
  
  /*
    rxdataF    = (int **)malloc16(2*sizeof(int*));
    rxdataF[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    rxdataF[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
    
    rxdata    = (int **)malloc16(2*sizeof(int*));
    rxdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    rxdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  */
  while ((c = getopt (argc, argv, "har:pg:d:c:i:j:n:s:t:x:y:z:L:M:N:I:F:R:S:")) != -1) {
    switch (c)
      {
      case 'a':
	printf("Running AWGN simulation\n");
	awgn_flag = 1;
	break;
      case 'd':
	N_RB_DL = atoi(optarg);
	break;
      case 'c':
	tdd_config=atoi(optarg);
	if (tdd_config>6) {
	  printf("Illegal tdd_config %d (should be 0-6)\n",tdd_config);
	  exit(-1);
	}
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
	  case 'E': 
	    channel_model=EPA;
	    break;
	  case 'F': 
	    channel_model=EVA;
	    break;
	  case 'G': 
	    channel_model=ETU;
	    break;
	  default:
	    msg("Unsupported channel model!\n");
	    exit(-1);
	  }
	break;
      case 'i':
	interf1=atoi(optarg);
	break;
      case 'j':
	interf2=atoi(optarg);
	break;
      case 'n':
	n_frames = atoi(optarg);
	break;
      case 's':
	snr0 = atoi(optarg);
	break;
      case 't':
	Td= atof(optarg);
	break;
      case 'p':
	extended_prefix_flag=1;
	break;
      case 'r':
	ricean_factor = pow(10,-.1*atof(optarg));
	if (ricean_factor>1) {
	  printf("Ricean factor must be between 0 and 1\n");
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
      case 'S':
	subframe=atoi(optarg);
	break;
      case 'L':
	log2L=atoi(optarg);
	
	if ((log2L!=0)&&
	    (log2L!=1)&&
	    (log2L!=2)&&
	    (log2L!=3)) {
	  msg("Unsupported DCI aggregation level %d (should be 0,1,2,3)\n",log2L);
	  exit(-1);
	}
	break;
	
      case 'M':
	log2Lcommon=atoi(optarg);
	
	if ((log2Lcommon!=2)&&
	    (log2Lcommon!=3)) {
	  msg("Unsupported Common DCI aggregation level %d (should be 2 or 3)\n",log2Lcommon);
	  exit(-1);
	}
	break;	  
      case 'N':
	format_selector = atoi(optarg);
	break;
      case 'O':
	osf = atoi(optarg);
	break;
      case 'I':
	Nid_cell = atoi(optarg);
	break;
      case 'F':
	input_fd = fopen(optarg,"r");
	if (input_fd==NULL) {
	  printf("Problem with filename %s\n",optarg);
	  exit(-1);
	}
	break;
      case 'R':
	n_rnti=atoi(optarg);
	break;
      case 'h':  
	printf("%s -h(elp) -a(wgn on) -c tdd_config -n n_frames -r RiceanFactor -s snr0 -t Delayspread -x transmission mode (1,2,6) -y TXant -z RXant -L AggregLevelUEspec -M AggregLevelCommonDCI -N DCIFormat\n\n",argv[0]);
	printf("-h This message\n");
	printf("-a Use AWGN channel and not multipath\n");
	printf("-c TDD config\n");
	printf("-d N_RB_DL\n");
	printf("-p Use extended prefix mode\n");
	printf("-n Number of frames to simulate\n");
	printf("-r Ricean factor (dB, 0 means Rayleigh, 100 is almost AWGN\n");
	printf("-s Starting SNR, runs from SNR to SNR + 5 dB.  If n_frames is 1 then just SNR is simulated\n");
	printf("-t Delay spread for multipath channel\n");
	printf("-x Transmission mode (1,2,6 for the moment)\n");
	printf("-y Number of TX antennas used in eNB\n");
	printf("-z Number of RX antennas used in UE\n");
	printf("-L log2 of Aggregation level for UE Specific DCI (1,2,4,8)\n");
	printf("-M log2 Aggregation level for Common DCI (4,8)\n");
	printf("-N Format for UE Spec DCI (0 - format1,\n");
	printf("                           1 - format1A,\n");
	printf("                           2 - format1B_2A,\n");
	printf("                           3 - format1B_4A,\n");
	printf("                           4 - format1C,\n");
	printf("                           5 - format1D_2A,\n");
	printf("                           6 - format1D_4A,\n");
	printf("                           7 - format2A_2A_L10PRB,\n");
	printf("                           8 - format2A_2A_M10PRB,\n");
	printf("                           9 - format2A_4A_L10PRB,\n");
	printf("                          10 - format2A_4A_M10PRB,\n");
	printf("                          11 - format2_2A_L10PRB,\n");
	printf("                          12 - format2_2A_M10PRB,\n");
	printf("                          13 - format2_4A_L10PRB,\n");
	printf("                          14 - format2_4A_M10PRB\n");  
	printf("                          15 - format2_2D_M10PRB\n");  
	printf("                          16 - format2_2D_L10PRB\n");  
	printf("-O Oversampling factor\n");
	printf("-I Cell Id\n");
	printf("-F Input sample stream\n");
	exit(1);
	break;
      }
  }

  switch(format_selector) {
  case 0:
    dlsch_pdu  = (void*) &DLSCH_alloc_pdu;
    format     = format1;
    dci_length = sizeof_DCI1_5MHz_TDD_t;
    dci_length_bytes = sizeof(DCI1_5MHz_TDD_t);
    break;
  case 1:
    format     = format1A;
    if (tdd_config==0) {
      dci_length = sizeof_DCI1A_5MHz_TDD_0_t;
      dci_length_bytes = sizeof(DCI1A_5MHz_TDD_0_t);
    }
    else {
      dlsch_pdu = (void*) &DLSCH_alloc_pdu1A;
      dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
      dci_length_bytes = sizeof(DCI1A_5MHz_TDD_1_6_t);
    }
    break;
  case 2:
    format     = format1B;
    dci_length = sizeof_DCI1B_5MHz_2A_TDD_t;
    dci_length_bytes = sizeof(DCI1B_5MHz_2A_TDD_t);
    break;
  case 3:
    format     = format1B;
    dci_length = sizeof_DCI1B_5MHz_4A_TDD_t;
    dci_length_bytes = sizeof(DCI1B_5MHz_4A_TDD_t);
    break;
  case 4:
    format     = format1C;
    dci_length = sizeof_DCI1C_5MHz_t;
    dci_length_bytes = sizeof(DCI1C_5MHz_t);
    break;
  case 5:
    format     = format1D;
    dci_length = sizeof_DCI1D_5MHz_2A_TDD_t;
    dci_length_bytes = sizeof(DCI1D_5MHz_2A_TDD_t);
    break;
  case 6:
    format     = format1D;
    dci_length = sizeof_DCI1D_5MHz_4A_TDD_t;
    dci_length_bytes = sizeof(DCI1D_5MHz_4A_TDD_t);
    break;
  case 7:
    format     = format2A_2A_L10PRB;
    dci_length = sizeof_DCI2A_5MHz_2A_L10PRB_TDD_t;
    dci_length_bytes = sizeof(DCI2A_5MHz_2A_L10PRB_TDD_t);
    break;
  case 8:
    format     = format2A_2A_M10PRB;
    dci_length = sizeof_DCI2A_5MHz_2A_M10PRB_TDD_t;
    dci_length_bytes = sizeof(DCI2A_5MHz_2A_M10PRB_TDD_t);
    break;
  case 9:
    format     = format2A_4A_L10PRB;
    dci_length = sizeof_DCI2A_5MHz_4A_L10PRB_TDD_t;
    dci_length_bytes = sizeof(DCI2A_5MHz_4A_L10PRB_TDD_t);
    break;
  case 10:
    format     = format2A_4A_M10PRB;
    dci_length = sizeof_DCI2A_5MHz_4A_M10PRB_TDD_t;
    dci_length_bytes = sizeof(DCI2A_5MHz_4A_M10PRB_TDD_t);
    break;
  case 11:
    dlsch_pdu = (void*) &DLSCH_alloc_pdu1;
    format     = format2_2A_L10PRB;
    dci_length = sizeof_DCI2_5MHz_2A_L10PRB_TDD_t;
    dci_length_bytes = sizeof(DCI2_5MHz_2A_L10PRB_TDD_t);
    break;
  case 12:
    dlsch_pdu = (void*) &DLSCH_alloc_pdu2;
    format     = format2_2A_M10PRB;
    dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
    dci_length_bytes = sizeof(DCI2_5MHz_2A_M10PRB_TDD_t);
    break;
  case 13:
    format     = format2_4A_L10PRB;
    dci_length = sizeof_DCI2_5MHz_4A_L10PRB_TDD_t;
    dci_length_bytes = sizeof(DCI2_5MHz_4A_L10PRB_TDD_t);
    break;
  case 14:
    format     = format2_4A_M10PRB;
    dci_length = sizeof_DCI2_5MHz_4A_M10PRB_TDD_t;
    dci_length_bytes = sizeof(DCI2_5MHz_4A_M10PRB_TDD_t);
    break;
  case 15:
    dlsch_pdu = (void*) &DLSCH_alloc_pdu2D;
    format     = format2_2D_M10PRB;
    dci_length = sizeof_DCI2_5MHz_2D_M10PRB_TDD_t;
    dci_length_bytes = sizeof(DCI2_5MHz_2D_M10PRB_TDD_t);
    break;
    /*
  case 16:
    format     = format2_2D_L10PRB;
    dci_length = sizeof_DCI2_5MHz_2D_L10PRB_TDD_t;
    dci_length_bytes = sizeof(DCI2_5MHz_2D_L10PRB_TDD_t);
    break;
    */
  default:
    break;
  }



  if ((transmission_mode>1) && (n_tx==1))
    n_tx=2;

  lte_param_init(n_tx,
		 n_rx,
		 transmission_mode,
		 extended_prefix_flag,
		 Nid_cell,
		 tdd_config,
		 N_RB_DL,
		 osf);

  mac_xface->computeRIV = computeRIV;
  mac_xface->lte_frame_parms = &PHY_vars_eNb->lte_frame_parms;
  init_transport_channels(transmission_mode);

  if (n_frames==1)
    snr1 = snr0+.1;
  else
    snr1 = snr0+5.0;

  printf("SNR0 %f, SNR1 %f\n",snr0,snr1);

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

  nsymb = (frame_parms->Ncp == 0) ? 14 : 12;

  printf("FFT Size %d, Extended Prefix %d, Samples per subframe %d, Symbols per subframe %d\n",NUMBER_OF_OFDM_CARRIERS,
	 frame_parms->Ncp,frame_parms->samples_per_tti,nsymb);

  if (channel_model==custom) {
    msg("[SIM] Using custom channel model\n");

    eNB2UE = new_channel_desc(n_tx,
			      n_rx,
			      nb_taps,
			      channel_length,
			      amps,
			      NULL,
			      NULL,
			      Td,
			      BW,
			      ricean_factor,
			      aoa,
			      0,
			      0,
			      0,
			      0);
  }
  else {
    msg("[SIM] Using SCM/101\n");
    eNB2UE = new_channel_desc_scm(PHY_vars_eNb->lte_frame_parms.nb_antennas_tx,
				  PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				  channel_model,
				  BW,
				  0,
				  0,
				  0);

  }
  for (i=0;i<2;i++) {

    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_re[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_im[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_re[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_im[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }

  
  ch = (struct complex**) malloc(4 * sizeof(struct complex*));
  for (i = 0; i<4; i++)
    ch[i] = (struct complex*) malloc(channel_length * sizeof(struct complex));

  if (input_fd==NULL) {
    msg("No input file, so starting TX\n");
    generate_pilots(PHY_vars_eNb,
		    PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
		    1024,
		    LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
  }    
  else {
    i=0;
    while (!feof(input_fd)) {
      fscanf(input_fd,"%s %s",input_val_str,input_val_str2);//&input_val1,&input_val2);
      
      if ((i%4)==0) {
	((short*)txdata[0])[i/2] = (short)((1<<15)*strtod(input_val_str,NULL));
	((short*)txdata[0])[(i/2)+1] = (short)((1<<15)*strtod(input_val_str2,NULL));
	if ((i/4)<100)
	  printf("sample %d => %e + j%e (%d +j%d)\n",i/4,strtod(input_val_str,NULL),strtod(input_val_str2,NULL),((short*)txdata[0])[i/4],((short*)txdata[0])[(i/4)+1]);//1,input_val2,);
      }
      i++;
      if (i>(FRAME_LENGTH_SAMPLES))
	break;
    }
    printf("Read in %d samples\n",i/4);
    write_output("txsig0.m","txs0", txdata[0],2*frame_parms->samples_per_tti,1,1);
    //    write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    tx_lev = signal_energy(&txdata[0][0],
			   OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
    tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
  }


  randominit(0);
  
  
  nCCE_max = get_nCCE(3,&PHY_vars_eNb->lte_frame_parms,get_mi(&PHY_vars_eNb->lte_frame_parms,0));
  printf("nCCE_max %d\n",nCCE_max);
  
  for (SNR=snr0;SNR<snr1;SNR+=1) {
      
      
    n_errors_common = 0;
    n_errors_ul = 0;
    n_errors_dl = 0;
    n_errors_cfi = 0;
    n_trials_common=0;
    n_trials_ul=0;
    n_trials_dl=0;
      
    for (trial=0; trial<n_frames; trial++) {

      /*
      for (aa=0; aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx;aa++) {
#ifdef IFFT_FPGA
      memset(&PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][0],0,NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_SYMBOLS_PER_FRAME*sizeof(mod_sym_t));
#else
      memset(&PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][0],0,FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
#endif
      }
      */

      if (input_fd == NULL) {
	common_active=0;
	dl_active=0;
	ul_active=0;
	num_dci = 0;
	num_common_dci=0;
	num_ue_spec_dci=0;
	numCCE=0;
	  
	// Do SI_RNTI
	rv = (n_frames==1) ? 0 : taus();
  
	if ((rv&1) == 0) {
	  memcpy(&dci_alloc[0].dci_pdu[0],&CCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
	  dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
	  dci_alloc[0].L          = log2Lcommon;
	  dci_alloc[0].rnti       = SI_RNTI;
	  dci_alloc[0].format     = format1A;
	  num_dci++;
	  num_common_dci++;
	  numCCE += (1<<log2Lcommon);
	  common_active=1;
	  n_trials_common++;
	}
	// Try UL DCI for 0x1234
	if (numCCE < nCCE_max) {
	  rv = (n_frames==1) ? 0 : taus();
	  if (((rv&1)==0) && 
	      ((numCCE+(1<<log2L)) <= nCCE_max)) {
	    memcpy(&dci_alloc[num_dci].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
	    dci_alloc[num_dci].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
	    dci_alloc[num_dci].L          = log2L;
	    dci_alloc[num_dci].rnti       = n_rnti;
	    dci_alloc[num_dci].format     = format0;
	    num_dci++;
	    num_ue_spec_dci++;
	    numCCE+=(1<<log2L);
	    ul_active=1;
	    n_trials_ul++;
	  }
	}
	// Now try DL DCI for 0x1234
	if (numCCE < nCCE_max) {
	  rv = (n_frames==1) ? 0 : taus();
	  if (((rv&1)==0) && 
	      ((numCCE+(1<<log2L)) <= nCCE_max)) {
	    if (dlsch_pdu==NULL) {
	      printf("DCI format not supported!\n");
	      exit(-1);
	    }
	    memcpy(&dci_alloc[num_dci].dci_pdu[0],dlsch_pdu,dci_length_bytes);
	    dci_alloc[num_dci].dci_length = dci_length;
	    dci_alloc[num_dci].L          = log2L;
	    dci_alloc[num_dci].rnti       = n_rnti;
	    dci_alloc[num_dci].format     = format;
	    num_dci++;
	    num_ue_spec_dci++;
	    numCCE+=(1<<log2L);
	    dl_active=1;
	    n_trials_dl++;
	  }
	}
	  
	num_pdcch_symbols = generate_dci_top(num_ue_spec_dci,
					     num_common_dci,
					     dci_alloc,
					     0,
					     1024,
					     &PHY_vars_eNb->lte_frame_parms,
					     PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
					     subframe);
	if (n_frames==1)
	  printf("Num_pdcch_symbols %d, Num_common_dci %d, Num_ue_spec_dci %d, Num_CCE %d/%d (nCCE %d, nREG %d)\n",num_pdcch_symbols,
		 num_common_dci,num_ue_spec_dci,numCCE,nCCE_max,get_nCCE(num_pdcch_symbols,&PHY_vars_eNb->lte_frame_parms,get_mi(&PHY_vars_eNb->lte_frame_parms,0)),get_nquad(num_pdcch_symbols,frame_parms,get_mi(&PHY_vars_eNb->lte_frame_parms,0)));
	  
	//  write_output("pilotsF.m","rsF",txdataF[0],lte_frame_parms->ofdm_symbol_size,1,1);
#ifdef IFFT_FPGA
	if (n_frames==1) {
	  write_output("txsigF0.m","txsF0", PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0],300*120,1,4);
	  if (nb_tx > 1)
	    write_output("txsigF1.m","txsF1", PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][1],300*120,1,4);
	}

	// do talbe lookup and write results to txdataF2
	for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx;aa++) {
	  l = 0;
	  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX;i++) 
	    if ((i%512>=1) && (i%512<=150))
	      txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][l++]];
	    else if (i%512>=362)
	      txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][l++]];
	    else 
	      txdataF2[aa][i] = 0;
	  //printf("l=%d\n",l);
	}
	if (n_frames==1) {
	  write_output("txsigF20.m","txsF20", txdataF2[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	//write_output("txsigF21.m","txsF21", txdataF2[1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	}
	  
	tx_lev=0;
	  
	for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
	    
	  if (frame_parms->Ncp == 1) 
	    PHY_ofdm_mod(&txdataF2[aa][subframe*nsymb*PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size],        // input
			 &txdata[aa][subframe*PHY_vars_eNb->lte_frame_parms.samples_per_tti],         // output
			 frame_parms->log2_symbol_size,                // log2_fft_size
			 2*nsymb,                 // number of symbols
			 frame_parms->nb_prefix_samples,               // number of prefix samples
			 frame_parms->twiddle_ifft,  // IFFT twiddle factors
			 frame_parms->rev,           // bit-reversal permutation
			 CYCLIC_PREFIX);
	  else {
	    normal_prefix_mod(&txdataF2[aa][subframe*nsymb*PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size],
			      &txdata[aa][subframe*PHY_vars_eNb->lte_frame_parms.samples_per_tti],2*nsymb,frame_parms);
	  }
	  tx_lev += signal_energy(&txdata[aa][subframe*PHY_vars_eNb->lte_frame_parms.samples_per_tti],
				  frame_parms->ofdm_symbol_size);
	}	
#else
	if (n_frames==1)
	  write_output("txsigF0.m","txsF0", PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0],2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	//write_output("txsigF1.m","txsF1", lte_eNB_common_vars->txdataF[1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	  
	tx_lev = 0;
	  
	  
	  
	for (aa=0; aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx; aa++) {
	  if (frame_parms->Ncp == 1) 
	    PHY_ofdm_mod(&PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][subframe*nsymb*PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size],        // input,
			 &txdata[aa][subframe*PHY_vars_eNb->lte_frame_parms.samples_per_tti],         // output
			 frame_parms->log2_symbol_size,                // log2_fft_size
			 2*nsymb,                 // number of symbols
			 frame_parms->nb_prefix_samples,               // number of prefix samples
			 frame_parms->twiddle_ifft,  // IFFT twiddle factors
			 frame_parms->rev,           // bit-reversal permutation
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
#endif
	
	
	tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
      }

	
	
      for (i=0;i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
	for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx;aa++) {
	  if (awgn_flag == 0) {
	    s_re[aa][i] = ((double)(((short *)txdata[aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) + (i<<1)]);
	    s_im[aa][i] = ((double)(((short *)txdata[aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) + (i<<1)+1]);
	  }
	  else {
	    r_re[aa][i] = ((double)(((short *)txdata[aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) + (i<<1)]);
	    r_im[aa][i] = ((double)(((short *)txdata[aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) + (i<<1)+1]);
	  }
	}
      }
	
	
	
      if (awgn_flag == 0) {	
	multipath_channel(eNB2UE,s_re,s_im,r_re,r_im,
			  2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
	  
      }
      //write_output("channel0.m","chan0",ch[0],channel_length,1,8);
	
      // scale by path_loss = NOW - P_noise
      //sigma2       = pow(10,sigma2_dB/10);
      //N0W          = -95.87;
      sigma2_dB = (double)tx_lev_dB +10*log10(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size/300) - SNR;
      if (n_frames==1)
	printf("sigma2_dB %f (SNR %f dB) tx_lev_dB %d\n",sigma2_dB,SNR,tx_lev_dB);
      //AWGN
      sigma2 = pow(10,sigma2_dB/10);
      //	printf("Sigma2 %f (sigma2_dB %f)\n",sigma2,sigma2_dB);
      for (i=0; i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES; i++) {
	for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_rx;aa++) {
	  ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) + 2*i] = (short) (.667*(r_re[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	  ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) + 2*i+1] = (short) (.667*(r_im[aa][i] + (iqim*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	}
      }    

      // UE receiver	
      for (l=0;l<PHY_vars_eNb->lte_frame_parms.symbols_per_tti;l++) {
	  
	subframe_offset = (l/PHY_vars_eNb->lte_frame_parms.symbols_per_tti)*PHY_vars_eNb->lte_frame_parms.samples_per_tti;
	//	    printf("subframe_offset = %d\n",subframe_offset);
	  
	slot_fep(PHY_vars_UE,
		 l%(PHY_vars_eNb->lte_frame_parms.symbols_per_tti/2),
		 (2*subframe)+(l/(PHY_vars_eNb->lte_frame_parms.symbols_per_tti/2)),
		 0,
		 0);
	  
	if (l==((PHY_vars_eNb->lte_frame_parms.Ncp==0)?4:3)) {
	    
	  //	    write_output("H00.m","h00",&(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][0][0]),((frame_parms->Ncp==0)?7:6)*(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size),1,1);

	  // do PDCCH procedures here
	  PHY_vars_UE->lte_ue_pdcch_vars[0]->crnti = n_rnti;
	    
	  //	  printf("Doing RX : num_pdcch_symbols at TX %d\n",num_pdcch_symbols);
	  rx_pdcch(&PHY_vars_UE->lte_ue_common_vars,
		   PHY_vars_UE->lte_ue_pdcch_vars,
		   &PHY_vars_UE->lte_frame_parms,
		   subframe,
		   0,
		   (PHY_vars_UE->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI,
		   PHY_vars_UE->is_secondary_ue); 
	  //	  if (PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols != num_pdcch_symbols)
	  //	    break;
	  dci_cnt = dci_decoding_procedure(PHY_vars_UE,
					   dci_alloc_rx,
					   0,subframe,
					   SI_RNTI,RA_RNTI);
	  common_rx=0;
	  ul_rx=0;
	  dl_rx=0;
	  for (i=0;i<dci_cnt;i++) {
	    if (dci_alloc_rx[i].rnti == SI_RNTI)
	      common_rx=1;
	    if ((dci_alloc_rx[i].rnti == n_rnti) && (dci_alloc_rx[i].format == format0))
	      ul_rx=1;
	    if ((dci_alloc_rx[i].rnti == n_rnti) && ((dci_alloc_rx[i].format == format))) {
	      if (n_frames==1)
		dump_dci(&PHY_vars_UE->lte_frame_parms,&dci_alloc_rx[i]);
	      dl_rx=1;		       
	    }

	    if ((dci_alloc_rx[i].rnti != n_rnti) && (dci_alloc_rx[i].rnti != SI_RNTI))
	      false_detection_cnt++;
	  }
	  if ((common_rx==0)&&(common_active==1))
	    n_errors_common++;
	  if ((ul_rx==0)&&(ul_active==1)) {
	    n_errors_ul++;
	    //     exit(-1);
	  }
	  if ((dl_rx==0)&&(dl_active==1)) {
	    n_errors_dl++;
	    //   exit(-1);
	  }
	  if (PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols != num_pdcch_symbols)
	    n_errors_cfi++;

	  if (n_errors_cfi > 0)
	    break;
	}

      } // symbol loop

      if (n_errors_cfi > 0)
	break;
      if ((n_errors_ul>100) && (n_errors_dl>100) && (n_errors_common>100))
	break;

    } //trials

    printf("SNR %f : n_errors_common = %d/%d (%e)\n", SNR,n_errors_common,n_trials_common,(double)n_errors_common/n_trials_common);
    printf("SNR %f : n_errors_ul = %d/%d (%e)\n", SNR,n_errors_ul,n_trials_ul,(double)n_errors_ul/n_trials_ul);
    printf("SNR %f : n_errors_dl = %d/%d (%e)\n", SNR,n_errors_dl,n_trials_dl,(double)n_errors_dl/n_trials_dl);
    printf("SNR %f : n_errors_cfi = %d/%d (%e)\n", SNR,n_errors_cfi,trial,(double)n_errors_cfi/trial);
    
  } // NSR
  
  if (n_frames==1) {
    write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    if (n_tx>1)
      write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("rxsig0.m","rxs0", PHY_vars_UE->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);
    write_output("rxsigF0.m","rxsF0", PHY_vars_UE->lte_ue_common_vars.rxdataF[0],NUMBER_OF_OFDM_CARRIERS*2*((frame_parms->Ncp==0)?14:12),2,1);   
    write_output("H00.m","h00",&(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][0][0]),((frame_parms->Ncp==0)?7:6)*(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size),1,1);
    if (n_tx==2)
      write_output("H10.m","h10",&(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][2][0]),((frame_parms->Ncp==0)?7:6)*(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size),1,1);
    write_output("pdcch_rxF_ext0.m","pdcch_rxF_ext0",PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->rxdataF_ext[0],3*300,1,1); 
    write_output("pdcch_rxF_comp0.m","pdcch0_rxF_comp0",PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->rxdataF_comp[0],4*300,1,1);
    write_output("pdcch_rxF_llr.m","pdcch_llr",PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->llr,2400,1,4);    
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
  
  lte_sync_time_free();
  
  return(n_errors_ul);
  
}
   


/*  
    for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
    (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
    12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/

