#include "PHY/defs.h"
#include "defs.h"
#include <string.h>
#include "PHY/CODING/extern.h"

void  dlsch_decoding(LTE_UE_DLSCH *lte_ue_dlsch_vars,
		     LTE_DL_FRAME_PARMS *lte_frame_parms,
		     unsigned short block_length,
		     unsigned char nb_rb,
		     unsigned char mod_order,
		     unsigned char *decoded_output,
		     unsigned int rmseed,
		     unsigned char crc_len) {

  unsigned int coded_bits_per_codeword,i;
  unsigned int ret;
  unsigned short iind;
  short channel_output[(3*8*block_length)+12] __attribute__ ((aligned(16)));
  unsigned char dummy_channel_output[(3*8*block_length)+12];
  short *dlsch_llr = lte_ue_dlsch_vars->llr;
  short coded_bits=0;

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

  memset(channel_output,0,((3*8*block_length)+12)*sizeof(short));
  memset(dummy_channel_output,0,(3*8*block_length)+12);
  rate_matching(coded_bits_per_codeword,
		(3*8*block_length)+12,
		dummy_channel_output,
		1,
		rmseed);

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

  write_output("decoder_in.m","dec",channel_output,(3*8*block_length)+12,1,0);
  printf("decoder input :");
  for (i=0;i<(3*8*block_length)+12;i++)
    printf("%d : %d (%d)\n",i,channel_output[i],dummy_channel_output[i]);
  printf("\n");

  memset(decoded_output,0,16);//block_length);
  ret = phy_threegpplte_turbo_decoder(channel_output,
				      decoded_output,
				      8*block_length,
				      f1f2mat[iind*2],   // f1 (see 36121-820, page 14)
				      f1f2mat[(iind*2)+1], // f2 (see 36121-820, page 14)
				      6,
				      crc_len);
  
}
