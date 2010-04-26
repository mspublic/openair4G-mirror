/* file: defs.h
   purpose: data structures and function prototypes for LTE_TRANSPORT procedures (TX/RX)
   author: raymond.knopp@eurecom.fr, florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it
   date: 21.10.2009 
*/

#ifndef __LTE_TRANSPORT_DEFS__H__
#define __LTE_TRANSPORT_DEFS__H__
#include "PHY/defs.h"
#include "dci.h"
#include "uci.h"

#define MOD_TABLE_QPSK_OFFSET 1
#define MOD_TABLE_16QAM_OFFSET 5
#define MOD_TABLE_64QAM_OFFSET 21
#define MOD_TABLE_PSS_OFFSET 85

// Functions and structures below implement 36-212

/** @addtogroup _PHY_TRANSPORT_
 * @{
 */

#define MAX_NUM_PHICH_GROUPS 56  //110 RBs Ng=2, p.60 36-212, Sec. 6.9

#define NSOFT 1827072
#define LTE_NULL 2 

// maximum of 3 segments before each coding block if data length exceeds 6144 bits. 

#define MAX_NUM_DLSCH_SEGMENTS 2
#define MAX_NUM_ULSCH_SEGMENTS 2
#define MAX_DLSCH_PAYLOAD_BYTES (MAX_NUM_DLSCH_SEGMENTS*768)
#define MAX_ULSCH_PAYLOAD_BYTES (MAX_NUM_ULSCH_SEGMENTS*768)

#define MAX_NUM_CHANNEL_BITS (14*1200*12)  // 14 symbols, 1200 REs, 12 bits/RE
#define MAX_NUM_RE (14*1200)

#define SI_RNTI 0xffff 
#define RA_RNTI 0xfffe
#define P_RNTI  0xfffd
#define C_RNTI  0x1234

#define PMI_2A_11 0
#define PMI_2A_1m1 1
#define PMI_2A_1j 2
#define PMI_2A_1mj 3

typedef enum {
  SCH_IDLE,
  ACTIVE,
  DISABLED
} SCH_status_t;


typedef struct {
  /// Flag indicating that this DLSCH is active (i.e. not the first round)
  unsigned char Ndi;
  /// Status Flag indicating for this DLSCH (idle,active,disabled)
  SCH_status_t status;
  /// Transport block size
  unsigned int TBS;
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
  /// MCS format for this DLSCH
  unsigned char mcs;            
  /// Redundancy-version of the current sub-frame
  unsigned char rvidx;
  /// MIMO mode for this DLSCH
  MIMO_mode_t mimo_mode;
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
} LTE_DL_eNb_HARQ_t;

typedef struct {
  /// Flag indicating that this ULSCH has a new packet (start of new round) 
  unsigned char Ndi;
  /// Status Flag indicating for this ULSCH (idle,active,disabled)
  SCH_status_t status;
  /// Subframe scheduling indicator (i.e. Transmission opportunity indicator)
  unsigned char subframe_scheduling_flag;
  /// First Allocated RB 
  unsigned short first_rb;
  /// Current Number of RBs
  unsigned short nb_rb;
  /// Last TPC command
  unsigned char TPC;
  /// Transport block size
  unsigned int TBS;
  /// The payload + CRC size in bits, "B" from 36-212 
  unsigned short B;  
  /// Pointer to the payload
  unsigned char *b;             
  /// Pointers to transport block segments
  unsigned char *c[MAX_NUM_ULSCH_SEGMENTS]; 
  /// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)
  unsigned int RTC[MAX_NUM_ULSCH_SEGMENTS];
  /// Index of current HARQ round for this ULSCH                
  unsigned char round;                
  /// MCS format of this ULSCH
  unsigned char mcs;            
  /// Redundancy-version of the current sub-frame
  unsigned char rvidx;
  /// Turbo-code outputs (36-212 V8.6 2009-03, p.12 
  unsigned char d[MAX_NUM_ULSCH_SEGMENTS][(96+3+(3*6144))];  
  /// Sub-block interleaver outputs (36-212 V8.6 2009-03, p.16-17)
  unsigned char w[MAX_NUM_ULSCH_SEGMENTS][3*6144]; 
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
  /// Msc_initial, Initial number of subcarriers for ULSCH (36-212, v8.6 2009-03, p.26-27)
  unsigned short Msc_initial;
  /// Nsymb_initial, Initial number of symbols for ULSCH (36-212, v8.6 2009-03, p.26-27)
  unsigned char Nsymb_initial;
} LTE_UL_UE_HARQ_t;

