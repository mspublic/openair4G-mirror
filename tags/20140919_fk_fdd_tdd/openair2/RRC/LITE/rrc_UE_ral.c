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


/*! \file rrc_UE_ral.c
 * \brief rrc procedures for handling RAL messages
 * \author Lionel GAUTHIER
 * \date 2013
 * \version 1.0
 * \company Eurecom
 * \email: lionel.gauthier@eurecom.fr
 */
#define RRC_UE
#define RRC_UE_RAL_C
//-----------------------------------------------------------------------------
#include "rrc_UE_ral.h"
#include "assertions.h"
#include "collection/hashtable/obj_hashtable.h"
#include "RRC/LITE/defs.h"
#include "RRC/LITE/extern.h"


//-----------------------------------------------------------------------------
int rrc_ue_ral_delete_all_thresholds_type(unsigned int mod_idP, ral_link_param_type_t *param_type_pP)
//-----------------------------------------------------------------------------
{
    hashtable_rc_t           rc;
    rrc_ral_threshold_key_t *key;
    rrc_ral_threshold_key_t *keys = NULL;
    unsigned int             num_keys = 0;
    int                      return_code = 0;

    rc =  obj_hashtable_get_keys(UE_rrc_inst[mod_idP].ral_meas_thresholds, (void*)&keys, &num_keys);
    if (rc == HASH_TABLE_OK) {
        key = keys;
        while (num_keys > 0) {
            if (memcmp(&key->link_param_type, param_type_pP, sizeof(ral_link_param_type_t)) == 0) {
                rc = obj_hashtable_remove (UE_rrc_inst[mod_idP].ral_meas_thresholds, key, sizeof(rrc_ral_threshold_key_t));
                if (rc != HASH_TABLE_OK) {
                    return_code = -1;
                }
            }
            key = &key[1];
            num_keys--;
        }
    } else {
        return_code = -1;
    }

    if (keys != NULL) {
        free(keys);
    }
    return return_code;
}


//-----------------------------------------------------------------------------
int rrc_ue_ral_delete_threshold(unsigned int mod_idP, ral_link_param_type_t *param_type_pP, ral_threshold_t *threshold_pP)
//-----------------------------------------------------------------------------
{
    hashtable_rc_t           rc;
    rrc_ral_threshold_key_t  ref_key;

    memcpy(&ref_key.link_param_type, param_type_pP, sizeof(ral_link_param_type_t));
    memcpy(&ref_key.threshold,       threshold_pP,  sizeof(ral_threshold_t));
    rc = obj_hashtable_remove (UE_rrc_inst[mod_idP].ral_meas_thresholds, (void*)&ref_key, sizeof(rrc_ral_threshold_key_t));
    if (rc == HASH_TABLE_OK) {
        return 0;
    } else {
        return -1;
    }
}


