/* file: defs.h
   purpose: data structures and function prototypes for LTE_TRANSPORT procedures (TX/RX)
   author: raymond.knopp@eurecom.fr, florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it
   date: 21.10.2009 
*/

#ifndef __LTE_TRANSPORT_DEFS__H__
#define __LTE_TRANSPORT_DEFS__H__
#include "PHY/defs.h"


/** Functions and structures below implement 36-212 **/

#define MAX_DLSCH_PAYLOAD_BYTES 768
#define NSOFT 1827072
#define LTE_NULL 2 


typedef struct {
  unsigned char active;
  unsigned short payload_size_bytes;
  unsigned char *payload;
  unsigned char *payload_segments[8];
  unsigned int RTC;
  unsigned char round;
  unsigned char mod_order;
  MIMO_mode_t mimo_mode;
  unsigned char d[3][3*(96+3+(3*6144))];  // allow for up to 3 length-6144 codewords in a TTI
  unsigned char w[3][3*6144];
  unsigned int C;
  unsigned int Cminus;
  unsigned int Cplus;
  unsigned int Kminus;
  unsigned int Kplus;
  unsigned int F;
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
  unsigned char *payload_segments[8];
  unsigned int RTC;
  unsigned char round;
  unsigned char mod_order;
  MIMO_mode_t mimo_mode;
  short w[3][3*6144];
  unsigned int C;
  unsigned int Cminus;
  unsigned int Cplus;
  unsigned int Kminus;
  unsigned int Kplus;
  unsigned int F;
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

/** \fn dlsch_encoding(unsigned char *input_buffer,
		    unsigned short input_buffer_length,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_eNb_DLSCH_t *dlsch,
		    unsigned char harq_pid,
		    unsigned short N_RB)
\brief This function performs a subset of the bit-coding functions for LTE as described in 36-212, Release 8.Support is limited to turbo-coded channels (DLSCH/ULSCH). The implemented functions are:
- CRC computation and addition
- Code block segmentation and sub-block CRC addition
- Channel coding (Turbo coding)
- Rate matching (sub-block interleaving, bit collection, selection and transmission
- Code block concatenation
@param input_buffer Pointer to input buffer for sub-frame
@param input_buffer_length Length of input buffer in bytes (note this is a small limtation w.r.t. LTE specs
@param frame_parms Pointer to frame descriptor structure
@param dlsch Pointer to dlsch to be encoded
@param harq_pid Identifier for harq process
@param N_RB Number of RBs in this allocation
*/
void dlsch_encoding(unsigned char *input_buffer,
		    unsigned short input_buffer_length,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_eNb_DLSCH_t *dlsch,
		    unsigned char harq_pid,
		    unsigned short N_RB);



/** Functions below implement 36-211 **/

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

void allocate_REs_in_RB(mod_sym_t **txdataF,
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

/** \fn dlsch_modulation(int **txdataF,
		      short amp,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_eNb_DLSCH_t *dlsch,
		      unsigned char harq_pid,
		      unsigned int  *rb_alloc)
\brief This function is the top-level routine for generation of the sub-frame signal (frequency-domain) for DLSCH.  
@param txdataF Table of pointers for frequency-domain TX signals
@param amp Amplitude of signal
@param frame_parms Pointer to frame descriptor
@param dlsch Pointer to DLSCH descriptor for this allocation
@param harq_pid Identifier of harq process
@param rb_alloc RB allocation vector
*/ 
void dlsch_modulation(mod_sym_t **txdataF,
		      short amp,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_eNb_DLSCH_t *dlsch,
		      unsigned char harq_pid,
		      unsigned int  *rb_alloc);

/** \fn generate_pilots(mod_sym_t **txdataF,
                        short amp,
		        LTE_DL_FRAME_PARMS *frame_parms,
		        unsigned short N);
\brief This function generates the frequency-domain pilots (cell-specific downlink reference signals)
for N subframes.
@param txdataF Table of pointers for frequency-domain TX signals
@param amp Amplitude of signal
@param frame_parms Pointer to frame descriptor
@param N Number of sub-frames to generate
*/
void generate_pilots(mod_sym_t **txdataF,
		     short amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     unsigned short N);

void generate_pss(mod_sym_t **txdataF,
		  short amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  unsigned short N);

int generate_pbch(mod_sym_t **txdataF,
		  int amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  unsigned char *pbch_pdu);

/** \fn dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    short *dlsch_llr,
		    unsigned char symbol,
		    unsigned short nb_rb)

\brief This function generates log-likelihood ratios (decoder input) for single-stream QPSK received waveforms.
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param dlsch_llr llr output
@param symbol OFDM symbol index in sub-frame
@param nb_rb number of RBs for this allocation
*/
void dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    short *dlsch_llr,
		    unsigned char symbol,
		    unsigned short nb_rb);

/** \fn dlsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                       int **rxdataF_comp,
		       short *dlsch_llr,
		       int **dl_ch_mag,
		       unsigned char symbol,
		       unsigned short nb_rb)
\brief This function generates log-likelihood ratios (decoder input) for single-stream 16QAM received waveforms
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param dlsch_llr llr output
@param dl_ch_mag Squared-magnitude of channel in each resource element position corresponding to allocation and weighted for mid-point in 16QAM constellation
@param symbol OFDM symbol index in sub-frame
@param nb_rb number of RBs for this allocation
*/

void dlsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     int **rxdataF_comp,
		     short *dlsch_llr,
		     int **dl_ch_mag,
		     unsigned char symbol,
		     unsigned short nb_rb);

