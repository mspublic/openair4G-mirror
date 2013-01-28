/* file: 3gpplte_turbo_decoder_sse.c
   purpose: Routines for implementing max-logmap decoding of Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 

   Note: This routine currently requires SSE2,SSSE3 and SSE4.1 equipped computers.  IT WON'T RUN OTHERWISE!
  
   Changelog: 17.11.2009 FK SSE4.1 not required anymore
              Aug. 2012 new parallelization options for higher speed
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

static short zero[8]  __attribute__ ((aligned(16))) = {0,0,0,0,0,0,0,0};

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




#define MAX 256//16383
#define THRES 8192

#define FRAME_LENGTH_MAX 6144+16
#define STATES 8

#define MAX_DECODING_THREADS 5 //4 for DLSCH, 1 for PBCH

//#define DEBUG_LOGMAP

#define NEW_IMPL 1
 
#if NEW_IMPL==2
typedef char llr_t; // internal decoder data is 16-bit fixed
typedef char channel_t;
#else
typedef short llr_t; // internal decoder data is 16-bit fixed
typedef short channel_t;
#endif
void log_map (llr_t* systematic,channel_t* y_parity, llr_t* ext,unsigned short frame_length,unsigned char term_flag,unsigned char F,unsigned char inst);
void compute_gamma(llr_t* m11,llr_t* m10,llr_t* systematic, channel_t* y_parity, unsigned short frame_length,unsigned char term_flag);
void compute_alpha(llr_t*alpha,llr_t *beta, llr_t* m11,llr_t* m10, unsigned short frame_length,unsigned char F,unsigned char inst,unsigned char rerun_flag);
void compute_beta(llr_t*alpha, llr_t* beta,llr_t* m11,llr_t* m10, unsigned short frame_length,unsigned char F,unsigned char inst,unsigned char rerun_flag);
void compute_ext(llr_t* alpha,llr_t* beta,llr_t* m11,llr_t* m10,llr_t* extrinsic, llr_t* ap, unsigned short frame_length,unsigned char inst);

// global variables
//
llr_t alpha_0[(FRAME_LENGTH_MAX)*8] __attribute__ ((aligned(16)));
llr_t beta_0[(FRAME_LENGTH_MAX)*8] __attribute__ ((aligned(16)));
llr_t m11_0[(FRAME_LENGTH_MAX)] __attribute__ ((aligned(16)));
llr_t m10_0[(FRAME_LENGTH_MAX)] __attribute__ ((aligned(16)));
llr_t alpha_1[(FRAME_LENGTH_MAX)*8] __attribute__ ((aligned(16)));
llr_t beta_1[(FRAME_LENGTH_MAX)*8] __attribute__ ((aligned(16)));
llr_t m11_1[(FRAME_LENGTH_MAX)] __attribute__ ((aligned(16)));
llr_t m10_1[(FRAME_LENGTH_MAX)] __attribute__ ((aligned(16)));
llr_t alpha_2[(FRAME_LENGTH_MAX)*8] __attribute__ ((aligned(16)));
llr_t beta_2[(FRAME_LENGTH_MAX)*8] __attribute__ ((aligned(16)));
llr_t m11_2[(FRAME_LENGTH_MAX)] __attribute__ ((aligned(16)));
llr_t m10_2[(FRAME_LENGTH_MAX)] __attribute__ ((aligned(16)));
llr_t alpha_3[(FRAME_LENGTH_MAX)*8] __attribute__ ((aligned(16)));
llr_t beta_3[(FRAME_LENGTH_MAX)*8] __attribute__ ((aligned(16)));
llr_t m11_3[(FRAME_LENGTH_MAX)] __attribute__ ((aligned(16)));
llr_t m10_3[(FRAME_LENGTH_MAX)] __attribute__ ((aligned(16)));
llr_t alpha_4[(FRAME_LENGTH_MAX)*8] __attribute__ ((aligned(16)));
llr_t beta_4[(FRAME_LENGTH_MAX)*8] __attribute__ ((aligned(16)));
llr_t m11_4[(FRAME_LENGTH_MAX)] __attribute__ ((aligned(16)));
llr_t m10_4[(FRAME_LENGTH_MAX)] __attribute__ ((aligned(16)));
llr_t *alpha_g[MAX_DECODING_THREADS] = {alpha_0, alpha_1, alpha_2, alpha_3, alpha_4};
llr_t *beta_g[MAX_DECODING_THREADS] = {beta_0, beta_1, beta_2, beta_3, beta_4};
llr_t *m11_g[MAX_DECODING_THREADS] = {m11_0, m11_1, m11_2, m11_3, m11_4};
llr_t *m10_g[MAX_DECODING_THREADS] = {m10_0, m10_1, m10_2, m10_3, m10_4};


void log_map(llr_t* systematic,channel_t* y_parity, llr_t* ext,unsigned short frame_length,unsigned char term_flag,unsigned char F,unsigned char inst) {

#ifdef DEBUG_LOGMAP
  msg("log_map, frame_length %d\n",frame_length);
#endif


  compute_gamma(m11_g[inst],m10_g[inst],systematic,y_parity,frame_length,term_flag);

  //  printf("Alpha 1\n");
  compute_alpha(alpha_g[inst],beta_g[inst],m11_g[inst],m10_g[inst],frame_length,F,inst,0);

  //  printf("Alpha 2\n");
  if (NEW_IMPL>0)  
    compute_alpha(alpha_g[inst],beta_g[inst],m11_g[inst],m10_g[inst],frame_length,F,inst,1);

  //  printf("beta (term): %d,%d, %d,%d, %d,%d\n",m11_g[inst][frame_length],m10_g[inst][frame_length],m11_g[inst][1+frame_length],m10_g[inst][1+frame_length],m11_g[inst][2+frame_length],m10_g[inst][2+frame_length]);

  compute_beta(alpha_g[inst],beta_g[inst],m11_g[inst],m10_g[inst],frame_length,F,inst,0);
  
  //  printf("beta (term): %d,%d, %d,%d, %d,%d\n",m11_g[inst][frame_length],m10_g[inst][frame_length],m11_g[inst][1+frame_length],m10_g[inst][1+frame_length],m11_g[inst][2+frame_length],m10_g[inst][2+frame_length]);
  if (NEW_IMPL>0)  
    compute_beta(alpha_g[inst],beta_g[inst],m11_g[inst],m10_g[inst],frame_length,F,inst,1);
  

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

  //  printf("gamma (term): %d,%d, %d,%d, %d,%d\n",m11[k<<3],m10[k<<3],m11[1+(k<<3)],m10[1+(k<<3)],m11[2+(k<<3)],m10[2+(k<<3)]);
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

__m128i mtmp[MAX_DECODING_THREADS] __attribute__ ((aligned(16)));
__m128i mtmp2[MAX_DECODING_THREADS] __attribute__ ((aligned(16)));
__m128i lsw[MAX_DECODING_THREADS] __attribute__ ((aligned(16)));
__m128i msw[MAX_DECODING_THREADS] __attribute__ ((aligned(16)));
__m128i new[MAX_DECODING_THREADS] __attribute__ ((aligned(16)));
__m128i mb[MAX_DECODING_THREADS] __attribute__ ((aligned(16)));
__m128i newcmp[MAX_DECODING_THREADS] __attribute__ ((aligned(16)));
__m128i TOP,BOT,THRES128;

#define L 40

void compute_alpha(llr_t* alpha,llr_t* beta,llr_t* m_11,llr_t* m_10,unsigned short frame_length,unsigned char F,unsigned char inst,unsigned char rerun_flag)
{
  int k,K1;
  __m128i *alpha128=(__m128i *)alpha,*alpha_ptr;
  __m128i m11_0,m10_0;
  __m128i m_b0,m_b1,m_b2,m_b3,m_b4,m_b5,m_b6,m_b7;
  __m128i new0,new1,new2,new3,new4,new5,new6,new7;
  __m128i alpha_max;
  //  __m128i mtmp,mtmp2,lsw,msw,new,mb,newcmp;
  //  __m128i TOP,BOT,THRES128;


#ifndef __SSE4_1__
  int* newcmp_int;
#endif

  llr_t m11,m10;


  if (NEW_IMPL == 0) { 
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
      
    }
      //      print_shorts("alpha",alpha128);
	  
  }
  else {

    K1 = (frame_length>>3);
    
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
      /*
      alpha128[0] = beta128[0];
      alpha128[1] = beta128[1];
      alpha128[2] = beta128[2];
      alpha128[3] = beta128[3];
      alpha128[4] = beta128[4];
      alpha128[5] = beta128[5];
      alpha128[6] = beta128[6];
      alpha128[7] = beta128[7];
      */
      //      alpha[0] = 0;
      

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
	 k<((rerun_flag == 0) ? K1 : L);
	 k++){

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
      
                  
      //      printf("alpha k %d (%d) (%p)\n",k,k<<3,alpha_ptr);
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
  }
  _mm_empty();
  _m_empty();
}

