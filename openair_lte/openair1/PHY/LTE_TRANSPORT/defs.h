/* file: defs.h
   purpose: data structures and function prototypes for LTE_TRANSPORT procedures (TX/RX)
   author: raymond.knopp@eurecom.fr, florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it
   date: 21.10.2009 
*/

#ifndef __LTE_TRANSPORT_DEFS__H__
#define __LTE_TRANSPORT_DEFS__H__
#include "PHY/defs.h"


// Functions and structures below implement 36-212

/** @addtogroup _PHY_TRANSPORT_
 * @{
 */
#define NSOFT 1827072
#define LTE_NULL 2 
#define MAX_NUM_DLSCH_SEGMENTS 3
#define MAX_DLSCH_PAYLOAD_BYTES 16384
typedef struct {
/// Flag indicating that this DLSCH is active (i.e. not the first round)
  unsigned char active;
/// The payload + CRC size in bits, "B" from 36-212 
  unsigned short B;  
/// Pointer to the payload
  unsigned char *b;             
/// Pointers to transport block segments
  unsigned char *c[MAX_NUM_DLSCH_SEGMENTS]; 
/// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)
  unsigned int RTC[MAX_NUM_DLSCH_SEGMENTS];
/// Index of current HARQ round for this DLSCH                
  unsigned char round;                
/// Modulation order of this DLSCH
  unsigned char mod_order;            
/// MIMO mode for this DLSCH
  MIMO_mode_t mimo_mode;
/// Layer index for this DLSCH
  unsigned char layer_index;              
/// Turbo-code outputs (36-212 V8.6 2009-03, p.12 
  unsigned char d[MAX_NUM_DLSCH_SEGMENTS][(96+3+(3*6144))];  
/// Sub-block interleaver outputs (36-212 V8.6 2009-03, p.16-17)
  unsigned char w[MAX_NUM_DLSCH_SEGMENTS][3*6144]; 
/// Number of code segments (for definition see 36-212 V8.6 2009-03, p.9)            
  unsigned int C;                         
/// Number of "small" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  unsigned int Cminus;                    
/// Number of "large" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  unsigned int Cplus;                     
/// Number of bits in "small" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10) 
  unsigned int Kminus;                    
/// Number of bits in "large" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  unsigned int Kplus;                     
/// Number of "Filler" bits (for definition see 36-212 V8.6 2009-03, p.10)
  unsigned int F;                         
/// Number of MIMO layers (streams) (for definition see 36-212 V8.6 2009-03, p.17)
  unsigned char Nl;                       
} LTE_eNb_HARQ_t;

typedef struct {
/// Pointers to 8 HARQ processes for the DLSCH
  LTE_eNb_HARQ_t *harq_processes[8];     
/// Redundancy-version of the current sub-frame
  unsigned char rvidx;
/// Layer index for this dlsch (0,1)
  unsigned char layer_index;          
/// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18) 
  unsigned char e[3*6144*MAX_NUM_DLSCH_SEGMENTS];
/// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)             
  unsigned char Mdlharq;  
/// MIMO transmission mode indicator for this sub-frame (for definition see 36-212 V8.6 2009-03, p.17)
  unsigned char Kmimo;    
} LTE_eNb_DLSCH_t;

typedef struct {
/// Flag indicating that this DLSCH is active (i.e. not the first round)
  unsigned char active;
/// The payload + CRC size in bits  
  unsigned short B; 
/// Pointer to the payload
  unsigned char *b;  
/// Pointers to transport block segments
  unsigned char *c[MAX_NUM_DLSCH_SEGMENTS];
/// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)  
  unsigned int RTC[8]; 
/// Index of current HARQ round for this DLSCH
  unsigned char round; 
/// Modulation order for this DLSCH
  unsigned char mod_order; 
/// MIMO mode for this DLSCH
  MIMO_mode_t mimo_mode;
/// Layer index for this DLSCH
  unsigned char layer_index;              
/// soft bits for each received segment ("w"-sequence)(for definition see 36-212 V8.6 2009-03, p.15) 
  short w[MAX_NUM_DLSCH_SEGMENTS][3*6144];
/// soft bits for each received segment ("d"-sequence)(for definition see 36-212 V8.6 2009-03, p.15)    
  short *d[MAX_NUM_DLSCH_SEGMENTS];
/// Number of code segments (for definition see 36-212 V8.6 2009-03, p.9)   
  unsigned int C;  
/// Number of "small" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  unsigned int Cminus; 
/// Number of "large" code segments (for definition see 36-212 V8.6 2009-03, p.10) 
  unsigned int Cplus;  
/// Number of bits in "small" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10) 
  unsigned int Kminus; 
/// Number of bits in "large" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10) 
  unsigned int Kplus;
/// Number of "Filler" bits (for definition see 36-212 V8.6 2009-03, p.10)  
  unsigned int F;  
/// Number of MIMO layers (streams) (for definition see 36-212 V8.6 2009-03, p.17)
  unsigned char Nl;  
} LTE_UE_HARQ_t;

