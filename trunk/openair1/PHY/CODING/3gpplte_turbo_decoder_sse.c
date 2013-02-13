/* file: 3gpplte_turbo_decoder_sse.c
   purpose: Routines for implementing max-logmap decoding of Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 

   Note: This routine currently requires SSE2,SSSE3 and SSE4.1 equipped computers.  IT WON'T RUN OTHERWISE!
  
   Changelog: 17.11.2009 FK SSE4.1 not required anymore
   Aug. 2012 new parallelization options for higher speed (8-way parallelization)
   Jan. 2013 8-bit LLR support with 16-way parallelization
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






//#define DEBUG_LOGMAP

 
#ifdef LLR8

typedef int8_t llr_t; // internal decoder LLR data is 8-bit fixed
typedef int8_t channel_t;
#define MAX 64
#define MINBY2 0xc0c0
#else 

typedef int16_t llr_t; // internal decoder LLR data is 16-bit fixed
typedef int16_t channel_t;
#define MAX 256

#endif

void log_map (llr_t* systematic,channel_t* y_parity, llr_t* m11, llr_t* m10, llr_t *alpha, llr_t *beta, llr_t* ext,unsigned short frame_length,unsigned char term_flag,unsigned char F,int offset8_flag);
void compute_gamma(llr_t* m11,llr_t* m10,llr_t* systematic, channel_t* y_parity, unsigned short frame_length,unsigned char term_flag);
void compute_alpha(llr_t*alpha,llr_t *beta, llr_t* m11,llr_t* m10, unsigned short frame_length,unsigned char F);
void compute_beta(llr_t*alpha, llr_t* beta,llr_t* m11,llr_t* m10, unsigned short frame_length,unsigned char F,int offset8_flag);
void compute_ext(llr_t* alpha,llr_t* beta,llr_t* m11,llr_t* m10,llr_t* extrinsic, llr_t* ap, unsigned short frame_length);


void print_bytes(char *s, __m128i *x) {

  int8_t *tempb = (int8_t *)x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",s,
	 tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7],
	 tempb[8],tempb[9],tempb[10],tempb[11],tempb[12],tempb[13],tempb[14],tempb[15]);

}


void log_map(llr_t* systematic,channel_t* y_parity, llr_t* m11, llr_t* m10, llr_t *alpha, llr_t *beta,llr_t* ext,unsigned short frame_length,unsigned char term_flag,unsigned char F,int offset8_flag) {

#ifdef DEBUG_LOGMAP
  msg("log_map, frame_length %d\n",frame_length);
#endif


  compute_gamma(m11,m10,systematic,y_parity,frame_length,term_flag);
  compute_alpha(alpha,beta,m11,m10,frame_length,F);
  compute_beta(alpha,beta,m11,m10,frame_length,F,offset8_flag);
  compute_ext(alpha,beta,m11,m10,ext,systematic,frame_length);


}

void compute_gamma(llr_t* m11,llr_t* m10,llr_t* systematic,channel_t* y_parity,
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
#ifndef LLR8
  K1=frame_length>>3;
  for (k=0;k<K1;k++) {

    m11_128[k] = _mm_srai_epi16(_mm_adds_epi16(systematic128[k],y_parity128[k]),1);
    m10_128[k] = _mm_srai_epi16(_mm_subs_epi16(systematic128[k],y_parity128[k]),1);

    //      msg("gamma %d : (%d,%d) -> (%d,%d)\n",k,systematic[k],y_parity[k],m11[k],m10[k]);
  }
  // Termination
  m11_128[k] = _mm_srai_epi16(_mm_adds_epi16(systematic128[k+term_flag],y_parity128[k]),1);
  m10_128[k] = _mm_srai_epi16(_mm_subs_epi16(systematic128[k+term_flag],y_parity128[k]),1);

  //  printf("gamma (term): %d,%d, %d,%d, %d,%d\n",m11[k<<3],m10[k<<3],m11[1+(k<<3)],m10[1+(k<<3)],m11[2+(k<<3)],m10[2+(k<<3)]);
#else
  K1 = (frame_length>>4);  
  for (k=0;k<K1;k++) {  // +1 since length is not necessarily a multiple of 16 bits
    m11_128[k] = _mm_adds_epi8(systematic128[k],y_parity128[k]);
    m10_128[k] = _mm_subs_epi8(systematic128[k],y_parity128[k]);
    
    //      msg("gamma %d : (%d,%d) -> (%d,%d)\n",k,systematic[k],y_parity[k],m11[k],m10[k]);
  }

  // Termination
  m11_128[k] = _mm_adds_epi8(systematic128[k+term_flag],y_parity128[k]);
  m10_128[k] = _mm_subs_epi8(systematic128[k+term_flag],y_parity128[k]);

#endif
  _mm_empty();
  _m_empty();
  
}

#define L 64

void compute_alpha(llr_t* alpha,llr_t* beta,llr_t* m_11,llr_t* m_10,unsigned short frame_length,unsigned char F) {
  int k,l,l2,K1,rerun_flag=0;
  __m128i *alpha128=(__m128i *)alpha,*alpha_ptr;
  __m128i m11_0,m10_0;
  __m128i m_b0,m_b1,m_b2,m_b3,m_b4,m_b5,m_b6,m_b7;
  __m128i new0,new1,new2,new3,new4,new5,new6,new7;
  __m128i alpha_max;

#ifndef LLR8
  l2 = L>>3;
  K1 = (frame_length>>3);
#else
  l2 = L>>4;
  K1 = (frame_length>>4);
#endif

  for (l=K1;;l=l2,rerun_flag=1) {
#ifndef LLR8
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
      alpha128[0] = _mm_slli_si128(alpha128[frame_length],2);
      alpha128[1] = _mm_slli_si128(alpha128[1+frame_length],2);
      alpha128[2] = _mm_slli_si128(alpha128[2+frame_length],2);
      alpha128[3] = _mm_slli_si128(alpha128[3+frame_length],2);
      alpha128[4] = _mm_slli_si128(alpha128[4+frame_length],2);
      alpha128[5] = _mm_slli_si128(alpha128[5+frame_length],2);
      alpha128[6] = _mm_slli_si128(alpha128[6+frame_length],2);
      alpha128[7] = _mm_slli_si128(alpha128[7+frame_length],2);
      alpha[8] = -MAX/2;
      alpha[16] = -MAX/2;
      alpha[24] = -MAX/2;
      alpha[32] = -MAX/2;
      alpha[40] = -MAX/2;
      alpha[48] = -MAX/2;
      alpha[56] = -MAX/2;
      /*
	printf("alpha (init, frame_length %d) alpha128[frame_length) = %p\n",frame_length,&alpha128[frame_length]);
	print_shorts("a0:",&alpha128[0]);
	print_shorts("a1:",&alpha128[1]);
	print_shorts("a2:",&alpha128[2]);
	print_shorts("a3:",&alpha128[3]);
	print_shorts("a4:",&alpha128[4]);
	print_shorts("a5:",&alpha128[5]);
	print_shorts("a6:",&alpha128[6]);
	print_shorts("a7:",&alpha128[7]);      
      */
    }
  
    alpha_ptr = &alpha128[0];
    /*
      printf("alpha k %d\n",-1);
      print_shorts("a0:",&alpha_ptr[0]);
      print_shorts("a1:",&alpha_ptr[1]);
      print_shorts("a2:",&alpha_ptr[2]);
      print_shorts("a3:",&alpha_ptr[3]);
      print_shorts("a4:",&alpha_ptr[4]);
      print_shorts("a5:",&alpha_ptr[5]);
      print_shorts("a6:",&alpha_ptr[6]);
      print_shorts("a7:",&alpha_ptr[7]);
    */

    for (k=0;
	 k<l;
	 k++){

      //            printf("*****%d/%d (rerun %d)\n",k,l,rerun_flag);

      m11_0=((__m128i*)m_11)[k];  
      m10_0=((__m128i*)m_10)[k];
    
    
      m_b0 = _mm_adds_epi16(alpha_ptr[1],m11_0);  // m11
      m_b4 = _mm_subs_epi16(alpha_ptr[1],m11_0);  // m00=-m11    
      m_b1 = _mm_subs_epi16(alpha_ptr[3],m10_0);  // m01=-m10
      m_b5 = _mm_adds_epi16(alpha_ptr[3],m10_0);  // m10
      m_b2 = _mm_adds_epi16(alpha_ptr[5],m10_0);  // m10
      m_b6 = _mm_subs_epi16(alpha_ptr[5],m10_0);  // m01=-m10
      m_b3 = _mm_subs_epi16(alpha_ptr[7],m11_0);  // m00=-m11
      m_b7 = _mm_adds_epi16(alpha_ptr[7],m11_0);  // m11
     
      new0 = _mm_subs_epi16(alpha_ptr[0],m11_0);  // m00=-m11
      new4 = _mm_adds_epi16(alpha_ptr[0],m11_0);  // m11
      new1 = _mm_adds_epi16(alpha_ptr[2],m10_0);  // m10
      new5 = _mm_subs_epi16(alpha_ptr[2],m10_0);  // m01=-m10
      new2 = _mm_subs_epi16(alpha_ptr[4],m10_0);  // m01=-m10
      new6 = _mm_adds_epi16(alpha_ptr[4],m10_0);  // m10
      new3 = _mm_adds_epi16(alpha_ptr[6],m11_0);  // m11
      new7 = _mm_subs_epi16(alpha_ptr[6],m11_0);  // m00=-m11
    
      alpha_ptr += 8;
    
      alpha_ptr[0] = _mm_max_epi16(m_b0,new0);
      alpha_ptr[1] = _mm_max_epi16(m_b1,new1);
      alpha_ptr[2] = _mm_max_epi16(m_b2,new2);
      alpha_ptr[3] = _mm_max_epi16(m_b3,new3);
      alpha_ptr[4] = _mm_max_epi16(m_b4,new4);
      alpha_ptr[5] = _mm_max_epi16(m_b5,new5);
      alpha_ptr[6] = _mm_max_epi16(m_b6,new6);
      alpha_ptr[7] = _mm_max_epi16(m_b7,new7);
    
    
      // compute and subtract maxima
      alpha_max = _mm_max_epi16(alpha_ptr[0],alpha_ptr[1]);
      alpha_max = _mm_max_epi16(alpha_max,alpha_ptr[2]);
      alpha_max = _mm_max_epi16(alpha_max,alpha_ptr[3]);
      alpha_max = _mm_max_epi16(alpha_max,alpha_ptr[4]);
      alpha_max = _mm_max_epi16(alpha_max,alpha_ptr[5]);
      alpha_max = _mm_max_epi16(alpha_max,alpha_ptr[6]);
      alpha_max = _mm_max_epi16(alpha_max,alpha_ptr[7]);
    
      alpha_ptr[0] = _mm_subs_epi16(alpha_ptr[0],alpha_max);
      alpha_ptr[1] = _mm_subs_epi16(alpha_ptr[1],alpha_max);
      alpha_ptr[2] = _mm_subs_epi16(alpha_ptr[2],alpha_max);
      alpha_ptr[3] = _mm_subs_epi16(alpha_ptr[3],alpha_max);
      alpha_ptr[4] = _mm_subs_epi16(alpha_ptr[4],alpha_max);
      alpha_ptr[5] = _mm_subs_epi16(alpha_ptr[5],alpha_max);
      alpha_ptr[6] = _mm_subs_epi16(alpha_ptr[6],alpha_max);
      alpha_ptr[7] = _mm_subs_epi16(alpha_ptr[7],alpha_max);
    
    
      //      printf("alpha k %d/%d (%d) (%p,%p)\n",k,K1,k<<3,alpha_ptr,&alpha[(frame_length+16)*8]);
      /*
	print_shorts("m11:",&m11_0);
	print_shorts("m10:",&m10_0);
	print_shorts("a0:",&alpha_ptr[0]);
	print_shorts("a1:",&alpha_ptr[1]);
	print_shorts("a2:",&alpha_ptr[2]);
	print_shorts("a3:",&alpha_ptr[3]);
	print_shorts("a4:",&alpha_ptr[4]);
	print_shorts("a5:",&alpha_ptr[5]);
	print_shorts("a6:",&alpha_ptr[6]);
	print_shorts("a7:",&alpha_ptr[7]);      
      */
      /*
	if (rerun_flag == 1) {
	int i,j;
	for (j=0;j<8;j++) {
	for (i=0;i<8;i++) {
	if (((int16_t*)&alpha_ptr[i])[j] == 0)
	printf("state %d in pos %d, alpha %d\n", i, k+(j*K1),((int16_t*)&alpha_ptr[i])[j]);
	}
	}
	}
      */
    }
