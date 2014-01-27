#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#ifndef USER_MODE
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#endif

//#define DEBUG_PHICH 1

extern unsigned short pcfich_reg[4];
extern unsigned char pcfich_first_reg_idx;

unsigned short phich_reg[MAX_NUM_PHICH_GROUPS][3];

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

unsigned char phich_subframe2_pusch_subframe(LTE_DL_FRAME_PARMS *frame_parms,unsigned char subframe) {

  if (frame_parms->frame_type == 0)
    return(subframe<4 ? subframe+8 : subframe-4);
 
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

int check_pcfich(u16 reg) {

  if ((reg == pcfich_reg[0]) ||
      (reg == pcfich_reg[1]) ||
      (reg == pcfich_reg[2]) ||
      (reg == pcfich_reg[3]))
    return(1);
  return(0);
}

void generate_phich_reg_mapping(LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned short n0 = (frame_parms->N_RB_DL * 2) - 4;  // 2 REG per RB less the 4 used by PCFICH in first symbol
  unsigned short n1 = (frame_parms->N_RB_DL * 3);      // 3 REG per RB in second and third symbol
  unsigned short n2 = n1;
  unsigned short mprime = 0;
  unsigned short Ngroup_PHICH;

  // compute Ngroup_PHICH (see formula at beginning of Section 6.9 in 36-211
  Ngroup_PHICH = frame_parms->phich_config_common.phich_resource*(frame_parms->N_RB_DL/48);
  if (((frame_parms->phich_config_common.phich_resource*frame_parms->N_RB_DL)%48) > 0)
    Ngroup_PHICH++;
  // check if Extended prefix
  if (frame_parms->Ncp == 1) {
    Ngroup_PHICH<<=1;
  }
  
#ifdef DEBUG_PHICH
  msg("[PHY] Ngroup_PHICH %d (phich_config_common.phich_resource %d)\n",((frame_parms->Ncp == 0)?Ngroup_PHICH:(Ngroup_PHICH>>1)),frame_parms->phich_config_common.phich_resource);
#endif

  // This is the algorithm from Section 6.9.3 in 36-211
  for (mprime=0;mprime<((frame_parms->Ncp == 0)?Ngroup_PHICH:(Ngroup_PHICH>>1));mprime++) {

    if (frame_parms->Ncp==0){  // normal prefix

      phich_reg[mprime][0] = (1+frame_parms->Nid_cell + mprime)%n0;

      if (phich_reg[mprime][0]>=pcfich_reg[pcfich_first_reg_idx])
	phich_reg[mprime][0]++;
      if (phich_reg[mprime][0]>=pcfich_reg[(pcfich_first_reg_idx+1)&3])
	phich_reg[mprime][0]++;
      if (phich_reg[mprime][0]>=pcfich_reg[(pcfich_first_reg_idx+2)&3])
	phich_reg[mprime][0]++;
      if (phich_reg[mprime][0]>=pcfich_reg[(pcfich_first_reg_idx+3)&3])
	phich_reg[mprime][0]++;

      phich_reg[mprime][1] = (1+frame_parms->Nid_cell + mprime + (n0/3))%n0;

      if (phich_reg[mprime][1]>=pcfich_reg[pcfich_first_reg_idx])
	phich_reg[mprime][1]++;
      if (phich_reg[mprime][1]>=pcfich_reg[(pcfich_first_reg_idx+1)&3])
	phich_reg[mprime][1]++;
      if (phich_reg[mprime][1]>=pcfich_reg[(pcfich_first_reg_idx+2)&3])
	phich_reg[mprime][1]++;
      if (phich_reg[mprime][1]>=pcfich_reg[(pcfich_first_reg_idx+3)&3])
	phich_reg[mprime][1]++;
 
     phich_reg[mprime][2] = (1+frame_parms->Nid_cell + mprime + (2*n0/3))%n0;
      if (phich_reg[mprime][2]>=pcfich_reg[pcfich_first_reg_idx])
	phich_reg[mprime][2]++;
      if (phich_reg[mprime][2]>=pcfich_reg[(pcfich_first_reg_idx+1)&3])
	phich_reg[mprime][2]++;
      if (phich_reg[mprime][2]>=pcfich_reg[(pcfich_first_reg_idx+2)&3])
	phich_reg[mprime][2]++;
      if (phich_reg[mprime][2]>=pcfich_reg[(pcfich_first_reg_idx+3)&3])
	phich_reg[mprime][2]++;

#ifdef DEBUG_PHICH
      msg("[PHY] phich_reg :%d => %d,%d,%d\n",mprime,phich_reg[mprime][0],phich_reg[mprime][1],phich_reg[mprime][2]);
#endif
    }
    else {  // extended prefix
      phich_reg[mprime<<1][0] = (frame_parms->Nid_cell + mprime)%n0;
      phich_reg[1+(mprime<<1)][0] = (frame_parms->Nid_cell + mprime)%n0;

      phich_reg[mprime<<1][1] = ((frame_parms->Nid_cell*n1/n0) + mprime + (n1/3))%n1;
      phich_reg[mprime<<1][2] = ((frame_parms->Nid_cell*n2/n0) + mprime + (2*n2/3))%n2;

      phich_reg[1+(mprime<<1)][1] = ((frame_parms->Nid_cell*n1/n0) + mprime + (n1/3))%n1;
      phich_reg[1+(mprime<<1)][2] = ((frame_parms->Nid_cell*n2/n0) + mprime + (2*n2/3))%n2;
#ifdef DEBUG_PHICH
      msg("[PHY] phich_reg :%d => %d,%d,%d\n",mprime<<1,phich_reg[mprime<<1][0],phich_reg[mprime][1],phich_reg[mprime][2]);
      msg("[PHY] phich_reg :%d => %d,%d,%d\n",1+(mprime<<1),phich_reg[1+(mprime<<1)][0],phich_reg[1+(mprime<<1)][1],phich_reg[1+(mprime<<1)][2]);
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

  memset(d,0,24*sizeof(s16));

  gain_lin_QPSK = (s16)(((s32)amp*ONE_OVER_SQRT2_Q15)>>15);  


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
    msg("PHICH d = ");
    for (i=0;i<24;i+=2)
      msg("(%d,%d)",d[i],d[i+1]);
    msg("\n");
#endif
      // modulation here
    if (frame_parms->mode1_flag == 0) {
      // do Alamouti precoding here
      
      // Symbol 0
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][0]*6);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset];
      y1 = (s16*)&y[1][re_offset];
      
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
	  y1[j]   += y0_16[m++];
	  y0[j+1] += y0_16[m];
	  y1[j+1] += y0_16[m++];
	}
      }
      // Symbol 1
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][1]*6);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset];
      y1 = (s16*)&y[1][re_offset];
      
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
	  y1[j]   += y0_16[m++];
	  y0[j+1] += y0_16[m];
	  y1[j+1] += y0_16[m++];
	}
      }

      // Symbol 2
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][2]*6);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset];
      y1 = (s16*)&y[1][re_offset];
      
      // first antenna position n -> x0
      y0_16[0]   = d[16]*gain_lin_QPSK;
      y0_16[1]   = d[17]*gain_lin_QPSK;
      // second antenna position n -> -x1*
      y1_16[0]   = d[18]*gain_lin_QPSK;
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
	  y1[j]   += y0_16[m++];
	  y0[j+1] += y0_16[m];
	  y1[j+1] += y0_16[m++];

	}
      }

    } // mode1_flag

    else {
      // Symbol 0
      //      printf("PHICH REG %d\n",phich_reg[ngroup_PHICH][0]);
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][0]*6);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset];
      
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
      //      printf("PHICH REG %d\n",phich_reg[ngroup_PHICH][1]);
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][1]*6);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset];
      
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
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][2]*6);
      //     printf("PHICH REG %d\n",phich_reg[ngroup_PHICH][2]);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);
 
      y0 = (s16*)&y[0][re_offset];
      
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
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][0]*6);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset];
      y1 = (s16*)&y[1][re_offset];
      
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
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][1]<<2);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);
      re_offset += (frame_parms->ofdm_symbol_size);

      y0 = (s16*)&y[0][re_offset];
      y1 = (s16*)&y[1][re_offset];

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
      re_offset = frame_parms->first_carrier_offset +  (phich_reg[ngroup_PHICH][2]<<2);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);
      re_offset += (frame_parms->ofdm_symbol_size<<1);

      y0 = (s16*)&y[0][re_offset];
      y1 = (s16*)&y[1][re_offset];

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
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][0]*6);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);

      y0 = (s16*)&y[0][re_offset];
      
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
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][1]<<2);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);
      re_offset += (frame_parms->ofdm_symbol_size);

      y0 = (s16*)&y[0][re_offset];
      
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
      re_offset = frame_parms->first_carrier_offset + (phich_reg[ngroup_PHICH][2]<<2);
      if (re_offset > frame_parms->ofdm_symbol_size)
	re_offset -= (frame_parms->ofdm_symbol_size-1);
      re_offset += (frame_parms->ofdm_symbol_size<<1);

      y0 = (s16*)&y[0][re_offset];
      
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

