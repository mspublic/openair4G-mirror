/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
   included in this distribution in the file called "COPYING". If not, 
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/

/*! \file PHY/LTE_TRANSPORT/proto.h
 * \brief Function prototypes for PHY physical/transport channel processing and generation V8.6 2009-03
 * \author R. Knopp, F. Kaltenberger
 * \date 2011
 * \version 0.1
 * \company Eurecom
 * \email: knopp@eurecom.fr
 * \note
 * \warning
 */
#ifndef __LTE_TRANSPORT_PROTO__H__
#define __LTE_TRANSPORT_PROTO__H__
#include "PHY/defs.h"
#include <math.h>

// Functions below implement 36-211 and 36-212

/** @addtogroup _PHY_TRANSPORT_
 * @{
 */

/** \fn free_eNB_dlsch(LTE_eNB_DLSCH_t *dlsch)
    \brief This function frees memory allocated for a particular DLSCH at eNB
    @param dlsch Pointer to DLSCH to be removed
*/
void free_eNB_dlsch(LTE_eNB_DLSCH_t *dlsch);

void clean_eNb_dlsch(LTE_eNB_DLSCH_t *dlsch, uint8_t abstraction_flag);

/** \fn new_eNB_dlsch(uint8_t Kmimo,uint8_t Mdlharq,uint8_t abstraction_flag)
    \brief This function allocates structures for a particular DLSCH at eNB
    @returns Pointer to DLSCH to be removed
    @param Kmimo Kmimo factor from 36-212/36-213
    @param Mdlharq Maximum number of HARQ rounds (36-212/36-213)
    @params N_RB_DL total number of resource blocks (determine the operating BW)
    @param abstraction_flag Flag to indicate abstracted interface
*/
LTE_eNB_DLSCH_t *new_eNB_dlsch(uint8_t Kmimo,uint8_t Mdlharq,uint8_t N_RB_DL, uint8_t abstraction_flag);

/** \fn free_ue_dlsch(LTE_UE_DLSCH_t *dlsch)
    \brief This function frees memory allocated for a particular DLSCH at UE
    @param dlsch Pointer to DLSCH to be removed
*/
void free_ue_dlsch(LTE_UE_DLSCH_t *dlsch);

LTE_UE_DLSCH_t *new_ue_dlsch(uint8_t Kmimo,uint8_t Mdlharq,uint8_t max_turbo_iterations,uint8_t N_RB_DL, uint8_t abstraction_flag);

void free_eNB_dlsch(LTE_eNB_DLSCH_t *dlsch);

LTE_eNB_ULSCH_t *new_eNB_ulsch(uint8_t Mdlharq,uint8_t max_turbo_iterations,uint8_t N_RB_UL, uint8_t abstraction_flag);

void clean_eNb_ulsch(LTE_eNB_ULSCH_t *ulsch, uint8_t abstraction_flag);

void free_ue_ulsch(LTE_UE_ULSCH_t *ulsch);

LTE_UE_ULSCH_t *new_ue_ulsch(uint8_t Mdlharq, unsigned char N_RB_UL, uint8_t abstraction_flag);



/** \fn dlsch_encoding(uint8_t *input_buffer,
    LTE_DL_FRAME_PARMS *frame_parms,
    uint8_t num_pdcch_symbols,
    LTE_eNB_DLSCH_t *dlsch,
    int frame,
    uint8_t subframe)
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
    @param frame Frame number
    @param subframe Subframe number
    @param rm_stats Time statistics for rate-matching
    @param te_stats Time statistics for turbo-encoding
    @param i_stats Time statistics for interleaving
    @returns status
*/
int32_t dlsch_encoding(uint8_t *a,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   uint8_t num_pdcch_symbols,
		   LTE_eNB_DLSCH_t *dlsch,
		   int frame,
		   uint8_t subframe,
		   time_stats_t *rm_stats,
		   time_stats_t *te_stats,
		   time_stats_t *i_stats);

void dlsch_encoding_emul(PHY_VARS_eNB *phy_vars_eNB,
			 uint8_t *DLSCH_pdu,
			 LTE_eNB_DLSCH_t *dlsch);


// Functions below implement 36-211

/** \fn allocate_REs_in_RB(mod_sym_t **txdataF,
    uint32_t *jj,
    uint16_t re_offset,
    uint32_t symbol_offset,
    uint8_t *output,
    MIMO_mode_t mimo_mode,
    uint8_t nu,
    uint8_t pilots,
    uint8_t mod_order,
    uint8_t precoder_index,
    int16_t amp,
    int16_t *qam_table_s,
    uint32_t *re_allocated,
    uint8_t skip_dc,
    uint8_t skip_half,
    uint8_t use2ndpilots,
    uint8_t Nlayers,
    uint8_t firstlayer,
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
    \param qam_table_s pointer to scaled QAM table (by rho_a or rho_b)
    \param re_allocated pointer to allocation counter
    \param skip_dc offset for positive RBs
    \param skip_half indicate that first or second half of RB must be skipped for PBCH/PSS/SSS
    \param use2ndpilots Set to use the pilots from antenna port 1 for PDSCH
    \param Nlayers Number of layers for this codeword
    \param firstlayer Index of first layer (minus 7, i.e. 0..7 <-> p=7,...,14
    \param frame_parms Frame parameter descriptor
*/

int32_t allocate_REs_in_RB(mod_sym_t **txdataF,
			   uint32_t *jj,
			   uint16_t re_offset,
			   uint32_t symbol_offset,
			   uint8_t *output,
			   MIMO_mode_t mimo_mode,
			   uint8_t nu,
			   uint8_t pilots,
			   uint8_t mod_order,
			   uint8_t precoder_index,
			   int16_t amp,
			   int16_t *qam_table_s,
			   uint32_t *re_allocated,
			   uint8_t skip_dc,
			   uint8_t skip_half,
			   uint8_t use2ndpilots,
			   uint8_t Nlayers,
			   uint8_t firstlayer,
			   LTE_DL_FRAME_PARMS *frame_parms);

/** \fn int32_t dlsch_modulation(mod_sym_t **txdataF,
    int16_t amp,
    uint32_t sub_frame_offset,
    LTE_DL_FRAME_PARMS *frame_parms,
    uint8_t num_pdcch_symbols,
    LTE_eNB_DLSCH_t *dlsch);

    \brief This function is the top-level routine for generation of the sub-frame signal (frequency-domain) for DLSCH.  
    @param txdataF Table of pointers for frequency-domain TX signals
    @param amp Amplitude of signal
    @param sub_frame_offset Offset of this subframe in units of subframes (usually 0)
    @param frame_parms Pointer to frame descriptor
    @param num_pdcch_symbols Number of PDCCH symbols in this subframe
    @param dlsch Pointer to DLSCH descriptor for this allocation

*/ 
int32_t dlsch_modulation(mod_sym_t **txdataF,
		     int16_t amp,
		     uint32_t sub_frame_offset,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     uint8_t num_pdcch_symbols,
		     LTE_eNB_DLSCH_t *dlsch);
/*
  \brief This function is the top-level routine for generation of the sub-frame signal (frequency-domain) for MCH.  
  @param txdataF Table of pointers for frequency-domain TX signals
  @param amp Amplitude of signal
  @param subframe_offset Offset of this subframe in units of subframes (usually 0)
  @param frame_parms Pointer to frame descriptor
  @param dlsch Pointer to DLSCH descriptor for this allocation
*/
int mch_modulation(mod_sym_t **txdataF,
		   int16_t amp,
		   uint32_t subframe_offset,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   LTE_eNB_DLSCH_t *dlsch);

/** \brief Top-level generation function for eNB TX of MBSFN
    @param phy_vars_eNB Pointer to eNB variables
    @param subframe Subframe for PMCH
    @param a Pointer to transport block
    @param abstraction_flag 

*/
void generate_mch(PHY_VARS_eNB *phy_vars_eNB,int subframe,uint8_t *a,int abstraction_flag);

/** \brief This function generates the frequency-domain pilots (cell-specific downlink reference signals)
    @param phy_vars_eNB Pointer to eNB variables
    @param mcs MCS for MBSFN
    @param ndi new data indicator
    @param rdvix
    @param abstraction_flag 

*/
void fill_eNB_dlsch_MCH(PHY_VARS_eNB *phy_vars_eNB,int mcs,int ndi,int rvidx,int abstraction_flag);

