/* file: dlsch_coding.c
   purpose: Top-level routines for implementing Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 
*/

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

void free_eNb_dlsch(LTE_eNb_DLSCH_t *dlsch) {
  int i;
  int r;

  if (dlsch) {
    for (i=0;i<dlsch->Mdlharq;i++) {
      if (dlsch->harq_processes[i]) {
	if (dlsch->harq_processes[i]->b)
	  free(dlsch->harq_processes[i]->b);
	if (dlsch->harq_processes[i]->c) {
	  for (r=0;r<8;r++)
	    if (dlsch->harq_processes[i]->c[r]) 
	      	  free(dlsch->harq_processes[i]->c[r]);
	  free(dlsch->harq_processes[i]->c);
	}
	free(dlsch->harq_processes[i]);
      }
    }
    free(dlsch);
  }
  
}

LTE_eNb_DLSCH_t *new_eNb_dlsch(unsigned char Kmimo,unsigned char Mdlharq) {

  LTE_eNb_DLSCH_t *dlsch;
  unsigned char exit_flag = 0,i,j,r;
  
  dlsch = (LTE_eNb_DLSCH_t *)malloc16(sizeof(LTE_eNb_DLSCH_t));
  if (dlsch) {
    dlsch->Kmimo = Kmimo;
    dlsch->Mdlharq = Mdlharq;

    for (i=0;i<Mdlharq;i++) {
      dlsch->harq_processes[i] = (LTE_eNb_HARQ_t *)malloc16(sizeof(LTE_eNb_HARQ_t));
      if (dlsch->harq_processes[i]) {
	dlsch->harq_processes[i]->b          = (unsigned char*)malloc16(MAX_DLSCH_PAYLOAD_BYTES);
	if (!dlsch->harq_processes[i]->b)
	  exit_flag=1;
	for (r=0;r<8;r++) {
	  dlsch->harq_processes[i]->c[r] = (unsigned char*)malloc16(((r==0)?8:0) + 3+(MAX_DLSCH_PAYLOAD_BYTES>>3));  // account for filler in first segment and CRCs for multiple segment case
	  if (!dlsch->harq_processes[i]->c[r])
	    exit_flag=1;
	}
      }	else {
	exit_flag=1;
      }
    }

    if (exit_flag==0) {
      for (i=0;i<8;i++)
	for (j=0;j<96;j++)
	  for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++)
	    dlsch->harq_processes[i]->d[r][j] = LTE_NULL;
      return(dlsch);
    }
  }

  free_eNb_dlsch(dlsch);
  return(NULL);
  
  
}


