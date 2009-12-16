#include <string.h>
#include <math.h>
#include "SIMULATION/TOOLS/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif

#define BW 10.0
#define Td 1.0

//#define OUTPUT_DEBUG 1

#define NB_RB 12
#define RBmask0 0x00fc00fc
#define RBmask1 0x0
#define RBmask2 0x0
#define RBmask3 0x0

void lte_param_init(unsigned char N_tx, unsigned char N_rx) {

  unsigned int ind;

  printf("Start lte_param_init\n");
  PHY_vars = malloc(sizeof(PHY_VARS));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  randominit(0);
  set_taus_seed(0);
  
  crcTableInit();


  lte_frame_parms = &(PHY_config->lte_frame_parms);   //openair1/PHY/impl_defs_lte.h
  lte_ue_common_vars = &(PHY_vars->lte_ue_common_vars);
  lte_ue_dlsch_vars = &(PHY_vars->lte_ue_dlsch_vars);
  lte_ue_pbch_vars = &(PHY_vars->lte_ue_pbch_vars);
  lte_eNB_common_vars = &(PHY_vars->lte_eNB_common_vars);
 
  lte_frame_parms->N_RB_DL            = 25;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  lte_frame_parms->first_dlsch_symbol = 2;
  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(N_tx); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  generate_64qam_table();
  generate_16qam_table();
  phy_init_lte_ue(lte_frame_parms,lte_ue_common_vars,lte_ue_dlsch_vars,lte_ue_pbch_vars);//allocation
  phy_init_lte_eNB(lte_frame_parms,lte_eNB_common_vars);
  printf("Done lte_param_init\n");
}


