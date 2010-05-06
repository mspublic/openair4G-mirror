#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "PHY/LTE_TRANSPORT/defs.h"
#include "defs.h"

//#define DEBUG_DLSCH_MODULATION

/*
#define is_not_pilot(pilots,first_pilot,re) (pilots==0) || \ 
	((pilots==1)&&(first_pilot==1)&&(((re>2)&&(re<6))||((re>8)&&(re<12)))) || \
	((pilots==1)&&(first_pilot==0)&&(((re<3))||((re>5)&&(re<9)))) \
*/
#define is_not_pilot(pilots,first_pilot,re) (1)



void generate_64qam_table(void) {

  int a,b,c,index;


  for (a=-1;a<=1;a+=2) 
    for (b=-1;b<=1;b+=2) 
      for (c=-1;c<=1;c+=2) {
	index = (1+a)*2 + (1+b) + (1+c)/2;  
	qam64_table[index] = a*(QAM64_n1 + b*(QAM64_n2 + (c*QAM64_n3))); // 0 1 2
      } 
}

void generate_16qam_table(void) {

  int a,b,index;

  for (a=-1;a<=1;a+=2) 
    for (b=-1;b<=1;b+=2) {
	index = (1+a) + (1+b)/2;  
	qam16_table[index] = a*(QAM16_n1 + (b*QAM16_n2)); 
      } 
}

#ifndef IFFT_FPGA
 
