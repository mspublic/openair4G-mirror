#include "PHY/defs.h"
#include "PHY/extern.h"
#include <emmintrin.h>
#include <xmmintrin.h>
//#define DEBUG_DRS

int generate_drs_pusch(LTE_DL_FRAME_PARMS *frame_parms,
		       mod_sym_t *txdataF,
		       short amp,
		       unsigned int sub_frame_number,
		       unsigned int first_rb,
		       unsigned int nb_rb,
		       u8 cyclic_shift) {

  unsigned short b,j,k,l,Msc_RS,Msc_RS_idx,rb,re_offset,symbol_offset,drs_offset;
  unsigned short * Msc_idx_ptr;
  int sub_frame_offset;

  //u32 phase_shift; // phase shift for cyclic delay in DM RS
  //u8 alpha_ind;

  s16 alpha_re[12] = {32767, 28377, 16383,     0,-16384,  -28378,-32768,-28378,-16384,    -1, 16383, 28377};
  s16 alpha_im[12] = {0,     16383, 28377, 32767, 28377,   16383,     0,-16384,-28378,-32768,-28378,-16384};

 
  //phase_shift = (2*M_PI*cyclic_shift)/12;
  Msc_RS = 12*nb_rb;    

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

#ifdef IFFT_FPGA_UE
    re_offset = frame_parms->N_RB_DL*12/2;
    sub_frame_offset = sub_frame_number*frame_parms->symbols_per_tti*frame_parms->N_RB_UL*12;
    symbol_offset = sub_frame_offset + frame_parms->N_RB_UL*12*l;
#else
    re_offset = frame_parms->first_carrier_offset;
    sub_frame_offset = sub_frame_number*frame_parms->symbols_per_tti*frame_parms->ofdm_symbol_size;
    symbol_offset = sub_frame_offset + frame_parms->ofdm_symbol_size*l;
#endif
    
#ifdef DEBUG_DRS
    debug_msg("generate_drs_puch: symbol_offset %d, subframe offset %d, cyclic shift %d\n",symbol_offset,sub_frame_offset,cyclic_shift);
#endif

    for (rb=0;rb<frame_parms->N_RB_UL;rb++) {

      if ((rb >= first_rb) && (rb<(first_rb+nb_rb))) {

#ifdef DEBUG_DRS	
	msg("generate_drs_puch: doing RB %d, re_offset=%d, drs_offset=%d\n",rb,re_offset,drs_offset);
#endif

#ifdef IFFT_FPGA_UE
	if(cyclic_shift == 0)
	  {
	    for (k=0;k<12;k++) {
	      if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] >= 0)) 
		txdataF[symbol_offset+re_offset] = (mod_sym_t) 1;
	      else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] < 0)) 
		txdataF[symbol_offset+re_offset] = (mod_sym_t) 2;
	      else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] >= 0)) 
		txdataF[symbol_offset+re_offset] = (mod_sym_t) 3;
	      else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] < 0)) 
		txdataF[symbol_offset+re_offset] = (mod_sym_t) 4;
	      re_offset++;
	      drs_offset++;
	      if (re_offset >= frame_parms->N_RB_UL*12)
		re_offset=0;
	    }
	  }
	else if(cyclic_shift == 6 )
	  {
	    for (k=0;k<12;k++) {
	      if(k%2 == 0)
		{
		  if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] >= 0)) 
		    txdataF[symbol_offset+re_offset] = (mod_sym_t) 4;
		  else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] < 0)) 
		    txdataF[symbol_offset+re_offset] = (mod_sym_t) 3;
		  else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] >= 0)) 
		    txdataF[symbol_offset+re_offset] = (mod_sym_t) 2;
		  else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] < 0)) 
		    txdataF[symbol_offset+re_offset] = (mod_sym_t) 1;
		}
	      else
		{
		  if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] >= 0)) 
		    txdataF[symbol_offset+re_offset] = (mod_sym_t) 1;
		  else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] < 0)) 
		    txdataF[symbol_offset+re_offset] = (mod_sym_t) 2;
		  else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] >= 0)) 
		    txdataF[symbol_offset+re_offset] = (mod_sym_t) 3;
		  else if ((ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] < 0)) 
		    txdataF[symbol_offset+re_offset] = (mod_sym_t) 4;
		}

	      re_offset++;
	      drs_offset++;
	      if (re_offset >= frame_parms->N_RB_UL*12)
		re_offset=0;
	    }
	  }
#else
	for (k=0;k<12;k++) {
	  if(cyclic_shift == 0){
	    ((short*) txdataF)[2*(symbol_offset + re_offset)]   = (short) (((int) amp * (int) ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1])>>15);
	    ((short*) txdataF)[2*(symbol_offset + re_offset)+1] = (short) (((int) amp * (int) ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1])>>15);
	    
	  }
	  /*
	  else if(cyclic_shift == 6)
	    {
	      ((short*) txdataF)[2*(symbol_offset + re_offset)]   = pow(-1,k+1) * (short) (((int) amp * (int) ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1])>>15);
	      ((short*) txdataF)[2*(symbol_offset + re_offset)+1] = pow(-1,k+1) * (short) (((int) amp * (int) ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1])>>15);

	      }
	  */
	  
	  else if(cyclic_shift == 6) // Cyclic Shift to DMRS
	    {
	      ((short*) txdataF)[2*(symbol_offset + re_offset)]   = 
		(short)	(((((int) (k%2 == 1? 1:-1) * (int) alpha_re[cyclic_shift] * (int) ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1] - 
			    (int) (k%2 == 1? 1:-1) * (int) alpha_im[cyclic_shift] * (int) ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1])>>15) *
			  ((int) amp))>>15);
	      
	      ((short*) txdataF)[2*(symbol_offset + re_offset)+1]   = 
		(short) (((((int) (k%2 == 1? 1:-1) * (int) alpha_re[cyclic_shift] * (int) ul_ref_sigs[0][0][Msc_RS_idx][(drs_offset<<1)+1] + 
			    (int) (k%2 == 1? 1:-1) * (int) alpha_im[cyclic_shift] * (int) ul_ref_sigs[0][0][Msc_RS_idx][drs_offset<<1])>>15) *
			  ((int) amp))>>15);
	    }
	  re_offset++;
	  drs_offset++;
	  if (re_offset >= frame_parms->ofdm_symbol_size)
	    re_offset = 0;
	}
	
#endif
      }
      else {
	re_offset+=12; // go to next RB
	
	// check if we crossed the symbol boundary and skip DC
#ifdef IFFT_FPGA_UE
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

