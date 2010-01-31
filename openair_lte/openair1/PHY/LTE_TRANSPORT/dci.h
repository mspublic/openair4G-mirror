/* file: dci.h
   purpose: typedefs for LTE DCI structures from 36-212, V8.6 2009-03.  Limited to 5 MHz formats for the moment.
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 
*/
///  DCI Format Type 0 (5 MHz,TDD0, 25 bits)
typedef struct {
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  unsigned char type:1;
  /// Hopping flag
  unsigned char hopping:1;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  unsigned short rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  unsigned char mcs:5;
  /// New Data Indicator
  unsigned char ndi:1;
  /// Power Control
  unsigned char TPC:2;
  /// Cyclic shift
  unsigned char cshift:3;
  /// DAI (TDD)
  unsigned char ulindex:2;
  /// CQI Request
  unsigned char cqi_req:1;
  /// Padding to get to size of DCI1A
  unsigned char padding:2;
} DCI0_5MHz_TDD0_t;
#define sizeof_DCI_0_5MHz_TDD_0_t 27;

///  DCI Format Type 0 (5 MHz,TDD1-6, 28 bits)
typedef struct {
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  unsigned char type:1;
  /// Hopping flag
  unsigned char hopping:1;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  unsigned short rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  unsigned char mcs:5;
  /// New Data Indicator
  unsigned char ndi:1;
  /// Power Control
  unsigned char TPC:2;
  /// Cyclic shift
  unsigned char dai:2;
  /// CQI Request
  unsigned char cqi_req:1;
  /// Padding
  unsigned char padding:5;
} DCI0_5MHz_TDD_1_6_t;
#define sizeof_DCI0_5MHz_TDD_1_6_t 27;

///  DCI Format Type 0 (5 MHz,FDD, 25 bits)
typedef struct {
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  unsigned char type:1;
  /// Hopping flag
  unsigned char hopping:1;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  unsigned short rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  unsigned char mcs:5;
  /// New Data Indicator
  unsigned char ndi:1;
  /// Power Control
  unsigned char TPC:2;
  /// CQI Request
  unsigned char cqi_req:1;
  /// Padding
  unsigned char padding:5;
} DCI0_5MHz_FDD_t;
#define sizeof_DCI0_5MHz_FDD_t 25;


/// DCI Format Type 1 (5 MHz, TDD, 30 bits)
typedef struct {
  /// Resource Allocation Header
  unsigned char rah:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Modulation and Coding Scheme and Redundancy Version
  unsigned char mcs:5;
  /// HARQ Process
  unsigned char harq_pid:4;
  /// New Data Indicator
  unsigned char ndi:1;
  /// Redundancy version
  unsigned char rv:2;
  /// Power Control
  unsigned char TPC:2;
  /// DAI (TDD)
  unsigned char dai:2;
} DCI1_5MHz_TDD_t;
#define sizeof_DCI1_5MHz_TDD_t 30;

/// DCI Format Type 1 (5 MHz, FDD, 28 bits)
typedef struct {
  /// Resource Allocation Header
  unsigned char rah:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Modulation and Coding Scheme and Redundancy Version
  unsigned char mcs:5;
  /// HARQ Process
  unsigned char harq_pid:3;
  /// New Data Indicator
  unsigned char ndi:1;
  /// Redundancy version
  unsigned char rv:2;
  /// Power Control
  unsigned char TPC:2;
} DCI1_5MHz_FDD_t;
#define sizeof_DCI1_5MHz_FDD_t 28;

/// RA Procedure PDSCH (FDD/TDD), 13 bits
typedef struct {
  /// Preamble Index
  unsigned char preamble_index:6;
  /// PRACH mask index
  unsigned char prach_mask_index:4;
  /// Padding
  unsigned char padding:3;
} RA_PDSCH_t;
#define sizeof_RA_PDSCH_t 13;

/// Normal PDSCH (FDD), 13 bits
typedef struct {
  /// Modulation and Coding Scheme and Redundancy Version
  unsigned char mcs:5;
  /// HARQ Process
  unsigned char harq_pid:3;
  /// New Data Indicator
  unsigned char ndi:1;
  /// Redundancy version
  unsigned char rv:2;
  /// Power Control
  unsigned char TPC:2;
} PDSCH_FDD_t;
#define sizeof_PDSCH_FDD_t 13;

