#include <string.h>
#include <math.h>
#include "SIMULATION/TOOLS/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"

void main() {

  int i;
  double sigma;
  int **txdataF, **txdata;
  //LTE_DL_FRAME_PARMS *frame_parms = (LTE_DL_FRAME_PARMS *)malloc(sizeof(LTE_DL_FRAME_PARMS));
  //LTE_UE_COMMON      *lte_ue_common_vars = (LTE_UE_COMMON *)malloc(sizeof(LTE_UE_COMMON));

  PHY_vars = malloc(sizeof(PHY_VARS));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  lte_frame_parms = &(PHY_config->lte_frame_parms);
  lte_ue_common_vars = &(PHY_vars->lte_ue_common_vars);
  
    lte_frame_parms->N_RB_DL            = 15;
    lte_frame_parms->Ncp                = 1;
    lte_frame_parms->Nid_cell           = 0;
    lte_frame_parms->nushift            = 1;
    lte_frame_parms->nb_antennas_tx     = 2;
    lte_frame_parms->nb_antennas_rx     = 2;
    lte_frame_parms->first_dlsch_symbol = 1;
    init_frame_parms(lte_frame_parms);

    copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));

    phy_init_top(NB_ANTENNAS_TX);

    lte_frame_parms->twiddle_fft      = twiddle_fft;
    lte_frame_parms->twiddle_ifft     = twiddle_ifft;
    lte_frame_parms->rev              = rev;

    phy_init_lte(lte_frame_parms,lte_ue_common_vars);

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

    generate_pss(txdataF,
		 1024,
		 lte_frame_parms,
		 LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);

    generate_pilots(txdataF,
		    1024,
		    lte_frame_parms,
		    LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);

 
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

    // AWGN channel
    sigma = 10;
    randominit();
    memcpy(lte_ue_common_vars->rxdata[0],txdata[0],FRAME_LENGTH_BYTES);
    memcpy(lte_ue_common_vars->rxdata[1],txdata[1],FRAME_LENGTH_BYTES);
    for (i=0; i<FRAME_LENGTH_SAMPLES; i++) {
      ((short*) lte_ue_common_vars->rxdata[0])[2*i]   += sqrt(sigma)*gaussdouble(0.0,1.0);
      ((short*) lte_ue_common_vars->rxdata[0])[2*i+1] += sqrt(sigma)*gaussdouble(0.0,1.0);
      ((short*) lte_ue_common_vars->rxdata[1])[2*i]   += sqrt(sigma)*gaussdouble(0.0,1.0);
      ((short*) lte_ue_common_vars->rxdata[1])[2*i+1] += sqrt(sigma)*gaussdouble(0.0,1.0);
    }

    lte_sync_time_init(lte_frame_parms);
    lte_sync_time(lte_ue_common_vars->rxdata, lte_frame_parms);
    lte_sync_time_free();
  
  slot_fep(lte_frame_parms,
	   0,
	   0,
	   lte_ue_common_vars->rxdata,
	   lte_ue_common_vars->rxdataF,
	   lte_ue_common_vars->dl_ch_estimates,
	   0);


  write_output("rxsigF0.m","rxsF0", lte_ue_common_vars->rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  write_output("dlsch00_ch0.m","dl00_ch0",&(lte_ue_common_vars->dl_ch_estimates[0][48]),NUMBER_OF_USEFUL_CARRIERS,1,1);
  write_output("dlsch01_ch0.m","dl01_ch0",&(lte_ue_common_vars->dl_ch_estimates[1][48]),NUMBER_OF_USEFUL_CARRIERS,1,1);
  write_output("dlsch10_ch0.m","dl10_ch0",&(lte_ue_common_vars->dl_ch_estimates[2][48]),NUMBER_OF_USEFUL_CARRIERS,1,1);
  write_output("dlsch11_ch0.m","dl11_ch0",&(lte_ue_common_vars->dl_ch_estimates[3][48]),NUMBER_OF_USEFUL_CARRIERS,1,1);


  free(txdataF[0]);
  free(txdataF[1]);
  free(txdataF);
  free(txdata[0]);
  free(txdata[1]);
  free(txdata);

}
   


/*  
  for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
	   (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
	   12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/

