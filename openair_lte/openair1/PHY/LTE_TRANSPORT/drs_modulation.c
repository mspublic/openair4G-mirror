#include "defs.h"
#include "extern.h"
#include <emmintrin.h>
#include <xmmintrin.h>
//#define DEBUG_DRS

int generate_drs_puch(LTE_DL_FRAME_PARMS *frame_parms,
		      mod_sym_t *txdataF,
		      short amp,
		      unsigned int sub_frame_offset,
		      unsigned int *rb_alloc) {

  unsigned short b,j,k,l,Msc_RS,Msc_RS_idx,rb,re_offset,symbol_offset,drs_offset,rb_alloc_ind;
  unsigned short * Msc_idx_ptr;

  Msc_RS = 0;    
  for (j=0;j<32;j++)
    Msc_RS += (((rb_alloc[0]>>j)&1) + ((rb_alloc[1]>>j)&1) + ((rb_alloc[2]>>j)&1) + ((rb_alloc[3]>>j)&1));
  Msc_RS *= 12;

#ifdef USER_MODE
  Msc_idx_ptr = (unsigned short*) bsearch(&Msc_RS, dftsizes, 33, sizeof(unsigned short), compareints);
  if (Msc_idx_ptr)
    Msc_RS_idx = Msc_idx_ptr - dftsizes;
  else {
    msg("generate_drs_puch: index for Msc_RS=%d not found\n",Msc_RS);
    return(-1);
  }
#else
  for (b=0;b<33;b++) 
    if (Msc_RS==dftsizes[b])
      Msc_RS_idx = b;
#endif
  //msg("Msc_RS = %d, Msc_RS_idx = %d\n",Msc_RS, Msc_RS_idx);

  for (l = (3 - frame_parms->Ncp); l<frame_parms->symbols_per_tti; l += (7 - frame_parms->Ncp)) {

    drs_offset = 0;

#ifdef IFFT_FPGA
    re_offset = frame_parms->N_RB_DL*12/2;
    symbol_offset = sub_frame_offset + frame_parms->N_RB_UL*12*l;
#else
    re_offset = frame_parms->first_carrier_offset;
    symbol_offset = sub_frame_offset + frame_parms->ofdm_symbol_size*l;
#endif
    
#ifdef DEBUG_DRS
    msg("generate_drs_puch: symbol_offset %d, subframe offset %d\n",symbol_offset,sub_frame_offset);
#endif

    for (rb=0;rb<frame_parms->N_RB_UL;rb++) {

      if (rb < 32)
	rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
      else if (rb < 64)
	rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
      else if (rb < 96)
	rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
      else if (rb < 100)
	rb_alloc_ind = (rb_alloc[3]>>(rb-96)) & 1;
      else
	rb_alloc_ind = 0;
      
      if (rb_alloc_ind > 0) { // the current RB is occupied

#ifdef DEBUG_DRS	
	msg("generate_drs_puch: doing RB %d, re_offset=%d, drs_offset=%d\n",rb,re_offset,drs_offset);
#endif

#ifndef IFFT_FPGA
	for (k=0;k<12;k++) {
	  ((short*) txdataF)[2*(symbol_offset + re_offset)]   = (short) (((int) amp * (int) ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1])>>15);
	  ((short*) txdataF)[2*(symbol_offset + re_offset)+1] = (short) (((int) amp * (int) ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1])>>15);
	  re_offset++;
	  drs_offset++;
	  if (re_offset >= frame_parms->ofdm_symbol_size)
	    re_offset = 1;
	}
#else
	for (k=0;k<12;k++) {
	  if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] >= 0)) 
	    txdataF[symbol_offset+re_offset] = (mod_sym_t) 4;
	  else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] < 0)) 
	    txdataF[symbol_offset+re_offset] = (mod_sym_t) 2;
	  else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] >= 0)) 
	    txdataF[symbol_offset+re_offset] = (mod_sym_t) 3;
	  else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] < 0)) 
	    txdataF[symbol_offset+re_offset] = (mod_sym_t) 1;
	  
	  re_offset++;
	  drs_offset++;
	  if (re_offset >= frame_parms->N_RB_UL*12)
	    re_offset=0;
	}
#endif
      } 
      else {
	re_offset+=12; // go to next RB
	
	// check if we crossed the symbol boundary and skip DC
#ifdef IFFT_FPGA
	if (re_offset >= frame_parms->N_RB_DL*12) {
	  if (frame_parms->N_RB_DL&1)  // odd number of RBs 
	    re_offset=6;
	  else                         // even number of RBs (doesn't straddle DC)
	    re_offset=0;  
	}
#else
	if (re_offset >= frame_parms->ofdm_symbol_size) {
	  if (frame_parms->N_RB_DL&1)  // odd number of RBs 
	    re_offset=7;
	  else                         // even number of RBs (doesn't straddle DC)
	    re_offset=1;  
	}
#endif
      }
    }
  }
  
  return(0);
}       

