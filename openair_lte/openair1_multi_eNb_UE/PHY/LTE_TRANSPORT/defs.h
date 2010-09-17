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
  u8 Ndi;
  /// Status Flag indicating for this DLSCH (idle,active,disabled)
  SCH_status_t status;
  /// Transport block size
  u32 TBS;
  /// The payload + CRC size in bits, "B" from 36-212 
  u16 B;  
  /// Pointer to the payload
  u8 *b;             
  /// Pointers to transport block segments
  u8 *c[MAX_NUM_DLSCH_SEGMENTS]; 
  /// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)
  u32 RTC[MAX_NUM_DLSCH_SEGMENTS];
  /// Index of current HARQ round for this DLSCH                
  u8 round;                
  /// MCS format for this DLSCH
  u8 mcs;            
  /// Redundancy-version of the current sub-frame
  u8 rvidx;
  /// MIMO mode for this DLSCH
  MIMO_mode_t mimo_mode;
  /// Turbo-code outputs (36-212 V8.6 2009-03, p.12 
  u8 d[MAX_NUM_DLSCH_SEGMENTS][(96+3+(3*6144))];  
  /// Sub-block interleaver outputs (36-212 V8.6 2009-03, p.16-17)
  u8 w[MAX_NUM_DLSCH_SEGMENTS][3*6144]; 
  /// Number of code segments (for definition see 36-212 V8.6 2009-03, p.9)            
  u32 C;                         
  /// Number of "small" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  u32 Cminus;                    
  /// Number of "large" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  u32 Cplus;                     
  /// Number of bits in "small" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10) 
  u32 Kminus;                    
  /// Number of bits in "large" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  u32 Kplus;                     
  /// Number of "Filler" bits (for definition see 36-212 V8.6 2009-03, p.10)
  u32 F;                         
  /// Number of MIMO layers (streams) (for definition see 36-212 V8.6 2009-03, p.17)
  u8 Nl;                       
} LTE_DL_eNb_HARQ_t;

typedef struct {
  /// Flag indicating that this ULSCH has a new packet (start of new round) 
  u8 Ndi;
  /// Status Flag indicating for this ULSCH (idle,active,disabled)
  SCH_status_t status;
  /// Subframe scheduling indicator (i.e. Transmission opportunity indicator)
  u8 subframe_scheduling_flag;
  /// First Allocated RB 
  u16 first_rb;
  /// Current Number of RBs
  u16 nb_rb;
  /// Last TPC command
  u8 TPC;
  /// Transport block size
  u32 TBS;
  /// The payload + CRC size in bits, "B" from 36-212 
  u16 B;  
  /// Pointer to the payload
  u8 *b;             
  /// Pointers to transport block segments
  u8 *c[MAX_NUM_ULSCH_SEGMENTS]; 
  /// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)
  u32 RTC[MAX_NUM_ULSCH_SEGMENTS];
  /// Index of current HARQ round for this ULSCH                
  u8 round;                
  /// MCS format of this ULSCH
  u8 mcs;            
  /// Redundancy-version of the current sub-frame
  u8 rvidx;
  /// Turbo-code outputs (36-212 V8.6 2009-03, p.12 
  u8 d[MAX_NUM_ULSCH_SEGMENTS][(96+3+(3*6144))];  
  /// Sub-block interleaver outputs (36-212 V8.6 2009-03, p.16-17)
  u8 w[MAX_NUM_ULSCH_SEGMENTS][3*6144]; 
  /// Number of code segments (for definition see 36-212 V8.6 2009-03, p.9)            
  u32 C;                         
  /// Number of "small" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  u32 Cminus;                    
  /// Number of "large" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  u32 Cplus;                     
  /// Number of bits in "small" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10) 
  u32 Kminus;                    
  /// Number of bits in "large" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  u32 Kplus;                     
  /// Number of "Filler" bits (for definition see 36-212 V8.6 2009-03, p.10)
  u32 F;                         
  /// Msc_initial, Initial number of subcarriers for ULSCH (36-212, v8.6 2009-03, p.26-27)
  u16 Msc_initial;
  /// Nsymb_initial, Initial number of symbols for ULSCH (36-212, v8.6 2009-03, p.26-27)
  u8 Nsymb_initial;
} LTE_UL_UE_HARQ_t;

