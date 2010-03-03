/* file: dci.c
   purpose: DCI processing (encoding,decoding,crc extraction) from 36-212, V8.6 2009-03.  
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PHY/defs.h"
#include "PHY/CODING/defs.h"
#include "extern.h"
#include "PHY/extern.h"
#include <emmintrin.h>
#include <xmmintrin.h>
#ifdef __SSE3__
#include <pmmintrin.h>
#include <tmmintrin.h>
#endif

//#define DEBUG_DCI_ENCODING 1
//#define DEBUG_DCI_DECODING 1

#ifndef __SSE3__
__m128i zero2;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero2,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero2,(xmmy)))
#endif



unsigned short extract_crc(unsigned char *dci,unsigned char DCI_LENGTH) {

  unsigned short crc;

  crc = ((short *)dci)[DCI_LENGTH>>4];
  //  printf("crc1: %x, shift %d\n",crc,DCI_LENGTH&0xf);
  crc = (crc>>(DCI_LENGTH&0xf));
  // clear crc bits
  ((short *)dci)[DCI_LENGTH>>4] &= 0xffff>>(16-(DCI_LENGTH&0xf));
  //  printf("crc2: %x, dci0 %x\n",crc,((short *)dci)[DCI_LENGTH>>4]);
  crc |= (((short *)dci)[1+(DCI_LENGTH>>4)])<<(16-(DCI_LENGTH&0xf));
  // clear crc bits
  (((short *)dci)[1+(DCI_LENGTH>>4)]) = 0;

  return(crc);

}
void dci_encoding(unsigned char *a,
		  unsigned char A,
		  unsigned short E,
		  unsigned char *e,
		  unsigned short rnti) {

  unsigned char d[3*(MAX_DCI_SIZE_BITS + 16) + 96];
  unsigned char D = (A + 16);
  unsigned int RCC;
  unsigned char w[3*3*(MAX_DCI_SIZE_BITS+16)];
#ifdef DEBUG_DCI_ENCODING
  int i;
#endif
  // encode dci 

#ifdef DEBUG_DCI_ENCODING
  printf("Doing DCI encoding for %d bits, e %p, rnti %x\n",A,e,rnti);
#endif

  memset((void *)d,LTE_NULL,96);

  ccodelte_encode(A,2,a,d+96,rnti);

#ifdef DEBUG_DCI_ENCODING
  for (i=0;i<16+A;i++)
    printf("%d : (%d,%d,%d)\n",i,*(d+96+(3*i)),*(d+97+(3*i)),*(d+98+(3*i)));
#endif
  
#ifdef DEBUG_DCI_ENCODING
  printf("Doing DCI interleaving for %d coded bits, e %p\n",D,e);
#endif
  RCC = sub_block_interleaving_cc(D,d+96,w);

#ifdef DEBUG_DCI_ENCODING
  printf("Doing DCI rate matching for %d channel bits, RCC %d, e %p\n",E,RCC,e);
#endif
  lte_rate_matching_cc(RCC,E,w,e);


}


unsigned char *generate_dci0(unsigned char *dci,
			     unsigned char *e,
			     unsigned char DCI_LENGTH,
			     unsigned char aggregation_level,
			     unsigned short rnti) {
  
  unsigned short coded_bits;

  if (aggregation_level>3) {
    msg("dci.c: generate_dci FATAL, illegal aggregation_level %d\n",aggregation_level);
    exit(-1);
  }

  coded_bits = 72 * (1<<aggregation_level);

  dci_encoding(dci,DCI_LENGTH,coded_bits,e,rnti);

  return(e+coded_bits);
}

unsigned int Y;

#define CCEBITS 72
#define CCEPERSYMBOL 8
#define PDCCHSYMBOLS 2
#define DCI_BITS (PDCCHSYMBOLS*CCEPERSYMBOL*CCEBITS)
#define Msymb (DCI_BITS/2)
#define Mquad (Msymb/4)

static unsigned int bitrev_cc_dci[32] = {1,17,9,25,5,21,13,29,3,19,11,27,7,23,15,31,0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30};

void pdcch_interleaving(mod_sym_t **z, mod_sym_t **wbar,unsigned short Nid_cell,unsigned char nb_antennas_tx) {

  mod_sym_t wtemp[2][Msymb],*wptr,*wptr2,*zptr;



  unsigned int RCC = (Mquad>>5), ND;
  unsigned int row,col,Kpi,index;
  int i,k,a;
#ifdef RM_DEBUG
  int nulled=0;
#endif
  //  printf("Mquad %d\n",Mquad);
  if ((Mquad&0x1f) > 0)
    RCC++;
  Kpi = (RCC<<5);
  ND = Kpi - Mquad;

  k=0;
  for (col=0;col<32;col++) {
    index = bitrev_cc_dci[col];

    for (row=0;row<RCC;row++) {
      //      printf("col %d, index %d, row %d\n",col,index,row);
      if (index>=ND) {
	for (a=0;a<nb_antennas_tx;a++){
	  //	  printf("a %d k %d\n",a,k);
	  wptr = &wtemp[a][k<<2];
	  zptr = &z[a][(index-ND)<<2];

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

    for (a=0;a<nb_antennas_tx;a++) {
      
      wptr  = &wtemp[a][i<<2];
      wptr2 = &wbar[a][((i+Nid_cell)%Mquad)<<2];
      wptr2[0] = wptr[0];
      wptr2[1] = wptr[1];
      wptr2[2] = wptr[2];
      wptr2[3] = wptr[3];
    }
  }
  
}

void pdcch_demapping(unsigned short *llr,unsigned short *wbar,LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned int i, lprime;
  unsigned short kprime,mprime,symbol_offset,tti_offset;
  short re_offset;

  // demapping from REGs (2 symbols for PDCCH, in symbol w/out reference signals, i.e. 3 REGs/RB)

  mprime=0;


  re_offset = 0;

  
  for (kprime=0;kprime<frame_parms->N_RB_DL*12;kprime+=4) {
    for (lprime=0;lprime<2;lprime++) {

      symbol_offset = (unsigned int)frame_parms->N_RB_DL*12*(1+lprime);
  
      // Copy REG to TX buffer
      for (i=0;i<4;i++) {
	
	tti_offset = symbol_offset + re_offset + i;
	
	
	wbar[mprime] = llr[tti_offset];
	//	printf("demapping %d (symbol %d re %d) -> %d,%d\n",tti_offset,symbol_offset,re_offset+i,((char *)&wbar[mprime])[0],((char *)&wbar[mprime])[1]);
	mprime++;
      }
    }
    re_offset+=4;
    // Stop when all REGs are copied in
    if (mprime>=Msymb)
      break;
  }
}

void pdcch_deinterleaving(unsigned short *z, unsigned short *wbar,unsigned short Nid_cell) {


  unsigned short wtemp[Msymb];
  unsigned short *wptr,*zptr,*wptr2;


  unsigned int RCC = (Mquad>>5), ND;
  unsigned int row,col,Kpi,index;
  int i,k;

  if (!z) {
    msg("dci.c: pdcch_deinterleaving: FATAL z is Null\n");
    exit(-1);
  }
  // undo permutation
  for (i=0;i<Mquad;i++) {
    wptr = &wtemp[i<<2];
    wptr2 = &wbar[((i+Nid_cell)%Mquad)<<2];

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

      if (index>=ND) {


	wptr = &wtemp[k<<2];
	zptr = &z[(index-ND)<<2];
	
	zptr[0] = wptr[0];
	zptr[1] = wptr[1];
	zptr[2] = wptr[2];
	zptr[3] = wptr[3];
	
	k++;
      }
      index+=32;
            
    }
  }


  
}


int pdcch_llr(LTE_DL_FRAME_PARMS *frame_parms,
		   int **rxdataF_comp,
		   char *pdcch_llr,
		   unsigned char symbol) {

  short *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  int i;
  char *pdcch_llr8;

  pdcch_llr8 = &pdcch_llr[2*symbol*frame_parms->N_RB_DL*12];
 
  if (!pdcch_llr8) {
    msg("pdcch_qpsk_llr: llr is null, symbol %d\n",symbol);
    return(-1);
  }
  //  printf("pdcch qpsk llr for symbol %d (pos %d), llr offset %d\n",symbol,(symbol*frame_parms->N_RB_DL*12),pdcch_llr8-pdcch_llr);

  for (i=0;i<(frame_parms->N_RB_DL*24);i++) {
    if (*rxF>7)
      *pdcch_llr8=7;
    else if (*rxF<-8)
      *pdcch_llr8=-8;
    else
      *pdcch_llr8 = (char)(*rxF);

    rxF++;
    pdcch_llr8++;
  }

  return(0);

}

__m128i avg128P;

//compute average channel_level on each (TX,RX) antenna pair
void pdcch_channel_level(int **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 int *avg,
			 unsigned char nb_rb) {

  short rb;
  unsigned char aatx,aarx;
  __m128i *dl_ch128;
  

  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
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

      avg[(aatx<<1)+aarx] = (((int*)&avg128P)[0] + 
			     ((int*)&avg128P)[1] + 
			     ((int*)&avg128P)[2] + 
			     ((int*)&avg128P)[3])/(nb_rb*12);

      //            printf("Channel level : %d\n",avg[(aatx<<1)+aarx]);
    }
  _mm_empty();
  _m_empty();

}

void pdcch_extract_rbs_single(int **rxdataF,
			      int **dl_ch_estimates,
			      int **rxdataF_ext,
			      int **dl_ch_estimates_ext,
			      unsigned char symbol,
			      LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short rb,nb_rb=0;
  unsigned char i,aarx;
  int *dl_ch0,*dl_ch0_ext,*rxF,*rxF_ext;



  unsigned char symbol_mod;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  //  printf("extract_rbs: symbol_mod %d\n",symbol_mod);
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    dl_ch0     = &dl_ch_estimates[aarx][5+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol_mod*(frame_parms->N_RB_DL*12)];

    rxF_ext   = &rxdataF_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];
    
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];
    
    if ((frame_parms->N_RB_DL&1) == 0)  // even number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	
	// For second half of RBs skip DC carrier
	if (rb==(frame_parms->N_RB_DL>>1)) {
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
	  //dl_ch0++; 
	}
	

	memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	/*
	  printf("rb %d\n",rb);
	  for (i=0;i<12;i++)
	  printf("(%d %d)",((short *)dl_ch)[i<<1],((short*)dl_ch)[1+(i<<1)]);
	  printf("\n");*/
	
	for (i=0;i<12;i++) {
	  rxF_ext[i]=rxF[i<<1];
	  //	      printf("%d : (%d,%d)\n",(rxF+(2*i)-&rxdataF[(aatx<<1)+aarx][( (symbol*(frame_parms->ofdm_symbol_size)))*2])/2,
	  //     ((short*)&rxF[i<<1])[0],((short*)&rxF[i<<1])[0]);
	}
	nb_rb++;
	dl_ch0_ext+=12;
	rxF_ext+=12;
	
	dl_ch0+=12;
	rxF+=24;
	
      }
    else {  // Odd number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL>>1;rb++) {
	//	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);
	

	/*	  
		 printf("rb %d\n",rb);
		 for (i=0;i<12;i++)
		 printf("(%d %d)",((short *)dl_ch0)[i<<1],((short*)dl_ch0)[1+(i<<1)]);
		 printf("\n");
	*/

	memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	for (i=0;i<12;i++)
	  rxF_ext[i]=rxF[i<<1];
	nb_rb++;
	dl_ch0_ext+=12;
	rxF_ext+=12;
	
	dl_ch0+=12;
	rxF+=24;
      }
      // Do middle RB (around DC)
      //	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);      

	
      for (i=0;i<6;i++) {
	dl_ch0_ext[i]=dl_ch0[i];
	rxF_ext[i]=rxF[i<<1];
      }
      rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
      for (;i<12;i++) {
	dl_ch0_ext[i]=dl_ch0[i];
	rxF_ext[i]=rxF[(1+i)<<1];
      }
      
      
      nb_rb++;
      dl_ch0_ext+=12;
      rxF_ext+=12;
      dl_ch0+=12;
