#include "PHY/defs.h"

void allocate_REs_in_RB(int **txdataF,
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

void generate_dlsch(int **txdataF,
		    short amp,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    unsigned short Ntti,
		    unsigned short N_RB,
		    unsigned int  *rb_alloc,
		    unsigned char slot_alloc,
		    unsigned char *input_data,
		    unsigned int Nbytes,
		    unsigned int Ncwords,
		    unsigned char mod_order,
		    MIMO_mode_t mimo_mode,
		    unsigned char crc_len);

void generate_pilots(int **txdataF,
		     short amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     unsigned short Ntti);

void generate_pss(int **txdataF,
		  short amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  unsigned short Ntti);
