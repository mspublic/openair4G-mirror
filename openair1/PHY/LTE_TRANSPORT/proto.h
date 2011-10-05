#ifndef __LTE_TRANSPORT_PROTO__H__
#define __LTE_TRANSPORT_PROTO__H__
#include "PHY/defs.h"

// Functions below implement 36-211 and 36-212

/** @addtogroup _PHY_TRANSPORT_
 * @{
 */

/** \fn free_eNB_dlsch(LTE_eNB_DLSCH_t *dlsch)
    \brief This function frees memory allocated for a particular DLSCH at eNB
    @param dlsch Pointer to DLSCH to be removed
 */
void free_eNB_dlsch(LTE_eNB_DLSCH_t *dlsch);

void clean_eNb_dlsch(LTE_eNB_DLSCH_t *dlsch, u8 abstraction_flag);

/** \fn new_eNB_dlsch(u8 Kmimo,u8 Mdlharq,u8 abstraction_flag)
    \brief This function allocates structures for a particular DLSCH at eNB
    @returns Pointer to DLSCH to be removed
    @param Kmimo Kmimo factor from 36-212/36-213
    @param Mdlharq Maximum number of HARQ rounds (36-212/36-213)
    @param abstraction_flag Flag to indicate abstracted interface
 */
LTE_eNB_DLSCH_t *new_eNB_dlsch(u8 Kmimo,u8 Mdlharq,u8 abstraction_flag);

/** \fn free_ue_dlsch(LTE_UE_DLSCH_t *dlsch)
    \brief This function frees memory allocated for a particular DLSCH at UE
    @param dlsch Pointer to DLSCH to be removed
 */
void free_ue_dlsch(LTE_UE_DLSCH_t *dlsch);

LTE_UE_DLSCH_t *new_ue_dlsch(u8 Kmimo,u8 Mdlharq,u8 abstraction_flag);

void free_eNB_dlsch(LTE_eNB_DLSCH_t *dlsch);

LTE_eNB_ULSCH_t *new_eNB_ulsch(u8 Mdlharq,u8 abstraction_flag);

void clean_eNb_ulsch(LTE_eNB_ULSCH_t *ulsch, u8 abstraction_flag);

void free_ue_ulsch(LTE_UE_ULSCH_t *ulsch);

LTE_UE_ULSCH_t *new_ue_ulsch(u8 Mdlharq,u8 abstraction_flag);



/** \fn dlsch_encoding(u8 *input_buffer,
    LTE_DL_FRAME_PARMS *frame_parms,
    u8 num_pdcch_symbols,
    LTE_eNB_DLSCH_t *dlsch,
    u8 subframe)
    \brief This function performs a subset of the bit-coding functions for LTE as described in 36-212, Release 8.Support is limited to turbo-coded channels (DLSCH/ULSCH). The implemented functions are:
    - CRC computation and addition
    - Code block segmentation and sub-block CRC addition
    - Channel coding (Turbo coding)
    - Rate matching (sub-block interleaving, bit collection, selection and transmission
    - Code block concatenation
    @param input_buffer Pointer to input buffer for sub-frame
    @param frame_parms Pointer to frame descriptor structure
    @param num_pdcch_symbols Number of PDCCH symbols in this subframe
    @param dlsch Pointer to dlsch to be encoded
    @param subframe Subframe number
    @returns status
*/
s32 dlsch_encoding(u8 *a,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   u8 num_pdcch_symbols,
		   LTE_eNB_DLSCH_t *dlsch,
		   u8 subframe);

void dlsch_encoding_emul(PHY_VARS_eNB *phy_vars_eNB,
			 u8 *DLSCH_pdu,
			 LTE_eNB_DLSCH_t *dlsch);


// Functions below implement 36-211

/** \fn allocate_REs_in_RB(mod_sym_t **txdataF,
    u32 *jj,
    u16 re_offset,
    u32 symbol_offset,
    u8 *output,
    MIMO_mode_t mimo_mode,
    u8 nu,
    u8 pilots,
    u8 mod_order,
    u8 precoder_index,
    s16 amp,
    u32 *re_allocated,
    u8 skip_dc,
    u8 skip_half,
    u8 use2ndpilots,
    LTE_DL_FRAME_PARMS *frame_parms);

    \brief Fills RB with data
    \param txdataF pointer to output data (frequency domain signal)
    \param jj index to output
    \param re_offset index of the first RE of the RB
    \param symbol_offset index to the OFDM symbol
    \param output output of the channel coder, one bit per byte
    \param mimo_mode MIMO mode
    \param nu Layer index
    \param pilots =1 if symbol_offset is an OFDM symbol that contains pilots, 0 otherwise
    \param mod_order 2=QPSK, 4=16QAM, 6=64QAM
    \param precoder_index 36-211 W precoder column (1 layer) or matrix (2 layer) selection index
    \param amp Amplitude for symbols
    \param re_allocated pointer to allocation counter
    \param skip_dc offset for positive RBs
    \param skip_half indicate that first or second half of RB must be skipped for PBCH/PSS/SSS
    \param use2ndpilots Set to use the pilots from antenna port 1 for PDSCH
    \param frame_parms Frame parameter descriptor
*/

s32 allocate_REs_in_RB(mod_sym_t **txdataF,
		       u32 *jj,
		       u16 re_offset,
		       u32 symbol_offset,
		       u8 *output,
		       MIMO_mode_t mimo_mode,
		       u8 nu,
		       u8 pilots,
		       u8 mod_order,
		       u8 precoder_index,
		       s16 amp,
		       u32 *re_allocated,
		       u8 skip_dc,
		       u8 skip_half,
		       u8 use2ndpilots,
		       LTE_DL_FRAME_PARMS *frame_parms);