typedef struct {
  /// Target MCS
  unsigned char target_mcs; 
  /// Transmission mode
  unsigned char mode;
  /// Current HARQ process id
  unsigned char current_harq_pid;
  /// Process ID's per subframe
  unsigned char harq_ids[10];
  /// Current RB allocation
  unsigned int rb_alloc[4];
  /// Current subband PMI allocation
  unsigned short pmi_alloc;
  /// Current subband RI allocation
  unsigned int ri_alloc;
  /// Current subband CQI1 allocation
  unsigned int cqi_alloc1;
  /// Current subband CQI2 allocation
  unsigned int cqi_alloc2;
  /// Current Number of RBs
  unsigned short nb_rb;
  /// Pointers to 8 HARQ processes for the DLSCH
  LTE_DL_eNb_HARQ_t *harq_processes[8];     
  /// Number of soft channel bits
  unsigned short G;
  /// Layer index for this dlsch (0,1)
  unsigned char layer_index;          
  /// Codebook index for this dlsch (0,1,2,3)
  unsigned char codebook_index;          
  /// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18) 
  unsigned char e[MAX_NUM_CHANNEL_BITS];
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)             
  unsigned char Mdlharq;  
  /// MIMO transmission mode indicator for this sub-frame (for definition see 36-212 V8.6 2009-03, p.17)
  unsigned char Kmimo;
} LTE_eNb_DLSCH_t;

typedef struct {
  /// Current Number of Symbols
  unsigned char Nsymb_pusch;
  /// Pointers to 8 HARQ processes for the ULSCH
  LTE_UL_UE_HARQ_t *harq_processes[8];     
  /// Pointer to CQI data
  unsigned char o[MAX_CQI_BITS];
  /// Length of CQI data (bits)
  unsigned char O;
  /// Rank information 
  unsigned char o_RI[2];
  /// Length of rank information (bits)
  unsigned char O_RI;
  /// Pointer to ACK
  unsigned char o_ACK[4];
  /// Length of ACK information (bits)
  unsigned char O_ACK;
  /// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18) 
  unsigned char e[MAX_NUM_CHANNEL_BITS];
  /// Interleaved "h"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18) 
  unsigned char h[MAX_NUM_CHANNEL_BITS];
  /// Scrambled "b"-sequences (for definition see 36-211 V8.6 2009-03, p.14) 
  unsigned char b_tilde[MAX_NUM_CHANNEL_BITS];
  /// Modulated "d"-sequences (for definition see 36-211 V8.6 2009-03, p.14) 
  mod_sym_t d[MAX_NUM_RE];
  /// Transform-coded "z"-sequences (for definition see 36-211 V8.6 2009-03, p.14-15) 
  mod_sym_t z[MAX_NUM_RE];
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)             
  unsigned char Mdlharq; 
  /// "q" sequences for CQI/PMI (for definition see 36-212 V8.6 2009-03, p.27)
  unsigned char q[MAX_CQI_PAYLOAD];
  /// coded and interleaved CQI bits
  unsigned char o_w[(MAX_CQI_BITS+8)*3];
  /// coded CQI bits
  unsigned char o_d[96+((MAX_CQI_BITS+8)*3)];
  /// coded ACK bits
  unsigned char q_ACK[MAX_ACK_PAYLOAD];
  /// coded RI bits
  unsigned char q_RI[MAX_RI_PAYLOAD];
  /// beta_offset_cqi times 8
  unsigned char beta_offset_cqi_times8;
  /// beta_offset_ri times 8
  unsigned char beta_offset_ri_times8;
  /// beta_offset_harqack times 8
  unsigned char beta_offset_harqack_times8;
  /// power_offset
  unsigned char power_offset;
} LTE_UE_ULSCH_t;