#else


    if (rerun_flag == 0) {

      alpha128[0] = _mm_set_epi8(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,0);
      alpha128[1] = _mm_set_epi8(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[2] = _mm_set_epi8(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[3] = _mm_set_epi8(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[4] = _mm_set_epi8(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[5] = _mm_set_epi8(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[6] = _mm_set_epi8(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);
      alpha128[7] = _mm_set_epi8(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2);

 
    }
    else { 
      alpha128[0] = _mm_slli_si128(alpha128[(K1<<3)],1);
      alpha128[1] = _mm_slli_si128(alpha128[1+(K1<<3)],1);
      alpha128[2] = _mm_slli_si128(alpha128[2+(K1<<3)],1);
      alpha128[3] = _mm_slli_si128(alpha128[3+(K1<<3)],1);
      alpha128[4] = _mm_slli_si128(alpha128[4+(K1<<3)],1);
      alpha128[5] = _mm_slli_si128(alpha128[5+(K1<<3)],1);
      alpha128[6] = _mm_slli_si128(alpha128[6+(K1<<3)],1);
      alpha128[7] = _mm_slli_si128(alpha128[7+(K1<<3)],1);
      alpha[8] =  -MAX/2;
      alpha[16] = -MAX/2;
      alpha[24] = -MAX/2;
      alpha[32] = -MAX/2;
      alpha[40] = -MAX/2;
      alpha[48] = -MAX/2;
      alpha[56] = -MAX/2;
    }
  
    alpha_ptr = &alpha128[0];

    for (k=0;
	 k<l;
	 k++){
    
      m11_0=((__m128i*)m_11)[k];  
      m10_0=((__m128i*)m_10)[k];
    
    
      m_b0 = _mm_adds_epi8(alpha_ptr[1],m11_0);  // m11
      m_b4 = _mm_subs_epi8(alpha_ptr[1],m11_0);  // m00=-m11    
      m_b1 = _mm_subs_epi8(alpha_ptr[3],m10_0);  // m01=-m10
      m_b5 = _mm_adds_epi8(alpha_ptr[3],m10_0);  // m10
      m_b2 = _mm_adds_epi8(alpha_ptr[5],m10_0);  // m10
      m_b6 = _mm_subs_epi8(alpha_ptr[5],m10_0);  // m01=-m10
      m_b3 = _mm_subs_epi8(alpha_ptr[7],m11_0);  // m00=-m11
      m_b7 = _mm_adds_epi8(alpha_ptr[7],m11_0);  // m11
    
      new0 = _mm_subs_epi8(alpha_ptr[0],m11_0);  // m00=-m11
      new4 = _mm_adds_epi8(alpha_ptr[0],m11_0);  // m11
      new1 = _mm_adds_epi8(alpha_ptr[2],m10_0);  // m10
      new5 = _mm_subs_epi8(alpha_ptr[2],m10_0);  // m01=-m10
      new2 = _mm_subs_epi8(alpha_ptr[4],m10_0);  // m01=-m10
      new6 = _mm_adds_epi8(alpha_ptr[4],m10_0);  // m10
      new3 = _mm_adds_epi8(alpha_ptr[6],m11_0);  // m11
      new7 = _mm_subs_epi8(alpha_ptr[6],m11_0);  // m00=-m11
    
      alpha_ptr += 8;
    
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
      /*
	printf("alpha k %d (%d) (%p)\n",k,k<<4,alpha_ptr);
    
	print_bytes("m11:",&m11_0);
	print_bytes("m10:",&m10_0);
	print_bytes("a0:",&alpha_ptr[0]);
	print_bytes("a1:",&alpha_ptr[1]);
	print_bytes("a2:",&alpha_ptr[2]);
	print_bytes("a3:",&alpha_ptr[3]);
	print_bytes("a4:",&alpha_ptr[4]);
	print_bytes("a5:",&alpha_ptr[5]);
	print_bytes("a6:",&alpha_ptr[6]);
	print_bytes("a7:",&alpha_ptr[7]);      
      */
    }
#endif
    if (rerun_flag==1)
      break;
  }  
  _mm_empty();
  _m_empty();
}


void compute_beta(llr_t* alpha,llr_t* beta,llr_t *m_11,llr_t* m_10,unsigned short frame_length,unsigned char F,int offset8_flag) {
  
  int k,rerun_flag=0;
  __m128i m11_128,m10_128;
  __m128i m_b0,m_b1,m_b2,m_b3,m_b4,m_b5,m_b6,m_b7;
  __m128i new0,new1,new2,new3,new4,new5,new6,new7;

  __m128i *beta128,*alpha128,*beta_ptr;
  __m128i beta_max; 
  int16_t m11,m10,beta0_16,beta1_16,beta2_16,beta3_16,beta4_16,beta5_16,beta6_16,beta7_16,beta0_2,beta1_2,beta2_2,beta3_2,beta_m; 
  llr_t beta0,beta1,beta2,beta3,beta4,beta5,beta6,beta7;
  __m128i beta_16;

#ifdef DEBUG_LOGMAP
  msg("compute_beta, %p,%p,%p,%p,framelength %d,F %d\n",
      beta,m_11,m_10,alpha,frame_length,F);
#endif


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

#ifdef LLR8
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

  for (rerun_flag=0;;rerun_flag=1) {
    beta_ptr   = (__m128i*)&beta[frame_length<<3];
    alpha128   = (__m128i*)&alpha[0];
    if (rerun_flag == 0) {
#ifndef LLR8
      beta_ptr[0] = alpha128[(frame_length)];
      beta_ptr[1] = alpha128[1+(frame_length)];
      beta_ptr[2] = alpha128[2+(frame_length)];
      beta_ptr[3] = alpha128[3+(frame_length)];
      beta_ptr[4] = alpha128[4+(frame_length)];
      beta_ptr[5] = alpha128[5+(frame_length)];
      beta_ptr[6] = alpha128[6+(frame_length)];
      beta_ptr[7] = alpha128[7+(frame_length)];
#else
      beta_ptr[0] = alpha128[(frame_length>>1)];
      beta_ptr[1] = alpha128[1+(frame_length>>1)];
      beta_ptr[2] = alpha128[2+(frame_length>>1)];
      beta_ptr[3] = alpha128[3+(frame_length>>1)];
      beta_ptr[4] = alpha128[4+(frame_length>>1)];
      beta_ptr[5] = alpha128[5+(frame_length>>1)];
      beta_ptr[6] = alpha128[6+(frame_length>>1)];
      beta_ptr[7] = alpha128[7+(frame_length>>1)];
#endif
    }
    else {
      beta128 = (__m128i*)&beta[0];
#ifndef LLR8
      
      beta_ptr[0] = _mm_srli_si128(beta128[0],2);
      beta_ptr[1] = _mm_srli_si128(beta128[1],2);
      beta_ptr[2] = _mm_srli_si128(beta128[2],2);
      beta_ptr[3] = _mm_srli_si128(beta128[3],2);
      beta_ptr[4] = _mm_srli_si128(beta128[4],2);
      beta_ptr[5] = _mm_srli_si128(beta128[5],2);
      beta_ptr[6] = _mm_srli_si128(beta128[6],2);
      beta_ptr[7] = _mm_srli_si128(beta128[7],2);


#else
      beta_ptr[0] = _mm_srli_si128(beta128[0],1);
      beta_ptr[1] = _mm_srli_si128(beta128[1],1);
      beta_ptr[2] = _mm_srli_si128(beta128[2],1);
      beta_ptr[3] = _mm_srli_si128(beta128[3],1);
      beta_ptr[4] = _mm_srli_si128(beta128[4],1);
      beta_ptr[5] = _mm_srli_si128(beta128[5],1);
      beta_ptr[6] = _mm_srli_si128(beta128[6],1);
      beta_ptr[7] = _mm_srli_si128(beta128[7],1);
      if (offset8_flag==0) {
	beta_ptr[0] = _mm_insert_epi8(beta_ptr[0],beta0,15);
	beta_ptr[1] = _mm_insert_epi8(beta_ptr[1],beta1,15);
	beta_ptr[2] = _mm_insert_epi8(beta_ptr[2],beta2,15);
	beta_ptr[3] = _mm_insert_epi8(beta_ptr[3],beta3,15);
	beta_ptr[4] = _mm_insert_epi8(beta_ptr[4],beta4,15);
	beta_ptr[5] = _mm_insert_epi8(beta_ptr[5],beta5,15);
	beta_ptr[6] = _mm_insert_epi8(beta_ptr[6],beta6,15);
	beta_ptr[7] = _mm_insert_epi8(beta_ptr[7],beta7,15);
	/*
	beta[15+(frame_length<<3)] = beta0;
	beta[31+(frame_length<<3)] = beta1;
	beta[47+(frame_length<<3)] = beta2;
	beta[63+(frame_length<<3)] = beta3;
	beta[79+(frame_length<<3)] = beta4;
	beta[95+(frame_length<<3)] = beta5;
	beta[111+(frame_length<<3)] = beta6;
	beta[127+(frame_length<<3)] = beta7;*/
      }
      else {
	beta_ptr[0] = _mm_insert_epi8(beta_ptr[0],beta0,7);
	beta_ptr[1] = _mm_insert_epi8(beta_ptr[1],beta1,7);
	beta_ptr[2] = _mm_insert_epi8(beta_ptr[2],beta2,7);
	beta_ptr[3] = _mm_insert_epi8(beta_ptr[3],beta3,7);
	beta_ptr[4] = _mm_insert_epi8(beta_ptr[4],beta4,7);
	beta_ptr[5] = _mm_insert_epi8(beta_ptr[5],beta5,7);
	beta_ptr[6] = _mm_insert_epi8(beta_ptr[6],beta6,7);
	beta_ptr[7] = _mm_insert_epi8(beta_ptr[7],beta7,7);
	/*
	beta[7+(frame_length<<3)] = beta0;
	beta[23+(frame_length<<3)] = beta1;
	beta[39+(frame_length<<3)] = beta2;
	beta[55+(frame_length<<3)] = beta3;
	beta[71+(frame_length<<3)] = beta4;
	beta[87+(frame_length<<3)] = beta5;
	beta[103+(frame_length<<3)] = beta6;
	beta[119+(frame_length<<3)] = beta7;*/
      }
#endif
    }

#ifndef LLR8    
    beta_ptr[0] = _mm_insert_epi16(beta_ptr[0],beta0_16,7);
    beta_ptr[1] = _mm_insert_epi16(beta_ptr[1],beta1_16,7);
    beta_ptr[2] = _mm_insert_epi16(beta_ptr[2],beta2_16,7);
    beta_ptr[3] = _mm_insert_epi16(beta_ptr[3],beta3_16,7);
    beta_ptr[4] = _mm_insert_epi16(beta_ptr[4],beta4_16,7);
    beta_ptr[5] = _mm_insert_epi16(beta_ptr[5],beta5_16,7);
    beta_ptr[6] = _mm_insert_epi16(beta_ptr[6],beta6_16,7);
    beta_ptr[7] = _mm_insert_epi16(beta_ptr[7],beta7_16,7);
    
    /*
      beta[7+(frame_length<<3)] = beta0_16;
      beta[15+(frame_length<<3)] = beta1_16;
      beta[23+(frame_length<<3)] = beta2_16;
      beta[31+(frame_length<<3)] = beta3_16;
      beta[39+(frame_length<<3)] = beta4_16;
      beta[47+(frame_length<<3)] = beta5_16;
      beta[55+(frame_length<<3)] = beta6_16;
      beta[63+(frame_length<<3)] = beta7_16;*/    
#endif
    
#ifndef LLR8    
    for (k=(frame_length>>3)-1;k>=((rerun_flag==0)?0:((frame_length-L)>>3));k--){
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

#else

    for (k=(frame_length>>4)-1;k>=((rerun_flag==0)?0:((frame_length-L)>>4));k--){
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
  
#endif
    if (rerun_flag==1)
      break;
  }
  _mm_empty();
  _m_empty();
}

void compute_ext(llr_t* alpha,llr_t* beta,llr_t* m_11,llr_t* m_10,llr_t* ext, llr_t* systematic,unsigned short frame_length)
{
  __m128i *alpha128=(__m128i *)alpha;
  __m128i *beta128=(__m128i *)beta;
  __m128i *m11_128,*m10_128,*ext_128,*systematic_128;
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


#ifndef LLR8
  for (k=0;k<(frame_length>>3);k++){

    m11_128        = (__m128i*)&m_11[k<<3];
    m10_128        = (__m128i*)&m_10[k<<3];
    ext_128        = (__m128i*)&ext[k<<3];
    systematic_128 = (__m128i*)&systematic[k<<3];
    /*
      printf("EXT %d\n",k);
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

    *ext_128 = _mm_subs_epi16(m10_1,_mm_adds_epi16(m01_1,*systematic_128));
      
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

#else

  for (k=0;k<(frame_length>>4);k++){

    m11_128        = (__m128i*)&m_11[k<<4];
    m10_128        = (__m128i*)&m_10[k<<4];
    ext_128        = (__m128i*)&ext[k<<4];
    systematic_128 = (__m128i*)&systematic[k<<4];

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


    *ext_128 = _mm_subs_epi8(m10_1,_mm_adds_epi8(m01_1,*systematic_128));
      
    alpha_ptr+=8;
    beta_ptr+=8;
  }
#endif

  _mm_empty();
  _m_empty();

}


//unsigned char decoder_in_use[MAX_DECODING_THREADS] = {0,0,0,0,0};

unsigned char phy_threegpplte_turbo_decoder(short *y,
					    unsigned char *decoded_bytes,
					    unsigned short n,
					    unsigned short f1,
					    unsigned short f2,
					    unsigned char max_iterations,
					    unsigned char crc_type,
					    unsigned char F,
					    unsigned char inst) {
  
  /*  y is a pointer to the input
      decoded_bytes is a pointer to the decoded output
      n is the size in bits of the coded block, with the tail */

  int n2;
#ifdef LLR8
  llr_t y8[3*(n+16)] __attribute__((aligned(16)));
#endif

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

  
  unsigned int pi,*pi2_p,*pi3_p,pi2[n],pi3[n+8],pi5[n+8],pi4[n+8],pi6[n+8],*pi4_p,*pi5_p,*pi6_p;
  llr_t *yp,*s,*s1,*s2,*yp1,*yp2;
  unsigned int i,j;//,pi;
  unsigned char iteration_cnt=0;
  unsigned int crc,oldcrc,crc_len;
  u8 temp;
  __m128i tmp128[(n+8)>>3];

  short * base_interleaver;
  __m128i tmp, zeros=_mm_setzero_si128();
#ifdef LLR8
  __m128i MAX128=_mm_set1_epi16(MAX/2);
#endif

  if (crc_type > 3) {
    msg("Illegal crc length!\n");
    return 255;
  }

#ifdef LLR8
  if ((n&15)>0) {
    n2 = n+8;
  }
  else 
    n2 = n;
#else
  n2=n;
#endif

  // zero out all global variables
  bzero(alpha,(n+16)*8*sizeof(llr_t));
  bzero(beta,(n+16)*8*sizeof(llr_t));
  bzero(m11,(n+16)*sizeof(llr_t));
  bzero(m10,(n+16)*sizeof(llr_t));
  bzero(systematic0,(n+16)*sizeof(llr_t));
  bzero(systematic1,(n+16)*sizeof(llr_t));
  bzero(systematic2,(n+16)*sizeof(llr_t));
  bzero(yparity1,(n+16)*sizeof(llr_t));
  bzero(yparity2,(n+16)*sizeof(llr_t));
  bzero(ext,(n+128)*sizeof(llr_t));
  bzero(ext2,(n+128)*sizeof(llr_t));
  bzero(pi3,(n+8)*sizeof(int));
  bzero(pi4,(n+8)*sizeof(int));
  bzero(pi5,(n+8)*sizeof(int));
  bzero(pi6,(n+8)*sizeof(int));

  //  threegpplte_interleaver_reset();

  for (i=0;f1f2mat[i].nb_bits!= n && i <188; i++);
  if ( i == 188 ) {
    msg("Illegal frame length!\n");
    return 255;
  }
  else {
    base_interleaver=il_tb+f1f2mat[i].beg_index;
  }
#ifndef LLR8
  for (j=0,i=0;i<n;i++,j+=8) {

    if (j>=n)
      j-=(n-1);
  
    pi2[i]  = j;
    //    printf("pi2[%d] = %d\n",i,j);
  }
#else
  for (j=0,i=0;i<n;i++,j+=16) {

    if (j>=n)
      j-=(n-1);
  
    pi2[i] = j;
    //    printf("pi2[%d] = %d\n",i,j);
  }

#endif
  for (i=0;i<n;i++) {
    pi = base_interleaver[i];//(unsigned int)threegpplte_interleaver(f1,f2,n);
    pi3[i] = pi2[pi];
    pi4[pi2[i]] = pi3[i];
    pi5[pi3[i]] = pi2[i];
    pi6[pi] = pi2[i];
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
 
#ifdef LLR8
  for (i=0,j=0;i<(3*(n2>>4))+1;i++,j+=2) {
    ((__m128i *)y8)[i] = _mm_packs_epi16(_mm_srai_epi16(((__m128i *)y)[j],1),_mm_srai_epi16(((__m128i *)y)[j+1],1));
//    ((__m128i *)y8)[i] = _mm_packs_epi16(((__m128i *)y)[j],((__m128i *)y)[j+1]);
  }
  yp = y8;
#else
  yp = y;
#endif

  s = systematic0;
  s1 = systematic1;
  s2 = systematic2;
  yp1 = yparity1;
  yp2 = yparity2;
 
  for (i=0;i<n;i+=8) {
    pi2_p = &pi2[i];
    
    j=pi2_p[0];



    s[j] = yp[0]; 
    yp1[j] = yp[1]; 
    yp2[j] = yp[2];
#ifdef DEBUG_LOGMAP
    msg("Position %d (%d): (%d,%d,%d)\n",i,j,s[j],yp1[j],yp2[j]);
#endif //DEBUG_LOGMAP
    
    j=pi2_p[1];


    s[j] = yp[3]; 
    yp1[j] = yp[4]; 
    yp2[j] = yp[5];
#ifdef DEBUG_LOGMAP
    msg("Position %d (%d): (%d,%d,%d)\n",i,j,s[j],yp1[j],yp2[j]);
#endif //DEBUG_LOGMAP
    
    j=pi2_p[2];

    s[j] = yp[6]; 
    yp1[j] = yp[7]; 
    yp2[j] = yp[8];
#ifdef DEBUG_LOGMAP
    msg("Position %d (%d): (%d,%d,%d)\n",i,j,s[j],yp1[j],yp2[j]);
#endif //DEBUG_LOGMAP
    
    j=pi2_p[3];

    s[j] = yp[9]; 
    yp1[j] = yp[10]; 
    yp2[j] = yp[11];
#ifdef DEBUG_LOGMAP
    msg("Position %d (%d): (%d,%d,%d)\n",i,j,s[j],yp1[j],yp2[j]);
#endif //DEBUG_LOGMAP
    
    j=pi2_p[4];

    s[j] = yp[12]; 
    yp1[j] = yp[13]; 
    yp2[j] = yp[14];
#ifdef DEBUG_LOGMAP
    msg("Position %d (%d): (%d,%d,%d)\n",i,j,s[j],yp1[j],yp2[j]);
#endif //DEBUG_LOGMAP
    
    j=pi2_p[5];

    s[j] = yp[15]; 
    yp1[j] = yp[16]; 
    yp2[j] = yp[17];
#ifdef DEBUG_LOGMAP
    msg("Position %d (%d): (%d,%d,%d)\n",i,j,s[j],yp1[j],yp2[j]);
#endif //DEBUG_LOGMAP
    
    j=pi2_p[6];

    s[j] = yp[18]; 
    yp1[j] = yp[19];
    yp2[j] = yp[20];

#ifdef DEBUG_LOGMAP
    msg("Position %d (%d): (%d,%d,%d)\n",i,j,s[j],yp1[j],yp2[j]);
#endif //DEBUG_LOGMAP
    
    j=pi2_p[7];

    s[j] = yp[21]; 
    yp1[j] = yp[22]; 
    yp2[j] = yp[23];
    
    yp+=24;
#ifdef DEBUG_LOGMAP
    msg("Position %d (%d): (%d,%d,%d)\n",i,j,s[j],yp1[j],yp2[j]);
#endif //DEBUG_LOGMAP
  }
  
  
  // Termination
  for (i=n2;i<n2+3;i++) {
    s[i]= *yp; s1[i] = s[i] ; s2[i] = s[i]; yp++;
    yp1[i] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    msg("Term 1 (%d): %d %d\n",i,s[i],yp1[i]);
#endif //DEBUG_LOGMAP
  }
#ifdef LLR8
  for (i=n2+16;i<n2+19;i++) {
    s[i]= *yp; s1[i] = s[i] ; s2[i] = s[i]; yp++;
    yp2[i-16] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    msg("Term 2 (%d): %d %d\n",i-3,s[i],yp2[i-16]);
#endif //DEBUG_LOGMAP
  }
#else
  for (i=n2+8;i<n2+11;i++) {
    s[i]= *yp; s1[i] = s[i] ; s2[i] = s[i]; yp++;
    yp2[i-8] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    msg("Term 2 (%d): %d %d\n",i-3,s[i],yp2[i-8]);
#endif //DEBUG_LOGMAP
  }
#endif
#ifdef DEBUG_LOGMAP
  msg("\n");
#endif //DEBUG_LOGMAP
  

  // do log_map from first parity bit

  log_map(systematic0,yparity1,m11,m10,alpha,beta,ext,n2,0,F,(n2==n)?0:1);

  while (iteration_cnt++ < max_iterations) {

#ifdef DEBUG_LOGMAP
    printf("\n*******************ITERATION %d (n %d), ext %p\n\n",iteration_cnt,n,ext);
#endif //DEBUG_LOGMAP

#ifndef LLR8
    for (i=0;i<(n>>3);i++) { 
      tmp128[i] = _mm_adds_epi16(((__m128i *)ext)[i],((__m128i *)systematic0)[i]);
    }
    pi4_p=pi4;
    for (i=0;i<(n>>3);i++) { // steady-state portion
      
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],0);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],1);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],2);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],3);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],4);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],5);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],6);
      ((__m128i *)systematic2)[i]=_mm_insert_epi16(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],7);
    }

