/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file PHY/LTE_TRANSPORT/dlsch_modulation.c
* \brief Top-level routines for generating the PDSCH physical channel from 36-211, V8.6 2009-03
* \author R. Knopp, F. Kaltenberger
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr
* \note
* \warning
*/
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "PHY/LTE_TRANSPORT/defs.h"
#include "defs.h"

//#define DEBUG_DLSCH_MODULATION 

//#define is_not_pilot(pilots,re,nushift,use2ndpilots) ((pilots==0) || ((re!=nushift) && (re!=nushift+6)&&((re!=nushift+3)||(use2ndpilots==1))&&((re!=nushift+9)||(use2ndpilots==1)))?1:0)

u8 is_not_pilot(u8 pilots, u8 re, u8 nushift, u8 use2ndpilots) {

  u8 offset = (pilots==2)?3:0;
  int nushiftmod3 = nushift%3;
 
  if (pilots==0)
    return(1);

  if (use2ndpilots==1) {  // This is for SISO (mode 1)
    if ((re!=nushift+offset) && (re!=((nushift+6+offset)%12)))
      return(1);
  }
  else { // 2 antenna pilots
    if ((re!=nushiftmod3) && (re!=nushiftmod3+6) && (re!=nushiftmod3+3) && (re!=nushiftmod3+9))
      return(1);
  }
  return(0);
}

void generate_64qam_table(void) {

  int a,b,c,index;


  for (a=-1;a<=1;a+=2) 
    for (b=-1;b<=1;b+=2) 
      for (c=-1;c<=1;c+=2) {
	index = (1+a)*2 + (1+b) + (1+c)/2;  
	qam64_table[index] = -a*(QAM64_n1 + b*(QAM64_n2 + (c*QAM64_n3))); // 0 1 2
      } 
}

void generate_16qam_table(void) {

  int a,b,index;

  for (a=-1;a<=1;a+=2) 
    for (b=-1;b<=1;b+=2) {
	index = (1+a) + (1+b)/2;  
	qam16_table[index] = -a*(QAM16_n1 + (b*QAM16_n2)); 
      } 
}


#ifndef IFFT_FPGA

void layer1prec2A(s32 *antenna0_sample, s32 *antenna1_sample, u8 precoding_index) {

  switch (precoding_index) {

  case 0: // 1 1
    *antenna1_sample=*antenna0_sample;
    break;

  case 1: // 1 -1
    ((s16 *)antenna1_sample)[0] = -((s16 *)antenna0_sample)[0];
    ((s16 *)antenna1_sample)[1] = -((s16 *)antenna0_sample)[1];
    break;

  case 2: // 1 j
    ((s16 *)antenna1_sample)[0] = -((s16 *)antenna0_sample)[1];
    ((s16 *)antenna1_sample)[1] = ((s16 *)antenna0_sample)[0];
    break;

  case 3: // 1 -j
    ((s16 *)antenna1_sample)[0] = ((s16 *)antenna0_sample)[1];
    ((s16 *)antenna1_sample)[1] = -((s16 *)antenna0_sample)[0];
    break;
  }

  // normalize
  /*  ((s16 *)antenna0_sample)[0] = (s16)((((s16 *)antenna0_sample)[0]*ONE_OVER_SQRT2_Q15)>>15);  
  ((s16 *)antenna0_sample)[1] = (s16)((((s16 *)antenna0_sample)[1]*ONE_OVER_SQRT2_Q15)>>15);  ((s16 *)antenna1_sample)[0] = (s16)((((s16 *)antenna1_sample)[0]*ONE_OVER_SQRT2_Q15)>>15);  
  ((s16 *)antenna1_sample)[1] = (s16)((((s16 *)antenna1_sample)[1]*ONE_OVER_SQRT2_Q15)>>15);  */
} 

