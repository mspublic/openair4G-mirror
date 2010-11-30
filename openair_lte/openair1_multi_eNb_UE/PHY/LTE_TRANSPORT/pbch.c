#include <emmintrin.h>
#include <xmmintrin.h>
#ifdef __SSE3__
#include <pmmintrin.h>
#include <tmmintrin.h>
#endif
#include "PHY/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "defs.h"
#include "extern.h"
#include "PHY/extern.h"

#ifndef __SSE3__
extern __m128i zero;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmy)))
#endif

//#define DEBUG_PBCH
//#define DEBUG_PBCH_ENCODING

#ifdef OPENAIR2
#include "PHY_INTERFACE/defs.h"
#endif

//#define DEBUG_PBCH

/* This implements the LTE PBCH channel with the following differences form the spec 3GPP 36.212-860
 *  * we do not use the OFDM symbols 0 and 3/4 (normal/extended CP respectively)
 *      (in a later version, when we use 4 Tx antennas, we should also skip over the pilots in symbol 1)
 *  * that gives us pbch_coded_bits = 36*6*2=432 (RE/RB * #RB * bits/RB) for normal CP and
 *                  pbch_coded_bits = 24*6*2=288 for extended CP
 *  * the input length (information bits) is 64 input bits (including CRC)
 *  * we use turbo code with rate 1/3, rate matching to the corresponding size
 */

//#define DEBUG_PBCH

#define PBCH_A 24