typedef struct {
  /// Allocated RNTI (0 means DLSCH_t is not currently used)
  u16 rnti; 
  /// Active flag for baseband transmitter processing
  u8 active;
  /// Target MCS
  u8 target_mcs; 
  /// Transmission mode
  u8 mode;
  /// Current HARQ process id
  u8 current_harq_pid;
  /// Process ID's per subframe
  u8 harq_ids[10];
  /// Current RB allocation
  u32 rb_alloc[4];
  /// Current subband PMI allocation
  u16 pmi_alloc;
  /// Current subband RI allocation
  u32 ri_alloc;
  /// Current subband CQI1 allocation
  u32 cqi_alloc1;
  /// Current subband CQI2 allocation
  u32 cqi_alloc2;
  /// Current Number of RBs
  u16 nb_rb;
  /// Pointers to 8 HARQ processes for the DLSCH
  LTE_DL_eNb_HARQ_t *harq_processes[8];     
  /// Number of soft channel bits
  u16 G;
  /// Layer index for this dlsch (0,1)
  u8 layer_index;          
  /// Codebook index for this dlsch (0,1,2,3)
  u8 codebook_index;          
  /// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18) 
  u8 e[MAX_NUM_CHANNEL_BITS];
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)             
  u8 Mdlharq;  
  /// MIMO transmission mode indicator for this sub-frame (for definition see 36-212 V8.6 2009-03, p.17)
  u8 Kmimo;
} LTE_eNb_DLSCH_t;

typedef struct {
  /// Current Number of Symbols
  u8 Nsymb_pusch;
  /// Pointers to 8 HARQ processes for the ULSCH
  LTE_UL_UE_HARQ_t *harq_processes[8];     
  /// Pointer to CQI data
  u8 o[MAX_CQI_BITS];
  /// Length of CQI data (bits)
  u8 O;
  /// Rank information 
  u8 o_RI[2];
  /// Length of rank information (bits)
  u8 O_RI;
  /// Pointer to ACK
  u8 o_ACK[4];
  /// Length of ACK information (bits)
  u8 O_ACK;
  /// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18) 
  u8 e[MAX_NUM_CHANNEL_BITS];
  /// Interleaved "h"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18) 
  u8 h[MAX_NUM_CHANNEL_BITS];
  /// Scrambled "b"-sequences (for definition see 36-211 V8.6 2009-03, p.14) 
  u8 b_tilde[MAX_NUM_CHANNEL_BITS];
  /// Modulated "d"-sequences (for definition see 36-211 V8.6 2009-03, p.14) 
  mod_sym_t d[MAX_NUM_RE];
  /// Transform-coded "z"-sequences (for definition see 36-211 V8.6 2009-03, p.14-15) 
  mod_sym_t z[MAX_NUM_RE];
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)             
  u8 Mdlharq; 
  /// "q" sequences for CQI/PMI (for definition see 36-212 V8.6 2009-03, p.27)
  u8 q[MAX_CQI_PAYLOAD];
  /// coded and interleaved CQI bits
  u8 o_w[(MAX_CQI_BITS+8)*3];
  /// coded CQI bits
  u8 o_d[96+((MAX_CQI_BITS+8)*3)];
  /// coded ACK bits
  u8 q_ACK[MAX_ACK_PAYLOAD];
  /// coded RI bits
  u8 q_RI[MAX_RI_PAYLOAD];
  /// beta_offset_cqi times 8
  u8 beta_offset_cqi_times8;
  /// beta_offset_ri times 8
  u8 beta_offset_ri_times8;
  /// beta_offset_harqack times 8
  u8 beta_offset_harqack_times8;
  /// power_offset
  u8 power_offset;
} LTE_UE_ULSCH_t;

typedef struct {
  /// Flag indicating that this ULSCH has new data
  u8 Ndi;
  /// Status Flag indicating for this ULSCH (idle,active,disabled)
  SCH_status_t status;
  /// Subframe scheduling indicator (i.e. Transmission opportunity indicator)
  u8 subframe_scheduling_flag;
  /// PHICH active flag
  u8 phich_active;
  /// PHICH ACK
  u8 phich_ACK;
  /// Last TPC command
  u8 TPC;
  /// First Allocated RB 
  u16 first_rb;
  /// Current Number of RBs
  u16 nb_rb;
  /// Transport block size
  u32 TBS;
  /// The payload + CRC size in bits  
  u16 B; 
  /// Pointer to the payload
  u8 *b;  
  /// Pointers to transport block segments
  u8 *c[MAX_NUM_ULSCH_SEGMENTS];
  /// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)  
  u32 RTC[8]; 
  /// Index of current HARQ round for this ULSCH
  u8 round; 
  /// MCS format for this DLSCH
  u8 mcs; 
  /// Redundancy-version of the current sub-frame
  u8 rvidx;
  /// soft bits for each received segment ("w"-sequence)(for definition see 36-212 V8.6 2009-03, p.15) 
  s16 w[MAX_NUM_ULSCH_SEGMENTS][3*(6144+64)];
  /// soft bits for each received segment ("d"-sequence)(for definition see 36-212 V8.6 2009-03, p.15)    
  s16 *d[MAX_NUM_ULSCH_SEGMENTS];
  /// Number of code segments (for definition see 36-212 V8.6 2009-03, p.9)   
  u32 C;  
  /// Number of "small" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  u32 Cminus; 
  /// Number of "large" code segments (for definition see 36-212 V8.6 2009-03, p.10) 
  u32 Cplus;  
  /// Number of bits in "small" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10) 
  u32 Kminus; 
  /// Number of bits in "large" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10) 
  u32 Kplus;
  /// Number of "Filler" bits (for definition see 36-212 V8.6 2009-03, p.10)  
  u32 F;  
  /// Number of MIMO layers (streams) (for definition see 36-212 V8.6 2009-03, p.17)
  u8 Nl;  
  /// Msc_initial, Initial number of subcarriers for ULSCH (36-212, v8.6 2009-03, p.26-27)
  u16 Msc_initial;
  /// Nsymb_initial, Initial number of symbols for ULSCH (36-212, v8.6 2009-03, p.26-27)
  u8 Nsymb_initial;
} LTE_UL_eNb_HARQ_t;