__m128i new[MAX_DECODING_THREADS],mb[MAX_DECODING_THREADS],oldh[MAX_DECODING_THREADS],oldl[MAX_DECODING_THREADS],THRES128,newcmp[MAX_DECODING_THREADS];

void compute_beta(llr_t* alpha,llr_t* beta,llr_t *m_11,llr_t* m_10,unsigned short frame_length,unsigned char F,unsigned char inst,unsigned char rerun_flag) {
  
  int k;
  __m128i m11_128,m10_128;
  __m128i m_b0,m_b1,m_b2,m_b3,m_b4,m_b5,m_b6,m_b7;
  __m128i new0,new1,new2,new3,new4,new5,new6,new7;

  __m128i *beta128,*alpha128,*beta128_i,*beta_ptr;
  //  __m128i new,mb,oldh,oldl,THRES128,newcp;
  __m128i beta_max; 
  llr_t m11,m10,beta0,beta1,beta2,beta3,beta4,beta5,beta6,beta7,beta0_2,beta1_2,beta2_2,beta3_2,beta_m; 
 

#ifndef __SSE4_1__
  int* newcmp_int;
#endif


#ifdef DEBUG_LOGMAP
  msg("compute_beta, %p,%p,%p,%p,framelength %d,F %d,inst %d\n",
      beta,m_11,m_10,alpha,frame_length,F,inst);
#endif

  if (NEW_IMPL==0) { //(frame_length < SHORT_LENGTH_CW) {
    THRES128 = _mm_set1_epi16(THRES);
    
    beta128   = (__m128i*)&beta[(frame_length+3)*STATES];
    beta128_i = (__m128i*)&beta[0];
    
    *beta128 = _mm_set_epi16(-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,-MAX/2,0);
    // Initialise zero state because of termination
    
    
    // set filler bit positions to 0 zero-state
    
    for (k=0;k<F;k++)
      beta128_i[k] = *beta128;
    
    // For from termination bits (same trellis for BETA) downto first filler bit
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
	/*
	printf("k: %d (m11 %d, m10 %d)\n",k,m_11[k],m_10[k]);
	print_shorts("mbot:",&mbot_g[inst][k]);
	print_shorts("mtop:",&mtop_g[inst][k]);
	print_shorts("beta:",beta128);
	*/
      }
  }
  else {

    // termination for beta initialization

 
    m11=m_11[2+frame_length];
    m10=m_10[2+frame_length];

    beta0 = -m11;//M0T_TERM;
    beta1 = m11;//M1T_TERM;
    m11=m_11[1+frame_length];
    m10=m_10[1+frame_length];

    beta0_2 = beta0-m11;//+M0T_TERM;
    beta1_2 = beta0+m11;//+M1T_TERM;
    beta2_2 = beta1+m10;//M2T_TERM;
    beta3_2 = beta1-m10;//+M3T_TERM;
    m11=m_11[frame_length];
    m10=m_10[frame_length];

    beta0 = beta0_2-m11;//+M0T_TERM;
    beta1 = beta0_2+m11;//+M1T_TERM;
    beta2 = beta1_2+m10;//+M2T_TERM;
    beta3 = beta1_2-m10;//+M3T_TERM;
    beta4 = beta2_2-m10;//+M4T_TERM;
    beta5 = beta2_2+m10;//+M5T_TERM;
    beta6 = beta3_2+m11;//+M6T_TERM;
    beta7 = beta3_2-m11;//+M7T_TERM;
    beta_m = (beta0>beta1) ? beta0 : beta1;
    beta_m = (beta_m>beta2) ? beta_m : beta2;
    beta_m = (beta_m>beta3) ? beta_m : beta3;
    beta_m = (beta_m>beta4) ? beta_m : beta4;
    beta_m = (beta_m>beta5) ? beta_m : beta5;
    beta_m = (beta_m>beta6) ? beta_m : beta6;
    beta_m = (beta_m>beta7) ? beta_m : beta7;
    beta0=beta0-beta_m;
    beta1=beta1-beta_m;
    beta2=beta2-beta_m;
    beta3=beta3-beta_m;
    beta4=beta4-beta_m;
    beta5=beta5-beta_m;
    beta6=beta6-beta_m;
    beta7=beta7-beta_m;
    //    printf("After term, beta %d,%d,%d,%d,%d,%d,%d,%d\n",beta0,beta1,beta2,beta3,beta4,beta5,beta6,beta7);
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
    
    beta[7+(frame_length<<3)] = beta0;
    beta[15+(frame_length<<3)] = beta1;
    beta[23+(frame_length<<3)] = beta2;
    beta[31+(frame_length<<3)] = beta3;
    beta[39+(frame_length<<3)] = beta4;
    beta[47+(frame_length<<3)] = beta5;
    beta[55+(frame_length<<3)] = beta6;
    beta[63+(frame_length<<3)] = beta7;
    /*
    beta[(frame_length<<3)] = beta0;
    beta[8+(frame_length<<3)] = beta1;
    beta[16+(frame_length<<3)] = beta2;
    beta[24+(frame_length<<3)] = beta3;
    beta[32+(frame_length<<3)] = beta4;
    beta[40+(frame_length<<3)] = beta5;
    beta[48+(frame_length<<3)] = beta6;
    beta[56+(frame_length<<3)] = beta7;
    */
    /*
    printf("beta k %d (alpha %p)\n",frame_length>>3,&alpha128[(frame_length)]);
    print_shorts("b0:",&beta_ptr[0]);
    print_shorts("b1:",&beta_ptr[1]);
    print_shorts("b2:",&beta_ptr[2]);
    print_shorts("b3:",&beta_ptr[3]);
    print_shorts("b4:",&beta_ptr[4]);
    print_shorts("b5:",&beta_ptr[5]);
    print_shorts("b6:",&beta_ptr[6]);
    print_shorts("b7:",&beta_ptr[7]);      
    */

    
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
      /*
      printf("\nbeta k %d (%d)\n",k,(unsigned int)beta_ptr-(unsigned int)beta);
      if (rerun_flag == 1) {
	int i,j;
	for (j=0;j<8;j++) {
	  for (i=0;i<8;i++) {
	    if (((int16_t*)&beta_ptr[i])[j] == 0)
	      printf("state %d in pos %d, beta %d\n", i, k+(j*(frame_length>>3)),((int16_t*)&beta_ptr[i])[j]);
	  }
	}
      } 
      */     
      /*
      printf("beta k %d (%d)\n",k,(unsigned int)beta_ptr-(unsigned int)beta);
      print_shorts("m11:",&m11_128);
      print_shorts("m10:",&m10_128);
      print_shorts("b0:",&beta_ptr[0]);
      print_shorts("b1:",&beta_ptr[1]);
      print_shorts("b2:",&beta_ptr[2]);
      print_shorts("b3:",&beta_ptr[3]);
      print_shorts("b4:",&beta_ptr[4]);
      print_shorts("b5:",&beta_ptr[5]);
      print_shorts("b6:",&beta_ptr[6]);
      print_shorts("b7:",&beta_ptr[7]);            

      printf("beta k %d (%d)\n",k,(unsigned int)beta_ptr-(unsigned int)beta);
      print_shorts("m11:",&m11_128);
      print_shorts("m10:",&m10_128);
      print_shorts("b0:",&beta_ptr[0]);
      print_shorts("b1:",&beta_ptr[1]);
      print_shorts("b2:",&beta_ptr[2]);
      print_shorts("b3:",&beta_ptr[3]);
      print_shorts("b4:",&beta_ptr[4]);
      print_shorts("b5:",&beta_ptr[5]);
      print_shorts("b6:",&beta_ptr[6]);
      print_shorts("b7:",&beta_ptr[7]);      
      */
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
  __m128i *alpha128=(__m128i *)alpha;
  __m128i *alpha128_ptr,*beta128_ptr;
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

  //  msg("compute_ext, %p, %p, %p, %p, %p, %p ,framelength %d\n",alpha,beta,m11,m10,ext,systematic,framelength);

  if (NEW_IMPL==0) {
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
	//	print_shorts("ext:",ext_128);
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
  }
  else {


    alpha_ptr = alpha128;
    beta_ptr = &beta128[8];


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


  }
  _mm_empty();
  _m_empty();

}

