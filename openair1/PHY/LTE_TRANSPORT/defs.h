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

// structures below implement 36-211 and 36-212

/** @addtogroup _PHY_TRANSPORT_
 * @{
 */

#define MAX_NUM_PHICH_GROUPS 56  //110 RBs Ng=2, p.60 36-212, Sec. 6.9

#define NSOFT 1827072
#define LTE_NULL 2 

// maximum of 3 segments before each coding block if data length exceeds 6144 bits. 

#define MAX_NUM_DLSCH_SEGMENTS 3
#define MAX_NUM_ULSCH_SEGMENTS 3
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
} LTE_DL_eNB_HARQ_t;

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
  /// Indicator of TX activation per subframe.  Used during PUCCH detection for ACK/NAK.
  u8 subframe_tx[10];
  /// First CCE of last PDSCH scheduling per subframe.  Again used during PUCCH detection for ACK/NAK. 
  u8 nCCE[10];
  /// Current HARQ process id
  u8 current_harq_pid;
  /// Process ID's per subframe.  Used to associate received ACKs on PUSCH/PUCCH to DLSCH harq process ids
  u8 harq_ids[10];
  /// Window size (in outgoing transport blocks) for fine-grain rate adaptation
  u8 ra_window_size;
  /// First-round error threshold for fine-grain rate adaptation
  u8 error_threshold;
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
  LTE_DL_eNB_HARQ_t *harq_processes[8];     
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
  // downlink power offset field
  u8 dl_power_off;
} LTE_eNB_DLSCH_t;

typedef struct {
  /// Current Number of Symbols
  u8 Nsymb_pusch;
  /// Pointers to 8 HARQ processes for the ULSCH
  LTE_UL_UE_HARQ_t *harq_processes[8];     
  /// Pointer to CQI data
  u8 o[MAX_CQI_BYTES];
  /// Length of CQI data (bits)
  u8 O;
  /// Format of CQI data 
  UCI_format_t uci_format;
  /// Rank information 
  u8 o_RI[2];
  /// Length of rank information (bits)
  u8 O_RI;
  /// Pointer to ACK
  u8 o_ACK[4];
  /// Length of ACK information (bits)
  u8 O_ACK;
  /// Minimum number of CQI bits for PUSCH (36-212 r8.6, Sec 5.2.4.1 p. 37)
  u8 O_CQI_MIN;
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
  /// n_DMRS 2 for cyclic shift of DMRS (36.211 Table 5.5.1.1.-1)
  u8 n_DMRS2;
  // for cooperative communication
  u8 cooperation_flag;
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
} LTE_UL_eNB_HARQ_t;

typedef struct {
  /// Current Number of Symbols
  u8 Nsymb_pusch;
  /// Pointers to 8 HARQ processes for the ULSCH
  LTE_UL_eNB_HARQ_t *harq_processes[8];     
  /// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18) 
  s16 e[MAX_NUM_CHANNEL_BITS];
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)             
  u8 Mdlharq; 
  /// CQI CRC status
  u8 cqi_crc_status;
  /// Pointer to CQI data
  u8 o[MAX_CQI_BYTES];
  /// Format of CQI data 
  UCI_format_t uci_format;
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
  /// n_DMRS2 for cyclic shift of DM RS ( 3GPP 36.211 Table 5.5.2.1.1-1)
  u8 n_DMRS2;
  /// cyclic shift for DM RS
  u8 cyclicShift;
  /// cooperation flag
  u8 cooperation_flag;
} LTE_eNB_ULSCH_t;

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
  /// UL RSSI per receive antenna
  s32 UL_rssi[NB_ANTENNAS_RX];
  /// DL Wideband CQI index (2 TBs)
  u8 DL_cqi[2];
  /// DL Subband CQI index (from HLC feedback)
  u8 DL_subband_cqi[2][13];
  /// DL PMI Single Stream
  u16 DL_pmi_single;
  /// DL PMI Dual Stream
  u16 DL_pmi_dual;
  /// Current RI
  u8 rank;
  /// CRNTI of UE
  u16 crnti; ///user id (rnti) of connected UEs
  /// Timing offset
  s32 UE_timing_offset; ///timing offset of connected UEs (for timing advance signalling)
  /// Current mode of UE (NOT SYCHED, RAR, PUSCH)
  UE_MODE_t mode;
  /// Current sector where UE is attached
  u8 sector;
  /// 
  u32 dlsch_sliding_cnt;
  ///
  u32 dlsch_NAK[8];
  ///
  u32 dlsch_l2_errors;
  ///
  u32 dlsch_trials[4];
  ///
  u32 ulsch_errors[3];
  ///
  u32 ulsch_consecutive_errors[3];
  ///
  u32 ulsch_decoding_attempts[3][4];
  ///
  u32 ulsch_round_errors[3][4];
  u32 ulsch_decoding_attempts_last[3][4];
  u32 ulsch_round_errors_last[3][4];
  u32 ulsch_round_fer[3][4];
  s8 dlsch_mcs_offset;
  /// Target mcs1 after rate-adaptation (used by MAC layer scheduler)
  u8 dlsch_mcs1;
  /// Target mcs2 after rate-adaptation (used by MAC layer scheduler)
  u8 dlsch_mcs2;
  //  SRS_param_t SRS_parameters;
} LTE_eNB_UE_stats;

typedef struct {
  /// HARQ process id
  u8 harq_id;
  /// ACK bits (after decoding)
  u8 ack;
  /// send status (for PUCCH)
  u8 send_harq_status;
  /// nCCE (for PUCCH)
  u8 nCCE;
} harq_status_t;

typedef struct {
  /// RNTI
  u16 rnti;
  /// Active flag for DLSCH demodulation
  u8 active;
  /// Transmission mode
  u8 mode1_flag;
  // downlink power offset field
  u8 dl_power_off;
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
	      format2_2A_L10PRB,
	      format2_2A_M10PRB,
	      format2_2D_L10PRB,
	      format2_2D_M10PRB,
	      format2_4A_L10PRB,
	      format2_4A_M10PRB,
	      format2A_2A_L10PRB,
	      format2A_2A_M10PRB,
	      format2A_4A_L10PRB,
	      format2A_4A_M10PRB,
	      format3
} DCI_format_t;

typedef enum {
  pucch_format1,
  pucch_format1a,
  pucch_format1b,
  pucch_format2,
  pucch_format2a,
  pucch_format2b
} PUCCH_FMT_t;


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
  u8 dci_pdu[8];
} DCI_ALLOC_t;


/**@}*/
#endif
