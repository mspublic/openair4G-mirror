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

/*! \file PHY/LTE_TRANSPORT/dci.c
* \brief Implements PDCCH physical channel TX/RX procedures (36.211) and DCI encoding/decoding (36.212/36.213). Current LTE compliance V8.6 2009-03.
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/
#ifdef USER_MODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/defs.h"
#include "SIMULATION/TOOLS/defs.h" // for taus 
#include <emmintrin.h>
#include <xmmintrin.h>
#ifdef __SSE3__
#include <pmmintrin.h>
#include <tmmintrin.h>
#endif

#include "assertions.h"

//#define DEBUG_DCI_ENCODING 1
//#define DEBUG_DCI_DECODING 1
//#define DEBUG_PHY
 
//#undef ALL_AGGREGATION

#ifndef __SSE3__
__m128i zero2;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero2,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero2,(xmmy)))
#endif

//extern uint16_t phich_reg[MAX_NUM_PHICH_GROUPS][3];
//extern uint16_t pcfich_reg[4];

uint32_t check_phich_reg(LTE_DL_FRAME_PARMS *frame_parms,uint32_t kprime,uint8_t lprime,uint8_t mi) {

  uint16_t i;
  uint16_t Ngroup_PHICH = (frame_parms->phich_config_common.phich_resource*frame_parms->N_RB_DL)/48;
  uint16_t mprime;
  uint16_t *pcfich_reg = frame_parms->pcfich_reg;

  if ((lprime>0) && (frame_parms->Ncp==0) )
    return(0);

  //  printf("check_phich_reg : mi %d\n",mi);

  // compute REG based on symbol
  if ((lprime == 0)||
      ((lprime==1)&&(frame_parms->nb_antennas_tx_eNB == 4)))
    mprime = kprime/6;
  else
    mprime = kprime>>2;

  // check if PCFICH uses mprime
  if ((lprime==0) && 
      ((mprime == pcfich_reg[0]) ||
       (mprime == pcfich_reg[1]) ||
       (mprime == pcfich_reg[2]) ||
       (mprime == pcfich_reg[3]))) {
#ifdef DEBUG_DCI_ENCODING
    msg("[PHY] REG %d allocated to PCFICH\n",mprime);
#endif
    return(1);
  }

  // handle Special subframe case for TDD !!!

  //  printf("Checking phich_reg %d\n",mprime);
  if (mi > 0) {
    if (((frame_parms->phich_config_common.phich_resource*frame_parms->N_RB_DL)%48) > 0)
      Ngroup_PHICH++;
    
    if (frame_parms->Ncp == 1) {
      Ngroup_PHICH<<=1;
    }
    
    
    
    for (i=0;i<Ngroup_PHICH;i++) {
      if ((mprime == frame_parms->phich_reg[i][0]) || 
	  (mprime == frame_parms->phich_reg[i][1]) || 
	  (mprime == frame_parms->phich_reg[i][2]))  {
#ifdef DEBUG_DCI_ENCODING
	msg("[PHY] REG %d (lprime %d) allocated to PHICH\n",mprime,lprime);
#endif
	return(1);
      }
    }
  }
  return(0);
}

uint16_t extract_crc(uint8_t *dci,uint8_t dci_len) {

  uint16_t crc16;
  //  uint8_t i;

  /*
  uint8_t crc;
  crc = ((uint16_t *)dci)[DCI_LENGTH>>4];
  printf("crc1: %x, shift %d (DCI_LENGTH %d)\n",crc,DCI_LENGTH&0xf,DCI_LENGTH);
  crc = (crc>>(DCI_LENGTH&0xf));
  // clear crc bits
  ((uint16_t *)dci)[DCI_LENGTH>>4] &= (0xffff>>(16-(DCI_LENGTH&0xf)));
  printf("crc2: %x, dci0 %x\n",crc,((int16_t *)dci)[DCI_LENGTH>>4]);
  crc |= (((uint16_t *)dci)[1+(DCI_LENGTH>>4)])<<(16-(DCI_LENGTH&0xf));
  // clear crc bits
  (((uint16_t *)dci)[1+(DCI_LENGTH>>4)]) = 0;
  printf("extract_crc: crc %x\n",crc);
  */
#ifdef DEBUG_DCI_DECODING  
  msg("dci_crc (%x,%x,%x), dci_len&0x7=%d\n",dci[dci_len>>3],dci[1+(dci_len>>3)],dci[2+(dci_len>>3)],
	 dci_len&0x7);
#endif
  if ((dci_len&0x7) > 0) {
    ((uint8_t *)&crc16)[0] = dci[1+(dci_len>>3)]<<(dci_len&0x7) | dci[2+(dci_len>>3)]>>(8-(dci_len&0x7));
    ((uint8_t *)&crc16)[1] = dci[(dci_len>>3)]<<(dci_len&0x7) | dci[1+(dci_len>>3)]>>(8-(dci_len&0x7));
  }
  else {
    ((uint8_t *)&crc16)[0] = dci[1+(dci_len>>3)];
    ((uint8_t *)&crc16)[1] = dci[(dci_len>>3)];
  }

#ifdef DEBUG_DCI_DECODING  
  msg("dci_crc =>%x\n",crc16);
#endif

  //  dci[(dci_len>>3)]&=(0xffff<<(dci_len&0xf));
  //  dci[(dci_len>>3)+1] = 0;
  //  dci[(dci_len>>3)+2] = 0;
  return((uint16_t)crc16);

}



static uint8_t d[3*(MAX_DCI_SIZE_BITS + 16) + 96];
static uint8_t w[3*3*(MAX_DCI_SIZE_BITS+16)];

void dci_encoding(uint8_t *a,
		  uint8_t A,
		  uint16_t E,
		  uint8_t *e,
		  uint16_t rnti) {


  uint8_t D = (A + 16);
  uint32_t RCC;

#ifdef DEBUG_DCI_ENCODING
  int32_t i;
#endif
  // encode dci 

#ifdef DEBUG_DCI_ENCODING
  msg("Doing DCI encoding for %d bits, e %p, rnti %x\n",A,e,rnti);
#endif

  memset((void *)d,LTE_NULL,96);

  ccodelte_encode(A,2,a,d+96,rnti);

#ifdef DEBUG_DCI_ENCODING
  for (i=0;i<16+A;i++)
    msg("%d : (%d,%d,%d)\n",i,*(d+96+(3*i)),*(d+97+(3*i)),*(d+98+(3*i)));
#endif
  
#ifdef DEBUG_DCI_ENCODING
  msg("Doing DCI interleaving for %d coded bits, e %p\n",D*3,e);
#endif
  RCC = sub_block_interleaving_cc(D,d+96,w);

#ifdef DEBUG_DCI_ENCODING
  msg("Doing DCI rate matching for %d channel bits, RCC %d, e %p\n",E,RCC,e);
#endif
  lte_rate_matching_cc(RCC,E,w,e);


}


uint8_t *generate_dci0(uint8_t *dci,
		  uint8_t *e,
		  uint8_t DCI_LENGTH,
		  uint8_t aggregation_level,
		  uint16_t rnti) {
  
  uint16_t coded_bits;
  uint8_t dci_flip[8];

  if (aggregation_level>3) {
    msg("dci.c: generate_dci FATAL, illegal aggregation_level %d\n",aggregation_level);
    return NULL;
  }

  coded_bits = 72 * (1<<aggregation_level);
  /*  

#ifdef DEBUG_DCI_ENCODING
  for (i=0;i<1+((DCI_LENGTH+16)/8);i++)
    msg("i %d : %x\n",i,dci[i]);
#endif
  */
  if (DCI_LENGTH<=32){
    dci_flip[0] = dci[3];
    dci_flip[1] = dci[2];
    dci_flip[2] = dci[1];
    dci_flip[3] = dci[0];   
  }
  else {
    dci_flip[0] = dci[7];
    dci_flip[1] = dci[6];
    dci_flip[2] = dci[5];
    dci_flip[3] = dci[4];
    dci_flip[4] = dci[3];
    dci_flip[5] = dci[2];
    dci_flip[6] = dci[1];
    dci_flip[7] = dci[0];
  }
	
  dci_encoding(dci_flip,DCI_LENGTH,coded_bits,e,rnti);

  return(e+coded_bits);
}

uint32_t Y;

#define CCEBITS 72
#define CCEPERSYMBOL 33  // This is for 1200 RE
#define CCEPERSYMBOL0 22  // This is for 1200 RE
#define DCI_BITS_MAX ((2*CCEPERSYMBOL+CCEPERSYMBOL0)*CCEBITS)
#define Msymb (DCI_BITS_MAX/2)
//#define Mquad (Msymb/4)

static uint32_t bitrev_cc_dci[32] = {1,17,9,25,5,21,13,29,3,19,11,27,7,23,15,31,0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30};
static mod_sym_t wtemp[2][Msymb];

void pdcch_interleaving(LTE_DL_FRAME_PARMS *frame_parms,mod_sym_t **z, mod_sym_t **wbar,uint8_t n_symbols_pdcch,uint8_t mi) {

  mod_sym_t *wptr,*wptr2,*zptr;
  uint32_t Mquad = get_nquad(n_symbols_pdcch,frame_parms,mi);
  uint32_t RCC = (Mquad>>5), ND;
  uint32_t row,col,Kpi,index;
  int32_t i,k,a;
#ifdef RM_DEBUG
  int32_t nulled=0;
#endif
  //  msg("[PHY] PDCCH Interleaving Mquad %d (Nsymb %d)\n",Mquad,n_symbols_pdcch);
  if ((Mquad&0x1f) > 0)
    RCC++;
  Kpi = (RCC<<5);
  ND = Kpi - Mquad;

  k=0;
  for (col=0;col<32;col++) {
    index = bitrev_cc_dci[col];

    for (row=0;row<RCC;row++) {
      //msg("col %d, index %d, row %d\n",col,index,row);
      if (index>=ND) {
	for (a=0;a<frame_parms->nb_antennas_tx_eNB;a++){
	  //msg("a %d k %d\n",a,k);

	  wptr = &wtemp[a][k<<2];
	  zptr = &z[a][(index-ND)<<2];

	  //msg("wptr=%p, zptr=%p\n",wptr,zptr);

	  wptr[0] = zptr[0];
	  wptr[1] = zptr[1];
	  wptr[2] = zptr[2];
	  wptr[3] = zptr[3];
	}
	k++;
      }
      index+=32;
    }
  }

  // permutation
  for (i=0;i<Mquad;i++) {

    for (a=0;a<frame_parms->nb_antennas_tx_eNB;a++) {
      
      //wptr  = &wtemp[a][i<<2];
      //wptr2 = &wbar[a][((i+frame_parms->Nid_cell)%Mquad)<<2];
      wptr = &wtemp[a][((i+frame_parms->Nid_cell)%Mquad)<<2];
      wptr2 = &wbar[a][i<<2];
      wptr2[0] = wptr[0];
      wptr2[1] = wptr[1];
      wptr2[2] = wptr[2];
      wptr2[3] = wptr[3];
    }
  }
}

void pdcch_demapping(uint16_t *llr,uint16_t *wbar,LTE_DL_FRAME_PARMS *frame_parms,uint8_t num_pdcch_symbols,uint8_t mi) {

  uint32_t i, lprime;
  uint16_t kprime,kprime_mod12,mprime,symbol_offset,tti_offset,tti_offset0;
  int16_t re_offset,re_offset0;

  // This is the REG allocation algorithm from 36-211, second part of Section 6.8.5

  int Msymb2;

  switch (frame_parms->N_RB_DL) {
  case 100:
    Msymb2 = Msymb;
    break;
  case 75:
    Msymb2 = 3*Msymb/4;
    break;
  case 50:
    Msymb2 = Msymb>>1;
    break;
  case 25:
    Msymb2 = Msymb>>2;
    break;
  case 15:
    Msymb2 = Msymb*15/100;
    break;
  case 6:
    Msymb2 = Msymb*6/100;
    break;
  default:
    Msymb2 = Msymb>>2;
    break;
  }
  mprime=0;


  re_offset = 0;
  re_offset0 = 0; // counter for symbol with pilots (extracted outside!)
  
  for (kprime=0;kprime<frame_parms->N_RB_DL*12;kprime++) {
    for (lprime=0;lprime<num_pdcch_symbols;lprime++) {

      symbol_offset = (uint32_t)frame_parms->N_RB_DL*12*lprime;
  
      tti_offset = symbol_offset + re_offset;
      tti_offset0 = symbol_offset + re_offset0;
      // if REG is allocated to PHICH, skip it
      if (check_phich_reg(frame_parms,kprime,lprime,mi) == 1) {
	//msg("dci_demapping : skipping REG %d\n",(lprime==0)?kprime/6 : kprime>>2);
	if ((lprime == 0)&&((kprime%6)==0))
	  re_offset0+=4;
      }
      else {  // not allocated to PHICH/PCFICH
	//	msg("dci_demapping: REG %d\n",(lprime==0)?kprime/6 : kprime>>2);
	if (lprime == 0) {
	  // first symbol, or second symbol+4 TX antennas skip pilots
	  kprime_mod12 = kprime%12;
	  if ((kprime_mod12 == 0) || (kprime_mod12 == 6)) {
	    // kprime represents REG	    

	    for (i=0;i<4;i++) {
	      wbar[mprime] = llr[tti_offset0+i];
#ifdef DEBUG_DCI_DECODING
	      msg("[PHY] PDCCH demapping mprime %d => %d (symbol %d re %d) -> (%d,%d)\n",mprime,tti_offset0,symbol_offset,re_offset0,*(char*)&wbar[mprime],*(1+(char*)&wbar[mprime]));
#endif
	      mprime++;
	      re_offset0++;
	    }
	  }
	}
	else if ((lprime==1)&&(frame_parms->nb_antennas_tx_eNB == 4)) {  
	  // LATER!!!!
	}
	else { // no pilots in this symbol
	  kprime_mod12 = kprime%12;
	  if ((kprime_mod12 == 0) || (kprime_mod12 == 4) || (kprime_mod12 == 8)) {
	    // kprime represents REG	    
	    for (i=0;i<4;i++) {
	      wbar[mprime] = llr[tti_offset+i];
#ifdef DEBUG_DCI_DECODING
	      msg("[PHY] PDCCH demapping mprime %d => %d (symbol %d re %d) -> (%d,%d)\n",mprime,tti_offset,symbol_offset,re_offset+i,*(char*)&wbar[mprime],*(1+(char*)&wbar[mprime]));
#endif
		mprime++;
	    }
	  }  // is representative
	} // no pilots case	
      } // not allocated to PHICH/PCFICH

      // Stop when all REGs are copied in
      if (mprime>=Msymb2)
	break;
    } //lprime loop
    re_offset++;

  } // kprime loop
}

