#ifdef __SSE2__
#include <emmintrin.h>
#include <xmmintrin.h>
#endif
#ifdef __SSE3__
#include <pmmintrin.h>
#include <tmmintrin.h>
#endif
#include "PHY/defs.h"
#include "PHY/impl_defs.h"
#include "PHY/extern.h"
#include "extern.h"
#include "kHz_7_5.h"

#ifndef __SSE3__
__m128i zero;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmy)))
#endif
short conjugate75[8]__attribute__((aligned(16))) = {-1,1,-1,1,-1,1,-1,1} ;
short negate[8]__attribute__((aligned(16))) = {-1,-1,-1,-1,-1,-1,-1,-1};

void apply_7_5_kHz(PHY_VARS_UE *phy_vars_ue,u8 subframe) {


  s32 **txdata=phy_vars_ue->lte_ue_common_vars.txdata;
  s32 *txptr;
  u16 len;
  u32 *kHz7_5ptr;
  __m128i *txptr128,*kHz7_5ptr128,mmtmp_re,mmtmp_im,mmtmp_75;
  u32 subframe_offset;
  u8 aa,seg;
  u32 i;

  switch (phy_vars_ue->lte_frame_parms.N_RB_UL) {
    
  case 6:
    len=256;
    kHz7_5ptr = (u32*)s6_kHz_7_5;
    break;
  case 15:
    len=512;
    kHz7_5ptr = (u32*)s15_kHz_7_5;
    break;
  case 25:
    len=1024;
    kHz7_5ptr = (u32*)s25_kHz_7_5;
    break;
  case 50:
    len=2048;
    kHz7_5ptr = (u32*)s50_kHz_7_5;
    break;
  case 75:
    len=3072;
    kHz7_5ptr = (u32*)s75_kHz_7_5;
    break;
  case 100:
    len=4096;
    kHz7_5ptr = (u32*)s100_kHz_7_5;
    break;
  }

  subframe_offset = (u32)subframe * phy_vars_ue->lte_frame_parms.samples_per_tti;

  for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++) {
    for (seg=0;seg<8;seg++) {
      if (seg==7)
	len>>=1;

      txptr128 = (__m128i *)&txdata[aa][subframe_offset + (seg*len)];
      kHz7_5ptr128 = (__m128i *)&kHz7_5ptr;
      // apply 7.5 kHz
      if ((subframe&1) == 0) { // apply the sinusoid from the table directly
	for (i=0;i<(len>>2);i++) {
	  mmtmp_re = _mm_madd_epi16(*txptr128,*kHz7_5ptr128);  
	  // Real part of complex multiplication (note: 7_5kHz signal is conjugated for this to work)
	  mmtmp_im = _mm_shufflelo_epi16(*kHz7_5ptr128,_MM_SHUFFLE(2,3,0,1));
	  mmtmp_im = _mm_shufflehi_epi16(mmtmp_im,_MM_SHUFFLE(2,3,0,1));
	  mmtmp_im = _mm_sign_epi16(mmtmp_im,*(__m128i*)&conjugate75[0]);
	  mmtmp_im = _mm_madd_epi16(mmtmp_im,txptr128[0]);
	  mmtmp_re = _mm_srai_epi32(mmtmp_re,15);
	  mmtmp_im = _mm_srai_epi32(mmtmp_im,15);
	  txptr128[0] = _mm_packs_epi32(mmtmp_re,mmtmp_im);
	  txptr128++;
	  kHz7_5ptr128++;
	}
      }
      else {
	for (i=0;i<(len>>2);i++) { // apply the inverse from the table
	  // negate the 7_5kHz signal
	  mmtmp_75 = _mm_sign_epi16(*kHz7_5ptr128,*(__m128i*)&negate[0]);
	  mmtmp_re = _mm_madd_epi16(*txptr128,mmtmp_75);  
	  // Real part of complex multiplication (note: kHz7_5 signal is conjugated for this to work)
	  mmtmp_im = _mm_shufflelo_epi16(mmtmp_75,_MM_SHUFFLE(2,3,0,1));
	  mmtmp_im = _mm_shufflehi_epi16(mmtmp_im,_MM_SHUFFLE(2,3,0,1));
	  mmtmp_im = _mm_sign_epi16(mmtmp_im,*(__m128i*)&conjugate75[0]);
	  mmtmp_im = _mm_madd_epi16(mmtmp_im,txptr128[0]);
	  mmtmp_re = _mm_srai_epi32(mmtmp_re,15);
	  mmtmp_im = _mm_srai_epi32(mmtmp_im,15);
	  txptr128[0] = _mm_packs_epi32(mmtmp_re,mmtmp_im);
	  txptr128++;
	  kHz7_5ptr128++;
	}
      }
    }
  }
}