/** \brief This function generates the frequency-domain pilots (cell-specific downlink reference signals)
    @param phy_vars_ue Pointer to UE variables
    @param mcs MCS for MBSFN
    @param eNB_id index of eNB in ue variables
*/
void fill_UE_dlsch_MCH(PHY_VARS_UE *phy_vars_ue,int mcs,int ndi,int rvidx,int eNB_id);

/** \brief Receiver processing for MBSFN, symbols can be done separately for time/CPU-scheduling purposes
    @param phy_vars_ue Pointer to UE variables
    @param eNB_id index of eNB in ue variables
    @param subframe Subframe index of PMCH
    @param symbol Symbol index on which to act
*/
int rx_pmch(PHY_VARS_UE *phy_vars_ue,
	    unsigned char eNB_id,
	    uint8_t subframe,
	    unsigned char symbol);

/** \brief Dump OCTAVE/MATLAB files for PMCH debugging
    @param phy_vars_ue Pointer to UE variables
    @param eNB_id index of eNB in ue variables
    @param coded_bits_per_codeword G from 36.211
    @param subframe Index of subframe
    @returns 0 on success
*/
void dump_mch(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint16_t coded_bits_per_codeword,int subframe);


/** \brief This function generates the frequency-domain pilots (cell-specific downlink reference signals)
    for N subframes.
    @param phy_vars_eNB Pointer to eNB variables
    @param txdataF Table of pointers for frequency-domain TX signals
    @param amp Amplitude of signal
    @param N Number of sub-frames to generate
*/
void generate_pilots(PHY_VARS_eNB *phy_vars_eNB,
		     mod_sym_t **txdataF,
		     int16_t amp,
		     uint16_t N);

/**
   \brief This function generates the frequency-domain pilots (cell-specific downlink reference signals) for one slot only
   @param phy_vars_eNB Pointer to eNB variables
   @param txdataF Table of pointers for frequency-domain TX signals
   @param amp Amplitude of signal
   @param slot index (0..19)
   @param first_pilot_only (0 no)
*/
int32_t generate_pilots_slot(PHY_VARS_eNB *phy_vars_eNB,
			 mod_sym_t **txdataF,
			 int16_t amp,
			 uint16_t slot,
			 int first_pilot_only);
			 
int32_t generate_mbsfn_pilot(PHY_VARS_eNB *phy_vars_eNB,
			 mod_sym_t **txdataF,
			 int16_t amp,
			 uint16_t subframe);

int32_t generate_pss(mod_sym_t **txdataF,
		 int16_t amp,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 uint16_t l,
		 uint16_t Ns);

int32_t generate_pss_emul(PHY_VARS_eNB *phy_vars_eNB,uint8_t sect_id);

int32_t generate_sss(mod_sym_t **txdataF,
		 short amp,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 unsigned short symbol,
		 unsigned short slot_offset);

int32_t generate_pbch(LTE_eNB_PBCH *eNB_pbch,
		  mod_sym_t **txdataF,
		  int32_t amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  uint8_t *pbch_pdu,
		  uint8_t frame_mod4);

int32_t generate_pbch_emul(PHY_VARS_eNB *phy_vars_eNB,uint8_t *pbch_pdu);

/** \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream QPSK/QPSK reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param stream0_out Output from LLR unit for stream0
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs*/
void qpsk_qpsk(int16_t *stream0_in,
	       int16_t *stream1_in,
	       int16_t *stream0_out,
	       int16_t *rho01,
	       int32_t length);

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
int32_t dlsch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
			int32_t **rxdataF_comp,
			int32_t **rxdataF_comp_i,
			int32_t **rho_i,
			int16_t *dlsch_llr,
			uint8_t symbol,
			uint8_t first_symbol_flag,
			uint16_t nb_rb,
			uint16_t pbch_pss_sss_adj,
			int16_t **llr128p);

/** \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream QPSK/16QAM reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param ch_mag_i Input from scaled channel magnitude square of h0'*g1
    @param stream0_out Output from LLR unit for stream0
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs*/
void qpsk_qam16(int16_t *stream0_in,
                int16_t *stream1_in,
                short *ch_mag_i,
                int16_t *stream0_out,
                int16_t *rho01,
                int32_t length);

/** \brief This function perform LLR computation for dual-stream (QPSK/16QAM) transmission.
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
int32_t dlsch_qpsk_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
			 int32_t **rxdataF_comp,
			 int32_t **rxdataF_comp_i,
			 int **dl_ch_mag_i, //|h_1|^2*(2/sqrt{10})
			 int32_t **rho_i,
			 int16_t *dlsch_llr,
			 uint8_t symbol,
			 uint8_t first_symbol_flag,
			 uint16_t nb_rb,
			 uint16_t pbch_pss_sss_adj,
			 int16_t **llr128p);

/** \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream QPSK/64QAM reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param ch_mag_i Input from scaled channel magnitude square of h0'*g1
    @param stream0_out Output from LLR unit for stream0
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs*/
void qpsk_qam64(int16_t *stream0_in,
                int16_t *stream1_in,
                short *ch_mag_i,
                int16_t *stream0_out,
                int16_t *rho01,
                int32_t length);

/** \brief This function perform LLR computation for dual-stream (QPSK/64QAM) transmission.
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
int32_t dlsch_qpsk_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
			 int32_t **rxdataF_comp,
			 int32_t **rxdataF_comp_i,
			 int **dl_ch_mag_i, //|h_1|^2*(2/sqrt{10})
			 int32_t **rho_i,
			 int16_t *dlsch_llr,
			 uint8_t symbol,
			 uint8_t first_symbol_flag,
			 uint16_t nb_rb,
			 uint16_t pbch_pss_sss_adj,
			 int16_t **llr128p);


/** \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream 16QAM/QPSK reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param stream0_out Output from LLR unit for stream0
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs*/
void qam16_qpsk(short *stream0_in,
                short *stream1_in,
                short *ch_mag,
                short *stream0_out,
                short *rho01,
                int length); 
/** \brief This function perform LLR computation for dual-stream (16QAM/QPSK) transmission.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp_i Compensated channel output for interference
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param rho_i Correlation between channel of signal and inteference
    @param dlsch_llr llr output
    @param symbol OFDM symbol index in sub-frame
    @param first_symbol_flag flag to indicate this is the first symbol of the dlsch
    @param nb_rb number of RBs for this allocation
    @param pbch_pss_sss_adj Number of channel bits taken by PBCH/PSS/SSS
    @param llr16p pointer to pointer to symbol in dlsch_llr*/
int dlsch_16qam_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
                         int **rxdataF_comp,
                         int **rxdataF_comp_i,
                         int **dl_ch_mag,   //|h_0|^2*(2/sqrt{10})
                         int **rho_i,
                         short *dlsch_llr,
                         unsigned char symbol,
                         unsigned char first_symbol_flag,
                         unsigned short nb_rb,
                         uint16_t pbch_pss_sss_adjust,
                         short **llr16p);

/** \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream 16QAM/16QAM reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param ch_mag_i Input from scaled channel magnitude square of h0'*g1
    @param stream0_out Output from LLR unit for stream0
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs*/
void qam16_qam16(short *stream0_in,
                 short *stream1_in,
                 short *ch_mag,
                 short *ch_mag_i,
                 short *stream0_out,
                 short *rho01,
                 int length);

/** \brief This function perform LLR computation for dual-stream (16QAM/16QAM) transmission.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp_i Compensated channel output for interference
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param ch_mag_i Input from scaled channel magnitude square of h0'*g1
    @param rho_i Correlation between channel of signal and inteference
    @param dlsch_llr llr output
    @param symbol OFDM symbol index in sub-frame
    @param first_symbol_flag flag to indicate this is the first symbol of the dlsch
    @param nb_rb number of RBs for this allocation
    @param pbch_pss_sss_adj Number of channel bits taken by PBCH/PSS/SSS
    @param llr16p pointer to pointer to symbol in dlsch_llr*/