static uint16_t wtemp_rx[Msymb];
void pdcch_deinterleaving(LTE_DL_FRAME_PARMS *frame_parms,uint16_t *z, uint16_t *wbar,uint8_t number_pdcch_symbols,uint8_t mi) {

  uint16_t *wptr,*zptr,*wptr2;

  uint16_t Mquad=get_nquad(number_pdcch_symbols,frame_parms,mi);
  uint32_t RCC = (Mquad>>5), ND;
  uint32_t row,col,Kpi,index;
  int32_t i,k;


  //  printf("Mquad %d, RCC %d\n",Mquad,RCC);

  if (!z) {
    msg("dci.c: pdcch_deinterleaving: FATAL z is Null\n");
    return;
  }
  // undo permutation
  for (i=0;i<Mquad;i++) {
    //wptr = &wtemp_rx[i<<2];
    //wptr2 = &wbar[((i+frame_parms->Nid_cell)%Mquad)<<2];
    wptr = &wtemp_rx[((i+frame_parms->Nid_cell)%Mquad)<<2];
    wptr2 = &wbar[i<<2];

    wptr[0] = wptr2[0];
    wptr[1] = wptr2[1];
    wptr[2] = wptr2[2];
    wptr[3] = wptr2[3];
    /*
    msg("pdcch_deinterleaving (%p,%p): quad %d -> (%d,%d %d,%d %d,%d %d,%d)\n",wptr,wptr2,i,
	((char*)wptr2)[0],
	((char*)wptr2)[1],
	((char*)wptr2)[2],
	((char*)wptr2)[3],
	((char*)wptr2)[4],
	((char*)wptr2)[5],
	((char*)wptr2)[6],
	((char*)wptr2)[7]);
    */	

  }

  if ((Mquad&0x1f) > 0)
    RCC++;
  Kpi = (RCC<<5);
  ND = Kpi - Mquad;

  k=0;
  for (col=0;col<32;col++) {
    index = bitrev_cc_dci[col];

    for (row=0;row<RCC;row++) {
      //      printf("row %d, index %d, Nd %d\n",row,index,ND);
      if (index>=ND) {



	wptr = &wtemp_rx[k<<2];
	zptr = &z[(index-ND)<<2];
	
	zptr[0] = wptr[0];
	zptr[1] = wptr[1];
	zptr[2] = wptr[2];
	zptr[3] = wptr[3];

	/*
	printf("deinterleaving ; k %d, index-Nd %d  => (%d,%d,%d,%d,%d,%d,%d,%d)\n",k,(index-ND),
	       ((int8_t *)wptr)[0],
	       ((int8_t *)wptr)[1],
	       ((int8_t *)wptr)[2],
	       ((int8_t *)wptr)[3],
	       ((int8_t *)wptr)[4],
	       ((int8_t *)wptr)[5],
	       ((int8_t *)wptr)[6],
	       ((int8_t *)wptr)[7]);
	*/
	k++;
      }
      index+=32;
            
    }
  }

  for (i=0;i<Mquad;i++) {
    zptr = &z[i<<2];
    /*
    printf("deinterleaving ; quad %d  => (%d,%d,%d,%d,%d,%d,%d,%d)\n",i,
	   ((int8_t *)zptr)[0],
	   ((int8_t *)zptr)[1],
	   ((int8_t *)zptr)[2],
	   ((int8_t *)zptr)[3],
	   ((int8_t *)zptr)[4],
	   ((int8_t *)zptr)[5],
	   ((int8_t *)zptr)[6],
	   ((int8_t *)zptr)[7]);
    */
  }
  
}


int32_t pdcch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
			 int32_t **rxdataF_comp,
			 int32_t **rxdataF_comp_i,
			 int32_t **rho_i,
			 int16_t *pdcch_llr16,
			 int16_t *pdcch_llr8in,
			 uint8_t symbol) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *rxF_i=(__m128i*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *rho=(__m128i*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *llr128;
  int32_t i;
  char *pdcch_llr8;
  int16_t *pdcch_llr;
  pdcch_llr8 = (char *)&pdcch_llr8in[symbol*frame_parms->N_RB_DL*12];
  pdcch_llr = &pdcch_llr16[symbol*frame_parms->N_RB_DL*12];

  //  printf("dlsch_qpsk_qpsk: symbol %d\n",symbol);
  
  llr128 = (__m128i*)pdcch_llr;

  if (!llr128) {
    msg("dlsch_qpsk_qpsk_llr: llr is null, symbol %d\n",symbol);
    return -1;
  }

  qpsk_qpsk((int16_t *)rxF,
	    (int16_t *)rxF_i,
	    (int16_t *)llr128,
	    (int16_t *)rho,
	    frame_parms->N_RB_DL*12);

  //prepare for Viterbi which accepts 8 bit, but prefers 4 bit, soft input.
  for (i=0;i<(frame_parms->N_RB_DL*24);i++) {
    if (*pdcch_llr>7)
      *pdcch_llr8=7;
    else if (*pdcch_llr<-8)
      *pdcch_llr8=-8;
    else
      *pdcch_llr8 = (char)(*pdcch_llr);

    pdcch_llr++;
    pdcch_llr8++;
  }

  return(0);
}


int32_t pdcch_llr(LTE_DL_FRAME_PARMS *frame_parms,
		   int32_t **rxdataF_comp,
		   char *pdcch_llr,
		   uint8_t symbol) {

  int16_t *rxF= (int16_t*) &rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  int32_t i;
  char *pdcch_llr8;

  pdcch_llr8 = &pdcch_llr[2*symbol*frame_parms->N_RB_DL*12];
 
  if (!pdcch_llr8) {
    msg("pdcch_qpsk_llr: llr is null, symbol %d\n",symbol);
    return(-1);
  }
  //    msg("pdcch qpsk llr for symbol %d (pos %d), llr offset %d\n",symbol,(symbol*frame_parms->N_RB_DL*12),pdcch_llr8-pdcch_llr);

  for (i=0;i<(frame_parms->N_RB_DL*((symbol==0) ? 16 : 24));i++) {

    if (*rxF>31)
      *pdcch_llr8=31;
    else if (*rxF<-32)
      *pdcch_llr8=-32;
    else
      *pdcch_llr8 = (char)(*rxF);

    //    printf("%d %d => %d\n",i,*rxF,*pdcch_llr8);
    rxF++;
    pdcch_llr8++;
  }

  return(0);

}

__m128i avg128P;

//compute average channel_level on each (TX,RX) antenna pair
void pdcch_channel_level(int32_t **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 int32_t *avg,
			 uint8_t nb_rb) {

  int16_t rb;
  uint8_t aatx,aarx;
  __m128i *dl_ch128;
  

  for (aatx=0;aatx<frame_parms->nb_antennas_tx_eNB;aatx++)
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
      //clear average level
      avg128P = _mm_xor_si128(avg128P,avg128P);
      dl_ch128=(__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][frame_parms->N_RB_DL*12];

      for (rb=0;rb<nb_rb;rb++) {
    
	avg128P = _mm_add_epi32(avg128P,_mm_madd_epi16(dl_ch128[0],dl_ch128[0]));
	avg128P = _mm_add_epi32(avg128P,_mm_madd_epi16(dl_ch128[1],dl_ch128[1]));
	avg128P = _mm_add_epi32(avg128P,_mm_madd_epi16(dl_ch128[2],dl_ch128[2]));

	dl_ch128+=3;	
	/*
	  if (rb==0) {
	  print_shorts("dl_ch128",&dl_ch128[0]);
	  print_shorts("dl_ch128",&dl_ch128[1]);
	  print_shorts("dl_ch128",&dl_ch128[2]);
	  }
	*/
      }

      avg[(aatx<<1)+aarx] = (((int32_t*)&avg128P)[0] + 
			     ((int32_t*)&avg128P)[1] + 
			     ((int32_t*)&avg128P)[2] + 
			     ((int32_t*)&avg128P)[3])/(nb_rb*12);

      //            msg("Channel level : %d\n",avg[(aatx<<1)+aarx]);
    }
  _mm_empty();
  _m_empty();

}

__m128i mmtmpPD0,mmtmpPD1,mmtmpPD2,mmtmpPD3;

void pdcch_dual_stream_correlation(LTE_DL_FRAME_PARMS *frame_parms,
				   uint8_t symbol,
				   int32_t **dl_ch_estimates_ext,
				   int32_t **dl_ch_estimates_ext_i,
				   int32_t **dl_ch_rho_ext,
				   uint8_t output_shift) {

  uint16_t rb;
  __m128i *dl_ch128,*dl_ch128i,*dl_ch_rho128;
  uint8_t aarx;

  //  printf("dlsch_dual_stream_correlation: symbol %d\n",symbol);


  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

    dl_ch128          = (__m128i *)&dl_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    dl_ch128i         = (__m128i *)&dl_ch_estimates_ext_i[aarx][symbol*frame_parms->N_RB_DL*12];
    dl_ch_rho128      = (__m128i *)&dl_ch_rho_ext[aarx][symbol*frame_parms->N_RB_DL*12];


    for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
      // multiply by conjugated channel
      mmtmpPD0 = _mm_madd_epi16(dl_ch128[0],dl_ch128i[0]);
      //	print_ints("re",&mmtmpPD0);
      
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpPD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpPD1 = _mm_shufflehi_epi16(mmtmpPD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpPD1 = _mm_sign_epi16(mmtmpPD1,*(__m128i*)&conjugate[0]);
      //	print_ints("im",&mmtmpPD1);
      mmtmpPD1 = _mm_madd_epi16(mmtmpPD1,dl_ch128i[0]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpPD0 = _mm_srai_epi32(mmtmpPD0,output_shift);
      //	print_ints("re(shift)",&mmtmpPD0);
      mmtmpPD1 = _mm_srai_epi32(mmtmpPD1,output_shift);
      //	print_ints("im(shift)",&mmtmpPD1);
      mmtmpPD2 = _mm_unpacklo_epi32(mmtmpPD0,mmtmpPD1);
      mmtmpPD3 = _mm_unpackhi_epi32(mmtmpPD0,mmtmpPD1);
      //       	print_ints("c0",&mmtmpPD2);
      //	print_ints("c1",&mmtmpPD3);
      dl_ch_rho128[0] = _mm_packs_epi32(mmtmpPD2,mmtmpPD3);
      
      //print_shorts("rx:",dl_ch128_2);
      //print_shorts("ch:",dl_ch128);
      //print_shorts("pack:",rho128);
      
      // multiply by conjugated channel
      mmtmpPD0 = _mm_madd_epi16(dl_ch128[1],dl_ch128i[1]);
      // mmtmpPD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpPD1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpPD1 = _mm_shufflehi_epi16(mmtmpPD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpPD1 = _mm_sign_epi16(mmtmpPD1,*(__m128i*)conjugate);
      mmtmpPD1 = _mm_madd_epi16(mmtmpPD1,dl_ch128i[1]);
      // mmtmpPD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpPD0 = _mm_srai_epi32(mmtmpPD0,output_shift);
      mmtmpPD1 = _mm_srai_epi32(mmtmpPD1,output_shift);
      mmtmpPD2 = _mm_unpacklo_epi32(mmtmpPD0,mmtmpPD1);
      mmtmpPD3 = _mm_unpackhi_epi32(mmtmpPD0,mmtmpPD1);
      
      
      dl_ch_rho128[1] =_mm_packs_epi32(mmtmpPD2,mmtmpPD3);
      //print_shorts("rx:",dl_ch128_2+1);
      //print_shorts("ch:",dl_ch128+1);
      //print_shorts("pack:",rho128+1);	
      // multiply by conjugated channel
      mmtmpPD0 = _mm_madd_epi16(dl_ch128[2],dl_ch128i[2]);
      // mmtmpPD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpPD1 = _mm_shufflelo_epi16(dl_ch128[2],_MM_SHUFFLE(2,3,0,1));
      mmtmpPD1 = _mm_shufflehi_epi16(mmtmpPD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpPD1 = _mm_sign_epi16(mmtmpPD1,*(__m128i*)conjugate);
      mmtmpPD1 = _mm_madd_epi16(mmtmpPD1,dl_ch128i[2]);
      // mmtmpPD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpPD0 = _mm_srai_epi32(mmtmpPD0,output_shift);
      mmtmpPD1 = _mm_srai_epi32(mmtmpPD1,output_shift);
      mmtmpPD2 = _mm_unpacklo_epi32(mmtmpPD0,mmtmpPD1);
      mmtmpPD3 = _mm_unpackhi_epi32(mmtmpPD0,mmtmpPD1);
      
      dl_ch_rho128[2] = _mm_packs_epi32(mmtmpPD2,mmtmpPD3);
      //print_shorts("rx:",dl_ch128_2+2);
      //print_shorts("ch:",dl_ch128+2);
      //print_shorts("pack:",rho128+2);
      
      dl_ch128+=3;
      dl_ch128i+=3;
      dl_ch_rho128+=3;
      
    }	
    
  }
  
  _mm_empty();
  _m_empty();
  
  
}


void pdcch_detection_mrc_i(LTE_DL_FRAME_PARMS *frame_parms,
			 int32_t **rxdataF_comp,
			 int32_t **rxdataF_comp_i,
			 int32_t **rho,
			 int32_t **rho_i,
			 uint8_t symbol) {

  uint8_t aatx;

  __m128i *rxdataF_comp128_0,*rxdataF_comp128_1,*rxdataF_comp128_i0,*rxdataF_comp128_i1,*rho128_0,*rho128_1,*rho128_i0,*rho128_i1;
  int32_t i;

  if (frame_parms->nb_antennas_rx>1) {
    for (aatx=0;aatx<frame_parms->nb_antennas_tx_eNB;aatx++) {
      //if (frame_parms->mode1_flag && (aatx>0)) break;

      rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
      rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];

      // MRC on each re of rb on MF output
      for (i=0;i<frame_parms->N_RB_DL*3;i++) {
	rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
      }
    }
    rho128_0 = (__m128i *) &rho[0][symbol*frame_parms->N_RB_DL*12];
    rho128_1 = (__m128i *) &rho[1][symbol*frame_parms->N_RB_DL*12];
    for (i=0;i<frame_parms->N_RB_DL*3;i++) {
      rho128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rho128_0[i],1),_mm_srai_epi16(rho128_1[i],1));
    }
    rho128_i0 = (__m128i *) &rho_i[0][symbol*frame_parms->N_RB_DL*12];
    rho128_i1 = (__m128i *) &rho_i[1][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128_i0   = (__m128i *)&rxdataF_comp_i[0][symbol*frame_parms->N_RB_DL*12];  
    rxdataF_comp128_i1   = (__m128i *)&rxdataF_comp_i[1][symbol*frame_parms->N_RB_DL*12];
      // MRC on each re of rb on MF and rho
    for (i=0;i<frame_parms->N_RB_DL*3;i++) {
      rxdataF_comp128_i0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_i0[i],1),_mm_srai_epi16(rxdataF_comp128_i1[i],1));
      rho128_i0[i]          = _mm_adds_epi16(_mm_srai_epi16(rho128_i0[i],1),_mm_srai_epi16(rho128_i1[i],1));
    }
  }
  _mm_empty();
  _m_empty();

}


void pdcch_extract_rbs_single(int32_t **rxdataF,
			      int32_t **dl_ch_estimates,
			      int32_t **rxdataF_ext,
			      int32_t **dl_ch_estimates_ext,
			      uint8_t symbol,
			      LTE_DL_FRAME_PARMS *frame_parms) {


  uint16_t rb,nb_rb=0;
  uint8_t i,j,aarx;
  int32_t *dl_ch0,*dl_ch0_ext,*rxF,*rxF_ext;
  

  int nushiftmod3 = frame_parms->nushift%3;
  uint8_t symbol_mod;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
#ifdef DEBUG_DCI_DECODING
  msg("[PHY] extract_rbs_single: symbol_mod %d\n",symbol_mod);
#endif
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    dl_ch0     = &dl_ch_estimates[aarx][5+(symbol*(frame_parms->ofdm_symbol_size))];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];

    rxF_ext   = &rxdataF_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];