typedef struct {
  /// Current Number of Symbols
  u8 Nsymb_pusch;
  /// Pointers to 8 HARQ processes for the ULSCH
  LTE_UL_eNb_HARQ_t *harq_processes[8];     
  /// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18) 
  s16 e[MAX_NUM_CHANNEL_BITS];
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)             
  u8 Mdlharq; 
  /// CQI CRC status
  u8 cqi_crc_status;
  /// Pointer to CQI data
  u8 o[MAX_CQI_BITS+8];
  /// Length of CQI data under RI=1 assumption(bits)
  u8 Or1;
  /// Length of CQI data under RI=2 assumption(bits)
  u8 Or2;
  /// Rank information 
  u8 o_RI[2];
  /// Length of rank information (bits)
  u8 O_RI;
  /// Pointer to ACK
  u8 o_ACK[4];
  /// Length of ACK information (bits)
  u8 O_ACK;
  /// "q" sequences for CQI/PMI (for definition see 36-212 V8.6 2009-03, p.27)
  s8 q[MAX_CQI_PAYLOAD];
  /// number of coded CQI bits after interleaving
  u8 o_RCC;
  /// coded and interleaved CQI bits
  s8 o_w[(MAX_CQI_BITS+8)*3];
  /// coded CQI bits
  s8 o_d[96+((MAX_CQI_BITS+8)*3)];
  /// coded ACK bits
  s16 q_ACK[MAX_ACK_PAYLOAD];
  /// coded RI bits
  s16 q_RI[MAX_RI_PAYLOAD];
  /// beta_offset_cqi times 8
  u8 beta_offset_cqi_times8;
  /// beta_offset_ri times 8
  u8 beta_offset_ri_times8;
  /// beta_offset_harqack times 8
  u8 beta_offset_harqack_times8;
  /// Flag to indicate that eNB awaits UE RRCConnRequest 
  u8 RRCConnRequest_active;
  /// Flag to indicate that eNB should decode UE RRCConnRequest 
  u8 RRCConnRequest_flag;
  /// Subframe for RRCConnRequest
  u8 RRCConnRequest_subframe;
  /// Frame for RRCConnRequest
  u32 RRCConnRequest_frame;
  /// RNTI attributed to this ULSCH
  u16 rnti;
} LTE_eNb_ULSCH_t;

typedef struct {
  /// Flag indicating that this DLSCH has a new transport block
  u8 Ndi;
  /// DLSCH status flag indicating 
  SCH_status_t status;
  /// Transport block size
  u32 TBS;
  /// The payload + CRC size in bits  
  u16 B; 
  /// Pointer to the payload
  u8 *b;  
  /// Pointers to transport block segments
  u8 *c[MAX_NUM_DLSCH_SEGMENTS];
  /// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)  
  u32 RTC[8]; 
  /// Index of current HARQ round for this DLSCH
  u8 round; 
  /// MCS format for this DLSCH
  u8 mcs; 
  /// Redundancy-version of the current sub-frame
  u8 rvidx;
  /// MIMO mode for this DLSCH
  MIMO_mode_t mimo_mode;
  /// soft bits for each received segment ("w"-sequence)(for definition see 36-212 V8.6 2009-03, p.15) 
  s16 w[MAX_NUM_DLSCH_SEGMENTS][3*(6144+64)];
  /// soft bits for each received segment ("d"-sequence)(for definition see 36-212 V8.6 2009-03, p.15)    
  s16 *d[MAX_NUM_DLSCH_SEGMENTS];
  /// Number of code segments (for definition see 36-212 V8.6 2009-03, p.9)   
  u32 C;  
  /// Number of "small" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  u32 Cminus; 
  /// Number of "large" code segments (for definition see 36-212 V8.6 2009-03, p.10) 
  u32 Cplus;  
  /// Number of bits in "small" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10) 
  u32 Kminus; 
  /// Number of bits in "large" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10) 
  u32 Kplus;
  /// Number of "Filler" bits (for definition see 36-212 V8.6 2009-03, p.10)  
  u32 F;  
  /// Number of MIMO layers (streams) (for definition see 36-212 V8.6 2009-03, p.17)
  u8 Nl;  
} LTE_DL_UE_HARQ_t;

