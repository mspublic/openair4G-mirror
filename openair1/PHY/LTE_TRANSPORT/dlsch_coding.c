/* file: dlsch_coding.c
   purpose: Top-level routines for implementing Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
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

//#define DEBUG_DLSCH_CODING 
//#define DEBUG_DLSCH_FREE 1

/*
#define is_not_pilot(pilots,first_pilot,re) (pilots==0) || \ 
	((pilots==1)&&(first_pilot==1)&&(((re>2)&&(re<6))||((re>8)&&(re<12)))) || \
	((pilots==1)&&(first_pilot==0)&&(((re<3))||((re>5)&&(re<9)))) \
*/
#define is_not_pilot(pilots,first_pilot,re) (1)


void free_eNB_dlsch(LTE_eNB_DLSCH_t *dlsch) {
  int i;
  int r;

  if (dlsch) {
#ifdef DEBUG_DLSCH_FREE
    msg("Freeing dlsch %p\n",dlsch);
#endif
    for (i=0;i<dlsch->Mdlharq;i++) {
#ifdef DEBUG_DLSCH_FREE
      msg("Freeing dlsch process %d\n",i);
#endif
      if (dlsch->harq_processes[i]) {
#ifdef DEBUG_DLSCH_FREE
	msg("Freeing dlsch process %d (%p)\n",i,dlsch->harq_processes[i]);
#endif
	if (dlsch->harq_processes[i]->b) {
	  free16(dlsch->harq_processes[i]->b,MAX_DLSCH_PAYLOAD_BYTES);
#ifdef DEBUG_DLSCH_FREE
	  msg("Freeing dlsch process %d b (%p)\n",i,dlsch->harq_processes[i]->b);
#endif
	}
	if (dlsch->harq_processes[i]->c) {
#ifdef DEBUG_DLSCH_FREE
	  msg("Freeing dlsch process %d c (%p)\n",i,dlsch->harq_processes[i]->c);
#endif
	  for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++) {

#ifdef DEBUG_DLSCH_FREE
	    msg("Freeing dlsch process %d c[%d] (%p)\n",i,r,dlsch->harq_processes[i]->c[r]);
#endif
	    if (dlsch->harq_processes[i]->c[r]) 
	      free16(dlsch->harq_processes[i]->c[r],((r==0)?8:0) + 3+(MAX_DLSCH_PAYLOAD_BYTES));
	  }
	}
	free16(dlsch->harq_processes[i],sizeof(LTE_DL_eNB_HARQ_t));
      }
    }
    free16(dlsch,sizeof(LTE_eNB_DLSCH_t));
  }
  
}

LTE_eNB_DLSCH_t *new_eNB_dlsch(unsigned char Kmimo,unsigned char Mdlharq,u8 abstraction_flag) {

  LTE_eNB_DLSCH_t *dlsch;
  unsigned char exit_flag = 0,i,j,r;
  
  dlsch = (LTE_eNB_DLSCH_t *)malloc16(sizeof(LTE_eNB_DLSCH_t));
  if (dlsch) {
    bzero(dlsch,sizeof(LTE_eNB_DLSCH_t));
    dlsch->Kmimo = Kmimo;
    dlsch->Mdlharq = Mdlharq;
    for (i=0;i<10;i++)
      dlsch->harq_ids[i] = Mdlharq;

    for (i=0;i<Mdlharq;i++) {
      dlsch->harq_processes[i] = (LTE_DL_eNB_HARQ_t *)malloc16(sizeof(LTE_DL_eNB_HARQ_t));
      //printf("dlsch->harq_processes[%d] %p\n",i,dlsch->harq_processes[i]);
      if (dlsch->harq_processes[i]) {
	bzero(dlsch->harq_processes[i],sizeof(LTE_DL_eNB_HARQ_t));
	dlsch->harq_processes[i]->b          = (unsigned char*)malloc16(MAX_DLSCH_PAYLOAD_BYTES);
	if (!dlsch->harq_processes[i]->b) {
	  msg("Can't get b\n");
	  exit_flag=1;
	}
	if (abstraction_flag==0) {
	  for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++) {
	    dlsch->harq_processes[i]->c[r] = (unsigned char*)malloc16(((r==0)?8:0) + 3+(MAX_DLSCH_PAYLOAD_BYTES));  // account for filler in first segment and CRCs for multiple segment case
	    if (!dlsch->harq_processes[i]->c[r]) {
	      msg("Can't get c\n");
	      exit_flag=2;
	    }
	  }
	}
      }	else {
	msg("Can't get harq_p %d\n",i);
	exit_flag=3;
      }
    }

    if ((exit_flag==0)) {
      for (i=0;i<Mdlharq;i++) {
	dlsch->harq_processes[i]->round=0;
	if (abstraction_flag==0) {
	  for (j=0;j<96;j++)
	    for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++)
	      dlsch->harq_processes[i]->d[r][j] = LTE_NULL;
	}
      }
      return(dlsch);
    }
  }
  msg("new_eNB_dlsch exit flag %d, size of  %d\n",exit_flag, sizeof(LTE_eNB_DLSCH_t));
  free_eNB_dlsch(dlsch);
  return(NULL);
  
  
}

