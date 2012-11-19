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

#include "OCG_vars.h"

#define BW 5.0

int current_dlsch_cqi; //FIXME! 

PHY_VARS_eNB *PHY_vars_eNB;
PHY_VARS_UE *PHY_vars_UE;

DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;
#define DLSCH_RB_ALLOC 0x1fbf // igore DC component,RB13



void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,unsigned char extended_prefix_flag,u16 Nid_cell,u8 N_RB_DL,u8 osf) {

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
  lte_frame_parms->tdd_config = 1;
  lte_frame_parms->frame_type = 1;
  init_frame_parms(lte_frame_parms,osf);
  
  //copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(lte_frame_parms); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  PHY_vars_UE->lte_frame_parms = *lte_frame_parms;

  phy_init_lte_top(lte_frame_parms);

  phy_init_lte_ue(PHY_vars_UE,0);

  phy_init_lte_eNB(PHY_vars_eNB,0,0,0);


  


  printf("Done lte_param_init\n");

}

extern u16 prach_root_sequence_map0_3[838];

int main(int argc, char **argv) {

  char c;

  int i,aa,aarx;
  double sigma2, sigma2_dB=0,SNR,snr0=-2.0,snr1=0.0;
  u8 snr1set=0;
  //mod_sym_t **txdataF;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  int **txdata;
  double **s_re,**s_im,**r_re,**r_im;
  double ricean_factor=0.0000005,Td=.8,iqim=0.0;
  u8 channel_length;
  int trial, ntrials=1;
  u8 transmission_mode = 1,n_tx=1,n_rx=1;
  u16 Nid_cell=0;

  u8 awgn_flag=0;
  int n_frames=1;
  channel_desc_t *UE2eNB;
  u32 nsymb,tx_lev,tx_lev_dB;
  u8 extended_prefix_flag=0;
  s8 interf1=-19,interf2=-19;
  LTE_DL_FRAME_PARMS *frame_parms;
#ifdef EMOS
  fifo_dump_emos emos_dump;
#endif


  SCM_t channel_model=Rayleigh1_corr;

  u8 abstraction_flag=0,calibration_flag=0;
  //  double prach_sinr;
  u8 osf=1,N_RB_DL=25;
  u32 prach_errors=0;
  u8 subframe=3;
  u16 preamble_energy_list[64],preamble_tx=99,preamble_delay_list[64];
  u16 preamble_max,preamble_energy_max;
  PRACH_RESOURCES_t prach_resources;
  u8 prach_fmt;
  int N_ZC;

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
  while ((c = getopt (argc, argv, "haA:Cr:p:g:i:j:n:s:S:t:x:y:z:N:F:")) != -1)
    {
      switch (c)
	{
	case 'a':
	  printf("Running AWGN simulation\n");
	  awgn_flag = 1;
	  ntrials=1;
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
	  case 'H':
	    channel_model=Rayleigh8;
	  case 'I':
	    channel_model=Rayleigh1;
	  case 'J':
	    channel_model=Rayleigh1_corr;
	  case 'K':
	    channel_model=Rayleigh1_anticorr;
	  case 'L':
	    channel_model=Rice8;
	  case 'M':
	    channel_model=Rice1;
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
	  preamble_tx=atoi(optarg);
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
	  printf("-f PRACH format (0=1,1=2,2=3,3=4)\n");
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


  txdata = PHY_vars_UE->lte_ue_common_vars.txdata;
  printf("txdata %p\n",&txdata[0][subframe*frame_parms->samples_per_tti]);
  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
  nsymb = (frame_parms->Ncp == 0) ? 14 : 12;

  printf("FFT Size %d, Extended Prefix %d, Samples per subframe %d, Symbols per subframe %d\n",NUMBER_OF_OFDM_CARRIERS,
	 frame_parms->Ncp,frame_parms->samples_per_tti,nsymb);


  
  msg("[SIM] Using SCM/101\n");
  UE2eNB = new_channel_desc_scm(PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,
				PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				channel_model,
				BW,
				0.0,
				0,
				0);
  

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
 
  PHY_vars_UE->lte_frame_parms.prach_config_common.rootSequenceIndex=1; 
  PHY_vars_UE->lte_frame_parms.prach_config_common.prach_ConfigInfo.prach_ConfigIndex=0; 
  PHY_vars_UE->lte_frame_parms.prach_config_common.prach_ConfigInfo.zeroCorrelationZoneConfig=1;
  PHY_vars_UE->lte_frame_parms.prach_config_common.prach_ConfigInfo.highSpeedFlag=0;
  PHY_vars_UE->lte_frame_parms.prach_config_common.prach_ConfigInfo.prach_FreqOffset=0;


  PHY_vars_eNB->lte_frame_parms.prach_config_common.rootSequenceIndex=1; 
  PHY_vars_eNB->lte_frame_parms.prach_config_common.prach_ConfigInfo.prach_ConfigIndex=0; 
  PHY_vars_eNB->lte_frame_parms.prach_config_common.prach_ConfigInfo.zeroCorrelationZoneConfig=1;
  PHY_vars_eNB->lte_frame_parms.prach_config_common.prach_ConfigInfo.highSpeedFlag=0;
  PHY_vars_eNB->lte_frame_parms.prach_config_common.prach_ConfigInfo.prach_FreqOffset=0;

  prach_fmt = get_prach_fmt(PHY_vars_eNB->lte_frame_parms.prach_config_common.prach_ConfigInfo.prach_ConfigIndex,
			    PHY_vars_eNB->lte_frame_parms.frame_type);
  N_ZC = (prach_fmt <4)?839:139;
  
  compute_prach_seq(prach_root_sequence_map0_3[PHY_vars_eNB->lte_frame_parms.prach_config_common.rootSequenceIndex],N_ZC, PHY_vars_eNB->X_u);

  compute_prach_seq(prach_root_sequence_map0_3[PHY_vars_UE->lte_frame_parms.prach_config_common.rootSequenceIndex],N_ZC, PHY_vars_UE->X_u);

  PHY_vars_UE->lte_ue_prach_vars[0]->amp = (s32)scfdma_amps[6];

  PHY_vars_UE->prach_resources[0] = &prach_resources;
  if (preamble_tx == 99)
    preamble_tx = (u16)(taus()&0x3f);
  if (n_frames == 1)
     printf("raPreamble %d\n",preamble_tx);

  PHY_vars_UE->prach_resources[0]->ra_PreambleIndex = preamble_tx;
  PHY_vars_UE->prach_resources[0]->ra_TDD_map_index = 0;

  tx_lev = generate_prach(PHY_vars_UE,
			  0, //eNB_id,
			  subframe, 
			  0); //Nf

  tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
    
  write_output("txsig0_new.m","txs0", &txdata[0][subframe*frame_parms->samples_per_tti],frame_parms->samples_per_tti,1,1);
    //write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);

    // multipath channel
  dump_prach_config(&PHY_vars_eNB->lte_frame_parms,subframe);

  for (i=0;i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
    for (aa=0;aa<1;aa++) {
      if (awgn_flag == 0) {
	s_re[aa][i] = ((double)(((short *)&txdata[aa][subframe*frame_parms->samples_per_tti]))[(i<<1)]);
	s_im[aa][i] = ((double)(((short *)&txdata[aa][subframe*frame_parms->samples_per_tti]))[(i<<1)+1]);
      }
      else {
	for (aarx=0;aarx<PHY_vars_eNB->lte_frame_parms.nb_antennas_rx;aarx++) {
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
    prach_errors=0;
    for (trial=0; trial<n_frames; trial++) {
      
      sigma2_dB = 10*log10((double)tx_lev) - SNR;
      if (n_frames==1)
	printf("sigma2_dB %f (SNR %f dB) tx_lev_dB %f\n",sigma2_dB,SNR,10*log10((double)tx_lev));
      //AWGN
      sigma2 = pow(10,sigma2_dB/10);
      //	printf("Sigma2 %f (sigma2_dB %f)\n",sigma2,sigma2_dB);
            

      if (awgn_flag == 0) {
	multipath_channel(UE2eNB,s_re,s_im,r_re,r_im,
			  2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
      }
      if (n_frames==1) {
	printf("rx_level data symbol %f, tx_lev %f\n",
	       10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0)),
	       10*log10(tx_lev));
      }

      for (i=0; i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES; i++) {
	for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_rx;aa++) {
	
	  ((short*) &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][aa][subframe*frame_parms->samples_per_tti])[2*i] = (short) (.167*(r_re[aa][i] +sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	  ((short*) &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][aa][subframe*frame_parms->samples_per_tti])[2*i+1] = (short) (.167*(r_im[aa][i] + (iqim*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	}
      }
	
      rx_prach(PHY_vars_eNB,
	       subframe,
	       preamble_energy_list,
	       preamble_delay_list,
	       0,   //Nf
	       0);    //tdd_mapindex

      preamble_energy_max = preamble_energy_list[0];
      preamble_max = 0;
      for (i=1;i<64;i++) {
	if (preamble_energy_max < preamble_energy_list[i]) {
	  //	  printf("preamble %d => %d\n",i,preamble_energy_list[i]);
	
	  preamble_energy_max = preamble_energy_list[i];
	  preamble_max = i;
	}
      }
      if (preamble_max!=preamble_tx)
	prach_errors++;
      if (n_frames==1) {
	write_output("prach0.m","prach0", &txdata[0][subframe*frame_parms->samples_per_tti],frame_parms->samples_per_tti,1,1);
	write_output("prachF0.m","prachF0", &PHY_vars_UE->lte_ue_prach_vars[0]->prachF[0],6144,1,1);
	write_output("rxsig0.m","rxs0", 
		     &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][subframe*frame_parms->samples_per_tti],
		     frame_parms->samples_per_tti,1,1);
	write_output("rxsigF0.m","rxsF0", &PHY_vars_eNB->lte_eNB_common_vars.rxdataF[0][0][0],512*nsymb*2,2,1);
	write_output("prach_preamble.m","prachp",&PHY_vars_eNB->X_u[0],839,1,1);
      }
    }
    printf("SNR %f dB: errors %d/%d\n",SNR,prach_errors,n_frames);
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

  return(0);

}
   


/*  
  for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
	   (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
	   12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/

