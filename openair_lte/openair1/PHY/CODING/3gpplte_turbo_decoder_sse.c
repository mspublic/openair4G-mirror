/* file: 3gpplte_turbo_decoder_sse.c
   purpose: Routines for implementing decoding of Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 

   Note: This routine currently requires SSE2,SSSE3 and SSE4.1 equipped computers.  IT WON'T RUN OTHERWISE!

   Changelog: 17.11.2009 FK SSE4.1 not required anymore
*/

///
///

#include "emmintrin.h"
#include "pmmintrin.h"
#include "tmmintrin.h"
#ifdef __SSE4_1__
#include "smmintrin.h"
#endif

#include "PHY/defs.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/lte_interleaver_inline.h"






typedef short llr_t; // internal decoder data is 16-bit fixed
typedef short channel_t;


#define MAX 16383
#define THRES 8192

#define FRAME_LENGTH_MAX 6144
#define STATES 8

void log_map (llr_t* systematic,channel_t* y_parity, llr_t* ext,unsigned short frame_length,unsigned char term_flag,unsigned char F);
void compute_gamma(llr_t* m11,llr_t* m10,llr_t* systematic, channel_t* y_parity, unsigned short frame_length,unsigned char term_flag);
void compute_alpha(llr_t*alpha,llr_t* m11,llr_t* m10, unsigned short frame_length,unsigned char F);
void compute_beta(llr_t* beta,llr_t* m11,llr_t* m10,llr_t* alpha, unsigned short frame_length,unsigned char F);
void compute_ext(llr_t* alpha,llr_t* beta,llr_t* m11,llr_t* m10,llr_t* extrinsic, llr_t* ap, unsigned short frame_length);

