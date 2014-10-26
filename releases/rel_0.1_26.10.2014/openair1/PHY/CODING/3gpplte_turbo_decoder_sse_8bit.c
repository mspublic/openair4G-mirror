/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
   included in this distribution in the file called "COPYING". If not, 
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/
/* file: 3gpplte_turbo_decoder_sse.c
   purpose: Routines for implementing max-logmap decoding of Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
   authors: raymond.knopp@eurecom.fr, Laurent Thomas (Alcatel-Lucent)
   date: 21.10.2009 

   Note: This routine currently requires SSE2,SSSE3 and SSE4.1 equipped computers.  It uses 16-bit inputs for LLRs and 8-bit arithmetic for internal computations!
  
   Changelog: 17.11.2009 FK SSE4.1 not required anymore
   Aug. 2012 new parallelization options for higher speed (8-way parallelization)
   Jan. 2013 8-bit LLR support with 16-way parallelization
   Feb. 2013 New interleaving and hard-decision optimizations (L. Thomas)
   May 2013 Extracted 8-bit code
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

#define SHUFFLE16(a,b,c,d,e,f,g,h) _mm_set_epi8(h==-1?-1:h*2+1,	\
						h==-1?-1:h*2,	\
						g==-1?-1:g*2+1,	\
						g==-1?-1:g*2,	\
						f==-1?-1:f*2+1,	\
						f==-1?-1:f*2,	\
						e==-1?-1:e*2+1,	\
						e==-1?-1:e*2,	\
						d==-1?-1:d*2+1,	\
						d==-1?-1:d*2,	\
						c==-1?-1:c*2+1,	\
						c==-1?-1:c*2,	\
						b==-1?-1:b*2+1,	\
						b==-1?-1:b*2,	\
						a==-1?-1:a*2+1,	\
						a==-1?-1:a*2);





//#define DEBUG_LOGMAP

 

typedef int8_t llr_t; // internal decoder LLR data is 8-bit fixed
typedef int8_t channel_t;
#define MAX8 127


void log_map8(llr_t* systematic,channel_t* y_parity, llr_t* m11, llr_t* m10, llr_t *alpha, llr_t *beta, llr_t* ext,unsigned short frame_length,unsigned char term_flag,unsigned char F,int offset8_flag,time_stats_t *alpha_stats,time_stats_t *beta_stats,time_stats_t *gamma_stats,time_stats_t *ext_stats);
void compute_gamma8(llr_t* m11,llr_t* m10,llr_t* systematic, channel_t* y_parity, unsigned short frame_length,unsigned char term_flag);
void compute_alpha8(llr_t*alpha,llr_t *beta, llr_t* m11,llr_t* m10, unsigned short frame_length,unsigned char F);
void compute_beta8(llr_t*alpha, llr_t* beta,llr_t* m11,llr_t* m10, unsigned short frame_length,unsigned char F,int offset8_flag);
void compute_ext8(llr_t* alpha,llr_t* beta,llr_t* m11,llr_t* m10,llr_t* extrinsic, llr_t* ap, unsigned short frame_length);


void print_bytes(char *s, __m128i *x) {

  int8_t *tempb = (int8_t *)x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",s,
	 tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7],
	 tempb[8],tempb[9],tempb[10],tempb[11],tempb[12],tempb[13],tempb[14],tempb[15]);

}


void log_map8(llr_t* systematic,
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

  start_meas(gamma_stats) ; compute_gamma8(m11,m10,systematic,y_parity,frame_length,term_flag) ; stop_meas(gamma_stats);
  start_meas(alpha_stats) ; compute_alpha8(alpha,beta,m11,m10,frame_length,F)                  ; stop_meas(alpha_stats);
  start_meas(beta_stats)  ; compute_beta8(alpha,beta,m11,m10,frame_length,F,offset8_flag)      ; stop_meas(beta_stats);
  start_meas(ext_stats)   ; compute_ext8(alpha,beta,m11,m10,ext,systematic,frame_length)       ; stop_meas(ext_stats);


}

void compute_gamma8(llr_t* m11,llr_t* m10,llr_t* systematic,channel_t* y_parity,
		   unsigned short frame_length,unsigned char term_flag)
{
  int k,K1;
  __m128i *systematic128 = (__m128i *)systematic;
  __m128i *y_parity128   = (__m128i *)y_parity;
  __m128i *m10_128        = (__m128i *)m10;
  __m128i *m11_128        = (__m128i *)m11;

#ifdef DEBUG_LOGMAP
  msg("compute_gamma, %p,%p,%p,%p,framelength %d\n",m11,m10,systematic,y_parity,frame_length);
#endif

  register __m128i sl,sh,ypl,yph; //K128=_mm_set1_epi8(-128);
  K1 = (frame_length>>4);  
  for (k=0;k<K1;k++) {  
    sl  = _mm_cvtepi8_epi16(systematic128[k]);   sh = _mm_cvtepi8_epi16(_mm_srli_si128(systematic128[k],8));
    ypl = _mm_cvtepi8_epi16(y_parity128[k]);    yph = _mm_cvtepi8_epi16(_mm_srli_si128(y_parity128[k],8));
    m11_128[k] = _mm_packs_epi16(_mm_srai_epi16(_mm_adds_epi16(sl,ypl),1),
				 _mm_srai_epi16(_mm_adds_epi16(sh,yph),1));
    m10_128[k] = _mm_packs_epi16(_mm_srai_epi16(_mm_subs_epi16(sl,ypl),1),
				 _mm_srai_epi16(_mm_subs_epi16(sh,yph),1));

  }
  // Termination

    sl  = _mm_cvtepi8_epi16(systematic128[k+term_flag]);   sh = _mm_cvtepi8_epi16(_mm_srli_si128(systematic128[k],8));
    ypl = _mm_cvtepi8_epi16(y_parity128[k+term_flag]);    yph = _mm_cvtepi8_epi16(_mm_srli_si128(y_parity128[k],8));
    m11_128[k] = _mm_packs_epi16(_mm_srai_epi16(_mm_adds_epi16(sl,ypl),1),
				 _mm_srai_epi16(_mm_adds_epi16(sh,yph),1));
    m10_128[k] = _mm_packs_epi16(_mm_srai_epi16(_mm_subs_epi16(sl,ypl),1),
				 _mm_srai_epi16(_mm_subs_epi16(sh,yph),1));


}

#define L 16

void compute_alpha8(llr_t* alpha,llr_t* beta,llr_t* m_11,llr_t* m_10,unsigned short frame_length,unsigned char F) {
  int k,loopval,rerun_flag;
  __m128i *alpha128=(__m128i *)alpha,*alpha_ptr;
  __m128i *m11p,*m10p;
  __m128i m_b0,m_b1,m_b2,m_b3,m_b4,m_b5,m_b6,m_b7;
  __m128i new0,new1,new2,new3,new4,new5,new6,new7;
  __m128i alpha_max;

  // Set initial state: first colum is known
  // the other columns are unknown, so all states are set to same value
  alpha128[0] = _mm_set_epi8(-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,0);
  alpha128[1] = _mm_set_epi8(-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2);
  alpha128[2] = _mm_set_epi8(-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2);
  alpha128[3] = _mm_set_epi8(-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2);
  alpha128[4] = _mm_set_epi8(-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2);
  alpha128[5] = _mm_set_epi8(-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2);
  alpha128[6] = _mm_set_epi8(-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2);
  alpha128[7] = _mm_set_epi8(-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2,-MAX8/2);  

  for (loopval=frame_length>>4, rerun_flag=0; rerun_flag<2; loopval=L, rerun_flag++) {
     
    alpha_ptr = &alpha128[0];
    m11p = (__m128i*)m_11;
    m10p = (__m128i*)m_10;
    for (k=0;	 k<loopval;	 k++){
      m_b0 = _mm_adds_epi8(alpha_ptr[1],*m11p);  // m11
      m_b4 = _mm_subs_epi8(alpha_ptr[1],*m11p);  // m00=-m11    
      m_b1 = _mm_subs_epi8(alpha_ptr[3],*m10p);  // m01=-m10
      m_b5 = _mm_adds_epi8(alpha_ptr[3],*m10p);  // m10
      m_b2 = _mm_adds_epi8(alpha_ptr[5],*m10p);  // m10
      m_b6 = _mm_subs_epi8(alpha_ptr[5],*m10p);  // m01=-m10
      m_b3 = _mm_subs_epi8(alpha_ptr[7],*m11p);  // m00=-m11
      m_b7 = _mm_adds_epi8(alpha_ptr[7],*m11p);  // m11 
      
      new0 = _mm_subs_epi8(alpha_ptr[0],*m11p);  // m00=-m11
      new4 = _mm_adds_epi8(alpha_ptr[0],*m11p);  // m11
      new1 = _mm_adds_epi8(alpha_ptr[2],*m10p);  // m10
      new5 = _mm_subs_epi8(alpha_ptr[2],*m10p);  // m01=-m10
      new2 = _mm_subs_epi8(alpha_ptr[4],*m10p);  // m01=-m10
      new6 = _mm_adds_epi8(alpha_ptr[4],*m10p);  // m10
      new3 = _mm_adds_epi8(alpha_ptr[6],*m11p);  // m11
      new7 = _mm_subs_epi8(alpha_ptr[6],*m11p);  // m00=-m11
    
      alpha_ptr += 8;
      m11p++;
      m10p++;
      alpha_ptr[0] = _mm_max_epi8(m_b0,new0);
      alpha_ptr[1] = _mm_max_epi8(m_b1,new1);
      alpha_ptr[2] = _mm_max_epi8(m_b2,new2);
      alpha_ptr[3] = _mm_max_epi8(m_b3,new3);
      alpha_ptr[4] = _mm_max_epi8(m_b4,new4);
      alpha_ptr[5] = _mm_max_epi8(m_b5,new5);
      alpha_ptr[6] = _mm_max_epi8(m_b6,new6);
      alpha_ptr[7] = _mm_max_epi8(m_b7,new7);
      
      // compute and subtract maxima
      alpha_max = _mm_max_epi8(alpha_ptr[0],alpha_ptr[1]);
      alpha_max = _mm_max_epi8(alpha_max,alpha_ptr[2]);
      alpha_max = _mm_max_epi8(alpha_max,alpha_ptr[3]);
      alpha_max = _mm_max_epi8(alpha_max,alpha_ptr[4]);
      alpha_max = _mm_max_epi8(alpha_max,alpha_ptr[5]);
      alpha_max = _mm_max_epi8(alpha_max,alpha_ptr[6]);
      alpha_max = _mm_max_epi8(alpha_max,alpha_ptr[7]);

      alpha_ptr[0] = _mm_subs_epi8(alpha_ptr[0],alpha_max);
      alpha_ptr[1] = _mm_subs_epi8(alpha_ptr[1],alpha_max);
      alpha_ptr[2] = _mm_subs_epi8(alpha_ptr[2],alpha_max);
      alpha_ptr[3] = _mm_subs_epi8(alpha_ptr[3],alpha_max);
      alpha_ptr[4] = _mm_subs_epi8(alpha_ptr[4],alpha_max);
      alpha_ptr[5] = _mm_subs_epi8(alpha_ptr[5],alpha_max);
      alpha_ptr[6] = _mm_subs_epi8(alpha_ptr[6],alpha_max);
      alpha_ptr[7] = _mm_subs_epi8(alpha_ptr[7],alpha_max);
    }

    // Set intial state for next iteration from the last state 
    // as acolum end states are the first states of the next column
    int K1= frame_length>>1;
    alpha128[0] = _mm_slli_si128(alpha128[K1],1);
    alpha128[1] = _mm_slli_si128(alpha128[1+K1],1);
    alpha128[2] = _mm_slli_si128(alpha128[2+K1],1);
    alpha128[3] = _mm_slli_si128(alpha128[3+K1],1);
    alpha128[4] = _mm_slli_si128(alpha128[4+K1],1);
    alpha128[5] = _mm_slli_si128(alpha128[5+K1],1);
    alpha128[6] = _mm_slli_si128(alpha128[6+K1],1);
    alpha128[7] = _mm_slli_si128(alpha128[7+K1],1);
    alpha[16] =  -MAX8/2;
    alpha[32] = -MAX8/2;
    alpha[48] = -MAX8/2;
    alpha[64] = -MAX8/2;
    alpha[80] = -MAX8/2;
    alpha[96] = -MAX8/2;
    alpha[112] = -MAX8/2;

  }  
  
}


void compute_beta8(llr_t* alpha,llr_t* beta,llr_t *m_11,llr_t* m_10,unsigned short frame_length,unsigned char F,int offset8_flag) {
  
  int k,rerun_flag, loopval;
  __m128i m11_128,m10_128;
  __m128i m_b0,m_b1,m_b2,m_b3,m_b4,m_b5,m_b6,m_b7;
  __m128i new0,new1,new2,new3,new4,new5,new6,new7;

  __m128i *beta128,*alpha128,*beta_ptr;
  __m128i beta_max; 
  int16_t m11,m10,beta0_16,beta1_16,beta2_16,beta3_16,beta4_16,beta5_16,beta6_16,beta7_16,beta0_2,beta1_2,beta2_2,beta3_2,beta_m; 
  llr_t beta0,beta1;

  llr_t beta2,beta3,beta4,beta5,beta6,beta7;
  __m128i beta_16;

#if 0
  // termination for beta initialization

  m11=(int16_t)m_11[2+frame_length];
  m10=(int16_t)m_10[2+frame_length];

  beta0 = -m11;//M0T_TERM;
  beta1 = m11;//M1T_TERM;
  m11=(int16_t)m_11[1+frame_length];
  m10=(int16_t)m_10[1+frame_length];

  beta0_2 = beta0-m11;//+M0T_TERM;
  beta1_2 = beta0+m11;//+M1T_TERM;
  beta2_2 = beta1+m10;//M2T_TERM;
  beta3_2 = beta1-m10;//+M3T_TERM;
  m11=(int16_t)m_11[frame_length];
  m10=(int16_t)m_10[frame_length];

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

  beta_16 = _mm_set_epi16(beta7_16,beta6_16,beta5_16,beta4_16,beta3_16,beta2_16,beta1_16,beta0_16);
  beta_16 = _mm_packs_epi16(beta_16,beta_16);
  beta0 = _mm_extract_epi8(beta_16,0);
  beta1 = _mm_extract_epi8(beta_16,1);
  beta2 = _mm_extract_epi8(beta_16,2);
  beta3 = _mm_extract_epi8(beta_16,3);
  beta4 = _mm_extract_epi8(beta_16,4);
  beta5 = _mm_extract_epi8(beta_16,5);
  beta6 = _mm_extract_epi8(beta_16,6);
  beta7 = _mm_extract_epi8(beta_16,7);

#endif

  // we are supposed to run compute_alpha just before compute_beta
  // so the initial states of backward computation can be set from last value of alpha states (forward computation)
  beta_ptr   = (__m128i*)&beta[frame_length<<3];
  alpha128   = (__m128i*)&alpha[0];
  beta_ptr[0] = alpha128[(frame_length>>1)];
  beta_ptr[1] = alpha128[1+(frame_length>>1)];
  beta_ptr[2] = alpha128[2+(frame_length>>1)];
  beta_ptr[3] = alpha128[3+(frame_length>>1)];
  beta_ptr[4] = alpha128[4+(frame_length>>1)];
  beta_ptr[5] = alpha128[5+(frame_length>>1)];
  beta_ptr[6] = alpha128[6+(frame_length>>1)];
  beta_ptr[7] = alpha128[7+(frame_length>>1)];

  for (rerun_flag=0, loopval=0; 
       rerun_flag<2 ; 
       loopval=(frame_length>>4)-L,rerun_flag++) {

    if (offset8_flag==0) {
      beta_ptr[0] = _mm_insert_epi8(beta_ptr[0],beta0,15);
      beta_ptr[1] = _mm_insert_epi8(beta_ptr[1],beta1,15);
      beta_ptr[2] = _mm_insert_epi8(beta_ptr[2],beta2,15);
      beta_ptr[3] = _mm_insert_epi8(beta_ptr[3],beta3,15);
      beta_ptr[4] = _mm_insert_epi8(beta_ptr[4],beta4,15);
      beta_ptr[5] = _mm_insert_epi8(beta_ptr[5],beta5,15);
      beta_ptr[6] = _mm_insert_epi8(beta_ptr[6],beta6,15);
      beta_ptr[7] = _mm_insert_epi8(beta_ptr[7],beta7,15);
    }

    for (k=(frame_length>>4)-1, beta_ptr = (__m128i*)&beta[frame_length<<3] ;
	 k>=loopval;
	 k--){
      
      m11_128=((__m128i*)m_11)[k];  
      m10_128=((__m128i*)m_10)[k];
      m_b0 = _mm_adds_epi8(beta_ptr[4],m11_128);  //m11
      m_b1 = _mm_subs_epi8(beta_ptr[4],m11_128);  //m00
      m_b2 = _mm_subs_epi8(beta_ptr[5],m10_128);  //m01
      m_b3 = _mm_adds_epi8(beta_ptr[5],m10_128);  //m10
      m_b4 = _mm_adds_epi8(beta_ptr[6],m10_128);  //m10   
      m_b5 = _mm_subs_epi8(beta_ptr[6],m10_128);  //m01
      m_b6 = _mm_subs_epi8(beta_ptr[7],m11_128);  //m00
      m_b7 = _mm_adds_epi8(beta_ptr[7],m11_128);  //m11
      
      new0 = _mm_subs_epi8(beta_ptr[0],m11_128);  //m00
      new1 = _mm_adds_epi8(beta_ptr[0],m11_128);  //m11
      new2 = _mm_adds_epi8(beta_ptr[1],m10_128);  //m10
      new3 = _mm_subs_epi8(beta_ptr[1],m10_128);  //m01
      new4 = _mm_subs_epi8(beta_ptr[2],m10_128);  //m01
      new5 = _mm_adds_epi8(beta_ptr[2],m10_128);  //m10
      new6 = _mm_adds_epi8(beta_ptr[3],m11_128);  //m11
      new7 = _mm_subs_epi8(beta_ptr[3],m11_128);  //m00
      
      beta_ptr-=8;
      
      beta_ptr[0] = _mm_max_epi8(m_b0,new0);
      beta_ptr[1] = _mm_max_epi8(m_b1,new1);
      beta_ptr[2] = _mm_max_epi8(m_b2,new2);
      beta_ptr[3] = _mm_max_epi8(m_b3,new3);
      beta_ptr[4] = _mm_max_epi8(m_b4,new4);
      beta_ptr[5] = _mm_max_epi8(m_b5,new5);
      beta_ptr[6] = _mm_max_epi8(m_b6,new6);
      beta_ptr[7] = _mm_max_epi8(m_b7,new7);
      
      beta_max = _mm_max_epi8(beta_ptr[0],beta_ptr[1]);
      beta_max = _mm_max_epi8(beta_max   ,beta_ptr[2]);
      beta_max = _mm_max_epi8(beta_max   ,beta_ptr[3]);
      beta_max = _mm_max_epi8(beta_max   ,beta_ptr[4]);
      beta_max = _mm_max_epi8(beta_max   ,beta_ptr[5]);
      beta_max = _mm_max_epi8(beta_max   ,beta_ptr[6]);
      beta_max = _mm_max_epi8(beta_max   ,beta_ptr[7]);

      beta_ptr[0] = _mm_subs_epi8(beta_ptr[0],beta_max);
      beta_ptr[1] = _mm_subs_epi8(beta_ptr[1],beta_max);
      beta_ptr[2] = _mm_subs_epi8(beta_ptr[2],beta_max);
      beta_ptr[3] = _mm_subs_epi8(beta_ptr[3],beta_max);
      beta_ptr[4] = _mm_subs_epi8(beta_ptr[4],beta_max);
      beta_ptr[5] = _mm_subs_epi8(beta_ptr[5],beta_max);
      beta_ptr[6] = _mm_subs_epi8(beta_ptr[6],beta_max);
      beta_ptr[7] = _mm_subs_epi8(beta_ptr[7],beta_max);

    }     
    // Set intial state for next iteration from the last state 
    // as column last states are the first states of the next column
    // The initial state of colum 0 is coming from tail bits (to be computed)
    beta128 = (__m128i*)&beta[0];
    beta_ptr   = (__m128i*)&beta[frame_length<<3];
    beta_ptr[0] = _mm_srli_si128(beta128[0],1);
    beta_ptr[1] = _mm_srli_si128(beta128[1],1);
    beta_ptr[2] = _mm_srli_si128(beta128[2],1);
    beta_ptr[3] = _mm_srli_si128(beta128[3],1);
    beta_ptr[4] = _mm_srli_si128(beta128[4],1);
    beta_ptr[5] = _mm_srli_si128(beta128[5],1);
    beta_ptr[6] = _mm_srli_si128(beta128[6],1);
    beta_ptr[7] = _mm_srli_si128(beta128[7],1);
  }
}

void compute_ext8(llr_t* alpha,llr_t* beta,llr_t* m_11,llr_t* m_10,llr_t* ext, llr_t* systematic,unsigned short frame_length) {

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


  for (k=0;k<(frame_length>>4);k++){

    m11_128        = (__m128i*)&m_11[k<<4];
    m10_128        = (__m128i*)&m_10[k<<4];
    ext_128        = (__m128i*)&ext[k<<4];

    m00_4 = _mm_adds_epi8(alpha_ptr[7],beta_ptr[3]); //ALPHA_BETA_4m00;
    m11_4 = _mm_adds_epi8(alpha_ptr[7],beta_ptr[7]); //ALPHA_BETA_4m11;
    m00_3 = _mm_adds_epi8(alpha_ptr[6],beta_ptr[7]); //ALPHA_BETA_3m00;
    m11_3 = _mm_adds_epi8(alpha_ptr[6],beta_ptr[3]); //ALPHA_BETA_3m11;
    m00_2 = _mm_adds_epi8(alpha_ptr[1],beta_ptr[4]); //ALPHA_BETA_2m00;
    m11_2 = _mm_adds_epi8(alpha_ptr[1],beta_ptr[0]); //ALPHA_BETA_2m11;
    m11_1 = _mm_adds_epi8(alpha_ptr[0],beta_ptr[4]); //ALPHA_BETA_1m11;
    m00_1 = _mm_adds_epi8(alpha_ptr[0],beta_ptr[0]); //ALPHA_BETA_1m00;
    m01_4 = _mm_adds_epi8(alpha_ptr[5],beta_ptr[6]); //ALPHA_BETA_4m01;
    m10_4 = _mm_adds_epi8(alpha_ptr[5],beta_ptr[2]); //ALPHA_BETA_4m10;
    m01_3 = _mm_adds_epi8(alpha_ptr[4],beta_ptr[2]); //ALPHA_BETA_3m01;
    m10_3 = _mm_adds_epi8(alpha_ptr[4],beta_ptr[6]); //ALPHA_BETA_3m10;
    m01_2 = _mm_adds_epi8(alpha_ptr[3],beta_ptr[1]); //ALPHA_BETA_2m01;
    m10_2 = _mm_adds_epi8(alpha_ptr[3],beta_ptr[5]); //ALPHA_BETA_2m10;
    m10_1 = _mm_adds_epi8(alpha_ptr[2],beta_ptr[1]); //ALPHA_BETA_1m10;
    m01_1 = _mm_adds_epi8(alpha_ptr[2],beta_ptr[5]); //ALPHA_BETA_1m01;

    m01_1 = _mm_max_epi8(m01_1,m01_2);
    m01_1 = _mm_max_epi8(m01_1,m01_3);
    m01_1 = _mm_max_epi8(m01_1,m01_4);
    m00_1 = _mm_max_epi8(m00_1,m00_2);
    m00_1 = _mm_max_epi8(m00_1,m00_3);
    m00_1 = _mm_max_epi8(m00_1,m00_4);
    m10_1 = _mm_max_epi8(m10_1,m10_2);
    m10_1 = _mm_max_epi8(m10_1,m10_3);
    m10_1 = _mm_max_epi8(m10_1,m10_4);
    m11_1 = _mm_max_epi8(m11_1,m11_2);
    m11_1 = _mm_max_epi8(m11_1,m11_3);
    m11_1 = _mm_max_epi8(m11_1,m11_4);

      
    m01_1 = _mm_subs_epi8(m01_1,*m10_128);
    m00_1 = _mm_subs_epi8(m00_1,*m11_128);
    m10_1 = _mm_adds_epi8(m10_1,*m10_128);
    m11_1 = _mm_adds_epi8(m11_1,*m11_128);


    m01_1 = _mm_max_epi8(m01_1,m00_1);
    m10_1 = _mm_max_epi8(m10_1,m11_1);


    *ext_128 = _mm_subs_epi8(m10_1,m01_1);
      
    alpha_ptr+=8;
    beta_ptr+=8;
  }


}



//int pi2[n],pi3[n+8],pi5[n+8],pi4[n+8],pi6[n+8],
int *pi2tab8[188],*pi5tab8[188],*pi4tab8[188],*pi6tab8[188];

void free_td8(void) {
    int ind;
    for (ind=0;ind<188;ind++) {
        free(pi2tab8[ind]);
        free(pi5tab8[ind]);
        free(pi4tab8[ind]);
        free(pi6tab8[ind]);
    }
}

void init_td8() {

  int ind,i,j,n,n2,pi,pi3;
  short * base_interleaver;

  for (ind=0;ind<188;ind++) {

    n = f1f2mat[ind].nb_bits;
    base_interleaver=il_tb+f1f2mat[ind].beg_index;
#ifdef MEX
    // This is needed for the Mex implementation to make the memory persistent
    pi2tab8[ind] = mxMalloc((n+8)*sizeof(int));
    pi5tab8[ind] = mxMalloc((n+8)*sizeof(int));
    pi4tab8[ind] = mxMalloc((n+8)*sizeof(int));
    pi6tab8[ind] = mxMalloc((n+8)*sizeof(int));
#else
    pi2tab8[ind] = malloc((n+8)*sizeof(int));
    pi5tab8[ind] = malloc((n+8)*sizeof(int));
    pi4tab8[ind] = malloc((n+8)*sizeof(int));
    pi6tab8[ind] = malloc((n+8)*sizeof(int));
#endif

    if ((n&15)>0) {
      n2 = n+8;
    }
    else 
      n2 = n;

    for (j=0,i=0;i<n2;i++,j+=16) {
      
      if (j>=n2)
	j-=(n2-1);
      
      pi2tab8[ind][i] = j;
      //    printf("pi2[%d] = %d\n",i,j);
    }
        
    for (i=0;i<n2;i++) {
      pi = base_interleaver[i];//(unsigned int)threegpplte_interleaver(f1,f2,n);
      pi3 = pi2tab8[ind][pi];
      pi4tab8[ind][pi2tab8[ind][i]] = pi3;
      pi5tab8[ind][pi3] = pi2tab8[ind][i];
      pi6tab8[ind][pi] = pi2tab8[ind][i];
    } 

  }
}

unsigned char phy_threegpplte_turbo_decoder8(short *y,
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

  int n2;

  llr_t y8[3*(n+16)] __attribute__((aligned(16)));


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
  uint8_t temp;
  __m128i tmp128[(n+8)>>3];
  __m128i tmp, zeros=_mm_setzero_si128();


  int offset8_flag=0;

  if (crc_type > 3) {
    msg("Illegal crc length!\n");
    return 255;
  }


  start_meas(init_stats);


  if ((n&15)>0) {
    n2 = n+8;
    offset8_flag=1;
  }
  else 
    n2 = n;


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
  
  __m128i avg=_mm_set1_epi32(0);
  for (i=0;i<(3*(n>>4))+1;i++) {
    __m128i tmp=_mm_abs_epi16(_mm_unpackhi_epi16(((__m128i*)y)[i],((__m128i*)y)[i]));
    avg=_mm_add_epi32(_mm_cvtepi16_epi32(_mm_abs_epi16(((__m128i*)y)[i])),avg);
    avg=_mm_add_epi32(_mm_cvtepi16_epi32(tmp),avg);
  }
  int round_avg=(_mm_extract_epi32(avg,0)+_mm_extract_epi32(avg,1)+_mm_extract_epi32(avg,2)+_mm_extract_epi32(avg,3))/(n*3);

   //printf("avg input turbo: %d sum %d taille bloc %d\n",round_avg,round_sum,n);

  if (round_avg < 16 ) 
    for (i=0,j=0;i<(3*(n2>>4))+1;i++,j+=2) 
      ((__m128i *)y8)[i] = _mm_packs_epi16(((__m128i *)y)[j],((__m128i *)y)[j+1]);
  else if (round_avg < 32) 
    for (i=0,j=0;i<(3*(n2>>4))+1;i++,j+=2) 
      ((__m128i *)y8)[i] = _mm_packs_epi16(_mm_srai_epi16(((__m128i *)y)[j],1),_mm_srai_epi16(((__m128i *)y)[j+1],1));
  else if (round_avg < 64 )
    for (i=0,j=0;i<(3*(n2>>4))+1;i++,j+=2) 
      ((__m128i *)y8)[i] = _mm_packs_epi16(_mm_srai_epi16(((__m128i *)y)[j],2),_mm_srai_epi16(((__m128i *)y)[j+1],2));
  else
    for (i=0,j=0;i<(3*(n2>>4))+1;i++,j+=2) 
      ((__m128i *)y8)[i] = _mm_packs_epi16(_mm_srai_epi16(((__m128i *)y)[j],3),_mm_srai_epi16(((__m128i *)y)[j+1],3));

  yp128 = (__m128i*)y8; 


  s = systematic0; 
  s1 = systematic1;
  s2 = systematic2;
  yp1 = yparity1;
  yp2 = yparity2;
  yp=y8;
#if 1
  for (i=0; i<16 ; i++ )
    for (j=0;j<n2;j+=16) {
      int k=i+j;
      s[k]=*yp++;
      yp1[k]=*yp++;
      yp2[k]=*yp++;
  }
#endif
#if 0
  for (i=0;i<n2;i+=16) {
   pi2_p = &pi2tab8[iind][i];
    
    j=pi2_p[0];
    s[j]   = _mm_extract_epi8(yp128[0],0); 
    yp1[j] = _mm_extract_epi8(yp128[0],1); 
    yp2[j] = _mm_extract_epi8(yp128[0],2);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[1];
    s[j]   = _mm_extract_epi8(yp128[0],3); 
    yp1[j] = _mm_extract_epi8(yp128[0],4); 
    yp2[j] = _mm_extract_epi8(yp128[0],5);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[2];
    s[j]   = _mm_extract_epi8(yp128[0],6); 
    yp1[j] = _mm_extract_epi8(yp128[0],7); 
    yp2[j] = _mm_extract_epi8(yp128[0],8);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[3];
    s[j]   = _mm_extract_epi8(yp128[0],9); 
    yp1[j] = _mm_extract_epi8(yp128[0],10); 
    yp2[j] = _mm_extract_epi8(yp128[0],11);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[4];
    s[j]   = _mm_extract_epi8(yp128[0],12); 
    yp1[j] = _mm_extract_epi8(yp128[0],13); 
    yp2[j] = _mm_extract_epi8(yp128[0],14);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[5];
    s[j]   = _mm_extract_epi8(yp128[0],15); 
    yp1[j] = _mm_extract_epi8(yp128[1],0); 
    yp2[j] = _mm_extract_epi8(yp128[1],1);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[6];
    s[j]   = _mm_extract_epi8(yp128[1],2); 
    yp1[j] = _mm_extract_epi8(yp128[1],3); 
    yp2[j] = _mm_extract_epi8(yp128[1],4);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[7];
    s[j]   = _mm_extract_epi8(yp128[1],5); 
    yp1[j] = _mm_extract_epi8(yp128[1],6); 
    yp2[j] = _mm_extract_epi8(yp128[1],7);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[8];
    s[j]   = _mm_extract_epi8(yp128[1],8); 
    yp1[j] = _mm_extract_epi8(yp128[1],9); 
    yp2[j] = _mm_extract_epi8(yp128[1],10);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[9];
    s[j]   = _mm_extract_epi8(yp128[1],11); 
    yp1[j] = _mm_extract_epi8(yp128[1],12); 
    yp2[j] = _mm_extract_epi8(yp128[1],13);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[10];
    s[j]   = _mm_extract_epi8(yp128[1],14); 
    yp1[j] = _mm_extract_epi8(yp128[1],15); 
    yp2[j] = _mm_extract_epi8(yp128[2],0);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[11];
    s[j]   = _mm_extract_epi8(yp128[2],1); 
    yp1[j] = _mm_extract_epi8(yp128[2],2); 
    yp2[j] = _mm_extract_epi8(yp128[2],3);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[12];
    s[j]   = _mm_extract_epi8(yp128[2],4); 
    yp1[j] = _mm_extract_epi8(yp128[2],5); 
    yp2[j] = _mm_extract_epi8(yp128[2],6);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[13];
    s[j]   = _mm_extract_epi8(yp128[2],7); 
    yp1[j] = _mm_extract_epi8(yp128[2],8); 
    yp2[j] = _mm_extract_epi8(yp128[2],9);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[14];
    s[j]   = _mm_extract_epi8(yp128[2],10); 
    yp1[j] = _mm_extract_epi8(yp128[2],11); 
    yp2[j] = _mm_extract_epi8(yp128[2],12);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

    j=pi2_p[15];
    s[j]   = _mm_extract_epi8(yp128[2],13); 
    yp1[j] = _mm_extract_epi8(yp128[2],14); 
    yp2[j] = _mm_extract_epi8(yp128[2],15);
    //    printf("init: j %d, s[j] %d yp1[j] %d yp2[j] %d\n",j,s[j],yp1[j],yp2[j]);

     yp128+=3;

  }
#endif
  
  yp=(llr_t*)yp128;

  if (n2>n) {
    /*
    s[n]=0;s[n+1]=0;s[n+2]=0;s[n+3]=0;
    s[n+4]=0;s[n+5]=0;s[n+6]=0;s[n+7]=0;
    s1[n]=0;s1[n+1]=0;s1[n+2]=0;s1[n+3]=0;
    s1[n+4]=0;s1[n+5]=0;s1[n+6]=0;s1[n+7]=0;
    s2[n]=0;s2[n+1]=0;s2[n+2]=0;s2[n+3]=0;
    s2[n+4]=0;s2[n+5]=0;s2[n+6]=0;s2[n+7]=0;*/
    yp=(llr_t*)(y8+n);
  }

  //  printf("n=%d,n2=%d\n",n,n2);

  // Termination
  for (i=n2;i<n2+3;i++) {
    s[i]= *yp; s1[i] = s[i] ; s2[i] = s[i]; yp++;
    yp1[i] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    msg("Term 1 (%d): %d %d\n",i,s[i],yp1[i]);
#endif //DEBUG_LOGMAP
  }

  for (i=n2+16;i<n2+19;i++) {
    s[i]= *yp; s1[i] = s[i] ; s2[i] = s[i]; yp++;
    yp2[i-16] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    msg("Term 2 (%d): %d %d\n",i-16,s[i],yp2[i-16]);
#endif //DEBUG_LOGMAP
  }