typedef struct {
  s32 UL_rssi[NB_ANTENNAS_RX];
  u8 DL_cqi[2];
  u8 DL_diffcqi[2];
  u16 DL_pmi_single;
  u16 DL_pmi_dual;
  u8 rank;
  u16 crnti; ///user id (rnti) of connected UEs
  s32 UE_timing_offset; ///timing offset of connected UEs (for timing advance signalling)
  UE_MODE_t mode;
  u8 sector;
  s32 dlsch_NAK[8];
  s32 dlsch_l2_errors;
  s32 dlsch_trials[4];
  s32 ulsch_errors[3];
  s32 ulsch_consecutive_errors[3];
  s32 ulsch_decoding_attempts[3][4];
  s32 ulsch_round_errors[3][4];
  s8 dlsch_mcs_offset;
  s8 cont_res_id[6];
} LTE_eNB_UE_stats;

typedef struct {
  /// HARQ process id
  u8 harq_id;
  /// ACK bits (after decoding)
  u8 ack;
} harq_status_t;

typedef struct {
  /// Active flag for DLSCH demodulation
  u8 active;
  /// Transmission mode
  u8 mode1_flag;
  /// Current HARQ process id
  u8 current_harq_pid;
  /// Current RB allocation
  u32 rb_alloc[4];
  /// Current subband PMI allocation
  u16 pmi_alloc;
  /// Current subband antenna selection
  u32 antenna_alloc;
  /// Current subband RI allocation
  u32 ri_alloc;
  /// Current subband CQI1 allocation
  u32 cqi_alloc1;
  /// Current subband CQI2 allocation
  u32 cqi_alloc2;
  /// Current Number of RBs
  u16 nb_rb;
  /// HARQ-ACKs
  harq_status_t harq_ack[10];
  /// Pointers to up to 8 HARQ processes
  LTE_DL_UE_HARQ_t *harq_processes[8];   
  /// Layer index for this DLSCH
  u8 layer_index;              
  /// Number of soft channel bits
  u16 G;
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17
  u8 Mdlharq;              
  /// MIMO transmission mode indicator for this sub-frame (for definition see 36-212 V8.6 2009-03, p.17)
  u8 Kmimo;                
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
  u8 dci_length;
  /// Aggregation level 
  u8 L;
  /// rnti
  u16 rnti;
  /// Format
  DCI_format_t format;
  /// DCI pdu
  u8 dci_pdu[1+(MAX_DCI_SIZE_BITS/8)];
} DCI_ALLOC_t;


void free_eNb_dlsch(LTE_eNb_DLSCH_t *dlsch);

LTE_eNb_DLSCH_t *new_eNb_dlsch(u8 Kmimo,u8 Mdlharq);

void free_ue_dlsch(LTE_UE_DLSCH_t *dlsch);

LTE_UE_DLSCH_t *new_ue_dlsch(u8 Kmimo,u8 Mdlharq);

void free_eNb_dlsch(LTE_eNb_DLSCH_t *dlsch);

LTE_eNb_ULSCH_t *new_eNb_ulsch(u8 Mdlharq);

void free_ue_ulsch(LTE_UE_ULSCH_t *ulsch);

LTE_UE_ULSCH_t *new_ue_ulsch(u8 Mdlharq);



/** \fn dlsch_encoding(u8 *input_buffer,
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
s32 dlsch_encoding(u8 *a,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   LTE_eNb_DLSCH_t *dlsch);




// Functions below implement 36-211

/** \fn allocate_REs_in_RB(mod_sym_t **txdataF,
    u32 *jj,
    u16 re_offset,
    u32 symbol_offset,
    u8 *output,
    MIMO_mode_t mimo_mode,
    u8 nu,
    u8 pilots,
    u8 first_pilot,
    u8 mod_order,
    u8 precoder_index,
    s16 amp,
    u32 *re_allocated,
    u8 skip_dc,
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

s32 allocate_REs_in_RB(mod_sym_t **txdataF,
		       u32 *jj,
		       u16 re_offset,
		       u32 symbol_offset,
		       u8 *output,
		       MIMO_mode_t mimo_mode,
		       u8 nu,
		       u8 pilots,
		       u8 first_pilot,
		       u8 mod_order,
		       u8 precoder_index,
		       s16 amp,
		       u32 *re_allocated,
		       u8 skip_dc,
		       LTE_DL_FRAME_PARMS *frame_parms);

/** \fn s32 dlsch_modulation(mod_sym_t **txdataF,
    s16 amp,
    u32 sub_frame_offset,
    LTE_DL_FRAME_PARMS *frame_parms,
    LTE_eNb_DLSCH_t *dlsch);

    \brief This function is the top-level routine for generation of the sub-frame signal (frequency-domain) for DLSCH.  
    @param txdataF Table of pointers for frequency-domain TX signals
    @param amp Amplitude of signal
    @param sub_frame_offset Offset of this subframe in units of subframes (usually 0)
    @param frame_parms Pointer to frame descriptor
    @param dlsch Pointer to DLSCH descriptor for this allocation

*/ 
s32 dlsch_modulation(mod_sym_t **txdataF,
		     s16 amp,
		     u32 sub_frame_offset,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     LTE_eNb_DLSCH_t *dlsch);


