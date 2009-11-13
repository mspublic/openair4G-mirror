#include <string.h>
#include "PHY/defs.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "PHY/LTE_TRANSPORT/defs.h"
#include "defs.h"

/*
#define is_not_pilot(pilots,first_pilot,re) (pilots==0) || \ 
	((pilots==1)&&(first_pilot==1)&&(((re>2)&&(re<6))||((re>8)&&(re<12)))) || \
	((pilots==1)&&(first_pilot==0)&&(((re<3))||((re>5)&&(re<9)))) \
*/
#define is_not_pilot(pilots,first_pilot,re) (1)


static int qam64_table[8],qam16_table[4];

void generate_64qam_table() {

  int a,b,c,index;

  printf("QAM64_n1 %d\n",QAM64_n1);
  for (a=-1;a<=1;a+=2) 
    for (b=-1;b<=1;b+=2) 
      for (c=-1;c<=1;c+=2) {
	index = (1+a)*2 + (1+b) + (1+c)/2;  
	qam64_table[index] = a*(QAM64_n1 + b*(QAM64_n2 + (c*QAM64_n3))); // 0 1 2
	
	printf("QAM64 i %d : %d\n",index,qam64_table[index]);
      } 
}

void generate_16qam_table() {

  int a,b,index;

  for (a=-1;a<=1;a+=2) 
    for (b=-1;b<=1;b+=2) {
	index = (1+a) + (1+b)/2;  
	qam16_table[index] = a*(QAM16_n1 + (b*QAM16_n2)); 
      } 
}

#ifndef IFFT_FPGA
 
