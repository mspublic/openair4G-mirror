/* file: 3gpplte_turbo_decoder_sse_16bit.c
   purpose: Routines for implementing max-logmap decoding of Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
   authors: raymond.knopp@eurecom.fr, Laurent Thomas (Alcatel-Lucent)
   date: 21.10.2009 

   Note: This version of the routine currently requires SSE2,SSSE3 and SSE4.1 equipped computers.  It uses 16-bit inputs for
         LLRS and uses 16-bit arithmetic for the internal computations!
  
   Changelog: 17.11.2009 FK SSE4.1 not required anymore
   Aug. 2012 new parallelization options for higher speed (8-way parallelization)
   Jan. 2013 8-bit LLR support with 16-way parallelization
   Feb. 2013 New interleaving and hard-decision optimizations (L. Thomas)
   May 2013 Extracted 16bit code
*/

///
///

#include "emmintrin.h"
#include "smmintrin.h"

#ifndef TEST_DEBUG 
#include "PHY/defs.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "extern_3GPPinterleaver.h"
#else

#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifdef MEX
#include "mex.h"
#endif


//#define DEBUG_LOGMAP

 
typedef int16_t llr_t; // internal decoder LLR data is 16-bit fixed
typedef int16_t channel_t;
#define MAX 256

void log_map16(llr_t* systematic,channel_t* y_parity, llr_t* m11, llr_t* m10, llr_t *alpha, llr_t *beta, llr_t* ext,unsigned short frame_length,unsigned char term_flag,unsigned char F,int offset8_flag,time_stats_t *alpha_stats,time_stats_t *beta_stats,time_stats_t *gamma_stats,time_stats_t *ext_stats);
void compute_gamma16(llr_t* m11,llr_t* m10,llr_t* systematic, channel_t* y_parity, unsigned short frame_length,unsigned char term_flag);
void compute_alpha16(llr_t*alpha,llr_t *beta, llr_t* m11,llr_t* m10, unsigned short frame_length,unsigned char F);
void compute_beta16(llr_t*alpha, llr_t* beta,llr_t* m11,llr_t* m10, unsigned short frame_length,unsigned char F,int offset8_flag);
void compute_ext16(llr_t* alpha,llr_t* beta,llr_t* m11,llr_t* m10,llr_t* extrinsic, llr_t* ap, unsigned short frame_length);


void log_map16(llr_t* systematic,
	       channel_t* y_parity, 
	       llr_t* m11, 
	       llr_t* m10, 
	       llr_t *alpha, 
	       llr_t *beta,
	       llr_t* ext,
	       unsigned short frame_length,
	       unsigned char term_flag,
	       unsigned char F,
	       int offset8_flag,
	       time_stats_t *alpha_stats,
	       time_stats_t *beta_stats,
	       time_stats_t *gamma_stats,
	       time_stats_t *ext_stats) {
  
#ifdef DEBUG_LOGMAP
  msg("log_map, frame_length %d\n",frame_length);
#endif

  start_meas(gamma_stats) ; compute_gamma16(m11,m10,systematic,y_parity,frame_length,term_flag) ; stop_meas(gamma_stats);
  start_meas(alpha_stats) ; compute_alpha16(alpha,beta,m11,m10,frame_length,F)                  ; stop_meas(alpha_stats);
  start_meas(beta_stats)  ; compute_beta16(alpha,beta,m11,m10,frame_length,F,offset8_flag)      ; stop_meas(beta_stats);
  start_meas(ext_stats)   ; compute_ext16(alpha,beta,m11,m10,ext,systematic,frame_length)       ; stop_meas(ext_stats);


}

