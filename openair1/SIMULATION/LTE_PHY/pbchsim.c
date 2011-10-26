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

#define BW 5.0


PHY_VARS_eNB *PHY_vars_eNb,*PHY_vars_eNb1,*PHY_vars_eNb2;
PHY_VARS_UE *PHY_vars_UE;

#define DLSCH_RB_ALLOC 0x1fbf // igore DC component,RB13


void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,unsigned char extended_prefix_flag,u16 Nid_cell,u8 N_RB_DL,u8 osf) {

  unsigned int ind;
  LTE_DL_FRAME_PARMS *lte_frame_parms;
  int i;

  printf("Start lte_param_init\n");
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
  //  lte_frame_parms->Csrs = 2;
  //  lte_frame_parms->Bsrs = 0;
  //  lte_frame_parms->kTC = 0;
  //  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->mode1_flag = (transmission_mode == 1)? 1 : 0;
  lte_frame_parms->tdd_config = 3;
  lte_frame_parms->frame_type = 1;
  init_frame_parms(lte_frame_parms,osf);
  
  //copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(lte_frame_parms); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  PHY_vars_UE->lte_frame_parms = *lte_frame_parms;

  phy_init_lte_top(lte_frame_parms);
  for (i=0;i<3;i++)
    lte_gold(lte_frame_parms,PHY_vars_UE->lte_gold_table[i],i);    

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

  printf("Done lte_param_init\n");


}