int allocate_REs_in_RB(mod_sym_t **txdataF,
		       unsigned int *jj,
		       unsigned short re_offset,
		       unsigned int symbol_offset,
		       unsigned char *output,
		       MIMO_mode_t mimo_mode,
		       unsigned char nu,
		       unsigned char pilots,
		       unsigned char first_pilot,
		       unsigned char mod_order,
		       unsigned char precoding,
		       short amp,
		       unsigned int *re_allocated,
		       unsigned char skip_dc,
		       LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned int tti_offset, aa;
  unsigned char re;
  unsigned char qam64_table_offset_re = 0;
  unsigned char qam64_table_offset_im = 0;
  unsigned char qam16_table_offset_re = 0;
  unsigned char qam16_table_offset_im = 0;
  short gain_lin_QPSK,gain_lin_16QAM1,gain_lin_16QAM2;
  short re_off=re_offset;
  gain_lin_QPSK = (short)((amp*ONE_OVER_SQRT2_Q15)>>15);  
  
  switch (mod_order) {
  case 2:
    // QPSK single stream
    
    break;
  case 4:
    //16QAM Single stream
    gain_lin_16QAM1 = (short)(((int)amp*QAM16_n1)>>15);
    gain_lin_16QAM2 = (short)(((int)amp*QAM16_n2)>>15);
    
    break;
    
  case 6:
    //64QAM Single stream
    break;
  default:
    break;
  }
#ifdef DEBUG_DLSCH_MODULATION
  printf("allocate_re : re_offset %d (%d), jj %d -> %d,%d, nu %d\n",re_offset,skip_dc,*jj, output[*jj], output[1+*jj],nu);
#endif

  for (re=0;re<12;re++) {
    
    
    if ((skip_dc == 1) && (re==6))
      re_off=re_off - frame_parms->ofdm_symbol_size+1;
    // check that re is not a pilot (need shift for 2nd pilot symbol!!!!)
    // Again this is not LTE, here for SISO only positions 3-5 and 8-11 are allowed for data REs
    // This allows for pilots from adjacent eNbs to be interference free
    
    tti_offset = symbol_offset + re_off + re;
    if (is_not_pilot(pilots,first_pilot,re)) { 
      
      *re_allocated = *re_allocated + 1;
      
      if (mimo_mode == SISO) {  //SISO mapping
	
	switch (mod_order) {
	case 2:  //QPSK
	  
	  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
	    ((short*)&txdataF[aa][tti_offset])[0] = (output[*jj]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  *jj = *jj + 1;
	  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++)
	    ((short*)&txdataF[aa][tti_offset])[1] = (output[*jj]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  *jj = *jj + 1;
	  break;
	  
	case 4:  //16QAM
	  
	  qam16_table_offset_re = 0;
	  if (output[*jj] == 1)
	    qam16_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_re+=1;
	  *jj=*jj+1;
	  
	  
	  qam16_table_offset_im = 0;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam16_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  ((short *)&txdataF[nu][tti_offset])[0]=(short)(((int)amp*qam16_table[qam16_table_offset_re])>>15);
	  ((short *)&txdataF[nu][tti_offset])[1]=(short)(((int)amp*qam16_table[qam16_table_offset_im])>>15);
	  
	  break;
	  
	case 6:  //64QAM
	  
	  
	  qam64_table_offset_re = 0;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_re+=1;
	  *jj=*jj+1;
	  
	  
	  qam64_table_offset_im = 0;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=4;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=2;
	  *jj=*jj+1;
	  if (output[*jj] == 1)
	    qam64_table_offset_im+=1;
	  *jj=*jj+1;
	  
	  ((short *)&txdataF[nu][tti_offset])[0]=(short)(((int)amp*qam64_table[qam64_table_offset_re])>>15);
	  ((short *)&txdataF[nu][tti_offset])[1]=(short)(((int)amp*qam64_table[qam64_table_offset_im])>>15);
	  break;
	  
	}
      }
      
	else if (mimo_mode == ALAMOUTI){
	  
	  switch (mod_order) {
	  case 2:  //QPSK
	    
	    // first antenna position n -> x0
	    
	    ((short*)&txdataF[0][tti_offset])[0] = (output[*jj]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    *jj=*jj+1;
	    ((short*)&txdataF[0][tti_offset])[1] = (output[*jj]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    *jj=*jj+1;
	    
	    // second antenna position n -> -x1*
	    
	    ((short*)&txdataF[1][tti_offset])[0] = (output[*jj]==0) ? (gain_lin_QPSK) : -gain_lin_QPSK;
	    *jj=*jj+1;
	    ((short*)&txdataF[1][tti_offset])[1] = (output[*jj]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    *jj=*jj+1;
	    
	    break;
	    
	  case 4:  //16QAM
	    
	    // Antenna 0 position n 
	    
	    qam16_table_offset_re = 0;
	    if (output[*jj] == 1)
	      qam16_table_offset_re+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset_re+=1;
	    *jj=*jj+1;
	    
	    
	    qam16_table_offset_im = 0;
	    if (output[*jj] == 1)
	      qam16_table_offset_im+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset_im+=1;
	    *jj=*jj+1;
	    
	    ((short *)&txdataF[0][tti_offset])[0]=(short)(((int)amp*qam16_table[qam16_table_offset_re])>>15);
	    ((short *)&txdataF[0][tti_offset])[1]=(short)(((int)amp*qam16_table[qam16_table_offset_im])>>15);
	    
	    // Antenna 1 position n Real part -> -x1*
	    
	    qam16_table_offset_re = 0;
	    if (output[*jj] == 1)
	      qam16_table_offset_re+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset_re+=1;
	    *jj=*jj+1;
	    
	    
	    qam16_table_offset_im = 0;
	    if (output[*jj] == 1)
	      qam16_table_offset_im+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset_im+=1;
	    *jj=*jj+1;
	    
	    ((short *)&txdataF[1][tti_offset])[0]=-(short)(((int)amp*qam16_table[qam16_table_offset_re])>>15);
	    ((short *)&txdataF[1][tti_offset])[1]=(short)(((int)amp*qam16_table[qam16_table_offset_im])>>15);
	    
	    
	    break;
	  case 6:   // 64-QAM
	    
	    // Antenna 0
	    qam64_table_offset_re = 0;
	    if (output[*jj] == 1)
	      qam64_table_offset_re+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_re+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_re+=1;
	    *jj=*jj+1;
	    
	    
	    qam64_table_offset_im = 0;
	    if (output[*jj] == 1)
	      qam64_table_offset_im+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_im+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_im+=1;
	    *jj=*jj+1;
	    
	    ((short *)&txdataF[0][tti_offset])[0]=(short)(((int)amp*qam64_table[qam64_table_offset_re])>>15);
	    ((short *)&txdataF[0][tti_offset])[1]=(short)(((int)amp*qam64_table[qam64_table_offset_im])>>15);


		    // Antenna 1 => -x1*
	    qam64_table_offset_re = 0;
	    if (output[*jj] == 1)
	      qam64_table_offset_re+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_re+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_re+=1;
	    *jj=*jj+1;
	    
	    
	    qam64_table_offset_im = 0;
	    if (output[*jj] == 1)
	      qam64_table_offset_im+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_im+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_im+=1;
	    *jj=*jj+1;
	    
	    ((short *)&txdataF[1][tti_offset])[0]=-(short)(((int)amp*qam64_table[qam64_table_offset_re])>>15);
	    ((short *)&txdataF[1][tti_offset])[1]=(short)(((int)amp*qam64_table[qam64_table_offset_im])>>15);
		    
	    break;
	  }
	  // fill in the rest of the ALAMOUTI precoding
	  ((short *)&txdataF[0][tti_offset+1])[0] = -((short *)&txdataF[1][tti_offset])[0]; //x1
	  ((short *)&txdataF[0][tti_offset+1])[1] = ((short *)&txdataF[1][tti_offset])[1];
	  ((short *)&txdataF[1][tti_offset+1])[0] = ((short *)&txdataF[0][tti_offset])[0];  //x0*
	  ((short *)&txdataF[1][tti_offset+1])[1] = -((short *)&txdataF[0][tti_offset])[1];
	  
	}
	else if (mimo_mode == ANTCYCLING ) {
	  switch (mod_order) {
	  case 2:  //QPSK
	    
	    ((short*)&txdataF[re&1][tti_offset])[0] = (output[*jj]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    *jj = *jj + 1;
	    ((short*)&txdataF[re&1][tti_offset])[1] = (output[*jj]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    *jj = *jj + 1;
	    break;
	    
	  case 4:  //16QAM
	    
	    qam16_table_offset_re = 0;
	    if (output[*jj] == 1)
	      qam16_table_offset_re+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset_re+=1;
	    *jj=*jj+1;
	    
	    
	    qam16_table_offset_im = 0;
	    if (output[*jj] == 1)
	      qam16_table_offset_im+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset_im+=1;
	    *jj=*jj+1;

	    ((short *)&txdataF[re&1][tti_offset])[0]=(short)(((int)amp*qam16_table[qam16_table_offset_re])>>15);
	    ((short *)&txdataF[re&1][tti_offset])[1]=(short)(((int)amp*qam16_table[qam16_table_offset_im])>>15);
	    
	    
	    break;
	  
	  case 6:  //64QAM

	    qam64_table_offset_re = 0;
	    if (output[*jj] == 1)
	      qam64_table_offset_re+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_re+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_re+=1;
	    *jj=*jj+1;
	    
	    
	    qam64_table_offset_im = 0;
	    if (output[*jj] == 1)
	      qam64_table_offset_im+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_im+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset_im+=1;
	    *jj=*jj+1;
	    
	    ((short *)&txdataF[re&1][tti_offset])[0]=(short)(((int)amp*qam64_table[qam64_table_offset_re])>>15);
	    ((short *)&txdataF[re&1][tti_offset])[1]=(short)(((int)amp*qam64_table[qam64_table_offset_im])>>15);

	  }
	}
		    
	else {
	  msg("allocate_REs_in_RB() [dlsch.c] : ERROR, unknown mimo_mode %d\n",mimo_mode);
	  return(-1);
	}
	
      } 
      
      if (mimo_mode == ALAMOUTI) {
	re++;  // adjacent carriers are taken care of by precoding
	*re_allocated = *re_allocated + 1;
      }
  }
  return(0);
}

#else // ifndef IFFT_FPGA

static mod_sym_t qpsk_precoder[4][4]   = {0,1,2,3,3,2,1,0,1,3,0,2,2,0,3,1};
static mod_sym_t qam16_precoder[4][16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                                10,11,8,9,14,15,12,13,2,3,0,1,6,7,4,5,
                                2,6,10,14,3,7,11,15,0,4,8,12,1,5,9,13,
                                8,12,0,4,9,13,1,5,10,14,2,6,11,15,3,7};
static mod_sym_t qam64_precoder[4][64] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
				36,37,38,39,32,33,34,35,44,45,46,47,40,41,42,43,52,53,54,55,48,49,50,51,60,61,62,63,56,57,58,59,4,5,6,7,0,1,2,3,12,13,14,15,8,9,10,11,20,21,22,23,16,17,18,19,28,29,30,31,24,25,26,27,
				4,12,20,28,36,44,52,60,5,13,21,29,37,45,53,61,6,14,22,30,38,46,54,62,7,15,23,31,39,47,55,63,0,8,16,24,32,40,48,56,1,9,17,25,33,41,49,57,2,10,18,26,34,42,50,58,3,11,19,27,35,43,51,59,
				32,40,48,56,0,8,16,24,33,41,49,57,1,9,17,25,34,42,50,58,2,10,18,26,35,43,51,59,3,11,19,27,36,44,52,60,4,12,20,28,37,45,53,61,5,13,21,29,38,46,54,62,6,14,22,30,39,47,55,63,7,15,23,31};


int allocate_REs_in_RB(mod_sym_t **txdataF,
		       unsigned int *jj,
		       unsigned short re_offset,
		       unsigned int symbol_offset,
		       unsigned char *output,
		       MIMO_mode_t mimo_mode,
		       unsigned char nu,
		       unsigned char pilots,
		       unsigned char first_pilot,
		       unsigned char mod_order,
		       unsigned char precoder_index,
		       short amp,
		       unsigned int *re_allocated,
		       unsigned char skip_dc,
		       LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned int tti_offset;
  unsigned char re;
  unsigned char qam64_table_offset = 0;
  unsigned char qam16_table_offset = 0;
  unsigned char qpsk_table_offset = 0; 
  unsigned char qam64_table_offset2 = 0; // for second symbol if Alamouti is used
  unsigned char qam16_table_offset2 = 0;
  unsigned char qpsk_table_offset2 = 0;
  short re_off=re_offset;
  

  if (nu>1) {
    msg("dlsch_modulation.c: allocate_REs_in_RB, error, unknown layer index %d\n",nu);
    return(-1);
  }

  for (re=0;re<12;re++) {


     if ((skip_dc == 1) && (re==6))
      re_off=re_off - frame_parms->N_RB_DL*12;
   // check that re is not a pilot (need shift for 2nd pilot symbol!!!!)
    // Again this is not LTE, here for SISO only positions 3-5 and 8-11 are allowed for data REs
    // This allows for pilots from adjacent eNbs to be interference free

    tti_offset = symbol_offset + re_off + re;
      
    if (is_not_pilot(pilots,first_pilot,re)) { 

      *re_allocated = *re_allocated + 1;

	if (mimo_mode == SISO) {  //SISO mapping
	  switch (mod_order) {
	  case 2:  //QPSK

	    qpsk_table_offset = 1;
	    if (output[*jj] == 1)
	      qpsk_table_offset+=1;
	    *jj=*jj+1;
	    if (output[*jj] == 1) 
	      qpsk_table_offset+=2;
	    *jj=*jj+1;

	    txdataF[nu][tti_offset] = (mod_sym_t) qpsk_table_offset;

	    break;
	    
	  case 4:  //16QAM

	    qam16_table_offset = 5;
	    if (output[*jj] == 1)
	      qam16_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset+=1;
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1)
	      qam16_table_offset+=8;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset+=4;
	    *jj=*jj+1;
	    
	    txdataF[0][tti_offset] = (mod_sym_t) qam16_table_offset;
	    	    
	    break;
	   
	  case 6:  //64QAM

		    
	    qam64_table_offset = 21;
	    if (output[*jj] == 1)
	      qam64_table_offset+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=1;
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1)
	      qam64_table_offset+=32;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=16;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=8;
	    *jj=*jj+1;
	    
	    txdataF[0][tti_offset] = (mod_sym_t) qam64_table_offset;
	    break;

	  }
	}
	else if (mimo_mode == ALAMOUTI){
	  
	  switch (mod_order) {
	  case 2:  //QPSK

	    
	    qpsk_table_offset = 1;  //x0
	    qpsk_table_offset2 = 1;  //x0*

	    if (output[*jj] == 1) { //real
	      qpsk_table_offset+=1;
	      qpsk_table_offset2+=1;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) //imag
	      qpsk_table_offset+=2;
	    else
	      qpsk_table_offset2+=2;
	    *jj=*jj+1;

	    txdataF[0][tti_offset] = (mod_sym_t) qpsk_table_offset;      // x0
	    txdataF[1][tti_offset+1] = (mod_sym_t) qpsk_table_offset2;   // x0*


	    qpsk_table_offset = 1; //-x1*
	    qpsk_table_offset2 = 1; //x1

	    if (output[*jj] == 1)    // flipping bit for real part of symbol means taking -x1*
	      qpsk_table_offset2+=1;
	    else
	      qpsk_table_offset+=1;
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qpsk_table_offset+=2;
	      qpsk_table_offset2+=2;
	    }
	    *jj=*jj+1;

	    txdataF[1][tti_offset] = (mod_sym_t) qpsk_table_offset;     // -x1*
	    txdataF[0][tti_offset+1] = (mod_sym_t) qpsk_table_offset2;  // x1

	    break;

	  case 4:  //16QAM
	    
	    // Antenna 0 position n 

	    qam16_table_offset = 5; //x0
	    qam16_table_offset2 = 5; //x0*
	    if (output[*jj] == 1) {
	      qam16_table_offset+=2;
	      qam16_table_offset2+=2;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam16_table_offset+=1;
	      qam16_table_offset2+=1;
	    }
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1) 
	      qam16_table_offset+=8;
	    else
	      qam16_table_offset2+=8;
	    *jj=*jj+1;

	    if (output[*jj] == 1){
	      qam16_table_offset+=4;
	      qam16_table_offset2+=4;
	    }
	    *jj=*jj+1;
	    
	    txdataF[0][tti_offset] = (mod_sym_t) qam16_table_offset; //x0
	    txdataF[1][tti_offset+1] = (mod_sym_t) qam16_table_offset2; //x0*


	    qam16_table_offset = 5; //-x1*
	    qam16_table_offset2 = 5; //x1
	    if (output[*jj] == 1)
	      qam16_table_offset2+=2;
	    else
	      qam16_table_offset+=2;
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam16_table_offset2+=1;
	      qam16_table_offset+=1;
	    }
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1) {
	      qam16_table_offset+=8;
	      qam16_table_offset2+=8;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam16_table_offset+=4;
	      qam16_table_offset2+=4;
	    }
	    *jj=*jj+1;
	    
	    txdataF[1][tti_offset] = (mod_sym_t) qam16_table_offset;  //x1*
	    txdataF[0][tti_offset+1] = (mod_sym_t) qam16_table_offset2; //x1

	    break;
	  case 6:   // 64-QAM

	    // Antenna 0
	    qam64_table_offset = 21; //x0
	    qam64_table_offset2 = 21; //x0*
	    if (output[*jj] == 1) {
	      qam64_table_offset+=4;
	      qam64_table_offset2+=4;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=2;
	      qam64_table_offset2+=2;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=1;
	      qam64_table_offset2+=1;
	    }
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1)
	      qam64_table_offset+=32;
	    else
	      qam64_table_offset2+=32;
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=16;
	      qam64_table_offset2+=16;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=8;
	      qam64_table_offset2+=8;
	    }
	    *jj=*jj+1;
	    
	    txdataF[0][tti_offset] = (mod_sym_t) qam64_table_offset; //x0
	    txdataF[1][tti_offset+1] = (mod_sym_t) qam64_table_offset2; //x0*

	    qam64_table_offset = 21; //-x1*
	    qam64_table_offset2 = 21; //x1
	    if (output[*jj] == 1)
	      qam64_table_offset2+=4;
	    else
	      qam64_table_offset+=4;
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=2;
	      qam64_table_offset2+=2;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=1;
	      qam64_table_offset2+=1;
	    }
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1) {
	      qam64_table_offset+=32;
	      qam64_table_offset2+=32;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=16;
	      qam64_table_offset2+=16;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1) {
	      qam64_table_offset+=8;
	      qam64_table_offset2+=8;
	    }
	    *jj=*jj+1;
	    
	    txdataF[1][tti_offset] = (mod_sym_t) qam64_table_offset; //-x1*
	    txdataF[0][tti_offset+1] = (mod_sym_t) qam64_table_offset2; //x1
		    
	    break;
	  }
	}
	/*
	else if (mimo_mode == ANTCYCLING ) {

	}
	else if (mimo_mode == DUALSTREAM) {

	}
	*/
	
	else if ((mimo_mode >= UNIFORM_PRECODING11)&&(mimo_mode <= PUSCH_PRECODING1)) {
	  // this is for transmission modes 4-6 (1 layer)
	  switch (mod_order) {
	  case 2:  //QPSK

	    
	    qpsk_table_offset = 1;  //x0 antenna 0

	    if (output[*jj] == 1) { //real
	      qpsk_table_offset+=1;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1)  {//imag
	      qpsk_table_offset+=2;
	    }

	    *jj=*jj+1;

	    txdataF[0][tti_offset] = (mod_sym_t) qpsk_table_offset;      // x0
	    txdataF[1][tti_offset+1] = (mod_sym_t) (1+qpsk_precoder[precoder_index][qpsk_table_offset-1]);   // x0*


	    break;

	  case 4:

	    qam16_table_offset = 5;
	    if (output[*jj] == 1)
	      qam16_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset+=1;
	    *jj=*jj+1;
	    

	    if (output[*jj] == 1)
	      qam16_table_offset+=8;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset+=4;
	    *jj=*jj+1;

	    txdataF[0][tti_offset] = (mod_sym_t) qam16_table_offset;
	    txdataF[1][tti_offset] = (mod_sym_t) (5+qam16_precoder[precoder_index][qam16_table_offset-5]);

	    break;

	  case 6:  //64QAM

		    
	    qam64_table_offset = 21;
	    if (output[*jj] == 1)
	      qam64_table_offset+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=1;
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1)
	      qam64_table_offset+=32;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=16;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam64_table_offset+=8;
	    *jj=*jj+1;
	    
	    txdataF[0][tti_offset] = (mod_sym_t) qam64_table_offset;
	    txdataF[1][tti_offset] = (mod_sym_t) (21+qam64_precoder[precoder_index][qam64_table_offset-21]);
	    break;

	  }
	}
	
	else {
	  msg("allocate_REs_in_RB() [dlsch.c] : ERROR, unknown mimo_mode %d\n",mimo_mode);
	  return(-1);
	}

    }
    else {

    }
    if (mimo_mode == ALAMOUTI) {
      re++;  // adjacent carriers are taken care of by precoding
      *re_allocated = *re_allocated + 1;
    }
  }
  return(0);
}
#endif

