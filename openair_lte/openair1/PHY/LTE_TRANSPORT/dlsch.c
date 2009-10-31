#include <string.h>
#include "PHY/defs.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"

#include "defs.h"

#define is_not_pilot(pilots,first_pilot,re) (pilots==0) || \ 
	((pilots==1)&&(first_pilot==1)&&(((re>2)&&(re<6))||((re>8)&&(re<12)))) || \
	((pilots==1)&&(first_pilot==0)&&(((re<3))||((re>5)&&(re<9)))) \

static short qam64_table[6][8];

void generate_64qam_table() {

  int a,b,c,index;


  for (a=-1;a<=1;a+=2) 
    for (b=-1;b<=1;b+=2) 
      for (c=-1;c<=1;c+=2) {
	index = (1+a)*2 + (1+b) + (1+c)/2;  
	qam64_table[0][index] = a*(QAM64_n1 + b*(QAM64_n2 + (c*QAM64_n3))); // 0 1 2
	
	qam64_table[1][index] = b*(QAM64_n1 + a*(QAM64_n2 + (c*QAM64_n3))); // 1 0 2
	
	qam64_table[2][index] = b*(QAM64_n1 + c*(QAM64_n2 + (a*QAM64_n3))); // 1 2 0
	
	qam64_table[3][index] = c*(QAM64_n1 + a*(QAM64_n2 + (b*QAM64_n3))); // 2 0 1
	
	qam64_table[4][index] = c*(QAM64_n1 + b*(QAM64_n2 + (a*QAM64_n3))); // 2 1 0
	
	qam64_table[5][index] = a*(QAM64_n1 + c*(QAM64_n2 + (b*QAM64_n3))); // 0 2 1
	//	printf("QAM64 i %d : %d\n",index,qam64_table[0][index]);
      } 
}

 
void allocate_REs_in_RB(int **txdataF,
			unsigned int *jj,
			unsigned short re_offset,
			unsigned int tti_size,
			unsigned int symbol_offset,
			unsigned char *output,
			MIMO_mode_t mimo_mode,
			unsigned char pilots,
			unsigned char first_pilot,
			unsigned short Ntti,
			unsigned char mod_order,
			short amp) {

  unsigned int tti_offset;
  unsigned char re;
  unsigned short tti;
  unsigned char qam64_table_offset_re = 0;
  unsigned char qam64_table_offset_im = 0;
  short gain_lin_QPSK,gain_lin_16QAM1,gain_lin_16QAM2;

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
  
  for (re=0;re<12;re++) {
	    
    tti_offset = symbol_offset + re_offset + re;
    
    // check that re is not a pilot (need shift for 2nd pilot symbol!!!!)
    // Again this is not LTE, here for SISO only positions 3-5 and 8-11 are allowed for data REs
    // This allows for pilots from adjacent eNbs to be interference free

    if (is_not_pilot(pilots,first_pilot,re)) {
      
      
      //	    printf("symbol = %d, tti_offset = %d\n",l,tti_offset);
      // This is TTI interleaving
      // NOT LTE, only for Satellite Performance Evaluation with long time interleaving
      // LTE would be Ntti=1
      for (tti=0;tti<Ntti;tti++) {
	
	//	      printf("TTI %d\n",tti);
	//skip over punctured bits
	while ((output[*jj] & 0x80) == 0) {
	  *jj = *jj+1;
	}
	
	if (mimo_mode == SISO) {  //SISO mapping
	  switch (mod_order) {
	  case 2:  //QPSK
	    //	    printf("jj %d,output[%d] %x\n",jj,jj,output[jj]);
	    ((short*)&txdataF[0][tti_offset])[0] = ((output[*jj]&0xbf)==0x80) ? (-gain_lin_QPSK) : gain_lin_QPSK;

	    if ((output[*jj] & 0x40) != 0)  // bit is to be repeated
	      output[*jj] &= 0xbf;
	    else 
	      *jj = *jj + 1;

	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj = *jj + 1;
	    
	    
	    ((short*)&txdataF[0][tti_offset])[1] = ((output[*jj]&0xbf)==0x80) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    if ((output[*jj] & 0x40) != 0)  // bit is to be repeated
	      output[*jj++] &= 0xbf;
	    else 
	      *jj = *jj + 1;
	    //skip punctured bits
	    while ((output[*jj] & 0x80) == 0) {
		*jj=*jj+1;
	    }
	    break;
	    
	  case 4:  //16QAM
	    
	    // Real part
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[0][tti_offset])[0] =  
		  -gain_lin_16QAM1 + (gain_lin_16QAM2);
	      else
		((short*)&txdataF[0][tti_offset])[0] = 
		  gain_lin_16QAM1 + gain_lin_16QAM2;
	      output[*jj] &= 0xbf;
	    }
	    else {
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[0][tti_offset])[0] = 
		  (-gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
	      else
		((short*)&txdataF[0][tti_offset])[0] = 
		  (gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (-gain_lin_16QAM2) : (gain_lin_16QAM2));
	      *jj=*jj+2;
	    }
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    // Imaginary part
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[0][tti_offset])[1] = 
		  (-gain_lin_16QAM1) + gain_lin_16QAM2;
	      else
		((short*)&txdataF[0][tti_offset])[1] = 
		  (gain_lin_16QAM1) + (gain_lin_16QAM2);
	      output[*jj] &= 0xbf;    
	    }
	    else {
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[0][tti_offset])[1] = 
		  (-gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
	      else
		((short*)&txdataF[0][tti_offset])[1] = 
		  (gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (-gain_lin_16QAM2) : (gain_lin_16QAM2));
	      
	      *jj=*jj+2;
	    }
	    
	    
	    
	    break;
	   
	  case 6:  //64QAM

		    
	    qam64_table_offset_re = 0;
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_re+=4;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_re+=2;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_re+=1;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    
	    qam64_table_offset_im = 0;
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_im+=4;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_im+=2;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_im+=1;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    
	    ((short *)&txdataF[0][tti_offset])[0]=(short)((amp*qam64_table[0][qam64_table_offset_re])>>14);
	    ((short *)&txdataF[0][tti_offset])[1]=(short)((amp*qam64_table[0][qam64_table_offset_im])>>14);


	    // Antenna 1 => -x1*
	    qam64_table_offset_re = 0;
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_re+=4;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_re+=2;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_re+=1;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    
	    qam64_table_offset_im = 0;
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_im+=4;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_im+=2;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    
	    if ((output[*jj]&0xbf) == 0x81)
	      qam64_table_offset_im+=1;
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      output[*jj] &= 0xbf;
	    }
	    else
	      *jj=*jj+1;
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    
	    ((short *)&txdataF[1][tti_offset])[0]=-(short)((amp*qam64_table[0][qam64_table_offset_re])>>14);
	    ((short *)&txdataF[1][tti_offset])[1]=(short)((amp*qam64_table[0][qam64_table_offset_im])>>14);
	    
	    
		    break;
	    
	  }
	}
	else if (mimo_mode == ALAMOUTI){
	  
	  switch (mod_order) {
	  case 2:  //QPSK
	    //		  printf("jj %d,output[%d] %x\n",jj,jj,output[jj]);

	    // first antenna position n -> x0
	    
	    ((short*)&txdataF[0][tti_offset])[0] = ((output[*jj]&0xbf)==0x80) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    if ((output[*jj] & 0x40) != 0)  // bit is to be repeated
	      output[*jj] &= 0xbf;
	    else
	      *jj = *jj + 1;

	    while ((output[*jj] & 0x80) == 0) {
	      *jj = *jj + 1;
	    }
	    
	    ((short*)&txdataF[0][tti_offset])[1] = ((output[*jj]&0xbf)==0x80) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    if ((output[*jj] & 0x40) != 0)  // bit is to be repeated
	      output[*jj++] &= 0xbf;
	    else
	      *jj = *jj + 1;

	    while ((output[*jj] & 0x80) == 0) {
	      *jj=*jj+1;
	    }

	    // second antenna position n -> -x1*

	    ((short*)&txdataF[1][tti_offset])[0] = ((output[*jj]&0xbf)==0x80) ? (gain_lin_QPSK) : -gain_lin_QPSK;
	    if ((output[*jj] & 0x40) != 0)  // bit is to be repeated
	      output[*jj] &= 0xbf;
	    else
	      *jj = *jj + 1;

	    while ((output[*jj] & 0x80) == 0) {
	      *jj = *jj + 1;
	    }

	    //
	    ((short*)&txdataF[1][tti_offset])[1] = ((output[*jj]&0xbf)==0x80) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    if ((output[*jj] & 0x40) != 0)  // bit is to be repeated
	      output[*jj++] &= 0xbf;
	    else
	      *jj = *jj + 1;

	    while ((output[*jj] & 0x80) == 0) {
	      *jj=*jj+1;
	    }

	    break;

	  case 4:  //16QAM
	    
	    // Antenna 0 position n Real part
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[0][tti_offset])[0] =  
		  -gain_lin_16QAM1 + (gain_lin_16QAM2);
	      else
		((short*)&txdataF[0][tti_offset])[0] = 
		  gain_lin_16QAM1 + gain_lin_16QAM2;
	      output[*jj] &= 0xbf;
	    }
	    else {
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[0][tti_offset])[0] = 
		  (-gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
	      else
		((short*)&txdataF[0][tti_offset])[0] = 
		  (gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (-gain_lin_16QAM2) : (gain_lin_16QAM2));
	      *jj=*jj+2;
	    }
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    // Imaginary part
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[0][tti_offset])[1] = 
		  (-gain_lin_16QAM1) + gain_lin_16QAM2;
	      else
		((short*)&txdataF[0][tti_offset])[1] = 
		  (gain_lin_16QAM1) + (gain_lin_16QAM2);
	      output[*jj] &= 0xbf;    
	    }
	    else {
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[0][tti_offset])[1] = 
		  (-gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
	      else
		((short*)&txdataF[0][tti_offset])[1] = 
		  (gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (-gain_lin_16QAM2) : (gain_lin_16QAM2));
	      
	      *jj=*jj+2;
	    }

	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;

	    // Antenna 1 position n Real part -> -x1*
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[1][tti_offset])[0] =  
		  gain_lin_16QAM1 - (gain_lin_16QAM2);
	      else
		((short*)&txdataF[1][tti_offset])[0] = 
		  -gain_lin_16QAM1 - gain_lin_16QAM2;
	      output[*jj] &= 0xbf;
	    }
	    else {
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[1][tti_offset])[0] = 
		  (gain_lin_16QAM1) - (((output[*jj+1]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
	      else
		((short*)&txdataF[1][tti_offset])[0] = 
		  -(gain_lin_16QAM1) - (((output[*jj+1]&0xbf)==0x80) ? (-gain_lin_16QAM2) : (gain_lin_16QAM2));
	      *jj=*jj+2;
	    }
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    // Imaginary part
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[1][tti_offset])[1] = 
		  (-gain_lin_16QAM1) + gain_lin_16QAM2;
	      else
		((short*)&txdataF[1][tti_offset])[1] = 
		  (gain_lin_16QAM1) + (gain_lin_16QAM2);
	      output[*jj] &= 0xbf;    
	    }
	    else {
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[1][tti_offset])[1] = 
		  (-gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
	      else
		((short*)&txdataF[1][tti_offset])[1] = 
		  (gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (-gain_lin_16QAM2) : (gain_lin_16QAM2));
	      
	      *jj=*jj+2;
	    }
	    
	    
	    
	    break;
	  case 6:   // 64-QAM

	    // Antenna 0
		    qam64_table_offset_re = 0;
		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_re+=4;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;

		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_re+=2;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_re+=1;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    qam64_table_offset_im = 0;
		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_im+=4;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;

		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_im+=2;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_im+=1;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    ((short *)&txdataF[0][tti_offset])[0]=(short)((amp*qam64_table[0][qam64_table_offset_re])>>14);
		    ((short *)&txdataF[0][tti_offset])[1]=(short)((amp*qam64_table[0][qam64_table_offset_im])>>14);


		    // Antenna 1 => -x1*
		    
		    qam64_table_offset_re = 0;
		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_re+=4;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;

		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_re+=2;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_re+=1;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    qam64_table_offset_im = 0;
		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_im+=4;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;

		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_im+=2;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_im+=1;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    ((short *)&txdataF[1][tti_offset])[0]=-(short)((amp*qam64_table[0][qam64_table_offset_re])>>14);
		    ((short *)&txdataF[1][tti_offset])[1]=(short)((amp*qam64_table[0][qam64_table_offset_im])>>14);
		    
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
	    //	    printf("re %d, a %d, jj %d,output[%d] %x\n",re,re&1,*jj,*jj,output[*jj]);
	    ((short*)&txdataF[re&1][tti_offset])[0] = ((output[*jj]&0xbf)==0x80) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    if ((output[*jj] & 0x40) != 0)  // bit is to be repeated
	      output[*jj] &= 0xbf;
	    else // skip punctured bit
	      *jj = *jj + 1;

	    while ((output[*jj] & 0x80) == 0) {
	      *jj = *jj + 1;
	    }
	    
	    ((short*)&txdataF[re&1][tti_offset])[1] = ((output[*jj]&0xbf)==0x80) ? (-gain_lin_QPSK) : gain_lin_QPSK;
	    if ((output[*jj] & 0x40) != 0)  // bit is to be repeated
	      output[*jj++] &= 0xbf;
	    else //skip punctured bits
	      *jj=*jj+1;	     
	     
	    while ((output[*jj] & 0x80) == 0) {
	      *jj=*jj+1;
	    }
	    break;
	    
	  case 4:  //16QAM
	    
	    // Real part
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[re&1][tti_offset])[0] =  
		  -gain_lin_16QAM1 + (gain_lin_16QAM2);
	      else
		((short*)&txdataF[re&1][tti_offset])[0] = 
		  gain_lin_16QAM1 + gain_lin_16QAM2;
	      output[*jj] &= 0xbf;
	    }
	    else {
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[re&1][tti_offset])[0] = 
		  (-gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
	      else
		((short*)&txdataF[re&1][tti_offset])[0] = 
		  (gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (-gain_lin_16QAM2) : (gain_lin_16QAM2));
	      *jj=*jj+2;
	    }
	    
	    // skip punctured bits
	    while ((output[*jj] & 0x80) == 0)
	      *jj=*jj+1;
	    
	    // Imaginary part
	    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[re&1][tti_offset])[1] = 
		  (-gain_lin_16QAM1) + gain_lin_16QAM2;
	      else
		((short*)&txdataF[re&1][tti_offset])[1] = 
		  (gain_lin_16QAM1) + (gain_lin_16QAM2);
	      output[*jj] &= 0xbf;    
	    }
	    else {
	      if (((output[*jj]&0xbf)==0x80))
		((short*)&txdataF[re&1][tti_offset])[1] = 
		  (-gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
	      else
		((short*)&txdataF[re&1][tti_offset])[1] = 
		  (gain_lin_16QAM1) + (((output[*jj+1]&0xbf)==0x80) ? (-gain_lin_16QAM2) : (gain_lin_16QAM2));
	      
	      *jj=*jj+2;
	    }
	    
	    
	    
	    break;

	      case 6:  //64QAM
		    qam64_table_offset_re = 0;
		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_re+=4;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;

		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_re+=2;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_re+=1;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    qam64_table_offset_im = 0;
		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_im+=4;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;

		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_im+=2;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    if ((output[*jj]&0xbf) == 0x81)
		      qam64_table_offset_im+=1;
		    if ((output[*jj] & 0x40) !=0) { // bit is to be repeated
		      output[*jj] &= 0xbf;
		    }
		    else
		      *jj=*jj+1;

		    // skip punctured bits
		    while ((output[*jj] & 0x80) == 0)
		      *jj=*jj+1;


		    ((short *)&txdataF[0][tti_offset])[0]=(short)((amp*qam64_table[0][qam64_table_offset_re])>>14);
		    ((short *)&txdataF[0][tti_offset])[1]=(short)((amp*qam64_table[0][qam64_table_offset_im])>>14);


	  }
	}
	else if (mimo_mode == DUALSTREAM) {

	}
	else {
	  msg("allocate_REs_in_RB() [dlsch.c] : ERROR, unknown mimo_mode %d\n",mimo_mode);
	  exit(-1);
	}

	tti_offset += tti_size;  // go to next TTI
      }
      //	    printf("re = %d\n",re_offset);
    }
    else {
      /*
      printf("pilot in symbol_offset %d, re_offset %d, re %d (%d,%d)\n",symbol_offset,re_offset,re,
	     ((short*)&txdataF[0][tti_offset])[0],
	     ((short*)&txdataF[0][tti_offset])[1]);
      */
    }
    if (mimo_mode == ALAMOUTI)
      re++;  // adjacent carriers are taken care of by precoding
  }
}