#else
    for (i=0;i<(n2>>4);i++) {
      tmp128[i] = _mm_adds_epi8(((__m128i *)ext)[i],((__m128i *)systematic0)[i]);
    }
    pi4_p=pi4;
    for (i=0;i<(n2>>4);i++) { // steady-state portion      
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],0);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],1);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],2);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],3);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],4);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],5);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],6);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],7);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],8);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],9);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],10);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],11);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],12);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],13);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],14);
      ((__m128i *)systematic2)[i]=_mm_insert_epi8(((__m128i *)systematic2)[i],((llr_t*)tmp128)[*pi4_p++],15);
    }
#endif

        

    // do log_map from second parity bit    

    log_map(systematic2,yparity2,m11,m10,alpha,beta,ext2,n2,1,F,(n2==n)?0:1);


    memset(decoded_bytes,0,n>>3);



#ifndef LLR8
    pi5_p=pi5;
    for (i=0;i<(n>>3);i++) {
      tmp128[i]=_mm_insert_epi16(tmp128[i],ext2[*pi5_p++],0);
      tmp128[i]=_mm_insert_epi16(tmp128[i],ext2[*pi5_p++],1);
      tmp128[i]=_mm_insert_epi16(tmp128[i],ext2[*pi5_p++],2);
      tmp128[i]=_mm_insert_epi16(tmp128[i],ext2[*pi5_p++],3);
      tmp128[i]=_mm_insert_epi16(tmp128[i],ext2[*pi5_p++],4);
      tmp128[i]=_mm_insert_epi16(tmp128[i],ext2[*pi5_p++],5);
      tmp128[i]=_mm_insert_epi16(tmp128[i],ext2[*pi5_p++],6);
      tmp128[i]=_mm_insert_epi16(tmp128[i],ext2[*pi5_p++],7);
//      tmp128[i]=_mm_adds_epi16(tmp128[i],((__m128i *)systematic0)[i]);
      ((__m128i *)systematic1)[i] =_mm_adds_epi16(tmp128[i],((__m128i *)systematic0)[i]);
    }
