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

/*! \file PHY/LTE_TRANSPORT/phich.c
* \brief Top-level routines for generating and decoding  the PHICH/HI physical/transport channel V8.6 2009-03
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#ifndef USER_MODE
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#endif

#define DEBUG_PHICH 1

//extern unsigned short pcfich_reg[4];
//extern unsigned char pcfich_first_reg_idx;

//unsigned short phich_reg[MAX_NUM_PHICH_GROUPS][3];

u8 get_mi(LTE_DL_FRAME_PARMS *frame_parms,u8 subframe) {

  switch (frame_parms->tdd_config) {

  case 0: 
    if ((subframe==0) || (subframe==5))
      return(2);
    else return(1);
    break;
  case 1: 
    if ((subframe==0) || (subframe==5))
      return(0);
    else return(1);
    break;
  case 2: 
    if ((subframe==3) || (subframe==8))
      return(1);
    else return(0);
    break;
  case 3: 
    if ((subframe==0) || (subframe==8) || (subframe==9))
      return(1);
    else return(0);
    break;
  default:
    return(0);
  }
}

unsigned char subframe2_ul_harq(LTE_DL_FRAME_PARMS *frame_parms,unsigned char subframe) {

  if (frame_parms->frame_type == 0)
    return(subframe&7);
  
  switch (frame_parms->tdd_config) {
  case 3:
    if ( (subframe == 8) || (subframe == 9) ){
      return(subframe-8);
    }
    else if (subframe==0)
      return(2);
    else {
      msg("phich.c: subframe2_ul_harq, illegal subframe %d for tdd_config %d\n",
	  subframe,frame_parms->tdd_config);
      return(0);
    }
    break;
    
  }
  return(0);
}

u8 phich_frame2_pusch_frame(LTE_DL_FRAME_PARMS *frame_parms,u8 frame,u8 subframe) {
  if (frame_parms->frame_type == 0) {
    return((subframe<4) ? (frame - 1) : frame);
  }
  else {
    // Note this is not true, but it doesn't matter, the frame number is irrelevant for TDD!
    return(frame);
  }
}

u8 phich_subframe2_pusch_subframe(LTE_DL_FRAME_PARMS *frame_parms,u8 subframe) {

  if (frame_parms->frame_type == 0)
    return(subframe<4 ? ((subframe+8)%10) : subframe-4);
 
  switch (frame_parms->tdd_config) {
  case 0:
    if (subframe == 0)
      return(3);
    else if (subframe == 5) {
      return (8);
    }
    else if (subframe == 6)
      return (2);
    else if (subframe == 1)
      return (7);
    else {
      msg("phich.c: phich_subframe2_pusch_subframe, illegal subframe %d for tdd_config %d\n",
	  subframe,frame_parms->tdd_config);
      return(0);
    }
    break;
  case 1:
    if (subframe == 6)
      return(2);
    else if (subframe == 9) 
      return (3);
    else if (subframe == 1)
      return (7);
    else if (subframe == 4)
      return (8);
    else {
      msg("phich.c: phich_subframe2_pusch_subframe, illegal subframe %d for tdd_config %d\n",
	  subframe,frame_parms->tdd_config);
      return(0);
    }
    break;
  case 2:
    if (subframe == 8)
      return(2);
    else if (subframe == 3)
      return (7);
    else {
      msg("phich.c: phich_subframe2_pusch_subframe, illegal subframe %d for tdd_config %d\n",
	  subframe,frame_parms->tdd_config);
      return(0);
    }
    break;
  case 3:
    if ( (subframe == 8) || (subframe == 9) ){
      return(subframe-6);
    }
    else if (subframe==0)
      return(4);
    else {
      msg("phich.c: phich_subframe2_pusch_subframe, illegal subframe %d for tdd_config %d\n",
	  subframe,frame_parms->tdd_config);
      return(0);
    }
    break;
  case 4:
    if ( (subframe == 8) || (subframe == 9) ){
      return(subframe-6);
    }
    else {
      msg("phich.c: phich_subframe2_pusch_subframe, illegal subframe %d for tdd_config %d\n",
	  subframe,frame_parms->tdd_config);
      return(0);
    }
    break;    
  case 5:
    if (subframe == 8){
      return(2);
    }
    else {
      msg("phich.c: phich_subframe2_pusch_subframe, illegal subframe %d for tdd_config %d\n",
	  subframe,frame_parms->tdd_config);
      return(0);
    }
    break;

  case 6:
    if (subframe == 6){
      return(2);
    }
    else if (subframe == 9){
      return(3);
    }
    else if (subframe == 0){
      return(4);
    }
    else if (subframe == 1){
      return(7);
    }
    else if (subframe == 5){
      return(8);
    }
    else {
      msg("phich.c: phich_subframe2_pusch_subframe, illegal subframe %d for tdd_config %d\n",
	  subframe,frame_parms->tdd_config);
      return(0);
    }
    break;
    
  }
  return(0);
}

int check_pcfich(LTE_DL_FRAME_PARMS *frame_parms,u16 reg) {

  if ((reg == frame_parms->pcfich_reg[0]) ||
      (reg == frame_parms->pcfich_reg[1]) ||
      (reg == frame_parms->pcfich_reg[2]) ||
      (reg == frame_parms->pcfich_reg[3]))
    return(1);
  return(0);
}

void generate_phich_reg_mapping(LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned short n0 = (frame_parms->N_RB_DL * 2) - 4;  // 2 REG per RB less the 4 used by PCFICH in first symbol
  unsigned short n1 = (frame_parms->N_RB_DL * 3);      // 3 REG per RB in second and third symbol
  unsigned short n2 = n1;
  unsigned short mprime = 0;
  unsigned short Ngroup_PHICH;
  //  u16 *phich_reg = frame_parms->phich_reg;
  u16 *pcfich_reg = frame_parms->pcfich_reg;

  // compute Ngroup_PHICH (see formula at beginning of Section 6.9 in 36-211
  Ngroup_PHICH = frame_parms->phich_config_common.phich_resource*(frame_parms->N_RB_DL/48);
  if (((frame_parms->phich_config_common.phich_resource*frame_parms->N_RB_DL)%48) > 0)
    Ngroup_PHICH++;
  // check if Extended prefix
  if (frame_parms->Ncp == 1) {
    Ngroup_PHICH<<=1;
  }
  
#ifdef DEBUG_PHICH
  msg("[PHY] Ngroup_PHICH %d (phich_config_common.phich_resource %d,NidCell %d,Ncp %d, frame_type %d)\n",((frame_parms->Ncp == 0)?Ngroup_PHICH:(Ngroup_PHICH>>1)),frame_parms->phich_config_common.phich_resource,
      frame_parms->Nid_cell,frame_parms->Ncp,frame_parms->frame_type);
#endif

  // This is the algorithm from Section 6.9.3 in 36-211
  for (mprime=0;mprime<((frame_parms->Ncp == 0)?Ngroup_PHICH:(Ngroup_PHICH>>1));mprime++) {

    if (frame_parms->Ncp==0){  // normal prefix

      frame_parms->phich_reg[mprime][0] = (frame_parms->Nid_cell + mprime)%n0;

      if (frame_parms->phich_reg[mprime][0]>=pcfich_reg[frame_parms->pcfich_first_reg_idx])
	frame_parms->phich_reg[mprime][0]++;
      if (frame_parms->phich_reg[mprime][0]>=pcfich_reg[(frame_parms->pcfich_first_reg_idx+1)&3])
	frame_parms->phich_reg[mprime][0]++;
      if (frame_parms->phich_reg[mprime][0]>=pcfich_reg[(frame_parms->pcfich_first_reg_idx+2)&3])
	frame_parms->phich_reg[mprime][0]++;
      if (frame_parms->phich_reg[mprime][0]>=pcfich_reg[(frame_parms->pcfich_first_reg_idx+3)&3])
	frame_parms->phich_reg[mprime][0]++;

      frame_parms->phich_reg[mprime][1] = (frame_parms->Nid_cell + mprime + (n0/3))%n0;

      if (frame_parms->phich_reg[mprime][1]>=pcfich_reg[frame_parms->pcfich_first_reg_idx])
	frame_parms->phich_reg[mprime][1]++;
      if (frame_parms->phich_reg[mprime][1]>=pcfich_reg[(frame_parms->pcfich_first_reg_idx+1)&3])
	frame_parms->phich_reg[mprime][1]++;
      if (frame_parms->phich_reg[mprime][1]>=pcfich_reg[(frame_parms->pcfich_first_reg_idx+2)&3])
	frame_parms->phich_reg[mprime][1]++;
      if (frame_parms->phich_reg[mprime][1]>=pcfich_reg[(frame_parms->pcfich_first_reg_idx+3)&3])
	frame_parms->phich_reg[mprime][1]++;
 
     frame_parms->phich_reg[mprime][2] = (frame_parms->Nid_cell + mprime + (2*n0/3))%n0;
      if (frame_parms->phich_reg[mprime][2]>=pcfich_reg[frame_parms->pcfich_first_reg_idx])
	frame_parms->phich_reg[mprime][2]++;
      if (frame_parms->phich_reg[mprime][2]>=pcfich_reg[(frame_parms->pcfich_first_reg_idx+1)&3])
	frame_parms->phich_reg[mprime][2]++;
      if (frame_parms->phich_reg[mprime][2]>=pcfich_reg[(frame_parms->pcfich_first_reg_idx+2)&3])
	frame_parms->phich_reg[mprime][2]++;
      if (frame_parms->phich_reg[mprime][2]>=pcfich_reg[(frame_parms->pcfich_first_reg_idx+3)&3])
	frame_parms->phich_reg[mprime][2]++;

#ifdef DEBUG_PHICH
      msg("[PHY] phich_reg :%d => %d,%d,%d\n",mprime,frame_parms->phich_reg[mprime][0],frame_parms->phich_reg[mprime][1],frame_parms->phich_reg[mprime][2]);
#endif
    }
    else {  // extended prefix
      frame_parms->phich_reg[mprime<<1][0] = (frame_parms->Nid_cell + mprime)%n0;
      frame_parms->phich_reg[1+(mprime<<1)][0] = (frame_parms->Nid_cell + mprime)%n0;

      frame_parms->phich_reg[mprime<<1][1] = ((frame_parms->Nid_cell*n1/n0) + mprime + (n1/3))%n1;
      frame_parms->phich_reg[mprime<<1][2] = ((frame_parms->Nid_cell*n2/n0) + mprime + (2*n2/3))%n2;

      frame_parms->phich_reg[1+(mprime<<1)][1] = ((frame_parms->Nid_cell*n1/n0) + mprime + (n1/3))%n1;
      frame_parms->phich_reg[1+(mprime<<1)][2] = ((frame_parms->Nid_cell*n2/n0) + mprime + (2*n2/3))%n2;
#ifdef DEBUG_PHICH
      msg("[PHY] phich_reg :%d => %d,%d,%d\n",mprime<<1,frame_parms->phich_reg[mprime<<1][0],frame_parms->phich_reg[mprime][1],frame_parms->phich_reg[mprime][2]);
      msg("[PHY] phich_reg :%d => %d,%d,%d\n",1+(mprime<<1),frame_parms->phich_reg[1+(mprime<<1)][0],frame_parms->phich_reg[1+(mprime<<1)][1],frame_parms->phich_reg[1+(mprime<<1)][2]);
#endif
    }
  } // mprime loop
}  // num_pdcch_symbols loop


mod_sym_t alam_bpsk_perm1[4] = {2,1,4,3}; // -conj(x) 1 (-1-j) -> 2 (1-j), 2->1, 3 (-1+j) -> (4) 1+j, 4->3
mod_sym_t alam_bpsk_perm2[4] = {3,4,2,1}; // conj(x) 1 (-1-j) -> 3 (-1+j), 3->1, 2 (1-j) -> 4 (1+j), 4->2 

// This routine generates the PHICH 

void generate_phich(LTE_DL_FRAME_PARMS *frame_parms,
		    s16 amp,
		    u8 nseq_PHICH,
		    u8 ngroup_PHICH,
		    u8 HI,
		    u8 subframe,
		    mod_sym_t **y) {
  
  s16 d[24],*dp;
  //  unsigned int i,aa;
  unsigned int re_offset;
  s16 y0_16[8],y1_16[8];
  s16 *y0,*y1;
  // scrambling
  u32 x1, x2, s=0;
  u8 reset = 1;
  s16 cs[12];
  u32 i,i2,i3,m,j;
  s16 gain_lin_QPSK;
  u32 subframe_offset=((frame_parms->Ncp==0)?14:12)*frame_parms->ofdm_symbol_size*subframe;

  memset(d,0,24*sizeof(s16));

  gain_lin_QPSK = (s16)(((s32)amp*ONE_OVER_SQRT2_Q15)>>15);  

  //printf("PHICH : gain_lin_QPSK %d\n",gain_lin_QPSK);

  // BPSK modulation of HI input (to be repeated 3 times, 36-212 Section 5.3.5, p. 56 in v8.6)
  if (HI>0)
    HI=1;
  //  c = (1-(2*HI))*SSS_AMP;
  // x1 is set in lte_gold_generic
  x2 = (((subframe+1)*(frame_parms->Nid_cell+1))<<9) + frame_parms->Nid_cell; 
  
  s = lte_gold_generic(&x1, &x2, reset);
  
  // compute scrambling sequence
  for (i=0;i<12;i++) {
    cs[i] = (u8)((s>>(i&0x1f))&1);
    cs[i] = (cs[i] == 0) ? (1-(HI<<1)) : ((HI<<1)-1);
  }

  if (frame_parms->Ncp == 0) { // Normal Cyclic Prefix

    //    printf("Doing PHICH : Normal CP, subframe %d\n",subframe);
    // 12 output symbols (Msymb)
    for (i=0,i2=0,i3=0;i<3;i++,i2+=4,i3+=8) {
      switch (nseq_PHICH) {
      case 0: // +1 +1 +1 +1
	d[i3]   = cs[i2];
	d[1+i3] = cs[i2];
	d[2+i3] = cs[1+i2];
	d[3+i3] = cs[1+i2];
	d[4+i3] = cs[2+i2];
	d[5+i3] = cs[2+i2];
	d[6+i3] = cs[3+i2];
	d[7+i3] = cs[3+i2];
	break;
      case 1: // +1 -1 +1 -1
	d[i3] = cs[i2];
	d[1+i3] = cs[i2];
	d[2+i3] = -cs[1+i2];
	d[3+i3] = -cs[1+i2];
	d[4+i3] = cs[2+i2];
	d[5+i3] = cs[2+i2];
	d[6+i3] = -cs[3+i2];
	d[7+i3] = -cs[3+i2];
	break;
      case 2: // +1 +1 -1 -1
	d[i3]   = cs[i2];
	d[1+i3]   = cs[i2];
	d[2+i3] = cs[1+i2];
	d[3+i3] = cs[1+i2];
	d[4+i3] = -cs[2+i2];
	d[5+i3] = -cs[2+i2];
	d[6+i3] = -cs[3+i2];
	d[7+i3] = -cs[3+i2];
	break;
      case 3: // +1 -1 -1 +1
	d[i3]   = cs[i2];
	d[1+i3]   = cs[i2];
	d[2+i3] = -cs[1+i2];
	d[3+i3] = -cs[1+i2];
	d[4+i3] = -cs[2+i2];
	d[5+i3] = -cs[2+i2];
	d[6+i3] = cs[3+i2];
	d[7+i3] = cs[3+i2];
	break;
      case 4: // +j +j +j +j
	d[i3]   = -cs[i2];
	d[1+i3] = cs[i2];
	d[2+i3] = -cs[1+i2];
	d[3+i3] = cs[1+i2];
	d[4+i3] = -cs[2+i2];
	d[5+i3] = cs[2+i2];
	d[6+i3] = -cs[3+i2];
	d[7+i3] = cs[3+i2];
	break;
      case 5: // +j -j +j -j
	d[1+i3] = cs[i2];
	d[3+i3] = -cs[1+i2];
	d[5+i3] = cs[2+i2];
	d[7+i3] = -cs[3+i2];
	d[i3]   = -cs[i2];
	d[2+i3] = cs[1+i2];
	d[4+i3] = -cs[2+i2];
	d[6+i3] = cs[3+i2];
	break;
      case 6: // +j +j -j -j
	d[1+i3] = cs[i2];
	d[3+i3] = cs[1+i2];
	d[5+i3] = -cs[2+i2];
	d[7+i3] = -cs[3+i2];
	d[i3]   = -cs[i2];
	d[2+i3] = -cs[1+i2];
	d[4+i3] = cs[2+i2];
	d[6+i3] = cs[3+i2];
	break;
      case 7: // +j -j -j +j
	d[1+i3] = cs[i2];
	d[3+i3] = -cs[1+i2];
	d[5+i3] = -cs[2+i2];
	d[7+i3] = cs[3+i2];
	d[i3]   = -cs[i2];
	d[2+i3] = cs[1+i2];
	d[4+i3] = cs[2+i2];
	d[6+i3] = -cs[3+i2];
	break;
      default:
	msg("phich_coding.c: Illegal PHICH Number\n");
      } // nseq_PHICH
    }
#ifdef DEBUG_PHICH
    msg("[PUSCH 0]PHICH d = ");
    for (i=0;i<24;i+=2)
      msg("(%d,%d)",d[i],d[i+1]);
    msg("\n");
#endif
      // modulation here
    if (frame_parms->mode1_flag == 0) {
      // do Alamouti precoding here
      
      // Symbol 0
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][0]*6);

      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      y1 = (s16*)&y[1][re_offset+subframe_offset];
      
      // first antenna position n -> x0
      y0_16[0]   = d[0]*gain_lin_QPSK;
      y0_16[1]   = d[1]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[0]   = -d[2]*gain_lin_QPSK;
      y1_16[1]   = d[3]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[2] = -y1_16[0];
      y0_16[3] = y1_16[1];
      y1_16[2] = y0_16[0];
      y1_16[3] = -y0_16[1];

      // first antenna position n -> x0
      y0_16[4]   = d[4]*gain_lin_QPSK;
      y0_16[5]   = d[5]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[4]   = -d[6]*gain_lin_QPSK;
      y1_16[5]   = d[7]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[6] = -y1_16[4];
      y0_16[7] = y1_16[5];
      y1_16[6] = y0_16[4];
      y1_16[7] = -y0_16[5];

      for (i=0,j=0,m=0;i<6;i++,j+=2){
	if ((i!=(frame_parms->nushift))&&(i!=(frame_parms->nushift+3))) {
	  y0[j]   += y0_16[m];
	  y1[j]   += y1_16[m++];
	  y0[j+1] += y0_16[m];
	  y1[j+1] += y1_16[m++];
	}
      }
      // Symbol 1
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][1]*6);

      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      y1 = (s16*)&y[1][re_offset+subframe_offset];
      
      // first antenna position n -> x0
      y0_16[0]   = d[8]*gain_lin_QPSK;
      y0_16[1]   = d[9]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[0]   = -d[10]*gain_lin_QPSK;
      y1_16[1]   = d[11]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[2] = -y1_16[0];
      y0_16[3] = y1_16[1];
      y1_16[2] = y0_16[0];
      y1_16[3] = -y0_16[1];

      // first antenna position n -> x0
      y0_16[4]   = d[12]*gain_lin_QPSK;
      y0_16[5]   = d[13]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[4]   = -d[14]*gain_lin_QPSK;
      y1_16[5]   = d[15]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[6] = -y1_16[4];
      y0_16[7] = y1_16[5];
      y1_16[6] = y0_16[4];
      y1_16[7] = -y0_16[5];

      for (i=0,j=0,m=0;i<6;i++,j+=2){
	if ((i!=(frame_parms->nushift))&&(i!=(frame_parms->nushift+3))) {
	  y0[j]   += y0_16[m];
	  y1[j]   += y1_16[m++];
	  y0[j+1] += y0_16[m];
	  y1[j+1] += y1_16[m++];
	}
      }

      // Symbol 2
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][2]*6);

      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      y1 = (s16*)&y[1][re_offset+subframe_offset];
      
      // first antenna position n -> x0
      y0_16[0]   = d[16]*gain_lin_QPSK;
      y0_16[1]   = d[17]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[0]   = -d[18]*gain_lin_QPSK;
      y1_16[1]   = d[19]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[2] = -y1_16[0];
      y0_16[3] = y1_16[1];
      y1_16[2] = y0_16[0];
      y1_16[3] = -y0_16[1];

      // first antenna position n -> x0
      y0_16[4]   = d[20]*gain_lin_QPSK;
      y0_16[5]   = d[21]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[4]   = -d[22]*gain_lin_QPSK;
      y1_16[5]   = d[23]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[6] = -y1_16[4];
      y0_16[7] = y1_16[5];
      y1_16[6] = y0_16[4];
      y1_16[7] = -y0_16[5];

      for (i=0,j=0,m=0;i<6;i++,j+=2){
	if ((i!=(frame_parms->nushift))&&(i!=(frame_parms->nushift+3))) {
	  y0[j]   += y0_16[m];
	  y1[j]   += y1_16[m++];
	  y0[j+1] += y0_16[m];
	  y1[j+1] += y1_16[m++];
	}
      }

    } // mode1_flag

    else {
      // Symbol 0
      //      printf("[PUSCH 0]PHICH REG %d\n",frame_parms->phich_reg[ngroup_PHICH][0]);
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][0]*6);

      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      //      printf("y0 %p\n",y0);

      y0_16[0]   = d[0]*gain_lin_QPSK;
      y0_16[1]   = d[1]*gain_lin_QPSK;
      y0_16[2]   = d[2]*gain_lin_QPSK;
      y0_16[3]   = d[3]*gain_lin_QPSK;
      y0_16[4]   = d[4]*gain_lin_QPSK;
      y0_16[5]   = d[5]*gain_lin_QPSK;
      y0_16[6]   = d[6]*gain_lin_QPSK;
      y0_16[7]   = d[7]*gain_lin_QPSK;

      for (i=0,j=0,m=0;i<6;i++,j+=2){
	if ((i!=(frame_parms->nushift))&&(i!=(frame_parms->nushift+3))) {
	  y0[j]   += y0_16[m++];
	  y0[j+1] += y0_16[m++];
	}
      }
      // Symbol 1
      //      printf("[PUSCH 0]PHICH REG %d\n",frame_parms->phich_reg[ngroup_PHICH][1]);
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][1]*6);

      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      
      y0_16[0]   = d[8]*gain_lin_QPSK;
      y0_16[1]   = d[9]*gain_lin_QPSK;
      y0_16[2]   = d[10]*gain_lin_QPSK;
      y0_16[3]   = d[11]*gain_lin_QPSK;
      y0_16[4]   = d[12]*gain_lin_QPSK;
      y0_16[5]   = d[13]*gain_lin_QPSK;
      y0_16[6]   = d[14]*gain_lin_QPSK;
      y0_16[7]   = d[15]*gain_lin_QPSK;

      for (i=0,j=0,m=0;i<6;i++,j+=2){
	if ((i!=(frame_parms->nushift))&&(i!=(frame_parms->nushift+3))) {
	  y0[j]   += y0_16[m++];
	  y0[j+1] += y0_16[m++];
	}
      }

      // Symbol 2
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][2]*6);

      //      printf("[PUSCH 0]PHICH REG %d\n",frame_parms->phich_reg[ngroup_PHICH][2]);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);
 
      y0 = (s16*)&y[0][re_offset+subframe_offset];
      
      y0_16[0]   = d[16]*gain_lin_QPSK;
      y0_16[1]   = d[17]*gain_lin_QPSK;
      y0_16[2]   = d[18]*gain_lin_QPSK;
      y0_16[3]   = d[19]*gain_lin_QPSK;
      y0_16[4]   = d[20]*gain_lin_QPSK;
      y0_16[5]   = d[21]*gain_lin_QPSK;
      y0_16[6]   = d[22]*gain_lin_QPSK;
      y0_16[7]   = d[23]*gain_lin_QPSK;

      for (i=0,j=0,m=0;i<6;i++,j+=2){
	if ((i!=(frame_parms->nushift))&&(i!=(frame_parms->nushift+3))) {
	  y0[j]   += y0_16[m++];
	  y0[j+1] += y0_16[m++];
	}
      }
      /*
      for (i=0;i<512;i++)
	printf("re %d (%d): %d,%d\n",i,subframe_offset+i,((s16*)&y[0][subframe_offset+i])[0],((s16*)&y[0][subframe_offset+i])[1]);
      */
    } // mode1_flag
  }
  else {  // extended prefix
    
    // 6 output symbols
    if ((ngroup_PHICH & 1) == 1)
      dp = &d[4];
    else  
      dp = d;

    switch (nseq_PHICH) {
    case 0: // +1 +1 
      dp[0]  = cs[0];
      dp[2]  = cs[1];
      dp[8]  = cs[2];
      dp[10] = cs[3];
      dp[16] = cs[4];
      dp[18] = cs[5];
      dp[1]  = cs[0];
      dp[3]  = cs[1];
      dp[9]  = cs[2];
      dp[11] = cs[3];
      dp[17] = cs[4];
      dp[19] = cs[5];
      break;
    case 1: // +1 -1
      dp[0]  = cs[0];
      dp[2]  = -cs[1];
      dp[8]  = cs[2];
      dp[10] = -cs[3];
      dp[16] = cs[4];
      dp[18] = -cs[5];
      dp[1]  = cs[0];
      dp[3]  = -cs[1];
      dp[9]  = cs[2];
      dp[11] = -cs[3];
      dp[17] = cs[4];
      dp[19] = -cs[5];
      break;
    case 2: // +j +j 
      dp[1]  = cs[0];
      dp[3]  = cs[1];
      dp[9]  = cs[2];
      dp[11] = cs[3];
      dp[17] = cs[4];
      dp[19] = cs[5];
      dp[0]  = -cs[0];
      dp[2]  = -cs[1];
      dp[8]  = -cs[2];
      dp[10] = -cs[3];
      dp[16] = -cs[4];
      dp[18] = -cs[5];
      
      break;
    case 3: // +j -j 
      dp[1]  = cs[0];
      dp[3]  = -cs[1];
      dp[9]  = cs[2];
      dp[11] = -cs[3];
      dp[17] = cs[4];
      dp[19] = -cs[5];
      dp[0]  = -cs[0];
      dp[2]  = cs[1];
      dp[8]  = -cs[2];
      dp[10] = cs[3];
      dp[16] = -cs[4];
      dp[18] = cs[5];
      break;
    default:
      msg("[PHY] phich_coding.c: Illegal PHICH Number\n");
    }
    
    
    
    if (frame_parms->mode1_flag == 0) {
      // do Alamouti precoding here
      // Symbol 0
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][0]*6);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      y1 = (s16*)&y[1][re_offset+subframe_offset];
      
      // first antenna position n -> x0
      y0_16[0]   = d[0]*gain_lin_QPSK;
      y0_16[1]   = d[1]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[0]   = -d[2]*gain_lin_QPSK;
      y1_16[1]   = d[3]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[2] = -y1_16[0];
      y0_16[3] = y1_16[1];
      y1_16[2] = y0_16[0];
      y1_16[3] = -y0_16[1];

      // first antenna position n -> x0
      y0_16[4]   = d[4]*gain_lin_QPSK;
      y0_16[5]   = d[5]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[4]   = -d[6]*gain_lin_QPSK;
      y1_16[5]   = d[7]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[6] = -y1_16[4];
      y0_16[7] = y1_16[5];
      y1_16[6] = y0_16[4];
      y1_16[7] = -y0_16[5];

      for (i=0,j=0,m=0;i<6;i++,j+=2){
	if ((i!=(frame_parms->nushift))&&(i!=(frame_parms->nushift+3))) {
	  y0[j] += y0_16[m];
	  y1[j] += y1_16[m++];
	  y0[j+1] += y0_16[m];
	  y1[j+1] += y1_16[m++];
	}
      }

      // Symbol 1
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][1]<<2);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);
      re_offset += (frame_parms->ofdm_symbol_size);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      y1 = (s16*)&y[1][re_offset+subframe_offset];

      // first antenna position n -> x0
      y0_16[0]   = d[8]*gain_lin_QPSK;
      y0_16[1]   = d[9]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[0]   = -d[10]*gain_lin_QPSK;
      y1_16[1]   = d[11]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[2] = -y1_16[0];
      y0_16[3] = y1_16[1];
      y1_16[2] = y0_16[0];
      y1_16[3] = -y0_16[1];

      // first antenna position n -> x0
      y0_16[4]   = d[12]*gain_lin_QPSK;
      y0_16[5]   = d[13]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[4]   = -d[14]*gain_lin_QPSK;
      y1_16[5]   = d[15]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[6] = -y1_16[4];
      y0_16[7] = y1_16[5];
      y1_16[6] = y0_16[4];
      y1_16[7] = -y0_16[5];

      for (i=0,j=0,m=0;i<4;i++,j+=2){
	y0[j] += y0_16[m];
	y1[j] += y1_16[m++];
	y0[j+1] += y0_16[m];
	y1[j+1] += y1_16[m++];
      }

      // Symbol 2
      re_offset = frame_parms->first_carrier_offset +  (frame_parms->phich_reg[ngroup_PHICH][2]<<2);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);
      re_offset += (frame_parms->ofdm_symbol_size<<1);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      y1 = (s16*)&y[1][re_offset+subframe_offset];

      // first antenna position n -> x0
      y0_16[0]   = d[16]*gain_lin_QPSK;
      y0_16[1]   = d[17]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[0]   = -d[18]*gain_lin_QPSK;
      y1_16[1]   = d[19]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[2] = -y1_16[0];
      y0_16[3] = y1_16[1];
      y1_16[2] = y0_16[0];
      y1_16[3] = -y0_16[1];

      // first antenna position n -> x0
      y0_16[4]   = d[20]*gain_lin_QPSK;
      y0_16[5]   = d[21]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[4]   = -d[22]*gain_lin_QPSK;
      y1_16[5]   = d[23]*gain_lin_QPSK;
      // fill in the rest of the ALAMOUTI precoding
      y0_16[6] = -y1_16[4];
      y0_16[7] = y1_16[5];
      y1_16[6] = y0_16[4];
      y1_16[7] = -y0_16[5];

      for (i=0,j=0,m=0;i<4;i++,j+=2){
	y0[j]   += y0_16[m];
	y1[j]   += y1_16[m++];
	y0[j+1] += y0_16[m];
	y1[j+1] += y1_16[m++];
      }
    }
    else {

      // Symbol 0
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][0]*6);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      
      y0_16[0]   = d[0]*gain_lin_QPSK;
      y0_16[1]   = d[1]*gain_lin_QPSK;
      y0_16[2]   = d[2]*gain_lin_QPSK;
      y0_16[3]   = d[3]*gain_lin_QPSK;
      y0_16[4]   = d[4]*gain_lin_QPSK;
      y0_16[5]   = d[5]*gain_lin_QPSK;
      y0_16[6]   = d[6]*gain_lin_QPSK;
      y0_16[7]   = d[7]*gain_lin_QPSK;

      for (i=0,j=0,m=0;i<6;i++,j+=2){
	if ((i!=(frame_parms->nushift))&&(i!=(frame_parms->nushift+3))) {
	  y0[j] += y0_16[m++];
	  y0[j+1] += y0_16[m++];
	}
      }

      // Symbol 1
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][1]<<2);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);
      re_offset += (frame_parms->ofdm_symbol_size);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      
      y0_16[0]   = d[8]*gain_lin_QPSK;
      y0_16[1]   = d[9]*gain_lin_QPSK;
      y0_16[2]   = d[10]*gain_lin_QPSK;
      y0_16[3]   = d[11]*gain_lin_QPSK;
      y0_16[4]   = d[12]*gain_lin_QPSK;
      y0_16[5]   = d[13]*gain_lin_QPSK;
      y0_16[6]   = d[14]*gain_lin_QPSK;
      y0_16[7]   = d[15]*gain_lin_QPSK;

      for (i=0,j=0,m=0;i<4;i++,j+=2){
	y0[j] += y0_16[m++];
	y0[j+1] += y0_16[m++];
      }


      // Symbol 2
      re_offset = frame_parms->first_carrier_offset + (frame_parms->phich_reg[ngroup_PHICH][2]<<2);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);
      re_offset += (frame_parms->ofdm_symbol_size<<1);

      y0 = (s16*)&y[0][re_offset+subframe_offset];
      
      y0_16[0]   = d[16]*gain_lin_QPSK;
      y0_16[1]   = d[17]*gain_lin_QPSK;
      y0_16[2]   = d[18]*gain_lin_QPSK;
      y0_16[3]   = d[19]*gain_lin_QPSK;
      y0_16[4]   = d[20]*gain_lin_QPSK;
      y0_16[5]   = d[21]*gain_lin_QPSK;
      y0_16[6]   = d[22]*gain_lin_QPSK;
      y0_16[7]   = d[23]*gain_lin_QPSK;

      for (i=0,j=0,m=0;i<4;i++){
	y0[j]   += y0_16[m++];
	y0[j+1] += y0_16[m++];
      }

    } // mode1_flag
  } // normal/extended prefix
} 

