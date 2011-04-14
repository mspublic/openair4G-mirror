#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#ifndef USER_MODE
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#endif

#define DEBUG_PHICH

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

unsigned char subframe2_ul_harq(unsigned char tdd_config,unsigned char subframe) {

  switch (tdd_config) {
  case 3:
    if ( (subframe == 8) || (subframe == 9) ){
      return(subframe-8);
    }
    else if (subframe==0)
      return(2);
    else {
      msg("phich.c: subframe2_ul_harq, illegal subframe %d for tdd_config %d\n",
	  subframe,tdd_config);
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
  u8 skip=0;

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
// Note: (slightly modified for IFFT_FPGA as : 1, 1 => (1+j), -1 => (-1-j), j => (1-j), -j => (-1+j))
void generate_phich_tdd(LTE_DL_FRAME_PARMS *frame_parms,
			unsigned char nseq_PHICH,
			unsigned char ngroup_PHICH,
			unsigned char HI,
			mod_sym_t **y) {

  mod_sym_t d[4],*dp;
  //  unsigned int i,aa;
  unsigned int re_offset;
  unsigned short c;
  // 
  // scrambling (later)

  memset(d,0,4*sizeof(mod_sym_t));

  if (HI>0)
    HI=1;
  c = (1-(2*HI))*1024;

  if (frame_parms->Ncp == 0) { // Normal Cyclic Prefix

      switch (nseq_PHICH) {
      case 0: // +1 +1 +1 +1
	break;
      case 1: // +1 -1 +1 -1
	break;
      case 2: // +1 +1 -1 -1
	break;
      case 3: // +1 -1 -1 +1
	break;
      case 4: // +j +j +j +j
	break;
      case 5: // +j -j +j -j
	break;
      case 6: // +j +j -j -j
	break;
      case 7: // +j -j -j +j
	break;
      default:
	msg("phich_coding.c: Illegal PHICH Number\n");
      } // nseq_PHICH

      // modulation here
  }

  else {

    if ((ngroup_PHICH & 1) == 1)
      dp = &d[2];
    else  
      dp = d;

    switch (nseq_PHICH) {
    case 0: // +1 +1 
#ifndef IFFT_FPGA
      ((short*)&dp)[0] = c;
      ((short*)&dp)[2] = c;
#else
      dp[0] = 4;
      dp[1] = 4;
#endif
      break;
    case 1: // +1 -1
#ifndef IFFT_FPGA 
      ((short*)&dp)[4] = c;
      ((short*)&dp)[6] = -c;
#else
      dp[2] = 4;
      dp[3] = 1;
#endif
      break;
    case 2: // +j +j 
#ifndef IFFT_FPGA
      ((short*)&dp)[1] = c;
      ((short*)&dp)[3] = c;
#else
      dp[0] = 2;
      dp[1] = 2;
#endif
      break;
    case 3: // +j -j 
#ifndef IFFT_FPGA
      ((short*)&dp)[5] = c;
      ((short*)&dp)[7] = -c;
#else
      dp[2]  = 2;
      dp[3]  = 1;
#endif
      break;
    default:
      msg("[PHY] phich_coding.c: Illegal PHICH Number\n");
    }



    if (frame_parms->mode1_flag == 0) {
      // do Alamouti precoding here
      
      // Symbol 0
      // ignore for now

      // Symbol 1
      re_offset = frame_parms->N_RB_DL*18 + (phich_reg[ngroup_PHICH][1]<<2);
      if (re_offset > frame_parms->N_RB_DL*24)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[0];
      y[1][re_offset]   += alam_bpsk_perm1[d[0]-1];
      y[0][re_offset+1] += d[1];
      y[1][re_offset+1] += alam_bpsk_perm2[d[1]-1];
      re_offset+=2;
      if (re_offset > frame_parms->N_RB_DL*24)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[2];
      y[1][re_offset]   += alam_bpsk_perm1[d[0]-1];
      y[0][re_offset+1] += d[3];
      y[1][re_offset+1] += alam_bpsk_perm2[d[1]-1];

      // Symbol 2
      re_offset = frame_parms->N_RB_DL*30 + (phich_reg[ngroup_PHICH][2]<<2);
      if (re_offset > frame_parms->N_RB_DL*36)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[0];
      y[1][re_offset]   += alam_bpsk_perm1[d[0]-1];
      y[0][re_offset+1] += d[1];
      y[1][re_offset+1] += alam_bpsk_perm2[d[1]-1];
      re_offset+=2;
      if (re_offset > frame_parms->N_RB_DL*36)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[2];
      y[1][re_offset]   += alam_bpsk_perm1[d[0]-1];
      y[0][re_offset+1] += d[3];
      y[1][re_offset+1] += alam_bpsk_perm2[d[1]-1];      
    } // mode1_flag
    else {
      // Symbol 0
      // ignore for now

      // Symbol 1
      re_offset = frame_parms->N_RB_DL*18 + (phich_reg[ngroup_PHICH][1]<<2);
      if (re_offset > frame_parms->N_RB_DL*24)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[0];
      y[0][re_offset+1] += d[1];
      if (frame_parms->nb_antennas_tx>1) {
	y[1][re_offset]   += d[0];
	y[1][re_offset+1] += d[1];
      }

      re_offset+=2;
      if (re_offset > frame_parms->N_RB_DL*24)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[2];
      y[0][re_offset+1] += d[3];
      if (frame_parms->nb_antennas_tx>1) {
	y[1][re_offset]   += d[2];
	y[1][re_offset+1] += d[3];
      }

      // Symbol 2
      re_offset = frame_parms->N_RB_DL*30 + (phich_reg[ngroup_PHICH][2]<<2);
      if (re_offset > frame_parms->N_RB_DL*36)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[0];
      y[0][re_offset+1] += d[1];
      if (frame_parms->nb_antennas_tx>1) {
	y[1][re_offset]   += d[0];
	y[1][re_offset+1] += d[1];
      }
      re_offset+=2;
      if (re_offset > frame_parms->N_RB_DL*36)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[2];
      y[0][re_offset+1] += d[3];
      if (frame_parms->nb_antennas_tx>1) {
	y[1][re_offset]   += d[2];
	y[1][re_offset+1] += d[3];
      }
    } // mode1_flag
  }// normal/extended
}  


void generate_phich_top(LTE_DL_FRAME_PARMS *frame_parms,
			unsigned char subframe,
			LTE_eNB_ULSCH_t *ulsch_eNB,
			mod_sym_t **txdataF) {

  unsigned char harq_pid;

  harq_pid = subframe2_ul_harq(frame_parms->tdd_config,subframe);

  if (ulsch_eNB->harq_processes[harq_pid]->phich_active == 1) {

    generate_phich_tdd(frame_parms,
		       0, // nseq_PHICH
		       0, // ngroup_PHICH,
		       ulsch_eNB->harq_processes[harq_pid]->phich_ACK,
		       txdataF);
  }
}

void generate_phich_emul(PHY_VARS_eNB *phy_vars_eNB,
			 u8 subframe,
			 LTE_eNB_ULSCH_t *ulsch_eNB) {

  msg("[PHY] EMUL eNB %d generate_phich_emul\n",phy_vars_eNB->Mod_id);
}