void compute_gamma16(llr_t* m11,llr_t* m10,llr_t* systematic,channel_t* y_parity,
		   unsigned short frame_length,unsigned char term_flag) {
  int k,K1;
  __m128i *systematic128 = (__m128i *)systematic;
  __m128i *y_parity128   = (__m128i *)y_parity;
  __m128i *m10_128        = (__m128i *)m10;
  __m128i *m11_128        = (__m128i *)m11;

#ifdef DEBUG_LOGMAP
  msg("compute_gamma, %p,%p,%p,%p,framelength %d\n",m11,m10,systematic,y_parity,frame_length);
#endif

  K1=frame_length>>3;
  for (k=0;k<K1;k++) {

    m11_128[k] = _mm_srai_epi16(_mm_adds_epi16(systematic128[k],y_parity128[k]),1);
    m10_128[k] = _mm_srai_epi16(_mm_subs_epi16(systematic128[k],y_parity128[k]),1);
    /*
    printf("gamma %d: s %d,%d,%d,%d,%d,%d,%d,%d\n",
	   k,
	   (int16_t)_mm_extract_epi16(systematic128[k],0),
	   (int16_t)_mm_extract_epi16(systematic128[k],1),
	   (int16_t)_mm_extract_epi16(systematic128[k],2),
	   (int16_t)_mm_extract_epi16(systematic128[k],3),
	   (int16_t)_mm_extract_epi16(systematic128[k],4),
	   (int16_t)_mm_extract_epi16(systematic128[k],5),
	   (int16_t)_mm_extract_epi16(systematic128[k],6),
	   (int16_t)_mm_extract_epi16(systematic128[k],7));

    printf("gamma %d: yp %d,%d,%d,%d,%d,%d,%d,%d\n",
	   k,
	   (int16_t)_mm_extract_epi16(y_parity128[k],0),
	   (int16_t)_mm_extract_epi16(y_parity128[k],1),
	   (int16_t)_mm_extract_epi16(y_parity128[k],2),
	   (int16_t)_mm_extract_epi16(y_parity128[k],3),
	   (int16_t)_mm_extract_epi16(y_parity128[k],4),
	   (int16_t)_mm_extract_epi16(y_parity128[k],5),
	   (int16_t)_mm_extract_epi16(y_parity128[k],6),
	   (int16_t)_mm_extract_epi16(y_parity128[k],7));

    printf("gamma %d: m11 %d,%d,%d,%d,%d,%d,%d,%d\n",
	   k,
	   (int16_t)_mm_extract_epi16(m11_128[k],0),
	   (int16_t)_mm_extract_epi16(m11_128[k],1),
	   (int16_t)_mm_extract_epi16(m11_128[k],2),
	   (int16_t)_mm_extract_epi16(m11_128[k],3),
	   (int16_t)_mm_extract_epi16(m11_128[k],4),
	   (int16_t)_mm_extract_epi16(m11_128[k],5),
	   (int16_t)_mm_extract_epi16(m11_128[k],6),
	   (int16_t)_mm_extract_epi16(m11_128[k],7));
    printf("gamma %d: m10 %d,%d,%d,%d,%d,%d,%d,%d\n",
	   k,
	   (int16_t)_mm_extract_epi16(m10_128[k],0),
	   (int16_t)_mm_extract_epi16(m10_128[k],1),
	   (int16_t)_mm_extract_epi16(m10_128[k],2),
	   (int16_t)_mm_extract_epi16(m10_128[k],3),
	   (int16_t)_mm_extract_epi16(m10_128[k],4),
	   (int16_t)_mm_extract_epi16(m10_128[k],5),
	   (int16_t)_mm_extract_epi16(m10_128[k],6),
	   (int16_t)_mm_extract_epi16(m10_128[k],7));
    */
          
  }
  // Termination
  m11_128[k] = _mm_srai_epi16(_mm_adds_epi16(systematic128[k+term_flag],y_parity128[k]),1);
  m10_128[k] = _mm_srai_epi16(_mm_subs_epi16(systematic128[k+term_flag],y_parity128[k]),1);

  //  printf("gamma (term): %d,%d, %d,%d, %d,%d\n",m11[k<<3],m10[k<<3],m11[1+(k<<3)],m10[1+(k<<3)],m11[2+(k<<3)],m10[2+(k<<3)]);
}

#define L 40

void compute_alpha16(llr_t* alpha,llr_t* beta,llr_t* m_11,llr_t* m_10,unsigned short frame_length,unsigned char F) {
  int k,l,l2,K1,rerun_flag=0;
  __m128i *alpha128=(__m128i *)alpha,*alpha_ptr;
  __m128i a0,a1,a2,a3,a4,a5,a6,a7,*m11p,*m10p;
  __m128i m_b0,m_b1,m_b2,m_b3,m_b4,m_b5,m_b6,m_b7;
  __m128i new0,new1,new2,new3,new4,new5,new6,new7;
  __m128i alpha_max;

  l2 = L>>3;
  K1 = (frame_length>>3);

  for (l=K1;;l=l2,rerun_flag=1) {
    alpha128 = (__m128i *)alpha;
    if (rerun_flag == 0) {
      alpha128[0] = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,0);
      alpha128[1] = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[2] = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[3] = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[4] = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[5] = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[6] = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[7] = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
    }
    else { 
      //set initial alpha in columns 1-7 from final alpha from last run in columns 0-6
      alpha128[0] = _mm_slli_si128(alpha128[frame_length],2);
      alpha128[1] = _mm_slli_si128(alpha128[1+frame_length],2);
      alpha128[2] = _mm_slli_si128(alpha128[2+frame_length],2);
      alpha128[3] = _mm_slli_si128(alpha128[3+frame_length],2);
      alpha128[4] = _mm_slli_si128(alpha128[4+frame_length],2);
      alpha128[5] = _mm_slli_si128(alpha128[5+frame_length],2);
      alpha128[6] = _mm_slli_si128(alpha128[6+frame_length],2);
      alpha128[7] = _mm_slli_si128(alpha128[7+frame_length],2);
      // set initial alpha in column 0 to (0,-MAX/2,...,-MAX/2)
      alpha[8] = -MAX/2;
      alpha[16] = -MAX/2;
      alpha[24] = -MAX/2;
      alpha[32] = -MAX/2;
      alpha[40] = -MAX/2;
      alpha[48] = -MAX/2;
      alpha[56] = -MAX/2;
    }
  
    alpha_ptr = &alpha128[0];

    m11p = (__m128i*)m_11;
    m10p = (__m128i*)m_10;

    for (k=0;
	 k<l;
	 k++){

      a1=_mm_load_si128(&alpha_ptr[1]);
      a3=_mm_load_si128(&alpha_ptr[3]);
      a5=_mm_load_si128(&alpha_ptr[5]);
      a7=_mm_load_si128(&alpha_ptr[7]);

      m_b0 = _mm_adds_epi16(a1,*m11p);  // m11
      m_b4 = _mm_subs_epi16(a1,*m11p);  // m00=-m11    
      m_b1 = _mm_subs_epi16(a3,*m10p);  // m01=-m10
      m_b5 = _mm_adds_epi16(a3,*m10p);  // m10
      m_b2 = _mm_adds_epi16(a5,*m10p);  // m10
      m_b6 = _mm_subs_epi16(a5,*m10p);  // m01=-m10
      m_b3 = _mm_subs_epi16(a7,*m11p);  // m00=-m11
      m_b7 = _mm_adds_epi16(a7,*m11p);  // m11

      a0=_mm_load_si128(&alpha_ptr[0]);
      a2=_mm_load_si128(&alpha_ptr[2]);
      a4=_mm_load_si128(&alpha_ptr[4]);
      a6=_mm_load_si128(&alpha_ptr[6]);
     
      new0 = _mm_subs_epi16(a0,*m11p);  // m00=-m11
      new4 = _mm_adds_epi16(a0,*m11p);  // m11
      new1 = _mm_adds_epi16(a2,*m10p);  // m10
      new5 = _mm_subs_epi16(a2,*m10p);  // m01=-m10
      new2 = _mm_subs_epi16(a4,*m10p);  // m01=-m10
      new6 = _mm_adds_epi16(a4,*m10p);  // m10
      new3 = _mm_adds_epi16(a6,*m11p);  // m11
      new7 = _mm_subs_epi16(a6,*m11p);  // m00=-m11
      
      a0 = _mm_max_epi16(m_b0,new0);
      a1 = _mm_max_epi16(m_b1,new1);
      a2 = _mm_max_epi16(m_b2,new2);
      a3 = _mm_max_epi16(m_b3,new3);
      a4 = _mm_max_epi16(m_b4,new4);
      a5 = _mm_max_epi16(m_b5,new5);
      a6 = _mm_max_epi16(m_b6,new6);
      a7 = _mm_max_epi16(m_b7,new7);

      alpha_max = _mm_max_epi16(a0,a1);
      alpha_max = _mm_max_epi16(alpha_max,a2);
      alpha_max = _mm_max_epi16(alpha_max,a3);
      alpha_max = _mm_max_epi16(alpha_max,a4);
      alpha_max = _mm_max_epi16(alpha_max,a5);
      alpha_max = _mm_max_epi16(alpha_max,a6);
      alpha_max = _mm_max_epi16(alpha_max,a7);

      alpha_ptr+=8;
      m11p++;
      m10p++;    
      alpha_ptr[0] = _mm_subs_epi16(a0,alpha_max);
      alpha_ptr[1] = _mm_subs_epi16(a1,alpha_max);
      alpha_ptr[2] = _mm_subs_epi16(a2,alpha_max);
      alpha_ptr[3] = _mm_subs_epi16(a3,alpha_max);
      alpha_ptr[4] = _mm_subs_epi16(a4,alpha_max);
      alpha_ptr[5] = _mm_subs_epi16(a5,alpha_max);
      alpha_ptr[6] = _mm_subs_epi16(a6,alpha_max);
      alpha_ptr[7] = _mm_subs_epi16(a7,alpha_max);

    }

    if (rerun_flag==1)
      break;
  }  
}