/*
    pi3_p=pi3;
    for (i=0;i<(n>>3);i++) { // steady-state portion
      // printf("i%d => pi4[i] %d\n",i,*pi4_p);
      ((__m128i *)systematic1)[i]=_mm_insert_epi16(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],0);
      ((__m128i *)systematic1)[i]=_mm_insert_epi16(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],1);
      ((__m128i *)systematic1)[i]=_mm_insert_epi16(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],2);
      ((__m128i *)systematic1)[i]=_mm_insert_epi16(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],3);
      ((__m128i *)systematic1)[i]=_mm_insert_epi16(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],4);
      ((__m128i *)systematic1)[i]=_mm_insert_epi16(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],5);
      ((__m128i *)systematic1)[i]=_mm_insert_epi16(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],6);
      ((__m128i *)systematic1)[i]=_mm_insert_epi16(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],7);
    }*/

    for (i=0;i<(n>>3);i++) {
      tmp128[i] = _mm_adds_epi16(((__m128i *)ext2)[i],((__m128i *)systematic2)[i]);
    }

    pi6_p=&pi6[0];
    for (i=0;i<(n>>3);i++) {
      tmp=_mm_insert_epi16(tmp, ((llr_t*)tmp128)[*pi6_p++],7);
      tmp=_mm_insert_epi16(tmp, ((llr_t*)tmp128)[*pi6_p++],6);
      tmp=_mm_insert_epi16(tmp, ((llr_t*)tmp128)[*pi6_p++],5);
      tmp=_mm_insert_epi16(tmp, ((llr_t*)tmp128)[*pi6_p++],4);
      tmp=_mm_insert_epi16(tmp, ((llr_t*)tmp128)[*pi6_p++],3);
      tmp=_mm_insert_epi16(tmp, ((llr_t*)tmp128)[*pi6_p++],2);
      tmp=_mm_insert_epi16(tmp, ((llr_t*)tmp128)[*pi6_p++],1);
      tmp=_mm_insert_epi16(tmp, ((llr_t*)tmp128)[*pi6_p++],0);
      tmp=_mm_cmpgt_epi8(_mm_packs_epi16(tmp,zeros),zeros);
      decoded_bytes[i]=(unsigned char)_mm_movemask_epi8(tmp);

    }
  
