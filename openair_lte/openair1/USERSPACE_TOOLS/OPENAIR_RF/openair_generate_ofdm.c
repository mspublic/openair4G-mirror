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
  mod_sym_t **txdataF;
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
    txdataF    = (mod_sym_t **)malloc16(2*sizeof(mod_sym_t*));
#ifdef IFFT_FPGA
    txdataF[0] = (mod_sym_t *) PHY_vars->tx_vars[0].TX_DMA_BUFFER;
    txdataF[1] = (mod_sym_t *) PHY_vars->tx_vars[1].TX_DMA_BUFFER;
#else
    txdataF[0] = (mod_sym_t *)malloc16(sizeof(mod_sym_t)*FRAME_LENGTH_COMPLEX_SAMPLES);
    txdataF[1] = (mod_sym_t *)malloc16(sizeof(mod_sym_t)*FRAME_LENGTH_COMPLEX_SAMPLES);
#endif

    generate_pss(txdataF,
		 256,
		 lte_frame_parms,
		 LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
 
    generate_pilots(txdataF,
		    256,
		    lte_frame_parms,
		    LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);

#ifdef IFFT_FPGA
    write_output("pilotsF.m","rsF",txdataF[0],lte_frame_parms->N_RB_DL,1,4);
#else
    write_output("pilotsF.m","rsF",txdataF[0],lte_frame_parms->ofdm_symbol_size,1,1);
#endif

#ifndef IFFT_FPGA
    PHY_ofdm_mod(txdataF[0],        // input
		 PHY_vars->tx_vars[0].TX_DMA_BUFFER,         // output
		 lte_frame_parms->log2_symbol_size,                // log2_fft_size
		 12*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
		 lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		 lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 lte_frame_parms->rev,           // bit-reversal permutation
		 NONE);

    PHY_ofdm_mod(txdataF[1],        // input
		 PHY_vars->tx_vars[1].TX_DMA_BUFFER,         // output
		 lte_frame_parms->log2_symbol_size,                // log2_fft_size
		 12*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
		 lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		 lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 lte_frame_parms->rev,           // bit-reversal permutation
		 NONE);


    //write_output("pss.m","pss0", PHY_vars->tx_vars[0].TX_DMA_BUFFER,FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);


#ifdef BIT8_TXMUX
    bit8_txmux(FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,0);
#endif //BIT8_TXMUX

    write_output("pss.m","pss0", PHY_vars->tx_vars[0].TX_DMA_BUFFER,FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,5);

    free(txdataF[0]);
    free(txdataF[1]);
#endif
    free(txdataF);

#endif
  default:
    break;
  }
}
   



