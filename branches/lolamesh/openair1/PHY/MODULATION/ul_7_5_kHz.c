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
#include "prach625Hz.h"
#ifdef USER_MODE
#include <math.h>
#else
#include "rtai_math.h"
#endif
#ifndef __SSE3__
__m128i zero;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmy)))
#endif
short conjugate75[8]__attribute__((aligned(16))) = {-1,1,-1,1,-1,1,-1,1} ;
short conjugate75_2[8]__attribute__((aligned(16))) = {1,-1,1,-1,1,-1,1,-1} ;
short negate[8]__attribute__((aligned(16))) = {-1,-1,-1,-1,-1,-1,-1,-1};

void apply_7_5_kHz(PHY_VARS_UE *phy_vars_ue,u8 slot, u8 eNB_id) { // apaposto


  s32 **txdata=phy_vars_ue->lte_ue_common_vars[eNB_id]->txdata; // apaposto
  u16 len;
  u32 *kHz7_5ptr;
  __m128i *txptr128,*kHz7_5ptr128,mmtmp_re,mmtmp_im,mmtmp_re2,mmtmp_im2;
  u32 slot_offset;
  u8 aa;
  u32 i;
  // LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms; // apaposto
  LTE_DL_FRAME_PARMS *frame_parms=phy_vars_ue->lte_frame_parms[eNB_id]; // apaposto

  switch (frame_parms->N_RB_UL) {
    
  case 6:
    kHz7_5ptr = (u32*)s6_kHz_7_5;
    break;
  case 15:
    kHz7_5ptr = (u32*)s15_kHz_7_5;
    break;
  case 25:
    kHz7_5ptr = (frame_parms->Ncp==0) ? (u32*)s25n_kHz_7_5 : (u32*)s25e_kHz_7_5;
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
    kHz7_5ptr = (frame_parms->Ncp==0) ? (u32*)s25n_kHz_7_5 : (u32*)s25e_kHz_7_5;
    break;
  }

  // slot_offset = (u32)slot * phy_vars_ue->lte_frame_parms.samples_per_tti/2; // apaposto
  slot_offset = (u32)slot * phy_vars_ue->lte_frame_parms[eNB_id]->samples_per_tti/2; // apaposto
  //  if ((slot&1)==1)
  //    slot_offset += (len/4);
  // len = phy_vars_ue->lte_frame_parms.samples_per_tti/2; // apaposto
  len = phy_vars_ue->lte_frame_parms[eNB_id]->samples_per_tti/2; // apaposto

  //  for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++) { // apaposto
 for (aa=0;aa<phy_vars_ue->lte_frame_parms[eNB_id]->nb_antennas_tx;aa++) {
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
  s32 **rxdata_7_5kHz=phy_vars_eNB->lte_eNB_common_vars.rxdata_7_5kHz[0];
  u16 len;
  u32 *kHz7_5ptr;
  __m128i *rxptr128,*rxptr128_7_5kHz,*kHz7_5ptr128,kHz7_5_2,mmtmp_re,mmtmp_im,mmtmp_re2,mmtmp_im2;
  u32 slot_offset,slot_offset2;
  u8 aa;
  u32 i;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_eNB->lte_frame_parms;

  switch (phy_vars_eNB->lte_frame_parms.N_RB_UL) {
    
  case 6:
    kHz7_5ptr = (u32*)s6_kHz_7_5;
    break;
  case 15:
    kHz7_5ptr = (u32*)s15_kHz_7_5;
    break;
  case 25:
    kHz7_5ptr = (frame_parms->Ncp==0) ? (u32*)s25n_kHz_7_5 : (u32*)s25e_kHz_7_5;
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
    kHz7_5ptr = (frame_parms->Ncp==0) ? (u32*)s25n_kHz_7_5 : (u32*)s25e_kHz_7_5;
    break;
  }

 
  slot_offset = (u32)slot * phy_vars_eNB->lte_frame_parms.samples_per_tti/2;
  slot_offset2 = (u32)(slot&1) * phy_vars_eNB->lte_frame_parms.samples_per_tti/2;

  len = phy_vars_eNB->lte_frame_parms.samples_per_tti/2;

  for (aa=0;aa<phy_vars_eNB->lte_frame_parms.nb_antennas_rx;aa++) {

    rxptr128        = (__m128i *)&rxdata[aa][slot_offset];
    rxptr128_7_5kHz = (__m128i *)&rxdata_7_5kHz[aa][slot_offset2];    
    kHz7_5ptr128    = (__m128i *)kHz7_5ptr;
    
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
      
      rxptr128_7_5kHz[0] = _mm_packs_epi32(mmtmp_re2,mmtmp_im2);
      rxptr128++;
      rxptr128_7_5kHz++;
      kHz7_5ptr128++;
    }
  }
}