void main(int argc,void **argv) {

  int i,aa,s,ind,Kr,Kr_bytes;;
  double sigma2, sigma2_dB=10,SNR,snr0,snr1,SNRmeas;
  //int **txdataF, **txdata;
  int **txdata;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  //LTE_DL_FRAME_PARMS *frame_parms = (LTE_DL_FRAME_PARMS *)malloc(sizeof(LTE_DL_FRAME_PARMS));
  //LTE_UE_COMMON      *lte_ue_common_vars = (LTE_UE_COMMON *)malloc(sizeof(LTE_UE_COMMON));
  double **s_re,**s_im,**r_re,**r_im;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=0.00005;
  int channel_length;
  struct complex **ch;

  double SE,target_code_rate;

  unsigned char Ns,l,m,mod_order[2]={2,2};
  unsigned int rb_alloc[4];
  MIMO_mode_t mimo_mode;
  unsigned char *input_data,*decoded_output;

  LTE_eNb_DLSCH_t *dlsch_eNb[2];
  LTE_UE_DLSCH_t *dlsch_ue[2];
  unsigned char *input_buffer;
  unsigned short input_buffer_length;
  unsigned int ret;
  unsigned int coded_bits_per_codeword,nsymb;

  unsigned int tx_lev,tx_lev_dB,trials,errs=0,num_layers;
  int re_allocated;
  FILE *bler_fd;
  char bler_fname[20];

  channel_length = (int) 11+2*BW*Td;

  lte_param_init(2,2);

  rb_alloc[0] = RBmask0; // RBs 0-31
  rb_alloc[1] = RBmask1;  // RBs 32-63
  rb_alloc[2] = RBmask2;  // RBs 64-95
  rb_alloc[3] = RBmask3;  // RBs 96-109
  
  num_layers = 1;

  mimo_mode = ANTCYCLING;

  if (argc<2) {
    SE = .66;
  }
  else if (argc<3){
    SE = atof(argv[1]);
  }
  else if (argc<5){
    SE = atof(argv[1]);
    num_layers = atoi(argv[2]);
    if (num_layers == 2)
      mimo_mode = DUALSTREAM;
  }



  /*
  txdataF    = (int **)malloc16(2*sizeof(int*));
  txdataF[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdataF[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  
  txdata    = (int **)malloc16(2*sizeof(int*));
  txdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  */


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
  txdata = lte_eNB_common_vars->txdata;
#endif

  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));

  if (SE <1.5) {
    mod_order[0]=2;
    mod_order[1]=2;
    target_code_rate = SE/2.0;
    snr0=-4;
    snr1=10;
  }
  else if (SE < 2.7) {
    mod_order[0]=4;
    mod_order[1]=4;
    target_code_rate = SE/4.0;
    snr0=0;
    snr1=16;
  }
  else if (SE < 6) {
    mod_order[0]=6;
    mod_order[1]=6;
    target_code_rate = SE/6.0;
    snr0=6;
    snr1=35;
  }

  if (argc==4) {
    snr0 = atof(argv[3]);
    snr1 = snr0+1.0;
  }
  printf("Target code rate %f, mod_order %d, num_layers %d\n",target_code_rate,mod_order[0],num_layers);

  nsymb = (lte_frame_parms->Ncp == 0) ? 14 : 12;

  coded_bits_per_codeword =( NB_RB * (12 * mod_order[0]) * (nsymb-lte_frame_parms->first_dlsch_symbol-3));

  sprintf(bler_fname,"bler_%d.m",(int)(SE*100.0));
  bler_fd = fopen(bler_fname,"w");
  fprintf(bler_fd,"bler = [");

  for (i=0;i<2;i++) {
    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }

  // Create transport channel structures for 2 transport blocks (MIMO)
  for (i=0;i<2;i++) {
    dlsch_eNb[i] = new_eNb_dlsch(1,8);
    dlsch_ue[i]  = new_ue_dlsch(1,8);
  
    if (!dlsch_eNb[i]) {
      printf("Can't get eNb dlsch structures\n");
      exit(-1);
    }
    
    if (!dlsch_ue[i]) {
      printf("Can't get ue dlsch structures\n");
      exit(-1);
    }

    dlsch_eNb[i]->harq_processes[0]->mimo_mode          = mimo_mode;
    dlsch_eNb[i]->layer_index        = i;
    dlsch_eNb[i]->codebook_index     = 0;
    dlsch_eNb[i]->harq_processes[0]->mod_order          = mod_order[i];
    dlsch_eNb[i]->harq_processes[0]->active             = 0;
    dlsch_eNb[i]->harq_processes[0]->Nl                 = num_layers;
    dlsch_eNb[i]->rvidx                                 = 0;
    
    dlsch_ue[i]->harq_processes[0]->mimo_mode           = mimo_mode;
    dlsch_ue[i]->harq_processes[0]->mod_order           = mod_order[i];
    dlsch_eNb[i]->codebook_index     = 0;
    dlsch_ue[i]->layer_index         = i;
    dlsch_ue[i]->harq_processes[0]->active              = 0;
    dlsch_ue[i]->harq_processes[0]->Nl                  = num_layers;
    dlsch_ue[i]->rvidx                                  = 0;
    
  }
  
  //  input_data     = (unsigned char*) malloc(block_length/8);
  //  decoded_output = (unsigned char*) malloc(block_length/8);

  input_buffer_length = ((int)(coded_bits_per_codeword*target_code_rate))>>3;
  printf("Input buffer size %d bytes\n",input_buffer_length);

  input_buffer = (unsigned char *)malloc(input_buffer_length+4);
  
  for (i=0;i<input_buffer_length;i++)
    input_buffer[i]= (unsigned char)(taus()&0xff);


  dlsch_encoding(input_buffer,
		 (input_buffer_length<<3),
		 lte_frame_parms,
		 dlsch_eNb[0],
		 0,               // harq_pid
		 NB_RB); // number of allocated RB

  if (mimo_mode == DUALSTREAM)
    dlsch_encoding(input_buffer,
		   (input_buffer_length<<3),
		   lte_frame_parms,
		   dlsch_eNb[1],
		   0,               // harq_pid
		   NB_RB); // number of allocated RB

#ifdef OUTPUT_DEBUG
  for (i=0;i<32;i++)
    printf("Segment 0 %d : %x\n",i,dlsch_eNb[0]->harq_processes[0]->c[0][i]);
  for (i=0;i<32;i++)
    printf("Segment 1 %d : %x\n",i,dlsch_eNb[0]->harq_processes[0]->c[1][i]);