u8 pbch_d[96+(3*(16+PBCH_A))], pbch_w[3*3*(16+PBCH_A)],pbch_e[1920];  //one bit per byte
int generate_pbch(mod_sym_t **txdataF,
		  int amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  u8 *pbch_pdu,
		  u8 frame_mod4) {



  int i, l;

  u32  pbch_D,pbch_D_bytes,pbch_E;//,pbch_coded_bytes;
  u8 pbch_a[PBCH_A>>3];
  u8 RCC;

  u32 nsymb = (frame_parms->Ncp==0) ? 14:12;
  u32 pilots;
  u32 second_pilot = (frame_parms->Ncp==0) ? 4 : 3;
  u32 jj=0;
  u32 re_allocated=0;
  u32 rb, re_offset, symbol_offset;
  u16 amask=0;

  pbch_D    = 16+PBCH_A;
  pbch_D_bytes   = pbch_D>>3;

  pbch_E  = (frame_parms->Ncp==0) ? 1920 : 1728; //RE/RB * #RB * bits/RB (QPSK)
  //  pbch_E_bytes = pbch_coded_bits>>3;

  if (frame_mod4==0) {
    bzero(pbch_a,pbch_A>>3);
    bzero(pbch_e,pbch_E);
    memset(pbch_d,LTE_NULL,96);
    // Encode data
    
    // CRC attachment
    //  crc = (u16) (crc16(pbch_pdu, pbch_crc_bits-16) >> 16); 
    
    /*
    // scramble crc with PBCH CRC mask (Table 5.3.1.1-1 of 3GPP 36.212-860)
    switch (frame_parms->nb_antennas_tx) {
    case 1:
    crc = crc ^ (u16) 0;
    break;
    case 2:
    crc = crc ^ (u16) 0xFFFF;
    break;
    case 4:
    crc = crc ^ (u16) 0xAAAA;
    break;
    default:
    msg("[PBCH] Unknown number of TX antennas!\n");
    break;
    }
    */
    
    // Fix byte endian of PBCH (bit 23 goes in first)
    for (i=0;i<(PBCH_A>>3);i++) 
      pbch_a[(PBCH_A>>3)-i-1] = pbch_pdu[i];
    //  pbch_data[i] = ((char*) &crc)[0];
    //  pbch_data[i+1] = ((char*) &crc)[1];
#ifdef DEBUG_PBCH_ENCODING
    for (i=0;i<(PBCH_A>>3);i++) 
      msg("[PBCH] pbch_data[%d] = %x\n",i,pbch_a[i]);
#endif
    switch (frame_parms->nb_antennas_tx) {
    case 1:
      amask = 0x0000;
      break;
    case 2:
      amask = 0xffff;
      break;
    case 4:
      amask = 0x5555;
    }
    ccodelte_encode(PBCH_A,2,pbch_a,pbch_d+96,amask);

     
#ifdef DEBUG_PBCH_ENCODING
#ifdef DEBUG_PBCH_ENCODING
    for (i=0;i<16+PBCH_A;i++)
      msg("%d : (%d,%d,%d)\n",i,*(pbch_d+96+(3*i)),*(pbch_d+97+(3*i)),*(pbch_d+98+(3*i)));
#endif
#endif //DEBUG_PBCH_ENCODING
    
    // Bit collection
    /*
      j2=0;
      for (j=0;j<pbch_crc_bits*3+12;j++) {
      if ((pbch_coded_data[j]&0x80) > 0) { // bit is to be transmitted
      pbch_coded_data2[j2++] = pbch_coded_data[j]&1;
      //Bit is repeated
      if ((pbch_coded_data[j]&0x40)>0)
      pbch_coded_data2[j2++] = pbch_coded_data[j]&1;
      }
      }
      
      #ifdef DEBUG_PBCH			
      msg("[PBCH] rate matched bits=%d, pbch_coded_bits=%d, pbch_crc_bits=%d\n",j2,pbch_coded_bits,pbch_crc_bits);
      #endif
      
      #ifdef DEBUG_PBCH
      #ifdef USER_MODE
      write_output("pbch_encoded_output2.m","pbch_encoded_out2",
      pbch_coded_data2,
      pbch_coded_bits,
      1,
      4);
      #endif //USER_MODE
      #endif //DEBUG_PBCH
    */
#ifdef DEBUG_PBCH_ENCODING
    msg("Doing PBCH interleaving for %d coded bits, e %p\n",pbch_D,pbch_e);
#endif
    RCC = sub_block_interleaving_cc(pbch_D,pbch_d+96,pbch_w);
    
    lte_rate_matching_cc(RCC,pbch_E,pbch_w,pbch_e);

#ifdef DEBUG_PBCH_ENCODING
    msg("PBCH_e:\n");
    for (i=0;i<pbch_E;i++)
      msg("%d %d\n",i,*(pbch_e+i));
    msg("\n");
#endif

#ifdef DEBUG_PBCH
#ifdef USER_MODE
    write_output("pbch_e.m","pbch_e",
		 pbch_e,
		 pbch_E,
		 1,
		 4);
#endif //USER_MODE
#endif //DEBUG_PBCH

    // scrambling

    pbch_scrambling(frame_parms,
		    pbch_e,
		    pbch_E);
    

  } // frame_mod4==0

  // modulation and mapping (slot 1, symbols 0..3)
  for (l=(nsymb>>1);l<(nsymb>>1)+4;l++) {
    
    pilots=0;
    if ((l==0) || (l==(nsymb>>1))){
      pilots=1;
    }

    if ((l==1) || (l==(nsymb>>1)+1)){
      pilots=1;
    }

    if ((l==second_pilot)||(l==(second_pilot+(nsymb>>1)))) {
      pilots=1;
    }

#ifdef DEBUG_PBCH
    msg("[PBCH] l=%d, pilots=%d\n",l,pilots);
#endif

    
    //    if (pilots==0) { // don't skip pilot symbols
      // This is not LTE, it guarantees that
      // pilots from adjacent base-stations
      // do not interfere with data
      // LTE is eNb centric.  "Smart" Interference
      // cancellation isn't possible
    //    printf("Doing PBCH symbol %d (pilots %d)\n",l,pilots);
#ifdef IFFT_FPGA
    re_offset = frame_parms->N_RB_DL*12-3*12;
    symbol_offset = frame_parms->N_RB_DL*12*l;
#else
    re_offset = frame_parms->ofdm_symbol_size-3*12;
    symbol_offset = frame_parms->ofdm_symbol_size*l;
#endif
    
    for (rb=0;rb<6;rb++) {

      //      printf("RB %d,jj %d\n",rb,jj);
      allocate_REs_in_RB(txdataF,
			 &jj,
			 re_offset,
			 symbol_offset,
			 &pbch_e[frame_mod4*(pbch_E>>2)],
			 (frame_parms->mode1_flag == 1) ? SISO : ALAMOUTI,
			 0,
			 pilots,
			 2,
			 0,
			 amp,
			 &re_allocated,
			 0,
			 0,
			 frame_parms);
      
      re_offset+=12; // go to next RB
      
      // check if we crossed the symbol boundary and skip DC
#ifdef IFFT_FPGA
      if (re_offset >= frame_parms->N_RB_DL*12) 
	re_offset = 0;
#else
      if (re_offset >= frame_parms->ofdm_symbol_size)
	re_offset=1;
#endif
    }
    
    //    }
  }
  
  
  return(0);
}