void compute_beta16(llr_t* alpha,llr_t* beta,llr_t *m_11,llr_t* m_10,unsigned short frame_length,unsigned char F,int offset8_flag) {
  
  int k,rerun_flag=0;
  __m128i m11_128,m10_128;
  __m128i m_b0,m_b1,m_b2,m_b3,m_b4,m_b5,m_b6,m_b7;
  __m128i new0,new1,new2,new3,new4,new5,new6,new7;

  __m128i *beta128,*alpha128,*beta_ptr;
  __m128i beta_max; 
  int16_t m11,m10,beta0_16,beta1_16,beta2_16,beta3_16,beta4_16,beta5_16,beta6_16,beta7_16,beta0_2,beta1_2,beta2_2,beta3_2,beta_m; 
  llr_t beta0,beta1;

#ifdef DEBUG_LOGMAP
  msg("compute_beta, %p,%p,%p,%p,framelength %d,F %d\n",
      beta,m_11,m_10,alpha,frame_length,F);
#endif


  // termination for beta initialization

  //  printf("beta init: offset8 %d\n",offset8_flag);
  m11=(int16_t)m_11[2+frame_length];
  m10=(int16_t)m_10[2+frame_length];

  //  printf("m11,m10 %d,%d\n",m11,m10);

  beta0 = -m11;//M0T_TERM;
  beta1 = m11;//M1T_TERM;
  m11=(int16_t)m_11[1+frame_length];
  m10=(int16_t)m_10[1+frame_length];

  //  printf("m11,m10 %d,%d\n",m11,m10);

  beta0_2 = beta0-m11;//+M0T_TERM;
  beta1_2 = beta0+m11;//+M1T_TERM;
  beta2_2 = beta1+m10;//M2T_TERM;
  beta3_2 = beta1-m10;//+M3T_TERM;
  m11=(int16_t)m_11[frame_length];
  m10=(int16_t)m_10[frame_length];
  //  printf("m11,m10 %d,%d (%p)\n",m11,m10,m_11+frame_length);

  beta0_16 = beta0_2-m11;//+M0T_TERM;
  beta1_16 = beta0_2+m11;//+M1T_TERM;
  beta2_16 = beta1_2+m10;//+M2T_TERM;
  beta3_16 = beta1_2-m10;//+M3T_TERM;
  beta4_16 = beta2_2-m10;//+M4T_TERM;
  beta5_16 = beta2_2+m10;//+M5T_TERM;
  beta6_16 = beta3_2+m11;//+M6T_TERM;
  beta7_16 = beta3_2-m11;//+M7T_TERM;
  

  beta_m = (beta0_16>beta1_16) ? beta0_16 : beta1_16;
  beta_m = (beta_m>beta2_16) ? beta_m : beta2_16;
  beta_m = (beta_m>beta3_16) ? beta_m : beta3_16;
  beta_m = (beta_m>beta4_16) ? beta_m : beta4_16;
  beta_m = (beta_m>beta5_16) ? beta_m : beta5_16;
  beta_m = (beta_m>beta6_16) ? beta_m : beta6_16;
  beta_m = (beta_m>beta7_16) ? beta_m : beta7_16;
  

  beta0_16=beta0_16-beta_m;
  beta1_16=beta1_16-beta_m;
  beta2_16=beta2_16-beta_m;
  beta3_16=beta3_16-beta_m;
  beta4_16=beta4_16-beta_m;
  beta5_16=beta5_16-beta_m;
  beta6_16=beta6_16-beta_m;
  beta7_16=beta7_16-beta_m;

  for (rerun_flag=0;;rerun_flag=1) {
    beta_ptr   = (__m128i*)&beta[frame_length<<3];
    alpha128   = (__m128i*)&alpha[0];
    if (rerun_flag == 0) {
      beta_ptr[0] = alpha128[(frame_length)];
      beta_ptr[1] = alpha128[1+(frame_length)];
      beta_ptr[2] = alpha128[2+(frame_length)];
      beta_ptr[3] = alpha128[3+(frame_length)];
      beta_ptr[4] = alpha128[4+(frame_length)];
      beta_ptr[5] = alpha128[5+(frame_length)];
      beta_ptr[6] = alpha128[6+(frame_length)];
      beta_ptr[7] = alpha128[7+(frame_length)];
    }
    else {
      beta128 = (__m128i*)&beta[0];
      beta_ptr[0] = _mm_srli_si128(beta128[0],2);
      beta_ptr[1] = _mm_srli_si128(beta128[1],2);
      beta_ptr[2] = _mm_srli_si128(beta128[2],2);
      beta_ptr[3] = _mm_srli_si128(beta128[3],2);
      beta_ptr[4] = _mm_srli_si128(beta128[4],2);
      beta_ptr[5] = _mm_srli_si128(beta128[5],2);
      beta_ptr[6] = _mm_srli_si128(beta128[6],2);
      beta_ptr[7] = _mm_srli_si128(beta128[7],2);
    }


    beta_ptr[0] = _mm_insert_epi16(beta_ptr[0],beta0_16,7);
    beta_ptr[1] = _mm_insert_epi16(beta_ptr[1],beta1_16,7);
    beta_ptr[2] = _mm_insert_epi16(beta_ptr[2],beta2_16,7);
    beta_ptr[3] = _mm_insert_epi16(beta_ptr[3],beta3_16,7);
    beta_ptr[4] = _mm_insert_epi16(beta_ptr[4],beta4_16,7);
    beta_ptr[5] = _mm_insert_epi16(beta_ptr[5],beta5_16,7);
    beta_ptr[6] = _mm_insert_epi16(beta_ptr[6],beta6_16,7);
    beta_ptr[7] = _mm_insert_epi16(beta_ptr[7],beta7_16,7);
    
    int loopval=((rerun_flag==0)?0:((frame_length-L)>>3));
    for (k=(frame_length>>3)-1;k>=loopval;k--){
      m11_128=((__m128i*)m_11)[k];  
      m10_128=((__m128i*)m_10)[k];

      m_b0 = _mm_adds_epi16(beta_ptr[4],m11_128);  //m11
      m_b1 = _mm_subs_epi16(beta_ptr[4],m11_128);  //m00
      m_b2 = _mm_subs_epi16(beta_ptr[5],m10_128);  //m01
      m_b3 = _mm_adds_epi16(beta_ptr[5],m10_128);  //m10
      m_b4 = _mm_adds_epi16(beta_ptr[6],m10_128);  //m10   
      m_b5 = _mm_subs_epi16(beta_ptr[6],m10_128);  //m01
      m_b6 = _mm_subs_epi16(beta_ptr[7],m11_128);  //m00
      m_b7 = _mm_adds_epi16(beta_ptr[7],m11_128);  //m11
      
      new0 = _mm_subs_epi16(beta_ptr[0],m11_128);  //m00
      new1 = _mm_adds_epi16(beta_ptr[0],m11_128);  //m11
      new2 = _mm_adds_epi16(beta_ptr[1],m10_128);  //m10
      new3 = _mm_subs_epi16(beta_ptr[1],m10_128);  //m01
      new4 = _mm_subs_epi16(beta_ptr[2],m10_128);  //m01
      new5 = _mm_adds_epi16(beta_ptr[2],m10_128);  //m10
      new6 = _mm_adds_epi16(beta_ptr[3],m11_128);  //m11
      new7 = _mm_subs_epi16(beta_ptr[3],m11_128);  //m00
      
      beta_ptr-=8;
      
      beta_ptr[0] = _mm_max_epi16(m_b0,new0);
      beta_ptr[1] = _mm_max_epi16(m_b1,new1);
      beta_ptr[2] = _mm_max_epi16(m_b2,new2);
      beta_ptr[3] = _mm_max_epi16(m_b3,new3);
      beta_ptr[4] = _mm_max_epi16(m_b4,new4);
      beta_ptr[5] = _mm_max_epi16(m_b5,new5);
      beta_ptr[6] = _mm_max_epi16(m_b6,new6);
      beta_ptr[7] = _mm_max_epi16(m_b7,new7);
      
      beta_max = _mm_max_epi16(beta_ptr[0],beta_ptr[1]);
      beta_max = _mm_max_epi16(beta_max   ,beta_ptr[2]);
      beta_max = _mm_max_epi16(beta_max   ,beta_ptr[3]);
      beta_max = _mm_max_epi16(beta_max   ,beta_ptr[4]);
      beta_max = _mm_max_epi16(beta_max   ,beta_ptr[5]);
      beta_max = _mm_max_epi16(beta_max   ,beta_ptr[6]);
      beta_max = _mm_max_epi16(beta_max   ,beta_ptr[7]);

      beta_ptr[0] = _mm_subs_epi16(beta_ptr[0],beta_max);
      beta_ptr[1] = _mm_subs_epi16(beta_ptr[1],beta_max);
      beta_ptr[2] = _mm_subs_epi16(beta_ptr[2],beta_max);
      beta_ptr[3] = _mm_subs_epi16(beta_ptr[3],beta_max);
      beta_ptr[4] = _mm_subs_epi16(beta_ptr[4],beta_max);
      beta_ptr[5] = _mm_subs_epi16(beta_ptr[5],beta_max);
      beta_ptr[6] = _mm_subs_epi16(beta_ptr[6],beta_max); 
      beta_ptr[7] = _mm_subs_epi16(beta_ptr[7],beta_max);
      


    }

    if (rerun_flag==1)
      break;
  }
}

