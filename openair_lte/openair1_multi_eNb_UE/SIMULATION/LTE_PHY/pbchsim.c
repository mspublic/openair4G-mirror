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

#define BW 10.0
#define N_TRIALS 100

int current_dlsch_cqi; //FIXME! 

PHY_VARS_eNB *PHY_vars_eNb;
PHY_VARS_UE *PHY_vars_UE;

void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,unsigned char extended_prefix_flag) {

  unsigned int ind;
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
  lte_frame_parms->Nid_cell           = 1;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  lte_frame_parms->first_dlsch_symbol = 4;
  lte_frame_parms->num_dlsch_symbols  = (lte_frame_parms->Ncp==0) ? 8: 6;
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
  
  lte_gold(lte_frame_parms);
  generate_ul_ref_sigs();
  generate_ul_ref_sigs_rx();
  generate_64qam_table();
  generate_16qam_table();
  generate_RIV_tables();

  generate_pcfich_reg_mapping(lte_frame_parms);
  generate_phich_reg_mapping(lte_frame_parms);

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

  phy_init_lte_top(lte_frame_parms);

  printf("Done lte_param_init\n");


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
  char fname[40], vname[40];
  int trial, n_errors;
  u8 transmission_mode = 1,n_tx=1,n_rx=1;
  unsigned char eNb_id = 0;

  u8 awgn_flag=0;
  double nf[2] = {3.0,3.0}; //currently unused
  double ip =0.0;
  double N0W, path_loss, path_loss_dB;
  int n_frames=1;
  channel_desc_t *eNB2UE;
  u32 nsymb,tx_lev,tx_lev_dB;
  u8 extended_prefix_flag=0;

  LTE_DL_FRAME_PARMS *frame_parms;
#ifdef EMOS
  fifo_dump_emos emos_dump;
#endif

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
  while ((c = getopt (argc, argv, "hapl:r:m:n:s:t:")) != -1)
    {
      switch (c)
	{
	case 'a':
	  printf("Running AWGN simulation\n");
	  awgn_flag = 1;
	  break;
	case 'h':
	  printf("%s -h(elp) -a(wgn on) -n n_frames -s snr0 -t DelaySpread -p (extended prefix flag)\n",argv[0]);
	  exit(1);
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
	  n_rx=atoi(optarg);
	  break;
	case 'm':
	  n_tx=atoi(optarg);
	  break;
	case 'l':
	  transmission_mode=atoi(optarg);
	  break;
	default:
	  printf("%s -h(elp) -p(extended_prefix) -m mcs -n n_frames -s snr0\n",argv[0]);
	  exit (-1);
	  break;
	}
    }
  lte_param_init(n_tx,n_rx,transmission_mode,extended_prefix_flag);


  snr1 = snr0+25.0;
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

  eNB2UE = new_channel_desc(1,
			    1,
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

  
  generate_pss(PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
	       1087,
	       &PHY_vars_eNb->lte_frame_parms,
	       eNb_id,
	       (PHY_vars_eNb->lte_frame_parms.Ncp==0)?6:5,
	       0);
  

  for (i=0;i<6;i++)
    pbch_pdu[i] = i;

  printf("Generating PBCH for mode1_flag = %d\n", PHY_vars_eNb->lte_frame_parms.mode1_flag);

  
  generate_pilots(PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
		  1087,
		  &PHY_vars_eNb->lte_frame_parms,
		  0,
		  2);//LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
  
  
  generate_pbch(PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id],
		1024,
		&PHY_vars_eNb->lte_frame_parms,
		pbch_pdu,
		0);
  
  //  write_output("pilotsF.m","rsF",txdataF[0],lte_frame_parms->ofdm_symbol_size,1,1);
#ifdef IFFT_FPGA
  write_output("txsigF0.m","txsF0", lte_eNB_common_vars->txdataF[eNb_id][0],300*120,1,4);
  //write_output("txsigF1.m","txsF1", lte_eNB_common_vars->txdataF[1],300*120,1,4);

  // do talbe lookup and write results to txdataF2
  for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
    l = 0;
    for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX;i++) 
      if ((i%512>=1) && (i%512<=150))
	txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_eNb->lte_eNB_common_vars->txdataF[eNb_id][aa][l++]];
      else if (i%512>=362)
	txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_eNb->lte_eNB_common_vars->txdataF[eNb_id][aa][l++]];
      else 
	txdataF2[aa][i] = 0;
    printf("l=%d\n",l);
  }

  write_output("txsigF20.m","txsF20", txdataF2[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
  //write_output("txsigF21.m","txsF21", txdataF2[1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);

	
  tx_lev=0;

  for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
     
    if (frame_parms->Ncp == 1) 
      PHY_ofdm_mod(txdataF2[aa],        // input
		   txdata[aa],         // output
		   frame_parms->log2_symbol_size,                // log2_fft_size
		   2*nsymb,                 // number of symbols
		   frame_parms->nb_prefix_samples,               // number of prefix samples
		   frame_parms->twiddle_ifft,  // IFFT twiddle factors
		   frame_parms->rev,           // bit-reversal permutation
		   CYCLIC_PREFIX);
    else {
      normal_prefix_mod(txdataF2[aa],txdata[aa],2*nsymb,frame_parms);
    }
    tx_lev += signal_energy(&txdata[aa][OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*2],
			  OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
  
#else
  write_output("txsigF0.m","txsF0", PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0],2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX,1,1);
  //write_output("txsigF1.m","txsF1", lte_eNB_common_vars->txdataF[1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);

  tx_lev = 0;


  
  for (aa=0; aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx; aa++) {
    if (frame_parms->Ncp == 1) 
      PHY_ofdm_mod(PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa],        // input,
		   txdata[aa],         // output
		   frame_parms->log2_symbol_size,                // log2_fft_size
		   2*nsymb,                 // number of symbols
		   frame_parms->nb_prefix_samples,               // number of prefix samples
		   frame_parms->twiddle_ifft,  // IFFT twiddle factors
		   frame_parms->rev,           // bit-reversal permutation
		   CYCLIC_PREFIX);
    else {
      normal_prefix_mod(PHY_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa],
			txdata[aa],
			2*nsymb,
			frame_parms);
    }

    tx_lev += signal_energy(&txdata[aa][0],
			    OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
  }  
#endif


  tx_lev_dB = (unsigned int) dB_fixed(tx_lev);

  //  write_output("txsig0.m","txs0", txdata[0],2*frame_parms->samples_per_tti,1,1);
  //write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);

  // multipath channel
  randominit(0);

  for (i=0;i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
    for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx;aa++) {
      if (awgn_flag == 0) {
	s_re[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)]);
	s_im[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)+1]);
      }
      else {
	r_re[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)]);
	r_im[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)+1]);
      }
    }
  }

  for (SNR=snr0;SNR<snr1;SNR+=.2) {

    
    n_errors = 0;
    for (trial=0; trial<n_frames; trial++) {
	
	if (awgn_flag == 0) {	
	  multipath_channel(eNB2UE,s_re,s_im,r_re,r_im,
			    2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
	  
	}
	//write_output("channel0.m","chan0",ch[0],channel_length,1,8);
	
	// scale by path_loss = NOW - P_noise
	//sigma2       = pow(10,sigma2_dB/10);
	//N0W          = -95.87;
	sigma2_dB = (double)tx_lev_dB +10*log10(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size/(50*PHY_vars_eNb->lte_frame_parms.nb_antennas_tx)) - SNR;
	//	printf("sigma2_dB %f (SNR %f dB) tx_lev_dB %d\n",sigma2_dB,SNR,tx_lev_dB);
	//AWGN
	sigma2 = pow(10,sigma2_dB/10);
	//	printf("Sigma2 %f (sigma2_dB %f)\n",sigma2,sigma2_dB);
	for (i=0; i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES; i++) {
	  for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_rx;aa++) {
	    ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[2*i] = (short) (.167*(r_re[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	    ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[2*i+1] = (short) (.167*(r_im[aa][i] + (iqim*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	  }
	}    
	
	/*
	// optional: read rx_frame from file
	if ((rx_frame_file = fopen("rx_frame.dat","r")) == NULL)
	{
	printf("[openair][CHBCH_TEST][INFO] Cannot open rx_frame.m data file\n");
	exit(0);
	}
	
	result = fread((void *)PHY_vars->rx_vars[0].RX_DMA_BUFFER,4,FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
	printf("Read %d bytes\n",result);
	result = fread((void *)PHY_vars->rx_vars[1].RX_DMA_BUFFER,4,FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
	printf("Read %d bytes\n",result);
	
	fclose(rx_frame_file);
	*/

	/*		
	sync_pos = lte_sync_time(PHY_vars_UE->lte_ue_common_vars.rxdata, 
				 &PHY_vars_UE->lte_frame_parms, 
				 LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*PHY_vars_eNb->lte_frame_parms.samples_per_tti,
				 &PHY_vars_UE->lte_ue_common_vars.eNb_id);
	//sync_pos = 3328;
	*/
	
	// the sync is in the last symbol of either the 0th or 10th slot
	// however, the pbch is only in the 0th slot
	// so we assume that sync_pos points to the 0th slot
	// so the position wrt to the start of the frame is 
	sync_pos_slot = (PHY_vars_UE->lte_frame_parms.samples_per_tti>>1)-OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;
	sync_pos = sync_pos_slot;

	//	msg("sync_pos = %d, sync_pos_slot =%d\n", sync_pos, sync_pos_slot);
	
	if (((sync_pos - sync_pos_slot) >=0 ) && 
	    ((sync_pos - sync_pos_slot) < (FRAME_LENGTH_COMPLEX_SAMPLES - PHY_vars_eNb->lte_frame_parms.samples_per_tti)) ) {
    
	  for (l=0;l<PHY_vars_eNb->lte_frame_parms.symbols_per_tti;l++) {
	    
	    subframe_offset = (l/PHY_vars_eNb->lte_frame_parms.symbols_per_tti)*PHY_vars_eNb->lte_frame_parms.samples_per_tti;
	    //	    printf("subframe_offset = %d\n",subframe_offset);
	    
	    slot_fep(&PHY_vars_UE->lte_frame_parms,
		     &PHY_vars_UE->lte_ue_common_vars,
		     l%(PHY_vars_eNb->lte_frame_parms.symbols_per_tti/2),
		     l/(PHY_vars_eNb->lte_frame_parms.symbols_per_tti/2),
		     sync_pos-sync_pos_slot,
		     0);
	    
#ifdef EMOS
	    if ((l%3==0) && (l<PHY_vars_eNb->lte_frame_parms.symbols_per_tti)) 
	      for (sector=0; sector<3; sector++) 
		for (aa=0;aa<PHY_vars_eNb->lte_frame_parms.nb_antennas_tx;aa++)
		  lte_dl_channel_estimation_emos(emos_dump.channel[sector],
						 lte_ue_common_vars->rxdataF,
						 &PHY_vars_eNb->lte_frame_parms,
						 l/(PHY_vars_eNb->lte_frame_parms.symbols_per_tti/2),
						 aa,
						 l%(PHY_vars_eNb->lte_frame_parms.symbols_per_tti/2),
						 sector);
#endif
	    
	    if (l==0) {
	      
	      lte_ue_measurements(PHY_vars_UE,
				  &PHY_vars_UE->lte_frame_parms,
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

	      if (rx_pbch(&PHY_vars_UE->lte_ue_common_vars,
			  PHY_vars_UE->lte_ue_pbch_vars[0],
			  &PHY_vars_UE->lte_frame_parms,
			  0,
			  SISO,
			  0)) {
		//		msg("pbch decoded sucessfully for SISO!\n");
	      }
	      /*
	      else if (rx_pbch(&PHY_vars_UE->lte_ue_common_vars,
			       PHY_vars_UE->lte_ue_pbch_vars[0],
			       &PHY_vars_eNb->lte_frame_parms,
			       0,
			       ALAMOUTI,
			       0)) {
		//		msg("pbch decoded sucessfully for ALAMOUTI!\n");
	      }
	      */
	      else {
		n_errors++;
	      }
	    }
	  }
	}
    } //trials
    
    printf("SNR %f : n_errors = %d/%d\n", SNR,n_errors,n_frames);

  } // NSR

    write_output("H00.m","h00",&(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][0][0]),((frame_parms->Ncp==0)?7:6)*(PHY_vars_eNb->lte_frame_parms.ofdm_symbol_size),1,1);
    write_output("rxsig0.m","rxs0", PHY_vars_UE->lte_ue_common_vars.rxdata[0],2*frame_parms->samples_per_tti,1,1);
    write_output("rxsigF0.m","rxsF0", PHY_vars_UE->lte_ue_common_vars.rxdataF[0],NUMBER_OF_OFDM_CARRIERS*2*((frame_parms->Ncp==0)?14:12),2,1);    
    write_output("PBCH_rxF0_ext.m","pbch0_ext",PHY_vars_UE->lte_ue_pbch_vars[0]->rxdataF_ext[0],12*4*6,1,1);
    write_output("PBCH_rxF0_comp.m","pbch0_comp",PHY_vars_UE->lte_ue_pbch_vars[0]->rxdataF_comp[0],12*4*6,1,1);
    write_output("PBCH_rxF_llr.m","pbch_llr",PHY_vars_UE->lte_ue_pbch_vars[0]->llr,(frame_parms->Ncp==0) ? 1920 : 1728,1,0);
    

#ifdef EMOS
  write_output("EMOS_ch0.m","emos_ch0",&emos_dump.channel[0][0][0],N_RB_DL_EMOS*N_PILOTS_PER_RB*N_SLOTS_EMOS,1,1);
  write_output("EMOS_ch1.m","emos_ch1",&emos_dump.channel[1][0][0],N_RB_DL_EMOS*N_PILOTS_PER_RB*N_SLOTS_EMOS,1,1);
  write_output("EMOS_ch2.m","emos_ch2",&emos_dump.channel[2][0][0],N_RB_DL_EMOS*N_PILOTS_PER_RB*N_SLOTS_EMOS,1,1);
#endif


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

