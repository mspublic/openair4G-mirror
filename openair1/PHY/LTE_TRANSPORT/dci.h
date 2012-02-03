/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file PHY/LTE_TRANSPORT/dci.h
* \brief typedefs for LTE DCI structures from 36-212, V8.6 2009-03.  Limited to 5 MHz formats for the moment.Current LTE compliance V8.6 2009-03.
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/
#include "PHY/types.h"

///  DCI Format Type 0 (5 MHz,TDD0, 27 bits)
struct DCI0_5MHz_TDD0 {
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
} __attribute__ ((__packed__));

typedef struct DCI0_5MHz_TDD0 DCI0_5MHz_TDD0_t;
#define sizeof_DCI0_5MHz_TDD_0_t 27

///  DCI Format Type 0 (5 MHz,TDD1-6, 27 bits)
struct DCI0_5MHz_TDD_1_6 {
  /// Padding
  u32 padding:7;
  /// CQI Request
  u32 cqi_req:1;
  /// DAI
  u32 dai:2;
  /// Cyclic shift
  u32 cshift:3;
  /// Power Control
  u32 TPC:2;
  /// New Data Indicator
  u32 ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version
  u32 mcs:5;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  u32 rballoc:9;
  /// Hopping flag
  u32 hopping:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  u32 type:1;
} __attribute__ ((__packed__));

typedef struct DCI0_5MHz_TDD_1_6 DCI0_5MHz_TDD_1_6_t;
#define sizeof_DCI0_5MHz_TDD_1_6_t 27

///  DCI Format Type 0 (5 MHz,FDD, 25 bits)
struct DCI0_5MHz_FDD {
  /// Padding
  u32 padding:2;
  /// CQI Request
  u32 cqi_req:1;
  /// DRS Cyclic Shift
  u32 cshift:3;
  /// Power Control
  u32 TPC:2;
  /// New Data Indicator
  u32 ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version
  u32 mcs:5;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  u32 rballoc:9;
  /// Hopping flag
  u32 hopping:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  u32 type:1;

} __attribute__ ((__packed__));

typedef struct DCI0_5MHz_FDD DCI0_5MHz_FDD_t;
#define sizeof_DCI0_5MHz_FDD_t 25


/// DCI Format Type 1 (5 MHz, TDD, 30 bits)
struct DCI1_5MHz_TDD {
  /// Dummy bits to align to 32-bits
  u32 dummy:2;
  /// DAI (TDD)
  u32 dai:2;
  /// Power Control
  u32 TPC:2;
  /// Redundancy version
  u32 rv:2;
  /// New Data Indicator
  u32 ndi:1;
  /// HARQ Process
  u32 harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  u32 mcs:5;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  u32 rballoc:13;
  /// Resource Allocation Header
  u32 rah:1;
} __attribute__ ((__packed__));

typedef struct DCI1_5MHz_TDD DCI1_5MHz_TDD_t;
#define sizeof_DCI1_5MHz_TDD_t 30

/// DCI Format Type 1 (5 MHz, FDD, 27 bits)
struct DCI1_5MHz_FDD {
  /// dummy bits (not transmitted)
  u32 dummy:5;
  /// Power Control
  u32 TPC:2;
  /// Redundancy version
  u32 rv:2;
  /// New Data Indicator
  u32 ndi:1;
  /// HARQ Process
  u32 harq_pid:3;
  /// Modulation and Coding Scheme and Redundancy Version
  u32 mcs:5;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  u32 rballoc:13;
  /// Resource Allocation Header
  u32 rah:1;
} __attribute__ ((__packed__));

typedef struct DCI1_5MHz_FDD DCI1_5MHz_FDD_t;
#define sizeof_DCI1_5MHz_FDD_t 27

/// RA Procedure PDSCH (FDD), 13 bits
struct RA_PDSCH_FDD {
  /// Preamble Index
  unsigned char preamble_index:6;
  /// PRACH mask index
  unsigned char prach_mask_index:4;
  /// Padding
  unsigned char padding:3;
} __attribute__ ((__packed__));