void compute_ext16(llr_t* alpha,llr_t* beta,llr_t* m_11,llr_t* m_10,llr_t* ext, llr_t* systematic,unsigned short frame_length) {
  __m128i *alpha128=(__m128i *)alpha;
  __m128i *beta128=(__m128i *)beta;
  __m128i *m11_128,*m10_128,*ext_128;
  __m128i *alpha_ptr,*beta_ptr;
  __m128i m00_1,m00_2,m00_3,m00_4;
  __m128i m01_1,m01_2,m01_3,m01_4;
  __m128i m10_1,m10_2,m10_3,m10_4;
  __m128i m11_1,m11_2,m11_3,m11_4;
  int k;

  //
  // LLR computation, 8 consequtive bits per loop
  //

#ifdef DEBUG_LOGMAP
  msg("compute_ext, %p, %p, %p, %p, %p, %p ,framelength %d\n",alpha,beta,m_11,m_10,ext,systematic,frame_length);
#endif

  alpha_ptr = alpha128;
  beta_ptr = &beta128[8];


  for (k=0;k<(frame_length>>3);k++){

    m11_128        = (__m128i*)&m_11[k<<3];
    m10_128        = (__m128i*)&m_10[k<<3];
    ext_128        = (__m128i*)&ext[k<<3];
    /*
      printf("EXT %03d\n",k);
      print_shorts("a0:",&alpha_ptr[0]);
      print_shorts("a1:",&alpha_ptr[1]);
      print_shorts("a2:",&alpha_ptr[2]);
      print_shorts("a3:",&alpha_ptr[3]);
      print_shorts("a4:",&alpha_ptr[4]);
      print_shorts("a5:",&alpha_ptr[5]);
      print_shorts("a6:",&alpha_ptr[6]);
      print_shorts("a7:",&alpha_ptr[7]);
      print_shorts("b0:",&beta_ptr[0]);
      print_shorts("b1:",&beta_ptr[1]);
      print_shorts("b2:",&beta_ptr[2]);
      print_shorts("b3:",&beta_ptr[3]);
      print_shorts("b4:",&beta_ptr[4]);
      print_shorts("b5:",&beta_ptr[5]);
      print_shorts("b6:",&beta_ptr[6]);
      print_shorts("b7:",&beta_ptr[7]);
    */      
    m00_4 = _mm_adds_epi16(alpha_ptr[7],beta_ptr[3]); //ALPHA_BETA_4m00;
    m11_4 = _mm_adds_epi16(alpha_ptr[7],beta_ptr[7]); //ALPHA_BETA_4m11;
    m00_3 = _mm_adds_epi16(alpha_ptr[6],beta_ptr[7]); //ALPHA_BETA_3m00;
    m11_3 = _mm_adds_epi16(alpha_ptr[6],beta_ptr[3]); //ALPHA_BETA_3m11;
    m00_2 = _mm_adds_epi16(alpha_ptr[1],beta_ptr[4]); //ALPHA_BETA_2m00;
    m11_2 = _mm_adds_epi16(alpha_ptr[1],beta_ptr[0]); //ALPHA_BETA_2m11;
    m11_1 = _mm_adds_epi16(alpha_ptr[0],beta_ptr[4]); //ALPHA_BETA_1m11;
    m00_1 = _mm_adds_epi16(alpha_ptr[0],beta_ptr[0]); //ALPHA_BETA_1m00;
    m01_4 = _mm_adds_epi16(alpha_ptr[5],beta_ptr[6]); //ALPHA_BETA_4m01;
    m10_4 = _mm_adds_epi16(alpha_ptr[5],beta_ptr[2]); //ALPHA_BETA_4m10;
    m01_3 = _mm_adds_epi16(alpha_ptr[4],beta_ptr[2]); //ALPHA_BETA_3m01;
    m10_3 = _mm_adds_epi16(alpha_ptr[4],beta_ptr[6]); //ALPHA_BETA_3m10;
    m01_2 = _mm_adds_epi16(alpha_ptr[3],beta_ptr[1]); //ALPHA_BETA_2m01;
    m10_2 = _mm_adds_epi16(alpha_ptr[3],beta_ptr[5]); //ALPHA_BETA_2m10;
    m10_1 = _mm_adds_epi16(alpha_ptr[2],beta_ptr[1]); //ALPHA_BETA_1m10;
    m01_1 = _mm_adds_epi16(alpha_ptr[2],beta_ptr[5]); //ALPHA_BETA_1m01;
    /*
      print_shorts("m11_1:",&m11_1);
      print_shorts("m11_2:",&m11_2);
      print_shorts("m11_3:",&m11_3);
      print_shorts("m11_4:",&m11_4);
      print_shorts("m00_1:",&m00_1);
      print_shorts("m00_2:",&m00_2);
      print_shorts("m00_3:",&m00_3);
      print_shorts("m00_4:",&m00_4);
      print_shorts("m10_1:",&m10_1);
      print_shorts("m10_2:",&m10_2);
      print_shorts("m10_3:",&m10_3);
      print_shorts("m10_4:",&m10_4);
      print_shorts("m01_1:",&m01_1);
      print_shorts("m01_2:",&m01_2);
      print_shorts("m01_3:",&m01_3);
      print_shorts("m01_4:",&m01_4);
    */
    m01_1 = _mm_max_epi16(m01_1,m01_2);
    m01_1 = _mm_max_epi16(m01_1,m01_3);
    m01_1 = _mm_max_epi16(m01_1,m01_4);
    m00_1 = _mm_max_epi16(m00_1,m00_2);
    m00_1 = _mm_max_epi16(m00_1,m00_3);
    m00_1 = _mm_max_epi16(m00_1,m00_4);
    m10_1 = _mm_max_epi16(m10_1,m10_2);
    m10_1 = _mm_max_epi16(m10_1,m10_3);
    m10_1 = _mm_max_epi16(m10_1,m10_4);
    m11_1 = _mm_max_epi16(m11_1,m11_2);
    m11_1 = _mm_max_epi16(m11_1,m11_3);
    m11_1 = _mm_max_epi16(m11_1,m11_4);

    //      print_shorts("m11_1:",&m11_1);
      
    m01_1 = _mm_subs_epi16(m01_1,*m10_128);
    m00_1 = _mm_subs_epi16(m00_1,*m11_128);
    m10_1 = _mm_adds_epi16(m10_1,*m10_128);
    m11_1 = _mm_adds_epi16(m11_1,*m11_128);

    //      print_shorts("m10_1:",&m10_1);
    //      print_shorts("m11_1:",&m11_1);
    m01_1 = _mm_max_epi16(m01_1,m00_1);
    m10_1 = _mm_max_epi16(m10_1,m11_1);
    //      print_shorts("m01_1:",&m01_1);
    //      print_shorts("m10_1:",&m10_1);

    *ext_128 = _mm_subs_epi16(m10_1,m01_1);
      
    /*
      print_shorts("ext:",ext_128);
      print_shorts("m11:",m11_128);
      print_shorts("m10:",m10_128);
      print_shorts("m10_1:",&m10_1);
      print_shorts("m01_1:",&m01_1);
      print_shorts("syst:",systematic_128);
    */

    alpha_ptr+=8;
    beta_ptr+=8;
  }
}



