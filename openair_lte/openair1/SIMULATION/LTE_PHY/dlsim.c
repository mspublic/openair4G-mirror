#include <string.h>
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/LTE_TRANSPORT/defs.h"
#include "PHY/vars.h"

void main() {

  int **txdataF, **txdata;
  LTE_DL_FRAME_PARMS *frame_parms = (LTE_DL_FRAME_PARMS *)malloc(sizeof(LTE_DL_FRAME_PARMS));
  
    frame_parms->N_RB_DL            = 25;
    frame_parms->Ncp                = 1;
    frame_parms->Nid_cell           = 0;
    frame_parms->nushift            = 1;
    frame_parms->nb_antennas_tx     = 1;
    frame_parms->nb_antennas_rx     = 1;
    frame_parms->first_dlsch_symbol = 1;
    init_frame_parms(frame_parms);

    init_fft(frame_parms->ofdm_symbol_size,frame_parms->log2_symbol_size,rev);
    frame_parms->twiddle_fft      = twiddle_fft512;
    frame_parms->twiddle_ifft      = twiddle_ifft512;
    frame_parms->rev              = rev;

    txdataF    = (int **)malloc16(2*sizeof(int*));
    txdataF[0] = (int *)malloc16(sizeof(int)*frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
    txdataF[1] = (int *)malloc16(sizeof(int)*frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);

    txdata    = (int **)malloc16(2*sizeof(int*));
    txdata[0] = (int *)malloc16(sizeof(int)*frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
    txdata[1] = (int *)malloc16(sizeof(int)*frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);

    generate_pss(txdataF,
		 1024,
		 frame_parms,
		 LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
 
    PHY_ofdm_mod(txdataF[0],        // input
		 txdata[0],         // output
		 frame_parms->log2_symbol_size,                // log2_fft_size
		 12*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
		 frame_parms->nb_prefix_samples,               // number of prefix samples
		 frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 frame_parms->rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);

    write_output("pss.m","pss0", txdata[0], (512+128)*12,1,1);


  lte_sync_time_init(frame_parms);
  lte_sync_time(txdata, frame_parms);
  lte_sync_time_free();
  

  free(txdataF[0]);
  free(txdataF[1]);
  free(txdataF);
  free(txdata[0]);
  free(txdata[1]);
  free(txdata);
  free(frame_parms);

}
   


/*  
  for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
	   (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
	   12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/