/** \fn s32 dlsch_modulation(mod_sym_t **txdataF,
    s16 amp,
    u32 sub_frame_offset,
    LTE_DL_FRAME_PARMS *frame_parms,
    u8 num_pdcch_symbols,
    LTE_eNB_DLSCH_t *dlsch);

    \brief This function is the top-level routine for generation of the sub-frame signal (frequency-domain) for DLSCH.  
    @param txdataF Table of pointers for frequency-domain TX signals
    @param amp Amplitude of signal
    @param sub_frame_offset Offset of this subframe in units of subframes (usually 0)
    @param frame_parms Pointer to frame descriptor
    @param num_pdcch_symbols Number of PDCCH symbols in this subframe
    @param dlsch Pointer to DLSCH descriptor for this allocation

*/ 
s32 dlsch_modulation(mod_sym_t **txdataF,
		     s16 amp,
		     u32 sub_frame_offset,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     u8 num_pdcch_symbols,
		     LTE_eNB_DLSCH_t *dlsch);


/** \brief This function generates the frequency-domain pilots (cell-specific downlink reference signals)
    for N subframes.
    @param phy_vars_eNB Pointer to eNB variables
    @param txdataF Table of pointers for frequency-domain TX signals
    @param amp Amplitude of signal
    @param N Number of sub-frames to generate
*/
void generate_pilots(PHY_VARS_eNB *phy_vars_eNB,
		     mod_sym_t **txdataF,
		     s16 amp,
		     u16 N);

/**
   \brief This function generates the frequency-domain pilots (cell-specific downlink reference signals) for one slot only
   @param phy_vars_eNB Pointer to eNB variables
   @param txdataF Table of pointers for frequency-domain TX signals
   @param amp Amplitude of signal
   @param slot index (0..19)
*/
s32 generate_pilots_slot(PHY_VARS_eNB *phy_vars_eNB,
			 mod_sym_t **txdataF,
			 s16 amp,
			 u16 slot);


s32 generate_pss(mod_sym_t **txdataF,
		 s16 amp,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 u16 l,
		 u16 Ns);

s32 generate_pss_emul(PHY_VARS_eNB *phy_vars_eNB,u8 sect_id);

s32 generate_sss(mod_sym_t **txdataF,
		 short amp,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 unsigned short symbol,
		 unsigned short slot_offset);

s32 generate_pbch(LTE_eNB_PBCH *eNB_pbch,
		  mod_sym_t **txdataF,
		  s32 amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  u8 *pbch_pdu,
		  u8 frame_mod4);

s32 generate_pbch_emul(PHY_VARS_eNB *phy_vars_eNB,u8 *pbch_pdu);

/** \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream QPSK/QPSK reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param stream0_out Output from LLR unit for stream0
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs*/
void qpsk_qpsk(s16 *stream0_in,
	       s16 *stream1_in,
	       s16 *stream0_out,
	       s16 *rho01,
	       s32 length
	       );

/** \brief This function perform LLR computation for dual-stream (QPSK/QPSK) transmission.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp_i Compensated channel output for interference
    @param rho_i Correlation between channel of signal and inteference
    @param dlsch_llr llr output
    @param symbol OFDM symbol index in sub-frame
    @param first_symbol_flag flag to indicate this is the first symbol of the dlsch
    @param nb_rb number of RBs for this allocation
    @param pbch_pss_sss_adj Number of channel bits taken by PBCH/PSS/SSS
    @param llr128p pointer to pointer to symbol in dlsch_llr*/
s32 dlsch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
			s32 **rxdataF_comp,
			s32 **rxdataF_comp_i,
			s32 **rho_i,
			s16 *dlsch_llr,
			u8 symbol,
			u8 first_symbol_flag,
			u16 nb_rb,
			u16 pbch_pss_sss_adj,
			s16 **llr128p);

/** \brief This function generates log-likelihood ratios (decoder input) for single-stream QPSK received waveforms.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param dlsch_llr llr output
    @param symbol OFDM symbol index in sub-frame
    @param first_symbol_flag 
    @param nb_rb number of RBs for this allocation
    @param pbch_pss_sss_adj Number of channel bits taken by PBCH/PSS/SSS
    @param llr128p pointer to pointer to symbol in dlsch_llr
*/
s32 dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
		   s32 **rxdataF_comp,
		   s16 *dlsch_llr,
		   u8 symbol,
		   u8 first_symbol_flag,
		   u16 nb_rb,
		   u16 pbch_pss_sss_adj,
		   s16 **llr128p);

/**
    \brief This function generates log-likelihood ratios (decoder input) for single-stream 16QAM received waveforms
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param dlsch_llr llr output
    @param dl_ch_mag Squared-magnitude of channel in each resource element position corresponding to allocation and weighted for mid-point in 16QAM constellation
    @param symbol OFDM symbol index in sub-frame
    @param first_symbol_flag
    @param nb_rb number of RBs for this allocation
    @param pbch_pss_sss_adjust  Adjustment factor in RE for PBCH/PSS/SSS allocations
    @param llr128p pointer to pointer to symbol in dlsch_llr
*/

void dlsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     s32 **rxdataF_comp,
		     s16 *dlsch_llr,
		     s32 **dl_ch_mag,
		     u8 symbol,
		     u8 first_symbol_flag,
		     u16 nb_rb,
		     u16 pbch_pss_sss_adjust,
		     s16 **llr128p);