//-----------------------------------------------------------------------------
int rrc_ue_ral_handle_configure_threshold_request(unsigned int mod_idP, MessageDef *msg_pP)
//-----------------------------------------------------------------------------
{
    ral_transaction_id_t               transaction_id            = 0;
    rrc_ral_configure_threshold_req_t *configure_threshold_req_p = NULL;
    ral_link_cfg_param_t              *link_cfg_param_p          = NULL;
    ral_threshold_t                   *threshold_p               = NULL;
    MessageDef                        *message_p                 = NULL;
    unsigned int                       ix_param                  = 0;
    unsigned int                       ix_thresholds             = 0;

    DevAssert(msg_pP != NULL);

    LOG_I(RRC, "[UE %d] Received %s\n", mod_idP, ITTI_MSG_NAME (msg_pP));
    configure_threshold_req_p = &RRC_RAL_CONFIGURE_THRESHOLD_REQ(msg_pP);

    transaction_id = configure_threshold_req_p->transaction_id;

    for (ix_param = 0; ix_param < configure_threshold_req_p->num_link_cfg_params; ix_param++) {
        link_cfg_param_p = &configure_threshold_req_p->link_cfg_params[ix_param];

        switch (link_cfg_param_p->th_action) {
            case RAL_TH_ACTION_SET_NORMAL_THRESHOLD:
            case RAL_TH_ACTION_SET_ONE_SHOT_THRESHOLD:
                switch (link_cfg_param_p->link_param_type.choice) {

                    case RAL_LINK_PARAM_TYPE_CHOICE_GEN:
                        switch (link_cfg_param_p->link_param_type._union.link_param_gen) {
                            case RAL_LINK_PARAM_GEN_DATA_RATE:
                            case RAL_LINK_PARAM_GEN_SIGNAL_STRENGTH:
                            case RAL_LINK_PARAM_GEN_SINR:
                            case RAL_LINK_PARAM_GEN_THROUGHPUT:
                            case RAL_LINK_PARAM_GEN_PACKET_ERROR_RATE:
                                message_p = itti_alloc_new_message (TASK_RRC_UE, PHY_MEAS_THRESHOLD_REQ);
                                PHY_MEAS_THRESHOLD_REQ(message_p).transaction_id  = transaction_id;
                                memcpy (&PHY_MEAS_THRESHOLD_REQ(message_p).cfg_param, (void *) link_cfg_param_p, sizeof(ral_link_cfg_param_t));
                                itti_send_msg_to_task (TASK_PHY_UE, ITTI_MSG_INSTANCE(msg_pP), message_p);
                                break;
                            default:
                                LOG_E(RRC, "Message RRC_RAL_CONFIGURE_THRESHOLD_REQ malformed, unknown link_param_gen %d\n", link_cfg_param_p->link_param_type._union.link_param_gen);
                                return -1;
                        }
                        break;

                    case RAL_LINK_PARAM_TYPE_CHOICE_QOS:
                        switch (link_cfg_param_p->link_param_type._union.link_param_qos) {
                            case RAL_LINK_PARAM_QOS_MAX_NUM_DIF_COS_SUPPORTED:
                            case RAL_LINK_PARAM_QOS_MIN_PACKET_TRANSFER_DELAY_ALL_COS:
                            case RAL_LINK_PARAM_QOS_AVG_PACKET_TRANSFER_DELAY_ALL_COS:
                            case RAL_LINK_PARAM_QOS_MAX_PACKET_TRANSFER_DELAY_ALL_COS:
                            case RAL_LINK_PARAM_QOS_STD_DEVIATION_PACKET_TRANSFER_DELAY:
                            case RAL_LINK_PARAM_QOS_PACKET_LOSS_RATE_ALL_COS_FRAME_RATIO:
                                message_p = itti_alloc_new_message (TASK_RRC_UE, PHY_MEAS_THRESHOLD_REQ);
                                PHY_MEAS_THRESHOLD_REQ(message_p).transaction_id  = transaction_id;
                                memcpy (&PHY_MEAS_THRESHOLD_REQ(message_p).cfg_param, (void *) link_cfg_param_p, sizeof(ral_link_cfg_param_t));
                                itti_send_msg_to_task (TASK_MAC_UE, ITTI_MSG_INSTANCE(msg_pP), message_p);
                                break;
                            default:
                                LOG_E(RRC, "Message RRC_RAL_CONFIGURE_THRESHOLD_REQ malformed, unknown link_param_qos %d\n", link_cfg_param_p->link_param_type._union.link_param_qos);
                                return -1;
                        }
                        break;

                    case RAL_LINK_PARAM_TYPE_CHOICE_LTE:
                        switch (link_cfg_param_p->link_param_type._union.link_param_lte) {
                            // group by dest task id
                            case RAL_LINK_PARAM_LTE_UE_RSRP:
                            case RAL_LINK_PARAM_LTE_UE_RSRQ:
                            case RAL_LINK_PARAM_LTE_UE_CQI:
                                message_p = itti_alloc_new_message (TASK_RRC_UE, PHY_MEAS_THRESHOLD_REQ);
                                PHY_MEAS_THRESHOLD_REQ(message_p).transaction_id  = transaction_id;
                                memcpy (&PHY_MEAS_THRESHOLD_REQ(message_p).cfg_param, (void *) link_cfg_param_p, sizeof(ral_link_cfg_param_t));
                                itti_send_msg_to_task (TASK_PHY_UE, ITTI_MSG_INSTANCE(msg_pP), message_p);
                                break;

                            case RAL_LINK_PARAM_LTE_AVAILABLE_BW:
                            case RAL_LINK_PARAM_LTE_PACKET_LOSS_RATE:
                            case RAL_LINK_PARAM_LTE_L2_BUFFER_STATUS:
                            case RAL_LINK_PARAM_LTE_PACKET_DELAY:
                                message_p = itti_alloc_new_message (TASK_RRC_UE, PHY_MEAS_THRESHOLD_REQ);
                                PHY_MEAS_THRESHOLD_REQ(message_p).transaction_id  = transaction_id;
                                memcpy (&PHY_MEAS_THRESHOLD_REQ(message_p).cfg_param, (void *) link_cfg_param_p, sizeof(ral_link_cfg_param_t));
                                itti_send_msg_to_task (TASK_MAC_UE, ITTI_MSG_INSTANCE(msg_pP), message_p);
                                break;

                            case RAL_LINK_PARAM_LTE_MOBILE_NODE_CAPABILITIES:
                            case RAL_LINK_PARAM_LTE_EMBMS_CAPABILITY:
                            case RAL_LINK_PARAM_LTE_JUMBO_FEASIBILITY:
                            case RAL_LINK_PARAM_LTE_JUMBO_SETUP_STATUS:
                            case RAL_LINK_PARAM_LTE_NUM_ACTIVE_EMBMS_RECEIVERS_PER_FLOW:
#warning "TO DO MIH LTE LINK PARAMS IN RRC UE"
                                break;

                            default:
                                LOG_E(RRC, "Message RRC_RAL_CONFIGURE_THRESHOLD_REQ malformed, unknown link_param_lte %d\n", link_cfg_param_p->link_param_type._union.link_param_lte);
                                return -1;
                        }
                        break;

                    default:
                        LOG_E(RRC, "Message RRC_RAL_CONFIGURE_THRESHOLD_REQ malformed, unknown link_param_type choice %d\n", link_cfg_param_p->link_param_type.choice);
                        return -1;
                }

                for (ix_thresholds=0; ix_thresholds < link_cfg_param_p->num_thresholds; ix_thresholds++) {
                    threshold_p = &link_cfg_param_p->thresholds[ix_thresholds];
                }
                break;
            case RAL_TH_ACTION_CANCEL_THRESHOLD:
                // IEEE Std 802.21-2008, Table F4, Data type name=LINK_CFG_PARAM (page 228):
                // When “Cancel threshold” is selected and no thresholds are specified, then all
                // currently configured thresholds for the given LINK_PARAM_TYPE are cancelled.
                if (link_cfg_param_p->num_thresholds == 0) {
                    rrc_ue_ral_delete_all_thresholds_type(mod_idP, &link_cfg_param_p->link_param_type);
                } else {
                //
                // When “Cancel threshold” is selected and thresholds are specified only those
                // configured thresholds for the given LINK_PARAM_TYPE and whose threshold value match what was
                // specified are cancelled.
                    for (ix_thresholds=0; ix_thresholds < link_cfg_param_p->num_thresholds; ix_thresholds++) {
                        threshold_p = &link_cfg_param_p->thresholds[ix_thresholds];
                        rrc_ue_ral_delete_threshold(mod_idP, &link_cfg_param_p->link_param_type, threshold_p);
                    }
                }
                break;
            default:
                LOG_E(RRC, "Message RRC_RAL_CONFIGURE_THRESHOLD_REQ malformed, unknown th_action %d\n", link_cfg_param_p->th_action);
                return -1;
        }
    }
    return 0;
}


