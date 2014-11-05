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

/*
                                enb_config.h
                             -------------------
  AUTHOR  : Lionel GAUTHIER, Laurent Winckel
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
*/

#ifndef ENB_CONFIG_H_
#define ENB_CONFIG_H_
#include  <netinet/in.h>

#include "commonDef.h"
#include "platform_types.h"
#include "PHY/impl_defs_lte.h"
#include "s1ap_messages_types.h"
#include "RRC/LITE/MESSAGES/SystemInformationBlockType2.h"

// Hard to find a defined value for max enb...
#define MAX_ENB 16

#define IPV4_STR_ADDR_TO_INT_NWBO(AdDr_StR,NwBo,MeSsAgE ) do {\
            struct in_addr inp;\
            if ( inet_aton(AdDr_StR, &inp ) < 0 ) {\
                AssertFatal (0, MeSsAgE);\
            } else {\
                NwBo = inp.s_addr;\
            }\
        } while (0);

typedef struct mme_ip_address_s {
    unsigned  ipv4:1;
    unsigned  ipv6:1;
    unsigned  active:1;
    char     *ipv4_address;
    char     *ipv6_address;
} mme_ip_address_t;

typedef struct Enb_properties_s {
    /* Unique eNB_id to identify the eNB within EPC.
     * For macro eNB ids this field should be 20 bits long.
     * For home eNB ids this field should be 28 bits long.
     */
    uint32_t            eNB_id;

    /* The type of the cell */
    enum cell_type_e    cell_type;

    /* Optional name for the cell
     * NOTE: the name can be NULL (i.e no name) and will be cropped to 150
     * characters.
     */
    char               *eNB_name;

    /* Tracking area code */
    uint16_t            tac;

    /* Mobile Country Code
     * Mobile Network Code
     */
    uint16_t            mcc;
    uint16_t            mnc;
    uint8_t             mnc_digit_length;



    /* Physical parameters */
  int16_t                 nb_cc;
  lte_frame_type_t        frame_type[1+MAX_NUM_CCs];
  uint8_t                 tdd_config[1+MAX_NUM_CCs];
  uint8_t                 tdd_config_s[1+MAX_NUM_CCs];
  lte_prefix_type_t       prefix_type[1+MAX_NUM_CCs];
  int16_t                 eutra_band[1+MAX_NUM_CCs];
  uint64_t                downlink_frequency[1+MAX_NUM_CCs];
  int32_t                 uplink_frequency_offset[1+MAX_NUM_CCs];

  int16_t                 Nid_cell[1+MAX_NUM_CCs];// for testing, change later
  int16_t                 N_RB_DL[1+MAX_NUM_CCs];// for testing, change later
  int	                  nb_antennas_tx[1+MAX_NUM_CCs];   
  int                     nb_antennas_rx[1+MAX_NUM_CCs];   
  int	                  tx_gain[1+MAX_NUM_CCs];   
  int                     rx_gain[1+MAX_NUM_CCs];   
  long                    prach_root[1+MAX_NUM_CCs];   
  long                    prach_config_index[1+MAX_NUM_CCs]; 
  BOOLEAN_t               prach_high_speed[1+MAX_NUM_CCs]; 
  long                    prach_zero_correlation[1+MAX_NUM_CCs]; 
  long                    prach_freq_offset[1+MAX_NUM_CCs]; 
  long                    pucch_delta_shift[1+MAX_NUM_CCs]; 
  long                    pucch_nRB_CQI[1+MAX_NUM_CCs]; 
  long                    pucch_nCS_AN[1+MAX_NUM_CCs];
#ifndef Rel10 
  long                    pucch_n1_AN[1+MAX_NUM_CCs]; 
#endif
  long                    pdsch_referenceSignalPower[1+MAX_NUM_CCs]; 
  long                    pdsch_p_b[1+MAX_NUM_CCs]; 
  long                    pusch_n_SB[1+MAX_NUM_CCs];
  long                    pusch_hoppingMode[1+MAX_NUM_CCs];
  long                    pusch_hoppingOffset[1+MAX_NUM_CCs];
  BOOLEAN_t               pusch_enable64QAM[1+MAX_NUM_CCs]; 
  BOOLEAN_t               pusch_groupHoppingEnabled[1+MAX_NUM_CCs]; 
  long                    pusch_groupAssignment[1+MAX_NUM_CCs]; 
  BOOLEAN_t               pusch_sequenceHoppingEnabled[1+MAX_NUM_CCs]; 
  long                    pusch_nDMRS1[1+MAX_NUM_CCs]; 
  long                    phich_duration[1+MAX_NUM_CCs]; 
  long                    phich_resource[1+MAX_NUM_CCs]; 
  BOOLEAN_t               srs_enable[1+MAX_NUM_CCs]; 
  long                    srs_BandwidthConfig[1+MAX_NUM_CCs]; 
  long                    srs_SubframeConfig[1+MAX_NUM_CCs]; 
  BOOLEAN_t               srs_ackNackST[1+MAX_NUM_CCs]; 
  BOOLEAN_t               srs_MaxUpPts[1+MAX_NUM_CCs]; 
  long                    pusch_p0_Nominal[1+MAX_NUM_CCs]; 
  long                    pusch_alpha[1+MAX_NUM_CCs]; 
  long                    pucch_p0_Nominal[1+MAX_NUM_CCs]; 
  long                    msg3_delta_Preamble[1+MAX_NUM_CCs]; 
  long                    ul_CyclicPrefixLength[1+MAX_NUM_CCs];
  e_DeltaFList_PUCCH__deltaF_PUCCH_Format1                    pucch_deltaF_Format1[1+MAX_NUM_CCs]; 
  e_DeltaFList_PUCCH__deltaF_PUCCH_Format1b                   pucch_deltaF_Format1b[1+MAX_NUM_CCs]; 
  e_DeltaFList_PUCCH__deltaF_PUCCH_Format2                    pucch_deltaF_Format2[1+MAX_NUM_CCs]; 
  e_DeltaFList_PUCCH__deltaF_PUCCH_Format2a                   pucch_deltaF_Format2a[1+MAX_NUM_CCs]; 
  e_DeltaFList_PUCCH__deltaF_PUCCH_Format2b                   pucch_deltaF_Format2b[1+MAX_NUM_CCs]; 
  long                    rach_numberOfRA_Preambles[1+MAX_NUM_CCs]; 
  BOOLEAN_t               rach_preamblesGroupAConfig[1+MAX_NUM_CCs];
  long	                  rach_sizeOfRA_PreamblesGroupA[1+MAX_NUM_CCs];
  long	                  rach_messageSizeGroupA[1+MAX_NUM_CCs];
  e_RACH_ConfigCommon__preambleInfo__preamblesGroupAConfig__messagePowerOffsetGroupB	                  rach_messagePowerOffsetGroupB[1+MAX_NUM_CCs];
  long                    rach_powerRampingStep[1+MAX_NUM_CCs]; 
  long                    rach_preambleInitialReceivedTargetPower[1+MAX_NUM_CCs]; 
  long                    rach_preambleTransMax[1+MAX_NUM_CCs]; 
  long                    rach_raResponseWindowSize[1+MAX_NUM_CCs]; 
  long                    rach_macContentionResolutionTimer[1+MAX_NUM_CCs];
  long                    rach_maxHARQ_Msg3Tx[1+MAX_NUM_CCs];
  long                    bcch_modificationPeriodCoeff[1+MAX_NUM_CCs];
  long                    pcch_defaultPagingCycle[1+MAX_NUM_CCs];
  long                    pcch_nB[1+MAX_NUM_CCs];
  long                    ue_TimersAndConstants_t300[1+MAX_NUM_CCs];
  long                    ue_TimersAndConstants_t301[1+MAX_NUM_CCs];
  long                    ue_TimersAndConstants_t310[1+MAX_NUM_CCs];
  long                    ue_TimersAndConstants_t311[1+MAX_NUM_CCs];
  long                    ue_TimersAndConstants_n310[1+MAX_NUM_CCs];
  long                    ue_TimersAndConstants_n311[1+MAX_NUM_CCs];
  

    /* Nb of MME to connect to */
  uint8_t             nb_mme;
    /* List of MME to connect to */
  mme_ip_address_t    mme_ip_address[S1AP_MAX_NB_MME_IP_ADDRESS];

  char               *enb_interface_name_for_S1U;
  in_addr_t           enb_ipv4_address_for_S1U;
  tcp_udp_port_t      enb_port_for_S1U;

  char               *enb_interface_name_for_S1_MME;
  in_addr_t           enb_ipv4_address_for_S1_MME;

  // log config
  int16_t           glog_level;
  int16_t           glog_verbosity;
  int16_t           hw_log_level;
  int16_t           hw_log_verbosity;
  int16_t           phy_log_level;
  int16_t           phy_log_verbosity;
  int16_t           mac_log_level;
  int16_t           mac_log_verbosity;
  int16_t           rlc_log_level;
  int16_t           rlc_log_verbosity;
  int16_t           pdcp_log_level;
  int16_t           pdcp_log_verbosity;
  int16_t           rrc_log_level;
  int16_t           rrc_log_verbosity;
  int16_t           gtpu_log_level;
  int16_t           gtpu_log_verbosity;
  int16_t           udp_log_level;
  int16_t           udp_log_verbosity;

} Enb_properties_t;

typedef struct Enb_properties_array_s {
    int                  number;
    Enb_properties_t    *properties[MAX_ENB];
} Enb_properties_array_t;

const Enb_properties_array_t *enb_config_init(char* lib_config_file_name_pP);

const Enb_properties_array_t *enb_config_get(void);

#endif /* ENB_CONFIG_H_ */