typedef struct {
  /// Flag indicating that this ULSCH has new data
  unsigned char Ndi;
  /// Status Flag indicating for this ULSCH (idle,active,disabled)
  SCH_status_t status;
  /// Subframe scheduling indicator (i.e. Transmission opportunity indicator)
  unsigned char subframe_scheduling_flag;
  /// PHICH active flag
  unsigned char phich_active;
  /// PHICH ACK
  unsigned char phich_ACK;
  /// Last TPC command
  unsigned char TPC;
  /// First Allocated RB 
  unsigned short first_rb;
  /// Current Number of RBs
  unsigned short nb_rb;
  /// Transport block size
  unsigned int TBS;
  /// The payload + CRC size in bits  
  unsigned short B; 
  /// Pointer to the payload
  unsigned char *b;  
  /// Pointers to transport block segments
  unsigned char *c[MAX_NUM_ULSCH_SEGMENTS];
  /// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)  
  unsigned int RTC[8]; 
  /// Index of current HARQ round for this ULSCH
  unsigned char round; 
  /// MCS format for this DLSCH
  unsigned char mcs; 
  /// Redundancy-version of the current sub-frame
  unsigned char rvidx;
  /// soft bits for each received segment ("w"-sequence)(for definition see 36-212 V8.6 2009-03, p.15) 
  short w[MAX_NUM_ULSCH_SEGMENTS][3*(6144+64)];
  /// soft bits for each received segment ("d"-sequence)(for definition see 36-212 V8.6 2009-03, p.15)    
  short *d[MAX_NUM_ULSCH_SEGMENTS];
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
  /// Msc_initial, Initial number of subcarriers for ULSCH (36-212, v8.6 2009-03, p.26-27)
  unsigned short Msc_initial;
  /// Nsymb_initial, Initial number of symbols for ULSCH (36-212, v8.6 2009-03, p.26-27)
  unsigned char Nsymb_initial;
} LTE_UL_eNb_HARQ_t;

typedef struct {
  /// Current Number of Symbols
  unsigned char Nsymb_pusch;
  /// Pointers to 8 HARQ processes for the ULSCH
  LTE_UL_eNb_HARQ_t *harq_processes[8];     
  /// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18) 
  short e[MAX_NUM_CHANNEL_BITS];
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)             
  unsigned char Mdlharq; 
  /// CQI CRC status
  unsigned char cqi_crc_status;
  /// Pointer to CQI data
  unsigned char o[MAX_CQI_BITS+8];
  /// Length of CQI data under RI=1 assumption(bits)
  unsigned char Or1;
  /// Length of CQI data under RI=2 assumption(bits)
  unsigned char Or2;
  /// Rank information 
  unsigned char o_RI[2];
  /// Length of rank information (bits)
  unsigned char O_RI;
  /// Pointer to ACK
  unsigned char o_ACK[4];
  /// Length of ACK information (bits)
  unsigned char O_ACK;
  /// "q" sequences for CQI/PMI (for definition see 36-212 V8.6 2009-03, p.27)
  char q[MAX_CQI_PAYLOAD];
  /// number of coded CQI bits after interleaving
  unsigned char o_RCC;
  /// coded and interleaved CQI bits
  char o_w[(MAX_CQI_BITS+8)*3];
  /// coded CQI bits
  char o_d[96+((MAX_CQI_BITS+8)*3)];
  /// coded ACK bits
  short q_ACK[MAX_ACK_PAYLOAD];
  /// coded RI bits
  short q_RI[MAX_RI_PAYLOAD];
  /// beta_offset_cqi times 8
  unsigned char beta_offset_cqi_times8;
  /// beta_offset_ri times 8
  unsigned char beta_offset_ri_times8;
  /// beta_offset_harqack times 8
  unsigned char beta_offset_harqack_times8;
  /// Flag to indicate that eNB awaits UE RAG 
  unsigned char RAG_active;
  /// Subframe for RAG
  unsigned char RAG_subframe;
  /// Frame for RAG
  unsigned int RAG_frame;
} LTE_eNb_ULSCH_t;

typedef struct {
  /// Flag indicating that this DLSCH has a new transport block
  unsigned char Ndi;
  /// DLSCH status flag indicating 
  SCH_status_t status;
  /// Transport block size
  unsigned int TBS;
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
  /// MCS format for this DLSCH
  unsigned char mcs; 
  /// Redundancy-version of the current sub-frame
  unsigned char rvidx;
  /// MIMO mode for this DLSCH
  MIMO_mode_t mimo_mode;
  /// soft bits for each received segment ("w"-sequence)(for definition see 36-212 V8.6 2009-03, p.15) 
  short w[MAX_NUM_DLSCH_SEGMENTS][3*(6144+64)];
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
} LTE_DL_UE_HARQ_t;

typedef struct {
  int UL_rssi[NUMBER_OF_UE_MAX][NB_ANTENNAS_RX];
  unsigned char DL_cqi[NUMBER_OF_UE_MAX][2];
  unsigned char DL_diffcqi[NUMBER_OF_UE_MAX][2];
  unsigned short DL_pmi_single[NUMBER_OF_UE_MAX];
  unsigned short DL_pmi_dual[NUMBER_OF_UE_MAX];
  unsigned char rank[NUMBER_OF_UE_MAX];
  unsigned short UE_id[NUMBER_OF_UE_MAX]; ///user id (rnti) of connected UEs
  unsigned short UE_timing_offset[NUMBER_OF_UE_MAX]; ///timing offset of connected UEs (for timing advance signalling)
  UE_MODE_t mode[NUMBER_OF_UE_MAX];
} LTE_eNB_UE_stats;