#ifdef DEBUG_LOGMAP
  msg("\n");
#endif //DEBUG_LOGMAP
  
  stop_meas(init_stats);

  // do log_map from first parity bit

  log_map8(systematic0,yparity1,m11,m10,alpha,beta,ext,n2,0,F,offset8_flag,alpha_stats,beta_stats,gamma_stats,ext_stats);

  while (iteration_cnt++ < max_iterations) {
 
#ifdef DEBUG_LOGMAP
    printf("\n*******************ITERATION %d (n %d), ext %p\n\n",iteration_cnt,n,ext);
#endif //DEBUG_LOGMAP
 
    start_meas(intl1_stats);
    pi4_p=pi4tab8[iind];
    for (i=0;i<(n2>>4);i++) { // steady-state portion      
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],0);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],1);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],2);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],3);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],4);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],5);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],6);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],7);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],8);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],9);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],10);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],11);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],12);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],13);
      tmp=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],14);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(tmp,((llr_t*)ext)[*pi4_p++],15);
    }
    stop_meas(intl1_stats);        

    // do log_map from second parity bit    

    log_map8(systematic2,yparity2,m11,m10,alpha,beta,ext2,n2,1,F,offset8_flag,alpha_stats,beta_stats,gamma_stats,ext_stats);



    pi5_p=pi5tab8[iind];
    uint16_t decoded_bytes_interl[6144/16] __attribute__((aligned(16)));
    if ((n2&0x7f) == 0) {  // n2 is a multiple of 128 bits
      for (i=0;i<(n2>>4);i++) {
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],0);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],1);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],2);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],3);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],4);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],5);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],6);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],7);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],8);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],9);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],10);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],11);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],12);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],13);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],14);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],15);
	decoded_bytes_interl[i]=(uint16_t) _mm_movemask_epi8(_mm_cmpgt_epi8(tmp,zeros));
	((__m128i *)systematic1)[i] = _mm_adds_epi8(_mm_subs_epi8(tmp,((__m128i*)ext)[i]),((__m128i *)systematic0)[i]);
      }
    }
    else {
      for (i=0;i<(n2>>4);i++) {
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],0);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],1);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],2);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],3);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],4);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],5);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],6);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],7);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],8);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],9);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],10);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],11);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],12);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],13);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],14);
	tmp=_mm_insert_epi8(tmp,ext2[*pi5_p++],15);
	tmp128[i] = _mm_adds_epi8(((__m128i *)ext2)[i],((__m128i *)systematic2)[i]);
	
	((__m128i *)systematic1)[i] = _mm_adds_epi8(_mm_subs_epi8(tmp,((__m128i*)ext)[i]),((__m128i *)systematic0)[i]);
      }
    }
    // Check if we decoded the block
    if (iteration_cnt>1) {
      start_meas(intl2_stats);

      if ((n2&0x7f) == 0) {  // n2 is a multiple of 128 bits

	// re-order the decoded bits in theregular order 
	// as it is presently ordered as 16 sequential columns
	__m128i* dbytes=(__m128i*)decoded_bytes_interl;
	__m128i shuffle=SHUFFLE16(7,6,5,4,3,2,1,0);
	__m128i mask  __attribute__((aligned(16)));
	int n_128=n2>>7;
	for (i=0;i<n_128;i++) {
	  mask=_mm_set1_epi16(1);
	  __m128i tmp __attribute__((aligned(16)));
	  tmp=_mm_shuffle_epi8(dbytes[i],shuffle);
	  __m128i tmp2 __attribute__((aligned(16))) ;
	  
	  tmp2=_mm_and_si128(tmp,mask);
	  tmp2=_mm_cmpeq_epi16(tmp2,mask);
	  decoded_bytes[n_128*0+i]=(uint8_t) _mm_movemask_epi8(_mm_packs_epi16(tmp2,zeros));
	  
	  int j;
	  for (j=1; j<16; j++) {
	    mask=_mm_slli_epi16(mask,1);
	    tmp2=_mm_and_si128(tmp,mask);
	    tmp2=_mm_cmpeq_epi16(tmp2,mask);
	    decoded_bytes[n_128*j +i]=(uint8_t) _mm_movemask_epi8(_mm_packs_epi16(tmp2,zeros));
	  }
	}
      }
      else {
	pi6_p=pi6tab8[iind];
	for (i=0;i<(n2>>4);i++) {
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],7);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],6);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],5);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],4);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],3);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],2);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],1);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],0);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],15);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],14);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],13);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],12);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],11);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],10);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],9);
	  tmp=_mm_insert_epi8(tmp, ((llr_t *)tmp128)[*pi6_p++],8);
	  tmp=_mm_cmpgt_epi8(tmp,zeros);
	  ((uint16_t *)decoded_bytes)[i]=(uint16_t)_mm_movemask_epi8(tmp);
	}
      }
      // check the CRC
      oldcrc= *((unsigned int *)(&decoded_bytes[(n>>3)-crc_len]));
      switch (crc_type) {
	
      case CRC24_A: 
	oldcrc&=0x00ffffff;
	crc = crc24a(&decoded_bytes[F>>3],
		     n-24-F)>>8;
	temp=((uint8_t *)&crc)[2];
	((uint8_t *)&crc)[2] = ((uint8_t *)&crc)[0];
	((uint8_t *)&crc)[0] = temp;
	break; 
      case CRC24_B:
	oldcrc&=0x00ffffff;
	crc = crc24b(decoded_bytes,
		     n-24)>>8;
	temp=((uint8_t *)&crc)[2];
	((uint8_t *)&crc)[2] = ((uint8_t *)&crc)[0];
	((uint8_t *)&crc)[0] = temp;
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
	return(255);
	break;
      }
      
      stop_meas(intl2_stats);
 
      if ((crc == oldcrc) && (crc!=0)) {
	return(iteration_cnt);
      }
    }
    
    // do a new iteration if it is not yet decoded  
    if (iteration_cnt < max_iterations) {
      log_map8(systematic1,yparity1,m11,m10,alpha,beta,ext,n2,0,F,offset8_flag,alpha_stats,beta_stats,gamma_stats,ext_stats);
      __m128i* ext_128=(__m128i*) ext;
      __m128i* s1_128=(__m128i*) systematic1;
      __m128i* s0_128=(__m128i*) systematic0;
      int myloop=n2>>4;
      for (i=0;i<myloop;i++) {
       *ext_128=_mm_adds_epi8(_mm_subs_epi8(*ext_128,*s1_128++),*s0_128++);
       ext_128++;
      }  
    }
  }
  return(iteration_cnt);
}