// This routine demodulates the PHICH and updates PUSCH/ULSCH parameters


void rx_phich(PHY_VARS_UE *phy_vars_ue,
	      u8 subframe,
	      u8 eNB_id) {


  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  LTE_UE_PDCCH **lte_ue_pdcch_vars = phy_vars_ue->lte_ue_pdcch_vars;

  //  u8 HI;
  u8 harq_pid = phich_subframe_to_harq_pid(frame_parms,phy_vars_ue->frame,subframe);
  LTE_UE_ULSCH_t *ulsch = phy_vars_ue->ulsch_ue[eNB_id];
  s16 phich_d[24],*phich_d_ptr,HI16;
  //  unsigned int i,aa;
  s8 d[24],*dp;
  u16 reg_offset;

  // scrambling
  u32 x1, x2, s=0;
  u8 reset = 1;
  s16 cs[12];
  u32 i,i2,i3,phich_quad;
  s32 **rxdataF_comp = lte_ue_pdcch_vars[eNB_id]->rxdataF_comp;
  u8 Ngroup_PHICH,ngroup_PHICH,nseq_PHICH;
  u8 NSF_PHICH = 4;
  u8 pusch_subframe;
  
  // check if we're expecting a PHICH in this subframe
  //  msg("[PHY][UE  %d][PUSCH %d] Frame %d subframe %d PHICH RX\n",phy_vars_ue->Mod_id,harq_pid,phy_vars_ue->frame,subframe);
  if (ulsch->harq_processes[harq_pid]->status == ACTIVE) {
    msg("[PHY][UE  %d][PUSCH %d] Frame %d subframe %d PHICH RX\n",phy_vars_ue->Mod_id,harq_pid,phy_vars_ue->frame,subframe);
    Ngroup_PHICH = frame_parms->phich_config_common.phich_resource*(frame_parms->N_RB_DL/48);
    if (((frame_parms->phich_config_common.phich_resource*frame_parms->N_RB_DL)%48) > 0)
      Ngroup_PHICH++;
    
    if (frame_parms->Ncp == 1)
      NSF_PHICH = 2;
    

    ngroup_PHICH = (ulsch->harq_processes[harq_pid]->first_rb + 
		    ulsch->harq_processes[harq_pid]->n_DMRS)%Ngroup_PHICH;
    if ((frame_parms->tdd_config == 0) && (frame_parms->frame_type == 1) ) {
      pusch_subframe = phich_subframe2_pusch_subframe(frame_parms,subframe);
      if ((pusch_subframe == 4) || (pusch_subframe == 9))
	ngroup_PHICH += Ngroup_PHICH; 
    }
    nseq_PHICH = ((ulsch->harq_processes[harq_pid]->first_rb/Ngroup_PHICH) + 
		  ulsch->harq_processes[harq_pid]->n_DMRS)%(2*NSF_PHICH);
  }
  else {
    return;
  }

  memset(d,0,24*sizeof(s8));
  phich_d_ptr = phich_d;

  // x1 is set in lte_gold_generic
  x2 = (((subframe+1)*(frame_parms->Nid_cell+1))<<9) + frame_parms->Nid_cell; 
  
  s = lte_gold_generic(&x1, &x2, reset);
  
  // compute scrambling sequence
  for (i=0;i<12;i++) {
    cs[i] = 1-(((s>>(i&0x1f))&1)<<1);
  }

  if (frame_parms->Ncp == 0) { // Normal Cyclic Prefix


    // 12 output symbols (Msymb)

    for (i=0,i2=0,i3=0;i<3;i++,i2+=4,i3+=8) {
      switch (nseq_PHICH) {
      case 0: // +1 +1 +1 +1
	d[i3]   = cs[i2];
	d[1+i3] = cs[i2];
	d[2+i3] = cs[1+i2];
	d[3+i3] = cs[1+i2];
	d[4+i3] = cs[2+i2];
	d[5+i3] = cs[2+i2];
	d[6+i3] = cs[3+i2];
	d[7+i3] = cs[3+i2];
	break;
      case 1: // +1 -1 +1 -1
	d[i3] = cs[i2];
	d[1+i3] = cs[i2];
	d[2+i3] = -cs[1+i2];
	d[3+i3] = -cs[1+i2];
	d[4+i3] = cs[2+i2];
	d[5+i3] = cs[2+i2];
	d[6+i3] = -cs[3+i2];
	d[7+i3] = -cs[3+i2];
	break;
      case 2: // +1 +1 -1 -1
	d[i3]   = cs[i2];
	d[1+i3]   = cs[i2];
	d[2+i3] = cs[1+i2];
	d[3+i3] = cs[1+i2];
	d[4+i3] = -cs[2+i2];
	d[5+i3] = -cs[2+i2];
	d[6+i3] = -cs[3+i2];
	d[7+i3] = -cs[3+i2];
	break;
      case 3: // +1 -1 -1 +1
	d[i3]   = cs[i2];
	d[1+i3]   = cs[i2];
	d[2+i3] = -cs[1+i2];
	d[3+i3] = -cs[1+i2];
	d[4+i3] = -cs[2+i2];
	d[5+i3] = -cs[2+i2];
	d[6+i3] = cs[3+i2];
	d[7+i3] = cs[3+i2];
	break;
      case 4: // +j +j +j +j
	d[i3]   = -cs[i2];
	d[1+i3] = cs[i2];
	d[2+i3] = -cs[1+i2];
	d[3+i3] = cs[1+i2];
	d[4+i3] = -cs[2+i2];
	d[5+i3] = cs[2+i2];
	d[6+i3] = -cs[3+i2];
	d[7+i3] = cs[3+i2];
	break;
      case 5: // +j -j +j -j
	d[1+i3] = cs[i2];
	d[3+i3] = -cs[1+i2];
	d[5+i3] = cs[2+i2];
	d[7+i3] = -cs[3+i2];
	d[i3]   = -cs[i2];
	d[2+i3] = cs[1+i2];
	d[4+i3] = -cs[2+i2];
	d[6+i3] = cs[3+i2];
	break;
      case 6: // +j +j -j -j
	d[1+i3] = cs[i2];
	d[3+i3] = cs[1+i2];
	d[5+i3] = -cs[2+i2];
	d[7+i3] = -cs[3+i2];
	d[i3]   = -cs[i2];
	d[2+i3] = -cs[1+i2];
	d[4+i3] = cs[2+i2];
	d[6+i3] = cs[3+i2];
	break;
      case 7: // +j -j -j +j
	d[1+i3] = cs[i2];
	d[3+i3] = -cs[1+i2];
	d[5+i3] = -cs[2+i2];
	d[7+i3] = cs[3+i2];
	d[i3]   = -cs[i2];
	d[2+i3] = cs[1+i2];
	d[4+i3] = cs[2+i2];
	d[6+i3] = -cs[3+i2];
	break;
      default:
	msg("phich_coding.c: Illegal PHICH Number\n");
      } // nseq_PHICH
    }
#ifdef DEBUG_PHICH
    msg("PHICH =>");
    for (i=0;i<24;i++) {
      msg("%2d,",d[i]);
    }
    msg("\n");
#endif
    // demodulation here


  }    
  else {  // extended prefix
    
    // 6 output symbols
    if ((ngroup_PHICH & 1) == 1)
      dp = &d[4];
    else  
      dp = d;
    
    switch (nseq_PHICH) {
    case 0: // +1 +1 
      dp[0]  = cs[0];
      dp[2]  = cs[1];
      dp[8]  = cs[2];
      dp[10] = cs[3];
      dp[16] = cs[4];
      dp[18] = cs[5];
      dp[1]  = cs[0];
      dp[3]  = cs[1];
      dp[9]  = cs[2];
      dp[11] = cs[3];
      dp[17] = cs[4];
      dp[19] = cs[5];
      break;
    case 1: // +1 -1
      dp[0]  = cs[0];
      dp[2]  = -cs[1];
      dp[8]  = cs[2];
      dp[10] = -cs[3];
      dp[16] = cs[4];
      dp[18] = -cs[5];
      dp[1]  = cs[0];
      dp[3]  = -cs[1];
      dp[9]  = cs[2];
      dp[11] = -cs[3];
      dp[17] = cs[4];
      dp[19] = -cs[5];
      break;
    case 2: // +j +j 
      dp[1]  = cs[0];
      dp[3]  = cs[1];
      dp[9]  = cs[2];
      dp[11] = cs[3];
      dp[17] = cs[4];
      dp[19] = cs[5];
      dp[0]  = -cs[0];
      dp[2]  = -cs[1];
      dp[8]  = -cs[2];
      dp[10] = -cs[3];
      dp[16] = -cs[4];
      dp[18] = -cs[5];
  
     break;
    case 3: // +j -j 
      dp[1]  = cs[0];
      dp[3]  = -cs[1];
      dp[9]  = cs[2];
      dp[11] = -cs[3];
      dp[17] = cs[4];
      dp[19] = -cs[5];
      dp[0]  = -cs[0];
      dp[2]  = cs[1];
      dp[8]  = -cs[2];
      dp[10] = cs[3];
      dp[16] = -cs[4];
      dp[18] = cs[5];
      break;
    default:
      msg("[PHY] phich_coding.c: Illegal PHICH Number\n");
    }
  }

  HI16 = 0;

  //#ifdef DEBUG_PHICH

  //#endif
  /*  
  for (i=0;i<200;i++)
    printf("re %d: %d %d\n",i,((s16*)&rxdataF_comp[0][i])[0],((s16*)&rxdataF_comp[0][i])[1]);
  */
  for (phich_quad=0;phich_quad<3;phich_quad++) {
    if (frame_parms->Ncp == 1) 
      reg_offset = (frame_parms->phich_reg[ngroup_PHICH][phich_quad]*4)+ (phich_quad*frame_parms->N_RB_DL*12);
    else
      reg_offset = (frame_parms->phich_reg[ngroup_PHICH][phich_quad]*4);

    //    msg("\n[PUSCH 0]PHICH (RX) quad %d (%d)=>",phich_quad,reg_offset);
    dp = &d[phich_quad*8];;

    for (i=0;i<8;i++) {      
      phich_d_ptr[i] = ((s16*)&rxdataF_comp[0][reg_offset])[i]; 
      
#ifdef DEBUG_PHICH
      msg("%d,",((s16*)&rxdataF_comp[0][reg_offset])[i]);
#endif	  
      
      HI16 += (phich_d_ptr[i] * dp[i]);	  
    } 
  }
#ifdef DEBUG_PHICH
  msg("\n");
  msg("HI16 %d\n",HI16);
#endif
  if (HI16>0) {   //NACK
    if (phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] == 1) {
      msg("[PHY][UE  %d][PUSCH %d][RAPROC] Frame %d subframe %d Msg3 PHICH, received NAK (%d) nseq %d, ngroup %d\n\n",
	  phy_vars_ue->Mod_id,harq_pid,
	  phy_vars_ue->frame,
	  subframe,
	  HI16,
	  nseq_PHICH,
	  ngroup_PHICH);
      get_Msg3_alloc_ret(&phy_vars_ue->lte_frame_parms,
			 subframe,
			 phy_vars_ue->frame,
			 &phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
			 &phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);
    }
    else {
      //#ifdef DEBUG_PHICH
      msg("[PHY][UE  %d][PUSCH %d] Frame %d subframe %d PHICH, received NAK (%d) nseq %d, ngroup %d\n\n",
	  phy_vars_ue->Mod_id,harq_pid,
	  phy_vars_ue->frame,
	  subframe,
	  HI16,
	  nseq_PHICH,
	  ngroup_PHICH);
      //#endif
    }
    ulsch->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
    ulsch->harq_processes[harq_pid]->Ndi = 0;
    //    ulsch->harq_processes[harq_pid]->round++;
  }
  else {    //ACK
    if (phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] == 1) 
      msg("[PHY][UE  %d][PUSCH %d][RAPROC] Frame %d subframe %d Msg3 PHICH, received ACK (%d) nseq %d, ngroup %d\n\n",
	  phy_vars_ue->Mod_id,harq_pid,
	  phy_vars_ue->frame,
	  subframe,
	  HI16,
	  nseq_PHICH,ngroup_PHICH);
    else {
      //#ifdef PHICH_DEBUG
      msg("[PHY][UE  %d][PUSCH %d] Frame %d subframe %d PHICH, received ACK (%d) nseq %d, ngroup %d\n\n",
	  phy_vars_ue->Mod_id,harq_pid,
	  phy_vars_ue->frame,
	  subframe, HI16,
	  nseq_PHICH,ngroup_PHICH);
      //#endif
    }
    ulsch->harq_processes[harq_pid]->subframe_scheduling_flag =0;
    ulsch->harq_processes[harq_pid]->status = IDLE;
    // inform MAC?
    phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] = 0;
  }
  
}