#ifndef NEW_FFT    
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];
#else
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))];
#endif    
    if ((frame_parms->N_RB_DL&1) == 0)  { // even number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	// For second half of RBs skip DC carrier
	if (rb==(frame_parms->N_RB_DL>>1)) {
#ifndef NEW_FFT
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
#else
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))];
#endif
	  //dl_ch0++; 
	}
	
	if (symbol_mod>0) {
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int32_t));
	  for (i=0;i<12;i++) {
#ifndef NEW_FFT
	    rxF_ext[i]=rxF[i<<1];
#else
	    rxF_ext[i]=rxF[i];
#endif
	  }
	  nb_rb++;
	  dl_ch0_ext+=12;
	  rxF_ext+=12;
	  
	  dl_ch0+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif
	}
	else {
	  j=0;
	  for (i=0;i<12;i++) {
	    if ((i!=nushiftmod3) &&
		(i!=(nushiftmod3+3)) &&
		(i!=(nushiftmod3+6)) &&
		(i!=(nushiftmod3+9))) {
#ifndef NEW_FFT
	      rxF_ext[j]=rxF[i<<1];
#else
	      rxF_ext[j]=rxF[i];
#endif
	      //	      	      	      printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
	      dl_ch0_ext[j++]=dl_ch0[i];
	      //	      	      printf("ch %d => (%d,%d)\n",i,*(short *)&dl_ch0[i],*(1+(short*)&dl_ch0[i]));
	    }
	  }
	  nb_rb++;
	  dl_ch0_ext+=8;
	  rxF_ext+=8;
	  
	  dl_ch0+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif
	}
      }
    }
    else {  // Odd number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL>>1;rb++) {

	if (symbol_mod>0) {
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int32_t));
	  for (i=0;i<12;i++)
#ifndef NEW_FFT
	    rxF_ext[i]=rxF[i<<1];
#else
	    rxF_ext[i]=rxF[i];
#endif
	  nb_rb++;
	  dl_ch0_ext+=12;
	  rxF_ext+=12;
	  
	  dl_ch0+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif
	}
	else {
	  j=0;
	  for (i=0;i<12;i++) {
	    if ((i!=nushiftmod3) &&
		(i!=(nushiftmod3+3)) &&
		(i!=(nushiftmod3+6)) &&
		(i!=(nushiftmod3+9))) {
#ifndef NEW_FFT
	      rxF_ext[j]=rxF[i<<1];
#else
	      rxF_ext[j]=rxF[i];
#endif
	      //	      	      	      printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
	      dl_ch0_ext[j++]=dl_ch0[i];
	      //	      	      printf("ch %d => (%d,%d)\n",i,*(short *)&dl_ch0[i],*(1+(short*)&dl_ch0[i]));
	    }
	  }
	  nb_rb++;
	  dl_ch0_ext+=8;
	  rxF_ext+=8;
	  
	  dl_ch0+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif
	}
      }
      // Do middle RB (around DC)
      //	msg("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);      

      if (symbol_mod==0) {
	j=0;
	for (i=0;i<6;i++) {
	  if ((i!=nushiftmod3) &&
	      (i!=(nushiftmod3+3))){
	    dl_ch0_ext[j]=dl_ch0[i];
#ifndef NEW_FFT
	    rxF_ext[j++]=rxF[i<<1];
#else
	    rxF_ext[j++]=rxF[i];
#endif
	    //	    	      printf("**extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	  }
	}
#ifndef NEW_FFT
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
#else
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))];
#endif
	for (;i<12;i++) {
	  if ((i!=(nushiftmod3+6)) &&
	      (i!=(nushiftmod3+9))){
	    dl_ch0_ext[j]=dl_ch0[i];
#ifndef NEW_FFT
	    rxF_ext[j++]=rxF[(1+i-6)<<1];
#else
	    rxF_ext[j++]=rxF[(1+i-6)];
#endif
	    //	    	      printf("**extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	  }
	}
      
	
	nb_rb++;
	dl_ch0_ext+=8;
	rxF_ext+=8;
	dl_ch0+=12;
#ifndef NEW_FFT
	rxF+=14;
#else
	rxF+=7;
#endif
	rb++;
      }
      else {
	for (i=0;i<6;i++) {
	  dl_ch0_ext[i]=dl_ch0[i];
#ifndef NEW_FFT
	  rxF_ext[i]=rxF[i<<1];
#else
	  rxF_ext[i]=rxF[i];
#endif
	}
#ifndef NEW_FFT
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
#else
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))];
#endif
	for (;i<12;i++) {
	  dl_ch0_ext[i]=dl_ch0[i];
#ifndef NEW_FFT
	  rxF_ext[i]=rxF[(1+i-6)<<1];
#else
	  rxF_ext[i]=rxF[(1+i-6)];
#endif
	}
      
	
	nb_rb++;
	dl_ch0_ext+=12;
	rxF_ext+=12;
	dl_ch0+=12;
#ifndef NEW_FFT
	rxF+=14;
#else
	rxF+=7;
#endif
	rb++;
      }

      for (;rb<frame_parms->N_RB_DL;rb++) {
	if (symbol_mod > 0) {
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int32_t));
	  for (i=0;i<12;i++)
#ifndef NEW_FFT
	    rxF_ext[i]=rxF[i<<1];
#else
	    rxF_ext[i]=rxF[i];
#endif
	  nb_rb++;
	  dl_ch0_ext+=12;
	  rxF_ext+=12;
	  
	  dl_ch0+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif
	}
	else {
	  j=0;
	  for (i=0;i<12;i++) {
	    if ((i!=(nushiftmod3)) &&
		(i!=(nushiftmod3+3)) &&
		(i!=(nushiftmod3+6)) &&
		(i!=(nushiftmod3+9))) {
#ifndef NEW_FFT
	      rxF_ext[j]=rxF[i<<1];
#else
	      rxF_ext[j]=rxF[i];
#endif
	      //	      	      printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
	      dl_ch0_ext[j++]=dl_ch0[i];
	    }
	  }
	  nb_rb++;
	  dl_ch0_ext+=8;
	  rxF_ext+=8;
	  
	  dl_ch0+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif
	}
      }
    }
  }

  _mm_empty();
  _m_empty();

}

void pdcch_extract_rbs_dual(int32_t **rxdataF,
			    int32_t **dl_ch_estimates,
			    int32_t **rxdataF_ext,
			    int32_t **dl_ch_estimates_ext,
			    uint8_t symbol,
			    LTE_DL_FRAME_PARMS *frame_parms) {
  

  uint16_t rb,nb_rb=0;
  uint8_t i,aarx,j;
  int32_t *dl_ch0,*dl_ch0_ext,*dl_ch1,*dl_ch1_ext,*rxF,*rxF_ext;
  uint8_t symbol_mod;
  int nushiftmod3 = frame_parms->nushift%3;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    dl_ch0     = &dl_ch_estimates[aarx][5+(symbol*(frame_parms->ofdm_symbol_size))];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];
    dl_ch1     = &dl_ch_estimates[2+aarx][5+(symbol*(frame_parms->ofdm_symbol_size))];
    dl_ch1_ext = &dl_ch_estimates_ext[2+aarx][symbol*(frame_parms->N_RB_DL*12)];

    //    msg("pdcch extract_rbs: rxF_ext pos %d\n",symbol*(frame_parms->N_RB_DL*12));
    rxF_ext   = &rxdataF_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];

#ifndef NEW_FFT    
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];
#else
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))];
#endif    
    if ((frame_parms->N_RB_DL&1) == 0)  // even number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	// For second half of RBs skip DC carrier
	if (rb==(frame_parms->N_RB_DL>>1)) {
#ifndef NEW_FFT
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
#else
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))];
#endif
	  //dl_ch0++;
	  //dl_ch1++;
	}
	
	if (symbol_mod>0) {
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int32_t));
	  memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int32_t));
	  /*
	    msg("rb %d\n",rb);
	    for (i=0;i<12;i++)
	    msg("(%d %d)",((int16_t *)dl_ch)[i<<1],((int16_t*)dl_ch)[1+(i<<1)]);
	    msg("\n");*/
	  
	  for (i=0;i<12;i++) {
#ifndef NEW_FFT
	    rxF_ext[i]=rxF[i<<1];
#else
	    rxF_ext[i]=rxF[i];
#endif
	    //	  	      msg("%d : (%d,%d)\n",(rxF+(2*i)-&rxdataF[aarx][( (symbol*(frame_parms->ofdm_symbol_size)))*2])/2,
	    //   ((int16_t*)&rxF[i<<1])[0],((int16_t*)&rxF[i<<1])[0]);
	  }
	  nb_rb++;
	  dl_ch0_ext+=12;
	  dl_ch1_ext+=12;
	  rxF_ext+=12;
	}
	else {
	  j=0;
	  for (i=0;i<12;i++) {
	    if ((i!=nushiftmod3) &&
		(i!=nushiftmod3+3) &&
		(i!=nushiftmod3+6) &&
		(i!=nushiftmod3+9)) {
#ifndef NEW_FFT
	      rxF_ext[j]=rxF[i<<1];
#else
	      rxF_ext[j]=rxF[i];
#endif
	      //	      	      	      printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
	      dl_ch0_ext[j++]=dl_ch0[i];
	      dl_ch1_ext[j++]=dl_ch1[i];
	    }
	  }
	  nb_rb++;
	  dl_ch0_ext+=8;
	  rxF_ext+=8;
	  
	  dl_ch0+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif

	}
	dl_ch0+=12;
	dl_ch1+=12;
#ifndef NEW_FFT
	rxF+=24;
#else
	rxF+=12;
#endif
      }
  
    else {  // Odd number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL>>1;rb++) {

	//	msg("rb %d: %d\n",rb,rxF-&rxdataF[aarx][(symbol*(frame_parms->ofdm_symbol_size))*2]);

	if (symbol_mod>0) {
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int32_t));
	  memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int32_t));
	  for (i=0;i<12;i++)
#ifndef NEW_FFT
	    rxF_ext[i]=rxF[i<<1];
#else
	    rxF_ext[i]=rxF[i];
#endif
	  nb_rb++;
	  dl_ch0_ext+=12;
	  dl_ch1_ext+=12;
	  rxF_ext+=12;
	  
	  dl_ch0+=12;
	  dl_ch1+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif
	  
	}
	else {
	  j=0;
	  for (i=0;i<12;i++) {
	    if ((i!=nushiftmod3) &&
		(i!=nushiftmod3+3) &&
		(i!=nushiftmod3+6) &&
		(i!=nushiftmod3+9)) {
#ifndef NEW_FFT
	      rxF_ext[j]=rxF[i<<1];
#else
	      rxF_ext[j]=rxF[i];
#endif
	      //	      	      	      printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
	      dl_ch0_ext[j]=dl_ch0[i];
	      dl_ch1_ext[j++]=dl_ch1[i];
	      //	      	      printf("ch %d => (%d,%d)\n",i,*(short *)&dl_ch0[i],*(1+(short*)&dl_ch0[i]));
	    }
	  }
	  nb_rb++;
	  dl_ch0_ext+=8;
	  dl_ch1_ext+=8;
	  rxF_ext+=8;
	  

	  dl_ch0+=12;
	  dl_ch1+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif
	}
      }      
	// Do middle RB (around DC)

      if (symbol_mod > 0) {
	for (i=0;i<6;i++) {
	  dl_ch0_ext[i]=dl_ch0[i];
	  dl_ch1_ext[i]=dl_ch1[i];
#ifndef NEW_FFT
	  rxF_ext[i]=rxF[i<<1];
#else
	  rxF_ext[i]=rxF[i];
#endif
	}
#ifndef NEW_FFT	
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
#else
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))];
#endif
	for (;i<12;i++) {
	  dl_ch0_ext[i]=dl_ch0[i];
	  dl_ch1_ext[i]=dl_ch1[i];
#ifndef NEW_FFT
	  rxF_ext[i]=rxF[(1+i)<<1];
#else
	  rxF_ext[i]=rxF[(1+i)];
#endif
	}

	nb_rb++;
	dl_ch0_ext+=12;
	dl_ch1_ext+=12;
	rxF_ext+=12;
	
	dl_ch0+=12;
	dl_ch1+=12;
#ifndef NEW_FFT
	rxF+=14;
#else
	rxF+=7;
#endif
	rb++;
      }
      else {
	j=0;
	for (i=0;i<6;i++) {
	  if ((i!=nushiftmod3) &&
	      (i!=nushiftmod3+3)){
	    dl_ch0_ext[j]=dl_ch0[i];
	    dl_ch1_ext[j]=dl_ch1[i];
#ifndef NEW_FFT
	    rxF_ext[j++]=rxF[i<<1];
#else
	    rxF_ext[j++]=rxF[i];
#endif
	    //	    	      printf("**extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	  }
	}
#ifndef NEW_FFT
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
#else
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))];
#endif
	for (;i<12;i++) {
	  if ((i!=nushiftmod3+6) &&
	      (i!=nushiftmod3+9)){
	    dl_ch0_ext[j]=dl_ch0[i];
	    dl_ch1_ext[j]=dl_ch1[i];
#ifndef NEW_FFT
	    rxF_ext[j++]=rxF[(1+i-6)<<1];
#else
	    rxF_ext[j++]=rxF[(1+i-6)];
#endif
	    //	    	      printf("**extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	  }
	}
      
	
	nb_rb++;
	dl_ch0_ext+=8;
	dl_ch1_ext+=8;
	rxF_ext+=8;
	dl_ch0+=12;
	dl_ch1+=12;
#ifndef NEW_FFT
	rxF+=14;
#else
	rxF+=7;
#endif
	rb++;
      }

      for (;rb<frame_parms->N_RB_DL;rb++) {

	if (symbol_mod>0) {
	  //	msg("rb %d: %d\n",rb,rxF-&rxdataF[aarx][(symbol*(frame_parms->ofdm_symbol_size))*2]);
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int32_t));
	  memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int32_t));
	  for (i=0;i<12;i++)
#ifndef NEW_FFT
	    rxF_ext[i]=rxF[i<<1];
#else
	    rxF_ext[i]=rxF[i];
#endif
	  nb_rb++;
	  dl_ch0_ext+=12;
	  dl_ch1_ext+=12;
	  rxF_ext+=12;
	  
	  dl_ch0+=12;
	  dl_ch1+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif	  
	  
	}
	else {
	  j=0;
	  for (i=0;i<12;i++) {
	    if ((i!=nushiftmod3) &&
		(i!=nushiftmod3+3) &&
		(i!=nushiftmod3+6) &&
		(i!=nushiftmod3+9)) {
#ifndef NEW_FFT
	      rxF_ext[j]=rxF[i<<1];
#else
	      rxF_ext[j]=rxF[i];
#endif
	      //	      	      printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
	      dl_ch0_ext[j]=dl_ch0[i];
	      dl_ch1_ext[j++]=dl_ch1[i];
	    }
	  }
	  nb_rb++;
	  dl_ch0_ext+=8;
	  dl_ch1_ext+=8;
	  rxF_ext+=8;
	  
	  dl_ch0+=12;
	  dl_ch1+=12;
#ifndef NEW_FFT
	  rxF+=24;
#else
	  rxF+=12;
#endif
	}
      }
    }
  }
  _mm_empty();
  _m_empty();
  

}


