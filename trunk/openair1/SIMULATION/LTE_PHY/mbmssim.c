#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

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
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "LAYER2/MAC/vars.h"

#ifdef XFORMS
#include <forms.h>
#include "../../USERSPACE_TOOLS/SCOPE/lte_scope.h"
#endif //XFORMS


#include "OCG_vars.h"

#define BW 5.0


PHY_VARS_eNB *PHY_vars_eNB;
PHY_VARS_UE *PHY_vars_UE;

void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,u8 extended_prefix_flag,u8 fdd_flag, u16 Nid_cell,u8 tdd_config,u8 N_RB_DL,u8 osf) {

  LTE_DL_FRAME_PARMS *lte_frame_parms;
  int i;


  printf("Start lte_param_init\n");
  PHY_vars_eNB = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  //PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  srand(1);
  randominit(1);
  set_taus_seed(1);
  
  lte_frame_parms = &(PHY_vars_eNB->lte_frame_parms);

  lte_frame_parms->N_RB_DL            = N_RB_DL;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->N_RB_UL            = N_RB_DL;   
  lte_frame_parms->Ncp                = extended_prefix_flag;
  lte_frame_parms->Nid_cell           = Nid_cell;
  lte_frame_parms->nushift            = Nid_cell%6;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  lte_frame_parms->phich_config_common.phich_resource         = oneSixth;
  lte_frame_parms->tdd_config         = tdd_config;
  lte_frame_parms->frame_type         = (fdd_flag==1)?0 : 1;
  //  lte_frame_parms->Csrs = 2;
  //  lte_frame_parms->Bsrs = 0;
  //  lte_frame_parms->kTC = 0;44
  //  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->mode1_flag = (transmission_mode == 1)? 1 : 0;

  init_frame_parms(lte_frame_parms,osf);
  
  //copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(lte_frame_parms); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  PHY_vars_UE->is_secondary_ue = 0;
  PHY_vars_UE->lte_frame_parms = *lte_frame_parms;
  PHY_vars_eNB->lte_frame_parms = *lte_frame_parms;

  phy_init_lte_top(lte_frame_parms);
  dump_frame_parms(lte_frame_parms);

  PHY_vars_UE->PHY_measurements.n_adj_cells=2;
  PHY_vars_UE->PHY_measurements.adj_cell_id[0] = Nid_cell+1;
  PHY_vars_UE->PHY_measurements.adj_cell_id[1] = Nid_cell+2;

  lte_gold_mbsfn(lte_frame_parms,PHY_vars_UE->lte_gold_mbsfn_table,Nid_cell);    
  lte_gold_mbsfn(lte_frame_parms,PHY_vars_eNB->lte_gold_mbsfn_table,Nid_cell);    

  phy_init_lte_ue(PHY_vars_UE,0,0);
  phy_init_lte_eNB(PHY_vars_eNB,0,0,0);

  
  printf("Done lte_param_init\n");


}
DCI1E_5MHz_2A_M10PRB_TDD_t  DLSCH_alloc_pdu2_1E[2];
#define UL_RB_ALLOC 0x1ff;
#define CCCH_RB_ALLOC computeRIV(PHY_vars_eNB->lte_frame_parms.N_RB_UL,0,2)
int main(int argc, char **argv) {

  char c;


  int s,Kr,Kr_bytes;

  int i,k,l,aa,aarx, aatx;
  double sigma2, sigma2_dB=0,SNR,snr0=-2.0,snr1,rate,saving_bler=1;
  u8 snr1set=0;
  double snr_step=1,input_snr_step=1, snr_int=30;
  //mod_sym_t **txdataF;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  int **txdata,**txdata1,**txdata2;
  double **s_re,**s_im,**s_re1,**s_im1,**s_re2,**s_im2,**r_re,**r_im,**r_re1,**r_im1,**r_re2,**r_im2;
  double iqim = 0.0;
  unsigned char pbch_pdu[6];
  int sync_pos, sync_pos_slot;
  FILE *rx_frame_file,*output_fd;
  u8 write_output_file=0;
  int result;
  int freq_offset;
  int subframe=1,subframe_offset; // Valid Subframe FDD - 1,2,3,6,7,8.  TDD - 3,4,7,8,9 ;
  char fname[40], vname[40];
  int trial, n_trials, ntrials=1, n_errors,n_errors2,n_alamouti;
  u8 transmission_mode = 1,n_tx=1,n_rx=1;
  u16 Nid_cell=0;


  int eNB_id = 0, eNB_id_i = NUMBER_OF_eNB_MAX;
  unsigned char mcs,dual_stream_UE = 0,awgn_flag=0,round,dci_flag=0;
  unsigned char i_mod = 2;
  unsigned char Ns,m;
  u16 n_rnti=0x1234;
  int n_users = 1;
  
  int n_frames=1;
  channel_desc_t *eNB2UE;
  u32 nsymb,tx_lev,tx_lev1,tx_lev2,tx_lev_dB;
  u8 extended_prefix_flag=1;
  LTE_DL_FRAME_PARMS *frame_parms;
    
#ifdef EMOS
  fifo_dump_emos emos_dump;
#endif
  u32 DLSCH_RB_ALLOC = 0x1fff;
  
  FILE *input_fd=NULL,*pbch_file_fd=NULL;
  char input_val_str[50],input_val_str2[50];
  double input_val1,input_val2;
  u16 amask=0;
  u8 frame_mod4,num_pdcch_symbols;
  u16 NB_RB=25;
  int tdd_config=3;
  
  u8 num_pdcch_symbols_2=0; //num_pdcch_symbols=3,
  
  SCM_t channel_model=AWGN;//Rayleigh1_anticorr;


  unsigned char *input_buffer[2];
  unsigned short input_buffer_length;
  unsigned int ret;
  unsigned int coded_bits_per_codeword,dci_cnt,tbs;
 
  unsigned int trials,errs[4]={0,0,0,0},round_trials[4]={0,0,0,0},dci_errors=0,dlsch_active=0,num_layers;
  int re_allocated;
  FILE *bler_fd;
  char bler_fname[256];
  FILE *tikz_fd;
  char tikz_fname[256];
  
  
  FILE *input_trch_fd;
  unsigned char input_trch_file=0;
  unsigned char input_file=0;
  char input_trch_val[16];
  double pilot_sinr, abs_channel,channelx,channely;

  u8 pilot1,pilot2,pilot3;
  
  //DCI_ALLOC_t dci_alloc[8];
  u8 abstraction_flag=0,calibration_flag=0;
  double pbch_sinr;
  int pbch_tx_ant;
  u8 N_RB_DL=25,osf=1;
  
  
  DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];
  int num_common_dci=0,num_ue_spec_dci=0,num_dci=0;
   //int len;
 
  int n_ch_rlz = 1;
  //channel_desc_t *eNB2UE[4];
 // u8 num_pdcch_symbols=3,num_pdcch_symbols_2=0;
 
 // u8 rx_sample_offset = 0;
  
  u8 num_rounds = 4,fix_rounds=0;
  int u;
  int abstx=0;
  int iii;
  FILE *csv_fd;
  char csv_fname[512];
  int ch_realization;
  int pmi_feedback=0;
  int hold_channel=0; 
  int hold_channel1=0; 
  // void *data;
  // int ii;
  // int bler;
  double blerr[4],uncoded_ber,avg_ber;
  short *uncoded_ber_bit;
  

  int openair_fd,rx_sig_fifo_fd,get_frame=0;
  int frequency=0,fc=0;
  unsigned char frame_type = 0; // Frame Type: 0 - FDD, 1 - TDD;
  unsigned char pbch_phase = 0;