#else
    pi5_p=pi5;
    for (i=0;i<(n2>>4);i++) {
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],0);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],1);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],2);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],3);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],4);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],5);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],6);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],7);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],8);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],9);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],10);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],11);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],12);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],13);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],14);
      tmp128[i]=_mm_insert_epi8(tmp128[i],ext2[*pi5_p++],15);
//      tmp128[i] = _mm_adds_epi8(tmp128[i],((__m128i *)systematic0)[i]);
      ((__m128i *)systematic1)[i] = _mm_adds_epi8(tmp128[i],((__m128i *)systematic0)[i]);
    }
/*
    pi3_p=pi3;
    for (i=0;i<(n2>>4);i++) { // steady-state portion
      //      printf("i %d/%d: pi3_p %p\n",i<<4,n,pi3_p);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],0);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],1);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],2);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],3);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],4);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],5);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],6);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],7);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],8);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],9);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],10);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],11);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],12);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],13);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],14);
      ((__m128i *)systematic1)[i]=_mm_insert_epi8(((__m128i *)systematic1)[i],((llr_t*)tmp128)[*pi3_p++],15);
	
    }
*/

    for (i=0;i<(n2>>4);i++) {
      tmp128[i] = _mm_adds_epi8(((__m128i *)ext2)[i],((__m128i *)systematic2)[i]);
    }
    pi6_p=&pi6[0];


    for (i=0;i<(n2>>4);i++) {
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],7);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],6);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],5);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],4);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],3);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],2);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],1);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],0);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],15);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],14);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],13);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],12);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],11);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],10);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],9);
      tmp=_mm_insert_epi8(tmp, ((llr_t*)tmp128)[*pi6_p++],8);
      tmp=_mm_cmpgt_epi8(tmp,zeros);
      ((uint16_t *)decoded_bytes)[i]=(uint16_t)_mm_movemask_epi8(tmp);
    }
    
