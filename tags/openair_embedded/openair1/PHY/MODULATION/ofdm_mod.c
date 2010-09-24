/*
* @defgroup _PHY_MODULATION_
* @ingroup _physical_layer_ref_implementation_
* @{
\section _phy_modulation_ OFDM Modulation Blocks
This section deals with basic functions for OFDM Modulation.


*/

#include "PHY/defs.h"

static short temp[512*4] __attribute__((aligned(16)));
//static short temp2[512*4] __attribute__((aligned(16)));

void PHY_ofdm_mod(int *input,                       /// pointer to complex input
	          int *output,                      /// pointer to complex output
	          unsigned char log2fftsize,        /// log2(FFT_SIZE)
	          unsigned char nb_symbols,         /// number of OFDM symbols
	          unsigned char nb_prefix_samples,  /// cyclic prefix length
		  short *twiddle_ifft,              /// pointer to precomputed twiddle table
		  unsigned short *rev,              /// pointer to bit-reversal table
		  Extension_t etype                /// type of extension
) {


  unsigned short i,j;
  short k;
  int *output_ptr=(int*)0,*temp_ptr=(int*)0;


  for (i=0;i<nb_symbols;i++){



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

      //      msg("Doing cyclic prefix method\n");

      for (j=0;j<((1<<log2fftsize)) ; j++) {
#ifdef PLATON
	// flip I/Q and set TX bit
	output_ptr[j] = ((*(int *)(&temp[1+(j<<2)]))<<PLATON_TX_SHIFT) | 0x00010001;
#else
	output_ptr[j] = temp_ptr[2*j];
#endif      

      }

      for (k=-1;k>=-nb_prefix_samples;k--)
	output_ptr[k] = output_ptr[--j];
      break;

    case CYCLIC_SUFFIX:


      output_ptr = &output[(i<<log2fftsize)+ (i*nb_prefix_samples)];

      temp_ptr = (int *)temp;
      
      //      msg("Doing cyclic suffix method\n");

      for (j=0;j<(1<<log2fftsize) ; j++) {
#ifdef PLATON
	// flip I/Q and set TX bit
	output_ptr[j] = ((*(int *)(&temp[1+(j<<2)]))<<PLATON_TX_SHIFT) | 0x00010001;
	
#else
	output_ptr[j] = temp_ptr[2*j];
#endif      
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
  
}


/** @} */