/**
    \brief This function generates log-likelihood ratios (decoder input) for single-stream 16QAM received waveforms
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param dlsch_llr llr output
    @param dl_ch_mag Squared-magnitude of channel in each resource element position corresponding to allocation, weighted by first mid-point of 64-QAM constellation
    @param dl_ch_magb Squared-magnitude of channel in each resource element position corresponding to allocation, weighted by second mid-point of 64-QAM constellation
    @param symbol OFDM symbol index in sub-frame
    @param first_symbol_flag
    @param nb_rb number of RBs for this allocation
    @param pbch_pss_sss_adjust PBCH/PSS/SSS RE adjustment (in REs)
*/
void dlsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     s32 **rxdataF_comp,
		     s16 *dlsch_llr,
		     s32 **dl_ch_mag,
		     s32 **dl_ch_magb,
		     u8 symbol,
		     u8 first_symbol_flag,
		     u16 nb_rb,
		     u16 pbch_pss_sss_adjust);

/** \fn dlsch_siso(LTE_DL_FRAME_PARMS *frame_parms,
    s32 **rxdataF_comp,
    s32 **rxdataF_comp_i,
    u8 l,
    u16 nb_rb)
    \brief This function does the first stage of llr computation for SISO, by just extracting the pilots, PBCH and primary/secondary synchronization sequences.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp_i Compensated channel output for interference
    @param l symbol in sub-frame
    @param nb_rb Number of RBs in this allocation
*/

void dlsch_siso(LTE_DL_FRAME_PARMS *frame_parms,
		s32 **rxdataF_comp,
		s32 **rxdataF_comp_i,
		u8 l,
		u16 nb_rb);

/** \fn dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
    s32 **rxdataF_comp,
    s32 **dl_ch_mag,
    s32 **dl_ch_magb,
    u8 symbol,
    u16 nb_rb)
    \brief This function does Alamouti combining on RX and prepares LLR inputs by skipping pilots, PBCH and primary/secondary synchronization signals.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param dl_ch_mag First squared-magnitude of channel (16QAM and 64QAM) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
    @param dl_ch_magb Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
    @param symbol Symbol in sub-frame
    @param nb_rb Number of RBs in this allocation
*/
void dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		    s32 **rxdataF_comp,
		    s32 **dl_ch_mag,
		    s32 **dl_ch_magb,
		    u8 symbol,
		    u16 nb_rb);

/** \fn dlsch_antcyc(LTE_DL_FRAME_PARMS *frame_parms,
    s32 **rxdataF_comp,
    s32 **dl_ch_mag,
    s32 **dl_ch_magb,
    u8 symbol,
    u16 nb_rb)
    \brief This function does antenna selection (based on antenna cycling pattern) on RX and prepares LLR inputs by skipping pilots, PBCH and primary/secondary synchronization signals.  Note that this is not LTE, it is just included for comparison purposes.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param dl_ch_mag First squared-magnitude of channel (16QAM and 64QAM) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
    @param dl_ch_magb Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
    @param symbol Symbol in sub-frame
    @param nb_rb Number of RBs in this allocation
*/
void dlsch_antcyc(LTE_DL_FRAME_PARMS *frame_parms,
		  s32 **rxdataF_comp,
		  s32 **dl_ch_mag,
		  s32 **dl_ch_magb,
		  u8 symbol,
		  u16 nb_rb);

/** \fn dlsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
    s32 **rxdataF_comp,
    s32 **rxdataF_comp_i,
    s32 **rho,
    s32 **rho_i,
    s32 **dl_ch_mag,
    s32 **dl_ch_magb,
    u8 symbol,
    u16 nb_rb,
    u8 dual_stream_UE)

    \brief This function does maximal-ratio combining for dual-antenna receivers.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp_i Compensated channel output for interference
    @param rho Cross correlation between spatial channels
    @param rho_i Cross correlation between signal and inteference channels
    @param dl_ch_mag First squared-magnitude of channel (16QAM and 64QAM) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
    @param dl_ch_magb Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
    @param symbol Symbol in sub-frame
    @param nb_rb Number of RBs in this allocation
    @param dual_stream_UE Flag to indicate dual-stream detection
*/
void dlsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			 s32 **rxdataF_comp,
			 s32 **rxdataF_comp_i,
			 s32 **rho,
			 s32 **rho_i,
			 s32 **dl_ch_mag,
			 s32 **dl_ch_magb,
			 u8 symbol,
			 u16 nb_rb,
			 u8 dual_stream_UE);

/** \fn dlsch_extract_rbs_single(s32 **rxdataF,
    s32 **dl_ch_estimates,
    s32 **rxdataF_ext,
    s32 **dl_ch_estimates_ext,
    u16 pmi,
    u8 *pmi_ext,
    u32 *rb_alloc,
    u8 symbol,
    u8 subframe,
    LTE_DL_FRAME_PARMS *frame_parms)
    \brief This function extracts the received resource blocks, both channel estimates and data symbols,
    for the current allocation and for single antenna eNB transmission.
    @param rxdataF Raw FFT output of received signal
    @param dl_ch_estimates Channel estimates of current slot
    @param rxdataF_ext FFT output for RBs in this allocation
    @param dl_ch_estimates_ext Channel estimates for RBs in this allocation
    @param pmi subband Precoding matrix indicator
    @param pmi_ext Extracted PMI for chosen RBs
    @param rb_alloc RB allocation vector
    @param symbol Symbol to extract
    @param subframe Subframe number
    @param frame_parms Pointer to frame descriptor
*/
u16 dlsch_extract_rbs_single(s32 **rxdataF,
			     s32 **dl_ch_estimates,
			     s32 **rxdataF_ext,
			     s32 **dl_ch_estimates_ext,
			     u16 pmi,
			     u8 *pmi_ext,
			     u32 *rb_alloc,
			     u8 symbol,
			     u8 subframe,
			     LTE_DL_FRAME_PARMS *frame_parms);