#ifdef KHZ66_NULL
      dl_ch0_ext[8] = 0;
      dl_ch0_ext[9] = 0;
      dl_ch0_ext[10] = 0;
      dl_ch0_ext[11] = 0;
#endif
      rxF+=14;
      rb++;
      
      for (;rb<frame_parms->N_RB_DL;rb++) {
	//	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);	
	  /*
  	    printf("rb %d\n",rb);
	    for (i=0;i<12;i++)
	    printf("(%d %d)",((short *)dl_ch0)[i<<1],((short*)dl_ch0)[1+(i<<1)]);
	    printf("\n");
	  */

	memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	for (i=0;i<12;i++)
	  rxF_ext[i]=rxF[i<<1];
	nb_rb++;
	dl_ch0_ext+=12;
	rxF_ext+=12;
	
	dl_ch0+=12;
	rxF+=24;
      }
    }
  }

  _mm_empty();
  _m_empty();

}

void pdcch_extract_rbs_dual(int **rxdataF,
			    int **dl_ch_estimates,
			    int **rxdataF_ext,
			    int **dl_ch_estimates_ext,
			    unsigned char symbol,
			    LTE_DL_FRAME_PARMS *frame_parms) {
  

  unsigned short rb,nb_rb=0;
  unsigned char i,aarx;
  int *dl_ch0,*dl_ch0_ext,*dl_ch1,*dl_ch1_ext,*rxF,*rxF_ext;
  unsigned char symbol_mod;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    dl_ch0     = &dl_ch_estimates[aarx][5+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol_mod*(frame_parms->N_RB_DL*12)];
    dl_ch1     = &dl_ch_estimates[2+aarx][5+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    dl_ch1_ext = &dl_ch_estimates_ext[2+aarx][symbol_mod*(frame_parms->N_RB_DL*12)];

    //    printf("pdcch extract_rbs: rxF_ext pos %d\n",symbol*(frame_parms->N_RB_DL*12));
    rxF_ext   = &rxdataF_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];
    
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];
    
    if ((frame_parms->N_RB_DL&1) == 0)  // even number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	// For second half of RBs skip DC carrier
	if (rb==(frame_parms->N_RB_DL>>1)) {
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
	  //dl_ch0++;
	  //dl_ch1++;
	}
	

	memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	  /*
	    printf("rb %d\n",rb);
	    for (i=0;i<12;i++)
	    printf("(%d %d)",((short *)dl_ch)[i<<1],((short*)dl_ch)[1+(i<<1)]);
	    printf("\n");*/
	  
	for (i=0;i<12;i++) {
	  rxF_ext[i]=rxF[i<<1];
	  //	  	      printf("%d : (%d,%d)\n",(rxF+(2*i)-&rxdataF[aarx][( (symbol*(frame_parms->ofdm_symbol_size)))*2])/2,
	  //   ((short*)&rxF[i<<1])[0],((short*)&rxF[i<<1])[0]);
	}
	nb_rb++;
	dl_ch0_ext+=12;
	dl_ch1_ext+=12;
	rxF_ext+=12;
	
	dl_ch0+=12;
	dl_ch1+=12;
	rxF+=24;

      }
    else {  // Odd number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL>>1;rb++) {

	//	printf("rb %d: %d\n",rb,rxF-&rxdataF[aarx][(symbol*(frame_parms->ofdm_symbol_size))*2]);

	memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	for (i=0;i<12;i++)
	  rxF_ext[i]=rxF[i<<1];
	nb_rb++;
	dl_ch0_ext+=12;
	dl_ch1_ext+=12;
	rxF_ext+=12;
	
	dl_ch0+=12;
	dl_ch1+=12;
	rxF+=24;

      }
      // Do middle RB (around DC)

      for (i=0;i<6;i++) {
	dl_ch0_ext[i]=dl_ch0[i];
	dl_ch1_ext[i]=dl_ch1[i];
	rxF_ext[i]=rxF[i<<1];
      }

      rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
      for (;i<12;i++) {
	dl_ch0_ext[i]=dl_ch0[i];
	dl_ch1_ext[i]=dl_ch1[i];
	rxF_ext[i]=rxF[(1+i)<<1];
      }