#endif 

  re_allocated = dlsch_modulation(lte_eNB_common_vars->txdataF,
				  1024,
				  0,
				  lte_frame_parms,
				  dlsch_eNb[0],
				  0,               // harq_pid
				  rb_alloc); // RB allocation vector

  if ((re_allocated/(7*12)) != NB_RB)
    printf("Bad RB count %d (%d)\n",re_allocated,re_allocated/7/12);

  if (num_layers>1)
    re_allocated = dlsch_modulation(lte_eNB_common_vars->txdataF,
				    1024,
				    0,
				    lte_frame_parms,
				    dlsch_eNb[1],
				    0,               // harq_pid
				    rb_alloc); // RB allocation vector

  /* 
     generate_pss(lte_eNB_common_vars->txdataF,
     1024,
     lte_frame_parms,
     LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
  */

  ch = (struct complex**) malloc(1 * 2 * sizeof(struct complex*));
  for (i = 0; i<4; i++)
    ch[i] = (struct complex*) malloc(channel_length * sizeof(struct complex));


  generate_pilots(lte_eNB_common_vars->txdataF,
		  1024,
		  lte_frame_parms,
		  LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
  
  
#ifdef IFFT_FPGA

#ifdef OUTPUT_DEBUG  
  write_output("txsigF0.m","txsF0", lte_eNB_common_vars->txdataF[0],300*120,1,4);
#endif

  // do talbe lookup and write results to txdataF2
  for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
    ind = 0;
    for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX/5;i++) 
      if (((i%512)>=1) && ((i%512)<=150))
	txdataF2[aa][i] = ((int*)mod_table)[lte_eNB_common_vars->txdataF[aa][ind++]];
      else if ((i%512)>=362)
	txdataF2[aa][i] = ((int*)mod_table)[lte_eNB_common_vars->txdataF[aa][ind++]];
      else 
	txdataF2[aa][i] = 0;
    printf("ind=%d\n",ind);
  }