#ifdef XFORMS
  FD_lte_scope *form_dl;
  char title[255];

  fl_initialize (&argc, argv, NULL, 0, 0);
  form_dl = create_form_lte_scope();
  sprintf (title, "LTE DL SCOPE UE");
  fl_show_form (form_dl->lte_scope, FL_PLACE_HOTSPOT, FL_FULLBORDER, title);
#endif

  logInit();
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
  while ((c = getopt (argc, argv, "hA:Cpf:g:n:s:S:t:x:y:z:N:F:GR:O:dP:")) != -1)
    {
      switch (c)
	{
	case 'f':
	  output_fd = fopen(optarg,"w");
	  write_output_file=1;
	  break;
	case 'd':
	  frame_type = 1;
	  break;
	case 'G':
	  if ((openair_fd = open("/dev/openair0", O_RDWR,0)) <0) {
	    fprintf(stderr,"Error %d opening /dev/openair0\n",openair_fd);
	    exit(-1);
	  }
	  if ((rx_sig_fifo_fd = open("/dev/rtf59",O_RDONLY,0)) <0) {
	    printf("[openair][INFO] Cannot open rx_sig_fifo\n");
	    exit(-1);
	  }
	  get_frame = 1;
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

	case 'n':
	  n_frames = atoi(optarg);
	  break;
	case 's':
	  snr0 = atof(optarg);
	  msg("Setting SNR0 to %f\n",snr0);
	  break;
	case 'S':
	  snr1 = atof(optarg);
	  snr1set=1;
	  msg("Setting SNR1 to %f\n",snr1);
	  break;
	  /*
	case 't':
	  Td= atof(optarg);
	  break;
	  */
	case 'p': // subframe no;
	  subframe=atoi(optarg);
	  break;
	  /*
	case 'r':
	  ricean_factor = pow(10,-.1*atof(optarg));
	  if (ricean_factor>1) {
	    printf("Ricean factor must be between 0 and 1\n");
	    exit(-1);
	  }
	  break;
	  */
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
	case 'A':
	  abstraction_flag=1;
	  ntrials=10000;
	  msg("Running Abstraction test\n");
	  pbch_file_fd=fopen(optarg,"r");
	  if (pbch_file_fd==NULL) {
	    printf("Problem with filename %s\n",optarg);
	    exit(-1);
	  }
	  break;
	case 'C':
	  calibration_flag=1;
	  msg("Running Abstraction calibration for Bias removal\n");
	  break;
	case 'N':
	  Nid_cell = atoi(optarg);
	  break;
	case 'R':
	  N_RB_DL = atoi(optarg);
	  break;
	case 'O':
	  osf = atoi(optarg);
	  break;
	case 'F':
	  input_fd = fopen(optarg,"r");
	  if (input_fd==NULL) {
	    printf("Problem with filename %s\n",optarg);
	    exit(-1);
	  }
	  break;
	case 'P':
	  pbch_phase = atoi(optarg);
	  if (pbch_phase>3)
	    printf("Illegal PBCH phase (0-3) got %d\n",pbch_phase);
	  break;
	default:
	case 'h':
	  printf("%s -h(elp) -p(extended_prefix) -N cell_id -f output_filename -F input_filename -g channel_model -n n_frames -t Delayspread -s snr0 -S snr1 -x transmission_mode -y TXant -z RXant -i Intefrence0 -j Interference1 -A interpolation_file -C(alibration offset dB) -N CellId\n",argv[0]);
	  printf("-h This message\n");
	  printf("-p Use extended prefix mode\n");
	  printf("-d Use TDD\n");
	  printf("-n Number of frames to simulate\n");
	  printf("-s Starting SNR, runs from SNR0 to SNR0 + 5 dB.  If n_frames is 1 then just SNR is simulated\n");
	  printf("-S Ending SNR, runs from SNR0 to SNR1\n");
	  printf("-t Delay spread for multipath channel\n");
	  printf("-g [A,B,C,D,E,F,G] Use 3GPP SCM (A,B,C,D) or 36-101 (E-EPA,F-EVA,G-ETU) models (ignores delay spread and Ricean factor)\n");
	  printf("-x Transmission mode (1,2,6 for the moment)\n");
	  printf("-y Number of TX antennas used in eNB\n");
	  printf("-z Number of RX antennas used in UE\n");
	  printf("-i Relative strength of first intefering eNB (in dB) - cell_id mod 3 = 1\n");
	  printf("-j Relative strength of second intefering eNB (in dB) - cell_id mod 3 = 2\n");
	  printf("-N Nid_cell\n");
	  printf("-R N_RB_DL\n");
	  printf("-O oversampling factor (1,2,4,8,16)\n");
	  printf("-A Interpolation_filname Run with Abstraction to generate Scatter plot using interpolation polynomial in file\n");
	  printf("-C Generate Calibration information for Abstraction (effective SNR adjustment to remove Pe bias w.r.t. AWGN)\n");
	  printf("-f Output filename (.txt format) for Pe/SNR results\n");
	  printf("-F Input filename (.txt format) for RX conformance testing\n");
	  exit (-1);
	  break;
	}
    }

  // check that subframe is legal for eMBMS

 if ((subframe == 0) || (subframe == 5) ||    // TDD and FDD SFn 0,5;
      ((frame_type == 0) && ((subframe == 4) || (subframe == 9))) || // FDD SFn 4,9;
	  ((frame_type == 1 ) && ((subframe<3) || (subframe==6)))) 	  {  // TDD SFn 1,2,6;
	  
	printf("Illegal subframe %d for eMBMS transmission (frame_type %d)\n",subframe,frame_type);
	exit(-1);
  } 
  if (transmission_mode==2)
    n_tx=2;

  lte_param_init(n_tx,n_rx,transmission_mode,extended_prefix_flag,frame_type,Nid_cell,tdd_config,N_RB_DL,osf);



  if (snr1set==0) {
    if (n_frames==1)
      snr1 = snr0+.1;
    else
      snr1 = snr0+5.0;
  }

  printf("SNR0 %f, SNR1 %f\n",snr0,snr1);

  frame_parms = &PHY_vars_eNB->lte_frame_parms;


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
  txdata = PHY_vars_eNB->lte_eNB_common_vars.txdata[0];

