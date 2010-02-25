/* file: ulsch_coding.c
   purpose: Top-level routines for implementing Turbo-coded (ULSCH) transport channels from 36-212, V8.6 2009-03
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 
*/

#include "PHY/defs.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "PHY/LTE_TRANSPORT/defs.h"
#include "defs.h"

//#define DEBUG_ULSCH_CODING 
//#define DEBUG_ULSCH_FREE 1

/*
#define is_not_pilot(pilots,first_pilot,re) (pilots==0) || \ 
	((pilots==1)&&(first_pilot==1)&&(((re>2)&&(re<6))||((re>8)&&(re<12)))) || \
	((pilots==1)&&(first_pilot==0)&&(((re<3))||((re>5)&&(re<9)))) \
*/
#define is_not_pilot(pilots,first_pilot,re) (1)


void free_ue_ulsch(LTE_UE_ULSCH_t *ulsch) {
  int i;
  int r;

  if (ulsch) {
#ifdef DEBUG_ULSCH_FREE
    msg("Freeing ulsch %p\n",ulsch);
#endif
    for (i=0;i<ulsch->Mdlharq;i++) {
#ifdef DEBUG_ULSCH_FREE
      msg("Freeing ulsch process %d\n",i);
#endif
      if (ulsch->harq_processes[i]) {
#ifdef DEBUG_ULSCH_FREE
	msg("Freeing ulsch process %d (%p)\n",i,ulsch->harq_processes[i]);
#endif
	if (ulsch->harq_processes[i]->b) {
	  free16(ulsch->harq_processes[i]->b,MAX_ULSCH_PAYLOAD_BYTES);
#ifdef DEBUG_ULSCH_FREE
	  msg("Freeing ulsch process %d b (%p)\n",i,ulsch->harq_processes[i]->b);
#endif
	}
	if (ulsch->harq_processes[i]->c) {
#ifdef DEBUG_ULSCH_FREE
	  msg("Freeing ulsch process %d c (%p)\n",i,ulsch->harq_processes[i]->c);
#endif
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++) {

#ifdef DEBUG_ULSCH_FREE
	    msg("Freeing ulsch process %d c[%d] (%p)\n",i,r,ulsch->harq_processes[i]->c[r]);
#endif
	    if (ulsch->harq_processes[i]->c[r]) 
	      free16(ulsch->harq_processes[i]->c[r],((r==0)?8:0) + 3+(MAX_ULSCH_PAYLOAD_BYTES));
	  }
	}
	free16(ulsch->harq_processes[i],sizeof(LTE_UL_UE_HARQ_t));
      }
    }
    free16(ulsch,sizeof(LTE_UE_ULSCH_t));
  }
  
}

LTE_UE_ULSCH_t *new_ue_ulsch(unsigned char Mdlharq) {

  LTE_UE_ULSCH_t *ulsch;
  unsigned char exit_flag = 0,i,j,r;
  
  ulsch = (LTE_UE_ULSCH_t *)malloc16(sizeof(LTE_UE_ULSCH_t));
  if (ulsch) {

    ulsch->Mdlharq = Mdlharq;
    for (i=0;i<Mdlharq;i++) {
      ulsch->harq_processes[i] = (LTE_UL_UE_HARQ_t *)malloc16(sizeof(LTE_UL_UE_HARQ_t));
      //      printf("ulsch->harq_processes[%d] %p\n",i,ulsch->harq_processes[i]);
      if (ulsch->harq_processes[i]) {
	ulsch->harq_processes[i]->b          = (unsigned char*)malloc16(MAX_ULSCH_PAYLOAD_BYTES);
	if (!ulsch->harq_processes[i]->b) {
	  msg("Can't get b\n");
	  exit_flag=1;
	}
	for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++) {
	  ulsch->harq_processes[i]->c[r] = (unsigned char*)malloc16(((r==0)?8:0) + 3+(MAX_ULSCH_PAYLOAD_BYTES));  // account for filler in first segment and CRCs for multiple segment case
	  if (!ulsch->harq_processes[i]->c[r]) {
	    msg("Can't get c\n");
	    exit_flag=2;
	  }
	}
      }	else {
	msg("Can't get harq_p %d\n",i);
	exit_flag=3;
      }
    }

    if (exit_flag==0) {
      for (i=0;i<Mdlharq;i++)
	for (j=0;j<96;j++)
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++)
	    ulsch->harq_processes[i]->d[r][j] = LTE_NULL;
      return(ulsch);
    }
  }
  msg("new_ue_ulsch exit flag, size of  %d ,   %d\n",exit_flag, sizeof(LTE_UE_ULSCH_t));
  free_ue_ulsch(ulsch);
  return(NULL);
  
  
}

