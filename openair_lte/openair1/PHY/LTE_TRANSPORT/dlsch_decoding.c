#include <string.h>
#include "defs.h"
#include "PHY/defs.h"
#include "PHY/CODING/extern.h"

void free_ue_dlsch(LTE_UE_DLSCH_t *dlsch) {

  int i;

  if (dlsch) {
    for (i=0;i<dlsch->Mdlharq;i++) {
      if (dlsch->harq_processes[i]) {
	if (dlsch->harq_processes[i]->payload)
	  free(dlsch->harq_processes[i]->payload);
	free(dlsch->harq_processes[i]);
      }
    }
    free(dlsch);
  }
}

LTE_UE_DLSCH_t *new_ue_dlsch(unsigned char Kmimo,unsigned char Mdlharq,unsigned char crc_len) {

  LTE_UE_DLSCH_t *dlsch;
  unsigned char exit_flag = 0,i;

  dlsch = (LTE_UE_DLSCH_t *)malloc16(sizeof(LTE_UE_DLSCH_t));
  if (dlsch) {
    dlsch->Kmimo = Kmimo;
    dlsch->Mdlharq = Mdlharq;
    dlsch->crc_len = crc_len;

    for (i=0;i<Mdlharq;i++) {
      dlsch->harq_processes[i] = (LTE_UE_HARQ_t *)malloc16(sizeof(LTE_UE_HARQ_t));
      if (dlsch->harq_processes[i]) {
	dlsch->harq_processes[i]->payload = (unsigned char*)malloc16(MAX_DLSCH_PAYLOAD_BYTES);
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
		     
		     //		     unsigned char mod_order,
		     //		     unsigned char *decoded_output,
		     //		     unsigned int rmseed,
		     //		     unsigned char crc_len) {

  unsigned short block_length = dlsch->harq_processes[harq_pid]->payload_size_bytes;
  unsigned char mod_order = dlsch->harq_processes[harq_pid]->mod_order;
  unsigned char *decoded_output = dlsch->harq_processes[harq_pid]->payload;
  unsigned char crc_len = dlsch->crc_len;

  unsigned int coded_bits_per_codeword,i;
  unsigned int ret;
  unsigned short iind;
  short d[(3*8*block_length)+12+96] __attribute__ ((aligned(16)));
  //  unsigned char dummy_channel_output[(3*8*block_length)+12];
  short *dlsch_llr = lte_ue_dlsch_vars->llr;
  short coded_bits=0;
  unsigned char dummy_w[3][3*6144];

  if (block_length<=64)
    iind = (block_length-5);
  else if (block_length <=128)
    iind = 59 + ((block_length-64)>>1);
  else if (block_length <= 256)
    iind = 91 + ((block_length-128)>>2);
  else if (block_length <= 768)
    iind = 123 + ((block_length-256)>>3);
  else {
    printf("Illegal codeword size !!!\n");
    exit(-1);
  }

  printf("f1 %d, f2 %d\n",f1f2mat[2*iind],f1f2mat[1+(2*iind)]);
  // This has to be updated for presence of PDCCH and PBCH
  coded_bits_per_codeword = (lte_frame_parms->Ncp == 0) ?
    ( nb_rb * (12 * mod_order) * (14-lte_frame_parms->first_dlsch_symbol-3)) :
    ( nb_rb * (12 * mod_order) * (12-lte_frame_parms->first_dlsch_symbol-3)) ;

  printf("Rate Matching (coded bits %d,unpunctured/repeated bits %d, mod_order %d, nb_rb %d)...\n",
	 coded_bits_per_codeword,
	 (3*8*block_length)+12,
	 mod_order,
	 nb_rb);

  dlsch->harq_processes[harq_pid]->RTC = generate_dummy_w(4+(block_length*8), 
							  dummy_w[0]);

  lte_rate_matching_turbo_rx(dlsch->harq_processes[harq_pid]->RTC,
			     coded_bits_per_codeword,
			     dlsch->harq_processes[harq_pid]->w[0],
			     dummy_w,
			     dlsch_llr,
			     1,
			     NSOFT,
			     dlsch->Mdlharq,
			     dlsch->Kmimo,
			     dlsch->rvidx,
			     dlsch->harq_processes[harq_pid]->mod_order,
			     dlsch->harq_processes[harq_pid]->Nl,
			     0);

  sub_block_deinterleaving_turbo(4+(block_length*8), 
				 &d[96], 
				 dlsch->harq_processes[harq_pid]->w[0]); 
  /*
  memset(channel_output,0,((3*8*block_length)+12)*sizeof(short));

  memset(dummy_channel_output,0,(3*8*block_length)+12);
  
  rate_matching(coded_bits_per_codeword,
		(3*8*block_length)+12,
		dummy_channel_output,
		1,
		rmseed);
  */
  write_output("decoder_llr.m","decllr",dlsch_llr,coded_bits_per_codeword,1,0);
  /*
  for (i=0;i<(3*8*block_length)+12;i++) {
    if ((dummy_channel_output[i]&0x40) != 0) { // bit was repeated
      coded_bits++;
      channel_output[i] = *dlsch_llr;
      printf("%d,%d : %d (%d)\n",coded_bits,i,channel_output[i],dummy_channel_output[i]);
      dlsch_llr++;
      coded_bits++;
      channel_output[i] += *dlsch_llr;
      printf("%d,%d : %d (%d)\n",coded_bits,i,channel_output[i],dummy_channel_output[i]);
      dlsch_llr++;
    }
    else if ((dummy_channel_output[i]&0x80) != 0) { // bit was transmitted
      coded_bits++;
      channel_output[i] = *dlsch_llr;
      printf("%d,%d : %d (%d)\n",coded_bits,i,channel_output[i],dummy_channel_output[i]);
      dlsch_llr++;
    }
    else {     //bit was punctured
      channel_output[i] = 0;
    }      
  }



  */
  write_output("decoder_in.m","dec",&d[96],(3*8*block_length)+12,1,0);
  printf("decoder input :");
  for (i=0;i<(3*8*block_length)+12;i++)
    printf("%d : %d\n",i,d[96+i]);
  printf("\n");

  memset(decoded_output,0,16);//block_length);
  ret = phy_threegpplte_turbo_decoder(&d[96],
				      decoded_output,
				      8*block_length,
				      f1f2mat[iind*2],   // f1 (see 36121-820, page 14)
				      f1f2mat[(iind*2)+1], // f2 (see 36121-820, page 14)
				      6,
				      crc_len);
  
}
