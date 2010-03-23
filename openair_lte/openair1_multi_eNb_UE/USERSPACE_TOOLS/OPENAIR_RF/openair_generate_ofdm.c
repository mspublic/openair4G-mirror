#include <string.h>
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#ifdef OPENAIR_LTE
#include "PHY/LTE_TRANSPORT/defs.h"
#endif

void openair_generate_ofdm(char format,unsigned short freq_alloc,char *pdu) {

  unsigned int i,l;
  unsigned char level;

#ifdef OPENAIR_LTE
  mod_sym_t **txdataF;
  unsigned char pbch_pdu[6];
  unsigned int rv;
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

    /*
      lte_ue_common_vars = &(PHY_vars->lte_ue_common_vars);
      lte_ue_dlsch_vars = &(PHY_vars->lte_ue_dlsch_vars[0]);
      lte_ue_pbch_vars = &(PHY_vars->lte_ue_pbch_vars[0]);
    */
    lte_eNB_common_vars = &PHY_vars->lte_eNB_common_vars;
    lte_eNB_ulsch_vars = &(PHY_vars->lte_eNB_ulsch_vars[0]);
    
    phy_init_top(NB_ANTENNAS_TX);
    
    lte_frame_parms->twiddle_fft      = twiddle_fft;
    lte_frame_parms->twiddle_ifft     = twiddle_ifft;
    lte_frame_parms->rev              = rev;
    
    lte_gold(lte_frame_parms);
      
    phy_init_lte_eNB(lte_frame_parms, lte_eNB_common_vars, lte_eNB_ulsch_vars);
    
    generate_pss(lte_eNB_common_vars->txdataF[0],
		 256,
		 lte_frame_parms,
		 0,
		 5,
		 0);

    for (i=0; i<20; i++)
      generate_pilots_slot(lte_eNB_common_vars->txdataF[0],
			   256,
			   lte_frame_parms,
			   0,
			   i);

    for (i=0;i<6;i++)
      pbch_pdu[i] = i;
    
    generate_pbch(lte_eNB_common_vars->txdataF[0],
		  256,
		  lte_frame_parms,
		  pbch_pdu);


#ifdef IFFT_FPGA
    write_output("pilotsF.m","rsF",lte_eNB_common_vars->txdataF[0],12*lte_frame_parms->N_RB_DL*12,1,4);
#else
    write_output("pilotsF.m","rsF",lte_eNB_common_vars->txdataF[0],lte_frame_parms->ofdm_symbol_size,1,1);
#endif

#ifndef IFFT_FPGA
    PHY_ofdm_mod(lte_eNB_common_vars->txdataF[0],        // input
		 lte_eNB_common_vars->txdata[0],         // output
		 lte_frame_parms->log2_symbol_size,                // log2_fft_size
		 12*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,                 // number of symbols
		 lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		 lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 lte_frame_parms->rev,           // bit-reversal permutation
		 NONE);

    PHY_ofdm_mod(lte_eNB_common_vars->txdataF[1],        // input
		 lte_eNB_common_vars->txdata[1],         // output
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
    break;
  case 4:
    txdataF    = (mod_sym_t **)malloc16(2*sizeof(mod_sym_t*));
    txdataF[0] = (mod_sym_t *) PHY_vars->tx_vars[0].TX_DMA_BUFFER;
    txdataF[1] = (mod_sym_t *) PHY_vars->tx_vars[1].TX_DMA_BUFFER;

    for (l=1;l<120;l++) {
      for (i=0;i<75;i+=3) {
	rv = ptaus();
	rv = rv & 0x03030303;
	((char*)&rv)[0] = ((char*)&rv)[0]+1;
	((char*)&rv)[1] = ((char*)&rv)[1]+1;
	((char*)&rv)[2] = ((char*)&rv)[2]+1;
	((char*)&rv)[3] = ((char*)&rv)[3]+1;
	((unsigned int *)PHY_vars->tx_vars[0].TX_DMA_BUFFER)[l*75+i] = rv;
	
	rv = ptaus();
	rv = rv & 0x03030303;
	((char*)&rv)[0] = ((char*)&rv)[0]+1;
	((char*)&rv)[1] = ((char*)&rv)[1]+1;
	((char*)&rv)[2] = ((char*)&rv)[2]+1;
	((char*)&rv)[3] = ((char*)&rv)[3]+1;
	((unsigned int *)PHY_vars->tx_vars[0].TX_DMA_BUFFER)[l*75+i+1] = rv;

	rv = ptaus();
	rv = rv & 0x03030303;
	((char*)&rv)[0] = ((char*)&rv)[0]+1;
	((char*)&rv)[1] = ((char*)&rv)[1]+1;
	((char*)&rv)[2] = ((char*)&rv)[2]+1;
	((char*)&rv)[3] = ((char*)&rv)[3]+1;
	((unsigned int *)PHY_vars->tx_vars[0].TX_DMA_BUFFER)[l*75+i+2] = rv;

	/*
	((unsigned int *)PHY_vars->tx_vars[1].TX_DMA_BUFFER)[(l+1)*75+i] = 0x01020304;	
	((unsigned int *)PHY_vars->tx_vars[1].TX_DMA_BUFFER)[(l+1)*75+i+1] = 0x04030201;
	((unsigned int *)PHY_vars->tx_vars[1].TX_DMA_BUFFER)[(l+1)*75+i+2] = 0x02030104;
	*/
      }     
    }

    /*
    generate_pilots(txdataF,
		    256,
		    lte_frame_parms,
		    LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
    generate_pss(txdataF,
		 256,
		 lte_frame_parms,
		 LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
    */
#endif

  default:
    break;
  }
}
   



