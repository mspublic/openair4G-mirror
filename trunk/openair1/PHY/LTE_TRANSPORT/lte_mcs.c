#include "PHY/defs.h"
#include "PHY/extern.h"

unsigned char get_Qm(unsigned char I_MCS) {

  if (I_MCS < 10)
    return(2);
  else if (I_MCS < 17)
    return(4);
  else
    return(6);
    
}

unsigned char get_I_TBS(unsigned char I_MCS) {

  if (I_MCS < 10)
    return(I_MCS);
  else if (I_MCS == 10)
    return(9);
  else if (I_MCS < 17)
    return(I_MCS-1);
  else if (I_MCS == 17)
    return(15);
  else return(I_MCS-2);

}

unsigned char I_TBS2I_MCS(unsigned char I_TBS) {
  unsigned char I_MCS = -1;
  ///note: change from <= to < to go from choosing higher modulation rather than high code-rate
	  if (I_TBS <= 9)
	    I_MCS = I_TBS;
	  else if (I_TBS <= 15)
	    I_MCS = I_TBS + 1;
	  else if (I_TBS > 15 && I_TBS <= 26)
	    I_MCS = I_TBS + 2;
#ifdef OUTPUT_DEBUG
  printf("I_MCS: %d, from mod_order %d and I_TBS %d\n",I_MCS,modOrder(I_MCS,I_TBS),I_TBS);
  if (I_MCS == 0xFF) getchar();
#endif
  return I_MCS;
}

u16 get_TBS(u8 mcs,u16 nb_rb) {

  u16 TBS;

  if ((nb_rb > 0) && (mcs < 28)) {
#ifdef TBS_FIX
    TBS = 3*dlsch_tbs25[get_I_TBS(mcs)][nb_rb-1]/4;
    TBS = TBS>>3;
#else
    TBS = dlsch_tbs25[get_I_TBS(mcs)][nb_rb-1];
    TBS = TBS>>3;
#endif
    return(TBS);
  }
  else {
    return(0);
  }
}

u16 adjust_G2(LTE_DL_FRAME_PARMS *frame_parms,u32 *rb_alloc,u8 mod_order,u8 subframe,u8 symbol) {

  u16 rb,re_pbch_sss=0;
  u8 rb_alloc_ind,nsymb;

  nsymb = (frame_parms->Ncp==0) ? 14 : 12;

  //    printf("adjust_G_tdd2 : symbol %d, subframe %d\n",symbol,subframe);
  if ((subframe!=0) && (subframe!=5))  // if not PBCH/SSS or SSS
    return(0);

  //first half of slot
  if (symbol<(nsymb>>1))
    return(0);

  // after PBCH
  if (frame_parms->frame_type==1) { //TDD 
    if ((symbol>((nsymb>>1)+3)) && (symbol!=(nsymb-1)))
      return(0);

    if ((subframe==5) && (symbol!=(nsymb-1)))
      return(0);
  }
  else {  // FDD
    if ((symbol>((nsymb>>1)+3)) && (symbol!=(nsymb-1)) && (symbol!=(nsymb-2)))
      return(0);

    if ((subframe==5) && (symbol!=(nsymb-1))&& (symbol!=(nsymb-1)))
      return(0);
  }

  if ((frame_parms->N_RB_DL&1) == 1) { // ODD N_RB_DL

    for (rb=((frame_parms->N_RB_DL>>1)-3);
	 rb<=((frame_parms->N_RB_DL>>1)+3);
	 rb++) {
    
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
      
      if (rb_alloc_ind==1) {
	if ((rb==(frame_parms->N_RB_DL>>1)-3) || 
	    (rb==((frame_parms->N_RB_DL>>1)+3))) {
	  re_pbch_sss += 6;
	}
	else
	  re_pbch_sss += 12;
      }
    }
  }
  else {
    for (rb=((frame_parms->N_RB_DL>>1)-3);
	 rb<((frame_parms->N_RB_DL>>1)+3);
	 rb++) {
    
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
      
      if (rb_alloc_ind==1) {
	  re_pbch_sss += 12;
      }
    }
  }
  //  printf("re_pbch_sss %d\n",re_pbch_sss);
  return(((frame_parms->frame_type==0) ? 2 : 1) * re_pbch_sss);
}

