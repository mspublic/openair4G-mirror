/*
                               rrc_L2_asconfig.h
                             -------------------
  AUTHOR  : Linus GASSER modified by Lionel GAUTHIER Raymond KNOPP
  COMPANY : EURECOM
  EMAIL   : linus.gasser@epfl.ch
  EMAIL   : lionel.gauthier@eurecom.fr
  EMAIL   : raymond.knopp@eurecom.fr



 ***************************************************************************/

#ifndef __RRC_ASCONFIG_H__
#define __RRC_ASCONFIG_H__

// #include "rrm_constants.h"
// #include "outer_loop_structs.h"
// #include "platform.h"

typedef struct {
  u32             bss;          // block set size
  u16             bs;           // block size
  u8              crc;          // size of CRC in bits
  u8              rm;           // rate matching
//  u8              cr;           // coding rate
//  u8              code;         // turbo, convolutional,etc
//  u8              tti;          // transmission time interval
} TF;

typedef struct {
  u32             e_r;
  int             timer_poll;
  int             timer_poll_prohibit;
  int             timer_discard;
  int             timer_poll_periodic;
  int             timer_status_prohibit;
  int             timer_status_periodic;
  int             timer_rst;
  int             timer_mrw;
  int             missing_pdu_indicator;
  u32             pdu_size;
//      int                                 in_sequence_delivery; // not implemented
  u8              max_rst;
  u8              max_dat;
  u16             poll_pdu;
  u16             poll_sdu;
  u8              poll_window;
  u32             tx_window_size;
  u32             rx_window_size;
  u8              max_mrw;
  u8              last_transmission_pdu_poll_trigger;
  u8              last_retransmission_pdu_poll_trigger;
  enum RLC_SDU_DISCARD_MODE sdu_discard_mode;
  u32             send_mrw;
} AM;

typedef struct {
  u32             e_r;
  u32             timer_discard;
  u32             sdu_discard_mode;
  u32             segmentation_indication;
  u32             delivery_of_erroneous_sdu;
} TM;

typedef struct {
  u32             e_r;
  u32             timer_discard;
  u32             sdu_discard_mode;
} UM;

typedef struct {
  u8              trch_type;
  u8              trch_identity;
  u8              logch_identity;
  u8              mac_logch_priority;
} MAP_INFO;

typedef struct {
  u32             rlc_mode;
  AM              am;
  TM              tm;
  UM              um;
} RLC_INFO;

typedef struct {
  int             header_compression_algorithm;
} PDCP_INFO;

// if you modify this structure, modify the rpc_rrm_data_conversion.c file
typedef struct {
  u8              num_tfc;      // number of combinations in the TFCS
  u8              tfc[JRRM_MAX_TFC][JRRM_MAX_TRCH_CCTRCH];      // transport format combination set
} TFCS;

/*typedef struct {
  u8              num_tfc;      // number of combinations in the TFCS
  u8              tfc[4][1];    // transport format combination set
} TFCS_FACH;*/


typedef struct {
  u8              id;
  u8              type;
  u8              peer_id;
  u8              cctrch;       // id of cctrch this trch is mapped onto
  u8              num_tf;       // number of transport formats of the transport format set
  // semi static transport format informations
  u8              tti;          // etc. (LG ADDED)
  u8              code;         // turbo, convolutional,etc
  TF              tf[JRRM_MAX_TF];      // transport format set
} TRCH;

typedef struct {
  u8              id;
  u8              cctrch;       // id of cctrch this trch is mapped onto
  u8              num_tf;       // number of transport formats of the transport format set
  // semi static transport format informations
  u8              tti;          // etc. (LG ADDED)
  u8              code;         // turbo, convolutional,etc
  TF              tf[4];        // transport format set
} TRCH_FACH;

typedef struct {
  u8              id;
  u8              cctrch;       // id of cctrch this trch is mapped onto
  TF              tf[1];        // transport format set (implicit only one tf)
  // semi static transport format informations
  u8              tti;
  u8              code;         // turbo, convolutional,etc
} TRCH_RACH;

typedef struct {
  u8              id;
  u8              cctrch;       // id of cctrch this trch is mapped onto
  TF              tf[1];        // transport format set (implicit only one tf)
  // semi static transport format informations
  u8              tti;
  u8              code;         // turbo, convolutional,etc
} TRCH_BCH;

