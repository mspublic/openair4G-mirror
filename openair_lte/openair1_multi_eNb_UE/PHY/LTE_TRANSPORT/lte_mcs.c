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

u16 get_G(LTE_DL_FRAME_PARMS *frame_parms,u16 nb_rb,u8 mod_order,u8 num_pdcch_symbols) {

  if (frame_parms->Ncp==0) { // normal prefix
  // PDDDPDD PDDDPDD - 13 PDSCH symbols, 10 full, 3 w/ pilots = 10*12 + 3*8
  // PCDDPDD PDDDPDD - 12 PDSCH symbols, 9 full, 3 w/ pilots = 9*12 + 3*8
  // PCCDPDD PDDDPDD - 11 PDSCH symbols, 8 full, 3 w/pilots = 8*12 + 3*8
    return(nb_rb * mod_order * (13-num_pdcch_symbols)*12);
  }
  else {
  // PDDPDD PDDPDD - 13 PDSCH symbols, 8 full, 3 w/ pilots = 8*12 + 3*8
  // PCDPDD PDDPDD - 12 PDSCH symbols, 7 full, 3 w/ pilots = 7*12 + 3*8
  // PCCPDD PDDPDD - 11 PDSCH symbols, 6 full, 3 w/pilots = 6*12 + 3*8
    return(nb_rb * mod_order * (11-num_pdcch_symbols)*12);
  }
}
// following function requires dlsch_tbs_full.h
#include "PHY/LTE_TRANSPORT/dlsch_tbs_full.h"

unsigned char SE2I_TBS(float SE,
		       unsigned char N_PRB,
		       unsigned char symbPerRB) {
  unsigned char I_TBS= -1;
  int throughPutGoal = 0;
  throughPutGoal = (int)(((SE*1024)*N_PRB*symbPerRB*12)/1024);
#ifdef OUTPUT_DEBUG
  printf("Throughput goal = %d, from SE = %f\n",throughPutGoal,SE);
#endif
    I_TBS = 0;
    short diffOld = abs(TBStable[0][N_PRB-1] - throughPutGoal);
    short diffNew = diffOld;
    I_TBS = 0;
    int i = 0;
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
