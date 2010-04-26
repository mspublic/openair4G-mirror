#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/LTE_TRANSPORT/defs.h"
#include "defs.h"
#include <math.h>
#define OFDMA_ULSCH

//#define DEBUG_ULSCH_MODULATION

__m128i dft_in_re128[2][300],dft_in_im128[2][300],dft_out_re128[2][300],dft_out_im128[2][300];

#ifndef OFDMA_ULSCH
void dft_lte(int *z,int *d, unsigned short Msc_PUSCH, unsigned char Nsymb) {

  short *dft_in_re=(short*)dft_in_re128[0],*dft_in_im=(short*)dft_in_im128[0],*dft_out_re=(short*)dft_out_re128[0],*dft_out_im=(short*)dft_out_im128[0];
  short *dft_in_re2=(short*)dft_in_re128[1],*dft_in_im2=(short*)dft_in_im128[1],*dft_out_re2=(short*)dft_out_re128[1],*dft_out_im2=(short*)dft_out_im128[1];
  int *d0,*d1,*d2,*d3,*d4,*d5,*d6,*d7,*d8,*d9,*d10;

  int *z0,*z1,*z2,*z3,*z4,*z5,*z6,*z7,*z8,*z9,*z10;
  int i,ip;

  printf("Doing lte_dft for Msc_PUSCH %d\n",Msc_PUSCH);

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

  for (i=0,ip=0;i<Msc_PUSCH;i++,ip+=8) {
    dft_in_re[ip] =  ((short*)d0)[i<<1];
    dft_in_im[ip] =  ((short*)d0)[1+(i<<1)];
    printf("%d+sqrt(-1)*(%d),\n",dft_in_re[ip],dft_in_im[ip]);
    dft_in_re[ip+1] =  ((short*)d1)[i<<1];
    dft_in_im[ip+1] =  ((short*)d1)[1+(i<<1)];

    dft_in_re[ip+2] =  ((short*)d2)[i<<1];
    dft_in_im[ip+2] =  ((short*)d2)[1+(i<<1)];

    dft_in_re[ip+3] =  ((short*)d3)[i<<1];
    dft_in_im[ip+3] =  ((short*)d3)[1+(i<<1)];

    dft_in_re[ip+4] =  ((short*)d4)[i<<1];
    dft_in_im[ip+4] =  ((short*)d4)[1+(i<<1)];

    dft_in_re[ip+5] =  ((short*)d5)[i<<1];
    dft_in_im[ip+5] =  ((short*)d5)[1+(i<<1)];

    dft_in_re[ip+6] =  ((short*)d6)[i<<1];
    dft_in_im[ip+6] =  ((short*)d6)[1+(i<<1)];

    dft_in_re[ip+7] =  ((short*)d7)[i<<1];
    dft_in_im[ip+7] =  ((short*)d7)[1+(i<<1)];

    dft_in_re2[ip] =  ((short*)d8)[i<<1];
    dft_in_im2[ip] =  ((short*)d8)[1+(i<<1)];

    dft_in_re2[ip+1] =  ((short*)d9)[i<<1];
    dft_in_im2[ip+1] =  ((short*)d9)[1+(i<<1)];

    dft_in_re2[ip+2] =  ((short*)d10)[i<<1];
    dft_in_im2[ip+2] =  ((short*)d10)[1+(i<<1)];
  }


  switch (Msc_PUSCH) {
  case 12:
    break;
  case 24:
    dft24(dft_in_re,dft_out_re,dft_in_im,dft_out_im);
    dft24(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2);
    break;
  case 36:
    dft36(dft_in_re,dft_out_re,dft_in_im,dft_out_im);
    dft36(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2);
    break;
  case 48:
    dft48(dft_in_re,dft_out_re,dft_in_im,dft_out_im,1);
    dft48(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2,1);
    break;
  case 60:
    dft60(dft_in_re,dft_out_re,dft_in_im,dft_out_im,1);
    dft60(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2,1);
    break;
  case 72:
    dft72(dft_in_re,dft_out_re,dft_in_im,dft_out_im);
    dft72(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2);
    break;
  case 96:
    dft96(dft_in_re,dft_out_re,dft_in_im,dft_out_im);
    dft96(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2);
    break;
  case 108:
    dft108(dft_in_re,dft_out_re,dft_in_im,dft_out_im);
    dft108(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2);
    break;
  case 120:
    dft120(dft_in_re,dft_out_re,dft_in_im,dft_out_im);
    dft120(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2);
    break;
  case 144:
    dft144(dft_in_re,dft_out_re,dft_in_im,dft_out_im);
    dft144(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2);
    break;
  case 168:
    //    dft168(dft_in_re,dft_out_re,dft_in_im,dft_out_im);
    //    dft168(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2);
    break;
  case 180:
    dft180(dft_in_re,dft_out_re,dft_in_im,dft_out_im);
    dft180(dft_in_re2,dft_out_re2,dft_in_im2,dft_out_im2);
    break;
  case 192:
    break;
  case 240:
    break;
  case 288:
    break;
  case 300:
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

  for (i=0,ip=0;i<Msc_PUSCH;i++,ip+=8) {
    ((short*)z0)[i<<1]     = dft_out_re[ip]; 
    ((short*)z0)[1+(i<<1)] = dft_out_im[ip];  
    printf("dft_out %d : (%d,%d)\n",i,dft_out_re[ip],dft_out_im[ip]);

    ((short*)z1)[i<<1]     = dft_out_re[ip+1]; 
    ((short*)z1)[1+(i<<1)] = dft_out_im[ip+1];  

    ((short*)z2)[i<<1]     = dft_out_re[ip+2]; 
    ((short*)z2)[1+(i<<1)] = dft_out_im[ip+2];  

    ((short*)z3)[i<<1]     = dft_out_re[ip+3]; 
    ((short*)z3)[1+(i<<1)] = dft_out_im[ip+3];  

    ((short*)z4)[i<<1]     = dft_out_re[ip+4]; 
    ((short*)z4)[1+(i<<1)] = dft_out_im[ip+4];  

    ((short*)z5)[i<<1]     = dft_out_re[ip+5]; 
    ((short*)z5)[1+(i<<1)] = dft_out_im[ip+5];  

    ((short*)z6)[i<<1]     = dft_out_re[ip+6]; 
    ((short*)z6)[1+(i<<1)] = dft_out_im[ip+6];  

    ((short*)z7)[i<<1]     = dft_out_re[ip+7]; 
    ((short*)z7)[1+(i<<1)] = dft_out_im[ip+7];  

    ((short*)z8)[i<<1]     = dft_out_re2[ip]; 
    ((short*)z8)[1+(i<<1)] = dft_out_im2[ip];  

    ((short*)z9)[i<<1]     = dft_out_re2[ip+1]; 
    ((short*)z9)[1+(i<<1)] = dft_out_im2[ip+1];  

    ((short*)z10)[i<<1]     = dft_out_re2[ip+2]; 
    ((short*)z10)[1+(i<<1)] = dft_out_im2[ip+2];  
  }

}

#endif
void ulsch_modulation(mod_sym_t **txdataF,
		      short amp,
		      unsigned int subframe,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_UE_ULSCH_t *ulsch,
		      unsigned char rag_flag) {

#ifdef IFFT_FPGA
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
  unsigned char harq_pid = (rag_flag == 1) ? 0 : subframe2harq_pid_tdd(frame_parms->tdd_config,subframe);
  unsigned char Q_m = get_Qm(ulsch->harq_processes[harq_pid]->mcs);
  mod_sym_t *txptr;
  unsigned int symbol_offset;
  unsigned short first_rb = ulsch->harq_processes[harq_pid]->first_rb;
  unsigned short nb_rb = ulsch->harq_processes[harq_pid]->nb_rb,G;


  G = ulsch->harq_processes[harq_pid]->nb_rb * (12 * Q_m) * (ulsch->Nsymb_pusch);
#ifdef DEBUG_ULSCH_MODULATION
  msg("ulsch_modulation.c: Doing modulation for G=%d bits, nb_rb %d, Q_m %d, Nsymb_pusch %d\n",
      G,ulsch->harq_processes[harq_pid]->nb_rb,Q_m, ulsch->Nsymb_pusch);
#endif
  // scrambling (Note the placeholding bits are handled in ulsch_coding.c directly!)
  for (i=0;i<G;i++) {
    ulsch->b_tilde[i] = ulsch->h[i];  // put Gold scrambling here later
  }

#ifndef IFFT_FPGA
  gain_lin_QPSK = (short)((amp*ONE_OVER_SQRT2_Q15)>>15);  
#endif

  // Modulation

  Msymb = G/Q_m;
  for (i=0,j=0;i<Msymb;i++,j+=Q_m) {

    switch (Q_m) {

    case 2:

#ifndef IFFT_FPGA
      ((short*)&ulsch->d[i])[0] = (ulsch->b_tilde[j] == 0)  ? (-gain_lin_QPSK) : gain_lin_QPSK;
      ((short*)&ulsch->d[i])[1] = (ulsch->b_tilde[j+1] == 0)? (-gain_lin_QPSK) : gain_lin_QPSK;
#else
      qpsk_table_offset = 1;
      if (ulsch->b_tilde[j] == 1)
	qpsk_table_offset+=1;
      if (ulsch->b_tilde[j+1] == 1) 
	qpsk_table_offset+=2;
      
      ulsch->d[i] = (mod_sym_t) qpsk_table_offset;
#endif    

      break;

    case 4:
#ifndef IFFT_FPGA
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

      
      ((short*)&ulsch->d[i])[0]=(short)(((int)amp*qam16_table[qam16_table_offset_re])>>15);
      ((short*)&ulsch->d[i])[1]=(short)(((int)amp*qam16_table[qam16_table_offset_im])>>15);

#else
      qam16_table_offset = 5;
      if (ulsch->b_tilde[j] == 1)
	qam16_table_offset+=2;

      if (ulsch->b_tilde[j+1] == 1)
	qam16_table_offset+=1;

      if (ulsch->b_tilde[j+2] == 1)
	qam16_table_offset+=8;

      if (ulsch->b_tilde[j+3] == 1)
	qam16_table_offset+=4;

      
      ulsch->d[i] = (mod_sym_t) qam16_table_offset;
      
#endif
      
      break;
     
    case 6:

#ifndef IFFT_FPGA
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
      
      
      ((short*)&ulsch->d[i])[0]=(short)(((int)amp*qam64_table[qam64_table_offset_re])>>15);
      ((short*)&ulsch->d[i])[1]=(short)(((int)amp*qam64_table[qam64_table_offset_im])>>15);

#else
      
      qam64_table_offset = 21;
      if (ulsch->b_tilde[j] == 1)
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
      
#endif //IFFT_FPGA
      break;

    }
  }

  // Mapping
  nsymb = (frame_parms->Ncp==0) ? 14:12;
  Msc_PUSCH = ulsch->harq_processes[harq_pid]->nb_rb*12;

  // Transform Precoding

#ifdef OFDMA_ULSCH
  for (i=0;i<Msymb;i++) {
    ulsch->z[i] = ulsch->d[i]; 
  }
#else
  dft_lte(ulsch->z,ulsch->d,Msc_PUSCH,ulsch->Nsymb_pusch);
#endif

#ifdef OFDMA_ULSCH
#ifdef IFFT_FPGA

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
#endif 
#endif

#ifndef OFDMA_ULSCH
  re_offset0 = frame_parms->first_carrier_offset + (ulsch->harq_processes[harq_pid]->first_rb*12);
  if (re_offset0>frame_parms->ofdm_symbol_size) {
    re_offset0 -= frame_parms->ofdm_symbol_size;
    re_offset0++;
  }
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
      //printf("copying %d REs\n",Msc_PUSCH);
      for (i=0;i<Msc_PUSCH;i++,j++) {
	txptr[re_offset++] = ulsch->z[j];
	if (re_offset==frame_parms->ofdm_symbol_size)
	  re_offset = 1;                                 // Skip DC
      }
    }
  }
#endif
  

}

