#include <string.h>
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#ifdef OPENAIR_LTE
#include "PHY/LTE_TRANSPORT/defs.h"
#endif

void openair_generate_ofdm(char format,unsigned short freq_alloc,char *pdu) {

  unsigned int i;
  unsigned char level;

#ifdef OPENAIR_LTE
  int **txdataF;
  LTE_DL_FRAME_PARMS *frame_parms = (LTE_DL_FRAME_PARMS *)malloc(sizeof(LTE_DL_FRAME_PARMS));
#endif

  switch (format) {
#ifndef OPENAIR_LTE
  case 0 : // generate CHSCH + CHBCH (10 symbols out of 64)
    printf("Generate OFDM: CHSCH0,CHSCH1 + CHBCH\n");
    phy_generate_chbch(1,0,NB_ANTENNAS_TX,pdu);
    break;
  case 1: // generate all SCH 0 repeated in frame (64 symbols)

    printf("Generate OFDM: SCH (symbols 0..63)\n");
    for (i=0;i<NUMBER_OF_SYMBOLS_PER_FRAME;i+=3) 
      phy_generate_sch(0,0,i,freq_alloc,0,NB_ANTENNAS_TX);
    break;
  case 2: // generate all CHSCH repeated in frame (64 symbols)
    printf("Generate OFDM: CHSCH0 (symbols 0..63)\n");
    phy_generate_chbch(1,0,NB_ANTENNAS_TX,pdu);
    for (i=1;i<NUMBER_OF_SYMBOLS_PER_FRAME;i++) 
      memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
	     (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
	     OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
    break;
#else
  case 3:
    txdataF    = (int **)malloc16(2*sizeof(int*));
    txdataF[0] = (int *)malloc16(sizeof(int)*FRAME_LENGTH_COMPLEX_SAMPLES);
    txdataF[1] = (int *)malloc16(sizeof(int)*FRAME_LENGTH_COMPLEX_SAMPLES);

    frame_parms->N_RB_DL            = 15;
    frame_parms->Ncp                = 1;
    frame_parms->Nid_cell           = 0;
    frame_parms->nushift            = 1;
    frame_parms->nb_antennas_tx     = 1;
    frame_parms->nb_antennas_rx     = 1;
    frame_parms->first_dlsch_symbol = 1;
    init_frame_parms(frame_parms);
    frame_parms->twiddle_fft      = twiddle_fft256;
    frame_parms->twiddle_ifft      = twiddle_ifft256;
    frame_parms->rev              = rev; //has been initialized in init_fft

    generate_pss(txdataF,
		 256,
		 frame_parms,
		 LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
 
    generate_pilots(txdataF,
		    256,
		    frame_parms,
		    LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);


    PHY_ofdm_mod(txdataF[0],        // input
		 PHY_vars->tx_vars[0].TX_DMA_BUFFER,         // output
		 frame_parms->log2_symbol_size,                // log2_fft_size
		 12*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
		 frame_parms->nb_prefix_samples,               // number of prefix samples
		 frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 frame_parms->rev,           // bit-reversal permutation
		 NONE);

#ifdef BIT8_TXMUX
    bit8_txmux(FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,0);
#endif //BIT8_TXMUX


    write_output("pss.m","pss0", PHY_vars->tx_vars[0].TX_DMA_BUFFER,(512)*12,1,5);


    free(txdataF[0]);
    free(txdataF[1]);
    free(txdataF);

#endif
  default:
    break;
  }
}
   