//int pi2[n],pi3[n+8],pi5[n+8],pi4[n+8],pi6[n+8],
int *pi2tab16[188],*pi5tab16[188],*pi4tab16[188],*pi6tab16[188];

void free_td16() {
    int ind;
    for (ind=0;ind<188;ind++) {
        free(pi2tab16[ind]);
        free(pi5tab16[ind]);
        free(pi4tab16[ind]);
        free(pi6tab16[ind]);
    }
}

void init_td16() {

  int ind,i,i2,i3,j,n,pi,pi3;
  short * base_interleaver;

  for (ind=0;ind<188;ind++) {

    n = f1f2mat[ind].nb_bits;
    base_interleaver=il_tb+f1f2mat[ind].beg_index;
#ifdef MEX
    // This is needed for the Mex implementation to make the memory persistent
    pi2tab16[ind] = mxMalloc((n+8)*sizeof(int));
    pi5tab16[ind] = mxMalloc((n+8)*sizeof(int));
    pi4tab16[ind] = mxMalloc((n+8)*sizeof(int));
    pi6tab16[ind] = mxMalloc((n+8)*sizeof(int));
#else
    pi2tab16[ind] = malloc((n+8)*sizeof(int));
    pi5tab16[ind] = malloc((n+8)*sizeof(int));
    pi4tab16[ind] = malloc((n+8)*sizeof(int));
    pi6tab16[ind] = malloc((n+8)*sizeof(int));
#endif

    for (i=i2=0;i2<8;i2++) {
      j=i2;
      for (i3=0;i3<(n>>3);i3++,i++,j+=8) {
	
	//    if (j>=n)
	//      j-=(n-1);
	
	pi2tab16[ind][i]  = j;
	//    printf("pi2[%d] = %d\n",i,j);
      }
    }
        
    for (i=0;i<n;i++) {
      pi = base_interleaver[i];//(unsigned int)threegpplte_interleaver(f1,f2,n);
      pi3 = pi2tab16[ind][pi];
      pi4tab16[ind][pi2tab16[ind][i]] = pi3;
      pi5tab16[ind][pi3] = pi2tab16[ind][i];
      pi6tab16[ind][pi] = pi2tab16[ind][i];
    } 

  }
}