/** \fn dlsch_extract_rbs_dual(s32 **rxdataF,
    s32 **dl_ch_estimates,
    s32 **rxdataF_ext,
    s32 **dl_ch_estimates_ext,
    u16 pmi,
    u8 *pmi_ext,
    u32 *rb_alloc,
    u8 symbol,
    LTE_DL_FRAME_PARMS *frame_parms)
    \brief This function extracts the received resource blocks, both channel estimates and data symbols,
    for the current allocation and for dual antenna eNB transmission.
    @param rxdataF Raw FFT output of received signal
    @param dl_ch_estimates Channel estimates of current slot
    @param rxdataF_ext FFT output for RBs in this allocation
    @param dl_ch_estimates_ext Channel estimates for RBs in this allocation
    @param pmi subband Precoding matrix indicator
    @param pmi_ext Extracted PMI for chosen RBs
    @param rb_alloc RB allocation vector
    @param symbol Symbol to extract
    @param frame_parms Pointer to frame descriptor
*/
u16 dlsch_extract_rbs_dual(s32 **rxdataF,
			   s32 **dl_ch_estimates,
			   s32 **rxdataF_ext,
			   s32 **dl_ch_estimates_ext,
			   u16 pmi,
			   u8 *pmi_ext,
			   u32 *rb_alloc,
			   u8 symbol,
			   LTE_DL_FRAME_PARMS *frame_parms);

/** \brief This function performs channel compensation (matched filtering) on the received RBs for this allocation.  In addition, it computes the squared-magnitude of the channel with weightings for 16QAM/64QAM detection as well as dual-stream detection (cross-correlation)
    @param rxdataF_ext Frequency-domain received signal in RBs to be demodulated
    @param dl_ch_estimates_ext Frequency-domain channel estimates in RBs to be demodulated
    @param dl_ch_mag First Channel magnitudes (16QAM/64QAM)
    @param dl_ch_magb Second weighted Channel magnitudes (64QAM)
    @param rxdataF_comp Compensated received waveform 
    @param rho Cross-correlation between two spatial channels on each RX antenna
    @param frame_parms Pointer to frame descriptor
    @param symbol Symbol on which to operate
    @param first_symbol_flag set to 1 on first DLSCH symbol
    @param mod_order Modulation order of allocation
    @param nb_rb Number of RBs in allocation
    @param output_shift Rescaling for compensated output (should be energy-normalizing)
    @param phy_measurements Pointer to UE PHY measurements
*/
void dlsch_channel_compensation(s32 **rxdataF_ext,
				s32 **dl_ch_estimates_ext,
				s32 **dl_ch_mag,
				s32 **dl_ch_magb,
				s32 **rxdataF_comp,
				s32 **rho,
				LTE_DL_FRAME_PARMS *frame_parms,
				u8 symbol,
				u8 first_symbol_flag,
				u8 mod_order,
				u16 nb_rb,
				u8 output_shift,
				PHY_MEASUREMENTS *phy_measurements);

/** \brief This function computes the average channel level over all allocated RBs and antennas (TX/RX) in order to compute output shift for compensated signal
    @param dl_ch_estimates_ext Channel estimates in allocated RBs
    @param frame_parms Pointer to frame descriptor
    @param avg Pointer to average signal strength
    @param pilots_flag Flag to indicate pilots in symbol
    @param nb_rb Number of allocated RBs
*/
void dlsch_channel_level(s32 **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 s32 *avg,
			 u8 pilots_flag,
			 u16 nb_rb);

/** \brief This is the top-level entry point for DLSCH decoding in UE.  It should be replicated on several
    threads (on multi-core machines) corresponding to different HARQ processes. The routine first 
    computes the segmentation information, followed by rate dematching and sub-block deinterleaving the of the
    received LLRs computed by dlsch_demodulation for each transport block segment. It then calls the
    turbo-decoding algorithm for each segment and stops after either after unsuccesful decoding of at least
    one segment or correct decoding of all segments.  Only the segment CRCs are check for the moment, the
    overall CRC is ignored.  Finally transport block reassembly is performed.
    @param dlsch_llr Pointer to LLR values computed by dlsch_demodulation
    @param lte_frame_parms Pointer to frame descriptor
    @param dlsch Pointer to DLSCH descriptor
    @param subframe Subframe number
    @param num_pdcch_symbols Number of PDCCH symbols
    @returns 0 on success, 1 on unsuccessful decoding
*/
u32 dlsch_decoding(s16 *dlsch_llr,
		   LTE_DL_FRAME_PARMS *lte_frame_parms,
		   LTE_UE_DLSCH_t *dlsch,
		   u8 subframe,
		   u8 num_pdcch_symbols);

u32 dlsch_decoding_emul(PHY_VARS_UE *phy_vars_ue,
			u8 subframe,
			u8 dlsch_id,
			u8 eNB_id);

/** \brief This function is the top-level entry point to dlsch demodulation, after frequency-domain transformation and channel estimation.  It performs
    - RB extraction (signal and channel estimates)
    - channel compensation (matched filtering)
    - RE extraction (pilot, PBCH, synch. signals)
    - antenna combining (MRC, Alamouti, cycling)
    - LLR computation
    @param lte_ue_common_vars Pointer to Common RX variable structure for UE
    @param lte_ue_dlsch_vars Pointer to DLSCH signal variable structure for UE
    @param frame_parms Pointer to frame descriptor
    @param eNB_id eNb index (Nid1) 0,1,2
    @param eNB_id_i Interfering eNB index (Nid1) 0,1,2, or 3 in case of MU-MIMO IC receiver
    @param dlsch_ue Pointer to DLSCH coding variable structure for UE
    @param subframe Subframe number
    @param symbol Symbol on which to act (within sub-frame)
    @param first_symbol_flag set to 1 on first DLSCH symbol
    @param dual_stream_UE Flag to indicate dual-stream interference cancellation
    @param phy_measurements Pointer to UE PHY measurements procedure
    @param i_mod Modulation order of the interfering stream
*/
s32 rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_DLSCH **lte_ue_dlsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     u8 eNB_id,
	     u8 eNB_id_i,
	     LTE_UE_DLSCH_t **dlsch_ue,
	     u8 subframe,
	     u8 symbol,
	     u8 first_symbol_flag,
	     u8 dual_stream_UE,
	     PHY_MEASUREMENTS *phy_measurements,
	     u8 i_mod);