/** \fn generate_pilots(mod_sym_t **txdataF,
    s16 amp,
    LTE_DL_FRAME_PARMS *frame_parms,
    u8 eNb_id,
    u16 N);
    \brief This function generates the frequency-domain pilots (cell-specific downlink reference signals)
    for N subframes.
    @param txdataF Table of pointers for frequency-domain TX signals
    @param amp Amplitude of signal
    @param frame_parms Pointer to frame descriptor
    @param eNb_id Nid2 (0,1,2)
    @param N Number of sub-frames to generate
*/
void generate_pilots(mod_sym_t **txdataF,
		     s16 amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     u8 eNb_id,
		     u16 N);

/**
   \brief This function generates the frequency-domain pilots (cell-specific downlink reference signals) for one slot only
   @param txdataF Table of pointers for frequency-domain TX signals
   @param amp Amplitude of signal
   @param frame_parms Pointer to frame descriptor
   @param eNb_id Nid2 (0,1,2)
   @param is_secondary_eNb (0,1,2)
   @param slot index (0..19)
*/
s32 generate_pilots_slot(mod_sym_t **txdataF,
			 s16 amp,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 u8 eNb_id,
			 u16 slot);


s32 generate_pss(mod_sym_t **txdataF,
		 s16 amp,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 u16 eNb_id,
		 u16 l,
		 u16 Ns);

s32 generate_pbch(mod_sym_t **txdataF,
		  s32 amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  u8 *pbch_pdu);


/** \fn qpsk_qpsk(s16 *stream0_in,
    s16 *stream1_in,
    s16 *stream0_out,
    s16 *rho01,
    s32 length
    ) 

    \brief This function computes the LLRs for ML (max-logsum approximation) dual-stream QPSK/QPSK reception.
    @param stream0_in Input from channel compensated (MR combined) stream 0
    @param stream1_in Input from channel compensated (MR combined) stream 1
    @param stream0_out Output from LLR unit for stream0
    @param stream0_out Output from LLR unit for stream1
    @param rho01 Cross-correlation between channels (MR combined)
    @param length in complex channel outputs
*/
void qpsk_qpsk(s16 *stream0_in,
	       s16 *stream1_in,
	       s16 *stream0_out,
	       s16 *rho01,
	       s32 length
	       );

/** \fn dlsch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
    s32 **rxdataF_comp,
    s32 **rxdataF_comp_i,
    s32 **rho_i,
    s16 *dlsch_llr,
    u8 symbol,
    u16 nb_rb,
    s16 **llr128p)

    \brief This function perform LLR computation for dual-stream (QPSK/QPSK) transmission.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp Compensated channel output for interference
    @param rho_i Correlation between channel of signal and inteference
    @param dlsch_llr llr output
    @param symbol OFDM symbol index in sub-frame
    @param nb_rb number of RBs for this allocation
    @param llr128p pointer to pointer to symbol in dlsch_llr
*/

s32 dlsch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
			s32 **rxdataF_comp,
			s32 **rxdataF_comp_i,
			s32 **rho_i,
			s16 *dlsch_llr,
			u8 symbol,
			u16 nb_rb,
			s16 **llr128p);

/** \fn dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
    s32 **rxdataF_comp,
    s16 *dlsch_llr,
    u8 symbol,
    u16 nb_rb,
    s16 **llr128p)

    \brief This function generates log-likelihood ratios (decoder input) for single-stream QPSK received waveforms.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param dlsch_llr llr output
    @param symbol OFDM symbol index in sub-frame
    @param nb_rb number of RBs for this allocation
    @param llr128p pointer to pointer to symbol in dlsch_llr
*/
s32 dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
		   s32 **rxdataF_comp,
		   s16 *dlsch_llr,
		   u8 symbol,
		   u16 nb_rb,
		   s16 **llr128p);

/** \fn dlsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
    s32 **rxdataF_comp,
    s16 *dlsch_llr,
    s32 **dl_ch_mag,
    u8 symbol,
    u16 nb_rb,
    s16 **llr128p)
    \brief This function generates log-likelihood ratios (decoder input) for single-stream 16QAM received waveforms
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param dlsch_llr llr output
    @param dl_ch_mag Squared-magnitude of channel in each resource element position corresponding to allocation and weighted for mid-point in 16QAM constellation
    @param symbol OFDM symbol index in sub-frame
    @param nb_rb number of RBs for this allocation
    @param llr128p pointer to pointer to symbol in dlsch_llr
*/

void dlsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     s32 **rxdataF_comp,
		     s16 *dlsch_llr,
		     s32 **dl_ch_mag,
		     u8 symbol,
		     u16 nb_rb,
		     s16 **llr128p);