int allocate_REs_in_RB(mod_sym_t **txdataF,
		       u32 *jj,
		       u16 re_offset,
		       u32 symbol_offset,
		       u8 *output,
		       MIMO_mode_t mimo_mode,
		       u8 nu,
		       u8 pilots,
		       u8 mod_order,
		       u8 precoder_index,
		       s16 amp,
		       int16_t *qam_table_s,
		       u32 *re_allocated,
		       u8 skip_dc,
		       u8 skip_half,
		       u8 use2ndpilots,
		       LTE_DL_FRAME_PARMS *frame_parms) {

  u32 tti_offset,aa;
  u8 re;
  u8 qam64_table_offset_re = 0;
  u8 qam64_table_offset_im = 0;
  u8 qam16_table_offset_re = 0;
  u8 qam16_table_offset_im = 0;
  s16 gain_lin_QPSK;//,gain_lin_16QAM1,gain_lin_16QAM2;
  s16 re_off=re_offset;
  gain_lin_QPSK = (s16)((amp*ONE_OVER_SQRT2_Q15)>>15);  
  u8 first_re,last_re;
  s32 tmp_sample1,tmp_sample2;
  s16 tmp_amp=amp;

  /*
  switch (mod_order) {
  case 2:
    // QPSK single stream
    
    break;
  case 4:
    //16QAM Single stream
    gain_lin_16QAM1 = (s16)(((s32)amp*QAM16_n1)>>15);
    gain_lin_16QAM2 = (s16)(((s32)amp*QAM16_n2)>>15);
    
    break;
    
  case 6:
    //64QAM Single stream
    break;
  default:
    break;
  }
*/

#ifdef DEBUG_DLSCH_MODULATION
  printf("allocate_re (mod %d): symbol_offset %d re_offset %d (%d,%d), jj %d -> %d,%d, nu %d\n",mod_order,symbol_offset,re_offset,skip_dc,skip_half,*jj, output[*jj], output[1+*jj],nu);
#endif

  first_re=0;
  last_re=12;

  if (skip_half==1) 
    last_re=6;
  else if (skip_half==2)
    first_re=6;
  
  for (re=first_re;re<last_re;re++) {
    
    if ((skip_dc == 1) && (re==6))
      re_off=re_off - frame_parms->ofdm_symbol_size+1;
    
    tti_offset = symbol_offset + re_off + re;
    
    // check that re is not a pilot
    if (is_not_pilot(pilots,re,frame_parms->nushift,use2ndpilots)==1) { 
      //     printf("re %d (jj %d)\n",re,*jj);
      *re_allocated = *re_allocated + 1;
      
      if (mimo_mode == SISO) {  //SISO mapping
	
	switch (mod_order) {
	case 2:  //QPSK
	  //	  printf("%d : %d,%d => ",tti_offset,((s16*)&txdataF[0][tti_offset])[0],((s16*)&txdataF[0][tti_offset])[1]);
	  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
	    ((s16*)&txdataF[aa][tti_offset])[0] += (output[*jj]==1) ? (-gain_lin_QPSK) : gain_lin_QPSK; //I //b_i
	  *jj = *jj + 1;
	  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
	    ((s16*)&txdataF[aa][tti_offset])[1] += (output[*jj]==1) ? (-gain_lin_QPSK) : gain_lin_QPSK; //Q //b_{i+1}
	  *jj = *jj + 1;

	  //	  printf("%d,%d\n",((s16*)&txdataF[0][tti_offset])[0],((s16*)&txdataF[0][tti_offset])[1]);
	  break;
	  
	case 4:  //16QAM
	  
	  qam16_table_offset_re = 0;
	  qam16_table_offset_im = 0;
	  if (output[*jj] == 1)
	    qam16_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=2;
	  *jj=*jj+1;
	  
	  
	  if (output[*jj] == 1)
	    qam16_table_offset_re+=1;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
	    ((s16 *)&txdataF[aa][tti_offset])[0]+=qam_table_s[qam16_table_offset_re];
	    ((s16 *)&txdataF[aa][tti_offset])[1]+=qam_table_s[qam16_table_offset_im];
	    //	    ((s16 *)&txdataF[aa][tti_offset])[0]+=(s16)(((s32)amp*qam16_table[qam16_table_offset_re])>>15);
	    //	    ((s16 *)&txdataF[aa][tti_offset])[1]+=(s16)(((s32)amp*qam16_table[qam16_table_offset_im])>>15);
	  }
	  
	  break;
	  
	case 6:  //64QAM
	  
	  
	  qam64_table_offset_re = 0;
	  qam64_table_offset_im = 0;

	  if (output[*jj] == 1)
	    qam64_table_offset_re+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=1;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
	    ((s16 *)&txdataF[aa][tti_offset])[0]+=qam_table_s[qam64_table_offset_re];//(s16)(((s32)amp*qam64_table[qam64_table_offset_re])>>15);
	    ((s16 *)&txdataF[aa][tti_offset])[1]+=qam_table_s[qam64_table_offset_im];//(s16)(((s32)amp*qam64_table[qam64_table_offset_im])>>15);
	  }
	  break;
	  
	}
      }
            
      else if (mimo_mode == ALAMOUTI){

          // normalization for 2 tx antennas
          amp = (s16)(((s32)tmp_amp*ONE_OVER_SQRT2_Q15)>>15);

	switch (mod_order) {
	case 2:  //QPSK
	  
	  // first antenna position n -> x0
	  
      ((s16*)&tmp_sample1)[0] = (output[*jj]==1) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  *jj=*jj+1;
	  ((s16*)&tmp_sample1)[1] = (output[*jj]==1) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  *jj=*jj+1;
	  
	  // second antenna position n -> -x1*
	  
	  ((s16*)&tmp_sample2)[0] = (output[*jj]==1) ? (gain_lin_QPSK) : -gain_lin_QPSK;
	  *jj=*jj+1;
	  ((s16*)&tmp_sample2)[1] = (output[*jj]==1) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  *jj=*jj+1;
	  
      // normalization for 2 tx antennas
      ((s16*)&txdataF[0][tti_offset])[0] += (s16)((((s16*)&tmp_sample1)[0]*ONE_OVER_SQRT2_Q15)>>15);
	  ((s16*)&txdataF[0][tti_offset])[1] += (s16)((((s16*)&tmp_sample1)[1]*ONE_OVER_SQRT2_Q15)>>15);
      ((s16*)&txdataF[1][tti_offset])[0] += (s16)((((s16*)&tmp_sample2)[0]*ONE_OVER_SQRT2_Q15)>>15);
      ((s16*)&txdataF[1][tti_offset])[1] += (s16)((((s16*)&tmp_sample2)[1]*ONE_OVER_SQRT2_Q15)>>15);

	  break;
	  
	case 4:  //16QAM
	  
	  // Antenna 0 position n 
	  
	  qam16_table_offset_re = 0;
	  qam16_table_offset_im = 0;
	  if (output[*jj] == 1)
	    qam16_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=2;
	  *jj=*jj+1;
	  
	  
	  if (output[*jj] == 1)
	    qam16_table_offset_re+=1;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  ((s16 *)&txdataF[0][tti_offset])[0]+=(s16)(((s32)amp*qam16_table[qam16_table_offset_re])>>15);
	  ((s16 *)&txdataF[0][tti_offset])[1]+=(s16)(((s32)amp*qam16_table[qam16_table_offset_im])>>15);
	  
	  // Antenna 1 position n Real part -> -x1*
	  
	  qam16_table_offset_re = 0;
	  qam16_table_offset_im = 0;
	  if (output[*jj] == 1)
	    qam16_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=2;
	  *jj=*jj+1;
	  
	  
	  if (output[*jj] == 1)
	    qam16_table_offset_re+=1;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  ((s16 *)&txdataF[1][tti_offset])[0]+=-(s16)(((s32)amp*qam16_table[qam16_table_offset_re])>>15);
	  ((s16 *)&txdataF[1][tti_offset])[1]+=(s16)(((s32)amp*qam16_table[qam16_table_offset_im])>>15);
	  
	  
	  break;
	case 6:   // 64-QAM
	  
	  // Antenna 0
	  qam64_table_offset_re = 0;
	  qam64_table_offset_im = 0;

	  if (output[*jj] == 1)
	    qam64_table_offset_re+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=1;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  ((s16 *)&txdataF[0][tti_offset])[0]+=(s16)(((s32)amp*qam64_table[qam64_table_offset_re])>>15);
	  ((s16 *)&txdataF[0][tti_offset])[1]+=(s16)(((s32)amp*qam64_table[qam64_table_offset_im])>>15);
	  
	  
	  // Antenna 1 => -x1*
	  qam64_table_offset_re = 0;
	  qam64_table_offset_im = 0;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=1;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  ((s16 *)&txdataF[1][tti_offset])[0]+=-(s16)(((s32)amp*qam64_table[qam64_table_offset_re])>>15);
	  ((s16 *)&txdataF[1][tti_offset])[1]+=(s16)(((s32)amp*qam64_table[qam64_table_offset_im])>>15);
	  
	  break;
	}
	// fill in the rest of the ALAMOUTI precoding
	if (is_not_pilot(pilots,re + 1,frame_parms->nushift,use2ndpilots)==1) {
	  ((s16 *)&txdataF[0][tti_offset+1])[0] += -((s16 *)&txdataF[1][tti_offset])[0]; //x1
	  ((s16 *)&txdataF[0][tti_offset+1])[1] += ((s16 *)&txdataF[1][tti_offset])[1];
	  ((s16 *)&txdataF[1][tti_offset+1])[0] += ((s16 *)&txdataF[0][tti_offset])[0];  //x0*
	  ((s16 *)&txdataF[1][tti_offset+1])[1] += -((s16 *)&txdataF[0][tti_offset])[1];
	}
	else {
	  ((s16 *)&txdataF[0][tti_offset+2])[0] += -((s16 *)&txdataF[1][tti_offset])[0]; //x1
	  ((s16 *)&txdataF[0][tti_offset+2])[1] += ((s16 *)&txdataF[1][tti_offset])[1];
	  ((s16 *)&txdataF[1][tti_offset+2])[0] += ((s16 *)&txdataF[0][tti_offset])[0];  //x0*
	  ((s16 *)&txdataF[1][tti_offset+2])[1] += -((s16 *)&txdataF[0][tti_offset])[1];
	}
      }
      else if (mimo_mode == ANTCYCLING ) {
	switch (mod_order) {
	case 2:  //QPSK
	  
	  ((s16*)&txdataF[re&1][tti_offset])[0] += (output[*jj]==1) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  *jj = *jj + 1;
	  ((s16*)&txdataF[re&1][tti_offset])[1] += (output[*jj]==1) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  *jj = *jj + 1;
	  break;
	  
	case 4:  //16QAM
	  
	  qam16_table_offset_re = 0;
	  qam16_table_offset_im = 0;

	  if (output[*jj] == 1)
	    qam16_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=2;
	  *jj=*jj+1;
	  
	  
	  if (output[*jj] == 1)
	    qam16_table_offset_re+=1;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  ((s16 *)&txdataF[re&1][tti_offset])[0]+=(s16)(((s32)amp*qam16_table[qam16_table_offset_re])>>15);
	  ((s16 *)&txdataF[re&1][tti_offset])[1]+=(s16)(((s32)amp*qam16_table[qam16_table_offset_im])>>15);
	  
	  
	  break;
	  
	case 6:  //64QAM
	  
	  qam64_table_offset_re = 0;
	  qam64_table_offset_im = 0;

	  if (output[*jj] == 1)
	    qam64_table_offset_re+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=1;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  ((s16 *)&txdataF[re&1][tti_offset])[0]+=(s16)(((s32)amp*qam64_table[qam64_table_offset_re])>>15);
	  ((s16 *)&txdataF[re&1][tti_offset])[1]+=(s16)(((s32)amp*qam64_table[qam64_table_offset_im])>>15);
	  
	}
      }
       
      else if ((mimo_mode >= UNIFORM_PRECODING11)&&(mimo_mode <= PUSCH_PRECODING1)) {
	// this is for transmission modes 4-6 (1 layer)
         
          amp = (s16)(((s32)tmp_amp*ONE_OVER_SQRT2_Q15)>>15);

	switch (mod_order) {
	case 2:  //QPSK

	  ((s16*)&tmp_sample1)[0] = (output[*jj]==1) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  *jj = *jj + 1;
	  ((s16*)&tmp_sample1)[1] = (output[*jj]==1) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  *jj = *jj + 1;

      // normalization for 2 tx antennas
	  ((s16*)&txdataF[0][tti_offset])[0] += (s16)((((s16*)&tmp_sample1)[0]*ONE_OVER_SQRT2_Q15)>>15);
	  ((s16*)&txdataF[0][tti_offset])[1] += (s16)((((s16*)&tmp_sample1)[1]*ONE_OVER_SQRT2_Q15)>>15);

	  if (frame_parms->nb_antennas_tx == 2) {
	    layer1prec2A(&tmp_sample1,&tmp_sample2,precoder_index);
        ((s16*)&txdataF[1][tti_offset])[0] += (s16)((((s16*)&tmp_sample2)[0]*ONE_OVER_SQRT2_Q15)>>15);
        ((s16*)&txdataF[1][tti_offset])[1] += (s16)((((s16*)&tmp_sample2)[1]*ONE_OVER_SQRT2_Q15)>>15);
	  }

	  break;
	  
	case 4:  //16QAM
	  
	  qam16_table_offset_re = 0;
	  qam16_table_offset_im = 0;

	  if (output[*jj] == 1)
	    qam16_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=2;
	  *jj=*jj+1;
	  
	  
	  if (output[*jj] == 1)
	    qam16_table_offset_re+=1;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=1;
	  *jj=*jj+1;
	  
	   ((s16*)&tmp_sample1)[0] = (s16)(((s32)amp*qam16_table[qam16_table_offset_re])>>15);
	   ((s16*)&tmp_sample1)[1] = (s16)(((s32)amp*qam16_table[qam16_table_offset_im])>>15);

	   ((s16 *)&txdataF[0][tti_offset])[0] += ((s16*)&tmp_sample1)[0];
	   ((s16 *)&txdataF[0][tti_offset])[1] += ((s16*)&tmp_sample1)[1];
	  
	  if (frame_parms->nb_antennas_tx == 2) {
	    layer1prec2A(&tmp_sample1,&tmp_sample2,precoder_index);
	    ((s16*)&txdataF[1][tti_offset])[0] += ((s16*)&tmp_sample2)[0];
	    ((s16*)&txdataF[1][tti_offset])[1] += ((s16*)&tmp_sample2)[1];
	  }

	  break;
	  
	case 6:  //64QAM
	  
	  
	  qam64_table_offset_re = 0;
	  qam64_table_offset_im = 0;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=1;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  ((s16*)&tmp_sample1)[0] = (s16)(((s32)amp*qam64_table[qam64_table_offset_re])>>15);
	  ((s16*)&tmp_sample1)[1] = (s16)(((s32)amp*qam64_table[qam64_table_offset_im])>>15);

	  ((s16 *)&txdataF[0][tti_offset])[0] += ((s16*)&tmp_sample1)[0];
	  ((s16 *)&txdataF[0][tti_offset])[1] += ((s16*)&tmp_sample1)[1];
	  
	  if (frame_parms->nb_antennas_tx == 2) {
	    layer1prec2A(&tmp_sample1,&tmp_sample2,precoder_index);
	    ((s16*)&txdataF[1][tti_offset])[0] += ((s16*)&tmp_sample2)[0];
	    ((s16*)&txdataF[1][tti_offset])[1] += ((s16*)&tmp_sample2)[1];
	  }
	  
	  break;
	  
	}
      }
      
      else {
	msg("allocate_REs_in_RB() [dlsch.c] : ERROR, unknown mimo_mode %d\n",mimo_mode);
	return(-1);
      }
      
     
    
      if (mimo_mode == ALAMOUTI) {
	re++;  // adjacent carriers are taken care of by precoding
	*re_allocated = *re_allocated + 1;
	if (is_not_pilot(pilots,re,frame_parms->nushift,use2ndpilots)==0) {
	  re++;  
	  *re_allocated = *re_allocated + 1;
	}
      }
    }
  }
  return(0);
}