void generate_phich_top(PHY_VARS_eNB *phy_vars_eNB,
			unsigned char subframe,
			s16 amp,
			u8 sect_id,
			u8 abstraction_flag) {


  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_eNB->lte_frame_parms;
  LTE_eNB_ULSCH_t **ulsch_eNB = phy_vars_eNB->ulsch_eNB;
  mod_sym_t **txdataF = phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id];  
  u8 harq_pid;
  u8 Ngroup_PHICH,ngroup_PHICH,nseq_PHICH;
  u8 NSF_PHICH = 4;
  u8 pusch_subframe;
  u8 UE_id;
  u32 pusch_frame;

  // compute Ngroup_PHICH (see formula at beginning of Section 6.9 in 36-211
  
  Ngroup_PHICH = frame_parms->phich_config_common.phich_resource*(frame_parms->N_RB_DL/48);
  if (((frame_parms->phich_config_common.phich_resource*frame_parms->N_RB_DL)%48) > 0)
    Ngroup_PHICH++;

  if (frame_parms->Ncp == 1)
    NSF_PHICH = 2;
  pusch_frame = phich_frame2_pusch_frame(frame_parms,phy_vars_eNB->frame,subframe);
  pusch_subframe = phich_subframe2_pusch_subframe(frame_parms,subframe);
  harq_pid = subframe2harq_pid(frame_parms,pusch_frame,pusch_subframe);

  for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
    if (ulsch_eNB[UE_id]) {
      /*
      msg("[PHY][eNB][PUSCH %x/%d] Frame %d subframe %d (pusch_subframe %d,pusch_frame %d) phich active %d\n",
	  ulsch_eNB[UE_id]->rnti,harq_pid,phy_vars_eNB->frame,subframe,pusch_subframe,pusch_frame,ulsch_eNB[UE_id]->harq_processes[harq_pid]->phich_active);
      */
      if (ulsch_eNB[UE_id]->harq_processes[harq_pid]->phich_active == 1) {
	ngroup_PHICH = (ulsch_eNB[UE_id]->harq_processes[harq_pid]->first_rb + 
			ulsch_eNB[UE_id]->harq_processes[harq_pid]->n_DMRS)%Ngroup_PHICH;
	if ((frame_parms->tdd_config == 0) && (frame_parms->frame_type == 1) ) {
	  
	  if ((pusch_subframe == 4) || (pusch_subframe == 9))
	    ngroup_PHICH += Ngroup_PHICH; 
	}
	nseq_PHICH = ((ulsch_eNB[UE_id]->harq_processes[harq_pid]->first_rb/Ngroup_PHICH) + 
		      ulsch_eNB[UE_id]->harq_processes[harq_pid]->n_DMRS)%(2*NSF_PHICH);
#ifdef DEBUG_PHICH
	msg("[PHY][eNB %d][PUSCH %d] Frame %d subframe %d Generating PHICH, ngroup_PHICH %d/%d, nseq_PHICH %d : HI %d, first_rb %d dci_alloc %d)\n",
	    phy_vars_eNB->Mod_id,harq_pid,((subframe==0)?1:0) +phy_vars_eNB->frame,
	    subframe,ngroup_PHICH,Ngroup_PHICH,nseq_PHICH,
	    ulsch_eNB[UE_id]->harq_processes[harq_pid]->phich_ACK,
	    ulsch_eNB[UE_id]->harq_processes[harq_pid]->first_rb,
	    ulsch_eNB[UE_id]->harq_processes[harq_pid]->dci_alloc);

#endif
	if (ulsch_eNB[UE_id]->Msg3_active == 1) {
	  msg("[PHY][eNB %d][PUSCH %d][RAPROC] Frame %d, subframe %d: Generating Msg3 PHICH for UE %d, ngroup_PHICH %d/%d, nseq_PHICH %d : HI %d, first_rb %d\n",
	      phy_vars_eNB->Mod_id,harq_pid,phy_vars_eNB->frame,subframe,
	      UE_id,ngroup_PHICH,Ngroup_PHICH,nseq_PHICH,ulsch_eNB[UE_id]->harq_processes[harq_pid]->phich_ACK,
	      ulsch_eNB[UE_id]->harq_processes[harq_pid]->first_rb);
	}
	if (abstraction_flag == 0) {
	  generate_phich(frame_parms,
			 amp,//amp*2,
			 nseq_PHICH,
			 ngroup_PHICH,
			 ulsch_eNB[UE_id]->harq_processes[harq_pid]->phich_ACK,
			 subframe,
			 txdataF);
	}
	else {

	}
	// if no format0 DCI was transmitted by MAC, prepare the 
	// MCS parameters for the retransmission

	if ((ulsch_eNB[UE_id]->harq_processes[harq_pid]->dci_alloc == 0) &&  
	    (ulsch_eNB[UE_id]->harq_processes[harq_pid]->rar_alloc == 0) ){
	  if (ulsch_eNB[UE_id]->harq_processes[harq_pid]->phich_ACK==0 ){
	    msg("[PHY][eNB %d][PUSCH %d] frame %d, subframe %d : PHICH ACK / (no format0 DCI) Setting subframe_scheduling_flag\n",
		phy_vars_eNB->Mod_id,harq_pid,phy_vars_eNB->frame,subframe);
	    ulsch_eNB[UE_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
	    ulsch_eNB[UE_id]->harq_processes[harq_pid]->Ndi = 0;
	    //	    ulsch_eNB[UE_id]->harq_processes[harq_pid]->round++;
	  }
	  else {
	    msg("[PHY][eNB %d][PUSCH %d] frame %d subframe %d PHICH ACK (no format0 DCI) Clearing subframe_scheduling_flag, setting round to 0\n",
		phy_vars_eNB->Mod_id,harq_pid,phy_vars_eNB->frame,subframe);
	    ulsch_eNB[UE_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;
	    ulsch_eNB[UE_id]->harq_processes[harq_pid]->round=0;
	  }
	}
	ulsch_eNB[UE_id]->harq_processes[harq_pid]->phich_active=0;
      } // phich_active==1
    } //ulsch_ue[UE_id] is non-null
  }// UE loop
}