int dlsch_16qam_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                          int **rxdataF_comp,
                          int **rxdataF_comp_i,
                          int **dl_ch_mag,   //|h_0|^2*(2/sqrt{10})
                          int **dl_ch_mag_i, //|h_1|^2*(2/sqrt{10})
                          int **rho_i,
                          short *dlsch_llr,
                          unsigned char symbol,
                          unsigned char first_symbol_flag,
                          unsigned short nb_rb,
                          uint16_t pbch_pss_sss_adjust,
                          short **llr16p);

/** \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream 16QAM/64QAM reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param ch_mag_i Input from scaled channel magnitude square of h0'*g1
    @param stream0_out Output from LLR unit for stream0
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs*/
void qam16_qam64(short *stream0_in,
                 short *stream1_in,
                 short *ch_mag,
                 short *ch_mag_i,
                 short *stream0_out,
                 short *rho01,
                 int length);

/** \brief This function perform LLR computation for dual-stream (16QAM/64QAM) transmission.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp_i Compensated channel output for interference
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param ch_mag_i Input from scaled channel magnitude square of h0'*g1
    @param rho_i Correlation between channel of signal and inteference
    @param dlsch_llr llr output
    @param symbol OFDM symbol index in sub-frame
    @param first_symbol_flag flag to indicate this is the first symbol of the dlsch
    @param nb_rb number of RBs for this allocation
    @param pbch_pss_sss_adj Number of channel bits taken by PBCH/PSS/SSS
    @param llr16p pointer to pointer to symbol in dlsch_llr*/
int dlsch_16qam_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                          int **rxdataF_comp,
                          int **rxdataF_comp_i,
                          int **dl_ch_mag,   //|h_0|^2*(2/sqrt{10})
                          int **dl_ch_mag_i, //|h_1|^2*(2/sqrt{10})
                          int **rho_i,
                          short *dlsch_llr,
                          unsigned char symbol,
                          unsigned char first_symbol_flag,
                          unsigned short nb_rb,
                          uint16_t pbch_pss_sss_adjust,
                          short **llr16p);

/** \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream 64QAM/64QAM reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param stream0_out Output from LLR unit for stream0
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs*/
void qam64_qpsk(short *stream0_in,
                short *stream1_in,
                short *ch_mag,
                short *stream0_out,
                short *rho01,
                int length);

/** \brief This function perform LLR computation for dual-stream (64QAM/64QAM) transmission.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp_i Compensated channel output for interference
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param rho_i Correlation between channel of signal and inteference
    @param dlsch_llr llr output
    @param symbol OFDM symbol index in sub-frame
    @param first_symbol_flag flag to indicate this is the first symbol of the dlsch
    @param nb_rb number of RBs for this allocation
    @param pbch_pss_sss_adj Number of channel bits taken by PBCH/PSS/SSS
    @param llr16p pointer to pointer to symbol in dlsch_llr*/
int dlsch_64qam_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
                         int **rxdataF_comp,
                         int **rxdataF_comp_i,
                         int **dl_ch_mag,
                         int **rho_i,
                         short *dlsch_llr,
                         unsigned char symbol,
                         unsigned char first_symbol_flag,
                         unsigned short nb_rb,
                         uint16_t pbch_pss_sss_adjust,
                         short **llr16p);

/** \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream 64QAM/16QAM reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param ch_mag_i Input from scaled channel magnitude square of h0'*g1
    @param stream0_out Output from LLR unit for stream0
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs*/
void qam64_qam16(short *stream0_in,
                 short *stream1_in,
                 short *ch_mag,
                 short *ch_mag_i,
                 short *stream0_out,
                 short *rho01,
                 int length);

/** \brief This function perform LLR computation for dual-stream (64QAM/16QAM) transmission.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp_i Compensated channel output for interference
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param ch_mag_i Input from scaled channel magnitude square of h0'*g1
    @param rho_i Correlation between channel of signal and inteference
    @param dlsch_llr llr output
    @param symbol OFDM symbol index in sub-frame
    @param first_symbol_flag flag to indicate this is the first symbol of the dlsch
    @param nb_rb number of RBs for this allocation
    @param pbch_pss_sss_adj Number of channel bits taken by PBCH/PSS/SSS
    @param llr16p pointer to pointer to symbol in dlsch_llr*/
int dlsch_64qam_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                          int **rxdataF_comp,
                          int **rxdataF_comp_i,
                          int **dl_ch_mag,
                          int **dl_ch_mag_i,
                          int **rho_i,
                          short *dlsch_llr,
                          unsigned char symbol,
                          unsigned char first_symbol_flag,
                          unsigned short nb_rb,
                          uint16_t pbch_pss_sss_adjust,
                          short **llr16p);

/** \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream 64QAM/64QAM reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param ch_mag_i Input from scaled channel magnitude square of h0'*g1
    @param stream0_out Output from LLR unit for stream0
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs*/
void qam64_qam64(short *stream0_in,
                 short *stream1_in,
                 short *ch_mag,
                 short *ch_mag_i,
                 short *stream0_out,
                 short *rho01,
                 int length);

/** \brief This function perform LLR computation for dual-stream (64QAM/64QAM) transmission.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp_i Compensated channel output for interference
    @param ch_mag   Input from scaled channel magnitude square of h0'*g0
    @param ch_mag_i Input from scaled channel magnitude square of h0'*g1
    @param rho_i Correlation between channel of signal and inteference
    @param dlsch_llr llr output
    @param symbol OFDM symbol index in sub-frame
    @param first_symbol_flag flag to indicate this is the first symbol of the dlsch
    @param nb_rb number of RBs for this allocation
    @param pbch_pss_sss_adj Number of channel bits taken by PBCH/PSS/SSS
    @param llr16p pointer to pointer to symbol in dlsch_llr*/
int dlsch_64qam_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                          int **rxdataF_comp,
                          int **rxdataF_comp_i,
                          int **dl_ch_mag,
                          int **dl_ch_mag_i,
                          int **rho_i,
                          short *dlsch_llr,
                          unsigned char symbol,
                          unsigned char first_symbol_flag,
                          unsigned short nb_rb,
                          uint16_t pbch_pss_sss_adjust,
                          short **llr16p);


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
int32_t dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
		   int32_t **rxdataF_comp,
		   int16_t *dlsch_llr,
		   uint8_t symbol,
		   uint8_t first_symbol_flag,
		   uint16_t nb_rb,
		   uint16_t pbch_pss_sss_adj,
		   int16_t **llr128p);

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
		     int32_t **rxdataF_comp,
		     int16_t *dlsch_llr,
		     int32_t **dl_ch_mag,
		     uint8_t symbol,
		     uint8_t first_symbol_flag,
		     uint16_t nb_rb,
		     uint16_t pbch_pss_sss_adjust,
		     int16_t **llr128p);

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
		     int32_t **rxdataF_comp,
		     int16_t *dlsch_llr,
		     int32_t **dl_ch_mag,
		     int32_t **dl_ch_magb,
		     uint8_t symbol,
		     uint8_t first_symbol_flag,
		     uint16_t nb_rb,
		     uint16_t pbch_pss_sss_adjust,
		     short **llr_save);

/** \fn dlsch_siso(LTE_DL_FRAME_PARMS *frame_parms,
    int32_t **rxdataF_comp,
    int32_t **rxdataF_comp_i,
    uint8_t l,
    uint16_t nb_rb)
    \brief This function does the first stage of llr computation for SISO, by just extracting the pilots, PBCH and primary/secondary synchronization sequences.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp_i Compensated channel output for interference
    @param l symbol in sub-frame
    @param nb_rb Number of RBs in this allocation
*/

void dlsch_siso(LTE_DL_FRAME_PARMS *frame_parms,
		int32_t **rxdataF_comp,
		int32_t **rxdataF_comp_i,
		uint8_t l,
		uint16_t nb_rb);

