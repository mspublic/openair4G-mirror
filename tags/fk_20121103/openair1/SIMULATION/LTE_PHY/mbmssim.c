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

int main(int argc, char **argv) {

  char c;


  int s,Kr,Kr_bytes;

  int i,k,l,aa,aarx, aatx;
  double sigma2, sigma2_dB=0,SNR,snr0=-2.0,snr1,rate,saving_bler=1;
  u8 snr1set=0;
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

  FILE *input_fd=NULL,*pbch_file_fd=NULL;
  char input_val_str[50],input_val_str2[50];
  double input_val1,input_val2;
  u16 amask=0;
  u8 frame_mod4,num_pdcch_symbols;
  u16 NB_RB=25;
  int tdd_config=3;
  
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
  
  DCI_ALLOC_t dci_alloc[8];
  u8 abstraction_flag=0,calibration_flag=0;
  double pbch_sinr;
  int pbch_tx_ant;
  u8 N_RB_DL=25,osf=1;
  
   //int len;
  u8 num_rounds = 4,fix_rounds=0;
  int u;
  int abstx=0;
  int iii;
  FILE *csv_fd;
  char csv_fname[512];
  int ch_realization;
  int pmi_feedback=0;
  int hold_channel=0; 
  // void *data;
  // int ii;
  // int bler;
  double blerr,uncoded_ber,avg_ber;
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

  eNB2UE = new_channel_desc_scm(PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,
				PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				channel_model,
				BW,
				0,
				0,
				0);

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

// }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 
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
		   
#ifdef PERFECT_CE
	      if (awgn_flag==0) {
		// fill in perfect channel estimates
		freq_channel(eNB2UE,PHY_vars_UE->lte_frame_parms.N_RB_DL,12*PHY_vars_UE->lte_frame_parms.N_RB_DL + 1);
		//write_output("channel.m","ch",desc1->ch[0],desc1->channel_length,1,8);
		//write_output("channelF.m","chF",desc1->chF[0],nb_samples,1,8);
		for(k=0;k<NUMBER_OF_eNB_MAX;k++) {
		  for(aa=0;aa<frame_parms->nb_antennas_tx;aa++) 
		    { 
		      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
			{
			  for (i=0;i<frame_parms->N_RB_DL*12;i++)
                  { 
                      ((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[k][(aa<<1)+aarx])[2*i+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(eNB2UE->chF[aarx+(aa*frame_parms->nb_antennas_rx)][i].x*AMP/2);
                      ((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[k][(aa<<1)+aarx])[2*i+1+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(eNB2UE->chF[aarx+(aa*frame_parms->nb_antennas_rx)][i].y*AMP/2);
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
			    ((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][(aa<<1)+aarx])[2*i+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(short)(((int)AMP*PHY_vars_UE->dlsch_ue[0][0]->sqrt_rho_b)>>13)/2;
			    ((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][(aa<<1)+aarx])[2*i+1+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=0/2;
			  }
		      }
		  }
	      }
#endif
 

       }
    
 }

