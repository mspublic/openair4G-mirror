/* file: dci.c
   purpose: DCI processing (encoding,decoding,crc extraction) from 36-212, V8.6 2009-03.  
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dci.h"
#include "PHY/defs.h"
#include "PHY/CODING/defs.h"

//#define DEBUG_DCI_ENCODING 1
//#define DEBUG_DCI_DECODING 1

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
  int i;

  // encode dci 

#ifdef DEBUG_DCI_ENCODING
  printf("Doing DCI encoding for %d bits, e %p\n",A,e);
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

#define DCI_BITS (2*576)
void generate_dci_top(unsigned char num_ue_spec_dci,
		      unsigned char num_common_dci,
		      DCI_ALLOC_t *dci_alloc, 
		      unsigned int subframe, 
		      unsigned int n_rnti,
		      short amp,
		      LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned char e[DCI_BITS], *e_ptr, i;

#ifndef IFFT_FPGA 
  short d[2][DCI_BITS];
#else
  modsym_t d[2][DCI_BITS>>1];
  unsigned char qpsk_table_offset = 0; 
  unsigned char qpsk_table_offset2 = 0;
#endif
  e_ptr = e;

  // generate common DCIs first
  i=0;
  for (i=0;i<num_common_dci;i++) {

    e_ptr = generate_dci0(dci_alloc[i].dci_pdu,
			  e_ptr,
			  dci_alloc[i].dci_length,
			  dci_alloc[i].L,
			  dci_alloc[i].rnti);    
  }

  for (;i<num_ue_spec_dci + num_common_dci;i++) {

    e_ptr = generate_dci0(dci_alloc[i].dci_pdu,
			  e_ptr,
			  dci_alloc[i].dci_length,
			  dci_alloc[i].L,
			  dci_alloc[i].rnti);        
  }

  // Scrambling
  // not yet

  // Now do modulation
  gain_lin_QPSK = (short)((amp*ONE_OVER_SQRT2_Q15)>>15);  
  e_ptr = e;
  switch (frame_parms->nb_antennas_tx) {

  case 1:
#ifndef IFFT_FPGA
    for (i=0;i<DCI_BITS;i++) {
      d[0][i] = (*e_ptr == 0) ? -gain_lin_QPSK : gain_lin_QPSK;
      e_ptr++;
    }
#else
    for (i=0;i<DCI_BITS>>1;i++) {
      qpsk_table_offset = 1;
      if (*e_ptr == 1)
	qpsk_table_offset+=1;
      *e_ptr++;
      if (*e_ptr == 1) 
	qpsk_table_offset+=2;
      *e_ptr++;
      
      d[0][i] = (mod_sym_t) qpsk_table_offset;
    }

#endif
    
    break;

  case 2:
#ifndef IFFT_FPGA
      for (i=0;i<DCI_BITS;i+=4) {

	// first antenna position n -> x0
	d[0][i] = (*e_ptr == 0) ? -gain_lin_QPSK : gain_lin_QPSK;
	e_ptr++;
	d[0][i+1] = (*e_ptr == 0) ? -gain_lin_QPSK : gain_lin_QPSK;
	e_ptr++;

	// second antenna position n -> -x1*
	d[1][i] = (*e_ptr == 0) ? gain_lin_QPSK : -gain_lin_QPSK;
	e_ptr++;
	d[1][i+1] = (*e_ptr == 0) ? -gain_lin_QPSK : -gain_lin_QPSK;
	e_ptr++;

	// fill in the rest of the ALAMOUTI precoding
	d[0][i+2] = -d[1][i];
	d[0][i+3] = d[1][i+1];
	d[1][i+2] = d[0][i];
	d[1][i+3] = -d[0][i+1];

      }
#else  
      for (i=0;i<DCI_BITS>>1;i+=2) {
	qpsk_table_offset = 1;  //x0
	qpsk_table_offset2 = 1;  //x0*
	
	if (*e_ptr == 1) { //real
	  qpsk_table_offset+=1;
	  qpsk_table_offset2+=1;
	}
	*e_ptr++;
	
	if (*e_ptr == 1) //imag
	  qpsk_table_offset+=2;
	else
	  qpsk_table_offset2+=2;
	*e_ptr++;
	
	d[0][i]   = (mod_sym_t) qpsk_table_offset;      // x0
	d[1][i+1] = (mod_sym_t) qpsk_table_offset2;   // x0*
	
	
	qpsk_table_offset = 1; //-x1*
	qpsk_table_offset2 = 1; //x1
	
	if (*e_ptr == 1)    // flipping bit for real part of symbol means taking -x1*
	  qpsk_table_offset2+=1;
	else
	  qpsk_table_offset+=1;
	*e_ptr++;
	
	if (*e_ptr == 1) {
	  qpsk_table_offset+=2;
	  qpsk_table_offset2+=2;
	}
	*e_ptr++;
	
	d[1][i] = (mod_sym_t) qpsk_table_offset;     // -x1*
	d[0][i+1] = (mod_sym_t) qpsk_table_offset2;  // x1
      }
#endif    
      break;
  default:
    msg("dci.c: generate_dci_top(), unsupported number of antennas %d\n",frame_parms->nb_antennas_tx);
    exit(-1);
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
  int i;

  if (aggregation_level>3) {
    msg("dci.c: dci_decoding FATAL, illegal aggregation_level %d\n",aggregation_level);
    exit(-1);
  }

  coded_bits = 72 * (1<<aggregation_level);

#ifdef DEBUG_DCI_DECODING
  printf("Doing DCI decoding for %d bits, DCI_LENGTH %d,coded_bits %d\n",3*(DCI_LENGTH+16),DCI_LENGTH,coded_bits);
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