// global variables
//
llr_t alpha[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t beta[(FRAME_LENGTH_MAX+3+1)*8] __attribute__ ((aligned(16)));
llr_t m11[(FRAME_LENGTH_MAX+3)] __attribute__ ((aligned(16)));
llr_t m10[(FRAME_LENGTH_MAX+3)] __attribute__ ((aligned(16)));

void log_map(llr_t* systematic,channel_t* y_parity, llr_t* ext,unsigned short frame_length,unsigned char term_flag,unsigned char F) {

  compute_gamma(m11,m10,systematic,y_parity,frame_length,term_flag);

  compute_alpha(alpha,m11,m10,frame_length,F);

  compute_beta(beta,m11,m10,alpha,frame_length,F);

  compute_ext(alpha,beta,m11,m10,ext,systematic,frame_length);

}

void compute_gamma(llr_t* m11,llr_t* m10,llr_t* systematic,channel_t* y_parity,
		   unsigned short frame_length,unsigned char term_flag)
{
  int k; 
  __m128i *systematic128 = (__m128i *)systematic;
  __m128i *y_parity128   = (__m128i *)y_parity;
  __m128i *m10_128        = (__m128i *)m10;
  __m128i *m11_128        = (__m128i *)m11;

  for (k=0;k<frame_length>>3;k++) {
      m11_128[k] = _mm_srai_epi16(_mm_adds_epi16(systematic128[k],y_parity128[k]),1);
      m10_128[k] = _mm_srai_epi16(_mm_subs_epi16(systematic128[k],y_parity128[k]),1);

      //      printf("gamma %d : (%d,%d) -> (%d,%d)\n",k,systematic[k],y_parity[k],m11[k],m10[k]);
  }
  // Termination
  m11_128[k] = _mm_srai_epi16(_mm_adds_epi16(systematic128[k+term_flag],y_parity128[k]),1);
  m10_128[k] = _mm_srai_epi16(_mm_subs_epi16(systematic128[k+term_flag],y_parity128[k]),1);


  
}

short systematic0[6144+16] __attribute__ ((aligned(16)));
short systematic1[6144+16] __attribute__ ((aligned(16)));
short systematic2[6144+16]__attribute__ ((aligned(16)));
short yparity1[6144+8] __attribute__ ((aligned(16)));
short yparity2[6144+8] __attribute__ ((aligned(16)));

__m128i mtop[6144] __attribute__ ((aligned(16)));
__m128i mbot[6144] __attribute__ ((aligned(16)));

void compute_alpha(llr_t* alpha,llr_t* m_11,llr_t* m_10,unsigned short frame_length,unsigned char F)
{
  int k;
  __m128i *alpha128=(__m128i *)alpha,mtmp,mtmp2,lsw,msw,new,mb,newcmp;

  __m128i TOP,BOT,THRES128;
  

#ifndef __SSE4_1__
  int* newcmp_int;
#endif

  llr_t m11,m10;
  
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
      //      printf("m11 %d, m10 %d\n",m11,m10);

      // First compute state transitions  (Think about LUT!)
      //      mtmp = m11_128;
      // Think about __mm_set1_epi16 instruction!!!!

      mtmp  = _mm_set1_epi16(m11);
      //      print_shorts("m11) mtmp",&mtmp);

      mtmp2 = _mm_set1_epi16(m10);
      //      print_shorts("m10) mtmp2",&mtmp2);

      mtmp = _mm_unpacklo_epi32(mtmp,mtmp2);
      //      print_shorts("unpacklo) mtmp",&mtmp);
      // mtmp = [m11(0) m11(0) m10(0) m10(0) m11(0) m11(0) m10(0) m10(0)]
      mtmp = _mm_shuffle_epi32(mtmp,_MM_SHUFFLE(0,1,3,2));
      // mtmp = [m11(0) m11(0) m10(0) m10(0) m10(0) m10(0) m11(0) m11(0)]
      //      print_shorts("shuffle) mtmp",&mtmp);

      mtop[k] = _mm_sign_epi16(mtmp,TOP);
      mbot[k] = _mm_sign_epi16(mtmp,BOT);

      //      print_shorts("mtop=",&mtop);
      //      printf("M0T %d, M1T %d, M2T %d, M3T %d, M4T %d, M5T %d, M6T %d, M7T %d\n",M0T,M1T,M2T,M3T,M4T,M5T,M6T,M7T);
      //      print_shorts("mbot=",&mbot);
      //      printf("M0B %d, M1B %d, M2B %d, M3B %d, M4B %d, M5B %d, M6B %d, M7B %d\n",M0B,M1B,M2B,M3B,M4B,M5B,M6B,M7B);


      // Now compute max-logmap

      lsw  = _mm_adds_epi16(*alpha128,mtop[k]);
      msw  = _mm_adds_epi16(*alpha128,mbot[k]);

      //      print_shorts("lsw=",&lsw);
      //      print_shorts("msw=",&msw);



      alpha128++;

      // lsw = [mb3 new3 mb2 new2 mb1 new1 mb0 new0] 
      // msw = [mb7 new7 mb6 new6 mb4 new4 mb3 new3] 

      lsw = _mm_shufflelo_epi16(lsw,_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shufflelo) lsw=",&lsw);
      lsw = _mm_shufflehi_epi16(lsw,_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shufflehi) lsw=",&lsw);

      msw = _mm_shufflelo_epi16(msw,_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shufflelo) msw=",&msw);
      msw = _mm_shufflehi_epi16(msw,_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shufflehi) msw=",&msw);

      // lsw = [mb3 mb2 new3 new2 mb1 mb0 new1 new0] 
      // msw = [mb7 mb6 new7 new6 mb4 mb3 new4 new3] 

      lsw = _mm_shuffle_epi32(lsw,_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shuffle) lsw=",&lsw);
      msw = _mm_shuffle_epi32(msw,_MM_SHUFFLE(3,1,2,0));
      //      print_shorts("shuffle) msw=",&msw);
      // lsw = [mb3 mb2 mb1 mb0 new3 new2 new1 new0] 
      // msw = [mb7 mb6 mb4 mb3 new7 new6 new4 new3] 

      new = _mm_unpacklo_epi64(lsw,msw);
      mb  = _mm_unpackhi_epi64(lsw,msw);
      // new = [new7 new6 new4 new3 new3 new2 new1 new0] 
      // mb = [mb7 mb6 mb4 mb3 mb3 mb2 mb1 mb0] 
      // Now both are in right order, so compute max



      //      *alpha128 = _mm_max_epi16(new,mb);
      
            
      new = _mm_max_epi16(new,mb);
      newcmp = _mm_cmpgt_epi16(new,THRES128);

#ifndef __SSE4_1__
      newcmp_int = (int*) &newcmp;
      if (newcmp_int[0]==0 && newcmp_int[1]==0 && newcmp_int[2]==0 && newcmp_int[3]==0) // if any states above THRES normalize
#else
      if (_mm_testz_si128(newcmp,newcmp)) // if any states above THRES normalize
#endif
	*alpha128 = new;
      else {
	//	print_shorts("new",&new);
	*alpha128 = _mm_subs_epi16(new,THRES128);
	//	printf("alpha overflow %d",k);
	//	print_shorts(" ",alpha128);
      }
      //
	  //      print_shorts("alpha",alpha128);
	  
  }

}
void compute_beta(llr_t* beta,llr_t *m_11,llr_t* m_10,llr_t* alpha,unsigned short frame_length,unsigned char F)
{
  int k;


  __m128i *beta128,*beta128_i,new,mb,oldh,oldl,THRES128,newcmp;

#ifndef __SSE4_1__
  int* newcmp_int;
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

      oldh = _mm_unpackhi_epi16(*beta128,*beta128);
      oldl = _mm_unpacklo_epi16(*beta128,*beta128);

      mb   = _mm_adds_epi16(oldh,mbot[k]);
      new  = _mm_adds_epi16(oldl,mtop[k]);

      beta128--;
      //      *beta128= _mm_max_epi16(new,mb);
            
      new = _mm_max_epi16(new,mb);
      //      print_shorts("alpha128",alpha128);

      newcmp = _mm_cmpgt_epi16(new,THRES128);

#ifndef __SSE4_1__
      newcmp_int = (int*) &newcmp;
      if (newcmp_int[0]==0 && newcmp_int[1]==0 && newcmp_int[2]==0 && newcmp_int[3]==0) // if any states above THRES normalize
#else
      if (_mm_testz_si128(newcmp,newcmp))
#endif
	*beta128 = new;
      else{
	*beta128 = _mm_subs_epi16(new,THRES128);
	//	printf("Beta overflow : %d\n",k);
      }
      
    }
}

