#ifndef __LTE_TRANSPORT_DEFS__H__
#define __LTE_TRANSPORT_DEFS__H__
#include "PHY/defs.h"

/** \fn allocate_REs_in_RB(int **txdataF,
			unsigned int *jj,
			unsigned short re_offset,
			unsigned int tti_size,
			unsigned int symbol_offset,
			unsigned char *output,
			MIMO_mode_t mimo_mode,
			unsigned char pilots,
			unsigned char first_pilot,
			unsigned short Ntti,
			unsigned char mod_order,
			unsigned short gain_lin_QPSK,
			unsigned short gain_lin_16QAM1,
			unsigned short gain_lin_16QAM2,
			unsigned short gain_lin_64QAM1,
			unsigned short gain_lin_64QAM2,
			unsigned short gain_lin_64QAM3);

\brief Fills RB with data
\param txdataF pointer to output data (frequency domain signal)
\param jj index to output
\param re_offset index of the first RE of the RB
\param tti_size number of samples in TTI (without CP)
\param symbol_offset index to the OFDM symbol
\param output output of the channel coder, one bit per byte
\param mimo_mode
\param pilots =1 if symbol_offset is an OFDM symbol that contains pilots, 0 otherwise
\param first_pilot =1 if symbol offset it the first OFDM symbol in a slot, 0 otherwise
\param Ntti,
\param mod_order 2=QPSK, 4=16QAM, 6=64QAM
\param gain_lin_QPSK,
\param gain_lin_16QAM1,
\param gain_lin_16QAM2,
\param gain_lin_64QAM1,
\param gain_lin_64QAM2,
\param gain_lin_64QAM3
*/

void allocate_REs_in_RB(int **txdataF,
			unsigned int *jj,
			unsigned short re_offset,
			unsigned int symbol_offset,
			unsigned char *output,
			MIMO_mode_t mimo_mode,
			unsigned char pilots,
			unsigned char first_pilot,
			unsigned char mod_order,
			short amp,
			unsigned int *re_allocated,
			unsigned char skip_dc,
			LTE_DL_FRAME_PARMS *frame_parms);

void generate_dlsch(int **txdataF,
		    short amp,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    unsigned short N_RB,
		    unsigned int  *rb_alloc,
		    unsigned char slot_alloc,
		    unsigned char *input_data,
		    unsigned int Nbytes,
		    unsigned char mod_order,
		    MIMO_mode_t mimo_mode,
		    unsigned char rmseed,
		    unsigned char crc_len);

void generate_pilots(int **txdataF,
		     short amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     unsigned short Ntti);

void generate_pss(int **txdataF,
		  short amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  unsigned short Ntti);

int generate_pbch(int **txdataF,
		  int amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  unsigned char *pbch_pdu);


void dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    short *dlsch_llr,
		    unsigned char symbol,
		    unsigned short nb_rb);

void dlsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     int **rxdataF_comp,
		     short *dlsch_llr,
		     int **dl_ch_mag,
		     unsigned char symbol,
		     unsigned short nb_rb);

void dlsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     int **rxdataF_comp,
		     char *dlsch_llr,
		     int **dl_ch_mag,
		     int **dl_ch_magb,
		     unsigned char symbol,
		     unsigned short nb_rb);

void dlsch_siso(LTE_DL_FRAME_PARMS *frame_parms,
		int **rxdataF_comp,
		unsigned char l,
		unsigned short nb_rb);

void dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    int **dl_ch_mag,
		    int **dl_ch_magb,
		    unsigned char symbol,
		    unsigned short nb_rb);

void dlsch_antcyc(LTE_DL_FRAME_PARMS *frame_parms,
		  int **rxdataF_comp,
		  int **dl_ch_mag,
		  int **dl_ch_magb,
		  unsigned char symbol,
		  unsigned short nb_rb);

void dlsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			 int **rxdataF_comp,
			 int **dl_ch_mag,
			 int **dl_ch_magb,
			 unsigned char symbol,
			 unsigned short nb_rb);

unsigned short dlsch_extract_rbs_single(int **rxdataF,
					int **dl_ch_estimates,
					int **rxdataF_ext,
					int **dl_ch_estimates_ext,
					unsigned int *rb_alloc,
					unsigned char symbol,
					LTE_DL_FRAME_PARMS *frame_parms);
unsigned short dlsch_extract_rbs_dual(int **rxdataF,
				      int **dl_ch_estimates,
				      int **rxdataF_ext,
				      int **dl_ch_estimates_ext,
				      unsigned int *rb_alloc,
				      unsigned char symbol,
				      LTE_DL_FRAME_PARMS *frame_parms);


void dlsch_channel_compensation(int **rxdataF_ext,
				int **dl_ch_estimates_ext,
				int **dl_ch_mag,
				int **dl_ch_magb,
				int **rxdataF_comp,
				LTE_DL_FRAME_PARMS *frame_parms,
				unsigned char symbol,
				unsigned char mod_order,
				unsigned short nb_rb,
				unsigned char output_shift);


void dlsch_channel_level(int **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 int *avg,
			 unsigned short nb_rb);

void rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	      LTE_UE_DLSCH *lte_ue_dlsch_vars,
	      LTE_DL_FRAME_PARMS *frame_parms,
	      unsigned char symbol,
	      unsigned int *rb_alloc,
	      unsigned char mod_order,
	      MIMO_mode_t mimo_mode);

void rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_DLSCH *lte_ue_dlsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     MIMO_mode_t mimo_mode);

#endif