#ifdef KHZ66_NULL
      dl_ch0_ext[8] = 0;
      dl_ch1_ext[8] = 0;
      dl_ch0_ext[9] = 0;
      dl_ch1_ext[9] = 0;
      dl_ch0_ext[10] = 0;
      dl_ch1_ext[10] = 0;
      dl_ch0_ext[11] = 0;
      dl_ch1_ext[11] = 0;
#endif
      nb_rb++;
      dl_ch0_ext+=12;
      dl_ch1_ext+=12;
      rxF_ext+=12;
    
      dl_ch0+=12;
      dl_ch1+=12;
      rxF+=14;
      rb++;

      for (;rb<frame_parms->N_RB_DL;rb++) {
	
	//	printf("rb %d: %d\n",rb,rxF-&rxdataF[aarx][(symbol*(frame_parms->ofdm_symbol_size))*2]);
	memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	for (i=0;i<12;i++)
	  rxF_ext[i]=rxF[i<<1];
	nb_rb++;
	dl_ch0_ext+=12;
	dl_ch1_ext+=12;
	rxF_ext+=12;
	
	dl_ch0+=12;
	dl_ch1+=12;
	rxF+=24;

	
      }
    }
  }
  _mm_empty();
  _m_empty();
  

}



__m128i mmtmpPD0,mmtmpPD1,mmtmpPD2,mmtmpPD3;