typedef struct {
  /// HARQ process id
  unsigned char harq_id;
  /// ACK bits (after decoding)
  unsigned char ack;
} harq_status_t;

typedef struct {
  /// Transmission mode
  unsigned char mode1_flag;
  /// Current HARQ process id
  unsigned char current_harq_pid;
  /// Current RB allocation
  unsigned int rb_alloc[4];
  /// Current subband PMI allocation
  unsigned short pmi_alloc;
  /// Current subband antenna selection
  unsigned int antenna_alloc;
  /// Current subband RI allocation
  unsigned int ri_alloc;
  /// Current subband CQI1 allocation
  unsigned int cqi_alloc1;
  /// Current subband CQI2 allocation
  unsigned int cqi_alloc2;
  /// Current Number of RBs
  unsigned short nb_rb;
  /// HARQ-ACKs
  harq_status_t harq_ack[10];
/// Pointers to up to 8 HARQ processes
  LTE_DL_UE_HARQ_t *harq_processes[8];   
/// Layer index for this DLSCH
  unsigned char layer_index;              
/// Number of soft channel bits
  unsigned short G;
/// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17
  unsigned char Mdlharq;              
/// MIMO transmission mode indicator for this sub-frame (for definition see 36-212 V8.6 2009-03, p.17)
  unsigned char Kmimo;                
} LTE_UE_DLSCH_t;

typedef enum {format0,
	   format1,
	   format1A,
	   format1B,
	   format1C,
	   format1D,
	   format2,
	   format2_2A_L10PRB,
	   format2_2A_M10PRB,
	   format2_4A_L10PRB,
	   format2_4A_M10PRB,
	   format2A_2A_L10PRB,
	   format2A_2A_M10PRB,
	   format2A_4A_L10PRB,
	   format2A_4A_M10PRB,
	   format3
} DCI_format_t;
typedef struct {
  /// Length of DCI in bits
  unsigned char dci_length;
  /// Aggregation level 
  unsigned char L;
  /// rnti
  unsigned short rnti;
  /// Format
  DCI_format_t format;
  /// DCI pdu
  unsigned char dci_pdu[1+(MAX_DCI_SIZE_BITS/8)];
} DCI_ALLOC_t;


void free_eNb_dlsch(LTE_eNb_DLSCH_t *dlsch);

LTE_eNb_DLSCH_t *new_eNb_dlsch(unsigned char Kmimo,unsigned char Mdlharq);

void free_ue_dlsch(LTE_UE_DLSCH_t *dlsch);

LTE_UE_DLSCH_t *new_ue_dlsch(unsigned char Kmimo,unsigned char Mdlharq);

void free_eNb_dlsch(LTE_eNb_DLSCH_t *dlsch);

LTE_eNb_ULSCH_t *new_eNb_ulsch(unsigned char Mdlharq);

void free_ue_ulsch(LTE_UE_ULSCH_t *ulsch);

LTE_UE_ULSCH_t *new_ue_ulsch(unsigned char Mdlharq);



/** \fn dlsch_encoding(unsigned char *input_buffer,
    LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_eNb_DLSCH_t *dlsch)
\brief This function performs a subset of the bit-coding functions for LTE as described in 36-212, Release 8.Support is limited to turbo-coded channels (DLSCH/ULSCH). The implemented functions are:
- CRC computation and addition
- Code block segmentation and sub-block CRC addition
- Channel coding (Turbo coding)
- Rate matching (sub-block interleaving, bit collection, selection and transmission
- Code block concatenation
@param input_buffer Pointer to input buffer for sub-frame
@param frame_parms Pointer to frame descriptor structure
@param dlsch Pointer to dlsch to be encoded
@returns status
*/
int dlsch_encoding(unsigned char *a,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   LTE_eNb_DLSCH_t *dlsch);




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
			unsigned char precoder_index,
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
\param precoder_index 36-211 W precoder column (1 layer) or matrix (2 layer) selection index
\param amp Amplitude for symbols
\param re_allocated pointer to allocation counter
\param skip_dc offset for positive RBs
\param frame_parms Frame parameter descriptor
*/

int allocate_REs_in_RB(mod_sym_t **txdataF,
		       unsigned int *jj,
		       unsigned short re_offset,
		       unsigned int symbol_offset,
		       unsigned char *output,
		       MIMO_mode_t mimo_mode,
		       unsigned char nu,
		       unsigned char pilots,
		       unsigned char first_pilot,
		       unsigned char mod_order,
		       unsigned char precoder_index,
		       short amp,
		       unsigned int *re_allocated,
		       unsigned char skip_dc,
		       LTE_DL_FRAME_PARMS *frame_parms);

