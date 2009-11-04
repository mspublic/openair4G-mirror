#ifndef __LTE_TRANSPORT_DEFS__H__
#define __LTE_TRANSPORT_DEFS__H__
#include "PHY/defs.h"

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
#endif