/** \fn dlsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                       int **rxdataF_comp,
		       short *dlsch_llr,
		       int **dl_ch_mag,
		       int **dl_ch_magb,
		       unsigned char symbol,
		       unsigned short nb_rb)
\brief This function generates log-likelihood ratios (decoder input) for single-stream 16QAM received waveforms
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param dlsch_llr llr output
@param dl_ch_mag Squared-magnitude of channel in each resource element position corresponding to allocation, weighted by first mid-point of 64-QAM constellation
@param dl_ch_magb Squared-magnitude of channel in each resource element position corresponding to allocation, weighted by second mid-point of 64-QAM constellation
@param symbol OFDM symbol index in sub-frame
@param nb_rb number of RBs for this allocation
*/
void dlsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     int **rxdataF_comp,
		     char *dlsch_llr,
		     int **dl_ch_mag,
		     int **dl_ch_magb,
		     unsigned char symbol,
		     unsigned short nb_rb);

/** \fn dlsch_siso(LTE_DL_FRAME_PARMS *frame_parms,
		int **rxdataF_comp,
		unsigned char l,
		unsigned short nb_rb)
\brief This function does the first stage of llr computation for SISO, by just extracting the pilots, PBCH and primary/secondary synchronization sequences.
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param l symbol in sub-frame
@param nb_rb Number of RBs in this allocation
*/

void dlsch_siso(LTE_DL_FRAME_PARMS *frame_parms,
		int **rxdataF_comp,
		unsigned char l,
		unsigned short nb_rb);

/** \fn dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    int **dl_ch_mag,
		    int **dl_ch_magb,
		    unsigned char symbol,
		    unsigned short nb_rb)
\brief This function does Alamouti combining on RX and prepares LLR inputs by skipping pilots, PBCH and primary/secondary synchronization signals.
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param dl_ch_mag First squared-magnitude of channel (16QAM and 64QAM) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param dl_ch_mag Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param symbol Symbol in sub-frame
@param nb_rb Number of RBs in this allocation
*/
void dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    int **dl_ch_mag,
		    int **dl_ch_magb,
		    unsigned char symbol,
		    unsigned short nb_rb);

/** \fn dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    int **dl_ch_mag,
		    int **dl_ch_magb,
		    unsigned char symbol,
		    unsigned short nb_rb)
\brief This function does antenna selection (based on antenna cycling pattern) on RX and prepares LLR inputs by skipping pilots, PBCH and primary/secondary synchronization signals.  Note that this is not LTE, it is just included for comparison purposes.
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param dl_ch_mag First squared-magnitude of channel (16QAM and 64QAM) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param dl_ch_mag Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param symbol Symbol in sub-frame
@param nb_rb Number of RBs in this allocation
*/
void dlsch_antcyc(LTE_DL_FRAME_PARMS *frame_parms,
		  int **rxdataF_comp,
		  int **dl_ch_mag,
		  int **dl_ch_magb,
		  unsigned char symbol,
		  unsigned short nb_rb);

/** \fn dlsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			 int **rxdataF_comp,
			 int **dl_ch_mag,
			 int **dl_ch_magb,
			 unsigned char symbol,
			 unsigned short nb_rb)

\brief This function does maximal-ratio combining for dual-antenna receivers.
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param dl_ch_mag First squared-magnitude of channel (16QAM and 64QAM) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param dl_ch_mag Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param symbol Symbol in sub-frame
@param nb_rb Number of RBs in this allocation
*/
void dlsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			 int **rxdataF_comp,
			 int **dl_ch_mag,
			 int **dl_ch_magb,
			 unsigned char symbol,
			 unsigned short nb_rb);

/** \fn dlsch_extract_rbs_single(int **rxdataF,
                                 int **dl_ch_estimates,
				 int **rxdataF_ext,
				 int **dl_ch_estimates_ext,
				 unsigned int *rb_alloc,
				 unsigned char symbol,
				 LTE_DL_FRAME_PARMS *frame_parms)
\brief This function extracts the received resource blocks, both channel estimates and data symbols,
for the current allocation and for single antenna eNb transmission.
@param rxdataF Raw FFT output of received signal
@param dl_ch_estimates Channel estimates of current slot
@param rxdataF_ext FFT output for RBs in this allocation
@param dl_ch_estimates_ext Channel estimates for RBs in this allocation
@param rb_alloc RB allocation vector
@param symbol Symbol to extract
@param frame_parms Pointer to frame descriptor
*/
unsigned short dlsch_extract_rbs_single(int **rxdataF,
					int **dl_ch_estimates,
					int **rxdataF_ext,
					int **dl_ch_estimates_ext,
					unsigned int *rb_alloc,
					unsigned char symbol,
					LTE_DL_FRAME_PARMS *frame_parms);