void allocate_REs_in_RB(int **txdataF,
			unsigned int *jj,
			unsigned short re_offset,
			unsigned int symbol_offset,
			unsigned char *output,
			MIMO_mode_t mimo_mode,
			unsigned char pilots,
			unsigned char first_pilot,
			unsigned char mod_order,
			short amp,
			unsigned int *re_allocated,
			unsigned char skip_dc,
			LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned int tti_offset;
  unsigned char re;
  unsigned char qam64_table_offset_re = 0;
  unsigned char qam64_table_offset_im = 0;
  unsigned char qam16_table_offset_re = 0;
  unsigned char qam16_table_offset_im = 0;
  short gain_lin_QPSK,gain_lin_16QAM1,gain_lin_16QAM2;
  short re_off=re_offset;
  gain_lin_QPSK = (short)((amp*ONE_OVER_SQRT2_Q15)>>15);  
  //  printf("DLSCH: gain_lin_QPSK = %d\n",gain_lin_QPSK);
  
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
  printf("allocate_re : re_offset %d (%d), jj %d -> %d,%d\n",re_offset,skip_dc,*jj, output[*jj], output[1+*jj]);
  for (re=0;re<12;re++) {


    if ((skip_dc == 1) && (re==6))
      re_off=re_off - frame_parms->ofdm_symbol_size+1;
    // check that re is not a pilot (need shift for 2nd pilot symbol!!!!)
    // Again this is not LTE, here for SISO only positions 3-5 and 8-11 are allowed for data REs
    // This allows for pilots from adjacent eNbs to be interference free

    tti_offset = symbol_offset + re_off + re;
    if (is_not_pilot(pilots,first_pilot,re)) { 
      //    printf("re %d, jj %d\n",re,*jj);     
      *re_allocated = *re_allocated + 1;
      //      printf("Symbol %d, Re %d\n",symbol_offset/512,re);
      //	    printf("symbol = %d, tti_offset = %d\n",l,tti_offset);
	//	      printf("TTI %d\n",tti);


	if (mimo_mode == SISO) {  //SISO mapping
	  switch (mod_order) {
	  case 2:  //QPSK

	    ((short*)&txdataF[0][tti_offset])[0] = (output[*jj]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	      *jj = *jj + 1;
	    ((short*)&txdataF[0][tti_offset])[1] = (output[*jj]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
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
	    
	    ((short *)&txdataF[0][tti_offset])[0]=(short)(((int)amp*qam16_table[qam16_table_offset_re])>>15);
	    ((short *)&txdataF[0][tti_offset])[1]=(short)(((int)amp*qam16_table[qam16_table_offset_im])>>15);
	    
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
	    
	    ((short *)&txdataF[0][tti_offset])[0]=(short)(((int)amp*qam64_table[qam64_table_offset_re])>>15);
	    ((short *)&txdataF[0][tti_offset])[1]=(short)(((int)amp*qam64_table[qam64_table_offset_im])>>15);
	    break;

	  }
	}
	else if (mimo_mode == ALAMOUTI){
	  
	  switch (mod_order) {
	  case 2:  //QPSK
	    //		  printf("jj %d,output[%d] %x\n",jj,jj,output[jj]);

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
	  ((short *)&txdataF[0][tti_offset+1])[0] = -((short *)&txdataF[1][tti_offset])[0];
	  ((short *)&txdataF[0][tti_offset+1])[1] = ((short *)&txdataF[1][tti_offset])[1];
	  ((short *)&txdataF[1][tti_offset+1])[0] = ((short *)&txdataF[0][tti_offset])[0];
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
	else if (mimo_mode == DUALSTREAM) {

	}
	else {
	  msg("allocate_REs_in_RB() [dlsch.c] : ERROR, unknown mimo_mode %d\n",mimo_mode);
	  exit(-1);
	}

    }
    else {
      /*
      printf("pilot in symbol_offset %d, re_offset %d, re %d (%d,%d)\n",symbol_offset,re_offset,re,
	     ((short*)&txdataF[0][tti_offset])[0],
	     ((short*)&txdataF[0][tti_offset])[1]);
      */
    }
    if (mimo_mode == ALAMOUTI) {
      re++;  // adjacent carriers are taken care of by precoding
      *re_allocated = *re_allocated + 1;
    }
  }
}

#else
void allocate_REs_in_RB(mod_sym_t **txdataF,
			unsigned int *jj,
			unsigned short re_offset,
			unsigned int symbol_offset,
			unsigned char *output,
			MIMO_mode_t mimo_mode,
			unsigned char pilots,
			unsigned char first_pilot,
			unsigned char mod_order,
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
  

  printf("allocate_re : re_offset %d (%d), jj %d -> %d,%d\n",re_offset,skip_dc,*jj, output[*jj], output[1+*jj]);
  for (re=0;re<12;re++) {


    // check that re is not a pilot (need shift for 2nd pilot symbol!!!!)
    // Again this is not LTE, here for SISO only positions 3-5 and 8-11 are allowed for data REs
    // This allows for pilots from adjacent eNbs to be interference free

    tti_offset = symbol_offset + re_off + re;
    if (is_not_pilot(pilots,first_pilot,re)) { 
      //    printf("re %d, jj %d\n",re,*jj);     
      *re_allocated = *re_allocated + 1;
      //      printf("Symbol %d, Re %d\n",symbol_offset/512,re);
      //	    printf("symbol = %d, tti_offset = %d\n",l,tti_offset);
	//	      printf("TTI %d\n",tti);


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

	    txdataF[0][tti_offset] = (mod_sym_t) qpsk_table_offset;

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
	    //		  printf("jj %d,output[%d] %x\n",jj,jj,output[jj]);

	    // first antenna position (n,n+1) -> x0,-x1 
	    
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

	    txdataF[0][tti_offset] = (mod_sym_t) qpsk_table_offset;
	    txdataF[1][tti_offset+1] = (mod_sym_t) qpsk_table_offset2;

	    // second antenna position (n,n+1) -> -x1*,x0* 

	    qpsk_table_offset = 1; //-x1*
	    qpsk_table_offset2 = 1; //-x1

	    if (output[*jj] == 1) {   // flipping bit for real part of symbol means taking -x1*
	      qpsk_table_offset2+=1;
	      qpsk_table_offset+=1;
	    }
	    *jj=*jj+1;

	    if (output[*jj] == 1)
	      qpsk_table_offset+=2;
	    else
	      qpsk_table_offset2+=2;
	    *jj=*jj+1;

	    txdataF[1][tti_offset] = (mod_sym_t) qpsk_table_offset;
	    txdataF[0][tti_offset+1] = (mod_sym_t) qpsk_table_offset2;

	    break;

	  case 4:  //16QAM
	    
	    // Antenna 0 position n 

	    qam16_table_offset = 5;
	    qam16_table_offset2 = 5;
	    if (output[*jj] == 1)
	      qam16_table_offset+=2;
	    else
	      qam16_table_offset2+=2;

	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset+=1;
	    else
	      qam16_table_offset2+=1;
	    *jj=*jj+1;
	    
	    
	    if (output[*jj] == 1) {
	      qam16_table_offset+=8;
	      qam16_table_offset2+=8;
	    }
	    *jj=*jj+1;
	    if (output[*jj] == 1){
	      qam16_table_offset+=4;
	      qam16_table_offset2+=4;
	    }
	    *jj=*jj+1;
	    
	    txdataF[0][tti_offset] = (mod_sym_t) qam16_table_offset;
	    txdataF[0][tti_offset+1] = (mod_sym_t) qam16_table_offset2;

	    // Antenna 1 position n Real part -> -x1*

	    qam16_table_offset = 5;
	    qam16_table_offset2 = 5;
	    if (output[*jj] == 1)
	      qam16_table_offset2+=2;
	    else
	      qam16_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 1)
	      qam16_table_offset2+=1;
	    else
	      qam16_table_offset+=1;
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
	    
	    txdataF[1][tti_offset] = (mod_sym_t) qam16_table_offset;
	    txdataF[1][tti_offset+1] = (mod_sym_t) qam16_table_offset2;

	    break;
	  case 6:   // 64-QAM

	    // Antenna 0
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

	    // Antenna 1 => -x1*
	    qam64_table_offset = 21;
	    if (output[*jj] == 0)
	      qam64_table_offset+=4;
	    *jj=*jj+1;
	    if (output[*jj] == 0)
	      qam64_table_offset+=2;
	    *jj=*jj+1;
	    if (output[*jj] == 0)
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
	    
	    txdataF[1][tti_offset] = (mod_sym_t) qam64_table_offset;
		    
	    break;
	  }
	  // fill in the rest of the ALAMOUTI precoding
	  ((short *)&txdataF[0][tti_offset+1])[0] = -((short *)&txdataF[1][tti_offset])[0]; //-real(0)
	  ((short *)&txdataF[0][tti_offset+1])[1] = ((short *)&txdataF[1][tti_offset])[1];  //imag(0)
	  ((short *)&txdataF[1][tti_offset+1])[0] = ((short *)&txdataF[0][tti_offset])[0];  //real(1)
	  ((short *)&txdataF[1][tti_offset+1])[1] = -((short *)&txdataF[0][tti_offset])[1]; //-imag(1)

	}
	/*
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
	*/
	else if (mimo_mode == DUALSTREAM) {

	}
	else {
	  msg("allocate_REs_in_RB() [dlsch.c] : ERROR, unknown mimo_mode %d\n",mimo_mode);
	  exit(-1);
	}

    }
    else {
      /*
      printf("pilot in symbol_offset %d, re_offset %d, re %d (%d,%d)\n",symbol_offset,re_offset,re,
	     ((short*)&txdataF[0][tti_offset])[0],
	     ((short*)&txdataF[0][tti_offset])[1]);
      */
    }
    if (mimo_mode == ALAMOUTI) {
      re++;  // adjacent carriers are taken care of by precoding
      *re_allocated = *re_allocated + 1;
    }
  }
}
#endif

void dlsch_modulation(int **txdataF,
		      short amp,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_eNb_DLSCH_t *dlsch,
		      unsigned char harq_pid,
		      unsigned int  *rb_alloc){
  
  unsigned int i,j,j2;
  unsigned char nsymb;
  unsigned int jj,re_allocated;
  unsigned short l,rb,re_offset;
  unsigned int rb_alloc_ind;
  unsigned char pilots,pilot_pos,first_pilot,second_pilot;
  unsigned char skip_dc;
  unsigned char mod_order = dlsch->harq_processes[harq_pid]->mod_order;

  nsymb = (frame_parms->Ncp==0) ? 14:12;
  second_pilot = (frame_parms->Ncp==0) ? 4 : 3;
  
  //Modulation mapping (difference w.r.t. LTE specs)
  
  jj=0;
  re_allocated=0;
  
  for (l=frame_parms->first_dlsch_symbol;l<nsymb;l++) {
    
    pilots=0;
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
      printf("Generating DLSCH in %d\n",l);
      re_offset = frame_parms->first_carrier_offset;
      
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;

	//	printf("rb %d, rb_alloc_ind %d\n",rb,rb_alloc_ind);
	if ((rb == frame_parms->N_RB_DL>>1) && ((frame_parms->N_RB_DL&1)>0))
	  skip_dc = 1;
	else
	  skip_dc = 0;
	
	if (rb_alloc_ind > 0)
	  allocate_REs_in_RB(txdataF,
			     &jj,
			     re_offset,
			     frame_parms->ofdm_symbol_size*l,
			     dlsch->e[0],
			     dlsch->harq_processes[harq_pid]->mimo_mode,
			     pilots,
			     first_pilot,
			     mod_order,
			     amp,
			     &re_allocated,
			     skip_dc,
			     frame_parms);

	re_offset+=12; // go to next RB
	
	// check if we crossed the symbol boundary and skip DC
	if (re_offset >= frame_parms->ofdm_symbol_size) {
	  if (skip_dc == 0)  //even number of RBs (doesn't straddle DC)
	    re_offset=1;
	  else
	    re_offset=7;  // odd number of RBs
	}
      }
	
    }
  }
  printf("generate_dlsch : jj = %d,re_allocated = %d\n",jj,re_allocated);
}

