#define modOrder(I_MCS,I_TBS) ((I_MCS-I_TBS)*2+2) // Find modulation order from I_TBS and I_MCS

/** \fn unsigned char I_TBS2I_MCS(unsigned char I_TBS);
\brief This function maps I_tbs to I_mcs according to Table 7.1.7.1-1 in 3GPP TS 36.213 V8.6.0. Where there is two supported modulation orders for the same I_TBS then either high or low modulation is chosen by changing the equality of the two first comparisons in the if-else statement.
\param I_TBS Index of Transport Block Size
\return I_MCS given I_TBS
*/
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

/** \fn unsigned char SE2I_TBS(float SE,
		    unsigned char N_PRB,
		    unsigned char symbPerRB);
\brief This function maps a requested throughput in number of bits to I_tbs. The throughput is calculated as a function of modulation order, RB allocation and number of symbols per RB. The mapping orginates in the "Transport block size table" (Table 7.1.7.2.1-1 in 3GPP TS 36.213 V8.6.0)
\param SE Spectral Efficiency (before casting to integer, multiply by 1024, remember to divide result by 1024!)
\param N_PRB Number of PhysicalResourceBlocks allocated \sa lte_frame_parms->N_RB_DL
\param symbPerRB Number of symbols per resource block allocated to this channel
\return I_TBS given an SE and an N_PRB
*/
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
