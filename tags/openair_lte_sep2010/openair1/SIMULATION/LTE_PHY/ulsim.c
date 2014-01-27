#include <string.h>
#include <math.h>
#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"
#include "MAC_INTERFACE/vars.h"
#include "SCHED/defs.h"
#include "SCHED/vars.h"
#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif
#ifdef EMOS
#include "SCHED/phy_procedures_emos.h"
#endif

#include "forms.h"
#include "ulsim_form.h"

#define BW 7.68
#define Td 4.0
#define N_TRIALS 10000

int current_dlsch_cqi;

DCI0_5MHz_TDD0_t          UL_alloc_pdu;
//DCI_ALLOC_t dci_alloc;

#define SI_RNTI 0xffff 
#define RA_RNTI 0xfffe
#define P_RNTI  0xfffd
#define C_RNTI  0x1111
//#define UL_RB_ALLOC 0x1ff;

FD_ulsim *form;

int main(int argc, char **argv) {

  int i,l,aa,sector;
  double sigma2, sigma2_dB=0,SNR,snr0=-2.0,snr1,SNRmeas;
  //mod_sym_t **txdataF;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  int **txdata;
  double **s_re,**s_im,**r_re,**r_im;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=0.000000000000000005;
  int channel_length;
  struct complex **ch;
  unsigned char pbch_pdu[6];
  int sync_pos, sync_pos_slot;
  FILE *rx_frame_file;
  int result;
  int freq_offset;
  int subframe_offset,subframe;
  char fname[40], vname[40];
  int trial;
  unsigned int tx_lev,tx_lev_dB;
  unsigned int nb_rb = 6;
  unsigned int first_rb = 3;
  unsigned int eNb_id = 0;

  double nf[2] = {3.0,3.0}; //currently unused
  double ip =0.0;
  double N0W, path_loss, path_loss_dB;


  unsigned char *input_buffer,harq_pid;
  unsigned short input_buffer_length;
  unsigned char mcs;
  unsigned int ret;
  unsigned int errs=0;

#ifdef EMOS
  fifo_dump_emos emos_dump;
#endif


  printf("argc %d\n",argc);

  if (argc>1)
    mcs = atoi(argv[1]);
  else
    mcs = 0;

  printf("mcs set to %d\n",mcs);

  if (argc>2)
    nb_rb = atoi(argv[2]);

  if (argc>3)
    first_rb = atoi(argv[3]);

  if (nb_rb < 2) {
    printf("nb_rb < 2!!\n");
    exit(-1);
  }

  if ((first_rb + nb_rb > 25)) {
    printf("first_rb + nb_rb > 25!!\n");
    exit(-1);
  }

  if (argc==5) {
    snr0 = atof(argv[4]);
  }

  snr1 = snr0+10.0;
  
  channel_length = (int) 11+2*BW*Td;

  PHY_vars = malloc(sizeof(PHY_VARS));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));
  randominit(0);
  set_taus_seed(0);
  
  crcTableInit();

  lte_frame_parms = &(PHY_config->lte_frame_parms);
  lte_ue_common_vars = &(PHY_vars->lte_ue_common_vars);
  lte_ue_dlsch_vars = &(PHY_vars->lte_ue_dlsch_vars[0]);
  lte_ue_dlsch_vars_cntl = &(PHY_vars->lte_ue_dlsch_vars_cntl[0]);
  lte_ue_dlsch_vars_ra = &(PHY_vars->lte_ue_dlsch_vars_ra[0]);
  lte_ue_dlsch_vars_1A = &(PHY_vars->lte_ue_dlsch_vars_1A[0]);
  lte_ue_pbch_vars = &(PHY_vars->lte_ue_pbch_vars[0]);
  lte_eNB_common_vars = &(PHY_vars->lte_eNB_common_vars);
  lte_eNB_ulsch_vars = &(PHY_vars->lte_eNB_ulsch_vars);
  lte_ue_pdcch_vars = &(PHY_vars->lte_ue_pdcch_vars[0]);

  lte_frame_parms->N_RB_DL            = 25;
  lte_frame_parms->N_RB_UL            = 25;
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 1;
  lte_frame_parms->nb_antennas_tx     = 2;
  lte_frame_parms->nb_antennas_rx     = 2;
  lte_frame_parms->first_dlsch_symbol = 1;
  lte_frame_parms->Csrs = 2;
  lte_frame_parms->Bsrs = 0;
  lte_frame_parms->kTC = 0;
  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->tdd_config = 3;
  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(NB_ANTENNAS_TX);
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  generate_64qam_table();
  generate_16qam_table();

  generate_RIV_tables();
  /*
    rxdataF    = (int **)malloc16(2*sizeof(int*));
    rxdataF[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    rxdataF[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
    
    rxdata    = (int **)malloc16(2*sizeof(int*));
    rxdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    rxdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  */

  lte_gold(lte_frame_parms);

  generate_ul_ref_sigs();
  generate_ul_ref_sigs_rx();

  phy_init_lte_ue(lte_frame_parms,lte_ue_common_vars,lte_ue_dlsch_vars,lte_ue_dlsch_vars_cntl,lte_ue_dlsch_vars_ra,lte_ue_dlsch_vars_1A,lte_ue_pbch_vars,lte_ue_pdcch_vars);
  phy_init_lte_eNB(lte_frame_parms,lte_eNB_common_vars,lte_eNB_ulsch_vars);

  // Create transport channel structures for SI pdus

  ulsch_eNb = (LTE_eNb_ULSCH_t**) malloc16(2*sizeof(LTE_eNb_ULSCH_t*));
  ulsch_ue = (LTE_UE_ULSCH_t**) malloc16(2*sizeof(LTE_UE_ULSCH_t*));

  ulsch_eNb[0] = new_eNb_ulsch(3);
  ulsch_ue[0]  = new_ue_ulsch(3);

  dlsch_eNb = (LTE_eNb_DLSCH_t**) malloc16(2*sizeof(LTE_eNb_DLSCH_t*));
  dlsch_ue = (LTE_UE_DLSCH_t**) malloc16(2*sizeof(LTE_UE_DLSCH_t*));
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
  }
  

  /*  
      txdataF    = (mod_sym_t **)malloc16(2*sizeof(mod_sym_t*));
      txdataF[0] = (mod_sym_t *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
      txdataF[1] = (mod_sym_t *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));

      bzero(txdataF[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
      bzero(txdataF[1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
  */

  phy_init_lte_eNB(lte_frame_parms,lte_eNB_common_vars,lte_eNB_ulsch_vars);


  //  dci_alloc.dci_length = sizeof_DCI0_5MHz_TDD_0_t;

  UL_alloc_pdu.type    = 0;
  UL_alloc_pdu.rballoc = computeRIV(lte_frame_parms->N_RB_UL,first_rb,nb_rb);// 12 RBs from position 8
  UL_alloc_pdu.mcs     = mcs;
  UL_alloc_pdu.ndi     = 1;
  UL_alloc_pdu.TPC     = 0;
  UL_alloc_pdu.cqi_req = 1;

  generate_ue_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&UL_alloc_pdu,
				    C_RNTI,
				    8,
				    format0,
				    ulsch_ue[0],
				    dlsch_ue,
				    &PHY_vars->PHY_measurements,
				    lte_frame_parms,
				    SI_RNTI,
				    RA_RNTI,
				    P_RNTI,
				    0);

  generate_eNb_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&UL_alloc_pdu,
				     SI_RNTI,
				     8,
				     format0,
				     ulsch_eNb[0],
				     lte_frame_parms,
				     SI_RNTI,
				     RA_RNTI,
				     P_RNTI);

  /*
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
  txdata = lte_eNB_common_vars->txdata[0];
#endif
  */

  txdata = lte_eNB_common_vars->txdata[0];

  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
  
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

  subframe=2;

  generate_srs_tx(lte_frame_parms,lte_ue_common_vars->txdataF[0],scfdma_amps[nb_rb],subframe);
  generate_drs_pusch(lte_frame_parms,lte_ue_common_vars->txdataF[0],scfdma_amps[nb_rb],subframe,ulsch_ue[0]->harq_processes[0]->first_rb,ulsch_ue[0]->harq_processes[0]->nb_rb);

  harq_pid = subframe2harq_pid_tdd(lte_frame_parms->tdd_config,subframe);
  input_buffer_length = ulsch_ue[0]->harq_processes[harq_pid]->TBS/8;
  printf("Input buffer size %d bytes\n",input_buffer_length);

  input_buffer = (unsigned char *)malloc(input_buffer_length+4);
  
  for (i=0;i<input_buffer_length;i++) {
    input_buffer[i]= (unsigned char)(taus()&0xff);
    if (i<16)
      printf("input %d : %x\n",i,input_buffer[i]);
  }

  ulsch_encoding(input_buffer,lte_frame_parms,ulsch_ue[0],harq_pid);

  ulsch_modulation(lte_ue_common_vars->txdataF,scfdma_amps[nb_rb],subframe,lte_frame_parms,ulsch_ue[0],0);

    //  exit(-1);
