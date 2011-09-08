#include <string.h>
#include <math.h>
#include <unistd.h>
#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "SCHED/extern.h"

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

#include "LAYER2/MAC/defs.h"
#include "PHY_INTERFACE/defs.h"
#include "LAYER2/MAC/vars.h"

#define BW 5.0

int current_dlsch_cqi; //FIXME! 

DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;
#define DLSCH_RB_ALLOC 0xf // igore DC component,RB13

void eNB_scheduler(u8 Mod_id,u8 subframe) {

  msg("Doing Scheduler for eNB, subframe %d\n",subframe);
}

DCI_PDU DCI_pdu;

DCI_PDU *get_dci(u8 Mod_id,u8 subframe) {
  msg("Getting DCI, subframe %d\n",subframe);
  DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_dci;

  DCI_pdu.Num_common_dci = 0;
  if (subframe==0) {
    DCI_pdu.Num_ue_spec_dci=0;
  }
  else {
    DCI_pdu.Num_ue_spec_dci = 3;
    DCI_pdu.dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
    DCI_pdu.dci_alloc[0].L          = 1;
    DCI_pdu.dci_alloc[0].rnti       = 1;
    DCI_pdu.dci_alloc[0].format     = format2_2A_M10PRB;
    
    DCI_pdu.dci_alloc[1].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
    DCI_pdu.dci_alloc[1].L          = 1;
    DCI_pdu.dci_alloc[1].rnti       = 2;
    DCI_pdu.dci_alloc[1].format     = format2_2A_M10PRB;
    
    DCI_pdu.dci_alloc[2].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
    DCI_pdu.dci_alloc[2].L          = 1;
    DCI_pdu.dci_alloc[2].rnti       = 3;
    DCI_pdu.dci_alloc[2].format     = format2_2A_M10PRB;

    DLSCH_dci.rah  = 0;
    DLSCH_dci.ndi1 = 1;
    DLSCH_dci.mcs1   = 6;
    DLSCH_dci.rballoc = 0xf;
    DLSCH_dci.tpmi    = 0;
    DLSCH_dci.rv1     = 0;
    
    memcpy((void*)&DCI_pdu.dci_alloc[0].dci_pdu[0],(void *)&DLSCH_dci,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
    DLSCH_dci.rballoc = 0xf0;
    DLSCH_dci.mcs1    = 12;
    memcpy((void *)&DCI_pdu.dci_alloc[1].dci_pdu[0],(void *)&DLSCH_dci,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
    
    DLSCH_dci.rballoc = 0xf00;
    DLSCH_dci.mcs1    = 20;
    memcpy((void*)&DCI_pdu.dci_alloc[2].dci_pdu[0],(void *)&DLSCH_dci,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
  }

  return(&DCI_pdu);
}

u8 DLSCH_pdu[768*8];

u8 *get_DLSCH_pdu(u8 Mod_id,u16 rnti,u8 abstraction_flag) {

  return(DLSCH_pdu);
}

//u8 NB_UE_INST = 3;

void lte_param_init(unsigned char N_tx, 
		    unsigned char N_rx,
		    unsigned char transmission_mode,
		    unsigned char extended_prefix_flag,
		    u16 Nid_cell,
		    u8 N_RB_DL,
		    u8 osf,
		    u8 frame_type,
		    u8 tdd_config) {

  unsigned int ind,i,j;
  LTE_DL_FRAME_PARMS *lte_frame_parms;


  printf("Start lte_param_init\n");
  PHY_vars_eNB_g = malloc(sizeof(PHY_VARS_eNB*));
  PHY_vars_eNB_g[0] = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_UE_g = malloc(sizeof(PHY_VARS_UE*));
  PHY_vars_UE_g[0] = malloc(sizeof(PHY_VARS_UE));


  mac_xface = malloc(sizeof(MAC_xface));


  mac_xface->eNB_dlsch_ulsch_scheduler = eNB_scheduler;
  mac_xface->get_dci_sdu = get_dci;
  mac_xface->get_dlsch_sdu = get_DLSCH_pdu;

  randominit(0);
  set_taus_seed(0);
  
  lte_frame_parms = &(PHY_vars_eNB_g[0]->lte_frame_parms);
  lte_frame_parms->frame_type         = frame_type;
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
  lte_frame_parms->tdd_config = tdd_config=3;
  lte_frame_parms->phich_config_common.phich_resource = oneSixth;
  init_frame_parms(lte_frame_parms,osf);
  
  
  phy_init_top(lte_frame_parms); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  PHY_vars_UE_g[0]->lte_frame_parms = *lte_frame_parms;

  phy_init_lte_top(lte_frame_parms);

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    for (j=0;j<2;j++) {
      PHY_vars_eNB_g[0]->dlsch_eNB[i][j] = new_eNB_dlsch(1,8,0);
      if (!PHY_vars_eNB_g[0]->dlsch_eNB[i][j]) {
	msg("Can't get eNb dlsch structures\n");
	exit(-1);
      }
      else {
	msg("dlsch_eNB[%d][%d] => %p\n",i,j,PHY_vars_eNB_g[0]->dlsch_eNB[i][j]);
	PHY_vars_eNB_g[0]->dlsch_eNB[i][j]->rnti=i+1;
      }
    }
    PHY_vars_eNB_g[0]->ulsch_eNB[1+i] = new_eNB_ulsch(3,0);
    if (!PHY_vars_eNB_g[0]->ulsch_eNB[1+i]) {
      msg("Can't get eNb ulsch structures\n");
      exit(-1);
    }
    
  }
  
  // ULSCH for RA
  PHY_vars_eNB_g[0]->ulsch_eNB[0] = new_eNB_ulsch(3,0);
  if (!PHY_vars_eNB_g[0]->ulsch_eNB[0]) {
    msg("Can't get eNb ulsch structures\n");
    exit(-1);
  }
  
  PHY_vars_eNB_g[0]->dlsch_eNB_SI  = new_eNB_dlsch(1,1,0);
  printf("eNB %d : SI %p\n",0,PHY_vars_eNB_g[0]->dlsch_eNB_SI);
  PHY_vars_eNB_g[0]->dlsch_eNB_ra  = new_eNB_dlsch(1,1,0);
  printf("eNB %d : RA %p\n",0,PHY_vars_eNB_g[0]->dlsch_eNB_ra);
  
  PHY_vars_eNB_g[0]->rx_total_gain_eNB_dB=150;


  phy_init_lte_ue(&PHY_vars_UE_g[0]->lte_frame_parms,
		  &PHY_vars_UE_g[0]->lte_ue_common_vars,
		  PHY_vars_UE_g[0]->lte_ue_dlsch_vars,
		  PHY_vars_UE_g[0]->lte_ue_dlsch_vars_SI,
		  PHY_vars_UE_g[0]->lte_ue_dlsch_vars_ra,
		  PHY_vars_UE_g[0]->lte_ue_pbch_vars,
		  PHY_vars_UE_g[0]->lte_ue_pdcch_vars,
		  PHY_vars_UE_g[0],0);

  phy_init_lte_eNB(&PHY_vars_eNB_g[0]->lte_frame_parms,
		   &PHY_vars_eNB_g[0]->lte_eNB_common_vars,
		   PHY_vars_eNB_g[0]->lte_eNB_ulsch_vars,
		   0,
		   PHY_vars_eNB_g[0],
		   0,
		   0);

  phy_init_lte_top(lte_frame_parms);

  printf("Done lte_param_init, txdataF %p\n",PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0]);


}


int main(int argc, char **argv) {

  char c;

  int i,l,aa,aarx,sector;
  double sigma2, sigma2_dB=0,SNR,snr0=-2.0,snr1;
  u8 snr1set=0;
  //mod_sym_t **txdataF;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  int **txdata,**txdata1,**txdata2;
  double **s_re,**s_im,**s_re1,**s_im1,**s_re2,**s_im2,**r_re,**r_im,**r_re1,**r_im1,**r_re2,**r_im2;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=0.0000005,Td=.8,iqim=0.0;
  u8 channel_length,nb_taps=8;
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
  unsigned char eNB_id = 0;
  u16 Nid_cell=0;

  u8 awgn_flag=0;
  double nf[2] = {3.0,3.0}; //currently unused
  double ip =0.0;
  double N0W, path_loss, path_loss_dB;
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

  SCM_t channel_model=custom;

  DCI_ALLOC_t dci_alloc[8];
  u8 abstraction_flag=0,calibration_flag=0;
  double pbch_sinr;
  int pbch_tx_ant;
  u8 N_RB_DL=25,osf=1;
  u32 frame;
  s8 slot,next_slot,last_slot;
  u8 tdd_config,frame_type;
  u32 slot_offset,slot_offset2;

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
	case 'd':
	  frame_type = atoi(optarg);
	  break;
	case 'T':
	  tdd_config = atoi(optarg);
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
	  printf("%s -h(elp) -a(wgn on) -d duplexing -T tdd_config -p(extended_prefix) -N cell_id -f output_filename -F input_filename -g channel_model -n n_frames -t Delayspread -r Ricean_FactordB -s snr0 -S snr1 -x transmission_mode -y TXant -z RXant -i Intefrence0 -j Interference1 -A interpolation_file -C(alibration offset dB) -N CellId\n",argv[0]);
	  printf("-h This message\n");
	  printf("-a Use AWGN channel and not multipath\n");
	  printf("-d Duplexing for transmission 0 = FDD (default), 1 TDD\n");
	  printf("-T TDD configuration for transmission\n");
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

  if (write_output_file == 0) {
    msg("Please provide an output filename\n");
    exit(-1);
  }
  if (transmission_mode==2)
    n_tx=2;

  lte_param_init(n_tx,n_rx,transmission_mode,extended_prefix_flag,Nid_cell,N_RB_DL,osf,frame_type,tdd_config);


  if (snr1set==0) {
    if (n_frames==1)
      snr1 = snr0+.1;
    else
      snr1 = snr0+5.0;
  }

  printf("SNR0 %f, SNR1 %f\n",snr0,snr1);

  frame_parms = &PHY_vars_eNB_g[0]->lte_frame_parms;


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
  txdata = PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0];
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
			      .999,
			      0,
			      0,
			      0);
    
    if (interf1>-20)
      eNB2UE1 = new_channel_desc(n_tx,
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
				 .999,
				 0,
				 0,
				 0);
    if (interf2>-20)
      eNB2UE2 = new_channel_desc(n_tx,
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
				 .999,
				 0,
				 0,
				 0);
  }
  else {
    msg("[SIM] Using SCM/101\n");
    eNB2UE = new_channel_desc_scm(PHY_vars_eNB_g[0]->lte_frame_parms.nb_antennas_tx,
				  PHY_vars_UE_g[0]->lte_frame_parms.nb_antennas_rx,
				  channel_model,
				  BW,
				  .999,
				  0,
				  0);

    if (interf1>-20)
      eNB2UE1 = new_channel_desc_scm(PHY_vars_eNB_g[0]->lte_frame_parms.nb_antennas_tx,
				  PHY_vars_UE_g[0]->lte_frame_parms.nb_antennas_rx,
				  channel_model,
				  BW,
				  .999,
				  0,
				  0);

    if (interf2>-20)
      eNB2UE2 = new_channel_desc_scm(PHY_vars_eNB_g[0]->lte_frame_parms.nb_antennas_tx,
				    PHY_vars_UE_g[0]->lte_frame_parms.nb_antennas_rx,
				    channel_model,
				    BW,
				    .999,
				    0,
				    0);

  }

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

  /*  
  phy_procedures_eNB_lte(18,0,PHY_vars_eNB_g[0],0);
  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
    if (frame_parms->Ncp == 1)
      PHY_ofdm_mod(&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][aa][0],        // input
		   &PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][0],         // output
		   frame_parms->log2_symbol_size,                // log2_fft_size
		   6,                 // number of symbols
		   frame_parms->nb_prefix_samples,               // number of prefix samples
		   frame_parms->twiddle_ifft,  // IFFT twiddle factors
		   frame_parms->rev,           // bit-reversal permutation
		   CYCLIC_PREFIX);
    else {
      normal_prefix_mod(&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][aa][0],
			&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][0],
			7,
			frame_parms);
	  }
  }  
  */
  
  for (frame=0;frame<n_frames;frame++) {
    mac_xface->frame = frame;
    printf("**********Frame %d*********************\n",frame);
    for (slot=-1;slot<19;slot++) {
      printf("Slot %d\n",slot);

      last_slot = (slot - 1)%20;
      if (last_slot <0)
	last_slot+=20;
      next_slot = (slot + 1)%20;
      
      phy_procedures_eNB_lte(last_slot,next_slot,PHY_vars_eNB_g[0],0);

      slot_offset = (next_slot)*(frame_parms->ofdm_symbol_size)*((frame_parms->Ncp==1) ? 6 : 7);
      
      slot_offset2 = (next_slot)*(frame_parms->samples_per_tti>>1);
       

      for (i=0;i<48;i++)
	printf("(%d,%d) ",((short *)&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][0][i+slot_offset])[0],
	       ((short *)&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][0][i+slot_offset])[0]);
      for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
	if (frame_parms->Ncp == 1)
	  PHY_ofdm_mod(&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][aa][slot_offset],        // input
		       &PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][slot_offset2],         // output
		       frame_parms->log2_symbol_size,                // log2_fft_size
		       6,                 // number of symbols
		       frame_parms->nb_prefix_samples,               // number of prefix samples
		       frame_parms->twiddle_ifft,  // IFFT twiddle factors
		       frame_parms->rev,           // bit-reversal permutation
		       CYCLIC_PREFIX);
	  else {
	    normal_prefix_mod(&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][aa][slot_offset],
			      &PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][slot_offset2],
			      7,
			      frame_parms);
	  }
	}  
    }
  
    printf("Writing output file ... txdata = %p\n",PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0]);
    fwrite(PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0],
	   sizeof(int),
	   10*PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti,
	   output_fd);
		 
  }
  fclose(output_fd);

}