typedef struct {
  u8              cctrch;       // id of cctrch this trch is mapped onto
  u8              num_tf;
  // semi static transport format informations
  u8              tti;
  u8              code;         // turbo, convolutional,etc
  TF              tf[32];       // transport format set 
} TRCH_DSCH;

typedef struct {
  u8              id;
  u8              bf;
  u8              num_bf;
  u8              seq_index;
  u8              mid_rot;
  u8              dsp_flag;
  u8              diversity_delay;
} PCH;

// WARNING: Hard coded encoding in rrc_utilities.c
typedef struct {
  PCH             pch[JRRM_MAX_PHY_CCTRCH];
  TFCS            tfcs;
  u16             gain_adjust;
  u8              num_trch;
  u8              trch_id;
  u8              pl;           // puncturing level
  u8              num_tfci_bits;        /* Number of bits to code TFCI (0 if none) */
  u8              tti;          /* Largest TTI of TrCh */
  u8              num_pch;
  u8              DSP_FLAG;
  u8              DIVERSITY_DELAY;
  u8              sec_interl;
  u8              type;
  u8              id;
} CCTRCH;

typedef struct {
  PCH             pch[8];
  //TFCS_FACH       tfcs;
  u16             gain_adjust;
  u8              trch_id;
  u8              pl;           // puncturing level
  u8              num_tfci_bits;        /* Number of bits to code TFCI (0 if none) */
  u8              tti;          /* Largest TTI of TrCh */
  u8              num_pch;
  u8              DSP_FLAG;
  u8              DIVERSITY_DELAY;
  u8              sec_interl;
  u8              id;
} CCTRCH_FACH;

/*typedef struct {
  u8              trch_id;
  int             pl;           // puncturing level
  int             SndI;
  u8              num_tfci_bits;        // Number of bits to code TFCI (0 if none)
  u8              MaxTTI;       // Largest TTI of TrCh
  u8              MaxNumPCh;    // Maximum number of physical channels per frame
  u8              PCh[80];      // ID's of potential channels (from 0 to 255),ie 15*slot + "number" implicit allocation betw. rx and tx
  u8              BF[80];       // Initial Burst Format to use
  u8              NumBF[80];    // Number of Burst Formats to search
  u8              SeqIndex[80]; // OVSF sequence for SF = 16
  u8              MidRot[80];
  u8              DSP_FLAG;
  int             gain_adjust;
  u8              sec_interl;
} CCTRCH_DSCH;*/


typedef struct {
  PCH             pch[4];
  u16             gain_adjust;
  u8              trch_id;
  u8              pl;           // puncturing level
  u8              num_tfci_bits;        /* Number of bits to code TFCI (0 if none) */
  u8              tti;          /* Largest TTI of TrCh */
  u8              num_pch;
  u8              DSP_FLAG;
  u8              DIVERSITY_DELAY;
  u8              sec_interl;
  u8              id;
} CCTRCH_RACH;


typedef struct {
  PCH             pch[1];
  u8              id;
  u16             gain_adjust;
  u8              trch_id;
  u8              pl;           // puncturing level
  u8              num_tfci_bits;        /* Number of bits to code TFCI (0 if none) */
  u8              tti;          /* Largest TTI of TrCh */
  u8              DSP_FLAG;
  u8              DIVERSITY_DELAY;
  u8              sec_interl;
  u8              BCH_Power;
} CCTRCH_BCH;

typedef struct {
  u8              rb_type;
  u32             traffic_class;        // for rb mux
  RLC_INFO        rlc_info;
  PDCP_INFO       pdcp_info;
  MAP_INFO        mapping_info;
  u16             rb_id;

  u32             max_br;
  u32             guaranteed_br;

} RADIOBEARER;

typedef struct {
  int             numPCh;
  int             TFCI;
  int             TF[2];
} TFCO;

typedef struct {
  int             TIMER300;
  int             TIMER302;
  int             TIMER305;
  int             TIMER307;
  int             TIMER308;
  int             TIMER312;
  int             TIMER313;
  int             TIMER314;
  int             TIMER315;
} L3TIMERS_;

typedef struct {
  int             COUNTERN300;
  int             COUNTERN302;
  int             COUNTERN308;
  int             COUNTERN312;
  int             COUNTERN313;
} L3COUNTERS_;


typedef struct {
  TRCH_BCH        bch_trch;
  CCTRCH_BCH      bch_cctrch;
  u8              code_group;
} PCCPCH_SYSTEM_INFO;

