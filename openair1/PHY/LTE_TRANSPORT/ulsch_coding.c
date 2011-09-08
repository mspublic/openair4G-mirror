/* file: ulsch_coding.c
   purpose: Top-level routines for implementing Turbo-coded (ULSCH) transport channels from 36-212, V8.6 2009-03
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 
*/

#include "PHY/defs.h"
#include "PHY/extern.h"

#include "PHY/CODING/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "PHY/LTE_TRANSPORT/defs.h"
#include "defs.h"
#include "extern.h"
#include "SIMULATION/ETH_TRANSPORT/extern.h"

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

LTE_UE_ULSCH_t *new_ue_ulsch(unsigned char Mdlharq,u8 abstraction_flag) {

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
	if (abstraction_flag==0) {
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++) {
	    ulsch->harq_processes[i]->c[r] = (unsigned char*)malloc16(((r==0)?8:0) + 3+(MAX_ULSCH_PAYLOAD_BYTES));  // account for filler in first segment and CRCs for multiple segment case
	    if (!ulsch->harq_processes[i]->c[r]) {
	      msg("Can't get c\n");
	      exit_flag=2;
	    }
	  }
	}
	ulsch->harq_processes[i]->subframe_scheduling_flag = 0;
      }	else {
	msg("Can't get harq_p %d\n",i);
	exit_flag=3;
      }
    }

    if ((abstraction_flag == 0) && (exit_flag==0)) {
      for (i=0;i<Mdlharq;i++)
	for (j=0;j<96;j++)
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++)
	    ulsch->harq_processes[i]->d[r][j] = LTE_NULL;
      return(ulsch);
    }
    else if (abstraction_flag==1)
      return(ulsch);
  }
  msg("new_ue_ulsch exit flag, size of  %d ,   %d\n",exit_flag, sizeof(LTE_UE_ULSCH_t));
  free_ue_ulsch(ulsch);
  return(NULL);
  
  
}


