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

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

*******************************************************************************/

/*
                               rrm_config_structs.h
                             -------------------
  AUTHOR  : Linus GASSER modified by Lionel GAUTHIER Raymond KNOPP
  COMPANY : EURECOM
  EMAIL   : linus.gasser@epfl.ch
  EMAIL   : lionel.gauthier@eurecom.fr
  EMAIL   : raymond.knopp@eurecom.fr



 ***************************************************************************/
#ifdef OLD_RRC_CELLULAR
#    ifndef __RRM_CONFIG_STRUCTS_H__
#        define __RRM_CONFIG_STRUCTS_H__

#        include "platform_types.h"
#        include "platform_constants.h"
#        include "rrm_constants.h"
#        include "rlc.h"

typedef volatile struct {
  uint32_t             e_r;
  int             timer_poll;
  int             timer_poll_prohibit;
  int             timer_discard;
  int             timer_poll_periodic;
  int             timer_status_prohibit;
  int             timer_status_periodic;
  int             timer_rst;
  int             timer_mrw;
  int             missing_pdu_indicator;
  uint32_t             pdu_size;
//      int                                 in_sequence_delivery; // not implemented
  uint8_t              max_rst;
  uint8_t              max_dat;
  uint16_t             poll_pdu;
  uint16_t             poll_sdu;
  uint8_t              poll_window;
  uint32_t             tx_window_size;
  uint32_t             rx_window_size;
  uint8_t              max_mrw;
  uint8_t              last_transmission_pdu_poll_trigger;
  uint8_t              last_retransmission_pdu_poll_trigger;
  enum RLC_SDU_DISCARD_MODE sdu_discard_mode;
  uint32_t             send_mrw;
} AM;

typedef volatile struct {
  uint32_t             e_r;
  uint32_t             timer_discard;
  uint32_t             sdu_discard_mode;
  uint32_t             segmentation_indication;
  uint32_t             delivery_of_erroneous_sdu;
} TM;

typedef volatile struct {
  uint32_t             e_r;
  uint32_t             timer_discard;
  uint32_t             sdu_discard_mode;
} UM;

typedef volatile struct {
  uint8_t              logch_identity;
  uint8_t              mac_logch_priority;
} MAP_INFO;

typedef volatile struct {
  uint32_t             rlc_mode;
  AM              am;
  TM              tm;
  UM              um;
} RLC_INFO;

typedef volatile struct {
  int             header_compression_algorithm;
} PDCP_INFO;




typedef volatile struct {
  rb_type_t       rb_type;
  RLC_INFO        rlc_info;
  PDCP_INFO       pdcp_info;
  uint16_t             rb_id;
  // Added for OPENAIR MAC
  //LCHAN_DESC      Lchan_desc;

} RADIOBEARER;


typedef volatile struct {
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

typedef volatile struct {
  int             COUNTERN300;
  int             COUNTERN302;
  int             COUNTERN308;
  int             COUNTERN312;
  int             COUNTERN313;
} L3COUNTERS_;


typedef volatile struct {
  int             MaxNumRemote;
  L3TIMERS_       Timers;
  L3COUNTERS_     Counters;
} L3_;


typedef volatile struct {
  uint8_t              rrm_action;   // ACTION_NULL,ADD,REMOVE,MODIFY
  uint8_t              rrm_element;  // rb,trch,cctrch
  uint8_t              rrm_element_index;    // rb/trch/cctrch index
} RRM_COMMAND_MT;

typedef volatile struct {
  uint8_t              rrm_action;   // ACTION_NULL,ADD,REMOVE,MODIFY
  uint8_t              mobile;
  uint8_t              rrm_element;  // rb,trch,cctrch
  uint8_t              rrm_element_index;    // rb/trch/cctrch index
} RRM_COMMAND_RG;


typedef volatile struct {
  uint8_t              nb_commands;
  RRM_COMMAND_RG  rrm_commands[JRRM_MAX_COMMANDS_PER_TRANSACTION];


  RADIOBEARER     bearer[MAX_RB_RG];
  L3_             L3;

} RG_CONFIG;

typedef volatile struct {
  uint8_t              nb_commands;
  RRM_COMMAND_MT  rrm_commands[JRRM_MAX_COMMANDS_PER_TRANSACTION];

  RADIOBEARER     bearer[MAX_RB_MOBILE];
  L3_             L3;

} MT_CONFIG;

#        ifdef NODE_RG
typedef volatile struct {
  RG_CONFIG       rg_config;
  MT_CONFIG       mt_config[MAX_MOBILES_PER_RG];
} RRM_VARS;

#        else
      /* NODE_RG */
typedef volatile struct {
  MT_CONFIG       mt_config[MAX_MANAGED_RG_PER_MOBILE];



} RRM_VARS;
#        endif
       /* NODE_RG */
//typedef   MT_CONFIG MAIN_MOBILE ;
//typedef   RG_CONFIG MAIN_RADIO_GATEWAY;

#    endif
#endif
