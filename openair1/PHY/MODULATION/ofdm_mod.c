/*
* @defgroup _PHY_MODULATION_
* @ingroup _physical_layer_ref_implementation_
* @{
\section _phy_modulation_ OFDM Modulation Blocks
This section deals with basic functions for OFDM Modulation.


*/

#include "PHY/defs.h"
#include <omp.h>
 
//static short temp2[2048*4] __attribute__((aligned(16)));

//#define DEBUG_OFDM_MOD
//#define DEBUG_OMP

void normal_prefix_mod(s32 *txdataF,s32 *txdata,u8 nsymb,LTE_DL_FRAME_PARMS *frame_parms) {

  u8 i;
  //  printf("nsymb %d\n",nsymb);
  for (i=0;i<2*nsymb/frame_parms->symbols_per_tti;i++) {
#ifdef DEBUG_OMP
        printf("slot i %d (txdata offset %d, txoutput %p)\n",i,(i*(frame_parms->samples_per_tti>>1)),
	       txdata+(i*(frame_parms->samples_per_tti>>1)));
#endif
    
    PHY_ofdm_mod(txdataF+(i*NUMBER_OF_OFDM_CARRIERS*frame_parms->symbols_per_tti>>1),        // input
#ifdef BIT8_TX
		 txdata+(i*frame_parms->samples_per_tti>>2),         // output
#else
		 txdata+(i*frame_parms->samples_per_tti>>1),         // output
#endif
		 frame_parms->log2_symbol_size,                // log2_fft_size
		 1,                 // number of symbols
		 frame_parms->nb_prefix_samples0,               // number of prefix samples
		 frame_parms->twiddle_ifft,  // IFFT twiddle factors
		 frame_parms->rev,           // bit-reversal permutation
		 CYCLIC_PREFIX);
#ifdef DEBUG_OMP
        printf("slot i %d (txdata offset %d)\n",i,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES0+(i*frame_parms->samples_per_tti>>1));
#endif    

    PHY_ofdm_mod(txdataF+NUMBER_OF_OFDM_CARRIERS+(i*NUMBER_OF_OFDM_CARRIERS*(frame_parms->symbols_per_tti>>1)),        // input
#ifdef BIT8_TX
		 txdata+(OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES0>>1)+(i*(frame_parms->samples_per_tti>>2)),         // output
#else
		 txdata+OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES0+(i*(frame_parms->samples_per_tti>>1)),         // output
#endif
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

  static short temp[2048*4] __attribute__((aligned(16)));
  unsigned short i,j;
  short k;
#ifdef DEBUG_OMP
  int nthreads,tid;
#endif

#ifdef BIT8_TX
  volatile short *output_ptr=(short*)0;
#else
  volatile int *output_ptr=(int*)0;
#endif
  int *temp_ptr=(int*)0;
  void *(*idft)(int16_t *,int16_t *, int);

  switch (log2fftsize) {
  case 7:
    idft = &idft128;
    break;
  case 8:
    idft = &idft256;
    break;
  case 9:
    idft = &idft512;
    break;
  case 10:
    idft = &idft1024;
    break;
  case 11:
    idft = &idft2048;
    break;
  default:
    idft = &idft512;
    break;
  }

#ifdef DEBUG_OFDM_MOD
  msg("[PHY] OFDM mod (size %d,prefix %d) Symbols %d, input %p, output %p\n",
      1<<log2fftsize,nb_prefix_samples,nb_symbols,input,output);
#endif

#ifdef DEBUG_OMP
#pragma omp parallel shared(input,log2fftsize,temp,output,nb_prefix_samples,nb_symbols) private(i,nthreads,tid)
#else
#pragma omp parallel shared(input,log2fftsize,temp,output,nb_prefix_samples,nb_symbols) private(i)
#endif
  {
#pragma omp for schedule(dynamic,1) nowait

  for (i=0;i<nb_symbols;i++){

#ifdef DEBUG_OMP
    tid=omp_get_thread_num();

    printf("In thread %d, i=%d,nb_symbols %d\n",tid,i,nb_symbols);
    if (tid==0) {
      nthreads=omp_get_num_threads();
      printf("Number of threads = %d\n",nthreads);
    }
#endif
#ifdef DEBUG_OFDM_MOD
    msg("[PHY] symbol %d/%d (%p,%p -> %p)\n",i,nb_symbols,input,&input[i<<log2fftsize],&output[(i<<log2fftsize) + ((i)*nb_prefix_samples)]);
#endif

#ifndef NEW_FFT
    fft((short *)&input[i<<log2fftsize],
	temp,
	twiddle_ifft,
	rev,
	log2fftsize,
	log2fftsize/2,     // normalized FFT (i.e. 1/sqrt(N) multiplicative factor)
	0);
#else
    idft((int16_t *)&input[i<<log2fftsize],
	 (log2fftsize==7) ? (int16_t *)temp : (int16_t *)&output[(i<<log2fftsize) + ((1+i)*nb_prefix_samples)],
	 1);
#endif
    //    write_output("fft_out.m","fftout",temp,(1<<log2fftsize)*2,1,1);

    //memset(temp,0,1<<log2fftsize);
    }
 
#ifdef DEBUG_OFDM_MOD
    for (j=0;j<(1<<log2fftsize);j++) {
      msg("twiddle_ifft(%d) = (%d, %d),rev(%d) = %d\n", j, twiddle_ifft[2*j], twiddle_ifft[2*j+1],j,rev[j]);
    }
#endif
    


    // Copy to frame buffer with Cyclic Extension
    // Note:  will have to adjust for synchronization offset!
    
    switch (etype) {
    case CYCLIC_PREFIX:
#ifdef BIT8_TX
      output_ptr = &(((short*)output)[(i<<log2fftsize) + ((1+i)*nb_prefix_samples)]);
#else
      output_ptr = &output[(i<<log2fftsize) + ((1+i)*nb_prefix_samples)];
#endif
      temp_ptr = (int *)temp;
      

      //      msg("Doing cyclic prefix method\n");

#ifndef NEW_FFT
      for (j=0;j<((1<<log2fftsize)) ; j++) {
#ifdef BIT8_TX
	((char*)output_ptr)[2*j] = (char)(((short*)temp_ptr)[4*j]);
	((char*)output_ptr)[2*j+1] = (char)(((short*)temp_ptr)[4*j+1]);
#else
	output_ptr[j] = temp_ptr[j];

	output_ptr[j] = temp_ptr[2*j];
#endif
      }
#else
      if (log2fftsize==7) {
	for (j=0;j<((1<<log2fftsize)) ; j++) {
	  output_ptr[j] = temp_ptr[j];
	}
      }
      j=(1<<log2fftsize);
#endif
      
      for (k=-1;k>=-nb_prefix_samples;k--) {
	output_ptr[k] = output_ptr[--j];
      }
      break;
      
    case CYCLIC_SUFFIX:
      
      
#ifdef BIT8_TX
      output_ptr = &(((short*)output)[(i<<log2fftsize)+ (i*nb_prefix_samples)]);
#else
      output_ptr = &output[(i<<log2fftsize)+ (i*nb_prefix_samples)];
#endif
      
      temp_ptr = (int *)temp;
      
      //      msg("Doing cyclic suffix method\n");

      for (j=0;j<(1<<log2fftsize) ; j++) {
#ifdef BIT8_TX
	((char*)output_ptr)[2*j] = (char)(((short*)temp_ptr)[4*j]);
	((char*)output_ptr)[2*j+1] = (char)(((short*)temp_ptr)[4*j+1]);
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
#ifdef BIT8_TX
      output_ptr = &(((short*)output)[(i<<log2fftsize)]);
#else
      output_ptr = &output[(i<<log2fftsize)];
#endif
      temp_ptr = (int *)temp;
      
      for (j=0;j<(1<<log2fftsize) ; j++) {
#ifdef BIT8_TX
	((char*)output_ptr)[2*j] = (char)(((short*)temp_ptr)[4*j]);
	((char*)output_ptr)[2*j+1] = (char)(((short*)temp_ptr)[4*j+1]);
#else
	output_ptr[j] = temp_ptr[2*j];
#endif

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