/** \fn int dlsch_modulation(mod_sym_t **txdataF,
		      short amp,
		      unsigned int sub_frame_offset,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_eNb_DLSCH_t *dlsch);

\brief This function is the top-level routine for generation of the sub-frame signal (frequency-domain) for DLSCH.  
@param txdataF Table of pointers for frequency-domain TX signals
@param amp Amplitude of signal
@param sub_frame_offset Offset of this subframe in units of subframes (usually 0)
@param frame_parms Pointer to frame descriptor
@param dlsch Pointer to DLSCH descriptor for this allocation

*/ 
int dlsch_modulation(mod_sym_t **txdataF,
		     short amp,
		     unsigned int sub_frame_offset,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     LTE_eNb_DLSCH_t *dlsch);


/** \fn generate_pilots(mod_sym_t **txdataF,
                        short amp,
		        LTE_DL_FRAME_PARMS *frame_parms,
		        unsigned char eNb_id,
			unsigned short N);
\brief This function generates the frequency-domain pilots (cell-specific downlink reference signals)
for N subframes.
@param txdataF Table of pointers for frequency-domain TX signals
@param amp Amplitude of signal
@param frame_parms Pointer to frame descriptor
@param eNb_id Nid2 (0,1,2)
@param N Number of sub-frames to generate
*/
void generate_pilots(mod_sym_t **txdataF,
		     short amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     unsigned char eNb_id,
		     unsigned short N);

/**
\brief This function generates the frequency-domain pilots (cell-specific downlink reference signals) for one slot only
@param txdataF Table of pointers for frequency-domain TX signals
@param amp Amplitude of signal
@param frame_parms Pointer to frame descriptor
@param eNb_id Nid2 (0,1,2)
@param slot index (0..19)
*/
int generate_pilots_slot(mod_sym_t **txdataF,
			 short amp,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 unsigned char eNb_id,
			 unsigned short slot);


int generate_pss(mod_sym_t **txdataF,
		 short amp,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 unsigned short eNb_id,
		 unsigned short l,
		 unsigned short Ns);

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
    int **rxdataF_comp_i,
    int **rho_i,
    short *dlsch_llr,
    unsigned char symbol,
    unsigned short nb_rb)

\brief This function perform LLR computation for dual-stream (QPSK/QPSK) transmission.
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param rxdataF_comp Compensated channel output for interference
@param rho_i Correlation between channel of signal and inteference
@param dlsch_llr llr output
@param symbol OFDM symbol index in sub-frame
@param nb_rb number of RBs for this allocation
*/

int dlsch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
			int **rxdataF_comp,
			int **rxdataF_comp_i,
			int **rho_i,
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
int dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
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
		int **rxdataF_comp_i,
		unsigned char l,
		unsigned short nb_rb)
\brief This function does the first stage of llr computation for SISO, by just extracting the pilots, PBCH and primary/secondary synchronization sequences.
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param rxdataF_comp Compensated channel output for interference
@param l symbol in sub-frame
@param nb_rb Number of RBs in this allocation
*/

void dlsch_siso(LTE_DL_FRAME_PARMS *frame_parms,
		int **rxdataF_comp,
		int **rxdataF_comp_i,
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
			 int **rxdataF_comp_i,
			 int **rho,
			 int **rho_i,
			 int **dl_ch_mag,
			 int **dl_ch_magb,
			 unsigned char symbol,
			 unsigned short nb_rb,
			 unsigned char dual_stream_UE)

\brief This function does maximal-ratio combining for dual-antenna receivers.
@param frame_parms Frame descriptor structure
@param rxdataF_comp Compensated channel output
@param rxdataF_comp_i Compensated channel output for interference
@param rho Cross correlation between spatial channels
@param rho Cross correlation between signal and inteference channels
@param dl_ch_mag First squared-magnitude of channel (16QAM and 64QAM) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param dl_ch_magb Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
@param symbol Symbol in sub-frame
@param nb_rb Number of RBs in this allocation
@param dual_stream_UE Flag to indicate dual-stream detection
*/
void dlsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			 int **rxdataF_comp,
			 int **rxdataF_comp_i,
			 int **rho,
			 int **rho_i,
			 int **dl_ch_mag,
			 int **dl_ch_magb,
			 unsigned char symbol,
			 unsigned short nb_rb,
			 unsigned char dual_stream_UE);