s32 rx_pdcch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_PDCCH **lte_ue_pdcch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     u8 subframe,
	     u8 eNB_id,
	     MIMO_mode_t mimo_mode,
	     u8 is_secondary_ue);
/*! \brief Performs detection of SSS to find cell ID and other framing parameters (FDD/TDD, normal/extended prefix)
@param phy_vars_ue Pointer to UE variables
@param tot_metric Pointer to variable containing maximum metric under framing hypothesis (to be compared to other hypotheses
@param flip_max Pointer to variable indicating if start of frame is in second have of RX buffer (i.e. PSS/SSS is flipped)
@param phase_max Pointer to variable (0 ... 6) containing rought phase offset between PSS and SSS (can be used for carrier
frequency adjustment. 0 means -pi/3, 6 means pi/3.
@returns 0 on success
*/
int rx_sss(PHY_VARS_UE *phy_vars_ue,s32 *tot_metric,u8 *flip_max,u8 *phase_max);

/*! \brief receiver for the PBCH
\returns number of tx antennas or -1 if error
*/
u16 rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
	    LTE_UE_PBCH *lte_ue_pbch_vars,
	    LTE_DL_FRAME_PARMS *frame_parms,
	    u8 eNB_id,
	    MIMO_mode_t mimo_mode,
	    u8 frame_mod4);

u16 rx_pbch_emul(PHY_VARS_UE *phy_vars_ue,
		 u8 eNB_id,
		 u8 pbch_phase);

/*! \brief PBCH scrambling. Applies 36.211 PBCH scrambling procedure.
  \param frame_parms Pointer to frame descriptor
  \param coded_data Output of the coding and rate matching
  \param length Length of the sequence*/ 
void pbch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		     u8* coded_data,
		     u32 length);

/*! \brief PBCH unscrambling
  This is similar to pbch_scrabling with the difference that inputs are signed s16s (llr values) and instead of flipping bits we change signs.
  \param frame_parms Pointer to frame descriptor
  \param llr Output of the demodulator
  \param length Length of the sequence
  \param frame_mod4 Frame number modulo 4*/ 
void pbch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
		       s8* llr,
		       u32 length,
		       u8 frame_mod4);

/*! \brief DCI Encoding. This routine codes an arbitrary DCI PDU after appending the 8-bit 3GPP CRC.  It then applied sub-block interleaving and rate matching.
  \param a Pointer to DCI PDU (coded in bytes)
  \param A Length of DCI PDU in bits
  \param E Length of DCI PDU in coded bits
  \param e Pointer to sequence
  \param rnti RNTI for CRC scrambling*/ 
void dci_encoding(u8 *a,
		  u8 A,
		  u16 E,
		  u8 *e,
		  u16 rnti);

/*! \brief Top-level DCI entry point. This routine codes an set of DCI PDUs and performs PDCCH modulation, interleaving and mapping.
  \param num_ue_spec_dci  Number of UE specific DCI pdus to encode
  \param num_common_dci Number of Common DCI pdus to encode
  \param dci_alloc Allocation vectors for each DCI pdu
  \param n_rnti n_RNTI (see )
  \param amp Amplitude of QPSK symbols
  \param frame_parms Pointer to DL Frame parameter structure
  \param txdataF Pointer to tx signal buffers
  \param sub_frame_offset subframe offset in frame
  @returns Number of PDCCH symbols
*/ 
u8 generate_dci_top(u8 num_ue_spec_dci,
		      u8 num_common_dci,
		      DCI_ALLOC_t *dci_alloc, 
		      u32 n_rnti,
		      s16 amp,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      mod_sym_t **txdataF,
		      u32 sub_frame_offset);

u8 generate_dci_top_emul(PHY_VARS_eNB *phy_vars_eNB,
			 u8 num_ue_spec_dci,
			 u8 num_common_dci,
			 DCI_ALLOC_t *dci_alloc,
			 u8 subframe); 


void generate_64qam_table(void);
void generate_16qam_table(void);

u16 extract_crc(u8 *dci,u8 DCI_LENGTH);

/*! \brief LLR from two streams. This function takes two streams (qpsk modulated) and calculates the LLR, considering one stream as interference.
  \param stream0_in pointer to first stream0
  \param stream1_in pointer to first stream1
  \param stream0_out pointer to output stream
  \param rho01 pointer to correlation matrix
  \param length*/ 
void qpsk_qpsk(s16 *stream0_in,
	       s16 *stream1_in,
	       s16 *stream0_out,
	       s16 *rho01,
	       s32 length
	       );

/** \brief Attempt decoding of a particular DCI with given length and format.
@param DCI_LENGTH length of DCI in bits
@param DCI_FMT Format of DCI
@param e e-sequence (soft bits)
@param decoded_output Output of Viterbi decoder
*/
void dci_decoding(u8 DCI_LENGTH,
		  u8 DCI_FMT,
		  s8 *e,
		  u8 *decoded_output);