void compute_ext(llr_t* alpha,llr_t* beta,llr_t* m_11,llr_t* m_10,llr_t* ext, llr_t* systematic,unsigned short frame_length)
{
  int k;

  __m128i *alpha128=(__m128i *)alpha,alpha_km1_top,alpha_km1_bot,alpha_k_top,alpha_k_bot,alpha_1,alpha_2,alpha_3,alpha_4;
  __m128i *alpha128_ptr,*beta128_ptr;
  __m128i *beta128=(__m128i *)beta,alpha_beta_1,alpha_beta_2,alpha_beta_3,alpha_beta_4,alpha_beta_max04,alpha_beta_max15,alpha_beta_max26,alpha_beta_max37;
  __m128i tmp0,tmp1,tmp2,tmp3,tmp00,tmp10,tmp20,tmp30;
  __m128i m00_max,m01_max,m10_max,m11_max;
  __m128i *m11_128,*m10_128,*ext_128,*systematic_128;
  //
  // LLR computation, 8 consequtive bits per loop
  //
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
	
	alpha_km1_top = _mm_unpacklo_epi32(alpha128_ptr[0],alpha128_ptr[0]);
      alpha_km1_bot = _mm_unpackhi_epi32(alpha128_ptr[0],alpha128_ptr[0]);
      alpha_k_top = _mm_unpacklo_epi32(alpha128_ptr[4],alpha128_ptr[4]);
      alpha_k_bot = _mm_unpackhi_epi32(alpha128_ptr[4],alpha128_ptr[4]);
      // these are [0 2 0 2 1 3 1 3] and [6 4 6 4 7 5 7 5]
      //      print_shorts("a_km1_top",&alpha_km1_top);      
      //      print_shorts("a_km1_top",&alpha_km1_bot);      

      alpha_1 = _mm_unpacklo_epi64(alpha_km1_top,alpha_k_top);
      alpha_2 = _mm_unpackhi_epi64(alpha_km1_top,alpha_k_top);
      alpha_3 = _mm_unpacklo_epi64(alpha_km1_bot,alpha_k_bot);
      alpha_4 = _mm_unpackhi_epi64(alpha_km1_bot,alpha_k_bot);
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

      alpha_beta_1   = _mm_unpacklo_epi64(beta128_ptr[0],beta128_ptr[4]);
      //      print_shorts("ab_1",&alpha_beta_1);
      alpha_beta_2   = _mm_shuffle_epi32(alpha_beta_1,_MM_SHUFFLE(2,3,0,1));
      //      print_shorts("ab_2",&alpha_beta_2);
      alpha_beta_3   = _mm_unpackhi_epi64(beta128_ptr[0],beta128_ptr[4]);
      alpha_beta_4   = _mm_shuffle_epi32(alpha_beta_3,_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 0 1 4 5] [4 5 0 1 4 5 0 1] [7 6 3 2 7 6 3 2] [3 2 7 6 3 2 7 6]
      alpha_beta_1   = _mm_adds_epi16(alpha_beta_1,alpha_1);
      alpha_beta_2   = _mm_adds_epi16(alpha_beta_2,alpha_2);
      alpha_beta_3   = _mm_adds_epi16(alpha_beta_3,alpha_3);      
      alpha_beta_4   = _mm_adds_epi16(alpha_beta_4,alpha_4);




      /*

      print_shorts("alpha_beta_1",&alpha_beta_1);
      print_shorts("alpha_beta_2",&alpha_beta_2);
      print_shorts("alpha_beta_3",&alpha_beta_3);
      print_shorts("alpha_beta_4",&alpha_beta_4);
      printf("m00: %d %d %d %d\n",m00_1,m00_2,m00_3,m00_4);
      printf("m10: %d %d %d %d\n",m10_1,m10_2,m10_3,m10_4);
      printf("m11: %d %d %d %d\n",m11_1,m11_2,m11_3,m11_4);
      printf("m01: %d %d %d %d\n",m01_1,m01_2,m01_3,m01_4);
      */
      alpha_beta_max04 = _mm_max_epi16(alpha_beta_1,alpha_beta_2);
      alpha_beta_max04 = _mm_max_epi16(alpha_beta_max04,alpha_beta_3);
      alpha_beta_max04 = _mm_max_epi16(alpha_beta_max04,alpha_beta_4);
      // these are the 4 mxy_1 below for k and k+4
      

      /*
      print_shorts("alpha_beta_max04",&alpha_beta_max04);
      printf("%d %d %d %d\n",m00_1,m10_1,m11_1,m01_1);
      */
      

      // bits 1 + 5

      alpha128_ptr[1] = _mm_shufflelo_epi16(alpha128_ptr[1],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[1] = _mm_shufflehi_epi16(alpha128_ptr[1],_MM_SHUFFLE(1,3,0,2));
      alpha128_ptr[5]   = _mm_shufflelo_epi16(alpha128_ptr[5],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[5]   = _mm_shufflehi_epi16(alpha128_ptr[5],_MM_SHUFFLE(1,3,0,2));
      // these are [0 2 1 3 6 4 7 5] 

      alpha_km1_top = _mm_unpacklo_epi32(alpha128_ptr[1],alpha128_ptr[1]);
      alpha_km1_bot = _mm_unpackhi_epi32(alpha128_ptr[1],alpha128_ptr[1]);
      alpha_k_top = _mm_unpacklo_epi32(alpha128_ptr[5],alpha128_ptr[5]);
      alpha_k_bot = _mm_unpackhi_epi32(alpha128_ptr[5],alpha128_ptr[5]);
      // these are [0 2 0 2 1 3 1 3] and [6 4 6 4 7 5 7 5]


      alpha_1 = _mm_unpacklo_epi64(alpha_km1_top,alpha_k_top);
      alpha_2 = _mm_unpackhi_epi64(alpha_km1_top,alpha_k_top);
      alpha_3 = _mm_unpacklo_epi64(alpha_km1_bot,alpha_k_bot);
      alpha_4 = _mm_unpackhi_epi64(alpha_km1_bot,alpha_k_bot);

      beta128_ptr[1] = _mm_shuffle_epi32(beta128_ptr[1],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[1] = _mm_shufflehi_epi16(beta128_ptr[1],_MM_SHUFFLE(2,3,0,1));
      beta128_ptr[5] = _mm_shuffle_epi32(beta128_ptr[5],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[5] = _mm_shufflehi_epi16(beta128_ptr[5],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 7 6 3 2]

      alpha_beta_1   = _mm_unpacklo_epi64(beta128_ptr[1],beta128_ptr[5]);
      alpha_beta_2   = _mm_shuffle_epi32(alpha_beta_1,_MM_SHUFFLE(2,3,0,1));
      alpha_beta_3   = _mm_unpackhi_epi64(beta128_ptr[1],beta128_ptr[5]);
      alpha_beta_4   = _mm_shuffle_epi32(alpha_beta_3,_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 0 1 4 5] [4 5 0 1 4 5 0 1] [7 6 3 2 7 6 3 2] [3 2 7 6 3 2 7 6]
      alpha_beta_1   = _mm_adds_epi16(alpha_beta_1,alpha_1);
      alpha_beta_2   = _mm_adds_epi16(alpha_beta_2,alpha_2);
      alpha_beta_3   = _mm_adds_epi16(alpha_beta_3,alpha_3);      
      alpha_beta_4   = _mm_adds_epi16(alpha_beta_4,alpha_4);


      alpha_beta_max15 = _mm_max_epi16(alpha_beta_1,alpha_beta_2);
      alpha_beta_max15 = _mm_max_epi16(alpha_beta_max15,alpha_beta_3);
      alpha_beta_max15 = _mm_max_epi16(alpha_beta_max15,alpha_beta_4);

      // bits 2 + 6


      alpha128_ptr[2] = _mm_shufflelo_epi16(alpha128_ptr[2],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[2] = _mm_shufflehi_epi16(alpha128_ptr[2],_MM_SHUFFLE(1,3,0,2));
      alpha128_ptr[6]   = _mm_shufflelo_epi16(alpha128_ptr[6],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[6]   = _mm_shufflehi_epi16(alpha128_ptr[6],_MM_SHUFFLE(1,3,0,2));
      // these are [0 2 1 3 6 4 7 5] 

      alpha_km1_top = _mm_unpacklo_epi32(alpha128_ptr[2],alpha128_ptr[2]);
      alpha_km1_bot = _mm_unpackhi_epi32(alpha128_ptr[2],alpha128_ptr[2]);
      alpha_k_top = _mm_unpacklo_epi32(alpha128_ptr[6],alpha128_ptr[6]);
      alpha_k_bot = _mm_unpackhi_epi32(alpha128_ptr[6],alpha128_ptr[6]);
      // these are [0 2 0 2 1 3 1 3] and [6 4 6 4 7 5 7 5]


      alpha_1 = _mm_unpacklo_epi64(alpha_km1_top,alpha_k_top);
      alpha_2 = _mm_unpackhi_epi64(alpha_km1_top,alpha_k_top);
      alpha_3 = _mm_unpacklo_epi64(alpha_km1_bot,alpha_k_bot);
      alpha_4 = _mm_unpackhi_epi64(alpha_km1_bot,alpha_k_bot);

      beta128_ptr[2] = _mm_shuffle_epi32(beta128_ptr[2],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[2] = _mm_shufflehi_epi16(beta128_ptr[2],_MM_SHUFFLE(2,3,0,1));
      beta128_ptr[6] = _mm_shuffle_epi32(beta128_ptr[6],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[6] = _mm_shufflehi_epi16(beta128_ptr[6],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 7 6 3 2]

      alpha_beta_1   = _mm_unpacklo_epi64(beta128_ptr[2],beta128_ptr[6]);
      alpha_beta_2   = _mm_shuffle_epi32(alpha_beta_1,_MM_SHUFFLE(2,3,0,1));
      alpha_beta_3   = _mm_unpackhi_epi64(beta128_ptr[2],beta128_ptr[6]);
      alpha_beta_4   = _mm_shuffle_epi32(alpha_beta_3,_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 0 1 4 5] [4 5 0 1 4 5 0 1] [7 6 3 2 7 6 3 2] [3 2 7 6 3 2 7 6]
      alpha_beta_1   = _mm_adds_epi16(alpha_beta_1,alpha_1);
      alpha_beta_2   = _mm_adds_epi16(alpha_beta_2,alpha_2);
      alpha_beta_3   = _mm_adds_epi16(alpha_beta_3,alpha_3);      
      alpha_beta_4   = _mm_adds_epi16(alpha_beta_4,alpha_4);
      /*
      print_shorts("alpha_beta_1",&alpha_beta_1);
      print_shorts("alpha_beta_2",&alpha_beta_2);
      print_shorts("alpha_beta_3",&alpha_beta_3);
      print_shorts("alpha_beta_4",&alpha_beta_4);
      */
      alpha_beta_max26 = _mm_max_epi16(alpha_beta_1,alpha_beta_2);
      alpha_beta_max26 = _mm_max_epi16(alpha_beta_max26,alpha_beta_3);
      alpha_beta_max26 = _mm_max_epi16(alpha_beta_max26,alpha_beta_4);

      
      //      print_shorts("alpha_beta_max26",&alpha_beta_max26);

      

      // bits 3 + 7

      alpha128_ptr[3] = _mm_shufflelo_epi16(alpha128_ptr[3],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[3] = _mm_shufflehi_epi16(alpha128_ptr[3],_MM_SHUFFLE(1,3,0,2));
      alpha128_ptr[7]   = _mm_shufflelo_epi16(alpha128_ptr[7],_MM_SHUFFLE(3,1,2,0));
      alpha128_ptr[7]   = _mm_shufflehi_epi16(alpha128_ptr[7],_MM_SHUFFLE(1,3,0,2));
      // these are [0 2 1 3 6 4 7 5] 

      alpha_km1_top = _mm_unpacklo_epi32(alpha128_ptr[3],alpha128_ptr[3]);
      alpha_km1_bot = _mm_unpackhi_epi32(alpha128_ptr[3],alpha128_ptr[3]);
      alpha_k_top = _mm_unpacklo_epi32(alpha128_ptr[7],alpha128_ptr[7]);
      alpha_k_bot = _mm_unpackhi_epi32(alpha128_ptr[7],alpha128_ptr[7]);
      // these are [0 2 0 2 1 3 1 3] and [6 4 6 4 7 5 7 5]


      alpha_1 = _mm_unpacklo_epi64(alpha_km1_top,alpha_k_top);
      alpha_2 = _mm_unpackhi_epi64(alpha_km1_top,alpha_k_top);
      alpha_3 = _mm_unpacklo_epi64(alpha_km1_bot,alpha_k_bot);
      alpha_4 = _mm_unpackhi_epi64(alpha_km1_bot,alpha_k_bot);

      beta128_ptr[3] = _mm_shuffle_epi32(beta128_ptr[3],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[3] = _mm_shufflehi_epi16(beta128_ptr[3],_MM_SHUFFLE(2,3,0,1));
      beta128_ptr[7] = _mm_shuffle_epi32(beta128_ptr[7],_MM_SHUFFLE(1,3,2,0));
      beta128_ptr[7] = _mm_shufflehi_epi16(beta128_ptr[7],_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 7 6 3 2]

      alpha_beta_1   = _mm_unpacklo_epi64(beta128_ptr[3],beta128_ptr[7]);
      alpha_beta_2   = _mm_shuffle_epi32(alpha_beta_1,_MM_SHUFFLE(2,3,0,1));
      alpha_beta_3   = _mm_unpackhi_epi64(beta128_ptr[3],beta128_ptr[7]);
      alpha_beta_4   = _mm_shuffle_epi32(alpha_beta_3,_MM_SHUFFLE(2,3,0,1));
      // these are [0 1 4 5 0 1 4 5] [4 5 0 1 4 5 0 1] [7 6 3 2 7 6 3 2] [3 2 7 6 3 2 7 6]
      alpha_beta_1   = _mm_adds_epi16(alpha_beta_1,alpha_1);
      alpha_beta_2   = _mm_adds_epi16(alpha_beta_2,alpha_2);
      alpha_beta_3   = _mm_adds_epi16(alpha_beta_3,alpha_3);      
      alpha_beta_4   = _mm_adds_epi16(alpha_beta_4,alpha_4);


      alpha_beta_max37 = _mm_max_epi16(alpha_beta_1,alpha_beta_2);
      alpha_beta_max37 = _mm_max_epi16(alpha_beta_max37,alpha_beta_3);
      alpha_beta_max37 = _mm_max_epi16(alpha_beta_max37,alpha_beta_4);

      // transpose alpha_beta matrix
      /*
      print_shorts("alpha_beta_max04",&alpha_beta_max04);
      print_shorts("alpha_beta_max15",&alpha_beta_max15);
      print_shorts("alpha_beta_max26",&alpha_beta_max26);
      print_shorts("alpha_beta_max37",&alpha_beta_max37);
      */
      tmp0 = _mm_unpacklo_epi16(alpha_beta_max04,alpha_beta_max15);
      tmp1 = _mm_unpackhi_epi16(alpha_beta_max04,alpha_beta_max15);
      tmp2 = _mm_unpacklo_epi16(alpha_beta_max26,alpha_beta_max37);
      tmp3 = _mm_unpackhi_epi16(alpha_beta_max26,alpha_beta_max37);


      tmp00 = _mm_unpacklo_epi32(tmp0,tmp2);
      tmp10 = _mm_unpackhi_epi32(tmp0,tmp2);
      tmp20 = _mm_unpacklo_epi32(tmp1,tmp3);
      tmp30 = _mm_unpackhi_epi32(tmp1,tmp3);


      m00_max = _mm_unpacklo_epi64(tmp00,tmp20);
      m10_max = _mm_unpackhi_epi64(tmp00,tmp20);
      m11_max = _mm_unpacklo_epi64(tmp10,tmp30);
      m01_max = _mm_unpackhi_epi64(tmp10,tmp30);

/*            print_shorts("m00_max",&m00_max);
            print_shorts("m01_max",&m01_max);
            print_shorts("m11_max",&m11_max);
            print_shorts("m10_max",&m10_max);
*/


      // compute extrinsics for 8 consecutive bits

      m11_128        = (__m128i*)&m_11[k];
      m10_128        = (__m128i*)&m_10[k];
      ext_128        = (__m128i*)&ext[k];
      systematic_128 = (__m128i*)&systematic[k];

      m11_max = _mm_adds_epi16(m11_max,*m11_128);
      m10_max = _mm_adds_epi16(m10_max,*m10_128);
      m00_max = _mm_subs_epi16(m00_max,*m11_128);
      m01_max = _mm_subs_epi16(m01_max,*m10_128);

      m01_max = _mm_max_epi16(m01_max,m00_max);
      m10_max = _mm_max_epi16(m11_max,m10_max);

      //      print_shorts("m01_max",&m01_max);
      //      print_shorts("m10_max",&m10_max);

       
   
      *ext_128 = _mm_subs_epi16(m10_max,_mm_adds_epi16(m01_max,*systematic_128));
      /*
      if ((((short *)ext_128)[0] > 8192) ||
	  (((short *)ext_128)[1] > 8192) ||
	  (((short *)ext_128)[2] > 8192) ||
	  (((short *)ext_128)[3] > 8192) ||
	  (((short *)ext_128)[4] > 8192) ||
	  (((short *)ext_128)[5] > 8192) ||
	  (((short *)ext_128)[6] > 8192) ||
	  (((short *)ext_128)[7] > 8192)) {
	printf("ext overflow %d:",k);
	print_shorts("**ext_128",ext_128);
      }
      */

    }


}



unsigned char phy_threegpplte_turbo_decoder(llr_t *y,
					    unsigned char *decoded_bytes,
					    unsigned short n,
					    unsigned short f1,
					    unsigned short f2,
					    unsigned char max_iterations,
					    unsigned char crc_type,
					    unsigned char F) {
  
  /*  y is a pointer to the input
    decoded_bytes is a pointer to the decoded output
    n is the size in bits of the coded block, with the tail */
  short ext[n],ext2[n];

  //  short systematic0[n],systematic1[n],systematic2[n],yparity1[n],yparity2[n];
  llr_t *yp = y;
  unsigned short i,pi;
  unsigned char iteration_cnt=0;
  unsigned int crc,oldcrc,crc_len;

  if (crc_type > 3) {
    msg("Illegal crc length!\n");
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
  for (i=0;i<n;i++) {
    systematic0[i] = *yp; yp++;
    yparity1[i] = *yp; yp++;
    yparity2[i] = *yp; yp++;
#ifdef DEBUG_LOGMAP
    printf("Position %d: (%d,%d,%d)\n",i,systematic0[i],yparity1[i],yparity2[i]);
#endif //DEBUG_LOGMAP

  }
  for (i=n;i<n+3;i++) {
    systematic0[i]= *yp ;yp++;
    yparity1[i] = *yp;yp++;
#ifdef DEBUG_LOGMAP
    printf("Term 1 (%d): %d %d\n",i,systematic0[i],yparity1[i]);
#endif //DEBUG_LOGMAP
  }
  for (i=n+8;i<n+11;i++) {
    systematic0[i]= *yp ;yp++;
    yparity2[i-8] = *yp;yp++;
#ifdef DEBUG_LOGMAP
    printf("Term 2 (%d): %d %d\n",i-3,systematic0[i],yparity2[i-3]);
#endif //DEBUG_LOGMAP
  }
#ifdef DEBUG_LOGMAP
  printf("\n");
#endif //DEBUG_LOGMAP



  // do log_map from first parity bit
  log_map(systematic0,yparity1,ext,n,0,F);


  while (iteration_cnt++ < max_iterations) {

#ifdef DEBUG_LOGMAP
   printf("\n*******************ITERATION %d\n\n",iteration_cnt);
#endif //DEBUG_LOGMAP

    threegpplte_interleaver_reset();
    pi=0;

    // compute input to second encoder by interleaving extrinsic info
    for (i=0;i<n;i++) { // steady-state portion
      systematic2[i] = (ext[pi] + systematic0[pi]);
      pi              = threegpplte_interleaver(f1,f2,n);
    }
    for (i=n;i<n+3;i++) { // termination
      systematic2[i] = (systematic0[i+8]);
    }
    // do log_map from second parity bit    
    log_map(systematic2,yparity2,ext2,n,1,0);


    threegpplte_interleaver_reset();
    pi=0;
    for (i=0;i<n>>3;i++)
      decoded_bytes[i]=0;
    // compute input to first encoder and output
    for (i=0;i<n;i++) {
      systematic1[pi] = (ext2[i] + systematic0[pi]);
#ifdef DEBUG_LOGMAP
      printf("Second half %d: ext2[i] %d, systematic0[i] %d (e+s %d)\n",i,ext2[i],systematic0[pi],
	     ext2[i]+systematic2[i]);
#endif //DEBUG_LOGMAP

      if ((systematic2[i] + ext2[i]) > 0)
	decoded_bytes[pi>>3] += (1 << (pi&7));

      pi              = threegpplte_interleaver(f1,f2,n);
    }
    
    for (i=n;i<n+3;i++) {
      systematic1[i] = (systematic0[i]);
#ifdef DEBUG_LOGMAP
      printf("Second half %d: ext2[i] %d, systematic0[i] %d (e+s %d)\n",i,ext2[i],systematic0[i],
	     ext2[i]+systematic2[i]);
#endif //DEBUG_LOGMAP
    }
    

    // check status on output

    oldcrc= *((unsigned int *)(&decoded_bytes[(n>>3)-crc_len]));
    switch (crc_type) {

    case CRC24_A: 
      oldcrc&=0x00ffffff;
      crc = crc24a(decoded_bytes,
		   n-24)>>8;
      if (crc == oldcrc)
	return(iteration_cnt);

      break;
    case CRC24_B:
      oldcrc&=0x00ffffff;
      crc = crc24b(decoded_bytes,
		  n-24)>>8;
      //printf("CRC24_B = %x, oldcrc = %x\n",crc,oldcrc);
      if (crc == oldcrc)
	return(iteration_cnt);


      break;
    case CRC16:
      oldcrc&=0x0000ffff;
      crc = crc16(decoded_bytes,
		  n-16)>>16;
      if (crc == oldcrc)
	return(iteration_cnt);

      break;
    case CRC8:
      oldcrc&=0x000000ff;
      crc = crc8(decoded_bytes,
		  n-8)>>24;
      if (crc == oldcrc)
	return(iteration_cnt);

      break;
    }


    // do log_map from first parity bit
    if (iteration_cnt < max_iterations)
      log_map(systematic1,yparity1,ext,n,0,F);
  }

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
  unsigned int i;

  
  test[0] = 7;
  test[1] = 0xa5;
  test[2] = 0;
  test[3] = 0xfe;
  test[4] = 0x1a;
  test[5] = 0x0;
  //  test[5] = 0x33;
  //  test[6] = 0x99;
  //  test[7] = 0;
  

  threegpplte_turbo_encoder(test,
			    5, 
			    output,
			    3,
			    10);

  for (i = 0; i < 132; i++){
    channel_output[i] = 15*(2*output[i] - 1);
    //    printf("Position %d : %d\n",i,channel_output[i]);
  }

  memset(decoded_output,0,16);
  phy_threegpplte_turbo_decoder(channel_output,decoded_output,40,3,10,6,3);




}




void main() {


  test_logmap8();

}

#endif // TEST_DEBUG