typedef struct {
/// Pointers to up to 8 HARQ processes
  LTE_UE_HARQ_t *harq_processes[8];   
/// redundancy version for this sub-frame
  unsigned char rvidx;                
/// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17
  unsigned char Mdlharq;              
/// MIMO transmission mode indicator for this sub-frame (for definition see 36-212 V8.6 2009-03, p.17)
  unsigned char Kmimo;                
} LTE_UE_DLSCH_t;


void free_eNb_dlsch(LTE_eNb_DLSCH_t *dlsch);

LTE_eNb_DLSCH_t *new_eNb_dlsch(unsigned char Kmimo,unsigned char Mdlharq);

void free_ue_dlsch(LTE_UE_DLSCH_t *dlsch);

LTE_UE_DLSCH_t *new_ue_dlsch(unsigned char Kmimo,unsigned char Mdlharq);



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



// Functions below implement 36-211

/** \fn allocate_REs_in_RB(mod_sym_t **txdataF,
			unsigned int *jj,
			unsigned short re_offset,
			unsigned int symbol_offset,
			unsigned char *output,
			MIMO_mode_t mimo_mode,
			unsigned char nu,
			unsigned char pilots,
			unsigned char first_pilot,
			unsigned char mod_order,
			short amp,
			unsigned int *re_allocated,
			unsigned char skip_dc,
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
\param first_pilot =1 if symbol offset it the first OFDM symbol in a slot, 0 otherwise
\param mod_order 2=QPSK, 4=16QAM, 6=64QAM
\param amp Amplitude for symbols
\param re_allocated pointer to allocation counter
\param skip_dc offset for positive RBs
\param frame_parms Frame parameter descriptor
*/

void allocate_REs_in_RB(mod_sym_t **txdataF,
			unsigned int *jj,
			unsigned short re_offset,
			unsigned int symbol_offset,
			unsigned char *output,
			MIMO_mode_t mimo_mode,
			unsigned char nu,
			unsigned char pilots,
			unsigned char first_pilot,
			unsigned char mod_order,
			short amp,
			unsigned int *re_allocated,
			unsigned char skip_dc,
			LTE_DL_FRAME_PARMS *frame_parms);

/** \fn void dlsch_modulation(mod_sym_t **txdataF,
		      short amp,
		      unsigned short sub_frame_offset,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_eNb_DLSCH_t *dlsch,
		      unsigned char harq_pid,
		      unsigned int  *rb_alloc)
\brief This function is the top-level routine for generation of the sub-frame signal (frequency-domain) for DLSCH.  
@param txdataF Table of pointers for frequency-domain TX signals
@param amp Amplitude of signal
@param sub_frame_offset Offset of this subframe in units of subframes (usually 0)
@param frame_parms Pointer to frame descriptor
@param dlsch Pointer to DLSCH descriptor for this allocation
@param harq_pid Identifier of harq process
@param rb_alloc RB allocation vector
*/ 
void dlsch_modulation(mod_sym_t **txdataF,
		      short amp,
		      unsigned short sub_frame_offset,
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


/** \fn qpsk_qpsk(short *stream0_in,
	       short *stream1_in,
	       short *stream0_out,
	       short *rho01,
	       int length
	       ) 

\brief This function computes the LLRs for ML (max-logsum approximation) dual-stream QPSK/QPSK reception.
@param stream0_in Input from channel compensated (MR combined) stream 0
@param stream1_in Input from channel compensated (MR combined) stream 1
@param stream0_out Output from LLR unit for stream0
@param stream0_out Output from LLR unit for stream1
@param rho01 Cross-correlation between channels (MR combined)
@param length in complex channel outputs
     */
void qpsk_qpsk(short *stream0_in,
	       short *stream1_in,
	       short *stream0_out,
	       short *rho01,
	       int length
	       );

/** \fn dlsch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
    int **rxdataF_comp,
    short *dlsch_llr,
    unsigned char symbol,
    unsigned short nb_rb)

\brief This function perform LLR computation for dual-stream (QPSK/QPSK) transmission.
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param dlsch_llr llr output
@param symbol OFDM symbol index in sub-frame
@param nb_rb number of RBs for this allocation
*/

void dlsch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
			 int **rxdataF_comp,
			 short *dlsch_llr,
			 unsigned char symbol,
			 unsigned short nb_rb);

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