void apply_625_Hz(PHY_VARS_UE *phy_vars_ue,s16 *prach, u8 eNB_id) { // apaposto

  u32 *Hz625ptr;
  __m128i *txptr128,*Hz625ptr128,mmtmp_re,mmtmp_im,mmtmp_re2,mmtmp_im2;
  u8 aa;
  u32 Ncp,len;
  u32 i;
  // LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms; // apaposto
  // u8 frame_type         = phy_vars_ue->lte_frame_parms.frame_type;  // apaposto
  // u8 prach_ConfigIndex  = phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.prach_ConfigIndex; // apaposto
  LTE_DL_FRAME_PARMS *frame_parms=phy_vars_ue->lte_frame_parms[eNB_id]; // apaposto
  u8 frame_type         = phy_vars_ue->lte_frame_parms[eNB_id]->frame_type; // apaposto
  u8 prach_ConfigIndex  = phy_vars_ue->lte_frame_parms[eNB_id]->prach_config_common.prach_ConfigInfo.prach_ConfigIndex; // apaposto
  u8 prach_fmt = get_prach_fmt(prach_ConfigIndex,frame_type);

  switch (prach_fmt) {
  case 0:
    Ncp = 3168;
    break;
  case 1:
  case 3:
    Ncp = 21024;
    break;
  case 2:
    Ncp = 6240;
    break;
  case 4:
    Ncp = 448;
    break;
  default:
    Ncp = 3168;
    break;
  }

  switch (frame_parms->N_RB_UL) {
    
  case 6:
    Hz625ptr = (u32*)sig625_1_25MHz;
    len = 1536 + (Ncp>>4);
    break;
  case 15:
    Hz625ptr = (u32*)sig625_2_5MHz;
    len = 3072 + (Ncp>>3);
    break;
  case 25:
    Hz625ptr = (u32*)sig625_5MHz;
    len = 6144+(Ncp>>2);
    break;
  case 50:
    Hz625ptr = (u32*)sig625_10MHz;
    len = 12288+(Ncp>>1);
    break;
  case 75:
    Hz625ptr = (u32*)sig625_15MHz;
    len = 18432+((2*Ncp)/3);
    break;
  case 100:
    Hz625ptr = (u32*)sig625_20MHz;
    len = 24576+Ncp;
    break;
  default:
    Hz625ptr = (u32*)sig625_5MHz;
    len = 6144+(Ncp>>2);
    break;
  }

  // for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++) { // apaposto
  for (aa=0;aa<phy_vars_ue->lte_frame_parms[eNB_id]->nb_antennas_tx;aa++) { // apaposto
    txptr128 = (__m128i *)prach;
    Hz625ptr128 = (__m128i *)Hz625ptr;
      // apply 7.5 kHz
    
      //      if (((slot>>1)&1) == 0) { // apply the sinusoid from the table directly
    for (i=0;i<(len>>2);i++) {
      mmtmp_re = _mm_madd_epi16(*txptr128,*Hz625ptr128);  
      // Real part of complex multiplication (note: 7_5kHz signal is conjugated for this to work)
      mmtmp_im = _mm_shufflelo_epi16(*Hz625ptr128,_MM_SHUFFLE(2,3,0,1));
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
	((short*)Hz625ptr128)[0],
	((short*)Hz625ptr128)[1],
	((short*)Hz625ptr128)[2],
	((short*)Hz625ptr128)[3],
	((short*)Hz625ptr128)[4],
	((short*)Hz625ptr128)[5],
	((short*)Hz625ptr128)[6],
	((short*)Hz625ptr128)[7]);*/
      
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
      Hz625ptr128++;
    }
  }
}