/** \brief Do 36.213 DCI decoding procedure by searching different RNTI options and aggregation levels.  Currently does
not employ the complexity reducing procedure based on RNTI.
@param phy_vars_ue UE variables
@param dci_alloc Pointer to DCI_ALLOC_t array to store results for DLSCH/ULSCH programming
@param eNB_id eNB Index on which to act
@param subframe Index of subframe
@param si_rnti Value for SI-RNTI
@param ra_rnti Value for RA-RNTI
@returns bitmap of occupied CCE positions (i.e. those detected)
*/
u16 dci_decoding_procedure(PHY_VARS_UE *phy_vars_ue,
			   DCI_ALLOC_t *dci_alloc,
			   s16 eNB_id,
			   u8 subframe,
			   u16 si_rnti,
			   u16 ra_rnti);


u16 dci_decoding_procedure_emul(LTE_UE_PDCCH **lte_ue_pdcch_vars,
				u8 num_ue_spec_dci,
				u8 num_common_dci,
				DCI_ALLOC_t *dci_alloc_tx,
				DCI_ALLOC_t *dci_alloc_rx,
				s16 eNB_id);

/** \brief Compute Q (modulation order) based on I_MCS.  Implements table 7.1.7.1-1 from 36.213.
    @param I_MCS */
u8 get_Qm(u8 I_MCS);

/** \brief Compute I_TBS (transport-block size) based on I_MCS.  Implements table 7.1.7.1-1 from 36.213.
    @param I_MCS */
u8 get_I_TBS(u8 I_MCS);

/** \brief Compute Q (modulation order) based on I_MCS.  Implements table 7.1.7.1-1 from 36.213.
    @param I_MCS */
u16 get_TBS(u8 mcs,u16 nb_rb);

/* \brief Return bit-map of resource allocation for a given DCI rballoc (RIV format) and vrb type
@param vrb_type VRB type (0=localized,1=distributed)
@param rb_alloc_dci rballoc field from DCI
*/
u32 get_rballoc(u8 vrb_type,u16 rb_alloc_dci);

/* \brief Return bit-map of resource allocation for a given DCI rballoc (RIV format) and vrb type
@returns Transmission mode (1-7)
*/
u8 get_transmission_mode(u16 Mod_id, u16 rnti);

/* \brief 
@param ra_header Header of resource allocation (0,1) (See sections 7.1.6.1/7.1.6.2 of 36.213 Rel8.6)
@param rb_alloc Bitmap allocation from DCI (format 1,2) 
@returns number of physical resource blocks
*/
u32 conv_nprb(u8 ra_header,u32 rb_alloc);

u16 get_G(LTE_DL_FRAME_PARMS *frame_parms,u16 nb_rb,u32 *rb_alloc,u8 mod_order,u8 num_pdcch_symbols,u8 subframe);

u16 adjust_G(LTE_DL_FRAME_PARMS *frame_parms,u32 *rb_alloc,u8 mod_order,u8 subframe);
u16 adjust_G2(LTE_DL_FRAME_PARMS *frame_parms,u32 *rb_alloc,u8 mod_order,u8 subframe,u8 symbol);


#ifndef modOrder
#define modOrder(I_MCS,I_TBS) ((I_MCS-I_TBS)*2+2) // Find modulation order from I_TBS and I_MCS
#endif

/** \fn u8 I_TBS2I_MCS(u8 I_TBS);
    \brief This function maps I_tbs to I_mcs according to Table 7.1.7.1-1 in 3GPP TS 36.213 V8.6.0. Where there is two supported modulation orders for the same I_TBS then either high or low modulation is chosen by changing the equality of the two first comparisons in the if-else statement.
    \param I_TBS Index of Transport Block Size
    \return I_MCS given I_TBS
*/
u8 I_TBS2I_MCS(u8 I_TBS);

/** \fn u8 SE2I_TBS(float SE,
    u8 N_PRB,
    u8 symbPerRB);
    \brief This function maps a requested throughput in number of bits to I_tbs. The throughput is calculated as a function of modulation order, RB allocation and number of symbols per RB. The mapping orginates in the "Transport block size table" (Table 7.1.7.2.1-1 in 3GPP TS 36.213 V8.6.0)
    \param SE Spectral Efficiency (before casting to integer, multiply by 1024, remember to divide result by 1024!)
    \param N_PRB Number of PhysicalResourceBlocks allocated \sa lte_frame_parms->N_RB_DL
    \param symbPerRB Number of symbols per resource block allocated to this channel
    \return I_TBS given an SE and an N_PRB
*/
u8 SE2I_TBS(float SE,
	    u8 N_PRB,
	    u8 symbPerRB);
/** \brief This function generates the sounding reference symbol (SRS) for the uplink according to 36.211 v8.6.0. If IFFT_FPGA is defined, the SRS is quantized to a QPSK sequence.
    @param frame_parms LTE DL Frame Parameters
    @param soundingrs_ul_config_dedicated Dynamic configuration from RRC during Connection Establishment
    @param txdataF pointer to the frequency domain TX signal
    @returns 0 on success*/
int generate_srs_rx(LTE_DL_FRAME_PARMS *frame_parms,
		    SOUNDINGRS_UL_CONFIG_DEDICATED *soundingrs_ul_config_dedicated,		    
		    int *txdataF);

s32 generate_srs_tx_emul(PHY_VARS_UE *phy_vars_ue,
			 u8 subframe);

/*!
  \brief This function is similar to generate_srs_tx but generates a conjugate sequence for channel estimation. If IFFT_FPGA is defined, the SRS is quantized to a QPSK sequence.
  @param frame_parms Pointer to LTE DL Frame Parameters
  @param soundingrs_ul_config_dedicated Dynamic configuration from RRC during Connection Establishment
  @param txdataF pointer to the frequency domain TX signal
*/

s32 generate_srs_tx(LTE_DL_FRAME_PARMS *frame_parms,
		    SOUNDINGRS_UL_CONFIG_DEDICATED *soundingrs_ul_config_dedicated,
		    mod_sym_t *txdataF,
		    short amp,
		    unsigned int sub_frame_number);