/** \fn void dlsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
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
		     short *dlsch_llr,
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
@param dl_ch_magb Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param symbol Symbol in sub-frame
@param nb_rb Number of RBs in this allocation
*/
void dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    int **dl_ch_mag,
		    int **dl_ch_magb,
		    unsigned char symbol,
		    unsigned short nb_rb);

/** \fn dlsch_antcyc(LTE_DL_FRAME_PARMS *frame_parms,
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
			 int **rho,
			 int **dl_ch_mag,
			 int **dl_ch_magb,
			 unsigned char symbol,
			 unsigned short nb_rb)

\brief This function does maximal-ratio combining for dual-antenna receivers.
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param rho Cross correlation between spatial channels
@param dl_ch_mag First squared-magnitude of channel (16QAM and 64QAM) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param dl_ch_magb Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param symbol Symbol in sub-frame
@param nb_rb Number of RBs in this allocation
*/
void dlsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			 int **rxdataF_comp,
			 int **rho,
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
				int **rho,
				LTE_DL_FRAME_PARMS *frame_parms,
				unsigned char symbol,
				unsigned char *mod_order,
				unsigned short nb_rb,
				unsigned char output_shift)
\brief This function performs channel compensation (matched filtering) on the received RBs for this allocation.  In addition, it computes the squared-magnitude of the channel with weightings for 16QAM/64QAM detection as well as dual-stream detection (cross-correlation)
@param rxdataF_ext Frequency-domain received signal in RBs to be demodulated
@param dl_ch_estimates_ext Frequency-domain channel estimates in RBs to be demodulated
@param dl_ch_mag First Channel magnitudes (16QAM/64QAM)
@param dl_ch_magb Second weighted Channel magnitudes (64QAM)
@param rxdataF_comp Compensated received waveform 
@param rho Cross-correlation between two spatial channels on each RX antenna
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
				int **rho,
				LTE_DL_FRAME_PARMS *frame_parms,
				unsigned char symbol,
				unsigned char *mod_order,
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

/** \fn unsigned int void  dlsch_decoding(unsigned short A,
		     short *dlsch_llr,
		     LTE_DL_FRAME_PARMS *lte_frame_parms,
		     LTE_UE_DLSCH_t *dlsch,
		     unsigned char harq_pid,
		     unsigned char nb_rb)

\brief This is the top-level entry point for DLSCH decoding in UE.  It should be replicated on several
threads (on multi-core machines) corresponding to different HARQ processes. The routine first 
computes the segmentation information, followed by rate dematching and sub-block deinterleaving the of the
received LLRs computed by dlsch_demodulation for each transport block segment. It then calls the
turbo-decoding algorithm for each segment and stops after either after unsuccesful decoding of at least
one segment or correct decoding of all segments.  Only the segment CRCs are check for the moment, the
overall CRC is ignored.  Finally transport block reassembly is performed.
@param A Transport block length in bits (i.e. length of "a"-sequence from 36-212)
@param dlsch_llr Pointer to LLR values computed by dlsch_demodulation
@param lte_frame_parms Pointer to frame descriptor
@param dlsch Pointer to DLSCH descriptor
@param harq_pid Process ID of DLSCH
@param nb_rb Number of RBs
@returns 0 on success, 1 on unsuccessful decoding
*/
unsigned int dlsch_decoding(unsigned short A,
			    short *dlsch_llr,
			    LTE_DL_FRAME_PARMS *lte_frame_parms,
			    LTE_UE_DLSCH_t *dlsch,
			    unsigned char harq_pid,
			    unsigned char nb_rb);

/** \fn rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	      LTE_UE_DLSCH *lte_ue_dlsch_vars,
	      LTE_DL_FRAME_PARMS *frame_parms,
	      unsigned char symbol,
	      unsigned int *rb_alloc,
	      unsigned char *Qm,
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
@param Qm Modulation orders on layers 0,1 for this DLSCH.  These must be set even for single-stream
@param mimo_mode MIMO mode for this DLSCH
*/
void rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	      LTE_UE_DLSCH *lte_ue_dlsch_vars,
	      LTE_DL_FRAME_PARMS *frame_parms,
	      unsigned char symbol,
	      unsigned int *rb_alloc,
	      unsigned char *Qm,
	      MIMO_mode_t mimo_mode);

int rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_DLSCH *lte_ue_dlsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     MIMO_mode_t mimo_mode);

/**@}*/
#endif