unsigned char phy_threegpplte_turbo_decoder16(short *y,
					      unsigned char *decoded_bytes,
					      unsigned short n,
					      unsigned short f1,
					      unsigned short f2,
					      unsigned char max_iterations,
					      unsigned char crc_type,
					      unsigned char F,
					      time_stats_t *init_stats,
					      time_stats_t *alpha_stats,
					      time_stats_t *beta_stats,
					      time_stats_t *gamma_stats,
					      time_stats_t *ext_stats,
					      time_stats_t *intl1_stats,
					      time_stats_t *intl2_stats) {
  
  /*  y is a pointer to the input
      decoded_bytes is a pointer to the decoded output
      n is the size in bits of the coded block, with the tail */


  llr_t systematic0[n+16] __attribute__ ((aligned(16)));
  llr_t systematic1[n+16] __attribute__ ((aligned(16)));
  llr_t systematic2[n+16] __attribute__ ((aligned(16)));
  llr_t yparity1[n+16] __attribute__ ((aligned(16)));
  llr_t yparity2[n+16] __attribute__ ((aligned(16)));

  llr_t ext[n+128] __attribute__((aligned(16)));
  llr_t ext2[n+128] __attribute__((aligned(16)));

  llr_t alpha[(n+16)*8] __attribute__ ((aligned(16)));
  llr_t beta[(n+16)*8] __attribute__ ((aligned(16)));
  llr_t m11[n+16] __attribute__ ((aligned(16)));
  llr_t m10[n+16] __attribute__ ((aligned(16)));

  
  int *pi2_p,*pi4_p,*pi5_p,*pi6_p;
  llr_t *s,*s1,*s2,*yp1,*yp2,*yp;
  __m128i *yp128;
  unsigned int i,j,iind;//,pi;
  unsigned char iteration_cnt=0;
  unsigned int crc,oldcrc,crc_len;
  u8 temp;

  __m128i tmp, zeros=_mm_setzero_si128();

  register __m128i tmpe;
  int offset8_flag=0;

  if (crc_type > 3) {
    msg("Illegal crc length!\n");
    return 255;
  }


  start_meas(init_stats);



  for (iind=0;f1f2mat[iind].nb_bits!=n && iind <188; iind++);
  if ( iind == 188 ) {
    msg("Illegal frame length!\n");
    return 255;
  }
  
  switch (crc_type) {
  case CRC24_A:
  case CRC24_B:
    crc_len=3;
    break;
  case CRC16:
    crc_len=2;
    break;
  case CRC8:
    crc_len=1;
    break;
  default: 
    crc_len=3; 
  }   
  
  yp128 = (__m128i*)y;    




  s = systematic0; 
  s1 = systematic1;
  s2 = systematic2;
  yp1 = yparity1;
  yp2 = yparity2;



  for (i=0;i<n;i+=8) {
    pi2_p = &pi2tab16[iind][i];
     
    j=pi2_p[0];


    tmpe = _mm_load_si128(yp128);

    s[j]   = _mm_extract_epi16(tmpe,0); 
    yp1[j] = _mm_extract_epi16(tmpe,1); 
    yp2[j] = _mm_extract_epi16(tmpe,2);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);
    
    j=pi2_p[1];

    s[j]   = _mm_extract_epi16(tmpe,3); 
    yp1[j] = _mm_extract_epi16(tmpe,4); 
    yp2[j] = _mm_extract_epi16(tmpe,5);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[2];

    s[j]   = _mm_extract_epi16(tmpe,6); 
    yp1[j] = _mm_extract_epi16(tmpe,7); 
    tmpe = _mm_load_si128(&yp128[1]);
    yp2[j] = _mm_extract_epi16(tmpe,0);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[3];

    s[j]   = _mm_extract_epi16(tmpe,1); 
    yp1[j] = _mm_extract_epi16(tmpe,2); 
    yp2[j] = _mm_extract_epi16(tmpe,3);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[4];

    s[j]   = _mm_extract_epi16(tmpe,4); 
    yp1[j] = _mm_extract_epi16(tmpe,5); 
    yp2[j] = _mm_extract_epi16(tmpe,6);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);
    
    j=pi2_p[5];

    s[j]   = _mm_extract_epi16(tmpe,7); 
    tmpe = _mm_load_si128(&yp128[2]);
    yp1[j] = _mm_extract_epi16(tmpe,0); 
    yp2[j] = _mm_extract_epi16(tmpe,1);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);
    
    j=pi2_p[6];

    s[j]   = _mm_extract_epi16(tmpe,2); 
    yp1[j] = _mm_extract_epi16(tmpe,3); 
    yp2[j] = _mm_extract_epi16(tmpe,4);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[7];

    s[j]   = _mm_extract_epi16(tmpe,5); 
    yp1[j] = _mm_extract_epi16(tmpe,6); 
    yp2[j] = _mm_extract_epi16(tmpe,7);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    yp128+=3;

  }

  yp=(llr_t*)yp128;


  // Termination
  for (i=n;i<n+3;i++) {
    s[i]= *yp; s1[i] = s[i] ; s2[i] = s[i]; yp++;
    yp1[i] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    msg("Term 1 (%d): %d %d\n",i,s[i],yp1[i]);
#endif //DEBUG_LOGMAP
  }

  for (i=n+8;i<n+11;i++) {
    s[i]= *yp; s1[i] = s[i] ; s2[i] = s[i]; yp++;
    yp2[i-8] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    msg("Term 2 (%d): %d %d\n",i-3,s[i],yp2[i-8]); 
#endif //DEBUG_LOGMAP
  }