unsigned char get_pmi_5MHz(MIMO_mode_t mode,unsigned int pmi_alloc,unsigned short rb) {
  if (mode <= PUSCH_PRECODING1)
    return((pmi_alloc>>(rb>>1))&3);
  else
    return((pmi_alloc>>(rb>>2))&1);
}

int dlsch_modulation(mod_sym_t **txdataF,
		     short amp,
		     unsigned int sub_frame_offset,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     LTE_eNb_DLSCH_t *dlsch){

  unsigned char nsymb;
  unsigned char harq_pid = dlsch->current_harq_pid;
  unsigned int jj,re_allocated,symbol_offset;
  unsigned short l,rb,re_offset;
  unsigned int rb_alloc_ind;
  unsigned int *rb_alloc = dlsch->rb_alloc;
  unsigned char pilots,first_pilot,second_pilot;
  unsigned char skip_dc;
  unsigned char mod_order = get_Qm(dlsch->harq_processes[harq_pid]->mcs);

  nsymb = (frame_parms->Ncp==0) ? 14:12;
  second_pilot = (frame_parms->Ncp==0) ? 4 : 3;
  
  //Modulation mapping (difference w.r.t. LTE specs)
  
  jj=0;
  re_allocated=0;
  
  for (l=frame_parms->first_dlsch_symbol;l<nsymb;l++) {

#ifdef DEBUG_DLSCH_MODULATION
    printf("Generating DLSCH (harq_pid %d,mimo %d, mod %d, nu %d, rb_alloc[0] %d) in %d\n",harq_pid,dlsch->harq_processes[harq_pid]->mimo_mode,mod_order, dlsch->layer_index, rb_alloc[0], l);
#endif    
    pilots=0;
    first_pilot=0;
    if ((l==(nsymb>>1))){
      pilots=1;
      first_pilot=1;
    }

    if ((l==second_pilot)||(l==(second_pilot+(nsymb>>1)))) {
      pilots=1;
      first_pilot=0;
    }

    if (pilots==0) { // don't skip pilot symbols
      // This is not LTE, it guarantees that
      // pilots from adjacent base-stations
      // do not interfere with data
      // LTE is eNb centric.  "Smart" Interference
      // cancellation isn't possible

#ifdef IFFT_FPGA
      re_offset = frame_parms->N_RB_DL*12/2;
      symbol_offset = (unsigned int)frame_parms->N_RB_DL*12*(l+(sub_frame_offset*nsymb));
#else
      re_offset = frame_parms->first_carrier_offset;
      symbol_offset = (unsigned int)frame_parms->ofdm_symbol_size*(l+(sub_frame_offset*nsymb));
#endif

      //      printf("symbol_offset %d,subframe offset %d\n",symbol_offset,sub_frame_offset);
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[3]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;

	if ((rb == frame_parms->N_RB_DL>>1) && ((frame_parms->N_RB_DL&1)>0))
	  skip_dc = 1;
	else
	  skip_dc = 0;

	if (dlsch->layer_index>1) {
	  msg("layer_index %d: re_offset %d, symbol %d offset %d\n",dlsch->layer_index,re_offset,l,symbol_offset); 
	  return(-1);
	}
	if (rb_alloc_ind > 0)
	  allocate_REs_in_RB(txdataF,
			     &jj,
			     re_offset,
			     symbol_offset,
			     dlsch->e,
			     dlsch->harq_processes[harq_pid]->mimo_mode,
			     dlsch->layer_index,
			     pilots,
			     first_pilot,
			     mod_order,
			     get_pmi_5MHz(dlsch->harq_processes[harq_pid]->mimo_mode,dlsch->pmi_alloc,rb),
			     amp,
			     &re_allocated,
			     skip_dc,
			     frame_parms);

	re_offset+=12; // go to next RB
	

	// check if we crossed the symbol boundary and skip DC
#ifdef IFFT_FPGA
	if (re_offset >= frame_parms->N_RB_DL*12) {
	  if (skip_dc == 0)  //even number of RBs (doesn't straddle DC)
	    re_offset=0;
	  else
	    re_offset=6;  // odd number of RBs
	}

#else
	if (re_offset >= frame_parms->ofdm_symbol_size) {
	  if (skip_dc == 0)  //even number of RBs (doesn't straddle DC)
	    re_offset=1;
	  else
	    re_offset=7;  // odd number of RBs
	}
#endif
      }
	
    }
  }


#ifdef DEBUG_DLSCH_MODULATION
  printf("generate_dlsch : jj = %d,re_allocated = %d\n",jj,re_allocated);
#endif

  return (re_allocated);
}

