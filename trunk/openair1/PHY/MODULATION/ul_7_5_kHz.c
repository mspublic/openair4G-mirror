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
short conjugate75_2[8]__attribute__((aligned(16))) = {1,-1,1,-1,1,-1,1,-1} ;
short negate[8]__attribute__((aligned(16))) = {-1,-1,-1,-1,-1,-1,-1,-1};

void apply_7_5_kHz(PHY_VARS_UE *phy_vars_ue,u8 slot) {


  s32 **txdata=phy_vars_ue->lte_ue_common_vars.txdata;
  u16 len;
  u32 *kHz7_5ptr;
  __m128i *txptr128,*kHz7_5ptr128,mmtmp_re,mmtmp_im,mmtmp_re2,mmtmp_im2;
  u32 slot_offset;
  u8 aa;
  u32 i;

  switch (phy_vars_ue->lte_frame_parms.N_RB_UL) {
    
  case 6:
    kHz7_5ptr = (u32*)s6_kHz_7_5;
    break;
  case 15:
    kHz7_5ptr = (u32*)s15_kHz_7_5;
    break;
  case 25:
    kHz7_5ptr = (u32*)s25_kHz_7_5;
    break;
  case 50:
    kHz7_5ptr = (u32*)s50_kHz_7_5;
    break;
  case 75:
    kHz7_5ptr = (u32*)s75_kHz_7_5;
    break;
  case 100:
    kHz7_5ptr = (u32*)s100_kHz_7_5;
    break;
  default:
    kHz7_5ptr = (u32*)s25_kHz_7_5;
    break;
  }

  slot_offset = (u32)slot * phy_vars_ue->lte_frame_parms.samples_per_tti/2;
  //  if ((slot&1)==1)
  //    slot_offset += (len/4);
  len = phy_vars_ue->lte_frame_parms.samples_per_tti/2;

  for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++) {
    txptr128 = (__m128i *)&txdata[aa][slot_offset];
    kHz7_5ptr128 = (__m128i *)kHz7_5ptr;
      // apply 7.5 kHz
    
      //      if (((slot>>1)&1) == 0) { // apply the sinusoid from the table directly
    for (i=0;i<(len>>2);i++) {
      mmtmp_re = _mm_madd_epi16(*txptr128,*kHz7_5ptr128);  
      // Real part of complex multiplication (note: 7_5kHz signal is conjugated for this to work)
      mmtmp_im = _mm_shufflelo_epi16(*kHz7_5ptr128,_MM_SHUFFLE(2,3,0,1));
      mmtmp_im = _mm_shufflehi_epi16(mmtmp_im,_MM_SHUFFLE(2,3,0,1));
      mmtmp_im = _mm_sign_epi16(mmtmp_im,*(__m128i*)&conjugate75[0]);
      mmtmp_im = _mm_madd_epi16(mmtmp_im,txptr128[0]);
      mmtmp_re = _mm_srai_epi32(mmtmp_re,15);
      mmtmp_im = _mm_srai_epi32(mmtmp_im,15);
      mmtmp_re2 = _mm_unpacklo_epi32(mmtmp_re,mmtmp_im);
      mmtmp_im2 = _mm_unpackhi_epi32(mmtmp_re,mmtmp_im);
      /*
	printf("%d: (%d,%d) (%d,%d) (%d,%d) (%d,%d) x (%d,%d) (%d,%d) (%d,%d) (%d,%d) => ",
	i,
	((short*)txptr128)[0],
	((short*)txptr128)[1],
	((short*)txptr128)[2],
	((short*)txptr128)[3],
	((short*)txptr128)[4],
	((short*)txptr128)[5],
	((short*)txptr128)[6],
	((short*)txptr128)[7],
	((short*)kHz7_5ptr128)[0],
	((short*)kHz7_5ptr128)[1],
	((short*)kHz7_5ptr128)[2],
	((short*)kHz7_5ptr128)[3],
	((short*)kHz7_5ptr128)[4],
	((short*)kHz7_5ptr128)[5],
	((short*)kHz7_5ptr128)[6],
	((short*)kHz7_5ptr128)[7]);*/
      
      txptr128[0] = _mm_packs_epi32(mmtmp_re2,mmtmp_im2);
      /*	  printf("%(%d,%d) (%d,%d) (%d,%d) (%d,%d)\n",
		  ((short*)txptr128)[0],
		  ((short*)txptr128)[1],
		  ((short*)txptr128)[2],
		  ((short*)txptr128)[3],
		  ((short*)txptr128)[4],
		  ((short*)txptr128)[5],
		  ((short*)txptr128)[6],
		  ((short*)txptr128)[7]);*/
      
      txptr128++;
      kHz7_5ptr128++;
    }
  }
}