/** \fn dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
    int32_t **rxdataF_comp,
    int32_t **dl_ch_mag,
    int32_t **dl_ch_magb,
    uint8_t symbol,
    uint16_t nb_rb)
    \brief This function does Alamouti combining on RX and prepares LLR inputs by skipping pilots, PBCH and primary/secondary synchronization signals.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param dl_ch_mag First squared-magnitude of channel (16QAM and 64QAM) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
    @param dl_ch_magb Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
    @param symbol Symbol in sub-frame
    @param nb_rb Number of RBs in this allocation
*/
void dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		    int32_t **rxdataF_comp,
		    int32_t **dl_ch_mag,
		    int32_t **dl_ch_magb,
		    uint8_t symbol,
		    uint16_t nb_rb);

/** \fn dlsch_antcyc(LTE_DL_FRAME_PARMS *frame_parms,
    int32_t **rxdataF_comp,
    int32_t **dl_ch_mag,
    int32_t **dl_ch_magb,
    uint8_t symbol,
    uint16_t nb_rb)
    \brief This function does antenna selection (based on antenna cycling pattern) on RX and prepares LLR inputs by skipping pilots, PBCH and primary/secondary synchronization signals.  Note that this is not LTE, it is just included for comparison purposes.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param dl_ch_mag First squared-magnitude of channel (16QAM and 64QAM) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
    @param dl_ch_magb Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
    @param symbol Symbol in sub-frame
    @param nb_rb Number of RBs in this allocation
*/
void dlsch_antcyc(LTE_DL_FRAME_PARMS *frame_parms,
		  int32_t **rxdataF_comp,
		  int32_t **dl_ch_mag,
		  int32_t **dl_ch_magb,
		  uint8_t symbol,
		  uint16_t nb_rb);

/** \fn dlsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
    int32_t **rxdataF_comp,
    int32_t **rxdataF_comp_i,
    int32_t **rho,
    int32_t **rho_i,
    int32_t **dl_ch_mag,
    int32_t **dl_ch_magb,
    uint8_t symbol,
    uint16_t nb_rb,
    uint8_t dual_stream_UE)

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
			 int32_t **rxdataF_comp,
			 int32_t **rxdataF_comp_i,
			 int32_t **rho,
			 int32_t **rho_i,
			 int32_t **dl_ch_mag,
			 int32_t **dl_ch_magb,
			 int32_t **dl_ch_mag_i,
			 int32_t **dl_ch_magb_i,
			 uint8_t symbol,
			 uint16_t nb_rb,
			 uint8_t dual_stream_UE);

/** \fn dlsch_extract_rbs_single(int32_t **rxdataF,
    int32_t **dl_ch_estimates,
    int32_t **rxdataF_ext,
    int32_t **dl_ch_estimates_ext,
    uint16_t pmi,
    uint8_t *pmi_ext,
    uint32_t *rb_alloc,
    uint8_t symbol,
    uint8_t subframe,
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
uint16_t dlsch_extract_rbs_single(int32_t **rxdataF,
				  int32_t **dl_ch_estimates,
				  int32_t **rxdataF_ext,
				  int32_t **dl_ch_estimates_ext,
				  uint16_t pmi,
				  uint8_t *pmi_ext,
				  uint32_t *rb_alloc,
				  uint8_t symbol,
				  uint8_t subframe,
				  LTE_DL_FRAME_PARMS *frame_parms);

/** \fn dlsch_extract_rbs_dual(int32_t **rxdataF,
    int32_t **dl_ch_estimates,
    int32_t **rxdataF_ext,
    int32_t **dl_ch_estimates_ext,
    uint16_t pmi,
    uint8_t *pmi_ext,
    uint32_t *rb_alloc,
    uint8_t symbol,
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
    @param subframe Subframe index
    @param frame_parms Pointer to frame descriptor
*/
uint16_t dlsch_extract_rbs_dual(int32_t **rxdataF,
				int32_t **dl_ch_estimates,
				int32_t **rxdataF_ext,
				int32_t **dl_ch_estimates_ext,
				uint16_t pmi,
				uint8_t *pmi_ext,
				uint32_t *rb_alloc,
				uint8_t symbol,
				uint8_t subframe,
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
void dlsch_channel_compensation(int32_t **rxdataF_ext,
				int32_t **dl_ch_estimates_ext,
				int32_t **dl_ch_mag,
				int32_t **dl_ch_magb,
				int32_t **rxdataF_comp,
				int32_t **rho,
				LTE_DL_FRAME_PARMS *frame_parms,
				uint8_t symbol,
				uint8_t first_symbol_flag,
				uint8_t mod_order,
				uint16_t nb_rb,
				uint8_t output_shift,
				PHY_MEASUREMENTS *phy_measurements);

void dlsch_dual_stream_correlation(LTE_DL_FRAME_PARMS *frame_parms,
                                   unsigned char symbol,
                                   unsigned short nb_rb,
                                   int **dl_ch_estimates_ext,
                                   int **dl_ch_estimates_ext_i,
                                   int **dl_ch_rho_ext,
                                   unsigned char output_shift);

void dlsch_channel_compensation_prec(int **rxdataF_ext,
				     int **dl_ch_estimates_ext,
				     int **dl_ch_mag,
				     int **dl_ch_magb,
				     int **rxdataF_comp,
				     unsigned char *pmi_ext,
				     LTE_DL_FRAME_PARMS *frame_parms,
				     PHY_MEASUREMENTS *phy_measurements,
				     int eNB_id,
				     unsigned char symbol,
				     unsigned char mod_order,
				     unsigned short nb_rb,
				     unsigned char output_shift,
				     unsigned char dl_power_off);

/** \brief This function computes the average channel level over all allocated RBs and antennas (TX/RX) in order to compute output shift for compensated signal
    @param dl_ch_estimates_ext Channel estimates in allocated RBs
    @param frame_parms Pointer to frame descriptor
    @param avg Pointer to average signal strength
    @param pilots_flag Flag to indicate pilots in symbol
    @param nb_rb Number of allocated RBs
*/
void dlsch_channel_level(int32_t **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 int32_t *avg,
			 uint8_t pilots_flag,
			 uint16_t nb_rb);

void dlsch_channel_level_prec(int32_t **dl_ch_estimates_ext,
                              LTE_DL_FRAME_PARMS *frame_parms,
                              unsigned char *pmi_ext,
                              int32_t *avg,
                              uint8_t symbol_mod,
                              uint16_t nb_rb);

void dlsch_scale_channel(int32_t **dl_ch_estimates_ext,
                         LTE_DL_FRAME_PARMS *frame_parms,
                         LTE_UE_DLSCH_t **dlsch_ue,
                         uint8_t symbol_mod,
                         uint16_t nb_rb);

/** \brief This is the top-level entry point for DLSCH decoding in UE.  It should be replicated on several
    threads (on multi-core machines) corresponding to different HARQ processes. The routine first 
    computes the segmentation information, followed by rate dematching and sub-block deinterleaving the of the
    received LLRs computed by dlsch_demodulation for each transport block segment. It then calls the
    turbo-decoding algorithm for each segment and stops after either after unsuccesful decoding of at least
    one segment or correct decoding of all segments.  Only the segment CRCs are check for the moment, the
    overall CRC is ignored.  Finally transport block reassembly is performed.
    @param phy_vars_ue Pointer to ue variables
    @param dlsch_llr Pointer to LLR values computed by dlsch_demodulation
    @param lte_frame_parms Pointer to frame descriptor
    @param dlsch Pointer to DLSCH descriptor
    @param subframe Subframe number
    @param num_pdcch_symbols Number of PDCCH symbols
    @param is_crnti indicates if PDSCH belongs to a CRNTI (necessary for parallelizing decoding threads)
    @param llr8_flag If 1, indicate that the 8-bit turbo decoder should be used
    @returns 0 on success, 1 on unsuccessful decoding
*/
uint32_t dlsch_decoding(PHY_VARS_UE *phy_vars_ue,
			int16_t *dlsch_llr,
			LTE_DL_FRAME_PARMS *lte_frame_parms,
			LTE_UE_DLSCH_t *dlsch,
			LTE_DL_UE_HARQ_t *harq_process,
			uint8_t subframe,
			uint8_t harq_pid,
			uint8_t is_crnti,
			uint8_t llr8_flag);

uint32_t dlsch_decoding_emul(PHY_VARS_UE *phy_vars_ue,
			     uint8_t subframe,
			     uint8_t dlsch_id,
			     uint8_t eNB_id);

/** \brief This function is the top-level entry point to PDSCH demodulation, after frequency-domain transformation and channel estimation.  It performs
    - RB extraction (signal and channel estimates)
    - channel compensation (matched filtering)
    - RE extraction (pilot, PBCH, synch. signals)
    - antenna combining (MRC, Alamouti, cycling)
    - LLR computation
    @param phy_vars_ue Pointer to PHY variables
    @param type Type of PDSCH (SI_PDSCH,RA_PDSCH,PDSCH,PMCH)
    @param eNB_id eNb index (Nid1) 0,1,2
    @param eNB_id_i Interfering eNB index (Nid1) 0,1,2, or 3 in case of MU-MIMO IC receiver
    @param subframe Subframe number
    @param symbol Symbol on which to act (within sub-frame)
    @param first_symbol_flag set to 1 on first DLSCH symbol
    @param dual_stream_UE Flag to indicate dual-stream interference cancellation
    @param i_mod Modulation order of the interfering stream
*/
int32_t rx_pdsch(PHY_VARS_UE *phy_vars_ue,
	     PDSCH_t type,
	     uint8_t eNB_id,
	     uint8_t eNB_id_i,
	     uint8_t subframe,
	     uint8_t symbol,
	     uint8_t first_symbol_flag,
	     uint8_t dual_stream_UE,
	     uint8_t i_mod,
	     uint8_t harq_pid);

int32_t rx_pdcch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_PDCCH **lte_ue_pdcch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     uint8_t subframe,
	     uint8_t eNB_id,
	     MIMO_mode_t mimo_mode,
	     uint8_t is_secondary_ue);
