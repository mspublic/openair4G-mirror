#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"
//#define DEBUG_ULSCH

unsigned short ulsch_extract_rbs_single(int **rxdataF,
					int **rxdataF_ext,
					unsigned int *rb_alloc,
					unsigned char l,
					unsigned char Ns,
					LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short rb,nb_rb=0;
  unsigned char rb_alloc_ind;
  unsigned char aarx;
  int *rxF,*rxF_ext;

  unsigned char symbol = l+Ns*frame_parms->symbols_per_tti/2;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    rxF_ext   = &rxdataF_ext[aarx][(symbol*frame_parms->N_RB_UL*12)*2];
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + symbol*frame_parms->ofdm_symbol_size)*2];
    
    if ((frame_parms->N_RB_UL&1) == 0)  // even number of RBs
      for (rb=0;rb<frame_parms->N_RB_UL;rb++) {
	
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	// For second half of RBs skip DC carrier
	if (rb==(frame_parms->N_RB_UL>>1)) {
	  rxF = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
	}
	
	if (rb_alloc_ind==1) {
#ifdef DEBUG_ULSCH
	  msg("ulsch_extract_rbs_single: extracting RB %d from %p to %p\n",rb,rxF,rxF_ext);
#endif
	  memcpy(rxF_ext,rxF,24*sizeof(int));
	  nb_rb++;
	  rxF_ext+=24;
	}
	rxF+=24;

      }
    else {  // Odd number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL>>1;rb++) {
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	if (rb_alloc_ind==1) {
#ifdef DEBUG_ULSCH
	  msg("ulsch_extract_rbs_single: extracting RB %d from %p to %p\n",rb,rxF,rxF_ext);
#endif
	  memcpy(rxF_ext,rxF,24*sizeof(int));
	  nb_rb++;
	  rxF_ext+=24;
	}
	rxF+=24;
      }

      // Do middle RB (around DC)
      if (rb < 32)
	rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
      else if (rb < 64)
	rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
      else if (rb < 96)
	rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
      else if (rb < 100)
	rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
      else
	rb_alloc_ind = 0;

      if (rb_alloc_ind==1) {
#ifdef DEBUG_ULSCH
	msg("ulsch_extract_rbs_single: extracting RB %d (middle) from %p to %p\n",rb,rxF,rxF_ext);
#endif
	memcpy(rxF_ext,rxF,12*sizeof(int));
	rxF_ext+=12;
	rxF     = &rxdataF[aarx][(1+(symbol*(frame_parms->ofdm_symbol_size)))*2];
	memcpy(rxF_ext,rxF,12*sizeof(int));
	nb_rb++;
	rxF_ext+=12;
      }
      else {
	rxF     = &rxdataF[aarx][(1+(symbol*(frame_parms->ofdm_symbol_size)))*2];
      }
      rxF+=12;
      rb++;
      
      for (;rb<frame_parms->N_RB_DL;rb++) {
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	if (rb_alloc_ind==1) {
#ifdef DEBUG_ULSCH
	  msg("ulsch_extract_rbs_single: extracting RB %d from %p to %p\n",rb,rxF,rxF_ext);
#endif
	  memcpy(rxF_ext,rxF,24*sizeof(int));
	  nb_rb++;
	  rxF_ext+=24;
	}
	rxF+=24;
      }
    }
  }

  _mm_empty();
  _m_empty();

  return(nb_rb/frame_parms->nb_antennas_rx);
}
