#ifndef __LTE_TRANSPORT_DEFS__H__
#define __LTE_TRANSPORT_DEFS__H__
#include "PHY/defs.h"

#define MAX_DLSCH_PAYLOAD_BYTES 768
#define NSOFT 1827072
#define LTE_NULL 2 

typedef struct {
  unsigned char active;
  unsigned short payload_size_bytes;
  unsigned char *payload;
  unsigned int RTC;
  unsigned char round;
  unsigned char mod_order;
  MIMO_mode_t mimo_mode;
  unsigned char d[3][3*(96+3+(3*6144))];  // allow for up to 3 length-6144 codewords in a TTI
  unsigned char w[3][3*6144];
  unsigned char C;
  unsigned char Nl;
} LTE_eNb_HARQ_t;

typedef struct {
  LTE_eNb_HARQ_t *harq_processes[8];
  unsigned char rvidx;
  unsigned char crc_len;
  unsigned char e[3][3*6144];
  unsigned char Mdlharq;
  unsigned char Kmimo;
} LTE_eNb_DLSCH_t;

typedef struct {
  unsigned char active;
  unsigned short payload_size_bytes;
  unsigned char *payload;
  unsigned int RTC;
  unsigned char round;
  unsigned char mod_order;
  MIMO_mode_t mimo_mode;
  short w[3][3*6144];
  unsigned char C;
  unsigned char Nl;
} LTE_UE_HARQ_t;

typedef struct {
  LTE_UE_HARQ_t *harq_processes[8];
  unsigned char rvidx;
  unsigned char crc_len;
  unsigned char Mdlharq;
  unsigned char Kmimo;
} LTE_UE_DLSCH_t;

/** \fn unsigned int sub_block_interleaving_turbo(unsigned int D, unsigned char *d,unsigned char *w)
\brief This is the subblock interleaving algorithm from 36-212 (Release 8, 8.6 2009-03) , pp. 15-16. 
This function takes the d-sequence and generates the w-sequence.  The nu-sequence from 36-212 is implicit.
\param D Number of systematic bits plus 4 (plus 4 for termination)
\param d Pointer to input (d-sequence, turbo code output)
\param w Pointer to output (w-sequence, interleaver output)
\returns Interleaving matrix cardinality (Kpi from 36-212)
*/

unsigned int sub_block_interleaving_turbo(unsigned int D, unsigned char *d,unsigned char *w);

void sub_block_deinterleaving_turbo(unsigned int D, short *d,short *w);

/** \fn generate_dummy_w(unsigned int D, unsigned char *w)
\brief This function generates a dummy interleaved sequence (first row) for receiver, in order to identify
the NULL positions used to make the matrix complete.
\param D Number of systematic bits plus 4 (plus 4 for termination)
\param w This is the dummy sequence (first row), it will contain zeros and at most 31 "LTE_NULL" values
\returns Interleaving matrix cardinality (Kpi from 36-212)
*/

unsigned int generate_dummy_w(unsigned int D, unsigned char *w);


/** \fn lte_rate_matching_turbo(unsigned int RTC,
			     unsigned int G, 
			     unsigned char *w,
			     unsigned char *e, 
			     unsigned char C, 
			     unsigned int Nsoft, 
			     unsigned char Mdlharq,
			     unsigned char Kmimo,
			     unsigned char rvidx,
			     unsigned char Qm, 
			     unsigned char Nl, 
			     unsigned char r)

\brief This is the LTE rate matching algorithm for Turbo-coded channels (e.g. DLSCH,ULSCH).  It is taken directly from 36-212 (Rel 8 8.6, 2009-03), pp.16-18 )
\param RTC R^TC_subblock from subblock interleaver (number of rows in interleaving matrix)
\param G This the number of coded transport bits allocated in sub-frame
\param w This is a pointer to the w-sequence (second interleaver output)
\param e This is a pointer to the e-sequence (rate matching output, channel input/output bits)
\param C Number of segments (codewords) in the sub-frame
\param Nsoft Total number of soft bits (from UE capabilities in 36-306)
\param Mdlharq Number of HARQ rounds 
\param Kmimo MIMO capability for this DLSCH (0 = no MIMO)
\param rvidx round index (0-3)
\param Qm modulation order (2,4,6)
\param Nl number of layers (1,2)
\param r segment number
*/


void lte_rate_matching_turbo(unsigned int RTC,
			     unsigned int G, 
			     unsigned char *w,
			     unsigned char *e, 
			     unsigned char C, 
			     unsigned int Nsoft, 
			     unsigned char Mdlharq,
			     unsigned char Kmimo,
			     unsigned char rvidx,
			     unsigned char Qm, 
			     unsigned char Nl, 
			     unsigned char r);

/** \fn lte_rate_matching_turbo_rx(unsigned int RTC,
				unsigned int G, 
				short *w,
				unsigned char *dummy_w,
				short *soft_input, 
				unsigned char C, 
				unsigned int Nsoft, 
				unsigned char Mdlharq,
				unsigned char Kmimo,
				unsigned char rvidx,
				unsigned char Qm, 
				unsigned char Nl, 
				unsigned char r)

\brief This is the LTE rate matching algorithm for Turbo-coded channels (e.g. DLSCH,ULSCH).  It is taken directly from 36-212 (Rel 8 8.6, 2009-03), pp.16-18 )
\param RTC R^TC_subblock from subblock interleaver (number of rows in interleaving matrix)
\param G This the number of coded transport bits allocated in sub-frame
\param w This is a pointer to the soft w-sequence (second interleaver output) with soft-combined outputs from successive HARQ rounds 
\param dummy_w This is the first row of the interleaver matrix for identifying/discarding the "LTE-NULL" positions
\param soft_input This is a pointer to the soft channel output 
\param C Number of segments (codewords) in the sub-frame
\param Nsoft Total number of soft bits (from UE capabilities in 36-306)
\param Mdlharq Number of HARQ rounds 
\param Kmimo MIMO capability for this DLSCH (0 = no MIMO)
\param rvidx round index (0-3)
\param Qm modulation order (2,4,6)
\param Nl number of layers (1,2)
\param r segment number
*/

void lte_rate_matching_turbo_rx(unsigned int RTC,
				unsigned int G, 
				short *w,
				unsigned char *dummy_w,
				short *soft_input, 
				unsigned char C, 
				unsigned int Nsoft, 
				unsigned char Mdlharq,
				unsigned char Kmimo,
				unsigned char rvidx,
				unsigned char Qm, 
				unsigned char Nl, 
				unsigned char r);

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
		    LTE_eNb_DLSCH_t *dlsch,
		    unsigned char harq_pid,
		    unsigned short N_RB,
		    unsigned int  *rb_alloc,
		    unsigned char slot_alloc);


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