/*! \brief Performs detection of SSS to find cell ID and other framing parameters (FDD/TDD, normal/extended prefix)
  @param phy_vars_ue Pointer to UE variables
  @param tot_metric Pointer to variable containing maximum metric under framing hypothesis (to be compared to other hypotheses
  @param flip_max Pointer to variable indicating if start of frame is in second have of RX buffer (i.e. PSS/SSS is flipped)
  @param phase_max Pointer to variable (0 ... 6) containing rought phase offset between PSS and SSS (can be used for carrier
  frequency adjustment. 0 means -pi/3, 6 means pi/3.
  @returns 0 on success
*/
int rx_sss(PHY_VARS_UE *phy_vars_ue,int32_t *tot_metric,uint8_t *flip_max,uint8_t *phase_max);

/*! \brief receiver for the PBCH
  \returns number of tx antennas or -1 if error
*/
uint16_t rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
		 LTE_UE_PBCH *lte_ue_pbch_vars,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 uint8_t eNB_id,
		 MIMO_mode_t mimo_mode,
		 uint8_t frame_mod4);

uint16_t rx_pbch_emul(PHY_VARS_UE *phy_vars_ue,
		      uint8_t eNB_id,
		      uint8_t pbch_phase);

/*! \brief PBCH scrambling. Applies 36.211 PBCH scrambling procedure.
  \param frame_parms Pointer to frame descriptor
  \param coded_data Output of the coding and rate matching
  \param length Length of the sequence*/ 
void pbch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		     uint8_t* coded_data,
		     uint32_t length);

/*! \brief PBCH unscrambling
  This is similar to pbch_scrabling with the difference that inputs are signed s16s (llr values) and instead of flipping bits we change signs.
  \param frame_parms Pointer to frame descriptor
  \param llr Output of the demodulator
  \param length Length of the sequence
  \param frame_mod4 Frame number modulo 4*/ 
void pbch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
		       int8_t* llr,
		       uint32_t length,
		       uint8_t frame_mod4);

/*! \brief DCI Encoding. This routine codes an arbitrary DCI PDU after appending the 8-bit 3GPP CRC.  It then applied sub-block interleaving and rate matching.
  \param a Pointer to DCI PDU (coded in bytes)
  \param A Length of DCI PDU in bits
  \param E Length of DCI PDU in coded bits
  \param e Pointer to sequence
  \param rnti RNTI for CRC scrambling*/ 
void dci_encoding(uint8_t *a,
		  uint8_t A,
		  uint16_t E,
		  uint8_t *e,
		  uint16_t rnti);

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
uint8_t generate_dci_top(uint8_t num_ue_spec_dci,
			 uint8_t num_common_dci,
			 DCI_ALLOC_t *dci_alloc, 
			 uint32_t n_rnti,
			 int16_t amp,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 mod_sym_t **txdataF,
			 uint32_t sub_frame_offset);

uint8_t generate_dci_top_emul(PHY_VARS_eNB *phy_vars_eNB,
			      uint8_t num_ue_spec_dci,
			      uint8_t num_common_dci,
			      DCI_ALLOC_t *dci_alloc,
			      uint8_t subframe); 


void generate_64qam_table(void);
void generate_16qam_table(void);

uint16_t extract_crc(uint8_t *dci,uint8_t DCI_LENGTH);

/*! \brief LLR from two streams. This function takes two streams (qpsk modulated) and calculates the LLR, considering one stream as interference.
  \param stream0_in pointer to first stream0
  \param stream1_in pointer to first stream1
  \param stream0_out pointer to output stream
  \param rho01 pointer to correlation matrix
  \param length*/ 
void qpsk_qpsk_prec(short *stream0_in,
		    short *stream1_in,
		    short *stream0_out,
		    short *rho01,
		    int length
		    );

/** \brief Attempt decoding of a particular DCI with given length and format.
    @param DCI_LENGTH length of DCI in bits
    @param DCI_FMT Format of DCI
    @param e e-sequence (soft bits)
    @param decoded_output Output of Viterbi decoder
*/
void dci_decoding(uint8_t DCI_LENGTH,
		  uint8_t DCI_FMT,
		  int8_t *e,
		  uint8_t *decoded_output);

/** \brief Do 36.213 DCI decoding procedure by searching different RNTI options and aggregation levels.  Currently does
    not employ the complexity reducing procedure based on RNTI.
    @param phy_vars_ue UE variables
    @param dci_alloc Pointer to DCI_ALLOC_t array to store results for DLSCH/ULSCH programming
    @param do_common If 1 perform search in common search-space else ue-specific search-space 
    @param eNB_id eNB Index on which to act
    @param subframe Index of subframe
    @returns bitmap of occupied CCE positions (i.e. those detected)
*/
uint16_t dci_decoding_procedure(PHY_VARS_UE *phy_vars_ue,
				DCI_ALLOC_t *dci_alloc,
				int do_common,
				int16_t eNB_id,
				uint8_t subframe);


uint16_t dci_decoding_procedure_emul(LTE_UE_PDCCH **lte_ue_pdcch_vars,
				     uint8_t num_ue_spec_dci,
				     uint8_t num_common_dci,
				     DCI_ALLOC_t *dci_alloc_tx,
				     DCI_ALLOC_t *dci_alloc_rx,
				     int16_t eNB_id);

/** \brief Compute Q (modulation order) based on I_MCS PDSCH.  Implements table 7.1.7.1-1 from 36.213.
    @param I_MCS */
uint8_t get_Qm(uint8_t I_MCS);

/** \brief Compute Q (modulation order) based on I_MCS for PUSCH.  Implements table 8.6.1-1 from 36.213.
    @param I_MCS */
uint8_t get_Qm_ul(uint8_t I_MCS);

/** \brief Compute I_TBS (transport-block size) based on I_MCS for PDSCH.  Implements table 7.1.7.1-1 from 36.213.
    @param I_MCS */
uint8_t get_I_TBS(uint8_t I_MCS);

/** \brief Compute I_TBS (transport-block size) based on I_MCS for PUSCH.  Implements table 8.6.1-1 from 36.213.
    @param I_MCS */
unsigned char get_I_TBS_UL(unsigned char I_MCS);

/** \brief Compute Q (modulation order) based on downlink I_MCS. Implements table 7.1.7.1-1 from 36.213.
    @param I_MCS
    @param nb_rb
    @return Transport block size */
uint64_t get_TBS_DL(uint8_t mcs, uint16_t nb_rb);

/** \brief Compute Q (modulation order) based on uplink I_MCS. Implements table 7.1.7.1-1 from 36.213.
    @param I_MCS
    @param nb_rb
    @return Transport block size */
