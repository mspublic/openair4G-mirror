#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#ifndef USER_MODE
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#endif

//#define DEBUG_PHICH

extern unsigned short pcfich_reg[4];
unsigned short phich_reg_ext[MAX_NUM_PHICH_GROUPS][3];

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


void generate_phich_reg_mapping_ext(LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned short n0 = (frame_parms->N_RB_DL * 2) - 4;
  unsigned short n1 = (frame_parms->N_RB_DL * 3);
  unsigned short n2 = n1;
  unsigned short mprime = 0,mprime2=0;
  unsigned short Ngroup_PHICH;

  Ngroup_PHICH = frame_parms->Ng_times6*(frame_parms->N_RB_DL/48);



  if (((frame_parms->Ng_times6*frame_parms->N_RB_DL)%48) > 0)
    Ngroup_PHICH++;

  if (frame_parms->Ncp == 1) {
    Ngroup_PHICH<<=1;
  }

#ifdef DEBUG_PHICH
  msg("Ngroup_PHICH %d (Ng_times6 %d)\n",Ngroup_PHICH,frame_parms->Ng_times6);
#endif
  

  for (mprime=0;mprime<(Ngroup_PHICH>>1);mprime++) {
    mprime2=mprime;
    phich_reg_ext[mprime][0] = (frame_parms->Nid_cell + mprime2)%n0;
    // check for overlap with PCFICH
    if ((phich_reg_ext[mprime][0] == pcfich_reg[0]) ||
	(phich_reg_ext[mprime][0] == pcfich_reg[1]) ||
	(phich_reg_ext[mprime][0] == pcfich_reg[2]) ||
	(phich_reg_ext[mprime][0] == pcfich_reg[3])) {
      mprime2++;
      phich_reg_ext[mprime][0] = (frame_parms->Nid_cell + mprime2)%n0;
    }

    phich_reg_ext[mprime][1] = ((frame_parms->Nid_cell*n1/n0) + mprime + (n1/3))%n1;
    phich_reg_ext[mprime][2] = ((frame_parms->Nid_cell*n1/n0) + mprime + (2*n2/3))%n2;
#ifdef DEBUG_PHICH
  debug_msg("phich_reg :%d => %d,%d,%d\n",mprime,phich_reg_ext[mprime][0],phich_reg_ext[mprime][1],phich_reg_ext[mprime][2]);
#endif
  }
}

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
  unsigned int i,aa;
  unsigned int re_offset;
  // 
  // scrambling (later)

  memset(d,0,4*sizeof(mod_sym_t));


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
      ((short*)&dp)[0] = z;
      ((short*)&dp)[2] = z;
#else
      dp[0] = 4;
      dp[1] = 4;
#endif
      break;
    case 1: // +1 -1
#ifndef IFFT_FPGA 
      ((short*)&dp)[4] = z;
      ((short*)&dp)[6] = -z;
#else
      dp[2] = 4;
      dp[3] = 1;
#endif
      break;
    case 2: // +j +j 
#ifndef IFFT_FPGA
      ((short*)&dp)[1] = z;
      ((short*)&dp)[3] = z;
#else
      dp[0] = 2;
      dp[1] = 2;
#endif
      break;
    case 3: // +j -j 
#ifndef IFFT_FPGA
      ((short*)&dp)[5] = z;
      ((short*)&dp)[7] = -z;
#else
      dp[2]  = 2;
      dp[3]  = 1;
#endif
      break;
    default:
      msg("phich_coding.c: Illegal PHICH Number\n");
    }



    if (frame_parms->mode1_flag == 0) {
      // do Alamouti precoding here
      
      // Symbol 0
      // ignore for now

      // Symbol 1
      re_offset = frame_parms->N_RB_DL*18 + (phich_reg_ext[ngroup_PHICH][1]<<2);
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
      re_offset = frame_parms->N_RB_DL*30 + (phich_reg_ext[ngroup_PHICH][2]<<2);
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
      re_offset = frame_parms->N_RB_DL*18 + (phich_reg_ext[ngroup_PHICH][1]<<2);
      if (re_offset > frame_parms->N_RB_DL*24)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[0];
      y[1][re_offset]   += d[0];
      y[0][re_offset+1] += d[1];
      y[1][re_offset+1] += d[1];

      re_offset+=2;
      if (re_offset > frame_parms->N_RB_DL*24)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[2];
      y[1][re_offset]   += d[2];
      y[0][re_offset+1] += d[3];
      y[1][re_offset+1] += d[3];

      // Symbol 2
      re_offset = frame_parms->N_RB_DL*30 + (phich_reg_ext[ngroup_PHICH][2]<<2);
      if (re_offset > frame_parms->N_RB_DL*36)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[0];
      y[1][re_offset]   += d[0];
      y[0][re_offset+1] += d[1];
      y[1][re_offset+1] += d[1];
      re_offset+=2;
      if (re_offset > frame_parms->N_RB_DL*36)
	re_offset-=frame_parms->N_RB_DL*12;
      y[0][re_offset]   += d[2];
      y[1][re_offset]   += d[2];
      y[0][re_offset+1] += d[3];
      y[1][re_offset+1] += d[3];
    } // mode1_flag
  }// normal/extended
}  


void generate_phich_top(LTE_DL_FRAME_PARMS *frame_parms,
			unsigned char eNb_id,
			unsigned char subframe) {

  unsigned short UE_id=0;
  unsigned char harq_pid;
  unsigned char sector_id;

  harq_pid = subframe2_ul_harq(frame_parms->tdd_config,subframe);

  if (ulsch_eNb[UE_id]->harq_processes[harq_pid]->phich_active == 1) {

 #ifndef USER_MODE
    for (eNb_id=0;eNb_id<number_of_cards;eNb_id++)
      generate_phich_tdd(frame_parms,
			 0, // nseq_PHICH
			 0, // ngroup_PHICH,
			 ulsch_eNb[UE_id]->harq_processes[harq_pid]->phich_ACK,
			 lte_eNB_common_vars->txdataF[sector_id]);
#else
      generate_phich_tdd(frame_parms,
			 0, // nseq_PHICH
			 0, // ngroup_PHICH,
			 ulsch_eNb[UE_id]->harq_processes[harq_pid]->phich_ACK,
			 lte_eNB_common_vars->txdataF[0]);

#endif
  }
}

