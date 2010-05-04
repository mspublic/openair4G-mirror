#include "PHY/defs.h"
#include "PHY/extern.h"
#include <emmintrin.h>
#include <xmmintrin.h>
//#define DEBUG_CH

int lte_ul_channel_estimation(int **ul_ch_estimates,
			      int **rxdataF_ext,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      unsigned char l,
			      unsigned char Ns,
			      unsigned int N_rb_alloc) {

  unsigned short aa,b,k,Msc_RS,Msc_RS_idx,symbol_offset;
  unsigned short * Msc_idx_ptr;
  unsigned short pilot_pos1 = 3 - frame_parms->Ncp, pilot_pos2 = 10 - 2*frame_parms->Ncp;
  short alpha, beta;
  int *ul_ch1, *ul_ch2;

  Msc_RS = N_rb_alloc*12;

#ifdef USER_MODE
  Msc_idx_ptr = (unsigned short*) bsearch(&Msc_RS, dftsizes, 33, sizeof(unsigned short), compareints);
  if (Msc_idx_ptr)
    Msc_RS_idx = Msc_idx_ptr - dftsizes;
  else {
    msg("lte_ul_channel_estimation: index for Msc_RS=%d not found\n",Msc_RS);
    return(-1);
  }
#else
  for (b=0;b<33;b++) 
    if (Msc_RS==dftsizes[b])
      Msc_RS_idx = b;
#endif

#ifdef DEBUG_CH
  msg("lte_ul_channel_estimation: Msc_RS = %d, Msc_RS_idx = %d\n",Msc_RS, Msc_RS_idx);
#ifdef USER_MODE
  write_output("drs_seq.m","drs",ul_ref_sigs_rx[0][0][Msc_RS_idx],2*Msc_RS,2,1);
#endif
#endif

  if (l == (3 - frame_parms->Ncp)) {

    symbol_offset = frame_parms->N_RB_UL*12*(l+((7-frame_parms->Ncp)*(Ns&1)));

    for (aa=0; aa<frame_parms->nb_antennas_rx; aa++){
      mult_cpx_vector_norep2((short*) &rxdataF_ext[aa][symbol_offset<<1],
			     (short*) ul_ref_sigs_rx[0][0][Msc_RS_idx],
			     (short*) &ul_ch_estimates[aa][symbol_offset],
			     Msc_RS,
			     15);

      if (Ns&1) {//we are in the second slot of the sub-frame, so do the interpolation

	ul_ch1 = &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*pilot_pos1];
	ul_ch2 = &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*pilot_pos2];

#ifdef DEBUG_CH
	msg("lte_ul_channel_estimation: ul_ch1 = %p, ul_ch2 = %p, pilot_pos1=%d, pilot_pos2=%d\n",ul_ch1, ul_ch2, pilot_pos1,pilot_pos2); 
#endif
	for (k=0;k<frame_parms->symbols_per_tti;k++) {

	  // we scale alpha and beta by 0x3FFF (instead of 0x7FFF) to avoid overflows 
	  alpha = (short) (((int) 0x3FFF * (int) (pilot_pos2-k))/(pilot_pos2-pilot_pos1));
	  beta  = (short) (((int) 0x3FFF * (int) (k-pilot_pos1))/(pilot_pos2-pilot_pos1));
	  
#ifdef DEBUG_CH
	  msg("lte_ul_channel_estimation: k=%d, alpha = %d, beta = %d\n",k,alpha,beta); 
#endif
	  //symbol_offset_subframe = frame_parms->N_RB_UL*12*k;

	  // interpolate between estimates
	  if ((k != pilot_pos1) && (k != pilot_pos2))  {
	    multadd_complex_vector_real_scalar((short*) ul_ch1,alpha,(short*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],1,N_rb_alloc*12);
	    multadd_complex_vector_real_scalar((short*) ul_ch2,beta ,(short*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],0,N_rb_alloc*12);
	  }
	} //for(k=...

	// because of the scaling of alpha and beta we also need to scale the final channel estimate at the pilot positions 
	multadd_complex_vector_real_scalar((short*) ul_ch1,0x3FFF,(short*) ul_ch1,1,N_rb_alloc*12);
	multadd_complex_vector_real_scalar((short*) ul_ch2,0x3FFF,(short*) ul_ch2,1,N_rb_alloc*12);
      } //if (Ns&1)

    } //for(aa=...
    
  } //if(l==...
  
  return(0);
}       