int main(int argc, char **argv) {

  char c;

  int i,l,aa,aarx;
  double sigma2, sigma2_dB=0,SNR,snr0=-2.0,snr1;
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
  int subframe_offset;
  char fname[40], vname[40];
  int trial, n_trials, ntrials=1, n_errors,n_errors2,n_alamouti;
  u8 transmission_mode = 1,n_tx=1,n_rx=1;
  unsigned char eNb_id = 0;
  u16 Nid_cell=0;

  u8 awgn_flag=0;
  int n_frames=1;
  channel_desc_t *eNB2UE,*eNB2UE1,*eNB2UE2;
  u32 nsymb,tx_lev,tx_lev_dB;
  u8 extended_prefix_flag=0;
  s8 interf1=-21,interf2=-21;
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

  SCM_t channel_model=Rayleigh1_anticorr;

  DCI_ALLOC_t dci_alloc[8];
  u8 abstraction_flag=0,calibration_flag=0;
  double pbch_sinr;
  int pbch_tx_ant;
  u8 N_RB_DL=25,osf=1;

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
  while ((c = getopt (argc, argv, "haA:Cr:pf:g:i:j:n:s:S:t:x:y:z:N:F:R:O:")) != -1)
    {
      switch (c)
	{
	case 'a':
	  printf("Running AWGN simulation\n");
	  awgn_flag = 1;
	  ntrials=1;
	  break;
	case 'f':
	  output_fd = fopen(optarg,"w");
	  write_output_file=1;
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
	case 'p':
	  extended_prefix_flag=1;
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
	default:
	case 'h':
	  printf("%s -h(elp) -a(wgn on) -p(extended_prefix) -N cell_id -f output_filename -F input_filename -g channel_model -n n_frames -t Delayspread -r Ricean_FactordB -s snr0 -S snr1 -x transmission_mode -y TXant -z RXant -i Intefrence0 -j Interference1 -A interpolation_file -C(alibration offset dB) -N CellId\n",argv[0]);
	  printf("-h This message\n");
	  printf("-a Use AWGN channel and not multipath\n");
	  printf("-p Use extended prefix mode\n");
	  printf("-n Number of frames to simulate\n");
	  printf("-r Ricean factor (dB, 0 means Rayleigh, 100 is almost AWGN\n");
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

  if (transmission_mode==2)
    n_tx=2;

  lte_param_init(n_tx,n_rx,transmission_mode,extended_prefix_flag,Nid_cell,N_RB_DL,osf);


  if (snr1set==0) {
    if (n_frames==1)
      snr1 = snr0+.1;
    else
      snr1 = snr0+5.0;
  }

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
  txdata1 = PHY_vars_eNb1->lte_eNB_common_vars.txdata[eNb_id];
  txdata2 = PHY_vars_eNb2->lte_eNB_common_vars.txdata[eNb_id];
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

  nsymb = (frame_parms->Ncp == 0) ? 14 : 12;

  printf("FFT Size %d, Extended Prefix %d, Samples per subframe %d, Symbols per subframe %d\n",NUMBER_OF_OFDM_CARRIERS,
	 frame_parms->Ncp,frame_parms->samples_per_tti,nsymb);

  printf("PHY_vars_eNb1->lte_eNB_common_vars.txdataF[0][0] = %p\n",
	 PHY_vars_eNb1->lte_eNB_common_vars.txdataF[0][0]);


  DLSCH_alloc_pdu2.rah              = 0;
  DLSCH_alloc_pdu2.rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2.TPC              = 0;
  DLSCH_alloc_pdu2.dai              = 0;
  DLSCH_alloc_pdu2.harq_pid         = 0;
  DLSCH_alloc_pdu2.tb_swap          = 0;
  DLSCH_alloc_pdu2.mcs1             = 0;  
  DLSCH_alloc_pdu2.ndi1             = 1;
  DLSCH_alloc_pdu2.rv1              = 0;
  // Forget second codeword
  DLSCH_alloc_pdu2.tpmi             = (transmission_mode==6 ? 5 : 0) ;  // precoding

  eNB2UE = new_channel_desc_scm(PHY_vars_eNb->lte_frame_parms.nb_antennas_tx,
				PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				channel_model,
				BW,
				0,
				0,
				0);

  if (interf1>-20)
    eNB2UE1 = new_channel_desc_scm(PHY_vars_eNb->lte_frame_parms.nb_antennas_tx,
				   PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				   channel_model,
				   BW,
				   0,
				   0,
				   0);
  
  if (interf2>-20)
    eNB2UE2 = new_channel_desc_scm(PHY_vars_eNb->lte_frame_parms.nb_antennas_tx,
				   PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				   channel_model,
				   BW,
				   0,
				   0,
				   0);
  

  if (eNB2UE==NULL) {
    msg("Problem generating channel model. Exiting.\n");
    exit(-1);
  }

  for (i=0;i<2;i++) {

    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_re[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_im[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_re1[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_re1[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im1[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_im1[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));    
    s_re2[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_re2[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im2[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_im2[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));

    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_re[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_im[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re1[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_re1[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im1[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_im1[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re2[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_re2[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im2[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_im2[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }

  switch (frame_parms->nb_antennas_tx) {
  case 1:
    amask = 0x0000;
    break;
  case 2:
    amask = 0xffff;
    break;
  case 4:
    amask = 0x5555;
  }

  if (pbch_file_fd!=NULL) {
    load_pbch_desc(pbch_file_fd);
  }

  if (input_fd==NULL) {

    //    for (i=0;i<6;i++)
    //      pbch_pdu[i] = i;
    pbch_pdu[0]=100;
    pbch_pdu[1]=1;
    pbch_pdu[2]=0;
    
    generate_pss(PHY_vars_eNb->lte_eNB_common_vars.txdataF[0],
		 1024,
		 &PHY_vars_eNb->lte_frame_parms,
		 2,
		 2);
    
    memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
    dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
    dci_alloc[0].L          = 2;
    dci_alloc[0].rnti       = 0x1234;
    /*    
    memcpy(&dci_alloc[1].dci_pdu[1],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
    dci_alloc[1].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
    dci_alloc[1].L          = 3;
    dci_alloc[1].rnti       = 0x1234;    
    */

    printf("Generating PBCH for mode1_flag = %d\n", PHY_vars_eNb->lte_frame_parms.mode1_flag);
    
    
    generate_pilots(PHY_vars_eNb,
		    PHY_vars_eNb->lte_eNB_common_vars.txdataF[0],
		    1024,
		    LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);


        
    num_pdcch_symbols = generate_dci_top(1,
					 0,
					 dci_alloc,
					 0,
					 1024,
					 &PHY_vars_eNb->lte_frame_parms,
					 PHY_vars_eNb->lte_eNB_common_vars.txdataF[0],
					 					 0);
    

    /*
    if (num_pdcch_symbols<3) {
      printf("Less than 3 pdcch symbols\n");
      exit(-1);
    }
    */
    generate_pbch(&PHY_vars_eNb->lte_eNB_pbch,
		  PHY_vars_eNb->lte_eNB_common_vars.txdataF[0],
		  1024,
		  &PHY_vars_eNb->lte_frame_parms,
		  pbch_pdu,
		  0);
    /*
    generate_pbch(PHY_vars_eNb->lte_eNB_common_vars.txdataF[0],
		  1024,
		  &PHY_vars_eNb->lte_frame_parms,
		  pbch_pdu,
		  3);
    */

    if (interf1>-20) {
      generate_pss(PHY_vars_eNb1->lte_eNB_common_vars.txdataF[0],
		   1024,
		   &PHY_vars_eNb1->lte_frame_parms,
		   (PHY_vars_eNb1->lte_frame_parms.Ncp==0)?6:5,
		   0);
      
      
      
      
      
      generate_pilots(PHY_vars_eNb1,
		      PHY_vars_eNb1->lte_eNB_common_vars.txdataF[0],
		      1024,
		      2);//LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
      
      
      generate_pbch(&PHY_vars_eNb1->lte_eNB_pbch,
		    PHY_vars_eNb1->lte_eNB_common_vars.txdataF[0],
		    1024,
		    &PHY_vars_eNb1->lte_frame_parms,
		    pbch_pdu,
		    0);
      
    }
    
    if (interf2>-20) {
      generate_pss(PHY_vars_eNb2->lte_eNB_common_vars.txdataF[0],
		   1024,
		   &PHY_vars_eNb2->lte_frame_parms,
		   (PHY_vars_eNb2->lte_frame_parms.Ncp==0)?6:5,
		   0);
      
      
      
      
      
      generate_pilots(PHY_vars_eNb2,
		      PHY_vars_eNb2->lte_eNB_common_vars.txdataF[0],
		      1024,
		      2);//LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
      
      
      generate_pbch(&PHY_vars_eNb2->lte_eNB_pbch,
		    PHY_vars_eNb2->lte_eNB_common_vars.txdataF[0],
		    1024,
		    &PHY_vars_eNb2->lte_frame_parms,
		    pbch_pdu,
		    0);
      
    }
    
    printf("PHY_vars_eNb1->lte_eNB_common_vars.txdataF[0][0] = %p\n",
	 PHY_vars_eNb1->lte_eNB_common_vars.txdataF[0][0]);
    
    
    
    
    //  write_output("pilotsF.m","rsF",txdataF[0],lte_frame_parms->ofdm_symbol_size,1,1);
#ifdef IFFT_FPGA
    write_output("txsigF0.m","txsF0", PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0],300*120,1,4);
    if (PHY_vars_eNb->lte_frame_parms.nb_antennas_tx>1)
      write_output("txsigF1.m","txsF1", PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][1],300*120,1,4);
    
    for (i=0;i<10;i++) 
      debug_msg("%08x\n",((unsigned int*)&PHY_vars_eNb->lte_eNB_common_vars.txdataF[0][0][1*(PHY_vars_eNb->lte_frame_parms.N_RB_DL*12)*(PHY_vars_eNb->lte_frame_parms.symbols_per_tti>>1)])[i]);
    
    
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
    
    write_output("txsigF20.m","txsF20", txdataF2[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
    if (PHY_vars_eNb->lte_frame_parms.nb_antennas_tx>1)
      write_output("txsigF21.m","txsF21", txdataF2[1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
    
    
    tx_lev=0;
    
    for (aa=0; aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx; aa++) {
      
      if (frame_parms->Ncp == 1) 
	PHY_ofdm_mod(txdataF2[aa],        // input
		     txdata[aa],         // output
		     PHY_vars_eNb->lte_frame_parms.log2_symbol_size,                // log2_fft_size
		     LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,                 // number of symbols
		     PHY_vars_eNb->lte_frame_parms.nb_prefix_samples,               // number of prefix samples
		     PHY_vars_eNb->lte_frame_parms.twiddle_ifft,  // IFFT twiddle factors
		     PHY_vars_eNb->lte_frame_parms.rev,           // bit-reversal permutation
		     CYCLIC_PREFIX);
      else {
	normal_prefix_mod(txdataF2[aa],txdata[aa],LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,frame_parms);
      }
      tx_lev += signal_energy(&txdata[aa][0],
			      OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
    }
#else
    write_output("txsigF0.m","txsF0", PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
    if (PHY_vars_eNb->lte_frame_parms.nb_antennas_tx>1)
      write_output("txsigF1.m","txsF1", PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
    
    tx_lev = 0;
    
    
    
    
    for (aa=0; aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx; aa++) {
      if (frame_parms->Ncp == 1) 
	PHY_ofdm_mod(PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa],        // input,
		     txdata[aa],         // output
		     frame_parms->log2_symbol_size,                // log2_fft_size
		     LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,                 // number of symbols
		     frame_parms->nb_prefix_samples,               // number of prefix samples
		     frame_parms->twiddle_ifft,  // IFFT twiddle factors
		     frame_parms->rev,           // bit-reversal permutation
		     CYCLIC_PREFIX);
      else {
	normal_prefix_mod(PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa],
			  txdata[aa],
			  LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,
			  frame_parms);
      }
      
      tx_lev += signal_energy(&txdata[aa][0],
			      OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
    }
    
    if (interf1>-20) {
      for (aa=0; aa<PHY_vars_eNb1->lte_frame_parms.nb_antennas_tx; aa++) {
	if (frame_parms->Ncp == 1) 
	  PHY_ofdm_mod(PHY_vars_eNb1->lte_eNB_common_vars.txdataF[eNb_id][aa],        // input,
		       PHY_vars_eNb1->lte_eNB_common_vars.txdata[eNb_id][aa],         // output
		       frame_parms->log2_symbol_size,                // log2_fft_size
		       2*nsymb,                 // number of symbols
		       frame_parms->nb_prefix_samples,               // number of prefix samples
		       frame_parms->twiddle_ifft,  // IFFT twiddle factors
		       frame_parms->rev,           // bit-reversal permutation
		       CYCLIC_PREFIX);
	else {
	  normal_prefix_mod(PHY_vars_eNb1->lte_eNB_common_vars.txdataF[eNb_id][aa],
			    PHY_vars_eNb1->lte_eNB_common_vars.txdata[eNb_id][aa],
			    2*nsymb,
			    frame_parms);
	}
      }
    }
    
    if (interf2>-20) {
      for (aa=0; aa<PHY_vars_eNb2->lte_frame_parms.nb_antennas_tx; aa++) {
	if (frame_parms->Ncp == 1) 
	  PHY_ofdm_mod(PHY_vars_eNb2->lte_eNB_common_vars.txdataF[eNb_id][aa],        // input,
		       PHY_vars_eNb2->lte_eNB_common_vars.txdata[eNb_id][aa],         // output
		       frame_parms->log2_symbol_size,                // log2_fft_size
		       2*nsymb,                 // number of symbols
		       frame_parms->nb_prefix_samples,               // number of prefix samples
		       frame_parms->twiddle_ifft,  // IFFT twiddle factors
		       frame_parms->rev,           // bit-reversal permutation
		       CYCLIC_PREFIX);
	else {
	  normal_prefix_mod(PHY_vars_eNb2->lte_eNB_common_vars.txdataF[eNb_id][aa],
			    PHY_vars_eNb2->lte_eNB_common_vars.txdata[eNb_id][aa],
			    2*nsymb,
			    frame_parms);
	}
      }
    }
#endif
    
    
    tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
    
    write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    if (frame_parms->nb_antennas_tx>1)
      write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  }
  else {  //read in from file
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


  write_output("txsig0.m","txs0", txdata[0],2*frame_parms->samples_per_tti,1,1);
    // multipath channel
  
  for (i=0;i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
    for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx;aa++) {
      if (awgn_flag == 0) {
	s_re[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)]);
	s_im[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)+1]);
	if (interf1>-20) {
	  s_re1[aa][i] = ((double)(((short *)txdata1[aa]))[(i<<1)]);
	  s_im1[aa][i] = ((double)(((short *)txdata1[aa]))[(i<<1)+1]);
	}
	if (interf2>-20) {
	  s_re2[aa][i] = ((double)(((short *)txdata2[aa]))[(i<<1)]);
	  s_im2[aa][i] = ((double)(((short *)txdata2[aa]))[(i<<1)+1]);
	}
      }
      else {
	for (aarx=0;aarx<PHY_vars_UE->lte_frame_parms.nb_antennas_rx;aarx++) {
	  if (aa==0) {
	    r_re[aarx][i] = ((double)(((short *)txdata[aa]))[(i<<1)]);
	    r_im[aarx][i] = ((double)(((short *)txdata[aa]))[(i<<1)+1]);
	  }
	  else {
	    r_re[aarx][i] += ((double)(((short *)txdata[aa]))[(i<<1)]);
	    r_im[aarx][i] += ((double)(((short *)txdata[aa]))[(i<<1)+1]);
	  }
	  	  
	  if (interf1>=-20) {
	    r_re[aarx][i]+= pow(10.0,.05*interf1)*((double)(((short *)PHY_vars_eNb1->lte_eNB_common_vars.txdata[eNb_id][aa]))[(i<<1)]);
	    r_im[aarx][i]+= pow(10.0,.05*interf1)*((double)(((short *)PHY_vars_eNb1->lte_eNB_common_vars.txdata[eNb_id][aa]))[(i<<1)+1]);
	    
	  }
	  if (interf2>=-20) {
	    r_re[aarx][i]+=pow(10.0,.05*interf2)*((double)(((short *)PHY_vars_eNb2->lte_eNB_common_vars.txdata[eNb_id][aa]))[(i<<1)]);
	    r_im[aarx][i]+=pow(10.0,.05*interf2)*((double)(((short *)PHY_vars_eNb2->lte_eNB_common_vars.txdata[eNb_id][aa]))[(i<<1)+1]);
	  }
	  
	}
      }
    }
  }


  for (SNR=snr0;SNR<snr1;SNR+=.2) {


    n_errors = 0;
    n_errors2 = 0;
    n_alamouti = 0;
    for (trial=0; trial<n_frames; trial++) {
      pbch_sinr=0.0;
      if (abstraction_flag==1)
	printf("*********************** trial %d ***************************\n",trial);

      while (pbch_sinr>-2.0) {
	
	if (awgn_flag == 0) {	


	  multipath_channel(eNB2UE,s_re,s_im,r_re,r_im,
			    2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
	  	  
	  if (interf1>-20) 
	    multipath_channel(eNB2UE1,s_re1,s_im1,r_re1,r_im1,
			      2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);

	  if (interf2>-20) 
	    multipath_channel(eNB2UE2,s_re2,s_im2,r_re2,r_im2,
			      2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
	  
	  if (abstraction_flag == 1) {
	    freq_channel(eNB2UE,25,51);
	    if (interf1>-20) 
	      freq_channel(eNB2UE1,25,51);
	    if (interf2>-20) 
	      freq_channel(eNB2UE2,25,51);
	    pbch_sinr = compute_pbch_sinr(eNB2UE,eNB2UE1,eNB2UE2,SNR,SNR+interf1,SNR+interf2,25);
	    printf("total_sinr %f\n",compute_sinr(eNB2UE,eNB2UE1,eNB2UE2,SNR,SNR+interf1,SNR+interf2,25));
	    printf("pbch_sinr %f => BLER %f\n",pbch_sinr,pbch_bler(pbch_sinr));
	  }
	  else {
	    pbch_sinr = -3.0;
	  }
	  //	  exit(-1);
	} // awgn_flag
	else {
	  pbch_sinr = -3.0;
	}
      }
      
      sigma2_dB = 10*log10((double)tx_lev) +10*log10(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size/(12*NB_RB)) - SNR;
      if (n_frames==1)
	printf("sigma2_dB %f (SNR %f dB) tx_lev_dB %f\n",sigma2_dB,SNR,10*log10((double)tx_lev));
      //AWGN
      sigma2 = pow(10,sigma2_dB/10);
        
      /*    
      if (n_frames==1) {
	printf("rx_level data symbol %f, tx_lev %f\n",
	       10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0)),
	       10*log10(tx_lev));
      }
      */
      
      for (n_trials=0;n_trials<ntrials;n_trials++) {
	//printf("n_trial %d\n",n_trials);
	for (i=0; i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES; i++) {
	  for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_rx;aa++) {
	    if (n_trials==0) {
	      r_re[aa][i] += (pow(10.0,.05*interf1)*r_re1[aa][i] + pow(10.0,.05*interf2)*r_re2[aa][i]);
	      r_im[aa][i] += (pow(10.0,.05*interf1)*r_im1[aa][i] + pow(10.0,.05*interf2)*r_im2[aa][i]);
	    }
	    ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[2*i] = (short) (.167*(r_re[aa][i] +sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	    ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[2*i+1] = (short) (.167*(r_im[aa][i] + (iqim*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	  }
	}    
	/*
	if (n_trials==0) {
	  printf("rx_level data symbol %f\n",
		 10*log10(signal_energy(PHY_vars_UE->lte_ue_common_vars.rxdata[0],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)));
	}
	*/

	/*
	// optional: read rx_frame from file
	if ((rx_frame_file = fopen("rx_frame.dat","r")) == NULL)
	{
	  printf("[openair][CHBCH_TEST][INFO] Cannot open rx_frame.m data file\n");
	  exit(0);
	}
	  
	result = fread((void *)PHY_vars_UE->lte_ue_common_vars.rxdata[0],4,FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
	printf("Read %d bytes\n",result);
	if (PHY_vars_UE->lte_frame_parms.nb_antennas_rx>1) {
	  result = fread((void *)PHY_vars_UE->lte_ue_common_vars.rxdata[1],4,FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
	  printf("Read %d bytes\n",result);
	}
	
	fclose(rx_frame_file);
	*/

	sync_pos = lte_sync_time(PHY_vars_UE->lte_ue_common_vars.rxdata, 
				 &PHY_vars_UE->lte_frame_parms, 
				 &PHY_vars_UE->lte_ue_common_vars.eNb_id);
	
	// the sync is in the 3rd (last_ symbol of the special subframe
	// so the position wrt to the start of the frame is 
	sync_pos_slot = OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*2+2) + 10;
	
	//sync_pos = sync_pos_slot;
	
	msg("eNb_id = %d, sync_pos = %d, sync_pos_slot =%d\n", PHY_vars_UE->lte_ue_common_vars.eNb_id, sync_pos, sync_pos_slot);
	  
	if (((sync_pos - sync_pos_slot) >=0 ) && 
	    ((sync_pos - sync_pos_slot) < (FRAME_LENGTH_COMPLEX_SAMPLES - PHY_vars_eNb->lte_frame_parms.samples_per_tti)) ) {
	  
	  for (l=0;l<PHY_vars_eNb->lte_frame_parms.symbols_per_tti;l++) {
	    
	    subframe_offset = (l/PHY_vars_eNb->lte_frame_parms.symbols_per_tti)*PHY_vars_eNb->lte_frame_parms.samples_per_tti;
	    //	    printf("subframe_offset = %d\n",subframe_offset);
	      
	    slot_fep(PHY_vars_UE,
		     l%(PHY_vars_eNb->lte_frame_parms.symbols_per_tti/2),
		     l/(PHY_vars_eNb->lte_frame_parms.symbols_per_tti/2),
		     sync_pos-sync_pos_slot,
		     0);
	      

	      if (l==0) {
		lte_ue_measurements(PHY_vars_UE,
				    0,
				    1,
				    0);
		
		if (trial%100 == 0) {
		  msg("[PHY_PROCEDURES_LTE] frame %d, RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), RX gain %d dB\n",
		      trial,
		      PHY_vars_UE->PHY_measurements.rx_rssi_dBm[0], 
		      PHY_vars_UE->PHY_measurements.wideband_cqi_dB[0][0],
		      PHY_vars_UE->PHY_measurements.wideband_cqi_dB[0][1],
		      PHY_vars_UE->PHY_measurements.wideband_cqi[0][0],
		      PHY_vars_UE->PHY_measurements.wideband_cqi[0][1],
		      PHY_vars_UE->rx_total_gain_dB);
		  
		  msg("[PHY_PROCEDURES_LTE] frame %d, N0 digital (%d, %d) dB, linear (%d, %d)\n",
		      trial,
		      PHY_vars_UE->PHY_measurements.n0_power_dB[0],
		      PHY_vars_UE->PHY_measurements.n0_power_dB[1],
		      PHY_vars_UE->PHY_measurements.n0_power[0],
		      PHY_vars_UE->PHY_measurements.n0_power[1]);
		  
		  msg("[PHY_PROCEDURES_LTE] frame %d, freq_offset_filt = %d\n",
		      trial, freq_offset);
		  
		}
		   
		
		lte_adjust_synch(&PHY_vars_UE->lte_frame_parms,
				 PHY_vars_UE,
				 0,
				 1,
				 16384);
	      }
	      
	      if (l==((PHY_vars_eNb->lte_frame_parms.Ncp==0)?4:3)) {
		//sprintf(fname,"dl_ch00_%d.m",l);
		//sprintf(vname,"dl_ch00_%d",l);
		//write_output(fname,vname,&(lte_ue_common_vars->dl_ch_estimates[0][lte_frame_parms->ofdm_symbol_size*(l%6)]),lte_frame_parms->ofdm_symbol_size,1,1);
		
		lte_est_freq_offset(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0],
				    &PHY_vars_UE->lte_frame_parms,
				    l,
				    &freq_offset);
	      }
	      
	      if (l==((PHY_vars_eNb->lte_frame_parms.Ncp==0)?10:9)) {
		
		for (frame_mod4=0;frame_mod4<4;frame_mod4++) {
		  pbch_tx_ant = rx_pbch(&PHY_vars_UE->lte_ue_common_vars,
					PHY_vars_UE->lte_ue_pbch_vars[0],
					&PHY_vars_UE->lte_frame_parms,
					0,
					SISO,
					frame_mod4);
		  if ((pbch_tx_ant>0) && (pbch_tx_ant<4)) {
		    PHY_vars_UE->lte_frame_parms.mode1_flag = 1;
		    break;
		  }

		  pbch_tx_ant = rx_pbch(&PHY_vars_UE->lte_ue_common_vars,
					PHY_vars_UE->lte_ue_pbch_vars[0],
					&PHY_vars_eNb->lte_frame_parms,
					0,
					ALAMOUTI,
					frame_mod4);
		  if ((pbch_tx_ant>0) && (pbch_tx_ant<4)) {
		    PHY_vars_UE->lte_frame_parms.mode1_flag = 0;
		    n_alamouti++;
		    break;
		  }
		}


		if ((pbch_tx_ant>0) && (pbch_tx_ant<4)) {
		  if (n_frames==1)
		    msg("pbch decoded sucessfully mode1_flag %d, frame_mod4 %d, tx_ant %d!\n",
			PHY_vars_UE->lte_frame_parms.mode1_flag,frame_mod4,pbch_tx_ant);
		}
		else {
		  n_errors++;
		  n_errors2++;
		  if (n_frames==1)
		    msg("pbch error\n");
		}
	      }
	    }
	  }
      } //noise trials
      if (abstraction_flag==1) {
	printf("SNR %f : n_errors = %d/%d, n_alamouti %d\n", SNR,n_errors,ntrials,n_alamouti);
	if (write_output_file==1)
	  fprintf(output_fd,"%f %f %e %e\n",SNR,pbch_sinr,(double)n_errors/ntrials,pbch_bler(pbch_sinr));
      }
      n_errors=0;
      if ((abstraction_flag==0) && (n_errors2>1000) && (trial>5000))
	break;
    } // trials
    if (abstraction_flag==0) {
      printf("SNR %f : n_errors2 = %d/%d (BLER %e,40ms BLER %e,%d,%d), n_alamouti %d\n", SNR,n_errors2,ntrials*(1+trial),(double)n_errors2/(ntrials*(1+trial)),pow((double)n_errors2/(ntrials*(1+trial)),4),ntrials,trial,n_alamouti);
      if (write_output_file==1)
	fprintf(output_fd,"%f %e\n",SNR,(double)n_errors2/(ntrials*(1+trial)));
    }
  } // NSR

  if (n_frames==1) {

    write_output("H00.m","h00",&(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][0][0]),((frame_parms->Ncp==0)?7:6)*(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size),1,1);
    if (n_tx==2)
      write_output("H10.m","h10",&(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][2][0]),((frame_parms->Ncp==0)?7:6)*(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size),1,1);
    write_output("rxsig0.m","rxs0", PHY_vars_UE->lte_ue_common_vars.rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("rxsigF0.m","rxsF0", PHY_vars_UE->lte_ue_common_vars.rxdataF[0],NUMBER_OF_OFDM_CARRIERS*2*((frame_parms->Ncp==0)?14:12),2,1);    
    write_output("PBCH_rxF0_ext.m","pbch0_ext",PHY_vars_UE->lte_ue_pbch_vars[0]->rxdataF_ext[0],12*4*6,1,1);
    write_output("PBCH_rxF0_comp.m","pbch0_comp",PHY_vars_UE->lte_ue_pbch_vars[0]->rxdataF_comp[0],12*4*6,1,1);
    write_output("PBCH_rxF_llr.m","pbch_llr",PHY_vars_UE->lte_ue_pbch_vars[0]->llr,(frame_parms->Ncp==0) ? 1920 : 1728,1,4);
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

  if (write_output_file)
    fclose(output_fd);
  return(n_errors);

}
   


/*  
  for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
	   (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
	   12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/