#else // ifndef IFFT_FPGA

static mod_sym_t qpsk_precoder[4][4]   = {{0,1,2,3}, {3,2,1,0}, {1,3,0,2}, {2,0,3,1}};
static mod_sym_t qam16_precoder[4][16] = {{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
					  {10,11,8,9,14,15,12,13,2,3,0,1,6,7,4,5},
					  {2,6,10,14,3,7,11,15,0,4,8,12,1,5,9,13},
					  {8,12,0,4,9,13,1,5,10,14,2,6,11,15,3,7}};
static mod_sym_t qam64_precoder[4][64] = {{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63},
					  {36,37,38,39,32,33,34,35,44,45,46,47,40,41,42,43,52,53,54,55,48,49,50,51,60,61,62,63,56,57,58,59,4,5,6,7,0,1,2,3,12,13,14,15,8,9,10,11,20,21,22,23,16,17,18,19,28,29,30,31,24,25,26,27},
					  {4,12,20,28,36,44,52,60,5,13,21,29,37,45,53,61,6,14,22,30,38,46,54,62,7,15,23,31,39,47,55,63,0,8,16,24,32,40,48,56,1,9,17,25,33,41,49,57,2,10,18,26,34,42,50,58,3,11,19,27,35,43,51,59},
					  {32,40,48,56,0,8,16,24,33,41,49,57,1,9,17,25,34,42,50,58,2,10,18,26,35,43,51,59,3,11,19,27,36,44,52,60,4,12,20,28,37,45,53,61,5,13,21,29,38,46,54,62,6,14,22,30,39,47,55,63,7,15,23,31}};


int allocate_REs_in_RB(mod_sym_t **txdataF,
		       u32 *jj,
		       u16 re_offset,
		       u32 symbol_offset,
		       u8 *output,
		       MIMO_mode_t mimo_mode,
		       u8 nu,
		       u8 pilots,
		       u8 mod_order,
		       u8 precoder_index,
		       s16 amp,
		       int16_t *qam_table_s,
		       u32 *re_allocated,
		       u8 skip_dc,
		       u8 skip_half,
		       u8 use2ndpilots,
		       LTE_DL_FRAME_PARMS *frame_parms) {

  u32 tti_offset,aa;
  u8 re;
  u8 qam64_table_offset = 0;
  u8 qam16_table_offset = 0;
  u8 qpsk_table_offset = 0; 
  u8 qam64_table_offset2 = 0; // for second symbol if Alamouti is used
  u8 qam16_table_offset2 = 0;
  u8 qpsk_table_offset2 = 0;
  s16 re_off=re_offset;
  u8 first_re,last_re;  

  if (nu>1) {
    msg("dlsch_modulation.c: allocate_REs_in_RB, error, unknown layer index %d\n",nu);
    return(-1);
  }

  first_re=0;
  last_re=12;

  if (skip_half==1)
    last_re=6;
  else if (skip_half==2)
    first_re=6;

  for (re=first_re;re<last_re;re++) {

    if ((skip_dc == 1) && (re==6))
      re_off=re_off - frame_parms->N_RB_DL*12;

    tti_offset = symbol_offset + re_off + re;

    //msg("pilots %d, re %d, frame_parms->nushift %d, use2ndpilots %d\n",pilots,re,frame_parms->nushift,use2ndpilots); 
    if (is_not_pilot(pilots,re,frame_parms->nushift,use2ndpilots)==1) { 
      //msg("dlsch_modulation tti_offset %d (re %d)\n",tti_offset,re);
      *re_allocated = *re_allocated + 1;

	if (mimo_mode == SISO) {  //SISO mapping
	  switch (mod_order) {
	  case 2:  //QPSK

	    qpsk_table_offset = MOD_TABLE_QPSK_OFFSET;
	    if (output[*jj] == 1) //b_i
	      qpsk_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1) //b_{i+1}
	      qpsk_table_offset+=1;
	    *jj=*jj+1;

	    for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
	      txdataF[aa][tti_offset] = (mod_sym_t) qpsk_table_offset;

	    break;
	    
	  case 4:  //16QAM

	    qam16_table_offset = MOD_TABLE_16QAM_OFFSET;
	    if (output[*jj] == 1)
	      qam16_table_offset+=8;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset+=4;
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1)
	      qam16_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset+=1;
	    *jj=*jj+1;
	    
	    for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
	      txdataF[aa][tti_offset] = (mod_sym_t) qam16_table_offset;
	    	    
	    break;
	   
	  case 6:  //64QAM

		    
	    qam64_table_offset = MOD_TABLE_64QAM_OFFSET;
	    if (output[*jj] == 1)
	      qam64_table_offset+=32;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=16;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=8;
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1)
	      qam64_table_offset+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=1;
	    *jj=*jj+1;
	    
	    for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
	      txdataF[aa][tti_offset] = (mod_sym_t) qam64_table_offset;
	    break;

	  }
	}
	else if (mimo_mode == ALAMOUTI){
	  
	  switch (mod_order) {
	  case 2:  //QPSK

	    
	    qpsk_table_offset = MOD_TABLE_QPSK_OFFSET;  //x0
	    qpsk_table_offset2 = MOD_TABLE_QPSK_OFFSET;  //x0*

	    if (output[*jj] == 1) { //real
	      qpsk_table_offset+=2;
	      qpsk_table_offset2+=2;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) //imag
	      qpsk_table_offset+=1;
	    else
	      qpsk_table_offset2+=1;
	    *jj=*jj+1;


	    txdataF[0][tti_offset] = (mod_sym_t) qpsk_table_offset;      // x0
	    if (is_not_pilot(pilots,re+1,frame_parms->nushift,use2ndpilots)==1) {
	      txdataF[1][tti_offset+1] = (mod_sym_t) qpsk_table_offset2;   // x0*
	    }
	    else {
	      txdataF[1][tti_offset+2] = (mod_sym_t) qpsk_table_offset2;   // x0*
	    }


	    qpsk_table_offset = MOD_TABLE_QPSK_OFFSET; //-x1*
	    qpsk_table_offset2 = MOD_TABLE_QPSK_OFFSET; //x1

	    if (output[*jj] == 1)    // flipping bit for real part of symbol means taking -x1*
	      qpsk_table_offset2+=2;
	    else
	      qpsk_table_offset+=2;
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qpsk_table_offset+=1;
	      qpsk_table_offset2+=1;
	    }
	    *jj=*jj+1;

	    txdataF[1][tti_offset] = (mod_sym_t) qpsk_table_offset;     // -x1*
	    if (is_not_pilot(pilots,re+1,frame_parms->nushift,use2ndpilots)==1) {
	      txdataF[0][tti_offset+1] = (mod_sym_t) qpsk_table_offset2;  // x1
	    }
	    else {
	      txdataF[0][tti_offset+2] = (mod_sym_t) qpsk_table_offset2;  // x1
	    }
	    //	    printf("txdataF[0][tti_offset] %d %d\n",txdataF[0][tti_offset],txdataF[0][tti_offset+1]); 
	    break;

	  case 4:  //16QAM
	    
	    // Antenna 0 position n 

	    qam16_table_offset = MOD_TABLE_16QAM_OFFSET; //x0
	    qam16_table_offset2 = MOD_TABLE_16QAM_OFFSET; //x0* = flip second bit
	    if (output[*jj] == 1) {
	      qam16_table_offset+=8;
	      qam16_table_offset2+=8;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) 
	      qam16_table_offset+=4;
	    else
	      qam16_table_offset2+=4;
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1) {
	      qam16_table_offset+=2;
	      qam16_table_offset2+=2;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam16_table_offset+=1;
	      qam16_table_offset2+=1;
	    }
	    *jj=*jj+1;
	    
	    txdataF[0][tti_offset] = (mod_sym_t) qam16_table_offset; //x0
	    if (is_not_pilot(pilots,re+1,frame_parms->nushift,use2ndpilots)==1) {
	      txdataF[1][tti_offset+1] = (mod_sym_t) qam16_table_offset2; //x0*
	    }
	    else {
	      txdataF[1][tti_offset+2] = (mod_sym_t) qam16_table_offset2; //x0*
	    }

	    qam16_table_offset = MOD_TABLE_16QAM_OFFSET; //-x1* = flip first bit
	    qam16_table_offset2 = MOD_TABLE_16QAM_OFFSET; //x1
	    if (output[*jj] == 1)
	      qam16_table_offset2+=8;
	    else
	      qam16_table_offset+=8;
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam16_table_offset2+=4;
	      qam16_table_offset+=4;
	    }
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1) {
	      qam16_table_offset+=2;
	      qam16_table_offset2+=2;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam16_table_offset+=1;
	      qam16_table_offset2+=1;
	    }
	    *jj=*jj+1;
	    
	    txdataF[1][tti_offset] = (mod_sym_t) qam16_table_offset;  //-x1*
	    if (is_not_pilot(pilots,re+1,frame_parms->nushift,use2ndpilots)==1) {
	      txdataF[0][tti_offset+1] = (mod_sym_t) qam16_table_offset2; //x1
	    }
	    else {
	      txdataF[0][tti_offset+2] = (mod_sym_t) qam16_table_offset2; //x1
	    }
	    break;
	  case 6:   // 64-QAM

	    // Antenna 0
	    qam64_table_offset = MOD_TABLE_64QAM_OFFSET; //x0
	    qam64_table_offset2 = MOD_TABLE_64QAM_OFFSET; //x0*
	    if (output[*jj] == 1) {
	      qam64_table_offset+=32;
	      qam64_table_offset2+=32;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=16;
	      qam64_table_offset2+=16;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=8;
	      qam64_table_offset2+=8;
	    }
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1)
	      qam64_table_offset+=4;
	    else
	      qam64_table_offset2+=4;
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=2;
	      qam64_table_offset2+=2;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=1;
	      qam64_table_offset2+=1;
	    }
	    *jj=*jj+1;
	    
	    txdataF[0][tti_offset] = (mod_sym_t) qam64_table_offset; //x0
	    if (is_not_pilot(pilots,re+1,frame_parms->nushift,use2ndpilots)==1) {
	      txdataF[1][tti_offset+1] = (mod_sym_t) qam64_table_offset2; //x0*
	    }
	    else {
	      txdataF[1][tti_offset+2] = (mod_sym_t) qam64_table_offset2; //x0*
	    }
	    qam64_table_offset = MOD_TABLE_64QAM_OFFSET; //-x1*
	    qam64_table_offset2 = MOD_TABLE_64QAM_OFFSET; //x1
	    if (output[*jj] == 1)
	      qam64_table_offset2+=32;
	    else
	      qam64_table_offset+=32;
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=16;
	      qam64_table_offset2+=16;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=8;
	      qam64_table_offset2+=8;
	    }
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1) {
	      qam64_table_offset+=4;
	      qam64_table_offset2+=4;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=2;
	      qam64_table_offset2+=2;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=1;
	      qam64_table_offset2+=1;
	    }
	    *jj=*jj+1;
	    
	    txdataF[1][tti_offset] = (mod_sym_t) qam64_table_offset; //-x1*
	    if (is_not_pilot(pilots,re+1,frame_parms->nushift,use2ndpilots)==1) {
	      txdataF[0][tti_offset+1] = (mod_sym_t) qam64_table_offset2; //x1
	    }
	    else {
	      txdataF[0][tti_offset+2] = (mod_sym_t) qam64_table_offset2; //x1
	    }
	    break;
	  }
	}
	/*
	else if (mimo_mode == ANTCYCLING ) {

	}
	else if (mimo_mode == DUALSTREAM) {

	}
	*/
	
	else if ((mimo_mode >= UNIFORM_PRECODING11)&&(mimo_mode <= PUSCH_PRECODING1)) {
	  // this is for transmission modes 4-6 (1 layer)
	  switch (mod_order) {
	  case 2:  //QPSK

	    
	    qpsk_table_offset = 1;  //x0 antenna 0

	    if (output[*jj] == 1) { //real
	      qpsk_table_offset+=1;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1)  {//imag
	      qpsk_table_offset+=2;
	    }

	    *jj=*jj+1;

	    txdataF[0][tti_offset] = (mod_sym_t) qpsk_table_offset;      // x0
	    txdataF[1][tti_offset] = (mod_sym_t) (1+qpsk_precoder[precoder_index][qpsk_table_offset-1]);   // prec(x0)

	    //	    printf("precoding %d (index %d): (%d,%d)\n",tti_offset, precoder_index,txdataF[0][tti_offset]-1,txdataF[1][tti_offset]-1);

	    
	    break;

	  case 4:

	    qam16_table_offset = 5;
	    if (output[*jj] == 1)
	      qam16_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset+=1;
	    *jj=*jj+1;
	    

	    if (output[*jj] == 1)
	      qam16_table_offset+=8;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset+=4;
	    *jj=*jj+1;

	    txdataF[0][tti_offset] = (mod_sym_t) qam16_table_offset;
	    txdataF[1][tti_offset] = (mod_sym_t) (5+qam16_precoder[precoder_index][qam16_table_offset-5]);

	    break;

	  case 6:  //64QAM

		    
	    qam64_table_offset = 21;
	    if (output[*jj] == 1)
	      qam64_table_offset+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=1;
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1)
	      qam64_table_offset+=32;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=16;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=8;
	    *jj=*jj+1;
	    
	    txdataF[0][tti_offset] = (mod_sym_t) qam64_table_offset;
	    txdataF[1][tti_offset] = (mod_sym_t) (21+qam64_precoder[precoder_index][qam64_table_offset-21]);
	    break;

	  }
	}
	
	else {
	  msg("allocate_REs_in_RB() [dlsch.c] : ERROR, unknown mimo_mode %d\n",mimo_mode);
	  return(-1);
	}

	if (mimo_mode == ALAMOUTI) {
	  re++;  // adjacent carriers are taken care of by precoding
	  *re_allocated = *re_allocated + 1;
	  if (is_not_pilot(pilots,re,frame_parms->nushift,use2ndpilots)==0) { // if the next position is a pilot, skip it
	    re++;  
	    *re_allocated = *re_allocated + 1;
	  }
	}
    }
  }
  return(0);
}
#endif