void pdcch_channel_compensation(int32_t **rxdataF_ext,
				int32_t **dl_ch_estimates_ext,
				int32_t **rxdataF_comp,
				int32_t **rho,
				LTE_DL_FRAME_PARMS *frame_parms,
				uint8_t symbol,
				uint8_t output_shift) {

  uint16_t rb;
  __m128i *dl_ch128,*rxdataF128,*rxdataF_comp128;
  __m128i *dl_ch128_2, *rho128;
  uint8_t aatx,aarx,pilots=0;



#ifndef __SSE3__
  zero2 = _mm_xor_si128(zero2,zero2);
#endif

#ifdef DEBUG_DCI_DECODING
  msg("[PHY] PDCCH comp: symbol %d\n",symbol);
#endif

  if (symbol==0)
    pilots=1;

  for (aatx=0;aatx<frame_parms->nb_antennas_tx_eNB;aatx++) {
    //if (frame_parms->mode1_flag && aatx>0) break; //if mode1_flag is set then there is only one stream to extract, independent of nb_antennas_tx_eNB

    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

      dl_ch128          = (__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];
      rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
      rxdataF_comp128   = (__m128i *)&rxdataF_comp[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];


      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	// multiply by conjugated channel
	mmtmpPD0 = _mm_madd_epi16(dl_ch128[0],rxdataF128[0]);
	//	print_ints("re",&mmtmpPD0);
	
	// mmtmpPD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpPD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
	mmtmpPD1 = _mm_shufflehi_epi16(mmtmpPD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpPD1 = _mm_sign_epi16(mmtmpPD1,*(__m128i*)&conjugate[0]);
	//	print_ints("im",&mmtmpPD1);
	mmtmpPD1 = _mm_madd_epi16(mmtmpPD1,rxdataF128[0]);
	// mmtmpPD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpPD0 = _mm_srai_epi32(mmtmpPD0,output_shift);
	//	print_ints("re(shift)",&mmtmpPD0);
	mmtmpPD1 = _mm_srai_epi32(mmtmpPD1,output_shift);
	//	print_ints("im(shift)",&mmtmpPD1);
	mmtmpPD2 = _mm_unpacklo_epi32(mmtmpPD0,mmtmpPD1);
	mmtmpPD3 = _mm_unpackhi_epi32(mmtmpPD0,mmtmpPD1);
	//       	print_ints("c0",&mmtmpPD2);
	//	print_ints("c1",&mmtmpPD3);
	rxdataF_comp128[0] = _mm_packs_epi32(mmtmpPD2,mmtmpPD3);
	//	print_shorts("rx:",rxdataF128);
	//	print_shorts("ch:",dl_ch128);
	//	print_shorts("pack:",rxdataF_comp128);

	// multiply by conjugated channel
	mmtmpPD0 = _mm_madd_epi16(dl_ch128[1],rxdataF128[1]);
	// mmtmpPD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpPD1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
	mmtmpPD1 = _mm_shufflehi_epi16(mmtmpPD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpPD1 = _mm_sign_epi16(mmtmpPD1,*(__m128i*)conjugate);
	mmtmpPD1 = _mm_madd_epi16(mmtmpPD1,rxdataF128[1]);
	// mmtmpPD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpPD0 = _mm_srai_epi32(mmtmpPD0,output_shift);
	mmtmpPD1 = _mm_srai_epi32(mmtmpPD1,output_shift);
	mmtmpPD2 = _mm_unpacklo_epi32(mmtmpPD0,mmtmpPD1);
	mmtmpPD3 = _mm_unpackhi_epi32(mmtmpPD0,mmtmpPD1);
	
	rxdataF_comp128[1] = _mm_packs_epi32(mmtmpPD2,mmtmpPD3);
	//	print_shorts("rx:",rxdataF128+1);
	//	print_shorts("ch:",dl_ch128+1);
	//	print_shorts("pack:",rxdataF_comp128+1);	
	// multiply by conjugated channel
	if (pilots == 0) {
	  mmtmpPD0 = _mm_madd_epi16(dl_ch128[2],rxdataF128[2]);
	  // mmtmpPD0 contains real part of 4 consecutive outputs (32-bit)
	  mmtmpPD1 = _mm_shufflelo_epi16(dl_ch128[2],_MM_SHUFFLE(2,3,0,1));
	  mmtmpPD1 = _mm_shufflehi_epi16(mmtmpPD1,_MM_SHUFFLE(2,3,0,1));
	  mmtmpPD1 = _mm_sign_epi16(mmtmpPD1,*(__m128i*)conjugate);
	  mmtmpPD1 = _mm_madd_epi16(mmtmpPD1,rxdataF128[2]);
	  // mmtmpPD1 contains imag part of 4 consecutive outputs (32-bit)
	  mmtmpPD0 = _mm_srai_epi32(mmtmpPD0,output_shift);
	  mmtmpPD1 = _mm_srai_epi32(mmtmpPD1,output_shift);
	  mmtmpPD2 = _mm_unpacklo_epi32(mmtmpPD0,mmtmpPD1);
	  mmtmpPD3 = _mm_unpackhi_epi32(mmtmpPD0,mmtmpPD1);
	  
	  rxdataF_comp128[2] = _mm_packs_epi32(mmtmpPD2,mmtmpPD3);
	}
	//	print_shorts("rx:",rxdataF128+2);
	//	print_shorts("ch:",dl_ch128+2);
	//      	print_shorts("pack:",rxdataF_comp128+2);
      
	if (pilots==0) {
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
  }
  

  if (rho) {

    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
      rho128        = (__m128i *)&rho[aarx][symbol*frame_parms->N_RB_DL*12];
      dl_ch128      = (__m128i *)&dl_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];
      dl_ch128_2    = (__m128i *)&dl_ch_estimates_ext[2+aarx][symbol*frame_parms->N_RB_DL*12];

      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	// multiply by conjugated channel
	mmtmpPD0 = _mm_madd_epi16(dl_ch128[0],dl_ch128_2[0]);
	//	print_ints("re",&mmtmpD0);
	
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpPD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
	mmtmpPD1 = _mm_shufflehi_epi16(mmtmpPD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpPD1 = _mm_sign_epi16(mmtmpPD1,*(__m128i*)&conjugate[0]);
	//	print_ints("im",&mmtmpPD1);
	mmtmpPD1 = _mm_madd_epi16(mmtmpPD1,dl_ch128_2[0]);
	// mmtmpPD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpPD0 = _mm_srai_epi32(mmtmpPD0,output_shift);
	//	print_ints("re(shift)",&mmtmpD0);
	mmtmpPD1 = _mm_srai_epi32(mmtmpPD1,output_shift);
	//	print_ints("im(shift)",&mmtmpD1);
	mmtmpPD2 = _mm_unpacklo_epi32(mmtmpPD0,mmtmpPD1);
	mmtmpPD3 = _mm_unpackhi_epi32(mmtmpPD0,mmtmpPD1);
	//       	print_ints("c0",&mmtmpPD2);
	//	print_ints("c1",&mmtmpPD3);
	rho128[0] = _mm_packs_epi32(mmtmpPD2,mmtmpPD3);

	//print_shorts("rx:",dl_ch128_2);
	//print_shorts("ch:",dl_ch128);
	//print_shorts("pack:",rho128);
	
	// multiply by conjugated channel
	mmtmpPD0 = _mm_madd_epi16(dl_ch128[1],dl_ch128_2[1]);
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpPD1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
	mmtmpPD1 = _mm_shufflehi_epi16(mmtmpPD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpPD1 = _mm_sign_epi16(mmtmpPD1,*(__m128i*)conjugate);
	mmtmpPD1 = _mm_madd_epi16(mmtmpPD1,dl_ch128_2[1]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpPD0 = _mm_srai_epi32(mmtmpPD0,output_shift);
	mmtmpPD1 = _mm_srai_epi32(mmtmpPD1,output_shift);
	mmtmpPD2 = _mm_unpacklo_epi32(mmtmpPD0,mmtmpPD1);
	mmtmpPD3 = _mm_unpackhi_epi32(mmtmpPD0,mmtmpPD1);


	rho128[1] =_mm_packs_epi32(mmtmpPD2,mmtmpPD3);
	//print_shorts("rx:",dl_ch128_2+1);
	//print_shorts("ch:",dl_ch128+1);
	//print_shorts("pack:",rho128+1);	
	// multiply by conjugated channel
	mmtmpPD0 = _mm_madd_epi16(dl_ch128[2],dl_ch128_2[2]);
	// mmtmpPD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpPD1 = _mm_shufflelo_epi16(dl_ch128[2],_MM_SHUFFLE(2,3,0,1));
	mmtmpPD1 = _mm_shufflehi_epi16(mmtmpPD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpPD1 = _mm_sign_epi16(mmtmpPD1,*(__m128i*)conjugate);
	mmtmpPD1 = _mm_madd_epi16(mmtmpPD1,dl_ch128_2[2]);
	// mmtmpPD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpPD0 = _mm_srai_epi32(mmtmpPD0,output_shift);
	mmtmpPD1 = _mm_srai_epi32(mmtmpPD1,output_shift);
	mmtmpPD2 = _mm_unpacklo_epi32(mmtmpPD0,mmtmpPD1);
	mmtmpPD3 = _mm_unpackhi_epi32(mmtmpPD0,mmtmpPD1);
	
	rho128[2] = _mm_packs_epi32(mmtmpPD2,mmtmpPD3);
	//print_shorts("rx:",dl_ch128_2+2);
	//print_shorts("ch:",dl_ch128+2);
	//print_shorts("pack:",rho128+2);
	
	dl_ch128+=3;
	dl_ch128_2+=3;
	rho128+=3;
	
      }	
    }

  }

  _mm_empty();
  _m_empty();

}     

void pdcch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			 int32_t **rxdataF_comp,
			 uint8_t symbol) {

  uint8_t aatx;

  __m128i *rxdataF_comp128_0,*rxdataF_comp128_1;
  int32_t i;

  if (frame_parms->nb_antennas_rx>1) {
    for (aatx=0;aatx<frame_parms->nb_antennas_tx_eNB;aatx++) {
      rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
      rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];  
      // MRC on each re of rb
      for (i=0;i<frame_parms->N_RB_DL*3;i++) {
	rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
      }
    }
  }
  _mm_empty();
  _m_empty();

}

void pdcch_siso(LTE_DL_FRAME_PARMS *frame_parms,
		int32_t **rxdataF_comp,
		uint8_t l) {


  uint8_t rb,re,jj,ii;

  jj=0;
  ii=0;
  for (rb=0;rb<frame_parms->N_RB_DL;rb++) {

    for (re=0;re<12;re++) {
      
      rxdataF_comp[0][jj++] = rxdataF_comp[0][ii];
      ii++;
    }
  }
}


void pdcch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		    int32_t **rxdataF_comp,
		    uint8_t symbol){


  int16_t *rxF0,*rxF1;
  uint8_t rb,re;
  int32_t jj=(symbol*frame_parms->N_RB_DL*12);

  rxF0     = (int16_t*)&rxdataF_comp[0][jj];  //tx antenna 0  h0*y
  rxF1     = (int16_t*)&rxdataF_comp[2][jj];  //tx antenna 1  h1*y

  for (rb=0;rb<frame_parms->N_RB_DL;rb++) {

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

int32_t avgP[4];

int32_t rx_pdcch(LTE_UE_COMMON *lte_ue_common_vars,
		 LTE_UE_PDCCH **lte_ue_pdcch_vars,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 uint8_t subframe,
		 uint8_t eNB_id,
		 MIMO_mode_t mimo_mode,
		 uint8_t is_secondary_ue) {

  uint8_t log2_maxh,aatx,aarx;
#ifdef MU_RECEIVER
  uint8_t eNB_id_i=eNB_id+1;//add 1 to eNB_id to separate from wanted signal, chosen as the B/F'd pilots from the SeNB are shifted by 1
#endif
  int32_t avgs,s;
  uint8_t n_pdcch_symbols = 3; //lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols;
  uint8_t mi = get_mi(frame_parms,subframe);

  //  printf("In rx_pdcch, subframe %d,  eNB_id %d\n",subframe,eNB_id);

  for (s=0;s<n_pdcch_symbols;s++) {
      if (is_secondary_ue == 1) {
	pdcch_extract_rbs_single(lte_ue_common_vars->rxdataF,
				 lte_ue_common_vars->dl_ch_estimates[eNB_id+1], //add 1 to eNB_id to compensate for the shifted B/F'd pilots from the SeNB
				 lte_ue_pdcch_vars[eNB_id]->rxdataF_ext,
				 lte_ue_pdcch_vars[eNB_id]->dl_ch_estimates_ext,
				 s,
				 frame_parms);
#ifdef MU_RECEIVER
	pdcch_extract_rbs_single(lte_ue_common_vars->rxdataF,
				 lte_ue_common_vars->dl_ch_estimates[eNB_id_i - 1],//subtract 1 to eNB_id_i to compensate for the non-shifted pilots from the PeNB
				 lte_ue_pdcch_vars[eNB_id_i]->rxdataF_ext,//shift by two to simulate transmission from a second antenna
				 lte_ue_pdcch_vars[eNB_id_i]->dl_ch_estimates_ext,//shift by two to simulate transmission from a second antenna
				 s,
				 frame_parms);
#endif //MU_RECEIVER
      } else if (frame_parms->nb_antennas_tx_eNB>1) {
	pdcch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
			       lte_ue_common_vars->dl_ch_estimates[eNB_id],
			       lte_ue_pdcch_vars[eNB_id]->rxdataF_ext,
			       lte_ue_pdcch_vars[eNB_id]->dl_ch_estimates_ext,
			       s,
			       frame_parms);
      } else {
	pdcch_extract_rbs_single(lte_ue_common_vars->rxdataF,
				 lte_ue_common_vars->dl_ch_estimates[eNB_id],
				 lte_ue_pdcch_vars[eNB_id]->rxdataF_ext,
				 lte_ue_pdcch_vars[eNB_id]->dl_ch_estimates_ext,
				 s,
				 frame_parms);
      }
  }
  pdcch_channel_level(lte_ue_pdcch_vars[eNB_id]->dl_ch_estimates_ext,
		      frame_parms,
		      avgP,
		      frame_parms->N_RB_DL);

  avgs = 0;
  for (aatx=0;aatx<frame_parms->nb_antennas_tx_eNB;aatx++)
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
      avgs = cmax(avgs,avgP[(aarx<<1)+aatx]);
  
  log2_maxh = (log2_approx(avgs)/2) + 2 + frame_parms->nb_antennas_rx - 1;
#ifdef DEBUG_PHY
  msg("[PDCCH] log2_maxh = %d (%d,%d)\n",log2_maxh,avgP[0],avgs);
#endif


  for (s=0;s<n_pdcch_symbols;s++) {
    pdcch_channel_compensation(lte_ue_pdcch_vars[eNB_id]->rxdataF_ext,
			       lte_ue_pdcch_vars[eNB_id]->dl_ch_estimates_ext,
			       lte_ue_pdcch_vars[eNB_id]->rxdataF_comp,
			       (aatx>1) ? lte_ue_pdcch_vars[eNB_id]->rho : NULL,
			       frame_parms,
			       s,
			       log2_maxh); // log2_maxh+I0_shift


#ifdef DEBUG_PHY
    if (subframe==5)
      write_output("rxF_comp_d.m","rxF_c_d",&lte_ue_pdcch_vars[eNB_id]->rxdataF_comp[0][s*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);
#endif

#ifdef MU_RECEIVER
    if (is_secondary_ue) {
      //get MF output for interfering stream
      pdcch_channel_compensation(lte_ue_pdcch_vars[eNB_id_i]->rxdataF_ext,
				 lte_ue_pdcch_vars[eNB_id_i]->dl_ch_estimates_ext,
				 lte_ue_pdcch_vars[eNB_id_i]->rxdataF_comp,
				 (aatx>1) ? lte_ue_pdcch_vars[eNB_id_i]->rho : NULL,
				 frame_parms,
				 s,
				 log2_maxh); // log2_maxh+I0_shift
#ifdef DEBUG_PHY
	write_output("rxF_comp_i.m","rxF_c_i",&lte_ue_pdcch_vars[eNB_id_i]->rxdataF_comp[0][s*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);
#endif
      pdcch_dual_stream_correlation(frame_parms,
				    s,
				    lte_ue_pdcch_vars[eNB_id]->dl_ch_estimates_ext,
				    lte_ue_pdcch_vars[eNB_id_i]->dl_ch_estimates_ext,
				    lte_ue_pdcch_vars[eNB_id]->dl_ch_rho_ext,
				    log2_maxh);
    }
#endif //MU_RECEIVER
    

    if (frame_parms->nb_antennas_rx > 1) {
#ifdef MU_RECEIVER
      if (is_secondary_ue) {
	pdcch_detection_mrc_i(frame_parms,
			      lte_ue_pdcch_vars[eNB_id]->rxdataF_comp,
			      lte_ue_pdcch_vars[eNB_id_i]->rxdataF_comp,
			      lte_ue_pdcch_vars[eNB_id]->rho,
			      lte_ue_pdcch_vars[eNB_id]->dl_ch_rho_ext,
			      s);
#ifdef DEBUG_PHY
	write_output("rxF_comp_d.m","rxF_c_d",&lte_ue_pdcch_vars[eNB_id]->rxdataF_comp[0][s*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);
	write_output("rxF_comp_i.m","rxF_c_i",&lte_ue_pdcch_vars[eNB_id_i]->rxdataF_comp[0][s*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);
#endif
      } else 
#endif //MU_RECEIVER
	pdcch_detection_mrc(frame_parms,
			    lte_ue_pdcch_vars[eNB_id]->rxdataF_comp,
			    s);
      
    }
  
    if (mimo_mode == SISO) 
      pdcch_siso(frame_parms,lte_ue_pdcch_vars[eNB_id]->rxdataF_comp,s);
    else
      pdcch_alamouti(frame_parms,lte_ue_pdcch_vars[eNB_id]->rxdataF_comp,s);

    
#ifdef MU_RECEIVER
    if (is_secondary_ue) {
      pdcch_qpsk_qpsk_llr(frame_parms,
			  lte_ue_pdcch_vars[eNB_id]->rxdataF_comp,
			  lte_ue_pdcch_vars[eNB_id_i]->rxdataF_comp,
			  lte_ue_pdcch_vars[eNB_id]->dl_ch_rho_ext,
			  lte_ue_pdcch_vars[eNB_id]->llr16, //subsequent function require 16 bit llr, but output must be 8 bit (actually clipped to 4, because of the Viterbi decoder)
			  lte_ue_pdcch_vars[eNB_id]->llr,
			  s);
      /*
#ifdef DEBUG_PHY
      if (subframe==5) {
	write_output("llr8_seq.m","llr8",&lte_ue_pdcch_vars[eNB_id]->llr[s*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,4);
	write_output("llr16_seq.m","llr16",&lte_ue_pdcch_vars[eNB_id]->llr16[s*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,4);
      }
      #endif*/
    } 
    else {
#endif //MU_RECEIVER
      pdcch_llr(frame_parms,
		lte_ue_pdcch_vars[eNB_id]->rxdataF_comp,
		(char *)lte_ue_pdcch_vars[eNB_id]->llr,
		s);
      /*#ifdef DEBUG_PHY
      write_output("llr8_seq.m","llr8",&lte_ue_pdcch_vars[eNB_id]->llr[s*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,4);
      #endif*/
#ifdef MU_RECEIVER
    }
#endif //MU_RECEIVER

  }

  // decode pcfich here
  n_pdcch_symbols = rx_pcfich(frame_parms,
			      subframe,
			      lte_ue_pdcch_vars[eNB_id],
			      mimo_mode);
  if (n_pdcch_symbols>3)
    n_pdcch_symbols=1;

#ifdef DEBUG_DCI_DECODING
  msg("[PDCCH] subframe %d n_pdcch_symbols from PCFICH =%d\n",subframe,n_pdcch_symbols);

  msg("demapping: subframe %d, mi %d, tdd_config %d\n",subframe,get_mi(frame_parms,subframe),frame_parms->tdd_config);
#endif

  pdcch_demapping(lte_ue_pdcch_vars[eNB_id]->llr,
		  lte_ue_pdcch_vars[eNB_id]->wbar,
		  frame_parms,
		  n_pdcch_symbols,
		  get_mi(frame_parms,subframe));

  pdcch_deinterleaving(frame_parms,
		       (uint16_t*)lte_ue_pdcch_vars[eNB_id]->e_rx,
		       lte_ue_pdcch_vars[eNB_id]->wbar,
		       n_pdcch_symbols,
		       mi);
  
  pdcch_unscrambling(frame_parms,
		     subframe,
		     lte_ue_pdcch_vars[eNB_id]->e_rx,
		     get_nCCE(n_pdcch_symbols,frame_parms,mi)*72);
  
  lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols = n_pdcch_symbols;

  return(0);
}


void pdcch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		      uint8_t subframe,
		      uint8_t *e,
		      uint32_t length) {
  int i;
  uint8_t reset;
  uint32_t x1, x2, s=0;

  reset = 1;
  // x1 is set in lte_gold_generic

  x2 = (subframe<<9) + frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.8.2

  for (i=0; i<length; i++) {
    if ((i&0x1f)==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }
    //    printf("scrambling %d : e %d, c %d\n",i,e[i],((s>>(i&0x1f))&1));
    if (e[i] != 2) // <NIL> element is 2
      e[i] = (e[i]&1) ^ ((s>>(i&0x1f))&1);
  }
}

void pdcch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
			uint8_t subframe,
			int8_t* llr,
			uint32_t length) {

  int i;
  uint8_t reset;
  uint32_t x1, x2, s=0;

  reset = 1;
  // x1 is set in first call to lte_gold_generic

  x2 = (subframe<<9) + frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.8.2

  for (i=0; i<length; i++) {
    if (i%32==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }
    // take the quarter of the PBCH that corresponds to this frame
    //    printf("unscrambling %d : e %d, c %d\n",i,llr[i],((s>>(i&0x1f))&1));
    if (((s>>(i%32))&1)==0)
      llr[i] = -llr[i];

  }
}
	     

uint8_t get_num_pdcch_symbols(uint8_t num_dci,
			 DCI_ALLOC_t *dci_alloc,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 uint8_t subframe) {

  uint16_t numCCE = 0;
  uint8_t i;
  uint8_t nCCEmin = 0;

  // check pdcch duration imposed by PHICH duration (Section 6.9 of 36-211)
  if (frame_parms->Ncp==1) { // extended prefix
    if ((frame_parms->frame_type == TDD) && 
	((frame_parms->tdd_config<3)||(frame_parms->tdd_config==6)) &&
	((subframe==1) || (subframe==6))) // subframes 1 and 6 (S-subframes) for 5ms switching periodicity are 2 symbols
      nCCEmin = 2;
    else {   // 10ms switching periodicity is always 3 symbols, any DL-only subframe is 3 symbols
      nCCEmin = 3;
    }
  }

  // compute numCCE
  for (i=0;i<num_dci;i++) {
    //     printf("dci %d => %d\n",i,dci_alloc[i].L);
    numCCE += (1<<(dci_alloc[i].L));
  }

  //if ((9*numCCE) <= (frame_parms->N_RB_DL*2))
  if (numCCE <= get_nCCE(1, frame_parms, get_mi(frame_parms, subframe)))
    return(cmax(1,nCCEmin));
  //else if ((9*numCCE) <= (frame_parms->N_RB_DL*((frame_parms->nb_antennas_tx_eNB==4) ? 4 : 5)))
  else if (numCCE < get_nCCE(2, frame_parms, get_mi(frame_parms, subframe)))
    return(cmax(2,nCCEmin));
  //else if ((9*numCCE) <= (frame_parms->N_RB_DL*((frame_parms->nb_antennas_tx_eNB==4) ? 7 : 8)))
  else if (numCCE < get_nCCE(3, frame_parms, get_mi(frame_parms, subframe)))
    return(cmax(3,nCCEmin));
  else if (frame_parms->N_RB_DL<=10) { 
    if (frame_parms->Ncp == 0) { // normal CP
      if ((9*numCCE) <= (frame_parms->N_RB_DL*((frame_parms->nb_antennas_tx_eNB==4) ? 10 : 11)))
	return(4);
    }
    else { // extended CP
      if ((9*numCCE) <= (frame_parms->N_RB_DL*((frame_parms->nb_antennas_tx_eNB==4) ? 9 : 10)))
	return(4);
    }
  }

  
  msg("[PHY] dci.c: get_num_pdcch_symbols subframe %d FATAL, illegal numCCE %d (num_dci %d)\n",subframe,numCCE,num_dci);
  //for (i=0;i<num_dci;i++) {
  //  printf("dci_alloc[%d].L = %d\n",i,dci_alloc[i].L);
  //}  
  //exit(-1);
  return(0);
}

uint8_t generate_dci_top(uint8_t num_ue_spec_dci,
		    uint8_t num_common_dci,
		    DCI_ALLOC_t *dci_alloc, 
		    uint32_t n_rnti,
		    int16_t amp,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    mod_sym_t **txdataF,
		    uint32_t subframe) {

  uint8_t *e_ptr,num_pdcch_symbols;
  int8_t L;
  uint32_t i, lprime;
  uint32_t gain_lin_QPSK,kprime,kprime_mod12,mprime,nsymb,symbol_offset,tti_offset;
  int16_t re_offset;
  uint8_t mi = get_mi(frame_parms,subframe);
  static uint8_t e[DCI_BITS_MAX];	
  static mod_sym_t yseq0[Msymb],yseq1[Msymb],wbar0[Msymb],wbar1[Msymb];
     
  mod_sym_t *y[2];
  mod_sym_t *wbar[2];
  
  int nushiftmod3 = frame_parms->nushift%3;

  int Msymb2;
  int split_flag=0;

  switch (frame_parms->N_RB_DL) {
  case 100:
    Msymb2 = Msymb;
    break;
  case 75:
    Msymb2 = 3*Msymb/4;
    break;
  case 50:
    Msymb2 = Msymb>>1;
    break;
  case 25:
    Msymb2 = Msymb>>2;
    break;
  case 15:
    Msymb2 = Msymb*15/100;
    break;
  case 6:
    Msymb2 = Msymb*6/100;
    break;
  default:
    Msymb2 = Msymb>>2;
    break;
  }

  num_pdcch_symbols = get_num_pdcch_symbols(num_ue_spec_dci+num_common_dci,dci_alloc,frame_parms,subframe);
  //   printf("subframe %d in generate_dci_top num_pdcch_symbols = %d, num_dci %d\n",
  //    	 subframe,num_pdcch_symbols,num_ue_spec_dci+num_common_dci);
  generate_pcfich(num_pdcch_symbols,
		  amp,
		  frame_parms,
		  txdataF,
		  subframe);

  wbar[0] = &wbar0[0];
  wbar[1] = &wbar1[0];
  y[0] = &yseq0[0];
  y[1] = &yseq1[0];

  // reset all bits to <NIL>, here we set <NIL> elements as 2
  //memset(e, 2, DCI_BITS_MAX);
  // here we interpred NIL as a random QPSK sequence. That makes power estimation easier.
  for (i=0;i<DCI_BITS_MAX;i++) 
    e[i]=taus()&1;
  e_ptr = e;

  // generate DCIs in order of decreasing aggregation level, then common/ue spec
  // MAC is assumed to have ordered the UE spec DCI according to the RNTI-based randomization
  for (L=3;L>=0;L--) {
    for (i=0;i<num_common_dci;i++) {

      if (dci_alloc[i].L == (uint8_t)L) {
	
#ifdef DEBUG_DCI_ENCODING
	LOG_I(PHY,"Generating common DCI %d/%d (nCCE %d) of length %d, aggregation %d (%x)\n",i,num_common_dci,dci_alloc[i].nCCE,dci_alloc[i].dci_length,1<<dci_alloc[i].L,*(unsigned int*)dci_alloc[i].dci_pdu);
	dump_dci(frame_parms,&dci_alloc[i]);
#endif

	e_ptr = generate_dci0(dci_alloc[i].dci_pdu,
			      e+(72*dci_alloc[i].nCCE),
			      dci_alloc[i].dci_length,
			      dci_alloc[i].L,
			      dci_alloc[i].rnti);    
      }
    }
    for (;i<num_ue_spec_dci + num_common_dci;i++) {

      if (dci_alloc[i].L == (uint8_t)L) {
      
#ifdef DEBUG_DCI_ENCODING
	LOG_I(PHY," Generating UE (rnti %x) specific DCI %d of length %d, aggregation %d, format %d (%x)\n",dci_alloc[i].rnti,i,dci_alloc[i].dci_length,1<<dci_alloc[i].L,dci_alloc[i].format,dci_alloc[i].dci_pdu);
	dump_dci(frame_parms,&dci_alloc[i]);
#endif

	e_ptr = generate_dci0(dci_alloc[i].dci_pdu,
			      e+(72*dci_alloc[i].nCCE),
			      dci_alloc[i].dci_length,
			      dci_alloc[i].L,
			      dci_alloc[i].rnti);        
      }
    }
  }

  // Scrambling
  //  printf("pdcch scrambling\n");
  pdcch_scrambling(frame_parms,
		   subframe,
		   e,
                   8*get_nquad(num_pdcch_symbols, frame_parms, mi));
		   //72*get_nCCE(num_pdcch_symbols,frame_parms,mi));
  

#ifdef DEBUG_DCI_ENCODING
  LOG_I(PHY," PDCCH Modulation, Msymb %d\n",Msymb);
#endif
  // Now do modulation
  if (frame_parms->mode1_flag==1) 
    gain_lin_QPSK = (int16_t)((amp*ONE_OVER_SQRT2_Q15)>>15);  
  else
    gain_lin_QPSK = amp/2;  

  e_ptr = e;
  if (frame_parms->mode1_flag) { //SISO


    for (i=0;i<Msymb2;i++) {
      //((int16_t*)(&(y[0][i])))[0] = (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;
      //((int16_t*)(&(y[1][i])))[0] = (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;
      ((int16_t*)(&(y[0][i])))[0] = (*e_ptr == 2) ? 0 : (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;
      ((int16_t*)(&(y[1][i])))[0] = (*e_ptr == 2) ? 0 : (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;

      e_ptr++;
      //((int16_t*)(&(y[0][i])))[1] = (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;
      //((int16_t*)(&(y[1][i])))[1] = (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;
      ((int16_t*)(&(y[0][i])))[1] = (*e_ptr == 2) ? 0 : (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;
      ((int16_t*)(&(y[1][i])))[1] = (*e_ptr == 2) ? 0 : (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;

      e_ptr++;
    }
  }
  else { //ALAMOUTI    


      for (i=0;i<Msymb2;i+=2) {

#ifdef DEBUG_DCI_ENCODING
	LOG_I(PHY," PDCCH Modulation (TX diversity): REG %d\n",i>>2);
#endif
	// first antenna position n -> x0
	((int16_t*)&y[0][i])[0] = (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;
	e_ptr++;
	((int16_t*)&y[0][i])[1] = (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;
	e_ptr++;

	// second antenna position n -> -x1*
	((int16_t*)&y[1][i])[0] = (*e_ptr == 1) ? gain_lin_QPSK : -gain_lin_QPSK;
	e_ptr++;
	((int16_t*)&y[1][i])[1] = (*e_ptr == 1) ? -gain_lin_QPSK : gain_lin_QPSK;
	e_ptr++;

	// fill in the rest of the ALAMOUTI precoding
	((int16_t*)&y[0][i+1])[0] = -((int16_t*)&y[1][i])[0];
	((int16_t*)&y[0][i+1])[1] = ((int16_t*)&y[1][i])[1];
	((int16_t*)&y[1][i+1])[0] = ((int16_t*)&y[0][i])[0];
	((int16_t*)&y[1][i+1])[1] = -((int16_t*)&y[0][i])[1];

      }
  }


#ifdef DEBUG_DCI_ENCODING
  LOG_I(PHY," PDCCH Interleaving\n");
#endif

  //  msg("y %p (%p,%p), wbar %p (%p,%p)\n",y,y[0],y[1],wbar,wbar[0],wbar[1]);
  // This is the interleaving procedure defined in 36-211, first part of Section 6.8.5
  pdcch_interleaving(frame_parms,&y[0],&wbar[0],num_pdcch_symbols,mi);

  mprime=0;
  nsymb = (frame_parms->Ncp==0) ? 14:12;
  re_offset = frame_parms->first_carrier_offset;

  // This is the REG allocation algorithm from 36-211, second part of Section 6.8.5
  //  printf("DCI (SF %d) : txdataF %p (0 %p)\n",subframe,&txdataF[0][512*14*subframe],&txdataF[0][0]);
  for (kprime=0;kprime<frame_parms->N_RB_DL*12;kprime++) {
    for (lprime=0;lprime<num_pdcch_symbols;lprime++) {

      symbol_offset = (uint32_t)frame_parms->ofdm_symbol_size*(lprime+(subframe*nsymb));


	  
      tti_offset = symbol_offset + re_offset;

      (re_offset==(frame_parms->ofdm_symbol_size-2)) ? (split_flag=1) : (split_flag=0);

      //            printf("kprime %d, lprime %d => REG %d (symbol %d)\n",kprime,lprime,(lprime==0)?(kprime/6) : (kprime>>2),symbol_offset);
      // if REG is allocated to PHICH, skip it
      if (check_phich_reg(frame_parms,kprime,lprime,mi) == 1) {
#ifdef DEBUG_DCI_ENCODING
	msg("generate_dci: skipping REG %d (kprime %d, lprime %d)\n",(lprime==0)?(kprime/6) : (kprime>>2),kprime,lprime);
#endif
      }
      else {
	// Copy REG to TX buffer      
	
	if ((lprime == 0)||
	    ((lprime==1)&&(frame_parms->nb_antennas_tx_eNB == 4))) {  
	  // first symbol, or second symbol+4 TX antennas skip pilots

	  kprime_mod12 = kprime%12;
	  if ((kprime_mod12 == 0) || (kprime_mod12 == 6)) {
	    // kprime represents REG	    

	    for (i=0;i<6;i++) {
	      if ((i!=(nushiftmod3))&&(i!=(nushiftmod3+3))) {
		txdataF[0][tti_offset+i] = wbar[0][mprime];
		if (frame_parms->nb_antennas_tx_eNB > 1)
		  txdataF[1][tti_offset+i] = wbar[1][mprime];
#ifdef DEBUG_DCI_ENCODING
		LOG_I(PHY," PDCCH mapping mprime %d => %d (symbol %d re %d) -> (%d,%d)\n",mprime,tti_offset,symbol_offset,re_offset+i,*(short*)&wbar[0][mprime],*(1+(short*)&wbar[0][mprime]));
#endif
		mprime++;
	      }
	    }
	  }
	}
	else { // no pilots in this symbol
	  kprime_mod12 = kprime%12;
	  if ((kprime_mod12 == 0) || (kprime_mod12 == 4) || (kprime_mod12 == 8)) {
	    // kprime represents REG
	    if (split_flag==0) {
	      for (i=0;i<4;i++) {
		txdataF[0][tti_offset+i] = wbar[0][mprime];
		if (frame_parms->nb_antennas_tx_eNB > 1)
		  txdataF[1][tti_offset+i] = wbar[1][mprime];
#ifdef DEBUG_DCI_ENCODING
		LOG_I(PHY," PDCCH mapping mprime %d => %d (symbol %d re %d) -> (%d,%d)\n",mprime,tti_offset,symbol_offset,re_offset+i,*(short*)&wbar[0][mprime],*(1+(short*)&wbar[0][mprime]));
#endif
		mprime++;
	      }
	    }
	    else {
		txdataF[0][tti_offset+0] = wbar[0][mprime];
		if (frame_parms->nb_antennas_tx_eNB > 1)
		  txdataF[1][tti_offset+0] = wbar[1][mprime];
#ifdef DEBUG_DCI_ENCODING
		LOG_I(PHY," PDCCH mapping mprime %d => %d (symbol %d re %d) -> (%d,%d)\n",mprime,tti_offset,symbol_offset,re_offset,*(short*)&wbar[0][mprime],*(1+(short*)&wbar[0][mprime]));
#endif
		mprime++;
		txdataF[0][tti_offset+1] = wbar[0][mprime];
		if (frame_parms->nb_antennas_tx_eNB > 1)
		  txdataF[1][tti_offset+1] = wbar[1][mprime];
#ifdef DEBUG_DCI_ENCODING
		LOG_I(PHY," PDCCH mapping mprime %d => %d (symbol %d re %d) -> (%d,%d)\n",mprime,tti_offset,symbol_offset,re_offset+1,*(short*)&wbar[0][mprime],*(1+(short*)&wbar[0][mprime]));
#endif
		mprime++;
		txdataF[0][tti_offset-frame_parms->ofdm_symbol_size+3] = wbar[0][mprime];
		if (frame_parms->nb_antennas_tx_eNB > 1)
		  txdataF[1][tti_offset-frame_parms->ofdm_symbol_size+3] = wbar[1][mprime];
#ifdef DEBUG_DCI_ENCODING
		LOG_I(PHY," PDCCH mapping mprime %d => %d (symbol %d re %d) -> (%d,%d)\n",mprime,tti_offset,symbol_offset,re_offset-frame_parms->ofdm_symbol_size+3,*(short*)&wbar[0][mprime],*(1+(short*)&wbar[0][mprime]));
#endif
		mprime++;
		txdataF[0][tti_offset-frame_parms->ofdm_symbol_size+4] = wbar[0][mprime];
		if (frame_parms->nb_antennas_tx_eNB > 1)
		  txdataF[1][tti_offset-frame_parms->ofdm_symbol_size+4] = wbar[1][mprime];
#ifdef DEBUG_DCI_ENCODING
		LOG_I(PHY," PDCCH mapping mprime %d => %d (symbol %d re %d) -> (%d,%d)\n",mprime,tti_offset,symbol_offset,re_offset-frame_parms->ofdm_symbol_size+4,*(short*)&wbar[0][mprime],*(1+(short*)&wbar[0][mprime]));
#endif
		mprime++;

	    }
	  }
	}
	if (mprime>=Msymb2)
	  return(num_pdcch_symbols);	
      } // check_phich_reg
      
    } //lprime loop
    
    re_offset++;
    if (re_offset == (frame_parms->ofdm_symbol_size))
      re_offset = 1;
  } // kprime loop
  return(num_pdcch_symbols);
}

#ifdef PHY_ABSTRACTION
uint8_t generate_dci_top_emul(PHY_VARS_eNB *phy_vars_eNB,
			 uint8_t num_ue_spec_dci,
			 uint8_t num_common_dci,
			 DCI_ALLOC_t *dci_alloc,
			 uint8_t subframe){ 
  int n_dci, n_dci_dl;
  uint8_t ue_id;
  LTE_eNB_DLSCH_t *dlsch_eNB;
  uint8_t num_pdcch_symbols = get_num_pdcch_symbols(num_ue_spec_dci+num_common_dci,
					       dci_alloc,
					       &phy_vars_eNB->lte_frame_parms,
					       subframe);
  eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].cntl.cfi=num_pdcch_symbols;

  memcpy(phy_vars_eNB->dci_alloc[subframe&1],dci_alloc,sizeof(DCI_ALLOC_t)*(num_ue_spec_dci+num_common_dci));
  phy_vars_eNB->num_ue_spec_dci[subframe&1]=num_ue_spec_dci;
  phy_vars_eNB->num_common_dci[subframe&1]=num_common_dci;
  eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].num_ue_spec_dci = num_ue_spec_dci;
  eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].num_common_dci = num_common_dci;

  LOG_D(PHY,"[eNB %d][DCI][EMUL] CC id %d:  num spec dci %d num comm dci %d num PMCH %d \n", 
	phy_vars_eNB->Mod_id, phy_vars_eNB->CC_id, num_ue_spec_dci,num_common_dci,
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].num_pmch);

  if (eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].cntl.pmch_flag == 1 )
    n_dci_dl = eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].num_pmch;
  else 
    n_dci_dl = 0;

  for (n_dci =0 ; 
       n_dci < (eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].num_ue_spec_dci+ eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].num_common_dci);
       n_dci++) {
    
    if (dci_alloc[n_dci].format > 0){ // exclude the uplink dci 

      if (dci_alloc[n_dci].rnti == SI_RNTI) { 
	dlsch_eNB = PHY_vars_eNB_g[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id]->dlsch_eNB_SI;
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].dlsch_type[n_dci_dl] = 0;//SI;
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].harq_pid[n_dci_dl] = 0;
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].tbs[n_dci_dl] = dlsch_eNB->harq_processes[0]->TBS>>3;
    	LOG_D(PHY,"[DCI][EMUL]SI tbs is %d and dci index %d harq pid is %d \n",eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].tbs[n_dci_dl],n_dci_dl, eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].harq_pid[n_dci_dl]);
      }
      else if (dci_alloc[n_dci_dl].ra_flag == 1) {
	dlsch_eNB = PHY_vars_eNB_g[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id]->dlsch_eNB_ra;
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].dlsch_type[n_dci_dl] = 1;//RA;
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].harq_pid[n_dci_dl] = 0;
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].tbs[n_dci_dl] = dlsch_eNB->harq_processes[0]->TBS>>3;
     	LOG_D(PHY,"[DCI][EMUL] RA  tbs is %d and dci index %d harq pid is %d \n",eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].tbs[n_dci_dl],n_dci_dl, eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].harq_pid[n_dci_dl]);
      }
      else {
	ue_id = find_ue(dci_alloc[n_dci_dl].rnti,PHY_vars_eNB_g[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id]);
	dlsch_eNB = PHY_vars_eNB_g[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id]->dlsch_eNB[ue_id][0];
	
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].dlsch_type[n_dci_dl] = 2;//TB0;
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].harq_pid[n_dci_dl] = dlsch_eNB->current_harq_pid;
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].ue_id[n_dci_dl] = ue_id;
	eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].tbs[n_dci_dl] = dlsch_eNB->harq_processes[dlsch_eNB->current_harq_pid]->TBS>>3;
	LOG_D(PHY,"[DCI][EMUL] TB1 tbs is %d and dci index %d harq pid is %d \n",eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].tbs[n_dci_dl],n_dci_dl, eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].harq_pid[n_dci_dl]);
	// check for TB1 later
	
      }
    }
    n_dci_dl++;
  }
  memcpy((void *)&eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].dci_alloc,
	 (void *)dci_alloc,
	 n_dci*sizeof(DCI_ALLOC_t));

  return(num_pdcch_symbols);
}
#endif 

 
void dci_decoding(uint8_t DCI_LENGTH,
		  uint8_t aggregation_level,
		  int8_t *e,
		  uint8_t *decoded_output) {

  uint8_t dummy_w_rx[3*(MAX_DCI_SIZE_BITS+16+64)];
  int8_t w_rx[3*(MAX_DCI_SIZE_BITS+16+32)],d_rx[96+(3*(MAX_DCI_SIZE_BITS+16))];

  uint16_t RCC;

  uint16_t D=(DCI_LENGTH+16+64);
  uint16_t coded_bits;
#ifdef DEBUG_DCI_DECODING
  int32_t i;
#endif
  if (aggregation_level>3) {
    LOG_I(PHY," dci.c: dci_decoding FATAL, illegal aggregation_level %d\n",aggregation_level);
    return;
  }

  coded_bits = 72 * (1<<aggregation_level);

#ifdef DEBUG_DCI_DECODING
  LOG_I(PHY," Doing DCI decoding for %d bits, DCI_LENGTH %d,coded_bits %d, e %p\n",3*(DCI_LENGTH+16),DCI_LENGTH,coded_bits,e);
#endif
  
  // now do decoding
  memset((void*)dummy_w_rx,0,3*D);
  RCC = generate_dummy_w_cc(DCI_LENGTH+16,
			    dummy_w_rx);


   
#ifdef DEBUG_DCI_DECODING
  LOG_I(PHY," Doing DCI Rate Matching RCC %d, w %p\n",RCC,w);
#endif

  lte_rate_matching_cc_rx(RCC,coded_bits,w_rx,dummy_w_rx,e);
 
  sub_block_deinterleaving_cc((uint32_t)(DCI_LENGTH+16), 
			      &d_rx[96], 
			      &w_rx[0]); 
 
#ifdef DEBUG_DCI_DECODING
  for (i=0;i<16+DCI_LENGTH;i++)
    LOG_I(PHY," DCI %d : (%d,%d,%d)\n",i,*(d_rx+96+(3*i)),*(d_rx+97+(3*i)),*(d_rx+98+(3*i)));
#endif  
  memset(decoded_output,0,2+((16+DCI_LENGTH)>>3));
  
#ifdef DEBUG_DCI_DECODING
  msg("Before Viterbi\n");

  for (i=0;i<16+DCI_LENGTH;i++)
    msg("%d : (%d,%d,%d)\n",i,*(d_rx+96+(3*i)),*(d_rx+97+(3*i)),*(d_rx+98+(3*i)));
#endif  
  //debug_msg("Doing DCI Viterbi \n");
  phy_viterbi_lte_sse2(d_rx+96,decoded_output,16+DCI_LENGTH);
  //debug_msg("Done DCI Viterbi \n");
}