void generate_dlsch(int **txdataF,
		    short amp,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    unsigned short Ntti,
		    unsigned short N_RB,
		    unsigned int  *rb_alloc,
		    unsigned char slot_alloc,
		    unsigned char *input_data,
		    unsigned int Nbytes,
		    unsigned int Ncwords,
		    unsigned char mod_order,
		    MIMO_mode_t mimo_mode,
		    unsigned char crc_len) {
  
  unsigned short i,j;
  unsigned short bytes_per_codeword=0,offset;
  unsigned int coded_bits_per_codeword;
  unsigned int crc=1;
  unsigned char input[6144];
  unsigned char output[((6144*3*8)+12)*Ncwords)];
  unsigned int output_offset=0;
  unsigned short iind;
  unsigned char nsymb,Nsymb;
  unsigned short symbol_offset;
  unsigned int jj;
  unsigned short l,rb,re_offset;
  unsigned int rb_alloc_ind;
  unsigned char pilots,pilot_pos,first_pilot,second_pilot;

  if (bytes_per_codeword<=64)
    iind = (bytes_per_codeword-5);
  else if (bytes_per_codeword <=128)
    iind = 59 + ((bytes_per_codeword-64)>>1);
  else if (bytes_per_codeword <= 256)
    iind = 91 + ((bytes_per_codeword-128)>>2);
  else if (bytes_per_codeword <= 768)
    iind = 123 + ((bytes_per_codeword-256)>>3);
  else {
    printf("Illegal codeword size !!!\n");
    exit(-1);
  }


  printf("Generating Codewords\n");
  // generate codewords
  
  bytes_per_codeword = Nbytes/Ncwords;
  printf("bytes_per_codeword = %d\n",bytes_per_codeword);
  
  offset=0;
  
  // useful bits per TTI per stream = (NB_DL_RB * (12 * mod_order) * 14) - NB_DL_RB*(nb_antennas_tx * 6) (Ncp = 0)
  // useful bits per TTI per stream = (NB_DL_RB * (12 * mod_order) * 12) - NB_DL_RB*(nb_antennas_tx * 6) (Ncp = 1)
  
  
  coded_bits_per_codeword = (frame_parms->Ncp == 0) ?
    ( N_RB * (12 * mod_order) * 7 * ((slot_alloc>2)?2:1)) - N_RB*(frame_parms->nb_antennas_tx*6) :
    ( N_RB * (12 * mod_order) * 6 * ((slot_alloc>2)?2:1)) - N_RB*(frame_parms->nb_antennas_tx*6);
  output_offset = 0;

  for (i=0;i<Ncwords;i++) {
    printf("Codeword %d\n",i);
    memcpy(input,&input_data[offset],bytes_per_codeword-crc_len);
    offset+=bytes_per_codeword;
    
    switch (crc_len) {
      
    case 1:
      crc = crc8(input,
		 (bytes_per_codeword-1)<<3)>>24;
      break;
    case 2:
      crc = crc16(input,
		  (bytes_per_codeword-2)<<3)>>16;
      break;
    case 3:
      crc = crc24(input,
		  (bytes_per_codeword-3)<<3)>>8;
      break;
    default:
      printf("Illegal crc_len %d\n",crc_len);
      break;
      
    }
    
    if (crc_len > 0)
      *(unsigned int*)(&input[bytes_per_codeword-crc_len]) = crc;
    
    printf("Encoding ...\n");
    threegpplte_turbo_encoder(input,
			      bytes_per_codeword, 
			      &output[output_offset],
			      f1f2mat[iind*2],   // f1 (see 36121-820, page 14)
			      f1f2mat[(iind*2)+1]  // f2 (see 36121-820, page 14)
			      );
    
    
    printf("Rate Matching (coded bits %d,unpunctured/repeated bits %d,output off %d)...\n",
	   coded_bits_per_codeword,
	   (3*8*bytes_per_codeword)+12,
	   output_offset);
    rate_matching(coded_bits_per_codeword,
		  (3*8*bytes_per_codeword)+12,
		  &output[output_offset],
		  1,
		  i);
    printf("Codeword:"); 
    for (j=0;j<32;j++)
      printf("%d ",output[output_offset+j]);
    printf("\n");

   
    output_offset+=(3*8*bytes_per_codeword)+12;
    // rate 1/3, 8 bytes/bit, 12 bit termination
    
  }
  
  nsymb = (frame_parms->Ncp==0) ? 7 : 6;
  Nsymb = nsymb<<1;
  symbol_offset = (slot_alloc==2) ? nsymb : 0; 
  nsymb = nsymb * ((slot_alloc>2)?2:1);
  printf("nsymb = %d\n",nsymb);
  second_pilot = (frame_parms->Ncp==0) ? 4 : 3;
  
  //Modulation mapping + TTI interleaving (difference w.r.t. LTE specs)
  
  jj=0;

  
  for (l=symbol_offset;l<nsymb+symbol_offset;l++) {
    
    pilots=0;
    if ((l==0)||(l==(Nsymb>>1))){
      pilots=1;
      first_pilot=1;
    }

    if ((l==second_pilot)||(l==(second_pilot+(Nsymb>>1)))) {
      pilots=1;
      first_pilot=0;
    }

    if ((mimo_mode==SISO)||(pilots==0)) { // don't skip pilot symbols
      // This is not LTE, it guarantees that
      // pilots from adjacent base-stations
      // do not interfere with data
      // LTE is eNb centric.  "Smart" Interference
      // cancellation is possible
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
	
	if (rb_alloc_ind > 0)
	  allocate_REs_in_RB(txdataF,
			     &jj,
			     re_offset,
			     frame_parms->ofdm_symbol_size*Nsymb,
			     frame_parms->ofdm_symbol_size*l,
			     output,
			     mimo_mode,
			     pilots,
			     first_pilot,
			     Ntti,
			     mod_order,
			     amp);

	re_offset+=12; // go to next RB
	
	// check if we crossed the symbol boundary and skip DC
	if (re_offset >= frame_parms->ofdm_symbol_size)
	  re_offset=1;
	
      }
	
    }
  }
}