void remove_625_Hz(PHY_VARS_eNB *phy_vars_eNB,s16 *prach) {

  u32 *Hz625ptr;
  __m128i *txptr128,*Hz625ptr128,Hz625_2,mmtmp_re,mmtmp_im,mmtmp_re2,mmtmp_im2;
  u8 aa;
  u32 i,Ncp,len;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_eNB->lte_frame_parms;
  u8 frame_type         = frame_parms->frame_type;
  u8 prach_ConfigIndex  = frame_parms->prach_config_common.prach_ConfigInfo.prach_ConfigIndex; 
  u8 prach_fmt = get_prach_fmt(prach_ConfigIndex,frame_type);

  switch (prach_fmt) {
  case 0:
    Ncp = 3168;
    break;
  case 1:
  case 3:
    Ncp = 21024;
    break;
  case 2:
    Ncp = 6240;
    break;
  case 4:
    Ncp = 448;
    break;
  default:
    Ncp = 3168;
    break;
  }

  switch (frame_parms->N_RB_UL) {
    
  case 6:
    Hz625ptr = (u32*)sig625_1_25MHz;
    len = 1536 + (Ncp>>4);
    break;
  case 15:
    Hz625ptr = (u32*)sig625_2_5MHz;
    len = 3072 + (Ncp>>3) ;
    break;
  case 25:
    Hz625ptr = (u32*)sig625_5MHz;
    len = 6144+(Ncp>>2);
    break;
  case 50:
    Hz625ptr = (u32*)sig625_10MHz;
    len = 12288+(Ncp>>1);
    break;
  case 75:
    Hz625ptr = (u32*)sig625_15MHz;
    len = 18432+((2*Ncp)/3);
    break;
  case 100:
    Hz625ptr = (u32*)sig625_20MHz;
    len = 24576+Ncp;
    break;
  default:
    Hz625ptr = (u32*)sig625_5MHz;
    len = 11400;
    break;
  }

  for (aa=0;aa<phy_vars_eNB->lte_frame_parms.nb_antennas_tx;aa++) {
    txptr128 = (__m128i *)prach;
    Hz625ptr128 = (__m128i *)Hz625ptr;
      // apply 7.5 kHz
    
      //      if (((slot>>1)&1) == 0) { // apply the sinusoid from the table directly
    for (i=0;i<(len>>2);i++) {
      Hz625_2 = _mm_sign_epi16(*Hz625ptr128,*(__m128i*)&conjugate75_2[0]);
      mmtmp_re = _mm_madd_epi16(*txptr128,Hz625_2);  
      // Real part of complex multiplication (note: 7_5kHz signal is conjugated for this to work)
      mmtmp_im = _mm_shufflelo_epi16(Hz625_2,_MM_SHUFFLE(2,3,0,1));
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
	((short*)Hz625ptr128)[0],
	((short*)Hz625ptr128)[1],
	((short*)Hz625ptr128)[2],
	((short*)Hz625ptr128)[3],
	((short*)Hz625ptr128)[4],
	((short*)Hz625ptr128)[5],
	((short*)Hz625ptr128)[6],
	((short*)Hz625ptr128)[7]);*/
      
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
      Hz625ptr128++;
    }
  }
}


void init_prach625(LTE_DL_FRAME_PARMS *frame_parms) {

  u32 len,i,Ncp;
  double fs;
  s16 *Hz625ptr;
  u8 frame_type         = frame_parms->frame_type;
  u8 prach_ConfigIndex  = frame_parms->prach_config_common.prach_ConfigInfo.prach_ConfigIndex; 
  u8 prach_fmt = get_prach_fmt(prach_ConfigIndex,frame_type);

  switch (prach_fmt) {
  case 0:
    Ncp = 3168;
    break;
  case 1:
  case 3:
    Ncp = 21024;
    break;
  case 2:
    Ncp = 6240;
    break;
  case 4:
    Ncp = 448;
    break;
  default:
    Ncp = 3168;
    break;
  }

  switch (frame_parms->N_RB_UL) {
  case 6:
    len = 1536 + (Ncp>>4);
    fs = 1920000.0;
    Hz625ptr = sig625_1_25MHz;
    break;
  case 15:
    len = 3072 + (Ncp>>3) ;
    fs = 3840000.0;
    Hz625ptr = sig625_2_5MHz;
    break;
  case 25:
    len = 6144+(Ncp>>2);
    fs = 7680000.0;
    Hz625ptr = sig625_5MHz;
    break;
  case 50:
    len = 12288+(Ncp>>1);
    fs = 15360000.0;
    Hz625ptr = sig625_10MHz;
    break;
  case 75:
    len = 18432+((2*Ncp)/3);
    fs = 23040000.0;
    Hz625ptr = sig625_15MHz;
    break;
  case 100:
    len = 24576+Ncp;
    fs = 30720000.0;
    Hz625ptr = sig625_20MHz;
    break;
  default:
    len = 6144+(Ncp>>2);
    fs = 7680000.0;
    Hz625ptr = sig625_5MHz;
    break;
  }

  for (i=0;i<len;i++) {
    Hz625ptr[i<<1]     = (s16)floor(32767.0*cos(2*M_PI*625*i/fs));
    Hz625ptr[1+(i<<1)] = (s16)floor(32767.0*sin(2*M_PI*625*i/fs));
    //    printf("prach625 %d: (%d,%d)\n",i,Hz625ptr[i<<1],Hz625ptr[1+(i<<1)]);
  }

}