// This routine generates the PHICH 

void rx_phich(LTE_DL_FRAME_PARMS *frame_parms,
	      u8 nseq_PHICH,
	      u8 ngroup_PHICH,
	      u8 *HI,
	      u8 subframe,
	      u8 eNB_id,
	      LTE_UE_PDCCH **lte_ue_pdcch_vars) {
  
  s16 phich_d[24],*phich_d_ptr,HI16;
  //  unsigned int i,aa;
  s8 d[24],*dp;
  u16 reg_offset;

  // scrambling
  u32 x1, x2, s=0;
  u8 reset = 1;
  s16 cs[12];
  u32 i,i2,i3,phich_quad,j;
  s32 **rxdataF_comp = lte_ue_pdcch_vars[eNB_id]->rxdataF_comp;

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
    for (i=0;i<24;i+=2) {
      msg("(%d,%d) ",d[i],d[i+1]);
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

#ifdef PHICH_DEBUG
  msg("PHICH (RX) =>");
#endif  
  for (phich_quad=0;phich_quad<3;phich_quad++) {
    if (frame_parms->Ncp == 1) 
      reg_offset = (phich_reg[ngroup_PHICH][phich_quad]*4)+ (phich_quad*frame_parms->N_RB_DL*12);
    else
      reg_offset = (phich_reg[ngroup_PHICH][phich_quad]*4);
    
    dp = &d[phich_quad*8];;
    if (frame_parms->mode1_flag == 0) {
      // do Alamouti combining here
      for (i=0;i<8;i+=2) {
	phich_d_ptr[0] = 0;
	phich_d_ptr[1] = 0;
	
	for (j=0;j<frame_parms->nb_antennas_rx;j++) {
	  
	  phich_d_ptr[0] += (((s16*)&rxdataF_comp[j][reg_offset+i])[0]+
			     ((s16*)&rxdataF_comp[j+2][reg_offset+i+1])[0]);
	  phich_d_ptr[1] += (((s16*)&rxdataF_comp[j][reg_offset+i+1])[0] -
			     ((s16*)&rxdataF_comp[j+2][reg_offset+i])[0]);
	}
	HI16 += ((phich_d_ptr[0] * dp[i<<1])+(phich_d_ptr[1] * dp[1+(i<<1)]));
	phich_d_ptr+=2;
      } // mode1_flag
    }
    else {
      
      for (i=0;i<8;i++) {      
	phich_d_ptr[i] = 0;
	for (j=0;j<frame_parms->nb_antennas_rx;j++)
	  phich_d_ptr[i] += ((s16*)&rxdataF_comp[j][reg_offset])[i]; 

#ifdef DEBUG_PHICH
	msg("%d,",((s16*)&rxdataF_comp[0][reg_offset])[i]);
#endif	  

	HI16 += (phich_d_ptr[i] * dp[i]);	  
      } 
    }// mode1_flag
  }
#ifdef DEBUG_PHICH
  msg("\n");
  msg("HI16 %d\n",HI16);
#endif
  if (HI16<0)
    *HI=1;
  else
    *HI=0;
}


void generate_phich_top(LTE_DL_FRAME_PARMS *frame_parms,
			unsigned char subframe,
			s16 amp,
			LTE_eNB_ULSCH_t *ulsch_eNB,
			mod_sym_t **txdataF) {

  unsigned char harq_pid;
  u8 Ngroup_PHICH,ngroup_PHICH,nseq_PHICH;
  u8 NSF_PHICH = 4;
  u8 pusch_subframe;

  // compute Ngroup_PHICH (see formula at beginning of Section 6.9 in 36-211
  Ngroup_PHICH = frame_parms->phich_config_common.phich_resource*(frame_parms->N_RB_DL/48);
  if (((frame_parms->phich_config_common.phich_resource*frame_parms->N_RB_DL)%48) > 0)
    Ngroup_PHICH++;

  if (frame_parms->Ncp == 1)
    NSF_PHICH = 2;

  harq_pid = subframe2_ul_harq(frame_parms,subframe);

  if (ulsch_eNB->harq_processes[harq_pid]->phich_active == 1) {
    ngroup_PHICH = (ulsch_eNB->harq_processes[harq_pid]->first_rb + 
		    ulsch_eNB->harq_processes[harq_pid]->n_DMRS)%Ngroup_PHICH;
    if ((frame_parms->tdd_config == 0) && (frame_parms->frame_type == 1) ) {
      pusch_subframe = phich_subframe2_pusch_subframe(frame_parms,subframe);
      if ((pusch_subframe == 4) || (pusch_subframe == 9))
	ngroup_PHICH += Ngroup_PHICH; 
    }
    nseq_PHICH = ((ulsch_eNB->harq_processes[harq_pid]->first_rb/Ngroup_PHICH) + 
		  ulsch_eNB->harq_processes[harq_pid]->n_DMRS)%(2*NSF_PHICH);
#ifdef DEBUG_PHICH
    msg("[PHY] phich.c: Generating PHICH for hard_pid %d,ngroup_PHICH %d/%d, nseq_PHICH %d : phich_ACK %d, first_rb %d\n",
	harq_pid,ngroup_PHICH,Ngroup_PHICH,nseq_PHICH,ulsch_eNB->harq_processes[harq_pid]->phich_ACK,
	ulsch_eNB->harq_processes[harq_pid]->first_rb);
#endif
    generate_phich(frame_parms,
		   amp,
		   nseq_PHICH,
		   ngroup_PHICH,
		   ulsch_eNB->harq_processes[harq_pid]->phich_ACK,
		   subframe,
		   txdataF);
								
  }
}

void generate_phich_emul(PHY_VARS_eNB *phy_vars_eNB,
			 u8 subframe,
			 LTE_eNB_ULSCH_t *ulsch_eNB) {

  msg("[PHY] EMUL eNB %d generate_phich_emul\n",phy_vars_eNB->Mod_id);
}