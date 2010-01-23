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


void free_ue_ulsch(LTE_UL_UE_ULSCH_t *ulsch) {
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
    free16(ulsch,sizeof(LTE_UL_UE_ULSCH_t));
  }
  
}

LTE_UL_UE_ULSCH_t *new_ue_ulsch(unsigned char Mdlharq) {

  LTE_UL_UE_ULSCH_t *ulsch;
  unsigned char exit_flag = 0,i,j,r;
  
  ulsch = (LTE_UL_UE_ULSCH_t *)malloc16(sizeof(LTE_ue_ULSCH_t));
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
  msg("new_ue_ulsch exit flag, size of  %d ,   %d\n",exit_flag, sizeof(LTE_ue_ULSCH_t));
  free_ue_ulsch(ulsch);
  return(NULL);
  
  
}


void ulsch_encoding(unsigned char *a,
		    unsigned short A,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_ue_ULSCH_t *ulsch,
		    unsigned char harq_pid,
		    unsigned short N_RB,
		    unsigned char *o,
		    unsigned char O,
		    unsigned char *o_RI,
		    unsigned char O_RI,
		    unsigned char *o_ACK,
		    unsigned char O_ACK) {
  
  unsigned short offset;
  unsigned int coded_bits_per_codeword;
  unsigned int crc=1;
  unsigned short iind;
  unsigned char mod_order = ulsch->harq_processes[harq_pid]->mod_order;
  unsigned int Kr,Kr_bytes,r,r_offset=0;

  if (ulsch->harq_processes[harq_pid]->active == 0) {  // this is a new packet
    
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
  printf("mod_order %d\n",mod_order);
#endif

  offset=0;
  
  // This has to be updated for presence of PBCH/PSCH
  // This assumes no data in pilot symbols (i.e. for multi-cell orthogonality, to be updated for strict LTE compliance
  /*
  coded_bits_per_codeword = (frame_parms->Ncp == 0) ?
    ( N_RB * (12 * mod_order) * (14-frame_parms->first_ulsch_symbol)) - (N_RB*(frame_parms->nb_antennas_tx*6*3*mod_order)) :
    ( N_RB * (12 * mod_order) * (12-frame_parms->first_ulsch_symbol)) - (N_RB*(frame_parms->nb_antennas_tx*6*3*mod_order));
  */

  // Compute Q_cqi, Q_ri, Q_ack

  coded_bits_per_codeword = (frame_parms->Ncp == 0) ?
    ( N_RB * (12 * mod_order) * (14-frame_parms->first_ulsch_symbol-3)) :
    ( N_RB * (12 * mod_order) * (12-frame_parms->first_ulsch_symbol-3)) ;  //number of bits per subframe = Number_of_resource_blocks*number_of_subcarriers per subblock(12)*bits_per_symbol*number_of_OFDM_Symbols_per_subframe



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

  // Fill in the "e"-sequence from 36-212, V8.6 2009-03, p. 16-17 (for each "e") and concatenate the
  // outputs for each code segment, see Section 5.1.5 p.20

  for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
#ifdef DEBUG_ULSCH_CODING
    printf("Rate Matching, Code segment %d (coded bits (G) %d,unpunctured/repeated bits per code segment %d,mod_order %d, nb_rb %d)...\n",
	   r,
	   coded_bits_per_codeword,
	   Kr*3,
	   mod_order,N_RB);
#endif


    r_offset += lte_rate_matching_turbo(ulsch->harq_processes[harq_pid]->RTC[r],
					coded_bits_per_codeword,  //G
					ulsch->harq_processes[harq_pid]->w[r],
					&ulsch->e[0],
					ulsch->harq_processes[harq_pid]->C, // C
					NSOFT,                    // Nsoft,
					ulsch->Mdlharq,
					1,
					ulsch->rvidx,
					ulsch->harq_processes[harq_pid]->mod_order,
					ulsch->harq_processes[harq_pid]->Nl,
					r);                       // r
#ifdef DEBUG_ULSCH_CODING
    if (r==ulsch->harq_processes[harq_pid]->C-1)
      write_output("enc_output.m","enc",ulsch->e,r_offset,1,4);
#endif
  }

  //  Do CQI coding
  if (O_CQI < 12) {
    msg("ulsch_coding: FATAL, short CQI sizes not supported yet\n");
    exit(-1);
  }
  else {
    
    crc = crc8a(ulsch->o,
		O_CQI)>>24;
    
    ccodelte_encode(O_CQI+8,
		    ulsch->o,
		    (unsigned char *)&crc,
		    ulsch->q);
 
    ulsch->o_RCC = sub_block_interleaving_cc(8+O_CQI, 
					     &ulsch->o_d[96], 
					     ulsch->o_w);

    E = lte_rate_matching_cc(ulsch->o_RCC,
			     ulsch->Q_cqi,
			     ulsch->o_w,
			     ulsch->o_e)
    
  }

  //  Do RI coding

  //  Do ACK coding
}
