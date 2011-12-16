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

int current_dlsch_cqi; //FIXME! 

PHY_VARS_eNB *PHY_vars_eNB;
PHY_VARS_UE *PHY_vars_UE;

DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;
#define DLSCH_RB_ALLOC 0x1fbf // igore DC component,RB13



void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,unsigned char extended_prefix_flag,u16 Nid_cell,u8 N_RB_DL,u8 osf) {

  unsigned int ind;
  LTE_DL_FRAME_PARMS *lte_frame_parms;

  printf("Start lte_param_init\n");
  PHY_vars_eNB = malloc(sizeof(PHY_VARS_eNB));

  PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  //PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  randominit(0);
  set_taus_seed(0);
  
  lte_frame_parms = &(PHY_vars_eNB->lte_frame_parms);

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

  phy_init_lte_ue(&PHY_vars_UE->lte_frame_parms,
		  &PHY_vars_UE->lte_ue_common_vars,
		  PHY_vars_UE->lte_ue_dlsch_vars,
		  PHY_vars_UE->lte_ue_dlsch_vars_SI,
		  PHY_vars_UE->lte_ue_dlsch_vars_ra,
		  PHY_vars_UE->lte_ue_pbch_vars,
		  PHY_vars_UE->lte_ue_pdcch_vars,
		  PHY_vars_UE,0);

  phy_init_lte_eNB(&PHY_vars_eNB->lte_frame_parms,
		   &PHY_vars_eNB->lte_eNB_common_vars,
		   PHY_vars_eNB->lte_eNB_ulsch_vars,
		   0,
		   PHY_vars_eNB,
		   0,
		   0);

  phy_init_lte_top(lte_frame_parms);

  


  printf("Done lte_param_init\n");

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
  double **s_re,**s_im,**r_re,**r_im;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=0.0000005,Td=.8,iqim=0.0;
  u8 channel_length,nb_taps=8;
  int sync_pos, sync_pos_slot;
  FILE *rx_frame_file,*output_fd;
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
  channel_desc_t *UE2eNB;
  u32 nsymb,tx_lev,tx_lev_dB;
  u8 extended_prefix_flag=0;
  s8 interf1=-19,interf2=-19;
  LTE_DL_FRAME_PARMS *frame_parms;
#ifdef EMOS
  fifo_dump_emos emos_dump;
#endif

  char input_val_str[50],input_val_str2[50];
  double input_val1,input_val2;
  u16 amask=0;
  u8 frame_mod4,num_pdcch_symbols;
  u16 NB_RB=25;

  SCM_t channel_model=custom;

  DCI_ALLOC_t dci_alloc[8];
  u8 abstraction_flag=0,calibration_flag=0;
  double pucch_sinr;
  u8 osf=1,N_RB_DL=25;
  u32 pucch_payload_tx,pucch_payload_rx,pucch_tx=0,pucch1_missed=0,pucch1_false=0,sig;
  PUCCH_FMT_t pucch_format = pucch_format1;
  PUCCH_CONFIG_DEDICATED pucch_config_dedicated;
  u8 subframe=3;

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
  while ((c = getopt (argc, argv, "haA:Cr:pf:g:i:j:n:s:S:t:x:y:z:N:F:")) != -1)
    {
      switch (c)
	{
	case 'a':
	  printf("Running AWGN simulation\n");
	  awgn_flag = 1;
	  ntrials=1;
	  break;
	case 'f':
	  if (atoi(optarg)==0)
	    pucch_format = pucch_format1;
	  else if (atoi(optarg)==1)
	    pucch_format = pucch_format1a;
	  else if (atoi(optarg)==2)
	    pucch_format = pucch_format1b;
	  else {
	    printf("Unsupported pucch_format %d\n",atoi(optarg));
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
	  printf("-f PUCCH format (0=1,1=1a,2=1b), formats 2/2a/2b not supported\n");
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
  txdata = PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id];
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

    UE2eNB = new_channel_desc(n_tx,
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
    UE2eNB = new_channel_desc_scm(PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,
				  PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				  channel_model,
				  BW,
				  .999,
				  0,
				  0);
  }

  if (UE2eNB==NULL) {
    msg("Problem generating channel model. Exiting.\n");
    exit(-1);
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
 
  init_ncs_cell(&PHY_vars_eNB->lte_frame_parms,PHY_vars_eNB->ncs_cell);

  init_ncs_cell(&PHY_vars_UE->lte_frame_parms,PHY_vars_UE->ncs_cell);
 
  PHY_vars_eNB->lte_frame_parms.pucch_config_common.deltaPUCCH_Shift = 1;
  PHY_vars_eNB->lte_frame_parms.pucch_config_common.nRB_CQI          = 1;
  PHY_vars_eNB->lte_frame_parms.pucch_config_common.nCS_AN           = 0;

  PHY_vars_UE->lte_frame_parms.pucch_config_common.deltaPUCCH_Shift = 1;
  PHY_vars_UE->lte_frame_parms.pucch_config_common.nRB_CQI          = 1;
  PHY_vars_UE->lte_frame_parms.pucch_config_common.nCS_AN           = 0;

  generate_pucch(PHY_vars_UE->lte_ue_common_vars.txdataF,
		 frame_parms,
		 PHY_vars_UE->ncs_cell,
		 pucch_format,
		 &pucch_config_dedicated,
		 37, //n1_pucch,
		 0, //n2_pucch,
		 0, //shortened_format,
		 1, //payload,
		 scfdma_amps[1], //amp,
		 subframe); //subframe
#ifdef IFFT_FPGA_UE  
  tx_lev=0;
  
  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
    
    if (frame_parms->Ncp == 1) 
      PHY_ofdm_mod(txdataF2[aa],        // input
		   txdata[aa],         // output
		   frame_parms->log2_symbol_size,                // log2_fft_size
		   2*nsymb,                 // number of symbols
		   frame_parms->nb_prefix_samples,               // number of prefix samples
		   frame_parms->twiddle_ifft,  // IFFT twiddle factors
		   frame_parms->rev,           // bit-reversal permutation
		   CYCLIC_PREFIX);
    else 
      normal_prefix_mod(txdataF2[aa],txdata[aa],2*nsymb,frame_parms);
  }
  tx_lev += signal_energy(&txdata[aa][OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
			  OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
    
#else

    write_output("txsigF0.m","txsF0", &PHY_vars_UE->lte_ue_common_vars.txdataF[0][512*nsymb*2],512*nsymb,1,1);

    tx_lev = 0;
    
    
    
    for (aa=0; aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx; aa++) {
      if (frame_parms->Ncp == 1) 
	PHY_ofdm_mod(&PHY_vars_UE->lte_ue_common_vars.txdataF[aa][2*subframe*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX],        // input,
		     &txdata[aa][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe],         // output
		     frame_parms->log2_symbol_size,                // log2_fft_size
		     nsymb,                 // number of symbols
		     frame_parms->nb_prefix_samples,               // number of prefix samples
		     frame_parms->twiddle_ifft,  // IFFT twiddle factors
		     frame_parms->rev,           // bit-reversal permutation
		     CYCLIC_PREFIX);
      else {
	normal_prefix_mod(&PHY_vars_UE->lte_ue_common_vars.txdataF[eNB_id][subframe*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX],
			  &txdata[aa][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe],
			  nsymb,
			  frame_parms);
      }
      
      tx_lev += signal_energy(&txdata[aa][subframe*PHY_vars_eNB->lte_frame_parms.samples_per_tti],
			      OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
    }
#endif
    
    
    tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
    
    //  write_output("txsig0.m","txs0", txdata[0],2*frame_parms->samples_per_tti,1,1);
    //write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);

    // multipath channel
  
  for (i=0;i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
    for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx;aa++) {
      if (awgn_flag == 0) {
	s_re[aa][i] = ((double)(((short *)&txdata[aa][subframe*frame_parms->samples_per_tti]))[(i<<1)]);
	s_im[aa][i] = ((double)(((short *)&txdata[aa][subframe*frame_parms->samples_per_tti]))[(i<<1)+1]);
      }
      else {
	for (aarx=0;aarx<PHY_vars_UE->lte_frame_parms.nb_antennas_rx;aarx++) {
	  if (aa==0) {
	    r_re[aarx][i] = ((double)(((short *)&txdata[aa][subframe*frame_parms->samples_per_tti]))[(i<<1)]);
	    r_im[aarx][i] = ((double)(((short *)&txdata[aa][subframe*frame_parms->samples_per_tti]))[(i<<1)+1]);
	  }
	  else {
	    r_re[aarx][i] += ((double)(((short *)&txdata[aa][subframe*frame_parms->samples_per_tti]))[(i<<1)]);
	    r_im[aarx][i] += ((double)(((short *)&txdata[aa][subframe*frame_parms->samples_per_tti]))[(i<<1)+1]);
	  }
	}
      }
    }
  }



  for (SNR=snr0;SNR<snr1;SNR+=.2) {

    printf("n_frames %d SNR %f\n",n_frames,SNR);

    n_errors = 0;
    n_errors2 = 0;
    n_alamouti = 0;
    pucch_tx = 0;
      pucch1_missed=0;
      pucch1_false=0;

    for (trial=0; trial<n_frames; trial++) {
      
      pucch_sinr=0.0;
      if (abstraction_flag==1)
	printf("*********************** trial %d ***************************\n",trial);

      while (pucch_sinr>-2.0) {

	if (awgn_flag == 0) {	

	  multipath_channel(UE2eNB,s_re,s_im,r_re,r_im,
			    2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
	  /*	  
	  if (abstraction_flag == 1) {
	    freq_channel(UE2eNB,25);
	    pucch_sinr = compute_pucch_sinr(UE2eNB,NULL,NULL,SNR,SNR+interf1,SNR+interf2,25);
	    printf("total_sinr %f\n",compute_sinr(UE2eNB,UE2eNB1,UE2eNB2,SNR,SNR+interf1,SNR+interf2,25));
	    printf("pucch_sinr %f => BLER %f\n",pucch_sinr,pucch_bler(pucch_sinr));
	  }
	  else {
	    pucch_sinr = -3.0;
	  }
	  */
	  pucch_sinr = -3.0;
	  //	  exit(-1);
	} // awgn_flag
	else {
	  pucch_sinr = -3.0;
	}
      }
      
      sigma2_dB = 10*log10((double)tx_lev) +10*log10(PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size/(12*NB_RB)) - SNR;
      if (n_frames==1)
	printf("sigma2_dB %f (SNR %f dB) tx_lev_dB %f\n",sigma2_dB,SNR,10*log10((double)tx_lev));
      //AWGN
      sigma2 = pow(10,sigma2_dB/10);
      //	printf("Sigma2 %f (sigma2_dB %f)\n",sigma2,sigma2_dB);
            
      if (n_frames==1) {
	printf("rx_level data symbol %f, tx_lev %f\n",
	       10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0)),
	       10*log10(tx_lev));
      }

      if ((pucch_format == pucch_format1) && ((taus()&1) == 0))
	sig=0;
      else {
	sig=1;
	pucch_tx++;
      }
      
      for (n_trials=0;n_trials<ntrials;n_trials++) {
	//printf("n_trial %d\n",n_trials);
	for (i=0; i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES; i++) {
	  for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_rx;aa++) {
	    if (n_trials==0) {
	      //		r_re[aa][i] += (pow(10.0,.05*interf1)*r_re1[aa][i] + pow(10.0,.05*interf2)*r_re2[aa][i]);
	      //		r_im[aa][i] += (pow(10.0,.05*interf1)*r_im1[aa][i] + pow(10.0,.05*interf2)*r_im2[aa][i]);
	    }


	    if (sig==1) {
	      ((short*) &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][aa][subframe*frame_parms->samples_per_tti])[2*i] = (short) (.167*(r_re[aa][i] +sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	      ((short*) &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][aa][subframe*frame_parms->samples_per_tti])[2*i+1] = (short) (.167*(r_im[aa][i] + (iqim*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	    }
	    else {
	      ((short*) &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][aa][subframe*frame_parms->samples_per_tti])[2*i] = (short) (.167*(sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	      ((short*) &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][aa][subframe*frame_parms->samples_per_tti])[2*i+1] = (short) (.167*(sqrt(sigma2/2)*gaussdouble(0.0,1.0)));

	    }
	  }
	}

	for (l=0;l<PHY_vars_eNB->lte_frame_parms.symbols_per_tti/2;l++) {
	  
	  slot_fep_ul(&PHY_vars_eNB->lte_frame_parms,
		      &PHY_vars_eNB->lte_eNB_common_vars,
		      l,
		      subframe*2,// slot
		      0, 
		      0
		      );
	  slot_fep_ul(&PHY_vars_eNB->lte_frame_parms,
		      &PHY_vars_eNB->lte_eNB_common_vars,
		      l,
		      1+(subframe*2),//slot
		      0, 
		      0
		      );


	  }
	}
      
      rx_pucch(&PHY_vars_eNB->lte_eNB_common_vars,
	       frame_parms,
	       PHY_vars_eNB->ncs_cell,
	       pucch_format,
	       &pucch_config_dedicated,
	       37, //n1_pucch,
	       0, //n2_pucch,
	       0, //shortened_format,
	       &pucch_payload_rx, //payload,
	       subframe); //subframe	       
      if (pucch_format==pucch_format1) {
	pucch1_missed = (pucch_payload_rx == 0) && (sig==1) ? (pucch1_missed+1) : pucch1_missed;
	pucch1_false  = (pucch_payload_rx == 1) && (sig==0) ? (pucch1_false+1) : pucch1_false;
      }
      else {
	pucch1_false = (pucch_payload_rx != 1) ? (pucch1_false+1) : pucch1_false;
      }
      //      printf("sig %d\n",sig);
    } // NSR
    if (pucch_format==pucch_format1)
      printf("pucch_trials %d : pucch1_false %d,pucch1_missed %d\n",pucch_tx,pucch1_false,pucch1_missed);
    else if (pucch_format==pucch_format1a)
      printf("pucch_trials %d : pucch1a_errors %d\n",pucch_tx,pucch1_false);
    else if (pucch_format==pucch_format1b)
      printf("pucch_trials %d : pucch1a_errors %d\n",pucch_tx,pucch1_false);

  }
  if (n_frames==1) {
    write_output("txsig0.m","txs0", &txdata[0][subframe*frame_parms->samples_per_tti],frame_parms->samples_per_tti,1,1);
    write_output("rxsig0.m","rxs0", &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][subframe*frame_parms->samples_per_tti],frame_parms->samples_per_tti,1,1);
    write_output("rxsigF0.m","rxsF0", &PHY_vars_eNB->lte_eNB_common_vars.rxdataF[0][0][0],512*nsymb*2,2,1);
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

  return(n_errors);

}
   


/*  
  for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
	   (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
	   12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/

