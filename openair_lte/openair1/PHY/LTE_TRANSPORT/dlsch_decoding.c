#include "defs.h"
#include "PHY/defs.h"
#include "PHY/CODING/extern.h"

//#define DEBUG_DLSCH_DECODING
void free_DL_ue_dlsch(LTE_DL_UE_DLSCH_t *dlsch) {

  int i,r;

  if (dlsch) {
    for (i=0;i<dlsch->Mdlharq;i++) {
      if (dlsch->harq_processes[i]) {
	if (dlsch->harq_processes[i]->b)
	  free16(dlsch->harq_processes[i]->b,MAX_DLSCH_PAYLOAD_BYTES);
	if (dlsch->harq_processes[i]->c) {
	  for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++)
	    free16(dlsch->harq_processes[i]->c[r],((r==0)?8:0) + 768);
	}
	for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++)
	  if (dlsch->harq_processes[i]->d[r])
	    free16(dlsch->harq_processes[i]->d[r],((3*8*6144)+12+96)*sizeof(short));
	free16(dlsch->harq_processes[i],sizeof(LTE_DL_UE_HARQ_t));
      }
    }
  free16(dlsch,sizeof(LTE_DL_UE_DLSCH_t));
  }
}

LTE_DL_UE_DLSCH_t *new_DL_ue_dlsch(unsigned char Kmimo,unsigned char Mdlharq) {

  LTE_DL_UE_DLSCH_t *dlsch;
  unsigned char exit_flag = 0,i,r;

  dlsch = (LTE_DL_UE_DLSCH_t *)malloc16(sizeof(LTE_DL_UE_DLSCH_t));
  if (dlsch) {
    dlsch->Kmimo = Kmimo;
    dlsch->Mdlharq = Mdlharq;

    for (i=0;i<Mdlharq;i++) {
      //      printf("new_ue_dlsch: Harq process %d\n",i);
      dlsch->harq_processes[i] = (LTE_DL_UE_HARQ_t *)malloc16(sizeof(LTE_DL_UE_HARQ_t));
      if (dlsch->harq_processes[i]) {
	dlsch->harq_processes[i]->b = (unsigned char*)malloc16(MAX_DLSCH_PAYLOAD_BYTES);
	if (!dlsch->harq_processes[i]->b)
	  exit_flag=3;
	for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++) {
	  dlsch->harq_processes[i]->c[r] = (unsigned char*)malloc16(((r==0)?8:0) + 768);	
	  if (!dlsch->harq_processes[i]->c[r])
	    exit_flag=2;
	  dlsch->harq_processes[i]->d[r] = (unsigned short*)malloc16(((3*8*6144)+12+96)*sizeof(short));
	}
      
      }	else {
	exit_flag=1;
      }
    }

    if (exit_flag==0)
      return(dlsch);
  }
  msg("new_ue_dlsch: exit_flag = %d\n",exit_flag);
  free_DL_ue_dlsch(dlsch);

  return(NULL);
}