/*!
  \brief This function generates the downlink reference signal for the PUSCH according to 36.211 v8.6.0. The DRS occuies the RS defined by rb_alloc and the symbols 2 and 8 for extended CP and 3 and 10 for normal CP.
*/

s32 generate_drs_pusch(LTE_DL_FRAME_PARMS *frame_parms,
		       mod_sym_t *txdataF,
		       s16 amp,
		       u32 sub_frame_number,
		       u32 first_rb,
		       u32 nb_rb,
		       u8 cyclic_shift);

s32 compareints (const void * a, const void * b);


void ulsch_modulation(mod_sym_t **txdataF,
		      s16 amp,
		      u32 subframe,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_UE_ULSCH_t *ulsch,
		      u8 cooperation_flag);


void ulsch_extract_rbs_single(s32 **rxdataF,
			      s32 **rxdataF_ext,
			      u32 first_rb,
			      u32 nb_rb,
			      u8 l,
			      u8 Ns,
			      LTE_DL_FRAME_PARMS *frame_parms);

u8 subframe2harq_pid(LTE_DL_FRAME_PARMS *frame_parms,u8 subframe);
u8 subframe2harq_pid_eNBrx(LTE_DL_FRAME_PARMS *frame_parms,u8 subframe);

s32 generate_ue_dlsch_params_from_dci(u8 subframe,
				      void *dci_pdu,
				      u16 rnti,
				      DCI_format_t dci_format,
				      LTE_UE_DLSCH_t **dlsch_ue,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      u16 si_rnti,
				      u16 ra_rnti,
				      u16 p_rnti);

s32 generate_eNB_dlsch_params_from_dci(u8 subframe,
				       void *dci_pdu,
				       u16 rnti,
				       DCI_format_t dci_format,
				       LTE_eNB_DLSCH_t **dlsch_eNB,
				       LTE_DL_FRAME_PARMS *frame_parms,
				       u16 si_rnti,
				       u16 ra_rnti,
				       u16 p_rnti,
				       u16 DL_pmi_single);

s32 generate_eNB_ulsch_params_from_rar(u8 *rar_pdu,
				       u8 subframe,
				       LTE_eNB_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms);

int generate_ue_ulsch_params_from_dci(void *dci_pdu,
				      u16 rnti,
				      u8 subframe,
				      u8 transmisison_mode,
				      DCI_format_t dci_format,
				      LTE_UE_ULSCH_t *ulsch,
				      LTE_UE_DLSCH_t **dlsch,
				      PHY_MEASUREMENTS *meas,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      u16 si_rnti,
				      u16 ra_rnti,
				      u16 p_rnti,
				      u8 eNB_id,
				      u32 current_dlsch_cqi,
				      u8 generate_srs); 

s32 generate_ue_ulsch_params_from_rar(u8 *rar_pdu,
				      u8 subframe,
				      LTE_UE_ULSCH_t *ulsch,
				      PHY_MEASUREMENTS *meas,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      u8 eNB_id,
				      s32 current_dlsch_cqi);

s32 generate_eNB_ulsch_params_from_dci(void *dci_pdu,
				       u16 rnti,
				       u8 subframe,
				       u8 transmission_mode,
				       DCI_format_t dci_format,
				       LTE_eNB_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms,
				       u16 si_rnti,
				       u16 ra_rnti,
				       u16 p_rnti,
				       u8 use_srs);

#ifdef USER_MODE
void dump_ulsch(PHY_VARS_eNB *phy_vars_eNb);

void dump_dlsch(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe);
void dump_dlsch_SI(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe);
void dump_dlsch_ra(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe);

void dump_dlsch2(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u16 coded_bits_per_codeword);
#endif

int dump_dci(LTE_DL_FRAME_PARMS *frame_parms, DCI_ALLOC_t *dci);

int dump_ue_stats(PHY_VARS_UE *phy_vars_ue, char* buffer, int len);
int dump_eNB_stats(PHY_VARS_eNB *phy_vars_eNb, char* buffer, int len);


void generate_pcfich_reg_mapping(LTE_DL_FRAME_PARMS *frame_parms);

void pcfich_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		       u8 subframe,
		       u8 *b,
		       u8 *bt);

void pcfich_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
			 u8 subframe,
			 s16 *d);

void generate_pcfich(u8 num_pdcch_symbols,
		     s16 amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     mod_sym_t **txdataF,
		     u8 subframe);

u8 rx_pcfich(LTE_DL_FRAME_PARMS *frame_parms,
	     u8 subframe,
	     LTE_UE_PDCCH *lte_ue_pdcch_vars,
	     MIMO_mode_t mimo_mode);

void generate_phich_reg_mapping(LTE_DL_FRAME_PARMS *frame_parms);


void init_transport_channels(u8);

void generate_RIV_tables(void);

/*!
  \brief This function performs the initial cell search procedure - PSS detection, SSS detection and PBCH detection.  At the 
end, the basic frame parameters are known (Frame configuration - TDD/FDD and cyclic prefix length, 
N_RB_DL, PHICH_CONFIG and Nid_cell) and the UE can begin decoding PDCCH and DLSCH SI to retrieve the rest.  Once these
parameters are know, the routine calls some basic initialization routines (cell-specific reference signals, etc.)
  @param phy_vars_ue Pointer to UE variables
*/
int initial_sync(PHY_VARS_UE *phy_vars_ue);

void rx_ulsch(LTE_eNB_COMMON *eNB_common_vars,
	      LTE_eNB_ULSCH *eNB_ulsch_vars,
	      LTE_DL_FRAME_PARMS *frame_parms,
	      u32 subframe,
	      u8 eNB_id,  // this is the effective sector id
	      LTE_eNB_ULSCH_t *ulsch,
	      u8 cooperation_flag);