#endif
    
    
    // check status on output

    oldcrc= *((unsigned int *)(&decoded_bytes[(n>>3)-crc_len]));
    switch (crc_type) {

    case CRC24_A: 
      oldcrc&=0x00ffffff;
      crc = crc24a(&decoded_bytes[F>>3],
		   n-24-F)>>8;
      temp=((u8 *)&crc)[2];
      ((u8 *)&crc)[2] = ((u8 *)&crc)[0];
      ((u8 *)&crc)[0] = temp;

      //      msg("CRC24_A = %x, oldcrc = %x (F %d)\n",crc,oldcrc,F);

      break;
    case CRC24_B:
      oldcrc&=0x00ffffff;
      crc = crc24b(decoded_bytes,
		   n-24)>>8;
      temp=((u8 *)&crc)[2];
      ((u8 *)&crc)[2] = ((u8 *)&crc)[0];
      ((u8 *)&crc)[0] = temp;

      //      msg("CRC24_B = %x, oldcrc = %x\n",crc,oldcrc);

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

    if ((crc == oldcrc) && (crc!=0)) {
      //      decoder_in_use[inst] = 0;
      //msg("setting turbo decoder inst %d to 0\n",inst);
      return(iteration_cnt);
    }


    // do log_map from first parity bit
    if (iteration_cnt < max_iterations) {
      //      printf("LOG_MAP 0\n");
      log_map(systematic1,yparity1,m11,m10,alpha,beta,ext,n,0,F,(n2==n)?0:1);
    }
  }

  //msg("setting turbo decoder inst %d to 0\n",inst);
  //  decoder_in_use[inst] = 0;

  return(iteration_cnt);
}

