/** @addtogroup _PHY_MODULATION_BLOCKS_
* @{
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
	          unsigned char nb_prefix_samples,        
		  short *twiddle_ifft,
		  unsigned short *rev,
		  Extension_t etype
		  );
/** @}*/