void remove_7_5_kHz(PHY_VARS_eNB *phy_vars_eNB,u8 slot) {


  s32 **rxdata=phy_vars_eNB->lte_eNB_common_vars.rxdata[0];
  u16 len;
  u32 *kHz7_5ptr;
  __m128i *rxptr128,*kHz7_5ptr128,kHz7_5_2,mmtmp_re,mmtmp_im,mmtmp_re2,mmtmp_im2;
  u32 slot_offset;
  u8 aa;
  u32 i;

  switch (phy_vars_eNB->lte_frame_parms.N_RB_UL) {
    
  case 6:
    kHz7_5ptr = (u32*)s6_kHz_7_5;
    break;
  case 15:
    kHz7_5ptr = (u32*)s15_kHz_7_5;
    break;
  case 25:
    kHz7_5ptr = (u32*)&s25_kHz_7_5[0];
    break;
  case 50:
    kHz7_5ptr = (u32*)s50_kHz_7_5;
    break;
  case 75:
    kHz7_5ptr = (u32*)s75_kHz_7_5;
    break;
  case 100:
    kHz7_5ptr = (u32*)s100_kHz_7_5;
    break;
  default:
    kHz7_5ptr = (u32*)s25_kHz_7_5;
    break;
  }
 
  slot_offset = (u32)slot * phy_vars_eNB->lte_frame_parms.samples_per_tti/2;

  len = phy_vars_eNB->lte_frame_parms.samples_per_tti/2;

  for (aa=0;aa<phy_vars_eNB->lte_frame_parms.nb_antennas_rx;aa++) {

    rxptr128 = (__m128i *)&rxdata[aa][slot_offset];
    kHz7_5ptr128 = (__m128i *)kHz7_5ptr;
    
    // apply 7.5 kHz
    
    //      if (((slot>>1)&1) == 0) { // apply the sinusoid from the table directly
    for (i=0;i<(len>>2);i++) {
      kHz7_5_2 = _mm_sign_epi16(*kHz7_5ptr128,*(__m128i*)&conjugate75_2[0]);
      mmtmp_re = _mm_madd_epi16(*rxptr128,kHz7_5_2);  
      // Real part of complex multiplication (note: 7_5kHz signal is conjugated for this to work)
      mmtmp_im = _mm_shufflelo_epi16(kHz7_5_2,_MM_SHUFFLE(2,3,0,1));
      mmtmp_im = _mm_shufflehi_epi16(mmtmp_im,_MM_SHUFFLE(2,3,0,1));
      mmtmp_im = _mm_sign_epi16(mmtmp_im,*(__m128i*)&conjugate75[0]);
      mmtmp_im = _mm_madd_epi16(mmtmp_im,rxptr128[0]);
      mmtmp_re = _mm_srai_epi32(mmtmp_re,15);
      mmtmp_im = _mm_srai_epi32(mmtmp_im,15);
      mmtmp_re2 = _mm_unpacklo_epi32(mmtmp_re,mmtmp_im);
      mmtmp_im2 = _mm_unpackhi_epi32(mmtmp_re,mmtmp_im);
      
      rxptr128[0] = _mm_packs_epi32(mmtmp_re2,mmtmp_im2);
      rxptr128++;
      kHz7_5ptr128++;
    }
  }
}