static uint8_t dci_decoded_output[(MAX_DCI_SIZE_BITS+64)/8];

uint16_t get_nCCE(uint8_t num_pdcch_symbols,LTE_DL_FRAME_PARMS *frame_parms,uint8_t mi) {
  return(get_nquad(num_pdcch_symbols,frame_parms,mi)/9);
}

uint16_t get_nquad(uint8_t num_pdcch_symbols,LTE_DL_FRAME_PARMS *frame_parms,uint8_t mi) {

  uint16_t Nreg=0;
  uint8_t Ngroup_PHICH = (frame_parms->phich_config_common.phich_resource*frame_parms->N_RB_DL)/48;								   

  if (((frame_parms->phich_config_common.phich_resource*frame_parms->N_RB_DL)%48) > 0)
    Ngroup_PHICH++;

  if (frame_parms->Ncp == 1) {
    Ngroup_PHICH<<=1;
  }
  Ngroup_PHICH*=mi;

  if ((num_pdcch_symbols>0) && (num_pdcch_symbols<4))
    switch (frame_parms->N_RB_DL) {
    case 6:
      Nreg=12+(num_pdcch_symbols-1)*18;
      break;
    case 25:
      Nreg=50+(num_pdcch_symbols-1)*75;
      break;
    case 50:
      Nreg=100+(num_pdcch_symbols-1)*150;
      break;
    case 100:
      Nreg=200+(num_pdcch_symbols-1)*300;
      break;
    default:
      return(0);
    }
  //   printf("Nreg %d (%d)\n",Nreg,Nreg - 4 - (3*Ngroup_PHICH));
  return(Nreg - 4 - (3*Ngroup_PHICH));
}

 uint16_t get_nCCE_max(uint8_t Mod_id,uint8_t CC_id) {

  // check for eNB only !
  return(get_nCCE(3,&PHY_vars_eNB_g[Mod_id][CC_id]->lte_frame_parms,1)); // 5, 15,21
}