#ifdef IFFT_FPGA_UE
#ifdef OUTPUT_DEBUG
  write_output("txsigF0.m","txsF0", &lte_ue_common_vars->txdataF[0][300*12*subframe],300*12,1,4);
#endif
  //write_output("txsigF1.m","txsF1", lte_ue_common_vars->txdataF[1],300*120,1,4);

  // do talbe lookup and write results to txdataF2
  for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
    l = 0;
    for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX;i++) 
      if ((i%512>=1) && (i%512<=150))
	txdataF2[aa][i] = ((int*)mod_table)[lte_ue_common_vars->txdataF[aa][l++]];
      else if (i%512>=362)
	txdataF2[aa][i] = ((int*)mod_table)[lte_ue_common_vars->txdataF[aa][l++]];
      else 
	txdataF2[aa][i] = 0;
    printf("l=%d\n",l);
  }
#ifdef OUTPUT_DEBUG
  write_output("txsigF20.m","txsF20", txdataF2[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
  //write_output("txsigF21.m","txsF21", txdataF2[1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
#endif

  for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) 
    PHY_ofdm_mod(txdataF2[aa],        // input
		 txdata[aa],         // output
		 lte_frame_parms->log2_symbol_size,                // log2_fft_size
		 12*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
		 lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		 lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 lte_frame_parms->rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);

  //  tx_lev += signal_energy(&txdata[aa][4*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
  //		  OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
    
#else
#ifdef OUTPUT_DEBUG
  write_output("txsigF0.m","txsF0", &lte_ue_common_vars->txdataF[0][512*12*subframe],512*12,1,1);
  //write_output("txsigF1.m","txsF1", lte_ue_common_vars->txdataF[1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
#endif
  tx_lev=0;
  for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
    PHY_ofdm_mod(&lte_ue_common_vars->txdataF[aa][subframe*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX],        // input
		 &txdata[aa][subframe*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],         // output
		 lte_frame_parms->log2_symbol_size,                // log2_fft_size
		 12,                 // number of symbols
		 lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		 lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 lte_frame_parms->rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);

    tx_lev += signal_energy(&txdata[aa][OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*subframe*12],
			    OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);

  }  