unsigned char cs_ri_normal[4]    = {1,4,7,10};
unsigned char cs_ri_extended[4]  = {0,3,5,8};
unsigned char cs_ack_normal[4]   = {2,3,8,9};
unsigned char cs_ack_extended[4] = {1,2,6,7};

int ulsch_encoding(unsigned char *a,
		   unsigned short A,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   LTE_UE_ULSCH_t *ulsch,
		   unsigned char harq_pid,
		   unsigned short N_RB,
		   unsigned char *o,
		   unsigned char O,
		   unsigned char *o_RI,
		   unsigned char O_RI,
		   unsigned char *o_ACK,
		   unsigned char O_ACK) {
  
  unsigned short offset;
  unsigned int crc=1;
  unsigned short iind;
  unsigned char Q_m = get_Qm(ulsch->harq_processes[harq_pid]->mcs);
  unsigned int Kr,Kr_bytes,r,r_offset=0;
  unsigned char y[6*14*1200];
  unsigned char *columnset;
  unsigned int sumKr=0;
  unsigned int Qprime,L,G,Q_CQI,Q_RI,Q_ACK,H,Hprime,Hpp,Cmux,Rmux,Rmux_prime;
  unsigned int Qprime_ACK,Qprime_CQI,Qprime_RI;
  unsigned int E;
  unsigned char ack_parity;
  unsigned int i,q,j;

  if (ulsch->harq_processes[harq_pid]->Ndi == 1) {  // this is a new packet
    
    // Add 24-bit crc (polynomial A) to payload
    crc = crc24a(a,
		 A)>>8;

    *(unsigned int*)(&a[A>>3]) = crc;
    ulsch->harq_processes[harq_pid]->B = A+24;
    ulsch->harq_processes[harq_pid]->b = a;
    lte_segmentation(ulsch->harq_processes[harq_pid]->b,
		     ulsch->harq_processes[harq_pid]->c,
		     ulsch->harq_processes[harq_pid]->B,
		     &ulsch->harq_processes[harq_pid]->C,
		     &ulsch->harq_processes[harq_pid]->Cplus,
		     &ulsch->harq_processes[harq_pid]->Cminus,
		     &ulsch->harq_processes[harq_pid]->Kplus,
		     &ulsch->harq_processes[harq_pid]->Kminus,		     
		     &ulsch->harq_processes[harq_pid]->F);

    for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
      if (r<ulsch->harq_processes[harq_pid]->Cminus)
	Kr = ulsch->harq_processes[harq_pid]->Kminus;
      else
	Kr = ulsch->harq_processes[harq_pid]->Kplus;
      Kr_bytes = Kr>>3;

      // get interleaver index for Turbo code (lookup in Table 5.1.3-3 36-212, V8.6 2009-03, p. 13-14)
      if (Kr_bytes<=64)
	iind = (Kr_bytes-5);
      else if (Kr_bytes <=128)
	iind = 59 + ((Kr_bytes-64)>>1);
      else if (Kr_bytes <= 256)
	iind = 91 + ((Kr_bytes-128)>>2);
      else if (Kr_bytes <= 768)
	iind = 123 + ((Kr_bytes-256)>>3);
      else {
	msg("ulsch_coding: Illegal codeword size %d!!!\n",Kr_bytes);
	return(-1);
      }
    
  
#ifdef DEBUG_ULSCH_CODING
  printf("Generating Code Segment %d (%d bits)\n",r,Kr);
  // generate codewords
  
  printf("bits_per_codeword (Kr)= %d\n",Kr);
  printf("N_RB = %d\n",N_RB);
  printf("first_ulsch_symbol %d\n",frame_parms->first_ulsch_symbol);
  printf("Ncp %d\n",frame_parms->Ncp);
  printf("Qm %d\n",Q_m);
#endif

  offset=0;
  




#ifdef DEBUG_ULSCH_CODING    
    printf("Encoding ... iind %d f1 %d, f2 %d\n",iind,f1f2mat[iind*2],f1f2mat[(iind*2)+1]);
#endif

    threegpplte_turbo_encoder(ulsch->harq_processes[harq_pid]->c[r],
			      Kr>>3, 
			      &ulsch->harq_processes[harq_pid]->d[r][96],
			      (r==0) ? ulsch->harq_processes[harq_pid]->F : 0,
			      f1f2mat[iind*2],   // f1 (see 36121-820, page 14)
			      f1f2mat[(iind*2)+1]  // f2 (see 36121-820, page 14)
			      );
#ifdef DEBUG_ULSCH_CODING
    if (r==0)
      write_output("enc_output0.m","enc0",&ulsch->harq_processes[harq_pid]->d[r][96],(3*8*Kr_bytes)+12,1,4);
#endif

      ulsch->harq_processes[harq_pid]->RTC[r] = 
	sub_block_interleaving_turbo(4+(Kr_bytes*8), 
				     &ulsch->harq_processes[harq_pid]->d[r][96], 
				     ulsch->harq_processes[harq_pid]->w[r]);
  
    }
    
  }

  sumKr = 0;
  for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
    if (r<ulsch->harq_processes[harq_pid]->Cminus)
      Kr = ulsch->harq_processes[harq_pid]->Kminus;
    else
      Kr = ulsch->harq_processes[harq_pid]->Kplus;
    sumKr += Kr;
  }
  // Compute Q_ri
  Qprime = O_RI*ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;
  if ((Qprime % (8*sumKr)) > 0)
    Qprime = 1+(Qprime/(8*sumKr));
  else
    Qprime = Qprime/(8*sumKr);

  if (Qprime > 4*ulsch->nb_rb * 12)
    Qprime = 4*ulsch->nb_rb * 12;

  Q_RI = Q_m*Qprime;

  // Compute Q_ack

  Qprime = O_ACK*ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;
  if ((Qprime % (8*sumKr)) > 0)
    Qprime = 1+(Qprime/(8*sumKr));
  else
    Qprime = Qprime/(8*sumKr);

  if (Qprime > 4*ulsch->nb_rb * 12)
    Qprime = 4*ulsch->nb_rb * 12;

  Q_ACK = Qprime * Q_m;

  // Compute Q_cqi
  L=8;
  Qprime = (O + L) * ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;
  if ((Qprime % (8*sumKr)) > 0)
    Qprime = 1+(Qprime/(8*sumKr));
  else
    Qprime = Qprime/(8*sumKr);

  G = ulsch->nb_rb * (12 * Q_m) * (ulsch->Nsymb_pusch);

  if (Qprime > (G - O_RI))
    Qprime = G - O_RI;

  Q_CQI = Q_m * Qprime;

  G = G - Q_RI - Q_CQI;
  H = G + Q_CQI;
  Hprime = H/Q_m;

  // Fill in the "e"-sequence from 36-212, V8.6 2009-03, p. 16-17 (for each "e") and concatenate the
  // outputs for each code segment, see Section 5.1.5 p.20

  for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