typedef struct RA_PDSCH_FDD RA_PDSCH_FDD_t;
#define sizeof_RA_PDSCH_FDD_t 13

/// RA Procedure PDSCH (TDD), 16 bits
struct RA_PDSCH_TDD {
  /// Preamble Index
  unsigned char preamble_index:6;
  /// PRACH mask index
  unsigned char prach_mask_index:4;
  /// Padding
  unsigned char padding:6;
} __attribute__ ((__packed__));

typedef struct RA_PDSCH_TDD RA_PDSCH_TDD_t;
#define sizeof_RA_PDSCH_TDD_t 16

/// Normal PDSCH (FDD), 13 bits
struct PDSCH_FDD {
  /// Modulation and Coding Scheme and Redundancy Version
  u32 mcs:5;
  /// HARQ Process
  u32 harq_pid:3;
  /// New Data Indicator
  u32 ndi:1;
  /// Redundancy version
  u32 rv:2;
  /// Power Control
  u32 TPC:2;
} __attribute__ ((__packed__));

typedef struct PDSCH_FDD PDSCH_FDD_t;
#define sizeof_PDSCH_FDD_t 13

/// Normal PDSCH (TDD), 16 bits
struct PDSCH_TDD {
  /// Modulation and Coding Scheme and Redundancy Version
  u32 mcs:5;
  /// HARQ Process
  u32 harq_pid:4;
  /// New Data Indicator
  u32 ndi:1;
  /// Redundancy version
  u32 rv:2;
  /// Power Control
  u32 TPC:2;
  /// Downlink Assignment Index
  u32 dai:2;
} __attribute__ ((__packed__));

typedef struct PDSCH_TDD PDSCH_TDD_t;
#define sizeof_PDSCH_TDD_t 16