void pdcch_channel_compensation(int **rxdataF_ext,
				int **dl_ch_estimates_ext,
				int **rxdataF_comp,
				LTE_DL_FRAME_PARMS *frame_parms,
				unsigned char symbol,
				unsigned char output_shift) {

  unsigned short rb;
  __m128i *dl_ch128,*rxdataF128,*rxdataF_comp128;
  unsigned char aatx,aarx;



#ifndef __SSE3__
  zero2 = _mm_xor_si128(zero2,zero2);
#endif

  //  printf("comp: symbol %d\n",symbol);
  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++) {
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
	//	print_shorts("rx:",rxdataF128+2);
	//	print_shorts("ch:",dl_ch128+2);
	//      	print_shorts("pack:",rxdataF_comp128+2);
      
	dl_ch128+=3;
	rxdataF128+=3;
	rxdataF_comp128+=3;
	
      }
    }
  }

  _mm_empty();
  _m_empty();

}     

void pdcch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			 int **rxdataF_comp,
			 unsigned char symbol) {

  unsigned char aatx;

  __m128i *rxdataF_comp128_0,*rxdataF_comp128_1;
  int i;

  if (frame_parms->nb_antennas_rx>1) {
    for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++) {
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
/*
void pdcch_siso(LTE_DL_FRAME_PARMS *frame_parms,
		int **rxdataF_comp,
		unsigned char l) {


  unsigned char symbol_offset,second_pilot,rb,re,jj,ii;

  jj=0;
  ii=0;
  for (rb=0;rb<frame_parms->N_RB_DL;rb++) {

    for (re=0;re<12;re++) {
      
      rxdataF_comp[0][jj++] = rxdataF_comp[0][ii];
      ii++;
    }
  }
}
*/

void pdcch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    unsigned char symbol){


  short *rxF0,*rxF1;
  unsigned char rb,re;
  int jj=(symbol*frame_parms->N_RB_DL*12);

  rxF0     = (short*)&rxdataF_comp[0][jj];  //tx antenna 0  h0*y
  rxF1     = (short*)&rxdataF_comp[2][jj];  //tx antenna 1  h1*y

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

int avgP[4];

int rx_pdcch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_PDCCH **lte_ue_pdcch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     unsigned char eNb_id,
	     unsigned char n_pdcch_symbols,
	     MIMO_mode_t mimo_mode) {

  unsigned char log2_maxh,aatx,aarx;
  int avgs,s;

  for (s=1;s<1+n_pdcch_symbols;s++) {
    if (frame_parms->nb_antennas_tx>1) {
      pdcch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
			     lte_ue_common_vars->dl_ch_estimates[eNb_id],
			     lte_ue_pdcch_vars[eNb_id]->rxdataF_ext,
			     lte_ue_pdcch_vars[eNb_id]->dl_ch_estimates_ext,
			     s,
			     frame_parms);
    }
    else {
      
      pdcch_extract_rbs_single(lte_ue_common_vars->rxdataF,
			       lte_ue_common_vars->dl_ch_estimates[eNb_id],
			       lte_ue_pdcch_vars[eNb_id]->rxdataF_ext,
			       lte_ue_pdcch_vars[eNb_id]->dl_ch_estimates_ext,
			       s,
			       frame_parms);
    }
  }
  pdcch_channel_level(lte_ue_pdcch_vars[eNb_id]->dl_ch_estimates_ext,
		      frame_parms,
		      avgP,
		      frame_parms->N_RB_DL);  
  
  avgs = 0;
  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
      avgs = max(avgs,avgP[(aarx<<1)+aatx]);
  
  log2_maxh = 4+(log2_approx(avgs)/2);
#ifdef DEBUG_PHY
  msg("[PDCCH] log2_maxh = %d (%d,%d)\n",log2_maxh,avgP[0],avgs);
#endif

  for (s=1;s<1+n_pdcch_symbols;s++) {
    pdcch_channel_compensation(lte_ue_pdcch_vars[frame_parms->Nid_cell%3]->rxdataF_ext,
			       lte_ue_pdcch_vars[frame_parms->Nid_cell%3]->dl_ch_estimates_ext,
			       lte_ue_pdcch_vars[frame_parms->Nid_cell % 3]->rxdataF_comp,
			       frame_parms,
			       s,
			       log2_maxh); // log2_maxh+I0_shift
    

    if (frame_parms->nb_antennas_rx > 1)
      pdcch_detection_mrc(frame_parms,
			  lte_ue_pdcch_vars[frame_parms->Nid_cell % 3]->rxdataF_comp,
			  s);

  

    if (mimo_mode == SISO) {
      //    pdcch_siso(frame_parms,lte_ue_pdcch_vars[frame_parms->Nid_cell % 3]->rxdataF_comp,s);
    }
    else
      pdcch_alamouti(frame_parms,lte_ue_pdcch_vars[frame_parms->Nid_cell % 3]->rxdataF_comp,s);

    pdcch_llr(frame_parms,
	      lte_ue_pdcch_vars[frame_parms->Nid_cell % 3]->rxdataF_comp,
	      lte_ue_pdcch_vars[frame_parms->Nid_cell % 3]->llr,
	      s);


  }

  pdcch_demapping(lte_ue_pdcch_vars[frame_parms->Nid_cell % 3]->llr,
		  lte_ue_pdcch_vars[frame_parms->Nid_cell % 3]->wbar,
		  frame_parms);

  pdcch_deinterleaving((unsigned short*)lte_ue_pdcch_vars[frame_parms->Nid_cell % 3]->e_rx,
		       lte_ue_pdcch_vars[frame_parms->Nid_cell % 3]->wbar,
		       frame_parms->Nid_cell);

}
	     

	     
void generate_dci_top(unsigned char num_ue_spec_dci,
		      unsigned char num_common_dci,
		      DCI_ALLOC_t *dci_alloc, 
		      unsigned int n_rnti,
		      short amp,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      mod_sym_t **txdataF,
		      unsigned int sub_frame_offset) {

  unsigned char e[DCI_BITS], *e_ptr;
  unsigned int i, lprime;
  unsigned short gain_lin_QPSK,kprime,mprime,nsymb,symbol_offset,tti_offset;
  short re_offset;

  mod_sym_t y0[Msymb],y1[Msymb],*y[2];
  mod_sym_t wbar0[Msymb],wbar1[Msymb],*wbar[2];
  
  wbar[0] = &wbar0[0];
  wbar[1] = &wbar1[0];
  y[0] = &y0[0];
  y[1] = &y1[0];
#ifdef IFFT_FPGA
  unsigned char qpsk_table_offset = 0; 
  unsigned char qpsk_table_offset2 = 0;
#endif

  e_ptr = e;

  // generate common DCIs first
  i=0;
  for (i=0;i<num_common_dci;i++) {

#ifdef DEBUG_DCI_ENCODING
    msg("Generating common DCI %d of length %d, aggregation %d\n",i,dci_alloc[i].dci_length,dci_alloc[i].L);
#endif
    e_ptr = generate_dci0(dci_alloc[i].dci_pdu,
			  e_ptr,
			  dci_alloc[i].dci_length,
			  dci_alloc[i].L,
			  dci_alloc[i].rnti);    
  }

  for (;i<num_ue_spec_dci + num_common_dci;i++) {

#ifdef DEBUG_DCI_ENCODING
    msg("Generating UE specific DCI %d of length %d, aggregation %d\n",i,dci_alloc[i].dci_length,dci_alloc[i].L);
#endif
    e_ptr = generate_dci0(dci_alloc[i].dci_pdu,
			  e_ptr,
			  dci_alloc[i].dci_length,
			  dci_alloc[i].L,
			  dci_alloc[i].rnti);        
  }

  // Scrambling
  // not yet

#ifdef DEBUG_DCI_ENCODING
  printf("PDCCH Modulation\n");
#endif
  // Now do modulation
  gain_lin_QPSK = (short)((amp*ONE_OVER_SQRT2_Q15)>>15);  
  e_ptr = e;
  switch (frame_parms->nb_antennas_tx) {

  case 1:
#ifndef IFFT_FPGA
    for (i=0;i<Msymb;i++) {
      ((short*)(&(y[0][i])))[0] = (*e_ptr == 0) ? -gain_lin_QPSK : gain_lin_QPSK;
      e_ptr++;
      ((short*)(&(y[0][i])))[1] = (*e_ptr == 0) ? -gain_lin_QPSK : gain_lin_QPSK;
      e_ptr++;
    }
#else
    for (i=0;i<Msymb;i++) {
      qpsk_table_offset = 1;
      if (*e_ptr == 1)
	qpsk_table_offset+=1;
      e_ptr++;
      if (*e_ptr == 1) 
	qpsk_table_offset+=2;
      e_ptr++;
      
      y[0][i] = (mod_sym_t) qpsk_table_offset;
    }

#endif
    
    break;

  case 2:
#ifndef IFFT_FPGA
      for (i=0;i<Msymb;i+=2) {

#ifdef DEBUG_DCI_ENCODING
  printf("PDCCH Modulation: REG %d\n",i>>2);
#endif
	// first antenna position n -> x0
	((short*)&y[0][i])[0] = (*e_ptr == 0) ? -gain_lin_QPSK : gain_lin_QPSK;
	e_ptr++;
	((short*)&y[0][i])[1] = (*e_ptr == 0) ? -gain_lin_QPSK : gain_lin_QPSK;
	e_ptr++;

	// second antenna position n -> -x1*
	((short*)&y[1][i])[0] = (*e_ptr == 0) ? gain_lin_QPSK : -gain_lin_QPSK;
	e_ptr++;
	((short*)&y[1][i])[1] = (*e_ptr == 0) ? -gain_lin_QPSK : -gain_lin_QPSK;
	e_ptr++;

	// fill in the rest of the ALAMOUTI precoding
	((short*)&y[0][i+1])[0] = -((short*)&y[1][i])[0];
	((short*)&y[0][i+1])[1] = ((short*)&y[1][i])[1];
	((short*)&y[1][i+1])[0] = ((short*)&y[0][i])[0];
	((short*)&y[1][i+1])[1] = -((short*)&y[0][i])[1];

      }
#else  
      for (i=0;i<Msymb;i+=2) {
#ifdef DEBUG_DCI_ENCODING
  printf("PDCCH Modulation: Symbol %d : REG %d/%d\n",i,i>>2,Msymb>>2);
#endif
	qpsk_table_offset = 1;  //x0
	qpsk_table_offset2 = 1;  //x0*
	
	if (*e_ptr == 1) { //real
	  qpsk_table_offset+=1;
	  qpsk_table_offset2+=1;
	}
	e_ptr++;
	
	if (*e_ptr == 1) //imag
	  qpsk_table_offset+=2;
	else
	  qpsk_table_offset2+=2;
	e_ptr++;
	
	y[0][i]   = (mod_sym_t) qpsk_table_offset;      // x0
	y[1][i+1] = (mod_sym_t) qpsk_table_offset2;   // x0*
	
	
	qpsk_table_offset = 1; //-x1*
	qpsk_table_offset2 = 1; //x1
	
	if (*e_ptr == 1)    // flipping bit for real part of symbol means taking -x1*
	  qpsk_table_offset2+=1;
	else
	  qpsk_table_offset+=1;
	e_ptr++;
	
	if (*e_ptr == 1) {
	  qpsk_table_offset+=2;
	  qpsk_table_offset2+=2;
	}
	e_ptr++;
	
	y[1][i] = (mod_sym_t) qpsk_table_offset;     // -x1*
	y[0][i+1] = (mod_sym_t) qpsk_table_offset2;  // x1
      }
#endif    
      break;
  default:
    msg("dci.c: generate_dci_top(), unsupported number of antennas %d\n",frame_parms->nb_antennas_tx);
    exit(-1);
    break;

  }

#ifdef DEBUG_DCI_ENCODING
  printf("PDCCH Interleaving\n");
#endif

  //  printf("y %p (%p,%p), wbar %p (%p,%p)\n",y,y[0],y[1],wbar,wbar[0],wbar[1]);
  pdcch_interleaving(&y[0],&wbar[0],frame_parms->Nid_cell,frame_parms->nb_antennas_tx);

  // mapping to REGs (2 symbols for PDCCH, in symbol w/out reference signals, i.e. 3 REGs/RB)

  mprime=0;
  nsymb = (frame_parms->Ncp==0) ? 14:12;
#ifdef IFFT_FPGA
  re_offset = frame_parms->N_RB_DL*12/2;
#else
  re_offset = frame_parms->first_carrier_offset;
#endif
  
  for (kprime=0;kprime<frame_parms->N_RB_DL*12;kprime+=4) {
    for (lprime=0;lprime<2;lprime++) {

#ifdef IFFT_FPGA      
      symbol_offset = (unsigned int)frame_parms->N_RB_DL*12*(1+lprime+(sub_frame_offset*nsymb));
  
#else
      symbol_offset = (unsigned int)frame_parms->ofdm_symbol_size*(1+lprime+(sub_frame_offset*nsymb));
#endif
      // Copy REG to TX buffer
      for (i=0;i<4;i++) {
	

	
#ifdef IFFT_FPGA
	if ((re_offset+i) == (frame_parms->N_RB_DL*12))
	  re_offset = re_offset - (frame_parms->N_RB_DL*12);
#else
	if ((re_offset+i) == (frame_parms->ofdm_symbol_size))
	  re_offset = re_offset + 1 - (frame_parms->ofdm_symbol_size);
#endif

	tti_offset = symbol_offset + re_offset+i;	
	txdataF[0][tti_offset] = wbar[0][mprime];
	if (frame_parms->nb_antennas_tx > 1)
	  txdataF[1][tti_offset] = wbar[1][mprime];
	//	printf("PDCCH mapping %d (symbol %d re %d) -> %d\n",tti_offset,symbol_offset,re_offset+i,wbar[0][mprime]);
	mprime++;
      }
    }
    re_offset+=4;
#ifdef IFFT_FPGA
    if (re_offset == (frame_parms->N_RB_DL*12))
      re_offset = 0;
#else
    if (re_offset == (frame_parms->ofdm_symbol_size))
      re_offset = 1;
#endif

    // Stop when all REGs are copied out
    if (mprime>=Msymb)
      break;
  }
}
 