void dci_decoding_procedure0(LTE_UE_PDCCH **lte_ue_pdcch_vars,int do_common,uint8_t subframe,
			     DCI_ALLOC_t *dci_alloc,
			     int16_t eNB_id,
			     LTE_DL_FRAME_PARMS *frame_parms,
			     uint8_t mi,
			     uint16_t si_rnti,
			     uint16_t ra_rnti,
			     uint8_t L,
			     uint8_t format_si,
			     uint8_t format_ra,
			     uint8_t format_c,
			     uint8_t sizeof_bits,
			     uint8_t sizeof_bytes,
			     uint8_t *dci_cnt,
			     uint8_t *format0_found,
			     uint8_t *format_c_found,
			     uint32_t *CCEmap0,
			     uint32_t *CCEmap1,
			     uint32_t *CCEmap2) {
  
  uint16_t crc,CCEind,nCCE;
  uint32_t *CCEmap=NULL,CCEmap_mask=0;
  int L2=(1<<L);
  unsigned int Yk,nb_candidates = 0,i,m;
  unsigned int CCEmap_cand;

  nCCE = get_nCCE(lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,frame_parms,mi);
  if (nCCE > get_nCCE(3,frame_parms,1))
      return;
  if (nCCE<L2)
    return;

  if (do_common == 1) {
    nb_candidates = (L2==4) ? 4 : 2;
    Yk=0;
  }
  else {
    // Find first available in ue specific search space
    // according to procedure in Section 9.1.1 of 36.213 (v. 8.6)
    // compute Yk
    Yk = (unsigned int)lte_ue_pdcch_vars[eNB_id]->crnti;

    for (i=0;i<=subframe;i++)
      Yk = (Yk*39827)%65537;

    Yk = Yk % (nCCE/L2);

    switch (L2) {
    case 1:
    case 2:
      nb_candidates = 6;
      break;
    case 4:
    case 8:
      nb_candidates = 2;
      break;
    default:
      DevParam(L2, do_common, eNB_id);
      break;
    }    
  }
  /*  for (CCEind=0;
       CCEind<nCCE2;
       CCEind+=(1<<L)) {*/

  if (nb_candidates*L2 > nCCE)
    nb_candidates = nCCE/L2;

  for (m=0;m<nb_candidates;m++) {

    CCEind = (((Yk+m)%(nCCE/L2))*L2);

    if (CCEind<32)
      CCEmap = CCEmap0;
    else if (CCEind<64)
      CCEmap = CCEmap1;
    else if (CCEind<96)
      CCEmap = CCEmap2;
    else {
      LOG_E(PHY,"Illegal CCEind %d (Yk %d, m %d, nCCE %d, L2 %d\n",CCEind,Yk,m,nCCE,L2);
      mac_xface->macphy_exit("Illegal CCEind\n");
    }
    switch (L2) {
    case 1:
      CCEmap_mask = (1<<(CCEind&0x1f));
      break;
    case 2:
      CCEmap_mask = (3<<(CCEind&0x1f));
      break;
    case 4:
      CCEmap_mask = (0xf<<(CCEind&0x1f));
      break;
    case 8:
      CCEmap_mask = (0xff<<(CCEind&0x1f));
      break;
    }	
    CCEmap_cand = (*CCEmap)&CCEmap_mask;
    
      // CCE is not allocated yet

    if (CCEmap_cand == 0) {
#ifdef DEBUG_DCI_DECODING
      if (do_common == 1) 
	LOG_I(PHY,"[DCI search - common] Attempting candidate %d Aggregation Level %d DCI length %d at CCE %d/%d (CCEmap %x,CCEmap_cand %x)\n",m,L2,sizeof_bits,CCEind,nCCE,*CCEmap,CCEmap_mask);
      else
	LOG_I(PHY,"[DCI search - ue spec] Attempting candidate %d Aggregation Level %d DCI length %d at CCE %d/%d (CCEmap %x,CCEmap_cand %x)\n",m,L2,sizeof_bits,CCEind,nCCE,*CCEmap,CCEmap_mask);

#endif 

	dci_decoding(sizeof_bits,
		     L,
		     &lte_ue_pdcch_vars[eNB_id]->e_rx[CCEind*72],
		     dci_decoded_output);
	/*
	  for (i=0;i<3+(sizeof_bits>>3);i++)
	  printf("dci_decoded_output[%d] => %x\n",i,dci_decoded_output[i]);
	*/
	crc = (crc16(dci_decoded_output,sizeof_bits)>>16) ^ extract_crc(dci_decoded_output,sizeof_bits); 
#ifdef DEBUG_DCI_DECODING
	msg("crc =>%x\n",crc);
#endif
	if (((L>1) && ((crc == si_rnti)||
		       (crc == ra_rnti)))||
	    (crc == lte_ue_pdcch_vars[eNB_id]->crnti))   {
	  dci_alloc[*dci_cnt].dci_length = sizeof_bits;
	  dci_alloc[*dci_cnt].rnti       = crc;
	  dci_alloc[*dci_cnt].L          = L;
          dci_alloc[*dci_cnt].nCCE       = CCEind;
	  
	  if (sizeof_bytes<=4) {
	    dci_alloc[*dci_cnt].dci_pdu[3] = dci_decoded_output[0];
	    dci_alloc[*dci_cnt].dci_pdu[2] = dci_decoded_output[1];
	    dci_alloc[*dci_cnt].dci_pdu[1] = dci_decoded_output[2];
	    dci_alloc[*dci_cnt].dci_pdu[0] = dci_decoded_output[3];
#ifdef DEBUG_DCI_DECODING
	    msg("DCI => %x,%x,%x,%x\n",dci_decoded_output[0],dci_decoded_output[1],dci_decoded_output[2],dci_decoded_output[3]);
#endif
	  }
	  else {
	    dci_alloc[*dci_cnt].dci_pdu[7] = dci_decoded_output[0];
	    dci_alloc[*dci_cnt].dci_pdu[6] = dci_decoded_output[1];
	    dci_alloc[*dci_cnt].dci_pdu[5] = dci_decoded_output[2];
	    dci_alloc[*dci_cnt].dci_pdu[4] = dci_decoded_output[3];
	    dci_alloc[*dci_cnt].dci_pdu[3] = dci_decoded_output[4];
	    dci_alloc[*dci_cnt].dci_pdu[2] = dci_decoded_output[5];
	    dci_alloc[*dci_cnt].dci_pdu[1] = dci_decoded_output[6];
	    dci_alloc[*dci_cnt].dci_pdu[0] = dci_decoded_output[7];
	  }
	  
	  if (crc==si_rnti) {
	    dci_alloc[*dci_cnt].format     = format_si;
	    *dci_cnt = *dci_cnt+1;
	  }
	  else if (crc==ra_rnti) {
	    dci_alloc[*dci_cnt].format     = format_ra;
	    // store first nCCE of group for PUCCH transmission of ACK/NAK
	    lte_ue_pdcch_vars[eNB_id]->nCCE[subframe]=CCEind;
	    *dci_cnt = *dci_cnt+1;
	  }
	  else if (crc==lte_ue_pdcch_vars[eNB_id]->crnti) {
	    
	    if ((format_c == format0)&&((dci_decoded_output[0]&0x80)==0)) {// check if pdu is format 0 or 1A
	      if (*format0_found == 0) {
		dci_alloc[*dci_cnt].format     = format0;
		*format0_found = 1;
		*dci_cnt = *dci_cnt+1;
		lte_ue_pdcch_vars[eNB_id]->nCCE[subframe]=CCEind;
	      }
	    }
	    else if (format_c == format0) { // this is a format 1A DCI
	      dci_alloc[*dci_cnt].format     = format1A;
	      *dci_cnt = *dci_cnt+1;
	      lte_ue_pdcch_vars[eNB_id]->nCCE[subframe]=CCEind;
	    }
	    else {
	      // store first nCCE of group for PUCCH transmission of ACK/NAK
	      if (*format_c_found == 0) {
		dci_alloc[*dci_cnt].format     = format_c;
		*dci_cnt = *dci_cnt+1;
		*format_c_found = 1;
		lte_ue_pdcch_vars[eNB_id]->nCCE[subframe]=CCEind;
	      }
	    }	    
	  }
	  
	  //	memcpy(&dci_alloc[*dci_cnt].dci_pdu[0],dci_decoded_output,sizeof_bytes);
	  
	  
	  
	  switch (1<<L) {
	  case 1:
	    *CCEmap|=(1<<(CCEind&0x1f));
	    break;
	  case 2:
	    *CCEmap|=(0x03<<(CCEind&0x1f));
	    break;
	  case 4:
	    *CCEmap|=(0x0f<<(CCEind&0x1f));
	    break;
	  case 8:
	    *CCEmap|=(0xff<<(CCEind&0x1f));
	    break;
	  }	
#ifdef DEBUG_DCI_DECODING
	  LOG_I(PHY,"[DCI search] Found DCI %d rnti %x Aggregation %d length %d format %s in CCE %d (CCEmap %x)\n",
	      *dci_cnt,crc,1<<L,sizeof_bits,dci_format_strings[dci_alloc[*dci_cnt-1].format],CCEind,*CCEmap);
	  dump_dci(frame_parms,&dci_alloc[*dci_cnt-1]);
	  
#endif
	
	  //	if (crc==lte_ue_pdcch_vars[eNB_id]->crnti)
	  //	  return;
	} // rnti match
      }  // CCEmap_cand == 0
  } // candidate loop
}

