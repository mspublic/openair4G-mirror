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

//#define DEBUG_PHY

#ifdef OPENAIR2
#include "PHY_INTERFACE/defs.h"
#endif

//#define DEBUG_PHY

/* This implements the LTE PBCH channel with the following differences form the spec 3GPP 36.212-860
 *  * we do not use the OFDM symbols 0 and 3/4 (normal/extended CP respectively)
 *      (in a later version, when we use 4 Tx antennas, we should also skip over the pilots in symbol 1)
 *  * that gives us pbch_coded_bits = 36*6*2=432 (RE/RB * #RB * bits/RB) for normal CP and
 *                  pbch_coded_bits = 24*6*2=288 for extended CP
 *  * the input length (information bits) is 64 input bits (including CRC)
 *  * we use turbo code with rate 1/3, rate matching to the corresponding size
 */

int generate_pbch(mod_sym_t **txdataF,
		  int amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  unsigned char *pbch_pdu) {



  int i, l, j, j2;
  unsigned short crc;

  unsigned int  pbch_crc_bits,pbch_crc_bytes,pbch_coded_bits,pbch_coded_bytes;
  unsigned char pbch_data[8];
  unsigned char pbch_coded_data[36*6*2], pbch_coded_data2[36*6*2], pbch_scrambled[36*6*2];  //one bit per byte

  unsigned int nsymb = (frame_parms->Ncp==0) ? 14:12;
  unsigned int pilots, first_pilot;
  unsigned int second_pilot = (frame_parms->Ncp==0) ? 4 : 3;
  unsigned int jj=0;
  unsigned int re_allocated=0;
  unsigned int rb, re_offset, symbol_offset;


  pbch_crc_bits    = 64;
  pbch_crc_bytes   = pbch_crc_bits>>3;
  pbch_coded_bits  = (frame_parms->Ncp==0) ? 36*6*2 : 24*6*2; //RE/RB * #RB * bits/RB (QPSK)
  pbch_coded_bytes = pbch_coded_bits>>3;

  bzero(pbch_data,pbch_crc_bytes);
  bzero(pbch_coded_data,pbch_coded_bits);
  bzero(pbch_scrambled,pbch_coded_bits);

  // Encode data

  // CRC attachment
  crc = (unsigned short) (crc16(pbch_pdu, pbch_crc_bits-16) >> 16); 

  /*
  // scramble crc with PBCH CRC mask (Table 5.3.1.1-1 of 3GPP 36.212-860)
  switch (frame_parms->nb_antennas_tx) {
  case 1:
    crc = crc ^ (unsigned short) 0;
    break;
  case 2:
    crc = crc ^ (unsigned short) 0xFFFF;
    break;
  case 4:
    crc = crc ^ (unsigned short) 0xAAAA;
    break;
  default:
    msg("[PBCH] Unknown number of TX antennas!\n");
    break;
  }
  */

  // Place crc
  // double check!!!
  for (i=0;i<6;i++) 
    pbch_data[i] = pbch_pdu[i];
  pbch_data[6] = ((char*) &crc)[0];
  pbch_data[7] = ((char*) &crc)[1];
#ifdef DEBUG_PHY
  for (i=0;i<8;i++) 
    msg("[PBCH] pbch_data[%d] = %x\n",i,pbch_data[i]);
#endif

  // this is not LTE compliant! LTE uses a rate 1/3 convolutional code
  threegpplte_turbo_encoder(pbch_data,
			    pbch_crc_bytes,
			    pbch_coded_data,
			    0,
			    f1f2mat[threegpp_interleaver_parameters(pbch_crc_bytes)*2],   // f1 (see 36121-820, page 14)
			    f1f2mat[(threegpp_interleaver_parameters(pbch_crc_bytes)*2)+1]  // f2 (see 36121-820, page 14)
			    );

  // rate matching
  if (rate_matching_lte(pbch_coded_bits, 
			pbch_crc_bits*3+12, 
			pbch_coded_data,
			0) !=0 ) {
    msg("[PBCH] Rate matching problem!\n");
    return(-1);
  }

#ifdef DEBUG_PHY
#ifdef USER_MODE
  write_output("pbch_encoded_output.m","pbch_encoded_out",
	       pbch_coded_data2,
	       pbch_coded_bits,
	       1,
	       4);
#endif //USER_MODE
#endif //DEBUG_PHY

  // Bit collection
  j2=0;
  for (j=0;j<pbch_crc_bits*3+12;j++) {
    if ((pbch_coded_data[j]&0x80) > 0) { // bit is to be transmitted
      pbch_coded_data2[j2++] = pbch_coded_data[j]&1;
      //Bit is repeated
      if ((pbch_coded_data[j]&0x40)>0)
	pbch_coded_data2[j2++] = pbch_coded_data[j]&1;
    }
  }		
#ifdef DEBUG_PHY			
  msg("[PBCH] rate matched bits=%d, pbch_coded_bits=%d, pbch_crc_bits=%d\n",j2,pbch_coded_bits,pbch_crc_bits);
#endif

#ifdef DEBUG_PHY
#ifdef USER_MODE
  write_output("pbch_encoded_output2.m","pbch_encoded_out2",
	       pbch_coded_data2,
	       pbch_coded_bits,
	       1,
	       4);
#endif //USER_MODE
#endif //DEBUG_PHY

  // scrambling
  pbch_scrambling(frame_parms,
		  pbch_coded_data2,
		  pbch_coded_bits);

#ifdef DEBUG_PHY
#ifdef USER_MODE
  write_output("pbch_encoded_output3.m","pbch_encoded_out3",
	       pbch_coded_data2,
	       pbch_coded_bits,
	       1,
	       4);
#endif //USER_MODE
#endif //DEBUG_PHY

  // modulation and mapping 
  for (l=(nsymb>>1);l<(nsymb>>1)+4;l++) {
    
    pilots=0;
    first_pilot = 0;
    if ((l==0) || (l==(nsymb>>1))){
      pilots=1;
      first_pilot=1;
    }

    if ((l==second_pilot)||(l==(second_pilot+(nsymb>>1)))) {
      pilots=1;
      first_pilot=0;
    }

#ifdef DEBUG_PHY
    msg("[PBCH] l=%d, pilots=%d, first_pilot=%d\n",l,pilots,first_pilot);
#endif

    if (pilots==0) { // don't skip pilot symbols
      // This is not LTE, it guarantees that
      // pilots from adjacent base-stations
      // do not interfere with data
      // LTE is eNb centric.  "Smart" Interference
      // cancellation isn't possible
#ifdef IFFT_FPGA
      re_offset = frame_parms->N_RB_DL*12-3*12;
      symbol_offset = frame_parms->N_RB_DL*12*l;
#else
      re_offset = frame_parms->ofdm_symbol_size-3*12;
      symbol_offset = frame_parms->ofdm_symbol_size*l;
#endif
      
      for (rb=0;rb<6;rb++) {
	
	allocate_REs_in_RB(txdataF,
			   &jj,
			   re_offset,
			   symbol_offset,
			   pbch_coded_data2,
			   (frame_parms->mode1_flag == 1) ? SISO : ALAMOUTI,
			   0,
			   pilots,
			   first_pilot,
			   2,
			   0,
			   amp,
			   &re_allocated,
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
	
    }
  }


  return(0);
}

unsigned short pbch_extract(int **rxdataF,
			    int **dl_ch_estimates,
			    int **rxdataF_ext,
			    int **dl_ch_estimates_ext,
			    unsigned int symbol,
			    LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short rb,nb_rb=6;
  unsigned char i,aarx,aatx;
  int *dl_ch0,*dl_ch0_ext,*rxF,*rxF_ext;

  unsigned int nsymb = (frame_parms->Ncp==0) ? 7:6;
  unsigned int symbol_mod = symbol % nsymb;

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
      for (i=0;i<12;i++) {
	rxF_ext[i]=rxF[i<<1];
      }
      rxF+=24;
      rxF_ext+=12;
    }

    for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++) {
      dl_ch0     = &dl_ch_estimates[(aatx<<1)+aarx][LTE_CE_OFFSET+ch_offset+(symbol_mod*(frame_parms->ofdm_symbol_size))];
      dl_ch0_ext = &dl_ch_estimates_ext[(aatx<<1)+aarx][symbol_mod*(6*12)];
      for (rb=0; rb<nb_rb; rb++) {
	// skip DC carrier
	// if (rb==3) dl_ch0++;
	memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
#ifdef KHZ66_NULL
	if (rb==3) {
	  dl_ch0_ext[2]=0;
	  dl_ch0_ext[3]=0;
	  dl_ch0_ext[4]=0;
	}
#endif
	dl_ch0+=12;
	dl_ch0_ext+=12;
      }
    }

  }
  return(0);
}