void dci_decoding(unsigned char DCI_LENGTH,
		  unsigned char aggregation_level,
		  char *e,
		  unsigned char *decoded_output) {

  unsigned char dummy_w[3*(MAX_DCI_SIZE_BITS+16+64)];
  unsigned short RCC;
  char w[3*(DCI_LENGTH+16+32)],d[96+(3*DCI_LENGTH+16)];
  unsigned short D=(DCI_LENGTH+16+64);
  unsigned short coded_bits;
#ifdef DEBUG_DCI_DECODING
  int i;
#endif
  if (aggregation_level>3) {
    msg("dci.c: dci_decoding FATAL, illegal aggregation_level %d\n",aggregation_level);
    exit(-1);
  }

  coded_bits = 72 * (1<<aggregation_level);

#ifdef DEBUG_DCI_DECODING
  printf("Doing DCI decoding for %d bits, DCI_LENGTH %d,coded_bits %d, e %p\n",3*(DCI_LENGTH+16),DCI_LENGTH,coded_bits,e);
#endif
  
  // now do decoding
  memset(dummy_w,0,3*D);
  RCC = generate_dummy_w_cc(DCI_LENGTH+16,
			    dummy_w);


   
#ifdef DEBUG_DCI_DECODING
  printf("Doing DCI Rate Matching RCC %d, w %p\n",RCC,w);
#endif

  lte_rate_matching_cc_rx(RCC,coded_bits,w,dummy_w,e);
 
  sub_block_deinterleaving_cc((unsigned int)(DCI_LENGTH+16), 
			      &d[96], 
			      &w[0]); 
 
  memset(decoded_output,0,1+(DCI_LENGTH/8));
  
#ifdef DEBUG_DCI_DECODING
  printf("Doing DCI Viterbi %d\n");

  for (i=0;i<16+DCI_LENGTH;i++)
    printf("%d : (%d,%d,%d)\n",i,*(d+96+(3*i)),*(d+97+(3*i)),*(d+98+(3*i)));
#endif  
  phy_viterbi_lte_sse2(d+96,decoded_output,16+DCI_LENGTH);
}

