#include <string.h>
#include <math.h>
#include "SIMULATION/TOOLS/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"

#define BW 10.0
#define Td 1.0

void main() {

  int i,aa;
  double sigma2, sigma2_dB=1;
  int **txdataF, **txdata;
  //LTE_DL_FRAME_PARMS *frame_parms = (LTE_DL_FRAME_PARMS *)malloc(sizeof(LTE_DL_FRAME_PARMS));
  //LTE_UE_COMMON      *lte_ue_common_vars = (LTE_UE_COMMON *)malloc(sizeof(LTE_UE_COMMON));
  double **s_re,**s_im,**r_re,**r_im;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=0.000005;
  int channel_length;
  struct complex **ch;

  unsigned char Ns,l,m,mod_order=6;
  unsigned int rb_alloc[4];
  MIMO_mode_t mimo_mode = ALAMOUTI;
  unsigned char *input_data,*decoded_output;

  LTE_eNb_DLSCH_t *dlsch_eNb;
  LTE_UE_DLSCH_t *dlsch_ue;
  unsigned char *input_buffer;
  unsigned short input_buffer_length;

  channel_length = (int) 11+2*BW*Td;

  PHY_vars = malloc(sizeof(PHY_VARS));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  randominit();
  set_taus_seed();
  
  crcTableInit();

  lte_frame_parms = &(PHY_config->lte_frame_parms);
  lte_ue_common_vars = &(PHY_vars->lte_ue_common_vars);
  lte_ue_dlsch_vars = &(PHY_vars->lte_ue_dlsch_vars);
  
  lte_frame_parms->N_RB_DL            = 25;
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 1;
  lte_frame_parms->nb_antennas_tx     = 2;
  lte_frame_parms->nb_antennas_rx     = 2;
  lte_frame_parms->first_dlsch_symbol = 2;
  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(NB_ANTENNAS_TX);
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  generate_64qam_table();
  generate_16qam_table();
  phy_init_lte_ue(lte_frame_parms,lte_ue_common_vars,lte_ue_dlsch_vars);
  
  rb_alloc[0] = 0x01ffffff;  // RBs 0-31
  rb_alloc[1] = 0x00000000;  // RBs 32-63
  rb_alloc[2] = 0x00000000;  // RBs 64-95
  rb_alloc[3] = 0x00000000;  // RBs 96-109
  

  txdataF    = (int **)malloc16(2*sizeof(int*));
  txdataF[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdataF[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  
  txdata    = (int **)malloc16(2*sizeof(int*));
  txdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  
  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
  
  input_buffer_length = 758;
  input_buffer = (unsigned char *)malloc(input_buffer_length+4);

  for (i=0;i<2;i++) {
    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }

  dlsch_eNb = new_eNb_dlsch(1,8);
  dlsch_ue  = new_ue_dlsch(1,8);

  if (!dlsch_eNb) {
    printf("Can't get eNb dlsch structures\n");
    exit(-1);
  }

  if (!dlsch_ue) {
    printf("Can't get ue dlsch structures\n");
    exit(-1);
  }

  //  input_data     = (unsigned char*) malloc(block_length/8);
  //  decoded_output = (unsigned char*) malloc(block_length/8);

  
  for (i=0;i<input_buffer_length;i++)
    input_buffer[i]= (unsigned char)(i%256);

  dlsch_eNb->harq_processes[0]->mimo_mode          = mimo_mode;
  dlsch_eNb->harq_processes[0]->mod_order          = mod_order;
  dlsch_eNb->harq_processes[0]->active             = 0;
  dlsch_eNb->harq_processes[0]->Nl                 = 1;
  dlsch_eNb->rvidx                                 = 0;

  dlsch_ue->harq_processes[0]->mimo_mode           = mimo_mode;
  dlsch_ue->harq_processes[0]->mod_order           = mod_order;
  dlsch_ue->harq_processes[0]->active              = 0;
  dlsch_ue->harq_processes[0]->Nl                  = 1;
  dlsch_ue->rvidx                                  = 0;

  dlsch_encoding(input_buffer,
		 (input_buffer_length<<3),
		 lte_frame_parms,
		 dlsch_eNb,
		 0,               // harq_pid
		 25); // number of allocated RB

  for (i=0;i<32;i++)
    printf("Segment 0 %d : %d\n",i,dlsch_eNb->harq_processes[0]->c[0][i]);

  dlsch_modulation(txdataF,
		   1024,
		   lte_frame_parms,
		   dlsch_eNb,
		   0,               // harq_pid
		   rb_alloc); // RB allocation vector


    /* 
    generate_pss(txdataF,
		 1024,
		 lte_frame_parms,
		 LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
    */
  ch = (struct complex**) malloc(1 * 2 * sizeof(struct complex*));
  for (i = 0; i<4; i++)
    ch[i] = (struct complex*) malloc(channel_length * sizeof(struct complex));


  generate_pilots(txdataF,
		  1024,
		  lte_frame_parms,
		  LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
  
  
  write_output("pilotsF.m","rsF",txdataF[0],lte_frame_parms->ofdm_symbol_size,1,1);
  write_output("txsigF0.m","txsF0", txdataF[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  
  PHY_ofdm_mod(txdataF[0],        // input
	       txdata[0],         // output
	       lte_frame_parms->log2_symbol_size,                // log2_fft_size
	       12*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
	       lte_frame_parms->nb_prefix_samples,               // number of prefix samples
	       lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
	       lte_frame_parms->rev,           // bit-reversal permutation
	       CYCLIC_PREFIX);
  
  PHY_ofdm_mod(txdataF[1],        // input
	       txdata[1],         // output
	       lte_frame_parms->log2_symbol_size,                // log2_fft_size
	       12*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
	       lte_frame_parms->nb_prefix_samples,               // number of prefix samples
	       lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
	       lte_frame_parms->rev,           // bit-reversal permutation
	       CYCLIC_PREFIX);
  
  write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  
  // multipath channel
  randominit();

  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++) {
    for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
      s_re[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)]);
      s_im[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)+1]);
    }
  }

  multipath_channel(ch,s_re,s_im,r_re,r_im,
		    amps,Td,BW,ricean_factor,aoa,
		    lte_frame_parms->nb_antennas_tx,
		    lte_frame_parms->nb_antennas_rx,
		    FRAME_LENGTH_COMPLEX_SAMPLES,
		    channel_length,
		    0);

  write_output("channel0.m","chan0",ch[0],channel_length,1,7);

  //AWGN
  sigma2 = pow(10,sigma2_dB/10);
  for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES; i++) {
    for (aa=0;aa<lte_frame_parms->nb_antennas_rx;aa++) {
      ((short*) lte_ue_common_vars->rxdata[aa])[2*i] = (short) (r_re[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
      ((short*) lte_ue_common_vars->rxdata[aa])[2*i+1] = (short) (r_im[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
    }
  }    
  //    lte_sync_time_init(lte_frame_parms,lte_ue_common_vars);
  //    lte_sync_time(lte_ue_common_vars->rxdata, lte_frame_parms);
  //    lte_sync_time_free();

    for (Ns=0;Ns<3;Ns++) {
      for (l=0;l<6;l++) {

	slot_fep(lte_frame_parms,
		 l,
		 Ns%20,
		 lte_ue_common_vars->rxdata,
		 lte_ue_common_vars->rxdataF,
		 lte_ue_common_vars->dl_ch_estimates,
		 (Ns>>1)*lte_frame_parms->samples_per_tti);
	
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
      
  write_output("rxsig0.m","rxs0", lte_ue_common_vars->rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  write_output("dlsch00_ch0.m","dl00_ch0",&(lte_ue_common_vars->dl_ch_estimates[0][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);

  write_output("dlsch01_ch0.m","dl01_ch0",&(lte_ue_common_vars->dl_ch_estimates[1][48]),NUMBER_OF_USEFUL_CARRIERS,1,1);
  write_output("dlsch10_ch0.m","dl10_ch0",&(lte_ue_common_vars->dl_ch_estimates[2][48]),NUMBER_OF_USEFUL_CARRIERS,1,1);
  write_output("dlsch11_ch0.m","dl11_ch0",&(lte_ue_common_vars->dl_ch_estimates[3][48]),NUMBER_OF_USEFUL_CARRIERS,1,1);

  //write_output("rxsigF0.m","rxsF0", lte_ue_common_vars->rxdataF[0],FRAME_LENGTH_COMPLEX_SAMPLES,2,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext",lte_ue_dlsch_vars->dl_ch_estimates_ext[0],NUMBER_OF_USEFUL_CARRIERS*12,1,1);
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0",lte_ue_dlsch_vars->rxdataF_comp[0],300*12,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr",lte_ue_dlsch_vars->llr,600*3,1,0);

  write_output("dlsch_mag1.m","dlschmag1",lte_ue_dlsch_vars->dl_ch_mag,300*12,1,1);
  write_output("dlsch_mag2.m","dlschmag2",lte_ue_dlsch_vars->dl_ch_magb,300*12,1,1);
  
  // Generate LLRs for decoding
  //  memset(dlsch_ue->harq_processes[0]->w[0],0,12+(block_length*8*3));
  printf("Calling decoding, dlsch_ue %p, active %d\n",dlsch_ue,dlsch_ue->harq_processes[0]->active);
  dlsch_decoding(input_buffer_length<<3,
		 lte_ue_dlsch_vars,
		 lte_frame_parms,
		 dlsch_ue,
		 0,               //harq_pid
		 25);             //NB allocated RBs

  printf("Decoded_output:\n");
  for (i=0;i<dlsch_ue->harq_processes[0]->Kplus/8;i++)
    printf("%d : %d\n",i,dlsch_ue->harq_processes[0]->c[0][i]);
  
  
  free(txdataF[0]);
  free(txdataF[1]);
  free(txdataF);
  free(txdata[0]);
  free(txdata[1]);
  free(txdata);
  
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