llr_t systematic0_0[6144+16] __attribute__ ((aligned(16)));
llr_t systematic1_0[6144+16] __attribute__ ((aligned(16)));
llr_t systematic2_0[6144+16]__attribute__ ((aligned(16)));
llr_t yparity1_0[6144+8] __attribute__ ((aligned(16)));
llr_t yparity2_0[6144+8] __attribute__ ((aligned(16)));
llr_t systematic0_1[6144+16] __attribute__ ((aligned(16)));
llr_t systematic1_1[6144+16] __attribute__ ((aligned(16)));
llr_t systematic2_1[6144+16]__attribute__ ((aligned(16)));
llr_t yparity1_1[6144+8] __attribute__ ((aligned(16)));
llr_t yparity2_1[6144+8] __attribute__ ((aligned(16)));
llr_t systematic0_2[6144+16] __attribute__ ((aligned(16)));
llr_t systematic1_2[6144+16] __attribute__ ((aligned(16)));
llr_t systematic2_2[6144+16]__attribute__ ((aligned(16)));
llr_t yparity1_2[6144+8] __attribute__ ((aligned(16)));
llr_t yparity2_2[6144+8] __attribute__ ((aligned(16)));
llr_t systematic0_3[6144+16] __attribute__ ((aligned(16)));
llr_t systematic1_3[6144+16] __attribute__ ((aligned(16)));
llr_t systematic2_3[6144+16]__attribute__ ((aligned(16)));
llr_t yparity1_3[6144+8] __attribute__ ((aligned(16)));
llr_t yparity2_3[6144+8] __attribute__ ((aligned(16)));
llr_t systematic0_4[6144+16] __attribute__ ((aligned(16)));
llr_t systematic1_4[6144+16] __attribute__ ((aligned(16)));
llr_t systematic2_4[6144+16]__attribute__ ((aligned(16)));
llr_t yparity1_4[6144+8] __attribute__ ((aligned(16)));
llr_t yparity2_4[6144+8] __attribute__ ((aligned(16)));
llr_t *systematic0_g[MAX_DECODING_THREADS] = { systematic0_0, systematic0_1, systematic0_2, systematic0_3, systematic0_4};
llr_t *systematic1_g[MAX_DECODING_THREADS] = { systematic1_0, systematic1_1, systematic1_2, systematic1_3, systematic1_4};
llr_t *systematic2_g[MAX_DECODING_THREADS] = { systematic2_0, systematic2_1, systematic2_2, systematic2_3, systematic2_4};
llr_t *yparity1_g[MAX_DECODING_THREADS] = {yparity1_0, yparity1_1, yparity1_2, yparity1_3, yparity1_4};
llr_t *yparity2_g[MAX_DECODING_THREADS] = {yparity2_0, yparity2_1, yparity2_2, yparity2_3, yparity2_4};