static unsigned char dci_decoded_output[MAX_DCI_SIZE_BITS+16];

unsigned short dci_decoding_procedure(LTE_UE_PDCCH **lte_ue_pdcch_vars,
				      DCI_ALLOC_t *dci_alloc,
				      short eNb_id,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      unsigned short si_rnti,
				      unsigned short ra_rnti,
				      unsigned short c_rnti) {
  
  unsigned short crc,dci_cnt,first_found,second_found,dci_len;


  // Aggregation level 8
  dci_cnt = 0;
  first_found  = 0;
  second_found = 0;
  // Try common DCI 0 and 1A first in first half
  dci_len = sizeof_DCI1A_5MHz_TDD_1_6_t;
  dci_decoding(dci_len,
	       3,
	       lte_ue_pdcch_vars[eNb_id]->e_rx,
	       dci_decoded_output);
  crc = extract_crc(dci_decoded_output,dci_len) ^ (crc16(dci_decoded_output,dci_len)>>16); 
  printf("CRC : %x\n",crc);
  if (crc == si_rnti) {
    dci_alloc[dci_cnt].dci_length = dci_len;
    dci_alloc[dci_cnt].rnti       = si_rnti;
    dci_alloc[dci_cnt].L          = 8;
    dci_alloc[dci_cnt].format     = format1A;
    memcpy(&dci_alloc[dci_cnt].dci_pdu[0],dci_decoded_output,sizeof(DCI1A_5MHz_TDD_1_6_t));
    dci_cnt++;
    first_found=1;
    msg("DCI Aggregation 8: Found DCI 1A (SI_RNTI) in first position\n");
  }
  else if (crc == c_rnti) {
    dci_alloc[dci_cnt].dci_length = dci_len;
    dci_alloc[dci_cnt].rnti       = c_rnti;
    dci_alloc[dci_cnt].L          = 8;
    dci_alloc[dci_cnt].format     = format0;
    memcpy(&dci_alloc[dci_cnt].dci_pdu[0],dci_decoded_output,sizeof(DCI0_5MHz_TDD_1_6_t));
    dci_cnt++;
    first_found=1;
    msg("DCI Aggregation 8: Found DCI 0 (C_RNTI) in first position\n");
  }
  else if (crc == ra_rnti) {
    dci_alloc[dci_cnt].dci_length = dci_len;
    dci_alloc[dci_cnt].rnti       = ra_rnti;
    dci_alloc[dci_cnt].L          = 8;
    dci_alloc[dci_cnt].format     = format0;
    memcpy(&dci_alloc[dci_cnt].dci_pdu[0],dci_decoded_output,sizeof(DCI0_5MHz_TDD_1_6_t));
    dci_cnt++;
    first_found=1;
    msg("DCI Aggregation 8: Found DCI 0 (RA_RNTI) in first position\n");
  }

  // Try common DCI 0 and 1A first in second half
  dci_len = sizeof_DCI1A_5MHz_TDD_1_6_t;
  dci_decoding(dci_len,
	       3,
	       &lte_ue_pdcch_vars[eNb_id]->e_rx[DCI_BITS/2],
	       dci_decoded_output);
  crc = (extract_crc(dci_decoded_output,dci_len) ^ (crc16(dci_decoded_output,dci_len)>>16)); 
  printf("CRC : %x\n",crc);
  if (crc == si_rnti) {
    dci_alloc[dci_cnt].dci_length = dci_len;
    dci_alloc[dci_cnt].rnti       = si_rnti;
    dci_alloc[dci_cnt].L          = 8;
    dci_alloc[dci_cnt].format     = format1A;
    memcpy(&dci_alloc[dci_cnt].dci_pdu[0],dci_decoded_output,sizeof(DCI1A_5MHz_TDD_1_6_t));
    dci_cnt++;
    second_found = 1;
    msg("DCI Aggregation 8: Found DCI 1A (SI_RNTI) in second position\n");
  }
  else if (crc == c_rnti) {
    dci_alloc[dci_cnt].dci_length = dci_len;
    dci_alloc[dci_cnt].rnti       = c_rnti;
    dci_alloc[dci_cnt].L          = 8;
    dci_alloc[dci_cnt].format     = format0;
    memcpy(&dci_alloc[dci_cnt].dci_pdu[0],dci_decoded_output,sizeof(DCI0_5MHz_TDD_1_6_t));
    dci_cnt++;
    second_found = 1;
    msg("DCI Aggregation 8: Found DCI 0 (C_RNTI) in second position\n");
  }
  else if (crc == ra_rnti) {
    dci_alloc[dci_cnt].dci_length = dci_len;
    dci_alloc[dci_cnt].rnti       = ra_rnti;
    dci_alloc[dci_cnt].L          = 8;
    dci_alloc[dci_cnt].format     = format0;
    memcpy(&dci_alloc[dci_cnt].dci_pdu[0],dci_decoded_output,sizeof(DCI0_5MHz_TDD_1_6_t));
    dci_cnt++;
    second_found = 1;
    msg("DCI Aggregation 8: Found DCI 0 (RA_RNTI) in second position\n");
  }

  if (dci_cnt==2)
    return(dci_cnt);

  // Try DLSCH 2 < 10 RB
  if (first_found == 0) {
    dci_len = sizeof_DCI2_5MHz_2A_L10PRB_TDD_t;
    dci_decoding(dci_len,
		 3,
		 lte_ue_pdcch_vars[eNb_id]->e_rx,
		 dci_decoded_output);
    crc = (extract_crc(dci_decoded_output,dci_len) ^ (crc16(dci_decoded_output,dci_len)>>16)); 
    printf("CRC : %x\n",crc);
    if (crc == c_rnti) {
      dci_alloc[dci_cnt].dci_length = dci_len;
      dci_alloc[dci_cnt].rnti       = c_rnti;
      dci_alloc[dci_cnt].L          = 8;
      dci_alloc[dci_cnt].format     = format2_2A_L10PRB;
      memcpy(&dci_alloc[dci_cnt].dci_pdu[0],dci_decoded_output,sizeof(DCI2_5MHz_2A_L10PRB_TDD_t));
      dci_cnt++;
      first_found=1;
    msg("DCI Aggregation 8: Found DCI 2 (C_RNTI) in first position\n");
    }
  }

  if (dci_cnt==2)
    return(dci_cnt);

  if (second_found == 0) {
    dci_len = sizeof_DCI2_5MHz_2A_L10PRB_TDD_t;
    dci_decoding(dci_len,
		 3,
		 &lte_ue_pdcch_vars[eNb_id]->e_rx[DCI_BITS/2],
		 dci_decoded_output);
    crc = (extract_crc(dci_decoded_output,dci_len) ^ (crc16(dci_decoded_output,dci_len)>>16)); 
    printf("CRC : %x\n",crc);
    if (crc == c_rnti) {
      dci_alloc[dci_cnt].dci_length = dci_len;
      dci_alloc[dci_cnt].rnti       = c_rnti;
      dci_alloc[dci_cnt].L          = 8;
      dci_alloc[dci_cnt].format     = format2_2A_L10PRB;
      memcpy(&dci_alloc[dci_cnt].dci_pdu[0],dci_decoded_output,sizeof(DCI2_5MHz_2A_L10PRB_TDD_t));
      dci_cnt++;
      second_found=1;
      msg("DCI Aggregation 8: Found DCI 2_L10PRB (C_RNTI) in second position\n");
    }
  }

  if (dci_cnt==2)
    return(dci_cnt);

  // Try DLSCH 2 > 10 RB
  if (first_found == 0) {
    dci_len = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
    dci_decoding(dci_len,
		 3,
		 lte_ue_pdcch_vars[eNb_id]->e_rx,
		 dci_decoded_output);
    crc = (extract_crc(dci_decoded_output,dci_len) ^ (crc16(dci_decoded_output,dci_len)>>16)); 
    printf("CRC : %x\n",crc);
    if (crc == c_rnti) {
      dci_alloc[dci_cnt].dci_length = dci_len;
      dci_alloc[dci_cnt].rnti       = c_rnti;
      dci_alloc[dci_cnt].L          = 8;
      dci_alloc[dci_cnt].format     = format2_2A_M10PRB;;
      memcpy(&dci_alloc[dci_cnt].dci_pdu[0],dci_decoded_output,sizeof(DCI2_5MHz_2A_L10PRB_TDD_t));
      dci_cnt++;
      first_found=1;
      msg("DCI Aggregation 8: Found DCI 2_M10PRB (C_RNTI) in first position\n");
    }
  }

  if (dci_cnt==2)
    return(dci_cnt);

  if (second_found == 0) {
    dci_len = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
    dci_decoding(dci_len,
		 3,
		 &lte_ue_pdcch_vars[eNb_id]->e_rx[DCI_BITS/2],
		 dci_decoded_output);
    crc = (extract_crc(dci_decoded_output,dci_len) ^ (crc16(dci_decoded_output,dci_len)>>16)); 
    printf("CRC : %x\n",crc);
    if (crc == c_rnti) {
      dci_alloc[dci_cnt].dci_length = dci_len;
      dci_alloc[dci_cnt].rnti       = c_rnti;
      dci_alloc[dci_cnt].L          = 8;
      dci_alloc[dci_cnt].format     = format2_2A_L10PRB;
      memcpy(&dci_alloc[dci_cnt].dci_pdu[0],dci_decoded_output,sizeof(DCI2_5MHz_2A_L10PRB_TDD_t));
      dci_cnt++;
      second_found=1;
      msg("DCI Aggregation 8: Found DCI 2_M10PRB (C_RNTI) in second position\n");
    }
  }

  return(dci_cnt);



}