uint64_t get_TBS_UL(uint8_t mcs, uint16_t nb_rb);

/* \brief Return bit-map of resource allocation for a given DCI rballoc (RIV format) and vrb type
   @param vrb_type VRB type (0=localized,1=distributed)
   @param rb_alloc_dci rballoc field from DCI
*/
uint32_t get_rballoc(uint8_t vrb_type,uint16_t rb_alloc_dci);

/* \brief Return bit-map of resource allocation for a given DCI rballoc (RIV format) and vrb type
   @returns Transmission mode (1-7)
*/
uint8_t get_transmission_mode(uint16_t Mod_id, uint8_t CC_id, uint16_t rnti);


int16_t get_hundred_times_delta_IF(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t harq_pid);

/* \brief 
   @param ra_header Header of resource allocation (0,1) (See sections 7.1.6.1/7.1.6.2 of 36.213 Rel8.6)
   @param rb_alloc Bitmap allocation from DCI (format 1,2) 
   @returns number of physical resource blocks
*/
uint32_t conv_nprb(uint8_t ra_header,uint32_t rb_alloc,int N_RB_DL);

int get_G(LTE_DL_FRAME_PARMS *frame_parms,uint16_t nb_rb,uint32_t *rb_alloc,uint8_t mod_order,uint8_t Nl,uint8_t num_pdcch_symbols,int frame,uint8_t subframe);

int adjust_G(LTE_DL_FRAME_PARMS *frame_parms,uint32_t *rb_alloc,uint8_t mod_order,uint8_t subframe);
int adjust_G2(LTE_DL_FRAME_PARMS *frame_parms,uint32_t *rb_alloc,uint8_t mod_order,uint8_t subframe,uint8_t symbol);


#ifndef modOrder
#define modOrder(I_MCS,I_TBS) ((I_MCS-I_TBS)*2+2) // Find modulation order from I_TBS and I_MCS
#endif

/** \fn uint8_t I_TBS2I_MCS(uint8_t I_TBS);
    \brief This function maps I_tbs to I_mcs according to Table 7.1.7.1-1 in 3GPP TS 36.213 V8.6.0. Where there is two supported modulation orders for the same I_TBS then either high or low modulation is chosen by changing the equality of the two first comparisons in the if-else statement.
    \param I_TBS Index of Transport Block Size
    \return I_MCS given I_TBS
*/
uint8_t I_TBS2I_MCS(uint8_t I_TBS);

/** \fn uint8_t SE2I_TBS(float SE,
    uint8_t N_PRB,
    uint8_t symbPerRB);
    \brief This function maps a requested throughput in number of bits to I_tbs. The throughput is calculated as a function of modulation order, RB allocation and number of symbols per RB. The mapping orginates in the "Transport block size table" (Table 7.1.7.2.1-1 in 3GPP TS 36.213 V8.6.0)
    \param SE Spectral Efficiency (before casting to integer, multiply by 1024, remember to divide result by 1024!)
    \param N_PRB Number of PhysicalResourceBlocks allocated \sa lte_frame_parms->N_RB_DL
    \param symbPerRB Number of symbols per resource block allocated to this channel
    \return I_TBS given an SE and an N_PRB
*/
uint8_t SE2I_TBS(float SE,
		 uint8_t N_PRB,
		 uint8_t symbPerRB);
/** \brief This function generates the sounding reference symbol (SRS) for the uplink according to 36.211 v8.6.0. If IFFT_FPGA is defined, the SRS is quantized to a QPSK sequence.
    @param frame_parms LTE DL Frame Parameters
    @param soundingrs_ul_config_dedicated Dynamic configuration from RRC during Connection Establishment
    @param txdataF pointer to the frequency domain TX signal
    @returns 0 on success*/
int generate_srs_rx(LTE_DL_FRAME_PARMS *frame_parms,
		    SOUNDINGRS_UL_CONFIG_DEDICATED *soundingrs_ul_config_dedicated,		    
		    int *txdataF);

int32_t generate_srs_tx_emul(PHY_VARS_UE *phy_vars_ue,
			 uint8_t subframe);

/*!
  \brief This function is similar to generate_srs_tx but generates a conjugate sequence for channel estimation. If IFFT_FPGA is defined, the SRS is quantized to a QPSK sequence.
  @param phy_vars_ue Pointer to PHY_VARS structure
  @param eNB_id Index of destination eNB for this SRS
  @param amp Linear amplitude of SRS
  @param subframe Index of subframe on which to act
  @returns 0 on success, -1 on error with message
*/

int32_t generate_srs_tx(PHY_VARS_UE *phy_vars_ue,
		    uint8_t eNB_id,
		    int16_t amp,
		    uint32_t subframe);

/*!
  \brief This function generates the downlink reference signal for the PUSCH according to 36.211 v8.6.0. The DRS occuies the RS defined by rb_alloc and the symbols 2 and 8 for extended CP and 3 and 10 for normal CP.
*/

int32_t generate_drs_pusch(PHY_VARS_UE *phy_vars_ue,
		       uint8_t eNB_id,
		       int16_t amp,
		       uint32_t subframe,
		       uint32_t first_rb,
		       uint32_t nb_rb,
		       uint8_t ant);

/*!
  \brief This function initializes the Group Hopping, Sequence Hopping and nPRS sequences for PUCCH/PUSCH according to 36.211 v8.6.0. It should be called after configuration of UE (reception of SIB2/3) and initial configuration of eNB (or after reconfiguration of cell-specific parameters).
  @param frame_parms Pointer to a LTE_DL_FRAME_PARMS structure (eNB or UE)*/
void init_ul_hopping(LTE_DL_FRAME_PARMS *frame_parms);

int32_t compareints (const void * a, const void * b);


void ulsch_modulation(mod_sym_t **txdataF,
		      int16_t amp,
		      frame_t frame,
		      uint32_t subframe,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_UE_ULSCH_t *ulsch);


void ulsch_extract_rbs_single(int32_t **rxdataF,
			      int32_t **rxdataF_ext,
			      uint32_t first_rb,
			      uint32_t nb_rb,
			      uint8_t l,
			      uint8_t Ns,
			      LTE_DL_FRAME_PARMS *frame_parms);

uint8_t subframe2harq_pid(LTE_DL_FRAME_PARMS *frame_parms,frame_t frame,uint8_t subframe);
uint8_t subframe2harq_pid_eNBrx(LTE_DL_FRAME_PARMS *frame_parms,uint8_t subframe);

int generate_ue_dlsch_params_from_dci(uint8_t subframe,
                                      void *dci_pdu,
                                      rnti_t rnti,
                                      DCI_format_t dci_format,
                                      LTE_UE_DLSCH_t **dlsch,
                                      LTE_DL_FRAME_PARMS *frame_parms,
                                      PDSCH_CONFIG_DEDICATED *pdsch_config_dedicated,
                                      uint16_t si_rnti,
                                      uint16_t ra_rnti,
                                      uint16_t p_rnti);

int32_t generate_eNB_dlsch_params_from_dci(uint8_t subframe,
				       void *dci_pdu,
				       rnti_t rnti,
				       DCI_format_t dci_format,
				       LTE_eNB_DLSCH_t **dlsch_eNB,
				       LTE_DL_FRAME_PARMS *frame_parms,
				       PDSCH_CONFIG_DEDICATED *pdsch_config_dedicated,
				       uint16_t si_rnti,
				       uint16_t ra_rnti,
				       uint16_t p_rnti,
				       uint16_t DL_pmi_single);

int32_t generate_eNB_ulsch_params_from_rar(uint8_t *rar_pdu,
				       frame_t frame,
				       uint8_t subframe,
				       LTE_eNB_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms);

int generate_ue_ulsch_params_from_dci(void *dci_pdu,
                                      rnti_t rnti,
				      uint8_t subframe,
				      DCI_format_t dci_format,
				      PHY_VARS_UE *phy_vars_ue,
				      uint16_t si_rnti,
				      uint16_t ra_rnti,
				      uint16_t p_rnti,
				      uint16_t cba_rnti,
				      uint8_t eNB_id,
				      uint8_t use_srs);