u16 adjust_G(LTE_DL_FRAME_PARMS *frame_parms,u32 *rb_alloc,u8 mod_order,u8 subframe) {

  u16 rb,re_pbch_sss=0;
  u8 rb_alloc_ind;

  if ((subframe!=0) && (subframe!=5))  // if not PBCH/SSS or SSS
    return(0);


  if ((frame_parms->N_RB_DL&1) == 1) { // ODD N_RB_DL

    for (rb=((frame_parms->N_RB_DL>>1)-3);
	 rb<=((frame_parms->N_RB_DL>>1)+3);
	 rb++) {
    
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
      
      if (rb_alloc_ind==1) {
	if ((rb==(frame_parms->N_RB_DL>>1)-3) || 
	    (rb==((frame_parms->N_RB_DL>>1)+3))) {
	  re_pbch_sss += 6;
	}
	else
	  re_pbch_sss += 12;
      }
    }
  }
  else {
    for (rb=((frame_parms->N_RB_DL>>1)-3);
	 rb<((frame_parms->N_RB_DL>>1)+3);
	 rb++) {
    
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
      
      if (rb_alloc_ind==1) {
	  re_pbch_sss += 12;
      }
    }
  }
  //  printf("re_pbch_sss %d\n",re_pbch_sss);
  if (subframe==0) {  //PBCH+SSS+PSS
    if (frame_parms->frame_type == 1) { // TDD
      if (frame_parms->mode1_flag==0)
	//2ant so PBCH 3+2/3 symbols, SSS 1 symbol * REs => (14/3)*re_pbch_sss for normal CP,  
	// 2+4/3 symbols, SSS 1 symbol => (13/3)*re_pbch_sss for ext. CP
	return((-frame_parms->Ncp+14)*re_pbch_sss * mod_order/3);
      else
	//SISO so PBCH 3+(5/6) symbols, SSS 1 symbol * REs => (29/6)*re_pbch_sss for normal CP,  
	// 2+10/6 symbols, SSS 1 symbol => (28/6)*re_pbch_sss for ext. CP
	return((-frame_parms->Ncp+29)*re_pbch_sss * mod_order/6);
    }
    else {  // FDD
      if (frame_parms->mode1_flag==0)
	//2ant so PBCH 3+2/3 symbols, PSS+SSS 2 symbols * REs => (17/3)*re_pbch_sss for normal CP,  
	// 2+4/3 symbols, PSS+SSS 2 symbols => (16/3)*re_pbch_sss for ext. CP
	return((-frame_parms->Ncp+17)*re_pbch_sss * mod_order/3);
      else
	//SISO so PBCH 3+(5/6) symbols, PSS+SSS 2symbols REs => (35/6)*re_pbch_sss for normal CP,  
	// 2+10/6 symbols, SSS+PSS 2 symbols => (34/6)*re_pbch_sss for ext. CP
	return((-frame_parms->Ncp+35)*re_pbch_sss * mod_order/6);

    }
  }
  else  // SSS+PSS
    return(((frame_parms->frame_type==0)?2:1)*re_pbch_sss * 1 * mod_order);
}

u16 get_G(LTE_DL_FRAME_PARMS *frame_parms,u16 nb_rb,u32 *rb_alloc,u8 mod_order,u8 num_pdcch_symbols,u8 subframe) {

  

  u16 G_adj = adjust_G(frame_parms,rb_alloc,mod_order,subframe);

  //  printf("get_G subframe %d mod_order %d: rb_alloc %x, G_adj %d\n",subframe,mod_order,rb_alloc[0], G_adj);
  if (frame_parms->Ncp==0) { // normal prefix
  // PDDDPDD PDDDPDD - 13 PDSCH symbols, 10 full, 3 w/ pilots = 10*12 + 3*8
  // PCDDPDD PDDDPDD - 12 PDSCH symbols, 9 full, 3 w/ pilots = 9*12 + 3*8
  // PCCDPDD PDDDPDD - 11 PDSCH symbols, 8 full, 3 w/pilots = 8*12 + 3*8
    if (frame_parms->mode1_flag==0)
      return((nb_rb * mod_order * ((11-num_pdcch_symbols)*12 + 3*8)) - G_adj);
    else
      return((nb_rb * mod_order * ((11-num_pdcch_symbols)*12 + 3*10)) - G_adj);
  }
  else {
  // PDDPDD PDDPDD - 11 PDSCH symbols, 8 full, 3 w/ pilots = 8*12 + 3*8
  // PCDPDD PDDPDD - 10 PDSCH symbols, 7 full, 3 w/ pilots = 7*12 + 3*8
  // PCCPDD PDDPDD - 9 PDSCH symbols, 6 full, 3 w/pilots = 6*12 + 3*8
    if (frame_parms->mode1_flag==0)
      return((nb_rb * mod_order * ((9-num_pdcch_symbols)*12 + 3*8)) - G_adj);
    else
      return((nb_rb * mod_order * ((9-num_pdcch_symbols)*12 + 3*10)) - G_adj);
  }
}
// following function requires dlsch_tbs_full.h
#include "PHY/LTE_TRANSPORT/dlsch_tbs_full.h"

unsigned char SE2I_TBS(float SE,
		       unsigned char N_PRB,
		       unsigned char symbPerRB) {
  unsigned char I_TBS= -1;
  int throughPutGoal = 0;
  short diffOld = abs(TBStable[0][N_PRB-1] - throughPutGoal);
  short diffNew = diffOld;
  int i = 0;
  throughPutGoal = (int)(((SE*1024)*N_PRB*symbPerRB*12)/1024);
#ifdef OUTPUT_DEBUG
  printf("Throughput goal = %d, from SE = %f\n",throughPutGoal,SE);
#endif
  I_TBS = 0;
  for (i = 0; i<TBStable_rowCnt; i++) {
    diffNew = abs(TBStable[i][N_PRB-1] - throughPutGoal);
    if (diffNew <= diffOld) {
      diffOld = diffNew;
      I_TBS = i;
    } else {
#ifdef OUTPUT_DEBUG
      printf("diff neglected: %d\n",diffNew);
#endif
      break; // no need to continue, strictly increasing function...
    }
#ifdef OUTPUT_DEBUG
    printf("abs(%d - %d) = %d, --> I_TBS = %d\n",TBStable[i][N_PRB-1],throughPutGoal,diffNew,I_TBS);
#endif
  }
  return I_TBS;
}

//added for ALU icic purpose

u8 Get_SB_size(u8 n_rb_dl){

	if(n_rb_dl<27)
		return 4;
	else
		if(n_rb_dl<64)
			return 6;
		else
			return 8;
	}


//end ALU's algo