unsigned char decoder_in_use[MAX_DECODING_THREADS] = {0,0,0,0,0};

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
    llr_t ext[n+128] __attribute__((aligned(16)));
    llr_t ext2[n+128] __attribute__((aligned(16)));
  unsigned int pi[n],*pi_p,*pi2_p,*pi3_p,pi2[n],pi3[n];
  //  short systematic0[n],systematic1[n],systematic2[n],yparity1[n],yparity2[n];
  llr_t *yp = y;
  unsigned int i,j;//,pi;
  unsigned char iteration_cnt=0;
  unsigned int crc,oldcrc,crc_len;
  u8 temp;
  llr_t tmp2[n],*t_p,*s_p;;
  int byte_pos[n],bit_pos[n],*byte_pos_p,*bit_pos_p;


  if (crc_type > 3) {
    msg("Illegal crc length!\n");
    return 255;
  }

  if (inst>=MAX_DECODING_THREADS) {
    msg("inst>=MAX_DECODING_THREADS\n");
    return(255);
  }

  if (decoder_in_use[inst]) {
    msg("turbo decoder for inst %d already in use\n",inst);
    return 255;
  }
  else {
    //msg("setting turbo decoder inst %d to 1\n",inst);
    decoder_in_use[inst] = 1;
  }
  
  // zero out all global variables
  bzero(alpha_g[inst],(FRAME_LENGTH_MAX)*8*sizeof(llr_t));
  bzero(beta_g[inst],(FRAME_LENGTH_MAX)*8*sizeof(llr_t));
  bzero(m11_g[inst],(FRAME_LENGTH_MAX)*sizeof(llr_t));
  bzero(m10_g[inst],(FRAME_LENGTH_MAX)*sizeof(llr_t));
  bzero(systematic0_g[inst],(6144+16)*sizeof(short));
  bzero(systematic1_g[inst],(6144+16)*sizeof(short));
  bzero(systematic2_g[inst],(6144+16)*sizeof(short));
  bzero(yparity1_g[inst],(6144+8)*sizeof(short));
  bzero(yparity2_g[inst],(6144+8)*sizeof(short));

  bzero(mtop_g[inst],6144*sizeof(__m128i));
  bzero(mbot_g[inst],6144*sizeof(__m128i));

  threegpplte_interleaver_reset();
  pi[0] = 0;
  bit_pos[0] = 128;
  byte_pos[0] = 0;

  for (i=1;i<n;i++) {
    pi[i] = (unsigned int)threegpplte_interleaver(f1,f2,n);
    bit_pos[i] = 128>>(pi[i]&7);
    byte_pos[i] = pi[i]>>3;
  }
  for (j=0,i=0;i<n;i++,j+=8) {

    if (j>=n)
      j-=(n-1);
  
    pi2[i] = j;
    //    printf("pi2[%d] = %d\n",i,j);
  }

  for (i=0;i<n;i++) {
    pi3[i] = pi2[pi[i]];
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

  if (NEW_IMPL==0) {
    for (i=0;i<n;i++) {
      systematic0_g[inst][i] = *yp; yp++;
      yparity1_g[inst][i] = *yp; yp++;
      yparity2_g[inst][i] = *yp; yp++;
#ifdef DEBUG_LOGMAP
      msg("Position %d: (%d,%d,%d)\n",i,systematic0_g[inst][i],yparity1_g[inst][i],yparity2_g[inst][i]) ;
#endif //DEBUG_LOGMAP
      
    }
  }
  else {  // interleave input 8/16-way
    for (i=0;i<n;i+=8) {
      pi2_p = &pi2[i];

      j=pi2_p[0];
      /*
      systematic0_g[inst][j] = *yp; yp++;
      yparity1_g[inst][j] = *yp; yp++;
      yparity2_g[inst][j] = *yp; yp++;
      */


      systematic0_g[inst][j] = yp[0]; 
      yparity1_g[inst][j] = yp[1]; 
      yparity2_g[inst][j] = yp[2];

      j=pi2_p[1];
      systematic0_g[inst][j] = yp[3]; 
      yparity1_g[inst][j] = yp[4]; 
      yparity2_g[inst][j] = yp[5];

      j=pi2_p[2];
      systematic0_g[inst][j] = yp[6]; 
      yparity1_g[inst][j] = yp[7]; 
      yparity2_g[inst][j] = yp[8];

      j=pi2_p[3];
      systematic0_g[inst][j] = yp[9]; 
      yparity1_g[inst][j] = yp[10]; 
      yparity2_g[inst][j] = yp[11];

      j=pi2_p[4];
      systematic0_g[inst][j] = yp[12]; 
      yparity1_g[inst][j] = yp[13]; 
      yparity2_g[inst][j] = yp[14];

      j=pi2_p[5];
      systematic0_g[inst][j] = yp[15]; 
      yparity1_g[inst][j] = yp[16]; 
      yparity2_g[inst][j] = yp[17];

      j=pi2_p[6];
      systematic0_g[inst][j] = yp[18]; 
      yparity1_g[inst][j] = yp[19];
      yparity2_g[inst][j] = yp[20];

      j=pi2_p[7];
      systematic0_g[inst][j] = yp[21]; 
      yparity1_g[inst][j] = yp[22]; 
      yparity2_g[inst][j] = yp[23];

      yp+=24;
#ifdef DEBUG_LOGMAP
      msg("Position %d (%d): (%d,%d,%d)\n",i,j,systematic0_g[inst][j],yparity1_g[inst][j],yparity2_g[inst][j]);
#endif //DEBUG_LOGMAP
    }

  }
  // Termination
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
  // initial state for alpha (initialized from beta values after the initial state so ...)
  beta_g[inst][1] = -MAX/2;
  beta_g[inst][2] = -MAX/2;
  beta_g[inst][3] = -MAX/2;
  beta_g[inst][4] = -MAX/2;
  beta_g[inst][5] = -MAX/2;
  beta_g[inst][6] = -MAX/2;
  beta_g[inst][7] = -MAX/2;

  //  printf("LOG_MAP 0\n");
  log_map(systematic0_g[inst],yparity1_g[inst],ext,n,0,F,inst);

  while (iteration_cnt++ < max_iterations) {

#ifdef DEBUG_LOGMAP
    msg("\n*******************ITERATION %d (n %d), ext %p\n\n",iteration_cnt,n,ext);
#endif //DEBUG_LOGMAP

    //    threegpplte_interleaver_reset();
    //    pi=0;

    if (NEW_IMPL==0) {
      pi_p=&pi[0];
      // compute input to second encoder by interleaving extrinsic info
      
      for (i=0;i<n;i+=8) { // steady-state portion
	s_p = &systematic2_g[inst][i];
	//	systematic2_g[inst][i] = (ext[*pi_p] + systematic0_g[inst][*pi_p]);
	//      pi              = threegpplte_interleaver(f1,f2,n);
	s_p[0] = (ext[*pi_p] + systematic0_g[inst][*pi_p]);
	pi_p++;
	s_p[1] = (ext[*pi_p] + systematic0_g[inst][*pi_p]);
	pi_p++;
	s_p[2] = (ext[*pi_p] + systematic0_g[inst][*pi_p]);
	pi_p++;
	s_p[3] = (ext[*pi_p] + systematic0_g[inst][*pi_p]);
	pi_p++;
	s_p[4] = (ext[*pi_p] + systematic0_g[inst][*pi_p]);
	pi_p++;
	s_p[5] = (ext[*pi_p] + systematic0_g[inst][*pi_p]);
	pi_p++;
	s_p[6] = (ext[*pi_p] + systematic0_g[inst][*pi_p]);
	pi_p++;
	s_p[7] = (ext[*pi_p] + systematic0_g[inst][*pi_p]);
	pi_p++;

      }
      for (i=n;i<n+3;i++) { // termination
	systematic2_g[inst][i] = (systematic0_g[inst][i+8]);
      }
    }
    else {
      pi2_p=&pi2[0];
      for (i=0;i<n;i+=8){
	t_p = &tmp2[i];
	//	tmp2[i] = (ext[*pi2_p] + systematic0_g[inst][*pi2_p]);

	t_p[0] = (ext[*pi2_p] + systematic0_g[inst][*pi2_p]);
	pi2_p++;
	t_p[1] = (ext[*pi2_p] + systematic0_g[inst][*pi2_p]);
	pi2_p++;
	t_p[2] = (ext[*pi2_p] + systematic0_g[inst][*pi2_p]);
	pi2_p++;
	t_p[3] = (ext[*pi2_p] + systematic0_g[inst][*pi2_p]);
	pi2_p++;
	t_p[4] = (ext[*pi2_p] + systematic0_g[inst][*pi2_p]);
	pi2_p++;
	t_p[5] = (ext[*pi2_p] + systematic0_g[inst][*pi2_p]);
	pi2_p++;
	t_p[6] = (ext[*pi2_p] + systematic0_g[inst][*pi2_p]);
	pi2_p++;
	t_p[7] = (ext[*pi2_p] + systematic0_g[inst][*pi2_p]);
	pi2_p++;
      }
      pi3_p=&pi3[0];
      pi2_p=&pi2[0];
      pi_p=&pi[0];
      for (i=0;i<n;i++) { // steady-state portion
	systematic2_g[inst][*pi2_p++] = tmp2[*pi_p++];
	//	systematic2_g[inst][*pi2_p] = (ext[*pi3_p] + systematic0_g[inst][*pi3_p]);  // ext[*pi2[i]] = ext[i]
#ifdef DEBUG_LOGMAP
	msg("First half %d: ext %d, systematic0 %d, %d ",i,ext[i],systematic0_g[inst][i],*(pi_p-1));
	//	    ext[*pi3_p]+systematic0_g[inst][*pi3_p]);
	if (ext[i]*systematic0_g[inst][i]<=0)
	  printf("+++++\n");
	else
	  printf("\n");
#endif //DEBUG_LOGMAP
      }      
    }
    // do log_map from second parity bit    
    //    printf("LOG_MAP 1\n");
    log_map(systematic2_g[inst],yparity2_g[inst],ext2,n,1,0,inst);


    //    threegpplte_interleaver_reset();
    //    pi=0;
    if (NEW_IMPL==0) {
      pi_p=&pi[0];
      for (i=0;i<n>>3;i++)
	decoded_bytes[i]=0;
      // compute input to first encoder and output
      for (i=0;i<n;i++) {
	systematic1_g[inst][*pi_p] = (ext2[i] + systematic0_g[inst][*pi_p]);
#ifdef DEBUG_LOGMAP
	msg("Second half %d: ext2[i] %d, systematic2[i] %d (e+s %d)",i,ext2[i],systematic2_g[inst][i],
	    ext2[i]+systematic2_g[inst][i]);
	if (systematic2_g[inst][i]*ext2[i]<=0)
	  printf("+++++\n");
	else
	  printf("\n");
#endif //DEBUG_LOGMAP
	
	if ((systematic2_g[inst][i] + ext2[i]) > 0)
	  decoded_bytes[*pi_p>>3] += (1 << (7-(*pi_p&7)));
      
	pi_p++;
      }
      
      for (i=n;i<n+3;i++) {
	systematic1_g[inst][i] = (systematic0_g[inst][i]);
#ifdef DEBUG_LOGMAP
	//      msg("Second half %d: ext2[i] %d, systematic0[i] %d (e+s %d)\n",i,ext2[i],systematic0_g[inst][i],
	//	     ext2[i]+systematic2_g[inst][i]);
#endif //DEBUG_LOGMAP
      }
    }
    else {  //new implementation


      //    for (i=0;i<(n>>3);i++)
      //	decoded_bytes[i]=0;
      memset(decoded_bytes,0,n>>3);
      // compute input to first encoder and output


      pi_p=&pi[0];
      pi2_p=&pi2[0];
      pi3_p=&pi3[0];
      byte_pos_p=&byte_pos[0];
      bit_pos_p=&bit_pos[0];
      for (i=0;i<n;i++) {
	//	systematic1_g[inst][*pi3_p] = (ext2[i] + systematic0_g[inst][*pi3_p]);
	systematic1_g[inst][*pi3_p] = ext2[*pi2_p]+systematic0_g[inst][*pi3_p];
#ifdef DEBUG_LOGMAP
	msg("Second half %d: ext2[i] %d, systematic2[i] %d, systmatic0[i] %d",i,ext2[i],systematic2_g[inst][i],systematic0_g[inst][*pi3_p]);
	if ((systematic2_g[inst][i]*ext2[i])<=0)
	  printf("+++++\n");
	else
	  printf("\n"); 
#endif //DEBUG_LOGMAP
	
	if (systematic2_g[inst][*pi2_p] > -ext2[*pi2_p])
	  //	  decoded_bytes[*pi_p>>3] += (1 << (7-(*pi_p&7)));
	  //	  	  decoded_bytes[*pi_p>>3] |= (128 >> (*pi_p&7));
	  decoded_bytes[*byte_pos_p] |= *bit_pos_p;
	pi3_p++;
	pi2_p++;
	pi_p++;
	byte_pos_p++;
	bit_pos_p++;
      }      

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
    }

    if ((crc == oldcrc) && (crc!=0)) {
      decoder_in_use[inst] = 0;
      //msg("setting turbo decoder inst %d to 0\n",inst);
      return(iteration_cnt);
    }


    // do log_map from first parity bit
    if (iteration_cnt < max_iterations) {
      //      printf("LOG_MAP 0\n");
      log_map(systematic1_g[inst],yparity1_g[inst],ext,n,0,F,inst);
    }
  }

  //msg("setting turbo decoder inst %d to 0\n",inst);
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


  for (i=0;i<8;i++)
    printf("output %d => %x (input %x)\n",i,decoded_output[i],test[i]);
}




int main() {


  test_logmap8();

  return(0);
}

#endif // TEST_DEBUG