uint16_t dci_decoding_procedure(PHY_VARS_UE *phy_vars_ue,
			   DCI_ALLOC_t *dci_alloc,
			   int do_common,
			   int16_t eNB_id,
			   uint8_t subframe) {
 
  uint8_t  dci_cnt=0,old_dci_cnt=0;
  uint32_t CCEmap0=0,CCEmap1=0,CCEmap2=0;
  LTE_UE_PDCCH **lte_ue_pdcch_vars = phy_vars_ue->lte_ue_pdcch_vars;
  LTE_DL_FRAME_PARMS *frame_parms  = &phy_vars_ue->lte_frame_parms;
  uint8_t mi = get_mi(&phy_vars_ue->lte_frame_parms,subframe);
  uint16_t ra_rnti=99;
  uint8_t format0_found=0,format_c_found=0;
  uint8_t tmode = phy_vars_ue->transmission_mode[eNB_id];
  uint8_t frame_type = frame_parms->frame_type;
  uint8_t format1A_size_bits,format1A_size_bytes;
  uint8_t format0_size_bits,format0_size_bytes;
  uint8_t format1_size_bits,format1_size_bytes;
  uint8_t format2_size_bits,format2_size_bytes;

  switch (frame_parms->N_RB_DL) {
  case 6:
    if (frame_type == TDD) {
      format1A_size_bits  = sizeof_DCI1A_1_5MHz_TDD_1_6_t;
      format1A_size_bytes = sizeof(DCI1A_1_5MHz_TDD_1_6_t);
      format0_size_bits  = sizeof_DCI0_1_5MHz_TDD_1_6_t;
      format0_size_bytes = sizeof(DCI0_1_5MHz_TDD_1_6_t);
      format1_size_bits  = sizeof_DCI1_1_5MHz_TDD_t;
      format1_size_bytes = sizeof(DCI1_1_5MHz_TDD_t);
      if (frame_parms->nb_antennas_tx == 2) {
	format2_size_bits  = sizeof_DCI2_1_5MHz_2A_TDD_t;
	format2_size_bytes = sizeof(DCI2_1_5MHz_2A_TDD_t);
      }
      else if (frame_parms->nb_antennas_tx == 4) {
	format2_size_bits  = sizeof_DCI2_1_5MHz_4A_TDD_t;
	format2_size_bytes = sizeof(DCI2_1_5MHz_4A_TDD_t);
      }
    }
    else {
      format1A_size_bits  = sizeof_DCI1A_1_5MHz_FDD_t;
      format1A_size_bytes = sizeof(DCI1A_1_5MHz_FDD_t);
      format0_size_bits  = sizeof_DCI0_1_5MHz_FDD_t;
      format0_size_bytes = sizeof(DCI0_1_5MHz_FDD_t);
      format1_size_bits  = sizeof_DCI1_1_5MHz_FDD_t;
      format1_size_bytes = sizeof(DCI1_1_5MHz_FDD_t);
      if (frame_parms->nb_antennas_tx == 2) {
	format2_size_bits  = sizeof_DCI2_1_5MHz_2A_FDD_t;
	format2_size_bytes = sizeof(DCI2_1_5MHz_2A_FDD_t);
      }
      else if (frame_parms->nb_antennas_tx == 4) {
	format2_size_bits  = sizeof_DCI2_1_5MHz_4A_FDD_t;
	format2_size_bytes = sizeof(DCI2_1_5MHz_4A_FDD_t);
      }
    }
    break;
  case 25:
  default:
    if (frame_type == TDD) {
      format1A_size_bits  = sizeof_DCI1A_5MHz_TDD_1_6_t;
      format1A_size_bytes = sizeof(DCI1A_5MHz_TDD_1_6_t);
      format0_size_bits  = sizeof_DCI0_5MHz_TDD_1_6_t;
      format0_size_bytes = sizeof(DCI0_5MHz_TDD_1_6_t);
      format1_size_bits  = sizeof_DCI1_5MHz_TDD_t;
      format1_size_bytes = sizeof(DCI1_5MHz_TDD_t);
      if (frame_parms->nb_antennas_tx == 2) {
	format2_size_bits  = sizeof_DCI2_5MHz_2A_TDD_t;
	format2_size_bytes = sizeof(DCI2_5MHz_2A_TDD_t);
      }
      else if (frame_parms->nb_antennas_tx == 4) {
	format2_size_bits  = sizeof_DCI2_5MHz_4A_TDD_t;
	format2_size_bytes = sizeof(DCI2_5MHz_4A_TDD_t);
      }
    }
    else {
      format1A_size_bits  = sizeof_DCI1A_5MHz_FDD_t;
      format1A_size_bytes = sizeof(DCI1A_5MHz_FDD_t);
      format0_size_bits  = sizeof_DCI0_5MHz_FDD_t;
      format0_size_bytes = sizeof(DCI0_5MHz_FDD_t);
      format1_size_bits  = sizeof_DCI1_5MHz_FDD_t;
      format1_size_bytes = sizeof(DCI1_5MHz_FDD_t);
      if (frame_parms->nb_antennas_tx == 2) {
	format2_size_bits  = sizeof_DCI2_5MHz_2A_FDD_t;
	format2_size_bytes = sizeof(DCI2_5MHz_2A_FDD_t);
      }
      else if (frame_parms->nb_antennas_tx == 4) {
	format2_size_bits  = sizeof_DCI2_5MHz_4A_FDD_t;
	format2_size_bytes = sizeof(DCI2_5MHz_4A_FDD_t);
      }
    }
    break;
  case 50:
    if (frame_type == TDD) {
      format1A_size_bits  = sizeof_DCI1A_10MHz_TDD_1_6_t;
      format1A_size_bytes = sizeof(DCI1A_10MHz_TDD_1_6_t);
      format0_size_bits  = sizeof_DCI0_10MHz_TDD_1_6_t;
      format0_size_bytes = sizeof(DCI0_10MHz_TDD_1_6_t);
      format1_size_bits  = sizeof_DCI1_10MHz_TDD_t;
      format1_size_bytes = sizeof(DCI1_10MHz_TDD_t);
      if (frame_parms->nb_antennas_tx == 2) {
	format2_size_bits  = sizeof_DCI2_10MHz_2A_TDD_t;
	format2_size_bytes = sizeof(DCI2_10MHz_2A_TDD_t);
      }
      else if (frame_parms->nb_antennas_tx == 4) {
	format2_size_bits  = sizeof_DCI2_10MHz_4A_TDD_t;
	format2_size_bytes = sizeof(DCI2_10MHz_4A_TDD_t);
      }
    }
    else {
      format1A_size_bits  = sizeof_DCI1A_10MHz_FDD_t;
      format1A_size_bytes = sizeof(DCI1A_10MHz_FDD_t);
      format0_size_bits  = sizeof_DCI0_10MHz_FDD_t;
      format0_size_bytes = sizeof(DCI0_10MHz_FDD_t);
      format1_size_bits  = sizeof_DCI1_10MHz_FDD_t;
      format1_size_bytes = sizeof(DCI1_10MHz_FDD_t);
      if (frame_parms->nb_antennas_tx == 2) {
	format2_size_bits  = sizeof_DCI2_10MHz_2A_FDD_t;
	format2_size_bytes = sizeof(DCI2_10MHz_2A_FDD_t);
      }
      else if (frame_parms->nb_antennas_tx == 4) {
	format2_size_bits  = sizeof_DCI2_10MHz_4A_FDD_t;
	format2_size_bytes = sizeof(DCI2_10MHz_4A_FDD_t);
      }
    }
    break;

  case 100:
    if (frame_type == TDD) {
      format1A_size_bits  = sizeof_DCI1A_20MHz_TDD_1_6_t;
      format1A_size_bytes = sizeof(DCI1A_20MHz_TDD_1_6_t);
      format0_size_bits  = sizeof_DCI0_20MHz_TDD_1_6_t;
      format0_size_bytes = sizeof(DCI0_20MHz_TDD_1_6_t);
      format1_size_bits  = sizeof_DCI1_20MHz_TDD_t;
      format1_size_bytes = sizeof(DCI1_20MHz_TDD_t);
      if (frame_parms->nb_antennas_tx == 2) {
	format2_size_bits  = sizeof_DCI2_20MHz_2A_TDD_t;
	format2_size_bytes = sizeof(DCI2_20MHz_2A_TDD_t);
      }
      else if (frame_parms->nb_antennas_tx == 4) {
	format2_size_bits  = sizeof_DCI2_20MHz_4A_TDD_t;
	format2_size_bytes = sizeof(DCI2_20MHz_4A_TDD_t);
      }
    }
    else {
      format1A_size_bits  = sizeof_DCI1A_20MHz_FDD_t;
      format1A_size_bytes = sizeof(DCI1A_20MHz_FDD_t);
      format0_size_bits  = sizeof_DCI0_20MHz_FDD_t;
      format0_size_bytes = sizeof(DCI0_20MHz_FDD_t);
      format1_size_bits  = sizeof_DCI1_20MHz_FDD_t;
      format1_size_bytes = sizeof(DCI1_20MHz_FDD_t);
      if (frame_parms->nb_antennas_tx == 2) {
	format2_size_bits  = sizeof_DCI2_20MHz_2A_FDD_t;
	format2_size_bytes = sizeof(DCI2_20MHz_2A_FDD_t);
      }
      else if (frame_parms->nb_antennas_tx == 4) {
	format2_size_bits  = sizeof_DCI2_20MHz_4A_FDD_t;
	format2_size_bytes = sizeof(DCI2_20MHz_4A_FDD_t);
      }
    }
    break;
  }

  if (do_common == 1) { 
#ifdef DEBUG_DCI_DECODING
    printf("[DCI search] doing common search/format0 aggregation 4\n");
#endif
    if (phy_vars_ue->prach_resources[eNB_id])
      ra_rnti = phy_vars_ue->prach_resources[eNB_id]->ra_RNTI;
    
    // First check common search spaces at aggregation 8 (SI_RNTI and RA_RNTI format 0/1A), 
    // and UE_SPEC format0 (PUSCH) too while we're at it
    dci_decoding_procedure0(lte_ue_pdcch_vars,1,subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    2,
			    format1A,
			    format1A,
			    format0,
			    format1A_size_bits,
			    format1A_size_bytes,
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);
    if ((CCEmap0==0xffff) || 
	((format0_found==1)&&(format_c_found==1)))
      return(dci_cnt);
    
    //#ifdef ALL_AGGREGATION
    // Disabled for performance
    // Now check common search spaces at aggregation 8 (SI_RNTI and RA_RNTI format 1A), 
    // and UE_SPEC format0 (PUSCH) too while we're at it
    //  printf("[DCI search] doing common search/format0 aggregation 3\n");
#ifdef DEBUG_DCI_DECODING
    printf("[DCI search] doing common search/format0 aggregation 8\n");
#endif
    dci_decoding_procedure0(lte_ue_pdcch_vars,1,subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    3,
			    format1A,
			    format1A,
			    format0,
			    format1A_size_bits,
			    format1A_size_bytes,
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);
    
    if ((CCEmap0==0xffff)|| 
	((format0_found==1)&&(format_c_found==1)))
      return(dci_cnt);
    //#endif

  }

  if (phy_vars_ue->UE_mode[eNB_id] <= PRACH)
    return(dci_cnt);

  if (phy_vars_ue->prach_resources[eNB_id])
    ra_rnti = phy_vars_ue->prach_resources[eNB_id]->ra_RNTI;

  // Now check UE_SPEC format0/1A ue_spec search spaces at aggregation 8 
  //  printf("[DCI search] Format 0/1A aggregation 8\n");
  dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			  subframe,
			  dci_alloc,
			  eNB_id,
			  frame_parms,
			  mi,
			  SI_RNTI,
			  ra_rnti,
			  3,
			  format1A,
			  format1A,
			  format0,
			  format0_size_bits,
			  format0_size_bytes,
			  &dci_cnt,
			  &format0_found,
			  &format_c_found,
			  &CCEmap0,
			  &CCEmap1,
			  &CCEmap2);
  
  if ((CCEmap0==0xffff)|| 
      ((format0_found==1)&&(format_c_found==1)))
    return(dci_cnt);
  //  printf("[DCI search] Format 0 aggregation 4\n");
  // Now check UE_SPEC format 0 search spaces at aggregation 4 
  dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			  subframe,
			  dci_alloc,
			  eNB_id,
			  frame_parms,
			  mi,
			  SI_RNTI,
			  ra_rnti,
			  2,
			  format1A,
			  format1A,
			  format0,
			  format0_size_bits,
			  format0_size_bytes,
			  &dci_cnt,
			  &format0_found,
			  &format_c_found,
			  &CCEmap0,
			  &CCEmap1,
			  &CCEmap2);
  if ((CCEmap0==0xffff)|| 
      ((format0_found==1)&&(format_c_found==1)))
    return(dci_cnt);

  if ((CCEmap0==0xffff)|| 
      ((format0_found==1)&&(format_c_found==1)))
    return(dci_cnt);
  //  printf("[DCI search] Format 0 aggregation 2\n");
  // Now check UE_SPEC format 0 search spaces at aggregation 2 
  dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			  subframe,
			  dci_alloc,
			  eNB_id,
			  frame_parms,
			  mi,
			  SI_RNTI,
			  ra_rnti,
			  1,
			  format1A,
			  format1A,
			  format0,
			  format0_size_bits,
			  format0_size_bytes,
			  &dci_cnt,
			  &format0_found,
			  &format_c_found,
			  &CCEmap0,
			  &CCEmap1,
			  &CCEmap2);
  if ((CCEmap0==0xffff)|| 
      ((format0_found==1)&&(format_c_found==1)))
    return(dci_cnt);

  //  printf("[DCI search] Format 0 aggregation 4\n");
  // Now check UE_SPEC format 0 search spaces at aggregation 1 
  dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			  subframe,
			  dci_alloc,
			  eNB_id,
			  frame_parms,
			  mi,
			  SI_RNTI,
			  ra_rnti,
			  0,
			  format1A,
			  format1A,
			  format0,
			  format0_size_bits,
			  format0_size_bytes,
			  &dci_cnt,
			  &format0_found,
			  &format_c_found,
			  &CCEmap0,
			  &CCEmap1,
			  &CCEmap2);
  if ((CCEmap0==0xffff)|| 
      ((format0_found==1)&&(format_c_found==1)))
    return(dci_cnt);




  // These are for CRNTI based on transmission mode
  if (tmode < 3) {
    // Now check UE_SPEC format 1 search spaces at aggregation 1 
    old_dci_cnt=dci_cnt;
    dci_decoding_procedure0(lte_ue_pdcch_vars,0,subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    0,
			    format1A,
			    format1A,
			    format1,
			    format1_size_bits,
			    format1_size_bytes,
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);
    if ((CCEmap0==0xffff) || 
	(format_c_found==1))
      return(dci_cnt);
    
    if (dci_cnt>old_dci_cnt)
      return(dci_cnt);
    
    // Now check UE_SPEC format 1 search spaces at aggregation 2 
    old_dci_cnt=dci_cnt;
    dci_decoding_procedure0(lte_ue_pdcch_vars,0,subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    1,
			    format1A,
			    format1A,
			    format1,
			    format1_size_bits,
			    format1_size_bytes,
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);


    if ((CCEmap0==0xffff)|| 
	(format_c_found==1))
      return(dci_cnt);
    if (dci_cnt>old_dci_cnt)
      return(dci_cnt);    

    // Now check UE_SPEC format 1 search spaces at aggregation 4 
    old_dci_cnt=dci_cnt;
    dci_decoding_procedure0(lte_ue_pdcch_vars,0,subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    2,
			    format1A,
			    format1A,
			    format1,
			    format1_size_bits,
			    format1_size_bytes,
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);
    if ((CCEmap0==0xffff)|| 
	((format0_found==1)&&(format_c_found==1)))
      return(dci_cnt);
    if (dci_cnt>old_dci_cnt)
      return(dci_cnt);

    //#ifdef ALL_AGGREGATION
    // Now check UE_SPEC format 1 search spaces at aggregation 8
    old_dci_cnt=dci_cnt;
    dci_decoding_procedure0(lte_ue_pdcch_vars,0,subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    3,
			    format1A,
			    format1A,
			    format1,
			    format1_size_bits,
			    format1_size_bytes,
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);
    if ((CCEmap0==0xffff)|| 
	((format0_found==1)&&(format_c_found==1)))
      return(dci_cnt);
    if (dci_cnt>old_dci_cnt)
      return(dci_cnt);
    
    //#endif //ALL_AGGREGATION    
  }
  else if (tmode == 4) {

    // Now check UE_SPEC format 2_2A_M10PRB search spaces at aggregation 1 
    dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			    subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    0,
			    format1A,
			    format1A,
			    format2,
			    format2_size_bits,
			    format2_size_bytes,
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);
    if ((CCEmap0==0xffff)|| 
	((format0_found==1)&&(format_c_found==1)))
      return(dci_cnt);
    if (dci_cnt>old_dci_cnt)
      return(dci_cnt);

    // Now check UE_SPEC format 2 search spaces at aggregation 2 
    dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			    subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    1,
			    format1A,
			    format1A,
			    format2,
			    format2_size_bits,
			    format2_size_bytes,
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);
    if ((CCEmap0==0xffff)|| 
	((format0_found==1)&&(format_c_found==1)))
      return(dci_cnt);
    if (dci_cnt>old_dci_cnt)
      return(dci_cnt);

    // Now check UE_SPEC format 2_2A_M10PRB search spaces at aggregation 4 
    dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			    subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    2,
			    format1A,
			    format1A,
			    format2,
			    format2_size_bits,
			    format2_size_bytes,
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);
    if ((CCEmap0==0xffff)|| 
	((format0_found==1)&&(format_c_found==1)))
      return(dci_cnt);
    if (dci_cnt>old_dci_cnt)
      return(dci_cnt);

    //#ifdef ALL_AGGREGATION
    // Now check UE_SPEC format 2_2A_M10PRB search spaces at aggregation 8 
    dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			    subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    3,
			    format1A,
			    format1A,
			    format2,
			    format2_size_bits,
			    format2_size_bytes,
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);    
    //#endif
  }
  else if (tmode >=5) { // This is MU-MIMO

  // Now check UE_SPEC format 1E_2A_M10PRB search spaces aggregation 1
#ifdef DEBUG_DCI_DECODING
    LOG_I(PHY," MU-MIMO check UE_SPEC format 1E_2A_M10PRB\n");
#endif 
    dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			    subframe,
			    dci_alloc,
			    eNB_id,
			    frame_parms,
			    mi,
			    SI_RNTI,
			    ra_rnti,
			    0,
			    format1A,
			    format1A,
			    format1E_2A_M10PRB,
			    sizeof_DCI1E_5MHz_2A_M10PRB_TDD_t,
			    sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t),
			    &dci_cnt,
			    &format0_found,
			    &format_c_found,
			    &CCEmap0,
			    &CCEmap1,
			    &CCEmap2);
    
    
    if ((CCEmap0==0xffff)|| 
	((format0_found==1)&&(format_c_found==1)))
      return(dci_cnt);
    
    if (dci_cnt>old_dci_cnt)
      return(dci_cnt);
    
  // Now check UE_SPEC format 1E_2A_M10PRB search spaces aggregation 2
    dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			  subframe,
			  dci_alloc,
			  eNB_id,
			  frame_parms,
			  mi,
			  SI_RNTI,
			  ra_rnti,
			  1,
			  format1A,
			  format1A,
			  format1E_2A_M10PRB,
			  sizeof_DCI1E_5MHz_2A_M10PRB_TDD_t,
			  sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t),
			  &dci_cnt,
			  &format0_found,
			  &format_c_found,
			  &CCEmap0,
			  &CCEmap1,
			  &CCEmap2);

  if ((CCEmap0==0xffff)|| 
      ((format0_found==1)&&(format_c_found==1)))
    return(dci_cnt);
  if (dci_cnt>old_dci_cnt)
    return(dci_cnt);

  // Now check UE_SPEC format 1E_2A_M10PRB search spaces aggregation 4
  dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			  subframe,
			  dci_alloc,
			  eNB_id,
			  frame_parms,
			  mi,
			  SI_RNTI,
			  ra_rnti,
			  2,
			  format1A,
			  format1A,
			  format1E_2A_M10PRB,
			  sizeof_DCI1E_5MHz_2A_M10PRB_TDD_t,
			  sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t),
			  &dci_cnt,
			  &format0_found,
			  &format_c_found,
			  &CCEmap0,
			  &CCEmap1,
			  &CCEmap2);

  if ((CCEmap0==0xffff)|| 
      ((format0_found==1)&&(format_c_found==1)))
    return(dci_cnt);
  if (dci_cnt>old_dci_cnt)
    return(dci_cnt);

  //#ifdef ALL_AGGREGATION

  // Now check UE_SPEC format 1E_2A_M10PRB search spaces at aggregation 8 
  dci_decoding_procedure0(lte_ue_pdcch_vars,0,
			  subframe,
			  dci_alloc,
			  eNB_id,
			  frame_parms,
			  mi,
			  SI_RNTI,
			  ra_rnti,
			  3,
			  format1A,
			  format1A,
			  format1E_2A_M10PRB,
			  sizeof_DCI1E_5MHz_2A_M10PRB_TDD_t,
			  sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t),
			  &dci_cnt,
			  &format0_found,
			  &format_c_found,
			  &CCEmap0,
			  &CCEmap1,
			  &CCEmap2);

  if ((CCEmap0==0xffff)|| 
      ((format0_found==1)&&(format_c_found==1)))
    return(dci_cnt);
  if (dci_cnt>old_dci_cnt)
    return(dci_cnt);

  //#endif  //ALL_AGGREGATION

  } 
  return(dci_cnt);
}

