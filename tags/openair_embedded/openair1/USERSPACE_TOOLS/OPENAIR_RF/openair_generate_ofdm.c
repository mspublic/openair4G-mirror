#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"

void openair_generate_ofdm(char format,unsigned short freq_alloc,char *pdu) {

  unsigned int i;
  unsigned char level;



  switch (format) {

  case 0 : // generate CHSCH + CHBCH (10 symbols out of 64)
    printf("Generate OFDM: CHSCH0,CHSCH1 + CHBCH\n");
    phy_generate_chbch(1,0,NB_ANTENNAS_TX,pdu);
    break;
  case 1: // generate all SCH 0 repeated in frame (64 symbols)

    printf("Generate OFDM: SCH (symbols 0..63)\n");
    for (i=0;i<NUMBER_OF_SYMBOLS_PER_FRAME;i++) 
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
  default:
    break;
  }
}
   


/*  
  for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
	   (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
	   12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/