s32 generate_pbch_emul(PHY_VARS_eNB *phy_vars_eNb) {
  
  msg("[PHY] EMUL UE generate_pbch_emul eNB %d\n",phy_vars_eNb->Mod_id);
}

u16 pbch_extract(int **rxdataF,
		 int **dl_ch_estimates,
		 int **rxdataF_ext,
		 int **dl_ch_estimates_ext,
		 u32 symbol,
		 LTE_DL_FRAME_PARMS *frame_parms) {
  

  u16 rb,nb_rb=6;
  u8 i,j,aarx,aatx;
  int *dl_ch0,*dl_ch0_ext,*rxF,*rxF_ext;

  u32 nsymb = (frame_parms->Ncp==0) ? 7:6;
  u32 symbol_mod = symbol % nsymb;

  int rx_offset = frame_parms->ofdm_symbol_size-3*12;
  int ch_offset = frame_parms->N_RB_DL*6-3*12;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    //printf("extract_rbs: symbol_mod=%d, rx_offset=%d, ch_offset=%d\n",symbol_mod,
    //   (rx_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2,
    //   LTE_CE_OFFSET+ch_offset+(symbol_mod*(frame_parms->ofdm_symbol_size)));

    rxF        = &rxdataF[aarx][(rx_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];
    rxF_ext    = &rxdataF_ext[aarx][symbol_mod*(6*12)];

    for (rb=0; rb<nb_rb; rb++) {
      // skip DC carrier
      if (rb==3) {
	rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
      }
      if ((symbol_mod==0) || (symbol_mod==1)) {
	j=0;
	for (i=0;i<12;i++) {
	  if ((i!=frame_parms->nushift) && 
	      (i!=(frame_parms->nushift+3)) && 
	      (i!=(frame_parms->nushift+6)) && 
	      (i!=(frame_parms->nushift+9))) {

	    rxF_ext[j++]=rxF[i<<1];
	  }
	}
	rxF+=24;
	rxF_ext+=8;
      }
      else {
	for (i=0;i<12;i++) {
	  rxF_ext[i]=rxF[i<<1];
	}
	rxF+=24;
	rxF_ext+=12;
      }
    }

    for (aatx=0;aatx<4;aatx++) {//frame_parms->nb_antennas_tx;aatx++) {
      dl_ch0     = &dl_ch_estimates[(aatx<<1)+aarx][LTE_CE_OFFSET+ch_offset+(symbol_mod*(frame_parms->ofdm_symbol_size))];
      dl_ch0_ext = &dl_ch_estimates_ext[(aatx<<1)+aarx][symbol_mod*(6*12)];
      for (rb=0; rb<nb_rb; rb++) {
	// skip DC carrier
	// if (rb==3) dl_ch0++;
	if (symbol_mod>1) {
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  dl_ch0+=12;
	  dl_ch0_ext+=12;
	}
	else {
	  j=0;
	  for (i=0;i<12;i++) {
	    if ((i!=frame_parms->nushift) && 
		(i!=(frame_parms->nushift+3)) && 
		(i!=(frame_parms->nushift+6)) && 
		(i!=(frame_parms->nushift+9))){
	      //	      printf("PBCH extract i %d j %d => (%d,%d)\n",i,j,*(short *)&dl_ch0[i],*(1+(short*)&dl_ch0[i]));
	      dl_ch0_ext[j++]=dl_ch0[i];
	    }
	  }	    
	  dl_ch0+=12;
	  dl_ch0_ext+=8;
	}
     }
    }

  }

  return(0);
}

__m128i avg128;