#ifdef DEBUG_LOGMAP
  msg("\n");
#endif //DEBUG_LOGMAP
  
  stop_meas(init_stats);

  // do log_map from first parity bit

  log_map16(systematic0,yparity1,m11,m10,alpha,beta,ext,n,0,F,offset8_flag,alpha_stats,beta_stats,gamma_stats,ext_stats);

  while (iteration_cnt++ < max_iterations) {
 
#ifdef DEBUG_LOGMAP
    printf("\n*******************ITERATION %d (n %d), ext %p\n\n",iteration_cnt,n,ext);
#endif //DEBUG_LOGMAP
 
    start_meas(intl1_stats);
 
    pi4_p=pi4tab16[iind];
    for (i=0;i<(n>>3);i++) { // steady-state portion
      
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)ext)[*pi4_p++],0);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)ext)[*pi4_p++],1);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)ext)[*pi4_p++],2);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)ext)[*pi4_p++],3);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)ext)[*pi4_p++],4);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)ext)[*pi4_p++],5);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)ext)[*pi4_p++],6);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)ext)[*pi4_p++],7);
    }

    stop_meas(intl1_stats);        

    // do log_map from second parity bit    

    log_map16(systematic2,yparity2,m11,m10,alpha,beta,ext2,n,1,F,offset8_flag,alpha_stats,beta_stats,gamma_stats,ext_stats);



    pi5_p=pi5tab16[iind];
    for (i=0;i<(n>>3);i++) {
      tmp=_mm_insert_epi16(tmp,ext2[*pi5_p++],0);
      tmp=_mm_insert_epi16(tmp,ext2[*pi5_p++],1);
      tmp=_mm_insert_epi16(tmp,ext2[*pi5_p++],2);
      tmp=_mm_insert_epi16(tmp,ext2[*pi5_p++],3);
      tmp=_mm_insert_epi16(tmp,ext2[*pi5_p++],4);
      tmp=_mm_insert_epi16(tmp,ext2[*pi5_p++],5);
      tmp=_mm_insert_epi16(tmp,ext2[*pi5_p++],6);
      tmp=_mm_insert_epi16(tmp,ext2[*pi5_p++],7);
      ((__m128i *)systematic1)[i] = _mm_adds_epi16(_mm_subs_epi16(tmp,((__m128i*)ext)[i]),((__m128i *)systematic0)[i]);
    }
    if (iteration_cnt>1) {
      start_meas(intl2_stats);
      pi6_p=pi6tab16[iind];
      for (i=0;i<(n>>3);i++) {
	tmp=_mm_insert_epi16(tmp, ((llr_t*)ext2)[*pi6_p++],7);
	tmp=_mm_insert_epi16(tmp, ((llr_t*)ext2)[*pi6_p++],6);
	tmp=_mm_insert_epi16(tmp, ((llr_t*)ext2)[*pi6_p++],5);
	tmp=_mm_insert_epi16(tmp, ((llr_t*)ext2)[*pi6_p++],4);
	tmp=_mm_insert_epi16(tmp, ((llr_t*)ext2)[*pi6_p++],3);
	tmp=_mm_insert_epi16(tmp, ((llr_t*)ext2)[*pi6_p++],2);
	tmp=_mm_insert_epi16(tmp, ((llr_t*)ext2)[*pi6_p++],1);
	tmp=_mm_insert_epi16(tmp, ((llr_t*)ext2)[*pi6_p++],0);
	tmp=_mm_cmpgt_epi8(_mm_packs_epi16(tmp,zeros),zeros);
	decoded_bytes[i]=(unsigned char)_mm_movemask_epi8(tmp);
	
      }
    }
    
    // check status on output
    if (iteration_cnt>1) {
      oldcrc= *((unsigned int *)(&decoded_bytes[(n>>3)-crc_len]));
      switch (crc_type) {
	
      case CRC24_A: 
	oldcrc&=0x00ffffff;
	crc = crc24a(&decoded_bytes[F>>3],
		     n-24-F)>>8;
	temp=((u8 *)&crc)[2];
	((u8 *)&crc)[2] = ((u8 *)&crc)[0];
	((u8 *)&crc)[0] = temp;
	break;
      case CRC24_B:
	oldcrc&=0x00ffffff;
	crc = crc24b(decoded_bytes,
		     n-24)>>8;
	temp=((u8 *)&crc)[2];
	((u8 *)&crc)[2] = ((u8 *)&crc)[0];
	((u8 *)&crc)[0] = temp;
	break;
      case CRC16:
	oldcrc&=0x0000ffff;
	crc = crc16(decoded_bytes,
		    n-16)>>16;
	break;
      case CRC8:
	oldcrc&=0x000000ff;
	crc = crc8(decoded_bytes,
		   n-8)>>24;
	break;
      default:
	printf("FATAL: 3gpplte_turbo_decoder_sse.c: Unknown CRC\n");
	exit(-1);
	break;
      }
      
      stop_meas(intl2_stats);
 
      if ((crc == oldcrc) && (crc!=0)) {
	return(iteration_cnt);
      }
    }
    // do log_map from first parity bit
    if (iteration_cnt < max_iterations) {
      log_map16(systematic1,yparity1,m11,m10,alpha,beta,ext,n,0,F,offset8_flag,alpha_stats,beta_stats,gamma_stats,ext_stats);
      __m128i* ext_128=(__m128i*) ext;
      __m128i* s1_128=(__m128i*) systematic1;
      __m128i* s0_128=(__m128i*) systematic0;

      int myloop=n>>3;
      for (i=0;i<myloop;i++) {
       *ext_128=_mm_adds_epi16(_mm_subs_epi16(*ext_128,*s1_128++),*s0_128++);
       ext_128++;
      }  
    }
  }

  _mm_empty();
  _m_empty();
  return(iteration_cnt);
}