__m128i avg128;

//compute average channel_level on each (TX,RX) antenna pair
int pbch_channel_level(int **dl_ch_estimates_ext,
		       LTE_DL_FRAME_PARMS *frame_parms,
		       unsigned int symbol) {

  short rb, nb_rb=6;
  unsigned char aatx,aarx;
  __m128i *dl_ch128;
  int avg1=0,avg2=0;

  unsigned int nsymb = (frame_parms->Ncp==0) ? 7:6;
  unsigned int symbol_mod = symbol % nsymb;

  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
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

  return(avg2);

}

__m128i mmtmpP0,mmtmpP1,mmtmpP2,mmtmpP3;

void pbch_channel_compensation(int **rxdataF_ext,
				int **dl_ch_estimates_ext,
				int **rxdataF_comp,
				LTE_DL_FRAME_PARMS *frame_parms,
				unsigned char symbol,
				unsigned char output_shift) {

  unsigned short rb,nb_rb=6;
  unsigned char aatx,aarx,symbol_mod;
  __m128i *dl_ch128,*rxdataF128,*rxdataF_comp128;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  
  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
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
    }
}     

void pbch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			int **rxdataF_comp,
			unsigned char symbol) {

  unsigned char aatx, symbol_mod;
  int i, nb_rb=6;
  __m128i *rxdataF_comp128_0,*rxdataF_comp128_1;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  if (frame_parms->nb_antennas_rx>1) {
    for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++) {
      rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[(aatx<<1)][symbol_mod*6*12];  
      rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[(aatx<<1)+1][symbol_mod*6*12];  
      // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
      for (i=0;i<nb_rb*3;i++) {
	rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
      }
    }
  }
}