void clean_eNb_dlsch(LTE_eNB_DLSCH_t *dlsch, u8 abstraction_flag) {

  unsigned char Kmimo, Mdlharq;
  unsigned char i,j,r;
  
  if (dlsch) {
    Kmimo = dlsch->Kmimo;
    Mdlharq = dlsch->Mdlharq;
    dlsch->rnti = 0;
    dlsch->active = 0;
    for (i=0;i<10;i++)
      dlsch->harq_ids[i] = Mdlharq;

    for (i=0;i<Mdlharq;i++) {
      if (dlsch->harq_processes[i]) {
	dlsch->harq_processes[i]->Ndi    = 0;
	dlsch->harq_processes[i]->status = 0;
	dlsch->harq_processes[i]->round  = 0;
	if (abstraction_flag==0) {
	  for (j=0;j<96;j++)
	    for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++)
	      dlsch->harq_processes[i]->d[r][j] = LTE_NULL;
	}
      }
    }
  }
}


int dlsch_encoding(unsigned char *a,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   u8 num_pdcch_symbols,
		   LTE_eNB_DLSCH_t *dlsch,
		   u8 subframe) {
  
  unsigned short offset;
  unsigned int G;
  unsigned int crc=1;
  unsigned short iind;
  unsigned short nb_rb = dlsch->nb_rb;
  unsigned char harq_pid = dlsch->current_harq_pid;
  unsigned int A; 
  unsigned char mod_order;
  unsigned int Kr,Kr_bytes,r,r_offset=0;

  A = dlsch->harq_processes[harq_pid]->TBS;

  mod_order = get_Qm(dlsch->harq_processes[harq_pid]->mcs);

  G = get_G(frame_parms,nb_rb,dlsch->rb_alloc,mod_order,num_pdcch_symbols,subframe);

   
  if (dlsch->harq_processes[harq_pid]->Ndi == 1) {  // this is a new packet

    /*
    printf("dlsch (tx): \n");
    for (i=0;i<4;i++)
      printf("%x ",a[i]);
    printf("\n");
    */
    // Add 24-bit crc (polynomial A) to payload
    crc = crc24a(a,
		 A)>>8;
    
    a[A>>3] = ((u8*)&crc)[2];
    a[1+(A>>3)] = ((u8*)&crc)[1];
    a[2+(A>>3)] = ((u8*)&crc)[0];

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
	msg("dlsch_coding: Illegal codeword size %d!!!\n",Kr_bytes);
	return(-1);
      }
      
      
#ifdef DEBUG_DLSCH_CODING
      printf("Generating Code Segment %d (%d bits)\n",r,Kr);
      // generate codewords
      
      msg("bits_per_codeword (Kr)= %d, A %d\n",Kr,A);
      msg("N_RB = %d\n",nb_rb);
      msg("Ncp %d\n",frame_parms->Ncp);
      msg("mod_order %d\n",mod_order);
#endif
      
      offset=0;
      
#ifdef DEBUG_DLSCH_CODING    
      msg("Encoding ... iind %d f1 %d, f2 %d\n",iind,f1f2mat[iind*2],f1f2mat[(iind*2)+1]);
#endif
      
      threegpplte_turbo_encoder(dlsch->harq_processes[harq_pid]->c[r],
				Kr>>3, 
				&dlsch->harq_processes[harq_pid]->d[r][96],
				(r==0) ? dlsch->harq_processes[harq_pid]->F : 0,
				f1f2mat[iind*2],   // f1 (see 36121-820, page 14)
				f1f2mat[(iind*2)+1]  // f2 (see 36121-820, page 14)
				);
#ifdef DEBUG_DLSCH_CODING
      if (r==0)
	write_output("enc_output0.m","enc0",&dlsch->harq_processes[harq_pid]->d[r][96],(3*8*Kr_bytes)+12,1,4);
#endif
      
      dlsch->harq_processes[harq_pid]->RTC[r] = 
	sub_block_interleaving_turbo(4+(Kr_bytes*8), 
				     &dlsch->harq_processes[harq_pid]->d[r][96], 
				     dlsch->harq_processes[harq_pid]->w[r]);
      
    }
    
  }

  // Fill in the "e"-sequence from 36-212, V8.6 2009-03, p. 16-17 (for each "e") and concatenate the
  // outputs for each code segment, see Section 5.1.5 p.20

  for (r=0;r<dlsch->harq_processes[harq_pid]->C;r++) {
#ifdef DEBUG_DLSCH_CODING
    msg("Rate Matching, Code segment %d (coded bits (G) %d,unpunctured/repeated bits per code segment %d,mod_order %d, nb_rb %d)...\n",
	   r,
	   G,
	   Kr*3,
	   mod_order,nb_rb);
#endif


    r_offset += lte_rate_matching_turbo(dlsch->harq_processes[harq_pid]->RTC[r],
					G,  //G
					dlsch->harq_processes[harq_pid]->w[r],
					&dlsch->e[0],
					dlsch->harq_processes[harq_pid]->C, // C
					NSOFT,                    // Nsoft,
					dlsch->Mdlharq,
					dlsch->Kmimo,
					dlsch->harq_processes[harq_pid]->rvidx,
					get_Qm(dlsch->harq_processes[harq_pid]->mcs),
					dlsch->harq_processes[harq_pid]->Nl,
					r);                       // r
#ifdef DEBUG_DLSCH_CODING
    if (r==dlsch->harq_processes[harq_pid]->C-1)
      write_output("enc_output.m","enc",dlsch->e,r_offset,1,4);
#endif
  }
  return(0);
}