void rx_ulsch_emul(PHY_VARS_eNB *phy_vars_eNB,
		   u8 subframe,
		   u8 sect_id,
		   u8 UE_index);

/*!
  \brief Encoding of PUSCH/ACK/RI/ACK from 36-212.
  @param a Pointer to ulsch SDU
  @param frame_parms Pointer to Frame parameters
  @param ulsch Pointer to ulsch descriptor
  @param harq_pid HARQ process ID
  @param tmode Transmission mode (1-7)
  @param control_only_flag Generate PUSCH with control information only
*/
u32 ulsch_encoding(u8 *a,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   LTE_UE_ULSCH_t *ulsch,
		   u8 harq_pid,
		   u8 tmode,
		   u8 control_only_flag);

/*!
  \brief Encoding of PUSCH/ACK/RI/ACK from 36-212 for emulation
  @param ulsch_buffer Pointer to ulsch SDU
  @param phy_vars_ue Pointer to UE top-level descriptor
  @param eNB_id ID of eNB receiving this PUSCH
  @param harq_pid HARQ process ID
  @param control_only_flag Generate PUSCH with control information only
*/
s32 ulsch_encoding_emul(u8 *ulsch_buffer,
			PHY_VARS_UE *phy_vars_ue,
			u8 eNB_id,
			u8 harq_pid,
			u8 control_only_flag);

u32  ulsch_decoding(s16 *ulsch_llr,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_eNB_ULSCH_t *ulsch,
		    u8 subframe,
		    u8 control_only_flag);

u32 ulsch_decoding_emul(PHY_VARS_eNB *phy_vars_eNB,
			u8 subframe,
			u8 UE_index);

void generate_phich_top(LTE_DL_FRAME_PARMS *frame_parms,
			u8 subframe,
			LTE_eNB_ULSCH_t *ulsch_eNB,
			mod_sym_t **txdataF);

void generate_phich_emul(PHY_VARS_eNB *phy_vars_eNB,
			 u8 subframe,
			 LTE_eNB_ULSCH_t *ulsch_eNB);

void print_CQI(void *o,UCI_format_t uci_format,u8 eNB_id);

void extract_CQI(void *o,UCI_format_t uci_format,LTE_eNB_UE_stats *stats);

void fill_CQI(void *o,UCI_format_t uci_format,PHY_MEASUREMENTS *meas,u8 eNB_id);

u16 quantize_subband_pmi(PHY_MEASUREMENTS *meas,u8 eNB_id);
u16 quantize_subband_pmi2(PHY_MEASUREMENTS *meas,u8 eNB_id,u8 a_id);

u32 pmi2hex_2Ar1(u16 pmi);

u32 pmi2hex_2Ar2(u8 pmi);

u32 cqi2hex(u16 cqi);

u16 computeRIV(u16 N_RB_DL,u16 RBstart,u16 Lcrbs);

u32 pmi_extend(LTE_DL_FRAME_PARMS *frame_parms,u8 wideband_pmi);


u16 get_nCCE(u8 num_pdcch_symbols,LTE_DL_FRAME_PARMS *frame_parms,u8 mi);

u16 get_nquad(u8 num_pdcch_symbols,LTE_DL_FRAME_PARMS *frame_parms,u8 mi);

u8 get_mi(LTE_DL_FRAME_PARMS *frame,u8 subframe);

u16 get_nCCE_max(u8 Mod_id);

u8 get_num_pdcch_symbols(u8 num_dci,DCI_ALLOC_t *dci_alloc,LTE_DL_FRAME_PARMS *frame_parms,u8 subframe);

void pdcch_interleaving(LTE_DL_FRAME_PARMS *frame_parms,mod_sym_t **z, mod_sym_t **wbar,u8 n_symbols_pdcch,u8 mi);

void pdcch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
			u8 subframe,
			s8* llr,
			u32 length);

void pdcch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		      u8 subframe,
		      u8 *e,
		      u32 length);

void dlsch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		      u8 num_pdcch_symbols,
		      LTE_eNB_DLSCH_t *dlsch,
		      u16 G,
		      u8 q,
		      u8 Ns);

void dlsch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
			u8 num_pdcch_symbols,
			LTE_UE_DLSCH_t *dlsch,
			u16 G,
			s16* llr,
			u8 q,
			u8 Ns);

void init_ncs_cell(LTE_DL_FRAME_PARMS *frame_parms,u8 ncs_cell[20][7]);

void generate_pucch(mod_sym_t **txdataF,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    u8 ncs_cell[20][7],
		    PUCCH_FMT_t fmt,
		    PUCCH_CONFIG_DEDICATED *pucch_config_dedicated,
		    u16 n1_pucch,
		    u16 n2_pucch,
		    u8 shortened_format,
		    u8 *payload,
		    s16 amp,
		    u8 subframe);

void generate_pucch_emul(PHY_VARS_UE *phy_vars_ue,
			 PUCCH_FMT_t format,
			 u8 ncs1,
			 u8 *pucch_ack_payload,
			 u8 sr,
			 u8 subframe);


s32 rx_pucch(LTE_eNB_COMMON *eNB_common_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     u8 ncs_cell[20][7],
	     PUCCH_FMT_t fmt,
	     PUCCH_CONFIG_DEDICATED *pucch_config_dedicated,
	     u16 n1_pucch,
	     u16 n2_pucch,
	     u8 shortened_format,
	     u8 *payload,
	     u8 subframe);

s32 rx_pucch_emul(PHY_VARS_eNB *phy_vars_eNB,
		   u8 UE_index,
		   PUCCH_FMT_t fmt,
		   u8 *payload,
		   u8 subframe);

//ICIC algos
u8 Get_SB_size(u8 n_rb_dl);
//end ALU's algo



/**@}*/
#endif