/** \fn void dlsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
    s32 **rxdataF_comp,
    s16 *dlsch_llr,
    s32 **dl_ch_mag,
    s32 **dl_ch_magb,
    u8 symbol,
    u16 nb_rb)
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
		     s32 **rxdataF_comp,
		     s16 *dlsch_llr,
		     s32 **dl_ch_mag,
		     s32 **dl_ch_magb,
		     u8 symbol,
		     u16 nb_rb);

/** \fn dlsch_siso(LTE_DL_FRAME_PARMS *frame_parms,
    s32 **rxdataF_comp,
    s32 **rxdataF_comp_i,
    u8 l,
    u16 nb_rb)
    \brief This function does the first stage of llr computation for SISO, by just extracting the pilots, PBCH and primary/secondary synchronization sequences.
    @param frame_parms Frame descriptor structure
    @param rxdataF_comp Compensated channel output
    @param rxdataF_comp Compensated channel output for interference
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
    @param dl_ch_mag Second squared-magnitude of channel (64QAM only) for LLR computation.  Alamouti combining should be performed on this as well. Result is stored in first antenna position
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
    @param rho Cross correlation between signal and inteference channels
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
u16 dlsch_extract_rbs_single(s32 **rxdataF,
					s32 **dl_ch_estimates,
					s32 **rxdataF_ext,
					s32 **dl_ch_estimates_ext,
					u16 pmi,
					u8 *pmi_ext,
					u32 *rb_alloc,
					u8 symbol,
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
u16 dlsch_extract_rbs_dual(s32 **rxdataF,
				      s32 **dl_ch_estimates,
				      s32 **rxdataF_ext,
				      s32 **dl_ch_estimates_ext,
				      u16 pmi,
				      u8 *pmi_ext,
				      u32 *rb_alloc,
				      u8 symbol,
				      LTE_DL_FRAME_PARMS *frame_parms);

/** \fn dlsch_channel_compensation(s32 **rxdataF_ext,
    s32 **dl_ch_estimates_ext,
    s32 **dl_ch_mag,
    s32 **dl_ch_magb,
    s32 **rxdataF_comp,
    s32 **rho,
    LTE_DL_FRAME_PARMS *frame_parms,
    u8 symbol,
    u8 mod_order,
    u16 nb_rb,
    u8 output_shift,
    PHY_VARS_UE *phy_vars_ue)
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
    @param UE PHY_measurements
*/
void dlsch_channel_compensation(s32 **rxdataF_ext,
				s32 **dl_ch_estimates_ext,
				s32 **dl_ch_mag,
				s32 **dl_ch_magb,
				s32 **rxdataF_comp,
				s32 **rho,
				LTE_DL_FRAME_PARMS *frame_parms,
				u8 symbol,
				u8 mod_order,
				u16 nb_rb,
				u8 output_shift,
				PHY_MEASUREMENTS *phy_measurements);

/** \fn dlsch_channel_level(s32 **dl_ch_estimates_ext,
    LTE_DL_FRAME_PARMS *frame_parms,
    s32 *avg,
    u16 nb_rb)
    \brief This function computes the average channel level over all allocated RBs and antennas (TX/RX) in order to compute output shift for compensated signal
    @param dl_ch_estimates_ext Channel estimates in allocated RBs
    @param frame_parms Pointer to frame descriptor
    @param avg Pointer to average signal strength
    @param nb_rb Number of allocated RBs
*/
void dlsch_channel_level(s32 **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 s32 *avg,
			 u16 nb_rb);

/** \fn u32 void  dlsch_decoding(u16 A,
    s16 *dlsch_llr,
    LTE_DL_FRAME_PARMS *lte_frame_parms,
    LTE_UE_DLSCH_t *dlsch,
    u8 subframe)

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
u32 dlsch_decoding(s16 *dlsch_llr,
			    LTE_DL_FRAME_PARMS *lte_frame_parms,
			    LTE_UE_DLSCH_t *dlsch,
			    u8 subframe);


/** \fn rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
    LTE_UE_DLSCH **lte_ue_dlsch_vars,
    LTE_DL_FRAME_PARMS *frame_parms,
    u8 eNb_id,
    u8 eNb_id_i,
    LTE_UE_DLSCH_t **dlsch_ue,
    u8 symbol,
    u8 dual_stream_UE)
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
    @param UE PHY_measurements
    @param is_secondary_ue Flag to indicate wether it should follow special receiver logic
*/
s32 rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_DLSCH **lte_ue_dlsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     u8 eNb_id,
	     u8 eNb_id_i,
	     LTE_UE_DLSCH_t **dlsch_ue,
	     u8 symbol,
	     u8 dual_stream_UE,
	     PHY_MEASUREMENTS *phy_measurements,
	     u8 is_secondary_ue);