#endif

  tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
#ifdef OUTPUT_DEBUG	
  write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  //write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
#endif
  
  // multipath channel
  randominit(0);

  for (i=0;i<12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
    for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
      s_re[aa][i] = ((double)(((short *)&txdata[aa][subframe*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES]))[(i<<1)]);
      s_im[aa][i] = ((double)(((short *)&txdata[aa][subframe*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES]))[(i<<1)+1]);
    }
  }

  fl_initialize(&argc, argv, NULL, 0, 0);    
  //  form = create_form_ulsim();                 
  //  fl_show_form(form->ulsim,FL_PLACE_HOTSPOT,FL_FULLBORDER,"ULSIM");   

  for (SNR=snr0;SNR<snr1;SNR+=.1) {
    errs = 0;
    for (trial=0; trial<N_TRIALS; trial++) {
      
      multipath_channel(ch,s_re,s_im,r_re,r_im,
			amps,Td,BW,ricean_factor,aoa,
			lte_frame_parms->nb_antennas_tx,
			lte_frame_parms->nb_antennas_rx,
			12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,
			channel_length,
			0,1,1);

      //write_output("channel0.m","chan0",ch[0],channel_length,1,8);

      // scale by path_loss = NOW - P_noise
      //sigma2       = pow(10,sigma2_dB/10);
      //N0W          = -95.87;
      //path_loss_dB = N0W - sigma2;
      //path_loss    = pow(10,path_loss_dB/10);
      path_loss_dB = 0;
      path_loss = 1;

      for (i=0;i<12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_rx;aa++) {
	  r_re[aa][i]=r_re[aa][i]*sqrt(path_loss); 
	  r_im[aa][i]=r_im[aa][i]*sqrt(path_loss); 
	}
      }

      /*
      // RF model
      rf_rx(r_re,
      r_im,
      NULL,
      NULL,
      0,
      lte_frame_parms->nb_antennas_rx,
      FRAME_LENGTH_COMPLEX_SAMPLES,
      1.0/7.68e6 * 1e9,      // sampling time (ns)
      500,            // freq offset (Hz) (-20kHz..20kHz)
      0.0,            // drift (Hz) NOT YET IMPLEMENTED
      nf,             // noise_figure NOT YET IMPLEMENTED
      -path_loss_dB,            // rx_gain (dB)
      200,            // IP3_dBm (dBm)
      &ip,            // initial phase
      30.0e3,         // pn_cutoff (kHz)
      -500.0,          // pn_amp (dBc) default: 50
      0.0,           // IQ imbalance (dB),
      0.0);           // IQ phase imbalance (rad)
    
      */

      //AWGN
      sigma2_dB = tx_lev_dB - SNR;

      sigma2 = pow(10,sigma2_dB/10.0);
#ifdef OUTPUT_DEBUG
      printf("tx_lev_dB %d, sigma2 = %g (%f dB), SNR %f\n",tx_lev_dB,sigma2,sigma2_dB,SNR);
#endif
      // loop over 12 symbols of subframe and add a 13th for noise measurement
      for (i=0; i<13*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES; i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_rx;aa++) {
	  ((short*) &lte_eNB_common_vars->rxdata[0][aa][12*subframe*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES])[2*i] = (short) ((r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	  ((short*) &lte_eNB_common_vars->rxdata[0][aa][12*subframe*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES])[2*i+1] = (short) ((r_im[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	}
      }
#ifdef OUTPUT_DEBUG
      printf("rx_level Null symbol %f\n",10*log10(signal_energy((int*)&lte_eNB_common_vars->rxdata[0][0][160+(12*(1+subframe)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
      printf("rx_level data symbol %f\n",10*log10(signal_energy((int*)&lte_eNB_common_vars->rxdata[0][0][160+(12*subframe*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
#endif
      SNRmeas = 10*log10(((double)signal_energy((int*)&lte_eNB_common_vars->rxdata[0][0][160+(12*subframe*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2))/((double)signal_energy((int*)&lte_eNB_common_vars->rxdata[0][0][160+(12*(1+subframe)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)) - 1);

#ifdef OUTPUT_DEBUG
      printf("SNRmeas %f\n",SNRmeas);

      write_output("rxsig0.m","rxs0", lte_eNB_common_vars->rxdata[0][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
      write_output("rxsig1.m","rxs1", lte_eNB_common_vars->rxdata[0][1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
#endif

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


      for (l=subframe*lte_frame_parms->symbols_per_tti;l<((1+subframe)*lte_frame_parms->symbols_per_tti);l++) {
    
	slot_fep_ul(lte_frame_parms,
		    lte_eNB_common_vars,
		    l%(lte_frame_parms->symbols_per_tti/2),
		    l/(lte_frame_parms->symbols_per_tti/2),
		    0,
		    0);
      }
  
      rx_ulsch(lte_eNB_common_vars,
	       lte_eNB_ulsch_vars[0],
	       lte_frame_parms,
	       subframe,
	       0,  // this is the effective sector id
	       0,   // this is the UE instance to act upon
	       ulsch_eNb,
	       0);


      ret= ulsch_decoding(lte_eNB_ulsch_vars[0]->llr,
			  lte_frame_parms,
			  ulsch_eNb[0],
			  subframe,
			  0);
      if (ret == (1+MAX_TURBO_ITERATIONS)) {
	errs++;
	//	printf("%d/%d\n",errs,1+trial);
      }
      //      exit(-1);
      if (((errs>=100) || ((double)errs/(1+trial) < 1e-3))&&(trial>100))
	break;
    } // trial
    printf("\n**********************SNR = %f dB -> %f dB (tx_lev %f, sigma2_dB %f) errors %d/%d (%f %%)**************************\n",
	   SNR,SNR-10*log10((double)nb_rb/25.0),
	   (double)tx_lev_dB,
	   sigma2_dB,
	   errs,
	   trial+1,
	   100.0*errs/(1+trial));    
    if (((double)errs/(1+trial) < 1e-3))
      break;
  } // SNR

#ifdef OUTPUT_DEBUG    
  write_output("rxsigF0.m","rxsF0", &lte_eNB_common_vars->rxdataF[0][0][0],512*12*2,2,1);
  write_output("rxsigF1.m","rxsF1", &lte_eNB_common_vars->rxdataF[0][1][0],512*12*2,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", &lte_eNB_ulsch_vars[0]->rxdataF_ext[0][0],300*12*2,2,1);
  write_output("rxsigF1_ext.m","rxsF1_ext", &lte_eNB_ulsch_vars[0]->rxdataF_ext[1][0],300*12*2,2,1);
  write_output("srs_seq.m","srs",lte_eNB_common_vars->srs,2*lte_frame_parms->ofdm_symbol_size,2,1);
  write_output("srs_est0.m","srsest0",lte_eNB_common_vars->srs_ch_estimates[0][0],512,1,1);
  write_output("srs_est1.m","srsest1",lte_eNB_common_vars->srs_ch_estimates[0][1],512,1,1);
  write_output("drs_est0.m","drsest0",lte_eNB_ulsch_vars[0]->drs_ch_estimates[0][0],300*12,1,1);
  write_output("drs_est1.m","drsest1",lte_eNB_ulsch_vars[0]->drs_ch_estimates[0][1],300*12,1,1);
  write_output("ulsch_rxF_comp0.m","ulsch0_rxF_comp0",&lte_eNB_ulsch_vars[0]->rxdataF_comp[0][0][0],300*12,1,1);
  write_output("ulsch_rxF_llr.m","ulsch_llr",lte_eNB_ulsch_vars[0]->llr,ulsch_ue[0]->harq_processes[0]->nb_rb*12*2*9,1,0);	
  write_output("ulsch_ch_mag.m","ulsch_ch_mag",&lte_eNB_ulsch_vars[0]->ul_ch_mag[0][0][0],300*12,1,1);	  
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

  //  return(n_errors);

}
   


/*  
    for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
    (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
    12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/
