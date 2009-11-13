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

  if (dlsch) {
    for (i=0;i<dlsch->Mdlharq;i++) {
      if (dlsch->harq_processes[i]) {
	if (dlsch->harq_processes[i]->payload)
	  free(dlsch->harq_processes[i]->payload);
	if (dlsch->harq_processes[i]->payload_segments) {
	  for (r=0;r<8;r++)
	    if (dlsch->harq_processes[i]->payload_segments[r]) 
	      	  free(dlsch->harq_processes[i]->payload_segments[r]);
	  free(dlsch->harq_processes[i]->payload_segments);
	}
	free(dlsch->harq_processes[i]);
      }
    }
    free(dlsch);
  }
  
}

LTE_eNb_DLSCH_t *new_eNb_dlsch(unsigned char Kmimo,unsigned char Mdlharq,unsigned char crc_len) {

  LTE_eNb_DLSCH_t *dlsch;
  unsigned char exit_flag = 0,i,j,r;

  dlsch = (LTE_eNb_DLSCH_t *)malloc16(sizeof(LTE_eNb_DLSCH_t));
  if (dlsch) {
    dlsch->Kmimo = Kmimo;
    dlsch->Mdlharq = Mdlharq;
    dlsch->crc_len = crc_len;

    for (i=0;i<Mdlharq;i++) {
      dlsch->harq_processes[i] = (LTE_eNb_HARQ_t *)malloc16(sizeof(LTE_eNb_HARQ_t));
      if (dlsch->harq_processes[i]) {
	dlsch->harq_processes[i]->payload          = (unsigned char*)malloc16(MAX_DLSCH_PAYLOAD_BYTES);
	if (!dlsch->harq_processes[i]->payload)
	  exit_flag=1;
	dlsch->harq_processes[i]->payload_segments = (unsigned char*)malloc16(sizeof(unsigned char*));
	if (!dlsch->harq_processes[i]->payload_segments)
	  exit_flag=1;	
	for (r=0;r<8;r++) {
	  dlsch->harq_processes[i]->payload_segments[r] = (unsigned char*)malloc16(((r==0)?8:0) + 3+(MAX_DLSCH_PAYLOAD_BYTES>>3));  // account for filler in first segment and CRCs for multiple segment case
	if (!dlsch->harq_processes[i]->payload_segments[r])
	  exit_flag=1;
	}
      }	else {
	exit_flag=1;
      }
    }

    if (exit_flag==0) {
      for (i=0;i<8;i++)
	for (j=0;j<96;j++)
	  for (r=0;r<3;r++)
	    dlsch->harq_processes[i]->d[r][j] = LTE_NULL;
      return(dlsch);
    }
  }

  free_eNb_dlsch(dlsch);
  return(NULL);
  
  
}


void dlsch_encoding(unsigned char *input_buffer,
		    unsigned short input_buffer_length,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_eNb_DLSCH_t *dlsch,
		    unsigned char harq_pid,
		    unsigned short N_RB) {
		    // char *input_data
		    //		    unsigned char mod_order,
		    //		    MIMO_mode_t mimo_mode,
		    //		    unsigned char rmseed,
		    //		    unsigned char crc_len) {
  
  unsigned short bytes_per_codeword=dlsch->harq_processes[harq_pid]->payload_size_bytes,offset;
  unsigned int coded_bits_per_codeword;
  unsigned int crc=1;
  unsigned char *input;
  unsigned short iind;
  unsigned char mod_order = dlsch->harq_processes[harq_pid]->mod_order;

  if (dlsch->harq_processes[harq_pid]->active == 0) {  // this is a new packet
    
    input = dlsch->harq_processes[harq_pid]->payload;
    switch (dlsch->crc_len) {
      
    case 1:
      crc = crc8(input,
		 (bytes_per_codeword-1)<<3)>>24;
      break;
    case 2:
      crc = crc16(input,
		  (bytes_per_codeword-2)<<3)>>16;
      break;
    case 3:
      crc = crc24a(input,
		  (bytes_per_codeword-3)<<3)>>8;
      break;
    default:
      printf("Illegal crc_len %d\n",dlsch->crc_len);
      break;
      
    }
    
    if (dlsch->crc_len > 0)
      *(unsigned int*)(&input[bytes_per_codeword-dlsch->crc_len]) = crc;

    lte_segmentation(input,
		     dlsch->harq_processes[harq_pid]->payload_segments,
		     &dlsch->harq_processes[harq_pid]->C,
		     &dlsch->harq_processes[harq_pid]->Cplus,
		     &dlsch->harq_processes[harq_pid]->Cminus,
		     &dlsch->harq_processes[harq_pid]->Kplus,
		     &dlsch->harq_processes[harq_pid]->Kminus,		     
		     &dlsch->harq_processes[harq_pid]->F);
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
  
  printf("bytes_per_codeword = %d\n",bytes_per_codeword);
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
    threegpplte_turbo_encoder(input,
			      bytes_per_codeword, 
			      &dlsch->harq_processes[harq_pid]->d[0][96], 
			      f1f2mat[iind*2],   // f1 (see 36121-820, page 14)
			      f1f2mat[(iind*2)+1]  // f2 (see 36121-820, page 14)
			      );
    write_output("enc_output0.m","enc0",&dlsch->harq_processes[harq_pid]->d[0][96],(3*8*bytes_per_codeword)+12,1,4);

    dlsch->harq_processes[harq_pid]->RTC = 
      sub_block_interleaving_turbo(4+(bytes_per_codeword*8), 
				   &dlsch->harq_processes[harq_pid]->d[0][96], 
				   dlsch->harq_processes[harq_pid]->w[0]);
  
  }
    
  printf("Rate Matching (coded bits %d,unpunctured/repeated bits %d,mod_order %d, nb_rb %d)...\n",
	 coded_bits_per_codeword,
	 (3*8*bytes_per_codeword)+12,
	 mod_order,N_RB);
  
  /*
    rate_matching(coded_bits_per_codeword,
    (3*8*bytes_per_codeword)+12,
    output,
    1,
    rmseed);
  */
  
  lte_rate_matching_turbo(dlsch->harq_processes[harq_pid]->RTC,
			  coded_bits_per_codeword,  //G
			  dlsch->harq_processes[harq_pid]->w[0],
			  dlsch->e[0],
			  1,                        // C
			  NSOFT,                    // Nsoft,
			  dlsch->Mdlharq,
			  dlsch->Kmimo,
			  dlsch->rvidx,
			  dlsch->harq_processes[harq_pid]->mod_order,
			  dlsch->harq_processes[harq_pid]->Nl,
			  0);                       // r
  
  write_output("enc_output.m","enc",dlsch->e[0],(3*8*bytes_per_codeword)+12,1,4);

}