/// Normal PDSCH (TDD), 16 bits
typedef struct {
  /// Modulation and Coding Scheme and Redundancy Version
  unsigned char mcs:5;
  /// HARQ Process
  unsigned char harq_pid:4;
  /// New Data Indicator
  unsigned char ndi:1;
  /// Redundancy version
  unsigned char rv:2;
  /// Power Control
  unsigned char TPC:2;
  /// Downlink Assignment Index
  unsigned char dai:2;
} PDSCH_TDD_t;
#define sizeof_PDSCH_TDD_t 16;

/// DCI Format Type 1A (5 MHz, FDD, 25 bits)
typedef struct {
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  unsigned char type:1;
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  union {
    RA_PDSCH_t ra_pdsch;
    PDSCH_FDD_t pdsch;
  } pdu;
  /// Padding to remove size ambiguity (24 bits -> 25 bits)
  unsigned char padding:1;
} DCI1A_5MHz_FDD_t;
#define sizeof_DCI1A_5MHz_FDD_t 25;

/// DCI Format Type 1A (5 MHz, TDD, frame 1-6, 27 bits)
typedef struct {
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  unsigned char type:1;
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  union {
    RA_PDSCH_t ra_pdsch;
    PDSCH_TDD_t pdsch;
  } pdu;
  unsigned char padding:1;
} DCI1A_5MHz_TDD_1_6_t;
#define sizeof_DCI1B_5MHz_TDD_1_6_t 27;

/// DCI Format Type 1A (5 MHz, TDD, frame 0, 27 bits)
typedef struct {
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  unsigned char type:1;
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  union {
    RA_PDSCH_t ra_pdsch;
    PDSCH_TDD_t pdsch;
  } pdu;
} DCI1A_5MHz_TDD_0_t;
#define sizeof_DCI1B_5MHz_TDD_0_t 27;


/// DCI Format Type 1B (5 MHz, FDD, 2 Antenna Ports, 27 bits)
typedef struct {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_FDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:2;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
  /// Padding to remove size ambiguity (26 bits -> 27 bits)
  unsigned char padding:1;
} DCI1B_5MHz_2A_FDD_t;
#define sizeof_DCI1B_5MHz_FDD_t 27;

/// DCI Format Type 1B (5 MHz, TDD, 2 Antenna Ports, 29 bits)
typedef struct {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_TDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:2;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
} DCI1B_5MHz_2A_TDD_t;
#define sizeof_DCI1B_5MHz_2A_TDD_t 29;

/// DCI Format Type 1B (5 MHz, FDD, 4 Antenna Ports, 28 bits)
typedef struct {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_FDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:4;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
} DCI1B_5MHz_4A_FDD_t;
#define sizeof_DCI1B_5MHz_4A_FDD_t 28;

/// DCI Format Type 1B (5 MHz, TDD, 4 Antenna Ports, 31 bits)
typedef struct {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_TDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:4;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
} DCI1B_5MHz_4A_TDD_t;
#define sizeof_DCI1B_5MHz_4A_TDD_t 31;

/// DCI Format Type 1C (5 MHz, 12 bits)
typedef struct {
  unsigned char rballoc:7;
  unsigned char tbs_index:5;
} DCI1C_5MHz_t;
#define sizeof_DCI1C_5MHz_t 12;

/// DCI Format Type 1D (5 MHz, FDD, 2 Antenna Ports, 27 bits)
typedef struct {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_FDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:2;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
  /// Downlink Power Offset
  unsigned char dl_power_off:1;
} DCI1D_5MHz_2A_FDD_t;
#define sizeof_DCI1D_5MHz_2A_FDD_t 27;

/// DCI Format Type 1D (5 MHz, TDD, 2 Antenna Ports, 30 bits)
typedef struct {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_TDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:2;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
  /// Downlink Power Offset
  unsigned char dl_power_off:1;
} DCI1D_5MHz_2A_TDD_t;
#define sizeof_DCI1D_5MHz_2A_TDD_t 30;

/// DCI Format Type 1D (5 MHz, FDD, 4 Antenna Ports, 29 bits)
typedef struct {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_FDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:4;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
  /// Downlink Power Offset
  unsigned char dl_power_off:1;
} DCI1D_5MHz_4A_FDD_t;
#define sizeof_DCI1D_5MHz_4A_FDD_t 29;