unsigned int  dlsch_decoding(unsigned short A,
			     short *dlsch_llr,
			     LTE_DL_FRAME_PARMS *lte_frame_parms,
			     LTE_DL_UE_DLSCH_t *dlsch,
			     unsigned char harq_pid,
			     unsigned char nb_rb) {
  
  unsigned char mod_order = dlsch->harq_processes[harq_pid]->mod_order;
  unsigned int coded_bits_per_codeword,i;
  unsigned int ret,offset;
  unsigned short iind;
  //  unsigned char dummy_channel_output[(3*8*block_length)+12];
  short coded_bits=0;
  short dummy_w[8][3*(6144+64)];
  unsigned int r,r_offset=0,Kr,Kr_bytes;
  unsigned char crc_type;


  // This has to be updated for presence of PDCCH and PBCH
  coded_bits_per_codeword = (lte_frame_parms->Ncp == 0) ?
    ( nb_rb * (12 * mod_order) * (14-lte_frame_parms->first_dlsch_symbol-3)) :
    ( nb_rb * (12 * mod_order) * (12-lte_frame_parms->first_dlsch_symbol-3)) ;


  if (dlsch->harq_processes[harq_pid]->active == 0) {
    // This is a new packet, so compute quantities regarding segmentation
    dlsch->harq_processes[harq_pid]->B = A+24;
    lte_segmentation(NULL,
		     NULL,
		     dlsch->harq_processes[harq_pid]->B,
		     &dlsch->harq_processes[harq_pid]->C,
		     &dlsch->harq_processes[harq_pid]->Cplus,
		     &dlsch->harq_processes[harq_pid]->Cminus,
		     &dlsch->harq_processes[harq_pid]->Kplus,
		     &dlsch->harq_processes[harq_pid]->Kminus,		     
		     &dlsch->harq_processes[harq_pid]->F);
    //  CLEAR LLR's HERE for first packet in process
  }


  r_offset = 0;
  for (r=0;r<dlsch->harq_processes[harq_pid]->C;r++) {

    // Get Turbo interleaver parameters
    if (r<dlsch->harq_processes[harq_pid]->Cminus)
      Kr = dlsch->harq_processes[harq_pid]->Kminus;
    else
      Kr = dlsch->harq_processes[harq_pid]->Kplus;
    Kr_bytes = Kr>>3;
    
    if (Kr_bytes<=64)
      iind = (Kr_bytes-5);
    else if (Kr_bytes <=128)
      iind = 59 + ((Kr_bytes-64)>>1);
    else if (Kr_bytes <= 256)
      iind = 91 + ((Kr_bytes-128)>>2);
    else if (Kr_bytes <= 768)
      iind = 123 + ((Kr_bytes-256)>>3);
    else {
      msg("dlsch_decoding: Illegal codeword size %d!!!\n",Kr_bytes);
      return(-1);
    }
  
#ifdef DEBUG_DLSCH_DECODING     
    printf("f1 %d, f2 %d, F %d\n",f1f2mat[2*iind],f1f2mat[1+(2*iind)],(r==0) ? dlsch->harq_processes[harq_pid]->F : 0);
#endif

    memset(dummy_w[r],0,3*(6144+64)*sizeof(short));
    dlsch->harq_processes[harq_pid]->RTC[r] = generate_dummy_w(4+(Kr_bytes*8), 
							       dummy_w[r],
							       (r==0) ? dlsch->harq_processes[harq_pid]->F : 0);

#ifdef DEBUG_DLSCH_DECODING    
    printf("Rate Matching Segment %d (coded bits %d,unpunctured/repeated bits %d, mod_order %d, nb_rb %d, Nl %d)...\n",
	   r, coded_bits_per_codeword,
	   Kr*3,
	   mod_order,
	   nb_rb,
	   dlsch->harq_processes[harq_pid]->Nl);
#endif    

    r_offset += lte_rate_matching_turbo_rx(dlsch->harq_processes[harq_pid]->RTC[r],
					   coded_bits_per_codeword,
			 		   dlsch->harq_processes[harq_pid]->w[r],
					   dummy_w[r],
					   dlsch_llr,
					   dlsch->harq_processes[harq_pid]->C,
					   NSOFT,
					   dlsch->Mdlharq,
					   dlsch->Kmimo,
					   dlsch->rvidx,
					   dlsch->harq_processes[harq_pid]->mod_order,
					   dlsch->harq_processes[harq_pid]->Nl,
					   r);
    /*
    printf("Subblock deinterleaving, d %p w %p\n",
	   dlsch->harq_processes[harq_pid]->d[r],
	   dlsch->harq_processes[harq_pid]->w);
    */

    sub_block_deinterleaving_turbo(4+Kr, 
				   &dlsch->harq_processes[harq_pid]->d[r][96], 

				   dlsch->harq_processes[harq_pid]->w[r]); 

    /*
#ifdef DEBUG_DLSCH_DECODING    
    if (r==0) {
      write_output("decoder_llr.m","decllr",dlsch_llr,coded_bits_per_codeword,1,0);
      write_output("decoder_in.m","dec",&dlsch->harq_processes[harq_pid]->d[0][96],(3*8*Kr_bytes)+12,1,0);
    }

    printf("decoder input(segment %d) :",r);
    for (i=0;i<(3*8*Kr_bytes)+12;i++)
      printf("%d : %d\n",i,dlsch->harq_processes[harq_pid]->d[r][96+i]);
    printf("\n");
#endif
    */

    //    printf("Clearing c, %p\n",dlsch->harq_processes[harq_pid]->c[r]);
    //    memset(dlsch->harq_processes[harq_pid]->c[r],0,16);//block_length);
    //    printf("done\n");
    if (dlsch->harq_processes[harq_pid]->C == 1) 
      crc_type = CRC24_A;
    else 
      crc_type = CRC24_B;

    /*        
    printf("decoder input(segment %d)\n",r);
    for (i=0;i<(3*8*Kr_bytes)+12;i++)
      if ((dlsch->harq_processes[harq_pid]->d[r][96+i]>7) || 
	  (dlsch->harq_processes[harq_pid]->d[r][96+i] < -8))
	printf("%d : %d\n",i,dlsch->harq_processes[harq_pid]->d[r][96+i]);
    printf("\n");
    */
    
    ret = phy_threegpplte_turbo_decoder(&dlsch->harq_processes[harq_pid]->d[r][96],
					dlsch->harq_processes[harq_pid]->c[r],
					Kr,
					f1f2mat[iind*2],   
					f1f2mat[(iind*2)+1], 
					MAX_TURBO_ITERATIONS,
					crc_type,
					(r==0) ? dlsch->harq_processes[harq_pid]->F : 0);
    

    if (ret==(1+MAX_TURBO_ITERATIONS)) {// a Code segment is in error so break;
      //      printf("CRC failed\n");
      return(ret);
    }
  }
  // Reassembly of Transport block here
  offset = 0;
  //  printf("F %d, Fbytes %d\n",dlsch->harq_processes[harq_pid]->F,dlsch->harq_processes[harq_pid]->F>>3);
  
  for (r=0;r<dlsch->harq_processes[harq_pid]->C;r++) {
    if (r<dlsch->harq_processes[harq_pid]->Cminus)
      Kr = dlsch->harq_processes[harq_pid]->Kminus;
    else
      Kr = dlsch->harq_processes[harq_pid]->Kplus;

    Kr_bytes = Kr>>3;
    
    if (r==0) {
      memcpy(dlsch->harq_processes[harq_pid]->b,
	     &dlsch->harq_processes[harq_pid]->c[0][(dlsch->harq_processes[harq_pid]->F>>3)],
	     Kr_bytes - (dlsch->harq_processes[harq_pid]->F>>3));
      offset = Kr_bytes - (dlsch->harq_processes[harq_pid]->F>>3);
      //            printf("copied %d bytes to b sequence\n",
      //      	     Kr_bytes - (dlsch->harq_processes[harq_pid]->F>>3));
    }
    else {
      memcpy(dlsch->harq_processes[harq_pid]->b+offset,
	     dlsch->harq_processes[harq_pid]->c[0],
	     Kr_bytes);
      offset += Kr_bytes;
    }
  }
  
  return(ret);
}
