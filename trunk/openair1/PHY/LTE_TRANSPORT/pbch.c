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

/*! \file PHY/LTE_TRANSPORT/pbch.c
* \brief Top-level routines for generating and decoding  the PBCH/BCH physical/transport channel V8.6 2009-03
* \author R. Knopp, F. Kaltenberger
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger.fr
* \note
* \warning
*/
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

#ifdef PHY_ABSTRACTION
#include "SIMULATION/TOOLS/defs.h"
#endif 

#ifndef __SSE3__
extern __m128i zero;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmy)))
#endif
  
//#define DEBUG_PBCH 1
//#define DEBUG_PBCH_ENCODING
//#define INTERFERENCE_MITIGATION 1

#ifdef OPENAIR2
#include "PHY_INTERFACE/defs.h"
#endif

#define PBCH_A 24

//uint8_t pbch_d[96+(3*(16+PBCH_A))], pbch_w[3*3*(16+PBCH_A)],pbch_e[1920];  //one bit per byte
int generate_pbch(LTE_eNB_PBCH *eNB_pbch,
		  mod_sym_t **txdataF,
		  int amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  uint8_t *pbch_pdu,
		  uint8_t frame_mod4) {

  int i, l;

  uint32_t  pbch_D,pbch_E;//,pbch_coded_bytes;
  uint8_t pbch_a[PBCH_A>>3];
  uint8_t RCC;

  uint32_t nsymb = (frame_parms->Ncp==NORMAL) ? 14:12;
  uint32_t pilots;
#ifdef INTERFERENCE_MITIGATION
  uint32_t pilots_2;
#endif
  uint32_t second_pilot = (frame_parms->Ncp==NORMAL) ? 4 : 3;
  uint32_t jj=0;
  uint32_t re_allocated=0;
  uint32_t rb, re_offset, symbol_offset;
  uint16_t amask=0;

  pbch_D    = 16+PBCH_A;

  pbch_E  = (frame_parms->Ncp==NORMAL) ? 1920 : 1728; //RE/RB * #RB * bits/RB (QPSK)
  //  pbch_E_bytes = pbch_coded_bits>>3;

  if (frame_mod4==0) {
    bzero(pbch_a,PBCH_A>>3);
    bzero(eNB_pbch->pbch_e,pbch_E);
    memset(eNB_pbch->pbch_d,LTE_NULL,96);
    // Encode data
    
    // CRC attachment
    //  crc = (uint16_t) (crc16(pbch_pdu, pbch_crc_bits-16) >> 16); 
    
    /*
    // scramble crc with PBCH CRC mask (Table 5.3.1.1-1 of 3GPP 36.212-860)
    switch (frame_parms->nb_antennas_tx_eNB) {
    case 1:
    crc = crc ^ (uint16_t) 0;
    break;
    case 2:
    crc = crc ^ (uint16_t) 0xFFFF;
    break;
    case 4:
    crc = crc ^ (uint16_t) 0xAAAA;
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
#ifdef DEBUG_PBCH
    for (i=0;i<(PBCH_A>>3);i++) 
      msg("[PBCH] pbch_data[%d] = %x\n",i,pbch_a[i]);
#endif
    if (frame_parms->mode1_flag == 1)
      amask = 0x0000;
    else {
      switch (frame_parms->nb_antennas_tx_eNB) {
      case 1:
	amask = 0x0000;
	break;
      case 2:
	amask = 0xffff;
	break;
      case 4:
	amask = 0x5555;
      }
    }
    ccodelte_encode(PBCH_A,2,pbch_a,eNB_pbch->pbch_d+96,amask);

     
#ifdef DEBUG_PBCH_ENCODING
    for (i=0;i<16+PBCH_A;i++)
      msg("%d : (%d,%d,%d)\n",i,*(eNB_pbch->pbch_d+96+(3*i)),*(eNB_pbch->pbch_d+97+(3*i)),*(eNB_pbch->pbch_d+98+(3*i)));
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
    msg("Doing PBCH interleaving for %d coded bits, e %p\n",pbch_D,eNB_pbch->pbch_e);
#endif
    RCC = sub_block_interleaving_cc(pbch_D,eNB_pbch->pbch_d+96,eNB_pbch->pbch_w);
    
    lte_rate_matching_cc(RCC,pbch_E,eNB_pbch->pbch_w,eNB_pbch->pbch_e);

#ifdef DEBUG_PBCH_ENCODING
    msg("PBCH_e:\n");
    for (i=0;i<pbch_E;i++)
      msg("%d %d\n",i,*(eNB_pbch->pbch_e+i));
    msg("\n");
#endif


   
#ifdef DEBUG_PBCH
#ifdef USER_MODE
    if (frame_mod4==0) {
      write_output("pbch_e.m","pbch_e",
		   eNB_pbch->pbch_e,
		   pbch_E,
		   1,
		   4);
      for (i=0;i<16;i++)
	printf("e[%d] %d\n",i,eNB_pbch->pbch_e[i]);
    }
#endif //USER_MODE
#endif //DEBUG_PBCH
    // scrambling

    pbch_scrambling(frame_parms,
		    eNB_pbch->pbch_e,
		    pbch_E);
#ifdef DEBUG_PBCH
#ifdef USER_MODE
    if (frame_mod4==0) {
      write_output("pbch_e_s.m","pbch_e_s",
		   eNB_pbch->pbch_e,
		   pbch_E,
		   1,
		   4);
      for (i=0;i<16;i++)
	printf("e_s[%d] %d\n",i,eNB_pbch->pbch_e[i]);
    }
#endif //USER_MODE
#endif //DEBUG_PBCH 
  } // frame_mod4==0

  // modulation and mapping (slot 1, symbols 0..3)
  for (l=(nsymb>>1);l<(nsymb>>1)+4;l++) {
    
    pilots=0;
#ifdef INTERFERENCE_MITIGATION
    pilots_2 = 0;
#endif
    if ((l==0) || (l==(nsymb>>1))){
      pilots=1;
#ifdef INTERFERENCE_MITIGATION
      pilots_2=1;
#endif
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

    
#ifdef IFFT_FPGA
    re_offset = frame_parms->N_RB_DL*12-3*12;
    symbol_offset = frame_parms->N_RB_DL*12*l;
#else
    re_offset = frame_parms->ofdm_symbol_size-3*12;
    symbol_offset = frame_parms->ofdm_symbol_size*l;
#endif
    
    for (rb=0;rb<6;rb++) {

#ifdef DEBUG_PBCH
      msg("RB %d, jj %d, re_offset %d, symbol_offset %d, pilots %d, nushift %d\n",rb,jj,re_offset, symbol_offset, pilots,frame_parms->nushift);
#endif
      allocate_REs_in_RB(txdataF,
			 &jj,
			 re_offset,
			 symbol_offset,
			 &eNB_pbch->pbch_e[frame_mod4*(pbch_E>>2)],
			 (frame_parms->mode1_flag == 1) ? SISO : ALAMOUTI,
			 0,
			 pilots,
			 2,
			 0,
#ifdef INTERFERENCE_MITIGATION
			 (pilots_2==1)?(amp/3):amp,
#else
			 amp,
#endif
			 NULL,
			 &re_allocated,
			 0,
			 0,
			 0,
			 1,
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
#ifdef DEBUG_PBCH
  msg("[PBCH] txdataF=\n");
  for (i=0;i<frame_parms->ofdm_symbol_size;i++) 
    msg("%d=>(%d,%d)\n",i,((short*)&txdataF[0][frame_parms->ofdm_symbol_size*(nsymb>>1)+i])[0],
	((short*)&txdataF[0][frame_parms->ofdm_symbol_size*(nsymb>>1)+i])[1]);
#endif
  
  
  return(0);
}

int32_t generate_pbch_emul(PHY_VARS_eNB *phy_vars_eNB,uint8_t *pbch_pdu) {
  
  LOG_D(PHY,"[eNB %d] generate_pbch_emul \n",phy_vars_eNB->Mod_id);
  eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].cntl.pbch_flag=1;
  // Copy PBCH payload 
  eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].cntl.pbch_payload=*(uint32_t *)pbch_pdu;
  return(0);
}

uint16_t pbch_extract(int **rxdataF,
		 int **dl_ch_estimates,
		 int **rxdataF_ext,
		 int **dl_ch_estimates_ext,
		 uint32_t symbol,
		 LTE_DL_FRAME_PARMS *frame_parms) {
  

  uint16_t rb,nb_rb=6;
  uint8_t i,j,aarx,aatx;
  int *dl_ch0,*dl_ch0_ext,*rxF,*rxF_ext;
 
  uint32_t nsymb = (frame_parms->Ncp==0) ? 7:6;
  uint32_t symbol_mod = symbol % nsymb;

  int rx_offset = frame_parms->ofdm_symbol_size-3*12;
  int ch_offset = frame_parms->N_RB_DL*6-3*12;
  int nushiftmod3 = frame_parms->nushift%3;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    /*
    printf("extract_rbs (nushift %d): symbol_mod=%d, rx_offset=%d, ch_offset=%d\n",frame_parms->nushift,symbol_mod,
	   (rx_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2,
	   LTE_CE_OFFSET+ch_offset+(symbol_mod*(frame_parms->ofdm_symbol_size)));
    */
#ifndef NEW_FFT
    rxF        = &rxdataF[aarx][(rx_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];
#else
    rxF        = &rxdataF[aarx][(rx_offset + (symbol*(frame_parms->ofdm_symbol_size)))];
#endif
    rxF_ext    = &rxdataF_ext[aarx][symbol_mod*(6*12)];

    for (rb=0; rb<nb_rb; rb++) {
      // skip DC carrier
      if (rb==3) {
#ifndef NEW_FFT
	rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
#else
	rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))];
#endif
      }
      if ((symbol_mod==0) || (symbol_mod==1)) {
	j=0;
	for (i=0;i<12;i++) {
	  if ((i!=nushiftmod3) && 
	      (i!=(nushiftmod3+3)) && 
	      (i!=(nushiftmod3+6)) && 
	      (i!=(nushiftmod3+9))) {
#ifndef NEW_FFT
	    rxF_ext[j++]=rxF[i<<1];
#else
	    rxF_ext[j++]=rxF[i];
#endif
	  }
	}
#ifndef NEW_FFT
	rxF+=24;
#else
	rxF+=12;
#endif
	rxF_ext+=8;
      }
      else {
	for (i=0;i<12;i++) {
#ifndef NEW_FFT
	  rxF_ext[i]=rxF[i<<1];
#else
	  rxF_ext[i]=rxF[i];
#endif
	}
#ifndef NEW_FFT
	rxF+=24;
#else
	rxF+=12;
#endif
	rxF_ext+=12;
      }
    }

    for (aatx=0;aatx<4;aatx++) {//frame_parms->nb_antennas_tx_eNB;aatx++) {
      dl_ch0     = &dl_ch_estimates[(aatx<<1)+aarx][LTE_CE_OFFSET+ch_offset+(symbol*(frame_parms->ofdm_symbol_size))];
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
	    if ((i!=nushiftmod3) && 
		(i!=(nushiftmod3+3)) && 
		(i!=(nushiftmod3+6)) && 
		(i!=(nushiftmod3+9))){
	      //	      printf("PBCH extract i %d j %d => (%d,%d)\n",i,j,*(short *)&dl_ch0[i],*(1+(short*)&dl_ch0[i]));
	      dl_ch0_ext[j++]=dl_ch0[i];
	    }
	  }	    
	  dl_ch0+=12;
	  dl_ch0_ext+=8;
	}
     }
    }  //tx antenna loop

  }

  return(0);
}