#ifdef OUTPUT_DEBUG  
  write_output("txsigF20.m","txsF20", txdataF2[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
#endif

  tx_lev = 0;
  for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
    PHY_ofdm_mod(txdataF2[aa],        // input
		 txdata[aa],         // output
		 lte_frame_parms->log2_symbol_size,                // log2_fft_size
		 2*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
		 lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		 lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 lte_frame_parms->rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);

    tx_lev += signal_energy(&txdata[aa][2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
			    OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
  }
    
#else //IFFT_FPGA

#ifdef OUTPUT_DEBUG  
  write_output("txsigF0.m","txsF0", lte_eNB_common_vars->txdataF[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX/5,1,1);
#endif
  
  tx_lev = 0;
  for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
    PHY_ofdm_mod(lte_eNB_common_vars->txdataF[aa],        // input
		 txdata[aa],         // output
		 lte_frame_parms->log2_symbol_size,                // log2_fft_size
		 2*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
		 lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		 lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 lte_frame_parms->rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);

    tx_lev += signal_energy(&txdata[aa][OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*2],
			    OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);

  }  
#endif //IFFT_FPGA


  printf("tx_lev = %d\n",tx_lev);
  tx_lev_dB = dB_fixed(tx_lev);


#ifdef OUTPUT_DEBUG  
  write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES/5,1,1);
#endif

  // multipath channel


  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES/5;i++) {
    for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
      s_re[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)]);
      s_im[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)+1]);
    }
  }

  printf("tx_lev_dB = %d\n",tx_lev_dB);
  for (SNR=snr0;SNR<snr1;SNR+=.2) {
    sigma2_dB = tx_lev_dB +10*log10(25/NB_RB) - SNR;
    printf("**********************SNR = %f dB (tx_lev %f, sigma2_dB %f)**************************\n",
	   SNR,
	   (double)tx_lev_dB+10*log10(25/NB_RB),
	   sigma2_dB);
    errs=0;
    printf("Channel attenuation %f\n",(double)tx_lev_dB - (SNR+sigma2_dB));
    for (trials = 0;trials<100000;trials++) {
      multipath_channel(ch,s_re,s_im,r_re,r_im,
			amps,Td,BW,ricean_factor,aoa,
			lte_frame_parms->nb_antennas_tx,
			lte_frame_parms->nb_antennas_rx,
			FRAME_LENGTH_COMPLEX_SAMPLES/5,
			channel_length,0);
			//(double)tx_lev_dB - (SNR+sigma2_dB));
#ifdef OUTPUT_DEBUG
			//	write_output("channel0.m","chan0",ch[0],channel_length,1,8);
#endif
    
	//AWGN
	sigma2 = pow(10,sigma2_dB/10);
	for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES/5; i++) {
	  for (aa=0;aa<lte_frame_parms->nb_antennas_rx;aa++) {
	    ((short*) lte_ue_common_vars->rxdata[aa])[2*i] = (short) (r_re[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	    ((short*) lte_ue_common_vars->rxdata[aa])[2*i+1] = (short) (r_im[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	  }
	}    
	//    lte_sync_time_init(lte_frame_parms,lte_ue_common_vars);
	//    lte_sync_time(lte_ue_common_vars->rxdata, lte_frame_parms);
	//    lte_sync_time_free();
#ifdef OUTPUT_DEBUG
	printf("RX level in null symbol %d\n",dB_fixed(signal_energy(&lte_ue_common_vars->rxdata[0][160+OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	printf("RX level in data symbol %d\n",dB_fixed(signal_energy(&lte_ue_common_vars->rxdata[0][160+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	printf("rx_level Null symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
	printf("rx_level data symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
#endif
	SNRmeas = 10*log10((signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))/signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(1*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))) - 1);
//	printf("SNRmeas %f\n",SNRmeas);
	// Inner receiver scheduling for 3 slots
	for (Ns=0;Ns<3;Ns++) {
	  for (l=0;l<6;l++) {
	
	    slot_fep(lte_frame_parms,
		     lte_ue_common_vars,
		     l,
		     Ns%20,
		     (Ns>>1)*lte_frame_parms->samples_per_tti,
		     0);
	
	    lte_ue_measurements(lte_ue_common_vars,
				lte_frame_parms,
				&PHY_vars->PHY_measurements,
				0);
	    //	printf("rx_avg_power_dB %d\n",PHY_vars->PHY_measurements.rx_avg_power_dB[0]);
	    //	printf("n0_power_dB %d\n",PHY_vars->PHY_measurements.n0_power_dB[0]);
	
	    if ((Ns==0) && (l==3)) // process symbols 0,1,2
	      for (m=lte_frame_parms->first_dlsch_symbol;m<3;m++)
		rx_dlsch(lte_ue_common_vars,
			 lte_ue_dlsch_vars,
			 lte_frame_parms,
			 m,
			 rb_alloc,
			 mod_order,
			 mimo_mode);
	
	    if ((Ns==1) && (l==0)) // process symbols 3,4,5
	      for (m=4;m<6;m++)
		rx_dlsch(lte_ue_common_vars,
			 lte_ue_dlsch_vars,
			 lte_frame_parms,
			 m,
			 rb_alloc,
			 mod_order,
			 mimo_mode);
	    if ((Ns==1) && (l==3)) // process symbols 6,7,8
	      for (m=7;m<9;m++)
		rx_dlsch(lte_ue_common_vars,
			 lte_ue_dlsch_vars,
			 lte_frame_parms,
			 m,
			 rb_alloc,
			 mod_order,
			 mimo_mode);
	
	    if ((Ns==2) && (l==0))  // process symbols 10,11, do deinterleaving for TTI
	      for (m=10;m<12;m++)
		rx_dlsch(lte_ue_common_vars,
			 lte_ue_dlsch_vars,
			 lte_frame_parms,
			 m,
			 rb_alloc,
			 mod_order,
			 mimo_mode);
	
	
	  }
	}

#ifdef OUTPUT_DEBUG      
	write_output("rxsig0.m","rxs0", lte_ue_common_vars->rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
	write_output("dlsch00_ch0.m","dl00_ch0",&(lte_ue_common_vars->dl_ch_estimates[0][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);

	if (num_layers>1) {
	  write_output("dlsch01_ch0.m","dl01_ch0",&(lte_ue_common_vars->dl_ch_estimates[1][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	  write_output("dlsch10_ch0.m","dl10_ch0",&(lte_ue_common_vars->dl_ch_estimates[2][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	  write_output("dlsch11_ch0.m","dl11_ch0",&(lte_ue_common_vars->dl_ch_estimates[3][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	}
	write_output("rxsigF0.m","rxsF0", lte_ue_common_vars->rxdataF[0],2*12*lte_frame_parms->ofdm_symbol_size,2,1);
	write_output("rxsigF0_ext.m","rxsF0_ext", lte_ue_dlsch_vars->rxdataF_ext[0],2*12*lte_frame_parms->ofdm_symbol_size,2,1);
	write_output("dlsch00_ch0_ext.m","dl00_ch0_ext",lte_ue_dlsch_vars->dl_ch_estimates_ext[0],300*12,1,1);
	if (num_layers>1) {
	  write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_dlsch_vars->dl_ch_estimates_ext[1],300*12,1,1);
	  write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_dlsch_vars->dl_ch_estimates_ext[2],300*12,1,1);
	  write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_dlsch_vars->dl_ch_estimates_ext[3],300*12,1,1);
	  write_output("dlsch_rho.m","dl_rho",lte_ue_dlsch_vars->rho[0],300*12,1,1);
	}
	write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0",lte_ue_dlsch_vars->rxdataF_comp[0],300*12,1,1);
	write_output("dlsch_rxF_llr.m","dlsch_llr",lte_ue_dlsch_vars->llr[0],coded_bits_per_codeword,1,0);

	write_output("dlsch_mag1.m","dlschmag1",lte_ue_dlsch_vars->dl_ch_mag,300*12,1,1);
	write_output("dlsch_mag2.m","dlschmag2",lte_ue_dlsch_vars->dl_ch_magb,300*12,1,1);
#endif //OUTPUT_DEBUG
  
	//  printf("Calling decoding, dlsch_ue %p, active %d\n",dlsch_ue,dlsch_ue[0]->harq_processes[0]->active);

	ret = dlsch_decoding(input_buffer_length<<3,
			     lte_ue_dlsch_vars->llr[0],		 
			     lte_frame_parms,
			     dlsch_ue[0],
			     0,               //harq_pid
			     NB_RB);             //NB allocated RBs

	if (ret <= MAX_TURBO_ITERATIONS) {
	//  printf("No DLSCH errors found\n");
	}	
	else
	  errs++;
#ifdef OUTPUT_DEBUG  
	for (s=0;s<dlsch_ue[0]->harq_processes[0]->C;s++) {
	  if (s<dlsch_ue[0]->harq_processes[0]->Cminus)
	    Kr = dlsch_ue[0]->harq_processes[0]->Kminus;
	  else
	    Kr = dlsch_ue[0]->harq_processes[0]->Kplus;
	  
	  Kr_bytes = Kr>>3;

	  printf("Decoded_output (Segment %d):\n",s);
	  for (i=0;i<Kr_bytes;i++)
	    printf("%d : %x (%x)\n",i,dlsch_ue[0]->harq_processes[0]->c[s][i],dlsch_ue[0]->harq_processes[0]->c[s][i]^dlsch_eNb[0]->harq_processes[0]->c[s][i]);
	}
	exit(-1);
#endif
	if (errs==100)
	  break;
    }   //trials
    printf("Errors %d/%d, Pe = %e\n",errs,1+trials,(double)errs/(trials+1));
    fprintf(bler_fd,"%f,%e,\n",SNR,(double)errs/(trials+1));
    if (((double)errs/(trials+1))<1e-2)
      break;
  } // SNR

  fclose(bler_fd);

  printf("Freeing dlsch structures\n");
  for (i=0;i<2;i++) {
    printf("eNb %d\n",i);
    free_eNb_dlsch(dlsch_eNb[i]);
    printf("UE %d\n",i);
    free_ue_dlsch(dlsch_ue[i]);
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


}
   


/*  
    for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
    (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
    12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/