s32 rx_pdcch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_PDCCH **lte_ue_pdcch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     u8 eNb_id,
	     u8 n_pdcch_symbols,
	     MIMO_mode_t mimo_mode,
	     u8 is_secondary_ue);


s32 rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
	    LTE_UE_PBCH *lte_ue_pbch_vars,
	    LTE_DL_FRAME_PARMS *frame_parms,
	    u8 eNb_id,
	    MIMO_mode_t mimo_mode);

/*! \brief PBCH unscrambling
  This is similar to pbch_scrabling with the difference that inputs are signed s16s (llr values) and instead of flipping bits we change signs.
  \param frame_parms Pointer to frame descriptor
  \param coded_data Output of the coding and rate matching
  \param length Length of the sequence
*/ 
void pbch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		     u8* coded_data,
		     u32 length);

/*! \brief PBCH unscrambling
  This is similar to pbch_scrabling with the difference that inputs are signed s16s (llr values) and instead of flipping bits we change signs.
  \param frame_parms Pointer to frame descriptor
  \param llr Output of the demodulator
  \param length Length of the sequence
*/ 
void pbch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
		       s16* llr,
		       u32 length);

/*! \brief DCI Encoding
  This routine codes an arbitrary DCI PDU after appending the 8-bit 3GPP CRC.  It then applied sub-block interleaving and rate matching.
  \param a Pointer to DCI PDU (coded in bytes)
  \param A Length of DCI PDU in bits
  \param E Length of DCI PDU in coded bits
  \param e Pointer to sequence
  \param rnti RNTI for CRC scrambling
*/ 
void dci_encoding(u8 *a,
		  u8 A,
		  u16 E,
		  u8 *e,
		  u16 rnti);

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
void generate_dci_top(u8 num_ue_spec_dci,
		      u8 num_common_dci,
		      DCI_ALLOC_t *dci_alloc, 
		      u32 n_rnti,
		      s16 amp,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      mod_sym_t **txdataF,
		      u32 sub_frame_offset);


void generate_64qam_table(void);
void generate_16qam_table(void);

u16 extract_crc(u8 *dci,u8 DCI_LENGTH);

/*! \brief LLR from two stream.
  This function takes two stream (qpsk modulated) and calculates the LLR, considering one stream as interference.
  \param stream0_in pointer to first stream0
  \param stream1_in pointer to first stream1
  \param stream0_out pointer to output stream
  \param rho pointer to correlation matrix
  \param length
*/ 
void qpsk_qpsk(s16 *stream0_in,
	       s16 *stream1_in,
	       s16 *stream0_out,
	       s16 *rho01,
	       s32 length
	       );

void dci_decoding(u8 DCI_LENGTH,
		  u8 DCI_FMT,
		  s8 *e,
		  u8 *decoded_output);

u16 dci_decoding_procedure(LTE_UE_PDCCH **lte_ue_pdcch_vars,
				      DCI_ALLOC_t *dci_alloc,
				      s16 eNb_id,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      u16 si_rnti,
				      u16 ra_rnti);

u8 get_Qm(u8 I_MCS);

u8 get_I_TBS(u8 I_MCS);

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

/*!
  \brief This function generate the sounding reference symbol (SRS) for the uplink according to 36.211 v8.6.0. If IFFT_FPGA is defined, the SRS is quantized to a QPSK sequence.
  @param frame_parms LTE DL Frame Parameters
  @param txdataF pointer to the frequency domain TX signal
  @param amp amplitudte of the transmit signal (irrelevant for #ifdef IFFT_FPGA)
  @sub_frame_offset  Offset of this subframe in units of subframes
*/


s32 generate_srs_tx(LTE_DL_FRAME_PARMS *frame_parms,
		    mod_sym_t *txdataF,
		    s16 amp,
		    u32 sub_frame_number);

/*!
  \brief This function is similar to generate_srs_tx but generates a conjugate sequence for channel estimation. If IFFT_FPGA is defined, the SRS is quantized to a QPSK sequence.
  @param frame_parms LTE DL Frame Parameters
  @param txdataF pointer to the frequency domain TX signal
  @param amp amplitudte of the transmit signal (irrelevant for #ifdef IFFT_FPGA)
  @sub_frame_offset  Offset of this subframe in units of subframes
*/

s32 generate_srs_rx(LTE_DL_FRAME_PARMS *frame_parms,
		    s32 *txdataF);

/*!
  \brief This function generates the downlink reference signal for the PUSCH according to 36.211 v8.6.0. The DRS occuies the RS defined by rb_alloc and the symbols 2 and 8 for extended CP and 3 and 10 for normal CP.
*/

s32 generate_drs_pusch(LTE_DL_FRAME_PARMS *frame_parms,
		       mod_sym_t *txdataF,
		       s16 amp,
		       u32 sub_frame_number,
		       u32 first_rb,
		       u32 nb_rb,
		       u8 n_ue,
		       u8 relay_flag,
		       u8 diversity_scheme);

s32 compareints (const void * a, const void * b);