/** \fn dlsch_extract_rbs_dual(int **rxdataF,
                               int **dl_ch_estimates,
			       int **rxdataF_ext,
			       int **dl_ch_estimates_ext,
			       unsigned int *rb_alloc,
			       unsigned char symbol,
			       LTE_DL_FRAME_PARMS *frame_parms)
\brief This function extracts the received resource blocks, both channel estimates and data symbols,
for the current allocation and for dual antenna eNb transmission.
@param rxdataF Raw FFT output of received signal
@param dl_ch_estimates Channel estimates of current slot
@param rxdataF_ext FFT output for RBs in this allocation
@param dl_ch_estimates_ext Channel estimates for RBs in this allocation
@param rb_alloc RB allocation vector
@param symbol Symbol to extract
@param frame_parms Pointer to frame descriptor
*/
unsigned short dlsch_extract_rbs_dual(int **rxdataF,
				      int **dl_ch_estimates,
				      int **rxdataF_ext,
				      int **dl_ch_estimates_ext,
				      unsigned int *rb_alloc,
				      unsigned char symbol,
				      LTE_DL_FRAME_PARMS *frame_parms);

/** \fn dlsch_channel_compensation(int **rxdataF_ext,
				int **dl_ch_estimates_ext,
				int **dl_ch_mag,
				int **dl_ch_magb,
				int **rxdataF_comp,
				LTE_DL_FRAME_PARMS *frame_parms,
				unsigned char symbol,
				unsigned char mod_order,
				unsigned short nb_rb,
				unsigned char output_shift)
\brief This function performs channel compensation (matched filtering) on the received RBs for this allocation.  In addition, it computes the squared-magnitude of the channel with weightings for 16QAM/64QAM detection as well as dual-stream detection (cross-correlation)
@param rxdataF_ext Frequency-domain received signal in RBs to be demodulated
@param dl_ch_estimates_ext Frequency-domain channel estimates in RBs to be demodulated
@param dl_ch_mag First Channel magnitudes (16QAM/64QAM)
@param dl_ch_magb Second weighted Channel magnitudes (64QAM)
@param rxdataF_comp Compensated received waveform 
@param frame_parms Pointer to frame descriptor
@param symbol Symbol on which to operate
@param mod_order Modulation order of allocation
@param nb_rb Number of RBs in allocation
@param output_shift Rescaling for compensated output (should be energy-normalizing)
*/
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

/** \fn dlsch_channel_level(int **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 int *avg,
			 unsigned short nb_rb)
\brief This function computes the average channel level over all allocated RBs and antennas (TX/RX) in order to compute output shift for compensated signal
@param dl_ch_estimates_ext Channel estimates in allocated RBs
@param frame_parms Pointer to frame descriptor
@param avg Pointer to average signal strength
@param nb_rb Number of allocated RBs
*/
void dlsch_channel_level(int **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 int *avg,
			 unsigned short nb_rb);

/** \fn rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	      LTE_UE_DLSCH *lte_ue_dlsch_vars,
	      LTE_DL_FRAME_PARMS *frame_parms,
	      unsigned char symbol,
	      unsigned int *rb_alloc,
	      unsigned char mod_order,
	      MIMO_mode_t mimo_mode)
\brief This function is the top-level entry point to dlsch demodulation, after frequency-domain transformation and channel estimation.  It performs
- RB extraction (signal and channel estimates)
- channel compensation (matched filtering)
- RE extraction (pilot, PBCH, synch. signals)
- antenna combining (MRC, Alamouti, cycling)
- LLR computation
@param lte_ue_common_vars Pointer to Common RX variable structure for UE
@param lte_ue_dlsch_vars Pointer to DLSCH variable structure for UE
@param frame_parms Pointer to frame descriptor
@param symbol Symbol on which to act
@param rb_alloc RB allocation vector
@param mod_order Modulation order for this DLSCH
@param mimo_mode MIMO mode for this DLSCH
*/
void rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	      LTE_UE_DLSCH *lte_ue_dlsch_vars,
	      LTE_DL_FRAME_PARMS *frame_parms,
	      unsigned char symbol,
	      unsigned int *rb_alloc,
	      unsigned char mod_order,
	      MIMO_mode_t mimo_mode);

int rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_DLSCH *lte_ue_dlsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     MIMO_mode_t mimo_mode);

#endif
