#ifndef __MODULATION_DEFS__H__
#define __MODULATION_DEFS__H__
/** @addtogroup _PHY_MODULATION_BLOCKS_
* @{
*/

/**
\fn void PHY_ofdm_mod(int *input,int *output,unsigned char log2fftsize,unsigned char nb_symbols,unsigned char nb_prefix_samples,short *twiddle_ifft,unsigned short *rev,Extension_t etype)
This function performs OFDM modulation with cyclic extension or zero-padding.

@param input The sequence input samples in the frequency-domain.  This is a concatenation of the input symbols in SIMD redundant format
@param output The time-domain output signal
@param log2fftsize Base-2 logarithm of the OFDM symbol size (\f$N_d\f$)
@param nb_symbols The number of OFDM symbols in the block
@param nb_prefix_samples The number of prefix/suffix/zero samples
@param twiddle_ifft Pointer to the ifft twiddle factors
@param rev Pointer to the bit-reversal permutation
@param etype Type of extension (CYCLIC_PREFIX,CYCLIC_SUFFIX,ZEROS)

*/
void PHY_ofdm_mod(int *input,           
	          int *output,          
	          unsigned char log2fftsize,       
	          unsigned char nb_symbols,
	          unsigned short nb_prefix_samples,        
		  short *twiddle_ifft,
		  unsigned short *rev,
		  Extension_t etype
		  );

/** \fn void slot_fep(LTE_DL_FRAME_PARMS *frame_parms,
	      unsigned char l,
	      unsigned char Ns,
	      int **rxdata,
	      int **rxdataF,
	      int **dl_ch_estimates)
This function implements the OFDM front end processor (FEP)

@param frame_parms LTE DL Frame Parameters
@param l symbol 
@param Ns Slot number (0..19)
@param rxdata received date in time domain (input)
@param rxdataF received date in frequency domain (output)
@param dl_ch_estimates estimated channel (output)
@param offset offset within rxdata (used for initial unsychronized estimate)
*/
#ifdef OPENAIR_LTE
void slot_fep(LTE_DL_FRAME_PARMS *frame_parms,
	      unsigned char l,
	      unsigned char Ns,
	      int **rxdata,
	      int **rxdataF,
	      int **dl_ch_estimates,
	      int offset);
#endif
/** @}*/
#endif