void dlsch_encoding(unsigned char *a,
		    unsigned short A,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_eNb_DLSCH_t *dlsch,
		    unsigned char harq_pid,
		    unsigned short N_RB) {
  
  unsigned short offset;
  unsigned int coded_bits_per_codeword;
  unsigned int crc=1;
  unsigned short iind;
  unsigned char mod_order = dlsch->harq_processes[harq_pid]->mod_order;
  unsigned int Kr,Kr_bytes,r,r_offset;

  if (dlsch->harq_processes[harq_pid]->active == 0) {  // this is a new packet
    
    // Add 24-bit crc (polynomial A) to payload
    crc = crc24a(a,
		 A)>>8;

    *(unsigned int*)(&a[A>>3]) = crc;
    dlsch->harq_processes[harq_pid]->B = A+24;
    dlsch->harq_processes[harq_pid]->b = a;
    lte_segmentation(dlsch->harq_processes[harq_pid]->b,
		     dlsch->harq_processes[harq_pid]->c,
		     dlsch->harq_processes[harq_pid]->B,
		     &dlsch->harq_processes[harq_pid]->C,
		     &dlsch->harq_processes[harq_pid]->Cplus,
		     &dlsch->harq_processes[harq_pid]->Cminus,
		     &dlsch->harq_processes[harq_pid]->Kplus,
		     &dlsch->harq_processes[harq_pid]->Kminus,		     
		     &dlsch->harq_processes[harq_pid]->F);

    for (r=0;r<dlsch->harq_processes[harq_pid]->C;r++) {
      if (r<dlsch->harq_processes[harq_pid]->Cminus)
	Kr = dlsch->harq_processes[harq_pid]->Kminus;
      else
	Kr = dlsch->harq_processes[harq_pid]->Kplus;
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
	printf("dlsch_coding: Illegal codeword size %d!!!\n",Kr_bytes);
	exit(-1);
      }
    
  

  printf("Generating Code Segment %d (%d bits)\n",r,Kr);
  // generate codewords
  
  printf("bits_per_codeword (Kr)= %d\n",Kr);
  printf("N_RB = %d\n",N_RB);
  printf("first_dlsch_symbol %d\n",frame_parms->first_dlsch_symbol);
  printf("Ncp %d\n",frame_parms->Ncp);
  printf("mod_order %d\n",mod_order);
  offset=0;
  
  // This has to be updated for presence of PBCH/PSCH
  // This assumes no data in pilot symbols (i.e. for multi-cell orthogonality, to be updated for strict LTE compliance
  /*
  coded_bits_per_codeword = (frame_parms->Ncp == 0) ?
    ( N_RB * (12 * mod_order) * (14-frame_parms->first_dlsch_symbol)) - (N_RB*(frame_parms->nb_antennas_tx*6*3*mod_order)) :
    ( N_RB * (12 * mod_order) * (12-frame_parms->first_dlsch_symbol)) - (N_RB*(frame_parms->nb_antennas_tx*6*3*mod_order));
  */
  coded_bits_per_codeword = (frame_parms->Ncp == 0) ?
    ( N_RB * (12 * mod_order) * (14-frame_parms->first_dlsch_symbol-3)) :
    ( N_RB * (12 * mod_order) * (12-frame_parms->first_dlsch_symbol-3)) ;

    
    printf("Encoding ... iind %d f1 %d, f2 %d\n",iind,f1f2mat[iind*2],f1f2mat[(iind*2)+1]);
    threegpplte_turbo_encoder(dlsch->harq_processes[harq_pid]->c[r],
			      Kr>>3, 
			      &dlsch->harq_processes[harq_pid]->d[r][96],
			      (r==0) ? dlsch->harq_processes[harq_pid]->F : 0,
			      f1f2mat[iind*2],   // f1 (see 36121-820, page 14)
			      f1f2mat[(iind*2)+1]  // f2 (see 36121-820, page 14)
			      );
    if (r==0)
      write_output("enc_output0.m","enc0",&dlsch->harq_processes[harq_pid]->d[r][96],(3*8*Kr_bytes)+12,1,4);

      dlsch->harq_processes[harq_pid]->RTC[r] = 
	sub_block_interleaving_turbo(4+(Kr_bytes*8), 
				     &dlsch->harq_processes[harq_pid]->d[r][96], 
				     dlsch->harq_processes[harq_pid]->w[r]);
  
    }
    
  }

  // Fill in the "e"-sequence from 36-212, V8.6 2009-03, p. 16-17 (for each "e") and concatenate the
  // outputs for each code segment, see Section 5.1.5 p.20
  r_offset = 0;
  for (r=0;r<dlsch->harq_processes[harq_pid]->C;r++) {
    printf("Rate Matching, Code segment %d (coded bits (G) %d,unpunctured/repeated bits per code segment %d,mod_order %d, nb_rb %d)...\n",
	   r,
	   coded_bits_per_codeword,
	   Kr*3,
	   mod_order,N_RB);
    

      
    r_offset += lte_rate_matching_turbo(dlsch->harq_processes[harq_pid]->RTC[r],
					coded_bits_per_codeword,  //G
					dlsch->harq_processes[harq_pid]->w[r],
					&dlsch->e[r_offset],
					dlsch->harq_processes[harq_pid]->C, // C
					NSOFT,                    // Nsoft,
					dlsch->Mdlharq,
					dlsch->Kmimo,
					dlsch->rvidx,
					dlsch->harq_processes[harq_pid]->mod_order,
					dlsch->harq_processes[harq_pid]->Nl,
					r);                       // r
    if (r==0)
      write_output("enc_output.m","enc",dlsch->e,(3*8*Kr_bytes)+12,1,4);
  }
  
}