void ulsch_modulation(mod_sym_t **txdataF,
		      s16 amp,
		      u32 subframe,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      LTE_UE_ULSCH_t *ulsch,
		      u8 relay_flag,
		      u8 diversity_scheme,
		      u8 n_ue);


void ulsch_extract_rbs_single(s32 **rxdataF,
			      s32 **rxdataF_ext,
			      u32 first_rb,
			      u32 nb_rb,
			      u8 l,
			      u8 Ns,
			      LTE_DL_FRAME_PARMS *frame_parms);

u8 subframe2harq_pid_tdd(u8 tdd_config,u8 subframe);
u8 subframe2harq_pid_tdd_eNBrx(u8 tdd_config,u8 subframe);

s32 generate_ue_dlsch_params_from_dci(u8 subframe,
				      void *dci_pdu,
				      u16 rnti,
				      DCI_format_t dci_format,
				      LTE_UE_DLSCH_t **dlsch_ue,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      u16 si_rnti,
				      u16 ra_rnti,
				      u16 p_rnti);

s32 generate_eNb_dlsch_params_from_dci(u8 subframe,
				       void *dci_pdu,
				       u16 rnti,
				       DCI_format_t dci_format,
				       LTE_eNb_DLSCH_t **dlsch_eNb,
				       LTE_DL_FRAME_PARMS *frame_parms,
				       u16 si_rnti,
				       u16 ra_rnti,
				       u16 p_rnti,
				       u16 DL_pmi_single);

s32 generate_eNb_ulsch_params_from_rar(u8 *rar_pdu,
				       u8 subframe,
				       LTE_eNb_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms);

s32 generate_ue_ulsch_params_from_dci(void *dci_pdu,
				      u16 rnti,
				      u8 subframe,
				      DCI_format_t dci_format,
				      LTE_UE_ULSCH_t *ulsch,
				      LTE_UE_DLSCH_t **dlsch,
				      PHY_MEASUREMENTS *meas,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      u16 si_rnti,
				      u16 ra_rnti,
				      u16 p_rnti,
				      u8 eNb_id,
				      s32 current_dlsch_cqi); 

s32 generate_ue_ulsch_params_from_rar(u8 *rar_pdu,
				      u8 subframe,
				      LTE_UE_ULSCH_t *ulsch,
				      PHY_MEASUREMENTS *meas,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      u8 eNb_id,
				      s32 current_dlsch_cqi);

s32 generate_eNb_ulsch_params_from_dci(void *dci_pdu,
				       u16 rnti,
				       u8 subframe,
				       DCI_format_t dci_format,
				       LTE_eNb_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms,
				       u16 si_rnti,
				       u16 ra_rnti,
				       u16 p_rnti);


void generate_pcfich_reg_mapping(LTE_DL_FRAME_PARMS *frame_parms);

void generate_phich_reg_mapping_ext(LTE_DL_FRAME_PARMS *frame_parms);

void init_transport_channels(u8);

void generate_RIV_tables(void);


s32 *rx_ulsch(LTE_eNB_COMMON *eNB_common_vars,
	      LTE_eNB_ULSCH *eNB_ulsch_vars,
	      LTE_DL_FRAME_PARMS *frame_parms,
	      u32 subframe,
	      u8 eNb_id,  // this is the effective sector id
	      LTE_eNb_ULSCH_t *ulsch,
	      u8 relay_flag,
	      u8 diversity_scheme);

s32 ulsch_encoding(u8 *a,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   LTE_UE_ULSCH_t *ulsch,
		   u8 harq_pid);

u32  ulsch_decoding(s16 *ulsch_llr,
		    LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_eNb_ULSCH_t *ulsch,
		    u8 subframe);


void generate_phich_top(LTE_DL_FRAME_PARMS *frame_parms,
			u8 subframe,
			LTE_eNb_ULSCH_t *ulsch_eNb,
			mod_sym_t **txdataF);

void print_CQI(void *o,u8 *o_RI,UCI_format fmt,u8 eNB_id);

void extract_CQI(void *o,u8 *o_RI,UCI_format fmt,LTE_eNB_UE_stats *stats);

void fill_CQI(void *o,UCI_format fmt,PHY_MEASUREMENTS *meas,u8 eNb_id, s32 current_dlsch_cqi);

u16 quantize_subband_pmi(PHY_MEASUREMENTS *meas,u8 eNb_id);
u16 quantize_subband_pmi2(PHY_MEASUREMENTS *meas,u8 eNb_id,u8 a_id);

u32 pmi2hex_2Ar1(u16 pmi);

u32 pmi2hex_2Ar2(u8 pmi);

u32 cqi2hex(u16 cqi);

u16 computeRIV(u16 N_RB_DL,u16 RBstart,u16 Lcrbs);

u32 pmi_extend(LTE_DL_FRAME_PARMS *frame_parms,u8 wideband_pmi);

/**@}*/
#endif
