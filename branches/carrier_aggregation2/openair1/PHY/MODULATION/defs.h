#ifndef __MODULATION_DEFS__H__
#define __MODULATION_DEFS__H__
#include "PHY/defs.h"
/** @addtogroup _PHY_MODULATION_
 * @{
*/

/**
\fn void PHY_ofdm_mod(int *input,int *output,unsigned char log2fftsize,unsigned char nb_symbols,unsigned short nb_prefix_samples,short *twiddle_ifft,unsigned short *rev,Extension_t etype)
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

#ifdef OPENAIR_LTE

/*! 
\brief This function implements the OFDM front end processor on reception (FEP)
\param phy_vars_ue Pointer to PHY variables
\param l symbol within slot (0..6/7)
\param Ns Slot number (0..19)
\param sample_offset offset within rxdata (points to beginning of subframe)
\param no_prefix if 1 prefix is removed by HW 
*/

int slot_fep(PHY_VARS_UE *phy_vars_ue,
	     unsigned char l,
	     unsigned char Ns,
	     int sample_offset,
	     int no_prefix);

int slot_fep_ul(LTE_DL_FRAME_PARMS *frame_parms,
		LTE_eNB_COMMON *eNb_common_vars,
		unsigned char l,
		unsigned char Ns,
		unsigned char eNb_id,
		int no_prefix);

void normal_prefix_mod(s32 *txdataF,s32 *txdata,u8 nsymb,LTE_DL_FRAME_PARMS *frame_parms);

void remove_7_5_kHz(PHY_VARS_eNB *phy_vars_eNB,u8 subframe);

void apply_7_5_kHz(PHY_VARS_UE *phy_vars_ue,u8 subframe);

void init_prach625(LTE_DL_FRAME_PARMS *frame_parms);

void remove_625_Hz(PHY_VARS_eNB *phy_vars_eNB,s16 *prach);

void apply_625_Hz(PHY_VARS_UE *phy_vars_ue,s16 *prach);

#endif
/** @}*/
#endif
