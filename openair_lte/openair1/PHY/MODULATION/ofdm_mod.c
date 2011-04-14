/*
* @defgroup _PHY_MODULATION_
* @ingroup _physical_layer_ref_implementation_
* @{
\section _phy_modulation_ OFDM Modulation Blocks
This section deals with basic functions for OFDM Modulation.


*/

#include "PHY/defs.h"

static short temp[2048*4] __attribute__((aligned(16)));
//static short temp2[2048*4] __attribute__((aligned(16)));

//#define DEBUG_OFDM_MOD

void normal_prefix_mod(s32 *txdataF,s32 *txdata,u8 nsymb,LTE_DL_FRAME_PARMS *frame_parms) {

  u8 i;
  //  printf("nsymb %d\n",nsymb);
  for (i=0;i<2*nsymb/frame_parms->symbols_per_tti;i++) {
    //    printf("slot i %d (txdata offset %d, txoutput %p)\n",i,(i*(frame_parms->samples_per_tti>>1)),
    //	   txdata+(i*(frame_parms->samples_per_tti>>1)));
    
    PHY_ofdm_mod(txdataF+(i*NUMBER_OF_OFDM_CARRIERS*frame_parms->symbols_per_tti>>1),        // input
		 txdata+(i*frame_parms->samples_per_tti>>1),         // output
		 frame_parms->log2_symbol_size,                // log2_fft_size
		 1,                 // number of symbols
		 frame_parms->nb_prefix_samples0,               // number of prefix samples
		 frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 frame_parms->rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);
    //    printf("slot i %d (txdata offset %d)\n",i,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES0+(i*frame_parms->samples_per_tti>>1));
    

    PHY_ofdm_mod(txdataF+NUMBER_OF_OFDM_CARRIERS+(i*NUMBER_OF_OFDM_CARRIERS*(frame_parms->symbols_per_tti>>1)),        // input
		 txdata+OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES0+(i*(frame_parms->samples_per_tti>>1)),         // output
		 frame_parms->log2_symbol_size,                // log2_fft_size
		 6,                 // number of symbols
		 frame_parms->nb_prefix_samples,               // number of prefix samples
		 frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 frame_parms->rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);
    

  }
}

void PHY_ofdm_mod(int *input,                       /// pointer to complex input
	          int *output,                      /// pointer to complex output
	          unsigned char log2fftsize,        /// log2(FFT_SIZE)
	          unsigned char nb_symbols,         /// number of OFDM symbols
	          unsigned short nb_prefix_samples,  /// cyclic prefix length
		  short *twiddle_ifft,              /// pointer to precomputed twiddle table
		  unsigned short *rev,              /// pointer to bit-reversal table
		  Extension_t etype                /// type of extension
) {


  unsigned short i,j;
  short k;
  int *output_ptr=(int*)0,*temp_ptr=(int*)0;


#ifdef DEBUG_OFDM_MOD
  msg("[PHY] OFDM mod (size %d,prefix %d) Symbols %d, input %p, output %p\n",
      1<<log2fftsize,nb_prefix_samples,nb_symbols,input,output);
#endif

  for (i=0;i<nb_symbols;i++){

#ifdef DEBUG_OFDM_MOD
    msg("[PHY] symbol %d/%d (%p,%p -> %p)\n",i,nb_symbols,input,&input[i<<log2fftsize],&output[(i<<log2fftsize) + ((i)*nb_prefix_samples)]);
#endif

    fft((short *)&input[i<<log2fftsize],
	temp,
	twiddle_ifft,
	rev,
	log2fftsize,
	log2fftsize/2,     // normalized FFT (i.e. 1/sqrt(N) multiplicative factor)
	0);

    // Copy to frame buffer with Cyclic Extension
    // Note:  will have to adjust for synchronization offset!
    
    switch (etype) {
    case CYCLIC_PREFIX:
      output_ptr = &output[(i<<log2fftsize) + ((1+i)*nb_prefix_samples)];
      temp_ptr = (int *)temp;
      

      //      msg("Doing cyclic prefix method\n");

      for (j=0;j<((1<<log2fftsize)) ; j++) {
	output_ptr[j] = temp_ptr[2*j];
      }

      for (k=-1;k>=-nb_prefix_samples;k--) {
	output_ptr[k] = output_ptr[--j];
      }
      break;

    case CYCLIC_SUFFIX:


      output_ptr = &output[(i<<log2fftsize)+ (i*nb_prefix_samples)];

      temp_ptr = (int *)temp;
      
      //      msg("Doing cyclic suffix method\n");

      for (j=0;j<(1<<log2fftsize) ; j++) {
	output_ptr[j] = temp_ptr[2*j];
      }
      
      
      for (j=0;j<nb_prefix_samples;j++)
	output_ptr[(1<<log2fftsize)+j] = output_ptr[j];
      
      break;

    case ZEROS:

      break;

    case NONE:

      //      msg("NO EXTENSION!\n");
      output_ptr = &output[(i<<log2fftsize)];

      temp_ptr = (int *)temp;
      
      for (j=0;j<(1<<log2fftsize) ; j++) {
	output_ptr[j] = temp_ptr[2*j];
      }

      break;

    default:
      break;

    }
    

    
  }
  /*
  printf("input %p, output %p, log2fftsize %d, nsymb %d\n",input,output,log2fftsize,nb_symbols);
  for (i=0;i<16;i++)
    printf("%d %d\n",((short *)input)[i<<1],((short *)input)[1+(i<<1)]);
  printf("------\n");
  for (i=0;i<16;i++)
    printf("%d %d\n",((short *)output)[i<<1],((short *)output)[1+(i<<1)]);  
  */
}


/** @} */