__m128i avg128;

//compute average channel_level on each (TX,RX) antenna pair
int pbch_channel_level(int **dl_ch_estimates_ext,
		       LTE_DL_FRAME_PARMS *frame_parms,
		       uint32_t symbol) {

  int16_t rb, nb_rb=6;
  uint8_t aatx,aarx;
  __m128i *dl_ch128;
  int avg1=0,avg2=0;

  uint32_t nsymb = (frame_parms->Ncp==0) ? 7:6;
  uint32_t symbol_mod = symbol % nsymb;

  for (aatx=0;aatx<4;aatx++) //frame_parms->nb_antennas_tx_eNB;aatx++)
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
			       uint8_t symbol,
			       uint8_t output_shift) {

  uint16_t rb,nb_rb=6;
  uint8_t aatx,aarx,symbol_mod;
  __m128i *dl_ch128,*rxdataF128,*rxdataF_comp128;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  
  for (aatx=0;aatx<4;aatx++) //frame_parms->nb_antennas_tx_eNB;aatx++)
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
			uint8_t symbol) {

  uint8_t aatx, symbol_mod;
  int i, nb_rb=6;
  __m128i *rxdataF_comp128_0,*rxdataF_comp128_1;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  if (frame_parms->nb_antennas_rx>1) {
    for (aatx=0;aatx<4;aatx++) {//frame_parms->nb_antennas_tx_eNB;aatx++) {
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
		     uint8_t *pbch_e,
		     uint32_t length) {
  int i;
  uint8_t reset;
  uint32_t x1, x2, s=0;

  reset = 1;
  // x1 is set in lte_gold_generic
  x2 = frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.6.1
  //  msg("pbch_scrambling: Nid_cell = %d\n",x2);

  for (i=0; i<length; i++) {
    if ((i&0x1f)==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //      printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }

    pbch_e[i] = (pbch_e[i]&1) ^ ((s>>(i&0x1f))&1);

  }
}

void pbch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
		       int8_t* llr,
		       uint32_t length,
		       uint8_t frame_mod4) {
  int i;
  uint8_t reset;
  uint32_t x1, x2, s=0;

  reset = 1;
  // x1 is set in first call to lte_gold_generic
  x2 = frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.6.1
  //  msg("pbch_unscrambling: Nid_cell = %d\n",x2);

  for (i=0; i<length; i++) {
    if (i%32==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //      printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    } 
    // take the quarter of the PBCH that corresponds to this frame
    if ((i>=(frame_mod4*(length>>2))) && (i<((1+frame_mod4)*(length>>2)))) {
      //      if (((s>>(i%32))&1)==1)

      if (((s>>(i%32))&1)==0)
	llr[i] = -llr[i];
    }
  }
}

void pbch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		   int **rxdataF_comp,
		   uint8_t symbol) {


  int16_t *rxF0,*rxF1;
  //  __m128i *ch_mag0,*ch_mag1,*ch_mag0b,*ch_mag1b;
  uint8_t rb,re,symbol_mod;
  int jj;

  //  printf("Doing alamouti\n");
  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  jj         = (symbol_mod*6*12);

  rxF0     = (int16_t*)&rxdataF_comp[0][jj];  //tx antenna 0  h0*y
  rxF1     = (int16_t*)&rxdataF_comp[2][jj];  //tx antenna 1  h1*y

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

void pbch_quantize(int8_t *pbch_llr8,
		   int16_t *pbch_llr,
		   uint16_t len) {

  uint16_t i;

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
static int8_t pbch_w_rx[3*3*(16+PBCH_A)],pbch_d_rx[96+(3*(16+PBCH_A))];


uint16_t rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
	    LTE_UE_PBCH *lte_ue_pbch_vars,
	    LTE_DL_FRAME_PARMS *frame_parms,
	    uint8_t eNB_id,
	    MIMO_mode_t mimo_mode,
	    uint8_t frame_mod4) {

  uint8_t log2_maxh;//,aatx,aarx;
  int max_h=0;

  int symbol,i;
  uint32_t nsymb = (frame_parms->Ncp==0) ? 14:12;
  uint16_t  pbch_E;
  uint8_t pbch_a[8];
  uint8_t RCC;

  int8_t *pbch_e_rx;
  uint8_t *decoded_output = lte_ue_pbch_vars->decoded_output;
  uint16_t crc;


  //  pbch_D    = 16+PBCH_A;

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
		 lte_ue_common_vars->dl_ch_estimates[eNB_id],
		 lte_ue_pbch_vars->rxdataF_ext,
		 lte_ue_pbch_vars->dl_ch_estimates_ext,
		 symbol,
		 frame_parms);
#ifdef DEBUG_PBCH    
    msg("[PHY] PBCH Symbol %d\n",symbol);
    msg("[PHY] PBCH starting channel_level\n");
#endif
    
    max_h = pbch_channel_level(lte_ue_pbch_vars->dl_ch_estimates_ext,
			       frame_parms,
			       symbol);
    log2_maxh = 3+(log2_approx(max_h)/2);
    
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

  memset(pbch_a,0,((16+PBCH_A)>>3));




  phy_viterbi_lte_sse2(pbch_d_rx+96,pbch_a,16+PBCH_A);
  
  // Fix byte endian of PBCH (bit 23 goes in first)
  for (i=0;i<(PBCH_A>>3);i++) 
    decoded_output[(PBCH_A>>3)-i-1] = pbch_a[i];

#ifdef DEBUG_PBCH
  for (i=0;i<(PBCH_A>>3);i++) 
    msg("[PBCH] pbch_a[%d] = %x\n",i,decoded_output[i]);

#endif //DEBUG_PBCH

#ifdef DEBUG_PBCH
  msg("PBCH CRC %x : %x\n",
      crc16(pbch_a,PBCH_A),
      ((uint16_t)pbch_a[PBCH_A>>3]<<8)+pbch_a[(PBCH_A>>3)+1]);
#endif

  crc = (crc16(pbch_a,PBCH_A)>>16) ^ 
    (((uint16_t)pbch_a[PBCH_A>>3]<<8)+pbch_a[(PBCH_A>>3)+1]);

  if (crc == 0x0000)
    return(1);
  else if (crc == 0xffff)
    return(2);
  else if (crc == 0x5555)
    return(4);
  else 
    return(-1);
  
  
}

#ifdef PHY_ABSTRACTION
uint16_t rx_pbch_emul(PHY_VARS_UE *phy_vars_ue,
		 uint8_t eNB_id,
		 uint8_t pbch_phase) {

  double bler=0.0, x=0.0;
  double sinr=0.0;
  uint16_t nb_rb = phy_vars_ue->lte_frame_parms.N_RB_DL;
  int16_t f;
  uint8_t CC_id=phy_vars_ue->CC_id;

  // compute effective sinr
  // TODO: adapt this to varible bandwidth
  for (f=(nb_rb*6-3*12);f<(nb_rb*6+3*12);f++) {
    if (f!=0) //skip DC
      sinr += pow(10, 0.1*(phy_vars_ue->sinr_dB[f]));
  }
  sinr = 10*log10(sinr/(6*12));
 
  bler = pbch_bler(sinr);
 
  LOG_D(PHY,"EMUL UE rx_pbch_emul: eNB_id %d, pbch_phase %d, sinr %f dB, bler %f \n",
	eNB_id,
	pbch_phase,
	sinr,
	bler);

  if (pbch_phase == (phy_vars_ue->frame % 4)) {
    if (uniformrandom() >= bler) {
      memcpy(phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output,PHY_vars_eNB_g[eNB_id][CC_id]->pbch_pdu,PBCH_PDU_SIZE);    
      return(PHY_vars_eNB_g[eNB_id][CC_id]->lte_frame_parms.nb_antennas_tx_eNB);
    }
    else
      return(-1);
  }
  else
    return(-1);
}
#endif