int32_t generate_ue_ulsch_params_from_rar(PHY_VARS_UE *phy_vars_ue,
				      uint8_t eNB_id);
double sinr_eff_cqi_calc(PHY_VARS_UE *phy_vars_ue,
			 uint8_t eNB_id);
int generate_eNB_ulsch_params_from_dci(void *dci_pdu,
				       rnti_t rnti,
				       uint8_t subframe,
				       DCI_format_t dci_format,
				       uint8_t UE_id,
				       PHY_VARS_eNB *PHY_vars_eNB,
				       uint16_t si_rnti,
				       uint16_t ra_rnti,
				       uint16_t p_rnti,
	 				   uint16_t cba_rnti,
				       uint8_t use_srs);

#ifdef USER_MODE
void dump_ulsch(PHY_VARS_eNB *phy_vars_eNb,uint8_t subframe, uint8_t UE_id);

void dump_dlsch(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t subframe,uint8_t harq_pid);
void dump_dlsch_SI(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t subframe);
void dump_dlsch_ra(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t subframe);

void dump_dlsch2(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint16_t coded_bits_per_codeword);
#endif

int dump_dci(LTE_DL_FRAME_PARMS *frame_parms, DCI_ALLOC_t *dci);

int dump_ue_stats(PHY_VARS_UE *phy_vars_ue, char* buffer, int length, runmode_t mode, int input_level_dBm);
int dump_eNB_stats(PHY_VARS_eNB *phy_vars_eNB, char* buffer, int length);



void generate_pcfich_reg_mapping(LTE_DL_FRAME_PARMS *frame_parms);

void pcfich_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		       uint8_t subframe,
		       uint8_t *b,
		       uint8_t *bt);

void pcfich_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
			 uint8_t subframe,
			 int16_t *d);

void generate_pcfich(uint8_t num_pdcch_symbols,
		     int16_t amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     mod_sym_t **txdataF,
		     uint8_t subframe);

uint8_t rx_pcfich(LTE_DL_FRAME_PARMS *frame_parms,
		  uint8_t subframe,
		  LTE_UE_PDCCH *lte_ue_pdcch_vars,
		  MIMO_mode_t mimo_mode);

void generate_phich_reg_mapping(LTE_DL_FRAME_PARMS *frame_parms);


void init_transport_channels(uint8_t);

void generate_RIV_tables(void);

/*!
  \brief This function performs the initial cell search procedure - PSS detection, SSS detection and PBCH detection.  At the 
  end, the basic frame parameters are known (Frame configuration - TDD/FDD and cyclic prefix length, 
  N_RB_DL, PHICH_CONFIG and Nid_cell) and the UE can begin decoding PDCCH and DLSCH SI to retrieve the rest.  Once these
  parameters are know, the routine calls some basic initialization routines (cell-specific reference signals, etc.)
  @param phy_vars_ue Pointer to UE variables
*/
int initial_sync(PHY_VARS_UE *phy_vars_ue, runmode_t mode);

void rx_ulsch(PHY_VARS_eNB *phy_vars_eNB,
	      uint32_t subframe,
	      uint8_t eNB_id,  // this is the effective sector id
	      uint8_t UE_id,
	      LTE_eNB_ULSCH_t **ulsch,
	      uint8_t cooperation_flag);

void rx_ulsch_emul(PHY_VARS_eNB *phy_vars_eNB,
		   uint8_t subframe,
		   uint8_t sect_id,
		   uint8_t UE_index);

/*!
  \brief Encoding of PUSCH/ACK/RI/ACK from 36-212.
  @param a Pointer to ulsch SDU
  @param frame_parms Pointer to Frame parameters
  @param ulsch Pointer to ulsch descriptor
  @param harq_pid HARQ process ID
  @param tmode Transmission mode (1-7)
  @param control_only_flag Generate PUSCH with control information only
  @param Nbundled Parameter for ACK/NAK bundling (36.213 Section 7.3)
*/
uint32_t ulsch_encoding(uint8_t *a,
			PHY_VARS_UE *phy_vars_ue,
			uint8_t harq_pid,
			uint8_t eNB_id,
			uint8_t tmode,
			uint8_t control_only_flag,
			uint8_t Nbundled);

/*!
  \brief Encoding of PUSCH/ACK/RI/ACK from 36-212 for emulation
  @param ulsch_buffer Pointer to ulsch SDU
  @param phy_vars_ue Pointer to UE top-level descriptor
  @param eNB_id ID of eNB receiving this PUSCH
  @param harq_pid HARQ process ID
  @param control_only_flag Generate PUSCH with control information only
*/
int32_t ulsch_encoding_emul(uint8_t *ulsch_buffer,
			PHY_VARS_UE *phy_vars_ue,
			uint8_t eNB_id,
			uint8_t harq_pid,
			uint8_t control_only_flag);

/*!
  \brief Decoding of PUSCH/ACK/RI/ACK from 36-212.
  @param phy_vars_eNB Pointer to eNB top-level descriptor
  @param UE_id ID of UE transmitting this PUSCH
  @param subframe Index of subframe for PUSCH
  @param control_only_flag Receive PUSCH with control information only
  @param Nbundled Nbundled parameter for ACK/NAK scrambling from 36-212/36-213
  @param llr8_flag If 1, indicate that the 8-bit turbo decoder should be used
  @returns 0 on success
*/
unsigned int  ulsch_decoding(PHY_VARS_eNB *phy_vars_eNB,
			     uint8_t UE_id,
			     uint8_t subframe,
			     uint8_t control_only_flag,
			     uint8_t Nbundled,
			     uint8_t llr8_flag);

uint32_t ulsch_decoding_emul(PHY_VARS_eNB *phy_vars_eNB,
			     uint8_t subframe,
			     uint8_t UE_index,
				 uint16_t *crnti);

void generate_phich_top(PHY_VARS_eNB *phy_vars_eNB,
			uint8_t subframe,
			int16_t amp,
			uint8_t sect_id,
			uint8_t abstraction_flag);

/* \brief  This routine demodulates the PHICH and updates PUSCH/ULSCH parameters.
   @param phy_vars_ue Pointer to UE variables
   @param subframe Subframe of received PDCCH/PHICH
   @param eNB_id Index of eNB
*/

void rx_phich(PHY_VARS_UE *phy_vars_ue,
	      uint8_t subframe,
	      uint8_t eNB_id);


/** \brief  This routine provides the relationship between a PHICH TXOp and its corresponding PUSCH subframe (Table 8.3.-1 from 36.213).
    @param frame_parms Pointer to DL frame configuration parameters
    @param subframe Subframe of received/transmitted PHICH
    @returns subframe of PUSCH transmission
*/
uint8_t phich_subframe2_pusch_subframe(LTE_DL_FRAME_PARMS *frame_parms,uint8_t subframe);

/** \brief  This routine provides the relationship between a PHICH TXOp and its corresponding PUSCH frame (Table 8.3.-1 from 36.213).
    @param frame_parms Pointer to DL frame configuration parameters
    @param frame Frame of received/transmitted PHICH
    @param subframe Subframe of received/transmitted PHICH
    @returns frame of PUSCH transmission
*/
uint8_t phich_frame2_pusch_frame(LTE_DL_FRAME_PARMS *frame_parms,frame_t frame,uint8_t subframe);;

void print_CQI(void *o,UCI_format_t uci_format,uint8_t eNB_id);

void extract_CQI(void *o,UCI_format_t uci_format,LTE_eNB_UE_stats *stats,uint16_t * crnti, uint8_t * access_mode);

void fill_CQI(void *o,UCI_format_t uci_format,PHY_MEASUREMENTS *meas,uint8_t eNB_id, rnti_t rnti, uint8_t trans_mode,double sinr_eff);
void reset_cba_uci(void *o);

uint16_t quantize_subband_pmi(PHY_MEASUREMENTS *meas,uint8_t eNB_id);
uint16_t quantize_subband_pmi2(PHY_MEASUREMENTS *meas,uint8_t eNB_id,uint8_t a_id);

uint32_t pmi2hex_2Ar1(uint16_t pmi);

uint32_t pmi2hex_2Ar2(uint8_t pmi);

uint32_t cqi2hex(uint16_t cqi);