struct DCI1A_5MHz_FDD {
  /// padding
  u32 padding:7;
  /// Downlink Assignment Index
  u32 srs_req:1;
  /// Power Control
  u32 TPC:2;
  /// Redundancy version
  u32 rv:2;
  /// New Data Indicator
  u32 ndi:1;
  /// HARQ Process
  u32 harq_pid:3;
  /// Modulation and Coding Scheme and Redundancy Version
  u32 mcs:5;
   /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  u32 rballoc:9;
  /// Localized/Distributed VRB
  u32 vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  u32 type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_5MHz_FDD DCI1A_5MHz_FDD_t;
#define sizeof_DCI1A_5MHz_FDD_t 25

/// DCI Format Type 1A (5 MHz, TDD, frame 1-6, 27 bits)
struct DCI1A_5MHz_TDD_1_6 {
  /// padding
  u32 padding:4;
  /// SRS request bit
  u32 srs_req:1;
  /// Downlink Assignment Index
  u32 dai:2;
  /// Power Control
  u32 TPC:2;
  /// Redundancy version
  u32 rv:2;
  /// New Data Indicator
  u32 ndi:1;
  /// HARQ Process
  u32 harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  u32 mcs:5;
   /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  u32 rballoc:9;
  /// Localized/Distributed VRB
  u32 vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  u32 type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_5MHz_TDD_1_6 DCI1A_5MHz_TDD_1_6_t;
#define sizeof_DCI1A_5MHz_TDD_1_6_t 27

/// DCI Format Type 1A (5 MHz, TDD, frame 1-6, 27 bits)
struct DCI1A_RA_5MHz_TDD_1_6 {
  /// Dummy bits to align to 32-bits
  u32 dummy:5;
  /// Padding
  u32 padding:6;
  /// PRACH mask index
  u32 prach_mask_index:4;
  /// Preamble Index
  u32 preamble_index:6;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  u32 rballoc:9;
  /// Localized/Distributed VRB
  u32 vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  u32 type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_RA_5MHz_TDD_1_6 DCI1A_RA_5MHz_TDD_1_6_t;
#define sizeof_DCI1A_RA_5MHz_TDD_1_6_t 27

/// DCI Format Type 1A (5 MHz, TDD, frame 0, 27 bits)
struct DCI1A_5MHz_TDD_0 {
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  unsigned char type:1;
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  union {
    RA_PDSCH_TDD_t ra_pdsch;
    PDSCH_TDD_t pdsch;
  } pdu;
} __attribute__ ((__packed__));

typedef struct DCI1A_5MHz_TDD_0 DCI1A_5MHz_TDD_0_t;
#define sizeof_DCI1A_5MHz_TDD_0_t 27


/// DCI Format Type 1B (5 MHz, FDD, 2 Antenna Ports, 27 bits)
struct DCI1B_5MHz_2A_FDD {
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
} __attribute__ ((__packed__));

typedef struct DCI1B_5MHz_2A_FDD DCI1B_5MHz_2A_FDD_t;
#define sizeof_DCI1B_5MHz_FDD_t 27

/// DCI Format Type 1B (5 MHz, TDD, 2 Antenna Ports, 29 bits)
struct DCI1B_5MHz_2A_TDD {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_TDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:2;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
} __attribute__ ((__packed__));

typedef struct DCI1B_5MHz_2A_TDD DCI1B_5MHz_2A_TDD_t;
#define sizeof_DCI1B_5MHz_2A_TDD_t 29

/// DCI Format Type 1B (5 MHz, FDD, 4 Antenna Ports, 28 bits)
struct DCI1B_5MHz_4A_FDD {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_FDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:4;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
} __attribute__ ((__packed__));

typedef struct DCI1B_5MHz_4A_FDD DCI1B_5MHz_4A_FDD_t;
#define sizeof_DCI1B_5MHz_4A_FDD_t 28

/// DCI Format Type 1B (5 MHz, TDD, 4 Antenna Ports, 31 bits)
struct DCI1B_5MHz_4A_TDD {
  /// Localized/Distributed VRB
  unsigned char vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  unsigned short rballoc:9;
  PDSCH_TDD_t pdsch;
  /// TPMI information for precoding
  unsigned char tpmi:4;
  /// TMI confirmation for precoding
  unsigned char pmi:1;
} __attribute__ ((__packed__));

typedef struct DCI1B_5MHz_4A_TDD DCI1B_5MHz_4A_TDD_t;
#define sizeof_DCI1B_5MHz_4A_TDD_t 31

/// DCI Format Type 1C (5 MHz, 12 bits)
typedef struct __attribute__ ((__packed__)){
  unsigned char rballoc:7;
  unsigned char tbs_index:5;
} DCI1C_5MHz_t;
#define sizeof_DCI1C_5MHz_t 12

/// DCI Format Type 1D (5 MHz, FDD, 2 Antenna Ports, 27 bits)
struct DCI1D_5MHz_2A_FDD {
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
} __attribute__ ((__packed__));

typedef struct DCI1D_5MHz_2A_FDD DCI1D_5MHz_2A_FDD_t;
#define sizeof_DCI1D_5MHz_2A_FDD_t 27

/// DCI Format Type 1D (5 MHz, TDD, 2 Antenna Ports, 30 bits)
struct DCI1D_5MHz_2A_TDD {
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
} __attribute__ ((__packed__));

typedef struct DCI1D_5MHz_2A_TDD DCI1D_5MHz_2A_TDD_t;
#define sizeof_DCI1D_5MHz_2A_TDD_t 30

/// DCI Format Type 1D (5 MHz, FDD, 4 Antenna Ports, 29 bits)
struct DCI1D_5MHz_4A_FDD {
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
}  __attribute__ ((__packed__));

typedef struct DCI1D_5MHz_4A_FDD DCI1D_5MHz_4A_FDD_t;
#define sizeof_DCI1D_5MHz_4A_FDD_t 29

/// DCI Format Type 1D (5 MHz, TDD, 4 Antenna Ports, 33 bits)
struct DCI1D_5MHz_4A_TDD {
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
} __attribute__ ((__packed__));

typedef struct DCI1D_5MHz_4A_TDD DCI1D_5MHz_4A_TDD_t;
#define sizeof_DCI1D_5MHz_4A_TDD_t 33


///******************NEW DCI Format for MU-MIMO****************///////////

/// DCI Format Type 1E (5 MHz, TDD, 2 Antenna Ports, more than 10 PRBs, 34 bits)
struct DCI1E_5MHz_2A_M10PRB_TDD {
  /// padding to 64bits
  u64 padding64:30;
  /// Redundancy version 2
  ///u64 rv2:2;
  /// New Data Indicator 2
  ///u64 ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  ///u64 mcs2:5;
  /// TPMI information for precoding
  u64 tpmi:3;
  /// Redundancy version 1
  u64 rv:2;
  /// New Data Indicator 1
  u64 ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  u64 mcs:5;
  /// TB swap
  ///u64 tb_swap:1;
  /// HARQ Process
  u64 harq_pid:4;
  /// Downlink Assignment Index
  u64 dai:2;
  /// Power Control
  u64 TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  u64 rballoc:13;
  /// Resource Allocation Header
  u64 rah:1;
  /// Downlink Power offset for MU-MIMO
  u8 dl_power_off:1;
} __attribute__ ((__packed__));
typedef struct DCI1E_5MHz_2A_M10PRB_TDD DCI1E_5MHz_2A_M10PRB_TDD_t;
#define sizeof_DCI1E_5MHz_2A_M10PRB_TDD_t 34


/// DCI Format Type 2 (5 MHz, TDD, 2 Antenna Ports, less than 10 PRBs, 41 bits)
struct DCI2_5MHz_2A_L10PRB_TDD {
  /// padding to 64bits
  u64 padding64:22;
  /// Redundancy version 2
  u64 rv2:2;
  /// New Data Indicator 2
  u64 ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  u64 mcs2:5;
  /// TPMI information for precoding
  u64 tpmi:3;
  /// Redundancy version 1
  u64 rv1:2;
  /// New Data Indicator 1
  u64 ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  u64 mcs1:5;
  /// TB swap
  u64 tb_swap:1;
  /// HARQ Process
  u64 harq_pid:4;
  /// Downlink Assignment Index
  u64 dai:2;
  /// Power Control
  u64 TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  u64 rballoc:13;
} __attribute__ ((__packed__));

typedef struct DCI2_5MHz_2A_L10PRB_TDD DCI2_5MHz_2A_L10PRB_TDD_t;
#define sizeof_DCI2_5MHz_2A_L10PRB_TDD_t 41

/// DCI Format Type 2 (5 MHz, TDD, 4 Antenna Ports, less than 10 PRBs, 45 bits)
struct DCI2_5MHz_4A_L10PRB_TDD {
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
}  __attribute__ ((__packed__));

typedef struct DCI2_5MHz_4A_L10PRB_TDD DCI2_5MHz_4A_L10PRB_TDD_t;
#define sizeof_DCI2_5MHz_4A_L10PRB_TDD_t 45

/// DCI Format Type 2 (5 MHz, TDD, 2 Antenna Ports, more than 10 PRBs, 42 bits)
struct DCI2_5MHz_2A_M10PRB_TDD {
  /// padding to 64bits
  u64 padding64:22;
  /// Redundancy version 2
  u64 rv2:2;
  /// New Data Indicator 2
  u64 ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  u64 mcs2:5;
  /// TPMI information for precoding
  u64 tpmi:3;
  /// Redundancy version 1
  u64 rv1:2;
  /// New Data Indicator 1
  u64 ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  u64 mcs1:5;
  /// TB swap
  u64 tb_swap:1;
  /// HARQ Process
  u64 harq_pid:4;
  /// Downlink Assignment Index
  u64 dai:2;
  /// Power Control
  u64 TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  u64 rballoc:13;
  /// Resource Allocation Header
  u64 rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2_5MHz_2A_M10PRB_TDD DCI2_5MHz_2A_M10PRB_TDD_t;
#define sizeof_DCI2_5MHz_2A_M10PRB_TDD_t 42

/// DCI Format Type 2 (5 MHz, TDD, 4 Antenna Ports, more than 10 PRBs, 45 bits)
struct DCI2_5MHz_4A_M10PRB_TDD {
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
} __attribute__ ((__packed__));

typedef struct DCI2_5MHz_4A_M10PRB_TDD DCI2_5MHz_4A_M10PRB_TDD_t;
#define sizeof_DCI2_5MHz_4A_M10PRB_TDD_t 45


/// DCI Format Type 2 (5 MHz, FDD, 2 Antenna Ports, less than 10 PRBs, 38 bits)
struct DCI2_5MHz_2A_L10PRB_FDD {
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
} __attribute__ ((__packed__));

typedef struct DCI2_5MHz_2A_L10PRB_FDD DCI2_5MHz_2A_L10PRB_FDD_t;
#define sizeof_DCI2_5MHz_2A_L10PRB_FDD_t 38

/// DCI Format Type 2 (5 MHz, FDD, 4 Antenna Ports, less than 10 PRBs, 41 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2_5MHz_4A_L10PRB_FDD_t 41

/// DCI Format Type 2 (5 MHz, FDD, 2 Antenna Ports, more than 10 PRBs, 39 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2_5MHz_2A_M10PRB_FDD_t 39

/// DCI Format Type 2 (5 MHz, TDD, 4 Antenna Ports, more than 10 PRBs, 42 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2_5MHz_4A_M10PRB_FDD_t 42



/// DCI Format Type 2A (5 MHz, TDD, 2 Antenna Ports, less than 10 PRBs, 38 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2A_5MHz_2A_L10PRB_TDD_t 38

/// DCI Format Type 2A (5 MHz, TDD, 4 Antenna Ports, less than 10 PRBs, 41 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2A_5MHz_4A_L10PRB_TDD_t 41

/// DCI Format Type 2A (5 MHz, TDD, 2 Antenna Ports, more than 10 PRBs, 39 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2A_5MHz_2A_M10PRB_TDD_t 39

/// DCI Format Type 2A (5 MHz, TDD, 4 Antenna Ports, more than 10 PRBs, 45 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2A_5MHz_4A_M10PRB_TDD_t 45


/// DCI Format Type 2A (5 MHz, FDD, 2 Antenna Ports, less than 10 PRBs, 35 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2A_5MHz_2A_L10PRB_FDD_t 35

/// DCI Format Type 2A (5 MHz, FDD, 4 Antenna Ports, less than 10 PRBs, 37 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2A_5MHz_4A_L10PRB_FDD_t 37

/// DCI Format Type 2A (5 MHz, FDD, 2 Antenna Ports, more than 10 PRBs, 36 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2A_5MHz_2A_M10PRB_FDD_t 36

/// DCI Format Type 2A (5 MHz, TDD, 4 Antenna Ports, more than 10 PRBs, 38 bits)
typedef struct __attribute__ ((__packed__)){
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
#define sizeof_DCI2A_5MHz_4A_M10PRB_FDD_t 38



typedef struct __attribute__ ((__packed__)){
  unsigned int TPC:28;
} DCI3_5MHz_TDD_0_t;
#define sizeof_DCI3_5MHz_TDD_0_t 27

typedef struct __attribute__ ((__packed__)){
  unsigned int TPC:28;
} DCI3_5MHz_TDD_1_6_t;
#define sizeof_DCI3_5MHz_TDD_1_6_t 27


typedef struct __attribute__ ((__packed__)){
  unsigned int TPC:26;
} DCI3_5MHz_FDD_t;
#define sizeof_DCI3_5MHz_FDD_t 25

#define MAX_DCI_SIZE_BITS 45