//compute average channel_level on each (TX,RX) antenna pair
int pbch_channel_level(int **dl_ch_estimates_ext,
		       LTE_DL_FRAME_PARMS *frame_parms,
		       u32 symbol) {

  s16 rb, nb_rb=6;
  u8 aatx,aarx;
  __m128i *dl_ch128;
  int avg1=0,avg2=0;

  u32 nsymb = (frame_parms->Ncp==0) ? 7:6;
  u32 symbol_mod = symbol % nsymb;

  for (aatx=0;aatx<4;aatx++) //frame_parms->nb_antennas_tx;aatx++)
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
      //clear average level
      avg128 = _mm_xor_si128(avg128,avg128);
      dl_ch128=(__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][symbol_mod*6*12];

      for (rb=0;rb<nb_rb;rb++) {
    
	avg128 = _mm_add_epi32(avg128,_mm_madd_epi16(dl_ch128[0],dl_ch128[0]));
	avg128 = _mm_add_epi32(avg128,_mm_madd_epi16(dl_ch128[1],dl_ch128[1]));
	avg128 = _mm_add_epi32(avg128,_mm_madd_epi16(dl_ch128[2],dl_ch128[2]));

	dl_ch128+=3;	
	/*
	  if (rb==0) {
	  print_shorts("dl_ch128",&dl_ch128[0]);
	  print_shorts("dl_ch128",&dl_ch128[1]);
	  print_shorts("dl_ch128",&dl_ch128[2]);
	  }
	*/
      }

      avg1 = (((int*)&avg128)[0] + 
	      ((int*)&avg128)[1] + 
	      ((int*)&avg128)[2] + 
	      ((int*)&avg128)[3])/(nb_rb*12);

      if (avg1>avg2) 
	avg2 = avg1;

      //msg("Channel level : %d, %d\n",avg1, avg2);
    }

  _mm_empty();
  _m_empty();

  return(avg2);

}

__m128i mmtmpP0,mmtmpP1,mmtmpP2,mmtmpP3;

void pbch_channel_compensation(int **rxdataF_ext,
				int **dl_ch_estimates_ext,
				int **rxdataF_comp,
				LTE_DL_FRAME_PARMS *frame_parms,
				u8 symbol,
				u8 output_shift) {

  u16 rb,nb_rb=6;
  u8 aatx,aarx,symbol_mod;
  __m128i *dl_ch128,*rxdataF128,*rxdataF_comp128;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  
  for (aatx=0;aatx<4;aatx++) //frame_parms->nb_antennas_tx;aatx++)
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

      dl_ch128          = (__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][symbol_mod*6*12];
      rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol_mod*6*12];
      rxdataF_comp128   = (__m128i *)&rxdataF_comp[(aatx<<1)+aarx][symbol_mod*6*12];


      for (rb=0;rb<nb_rb;rb++) {
	//printf("rb %d\n",rb);
	
	// multiply by conjugated channel
	mmtmpP0 = _mm_madd_epi16(dl_ch128[0],rxdataF128[0]);
	//	print_ints("re",&mmtmpP0);
	// mmtmpP0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpP1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
	mmtmpP1 = _mm_shufflehi_epi16(mmtmpP1,_MM_SHUFFLE(2,3,0,1));
	mmtmpP1 = _mm_sign_epi16(mmtmpP1,*(__m128i*)&conjugate[0]);
	//	print_ints("im",&mmtmpP1);
	mmtmpP1 = _mm_madd_epi16(mmtmpP1,rxdataF128[0]);
	// mmtmpP1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpP0 = _mm_srai_epi32(mmtmpP0,output_shift);
	//	print_ints("re(shift)",&mmtmpP0);
	mmtmpP1 = _mm_srai_epi32(mmtmpP1,output_shift);
	//	print_ints("im(shift)",&mmtmpP1);
	mmtmpP2 = _mm_unpacklo_epi32(mmtmpP0,mmtmpP1);
	mmtmpP3 = _mm_unpackhi_epi32(mmtmpP0,mmtmpP1);
	//      print_ints("c0",&mmtmpP2);
	//	print_ints("c1",&mmtmpP3);
	rxdataF_comp128[0] = _mm_packs_epi32(mmtmpP2,mmtmpP3);
	//	print_shorts("rx:",rxdataF128);
	//	print_shorts("ch:",dl_ch128);
	//	print_shorts("pack:",rxdataF_comp128);

	// multiply by conjugated channel
	mmtmpP0 = _mm_madd_epi16(dl_ch128[1],rxdataF128[1]);
	// mmtmpP0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpP1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
	mmtmpP1 = _mm_shufflehi_epi16(mmtmpP1,_MM_SHUFFLE(2,3,0,1));
	mmtmpP1 = _mm_sign_epi16(mmtmpP1,*(__m128i*)&conjugate[0]);
	mmtmpP1 = _mm_madd_epi16(mmtmpP1,rxdataF128[1]);
	// mmtmpP1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpP0 = _mm_srai_epi32(mmtmpP0,output_shift);
	mmtmpP1 = _mm_srai_epi32(mmtmpP1,output_shift);
	mmtmpP2 = _mm_unpacklo_epi32(mmtmpP0,mmtmpP1);
	mmtmpP3 = _mm_unpackhi_epi32(mmtmpP0,mmtmpP1);
	rxdataF_comp128[1] = _mm_packs_epi32(mmtmpP2,mmtmpP3);
	//	print_shorts("rx:",rxdataF128+1);
	//	print_shorts("ch:",dl_ch128+1);
	//	print_shorts("pack:",rxdataF_comp128+1);	

	if (symbol_mod>1) {
	  // multiply by conjugated channel
	  mmtmpP0 = _mm_madd_epi16(dl_ch128[2],rxdataF128[2]);
	  // mmtmpP0 contains real part of 4 consecutive outputs (32-bit)
	  mmtmpP1 = _mm_shufflelo_epi16(dl_ch128[2],_MM_SHUFFLE(2,3,0,1));
	  mmtmpP1 = _mm_shufflehi_epi16(mmtmpP1,_MM_SHUFFLE(2,3,0,1));
	  mmtmpP1 = _mm_sign_epi16(mmtmpP1,*(__m128i*)&conjugate[0]);
	  mmtmpP1 = _mm_madd_epi16(mmtmpP1,rxdataF128[2]);
	  // mmtmpP1 contains imag part of 4 consecutive outputs (32-bit)
	  mmtmpP0 = _mm_srai_epi32(mmtmpP0,output_shift);
	  mmtmpP1 = _mm_srai_epi32(mmtmpP1,output_shift);
	  mmtmpP2 = _mm_unpacklo_epi32(mmtmpP0,mmtmpP1);
	  mmtmpP3 = _mm_unpackhi_epi32(mmtmpP0,mmtmpP1);
	  rxdataF_comp128[2] = _mm_packs_epi32(mmtmpP2,mmtmpP3);
	  //	print_shorts("rx:",rxdataF128+2);
	  //	print_shorts("ch:",dl_ch128+2);
	  //      print_shorts("pack:",rxdataF_comp128+2);
	  
	  dl_ch128+=3;
	  rxdataF128+=3;
	  rxdataF_comp128+=3;
	}
	else {
	  dl_ch128+=2;
	  rxdataF128+=2;
	  rxdataF_comp128+=2;
	}
      }
    }
  _mm_empty();
  _m_empty();
}     