#endif
  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  s_re1 = malloc(2*sizeof(double*));
  s_im1 = malloc(2*sizeof(double*));
  s_re2 = malloc(2*sizeof(double*));
  s_im2 = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
  r_re1 = malloc(2*sizeof(double*));
  r_im1 = malloc(2*sizeof(double*));
  r_re2 = malloc(2*sizeof(double*));
  r_im2 = malloc(2*sizeof(double*));

  nsymb = 12;

  printf("FFT Size %d, Extended Prefix %d, Samples per subframe %d, Symbols per subframe %d\n",NUMBER_OF_OFDM_CARRIERS,
	 frame_parms->Ncp,frame_parms->samples_per_tti,nsymb);

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
  
  PHY_vars_UE->lte_ue_pdcch_vars[0]->crnti = n_rnti;

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

  DLSCH_alloc_pdu2_1E[0].rah              = 0;
  DLSCH_alloc_pdu2_1E[0].rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2_1E[0].TPC              = 0;
  DLSCH_alloc_pdu2_1E[0].dai              = 0;
  DLSCH_alloc_pdu2_1E[0].harq_pid         = 0;
  //DLSCH_alloc_pdu2_1E[0].tb_swap          = 0;
  DLSCH_alloc_pdu2_1E[0].mcs             = mcs;  
  DLSCH_alloc_pdu2_1E[0].ndi             = 1;
  DLSCH_alloc_pdu2_1E[0].rv              = 0;
  // Forget second codeword
  DLSCH_alloc_pdu2_1E[0].tpmi             = (transmission_mode>=5 ? 5 : 0);  // precoding
  DLSCH_alloc_pdu2_1E[0].dl_power_off     = (transmission_mode==5 ? 0 : 1);

  DLSCH_alloc_pdu2_1E[1].rah              = 0;
  DLSCH_alloc_pdu2_1E[1].rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2_1E[1].TPC              = 0;
  DLSCH_alloc_pdu2_1E[1].dai              = 0;
  DLSCH_alloc_pdu2_1E[1].harq_pid         = 0;
  //DLSCH_alloc_pdu2_1E[1].tb_swap          = 0;
  DLSCH_alloc_pdu2_1E[1].mcs             = mcs;  
  DLSCH_alloc_pdu2_1E[1].ndi             = 1;
  DLSCH_alloc_pdu2_1E[1].rv              = 0;
  // Forget second codeword
  DLSCH_alloc_pdu2_1E[1].tpmi             = (transmission_mode>=5 ? 5 : 0) ;  // precoding
  DLSCH_alloc_pdu2_1E[1].dl_power_off     = (transmission_mode==5 ? 0 : 1);

  eNB2UE = new_channel_desc_scm(PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,
				PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				channel_model,
				BW,
				0,
				0,
				0);
				
  for (k=0;k<n_users;k++) {
    // Create transport channel structures for 2 transport blocks (MIMO)
    for (i=0;i<2;i++) {
      PHY_vars_eNB->dlsch_eNB[k][i] = new_eNB_dlsch(1,8,0);
      
      if (!PHY_vars_eNB->dlsch_eNB[k][i]) {
	printf("Can't get eNB dlsch structures\n");
	exit(-1);
      }
      
      PHY_vars_eNB->dlsch_eNB[k][i]->rnti = n_rnti+k;
    }
  }

  for (i=0;i<2;i++) {
    PHY_vars_UE->dlsch_ue[0][i]  = new_ue_dlsch(1,8,0);
    if (!PHY_vars_UE->dlsch_ue[0][i]) {
      printf("Can't get ue dlsch structures\n");
      exit(-1);
    }    
    PHY_vars_UE->dlsch_ue[0][i]->rnti   = n_rnti;
  }
  
  if (DLSCH_alloc_pdu2_1E[0].tpmi == 5) {

    PHY_vars_eNB->eNB_UE_stats[0].DL_pmi_single = (unsigned short)(taus()&0xffff);
    if (n_users>1)
      PHY_vars_eNB->eNB_UE_stats[1].DL_pmi_single = (PHY_vars_eNB->eNB_UE_stats[0].DL_pmi_single ^ 0x1555); //opposite PMI 
  }
  else {
    PHY_vars_eNB->eNB_UE_stats[0].DL_pmi_single = 0;
    if (n_users>1)
      PHY_vars_eNB->eNB_UE_stats[1].DL_pmi_single = 0;
  }


  if (input_fd==NULL) {

    for(k=0;k<n_users;k++) {
      printf("Generating dlsch params for user %d\n",k);
      generate_eNB_dlsch_params_from_dci(0,
					 &DLSCH_alloc_pdu2_1E[k],
					 n_rnti+k,
					 format1E_2A_M10PRB,
					 PHY_vars_eNB->dlsch_eNB[k],
					 &PHY_vars_eNB->lte_frame_parms,
					 PHY_vars_eNB->pdsch_config_dedicated,
					 SI_RNTI,
					 0,
					 P_RNTI,
					 PHY_vars_eNB->eNB_UE_stats[k].DL_pmi_single);
    }

    num_dci = 0;
    num_ue_spec_dci = 0;
    num_common_dci = 0;

   
    // common DCI 
    memcpy(&dci_alloc[num_dci].dci_pdu[0],&CCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
    dci_alloc[num_dci].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
    dci_alloc[num_dci].L          = 2;
    dci_alloc[num_dci].rnti       = SI_RNTI;
    num_dci++;
    num_common_dci++;
    

    // UE specific DCI
 /*   for(k=0;k<n_users;k++) {
      memcpy(&dci_alloc[num_dci].dci_pdu[0],&DLSCH_alloc_pdu2_1E[k],sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t));
      dci_alloc[num_dci].dci_length = sizeof_DCI1E_5MHz_2A_M10PRB_TDD_t;
      dci_alloc[num_dci].L          = 2;
      dci_alloc[num_dci].rnti       = n_rnti+k;
      dci_alloc[num_dci].format     = format1E_2A_M10PRB;
      dci_alloc[num_dci].nCCE       = 4*k;
      dump_dci(&PHY_vars_eNB->lte_frame_parms,&dci_alloc[num_dci]);

      num_dci++;
      num_ue_spec_dci++;

      
	memcpy(&dci_alloc[1].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
	dci_alloc[1].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
	dci_alloc[1].L          = 2;
	dci_alloc[1].rnti       = n_rnti;
      
    } */

    for (k=0;k<n_users;k++) {

      input_buffer_length = PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->TBS/8;
      input_buffer[k] = (unsigned char *)malloc(input_buffer_length+4);
      memset(input_buffer[k],0,input_buffer_length+4);
    
      if (input_trch_file==0) {
	for (i=0;i<input_buffer_length;i++) {
	  input_buffer[k][i]= (unsigned char)(taus()&0xff);
	}
      }
      
      else {
	i=0;
	while ((!feof(input_trch_fd)) && (i<input_buffer_length<<3)) {
	  fscanf(input_trch_fd,"%s",input_trch_val);
	  if (input_trch_val[0] == '1')
	    input_buffer[k][i>>3]+=(1<<(7-(i&7)));
	  if (i<16)
	    printf("input_trch_val %d : %c\n",i,input_trch_val[0]);
	  i++;
	  if (((i%8) == 0) && (i<17))
	    printf("%x\n",input_buffer[k][(i-1)>>3]);
	}
	printf("Read in %d bits\n",i);
      }
    }
   }  
  snr_step = input_snr_step;
  for (ch_realization=0;ch_realization<n_ch_rlz;ch_realization++){
    if(abstx){
      printf("**********************Channel Realization Index = %d **************************\n", ch_realization);
       saving_bler=0;
       hold_channel1=0;
    }
    
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
      avg_ber = 0;

      round=0;

      
      for (trials = 0;trials<n_frames;trials++) {
	//  printf("Trial %d\n",trials);
	fflush(stdout);
	round=0;

	//if (trials%100==0)
	//eNB2UE[0]->first_run = 1;
	eNB2UE->first_run = 1;

	while (round < num_rounds) {
	  round_trials[round]++;

	  if(transmission_mode>=5)
	    pmi_feedback=1;
	  else 
	    pmi_feedback=0;
	  
	  if (abstx) {
	    if (trials==0 && round==0 && SNR==snr0)  //generate a new channel
	      hold_channel = 0;
	    else
	      hold_channel = 1;
	  }
	  else
	    hold_channel = 0;

	PMI_FEEDBACK:
	
	  //  printf("Trial %d : Round %d, pmi_feedback %d \n",trials,round,pmi_feedback);
	  for (aa=0; aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx;aa++) {
#ifdef IFFT_FPGA
	    memset(&PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id][aa][0],0,NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_SYMBOLS_PER_FRAME*sizeof(mod_sym_t));
#else
	    memset(&PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id][aa][0],0,FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
#endif
	  }
	
	  if (input_fd==NULL) {

	    // Simulate HARQ procedures!!!
	    //if (round == 0) {   // First round, set Ndi to 1 and rv to floor(round/2)
	      //PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->Ndi = 1;
	      //PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->rvidx = round&3;
	      //DLSCH_alloc_pdu2_1E[0].ndi             = 1;
	      //DLSCH_alloc_pdu2_1E[0].rv              = 0;
	      //memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2_1E[0],sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t));
	   // }
	    //else { // set Ndi to 0
	      //PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->Ndi = 0;
	      //PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->rvidx = round&3;
	      //DLSCH_alloc_pdu2_1E[0].ndi             = 0;
	      //DLSCH_alloc_pdu2_1E[0].rv              = round&3;
	      //memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2_1E[0],sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t));
	    // }
	    
	    num_pdcch_symbols_2 = generate_dci_top(num_ue_spec_dci,
						   num_common_dci,
						   dci_alloc,
						   0,
						   1024,
						   &PHY_vars_eNB->lte_frame_parms,
						   PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id],
						   subframe);

	    if (num_pdcch_symbols_2 > num_pdcch_symbols) {
	      msg("Error: given num_pdcch_symbols not big enough\n");
	      exit(-1);
	    }

	    for (k=0;k<n_users;k++) {
	      coded_bits_per_codeword = get_G(&PHY_vars_eNB->lte_frame_parms,
					      PHY_vars_eNB->dlsch_eNB[k][0]->nb_rb,
					      PHY_vars_eNB->dlsch_eNB[k][0]->rb_alloc,
					      get_Qm(PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->mcs),
					      num_pdcch_symbols,
					      subframe);
	  /*    
#ifdef TBS_FIX   // This is for MESH operation!!!
	      tbs = (double)3*dlsch_tbs25[get_I_TBS(PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->mcs)][PHY_vars_eNB->dlsch_eNB[k][0]->nb_rb-1]/4;
#else
	      tbs = (double)dlsch_tbs25[get_I_TBS(PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->mcs)][PHY_vars_eNB->dlsch_eNB[k][0]->nb_rb-1];
#endif          */
	      
	      rate = (double)tbs/(double)coded_bits_per_codeword;

	      uncoded_ber_bit = (short*) malloc(2*coded_bits_per_codeword);
	      
	      if (trials==0 && round==0) 
		printf("Rate = %f (%f bits/dim) (G %d, TBS %d, mod %d, pdcch_sym %d)\n",
		       rate,rate*get_Qm(PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->mcs),
		       coded_bits_per_codeword,
		       tbs,
		       get_Qm(PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->mcs),
		       num_pdcch_symbols);
	      
	      /*
	      // generate channel here
	      random_channel(eNB2UE);
	      // generate frequency response
	      freq_channel(eNB2UE,NB_RB);
	      // generate PMI from channel
	     
	      */
		
	      // use the PMI from previous trial
	      if (DLSCH_alloc_pdu2_1E[0].tpmi == 5) {
		PHY_vars_eNB->dlsch_eNB[0][0]->pmi_alloc = quantize_subband_pmi(&PHY_vars_UE->PHY_measurements,0);
		PHY_vars_UE->dlsch_ue[0][0]->pmi_alloc = quantize_subband_pmi(&PHY_vars_UE->PHY_measurements,0);
		if (n_users>1) 
		  PHY_vars_eNB->dlsch_eNB[1][0]->pmi_alloc = (PHY_vars_eNB->dlsch_eNB[0][0]->pmi_alloc ^ 0x1555); 
		/*
		if ((trials<10) && (round==0)) {
		  printf("tx PMI UE0 %x (pmi_feedback %d)\n",pmi2hex_2Ar1(PHY_vars_eNB->dlsch_eNB[0][0]->pmi_alloc),pmi_feedback);
		  if (transmission_mode ==5)
		    printf("tx PMI UE1 %x\n",pmi2hex_2Ar1(PHY_vars_eNB->dlsch_eNB[1][0]->pmi_alloc));
		}
		*/		
	     } 
		 
	      
	      if (dlsch_encoding(input_buffer[k],
				 &PHY_vars_eNB->lte_frame_parms,
				 num_pdcch_symbols,
				 PHY_vars_eNB->dlsch_eNB[k][0],
				 subframe,
				 &PHY_vars_eNB->dlsch_rate_matching_stats,
				 &PHY_vars_eNB->dlsch_turbo_encoding_stats,
				 &PHY_vars_eNB->dlsch_interleaving_stats)<0)
		exit(-1);
	      
	      // printf("Did not Crash here 1\n");
	      PHY_vars_eNB->dlsch_eNB[k][0]->rnti = n_rnti+k;	  
	      dlsch_scrambling(&PHY_vars_eNB->lte_frame_parms,
			       num_pdcch_symbols,
			       PHY_vars_eNB->dlsch_eNB[k][0],
			       coded_bits_per_codeword,
			       0,
			       subframe<<1);
	   /*   if (n_frames==1) {
		for (s=0;s<PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->C;s++) {
		  if (s<PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->Cminus)
		    Kr = PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->Kminus;
		  else
		    Kr = PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->Kplus;
	      
		  Kr_bytes = Kr>>3;
	      
		  for (i=0;i<Kr_bytes;i++)
		    printf("%d : (%x)\n",i,PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->c[s][i]);
		}
	      } */
	      // printf("Did not Crash here 2\n");
	  
	      //	      if (k==1)

	      re_allocated = dlsch_modulation(PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id],
					      AMP,
					      subframe,
					      &PHY_vars_eNB->lte_frame_parms,
					      num_pdcch_symbols,
					      PHY_vars_eNB->dlsch_eNB[k][0]);

	      // printf("Did not Crash here 3\n");
	      if (trials==0 && round==0)
		printf("RE count %d\n",re_allocated);
	  
	      if (num_layers>1)
		  re_allocated = dlsch_modulation(PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id],
						1024,
						subframe,
						&PHY_vars_eNB->lte_frame_parms,
						num_pdcch_symbols,
						PHY_vars_eNB->dlsch_eNB[k][1]);
	     } //n_users

	    
  generate_mbsfn_pilot(PHY_vars_eNB,
			PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
			AMP,
			subframe);

  
  PHY_ofdm_mod(PHY_vars_eNB->lte_eNB_common_vars.txdataF[0][0],        // input,
	       txdata[0],         // output
	       frame_parms->log2_symbol_size,                // log2_fft_size
	       LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,                 // number of symbols
	       frame_parms->nb_prefix_samples,               // number of prefix samples
	       frame_parms->twiddle_ifft,  // IFFT twiddle factors
	       frame_parms->rev,           // bit-reversal permutation
	       CYCLIC_PREFIX);
  
  write_output("txsigF0.m","txsF0", PHY_vars_eNB->lte_eNB_common_vars.txdataF[0][0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
  write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);

#ifdef IFFT_FPGA
	    if (n_frames==1) {
	      write_output("txsigF0.m","txsF0", &PHY_vars_eNB->lte_eNB_common_vars.txdataF[0][0][subframe*nsymb*300],300*nsymb,1,4);
	      //if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
		//write_output("txsigF1.m","txsF1", &PHY_vars_eNB->lte_eNB_common_vars.txdataF[0][1][subframe*nsymb*300],300*nsymb,1,4);
	      //write_output("txsigF20.m","txsF20", txdataF2[0], FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	      //if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
		//write_output("txsigF21.m","txsF21", txdataF2[1], FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	    }
#else //IFFT_FPGA
	    if (n_frames==1) {
	      write_output("txsigF0.m","txsF0", &PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id][0][subframe*nsymb*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size],nsymb*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size,1,1);
	      //if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
		//write_output("txsigF1.m","txsF1", &PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id][1][subframe*nsymb*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size],nsymb*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size,1,1);
	    }
#endif


tx_lev = 0;
	    for (aa=0; aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx; aa++) {
	      tx_lev += signal_energy(&PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id][aa]
				      [subframe*PHY_vars_eNB->lte_frame_parms.samples_per_tti],
				      PHY_vars_eNB->lte_frame_parms.samples_per_tti);
	    }
	    tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
	  
	    if (n_frames==1) {
	      printf("tx_lev = %d (%d dB)\n",tx_lev,tx_lev_dB);
	  //    write_output("txsig0.m","txs0", &PHY_vars_eNB->lte_eNB_common_vars.txdata[0][0][subframe* PHY_vars_eNB->lte_frame_parms.samples_per_tti],
			   
		//	   PHY_vars_eNB->lte_frame_parms.samples_per_tti,1,1);
	    }
 }
  for (i=0;i<2*frame_parms->samples_per_tti;i++) {
	    for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx;aa++) {
	      if (awgn_flag == 0) {
		s_re[aa][i] = ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[0][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) + (i<<1)]);
		s_im[aa][i] = ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[0][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)+1]);
	      }
	      else {
		for (aarx=0;aarx<PHY_vars_UE->lte_frame_parms.nb_antennas_rx;aarx++) {
		  if (aa==0) {
		    r_re[aarx][i] = ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[0][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)]);
		    r_im[aarx][i] = ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[0][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)+1]);
		  }
		  else {
		    r_re[aarx][i] += ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[0][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)]);
		    r_im[aarx][i] += ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[0][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)+1]);
		  }
 
		}
	      }
	    }
	  }
 
  //Multipath channel
	  if (awgn_flag == 0) {	
	    multipath_channel(eNB2UE,s_re,s_im,r_re,r_im,
			      2*frame_parms->samples_per_tti,hold_channel);
	  }
	  
	  if(abstx){
	    if(saving_bler==0)
	    if (trials==0 && round==0) {
	      // calculate freq domain representation to compute SINR
	      freq_channel(eNB2UE, NB_RB,12*NB_RB + 1);
	      // snr=pow(10.0,.1*SNR);
	      fprintf(csv_fd,"%f,",SNR);
	      
	      for (u=0;u<12*NB_RB;u++){
		for (aarx=0;aarx<eNB2UE->nb_rx;aarx++) {
		  for (aatx=0;aatx<eNB2UE->nb_tx;aatx++) {
		    // abs_channel = (eNB2UE->chF[aarx+(aatx*eNB2UE->nb_rx)][u].x*eNB2UE->chF[aarx+(aatx*eNB2UE->nb_rx)][u].x + eNB2UE->chF[aarx+(aatx*eNB2UE->nb_rx)][u].y*eNB2UE->chF[aarx+(aatx*eNB2UE->nb_rx)][u].y);
		    channelx = eNB2UE->chF[aarx+(aatx*eNB2UE->nb_rx)][u].x;
		    channely = eNB2UE->chF[aarx+(aatx*eNB2UE->nb_rx)][u].y;
		    // if(transmission_mode==5){
		    fprintf(csv_fd,"%e+i*(%e),",channelx,channely);
		    // }
		    // else{
		    //	pilot_sinr = 10*log10(snr*abs_channel);
		    //	fprintf(csv_fd,"%e,",pilot_sinr);
		    // }
		  }
		}
	      }
	    }
	  }

	 
	  SNR =snr0;
	  //AWGN
	  sigma2_dB = 10*log10((double)tx_lev) +10*log10(PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size/(NB_RB*12)) - SNR;
	  sigma2 = pow(10,sigma2_dB/10);
	  if (n_frames==1)
	    printf("Sigma2 %f (sigma2_dB %f)\n",sigma2,sigma2_dB);

	  for (i=0; i<2*frame_parms->samples_per_tti; i++) {
	    for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_rx;aa++) {
	      //printf("s_re[0][%d]=> %f , r_re[0][%d]=> %f\n",i,s_re[aa][i],i,r_re[aa][i]);
	      ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti)+2*i] = 
		(short) (r_re[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	      ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti)+2*i+1] = 
		(short) (r_im[aa][i] + (iqim*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	    }
	  }   

	  //    lte_sync_time_init(PHY_vars_eNB->lte_frame_parms,lte_ue_common_vars);
	  //    lte_sync_time(lte_ue_common_vars->rxdata, PHY_vars_eNB->lte_frame_parms);
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
	  
	  /*//if (n_frames==1) {
	    //printf("RX level in null symbol %d\n",dB_fixed(signal_energy(&PHY_vars_UE->lte_ue_common_vars.rxdata[0][160+OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	    //printf("RX level in data symbol %d\n",dB_fixed(signal_energy(&PHY_vars_UE->lte_ue_common_vars.rxdata[0][160+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	    //printf("rx_level Null symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));

	    //printf("rx_level data symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
	  }*/
	  
     if (n_frames==1) {
	    printf("RX level in null symbol %d\n",dB_fixed(signal_energy(&PHY_vars_UE->lte_ue_common_vars.rxdata[0][160+OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	    printf("RX level in data symbol %d\n",dB_fixed(signal_energy(&PHY_vars_UE->lte_ue_common_vars.rxdata[0][160+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	    printf("rx_level Null symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
	    printf("rx_level data symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
	  }
	  
	  if (PHY_vars_eNB->lte_frame_parms.Ncp == 1) {  // extended prefix
	    pilot1 = 2;
	    pilot2 = 6;
	    pilot3 = 10; 
	  } 
	  //else {  // extended prefix
	    //pilot1 = 3;
	    //pilot2 = 6;
	    //pilot3 = 9;
	  
	  
	 // i_mod = get_Qm(mcs); 
	  
	  // Inner receiver scheduling for 3 slots
	  //for (Ns=(2*subframe);Ns<((2*subframe)+3);Ns++) {
//	for (subframe=0;subframe<10;subframe++) {
	    for (l=0;l<12;l++) {
	      if (n_frames==1)
		printf("subframe %d, l %d\n",subframe,l);
		
	  slot_fep_mbsfn(PHY_vars_UE,
		       l,
		       subframe%10,
		       0,
		       0);
	//if ((l=2) || (l=6) || (l=10))
	/* write_output("pmch00_ch0.m","mch00_ch0",
			       &PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][0][0],
			       PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,1,1);
       }
	 write_output("rxsigF0.m","rxsF0", &PHY_vars_UE->lte_ue_common_vars.rxdataF[0][0],
	              2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);    
 */

#ifdef PERFECT_CE
	      if (awgn_flag==0) {
		// fill in perfect channel estimates
		if (abstx==1)
		freq_channel(eNB2UE[round],PHY_vars_UE->lte_frame_parms.N_RB_DL,12*PHY_vars_UE->lte_frame_parms.N_RB_DL + 1);
		else
		    freq_channel(eNB2UE[0],PHY_vars_UE->lte_frame_parms.N_RB_DL,12*PHY_vars_UE->lte_frame_parms.N_RB_DL + 1);
		//write_output("channel.m","ch",desc1->ch[0],desc1->channel_length,1,8);
		//write_output("channelF.m","chF",desc1->chF[0],nb_samples,1,8);
		for(k=0;k<NUMBER_OF_eNB_MAX;k++) {
		  for(aa=0;aa<frame_parms->nb_antennas_tx;aa++) 
		    { 
		      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
			{
			  for (i=0;i<frame_parms->N_RB_DL*12;i++)
			    { 
			      if (abstx==1){
				((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[k][(aa<<1)+aarx])[2*i+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(eNB2UE[round]->chF[aarx+(aa*frame_parms->nb_antennas_rx)][i].x*AMP/2);
				((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[k][(aa<<1)+aarx])[2*i+1+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(eNB2UE[round]->chF[aarx+(aa*frame_parms->nb_antennas_rx)][i].y*AMP/2) ;
			      }
			      else {
				((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[k][(aa<<1)+aarx])[2*i+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(eNB2UE[0]->chF[aarx+(aa*frame_parms->nb_antennas_rx)][i].x*AMP/2);
				((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[k][(aa<<1)+aarx])[2*i+1+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(eNB2UE[0]->chF[aarx+(aa*frame_parms->nb_antennas_rx)][i].y*AMP/2) ;
			      }
			    }
			}
		    }
		}
	      }
	      else {
		for(aa=0;aa<frame_parms->nb_antennas_tx;aa++) 
		  { 
		    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
		      {
			for (i=0;i<frame_parms->N_RB_DL*12;i++)
			  { 
			    ((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][(aa<<1)+aarx])[2*i+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(short)(AMP/2);
			    ((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][(aa<<1)+aarx])[2*i+1+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=0/2;
			  }
		      }
		  }
	      }
#endif
 

	      if (l==0) {
		lte_ue_measurements(PHY_vars_UE,
				    subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti,
				    1,
				    0);
		/*
	debug_msg("RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), avg rx power %d dB (%d lin), RX gain %d dB\n",
		  PHY_vars_UE->PHY_measurements.rx_rssi_dBm[0] - ((PHY_vars_UE->lte_frame_parms.nb_antennas_rx==2) ? 3 : 0), 
		  PHY_vars_UE->PHY_measurements.wideband_cqi_dB[0][0],
		  PHY_vars_UE->PHY_measurements.wideband_cqi_dB[0][1],
		  PHY_vars_UE->PHY_measurements.wideband_cqi[0][0],
		  PHY_vars_UE->PHY_measurements.wideband_cqi[0][1],		  
		  PHY_vars_UE->PHY_measurements.rx_power_avg_dB[0],
		  PHY_vars_UE->PHY_measurements.rx_power_avg[0],
		  PHY_vars_UE->rx_total_gain_dB);
	debug_msg("N0 %d dBm digital (%d, %d) dB, linear (%d, %d), avg noise power %d dB (%d lin)\n",
		  PHY_vars_UE->PHY_measurements.n0_power_tot_dBm,
		  PHY_vars_UE->PHY_measurements.n0_power_dB[0],
		  PHY_vars_UE->PHY_measurements.n0_power_dB[1],
		  PHY_vars_UE->PHY_measurements.n0_power[0],
		  PHY_vars_UE->PHY_measurements.n0_power[1],
		  PHY_vars_UE->PHY_measurements.n0_power_avg_dB,
		  PHY_vars_UE->PHY_measurements.n0_power_avg);
	debug_msg("Wideband CQI tot %d dB, wideband cqi avg %d dB\n",
		  PHY_vars_UE->PHY_measurements.wideband_cqi_tot[0],
		  PHY_vars_UE->PHY_measurements.wideband_cqi_avg[0]);
		*/
		    
		if (transmission_mode==5 || transmission_mode==6) {
		  if (pmi_feedback == 1) {
		    pmi_feedback = 0;
		    hold_channel = 1;
		    goto PMI_FEEDBACK;
		  }
		}

	      }


	    if (l==pilot1) {// process symbols 0,1,2

		if (dci_flag == 1) {
		  rx_pdcch(&PHY_vars_UE->lte_ue_common_vars,
			   PHY_vars_UE->lte_ue_pdcch_vars,
			   &PHY_vars_UE->lte_frame_parms,
			   subframe,
			   0,
			   (PHY_vars_UE->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI,
			   0);

		  // overwrite number of pdcch symbols
		  PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols = num_pdcch_symbols;

		  dci_cnt = dci_decoding_procedure(PHY_vars_UE,
						   dci_alloc_rx,
						   eNB_id,
						   subframe);
		  //printf("dci_cnt %d\n",dci_cnt);
		
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
		    //printf("Generating dlsch parameters for RNTI %x\n",dci_alloc_rx[i].rnti);
		    if ((dci_alloc_rx[i].rnti == n_rnti) && 
			(generate_ue_dlsch_params_from_dci(0,
							   dci_alloc_rx[i].dci_pdu,
							   dci_alloc_rx[i].rnti,
							   dci_alloc_rx[i].format,
							   PHY_vars_UE->dlsch_ue[0],
							   &PHY_vars_UE->lte_frame_parms,
                               PHY_vars_UE->pdsch_config_dedicated,
							   SI_RNTI,
							   0,
							   P_RNTI)==0)) {
		      //dump_dci(&PHY_vars_UE->lte_frame_parms,&dci_alloc_rx[i]);
		      coded_bits_per_codeword = get_G(&PHY_vars_eNB->lte_frame_parms,
						      PHY_vars_UE->dlsch_ue[0][0]->nb_rb,
						      PHY_vars_UE->dlsch_ue[0][0]->rb_alloc,
						      get_Qm(PHY_vars_UE->dlsch_ue[0][0]->harq_processes[PHY_vars_UE->dlsch_ue[0][0]->current_harq_pid]->mcs),
						      PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols,
						      subframe);
		      /*
			rate = (double)dlsch_tbs25[get_I_TBS(PHY_vars_UE->dlsch_ue[0][0]->harq_processes[PHY_vars_UE->dlsch_ue[0][0]->current_harq_pid]->mcs)][PHY_vars_UE->dlsch_ue[0][0]->nb_rb-1]/(coded_bits_per_codeword);
			rate*=get_Qm(PHY_vars_UE->dlsch_ue[0][0]->harq_processes[PHY_vars_UE->dlsch_ue[0][0]->current_harq_pid]->mcs);

			printf("num_pdcch_symbols %d, G %d, TBS %d\n",PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols,coded_bits_per_codeword,PHY_vars_UE->dlsch_ue[0][0]->harq_processes[PHY_vars_UE->dlsch_ue[0][0]->current_harq_pid]->TBS);
		      */
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
		}  // if dci_flag==1
		else { //dci_flag == 0

		  PHY_vars_UE->lte_ue_pdcch_vars[0]->crnti = n_rnti;
		  PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols = num_pdcch_symbols;

		  generate_ue_dlsch_params_from_dci(0,
						    &DLSCH_alloc_pdu2_1E[0],
						    C_RNTI,
						    format1E_2A_M10PRB,
						    PHY_vars_UE->dlsch_ue[0],
						    &PHY_vars_UE->lte_frame_parms,
                            PHY_vars_UE->pdsch_config_dedicated,
						    SI_RNTI,
						    0,
						    P_RNTI);
		  dlsch_active = 1;
		} // if dci_flag == 1
	      }

	      if (dlsch_active == 1) {
		if  (l==0) { // process PDSCH symbols 0,1

		  for (m=PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols;
		       m<pilot1;
		       m++) 
		    {
		      if (rx_pdsch(PHY_vars_UE,
				   PDSCH,
				   eNB_id,
				   eNB_id_i,
				   subframe,
				   m,
				   (m==PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols)?1:0,
				   dual_stream_UE,
				   i_mod)==-1)
			{
			  dlsch_active = 0;
			  break;
			}
		    }
		}
		  
		if (l==pilot2)
		  {// process symbols  7,8,9 
		    for (m=pilot2;
			 m<pilot3;
			 m++)
		      {
			if (rx_pdsch(PHY_vars_UE,
				     PDSCH,
				     eNB_id,
				     eNB_id_i,
				     subframe,
				     m,
				     0,
				     dual_stream_UE,
				     i_mod)==-1)
			  {
			    dlsch_active=0;
			    break;
			  }
		      }
		  }
		
		    //for (m=pilot3;
			 //m<PHY_vars_UE->lte_frame_parms.symbols_per_tti;
			 //m++)
		      //{
			if ((l==pilot3) &&(rx_pdsch(PHY_vars_UE,
				     PDSCH,
				     eNB_id,
				     eNB_id_i,
				     subframe,
				     m,
				     0,
				     dual_stream_UE,
				     i_mod)==-1))
			  {
			    dlsch_active=0;
			    break;
			  }
		      
		  
		
		//if ((n_frames==1) && (Ns==(2+(2*subframe))) && (l==0))  {
		  //write_output("ch0.m","ch0",eNB2UE[0]->ch[0],eNB2UE[0]->channel_length,1,8);
		  //if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
		    //write_output("ch1.m","ch1",eNB2UE[0]->ch[PHY_vars_eNB->lte_frame_parms.nb_antennas_rx],eNB2UE[0]->channel_length,1,8);

		  //common vars
		  write_output("rxsig0.m","rxs0", &PHY_vars_UE->lte_ue_common_vars.rxdata[0][0],10*PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
		  write_output("rxsigF0.m","rxsF0", &PHY_vars_UE->lte_ue_common_vars.rxdataF[0][0],2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);
		  //write_output("rxsig0.m","rxs0", &PHY_vars_UE->lte_ue_common_vars.rxdata[0][0],10*PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
		  //write_output("rxsigF0.m","rxsF0", &PHY_vars_UE->lte_ue_common_vars.rxdataF[0][0],2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);
		  //if (PHY_vars_UE->lte_frame_parms.nb_antennas_rx>1) {
		    //write_output("rxsig1.m","rxs1", PHY_vars_UE->lte_ue_common_vars.rxdata[1],PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
		    //write_output("rxsigF1.m","rxsF1", PHY_vars_UE->lte_ue_common_vars.rxdataF[1],2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);
		  //}

		 write_output("pmch00_ch0.m","mch00_ch0",
			       &PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][0][0],
			       PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,1,1);
		    
		  //pdsch_vars
		  dump_dlsch2(PHY_vars_UE,eNB_id,coded_bits_per_codeword);
          dump_dlsch2(PHY_vars_UE,eNB_id_i,coded_bits_per_codeword);
		  write_output("dlsch_e.m","e",PHY_vars_eNB->dlsch_eNB[0][0]->e,coded_bits_per_codeword,1,4);

		  //pdcch_vars
		  write_output("pdcchF0_ext.m","pdcchF_ext", PHY_vars_UE->lte_ue_pdcch_vars[eNB_id]->rxdataF_ext[0],2*3*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size,1,1);
		  write_output("pdcch00_ch0_ext.m","pdcch00_ch0_ext",PHY_vars_UE->lte_ue_pdcch_vars[eNB_id]->dl_ch_estimates_ext[0],300*3,1,1);

		  write_output("pdcch_rxF_comp0.m","pdcch0_rxF_comp0",PHY_vars_UE->lte_ue_pdcch_vars[eNB_id]->rxdataF_comp[0],4*300,1,1);
		  write_output("pdcch_rxF_llr.m","pdcch_llr",PHY_vars_UE->lte_ue_pdcch_vars[eNB_id]->llr,2400,1,4);
		    
		}
	      	      
	       }
	     // }
     			   
	//saving PMI incase of Transmission Mode > 5

	/*  if(abstx){
	    if(saving_bler==0)
	    if (trials==0 && round==0 && transmission_mode>=5){
	      for (iii=0; iii<NB_RB; iii++){
		//fprintf(csv_fd, "%d, %d", (PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->pmi_ext[iii]),(PHY_vars_UE->lte_ue_pdsch_vars[eNB_id_i]->pmi_ext[iii]));
		fprintf(csv_fd,"%x,%x,",(PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->pmi_ext[iii]),(PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->pmi_ext[iii]));
		msg(" %x",(PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->pmi_ext[iii]));
	      }
	    }
	  }
	
	  // calculate uncoded BLER
	  /* uncoded_ber=0;
	  for (i=0;i<coded_bits_per_codeword;i++) 
	    if (PHY_vars_eNB->dlsch_eNB[0][0]->e[i] != (PHY_vars_UE->lte_ue_pdsch_vars[0]->llr[0][i]<0)) {
	      uncoded_ber_bit[i] = 1;
	      uncoded_ber++;
	    }
	    else
	      uncoded_ber_bit[i] = 0;

	  uncoded_ber/=coded_bits_per_codeword;
	  avg_ber += uncoded_ber;
	  */
	  //write_output("uncoded_ber_bit.m","uncoded_ber_bit",uncoded_ber_bit,coded_bits_per_codeword,1,0);
	 
	  /*
	    printf("precoded CQI %d dB, opposite precoded CQI %d dB\n",
	    PHY_vars_UE->PHY_measurements.precoded_cqi_dB[eNB_id][0],
	    PHY_vars_UE->PHY_measurements.precoded_cqi_dB[eNB_id_i][0]);
	  */

	  // clip the llrs
	  /*	   for (i=0; i<coded_bits_per_codeword; i++) {
	    if (PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0][i]>127)
	      PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0][i] = 127;
	    else if (PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0][i]<-128)
	      PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0][i] = -128;
	      }
	  */
	  PHY_vars_UE->dlsch_ue[0][0]->rnti = n_rnti;
	  dlsch_unscrambling(&PHY_vars_UE->lte_frame_parms,
			     PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols,
			     PHY_vars_UE->dlsch_ue[0][0],
			     coded_bits_per_codeword,
			     PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0],
			     0,
			     subframe<<1);

	  /*
	  for (i=0;i<coded_bits_per_codeword;i++) 
	    PHY_vars_UE->lte_ue_pdsch_vars[0]->llr[0][i] = (short)quantize(100,PHY_vars_UE->lte_ue_pdsch_vars[0]->llr[0][i],4);
	  */

	  ret = dlsch_decoding(PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0],		 
			       &PHY_vars_UE->lte_frame_parms,
			       PHY_vars_UE->dlsch_ue[0][0],
			       subframe,
			       PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols);
 
#ifdef XFORMS
	  do_forms(form,
		   &PHY_vars_UE->lte_frame_parms,  
		   PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates_time[0],
		   PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0],
		   PHY_vars_UE->lte_ue_common_vars.rxdata,
		   PHY_vars_UE->lte_ue_common_vars.rxdataF,
		   PHY_vars_UE->lte_ue_pdsch_vars[0]->rxdataF_comp[0],
		   PHY_vars_UE->lte_ue_pdsch_vars[1]->rxdataF_comp[0],
		   PHY_vars_UE->lte_ue_pdsch_vars[0]->dl_ch_rho_ext[0],
		   PHY_vars_UE->lte_ue_pdsch_vars[0]->llr[0],coded_bits_per_codeword);
	  //PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->w[0],3*(tbs+64)); 
	  //uncoded_ber_bit,coded_bits_per_codeword);


	  /*
	  printf("Hit a key to continue\n");
	  c = getchar();
	  */
 
#endif

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
	      //if ((n_frames==1) || (SNR>=30)) {
	      printf("DLSCH errors found, uncoded ber %f\n",uncoded_ber);
	      //for (s=0;s<PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->C;s++) {
		//if (s<PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->Cminus)
		  //Kr = PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->Kminus;
		//else
		  //Kr = PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->Kplus;
		    
		//Kr_bytes = Kr>>3;
		    
		//printf("Decoded_output (Segment %d):\n",s);
		//for (i=0;i<Kr_bytes;i++)
		 //// printf("%d : %x (%x)\n",i,PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->c[s][i],PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->c[s][i]^PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->c[s][i]);
	      //}
	      write_output("rxsig0.m","rxs0", &PHY_vars_UE->lte_ue_common_vars.rxdata[0][0],10*PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
	      write_output("rxsigF0.m","rxsF0", &PHY_vars_UE->lte_ue_common_vars.rxdataF[0][0],2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);
	   /*   if (PHY_vars_UE->lte_frame_parms.nb_antennas_rx>1) {
		write_output("rxsig1.m","rxs1", PHY_vars_UE->lte_ue_common_vars.rxdata[1],PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
		write_output("rxsigF1.m","rxsF1", PHY_vars_UE->lte_ue_common_vars.rxdataF[1],2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);
	      }*/
	      
	    write_output("pmch00_ch0.m","mch00_ch0",
			       &PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][0][0],
			       PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,1,1);
	      
	      //pdsch_vars
	      //dump_dlsch2(PHY_vars_UE,eNB_id,coded_bits_per_codeword);
	      //write_output("dlsch_e.m","e",PHY_vars_eNB->dlsch_eNB[0][0]->e,coded_bits_per_codeword,1,4);
	      //write_output("dlsch_ber_bit.m","ber_bit",uncoded_ber_bit,coded_bits_per_codeword,1,0);
	      //write_output("dlsch_eNB_w.m","w",PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->w[0],3*(tbs+64),1,4);
	      //write_output("dlsch_UE_w.m","w",PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->w[0],3*(tbs+64),1,0);

	      
	      exit(-1);
	    }
	    //	    printf("round %d errors %d/%d\n",round,errs[round],trials);

	
	    round++;
		
	    if (n_frames==1)
	      printf("DLSCH in error in round %d\n",round);
		
	  }
	  //free(uncoded_ber_bit);
	  //uncoded_ber_bit = NULL;
	  
	}  //round
	//      printf("\n");

	if ((errs[0]>=n_frames/10) && (trials>(n_frames/2)))
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
	     (1.0*(round_trials[0]-errs[0])+2.0*(round_trials[1]-errs[1])+3.0*(round_trials[2]-errs[2])+4.0*(round_trials[3]-errs[3]))/((double)round_trials[0])/(double)PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->TBS,
	     (1.0*(round_trials[0]-errs[0])+2.0*(round_trials[1]-errs[1])+3.0*(round_trials[2]-errs[2])+4.0*(round_trials[3]-errs[3]))/((double)round_trials[0]));
    
      
      fprintf(bler_fd,"%f;%d;%d;%f;%d;%d;%d;%d;%d;%d;%d;%d;%d\n",
	      SNR,
	      mcs,
	      PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->TBS,
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

      fprintf(tikz_fd,"(%f,%f)", SNR, (float)errs[0]/round_trials[0]);
    
      if(abstx){ //ABSTRACTION         
	blerr[0] = (double)errs[0]/(round_trials[0]);
	blerr[1] = (double)errs[1]/(round_trials[1]);
	blerr[2] = (double)errs[2]/(round_trials[2]);
	blerr[3] = (double)errs[3]/(round_trials[3]);
	fprintf(csv_fd,"%e,%e,%e,%e;\n",blerr[0],blerr[1],blerr[2],blerr[3]);
	/*	
	blerr = (double)errs[0]/(round_trials[0]);
	
	if(saving_bler==0)
	   fprintf(csv_fd,"%e;\n",blerr);

	if(blerr<1)
	  saving_bler = 0;
	else saving_bler =1;
	*/
	 
      } //ABStraction
      /* if(num_rounds==1){
      blerr= (double)errs[1]/(round_trials[1]);
	if (blerr>.1)
	  snr_step = 1.5;
	else snr_step = input_snr_step;
	}*/
      if (((double)errs[0]/(round_trials[0]))<1e-2) 
	break;
    }// SNR
  
  
  } //ch_realization
  
  
  fclose(bler_fd);
  fprintf(tikz_fd,"};\n");
  fclose(tikz_fd);

  if (input_trch_file==1)
    fclose(input_trch_fd);
  if (input_file==1)
    fclose(input_fd);
  if(abstx){// ABSTRACTION
    fprintf(csv_fd,"];");
    fclose(csv_fd);
  }
  
  
  
  
  printf("Freeing dlsch structures\n");
  for (i=0;i<2;i++) {
    printf("eNB %d\n",i);
    free_eNB_dlsch(PHY_vars_eNB->dlsch_eNB[0][i]);
    printf("UE %d\n",i);
    free_ue_dlsch(PHY_vars_UE->dlsch_ue[0][i]);
  }
  
  
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
  



  //for (i=0;i<2;i++) {
    //free(s_re[i]);
    //free(s_im[i]);
    //free(r_re[i]);
    //free(r_im[i]);

  ////free(s_re);
  ////free(s_im);
  ////free(r_re);
  ////free(r_im);
  
  
  //free(s_re);
  //free(s_im );
  //free(s_re1);
  //free(s_im1);
  //free(s_re2);
  //free(s_im2);
  //free(r_re);
  //free(r_im);
  //free(r_re1);
  //free(r_im1);
  //free(r_re2);
  //free(r_im2);