/** \fn dlsch_extract_rbs_single(int **rxdataF,
                                 int **dl_ch_estimates,
				 int **rxdataF_ext,
				 int **dl_ch_estimates_ext,
				 unsigned short pmi,
				 unsigned char *pmi_ext,
				 unsigned int *rb_alloc,
				 unsigned char symbol,
				 LTE_DL_FRAME_PARMS *frame_parms)
\brief This function extracts the received resource blocks, both channel estimates and data symbols,
for the current allocation and for single antenna eNb transmission.
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
unsigned short dlsch_extract_rbs_single(int **rxdataF,
					int **dl_ch_estimates,
					int **rxdataF_ext,
					int **dl_ch_estimates_ext,
					unsigned short pmi,
					unsigned char *pmi_ext,
					unsigned int *rb_alloc,
					unsigned char symbol,
					LTE_DL_FRAME_PARMS *frame_parms);

/** \fn dlsch_extract_rbs_dual(int **rxdataF,
                               int **dl_ch_estimates,
			       int **rxdataF_ext,
			       int **dl_ch_estimates_ext,
			       unsigned short pmi,
			       unsigned char *pmi_ext,
			       unsigned int *rb_alloc,
			       unsigned char symbol,
			       LTE_DL_FRAME_PARMS *frame_parms)
\brief This function extracts the received resource blocks, both channel estimates and data symbols,
for the current allocation and for dual antenna eNb transmission.
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
unsigned short dlsch_extract_rbs_dual(int **rxdataF,
				      int **dl_ch_estimates,
				      int **rxdataF_ext,
				      int **dl_ch_estimates_ext,
				      unsigned short pmi,
				      unsigned char *pmi_ext,
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
				unsigned char mod_order,
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

/** \fn unsigned int void  dlsch_decoding(unsigned short A,
		     short *dlsch_llr,
		     LTE_DL_FRAME_PARMS *lte_frame_parms,
		     LTE_UE_DLSCH_t *dlsch,
		     unsigned char subframe)

\brief This is the top-level entry point for DLSCH decoding in UE.  It should be replicated on several
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
@returns 0 on success, 1 on unsuccessful decoding
*/
unsigned int dlsch_decoding(short *dlsch_llr,
			    LTE_DL_FRAME_PARMS *lte_frame_parms,
			    LTE_UE_DLSCH_t *dlsch,
			    unsigned char subframe);


/** \fn rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	      LTE_UE_DLSCH **lte_ue_dlsch_vars,
	      LTE_DL_FRAME_PARMS *frame_parms,
	      unsigned char eNb_id,
	      unsigned char eNb_id_i,
	      LTE_UE_DLSCH_t **dlsch_ue,
	      unsigned char symbol,
	      unsigned char dual_stream_UE)
\brief This function is the top-level entry point to dlsch demodulation, after frequency-domain transformation and channel estimation.  It performs
- RB extraction (signal and channel estimates)
- channel compensation (matched filtering)
- RE extraction (pilot, PBCH, synch. signals)
- antenna combining (MRC, Alamouti, cycling)
- LLR computation
@param lte_ue_common_vars Pointer to Common RX variable structure for UE
@param lte_ue_dlsch_vars Pointer to DLSCH variable structure for UE
@param frame_parms Pointer to frame descriptor
@param eNb_id eNb index (Nid1) 0,1,2
@param eNb_id_i Interfering eNb index (Nid1) 0,1,2
@param dlsch_ue 
@param symbol Symbol on which to act (within sub-frame)
@param dual_stream_UE Flag to indicate dual-stream interference cancellation
*/
int rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_DLSCH **lte_ue_dlsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     unsigned char eNb_id,
	     unsigned char eNb_id_i,
	     LTE_UE_DLSCH_t **dlsch_ue,
	     unsigned char symbol,
	     unsigned char dual_stream_UE);

int rx_pdcch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_PDCCH **lte_ue_pdcch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     unsigned char eNb_id,
	     unsigned char n_pdcch_symbols,
	     MIMO_mode_t mimo_mode);


int rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
	    LTE_UE_PBCH *lte_ue_pbch_vars,
	    LTE_DL_FRAME_PARMS *frame_parms,
	    unsigned char eNb_id,
	    MIMO_mode_t mimo_mode);

/*! \brief PBCH unscrambling
This is similar to pbch_scrabling with the difference that inputs are signed shorts (llr values) and instead of flipping bits we change signs.
\param frame_parms Pointer to frame descriptor
\param coded_data Output of the coding and rate matching
\param length Length of the sequence
*/ 
void pbch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		     unsigned char* coded_data,
		     unsigned int length);

