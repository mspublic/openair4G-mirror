/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
   included in this distribution in the file called "COPYING". If not, 
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

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
#ifndef USER_MODE
#include "PHY/types.h"
#else
#include <stdint.h>
#endif

///  DCI Format Type 0 (5 MHz,TDD0, 27 bits)
struct DCI0_5MHz_TDD0 {
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
  /// Hopping flag
  uint32_t hopping:1;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Power Control
  uint32_t TPC:2;
  /// Cyclic shift
  uint32_t cshift:3;
  /// DAI (TDD)
  uint32_t ulindex:2;
  /// CQI Request
  uint32_t cqi_req:1;
  /// Padding to get to size of DCI1A
  uint32_t padding:2;
} __attribute__ ((__packed__));

typedef struct DCI0_5MHz_TDD0 DCI0_5MHz_TDD0_t;
#define sizeof_DCI0_5MHz_TDD_0_t 27

///  DCI Format Type 0 (1.5 MHz,TDD1-6, 23 bits)
struct DCI0_1_5MHz_TDD_1_6 {
  /// Padding
  uint32_t padding:11;
  /// CQI Request
  uint32_t cqi_req:1;
  /// DAI
  uint32_t dai:2;
  /// Cyclic shift
  uint32_t cshift:3;
  /// Power Control
  uint32_t TPC:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:5;
  /// Hopping flag
  uint32_t hopping:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI0_1_5MHz_TDD_1_6 DCI0_1_5MHz_TDD_1_6_t;
#define sizeof_DCI0_1_5MHz_TDD_1_6_t 24

/// DCI Format Type 1A (1.5 MHz, TDD, frame 1-6, 24 bits)
struct DCI1A_1_5MHz_TDD_1_6 {
  /// padding
  uint32_t padding:9;
  /// Downlink Assignment Index
  uint32_t dai:2;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
   /// RB Assignment (ceil(log2(N_RB_DL*(N_RB_DL-1)/2)) bits)
  uint32_t rballoc:5;
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_1_5MHz_TDD_1_6 DCI1A_1_5MHz_TDD_1_6_t;
#define sizeof_DCI1A_1_5MHz_TDD_1_6_t 24


///  DCI Format Type 0 (5 MHz,TDD1-6, 27 bits)
struct DCI0_5MHz_TDD_1_6 {
  /// Padding
  uint32_t padding:7;
  /// CQI Request
  uint32_t cqi_req:1;
  /// DAI
  uint32_t dai:2;
  /// Cyclic shift
  uint32_t cshift:3;
  /// Power Control
  uint32_t TPC:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:9;
  /// Hopping flag
  uint32_t hopping:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI0_5MHz_TDD_1_6 DCI0_5MHz_TDD_1_6_t;
#define sizeof_DCI0_5MHz_TDD_1_6_t 27

/// DCI Format Type 1A (5 MHz, TDD, frame 1-6, 27 bits)
struct DCI1A_5MHz_TDD_1_6 {
  /// padding
  uint32_t padding:5;
  /// Downlink Assignment Index
  uint32_t dai:2;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
   /// RB Assignment (ceil(log2(N_RB_DL*(N_RB_DL-1)/2)) bits)
  uint32_t rballoc:9;
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_5MHz_TDD_1_6 DCI1A_5MHz_TDD_1_6_t;
#define sizeof_DCI1A_5MHz_TDD_1_6_t 27


///  DCI Format Type 0 (10 MHz,TDD1-6, 29 bits)
struct DCI0_10MHz_TDD_1_6 {
  /// Padding
  uint32_t padding:5;
  /// CQI Request
  uint32_t cqi_req:1;
  /// DAI
  uint32_t dai:2;
  /// Cyclic shift
  uint32_t cshift:3;
  /// Power Control
  uint32_t TPC:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:11;
  /// Hopping flag
  uint32_t hopping:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI0_10MHz_TDD_1_6 DCI0_10MHz_TDD_1_6_t;
#define sizeof_DCI0_10MHz_TDD_1_6_t 29

/// DCI Format Type 1A (10 MHz, TDD, frame 1-6, 30 bits)
struct DCI1A_10MHz_TDD_1_6 {
  /// padding
  uint32_t padding:3;
  /// Downlink Assignment Index
  uint32_t dai:2;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
   /// RB Assignment (ceil(log2(N_RB_DL*(N_RB_DL-1)/2)) bits)
  uint32_t rballoc:11;
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_10MHz_TDD_1_6 DCI1A_10MHz_TDD_1_6_t;
#define sizeof_DCI1A_10MHz_TDD_1_6_t 29


///  DCI Format Type 0 (20 MHz,TDD1-6, 27 bits)
struct DCI0_20MHz_TDD_1_6 {
  /// Padding
  uint32_t padding:2;
  /// CQI request
  uint32_t cqi_req:1;
  /// DAI
  uint32_t dai:2;
  /// Cyclic shift
  uint32_t cshift:3;
  /// Power Control
  uint32_t TPC:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:13;
  /// Hopping flag
  uint32_t hopping:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI0_20MHz_TDD_1_6 DCI0_20MHz_TDD_1_6_t;
#define sizeof_DCI0_20MHz_TDD_1_6_t 31

/// DCI Format Type 1A (20 MHz, TDD, frame 1-6, 27 bits)
struct DCI1A_20MHz_TDD_1_6 {
  uint32_t padding:1;
  /// Downlink Assignment Index
  uint32_t dai:2;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
   /// RB Assignment (ceil(log2(N_RB_DL*(N_RB_DL-1)/2)) bits)
  uint32_t rballoc:13;
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_20MHz_TDD_1_6 DCI1A_20MHz_TDD_1_6_t;
#define sizeof_DCI1A_20MHz_TDD_1_6_t 31

///  DCI Format Type 0 (1.5 MHz,FDD, 25 bits)
struct DCI0_1_5MHz_FDD {
  /// Padding
  uint32_t padding:13;
  /// CQI Request
  uint32_t cqi_req:1;
  /// DRS Cyclic Shift
  uint32_t cshift:3;
  /// Power Control
  uint32_t TPC:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:5;
  /// Hopping flag
  uint32_t hopping:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;

} __attribute__ ((__packed__));

typedef struct DCI0_1_5MHz_FDD DCI0_1_5MHz_FDD_t;
#define sizeof_DCI0_1_5MHz_FDD_t 21

struct DCI1A_1_5MHz_FDD {
  /// padding
  uint32_t padding:12;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
   /// RB Assignment (ceil(log2(N_RB_DL(N_RB_DL+1)/2)) bits)
  uint32_t rballoc:5;
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_1_5MHz_FDD DCI1A_1_5MHz_FDD_t;
#define sizeof_DCI1A_1_5MHz_FDD_t 21


///  DCI Format Type 0 (5 MHz,FDD, 23 bits)
struct DCI0_5MHz_FDD {
  /// Padding
  uint32_t padding:9;
  /// CQI Request
  uint32_t cqi_req:1;
  /// DRS Cyclic Shift
  uint32_t cshift:3;
  /// Power Control
  uint32_t TPC:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:9;
  /// Hopping flag
  uint32_t hopping:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;

} __attribute__ ((__packed__));

typedef struct DCI0_5MHz_FDD DCI0_5MHz_FDD_t;
#define sizeof_DCI0_5MHz_FDD_t 25

struct DCI1A_5MHz_FDD {
  /// padding
  uint32_t padding:8;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
   /// RB Assignment (ceil(log2(N_RB_DL(N_RB_DL+1)/2)) bits)
  uint32_t rballoc:9;
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_5MHz_FDD DCI1A_5MHz_FDD_t;
#define sizeof_DCI1A_5MHz_FDD_t 25



///  DCI Format Type 0 (10 MHz,FDD, 25 bits)
struct DCI0_10MHz_FDD {
  /// Padding
  uint32_t padding:7;
  /// CQI Request
  uint32_t cqi_req:1;
  /// DRS Cyclic Shift
  uint32_t cshift:3;
  /// Power Control
  uint32_t TPC:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:11;
  /// Hopping flag
  uint32_t hopping:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;

} __attribute__ ((__packed__));

typedef struct DCI0_10MHz_FDD DCI0_10MHz_FDD_t;
#define sizeof_DCI0_10MHz_FDD_t 27

struct DCI1A_10MHz_FDD {
  /// padding
  uint32_t padding:6;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
   /// RB Assignment (ceil(log2(N_RB_DL(N_RB_DL+1)/2)) bits)
  uint32_t rballoc:11;
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_10MHz_FDD DCI1A_10MHz_FDD_t;
#define sizeof_DCI1A_10MHz_FDD_t 27

///  DCI Format Type 0 (20 MHz,FDD, 25 bits)
struct DCI0_20MHz_FDD {
  /// Padding
  uint32_t padding:5;
  /// CQI Request
  uint32_t cqi_req:1;
  /// DRS Cyclic Shift
  uint32_t cshift:3;
  /// Power Control
  uint32_t TPC:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:13;
  /// Hopping flag
  uint32_t hopping:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;

} __attribute__ ((__packed__));

typedef struct DCI0_20MHz_FDD DCI0_20MHz_FDD_t;
#define sizeof_DCI0_20MHz_FDD_t 28

struct DCI1A_20MHz_FDD {
  /// padding
  uint32_t padding:4;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
   /// RB Assignment (ceil(log2(N_RB_DL*(N_RB_DL+1)/2)) bits)
  uint32_t rballoc:13;
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_20MHz_FDD DCI1A_20MHz_FDD_t;
#define sizeof_DCI1A_20MHz_FDD_t 28

/// DCI Format Type 1 (1.5 MHz, TDD, 23 bits)
struct DCI1_1_5MHz_TDD {
  /// Dummy bits to align to 32-bits
  uint32_t dummy:9;
  /// DAI (TDD)
  uint32_t dai:2;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:6;
  /// Resource Allocation Header
  uint32_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI1_1_5MHz_TDD DCI1_1_5MHz_TDD_t;
#define sizeof_DCI1_1_5MHz_TDD_t 23

/// DCI Format Type 1 (5 MHz, TDD, 30 bits)
struct DCI1_5MHz_TDD {
  /// Dummy bits to align to 32-bits
  uint32_t dummy:2;
  /// DAI (TDD)
  uint32_t dai:2;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:13;
  /// Resource Allocation Header
  uint32_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI1_5MHz_TDD DCI1_5MHz_TDD_t;
#define sizeof_DCI1_5MHz_TDD_t 30

/// DCI Format Type 1 (10 MHz, TDD, 34 bits)
struct DCI1_10MHz_TDD {
  /// Dummy bits to align to 64-bits
  uint64_t dummy:30;
  /// DAI (TDD)
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// Redundancy version
  uint64_t rv:2;
  /// New Data Indicator
  uint64_t ndi:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  uint64_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI1_10MHz_TDD DCI1_10MHz_TDD_t;
#define sizeof_DCI1_10MHz_TDD_t 34

/// DCI Format Type 1 (20 MHz, TDD, 42 bits)
struct DCI1_20MHz_TDD {
  /// Dummy bits to align to 64-bits
  uint64_t dummy:22;
  /// DAI (TDD)
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// Redundancy version
  uint64_t rv:2;
  /// New Data Indicator
  uint64_t ndi:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  uint64_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI1_20MHz_TDD DCI1_20MHz_TDD_t;
#define sizeof_DCI1_20MHz_TDD_t 42

/// DCI Format Type 1 (1.5 MHz, FDD, 21 bits)
struct DCI1_1_5MHz_FDD {
  /// Dummy bits to align to 32-bits
  uint32_t dummy:11;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(N_RB_DL/P)) bits)
  uint32_t rballoc:6;
  /// Resource Allocation Header
  uint32_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI1_1_5MHz_FDD DCI1_1_5MHz_FDD_t;
#define sizeof_DCI1_1_5MHz_FDD_t 23

/// DCI Format Type 1 (5 MHz, FDD, 27 bits)
struct DCI1_5MHz_FDD {
  /// dummy bits (not transmitted)
  uint32_t dummy:5;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits
  uint32_t rballoc:13;
  /// Resource Allocation Header
  uint32_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI1_5MHz_FDD DCI1_5MHz_FDD_t;
#define sizeof_DCI1_5MHz_FDD_t 27

/// DCI Format Type 1 (10 MHz, FDD, 31 bits)
struct DCI1_10MHz_FDD {
  /// dummy bits (not transmitted)
  uint32_t dummy:1;
  /// Power Control
  uint32_t TPC:2;
  /// Redundancy version
  uint32_t rv:2;
  /// New Data Indicator
  uint32_t ndi:1;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits
  uint32_t rballoc:17;
  /// Resource Allocation Header
  uint32_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI1_10MHz_FDD DCI1_10MHz_FDD_t;
#define sizeof_DCI1_10MHz_FDD_t 31

/// DCI Format Type 1 (20 MHz, FDD, 39 bits)
struct DCI1_20MHz_FDD {
  /// dummy bits (not transmitted)
  uint64_t dummy:25;
  /// Power Control
  uint64_t TPC:2;
  /// Redundancy version
  uint64_t rv:2;
  /// New Data Indicator
  uint64_t ndi:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Modulation and Coding Scheme and Redundancy Version
  uint64_t mcs:5;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI1_20MHz_FDD DCI1_20MHz_FDD_t;
#define sizeof_DCI1_20MHz_FDD_t 39

/// DCI Format Type 1A (5 MHz, TDD, frame 1-6, 27 bits)
struct DCI1A_RA_5MHz_TDD_1_6 {
  /// Dummy bits to align to 32-bits
  uint32_t dummy:5;
  /// Padding
  uint32_t padding:6;
  /// PRACH mask index
  uint32_t prach_mask_index:4;
  /// Preamble Index
  uint32_t preamble_index:6;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:9;
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
} __attribute__ ((__packed__));

typedef struct DCI1A_RA_5MHz_TDD_1_6 DCI1A_RA_5MHz_TDD_1_6_t;
#define sizeof_DCI1A_RA_5MHz_TDD_1_6_t 27

/// DCI Format Type 1A (5 MHz, TDD, frame 0, 27 bits)
/*
struct DCI1A_5MHz_TDD_0 {
  /// type = 0 => DCI Format 0, type = 1 => DCI Format 1A 
  uint32_t type:1;
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:9;
  union {
    RA_PDSCH_TDD_t ra_pdsch;
    PDSCH_TDD_t pdsch;
  } pdu;
} __attribute__ ((__packed__));

typedef struct DCI1A_5MHz_TDD_0 DCI1A_5MHz_TDD_0_t;
#define sizeof_DCI1A_5MHz_TDD_0_t 27
*/

/// DCI Format Type 1B (5 MHz, FDD, 2 Antenna Ports, 27 bits)
struct DCI1B_5MHz_2A_FDD {
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Redundancy version
  uint32_t rv:2;
  /// Power Control
  uint32_t TPC:2;
  /// TPMI information for precoding
  uint32_t tpmi:2;
  /// TMI confirmation for precoding
  uint32_t pmi:1;
  /// Padding to remove size ambiguity (26 bits -> 27 bits)
  uint32_t padding:1;
} __attribute__ ((__packed__));

typedef struct DCI1B_5MHz_2A_FDD DCI1B_5MHz_2A_FDD_t;
#define sizeof_DCI1B_5MHz_FDD_t 27

/// DCI Format Type 1B (5 MHz, TDD, 2 Antenna Ports, 29 bits)
struct DCI1B_5MHz_2A_TDD {
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Redundancy version
  uint32_t rv:2;
  /// Power Control
  uint32_t TPC:2;
  /// Downlink Assignment Index
  uint32_t dai:2;
  /// TPMI information for precoding
  uint32_t tpmi:2;
  /// TMI confirmation for precoding
  uint32_t pmi:1;
} __attribute__ ((__packed__));

typedef struct DCI1B_5MHz_2A_TDD DCI1B_5MHz_2A_TDD_t;
#define sizeof_DCI1B_5MHz_2A_TDD_t 29

/// DCI Format Type 1B (5 MHz, FDD, 4 Antenna Ports, 28 bits)
struct DCI1B_5MHz_4A_FDD {
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Redundancy version
  uint32_t rv:2;
  /// Power Control
  uint32_t TPC:2;
  /// TPMI information for precoding
  uint32_t tpmi:4;
  /// TMI confirmation for precoding
  uint32_t pmi:1;
} __attribute__ ((__packed__));

typedef struct DCI1B_5MHz_4A_FDD DCI1B_5MHz_4A_FDD_t;
#define sizeof_DCI1B_5MHz_4A_FDD_t 28

/// DCI Format Type 1B (5 MHz, TDD, 4 Antenna Ports, 31 bits)
struct DCI1B_5MHz_4A_TDD {
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Redundancy version
  uint32_t rv:2;
  /// Power Control
  uint32_t TPC:2;
  /// Downlink Assignment Index
  uint32_t dai:2;
  /// TPMI information for precoding
  uint32_t tpmi:4;
  /// TMI confirmation for precoding
  uint32_t pmi:1;
} __attribute__ ((__packed__));

typedef struct DCI1B_5MHz_4A_TDD DCI1B_5MHz_4A_TDD_t;
#define sizeof_DCI1B_5MHz_4A_TDD_t 31

/// DCI Format Type 1C (5 MHz, 12 bits)
typedef struct __attribute__ ((__packed__)){
  uint32_t rballoc:7;
  uint32_t tbs_index:5;
} DCI1C_5MHz_t;
#define sizeof_DCI1C_5MHz_t 12

/// DCI Format Type 1D (5 MHz, FDD, 2 Antenna Ports, 27 bits)
struct DCI1D_5MHz_2A_FDD {
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Redundancy version
  uint32_t rv:2;
  /// Power Control
  uint32_t TPC:2;
  /// TPMI information for precoding
  uint32_t tpmi:2;
  /// TMI confirmation for precoding
  uint32_t pmi:1;
  /// Downlink Power Offset
  uint32_t dl_power_off:1;
} __attribute__ ((__packed__));

typedef struct DCI1D_5MHz_2A_FDD DCI1D_5MHz_2A_FDD_t;
#define sizeof_DCI1D_5MHz_2A_FDD_t 27

/// DCI Format Type 1D (5 MHz, TDD, 2 Antenna Ports, 30 bits)
struct DCI1D_5MHz_2A_TDD {
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Redundancy version
  uint32_t rv:2;
  /// Power Control
  uint32_t TPC:2;
  /// Downlink Assignment Index
  uint32_t dai:2;
  /// TPMI information for precoding
  uint32_t tpmi:2;
  /// TMI confirmation for precoding
  uint32_t pmi:1;
  /// Downlink Power Offset
  uint32_t dl_power_off:1;
} __attribute__ ((__packed__));

typedef struct DCI1D_5MHz_2A_TDD DCI1D_5MHz_2A_TDD_t;
#define sizeof_DCI1D_5MHz_2A_TDD_t 30

/// DCI Format Type 1D (5 MHz, FDD, 4 Antenna Ports, 29 bits)
struct DCI1D_5MHz_4A_FDD {
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Redundancy version
  uint32_t rv:2;
  /// Power Control
  uint32_t TPC:2;
  /// TPMI information for precoding
  uint32_t tpmi:4;
  /// TMI confirmation for precoding
  uint32_t pmi:1;
  /// Downlink Power Offset
  uint32_t dl_power_off:1;
}  __attribute__ ((__packed__));

typedef struct DCI1D_5MHz_4A_FDD DCI1D_5MHz_4A_FDD_t;
#define sizeof_DCI1D_5MHz_4A_FDD_t 29

/// DCI Format Type 1D (5 MHz, TDD, 4 Antenna Ports, 33 bits)
struct DCI1D_5MHz_4A_TDD {
  /// Localized/Distributed VRB
  uint32_t vrb_type:1;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:9;
  /// Modulation and Coding Scheme and Redundancy Version
  uint32_t mcs:5;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// New Data Indicator
  uint32_t ndi:1;
  /// Redundancy version
  uint32_t rv:2;
  /// Power Control
  uint32_t TPC:2;
  /// Downlink Assignment Index
  uint32_t dai:2;
  /// TPMI information for precoding
  uint32_t tpmi:4;
  /// TMI confirmation for precoding
  uint32_t pmi:1;
  /// Downlink Power Offset
  uint32_t dl_power_off:1;
  /// Padding to remove size ambiguity (32 bits -> 33 bits)
  uint32_t padding:1;
} __attribute__ ((__packed__));

typedef struct DCI1D_5MHz_4A_TDD DCI1D_5MHz_4A_TDD_t;
#define sizeof_DCI1D_5MHz_4A_TDD_t 33


///******************NEW DCI Format for MU-MIMO****************///////////

/// DCI Format Type 1E (5 MHz, TDD, 2 Antenna Ports, more than 10 PRBs, 34 bits)
struct DCI1E_5MHz_2A_M10PRB_TDD {
  /// padding to 64bits
  uint64_t padding64:30;
  /// Redundancy version 2
  ///uint64_t rv2:2;
  /// New Data Indicator 2
  ///uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  ///uint64_t mcs2:5;
  /// TPMI information for precoding
  uint64_t tpmi:3;
  /// Redundancy version 1
  uint64_t rv:2;
  /// New Data Indicator 1
  uint64_t ndi:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs:5;
  /// TB swap
  ///uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
  /// Downlink Power offset for MU-MIMO
  uint64_t dl_power_off:1;
} __attribute__ ((__packed__));
typedef struct DCI1E_5MHz_2A_M10PRB_TDD DCI1E_5MHz_2A_M10PRB_TDD_t;
#define sizeof_DCI1E_5MHz_2A_M10PRB_TDD_t 34

// **********************************************************
// ********************FORMAT 2 DCIs ************************
// **********************************************************
/// DCI Format Type 2 (1.5 MHz, TDD, 2 Antenna Ports, 34 bits)
struct DCI2_1_5MHz_2A_TDD {
  /// padding to 64bits
  uint64_t padding64:30;
  /// TPMI information for precoding
  uint64_t tpmi:3;  
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:6;
} __attribute__ ((__packed__));

typedef struct DCI2_1_5MHz_2A_TDD DCI2_1_5MHz_2A_TDD_t;
#define sizeof_DCI2_1_5MHz_2A_TDD_t 34

/// DCI Format Type 2 (1.5 MHz, TDD, 4 Antenna Ports, 37 bits)
struct DCI2_1_5MHz_4A_TDD {
  /// padding for 64-bit
  uint64_t padding64:27;
  /// TPMI information for precoding
  uint64_t tpmi:6;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;  
/// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:6;
}  __attribute__ ((__packed__));

typedef struct DCI2_1_5MHz_4A_TDD DCI2_1_5MHz_4A_TDD_t;
#define sizeof_DCI2_1_5MHz_4A_TDD_t 37

/// DCI Format Type 2 (5 MHz, TDD, 2 Antenna Ports, 42 bits)
struct DCI2_5MHz_2A_TDD {
  /// padding to 64bits
  uint64_t padding64:22;
  /// TPMI information for precoding
  uint64_t tpmi:3;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2_5MHz_2A_TDD DCI2_5MHz_2A_TDD_t;
#define sizeof_DCI2_5MHz_2A_TDD_t 42

/// DCI Format Type 2 (5 MHz, TDD, 4 Antenna Ports, 45 bits)
struct DCI2_5MHz_4A_TDD {
  /// padding for 64-bit
  uint64_t padding64:19;
  /// TPMI information for precoding
  uint64_t tpmi:6;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2_5MHz_4A_TDD DCI2_5MHz_4A_TDD_t;
#define sizeof_DCI2_5MHz_4A_TDD_t 45

/// DCI Format Type 2 (10 MHz, TDD, 2 Antenna Ports, 46 bits)
struct DCI2_10MHz_2A_TDD {
  /// padding to 64bits
  uint64_t padding64:18;
  /// TPMI information for precoding
  uint64_t tpmi:3;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2_10MHz_2A_TDD DCI2_10MHz_2A_TDD_t;
#define sizeof_DCI2_10MHz_2A_TDD_t 46

/// DCI Format Type 2 (10 MHz, TDD, 4 Antenna Ports, 49 bits)
struct DCI2_10MHz_4A_TDD {
  /// padding for 64-bit
  uint64_t padding64:15;
  /// TPMI information for precoding
  uint64_t tpmi:6;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2_10MHz_4A_TDD DCI2_10MHz_4A_TDD_t;
#define sizeof_DCI2_10MHz_4A_TDD_t 49

/// DCI Format Type 2 (20 MHz, TDD, 2 Antenna Ports, 54 bits)
struct DCI2_20MHz_2A_TDD {
  /// padding to 64bits
  uint64_t padding64:10;
  /// TPMI information for precoding
  uint64_t tpmi:3;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2_20MHz_2A_TDD DCI2_20MHz_2A_TDD_t;
#define sizeof_DCI2_20MHz_2A_TDD_t 54

/// DCI Format Type 2 (20 MHz, TDD, 4 Antenna Ports, 57 bits)
struct DCI2_20MHz_4A_TDD {
  /// padding for 64-bit
  uint64_t padding64:7;
  /// TPMI information for precoding
  uint64_t tpmi:6;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2_20MHz_4A_TDD DCI2_20MHz_4A_TDD_t;
#define sizeof_DCI2_20MHz_4A_TDD_t 58

/// DCI Format Type 2 (1.5 MHz, FDD, 2 Antenna Ports, 31 bits)
struct DCI2_1_5MHz_2A_FDD {
  //padding for 32 bits
  uint32_t padding32:1;
  /// precoding bits
  uint32_t tpmi:3;
  /// Redundancy version 2
  uint32_t rv2:2;
  /// New Data Indicator 2
  uint32_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint32_t mcs2:5;
  /// Redundancy version 1
  uint32_t rv1:2;
  /// New Data Indicator 1
  uint32_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint32_t mcs1:5;
  /// TB swap
  uint32_t tb_swap:1;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Power Control
  uint32_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:6;
} __attribute__ ((__packed__));

typedef struct DCI2_1_5MHz_2A_FDD DCI2_1_5MHz_2A_FDD_t;
#define sizeof_DCI2_1_5MHz_2A_FDD_t 31

/// DCI Format Type 2 (1.5 MHz, FDD, 4 Antenna Ports, 34 bits)
struct DCI2_1_5MHz_4A_FDD {
  /// padding for 32 bits
  uint64_t padding64:30;
  /// precoding bits
  uint64_t tpmi:6;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:6;
} __attribute__ ((__packed__));

typedef struct DCI2_1_5MHz_4A_FDD DCI2_1_5MHz_4A_FDD_t;
#define sizeof_DCI2_1_5MHz_4A_FDD_t 34

/// DCI Format Type 2 (5 MHz, FDD, 2 Antenna Ports, 39 bits)
struct DCI2_5MHz_2A_FDD{
  /// padding for 64-bit
  uint64_t padding64:25;
  /// TPMI information for precoding
  uint64_t tpmi:3;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2_5MHz_2A_FDD_t 39
typedef struct DCI2_5MHz_2A_FDD DCI2_5MHz_2A_FDD_t;

/// DCI Format Type 2 (5 MHz, TDD, 4 Antenna Ports, 42 bits)
struct DCI2_5MHz_4A_FDD {
  /// padding for 64-bit
  uint64_t padding64:21;
  /// TPMI information for precoding
  uint64_t tpmi:6;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2_5MHz_4A_FDD_t 42
typedef struct DCI2_5MHz_4A_FDD DCI2_5MHz_4A_FDD_t;

/// DCI Format Type 2 (10 MHz, FDD, 2 Antenna Ports, 43 bits)
struct DCI2_10MHz_2A_FDD {
  /// padding for 64-bit
  uint64_t padding64:21;
  /// TPMI information for precoding
  uint64_t tpmi:3;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2_10MHz_2A_FDD_t 43
typedef struct DCI2_10MHz_2A_FDD DCI2_10MHz_2A_FDD_t;

/// DCI Format Type 2 (5 MHz, TDD, 4 Antenna Ports, 46 bits)
struct DCI2_10MHz_4A_FDD {
  /// padding for 64-bit
  uint64_t padding64:18;
  /// TPMI information for precoding
  uint64_t tpmi:6;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2_10MHz_4A_FDD_t 46
typedef struct DCI2_10MHz_4A_FDD DCI2_10MHz_4A_FDD_t;

/// DCI Format Type 2 (20 MHz, FDD, 2 Antenna Ports, 51 bits)
struct DCI2_20MHz_2A_FDD {
  /// padding for 64-bit
  uint64_t padding64:13;
  /// TPMI information for precoding
  uint64_t tpmi:3;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2_20MHz_2A_FDD_t 51
typedef struct DCI2_20MHz_2A_FDD DCI2_20MHz_2A_FDD_t;

/// DCI Format Type 2 (20 MHz, FDD, 4 Antenna Ports, 54 bits)
struct DCI2_20MHz_4A_FDD {
  /// padding for 64-bit
  uint64_t padding64:10;
  /// TPMI information for precoding
  uint64_t tpmi:6;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
}  __attribute__ ((__packed__));
#define sizeof_DCI2_20MHz_4A_FDD_t 54
typedef struct DCI2_20MHz_4A_FDD DCI2_20MHz_4A_FDD_t;




// *******************************************************************
// ********************FORMAT 2A DCIs*********************************
// *******************************************************************

/// DCI Format Type 2A (1.5 MHz, TDD, 2 Antenna Ports, 32 bits)
struct DCI2A_1_5MHz_2A_TDD {
  /// Redundancy version 2
  uint32_t rv2:2;
  /// New Data Indicator 2
  uint32_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint32_t mcs2:5;
  /// Redundancy version 1
  uint32_t rv1:2;
  /// New Data Indicator 1
  uint32_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint32_t mcs1:5;
  /// TB swap
  uint32_t tb_swap:1;
  /// HARQ Process
  uint32_t harq_pid:4;
  /// Downlink Assignment Index
  uint32_t dai:2;
  /// Power Control
  uint32_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:6;
  /// Resource Allocation Header
  uint32_t rah:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2A_1_5MHz_2A_TDD_t 32
typedef struct DCI2A_1_5MHz_2A_TDD DCI2A_1_5MHz_2A_TDD_t;

/// DCI Format Type 2A (1.5 MHz, TDD, 4 Antenna Ports, 34 bits)
struct DCI2A_1_5MHz_4A_TDD {
  uint64_t padding:30;
  /// TPMI information for precoding
  uint64_t tpmi:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:6;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_1_5MHz_4A_TDD_t 34
typedef struct DCI2A_1_5MHz_4A_TDD DCI2A_1_5MHz_4A_TDD_t;

/// DCI Format Type 2A (1.5 MHz, FDD, 2 Antenna Ports, 29 bits)
struct DCI2A_1_5MHz_2A_FDD {
  uint32_t padding:18;
  /// Redundancy version 2
  uint32_t rv2:2;
  /// New Data Indicator 2
  uint32_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint32_t mcs2:5;
  /// Redundancy version 1
  uint32_t rv1:2;
  /// New Data Indicator 1
  uint32_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint32_t mcs1:5;
  /// TB swap
  uint32_t tb_swap:1;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Power Control
  uint32_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:6;
  /// Resource Allocation Header
  uint32_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_1_5MHz_2A_FDD_t 29
typedef struct DCI2A_1_5MHz_2A_FDD DCI2A_1_5MHz_2A_FDD_t;

/// DCI Format Type 2A (1.5 MHz, FDD, 4 Antenna Ports, 31 bits)
struct DCI2A_1_5MHz_4A_FDD{
  uint32_t padding:1;
  /// TPMI information for precoding
  uint32_t tpmi:2;
  /// Redundancy version 2
  uint32_t rv2:2;
  /// New Data Indicator 2
  uint32_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint32_t mcs2:5;
  /// Redundancy version 1
  uint32_t rv1:2;
  /// New Data Indicator 1
  uint32_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint32_t mcs1:5;
  /// TB swap
  uint32_t tb_swap:1;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Power Control
  uint32_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:6;
  /// Resource Allocation Header
  uint32_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_1_5MHz_4A_FDD_t 38
typedef struct DCI2A_1_5MHz_4A_FDD DCI2A_1_5MHz_4A_FDD_t;

/// DCI Format Type 2A (5 MHz, TDD, 2 Antenna Ports, 39 bits)
struct DCI2A_5MHz_2A_TDD {
  uint64_t padding:25;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_5MHz_2A_TDD_t 39
typedef struct DCI2A_5MHz_2A_TDD DCI2A_5MHz_2A_TDD_t;

/// DCI Format Type 2A (5 MHz, TDD, 4 Antenna Ports, 41 bits)
struct DCI2A_5MHz_4A_TDD {
  uint64_t padding:23;
  /// TPMI information for precoding
  uint64_t tpmi:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_5MHz_4A_TDD_t 41
typedef struct DCI2A_5MHz_4A_TDD DCI2A_5MHz_4A_TDD_t;

/// DCI Format Type 2A (5 MHz, FDD, 2 Antenna Ports, 36 bits)
struct DCI2A_5MHz_2A_FDD {
  uint64_t padding:28;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_5MHz_2A_FDD_t 36
typedef struct DCI2A_5MHz_2A_FDD DCI2A_5MHz_2A_FDD_t;

/// DCI Format Type 2A (5 MHz, FDD, 4 Antenna Ports, 38 bits)
struct DCI2A_5MHz_4A_FDD {
  uint64_t padding:26;
  /// TPMI information for precoding
  uint64_t tpmi:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_5MHz_4A_FDD_t 38
typedef struct DCI2A_5MHz_4A_FDD DCI2A_5MHz_4A_FDD_t;

/// DCI Format Type 2A (10 MHz, TDD, 2 Antenna Ports, 39 bits)
struct DCI2A_10MHz_2A_TDD {
  uint64_t padding:25;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_10MHz_2A_TDD_t 39
typedef struct DCI2A_10MHz_2A_TDD DCI2A_10MHz_2A_TDD_t;

/// DCI Format Type 2A (10 MHz, TDD, 4 Antenna Ports, 41 bits)
struct DCI2A_10MHz_4A_TDD{
  uint64_t padding:23;
  /// TPMI information for precoding
  uint64_t tpmi:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_10MHz_4A_TDD_t 41
typedef struct DCI2A_10MHz_4A_TDD DCI2A_10MHz_4A_TDD_t;

/// DCI Format Type 2A (10 MHz, FDD, 2 Antenna Ports, 36 bits)
struct DCI2A_10MHz_2A_FDD {
  uint64_t padding:28;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_10MHz_2A_FDD_t 36
typedef struct DCI2A_10MHz_2A_FDD DCI2A_10MHz_2A_FDD_t;

/// DCI Format Type 2A (10 MHz, FDD, 4 Antenna Ports, 38 bits)
struct DCI2A_10MHz_4A_FDD{
  uint64_t padding:26;
  /// TPMI information for precoding
  uint64_t tpmi:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_10MHz_4A_FDD_t 38
typedef struct DCI2A_10MHz_4A_FDD DCI2A_10MHz_4A_FDD_t;

/// DCI Format Type 2A (20 MHz, TDD, 2 Antenna Ports, 51 bits)
struct DCI2A_20MHz_2A_TDD{
  uint64_t padding:13;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_20MHz_2A_TDD_t 51
typedef struct DCI2A_20MHz_2A_TDD DCI2A_20MHz_2A_TDD_t;

/// DCI Format Type 2A (20 MHz, TDD, 4 Antenna Ports, 53 bits)
struct DCI2A_20MHz_4A_TDD{
  uint64_t padding:11;
  /// TPMI information for precoding
  uint64_t tpmi:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_20MHz_4A_TDD_t 53
typedef struct DCI2A_20MHz_4A_TDD DCI2A_20MHz_4A_TDD_t;

/// DCI Format Type 2A (20 MHz, FDD, 2 Antenna Ports, 48 bits)
struct DCI2A_20MHz_2A_FDD {
  uint64_t padding:16;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_20MHz_2A_FDD_t 48
typedef struct DCI2A_20MHz_2A_FDD DCI2A_20MHz_2A_FDD_t;

/// DCI Format Type 2A (20 MHz, FDD, 4 Antenna Ports, 50 bits)
struct DCI2A_20MHz_4A_FDD {
  uint64_t padding:14;
  /// TPMI information for precoding
  uint64_t tpmi:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// TB swap
  uint64_t tb_swap:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));
#define sizeof_DCI2A_20MHz_4A_FDD_t 50
typedef struct DCI2A_20MHz_4A_FDD DCI2A_20MHz_4A_FDD_t;

// *******************************************************************
// ********************FORMAT 2B DCIs*********************************
// *******************************************************************
/// DCI Format Type 2B (1.5 MHz, TDD,  33 bits)
struct DCI2B_1_5MHz_TDD {
  uint64_t padding64:31;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t scrambling_id:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:6;
  /// Padding for ambiguity
  uint64_t padding:1;
} __attribute__ ((__packed__));

typedef struct DCI2B_1_5MHz_TDD DCI2B_1_5MHz_TDD_t;
#define sizeof_DCI2B_1_5MHz_TDD_t 33

/// DCI Format Type 2B (5 MHz, TDD,  39 bits)
struct DCI2B_5MHz_TDD {
  /// padding to 64bits
  uint64_t padding64:25;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t scrambling_id:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2B_5MHz_TDD DCI2B_5MHz_TDD_t;
#define sizeof_DCI2B_5MHz_TDD_t 39

/// DCI Format Type 2B (10 MHz, TDD,  43 bits)
struct DCI2B_10MHz_TDD {
  /// padding to 64bits
  uint64_t padding64:21;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t scrambling_id:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2B_10MHz_TDD DCI2B_10MHz_TDD_t;
#define sizeof_DCI2B_10MHz_TDD_t 43

/// DCI Format Type 2B (20 MHz, TDD,  51 bits)
struct DCI2B_20MHz_TDD {
  /// padding to 64bits
  uint64_t padding64:13;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t scrambling_id:1;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2B_20MHz_TDD DCI2B_20MHz_TDD_t;
#define sizeof_DCI2B_20MHz_TDD_t 51

/// DCI Format Type 2B (1.5 MHz, FDD,  28 bits)
struct DCI2B_1_5MHz_FDD {
  //padding for 32 bits
  uint32_t padding32:4;
  /// Redundancy version 2
  uint32_t rv2:2;
  /// New Data Indicator 2
  uint32_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint32_t mcs2:5;
  /// Redundancy version 1
  uint32_t rv1:2;
  /// New Data Indicator 1
  uint32_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint32_t mcs1:5;
  /// Scrambling ID
  uint32_t scrambling_id:1;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Power Control
  uint32_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:6;
} __attribute__ ((__packed__));

typedef struct DCI2B_1_5MHz_FDD DCI2B_1_5MHz_FDD_t;
#define sizeof_DCI2B_1_5MHz_FDD_t 28

/// DCI Format Type 2B (5 MHz, FDD,  36 bits)
struct DCI2B_5MHz_FDD {
  /// padding for 64-bit
  uint64_t padding64:28;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// Scrambling ID
  uint64_t scrambling_id:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2B_5MHz_FDD_t 36
typedef struct DCI2B_5MHz_FDD DCI2B_5MHz_FDD_t;

/// DCI Format Type 2B (10 MHz, FDD,  41 bits)
struct DCI2B_10MHz_FDD {
  /// padding for 64-bit
  uint64_t padding64:23;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// Scrambling ID
  uint64_t scrambling_id:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
  /// Padding for ambiguity
  uint64_t padding:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2B_10MHz_FDD_t 41
typedef struct DCI2B_10MHz_FDD DCI2B_10MHz_FDD_t;

/// DCI Format Type 2B (20 MHz, FDD,  48 bits)
struct DCI2B_20MHz_FDD {
  /// padding for 64-bit
  uint64_t padding64:16;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// Scrambling ID
  uint64_t scrambling_id:1;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2B_20MHz_FDD_t 48
typedef struct DCI2B_20MHz_FDD DCI2B_20MHz_FDD_t;

// *******************************************************************
// ********************FORMAT 2C DCIs*********************************
// *******************************************************************

/// DCI Format Type 2C (1.5 MHz, TDD,  34 bits)
struct DCI2C_1_5MHz_TDD {
  uint64_t padding64:30;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:6;
} __attribute__ ((__packed__));

typedef struct DCI2C_1_5MHz_TDD DCI2C_1_5MHz_TDD_t;
#define sizeof_DCI2C_1_5MHz_TDD_t 34

/// DCI Format Type 2C (5 MHz, TDD,  41 bits)
struct DCI2C_5MHz_TDD {
  /// padding to 64bits
  uint64_t padding64:23;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2C_5MHz_TDD DCI2C_5MHz_TDD_t;
#define sizeof_DCI2C_5MHz_TDD_t 41

/// DCI Format Type 2C (10 MHz, TDD,  45 bits)
struct DCI2C_10MHz_TDD {
  /// padding to 64bits
  uint64_t padding64:19;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2C_10MHz_TDD DCI2C_10MHz_TDD_t;
#define sizeof_DCI2C_10MHz_TDD_t 45

/// DCI Format Type 2C (20 MHz, TDD,  53 bits)
struct DCI2C_20MHz_TDD {
  /// padding to 64bits
  uint64_t padding64:11;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2C_20MHz_TDD DCI2C_20MHz_TDD_t;
#define sizeof_DCI2C_20MHz_TDD_t 53

/// DCI Format Type 2C (1.5 MHz, FDD,  30 bits)
struct DCI2C_1_5MHz_FDD {
  //padding for 32 bits
  uint32_t padding32:2;
  /// Redundancy version 2
  uint32_t rv2:2;
  /// New Data Indicator 2
  uint32_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint32_t mcs2:5;
  /// Redundancy version 1
  uint32_t rv1:2;
  /// New Data Indicator 1
  uint32_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint32_t mcs1:5;
  /// Scrambling ID
  uint32_t ap_si_nl_id:3;
  /// HARQ Process
  uint32_t harq_pid:3;
  /// Power Control
  uint32_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint32_t rballoc:6;
} __attribute__ ((__packed__));

typedef struct DCI2C_1_5MHz_FDD DCI2C_1_5MHz_FDD_t;
#define sizeof_DCI2C_1_5MHz_FDD_t 30

/// DCI Format Type 2C (5 MHz, FDD,  38 bits)
struct DCI2C_5MHz_FDD {
  /// padding for 64-bit
  uint64_t padding64:26;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2C_5MHz_FDD_t 38
typedef struct DCI2C_5MHz_FDD DCI2C_5MHz_FDD_t;

/// DCI Format Type 2C (10 MHz, FDD,  42 bits)
struct DCI2C_10MHz_FDD {
  /// padding for 64-bit
  uint64_t padding64:22;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2C_10MHz_FDD_t 43
typedef struct DCI2C_10MHz_FDD DCI2C_10MHz_FDD_t;

/// DCI Format Type 2C (20 MHz, FDD,  50 bits)
struct DCI2C_20MHz_FDD {
  /// padding for 64-bit
  uint64_t padding64:14;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2C_20MHz_FDD_t 50
typedef struct DCI2C_20MHz_FDD DCI2C_20MHz_FDD_t;

// *******************************************************************
// ********************FORMAT 2D DCIs*********************************
// *******************************************************************

/// DCI Format Type 2D (1.5 MHz, TDD,  36 bits)
struct DCI2D_1_5MHz_TDD {
  uint64_t padding64:28;
  /// PDSCH REsource Mapping and Quasi-Co-Location Indicator
  uint64_t REMQCL:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:6;
} __attribute__ ((__packed__));

typedef struct DCI2D_1_5MHz_TDD DCI2D_1_5MHz_TDD_t;
#define sizeof_DCI2D_1_5MHz_TDD_t 36

/// DCI Format Type 2D (5 MHz, TDD,  43 bits)
struct DCI2D_5MHz_TDD {
  /// padding to 64bits
  uint64_t padding64:21;
  /// PDSCH REsource Mapping and Quasi-Co-Location Indicator
  uint64_t REMQCL:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2D_5MHz_TDD DCI2D_5MHz_TDD_t;
#define sizeof_DCI2D_5MHz_TDD_t 43

/// DCI Format Type 2D (10 MHz, TDD,  47 bits)
struct DCI2D_10MHz_TDD {
  /// padding to 64bits
  uint64_t padding64:17;
  /// PDSCH REsource Mapping and Quasi-Co-Location Indicator
  uint64_t REMQCL:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2D_10MHz_TDD DCI2D_10MHz_TDD_t;
#define sizeof_DCI2D_10MHz_TDD_t 47

/// DCI Format Type 2D (20 MHz, TDD,  55 bits)
struct DCI2D_20MHz_TDD {
  /// padding to 64bits
  uint64_t padding64:9;
  /// PDSCH REsource Mapping and Quasi-Co-Location Indicator
  uint64_t REMQCL:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// SRS Request
  uint64_t srs_req:1;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:4;
  /// Downlink Assignment Index
  uint64_t dai:2;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

typedef struct DCI2D_20MHz_TDD DCI2D_20MHz_TDD_t;
#define sizeof_DCI2D_20MHz_TDD_t 55

/// DCI Format Type 2D (1.5 MHz, FDD,  33 bits)
struct DCI2D_1_5MHz_FDD {
  //padding for 33 bits
  uint64_t padding64:31;
  /// PDSCH REsource Mapping and Quasi-Co-Location Indicator
  uint64_t REMQCL:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:6;
  /// padding for ambiguity
  uint64_t padding;
} __attribute__ ((__packed__));

typedef struct DCI2D_1_5MHz_FDD DCI2D_1_5MHz_FDD_t;
#define sizeof_DCI2D_1_5MHz_FDD_t 33

/// DCI Format Type 2D (5 MHz, FDD,  41 bits)
struct DCI2D_5MHz_FDD {
  /// padding for 64-bit
  uint64_t padding64:23;
  /// PDSCH REsource Mapping and Quasi-Co-Location Indicator
  uint64_t REMQCL:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:13;
  /// Resource Allocation Header
  uint64_t rah:1;
  /// padding for ambiguity
  uint64_t padding:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2D_5MHz_FDD_t 41
typedef struct DCI2D_5MHz_FDD DCI2D_5MHz_FDD_t;

/// DCI Format Type 2D (10 MHz, FDD,  45 bits)
struct DCI2D_10MHz_FDD {
  /// padding for 64-bit
  uint64_t padding64:19;
  /// PDSCH REsource Mapping and Quasi-Co-Location Indicator
  uint64_t REMQCL:2;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:17;
  /// Resource Allocation Header
  uint64_t rah:1;
  /// padding for ambiguity
  uint64_t padding:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2D_10MHz_FDD_t 45
typedef struct DCI2D_10MHz_FDD DCI2D_10MHz_FDD_t;

/// DCI Format Type 2D (20 MHz, FDD,  52 bits)
struct DCI2D_20MHz_FDD {
  /// padding for 64-bit
  uint64_t padding64:12;
  /// Redundancy version 2
  uint64_t rv2:2;
  /// New Data Indicator 2
  uint64_t ndi2:1;
  /// Modulation and Coding Scheme and Redundancy Version 2
  uint64_t mcs2:5;
  /// Redundancy version 1
  uint64_t rv1:2;
  /// New Data Indicator 1
  uint64_t ndi1:1;
  /// Modulation and Coding Scheme and Redundancy Version 1
  uint64_t mcs1:5;
  /// Scrambling ID
  uint64_t ap_si_nl_id:3;
  /// HARQ Process
  uint64_t harq_pid:3;
  /// Power Control
  uint64_t TPC:2;
  /// RB Assignment (ceil(log2(N_RB_DL/P)) bits)
  uint64_t rballoc:25;
  /// Resource Allocation Header
  uint64_t rah:1;
} __attribute__ ((__packed__));

#define sizeof_DCI2D_20MHz_FDD_t 52
typedef struct DCI2D_20MHz_FDD DCI2D_20MHz_FDD_t;


typedef struct __attribute__ ((__packed__)){
  uint32_t TPC:28;
} DCI3_5MHz_TDD_0_t;
#define sizeof_DCI3_5MHz_TDD_0_t 27

typedef struct __attribute__ ((__packed__)){
  uint32_t TPC:28;
} DCI3_5MHz_TDD_1_6_t;
#define sizeof_DCI3_5MHz_TDD_1_6_t 27


typedef struct __attribute__ ((__packed__)){
  uint32_t TPC:26;
} DCI3_5MHz_FDD_t;
#define sizeof_DCI3_5MHz_FDD_t 25

///  DCI Format Type 0 (1.5 MHz,9 bits)
struct DCI0A_1_5MHz {
  /// Padding
  uint32_t padding:19;
    /// Cyclic shift
  uint32_t cshift:3;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:5;
  /// Hopping flag
  uint32_t hopping:1;
} __attribute__ ((__packed__));
#define sizeof_DCI0A_1_5MHz 9

///  DCI Format Type 0 (5 MHz,13 bits)
struct DCI0A_5MHz {
  /// Padding
  uint32_t padding:19;
    /// Cyclic shift
  uint32_t cshift:3;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:9;
  /// Hopping flag
  uint32_t hopping:1;
} __attribute__ ((__packed__));
#define sizeof_DCI0A_5MHz 13

///  DCI Format Type 0 (10 MHz,15 bits)
struct DCI0A_10_MHz {
  /// Padding
  uint32_t padding:17;
    /// Cyclic shift
  uint32_t cshift:3;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:11;
  /// Hopping flag
  uint32_t hopping:1;
} __attribute__ ((__packed__));
#define sizeof_DCI0A_10MHz 15

///  DCI Format Type 0 (20 MHz,17 bits)
struct DCI0A_20_MHz {
  /// Padding
  uint32_t padding:15;
    /// Cyclic shift
  uint32_t cshift:3;
  /// RB Assignment (ceil(log2(N_RB_UL*(N_RB_UL+1)/2)) bits)
  uint32_t rballoc:13;
  /// Hopping flag
  uint32_t hopping:1;
} __attribute__ ((__packed__));
#define sizeof_DCI0A_20MHz 17

#define MAX_DCI_SIZE_BITS 45