#ifdef PHY_ABSTRACTION
void dlsch_encoding_emul(PHY_VARS_eNB *phy_vars_eNB,
			 u8 *DLSCH_pdu,
			 LTE_eNB_DLSCH_t *dlsch) {

  // int payload_offset = 0;
  unsigned char harq_pid = dlsch->current_harq_pid;
  unsigned short i;

  if (dlsch->harq_processes[harq_pid]->Ndi == 1) {
    memcpy(dlsch->harq_processes[harq_pid]->b,
	   DLSCH_pdu,
	   dlsch->harq_processes[harq_pid]->TBS>>3);
    msg("[PHY] EMUL eNB %d dlsch_encoding_emul, tbs is %d\n", 
	phy_vars_eNB->Mod_id,
	dlsch->harq_processes[harq_pid]->TBS>>3);

    for (i=0;i<dlsch->harq_processes[harq_pid]->TBS>>3;i++)
      msg("%x.",DLSCH_pdu[i]);
    msg("\n");

    memcpy(&eNB_transport_info[phy_vars_eNB->Mod_id].transport_blocks[eNB_transport_info_TB_index[phy_vars_eNB->Mod_id]],
	   //memcpy(&eNB_transport_info[phy_vars_eNB->Mod_id].transport_blocks[payload_offset],
    	   DLSCH_pdu,
	   dlsch->harq_processes[harq_pid]->TBS>>3);
  }  
  eNB_transport_info_TB_index[phy_vars_eNB->Mod_id]+=dlsch->harq_processes[harq_pid]->TBS>>3;
  //payload_offset +=dlsch->harq_processes[harq_pid]->TBS>>3;
  
}
#endif
