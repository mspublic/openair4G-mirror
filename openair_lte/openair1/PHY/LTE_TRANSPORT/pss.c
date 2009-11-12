/* file: pss.c
   purpose: generate the primary synchronization signals of LTE
   author: florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it
   date: 21.10.2009 
*/

#include "defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"

/*
typedef struct {
  unsigned char N_RB_DL;
  unsigned char Nid_cell;
  unsigned char Ncp;
  unsigned char nushift;
  unsigned short ofdm_symbol_size;
  unsigned char log2_symbol_size;
  unsigned short nb_prefix_samples;
  unsigned short first_carrier_offset;
  unsigned int samples_per_tti;
  unsigned char nb_antennas_tx;
  unsigned char nb_antennas_rx;
  unsigned char first_dlsch_symbol;
  short *twiddle_fft;
  unsigned short *rev;
} LTE_DL_FRAME_PARMS;
*/

void generate_pss(mod_sym_t **txdataF,
		  short amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  unsigned short Ntti) {

  // write primary_synch0 to txdataF

  unsigned int tti,tti_offset,slot_offset,Nsymb;
  unsigned short k,m;

  //a = (amp*ONE_OVER_SQRT2_Q15)>>15;
  //printf("[PSS] amp=%d, a=%d\n",amp,a);

  Nsymb = (frame_parms->Ncp==0)?14:12;

  for (tti=0;tti<Ntti;tti++) {
    slot_offset = (tti*2)%20;
    //printf("[PSS] slot_offset = %d\n",slot_offset);
    
    // the pss goes in the last symbol of the zeroth and 10th slot of a frame 
    if (slot_offset%10==0) {
      // it occupies the inner 6 RBs, which start at
#ifdef IFFT_FPGA
      k = (frame_parms->N_RB_DL-3)*12;
#else
      k = frame_parms->ofdm_symbol_size-3*12;
#endif
      //printf("[PSS] k = %d\n",k);
      for (m=0;m<72;m++) {
#ifdef IFFT_FPGA
	txdataF[0][slot_offset*Nsymb/2*frame_parms->N_RB_DL*12 + (Nsymb/2-1)*frame_parms->N_RB_DL*12+k] = primary_synch0_tab[m];
#else
	((short*)txdataF[0])[2*(slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
				(Nsymb/2-1)*frame_parms->ofdm_symbol_size+k)] = 
	  (amp * primary_synch0[2*m]) >> 15;
	((short*)txdataF[0])[2*(slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
				(Nsymb/2-1)*frame_parms->ofdm_symbol_size+k)+1] = 
	  (amp * primary_synch0[2*m+1]) >> 15;
#endif	
	k+=1;
#ifdef IFFT_FPGA
	if (k >= frame_parms->N_RB_DL*12) {
	  k-=frame_parms->N_RB_DL*12;
#else
	if (k >= frame_parms->ofdm_symbol_size) {
	  k++; //skip DC
	  k-=frame_parms->ofdm_symbol_size;
#endif
	}
      }
    }
  }
}
