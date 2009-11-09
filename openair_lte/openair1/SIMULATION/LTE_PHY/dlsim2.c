#include <string.h>
#include <math.h>
#include "SIMULATION/TOOLS/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"

#define BW 10.0
#define Td 1.0

int main(int argc, char **argv) {

  int i,aa;
  double sigma2, sigma2_dB=0;
  int **txdataF, **txdata;
  //LTE_DL_FRAME_PARMS *frame_parms = (LTE_DL_FRAME_PARMS *)malloc(sizeof(LTE_DL_FRAME_PARMS));
  //LTE_UE_COMMON      *lte_ue_common_vars = (LTE_UE_COMMON *)malloc(sizeof(LTE_UE_COMMON));
  double **s_re,**s_im,**r_re,**r_im;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=1; //0.0000005;
  int channel_length;
  struct complex **ch;
  unsigned char pbch_pdu[6];

  if (argc>1)
    sigma2_dB = atoi(argv[1]);


  channel_length = (int) 11+2*BW*Td;

  PHY_vars = malloc(sizeof(PHY_VARS));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  lte_frame_parms = &(PHY_config->lte_frame_parms);
  lte_ue_common_vars = &(PHY_vars->lte_ue_common_vars);
  lte_ue_dlsch_vars = &(PHY_vars->lte_ue_dlsch_vars);
  
  lte_frame_parms->N_RB_DL            = 25;
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 1;
  lte_frame_parms->nb_antennas_tx     = 1;
  lte_frame_parms->nb_antennas_rx     = 1;
  lte_frame_parms->first_dlsch_symbol = 1;
  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(NB_ANTENNAS_TX);
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  phy_init_lte_ue(lte_frame_parms,lte_ue_common_vars,lte_ue_dlsch_vars);
  
  txdataF    = (int **)malloc16(2*sizeof(int*));
  txdataF[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdataF[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  
  txdata    = (int **)malloc16(2*sizeof(int*));
  txdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  
  /*
    rxdataF    = (int **)malloc16(2*sizeof(int*));
    rxdataF[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    rxdataF[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
    
    rxdata    = (int **)malloc16(2*sizeof(int*));
    rxdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    rxdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  */
  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
  
  for (i=0;i<2;i++) {

    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }

  ch = (struct complex**) malloc(1 * 2 * sizeof(struct complex*));
  for (i = 0; i<4; i++)
    ch[i] = (struct complex*) malloc(channel_length * sizeof(struct complex));


  generate_pss(txdataF,
	       1024,
	       lte_frame_parms,
	       LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);

  for (i=0;i<6;i++)
    pbch_pdu[i] = i;

  generate_pbch(txdataF,
		1024,
		lte_frame_parms,
		pbch_pdu);

  generate_pilots(txdataF,
		  1024,
		  lte_frame_parms,
		  LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
  
  
  write_output("pilotsF.m","rsF",txdataF[0],lte_frame_parms->ofdm_symbol_size,1,1);
  write_output("txsigF0.m","txsF0", txdataF[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  
  for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
    PHY_ofdm_mod(txdataF[aa],        // input
		 txdata[aa],         // output
		 lte_frame_parms->log2_symbol_size,                // log2_fft_size
		 12*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
		 lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		 lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 lte_frame_parms->rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);
  }  
  //write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  
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

  write_output("channel0.m","chan0",ch[0],channel_length,1,8);

  //AWGN
  sigma2 = pow(10,sigma2_dB/10);
  //printf("sigma2 = %g\n",sigma2);
  for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES; i++) {
    for (aa=0;aa<lte_frame_parms->nb_antennas_rx;aa++) {
      ((short*) lte_ue_common_vars->rxdata[aa])[2*i] = (short) (s_re[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
      ((short*) lte_ue_common_vars->rxdata[aa])[2*i+1] = (short) (s_im[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
    }
  }
  
  //lte_sync_time(lte_ue_common_vars->rxdata, lte_frame_parms);
  
  int Ns;
  int l;
   for (Ns=0;Ns<2;Ns++) {
      for (l=0;l<6;l++) {

        slot_fep(lte_frame_parms,
                 l,
                 Ns%20,
                 lte_ue_common_vars->rxdata,
                 lte_ue_common_vars->rxdataF,
                 lte_ue_common_vars->dl_ch_estimates,
                 (Ns>>1)*lte_frame_parms->samples_per_tti);
      }
   }

   // ATTENTION: I am using the dlsch_vars for the pbch for the moment!
   rx_pbch(lte_ue_common_vars,
	   lte_ue_dlsch_vars,
	   lte_frame_parms,
	   SISO);


   //write_output("rxsig0.m","rxs0", lte_ue_common_vars->rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  write_output("rxsigF0.m","rxsF0", lte_ue_common_vars->rxdataF[0],NUMBER_OF_OFDM_CARRIERS*2*12,2,1);
  write_output("dlsch00_ch0.m","dl00_ch0",&(lte_ue_common_vars->dl_ch_estimates[0][0]),6*(96+lte_frame_parms->ofdm_symbol_size),1,1);
  //write_output("dlsch00_ch0.m","dl00_ch0",&(lte_ue_common_vars->dl_ch_estimates[0][5]),NUMBER_OF_USEFUL_CARRIERS,1,1);
  //write_output("dlsch01_ch0.m","dl01_ch0",&(lte_ue_common_vars->dl_ch_estimates[1][48]),NUMBER_OF_USEFUL_CARRIERS,1,1);
  //write_output("dlsch10_ch0.m","dl10_ch0",&(lte_ue_common_vars->dl_ch_estimates[2][48]),NUMBER_OF_USEFUL_CARRIERS,1,1);
  //write_output("dlsch11_ch0.m","dl11_ch0",&(lte_ue_common_vars->dl_ch_estimates[3][48]),NUMBER_OF_USEFUL_CARRIERS,1,1);


  write_output("pbch_rxF_ext.m","pbch_ext",lte_ue_dlsch_vars->rxdataF_ext[0],12*4*6,1,1);
  write_output("pbch_ch0_ext.m","pbch_ch0",lte_ue_dlsch_vars->dl_ch_estimates_ext[0],12*4*6,1,1);
  write_output("pbch_rxF_comp.m","pbch_comp",lte_ue_dlsch_vars->rxdataF_comp[0],24*6,1,1);
  write_output("pbch_rxF_llr.m","pbch_llr",lte_ue_dlsch_vars->llr,24*6*2,1,0);


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
  
  lte_sync_time_free();

  return(0);

}
   


/*  
  for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
	   (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
	   12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/