#ifdef PHY_ABSTRACTION
uint16_t dci_decoding_procedure_emul(LTE_UE_PDCCH **lte_ue_pdcch_vars,
				uint8_t num_ue_spec_dci,
				uint8_t num_common_dci,
				DCI_ALLOC_t *dci_alloc_tx,
				DCI_ALLOC_t *dci_alloc_rx,
				int16_t eNB_id) {
 
  uint8_t  dci_cnt=0,i;
  
  memcpy(dci_alloc_rx,dci_alloc_tx,num_common_dci*sizeof(DCI_ALLOC_t));
  dci_cnt = num_common_dci;
  LOG_D(PHY,"[DCI][EMUL] : num_common_dci %d\n",num_common_dci);

  for (i=num_common_dci;i<(num_ue_spec_dci+num_common_dci);i++) {
    LOG_D(PHY,"[DCI][EMUL] Checking dci %d => %x format %d (bit 0 %d)\n",i,lte_ue_pdcch_vars[eNB_id]->crnti,dci_alloc_tx[i].format,
	       dci_alloc_tx[i].dci_pdu[0]&0x80);
    if (dci_alloc_tx[i].rnti == lte_ue_pdcch_vars[eNB_id]->crnti) {
      memcpy(dci_alloc_rx+dci_cnt,dci_alloc_tx+i,sizeof(DCI_ALLOC_t));
      dci_cnt++;
    }
  }


  return(dci_cnt);
}
#endif