void pbch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			int **rxdataF_comp,
			u8 symbol) {

  u8 aatx, symbol_mod;
  int i, nb_rb=6;
  __m128i *rxdataF_comp128_0,*rxdataF_comp128_1;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  if (frame_parms->nb_antennas_rx>1) {
    for (aatx=0;aatx<4;aatx++) {//frame_parms->nb_antennas_tx;aatx++) {
      rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[(aatx<<1)][symbol_mod*6*12];  
      rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[(aatx<<1)+1][symbol_mod*6*12];  
      // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
      for (i=0;i<nb_rb*3;i++) {
	rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
      }
    }
  }
  _mm_empty();
  _m_empty();
}

void pbch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		     u8 *pbch_e,
		     u32 length) {
  int i;
  u8 reset;
  u32 x1, x2, s=0;

  reset = 1;
  // x1 is set in lte_gold_generic
  x2 = frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.6.1

  for (i=0; i<length; i++) {
    if ((i&0x1f)==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }
    pbch_e[i] = (pbch_e[i]&1) ^ ((s>>(i&0x1f))&1);
  }
}

void pbch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
		       s8* llr,
		       u32 length,
		       u8 frame_mod4) {
  int i;
  u8 reset;
  u32 x1, x2, s=0;

  reset = 1;
  // x1 is set in first call to lte_gold_generic
  x2 = frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.6.1

  for (i=0; i<length; i++) {
    if (i%32==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }
    // take the quarter of the PBCH that corresponds to this frame
    if ((i>(frame_mod4*(length>>2))) && (i<((1+frame_mod4)*(length>>2)))) {
      //      if (((s>>(i%32))&1)==1)
      if (((s>>(i%32))&1)==0)
	llr[i] = -llr[i];
    }
  }
}