uint16_t computeRIV(uint16_t N_RB_DL,uint16_t RBstart,uint16_t Lcrbs);

uint32_t pmi_extend(LTE_DL_FRAME_PARMS *frame_parms,uint8_t wideband_pmi);


uint16_t get_nCCE(uint8_t num_pdcch_symbols,LTE_DL_FRAME_PARMS *frame_parms,uint8_t mi);

uint16_t get_nquad(uint8_t num_pdcch_symbols,LTE_DL_FRAME_PARMS *frame_parms,uint8_t mi);

uint8_t get_mi(LTE_DL_FRAME_PARMS *frame,uint8_t subframe);

uint16_t get_nCCE_max(uint8_t Mod_id,uint8_t CC_id);

uint8_t get_num_pdcch_symbols(uint8_t num_dci,DCI_ALLOC_t *dci_alloc,LTE_DL_FRAME_PARMS *frame_parms,uint8_t subframe);

void pdcch_interleaving(LTE_DL_FRAME_PARMS *frame_parms,mod_sym_t **z, mod_sym_t **wbar,uint8_t n_symbols_pdcch,uint8_t mi);

void pdcch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
			uint8_t subframe,
			int8_t* llr,
			uint32_t length);

void pdcch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		      uint8_t subframe,
		      uint8_t *e,
		      uint32_t length);

void dlsch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		      int mbsfn_flag,
		      LTE_eNB_DLSCH_t *dlsch,
		      int G,
		      uint8_t q,
		      uint8_t Ns);

void dlsch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
			int mbsfn_flag,
			LTE_UE_DLSCH_t *dlsch,
			int G,
			int16_t* llr,
			uint8_t q,
			uint8_t Ns);

void init_ncs_cell(LTE_DL_FRAME_PARMS *frame_parms,uint8_t ncs_cell[20][7]);

void generate_pucch(mod_sym_t **txdataF,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    uint8_t ncs_cell[20][7],
		    PUCCH_FMT_t fmt,
		    PUCCH_CONFIG_DEDICATED *pucch_config_dedicated,
		    uint16_t n1_pucch,
		    uint16_t n2_pucch,
		    uint8_t shortened_format,
		    uint8_t *payload,
		    int16_t amp,
		    uint8_t subframe);

void generate_pucch_emul(PHY_VARS_UE *phy_vars_ue,
			 PUCCH_FMT_t format,
			 uint8_t ncs1,
			 uint8_t *pucch_ack_payload,
			 uint8_t sr,
			 uint8_t subframe);


int32_t rx_pucch(PHY_VARS_eNB *phy_vars_eNB,
	     PUCCH_FMT_t fmt,
	     uint8_t UE_id,
	     uint16_t n1_pucch,
	     uint16_t n2_pucch,
	     uint8_t shortened_format,
	     uint8_t *payload,
	     uint8_t subframe,
	     uint8_t pucch1_thres);

int32_t rx_pucch_emul(PHY_VARS_eNB *phy_vars_eNB,
		  uint8_t UE_index,
		  PUCCH_FMT_t fmt,
		  uint8_t n1_pucch_sel,
		  uint8_t *payload,
		  uint8_t subframe);


/*!
  \brief Check for PRACH TXop in subframe
  @param frame_parms Pointer to LTE_DL_FRAME_PARMS
  @param frame frame index to check
  @param subframe subframe index to check
  @returns 0 on success
*/
int is_prach_subframe(LTE_DL_FRAME_PARMS *frame_parms,frame_t frame, uint8_t subframe);

/*!
  \brief Generate PRACH waveform
  @param phy_vars_ue Pointer to ue top-level descriptor
  @param eNB_id Index of destination eNB
  @param subframe subframe index to operate on
  @param index of preamble (0-63)
  @param Nf System frame number
  @returns 0 on success
  
*/
int32_t generate_prach(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t subframe,uint16_t Nf);

/*!
  \brief Process PRACH waveform
  @param phy_vars_eNB Pointer to eNB top-level descriptor
  @param subframe subframe index to operate on
  @param preamble_energy_list List of energies for each candidate preamble
  @param preamble_delay_list List of delays for each candidate preamble
  @param Nf System frame number
  @param tdd_mapindex Index of PRACH resource in Table 5.7.1-4 (TDD)
  @returns 0 on success
  
*/
void rx_prach(PHY_VARS_eNB *phy_vars_eNB,uint8_t subframe,uint16_t *preamble_energy_list, uint16_t *preamble_delay_list, uint16_t Nf, uint8_t tdd_mapindex);

/*!
  \brief Helper for MAC, returns number of available PRACH in TDD for a particular configuration index
  @param frame_parms Pointer to LTE_DL_FRAME_PARMS structure
  @returns 0-5 depending on number of available prach
*/
uint8_t get_num_prach_tdd(LTE_DL_FRAME_PARMS *frame_parms);

/*!
  \brief Return the PRACH format as a function of the Configuration Index and Frame type.
  @param prach_ConfigIndex PRACH Configuration Index
  @param frame_type 0-FDD, 1-TDD
  @returns 0-1 accordingly
*/
uint8_t get_prach_fmt(uint8_t prach_ConfigIndex,lte_frame_type_t frame_type);

/*!
  \brief Helper for MAC, returns frequency index of PRACH resource in TDD for a particular configuration index
  @param frame_parms Pointer to LTE_DL_FRAME_PARMS structure
  @returns 0-5 depending on number of available prach
*/
uint8_t get_fid_prach_tdd(LTE_DL_FRAME_PARMS *frame_parms,uint8_t tdd_map_index);

/*!
  \brief Comp ute DFT of PRACH ZC sequences.  Used for generation of prach in UE and reception of PRACH in eNB.
  @param prach_config_common Pointer to prachConfigCommon structure
  @param Xu DFT output 
*/
void compute_prach_seq(PRACH_CONFIG_COMMON *prach_config_common,
		       lte_frame_type_t frame_type,
		       uint32_t X_u[64][839]);

void init_prach_tables(int N_ZC);

/*!
  \brief Return the status of MBSFN in this frame/subframe
  @param frame Frame index
  @param subframe Subframe index
  @param frame_parms Pointer to frame parameters
  @returns 1 if subframe is for MBSFN
*/
int is_pmch_subframe(frame_t frame, int subframe, LTE_DL_FRAME_PARMS *frame_parms);
 
//ICIC algos
uint8_t Get_SB_size(uint8_t n_rb_dl);
//end ALU's algo


uint32_t dlsch_decoding_abstraction(double *dlsch_MIPB,
				    LTE_DL_FRAME_PARMS *lte_frame_parms,
				    LTE_UE_DLSCH_t *dlsch,
				    uint8_t subframe,
				    uint8_t num_pdcch_symbols);

// DL power control functions
double get_pa_dB(PDSCH_CONFIG_DEDICATED *pdsch_config_dedicated);

double computeRhoA_eNB(PDSCH_CONFIG_DEDICATED *pdsch_config_dedicated,  
                       LTE_eNB_DLSCH_t *dlsch_eNB );

double computeRhoB_eNB(PDSCH_CONFIG_DEDICATED  *pdsch_config_dedicated,
                       PDSCH_CONFIG_COMMON *pdsch_config_common,
                       uint8_t n_antenna_port,
                       LTE_eNB_DLSCH_t *dlsch_eNB);

double computeRhoA_UE(PDSCH_CONFIG_DEDICATED *pdsch_config_dedicated,  
                      LTE_UE_DLSCH_t *dlsch_ue,
                      uint8_t dl_power_off);

double computeRhoB_UE(PDSCH_CONFIG_DEDICATED  *pdsch_config_dedicated,
                      PDSCH_CONFIG_COMMON *pdsch_config_common,
                      uint8_t n_antenna_port,
                      LTE_UE_DLSCH_t *dlsch_ue,
                      uint8_t dl_power_off);

/*void compute_sqrt_RhoAoRhoB(PDSCH_CONFIG_DEDICATED  *pdsch_config_dedicated,
  PDSCH_CONFIG_COMMON *pdsch_config_common,
  uint8_t n_antenna_port,
  LTE_UE_DLSCH_t *dlsch_ue);
*/
/**@}*/
#endif
