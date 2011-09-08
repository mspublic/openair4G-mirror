#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/LTE_TRANSPORT/defs.h"
#include "defs.h"

//#define OFDMA_ULSCH

//#define DEBUG_ULSCH_MODULATION

__m128i dft_in128[4][1200],dft_in128[4][1200],dft_out128[4][1200],dft_out128[4][1200];

#ifndef OFDMA_ULSCH
void dft_lte(mod_sym_t *z,mod_sym_t *d, unsigned short Msc_PUSCH, unsigned char Nsymb) {

  int *dft_in0=(int*)dft_in128[0],*dft_out0=(int*)dft_out128[0];
  int *dft_in1=(int*)dft_in128[1],*dft_out1=(int*)dft_out128[1];
  int *dft_in2=(int*)dft_in128[2],*dft_out2=(int*)dft_out128[2];
  int *dft_in3=(int*)dft_in128[3],*dft_out3=(int*)dft_out128[3];

  int *d0,*d1,*d2,*d3,*d4,*d5,*d6,*d7,*d8,*d9,*d10,*d11;

  int *z0,*z1,*z2,*z3,*z4,*z5,*z6,*z7,*z8,*z9,*z10,*z11;
  int i,ip;

  //  printf("Doing lte_dft for Msc_PUSCH %d\n",Msc_PUSCH);

  d0 = d;
  d1 = d0+Msc_PUSCH;
  d2 = d1+Msc_PUSCH;
  d3 = d2+Msc_PUSCH;
  d4 = d3+Msc_PUSCH;
  d5 = d4+Msc_PUSCH;  
  d6 = d5+Msc_PUSCH;
  d7 = d6+Msc_PUSCH;
  d8 = d7+Msc_PUSCH;
  d9 = d8+Msc_PUSCH;
  d10 = d9+Msc_PUSCH;
  d11 = d10+Msc_PUSCH;

  //  printf("symbol 0 (d0 %p, d %p)\n",d0,d);
  for (i=0,ip=0;i<Msc_PUSCH;i++,ip+=4) {
    dft_in0[ip]   =  d0[i];
    dft_in0[ip+1] =  d1[i];
    dft_in0[ip+2] =  d2[i];
    dft_in0[ip+3] =  d3[i];
    dft_in1[ip]   =  d4[i];
    dft_in1[ip+1] =  d5[i];
    dft_in1[ip+2] =  d6[i];
    dft_in1[ip+3] =  d7[i];
    dft_in2[ip]   =  d8[i];
    dft_in2[ip+1] =  d9[i];
    dft_in2[ip+2] =  d10[i];
    dft_in2[ip+3] =  d11[i];
    //    printf("dft%d %d: %d,%d,%d,%d\n",Msc_PUSCH,ip,d0[i],d1[i],d2[i],d3[i]);

    //    dft_in_re2[ip+1] =  d9[i];
    //    dft_in_re2[ip+2] =  d10[i];
  }

  //  printf("\n");

  switch (Msc_PUSCH) {
  case 12:
    /*
    dft12(dft_in0,dft_out0,1);
    dft12(dft_in1,dft_out1,1);
    dft12(dft_in2,dft_out2,1);
    if (Nsymb>11) {
      dft12(dft_in3,dft_out3,1);
    }
    */
    break;
  case 24:
    dft24(dft_in0,dft_out0,1);
    dft24(dft_in1,dft_out1,1);
    dft24(dft_in2,dft_out2,1);
    break;
  case 36:
    dft36(dft_in0,dft_out0,1);
    dft36(dft_in1,dft_out1,1);
    dft36(dft_in2,dft_out2,1);
    break;
  case 48:
    dft48(dft_in0,dft_out0,1);
    dft48(dft_in1,dft_out1,1);
    dft48(dft_in2,dft_out2,1);
    break;
  case 60:
    dft60(dft_in0,dft_out0,1);
    dft60(dft_in1,dft_out1,1);
    dft60(dft_in2,dft_out2,1);
    break;
  case 72:
    dft72(dft_in0,dft_out0,1);
    dft72(dft_in1,dft_out1,1);
    dft72(dft_in2,dft_out2,1);
    break;
  case 96:
    dft96(dft_in0,dft_out0,1);
    dft96(dft_in1,dft_out1,1);
    dft96(dft_in2,dft_out2,1);
    break;
  case 108:
    dft108(dft_in0,dft_out0,1);
    dft108(dft_in1,dft_out1,1);
    dft108(dft_in2,dft_out2,1);
    break;
  case 120:
    dft120(dft_in0,dft_out0,1);
    dft120(dft_in1,dft_out1,1);
    dft120(dft_in2,dft_out2,1);
    break;
  case 144:
    dft144(dft_in0,dft_out0,1);
    dft144(dft_in1,dft_out1,1);
    dft144(dft_in2,dft_out2,1);
    break;
  case 180:
    dft180(dft_in0,dft_out0,1);
    dft180(dft_in1,dft_out1,1);
    dft180(dft_in2,dft_out2,1);
    break;
  case 192:
    dft192(dft_in0,dft_out0,1);
    dft192(dft_in1,dft_out1,1);
    dft192(dft_in2,dft_out2,1);
    break;
  case 240:
    dft240(dft_in0,dft_out0,1);
    dft240(dft_in1,dft_out1,1);
    dft240(dft_in2,dft_out2,1);
    break;
  case 288:
    dft288(dft_in0,dft_out0,1);
    dft288(dft_in1,dft_out1,1);
    dft288(dft_in2,dft_out2,1);
    break;
  case 300:
    dft300(dft_in0,dft_out0,1);
    dft300(dft_in1,dft_out1,1);
    dft300(dft_in2,dft_out2,1);
    break;
  }

  z0 = z;
  z1 = z0+Msc_PUSCH;
  z2 = z1+Msc_PUSCH;
  z3 = z2+Msc_PUSCH;
  z4 = z3+Msc_PUSCH;
  z5 = z4+Msc_PUSCH;  
  z6 = z5+Msc_PUSCH;
  z7 = z6+Msc_PUSCH;
  z8 = z7+Msc_PUSCH;
  z9 = z8+Msc_PUSCH;
  z10 = z9+Msc_PUSCH;
  z11 = z10+Msc_PUSCH;
  //  printf("symbol0 (dft)\n");
  for (i=0,ip=0;i<Msc_PUSCH;i++,ip+=4) {
    z0[i]     = dft_out0[ip]; 
    //    printf("%d,%d,",((short*)&z0[i])[0],((short*)&z0[i])[1]);
    z1[i]     = dft_out0[ip+1]; 
    z2[i]     = dft_out0[ip+2]; 
    z3[i]     = dft_out0[ip+3]; 
    z4[i]     = dft_out1[ip+0]; 
    z5[i]     = dft_out1[ip+1]; 
    z6[i]     = dft_out1[ip+2]; 
    z7[i]     = dft_out1[ip+3]; 
    z8[i]     = dft_out2[ip]; 
    z9[i]     = dft_out2[ip+1]; 
    z10[i]    = dft_out2[ip+2]; 
    z11[i]    = dft_out2[ip+3]; 
    //    printf("out dft%d %d: %d,%d,%d,%d,%d,%d,%d,%d\n",Msc_PUSCH,ip,z0[i],z1[i],z2[i],z3[i],z4[i],z5[i],z6[i],z7[i]);

  }
  //  printf("\n");
}