/// DCI Format Type 1D (5 MHz, TDD, 4 Antenna Ports, 33 bits)
typedef struct {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_TDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:4;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
  /// Downlink Power Offset
  unsigned char dl_power_off:1;
  /// Padding to remove size ambiguity (32 bits -> 33 bits)
  unsigned char padding:1;
} DCI1D_5MHz_4A_TDD_t;
#define sizeof_DCI1D_5MHz_4A_TDD_t 33;

/// DCI Format Type 2 (5 MHz, TDD, 2 Antenna Ports, less than 10 PRBs, 41 bits)
typedef struct {
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// Downlink Assignment Index
  unsigned char dai:2;
  /// HARQ Process
  unsigned char harq_pid:4;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:3;
} DCI2_5MHz_2A_L10PRB_TDD_t;
#define sizeof_DCI2_5MHz_2A_L10PRB_TDD_t 41;

/// DCI Format Type 2 (5 MHz, TDD, 4 Antenna Ports, less than 10 PRBs, 45 bits)
typedef struct {
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// Downlink Assignment Index
  unsigned char dai:2;
  /// HARQ Process
  unsigned char harq_pid:4;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:6;
  /// Padding for ambiguous sizes (44 -> 45 bits)
  unsigned char padding:1;
} DCI2_5MHz_4A_L10PRB_TDD_t;
#define sizeof_DCI2_5MHz_4A_L10PRB_TDD_t 45;

/// DCI Format Type 2 (5 MHz, TDD, 2 Antenna Ports, more than 10 PRBs, 42 bits)
typedef struct {
  /// Resource Allocation Header
  unsigned char rah:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// Downlink Assignment Index
  unsigned char dai:2;
  /// HARQ Process
  unsigned char harq_pid:4;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:3;
} DCI2_5MHz_2A_M10PRB_TDD_t;
#define sizeof_DCI2_5MHz_2A_M10PRB_TDD_t 42;

/// DCI Format Type 2 (5 MHz, TDD, 4 Antenna Ports, more than 10 PRBs, 45 bits)
typedef struct {
  /// Resource Allocation Header
  unsigned char rah:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// Downlink Assignment Index
  unsigned char dai:2;
  /// HARQ Process
  unsigned char harq_pid:4;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:6;
} DCI2_5MHz_4A_M10PRB_TDD_t;
#define sizeof_DCI2_5MHz_4A_M10PRBTDD_t 45;


/// DCI Format Type 2 (5 MHz, FDD, 2 Antenna Ports, less than 10 PRBs, 38 bits)
typedef struct {
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// HARQ Process
  unsigned char harq_pid:3;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:3;
} DCI2_5MHz_2A_L10PRB_FDD_t;
#define sizeof_DCI2_5MHz_2A_L10PRB_FDD_t 38;

/// DCI Format Type 2 (5 MHz, FDD, 4 Antenna Ports, less than 10 PRBs, 41 bits)
typedef struct {
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// HARQ Process
  unsigned char harq_pid:3;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:6;
} DCI2_5MHz_4A_L10PRB_FDD_t;
#define sizeof_DCI2_5MHz_4A_L10PRB_FDD_t 41;

/// DCI Format Type 2 (5 MHz, FDD, 2 Antenna Ports, more than 10 PRBs, 39 bits)
typedef struct {
  /// Resource Allocation Header
  unsigned char rah:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// HARQ Process
  unsigned char harq_pid:3;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:3;
} DCI2_5MHz_2A_M10PRB_FDD_t;
#define sizeof_DCI2_5MHz_2A_M10PRB_FDD_t 39;

/// DCI Format Type 2 (5 MHz, TDD, 4 Antenna Ports, more than 10 PRBs, 42 bits)
typedef struct {
  /// Resource Allocation Header
  unsigned char rah:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// HARQ Process
  unsigned char harq_pid:3;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:6;
} DCI2_5MHz_4A_M10PRB_FDD_t;
#define sizeof_DCI2_5MHz_4A_M10PRBFDD_t 42;



/// DCI Format Type 2A (5 MHz, TDD, 2 Antenna Ports, less than 10 PRBs, 38 bits)
typedef struct {
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// Downlink Assignment Index
  unsigned char dai:2;
  /// HARQ Process
  unsigned char harq_pid:4;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
} DCI2A_5MHz_2A_L10PRB_TDD_t;
#define sizeof_DCI2A_5MHz_2A_L10PRB_TDD_t 38;