#ifdef TEST_DEBUG

int test_logmap8()
{
  unsigned char test[8];
  //_declspec(align(16))  char channel_output[512];
  //_declspec(align(16))  unsigned char output[512],decoded_output[16], *inPtr, *outPtr;

  short channel_output[512];
  unsigned char output[512],decoded_output[16];
  unsigned int i,crc,ret;
  
  test[0] = 7;
  test[1] = 0xa5;
  test[2] = 0x11;
  test[3] = 0x92;
  test[4] = 0xfe;

  crcTableInit();

  crc = crc24a(test,
	       40)>>8;
    
  *(unsigned int*)(&test[5]) = crc;
 
  printf("crc24 = %x\n",crc);
  threegpplte_turbo_encoder(test,   //input
			    8,      //input length bytes
			    output, //output
			    0,      //filler bits
			    7,      //interleaver f1
			    16);    //interleaver f2

  for (i = 0; i < 204; i++){
    channel_output[i] = 15*(2*output[i] - 1);
    //    msg("Position %d : %d\n",i,channel_output[i]);
  }

  memset(decoded_output,0,16);
  ret = phy_threegpplte_turbo_decoder(channel_output,
				      decoded_output,
				      64,       // length bits
				      7,        // interleaver f1
				      16,       // interleaver f2
				      6,        // max iterations
				      CRC24_A,  // CRC type (CRC24_A,CRC24_B)
				      0,        // filler bits
				      0);       // decoder instance


  for (i=0;i<8;i++)
    printf("output %d => %x (input %x)\n",i,decoded_output[i],test[i]);
}




int main() {


  test_logmap8();

  return(0);
}

#endif // TEST_DEBUG