/*! \brief PBCH unscrambling
This is similar to pbch_scrabling with the difference that inputs are signed shorts (llr values) and instead of flipping bits we change signs.
\param frame_parms Pointer to frame descriptor
\param llr Output of the demodulator
\param length Length of the sequence
*/ 
void pbch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
		       short* llr,
		       unsigned int length);

/*! \brief DCI Encoding
This routine codes an arbitrary DCI PDU after appending the 8-bit 3GPP CRC.  It then applied sub-block interleaving and rate matching.
\param a Pointer to DCI PDU (coded in bytes)
\param A Length of DCI PDU in bits
\param E Length of DCI PDU in coded bits
\param e Pointer to sequence
\param rnti RNTI for CRC scrambling
*/ 
void dci_encoding(unsigned char *a,
		  unsigned char A,
		  unsigned short E,
		  unsigned char *e,
		  unsigned short rnti);

/*! \brief Top-level DCI entry point.
This routine codes an set of DCI PDUs and performs PDCCH modulation, interleaving and mapping.
\param num_ue_spec_dci  Number of UE specific DCI pdus to encode
\param num_common_dci Number of Common DCI pdus to encode
\param dci_alloc Allocation vectors for each DCI pdu
\param n_rnti n_RNTI (see )
\param amp Amplitude of QPSK symbols
\param frame_parms Pointer to DL Frame parameter structure
\param txdataF Pointer to tx signal buffers
\param sub_frame_offset subframe offset in frame

*/ 
void generate_dci_top(unsigned char num_ue_spec_dci,
		      unsigned char num_common_dci,
		      DCI_ALLOC_t *dci_alloc, 
		      unsigned int n_rnti,
		      short amp,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      mod_sym_t **txdataF,
		      unsigned int sub_frame_offset);


void generate_64qam_table(void);
void generate_16qam_table(void);

unsigned short extract_crc(unsigned char *dci,unsigned char DCI_LENGTH);

void dci_decoding(unsigned char DCI_LENGTH,
		  unsigned char DCI_FMT,
		  char *e,
		  unsigned char *decoded_output);

unsigned short dci_decoding_procedure(LTE_UE_PDCCH **lte_ue_pdcch_vars,
				      DCI_ALLOC_t *dci_alloc,
				      short eNb_id,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      unsigned short si_rnti,
				      unsigned short ra_rnti);

unsigned char get_Qm(unsigned char I_MCS);

unsigned char get_I_TBS(unsigned char I_MCS);



/*!
\brief This function generate the sounding reference symbol (SRS) for the uplink according to 36.211 v8.6.0. If IFFT_FPGA is defined, the SRS is quantized to a QPSK sequence.
@param frame_parms LTE DL Frame Parameters
@param txdataF pointer to the frequency domain TX signal
@param amp amplitudte of the transmit signal (irrelevant for #ifdef IFFT_FPGA)
@sub_frame_offset  Offset of this subframe in units of subframes
*/


int generate_srs_tx(LTE_DL_FRAME_PARMS *frame_parms,
		    mod_sym_t *txdataF,
		    short amp,
		    unsigned int sub_frame_number);

/*!
\brief This function is similar to generate_srs_tx but generates a conjugate sequence for channel estimation. If IFFT_FPGA is defined, the SRS is quantized to a QPSK sequence.
@param frame_parms LTE DL Frame Parameters
@param txdataF pointer to the frequency domain TX signal
@param amp amplitudte of the transmit signal (irrelevant for #ifdef IFFT_FPGA)
@sub_frame_offset  Offset of this subframe in units of subframes
*/

int generate_srs_rx(LTE_DL_FRAME_PARMS *frame_parms,
		    int *txdataF);

/*!
\brief This function generates the downlink reference signal for the PUSCH according to 36.211 v8.6.0. The DRS occuies the RS defined by rb_alloc and the symbols 2 and 8 for extended CP and 3 and 10 for normal CP.
*/

int generate_drs_puch(LTE_DL_FRAME_PARMS *frame_parms,
		      mod_sym_t *txdataF,
		      short amp,
		      unsigned int sub_frame_number,
		      unsigned int first_rb,
		      unsigned int nb_rb);

int compareints (const void * a, const void * b);


void ulsch_modulation(mod_sym_t **txdataF,
		      short amp,
		      unsigned int subframe,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_UE_ULSCH_t *ulsch,
		      unsigned char rag_flag);


void ulsch_extract_rbs_single(int **rxdataF,
			      int **rxdataF_ext,
			      unsigned int first_rb,
			      unsigned int nb_rb,
			      unsigned char l,
			      unsigned char Ns,
			      LTE_DL_FRAME_PARMS *frame_parms);