#ifdef DEBUG_ULSCH_CODING
    printf("Rate Matching, Code segment %d (coded bits (G) %d,unpunctured/repeated bits per code segment %d,mod_order %d, nb_rb %d)...\n",
	   r,
	   G,
	   Kr*3,
	   Q_m,N_RB);
#endif


    r_offset += lte_rate_matching_turbo(ulsch->harq_processes[harq_pid]->RTC[r],
					G,
					ulsch->harq_processes[harq_pid]->w[r],
					&ulsch->e[0],
					ulsch->harq_processes[harq_pid]->C, // C
					NSOFT,                    // Nsoft,
					ulsch->Mdlharq,
					1,
					ulsch->harq_processes[harq_pid]->rvidx,
					get_Qm(ulsch->harq_processes[harq_pid]->mcs),
					ulsch->harq_processes[harq_pid]->Nl,
					r);                       // r
#ifdef DEBUG_ULSCH_CODING
    if (r==ulsch->harq_processes[harq_pid]->C-1)
      write_output("enc_output.m","enc",ulsch->e,r_offset,1,4);
#endif
  }

  //  Do CQI coding
  if (O < 12) {
    msg("ulsch_coding: FATAL, short CQI sizes not supported yet\n");
    exit(-1);
  }
  else {

    // add 8-bit CRC
    crc = crc8(o,
	       O)>>24;
    
    ccodelte_encode(O,
		    1,
		    o,
		    &ulsch->o_d[96],
		    0);
 
    ulsch->o_RCC = sub_block_interleaving_cc(8+O, 
					     &ulsch->o_d[96], 
					     ulsch->o_w);

    E = lte_rate_matching_cc(ulsch->o_RCC,
			     ulsch->Q_cqi,
			     ulsch->o_w,
			     ulsch->q);
    
  }

  //  Do RI coding
  if (O_RI == 1) {
    switch (Q_m) {
    case 2:
      ulsch->q_RI[0] = o_RI[0];
      ulsch->q_RI[1] = o_RI[0];
      break;
    case 4:
      ulsch->q_RI[0] = o_RI[0];
      ulsch->q_RI[1] = 1;
      ulsch->q_RI[2] = o_RI[0];
      ulsch->q_RI[3] = 1;
      break;
    case 6:
      ulsch->q_RI[0] = o_RI[0];
      ulsch->q_RI[1] = 1;
      ulsch->q_RI[2] = 1;
      ulsch->q_RI[3] = o_RI[0];
      ulsch->q_RI[4] = 1;
      ulsch->q_RI[5] = 1;
      break;
    }
  }
  else {
    msg("ulsch_coding: FATAL, RI cannot be more than 1 bit yet\n");
    return(-1);
  }
  //  Do ACK coding
  if (O_ACK == 1) {
    switch (Q_m) {
    case 2:
      ulsch->q_ACK[0] = o_ACK[0];
      ulsch->q_ACK[1] = o_ACK[0];
      break;
    case 4:
      ulsch->q_ACK[0] = o_ACK[0];
      ulsch->q_ACK[1] = 1;
      ulsch->q_ACK[2] = o_ACK[0];
      ulsch->q_ACK[3] = 1;
      break;
    case 6:
      ulsch->q_ACK[0] = o_ACK[0];
      ulsch->q_ACK[1] = 1;
      ulsch->q_ACK[2] = 1;
      ulsch->q_ACK[3] = o_ACK[0];
      ulsch->q_ACK[4] = 1;
      ulsch->q_ACK[6] = 1;
      break;
    }
  }
  if (O_ACK == 2) {
    ack_parity = (o_ACK[0]+o_ACK[1])&1;
    switch (Q_m) {
    case 2:
      ulsch->q_ACK[0] = o_ACK[0];
      ulsch->q_ACK[1] = o_ACK[1];
      ulsch->q_ACK[2] = ack_parity;
      ulsch->q_ACK[3] = o_ACK[0];
      ulsch->q_ACK[4] = o_ACK[1];
      ulsch->q_ACK[5] = ack_parity;
      break;
    case 4:
      ulsch->q_ACK[0]  = o_ACK[0];
      ulsch->q_ACK[1]  = 1;
      ulsch->q_ACK[2]  = o_ACK[1];
      ulsch->q_ACK[3]  = 1;
      ulsch->q_ACK[4]  = ack_parity;
      ulsch->q_ACK[5]  = 1;
      ulsch->q_ACK[6]  = o_ACK[0];
      ulsch->q_ACK[7]  = 1;
      ulsch->q_ACK[8]  = o_ACK[1];
      ulsch->q_ACK[9]  = 1;
      ulsch->q_ACK[10] = ack_parity;
      ulsch->q_ACK[11] = 1;
      break;
    case 6:
      ulsch->q_ACK[0] = o_ACK[0];
      ulsch->q_ACK[1] = 1;
      ulsch->q_ACK[2] = 1;
      ulsch->q_ACK[3] = o_ACK[1];
      ulsch->q_ACK[4] = 1;
      ulsch->q_ACK[5] = 1;

      ulsch->q_ACK[6] = ack_parity;
      ulsch->q_ACK[7] = 1;
      ulsch->q_ACK[8] = 1;
      ulsch->q_ACK[9] = o_ACK[1];
      ulsch->q_ACK[10] = 1;
      ulsch->q_ACK[11] = 1;

      ulsch->q_ACK[12] = o_ACK[1];
      ulsch->q_ACK[13] = 1;
      ulsch->q_ACK[14] = 1;
      ulsch->q_ACK[15] = ack_parity;
      ulsch->q_ACK[16] = 1;
      ulsch->q_ACK[17] = 1;


      break;
    }
  }
  else{
    msg("ulsch_coding: FATAL, ACK cannot be more than 2 bits yet\n");
    return(-1);
  }


  // channel multiplexing/interleaving


  Hpp = Hprime + Q_RI;
 
  Cmux       = ulsch->Nsymb_pusch;
  Rmux       = Hpp*Q_m/Cmux;
  Rmux_prime = Rmux/Q_m;

  Qprime_RI  = Q_RI / Q_m;
  Qprime_ACK = Q_ACK / Q_m;
  Qprime_CQI = Q_CQI / Q_m;


  // RI BITS 
  r = Rmux_prime-1;
  memset(y,0,Q_m*Hpp);

  if (frame_parms->Ncp == 0)
    columnset = cs_ri_normal;
  else
    columnset = cs_ri_extended;
  j=0;   
  for (i=0;i<Qprime_RI;i++) {
    for (q=0;q<Q_m;q++) 
      y[q+(Q_m*((r*Cmux) + columnset[j]))]  = ulsch->q_RI[q+(Q_m*i)];
    j=(j+3)&3;
    r = Rmux_prime - (i>>2);
  }


  // CQI and Data bits
  j=0;
  for (i=0;i<Hprime;i++) {
    if (y[Q_m*j] == 0) {
      if (i<Qprime_CQI) {
	for (q=0;q<Q_m;q++) {
	  y[q+(Q_m*j)] = ulsch->q[q+(Q_m*i)];
	}
      }
      else {
	for (q=0;q<Q_m;q++) {
	  y[q+(Q_m*j)] = ulsch->e[q+(Q_m*i)];
	}
      }
    }
    j++;
  }

  // HARQ-ACK Bits (Note these overwrite some bits)
  r = Rmux_prime-1;

  if (frame_parms->Ncp == 0)
    columnset = cs_ack_normal;
  else
    columnset = cs_ack_extended;

  j=0;
  for (i=0;i<Qprime_ACK;i++) {
    for (q=0;q<Q_m;q++) 
      y[q+(Q_m*((r*Cmux) + columnset[j]))]  = ulsch->q_RI[q+(Q_m*i)];
    j=(j+3)&3;
    r = Rmux_prime - (i>>2);
  }

  // write out buffer
  j=0;
  for (i=0;i<Cmux;i++)
    for (r=0;r<Rmux_prime;r++)
      for (q=0;q<Q_m;q++) 
	ulsch->h[j++] = y[q+(Q_m*((r*Cmux)+i))];

  if (j!=(H+Q_RI))
    msg("ulsch_coding.c: Error in output buffer length (j %d, H+Q_RI %d)\n",j,H+Q_RI); 

  return(0);
}