/// DCI Format Type 2A (5 MHz, TDD, 4 Antenna Ports, less than 10 PRBs, 41 bits)
typedef struct {
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// Downlink Assignment Index
  unsigned char dai:2;
  /// HARQ Process
  unsigned char harq_pid:4;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:2;
  /// Padding for ambiguous sizes (40 -> 41 bits)
  unsigned char padding:1;
} DCI2A_5MHz_4A_L10PRB_TDD_t;
#define sizeof_DCI2A_5MHz_4A_L10PRB_TDD_t 41;

/// DCI Format Type 2A (5 MHz, TDD, 2 Antenna Ports, more than 10 PRBs, 39 bits)
typedef struct {
  /// Resource Allocation Header
  unsigned char rah:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// Downlink Assignment Index
  unsigned char dai:2;
  /// HARQ Process
  unsigned char harq_pid:4;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
} DCI2A_5MHz_2A_M10PRB_TDD_t;
#define sizeof_DCI2A_5MHz_2A_M10PRB_TDD_t 39;

/// DCI Format Type 2A (5 MHz, TDD, 4 Antenna Ports, more than 10 PRBs, 45 bits)
typedef struct {
  /// Resource Allocation Header
  unsigned char rah:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// Downlink Assignment Index
  unsigned char dai:2;
  /// HARQ Process
  unsigned char harq_pid:4;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:2;
} DCI2A_5MHz_4A_M10PRB_TDD_t;
#define sizeof_DCI2A_5MHz_4A_M10PRBTDD_t 45;


/// DCI Format Type 2A (5 MHz, FDD, 2 Antenna Ports, less than 10 PRBs, 35 bits)
typedef struct {
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// HARQ Process
  unsigned char harq_pid:3;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
} DCI2A_5MHz_2A_L10PRB_FDD_t;
#define sizeof_DCI2A_5MHz_2A_L10PRB_FDD_t 35;

/// DCI Format Type 2A (5 MHz, FDD, 4 Antenna Ports, less than 10 PRBs, 37 bits)
typedef struct {
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// HARQ Process
  unsigned char harq_pid:3;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:2;
} DCI2A_5MHz_4A_L10PRB_FDD_t;
#define sizeof_DCI2A_5MHz_4A_L10PRB_FDD_t 37;

/// DCI Format Type 2A (5 MHz, FDD, 2 Antenna Ports, more than 10 PRBs, 36 bits)
typedef struct {
  /// Resource Allocation Header
  unsigned char rah:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// HARQ Process
  unsigned char harq_pid:3;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
} DCI2A_5MHz_2A_M10PRB_FDD_t;
#define sizeof_DCI2A_5MHz_2A_M10PRB_FDD_t 36;

/// DCI Format Type 2A (5 MHz, TDD, 4 Antenna Ports, more than 10 PRBs, 38 bits)
typedef struct {
  /// Resource Allocation Header
  unsigned char rah:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:13;
  /// Power Control
  unsigned char TPC:2;
  /// HARQ Process
  unsigned char harq_pid:3;
  /// TB swap
  unsigned char tb_swap:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  unsigned char mcs1:5;
  /// New Data Indicator 1
  unsigned char ndi1:1;
  /// Redundancy version 1
  unsigned char rv1:2;
  /// Modulation and Coding Scheme and Redundancy Version 2
  unsigned char mcs2:5;
  /// New Data Indicator 2
  unsigned char ndi2:1;
  /// Redundancy version 2
  unsigned char rv2:2;
  /// TPMI information for precoding
  unsigned char tpmi:2;
} DCI2A_5MHz_4A_M10PRB_FDD_t;
#define sizeof_DCI2A_5MHz_4A_M10PRBFDD_t 38;


typedef struct {
  unsigned int TPC:28;
} DCI3_5MHz_TDD_0_t;
#define sizeof_DCI0_5MHz_TDD_0_t 27;

typedef struct {
  unsigned int TPC:28;
} DCI3_5MHz_TDD_1_6_t;
#define sizeof_DCI0_5MHz_TDD_1_6_t 27;


typedef struct {
  unsigned int TPC:26;
} DCI3_5MHz_FDD_t;
#define sizeof_DCI0_5MHz_FDD_t 25;

#define MAX_DCI_SIZE_BITS 45
