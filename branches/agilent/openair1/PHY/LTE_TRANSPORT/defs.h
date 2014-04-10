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

/*! \file PHY/LTE_TRANSPORT/defs.h
* \brief data structures for PDSCH/DLSCH/PUSCH/ULSCH physical and transport channel descriptors (TX/RX)
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: raymond.knopp@eurecom.fr, florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it
* \note
* \warning
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



#define NSOFT 1827072
#define LTE_NULL 2

// maximum of 3 segments before each coding block if data length exceeds 6144 bits.

#define MAX_NUM_DLSCH_SEGMENTS 16
#define MAX_NUM_ULSCH_SEGMENTS MAX_NUM_DLSCH_SEGMENTS
#define MAX_DLSCH_PAYLOAD_BYTES (MAX_NUM_DLSCH_SEGMENTS*768)
#define MAX_ULSCH_PAYLOAD_BYTES (MAX_NUM_ULSCH_SEGMENTS*768)

#define MAX_NUM_CHANNEL_BITS (14*1200*6)  // 14 symbols, 1200 REs, 12 bits/RE
#define MAX_NUM_RE (14*1200)

#define SI_RNTI 0xffff
#define P_RNTI  0xfffd
#define CBA_RNTI 0xfff4
#define C_RNTI  0x1234

#define PMI_2A_11 0
#define PMI_2A_1m1 1
#define PMI_2A_1j 2
#define PMI_2A_1mj 3

typedef enum {
  SCH_IDLE,
  ACTIVE,
  CBA_ACTIVE,
  DISABLED
} SCH_status_t;


typedef struct {
  /// Flag indicating that this DLSCH is active (i.e. not the first round)
  uint8_t Ndi;
  /// Status Flag indicating for this DLSCH (idle,active,disabled)
  SCH_status_t status;
  /// Transport block size
  uint32_t TBS;
  /// The payload + CRC size in bits, "B" from 36-212
  uint32_t B;
  /// Pointer to the payload
  uint8_t *b;
  /// Pointers to transport block segments
  uint8_t *c[MAX_NUM_DLSCH_SEGMENTS];
  /// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)
  uint32_t RTC[MAX_NUM_DLSCH_SEGMENTS];
  /// Index of current HARQ round for this DLSCH
  uint8_t round;
  /// MCS format for this DLSCH
  uint8_t mcs;
  /// Redundancy-version of the current sub-frame
  uint8_t rvidx;
  /// MIMO mode for this DLSCH
  MIMO_mode_t mimo_mode;
  /// Turbo-code outputs (36-212 V8.6 2009-03, p.12
  uint8_t d[MAX_NUM_DLSCH_SEGMENTS][(96+3+(3*6144))];
  /// Sub-block interleaver outputs (36-212 V8.6 2009-03, p.16-17)
  uint8_t w[MAX_NUM_DLSCH_SEGMENTS][3*6144];
  /// Number of code segments (for definition see 36-212 V8.6 2009-03, p.9)
  uint32_t C;
  /// Number of "small" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Cminus;
  /// Number of "large" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Cplus;
  /// Number of bits in "small" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Kminus;
  /// Number of bits in "large" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Kplus;
  /// Number of "Filler" bits (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t F;
  /// Number of MIMO layers (streams) (for definition see 36-212 V8.6 2009-03, p.17)
  uint8_t Nl;
} LTE_DL_eNB_HARQ_t;

typedef struct {
  /// Flag indicating that this ULSCH has a new packet (start of new round)
  uint8_t Ndi;
  /// Status Flag indicating for this ULSCH (idle,active,disabled)
  SCH_status_t status;
  /// Subframe scheduling indicator (i.e. Transmission opportunity indicator)
  uint8_t subframe_scheduling_flag;
  /// Subframe cba scheduling indicator (i.e. Transmission opportunity indicator)
  uint8_t subframe_cba_scheduling_flag;
  /// First Allocated RB
  uint16_t first_rb;
  /// Current Number of RBs
  uint16_t nb_rb;
  /// Last TPC command
  uint8_t TPC;
  /// Transport block size
  uint32_t TBS;
  /// The payload + CRC size in bits, "B" from 36-212
  uint32_t B;
  /// Length of ACK information (bits)
  uint8_t O_ACK;
  /// Pointer to the payload
  uint8_t *b;
  /// Pointers to transport block segments
  uint8_t *c[MAX_NUM_ULSCH_SEGMENTS];
  /// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)
  uint32_t RTC[MAX_NUM_ULSCH_SEGMENTS];
  /// Index of current HARQ round for this ULSCH
  uint8_t round;
  /// MCS format of this ULSCH
  uint8_t mcs;
  /// Redundancy-version of the current sub-frame
  uint8_t rvidx;
  /// Turbo-code outputs (36-212 V8.6 2009-03, p.12
  uint8_t d[MAX_NUM_ULSCH_SEGMENTS][(96+3+(3*6144))];
  /// Sub-block interleaver outputs (36-212 V8.6 2009-03, p.16-17)
  uint8_t w[MAX_NUM_ULSCH_SEGMENTS][3*6144];
  /// Number of code segments (for definition see 36-212 V8.6 2009-03, p.9)
  uint32_t C;
  /// Number of "small" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Cminus;
  /// Number of "large" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Cplus;
  /// Number of bits in "small" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Kminus;
  /// Number of bits in "large" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Kplus;
  /// Total number of bits across all segments
  uint32_t sumKr;
  /// Number of "Filler" bits (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t F;
  /// Msc_initial, Initial number of subcarriers for ULSCH (36-212, v8.6 2009-03, p.26-27)
  uint16_t Msc_initial;
  /// Nsymb_initial, Initial number of symbols for ULSCH (36-212, v8.6 2009-03, p.26-27)
  uint8_t Nsymb_initial;
  /// n_DMRS  for cyclic shift of DMRS (36.213 Table 9.1.2-2)
  uint8_t n_DMRS;
  /// n_DMRS2 for cyclic shift of DMRS (36.211 Table 5.5.1.1.-1)
  uint8_t n_DMRS2;
  /// Flag to indicate that this is a control only ULSCH (i.e. no MAC SDU)
  uint8_t control_only;
  /// Flag to indicate that this is a calibration ULSCH (i.e. no MAC SDU and filled with TDD calibration information)
  //  int calibration_flag;
} LTE_UL_UE_HARQ_t;

typedef struct {
  /// Allocated RNTI (0 means DLSCH_t is not currently used)
  uint16_t rnti;
  /// Active flag for baseband transmitter processing
  uint8_t active;
  /// Indicator of TX activation per subframe.  Used during PUCCH detection for ACK/NAK.
  uint8_t subframe_tx[10];
  /// First CCE of last PDSCH scheduling per subframe.  Again used during PUCCH detection for ACK/NAK.
  uint8_t nCCE[10];
  /// Current HARQ process id
  uint8_t current_harq_pid;
  /// Process ID's per subframe.  Used to associate received ACKs on PUSCH/PUCCH to DLSCH harq process ids
  uint8_t harq_ids[10];
  /// Window size (in outgoing transport blocks) for fine-grain rate adaptation
  uint8_t ra_window_size;
  /// First-round error threshold for fine-grain rate adaptation
  uint8_t error_threshold;
  /// Current RB allocation
  uint32_t rb_alloc[4];
  /// Current subband PMI allocation
  uint16_t pmi_alloc;
  /// Current subband RI allocation
  uint32_t ri_alloc;
  /// Current subband CQI1 allocation
  uint32_t cqi_alloc1;
  /// Current subband CQI2 allocation
  uint32_t cqi_alloc2;
  /// Current Number of RBs
  uint16_t nb_rb;
  /// Pointers to 8 HARQ processes for the DLSCH
  LTE_DL_eNB_HARQ_t *harq_processes[8];
  /// Number of soft channel bits
  uint32_t G;
  /// Layer index for this dlsch (0,1)
  uint8_t layer_index;
  /// Codebook index for this dlsch (0,1,2,3)
  uint8_t codebook_index;
  /// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18)
  uint8_t e[MAX_NUM_CHANNEL_BITS];
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)
  uint8_t Mdlharq;
  /// MIMO transmission mode indicator for this sub-frame (for definition see 36-212 V8.6 2009-03, p.17)
  uint8_t Kmimo;
  /// downlink power offset field
  uint8_t dl_power_off;
  /// amplitude of PDSCH (compared to RS) in symbols without pilots
  int16_t sqrt_rho_a;
  /// amplitude of PDSCH (compared to RS) in symbols containing pilots
  int16_t sqrt_rho_b;
} LTE_eNB_DLSCH_t;

#define PUSCH_x 2
#define PUSCH_y 3

typedef struct {
  /// Current Number of Symbols
  uint8_t Nsymb_pusch;
  /// SRS active flag
  uint8_t srs_active;
  /// Pointers to 8 HARQ processes for the ULSCH
  LTE_UL_UE_HARQ_t *harq_processes[8];
  /// Pointer to CQI data
  uint8_t o[MAX_CQI_BYTES];
  /// Length of CQI data (bits)
  uint8_t O;
  /// Format of CQI data
  UCI_format_t uci_format;
  /// Rank information
  uint8_t o_RI[2];
  /// Length of rank information (bits)
  uint8_t O_RI;
  /// Pointer to ACK
  uint8_t o_ACK[4];
  /// Minimum number of CQI bits for PUSCH (36-212 r8.6, Sec 5.2.4.1 p. 37)
  uint8_t O_CQI_MIN;
  /// ACK/NAK Bundling flag
  uint8_t bundling;
  /// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18)
  uint8_t e[MAX_NUM_CHANNEL_BITS];
  /// Interleaved "h"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18)
  uint8_t h[MAX_NUM_CHANNEL_BITS];
  /// Scrambled "b"-sequences (for definition see 36-211 V8.6 2009-03, p.14)
  uint8_t b_tilde[MAX_NUM_CHANNEL_BITS];
  /// Modulated "d"-sequences (for definition see 36-211 V8.6 2009-03, p.14)
  mod_sym_t d[MAX_NUM_RE];
  /// Transform-coded "z"-sequences (for definition see 36-211 V8.6 2009-03, p.14-15)
  mod_sym_t z[MAX_NUM_RE];
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)
  uint8_t Mdlharq;
  /// "q" sequences for CQI/PMI (for definition see 36-212 V8.6 2009-03, p.27)
  uint8_t q[MAX_CQI_PAYLOAD];
  /// coded and interleaved CQI bits
  uint8_t o_w[(MAX_CQI_BITS+8)*3];
  /// coded CQI bits
  uint8_t o_d[96+((MAX_CQI_BITS+8)*3)];
  /// coded ACK bits
  uint8_t q_ACK[MAX_ACK_PAYLOAD];
  /// coded RI bits
  uint8_t q_RI[MAX_RI_PAYLOAD];
  /// beta_offset_cqi times 8
  uint16_t beta_offset_cqi_times8;
  /// beta_offset_ri times 8
  uint16_t beta_offset_ri_times8;
  /// beta_offset_harqack times 8
  uint16_t beta_offset_harqack_times8;
  /// power_offset
  uint8_t power_offset;
  // for cooperative communication
  uint8_t cooperation_flag;
  /// RNTI attributed to this ULSCH
  uint16_t rnti;
  /// f_PUSCH parameter for PUSCH power control
  int16_t f_pusch;
  /// Po_PUSCH - target output power for PUSCH
  int16_t Po_PUSCH;
  /// PHR - current power headroom (based on last PUSCH transmission)
  int16_t PHR;
  /// num active cba group
  uint8_t num_active_cba_groups;
  /// num dci found for cba
  uint8_t num_cba_dci[10];
  /// allocated CBA RNTI
  uint16_t cba_rnti[4];//NUM_MAX_CBA_GROUP];
} LTE_UE_ULSCH_t;

typedef struct {
  /// Flag indicating that this ULSCH has been allocated by a DCI (otherwise it is a retransmission based on PHICH NAK)
  uint8_t dci_alloc;
  /// Flag indicating that this ULSCH has been allocated by a RAR (otherwise it is a retransmission based on PHICH NAK or DCI)
  uint8_t rar_alloc;
  /// Flag indicating that this ULSCH has new data
  uint8_t Ndi;
  /// Status Flag indicating for this ULSCH (idle,active,disabled)
  SCH_status_t status;
  /// Subframe scheduling indicator (i.e. Transmission opportunity indicator)
  uint8_t subframe_scheduling_flag;
  /// Subframe cba scheduling indicator (i.e. CBA Transmission opportunity indicator)
  uint8_t subframe_cba_scheduling_flag;
  /// PHICH active flag
  uint8_t phich_active;
  /// PHICH ACK
  uint8_t phich_ACK;
  /// Last TPC command
  uint8_t TPC;
  /// First Allocated RB
  uint16_t first_rb;
  /// Current Number of RBs
  uint16_t nb_rb;
  /// Transport block size
  uint32_t TBS;
  /// The payload + CRC size in bits
  uint32_t B;
  /// Length of ACK information (bits)
  uint8_t O_ACK;
  /// The value of DAI in DCI format 0
  uint8_t V_UL_DAI;
  /// Pointer to the payload
  uint8_t *b;
  /// Pointers to transport block segments
  uint8_t *c[MAX_NUM_ULSCH_SEGMENTS];
  /// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)
  uint32_t RTC[MAX_NUM_ULSCH_SEGMENTS];
  /// Index of current HARQ round for this ULSCH
  uint8_t round;
  /// MCS format for this ULSCH
  uint8_t mcs;
  /// Redundancy-version of the current sub-frame
  uint8_t rvidx;
  /// soft bits for each received segment ("w"-sequence)(for definition see 36-212 V8.6 2009-03, p.15)
  int16_t w[MAX_NUM_ULSCH_SEGMENTS][3*(6144+64)];
  /// soft bits for each received segment ("d"-sequence)(for definition see 36-212 V8.6 2009-03, p.15)
  int16_t *d[MAX_NUM_ULSCH_SEGMENTS];
  /// Number of code segments (for definition see 36-212 V8.6 2009-03, p.9)
  uint32_t C;
  /// Number of "small" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Cminus;
  /// Number of "large" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Cplus;
  /// Number of bits in "small" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Kminus;
  /// Number of bits in "large" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Kplus;
  /// Number of "Filler" bits (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t F;
  /// Number of MIMO layers (streams) (for definition see 36-212 V8.6 2009-03, p.17)
  uint8_t Nl;
  /// Msc_initial, Initial number of subcarriers for ULSCH (36-212, v8.6 2009-03, p.26-27)
  uint16_t Msc_initial;
  /// Nsymb_initial, Initial number of symbols for ULSCH (36-212, v8.6 2009-03, p.26-27)
  uint8_t Nsymb_initial;
  /// n_DMRS  for cyclic shift of DMRS (36.213 Table 9.1.2-2)
  uint8_t n_DMRS;
  /// n_DMRS 2 for cyclic shift of DMRS (36.211 Table 5.5.1.1.-1)
  uint8_t n_DMRS2;
  /// Flag to indicate that this ULSCH is for calibration information sent from UE (i.e. no MAC SDU to pass up)
  //  int calibration_flag;
} LTE_UL_eNB_HARQ_t;

typedef struct {
  /// Current Number of Symbols
  uint8_t Nsymb_pusch;
  /// SRS active flag
  uint8_t srs_active;
  /// Pointers to 8 HARQ processes for the ULSCH
  LTE_UL_eNB_HARQ_t *harq_processes[8];
  /// Concatenated "e"-sequences (for definition see 36-212 V8.6 2009-03, p.17-18)
  int16_t e[MAX_NUM_CHANNEL_BITS];
  /// Temporary h sequence to flag PUSCH_x/PUSCH_y symbols which are not scrambled
  uint8_t h[MAX_NUM_CHANNEL_BITS];
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17)
  uint8_t Mdlharq;
  /// Maximum number of iterations used in eNB turbo decoder
  uint8_t max_turbo_iterations;
  /// CQI CRC status
  uint8_t cqi_crc_status;
  /// Pointer to CQI data
  uint8_t o[MAX_CQI_BYTES];
  /// Format of CQI data
  UCI_format_t uci_format;
  /// Length of CQI data under RI=1 assumption(bits)
  uint8_t Or1;
  /// Length of CQI data under RI=2 assumption(bits)
  uint8_t Or2;
  /// Rank information
  uint8_t o_RI[2];
  /// Length of rank information (bits)
  uint8_t O_RI;
  /// Pointer to ACK
  uint8_t o_ACK[4];
  /// ACK/NAK Bundling flag
  uint8_t bundling;
  /// "q" sequences for CQI/PMI (for definition see 36-212 V8.6 2009-03, p.27)
  s8 q[MAX_CQI_PAYLOAD];
  /// number of coded CQI bits after interleaving
  uint8_t o_RCC;
  /// coded and interleaved CQI bits
  s8 o_w[(MAX_CQI_BITS+8)*3];
  /// coded CQI bits
  s8 o_d[96+((MAX_CQI_BITS+8)*3)];
  /// coded ACK bits
  int16_t q_ACK[MAX_ACK_PAYLOAD];
  /// coded RI bits
  int16_t q_RI[MAX_RI_PAYLOAD];
  /// beta_offset_cqi times 8
  uint16_t beta_offset_cqi_times8;
  /// beta_offset_ri times 8
  uint16_t beta_offset_ri_times8;
  /// beta_offset_harqack times 8
  uint16_t beta_offset_harqack_times8;
  /// Flag to indicate that eNB awaits UE Msg3
  uint8_t Msg3_active;
  /// Flag to indicate that eNB should decode UE Msg3
  uint8_t Msg3_flag;
  /// Subframe for Msg3
  uint8_t Msg3_subframe;
  /// Frame for Msg3
  uint32_t Msg3_frame;
  /// RNTI attributed to this ULSCH
  uint16_t rnti;
  /// cyclic shift for DM RS
  uint8_t cyclicShift;
  /// cooperation flag
  uint8_t cooperation_flag;
  /// num active cba group
  uint8_t num_active_cba_groups;
  /// allocated CBA RNTI for this ulsch
  uint16_t cba_rnti[4];//NUM_MAX_CBA_GROUP];
} LTE_eNB_ULSCH_t;

typedef struct {
  /// Flag indicating that this DLSCH has a new transport block
  uint8_t Ndi;
  /// DLSCH status flag indicating
  SCH_status_t status;
  /// Transport block size
  uint32_t TBS;
  /// The payload + CRC size in bits
  uint32_t B;
  /// Pointer to the payload
  uint8_t *b;
  /// Pointers to transport block segments
  uint8_t *c[MAX_NUM_DLSCH_SEGMENTS];
  /// RTC values for each segment (for definition see 36-212 V8.6 2009-03, p.15)
  uint32_t RTC[MAX_NUM_DLSCH_SEGMENTS];
  /// Index of current HARQ round for this DLSCH
  uint8_t round;
  /// MCS format for this DLSCH
  uint8_t mcs;
  /// Redundancy-version of the current sub-frame
  uint8_t rvidx;
  /// MIMO mode for this DLSCH
  MIMO_mode_t mimo_mode;
  /// soft bits for each received segment ("w"-sequence)(for definition see 36-212 V8.6 2009-03, p.15)
  int16_t w[MAX_NUM_DLSCH_SEGMENTS][3*(6144+64)];
  /// for abstraction soft bits for each received segment ("w"-sequence)(for definition see 36-212 V8.6 2009-03, p.15)
  double w_abs[MAX_NUM_DLSCH_SEGMENTS][3*(6144+64)];
  /// soft bits for each received segment ("d"-sequence)(for definition see 36-212 V8.6 2009-03, p.15)
  int16_t *d[MAX_NUM_DLSCH_SEGMENTS];
  /// Number of code segments (for definition see 36-212 V8.6 2009-03, p.9)
  uint32_t C;
  /// Number of "small" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Cminus;
  /// Number of "large" code segments (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Cplus;
  /// Number of bits in "small" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Kminus;
  /// Number of bits in "large" code segments (<6144) (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t Kplus;
  /// Number of "Filler" bits (for definition see 36-212 V8.6 2009-03, p.10)
  uint32_t F;
  /// Number of MIMO layers (streams) (for definition see 36-212 V8.6 2009-03, p.17)
  uint8_t Nl;
  /// current delta_pucch
  s8 delta_PUCCH;
  /// Number of soft channel bits
  uint32_t G;
  /// Current Number of RBs
  uint16_t nb_rb;
  /// Current subband PMI allocation
  uint16_t pmi_alloc;
  /// Current RB allocation
  uint32_t rb_alloc[4];
  /// downlink power offset field
  uint8_t dl_power_off;
} LTE_DL_UE_HARQ_t;


typedef struct {
  /// UL RSSI per receive antenna
  s32 UL_rssi[NB_ANTENNAS_RX];
  /// DL Wideband CQI index (2 TBs)
  uint8_t DL_cqi[2];
  /// DL Subband CQI index (from HLC feedback)
  uint8_t DL_subband_cqi[2][13];
  /// DL PMI Single Stream
  uint16_t DL_pmi_single;
  /// DL PMI Dual Stream
  uint16_t DL_pmi_dual;
  /// Current RI
  uint8_t rank;
  /// CRNTI of UE
  uint16_t crnti; ///user id (rnti) of connected UEs
  /// Initial timing offset estimate from PRACH for RAR
  s32 UE_timing_offset;
  /// Timing advance estimate from PUSCH for MAC timing advance signalling
  s32 timing_advance_update;
  /// Current mode of UE (NOT SYCHED, RAR, PUSCH)
  UE_MODE_t mode;
  /// Current sector where UE is attached
  uint8_t sector;
  ///
  uint32_t dlsch_sliding_cnt;
  ///
  uint32_t dlsch_ACK[8];
  uint32_t dlsch_NAK[8];
  ///
  uint32_t dlsch_l2_errors;
  ///
  uint32_t dlsch_trials[8];
  ///
  uint32_t ulsch_errors[3];
  ///
  uint32_t ulsch_consecutive_errors[3];
  ///
  uint32_t ulsch_decoding_attempts[3][8];
  ///
  uint32_t ulsch_round_errors[3][8];
  uint32_t ulsch_decoding_attempts_last[3][8];
  uint32_t ulsch_round_errors_last[3][8];
  uint32_t ulsch_round_fer[3][8];
  uint32_t sr_received;
  uint32_t sr_total;

  s8 dlsch_mcs_offset;
  /// Target mcs1 after rate-adaptation (used by MAC layer scheduler)
  uint8_t dlsch_mcs1;
  /// Target mcs2 after rate-adaptation (used by MAC layer scheduler)
  uint8_t dlsch_mcs2;
  //  SRS_param_t SRS_parameters;
  int total_TBS;
  //
  int total_TBS_last;
  //
  unsigned int dlsch_bitrate;
  //
  unsigned int total_transmitted_bits;
} LTE_eNB_UE_stats;

typedef struct {
  /// HARQ process id
  uint8_t harq_id;
  /// ACK bits (after decoding)
  uint8_t ack;
  /// send status (for PUCCH)
  uint8_t send_harq_status;
  /// nCCE (for PUCCH)
  uint8_t nCCE;
} harq_status_t;

typedef struct {
  /// RNTI
  uint16_t rnti;
  /// Active flag for DLSCH demodulation
  uint8_t active;
  /// Transmission mode
  uint8_t mode1_flag;
  /// amplitude of PDSCH (compared to RS) in symbols without pilots
  int16_t sqrt_rho_a;
  /// amplitude of PDSCH (compared to RS) in symbols containing pilots
  int16_t sqrt_rho_b;
  /// Current HARQ process id
  uint8_t current_harq_pid;
  /// Current subband antenna selection
  uint32_t antenna_alloc;
  /// Current subband RI allocation
  uint32_t ri_alloc;
  /// Current subband CQI1 allocation
  uint32_t cqi_alloc1;
  /// Current subband CQI2 allocation
  uint32_t cqi_alloc2;
  /// HARQ-ACKs
  harq_status_t harq_ack[10];
  /// Pointers to up to 8 HARQ processes
  LTE_DL_UE_HARQ_t *harq_processes[8];
  /// Layer index for this DLSCH
  uint8_t layer_index;
  /// Maximum number of HARQ rounds (for definition see 36-212 V8.6 2009-03, p.17
  uint8_t Mdlharq;
  /// MIMO transmission mode indicator for this sub-frame (for definition see 36-212 V8.6 2009-03, p.17)
  uint8_t Kmimo;
  /// Maximum number of Turbo iterations
  uint8_t max_turbo_iterations;
} LTE_UE_DLSCH_t;

typedef enum {format0,
	      format1,
	      format1A,
	      format1A_RA,
	      format1B,
	      format1C,
	      format1D,
	      format1E_2A_M10PRB,
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

typedef enum {
  SI_PDSCH=0,
  RA_PDSCH,
  PDSCH,
  PMCH
} PDSCH_t;

typedef enum {
  pucch_format1=0,
  pucch_format1a,
  pucch_format1b,
  pucch_format2,
  pucch_format2a,
  pucch_format2b
} PUCCH_FMT_t;


typedef struct {
  /// Length of DCI in bits
  uint8_t dci_length;
  /// Aggregation level
  uint8_t L;
  /// Position of first CCE of the dci
  unsigned int nCCE;
  /// flag to indicate that this is a RA response
  uint8_t ra_flag;
  /// rnti
  uint16_t rnti;
  /// Format
  DCI_format_t format;
  /// DCI pdu
  uint8_t dci_pdu[8];
} DCI_ALLOC_t;


/**@}*/
#endif