u8 get_pmi_5MHz(MIMO_mode_t mode,u32 pmi_alloc,u16 rb) {

  //  printf("Getting pmi for RB %d => %d\n",rb,(pmi_alloc>>((rb>>2)<<1))&3);
  if (mode <= PUSCH_PRECODING1)
    return((pmi_alloc>>((rb>>2)<<1))&3);
  else
    return((pmi_alloc>>(rb>>2))&1);
}

int dlsch_modulation(mod_sym_t **txdataF,
		     s16 amp,
		     u32 subframe_offset,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     u8 num_pdcch_symbols,
		     LTE_eNB_DLSCH_t *dlsch){

  u8 nsymb;
  u8 harq_pid = dlsch->current_harq_pid;
  u32 i,jj,re_allocated,symbol_offset;
  u16 l,rb,re_offset;
  u32 rb_alloc_ind;
  u32 *rb_alloc = dlsch->rb_alloc;
  u8 pilots=0;
  u8 skip_dc,skip_half;
  u8 mod_order = get_Qm(dlsch->harq_processes[harq_pid]->mcs);
  s16 amp_rho_a, amp_rho_b;
  int16_t qam16_table_a[4],qam64_table_a[8],qam16_table_b[4],qam64_table_b[8];
  int16_t *qam_table_s;

  nsymb = (frame_parms->Ncp==0) ? 14:12;
  
  amp_rho_a = (s16)(((s32)amp*dlsch->sqrt_rho_a)>>13);
  amp_rho_b = (s16)(((s32)amp*dlsch->sqrt_rho_b)>>13);

  if (mod_order == 4)
    for (i=0;i<4;i++) {
      qam16_table_a[i] = (int16_t)(((int32_t)qam16_table[i]*amp_rho_a)>>15);
      qam16_table_b[i] = (int16_t)(((int32_t)qam16_table[i]*amp_rho_b)>>15);
    }
  else if (mod_order == 6)
    for (i=0;i<8;i++) {
      qam64_table_a[i] = (int16_t)(((int32_t)qam64_table[i]*amp_rho_a)>>15);
      qam64_table_b[i] = (int16_t)(((int32_t)qam64_table[i]*amp_rho_b)>>15);
    } 
  //Modulation mapping (difference w.r.t. LTE specs)
  
  jj=0;
  re_allocated=0;
  //  printf("num_pdcch_symbols %d, nsymb %d\n",num_pdcch_symbols,nsymb);
  for (l=num_pdcch_symbols;l<nsymb;l++) {
    
#ifdef DEBUG_DLSCH_MODULATION
    msg("Generating DLSCH (harq_pid %d,mimo %d, pmi_alloc %x, mod %d, nu %d, rb_alloc[0] %d) in %d\n",harq_pid,dlsch->harq_processes[harq_pid]->mimo_mode,pmi2hex_2Ar1(dlsch->pmi_alloc),mod_order, dlsch->layer_index, rb_alloc[0], l);
#endif    

    if (frame_parms->Ncp==0) { // normal prefix
      if ((l==4)||(l==11))
	pilots=2;   // pilots in nushift+3, nushift+9
      else if (l==7)
	pilots=1;   // pilots in nushift, nushift+6
      else
	pilots=0;
    }
    else {
      if ((l==3)||(l==9))
	pilots=2;
      else if (l==6)
	pilots=1;
      else
	pilots=0;
    }

#ifdef IFFT_FPGA
    re_offset = frame_parms->N_RB_DL*12/2;
    symbol_offset = (u32)frame_parms->N_RB_DL*12*(l+(subframe_offset*nsymb));
#else
    re_offset = frame_parms->first_carrier_offset;
    symbol_offset = (u32)frame_parms->ofdm_symbol_size*(l+(subframe_offset*nsymb));
#endif

    //for (aa=0;aa<frame_parms->nb_antennas_tx;aa++)
    //	memset(&txdataF[aa][symbol_offset],0,frame_parms->ofdm_symbol_size<<2);
    //printf("symbol_offset %d,subframe offset %d : pilots %d\n",symbol_offset,subframe_offset,pilots);
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {

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
	
	// check for PBCH
	skip_half=0;
	if ((frame_parms->N_RB_DL&1) == 1) { // ODD N_RB_DL

	  if ((rb==frame_parms->N_RB_DL>>1))
	    skip_dc = 1;
	  else
	    skip_dc = 0;
	  // PBCH
	  if ((subframe_offset==0) && 
	      (rb>((frame_parms->N_RB_DL>>1)-3)) && 
	      (rb<((frame_parms->N_RB_DL>>1)+3)) && 
	      (l>=(nsymb>>1)) && 
	      (l<((nsymb>>1) + 4))) {
	    rb_alloc_ind = 0;
	  }
	  //PBCH subframe 0, symbols nsymb>>1 ... nsymb>>1 + 3
	  if ((subframe_offset==0) && 
	      (rb==((frame_parms->N_RB_DL>>1)-3)) && 
	      (l>=(nsymb>>1)) && 
	      (l<((nsymb>>1) + 4)))
	    skip_half=1;
	  else if ((subframe_offset==0) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	    skip_half=2;
	
	  if (frame_parms->frame_type == TDD) { // TDD
	    //SSS TDD
	    if (((subframe_offset==0)||(subframe_offset==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==(nsymb-1)) ) {
	      rb_alloc_ind = 0;
	    }
	    //SSS TDD
	    if (((subframe_offset==0)||(subframe_offset==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==(nsymb-1)))
	      skip_half=1;
	    else if (((subframe_offset==0)||(subframe_offset==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==(nsymb-1)))
	      skip_half=2;
	    
	    //PSS TDD
	    if (((subframe_offset==1) || (subframe_offset==6)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==2) ) {
	      rb_alloc_ind = 0;
	    }
	    //PSS TDD
	    if (((subframe_offset==1)||(subframe_offset==6)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==2))
	      skip_half=1;
	    else if (((subframe_offset==1)||(subframe_offset==6)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==2))
	      skip_half=2;
	  }
	  else {
	    //PSS FDD
	    if (((subframe_offset==0)||(subframe_offset==5)) && 
		(rb>((frame_parms->N_RB_DL>>1)-3)) && 
		(rb<((frame_parms->N_RB_DL>>1)+3)) && 
		(l==((nsymb>>1)-1)) ) {
	      rb_alloc_ind = 0;
	    }
	    //PSS FDD
	    if (((subframe_offset==0)||(subframe_offset==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==((nsymb>>1)-1)))
	      skip_half=1;
	    else if (((subframe_offset==0)||(subframe_offset==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==(((nsymb>>1)-1))))
	      skip_half=2;

	    //SSS FDD
	    if (((subframe_offset==0)||(subframe_offset==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==((nsymb>>1)-2)) ) {
	      rb_alloc_ind = 0;
	    }
	    //SSS FDD
	    if (((subframe_offset==0)||(subframe_offset==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && ((l==((nsymb>>1)-2))))
	      skip_half=1;
	    else if (((subframe_offset==0)||(subframe_offset==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && ((l==(nsymb>>1)-2)))
	      skip_half=2;

	  }
	  
	}
	else {  // EVEN N_RB_DL
	  //PBCH
	  if ((subframe_offset==0) && 
	      (rb>=((frame_parms->N_RB_DL>>1)-3)) && 
	      (rb<((frame_parms->N_RB_DL>>1)+3)) && 
	      (l>=nsymb>>1) && (l<((nsymb>>1) + 4)))
	    rb_alloc_ind = 0;
	  skip_dc=0;
	  skip_half=0;
	  
	  if (frame_parms->frame_type == TDD) { // TDD
	    //SSS
	    if (((subframe_offset==0)||
		 (subframe_offset==5)) && 
		(rb>=((frame_parms->N_RB_DL>>1)-3)) && 
		(rb<((frame_parms->N_RB_DL>>1)+3)) && 
		(l==nsymb-1) ) {
	      rb_alloc_ind = 0;
	    }	    
	    //PSS
	    if (((subframe_offset==1)||
		 (subframe_offset==6)) && 
		(rb>=((frame_parms->N_RB_DL>>1)-3)) && 
		(rb<((frame_parms->N_RB_DL>>1)+3)) && 
		(l==2) ) {
	      rb_alloc_ind = 0;
	    }
	  }
	  else { // FDD
	    //SSS
	    if (((subframe_offset==0)||(subframe_offset==5)) && (rb>=((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==((nsymb>>1)-2)) ) {
	      rb_alloc_ind = 0;
	    }	    
	    //PSS
	    if (((subframe_offset==0)||(subframe_offset==5)) && (rb>=((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==((nsymb>>1)-1)) ) {
	      rb_alloc_ind = 0;
	    }
	  }
	}
	
	if (dlsch->layer_index>1) {
	  msg("layer_index %d: re_offset %d, symbol %d offset %d\n",dlsch->layer_index,re_offset,l,symbol_offset); 
	  return(-1);
	}
	if (mod_order == 4)
	  qam_table_s = ((pilots) ? qam16_table_b : qam16_table_a);
	else if (mod_order == 6)
	  qam_table_s = ((pilots) ? qam64_table_b : qam64_table_a);
	else
	  qam_table_s = NULL;

	if (rb_alloc_ind > 0)
	  //	  printf("Allocated rb %d, subframe_offset %d\n",rb,subframe_offset);
	  allocate_REs_in_RB(txdataF,
			     &jj,
			     re_offset,
			     symbol_offset,
			     dlsch->e,
			     dlsch->harq_processes[harq_pid]->mimo_mode,
			     dlsch->layer_index,
			     pilots,
			     mod_order,
			     get_pmi_5MHz(dlsch->harq_processes[harq_pid]->mimo_mode,dlsch->pmi_alloc,rb),
			     ((pilots) ? amp_rho_b : amp_rho_a),
			     qam_table_s,
			     &re_allocated,
			     skip_dc,
			     skip_half,
			     (frame_parms->mode1_flag==1)?1:0,
			     frame_parms);

	re_offset+=12; // go to next RB
	

	// check if we crossed the symbol boundary and skip DC
#ifdef IFFT_FPGA
	if (re_offset >= frame_parms->N_RB_DL*12) {
	  if (skip_dc == 0)  //even number of RBs (doesn't straddle DC)
	    re_offset=0;
	  else
	    re_offset=6;  // odd number of RBs
	}

#else
	if (re_offset >= frame_parms->ofdm_symbol_size) {
	  if (skip_dc == 0)  //even number of RBs (doesn't straddle DC)
	    re_offset=1;
	  else
	    re_offset=7;  // odd number of RBs
	}
#endif
      }
	
  }
  


#ifdef DEBUG_DLSCH_MODULATION
  msg("generate_dlsch : jj = %d,re_allocated = %d (G %d)\n",jj,re_allocated,get_G(frame_parms,dlsch->nb_rb,dlsch->rb_alloc,mod_order,num_pdcch_symbols,subframe_offset));
#endif
  
  return (re_allocated);
}
