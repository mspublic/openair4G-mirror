#include <string.h>
#include "defs.h"
#include "PHY/defs.h"
#include "PHY/CODING/extern.h"

void free_ue_dlsch(LTE_UE_DLSCH_t *dlsch) {

  int i,r;

  if (dlsch) {
    for (i=0;i<dlsch->Mdlharq;i++) {
      if (dlsch->harq_processes[i]) {
	if (dlsch->harq_processes[i]->payload)
	  free(dlsch->harq_processes[i]->payload);
	if (dlsch->harq_processes[i]->payload_segments) {
	  for (r=0;r<8;r++)
	    free(dlsch->harq_processes[i]->payload_segments[r]);
	  free(dlsch->harq_processes[i]->payload_segments);
	}
	free(dlsch->harq_processes[i]);
      }
    }
    free(dlsch);
  }
}

LTE_UE_DLSCH_t *new_ue_dlsch(unsigned char Kmimo,unsigned char Mdlharq) {

  LTE_UE_DLSCH_t *dlsch;
  unsigned char exit_flag = 0,i,r;

  dlsch = (LTE_UE_DLSCH_t *)malloc16(sizeof(LTE_UE_DLSCH_t));
  if (dlsch) {
    dlsch->Kmimo = Kmimo;
    dlsch->Mdlharq = Mdlharq;

    for (i=0;i<Mdlharq;i++) {
      dlsch->harq_processes[i] = (LTE_UE_HARQ_t *)malloc16(sizeof(LTE_UE_HARQ_t));
      if (dlsch->harq_processes[i]) {
	dlsch->harq_processes[i]->payload = (unsigned char*)malloc16(MAX_DLSCH_PAYLOAD_BYTES);
	if (!dlsch->harq_processes[i]->payload)
	  exit_flag=1;
	for (r=0;r<8;r++) {
	  dlsch->harq_processes[i]->payload_segments[r] = (unsigned char*)malloc16((r==0)?8:0) + 3 + (MAX_DLSCH_PAYLOAD_BYTES/8);	
	  if (!dlsch->harq_processes[i]->payload_segments[r])
	    exit_flag=1;
	}
      
      }	else {
	exit_flag=1;
      }
    }
    if (exit_flag==0)
      return(dlsch);
  }
  free_ue_dlsch(dlsch);

  return(NULL);
}

void  dlsch_decoding(LTE_UE_DLSCH *lte_ue_dlsch_vars,
		     LTE_DL_FRAME_PARMS *lte_frame_parms,
		     LTE_UE_DLSCH_t *dlsch,
		     unsigned char harq_pid,
		     //		     unsigned short block_length,
		     unsigned char nb_rb) {
		     
  unsigned short block_length;
  unsigned char mod_order = dlsch->harq_processes[harq_pid]->mod_order;
  unsigned char **decoded_output = dlsch->harq_processes[harq_pid]->payload_segments;


  unsigned int coded_bits_per_codeword,i;
  unsigned int ret;
  unsigned short iind;
  short d[8][(3*8*6144)+12+96] __attribute__ ((aligned(16)));
  //  unsigned char dummy_channel_output[(3*8*block_length)+12];
  short *dlsch_llr = lte_ue_dlsch_vars->llr;
  short coded_bits=0;
  unsigned char dummy_w[8][3*6144];
  unsigned int r,r_offset=0,Kr,Kr_bytes;
  unsigned char crc_type;

  // This has to be updated for presence of PDCCH and PBCH
  coded_bits_per_codeword = (lte_frame_parms->Ncp == 0) ?
    ( nb_rb * (12 * mod_order) * (14-lte_frame_parms->first_dlsch_symbol-3)) :
    ( nb_rb * (12 * mod_order) * (12-lte_frame_parms->first_dlsch_symbol-3)) ;


  if (dlsch->harq_processes[harq_pid]->active == 0) {
    // This is a new packet, so compute quantities regarding segmentation
    lte_segmentation(NULL,
		     NULL,
		     dlsch->harq_processes[harq_pid]->payload_size_bytes<<3,
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
    Kr_bytes = Kr<<3;
    
    if (Kr_bytes<=64)
      iind = (Kr_bytes-5);
    else if (Kr_bytes <=128)
      iind = 59 + ((Kr_bytes-64)>>1);
    else if (Kr_bytes <= 256)
      iind = 91 + ((Kr_bytes-128)>>2);
    else if (Kr_bytes <= 768)
      iind = 123 + ((Kr_bytes-256)>>3);
    else {
      printf("dlsch_decoding: Illegal codeword size %d!!!\n",Kr_bytes);
      exit(-1);
    }
  
     
    printf("f1 %d, f2 %d\n",f1f2mat[2*iind],f1f2mat[1+(2*iind)]);
    dlsch->harq_processes[harq_pid]->RTC[r] = generate_dummy_w(4+(Kr_bytes*8), 
							       dummy_w[r]);

    printf("Rate Matching (coded bits %d,unpunctured/repeated bits %d, mod_order %d, nb_rb %d)...\n",
	   coded_bits_per_codeword,
	   (3*8*block_length)+12,
	   mod_order,
	   nb_rb);



    r_offset += lte_rate_matching_turbo_rx(dlsch->harq_processes[harq_pid]->RTC[r],
					   coded_bits_per_codeword,
					   dlsch->harq_processes[harq_pid]->w[r],
					   dummy_w[r],
					   &dlsch_llr[r_offset],
					   1,
					   NSOFT,
					   dlsch->Mdlharq,
					   dlsch->Kmimo,
					   dlsch->rvidx,
					   dlsch->harq_processes[harq_pid]->mod_order,
					   dlsch->harq_processes[harq_pid]->Nl,
					   0);
    
    sub_block_deinterleaving_turbo(4+Kr, 
				   &d[r][96], 
				   dlsch->harq_processes[harq_pid]->w[r]); 
    
    if (r==0) {
      write_output("decoder_llr.m","decllr",dlsch_llr,coded_bits_per_codeword,1,0);
      write_output("decoder_in.m","dec",&d[96],(3*8*Kr_bytes)+12,1,0);
      printf("decoder input :");
      for (i=0;i<(3*8*Kr_bytes)+12;i++)
	printf("%d : %d\n",i,d[96+i]);
      printf("\n");
    }
    
    memset(decoded_output,0,16);//block_length);
    
    if (dlsch->harq_processes[harq_pid]->C == 1) 
      crc_type = CRC24_A;
    else 
      crc_type = CRC24_B;

       

    ret = phy_threegpplte_turbo_decoder(&d[r][96],
					decoded_output[r],
					Kr,
					f1f2mat[iind*2],   
					f1f2mat[(iind*2)+1], 
					MAX_TURBO_ITERATIONS,
					crc_type);

    if (ret==MAX_TURBO_ITERATIONS) // a Code segment is in error so break;
      break;
  }
  // Reassembly of packets

}