#endif
void ulsch_modulation(mod_sym_t **txdataF,
		      short amp,
		      unsigned int subframe,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_UE_ULSCH_t *ulsch,
		      u8 cooperation_flag) {

#ifdef IFFT_FPGA_UE
  unsigned char qam64_table_offset = 0;
  unsigned char qam16_table_offset = 0;
  unsigned char qpsk_table_offset = 0;
#else
  unsigned char qam64_table_offset_re = 0;
  unsigned char qam64_table_offset_im = 0;
  unsigned char qam16_table_offset_re = 0;
  unsigned char qam16_table_offset_im = 0;
  short gain_lin_QPSK;
#endif

  short re_offset,re_offset0,i,Msymb,j,nsymb,Msc_PUSCH,l;
  //  unsigned char harq_pid = (rag_flag == 1) ? 0 : subframe2harq_pid_tdd(frame_parms->tdd_config,subframe);
  unsigned char harq_pid = subframe2harq_pid(frame_parms,subframe);
  unsigned char Q_m;
  mod_sym_t *txptr;
  unsigned int symbol_offset;
  unsigned short first_rb;
  unsigned short nb_rb,G;

  if (!ulsch) {
    msg("ulsch_modulation.c: Null ulsch\n");
    return;
  }

  if (harq_pid > 2) {
    msg("ulsch_modulation.c: Illegal harq_pid %d\n",harq_pid);
    return;
  }

  first_rb = ulsch->harq_processes[harq_pid]->first_rb;
  nb_rb = ulsch->harq_processes[harq_pid]->nb_rb;

  if (nb_rb == 0) {
    msg("ulsch_modulation.c: Illegal nb_rb %d\n",nb_rb);
    return;
  }

  if (first_rb >25 ) {
    msg("ulsch_modulation.c: Illegal first_rb %d\n",first_rb);
    return;
  }

  Q_m = get_Qm(ulsch->harq_processes[harq_pid]->mcs);

  G = ulsch->harq_processes[harq_pid]->nb_rb * (12 * Q_m) * (ulsch->Nsymb_pusch);

  // Mapping
  nsymb = (frame_parms->Ncp==0) ? 14:12;
  Msc_PUSCH = ulsch->harq_processes[harq_pid]->nb_rb*12;

#ifdef DEBUG_ULSCH_MODULATION
  msg("ulsch_modulation.c: Doing modulation for G=%d bits, harq_pid %d , nb_rb %d, Q_m %d, Nsymb_pusch %d (nsymb %d)\n",
      G,harq_pid,ulsch->harq_processes[harq_pid]->nb_rb,Q_m, ulsch->Nsymb_pusch,nsymb);
#endif

  // scrambling (Note the placeholding bits are handled in ulsch_coding.c directly!)
  //msg("ulsch bits: ");
  for (i=0;i<G;i++) {
    ulsch->b_tilde[i] = ulsch->h[i];  // put Gold scrambling here later
    //msg("%d,", ulsch->b_tilde[i]);
  }
  //msg("\n");

#ifndef IFFT_FPGA_UE
  gain_lin_QPSK = (short)((amp*ONE_OVER_SQRT2_Q15)>>15);
#endif

  // Modulation

  Msymb = G/Q_m;
  if(cooperation_flag == 2)
    // For Distributed Alamouti Scheme in Collabrative Communication
    {
      for (i=0,j=Q_m;i<Msymb;i+=2,j+=2*Q_m) {

	switch (Q_m) {

	case 2:

#ifndef IFFT_FPGA_UE
	  //UE1, -x1*
	  ((short*)&ulsch->d[i])[0] = (ulsch->b_tilde[j] == 0)  ? (gain_lin_QPSK) : -gain_lin_QPSK;
	  ((short*)&ulsch->d[i])[1] = (ulsch->b_tilde[j+1] == 0)? (-gain_lin_QPSK) : gain_lin_QPSK;
	  //      if (i<Msc_PUSCH)
	  //	printf("input %d (%p): %d,%d\n", i,&ulsch->d[i],((short*)&ulsch->d[i])[0],((short*)&ulsch->d[i])[1]);

	  // UE1, x0*
	  ((short*)&ulsch->d[i+1])[0] = (ulsch->b_tilde[j-2] == 0)  ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  ((short*)&ulsch->d[i+1])[1] = (ulsch->b_tilde[j-1] == 0)? (gain_lin_QPSK) : -gain_lin_QPSK;
#else
	  qpsk_table_offset = MOD_TABLE_QPSK_OFFSET;// UE1, -x1*
	  if (ulsch->b_tilde[j] == 0)
	    {}
	  else
	    qpsk_table_offset+=2;

	  if (ulsch->b_tilde[j+1] == 0) 
	    qpsk_table_offset+=1;
      
	  ulsch->d[i] = (mod_sym_t) qpsk_table_offset;

	  qpsk_table_offset = MOD_TABLE_QPSK_OFFSET;//UE1,x0*

	  if (ulsch->b_tilde[j-2] == 0)
	    qpsk_table_offset+=2;

	  if(ulsch->b_tilde[j-1] == 0)
	    {}
	  else
	    qpsk_table_offset+=1;

	  ulsch->d[i+1] = (mod_sym_t) qpsk_table_offset;
#endif    

	  break;

	case 4:
#ifndef IFFT_FPGA_UE

	  //UE1,-x1*
	  qam16_table_offset_re = 0;
	  if (ulsch->b_tilde[j] == 1)
	    qam16_table_offset_re+=2;

	  if (ulsch->b_tilde[j+1] == 1)
	    qam16_table_offset_re+=1;
      
      
	  qam16_table_offset_im = 0;
	  if (ulsch->b_tilde[j+2] == 1)
	    qam16_table_offset_im+=2;

	  if (ulsch->b_tilde[j+3] == 1)
	    qam16_table_offset_im+=1;

      
	  ((short*)&ulsch->d[i])[0]=-(short)(((int)amp*qam16_table[qam16_table_offset_re])>>15);
	  ((short*)&ulsch->d[i])[1]=(short)(((int)amp*qam16_table[qam16_table_offset_im])>>15);

	  //UE1,x0*
	  qam16_table_offset_re = 0;
	  if (ulsch->b_tilde[j-4] == 1)
	    qam16_table_offset_re+=2;

	  if (ulsch->b_tilde[j-3] == 1)
	    qam16_table_offset_re+=1;
      
      
	  qam16_table_offset_im = 0;
	  if (ulsch->b_tilde[j-2] == 1)
	    qam16_table_offset_im+=2;

	  if (ulsch->b_tilde[j-1] == 1)
	    qam16_table_offset_im+=1;

      
	  ((short*)&ulsch->d[i+1])[0]=(short)(((int)amp*qam16_table[qam16_table_offset_re])>>15);
	  ((short*)&ulsch->d[i+1])[1]=-(short)(((int)amp*qam16_table[qam16_table_offset_im])>>15);

#else
	  qam16_table_offset = 5;//UE1,-x1*
	  if (ulsch->b_tilde[j] == 1)
	    {}
	  else
	    qam16_table_offset+=2;

	  if (ulsch->b_tilde[j+1] == 1)
	    qam16_table_offset+=1;

	  if (ulsch->b_tilde[j+2] == 1)
	    qam16_table_offset+=8;

	  if (ulsch->b_tilde[j+3] == 1)
	    qam16_table_offset+=4;

      
	  ulsch->d[i] = (mod_sym_t) qam16_table_offset;
      
	  qam16_table_offset = 5;//UE1,x0*
	  if (ulsch->b_tilde[j-4] == 1)
	    qam16_table_offset+=2;

	  if (ulsch->b_tilde[j-3] == 1)
	    qam16_table_offset+=1;

	  if (ulsch->b_tilde[j-2] == 1)
	    {}
	  else
	    qam16_table_offset+=8;

	  if (ulsch->b_tilde[j-1] == 1)
	    qam16_table_offset+=4;

      
	  ulsch->d[i+1] = (mod_sym_t) qam16_table_offset;     
#endif
      
	  break;
     
	case 6:

#ifndef IFFT_FPGA_UE

	  //UE1,-x1*FPGA_UE
	  qam64_table_offset_re = 0;
	  if (ulsch->b_tilde[j] == 1)
	    qam64_table_offset_re+=4;
      
	  if (ulsch->b_tilde[j+1] == 1)
	    qam64_table_offset_re+=2;
      
	  if (ulsch->b_tilde[j+2] == 1)
	    qam64_table_offset_re+=1;
      
	  qam64_table_offset_im = 0;
	  if (ulsch->b_tilde[j+3] == 1)
	    qam64_table_offset_im+=4;
      
	  if (ulsch->b_tilde[j+4] == 1)
	    qam64_table_offset_im+=2;
      
	  if (ulsch->b_tilde[j+5] == 1)
	    qam64_table_offset_im+=1;
      
      
	  ((short*)&ulsch->d[i])[0]=-(short)(((int)amp*qam64_table[qam64_table_offset_re])>>15);
	  ((short*)&ulsch->d[i])[1]=(short)(((int)amp*qam64_table[qam64_table_offset_im])>>15);

	  //UE1,x0*
	  qam64_table_offset_re = 0;
	  if (ulsch->b_tilde[j-6] == 1)
	    qam64_table_offset_re+=4;
      
	  if (ulsch->b_tilde[j-5] == 1)
	    qam64_table_offset_re+=2;
      
	  if (ulsch->b_tilde[j-4] == 1)
	    qam64_table_offset_re+=1;
      
	  qam64_table_offset_im = 0;
	  if (ulsch->b_tilde[j-3] == 1)
	    qam64_table_offset_im+=4;
      
	  if (ulsch->b_tilde[j-2] == 1)
	    qam64_table_offset_im+=2;
      
	  if (ulsch->b_tilde[j-1] == 1)
	    qam64_table_offset_im+=1;
      
      
	  ((short*)&ulsch->d[i+1])[0]=(short)(((int)amp*qam64_table[qam64_table_offset_re])>>15);
	  ((short*)&ulsch->d[i+1])[1]=-(short)(((int)amp*qam64_table[qam64_table_offset_im])>>15);

#else
	  qam64_table_offset = 21; //UE1,-x1*
	  if (ulsch->b_tilde[j] == 1)
	    {}
	  else
	    qam64_table_offset+=4;
      
	  if (ulsch->b_tilde[j+1] == 1)
	    qam64_table_offset+=2;
      
	  if (ulsch->b_tilde[j+2] == 1)
	    qam64_table_offset+=1;
      
      
      
	  if (ulsch->b_tilde[j+3] == 1)
	    qam64_table_offset+=32;
      
	  if (ulsch->b_tilde[j+4] == 1)
	    qam64_table_offset+=16;
      
	  if (ulsch->b_tilde[j+5] == 1)
	    qam64_table_offset+=8;
      
      
	  ulsch->d[i] = (mod_sym_t) qam64_table_offset;

	  qam64_table_offset = 21; //UE1,x0*
	  if (ulsch->b_tilde[j-6] == 1)
	    qam64_table_offset+=4;
      
	  if (ulsch->b_tilde[j-5] == 1)
	    qam64_table_offset+=2;
      
	  if (ulsch->b_tilde[j-4] == 1)
	    qam64_table_offset+=1;
      
      
	  if (ulsch->b_tilde[j-3] == 1)
	    {}
	  else
	    qam64_table_offset+=32;
      
	  if (ulsch->b_tilde[j-2] == 1)
	    qam64_table_offset+=16;
      
	  if (ulsch->b_tilde[j-1] == 1)
	    qam64_table_offset+=8;
      
      
	  ulsch->d[i+1] = (mod_sym_t) qam64_table_offset;
#endif //IFFT_FPGA_UE
	  break;

	}//switch
      }//for
    }//cooperation_flag == 2
  else
    {
      for (i=0,j=0;i<Msymb;i++,j+=Q_m) {

	switch (Q_m) {

	case 2:
	  // TODO: this has to be updated!!!
#ifndef IFFT_FPGA_UE
	  ((short*)&ulsch->d[i])[0] = (ulsch->b_tilde[j] == 0)  ? (-gain_lin_QPSK) : gain_lin_QPSK;
	  ((short*)&ulsch->d[i])[1] = (ulsch->b_tilde[j+1] == 0)? (-gain_lin_QPSK) : gain_lin_QPSK;
	  //      if (i<Msc_PUSCH)
	  //	printf("input %d (%p): %d,%d\n", i,&ulsch->d[i],((short*)&ulsch->d[i])[0],((short*)&ulsch->d[i])[1]);
#else
	  qpsk_table_offset = MOD_TABLE_QPSK_OFFSET;
	  if (ulsch->b_tilde[j] == 0) //real
	    qpsk_table_offset+=2;
	  if (ulsch->b_tilde[j+1] == 0) //imag
	    qpsk_table_offset+=1;
      
	  ulsch->d[i] = (mod_sym_t) qpsk_table_offset;
#endif    

	  break;

	case 4:
#ifndef IFFT_FPGA_UE
	  qam16_table_offset_re = 0;
	  qam16_table_offset_im = 0;

	  if (ulsch->b_tilde[j] == 1)
	    qam16_table_offset_re+=2;

	  if (ulsch->b_tilde[j+1] == 1)
	    qam16_table_offset_im+=2;
      
	  if (ulsch->b_tilde[j+2] == 1)
	    qam16_table_offset_re+=1;

	  if (ulsch->b_tilde[j+3] == 1)
	    qam16_table_offset_im+=1;

      
	  ((short*)&ulsch->d[i])[0]=(short)(((int)amp*qam16_table[qam16_table_offset_re])>>15);
	  ((short*)&ulsch->d[i])[1]=(short)(((int)amp*qam16_table[qam16_table_offset_im])>>15);
	  //      printf("input(16qam) %d (%p): %d,%d\n", i,&ulsch->d[i],((short*)&ulsch->d[i])[0],((short*)&ulsch->d[i])[1]);
#else
	  qam16_table_offset = MOD_TABLE_16QAM_OFFSET;
	  if (ulsch->b_tilde[j] == 1)
	    qam16_table_offset+=8;

	  if (ulsch->b_tilde[j+1] == 1)
	    qam16_table_offset+=4;

	  if (ulsch->b_tilde[j+2] == 1)
	    qam16_table_offset+=2;

	  if (ulsch->b_tilde[j+3] == 1)
	    qam16_table_offset+=1;

      
	  ulsch->d[i] = (mod_sym_t) qam16_table_offset;
#endif
      
	  break;
     
	case 6:

#ifndef IFFT_FPGA_UE
	  qam64_table_offset_re = 0;
	  qam64_table_offset_im = 0;

	  if (ulsch->b_tilde[j] == 1)
	    qam64_table_offset_re+=4;
      
	  if (ulsch->b_tilde[j+1] == 1)
	    qam64_table_offset_im+=4;
      
	  if (ulsch->b_tilde[j+2] == 1)
	    qam64_table_offset_re+=2;
      
	  if (ulsch->b_tilde[j+3] == 1)
	    qam64_table_offset_im+=2;
      
	  if (ulsch->b_tilde[j+4] == 1)
	    qam64_table_offset_re+=1;
      
	  if (ulsch->b_tilde[j+5] == 1)
	    qam64_table_offset_im+=1;
      
      
	  ((short*)&ulsch->d[i])[0]=(short)(((int)amp*qam64_table[qam64_table_offset_re])>>15);
	  ((short*)&ulsch->d[i])[1]=(short)(((int)amp*qam64_table[qam64_table_offset_im])>>15);

#else
	  qam64_table_offset = MOD_TABLE_64QAM_OFFSET;
	  if (ulsch->b_tilde[j] == 1)
	    qam64_table_offset+=32;
      
	  if (ulsch->b_tilde[j+1] == 1)
	    qam64_table_offset+=16;
      
	  if (ulsch->b_tilde[j+2] == 1)
	    qam64_table_offset+=8;
      
      
      
	  if (ulsch->b_tilde[j+3] == 1)
	    qam64_table_offset+=4;
      
	  if (ulsch->b_tilde[j+4] == 1)
	    qam64_table_offset+=2;
      
	  if (ulsch->b_tilde[j+5] == 1)
	    qam64_table_offset+=1;
      
      
	  ulsch->d[i] = (mod_sym_t) qam64_table_offset;
#endif //IFFT_FPGA_UE
	  break;

	}
      }
    }// normal symbols 


  // Transform Precoding

#ifdef OFDMA_ULSCH
  for (i=0;i<Msymb;i++) {
    ulsch->z[i] = ulsch->d[i]; 
  }
#else  
  dft_lte(ulsch->z,ulsch->d,Msc_PUSCH,ulsch->Nsymb_pusch);
#endif

#ifdef OFDMA_ULSCH
#ifdef IFFT_FPGA_UE

  for (j=0,l=0;l<(nsymb-1);l++) {
    re_offset = ulsch->harq_processes[harq_pid]->first_rb*12 + frame_parms->N_RB_DL*12/2;
    if (re_offset > (frame_parms->N_RB_DL*12))
      re_offset -= (frame_parms->N_RB_DL*12);

    symbol_offset = (unsigned int)frame_parms->N_RB_DL*12*(l+(subframe*nsymb));
    txptr = &txdataF[0][symbol_offset];
    //printf("symbol %d: symbol_offset %d\n",l,symbol_offset);
    if (((frame_parms->Ncp == 0) && ((l==3) || (l==10)))||
	((frame_parms->Ncp == 1) && ((l==2) || (l==8)))) {
    }
    else {
      //printf("copying %d REs\n",Msc_PUSCH);
      for (i=0;i<Msc_PUSCH;i++,j++) {
	txptr[re_offset++] = ulsch->z[j];

	if (re_offset==(frame_parms->N_RB_DL*12))
	  re_offset = 0;                                 
      }
    }
  }
# else
  re_offset0 = frame_parms->first_carrier_offset + (ulsch->harq_processes[harq_pid]->first_rb*12);
  if (re_offset0>frame_parms->ofdm_symbol_size) {
    re_offset0 -= frame_parms->ofdm_symbol_size;
    re_offset0++;
  }
  //  printf("re_offset0 %d\n",re_offset0);
  for (j=0,l=0;l<(nsymb-1);l++) {
    re_offset = re_offset0;
    symbol_offset = (unsigned int)frame_parms->ofdm_symbol_size*(l+(subframe*nsymb));
#ifdef DEBUG_ULSCH_MODULATION
        printf("symbol %d (subframe %d): symbol_offset %d\n",l,subframe,symbol_offset);
#endif
    txptr = &txdataF[0][symbol_offset];
    if (((frame_parms->Ncp == 0) && ((l==3) || (l==10)))||
	((frame_parms->Ncp == 1) && ((l==2) || (l==8)))) {
    }
    // Skip reference symbols
    else {

      //      printf("copying %d REs\n",Msc_PUSCH);
      for (i=0;i<Msc_PUSCH;i++,j++) {
#ifdef DEBUG_ULSCH_MODULATION
	printf("re_offset %d (%p): %d,%d\n", re_offset,&ulsch->z[j],((short*)&ulsch->z[j])[0],((short*)&ulsch->z[j])[1]);
#endif
	txptr[re_offset++] = ulsch->z[j];
	if (re_offset==frame_parms->ofdm_symbol_size)
	  re_offset = 0;                                 
      }
    }
  }
#endif 
# else
  re_offset0 = frame_parms->first_carrier_offset + (ulsch->harq_processes[harq_pid]->first_rb*12);
  if (re_offset0>frame_parms->ofdm_symbol_size) {
    re_offset0 -= frame_parms->ofdm_symbol_size;
    re_offset0++;
  }
  //    printf("re_offset0 %d\n",re_offset0);
  for (j=0,l=0;l<(nsymb-1);l++) {
    re_offset = re_offset0;
    symbol_offset = (unsigned int)frame_parms->ofdm_symbol_size*(l+(subframe*nsymb));
    //printf("symbol %d (subframe %d): symbol_offset %d\n",l,subframe,symbol_offset);
    txptr = &txdataF[0][symbol_offset];
    if (((frame_parms->Ncp == 0) && ((l==3) || (l==10)))||
	((frame_parms->Ncp == 1) && ((l==2) || (l==8)))) {
    }
    // Skip reference symbols
    else {
      //      printf("copying %d REs\n",Msc_PUSCH);
      for (i=0;i<Msc_PUSCH;i++,j++) {

#ifdef DEBUG_ULSCH_MODULATION
	printf("re_offset %d (%p): %d,%d\n", re_offset,&ulsch->z[j],((short*)&ulsch->z[j])[0],((short*)&ulsch->z[j])[1]);
#endif //DEBUG_ULSCH_MODULATION
	txptr[re_offset++] = ulsch->z[j];

	if (re_offset==frame_parms->ofdm_symbol_size)
	  re_offset = 0;                                 
      }
    }
  }
#endif

}

