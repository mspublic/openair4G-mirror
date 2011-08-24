#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"
#include <emmintrin.h>

#define max(a,b) (a<b?b:a)

static  int  A[2][2][2*1024] __attribute__((aligned(16)));
static  int  X[14][2*1024]    __attribute__((aligned(16)));
//static  long long int det[1024]       __attribute__((aligned(16)));

// H = [channel_est_f1[0] channel_est_f2[0]]
//     [channel_est_f1[1] channel_est_f2[1]]

int phy_calc_mmse_filter(int **channel_est_f1, 
			 int **channel_est_f2,
			 int *channel_mmse_filter[NB_ANTENNAS_TX][NB_ANTENNAS_RX],
			 int *deti,
			 int *idet,
			 int *sigma2 ) {

  short *dets  = (short*) deti;
  short *idets = (short*) idet;
  //int *deti;
  long long int *det = (long long int *) deti;
  __m128i *det_m128i = (__m128i*)deti;
  __m128i *idet_m128i = (__m128i*)idet;
  __m128i shift, temp_m1, temp_m2;
  int temp;
  int i,ind;
  int ii,jj,aa;
  int avg1,avg2,maxh,log2_maxh,chi,chr;
  long long int max_det, min_det, mean_det;
  int log2_max_det;
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif
  __m128i *m_128; 
  __m128i mask;
  struct complex16 alpha;


  //let H = [a b; c d], then (I+H*H^H) = [1+aa*+bb* c*a+d*b; a*c+b*d 1+cc*+dd*]
  //(I+H*H^H)^-1 = 1/DET [1+cc*+dd* -c*a-d*b; -a*c-b*d 1+aa*+bb*]
  //DET = 

  if ((NB_ANTENNAS_RX != 2) || (NUMBER_OF_OFDM_CARRIERS > 1024)) {
    msg("[openair][PHY][MMSE calc] Error: MMSE filter only works with NB_ANTENNAS==2 and NUMBER_OF_OFDM_CARRIERS < 1024\n");
    return -1;
  }

  // construct the channel matrix H*H^H = [HH11 HH12; HH12 HH22] for all subcarriers.
  // Zero_Buffer(HH11,8*1024);
  // Zero_Buffer(HH22,8*1024);
  // Zero_Buffer(HH12,8*1024);
  Zero_Buffer(channel_mmse_filter[0][0],NUMBER_OF_OFDM_CARRIERS*8);
  Zero_Buffer(channel_mmse_filter[0][1],NUMBER_OF_OFDM_CARRIERS*8);
  Zero_Buffer(channel_mmse_filter[1][0],NUMBER_OF_OFDM_CARRIERS*8);
  Zero_Buffer(channel_mmse_filter[1][1],NUMBER_OF_OFDM_CARRIERS*8);
  Zero_Buffer(A[0][0],NUMBER_OF_OFDM_CARRIERS*8);
  Zero_Buffer(A[0][1],NUMBER_OF_OFDM_CARRIERS*8);
  Zero_Buffer(A[1][0],NUMBER_OF_OFDM_CARRIERS*8);
  Zero_Buffer(A[1][1],NUMBER_OF_OFDM_CARRIERS*8);
  for (i=0;i<14;i++)
    Zero_Buffer(X[i],NUMBER_OF_OFDM_CARRIERS*8);
  Zero_Buffer(det,NUMBER_OF_OFDM_CARRIERS*8);
  Zero_Buffer(idet,NUMBER_OF_OFDM_CARRIERS*8);

#ifdef DEBUG_PHY    
  msg("[OPENAIR1][CALC_MMSE] Zero_Buffer complete\n");
#endif

  //channel_est_f1/2 have a dynamic range of 12 bit (Q5.11). The entries of H^H*H will thus be Q10.22. 

  mult_cpx_vector_h_add32((short*)channel_est_f1[0],(short*)channel_est_f1[0],(short*)X[0],NUMBER_OF_OFDM_CARRIERS,1);
  mult_cpx_vector_h_add32((short*)channel_est_f1[1],(short*)channel_est_f1[1],(short*)X[2],NUMBER_OF_OFDM_CARRIERS,1);
  mult_cpx_vector_h_add32((short*)channel_est_f2[0],(short*)channel_est_f2[0],(short*)X[1],NUMBER_OF_OFDM_CARRIERS,1);
  mult_cpx_vector_h_add32((short*)channel_est_f2[1],(short*)channel_est_f2[1],(short*)X[3],NUMBER_OF_OFDM_CARRIERS,1);
  mult_cpx_vector_h_add32((short*)channel_est_f2[0],(short*)channel_est_f1[0],(short*)X[4],NUMBER_OF_OFDM_CARRIERS,-1);
  mult_cpx_vector_h_add32((short*)channel_est_f2[1],(short*)channel_est_f1[1],(short*)X[5],NUMBER_OF_OFDM_CARRIERS,-1);
  //  mult_cpx_vector_h_add32((short*)channel_est_f1[0],(short*)channel_est_f1[1],(short*)X[12],NUMBER_OF_OFDM_CARRIERS,1);
  //  mult_cpx_vector_h_add32((short*)channel_est_f2[1],(short*)channel_est_f2[0],(short*)X[13],NUMBER_OF_OFDM_CARRIERS,1);

#ifdef DEBUG_PHY    
  msg("[OPENAIR][CALC_MMSE] s2[0] = %d, s2[1] = %d\n",sigma2[0],sigma2[1]);
#endif  

#ifdef USER_MODE
#ifdef DEBUG_PHY    
  for (ii=0;ii<6;ii++){
      sprintf(fname,"X%d.m",ii);
      sprintf(vname,"x%d",ii);
      write_output(fname,vname,
		   (s16 *)X[ii],
		   NUMBER_OF_OFDM_CARRIERS,
		   1,
		   3);
    }
  for (ii=12;ii<14;ii++){
      sprintf(fname,"X%d.m",ii);
      sprintf(vname,"x%d",ii);
      write_output(fname,vname,
		   (s16 *)X[ii],
		   NUMBER_OF_OFDM_CARRIERS,
		   1,
		   3);
    }
#endif //DEBUG_PHY	       
#endif //USER_MODE

  //compute the determinant of (sI + HH) in 64bit precision

  add_cpx_vector32(X[0],X[2],X[6],NUMBER_OF_OFDM_CARRIERS);
  add_cpx_vector32(X[1],X[3],X[7],NUMBER_OF_OFDM_CARRIERS);
  add_cpx_vector32(X[4],X[5],X[8],NUMBER_OF_OFDM_CARRIERS);

  mult_vector32_scalar(X[6],((int)sigma2[1]),X[9],NUMBER_OF_OFDM_CARRIERS);
  mult_vector32_scalar(X[7],((int)sigma2[0]),X[10],NUMBER_OF_OFDM_CARRIERS);
  mult_vector32(X[6],X[7],X[11],NUMBER_OF_OFDM_CARRIERS);
  mult_cpx_vector32_conj(X[8],X[12],NUMBER_OF_OFDM_CARRIERS);

  //dm = s1*s2 + s1*x6 + s2*x7 + x6.*x7 - x8.*conj(x8);

  add_real_vector64_scalar(det,((long long) (sigma2[0]*sigma2[1])), det, NUMBER_OF_OFDM_CARRIERS);
  add_real_vector64(det, X[9], det, NUMBER_OF_OFDM_CARRIERS);
  add_real_vector64(det, X[10], det, NUMBER_OF_OFDM_CARRIERS);
  add_real_vector64(det, X[11], det, NUMBER_OF_OFDM_CARRIERS);
  sub_real_vector64(det, X[12], det, NUMBER_OF_OFDM_CARRIERS);


#ifdef USER_MODE
#ifdef DEBUG_PHY    
  for (ii=6;ii<9;ii++) {
      sprintf(fname,"X%d.m",ii);
      sprintf(vname,"x%d",ii);
      write_output(fname,vname,
		   (s16 *)X[ii],
		   NUMBER_OF_OFDM_CARRIERS,
		   1,
		   3);
    }
  for (ii=9;ii<13;ii++) {
      sprintf(fname,"X%d.m",ii);
      sprintf(vname,"x%d",ii);
      write_output(fname,vname,
		   (s16 *)X[ii],
		   NUMBER_OF_OFDM_CARRIERS,
		   1,
		   6);
    }
  write_output("determ.m","d",
	       (s16 *)det,
	       NUMBER_OF_OFDM_CARRIERS,
	       1,
	       6);
#endif //DEBUG_PHY	       
#endif //USER_MODE

/*
  //since a and d are real, we can use mult_cpx_vector_h
  mult_cpx_vector_h_add32((short*)HH[0][0],(short*)HH[1][1],(short*)det,NUMBER_OF_OFDM_CARRIERS,1);
  //since b=c*, bc=bb* and we can use mult_cpx_vector_h
  mult_cpx_vector_h_add32((short*)HH[0][1],(short*)HH[0][1],(short*)det,NUMBER_OF_OFDM_CARRIERS,-1);
  //the determinant is real and of the format Q1.31 
  */

  //find the minumum in det
  min_det = (long long int) 0x7FFFFFFFFFFFFFFF;
  max_det = 0;
  mean_det = 0;
  ind = FIRST_CARRIER_OFFSET;
  for (i=0;i<NUMBER_OF_USEFUL_CARRIERS-4;i++) {
    if (det[ind] < min_det)
      min_det = det[ind];
    if (det[ind] > max_det)
      max_det = det[ind];
    mean_det = mean_det + det[ind];
    ind++;
    if (ind>=NUMBER_OF_OFDM_CARRIERS-2)
      ind = 2;
  }
  log2_max_det=log2_approx64((unsigned long long int) max_det);
  mean_det = mean_det >> LOG2_NUMBER_OF_OFDM_CARRIERS;

#ifdef DEBUG_PHY
  //if (mac_xface->frame % 100 ==0)
    msg("[OPENAIR1][CALC_MMSE] max_det=%d, min_det = %d, mean_det =  %d\n",log2_approx64((unsigned long long int) max_det), log2_approx64((unsigned long long int)min_det), log2_approx64((unsigned long long int)mean_det));
#endif


  //right shift det, so that it fully uses 16 bit
  //then compute the inverse of the determinant, scaling by mean_det (i.e. d*id = mean_det)
  //idet has format Re -Im Im Re with elements in format  Q1.15 
  //max_idet = 0;

  shift = _mm_cvtsi32_si128(log2_max_det-15);
  min_det >>= (log2_max_det-15);
  mean_det >>= (log2_max_det-14);

#ifdef DEBUG_PHY
  msg("[OPENAIR1][CALC_MMSE] min_det=%lld\n", min_det);
#endif


  for (i=0;i<NUMBER_OF_OFDM_CARRIERS>>1;i++) {
    det_m128i[i] = _mm_srl_epi64(det_m128i[i], shift);
    
    if (deti[4*i] != 0) 
      temp = (int) ((int) (0x7FFF * ((int) mean_det)) / (int) deti[4*i]);
    else
      temp = 0x7FFF;

    //idets[8*i]   = (short) temp;
    //idets[8*i+3] = (short) temp;
    temp_m1 = _mm_setr_epi32(temp,0,0,temp);

    if (deti[4*i+2] != 0) 
      temp = (int) ((int) (0x7FFF * ((int) mean_det)) / (int) deti[4*i+2]);
    else 
      temp = 0x7FFF;

    //idets[8*i+4] = (short) temp;
    //idets[8*i+7] = (short) temp;
    temp_m2 = _mm_setr_epi32(temp,0,0,temp);

    idet_m128i[i] = _mm_packs_epi32(temp_m1,temp_m2);

  }
    /*
#ifdef DEBUG_PHY
  msg("[OPENAIR1][CALC_MMSE] max_idet=%d\n", max_idet);
#endif
    */

#ifdef USER_MODE
#ifdef DEBUG_PHY
  write_output("determ2.m","d2",
	       (s16 *)det,
	       NUMBER_OF_OFDM_CARRIERS,
	       1,
	       3);
  write_output("ideterm.m","id",
	       (s16 *)idet,
	       NUMBER_OF_OFDM_CARRIERS*2,
	       2,
	       1);
#endif //DEBUG_PHY
#endif //USER_MODE

  //shift and pack 
  //add s to diagonals
  //multiply by idet

  add_cpx_vector32(X[0],X[2],A[1][1],NUMBER_OF_OFDM_CARRIERS);
  add_cpx_vector32(X[1],X[3],A[0][0],NUMBER_OF_OFDM_CARRIERS);
  add_cpx_vector32(X[4],X[5],A[0][1],NUMBER_OF_OFDM_CARRIERS);

  //apply idet to M
  //the result should have again Q1.15 format???
  shift_and_pack((short*)A[0][0], 
		 NUMBER_OF_OFDM_CARRIERS, 
		 7);
  shift_and_pack((short*)A[0][1], 
		 NUMBER_OF_OFDM_CARRIERS, 
		 7);
  shift_and_pack((short*)A[1][1], 
		 NUMBER_OF_OFDM_CARRIERS, 
		 7);

  complex_conjugate(A[0][1],A[1][0],NUMBER_OF_OFDM_CARRIERS);


  //add the noise to the diagonal elements 
  //we also have to shift s2 by 7
  for (ii=0;ii<NB_ANTENNAS_TX;ii++) {
    alpha.r = sigma2[ii] >> 7;
    alpha.i = 0;
      add_cpx_vector((s16 *)A[ii][ii], 
		     (short*) &alpha,  
		     (s16 *)A[ii][ii],
		     NUMBER_OF_OFDM_CARRIERS);
  }

  /*
  //apply idet to A
  //the result should have again Q1.15 format???
  mult_cpx_vector((short*)channel_mmse_filter[0][0], 
		  (short*)idet,
		  (short*)channel_mmse_filter[0][0], 
		  NUMBER_OF_OFDM_CARRIERS, 
		  7);
 
  mult_cpx_vector((short*)channel_mmse_filter[0][1], 
		  (short*)idet,
		  (short*)channel_mmse_filter[0][1], 
		  NUMBER_OF_OFDM_CARRIERS, 
		  7);

//  mult_cpx_vector((short*)channel_mmse_filter[1][0], 
//		  (short*)idet,
//		  (short*)channel_mmse_filter[1][0], 
//		  NUMBER_OF_OFDM_CARRIERS, 
//		  7);

  mult_cpx_vector((short*)channel_mmse_filter[1][1], 
		  (short*)idet,
		  (short*)channel_mmse_filter[1][1], 
		  NUMBER_OF_OFDM_CARRIERS, 
		  7);
  */

#ifdef USER_MODE
#ifdef DEBUG_PHY    
  for (ii=0;ii<NB_ANTENNAS_TX;ii++)
    for (jj=0;jj<NB_ANTENNAS_RX;jj++) {
      sprintf(fname,"A%d%d.m",ii,jj);
      sprintf(vname,"a%d%d",ii,jj);
      write_output(fname,vname,
		   (s16 *)A[ii][jj],
		   NUMBER_OF_OFDM_CARRIERS*2,
		   2,
		   1);
    }
#endif //DEBUG_PHY	       
#endif //USER_MODE

  /*
  avg1 = 0;
  avg2 = 0;
  maxh = 0;

  for (aa=0;aa<NB_ANTENNAS;aa++) {
    for (i=0;i<NUMBER_OF_USEFUL_CARRIERS;i++) {
      ii = (FIRST_CARRIER_OFFSET + i)% NUMBER_OF_OFDM_CARRIERS;
      chr = (unsigned int)((short*) channel_est_f1[aa])[0+(ii<<2)];  // real-part
      chi = (unsigned int)((short*) channel_est_f1[aa])[1+(ii<<2)];  // -imag-part
      avg1 += chr*chr + chi*chi;
    }
  }

  for (aa=0;aa<NB_ANTENNAS;aa++) {
    for (i=0;i<NUMBER_OF_USEFUL_CARRIERS;i++) {
      ii = (FIRST_CARRIER_OFFSET + i)% NUMBER_OF_OFDM_CARRIERS;
      chr = (unsigned int)((short*) channel_est_f2[aa])[0+(ii<<2)];  // real-part
      chi = (unsigned int)((short*) channel_est_f2[aa])[1+(ii<<2)];  // -imag-part
      avg1 += chr*chr + chi*chi;
    }
  }

  avg1/=(2 * NB_ANTENNAS * NUMBER_OF_USEFUL_CARRIERS);
  //avg2/=(NB_ANTENNAS * NUMBER_OF_USEFUL_CARRIERS);

  log2_maxh = log2_approx(iSqrt(avg1));

#ifdef DEBUG_PHY
  msg("[OPENAIR1][CALC_MMSE] avg1=%d, avg2=%d, log2_maxh=%d\n",avg1,avg2,log2_maxh);
#endif
  */
  log2_maxh = 7;

  // calcualte the MMSE filter M = HH''*H^H 
  mult_cpx_vector_h((short*)A[0][0],(short*)channel_est_f1[0],(short*)channel_mmse_filter[0][0],NUMBER_OF_OFDM_CARRIERS,log2_maxh,1);
  mult_cpx_vector_h((short*)A[0][1],(short*)channel_est_f2[0],(short*)channel_mmse_filter[0][0],NUMBER_OF_OFDM_CARRIERS,log2_maxh,1);
  mult_cpx_vector_h((short*)A[0][0],(short*)channel_est_f1[1],(short*)channel_mmse_filter[0][1],NUMBER_OF_OFDM_CARRIERS,log2_maxh,1);
  mult_cpx_vector_h((short*)A[0][1],(short*)channel_est_f2[1],(short*)channel_mmse_filter[0][1],NUMBER_OF_OFDM_CARRIERS,log2_maxh,1);
  mult_cpx_vector_h((short*)A[1][0],(short*)channel_est_f1[0],(short*)channel_mmse_filter[1][0],NUMBER_OF_OFDM_CARRIERS,log2_maxh,1);
  mult_cpx_vector_h((short*)A[1][1],(short*)channel_est_f2[0],(short*)channel_mmse_filter[1][0],NUMBER_OF_OFDM_CARRIERS,log2_maxh,1);
  mult_cpx_vector_h((short*)A[1][0],(short*)channel_est_f1[1],(short*)channel_mmse_filter[1][1],NUMBER_OF_OFDM_CARRIERS,log2_maxh,1);
  mult_cpx_vector_h((short*)A[1][1],(short*)channel_est_f2[1],(short*)channel_mmse_filter[1][1],NUMBER_OF_OFDM_CARRIERS,log2_maxh,1);

  /*
  //apply idet to M
  //the result should have again Q1.15 format???
  mult_cpx_vector((short*)channel_mmse_filter[0][0], 
		  (short*)idet,
		  (short*)channel_mmse_filter[0][0], 
		  NUMBER_OF_OFDM_CARRIERS, 
		  6);
  mult_cpx_vector((short*)channel_mmse_filter[0][1], 
		  (short*)idet,
		  (short*)channel_mmse_filter[0][1], 
		  NUMBER_OF_OFDM_CARRIERS, 
		  6);
  mult_cpx_vector((short*)channel_mmse_filter[1][0], 
		  (short*)idet,
		  (short*)channel_mmse_filter[1][0], 
		  NUMBER_OF_OFDM_CARRIERS, 
		  6);
  mult_cpx_vector((short*)channel_mmse_filter[1][1], 
		  (short*)idet,
		  (short*)channel_mmse_filter[1][1], 
		  NUMBER_OF_OFDM_CARRIERS, 
		  6);
*/

  //last step: bring M into alternate complex format Re -Im Im Re
  mask = (__m128i) _mm_set_epi16 (1,1,-1,1,1,1,-1,1);
  for (ii=0;ii<NB_ANTENNAS_TX;ii++)
    for (jj=0;jj<NB_ANTENNAS_RX;jj++) {
      m_128 = (__m128i *)channel_mmse_filter[ii][jj];
      for (i=1;i<NUMBER_OF_OFDM_CARRIERS>>1;i++)
	{
	  // the first two instructions might be replaced with a single one in SSE3
	  *m_128 = _mm_shufflelo_epi16(*m_128,_MM_SHUFFLE(0,1,3,2));
	  *m_128 = _mm_shufflehi_epi16(*m_128,_MM_SHUFFLE(0,1,3,2));
	  *m_128 = _mm_mullo_epi16(*m_128, mask);
	  
	  m_128++;
	}
    }
  return 0;
  
}