void pbch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		   int **rxdataF_comp,
		   u8 symbol) {


  s16 *rxF0,*rxF1;
  //  __m128i *ch_mag0,*ch_mag1,*ch_mag0b,*ch_mag1b;
  u8 rb,re,symbol_mod;
  int jj;


  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  jj         = (symbol_mod*6*12);

  rxF0     = (s16*)&rxdataF_comp[0][jj];  //tx antenna 0  h0*y
  rxF1     = (s16*)&rxdataF_comp[2][jj];  //tx antenna 1  h1*y

  for (rb=0;rb<6;rb++) {

    for (re=0;re<12;re+=2) {

      // Alamouti RX combining
      
      rxF0[0] = rxF0[0] + rxF1[2];
      rxF0[1] = rxF0[1] - rxF1[3];

      rxF0[2] = rxF0[2] - rxF1[0];
      rxF0[3] = rxF0[3] + rxF1[1];
 
      rxF0+=4;
      rxF1+=4;
    }

  }

  _mm_empty();
  _m_empty();
  
}

void pbch_quantize(s8 *pbch_llr8,
		   s16 *pbch_llr,
		   u16 len) {

  u16 i;

  for (i=0;i<len;i++) { 
    if (pbch_llr[i]>7)
      pbch_llr8[i]=7;
    else if (pbch_llr[i]<-8)
      pbch_llr8[i]=-8;
    else
      pbch_llr8[i] = (char)(pbch_llr[i]);

  }
}

static unsigned char dummy_w_rx[3*3*(16+PBCH_A)];
static char pbch_w_rx[3*3*(16+PBCH_A)],pbch_d_rx[96+(3*(16+PBCH_A))];