unsigned char subframe2harq_pid_tdd(unsigned char tdd_config,unsigned char subframe);
unsigned char subframe2harq_pid_tdd_eNBrx(unsigned char tdd_config,unsigned char subframe);

int generate_ue_dlsch_params_from_dci(unsigned char subframe,
				       void *dci_pdu,
				       unsigned short rnti,
				       DCI_format_t dci_format,
				       LTE_UE_DLSCH_t **dlsch_ue,
				       LTE_DL_FRAME_PARMS *frame_parms,
				       unsigned short si_rnti,
				       unsigned short ra_rnti,
				       unsigned short p_rnti);

int generate_eNb_dlsch_params_from_dci(unsigned char subframe,
					void *dci_pdu,
					unsigned short rnti,
					DCI_format_t dci_format,
					LTE_eNb_DLSCH_t **dlsch_eNb,
					LTE_DL_FRAME_PARMS *frame_parms,
					unsigned short si_rnti,
					unsigned short ra_rnti,
					unsigned short p_rnti);

int generate_eNb_ulsch_params_from_rar(unsigned char *rar_pdu,
				       unsigned char subframe,
				       LTE_eNb_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms);

int generate_ue_ulsch_params_from_dci(void *dci_pdu,
				      unsigned short rnti,
				      unsigned char subframe,
				      DCI_format_t dci_format,
				      LTE_UE_ULSCH_t *ulsch,
				      LTE_UE_DLSCH_t **dlsch,
				      PHY_MEASUREMENTS *meas,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      unsigned short si_rnti,
				      unsigned short ra_rnti,
				      unsigned short p_rnti,
				      unsigned char eNb_id); 

int generate_ue_ulsch_params_from_rar(unsigned char *rar_pdu,
				      unsigned char subframe,
				      LTE_UE_ULSCH_t *ulsch,
				      PHY_MEASUREMENTS *meas,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      unsigned char eNb_id);

int generate_eNb_ulsch_params_from_dci(void *dci_pdu,
					unsigned short rnti,
					unsigned char subframe,
					DCI_format_t dci_format,
					LTE_eNb_ULSCH_t *ulsch,
					LTE_DL_FRAME_PARMS *frame_parms,
					unsigned short si_rnti,
					unsigned short ra_rnti,
					unsigned short p_rnti);


void generate_pcfich_reg_mapping(LTE_DL_FRAME_PARMS *frame_parms);

void generate_phich_reg_mapping_ext(LTE_DL_FRAME_PARMS *frame_parms);

void init_transport_channels(unsigned char);

void generate_RIV_tables(void);


int *rx_ulsch(LTE_eNB_COMMON *eNB_common_vars,
	     LTE_eNB_ULSCH *eNB_ulsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     unsigned int subframe,
	     unsigned char eNb_id,  // this is the effective sector id
	     unsigned char UE_id,   // this is the UE instance to act upon
	     LTE_eNb_ULSCH_t **ulsch,
	     unsigned char rag_flag);

int ulsch_encoding(unsigned char *a,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   LTE_UE_ULSCH_t *ulsch,
		   unsigned char harq_pid);

unsigned int  ulsch_decoding(short *ulsch_llr,
			     LTE_DL_FRAME_PARMS *frame_parms,
			     LTE_eNb_ULSCH_t *ulsch,
			     unsigned char subframe,
			     unsigned char rag_flag);

void generate_phich_top(LTE_DL_FRAME_PARMS *frame_parms,unsigned char subframe,unsigned char mode1_flag);

void print_CQI(void *o,unsigned char *o_RI,UCI_format fmt,unsigned char eNB_id);

void extract_CQI(void *o,unsigned char *o_RI,UCI_format fmt,unsigned char UE_id,LTE_eNB_UE_stats *stats);

void fill_CQI(void *o,UCI_format fmt,PHY_MEASUREMENTS *meas,unsigned char eNb_id);

unsigned short quantize_subband_pmi(PHY_MEASUREMENTS *meas,unsigned char eNb_id);

unsigned int pmi2hex_2Ar1(unsigned short pmi);

unsigned int pmi2hex_2Ar2(unsigned char pmi);

unsigned int cqi2hex(unsigned short cqi);

unsigned short computeRIV(unsigned short N_RB_DL,unsigned short RBstart,unsigned short Lcrbs);

unsigned int pmi_extend(LTE_DL_FRAME_PARMS *frame_parms,unsigned char wideband_pmi);

/**@}*/
#endif