u32 ulsch_encoding(u8 *a,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   LTE_UE_ULSCH_t *ulsch,
		   u8 harq_pid,
		   u8 tmode,
		   u8 control_only_flag) {
  
  unsigned short offset;
  unsigned int crc=1;
  unsigned short iind;
  unsigned short A;
  unsigned char Q_m=0;
  unsigned int Kr=0,Kr_bytes,r,r_offset=0;
  unsigned char y[6*14*1200];
  unsigned char *columnset;
  unsigned int sumKr=0;
  unsigned int Qprime,L,G,Q_CQI=0,Q_RI=0,Q_ACK=0,H=0,Hprime=0,Hpp=0,Cmux=0,Rmux=0,Rmux_prime=0;
  unsigned int Qprime_ACK=0,Qprime_CQI=0,Qprime_RI=0,len_ACK=0,len_RI=0;
  unsigned int E;
  unsigned char ack_parity;
  unsigned int i,q,j,iprime;
  unsigned short o_RCC;
  unsigned char o_flip[8];

  if (!ulsch) {
    msg("ulsch_coding.c: Null ulsch ptr %p\n",ulsch);
    return(-1);
  }

  if (harq_pid > 2) {
    msg("ulsch_coding.c: Illegal harq_pid %d\n",harq_pid);
    return(-1);
  }
    
  if (ulsch->O_ACK > 2)
    {
    msg("ulsch_coding.c: Illegal O_ACK %d\n",ulsch->O_ACK);
    return(-1);
  }

  if (ulsch->O_RI > 1)
    {
    msg("ulsch_coding.c: Illegal O_RI %d\n",ulsch->O_RI);
    return(-1);
  }

  if (ulsch->O<=32) {
    o_flip[0] = ulsch->o[3];
    o_flip[1] = ulsch->o[2];
    o_flip[2] = ulsch->o[1];
    o_flip[3] = ulsch->o[0];   
  }
  else {
    o_flip[0] = ulsch->o[7];
    o_flip[1] = ulsch->o[6];
    o_flip[2] = ulsch->o[5];
    o_flip[3] = ulsch->o[4];
    o_flip[4] = ulsch->o[3];
    o_flip[5] = ulsch->o[2];
    o_flip[6] = ulsch->o[1];
    o_flip[7] = ulsch->o[0];
  }
  if (control_only_flag == 0) {
    A=ulsch->harq_processes[harq_pid]->TBS;
    Q_m = get_Qm(ulsch->harq_processes[harq_pid]->mcs);

    
#ifdef DEBUG_ULSCH_CODING
  msg("[PHY][UE] ULSCH coding : A %d, Qm %d, mcs %d, harq_pid %d, Ndi %d\n",
	 ulsch->harq_processes[harq_pid]->TBS,
	 Q_m,
	 ulsch->harq_processes[harq_pid]->mcs,
	 harq_pid,
	 ulsch->harq_processes[harq_pid]->Ndi);

  for (i=0;i<ulsch->O_ACK;i++)
    msg("ulsch_coding: O_ACK[%d] %d\n",i,ulsch->o_ACK[i]);
  for (i=0;i<ulsch->O_RI;i++)
    msg("ulsch_coding: O_RI[%d] %d\n",i,ulsch->o_RI[i]);
  msg("ulsch_coding: O=%d\n",ulsch->O);
  for (i=0;i<1+((ulsch->O)/8);i++) {
    //    ulsch->o[i] = i;
    msg("ulsch_coding: O[%d] %d\n",i,o_flip[i]);
  }
  if ((tmode != 4))
    print_CQI(ulsch->o,ulsch->o_RI,wideband_cqi,0);
  else
    print_CQI(ulsch->o,ulsch->o_RI,hlc_cqi,0);
#endif
    
    if (ulsch->harq_processes[harq_pid]->Ndi == 1) {  // this is a new packet
      
      // Add 24-bit crc (polynomial A) to payload
      crc = crc24a(a,
		   A)>>8;
      
      a[A>>3] = ((u8*)&crc)[2];
      a[1+(A>>3)] = ((u8*)&crc)[1];
      a[2+(A>>3)] = ((u8*)&crc)[0];
      
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
	msg("Generating Code Segment %d (%d bits)\n",r,Kr);
	// generate codewords
	
	msg("bits_per_codeword (Kr)= %d\n",Kr);
	msg("N_RB = %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
	msg("Ncp %d\n",frame_parms->Ncp);
	msg("Qm %d\n",Q_m);
#endif
	
	offset=0;
	
	
#ifdef DEBUG_ULSCH_CODING    
    msg("Encoding ... iind %d f1 %d, f2 %d\n",iind,f1f2mat[iind*2],f1f2mat[(iind*2)+1]);
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
    
    if (ulsch->harq_processes[harq_pid]->C == 0) {
      msg("[PHY][UE] FATAL : ulsch_coding.c : null segment\n");
      return(-1);
    }
    
    sumKr = 0;
    for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
      if (r<ulsch->harq_processes[harq_pid]->Cminus)
	Kr = ulsch->harq_processes[harq_pid]->Kminus;
      else
      Kr = ulsch->harq_processes[harq_pid]->Kplus;
      sumKr += Kr;
    }
  }
  else { // This is a control-only PUSCH, set sumKr to O_CQI-MIN

    sumKr = ulsch->O_CQI_MIN;
  }
  // Compute Q_ri (p. 23 36-212)

  Qprime = ulsch->O_RI*ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;
  if ((Qprime % (8*sumKr)) > 0)
    Qprime = 1+(Qprime/(8*sumKr));
  else
    Qprime = Qprime/(8*sumKr);

  if (Qprime > 4*ulsch->harq_processes[harq_pid]->nb_rb * 12)
    Qprime = 4*ulsch->harq_processes[harq_pid]->nb_rb * 12;

  Q_RI = Q_m*Qprime;
  Qprime_RI = Qprime;

  // Compute Q_ack (p. 23 36-212)
  Qprime = ulsch->O_ACK*ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;
  if ((Qprime % (8*sumKr)) > 0)
    Qprime = 1+(Qprime/(8*sumKr));
  else
    Qprime = Qprime/(8*sumKr);

  if (Qprime > 4*ulsch->harq_processes[harq_pid]->nb_rb * 12)
    Qprime = 4*ulsch->harq_processes[harq_pid]->nb_rb * 12;

  Q_ACK = Qprime * Q_m;
  Qprime_ACK = Qprime;

  // Compute Q_cqi, assume O>11, p. 26 36-212
  if (control_only_flag == 0) {
    L=8;
    Qprime = (ulsch->O + L) * ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;
    if ((Qprime % (8*sumKr)) > 0)
      Qprime = 1+(Qprime/(8*sumKr));
    else
      Qprime = Qprime/(8*sumKr);
    
    
    G = ulsch->harq_processes[harq_pid]->nb_rb * (12 * Q_m) * (ulsch->Nsymb_pusch);
    //    printf("G %d, Q_RI %d, Q_CQI %d\n",G,Q_RI,Q_CQI);
    if (Qprime > (G - ulsch->O_RI))
      Qprime = G - ulsch->O_RI;
    Q_CQI = Q_m * Qprime;
    Qprime_CQI = Qprime;
  
    G = G - Q_RI - Q_CQI;
    
    if ((int)G < 0) {
      msg("[PHY] FATAL: ulsch_coding.c G < 0 (%d) : Q_RI %d, Q_CQI %d\n",G,Q_RI,Q_CQI);
      return(-1);
    }


    // Data and control multiplexing (5.2.2.7 36-212)

    H = G + Q_CQI;
    Hprime = H/Q_m;



    // Fill in the "e"-sequence from 36-212, V8.6 2009-03, p. 16-17 (for each "e") and concatenate the
    // outputs for each code segment, see Section 5.1.5 p.20
    
    for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
#ifdef DEBUG_ULSCH_CODING
      msg("Rate Matching, Code segment %d (coded bits (G) %d,unpunctured/repeated bits per code segment %d,mod_order %d, nb_rb %d)...\n",
	  r,
	  G,
	  Kr*3,
	  Q_m,ulsch->harq_processes[harq_pid]->nb_rb);
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
					  1,
					  r);                       // r
#ifdef DEBUG_ULSCH_CODING
      if (r==ulsch->harq_processes[harq_pid]->C-1)
	write_output("enc_output.m","enc",ulsch->e,r_offset,1,4);
#endif
    }
  }
  else {  //control-only PUSCH
    Q_CQI = (ulsch->harq_processes[harq_pid]->nb_rb * (12 * Q_m) * (ulsch->Nsymb_pusch)) - Q_RI;
    H = Q_CQI;
    Hprime = H/Q_m;
  }
  

  //  Do CQI coding
  if (ulsch->O < 12) {
    msg("ulsch_coding: FATAL, short CQI sizes not supported yet\n");
    return(-1);
  }
  else {

    // add 8-bit CRC
    crc = crc8(o_flip,
	       ulsch->O)>>24;
    //    printf("crc(cqi) tx : %x\n",crc);
    memset((void *)&ulsch->o_d[0],LTE_NULL,96);
    
    ccodelte_encode(ulsch->O,
		    1,
		    o_flip,
		    &ulsch->o_d[96],
		    0);


    o_RCC = sub_block_interleaving_cc(8+ulsch->O, 
				      &ulsch->o_d[96], 
				      ulsch->o_w);

    E = lte_rate_matching_cc(o_RCC,
			     Q_CQI,
			     ulsch->o_w,
			     ulsch->q);
    
  }

  //  Do RI coding
  if (ulsch->O_RI == 1) {
    switch (Q_m) {
    case 2:
      ulsch->q_RI[0] = ulsch->o_RI[0];
      ulsch->q_RI[1] = ulsch->o_RI[0];
      len_RI=2;
      break;
    case 4:
      ulsch->q_RI[0] = ulsch->o_RI[0];
      ulsch->q_RI[1] = 1;
      ulsch->q_RI[2] = ulsch->o_RI[0];
      ulsch->q_RI[3] = 1;
      len_RI=4;
      break;
    case 6:
      ulsch->q_RI[0] = ulsch->o_RI[0];
      ulsch->q_RI[1] = 1;
      ulsch->q_RI[2] = 1;
      ulsch->q_RI[3] = ulsch->o_RI[0];
      ulsch->q_RI[4] = 1;
      ulsch->q_RI[5] = 1;
      len_RI=6;
      break;
    }
  }
  else {
    msg("ulsch_coding: FATAL, RI cannot be more than 1 bit yet\n");
    return(-1);
  }
  //  Do ACK coding
  if (ulsch->O_ACK == 1) {
    switch (Q_m) {
    case 2:
      ulsch->q_ACK[0] = ulsch->o_ACK[0];
      ulsch->q_ACK[1] = ulsch->o_ACK[0];
      len_ACK = 2;
      break;
    case 4:
      ulsch->q_ACK[0] = ulsch->o_ACK[0];
      ulsch->q_ACK[1] = 1;
      ulsch->q_ACK[2] = ulsch->o_ACK[0];
      ulsch->q_ACK[3] = 1;
      len_ACK = 4;
      break;
    case 6:
      ulsch->q_ACK[0] = ulsch->o_ACK[0];
      ulsch->q_ACK[1] = 1;
      ulsch->q_ACK[2] = 1;
      ulsch->q_ACK[3] = ulsch->o_ACK[0];
      ulsch->q_ACK[4] = 1;
      ulsch->q_ACK[6] = 1;
      len_ACK = 6;
      break;
    }
  }
  if (ulsch->O_ACK == 2) {
    ack_parity = (ulsch->o_ACK[0]+ulsch->o_ACK[1])&1;
    switch (Q_m) {
    case 2:
      ulsch->q_ACK[0] = ulsch->o_ACK[0];
      ulsch->q_ACK[1] = ulsch->o_ACK[1];
      ulsch->q_ACK[2] = ack_parity;
      ulsch->q_ACK[3] = ulsch->o_ACK[0];
      ulsch->q_ACK[4] = ulsch->o_ACK[1];
      ulsch->q_ACK[5] = ack_parity;
      len_ACK = 6;
      break;
    case 4:
      ulsch->q_ACK[0]  = ulsch->o_ACK[0];
      ulsch->q_ACK[1]  = 1;
      ulsch->q_ACK[2]  = ulsch->o_ACK[1];
      ulsch->q_ACK[3]  = 1;
      ulsch->q_ACK[4]  = ack_parity;
      ulsch->q_ACK[5]  = 1;
      ulsch->q_ACK[6]  = ulsch->o_ACK[0];
      ulsch->q_ACK[7]  = 1;
      ulsch->q_ACK[8]  = ulsch->o_ACK[1];
      ulsch->q_ACK[9]  = 1;
      ulsch->q_ACK[10] = ack_parity;
      ulsch->q_ACK[11] = 1;
      len_ACK = 12;
      break;
    case 6:
      ulsch->q_ACK[0] = ulsch->o_ACK[0];
      ulsch->q_ACK[1] = 1;
      ulsch->q_ACK[2] = 1;
      ulsch->q_ACK[3] = ulsch->o_ACK[1];
      ulsch->q_ACK[4] = 1;
      ulsch->q_ACK[5] = 1;

      ulsch->q_ACK[6] = ack_parity;
      ulsch->q_ACK[7] = 1;
      ulsch->q_ACK[8] = 1;
      ulsch->q_ACK[9] = ulsch->o_ACK[0];
      ulsch->q_ACK[10] = 1;
      ulsch->q_ACK[11] = 1;

      ulsch->q_ACK[12] = ulsch->o_ACK[1];
      ulsch->q_ACK[13] = 1;
      ulsch->q_ACK[14] = 1;
      ulsch->q_ACK[15] = ack_parity;
      ulsch->q_ACK[16] = 1;
      ulsch->q_ACK[17] = 1;
      len_ACK = 18;

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

  //  printf("Qprime_CQI = %d\n",Qprime_CQI);
  // RI BITS 
  r = Rmux_prime-1;
  memset(y,LTE_NULL,Q_m*Hpp);

  if (frame_parms->Ncp == 0)
    columnset = cs_ri_normal;
  else
    columnset = cs_ri_extended;
  j=0;   
  for (i=0;i<Qprime_RI;i++) {
    for (q=0;q<Q_m;q++) 
      y[q+(Q_m*((r*Cmux) + columnset[j]))]  = ulsch->q_RI[(q+(Q_m*i))%len_RI];
    j=(j+3)&3;
    r = Rmux_prime - 1 - (i>>2);
  }


  // CQI and Data bits
  j=0;
  for (i=0,iprime=-Qprime_CQI;i<Hprime;i++,iprime++) {
    while (y[Q_m*j] != LTE_NULL)
      j++;
    if (i<Qprime_CQI) {
      for (q=0;q<Q_m;q++) {
	y[q+(Q_m*j)] = ulsch->q[q+(Q_m*i)];
	//		printf("cqi[%d] %d => y[%d]\n",q+(Q_m*i),ulsch->q[q+(Q_m*i)],q+(Q_m*j));
      }
    }
    else {
      for (q=0;q<Q_m;q++) {
	y[q+(Q_m*j)] = ulsch->e[q+(Q_m*iprime)];
	//		printf("e[%d] %d => y[%d]\n",q+(Q_m*iprime),ulsch->e[q+(Q_m*iprime)],q+(Q_m*j));
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
    for (q=0;q<Q_m;q++) {
      y[q+(Q_m*((r*Cmux) + columnset[j]))]  = ulsch->q_ACK[(q+(Q_m*i))%len_ACK];
      //            printf("ACK %d => %d (%d,%d,%d)\n",q+(Q_m*i),ulsch->q_ACK[(q+(Q_m*i))%len_ACK],q+(Q_m*((r*Cmux) + columnset[j])),r,columnset[j]);
    }
    j=(j+3)&3;
    r = Rmux_prime - 1 - (i>>2);
  }

  // write out buffer
  j=0;
  for (i=0;i<Cmux;i++)
    for (r=0;r<Rmux_prime;r++)
      for (q=0;q<Q_m;q++) 
	ulsch->h[j++] = y[q+(Q_m*((r*Cmux)+i))];

  if (j!=(H+Q_RI)) {
    msg("ulsch_coding.c: Error in output buffer length (j %d, H+Q_RI %d)\n",j,H+Q_RI); 
    return(-1);
  }

  return(0);
}


#ifdef PHY_ABSTRACTION
#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
#endif
int ulsch_encoding_emul(u8 *ulsch_buffer,
			PHY_VARS_UE *phy_vars_ue,
			u8 eNB_id,
			u8 harq_pid,
			u8 control_only_flag) {

  LTE_UE_ULSCH_t *ulsch = phy_vars_ue->ulsch_ue[eNB_id];

  msg("[PHY] EMUL UE ulsch_encoding for eNB %d,mod_id %d, harq_pid %d rnti %x, ACK(%d,%d) \n",eNB_id,phy_vars_ue->Mod_id, harq_pid, phy_vars_ue->lte_ue_pdcch_vars[0]->crnti,ulsch->o_ACK[0],ulsch->o_ACK[1]);


  memcpy(phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->b,
	 ulsch_buffer,
	 phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3);

     
  //memcpy(&UE_transport_info[phy_vars_ue->Mod_id].transport_blocks[UE_transport_info_TB_index[phy_vars_ue->Mod_id]],
  memcpy(&UE_transport_info[phy_vars_ue->Mod_id].transport_blocks,
	 ulsch_buffer,
	 phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3);  
  //UE_transport_info_TB_index[phy_vars_ue->Mod_id]+=phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3;
  // navid: currently more than one is not supported in the code 
  UE_transport_info[phy_vars_ue->Mod_id].num_eNB = 1; 
  UE_transport_info[phy_vars_ue->Mod_id].rnti[0] = phy_vars_ue->lte_ue_pdcch_vars[0]->crnti; 
  UE_transport_info[phy_vars_ue->Mod_id].eNB_id[0]  = eNB_id;
  UE_transport_info[phy_vars_ue->Mod_id].harq_pid[0] = harq_pid;
  UE_transport_info[phy_vars_ue->Mod_id].tbs[0]     = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3 ;
  // msg("\nphy_vars_ue->Mod_id%d\n",phy_vars_ue->Mod_id);
  
  UE_transport_info[phy_vars_ue->Mod_id].cntl.pusch_flag = 1;
  UE_transport_info[phy_vars_ue->Mod_id].cntl.pusch_uci = *(u8 *)ulsch->o;
  UE_transport_info[phy_vars_ue->Mod_id].cntl.pusch_ri = (ulsch->o_RI[0]&1)+((ulsch->o_RI[1]&1)<<1);
  UE_transport_info[phy_vars_ue->Mod_id].cntl.pusch_ack =   (ulsch->o_ACK[0]&1) + ((ulsch->o_ACK[1]&1)<<1);
  msg("ack is %d %d %d\n",UE_transport_info[phy_vars_ue->Mod_id].cntl.pusch_ack, (ulsch->o_ACK[1]&1)<<1, ulsch->o_ACK[0]&1);
  return(0);
  
}
#endif
