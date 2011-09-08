/* file: 3gpplte_turbo_decoder_sse.c
   purpose: Routines for implementing max-logmap decoding of Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 

   Note: This routine currently requires SSE2,SSSE3 and SSE4.1 equipped computers.  IT WON'T RUN OTHERWISE!

   Changelog: 17.11.2009 FK SSE4.1 not required anymore
*/

///
///

#include "emmintrin.h"

#ifdef __SSE3__
#include "pmmintrin.h"
#include "tmmintrin.h"
#else
#define abs_pi16(x,zero,res,sign)     sign=_mm_cmpgt_pi16(zero,x) ; res=_mm_xor_si64(x,sign);   //negate negatives
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(*(__m128i *)&zero[0],(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(*(__m128i *)&zero[0],(xmmy)))
#endif

static short zero[8]  __attribute__ ((aligned(16))) = {0,0,0,0,0,0,0,0} ;

#ifdef __SSE4_1__
#include "smmintrin.h"
#endif

#ifndef TEST_DEBUG 
#include "PHY/defs.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/lte_interleaver_inline.h"

#else

#include "defs.h"
#define msg printf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

typedef short llr_t; // internal decoder data is 16-bit fixed
typedef short channel_t;


#define MAX 16383
#define THRES 8192

#define FRAME_LENGTH_MAX 6144
#define STATES 8

#define MAX_DECODING_THREADS 5 //4 for DLSCH, 1 for PBCH

//#define DEBUG_LOGMAP

void log_map (llr_t* systematic,channel_t* y_parity, llr_t* ext,unsigned short frame_length,unsigned char term_flag,unsigned char F,unsigned char inst);
void compute_gamma(llr_t* m11,llr_t* m10,llr_t* systematic, channel_t* y_parity, unsigned short frame_length,unsigned char term_flag);
void compute_alpha(llr_t*alpha,llr_t* m11,llr_t* m10, unsigned short frame_length,unsigned char F,unsigned char inst);
void compute_beta(llr_t* beta,llr_t* m11,llr_t* m10,llr_t* alpha, unsigned short frame_length,unsigned char F,unsigned char inst);
void compute_ext(llr_t* alpha,llr_t* beta,llr_t* m11,llr_t* m10,llr_t* extrinsic, llr_t* ap, unsigned short frame_length,unsigned char inst);

// global variables
//
llr_t alpha_0[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t beta_0[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t m11_0[(FRAME_LENGTH_MAX+3+1)] __attribute__ ((aligned(16)));
llr_t m10_0[(FRAME_LENGTH_MAX+3+1)] __attribute__ ((aligned(16)));
llr_t alpha_1[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t beta_1[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t m11_1[(FRAME_LENGTH_MAX+3+1)] __attribute__ ((aligned(16)));
llr_t m10_1[(FRAME_LENGTH_MAX+3+1)] __attribute__ ((aligned(16)));
llr_t alpha_2[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t beta_2[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t m11_2[(FRAME_LENGTH_MAX+3+1)] __attribute__ ((aligned(16)));
llr_t m10_2[(FRAME_LENGTH_MAX+3+1)] __attribute__ ((aligned(16)));
llr_t alpha_3[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t beta_3[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t m11_3[(FRAME_LENGTH_MAX+3+1)] __attribute__ ((aligned(16)));
llr_t m10_3[(FRAME_LENGTH_MAX+3+1)] __attribute__ ((aligned(16)));
llr_t alpha_4[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t beta_4[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t m11_4[(FRAME_LENGTH_MAX+3+1)] __attribute__ ((aligned(16)));
llr_t m10_4[(FRAME_LENGTH_MAX+3+1)] __attribute__ ((aligned(16)));
llr_t *alpha_g[MAX_DECODING_THREADS] = {alpha_0, alpha_1, alpha_2, alpha_3, alpha_4};
llr_t *beta_g[MAX_DECODING_THREADS] = {beta_0, beta_1, beta_2, beta_3, beta_4};
llr_t *m11_g[MAX_DECODING_THREADS] = {m11_0, m11_1, m11_2, m11_3, m11_4};
llr_t *m10_g[MAX_DECODING_THREADS] = {m10_0, m10_1, m10_2, m10_3, m10_4};


void log_map(llr_t* systematic,channel_t* y_parity, llr_t* ext,unsigned short frame_length,unsigned char term_flag,unsigned char F,unsigned char inst) {

#ifdef DEBUG_LOGMAP
  msg("log_map\n");
#endif

  compute_gamma(m11_g[inst],m10_g[inst],systematic,y_parity,frame_length,term_flag);

  compute_alpha(alpha_g[inst],m11_g[inst],m10_g[inst],frame_length,F,inst);

  compute_beta(beta_g[inst],m11_g[inst],m10_g[inst],alpha_g[inst],frame_length,F,inst);

  compute_ext(alpha_g[inst],beta_g[inst],m11_g[inst],m10_g[inst],ext,systematic,frame_length,inst);

}

void compute_gamma(llr_t* m11,llr_t* m10,llr_t* systematic,channel_t* y_parity,
		   unsigned short frame_length,unsigned char term_flag)
{
  int k; 
  __m128i *systematic128 = (__m128i *)systematic;
  __m128i *y_parity128   = (__m128i *)y_parity;
  __m128i *m10_128        = (__m128i *)m10;
  __m128i *m11_128        = (__m128i *)m11;

#ifdef DEBUG_LOGMAP
  msg("compute_gamma, %p,%p,%p,%p,framelength %d\n",m11,m10,systematic,y_parity,frame_length);
#endif

  for (k=0;k<frame_length>>3;k++) {
      m11_128[k] = _mm_srai_epi16(_mm_adds_epi16(systematic128[k],y_parity128[k]),1);
      m10_128[k] = _mm_srai_epi16(_mm_subs_epi16(systematic128[k],y_parity128[k]),1);

      //      msg("gamma %d : (%d,%d) -> (%d,%d)\n",k,systematic[k],y_parity[k],m11[k],m10[k]);
  }
  // Termination
  m11_128[k] = _mm_srai_epi16(_mm_adds_epi16(systematic128[k+term_flag],y_parity128[k]),1);
  m10_128[k] = _mm_srai_epi16(_mm_subs_epi16(systematic128[k+term_flag],y_parity128[k]),1);

  _mm_empty();
  _m_empty();
  
}


__m128i mtop_0[6144] __attribute__ ((aligned(16)));
__m128i mbot_0[6144] __attribute__ ((aligned(16)));
__m128i mtop_1[6144] __attribute__ ((aligned(16)));
__m128i mbot_1[6144] __attribute__ ((aligned(16)));
__m128i mtop_2[6144] __attribute__ ((aligned(16)));
__m128i mbot_2[6144] __attribute__ ((aligned(16)));
__m128i mtop_3[6144] __attribute__ ((aligned(16)));
__m128i mbot_3[6144] __attribute__ ((aligned(16)));
__m128i mtop_4[6144] __attribute__ ((aligned(16)));
__m128i mbot_4[6144] __attribute__ ((aligned(16)));
__m128i *mtop_g[MAX_DECODING_THREADS] = {mtop_0, mtop_1, mtop_2, mtop_3, mtop_4};
__m128i *mbot_g[MAX_DECODING_THREADS] = {mbot_0, mbot_1, mbot_2, mbot_3, mbot_4};

__m128i mtmp[MAX_DECODING_THREADS],mtmp2[MAX_DECODING_THREADS],lsw[MAX_DECODING_THREADS],msw[MAX_DECODING_THREADS],new[MAX_DECODING_THREADS],mb[MAX_DECODING_THREADS],newcmp[MAX_DECODING_THREADS] __attribute__ ((aligned(16)));
__m128i TOP,BOT,THRES128;

void compute_alpha(llr_t* alpha,llr_t* m_11,llr_t* m_10,unsigned short frame_length,unsigned char F,unsigned char inst)
{
  int k;
  __m128i *alpha128=(__m128i *)alpha;

  //  __m128i mtmp,mtmp2,lsw,msw,new,mb,newcmp;
  //  __m128i TOP,BOT,THRES128;


#ifndef __SSE4_1__
  int* newcmp_int;
#endif

  llr_t m11,m10;


#ifdef DEBUG_LOGMAP
  msg("compute_alpha(%x,%x,%x,%d,%d,%d)\n",alpha,m_11,m10,frame_length,F,inst);
#endif
  
  THRES128 = _mm_set1_epi16(THRES);

  alpha128[0] = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,0);

  TOP = _mm_set_epi16(-1,1,1,-1,-1,1,1,-1);
  
  BOT = _mm_set_epi16(1,-1,-1,1,1,-1,-1,1);

  for (k=1;k<=F;k++)
    alpha128[k]=alpha128[0];

  alpha128+=F;
  //
  // compute log_alpha[k][m]
  // Steady-state portion


  for (k=F;k<frame_length+4;k++){
      // get 8 consecutive gammas

      //      m11_128=m_11_128[k];
      //      m10_128=m_10_128[k];

      m11=m_11[k];
      m10=m_10[k];
      //      msg("m11 %d, m10 %d\n",m11,m10);

      // First compute state transitions  (Think about LUT!)
      //      mtmp = m11_128;
      // Think about __mm_set1_epi16 instruction!!!!

      mtmp[inst]  = _mm_set1_epi16(m11);
      //      print_shorts("m11) mtmp",&mtmp);

      mtmp2[inst] = _mm_set1_epi16(m10);
      //      print_shorts("m10) mtmp2",&mtmp2);

      mtmp[inst] = _mm_unpacklo_epi32(mtmp[inst],mtmp2[inst]);
      //      print_shorts("unpacklo) mtmp",&mtmp);
      // mtmp = [m11(0) m11(0) m10(0) m10(0) m11(0) m11(0) m10(0) m10(0)]
      mtmp[inst] = _mm_shuffle_epi32(mtmp[inst],_MM_SHUFFLE(0,1,3,2));
      // mtmp = [m11(0) m11(0) m10(0) m10(0) m10(0) m10(0) m11(0) m11(0)]
      //      print_shorts("shuffle) mtmp",&mtmp);

      //mtop_g[inst][k] = _mm_xor_si128(mtmp[inst],mtmp[inst]);
      mtop_g[inst][k] = _mm_cmpgt_epi16(*(__m128i *)&zero[0],mtmp[inst]);
      mtop_g[inst][k] = _mm_sign_epi16(mtmp[inst],TOP);
      mbot_g[inst][k] = _mm_sign_epi16(mtmp[inst],BOT);

      //      print_shorts("mtop=",&mtop);
      //      msg("M0T %d, M1T %d, M2T %d, M3T %d, M4T %d, M5T %d, M6T %d, M7T %d\n",M0T,M1T,M2T,M3T,M4T,M5T,M6T,M7T);
      //      print_shorts("mbot=",&mbot);
      //      msg("M0B %d, M1B %d, M2B %d, M3B %d, M4B %d, M5B %d, M6B %d, M7B %d\n",M0B,M1B,M2B,M3B,M4B,M5B,M6B,M7B);


      // Now compute max-logmap

      lsw[inst]  = _mm_adds_epi16(*alpha128,mtop_g[inst][k]);
      msw[inst]  = _mm_adds_epi16(*alpha128,mbot_g[inst][k]);

      //      print_shorts("lsw=",&lsw);
      //      print_shorts("msw=",&msw);



      alpha128++;

      // lsw = [mb3 new3 mb2 new2 mb1 new1 mb0 new0] 
      // msw = [mb7 new7 mb6 new6 mb4 new4 mb3 new3] 

      lsw[inst] = _mm_shufflelo_epi16(lsw[inst],_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shufflelo) lsw=",&lsw);
      lsw[inst] = _mm_shufflehi_epi16(lsw[inst],_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shufflehi) lsw=",&lsw);

      msw[inst] = _mm_shufflelo_epi16(msw[inst],_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shufflelo) msw=",&msw);
      msw[inst] = _mm_shufflehi_epi16(msw[inst],_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shufflehi) msw=",&msw);

      // lsw = [mb3 mb2 new3 new2 mb1 mb0 new1 new0] 
      // msw = [mb7 mb6 new7 new6 mb4 mb3 new4 new3] 

      lsw[inst] = _mm_shuffle_epi32(lsw[inst],_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shuffle) lsw=",&lsw);
      msw[inst] = _mm_shuffle_epi32(msw[inst],_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shuffle) msw=",&msw);
      // lsw = [mb3 mb2 mb1 mb0 new3 new2 new1 new0] 
      // msw = [mb7 mb6 mb4 mb3 new7 new6 new4 new3] 

      new[inst] = _mm_unpacklo_epi64(lsw[inst],msw[inst]);
      mb[inst]  = _mm_unpackhi_epi64(lsw[inst],msw[inst]);
      // new = [new7 new6 new4 new3 new3 new2 new1 new0] 
      // mb = [mb7 mb6 mb4 mb3 mb3 mb2 mb1 mb0] 
      // Now both are in right order, so compute max



      //      *alpha128 = _mm_max_epi16(new,mb);
      
            
      new[inst] = _mm_max_epi16(new[inst],mb[inst]);
      newcmp[inst] = _mm_cmpgt_epi16(new[inst],THRES128);

#ifndef __SSE4_1__
      newcmp_int = (int*) &newcmp[inst];
      if (newcmp_int[0]==0 && newcmp_int[1]==0 && newcmp_int[2]==0 && newcmp_int[3]==0) // if any states above THRES normalize
#else
      if (_mm_testz_si128(newcmp[inst],newcmp[inst])) // if any states above THRES normalize
#endif
	*alpha128 = new[inst];
      else {
	//	print_shorts("new",&new);
	*alpha128 = _mm_subs_epi16(new[inst],THRES128);
	//	msg("alpha overflow %d",k);
	//	print_shorts(" ",alpha128);
      }
      
      //      print_shorts("alpha",alpha128);
	  
  }
  _mm_empty();
  _m_empty();
}

__m128i new[MAX_DECODING_THREADS],mb[MAX_DECODING_THREADS],oldh[MAX_DECODING_THREADS],oldl[MAX_DECODING_THREADS],THRES128,newcmp[MAX_DECODING_THREADS];

void compute_beta(llr_t* beta,llr_t *m_11,llr_t* m_10,llr_t* alpha,unsigned short frame_length,unsigned char F,unsigned char inst)
{
  int k;

  __m128i *beta128,*beta128_i;
  //  __m128i new,mb,oldh,oldl,THRES128,newcmp;

#ifndef __SSE4_1__
  int* newcmp_int;
#endif


#ifdef DEBUG_LOGMAP
  msg("compute_beta, %p,%p,%p,%p,framelength %d,F %d,inst %d\n",
      beta,m_11,m_10,alpha,frame_length,F,inst);
#endif

  THRES128 = _mm_set1_epi16(THRES);

  beta128   = (__m128i*)&beta[(frame_length+3)*STATES];
  beta128_i = (__m128i*)&beta[0];

  *beta128 = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,0);
  // Initialise zero state because of termination


  // set filler bit positions to 0 zero-state

  for (k=0;k<F;k++)
    beta128_i[k] = *beta128;


  for (k=(frame_length+2);k>=F;k--)
    {

      oldh[inst] = _mm_unpackhi_epi16(*beta128,*beta128);
      oldl[inst] = _mm_unpacklo_epi16(*beta128,*beta128);

      mb[inst]   = _mm_adds_epi16(oldh[inst],mbot_g[inst][k]);
      new[inst]  = _mm_adds_epi16(oldl[inst],mtop_g[inst][k]);

      beta128--;
      //      *beta128= _mm_max_epi16(new,mb);
            
      new[inst] = _mm_max_epi16(new[inst],mb[inst]);
      //      print_shorts("alpha128",alpha128);

      newcmp[inst] = _mm_cmpgt_epi16(new[inst],THRES128);

#ifndef __SSE4_1__
      newcmp_int = (int*) &newcmp[inst];
      if (newcmp_int[0]==0 && newcmp_int[1]==0 && newcmp_int[2]==0 && newcmp_int[3]==0) // if any states above THRES normalize
#else
      if (_mm_testz_si128(newcmp[inst],newcmp[inst]))
#endif
	*beta128 = new[inst];
      else{
	*beta128 = _mm_subs_epi16(new[inst],THRES128);
	//	msg("Beta overflow : %d\n",k);
      }
      
    }
  _mm_empty();
  _m_empty();
}

__m128i alpha_km1_top[MAX_DECODING_THREADS],alpha_km1_bot[MAX_DECODING_THREADS],alpha_k_top[MAX_DECODING_THREADS],alpha_k_bot[MAX_DECODING_THREADS],alphaloc_1[MAX_DECODING_THREADS],alphaloc_2[MAX_DECODING_THREADS],alphaloc_3[MAX_DECODING_THREADS],alphaloc_4[MAX_DECODING_THREADS];
__m128i alpha_beta_1[MAX_DECODING_THREADS],alpha_beta_2[MAX_DECODING_THREADS],alpha_beta_3[MAX_DECODING_THREADS],alpha_beta_4[MAX_DECODING_THREADS],alpha_beta_max04[MAX_DECODING_THREADS],alpha_beta_max15[MAX_DECODING_THREADS],alpha_beta_max26[MAX_DECODING_THREADS],alpha_beta_max37[MAX_DECODING_THREADS];
__m128i tmp0[MAX_DECODING_THREADS],tmp1[MAX_DECODING_THREADS],tmp2[MAX_DECODING_THREADS],tmp3[MAX_DECODING_THREADS],tmp00[MAX_DECODING_THREADS],tmp10[MAX_DECODING_THREADS],tmp20[MAX_DECODING_THREADS],tmp30[MAX_DECODING_THREADS];
__m128i m00_max[MAX_DECODING_THREADS],m01_max[MAX_DECODING_THREADS],m10_max[MAX_DECODING_THREADS],m11_max[MAX_DECODING_THREADS];

void compute_ext(llr_t* alpha,llr_t* beta,llr_t* m_11,llr_t* m_10,llr_t* ext, llr_t* systematic,unsigned short frame_length,unsigned char inst)
{
  int k;

  //  __m128i alpha_km1_top,alpha_km1_bot,alpha_k_top,alpha_k_bot,alpha_1,alpha_2,alpha_3,alpha_4;
  //  __m128i alpha_beta_1,alpha_beta_2,alpha_beta_3,alpha_beta_4,alpha_beta_max04,alpha_beta_max15,alpha_beta_max26,alpha_beta_max37;
  //  __m128i tmp0,tmp1,tmp2,tmp3,tmp00,tmp10,tmp20,tmp30;
  //  __m128i m00_max,m01_max,m10_max,m11_max;
  __m128i *alpha128=(__m128i *)alpha;
  __m128i *alpha128_ptr,*beta128_ptr;
  __m128i *beta128=(__m128i *)beta;
  __m128i *m11_128,*m10_128,*ext_128,*systematic_128;

  //
  // LLR computation, 8 consequtive bits per loop
  //

  //  msg("compute_ext, %p, %p, %p, %p, %p, %p ,framelength %d\n",alpha,beta,m11,m10,ext,systematic,framelength);
  for (k=0;k<(frame_length+3);k+=8)
    {

      alpha128_ptr = &alpha128[k];
      beta128_ptr  = &beta128[k+1];
      
      alpha128_ptr[0] = _mm_shufflelo_epi16(alpha128_ptr[0],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[0] = _mm_shufflehi_epi16(alpha128_ptr[0],_MM_SHUFFLE(1,3,0,2));
      alpha128_ptr[4]   = _mm_shufflelo_epi16(alpha128_ptr[4],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[4]   = _mm_shufflehi_epi16(alpha128_ptr[4],_MM_SHUFFLE(1,3,0,2));
      // these are [0 2 1 3 6 4 7 5] 
      //      print_shorts("a_km1",&alpha128_ptr[0]);
      
      alpha_km1_top[inst] = _mm_unpacklo_epi32(alpha128_ptr[0],alpha128_ptr[0]);
      alpha_km1_bot[inst] = _mm_unpackhi_epi32(alpha128_ptr[0],alpha128_ptr[0]);
      alpha_k_top[inst] = _mm_unpacklo_epi32(alpha128_ptr[4],alpha128_ptr[4]);
      alpha_k_bot[inst] = _mm_unpackhi_epi32(alpha128_ptr[4],alpha128_ptr[4]);
      // these are [0 2 0 2 1 3 1 3] and [6 4 6 4 7 5 7 5]
      //      print_shorts("a_km1_top",&alpha_km1_top);      
      //      print_shorts("a_km1_top",&alpha_km1_bot);      

      alphaloc_1[inst] = _mm_unpacklo_epi64(alpha_km1_top[inst],alpha_k_top[inst]);
      alphaloc_2[inst] = _mm_unpackhi_epi64(alpha_km1_top[inst],alpha_k_top[inst]);
      alphaloc_3[inst] = _mm_unpacklo_epi64(alpha_km1_bot[inst],alpha_k_bot[inst]);
      alphaloc_4[inst] = _mm_unpackhi_epi64(alpha_km1_bot[inst],alpha_k_bot[inst]);
      //      print_shorts("a_1",&alpha_1);      
      //      print_shorts("a_2",&alpha_2);      
      //      print_shorts("a_3",&alpha_3);      
      //      print_shorts("a_4",&alpha_4);      

      beta128_ptr[0] = _mm_shuffle_epi32(beta128_ptr[0],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[0] = _mm_shufflehi_epi16(beta128_ptr[0],_MM_SHUFFLE(2,3,0,1));
      beta128_ptr[4] = _mm_shuffle_epi32(beta128_ptr[4],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[4] = _mm_shufflehi_epi16(beta128_ptr[4],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 7 6 3 2]
      //      print_shorts("b",&beta128_ptr[0]);

      alpha_beta_1[inst]   = _mm_unpacklo_epi64(beta128_ptr[0],beta128_ptr[4]);
      //      print_shorts("ab_1",&alpha_beta_1);
      alpha_beta_2[inst]   = _mm_shuffle_epi32(alpha_beta_1[inst],_MM_SHUFFLE(2,3,0,1));
      //      print_shorts("ab_2",&alpha_beta_2);
      alpha_beta_3[inst]   = _mm_unpackhi_epi64(beta128_ptr[0],beta128_ptr[4]);
      alpha_beta_4[inst]   = _mm_shuffle_epi32(alpha_beta_3[inst],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 0 1 4 5] [4 5 0 1 4 5 0 1] [7 6 3 2 7 6 3 2] [3 2 7 6 3 2 7 6]
      alpha_beta_1[inst]   = _mm_adds_epi16(alpha_beta_1[inst],alphaloc_1[inst]);
      alpha_beta_2[inst]   = _mm_adds_epi16(alpha_beta_2[inst],alphaloc_2[inst]);
      alpha_beta_3[inst]   = _mm_adds_epi16(alpha_beta_3[inst],alphaloc_3[inst]);      
      alpha_beta_4[inst]   = _mm_adds_epi16(alpha_beta_4[inst],alphaloc_4[inst]);




      /*
	print_shorts("alpha_beta_1",&alpha_beta_1);
	print_shorts("alpha_beta_2",&alpha_beta_2);
	print_shorts("alpha_beta_3",&alpha_beta_3);
	print_shorts("alpha_beta_4",&alpha_beta_4);
	msg("m00: %d %d %d %d\n",m00_1,m00_2,m00_3,m00_4);
	msg("m10: %d %d %d %d\n",m10_1,m10_2,m10_3,m10_4);
	msg("m11: %d %d %d %d\n",m11_1,m11_2,m11_3,m11_4);
	msg("m01: %d %d %d %d\n",m01_1,m01_2,m01_3,m01_4);
      */
      alpha_beta_max04[inst] = _mm_max_epi16(alpha_beta_1[inst],alpha_beta_2[inst]);
      alpha_beta_max04[inst] = _mm_max_epi16(alpha_beta_max04[inst],alpha_beta_3[inst]);
      alpha_beta_max04[inst] = _mm_max_epi16(alpha_beta_max04[inst],alpha_beta_4[inst]);
      // these are the 4 mxy_1 below for k and k+4
      

      /*
	print_shorts("alpha_beta_max04",&alpha_beta_max04);
	msg("%d %d %d %d\n",m00_1,m10_1,m11_1,m01_1);
      */
      

      // bits 1 + 5

      alpha128_ptr[1] = _mm_shufflelo_epi16(alpha128_ptr[1],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[1] = _mm_shufflehi_epi16(alpha128_ptr[1],_MM_SHUFFLE(1,3,0,2));
      alpha128_ptr[5]   = _mm_shufflelo_epi16(alpha128_ptr[5],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[5]   = _mm_shufflehi_epi16(alpha128_ptr[5],_MM_SHUFFLE(1,3,0,2));
      // these are [0 2 1 3 6 4 7 5] 

      alpha_km1_top[inst] = _mm_unpacklo_epi32(alpha128_ptr[1],alpha128_ptr[1]);
      alpha_km1_bot[inst] = _mm_unpackhi_epi32(alpha128_ptr[1],alpha128_ptr[1]);
      alpha_k_top[inst] = _mm_unpacklo_epi32(alpha128_ptr[5],alpha128_ptr[5]);
      alpha_k_bot[inst] = _mm_unpackhi_epi32(alpha128_ptr[5],alpha128_ptr[5]);
      // these are [0 2 0 2 1 3 1 3] and [6 4 6 4 7 5 7 5]


      alphaloc_1[inst] = _mm_unpacklo_epi64(alpha_km1_top[inst],alpha_k_top[inst]);
      alphaloc_2[inst] = _mm_unpackhi_epi64(alpha_km1_top[inst],alpha_k_top[inst]);
      alphaloc_3[inst] = _mm_unpacklo_epi64(alpha_km1_bot[inst],alpha_k_bot[inst]);
      alphaloc_4[inst] = _mm_unpackhi_epi64(alpha_km1_bot[inst],alpha_k_bot[inst]);

      beta128_ptr[1] = _mm_shuffle_epi32(beta128_ptr[1],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[1] = _mm_shufflehi_epi16(beta128_ptr[1],_MM_SHUFFLE(2,3,0,1));
      beta128_ptr[5] = _mm_shuffle_epi32(beta128_ptr[5],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[5] = _mm_shufflehi_epi16(beta128_ptr[5],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 7 6 3 2]

      alpha_beta_1[inst]   = _mm_unpacklo_epi64(beta128_ptr[1],beta128_ptr[5]);
      alpha_beta_2[inst]   = _mm_shuffle_epi32(alpha_beta_1[inst],_MM_SHUFFLE(2,3,0,1));
      alpha_beta_3[inst]   = _mm_unpackhi_epi64(beta128_ptr[1],beta128_ptr[5]);
      alpha_beta_4[inst]   = _mm_shuffle_epi32(alpha_beta_3[inst],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 0 1 4 5] [4 5 0 1 4 5 0 1] [7 6 3 2 7 6 3 2] [3 2 7 6 3 2 7 6]
      alpha_beta_1[inst]   = _mm_adds_epi16(alpha_beta_1[inst],alphaloc_1[inst]);
      alpha_beta_2[inst]   = _mm_adds_epi16(alpha_beta_2[inst],alphaloc_2[inst]);
      alpha_beta_3[inst]   = _mm_adds_epi16(alpha_beta_3[inst],alphaloc_3[inst]);      
      alpha_beta_4[inst]   = _mm_adds_epi16(alpha_beta_4[inst],alphaloc_4[inst]);


      alpha_beta_max15[inst] = _mm_max_epi16(alpha_beta_1[inst],alpha_beta_2[inst]);
      alpha_beta_max15[inst] = _mm_max_epi16(alpha_beta_max15[inst],alpha_beta_3[inst]);
      alpha_beta_max15[inst] = _mm_max_epi16(alpha_beta_max15[inst],alpha_beta_4[inst]);

      // bits 2 + 6


      alpha128_ptr[2] = _mm_shufflelo_epi16(alpha128_ptr[2],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[2] = _mm_shufflehi_epi16(alpha128_ptr[2],_MM_SHUFFLE(1,3,0,2));
      alpha128_ptr[6]   = _mm_shufflelo_epi16(alpha128_ptr[6],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[6]   = _mm_shufflehi_epi16(alpha128_ptr[6],_MM_SHUFFLE(1,3,0,2));
      // these are [0 2 1 3 6 4 7 5] 

      alpha_km1_top[inst] = _mm_unpacklo_epi32(alpha128_ptr[2],alpha128_ptr[2]);
      alpha_km1_bot[inst] = _mm_unpackhi_epi32(alpha128_ptr[2],alpha128_ptr[2]);
      alpha_k_top[inst] = _mm_unpacklo_epi32(alpha128_ptr[6],alpha128_ptr[6]);
      alpha_k_bot[inst] = _mm_unpackhi_epi32(alpha128_ptr[6],alpha128_ptr[6]);
      // these are [0 2 0 2 1 3 1 3] and [6 4 6 4 7 5 7 5]


      alphaloc_1[inst] = _mm_unpacklo_epi64(alpha_km1_top[inst],alpha_k_top[inst]);
      alphaloc_2[inst] = _mm_unpackhi_epi64(alpha_km1_top[inst],alpha_k_top[inst]);
      alphaloc_3[inst] = _mm_unpacklo_epi64(alpha_km1_bot[inst],alpha_k_bot[inst]);
      alphaloc_4[inst] = _mm_unpackhi_epi64(alpha_km1_bot[inst],alpha_k_bot[inst]);

      beta128_ptr[2] = _mm_shuffle_epi32(beta128_ptr[2],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[2] = _mm_shufflehi_epi16(beta128_ptr[2],_MM_SHUFFLE(2,3,0,1));
      beta128_ptr[6] = _mm_shuffle_epi32(beta128_ptr[6],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[6] = _mm_shufflehi_epi16(beta128_ptr[6],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 7 6 3 2]

      alpha_beta_1[inst]   = _mm_unpacklo_epi64(beta128_ptr[2],beta128_ptr[6]);
      alpha_beta_2[inst]   = _mm_shuffle_epi32(alpha_beta_1[inst],_MM_SHUFFLE(2,3,0,1));
      alpha_beta_3[inst]   = _mm_unpackhi_epi64(beta128_ptr[2],beta128_ptr[6]);
      alpha_beta_4[inst]   = _mm_shuffle_epi32(alpha_beta_3[inst],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 0 1 4 5] [4 5 0 1 4 5 0 1] [7 6 3 2 7 6 3 2] [3 2 7 6 3 2 7 6]
      alpha_beta_1[inst]   = _mm_adds_epi16(alpha_beta_1[inst],alphaloc_1[inst]);
      alpha_beta_2[inst]   = _mm_adds_epi16(alpha_beta_2[inst],alphaloc_2[inst]);
      alpha_beta_3[inst]   = _mm_adds_epi16(alpha_beta_3[inst],alphaloc_3[inst]);      
      alpha_beta_4[inst]   = _mm_adds_epi16(alpha_beta_4[inst],alphaloc_4[inst]);
      /*
	print_shorts("alpha_beta_1",&alpha_beta_1);
	print_shorts("alpha_beta_2",&alpha_beta_2);
	print_shorts("alpha_beta_3",&alpha_beta_3);
	print_shorts("alpha_beta_4",&alpha_beta_4);
      */
      alpha_beta_max26[inst] = _mm_max_epi16(alpha_beta_1[inst],alpha_beta_2[inst]);
      alpha_beta_max26[inst] = _mm_max_epi16(alpha_beta_max26[inst],alpha_beta_3[inst]);
      alpha_beta_max26[inst] = _mm_max_epi16(alpha_beta_max26[inst],alpha_beta_4[inst]);

      
      //      print_shorts("alpha_beta_max26",&alpha_beta_max26);

      // bits 3 + 7

      alpha128_ptr[3] = _mm_shufflelo_epi16(alpha128_ptr[3],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[3] = _mm_shufflehi_epi16(alpha128_ptr[3],_MM_SHUFFLE(1,3,0,2));
      alpha128_ptr[7]   = _mm_shufflelo_epi16(alpha128_ptr[7],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[7]   = _mm_shufflehi_epi16(alpha128_ptr[7],_MM_SHUFFLE(1,3,0,2));
      // these are [0 2 1 3 6 4 7 5] 

      alpha_km1_top[inst] = _mm_unpacklo_epi32(alpha128_ptr[3],alpha128_ptr[3]);
      alpha_km1_bot[inst] = _mm_unpackhi_epi32(alpha128_ptr[3],alpha128_ptr[3]);
      alpha_k_top[inst] = _mm_unpacklo_epi32(alpha128_ptr[7],alpha128_ptr[7]);
      alpha_k_bot[inst] = _mm_unpackhi_epi32(alpha128_ptr[7],alpha128_ptr[7]);
      // these are [0 2 0 2 1 3 1 3] and [6 4 6 4 7 5 7 5]


      alphaloc_1[inst] = _mm_unpacklo_epi64(alpha_km1_top[inst],alpha_k_top[inst]);
      alphaloc_2[inst] = _mm_unpackhi_epi64(alpha_km1_top[inst],alpha_k_top[inst]);
      alphaloc_3[inst] = _mm_unpacklo_epi64(alpha_km1_bot[inst],alpha_k_bot[inst]);
      alphaloc_4[inst] = _mm_unpackhi_epi64(alpha_km1_bot[inst],alpha_k_bot[inst]);

      beta128_ptr[3] = _mm_shuffle_epi32(beta128_ptr[3],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[3] = _mm_shufflehi_epi16(beta128_ptr[3],_MM_SHUFFLE(2,3,0,1));
      beta128_ptr[7] = _mm_shuffle_epi32(beta128_ptr[7],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[7] = _mm_shufflehi_epi16(beta128_ptr[7],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 7 6 3 2]

      alpha_beta_1[inst]   = _mm_unpacklo_epi64(beta128_ptr[3],beta128_ptr[7]);
      alpha_beta_2[inst]   = _mm_shuffle_epi32(alpha_beta_1[inst],_MM_SHUFFLE(2,3,0,1));
      alpha_beta_3[inst]   = _mm_unpackhi_epi64(beta128_ptr[3],beta128_ptr[7]);
      alpha_beta_4[inst]   = _mm_shuffle_epi32(alpha_beta_3[inst],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 0 1 4 5] [4 5 0 1 4 5 0 1] [7 6 3 2 7 6 3 2] [3 2 7 6 3 2 7 6]
      alpha_beta_1[inst]   = _mm_adds_epi16(alpha_beta_1[inst],alphaloc_1[inst]);
      alpha_beta_2[inst]   = _mm_adds_epi16(alpha_beta_2[inst],alphaloc_2[inst]);
      alpha_beta_3[inst]   = _mm_adds_epi16(alpha_beta_3[inst],alphaloc_3[inst]);      
      alpha_beta_4[inst]   = _mm_adds_epi16(alpha_beta_4[inst],alphaloc_4[inst]);


      alpha_beta_max37[inst] = _mm_max_epi16(alpha_beta_1[inst],alpha_beta_2[inst]);
      alpha_beta_max37[inst] = _mm_max_epi16(alpha_beta_max37[inst],alpha_beta_3[inst]);
      alpha_beta_max37[inst] = _mm_max_epi16(alpha_beta_max37[inst],alpha_beta_4[inst]);

      // transpose alpha_beta matrix
      /*
	print_shorts("alpha_beta_max04",&alpha_beta_max04);
	print_shorts("alpha_beta_max15",&alpha_beta_max15);
	print_shorts("alpha_beta_max26",&alpha_beta_max26);
	print_shorts("alpha_beta_max37",&alpha_beta_max37);
      */
      tmp0[inst] = _mm_unpacklo_epi16(alpha_beta_max04[inst],alpha_beta_max15[inst]);
      tmp1[inst] = _mm_unpackhi_epi16(alpha_beta_max04[inst],alpha_beta_max15[inst]);
      tmp2[inst] = _mm_unpacklo_epi16(alpha_beta_max26[inst],alpha_beta_max37[inst]);
      tmp3[inst] = _mm_unpackhi_epi16(alpha_beta_max26[inst],alpha_beta_max37[inst]);


      tmp00[inst] = _mm_unpacklo_epi32(tmp0[inst],tmp2[inst]);
      tmp10[inst] = _mm_unpackhi_epi32(tmp0[inst],tmp2[inst]);
      tmp20[inst] = _mm_unpacklo_epi32(tmp1[inst],tmp3[inst]);
      tmp30[inst] = _mm_unpackhi_epi32(tmp1[inst],tmp3[inst]);


      m00_max[inst] = _mm_unpacklo_epi64(tmp00[inst],tmp20[inst]);
      m10_max[inst] = _mm_unpackhi_epi64(tmp00[inst],tmp20[inst]);
      m11_max[inst] = _mm_unpacklo_epi64(tmp10[inst],tmp30[inst]);
      m01_max[inst] = _mm_unpackhi_epi64(tmp10[inst],tmp30[inst]);

      /*
	print_shorts("m00_max",&m00_max);
	print_shorts("m01_max",&m01_max);
	print_shorts("m11_max",&m11_max);
	print_shorts("m10_max",&m10_max);
      */


      // compute extrinsics for 8 consecutive bits

      m11_128        = (__m128i*)&m_11[k];
      m10_128        = (__m128i*)&m_10[k];
      ext_128        = (__m128i*)&ext[k];
      systematic_128 = (__m128i*)&systematic[k];

      m11_max[inst] = _mm_adds_epi16(m11_max[inst],*m11_128);
      m10_max[inst] = _mm_adds_epi16(m10_max[inst],*m10_128);
      m00_max[inst] = _mm_subs_epi16(m00_max[inst],*m11_128);
      m01_max[inst] = _mm_subs_epi16(m01_max[inst],*m10_128);

      m01_max[inst] = _mm_max_epi16(m01_max[inst],m00_max[inst]);
      m10_max[inst] = _mm_max_epi16(m11_max[inst],m10_max[inst]);

      //      print_shorts("m01_max",&m01_max);
      //      print_shorts("m10_max",&m10_max);

       
   
      *ext_128 = _mm_subs_epi16(m10_max[inst],_mm_adds_epi16(m01_max[inst],*systematic_128));
      /*
      if ((((short *)ext_128)[0] > 8192) ||
	  (((short *)ext_128)[1] > 8192) ||
	  (((short *)ext_128)[2] > 8192) ||
	  (((short *)ext_128)[3] > 8192) ||
	  (((short *)ext_128)[4] > 8192) ||
	  (((short *)ext_128)[5] > 8192) ||
	  (((short *)ext_128)[6] > 8192) ||
	  (((short *)ext_128)[7] > 8192)) {
	msg("ext overflow %d:",k);
	print_shorts("**ext_128",ext_128);
      }
      */

    }

  _mm_empty();
  _m_empty();

}

short systematic0_0[6144+16] __attribute__ ((aligned(16)));
short systematic1_0[6144+16] __attribute__ ((aligned(16)));
short systematic2_0[6144+16]__attribute__ ((aligned(16)));
short yparity1_0[6144+8] __attribute__ ((aligned(16)));
short yparity2_0[6144+8] __attribute__ ((aligned(16)));
short systematic0_1[6144+16] __attribute__ ((aligned(16)));
short systematic1_1[6144+16] __attribute__ ((aligned(16)));
short systematic2_1[6144+16]__attribute__ ((aligned(16)));
short yparity1_1[6144+8] __attribute__ ((aligned(16)));
short yparity2_1[6144+8] __attribute__ ((aligned(16)));
short systematic0_2[6144+16] __attribute__ ((aligned(16)));
short systematic1_2[6144+16] __attribute__ ((aligned(16)));
short systematic2_2[6144+16]__attribute__ ((aligned(16)));
short yparity1_2[6144+8] __attribute__ ((aligned(16)));
short yparity2_2[6144+8] __attribute__ ((aligned(16)));
short systematic0_3[6144+16] __attribute__ ((aligned(16)));
short systematic1_3[6144+16] __attribute__ ((aligned(16)));
short systematic2_3[6144+16]__attribute__ ((aligned(16)));
short yparity1_3[6144+8] __attribute__ ((aligned(16)));
short yparity2_3[6144+8] __attribute__ ((aligned(16)));
short systematic0_4[6144+16] __attribute__ ((aligned(16)));
short systematic1_4[6144+16] __attribute__ ((aligned(16)));
short systematic2_4[6144+16]__attribute__ ((aligned(16)));
short yparity1_4[6144+8] __attribute__ ((aligned(16)));
short yparity2_4[6144+8] __attribute__ ((aligned(16)));
short *systematic0_g[MAX_DECODING_THREADS] = { systematic0_0, systematic0_1, systematic0_2, systematic0_3, systematic0_4};
short *systematic1_g[MAX_DECODING_THREADS] = { systematic1_0, systematic1_1, systematic1_2, systematic1_3, systematic1_4};
short *systematic2_g[MAX_DECODING_THREADS] = { systematic2_0, systematic2_1, systematic2_2, systematic2_3, systematic2_4};
short *yparity1_g[MAX_DECODING_THREADS] = {yparity1_0, yparity1_1, yparity1_2, yparity1_3, yparity1_4};
short *yparity2_g[MAX_DECODING_THREADS] = {yparity2_0, yparity2_1, yparity2_2, yparity2_3, yparity2_4};

unsigned char decoder_in_use[MAX_DECODING_THREADS] = {0,0,0,0,0};

unsigned char phy_threegpplte_turbo_decoder(llr_t *y,
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
  short ext[n],ext2[n];

  //  short systematic0[n],systematic1[n],systematic2[n],yparity1[n],yparity2[n];
  llr_t *yp = y;
  unsigned short i,pi;
  unsigned char iteration_cnt=0;
  unsigned int crc,oldcrc,crc_len;
  u8 temp;

  if (crc_type > 3) {
    msg("Illegal crc length!\n");
    return 255;
  }

  if (inst>=MAX_DECODING_THREADS) {
    msg("inst>=MAX_DECODING_THREADS\n");
    return(255);
  }

  if (decoder_in_use[inst]) {
    msg("turbo decoder already in use\n");
    return 255;
  }
  else {
    decoder_in_use[inst] = 1;
  }
  
  // zero out all global variables
  bzero(alpha_g[inst],(FRAME_LENGTH_MAX+3+1)*8*sizeof(llr_t));
  bzero(beta_g[inst],(FRAME_LENGTH_MAX+3+1)*8*sizeof(llr_t));
  bzero(m11_g[inst],(FRAME_LENGTH_MAX+3)*sizeof(llr_t));
  bzero(m10_g[inst],(FRAME_LENGTH_MAX+3)*sizeof(llr_t));
  bzero(systematic0_g[inst],(6144+16)*sizeof(short));
  bzero(systematic1_g[inst],(6144+16)*sizeof(short));
  bzero(systematic2_g[inst],(6144+16)*sizeof(short));
  bzero(yparity1_g[inst],(6144+8)*sizeof(short));
  bzero(yparity2_g[inst],(6144+8)*sizeof(short));

  bzero(mtop_g[inst],6144*sizeof(__m128i));
  bzero(mbot_g[inst],6144*sizeof(__m128i));

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
  for (i=0;i<n;i++) {
    systematic0_g[inst][i] = *yp; yp++;
    yparity1_g[inst][i] = *yp; yp++;
    yparity2_g[inst][i] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    msg("Position %d: (%d,%d,%d)\n",i,systematic0_g[inst][i],yparity1_g[inst][i],yparity2_g[inst][i]);
#endif //DEBUG_LOGMAP

  }
  for (i=n;i<n+3;i++) {
    systematic0_g[inst][i]= *yp; yp++;
    yparity1_g[inst][i] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    msg("Term 1 (%d): %d %d\n",i,systematic0_g[inst][i],yparity1_g[inst][i]);
#endif //DEBUG_LOGMAP
  }
  for (i=n+8;i<n+11;i++) {
    systematic0_g[inst][i]= *yp; yp++;
    yparity2_g[inst][i-8] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    msg("Term 2 (%d): %d %d\n",i-3,systematic0_g[inst][i],yparity2_g[inst][i-3]);
#endif //DEBUG_LOGMAP
  }
#ifdef DEBUG_LOGMAP
  msg("\n");
#endif //DEBUG_LOGMAP



  // do log_map from first parity bit
  log_map(systematic0_g[inst],yparity1_g[inst],ext,n,0,F,inst);

  while (iteration_cnt++ < max_iterations) {

#ifdef DEBUG_LOGMAP
   msg("\n*******************ITERATION %d\n\n",iteration_cnt);
#endif //DEBUG_LOGMAP

    threegpplte_interleaver_reset();
    pi=0;

    // compute input to second encoder by interleaving extrinsic info
    for (i=0;i<n;i++) { // steady-state portion
      systematic2_g[inst][i] = (ext[pi] + systematic0_g[inst][pi]);
      pi              = threegpplte_interleaver(f1,f2,n);
    }
    for (i=n;i<n+3;i++) { // termination
      systematic2_g[inst][i] = (systematic0_g[inst][i+8]);
    }
    // do log_map from second parity bit    
    log_map(systematic2_g[inst],yparity2_g[inst],ext2,n,1,0,inst);


    threegpplte_interleaver_reset();
    pi=0;
    for (i=0;i<n>>3;i++)
      decoded_bytes[i]=0;
    // compute input to first encoder and output
    for (i=0;i<n;i++) {
      systematic1_g[inst][pi] = (ext2[i] + systematic0_g[inst][pi]);
#ifdef DEBUG_LOGMAP
      msg("Second half %d: ext2[i] %d, systematic0[i] %d (e+s %d)\n",i,ext2[i],systematic0_g[inst][pi],
	     ext2[i]+systematic2_g[inst][i]);
#endif //DEBUG_LOGMAP

      if ((systematic2_g[inst][i] + ext2[i]) > 0)
	decoded_bytes[pi>>3] += (1 << (7-(pi&7)));

      pi              = threegpplte_interleaver(f1,f2,n);
    }
    
    for (i=n;i<n+3;i++) {
      systematic1_g[inst][i] = (systematic0_g[inst][i]);
#ifdef DEBUG_LOGMAP
      msg("Second half %d: ext2[i] %d, systematic0[i] %d (e+s %d)\n",i,ext2[i],systematic0_g[inst][i],
	     ext2[i]+systematic2_g[inst][i]);
#endif //DEBUG_LOGMAP
    }
    

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

      //           msg("CRC24_A = %x, oldcrc = %x (F %d)\n",crc,oldcrc,F);

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
    }

    if ((crc == oldcrc) && (crc!=0)) {
      decoder_in_use[inst] = 0;
      return(iteration_cnt);
    }

    // do log_map from first parity bit
    if (iteration_cnt < max_iterations)
      log_map(systematic1_g[inst],yparity1_g[inst],ext,n,0,F,inst);
  }

  decoder_in_use[inst] = 0;
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

  printf("Number of iterations %d\n",ret);
  for (i=0;i<8;i++)
    printf("output %d => %x (input %x)\n",i,decoded_output[i],test[i]);
}




int main() {


  test_logmap8();

  return(0);
}

#endif // TEST_DEBUG