typedef struct {
  TRCH_FACH       fach_trch;
  CCTRCH_FACH     fach_cctrch;
} SCCPCH_SYSTEM_INFO;

typedef struct {
  TRCH_RACH       rach_trch;
  CCTRCH_RACH     rach_cctrch;
  s16             RG_RX_GAIN;
  //int     persistence_scaling_factors[10];
  //int     ac_to_asc_mapping[10];
} PRACH_SYSTEM_INFO;



typedef struct {
  int             MaxNumRemote;
  L3TIMERS_       Timers;
  L3COUNTERS_     Counters;
} L3_;


typedef struct {
  u8              rrm_action;   // ACTION_NULL,ADD,REMOVE,MODIFY
  u8              rrm_element;  // rb,trch,cctrch
  u8              rrm_element_index;    // rb/trch/cctrch index
} RRM_COMMAND_MT;

typedef struct {
  u8              rrm_action;   // ACTION_NULL,ADD,REMOVE,MODIFY
  u8              mobile;
  u8              rrm_element;  // rb,trch,cctrch
  u8              rrm_element_index;    // rb/trch/cctrch index
} RRM_COMMAND_RG;


typedef struct {
  u8              nb_commands;
  RRM_COMMAND_RG  rrm_commands[JRRM_MAX_COMMANDS_PER_TRANSACTION];
  TRCH            trch_dl[JRRM_MAX_TRCH_RG];
  TRCH            trch_ul[JRRM_MAX_TRCH_RG];
  CCTRCH          cctrch_dl[JRRM_MAX_CCTRCH_RG];
  CCTRCH          cctrch_ul[JRRM_MAX_CCTRCH_RG];
  RADIOBEARER     bearer_dl[JRRM_MAX_RB_RG];
  RADIOBEARER     bearer_ul[JRRM_MAX_RB_RG];
  L3_             L3;

} RG_CONFIG;

typedef struct {
  u8              nb_commands;
  RRM_COMMAND_MT  rrm_commands[JRRM_MAX_COMMANDS_PER_TRANSACTION];
  TRCH            trch_dl[JRRM_MAX_TRCH_MOBILE];
  TRCH            trch_ul[JRRM_MAX_TRCH_MOBILE];
  CCTRCH          cctrch_dl[JRRM_MAX_CCTRCH_MOBILE];
  CCTRCH          cctrch_ul[JRRM_MAX_CCTRCH_MOBILE];
  RADIOBEARER     bearer_dl[JRRM_MAX_RB_MOBILE];
  RADIOBEARER     bearer_ul[JRRM_MAX_RB_MOBILE];
  L3_             L3;

#        ifdef ONLY_L1
#            define RRM_CONFIG_SERIALIZE_BUFFER_SIZE 1550
  u8              serialize_buffer[RRM_CONFIG_SERIALIZE_BUFFER_SIZE];
  int             serialize_length;
  int             serialize_buffer_up_to_date;
  int             config_id;
#        endif
} MT_CONFIG;

#        ifdef ONLY_L1
typedef struct {
  u16             mobile_id;
  u16             config_id;    // the mobile knows if this id is different from the previously received 
  // that it must take into account this configuration
  // compressed config follows this struct in a message
} rrc_config_ota;
#        endif

#ifdef NODE_RG
typedef struct {
  RG_CONFIG       rg_config;
  MT_CONFIG       mt_config[JRRM_MAX_MANAGED_MOBILES_PER_RG];
  PCCPCH_SYSTEM_INFO pccpch;
  SCCPCH_SYSTEM_INFO sccpch;
  PRACH_SYSTEM_INFO prach;
  OUTER_LOOP_PC_VARS outer_loop_vars;
  u8              active_rabs[JRRM_MAX_MANAGED_MOBILES_PER_RG][JRRM_MAX_RB_MOBILE];
} RRC_AS_CONFIG;

#else
      /* NODE_RG */
typedef struct {
  MT_CONFIG       mt_config;
  PCCPCH_SYSTEM_INFO pccpch;
  SCCPCH_SYSTEM_INFO sccpch;
  PRACH_SYSTEM_INFO prach;
  OUTER_LOOP_PC_VARS outer_loop_vars;
  int             prach_slot;
  int             power_control_ul_received;
} RRC_AS_CONFIG;
#endif
       /* NODE_RG */
//typedef   MT_CONFIG MAIN_MOBILE ;
//typedef   RG_CONFIG MAIN_RADIO_GATEWAY;

#    endif