u16 rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
	    LTE_UE_PBCH *lte_ue_pbch_vars,
	    LTE_DL_FRAME_PARMS *frame_parms,
	    u8 eNb_id,
	    MIMO_mode_t mimo_mode,
	    u8 frame_mod4) {

  u8 log2_maxh;//,aatx,aarx;
  int max_h=0;

  int symbol,i,ret;
  //int nb_rb = 6;
  //int mod_order = 2;
  u32 nsymb = (frame_parms->Ncp==0) ? 14:12;
  u32 pilots, first_pilot;
  u32 second_pilot = (frame_parms->Ncp==0) ? 4 : 3;
  //  u32  pbch_crc_bits,pbch_crc_bytes,pbch_coded_bits,pbch_coded_bytes,coded_bits;
  //  u8 max_iterations = 6;

  u32  pbch_D,pbch_D_bytes,pbch_E;//,pbch_coded_bytes;
  u8 pbch_a[8];
  u8 RCC;

  s8 *pbch_e_rx;
  u8 *decoded_output = lte_ue_pbch_vars->decoded_output;
  //  s16 *channel_output = lte_ue_pbch_vars->channel_output;
  u16 crc;

  pbch_D    = 16+PBCH_A;
  pbch_D_bytes   = pbch_D>>3;

  pbch_E  = (frame_parms->Ncp==0) ? 1920 : 1728; //RE/RB * #RB * bits/RB (QPSK)
  pbch_e_rx = &lte_ue_pbch_vars->llr[frame_mod4*(pbch_E>>2)];
#ifdef DEBUG_PBCH
  msg("[PBCH] starting symbol loop\n");
#endif

  // clear LLR buffer
  memset(lte_ue_pbch_vars->llr,0,pbch_E);

  for (symbol=(nsymb>>1);symbol<(nsymb>>1)+4;symbol++) {

#ifdef DEBUG_PBCH
    msg("[PBCH] starting extract\n");
#endif
    pbch_extract(lte_ue_common_vars->rxdataF,
		 lte_ue_common_vars->dl_ch_estimates[eNb_id],
		 lte_ue_pbch_vars->rxdataF_ext,
		 lte_ue_pbch_vars->dl_ch_estimates_ext,
		 symbol,
		 frame_parms);
#ifdef DEBUG_PBCH    
    printf("[PHY] PBCH Symbol %d\n",symbol);
    msg("[PHY] PBCH starting channel_level\n");
#endif
    
    max_h = pbch_channel_level(lte_ue_pbch_vars->dl_ch_estimates_ext,
			       frame_parms,
			       symbol);
    log2_maxh = 5+(log2_approx(max_h)/2);
    
#ifdef DEBUG_PBCH
    msg("[PHY] PBCH log2_maxh = %d (%d)\n",log2_maxh,max_h);
#endif
    
    pbch_channel_compensation(lte_ue_pbch_vars->rxdataF_ext,
			      lte_ue_pbch_vars->dl_ch_estimates_ext,
			      lte_ue_pbch_vars->rxdataF_comp,
			      frame_parms,
			      symbol,
			      log2_maxh); // log2_maxh+I0_shift
    
    if (frame_parms->nb_antennas_rx > 1)
      pbch_detection_mrc(frame_parms,
			 lte_ue_pbch_vars->rxdataF_comp,
			 symbol);
    
    
    if (mimo_mode == ALAMOUTI) {
      pbch_alamouti(frame_parms,lte_ue_pbch_vars->rxdataF_comp,symbol);
      //	msg("[PBCH][RX] Alamouti receiver not yet implemented!\n");
      //	return(-1);
    }
    else if ((mimo_mode != ANTCYCLING) && (mimo_mode != SISO)) {
      msg("[PBCH][RX] Unsupported MIMO mode\n");
      return(-1);
    }
  
    if (symbol>(nsymb>>1)+1) {
      pbch_quantize(pbch_e_rx,
		    (short*)&(lte_ue_pbch_vars->rxdataF_comp[0][(symbol%(nsymb>>1))*72]),
		    144);
      
      pbch_e_rx+=144;
    }
    else {
      pbch_quantize(pbch_e_rx,
		    (short*)&(lte_ue_pbch_vars->rxdataF_comp[0][(symbol%(nsymb>>1))*72]),
		    96);

      pbch_e_rx+=96;
    }


  }

  pbch_e_rx = lte_ue_pbch_vars->llr;
  //un-scrambling
#ifdef DEBUG_PBCH
  msg("[PBCH] doing unscrambling\n");
#endif
  pbch_unscrambling(frame_parms,
		    pbch_e_rx,
		    pbch_E,
		    frame_mod4);


  //un-rate matching
#ifdef DEBUG_PBCH
  msg("[PBCH] doing un-rate-matching\n");
#endif


  memset(dummy_w_rx,0,3*3*(16+PBCH_A));
  RCC = generate_dummy_w_cc(16+PBCH_A,
			    dummy_w_rx);


  lte_rate_matching_cc_rx(RCC,pbch_E,pbch_w_rx,dummy_w_rx,pbch_e_rx);

  sub_block_deinterleaving_cc((unsigned int)(PBCH_A+16), 
			      &pbch_d_rx[96], 
			      &pbch_w_rx[0]); 

  memset(decoded_output,0,((16+PBCH_A)>>3));

  phy_viterbi_lte_sse2(pbch_d_rx+96,decoded_output,16+PBCH_A);
  
#ifdef DEBUG_PBCH
  for (i=0;i<2+(PBCH_A>>3);i++) 
    msg("[PBCH] decoded_output[%d] = %x\n",i,decoded_output[i]);
#ifdef USER_MODE
  write_output("pbch_decoded_out.m","pbch_dec_out",
	       decoded_output,
	       PBCH_A>>3,
	       1,
	       4);
#endif //USER_MODE
#endif //DEBUG_PBCH

#ifdef DEBUG_PBCH
  printf("PBCH CRC %x : %x\n",
	 crc16(decoded_output,PBCH_A),
	 ((u16)decoded_output[PBCH_A>>3]<<8)+decoded_output[(PBCH_A>>3)+1]);
#endif

  crc = (crc16(decoded_output,PBCH_A)>>16) ^ 
    (((u16)decoded_output[PBCH_A>>3]<<8)+decoded_output[(PBCH_A>>3)+1]);
  return(crc);

}


u16 rx_pbch_emul(PHY_VARS_UE *phy_vars_ue,
		 u8 eNB_id,
		 u8 pbch_phase) {

  u8 pbch_error=0;

  msg("[PHY] EMUL UE rx_pbch_emul: eNB_id %d, pbch_phase %d\n",eNB_id,pbch_phase);

  if (pbch_phase == (mac_xface->frame % 4)) {

    // abtract pbch error here
    // pbch_error = pbch_abstraction();

    if (pbch_error == 0) {
      memcpy(phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output,PHY_vars_eNb_g[eNB_id]->pbch_pdu,PBCH_PDU_SIZE);    
      return(PHY_vars_eNb_g[eNB_id]->lte_frame_parms.nb_antennas_tx);
    }
    else
      return(0xffff);
  }
  else
    return(0xffff);
}