void pbch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		     unsigned char* coded_data,
		     unsigned int length) {
  int i;
  unsigned char reset;
  unsigned int x1, x2, s=0;

  reset = 1;
  // x1 is set in lte_gold_generic
  x2 = frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.6.1

  for (i=0; i<length; i++) {
    if (i%32==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }
    coded_data[i] = (coded_data[i]&1) ^ ((s>>(i%32))&1);
  }
}

void pbch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
		       short* llr,
		       unsigned int length) {
  int i;
  unsigned char reset;
  unsigned int x1, x2, s=0;

  reset = 1;
  // x1 is set in first call to lte_gold_generic
  x2 = frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.6.1

  for (i=0; i<length; i++) {
    if (i%32==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }
    if (((s>>(i%32))&1)==1)
      llr[i] = -llr[i];
  }
}

void pbch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		   int **rxdataF_comp,
		   unsigned char symbol) {


  short *rxF0,*rxF1;
  //  __m128i *ch_mag0,*ch_mag1,*ch_mag0b,*ch_mag1b;
  unsigned char rb,re,symbol_mod;
  int jj;


  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  jj         = (symbol_mod*6*12);

  rxF0     = (short*)&rxdataF_comp[0][jj];  //tx antenna 0  h0*y
  rxF1     = (short*)&rxdataF_comp[2][jj];  //tx antenna 1  h1*y

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

int rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
	    LTE_UE_PBCH *lte_ue_pbch_vars,
	    LTE_DL_FRAME_PARMS *frame_parms,
	    unsigned char eNb_id,
	    MIMO_mode_t mimo_mode) {

  unsigned char log2_maxh;//,aatx,aarx;
  int max_h=0;

  int symbol,i,ret;
  //int nb_rb = 6;
  //int mod_order = 2;
  unsigned int nsymb = (frame_parms->Ncp==0) ? 14:12;
  unsigned int pilots, first_pilot;
  unsigned int second_pilot = (frame_parms->Ncp==0) ? 4 : 3;
  unsigned int  pbch_crc_bits,pbch_crc_bytes,pbch_coded_bits,pbch_coded_bytes,coded_bits;
  unsigned char max_iterations = 6;

  short* pbch_llr = lte_ue_pbch_vars->llr;
  unsigned char *decoded_output = lte_ue_pbch_vars->decoded_output;
  short *channel_output = lte_ue_pbch_vars->channel_output;
  unsigned char dummy_channel_output[36*6*2]; //corresponds to max coded bits

  pbch_crc_bits    = 64;
  pbch_crc_bytes   = pbch_crc_bits>>3;
  pbch_coded_bits  = (frame_parms->Ncp==0) ? 36*6*2 : 24*6*2; //RE/RB * #RB * bits/RB (QPSK)
  pbch_coded_bytes = pbch_coded_bits>>3;

#ifdef DEBUG_PHY
  msg("[PBCH] starting symbol loop\n");
#endif

  for (symbol=(nsymb>>1);symbol<(nsymb>>1)+4;symbol++) {

    pilots=0;
    first_pilot = 0;
    if ((symbol==0) || (symbol==(nsymb>>1))){
      pilots=1;
      first_pilot=1;
    }

    if ((symbol==second_pilot)||(symbol==(second_pilot+(nsymb>>1)))) {
      pilots=1;
      first_pilot=0;
    }

    if (pilots==0) { 

#ifdef DEBUG_PHY
      msg("[PBCH] starting extract\n");
#endif
      pbch_extract(lte_ue_common_vars->rxdataF,
		   lte_ue_common_vars->dl_ch_estimates[eNb_id],
		   lte_ue_pbch_vars->rxdataF_ext,
		   lte_ue_pbch_vars->dl_ch_estimates_ext,
		   symbol,
		   frame_parms);

#ifdef DEBUG_PHY
      msg("[PBCH] starting channel_level\n");
#endif

      max_h = pbch_channel_level(lte_ue_pbch_vars->dl_ch_estimates_ext,
				 frame_parms,
				 symbol);
      log2_maxh = 4+(log2_approx(max_h)/2);

#ifdef DEBUG_PHY
      msg("[PBCH] log2_maxh = %d (%d)\n",log2_maxh,max_h);
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

      memcpy(pbch_llr,&(lte_ue_pbch_vars->rxdataF_comp[0][(symbol%(nsymb>>1))*72]),72*sizeof(int));
      pbch_llr+=144;
    }
  }

  //un-scrambling
#ifdef DEBUG_PHY
  msg("[PBCH] doing unscrambling\n");
#endif
  pbch_unscrambling(frame_parms,
		    lte_ue_pbch_vars->llr,
		    pbch_coded_bits);

  //un-rate matching
#ifdef DEBUG_PHY
  msg("[PBCH] doing un-rate-matching\n");
#endif

  bzero(dummy_channel_output,pbch_coded_bits);
  if (rate_matching_lte(pbch_coded_bits, 
			pbch_crc_bits*3+12, 
			dummy_channel_output,
			0) !=0 ) {
    msg("[openair1][PBCH] Rate matching problem!\n");
    return(-1);
  }

  coded_bits=0;
  pbch_llr = lte_ue_pbch_vars->llr;
  for (i=0;i<(3*pbch_crc_bits)+12;i++) {
    if ((dummy_channel_output[i]&0x40) != 0) { // bit was repeated
      coded_bits++;
      channel_output[i] = *pbch_llr;
      //printf("%d,%d : %d (%d)\n",coded_bits,i,channel_output[i],dummy_channel_output[i]);
      pbch_llr++;
      coded_bits++;
      channel_output[i] += *pbch_llr;
      //printf("%d,%d : %d (%d)\n",coded_bits,i,channel_output[i],dummy_channel_output[i]);
      pbch_llr++;
    }
    else if ((dummy_channel_output[i]&0x80) != 0) { // bit was transmitted
      coded_bits++;
      channel_output[i] = *pbch_llr;
      //printf("%d,%d : %d (%d)\n",coded_bits,i,channel_output[i],dummy_channel_output[i]);
      pbch_llr++;
    }
    else {     //bit was punctured
      channel_output[i] = 0;
    }      
  }


#ifdef DEBUG_PHY
#ifdef USER_MODE
  write_output("pbch_channel_out.m","pbch_chan_out",
	       channel_output,
	       3*pbch_crc_bits+12,
	       1,
	       4);
#endif //USER_MODE
#endif //DEBUG_PHY

  //turbo decoding
#ifdef DEBUG_PHY
  msg("[PBCH] doing turbo-decoding\n");
#endif

  bzero(decoded_output,pbch_crc_bits);//block_length);
  ret = phy_threegpplte_turbo_decoder(channel_output,
                                      decoded_output,
                                      pbch_crc_bits,
				      f1f2mat[threegpp_interleaver_parameters(pbch_crc_bytes)*2],   // f1 (see 36121-820, page 14)
				      f1f2mat[(threegpp_interleaver_parameters(pbch_crc_bytes)*2)+1],  // f2 (see 36121-820, page 14)
                                      max_iterations,
                                      CRC16,
				      0,
				      0);
#ifdef DEBUG_PHY
  msg("[PBCH] ret=%d\n",ret);
  for (i=0;i<8;i++) 
    msg("[PBCH] decoded_output[%d] = %x\n",i,decoded_output[i]);
#ifdef USER_MODE
  write_output("pbch_decoded_out.m","pbch_dec_out",
	       decoded_output,
	       pbch_crc_bits,
	       1,
	       4);
#endif //USER_MODE
#endif //DEBUG_PHY



  return(ret<=max_iterations);

}
