#define MRAL_MODULE
#define MRALLTEINT_MIH_FSM_C
#include "mRALlteInt_mih_fsm.h"

//-----------------------------------------------------------------------------
void mRALlte_mih_fsm(MIH_C_Message_Wrapper_t* message_wrapperP, int statusP) {
//-----------------------------------------------------------------------------
    MIH_C_TRANSACTION_ID_T                   transaction_id;
    MIH_C_STATUS_T                           status;
    MIH_C_LINK_EVENT_LIST_T                  supported_link_event_list;
    MIH_C_LINK_CMD_LIST_T                    supported_link_command_list;
    MIH_C_LINK_DET_INFO_T                    link_detected_info;
    MIH_C_LINK_TUPLE_ID_T                    link_identifier;
    MIH_C_LINK_PARAM_RPT_LIST_T              link_parameters_report_list;
    MIH_C_LINK_GD_REASON_T                   link_going_down_reason;
    MIH_C_UNSIGNED_INT2_T                    time_interval;
    MIH_C_LINK_DN_REASON_T                   reason_code;

    static int                               link_detected_indication_sent = 0;

    switch (g_mih_fsm_state) {
        case MIH_FSM_STATE_INIT:
            g_mih_fsm_state = MIH_FSM_STATE_REGISTER_SENT;
            transaction_id = (MIH_C_TRANSACTION_ID_T)0;
            mRALlte_send_link_register_indication(&transaction_id);
            break;

        case MIH_FSM_STATE_REGISTER_SENT:
            if (message_wrapperP == NULL) return;
            switch (message_wrapperP->message_id) {
                case MIH_C_MESSAGE_LINK_CAPABILITY_DISCOVER_REQUEST_ID:
                    if (statusP == MIH_MESSAGE_DECODE_OK) {
                        MIH_C_Link_Message_Link_Capability_Discover_request2String(&message_wrapperP->_union_message.link_capability_discover_request, g_msg_print_buffer);
                        DEBUG("%s", g_msg_print_buffer);

                        g_mih_fsm_state = MIH_FSM_STATE_DISCOVERED;

                        status = MIH_C_STATUS_SUCCESS;

                        supported_link_event_list =  MIH_C_BIT_LINK_DETECTED | MIH_C_BIT_LINK_UP | \
                                                 MIH_C_BIT_LINK_DOWN     | MIH_C_BIT_LINK_PARAMETERS_REPORT | \
                                                 MIH_C_BIT_LINK_GOING_DOWN;

                        supported_link_command_list =  MIH_C_BIT_LINK_EVENT_SUBSCRIBE | MIH_C_BIT_LINK_EVENT_UNSUBSCRIBE | \
                                                   MIH_C_BIT_LINK_GET_PARAMETERS  | MIH_C_BIT_LINK_CONFIGURE_THRESHOLDS | \
                                                   MIH_C_BIT_LINK_ACTION;

                        mRALte_send_capability_discover_confirm(&message_wrapperP->_union_message.link_capability_discover_request.header.transaction_id,
                                                                &status,
                                                                &supported_link_event_list,
                                                                &supported_link_command_list);
                    }
                    break;
                case MIH_C_MESSAGE_LINK_EVENT_SUBSCRIBE_REQUEST_ID:
                case MIH_C_MESSAGE_LINK_EVENT_UNSUBSCRIBE_REQUEST_ID:
                case MIH_C_MESSAGE_LINK_GET_PARAMETERS_REQUEST_ID:
                case MIH_C_MESSAGE_LINK_CONFIGURE_THRESHOLDS_REQUEST_ID:
                case MIH_C_MESSAGE_LINK_ACTION_REQUEST_ID:
                    DEBUG("%s IGNORING MESSAGE ID %d - RE-SEND Link_Register.indication\n", __FUNCTION__, message_wrapperP->message_id);
                    transaction_id = (MIH_C_TRANSACTION_ID_T)0;
                    mRALlte_send_link_register_indication(&transaction_id);
                    break;
                default:
                    DEBUG("%s IGNORING UNKNOWN MESSAGE ID %d \n", __FUNCTION__, message_wrapperP->message_id);
                    transaction_id = (MIH_C_TRANSACTION_ID_T)0;
                    mRALlte_send_link_register_indication(&transaction_id);
            }
            break;

        case MIH_FSM_STATE_DISCOVERED:
            if (message_wrapperP == NULL) return;
            switch (message_wrapperP->message_id) {
                case MIH_C_MESSAGE_LINK_CAPABILITY_DISCOVER_REQUEST_ID:
                    if (statusP == MIH_MESSAGE_DECODE_OK) {
                        MIH_C_Link_Message_Link_Capability_Discover_request2String(&message_wrapperP->_union_message.link_capability_discover_request, g_msg_print_buffer);
                        DEBUG("%s", g_msg_print_buffer);

                        status = MIH_C_STATUS_SUCCESS;

                        supported_link_event_list =  MIH_C_BIT_LINK_DETECTED | MIH_C_BIT_LINK_UP | \
                                                 MIH_C_BIT_LINK_DOWN     | MIH_C_BIT_LINK_PARAMETERS_REPORT | \
                                                 MIH_C_BIT_LINK_GOING_DOWN;

                        supported_link_command_list =  MIH_C_BIT_LINK_EVENT_SUBSCRIBE | MIH_C_BIT_LINK_EVENT_UNSUBSCRIBE | \
                                                   MIH_C_BIT_LINK_GET_PARAMETERS  | MIH_C_BIT_LINK_CONFIGURE_THRESHOLDS | \
                                                   MIH_C_BIT_LINK_ACTION;

                        mRALte_send_capability_discover_confirm(&message_wrapperP->_union_message.link_capability_discover_request.header.transaction_id,
                                                                &status,
                                                                &supported_link_event_list,
                                                                &supported_link_command_list);
                    }
                    break;

                case MIH_C_MESSAGE_LINK_EVENT_SUBSCRIBE_REQUEST_ID:
                    if (statusP == MIH_MESSAGE_DECODE_OK) {
                        MIH_C_Link_Message_Link_Event_Subscribe_request2String(&message_wrapperP->_union_message.link_event_subscribe_request, g_msg_print_buffer);
                        DEBUG("%s", g_msg_print_buffer);

                        status = MIH_C_STATUS_SUCCESS;

                        mRALte_send_event_subscribe_confirm(&message_wrapperP->_union_message.link_event_subscribe_request.header.transaction_id,
                                                            &status,
                                                            &message_wrapperP->_union_message.link_event_subscribe_request.primitive.RequestedLinkEventList);
                    }
                    break;

                case MIH_C_MESSAGE_LINK_EVENT_UNSUBSCRIBE_REQUEST_ID:
                    if (statusP == MIH_MESSAGE_DECODE_OK) {
                        MIH_C_Link_Message_Link_Event_Unsubscribe_request2String(&message_wrapperP->_union_message.link_event_unsubscribe_request, g_msg_print_buffer);
                        DEBUG("%s", g_msg_print_buffer);

                        status = MIH_C_STATUS_SUCCESS;

                        mRALte_send_event_unsubscribe_confirm(&message_wrapperP->_union_message.link_event_unsubscribe_request.header.transaction_id,
                                                            &status,
                                                            &message_wrapperP->_union_message.link_event_unsubscribe_request.primitive.RequestedLinkEventList);
                    }

                case MIH_C_MESSAGE_LINK_CONFIGURE_THRESHOLDS_REQUEST_ID:
                    if (statusP == MIH_MESSAGE_DECODE_OK) {
                        MIH_C_Link_Message_Link_Configure_Thresholds_request2String(&message_wrapperP->_union_message.link_configure_thresholds_request, g_msg_print_buffer);
                        DEBUG("%s", g_msg_print_buffer);

                        mRALlte_configure_thresholds_request(&message_wrapperP->_union_message.link_configure_thresholds_request);
                    }
                    break;

                case MIH_C_MESSAGE_LINK_GET_PARAMETERS_REQUEST_ID:
                    if (statusP == MIH_MESSAGE_DECODE_OK) {
                        MIH_C_Link_Message_Link_Get_Parameters_request2String(&message_wrapperP->_union_message.link_get_parameters_request, g_msg_print_buffer);
                        DEBUG("%s", g_msg_print_buffer);

                        if (link_detected_indication_sent == 0) {
                            g_mih_fsm_state = MIH_FSM_STATE_SEND_LINK_DETECTED_INDICATION;
                            link_detected_indication_sent = 1;
                        }

                        mRALlte_get_parameters_request(&message_wrapperP->_union_message.link_get_parameters_request);
                    }
                    break;


                case MIH_C_MESSAGE_LINK_ACTION_REQUEST_ID:
                    if (statusP == MIH_MESSAGE_DECODE_OK) {
                        //MIH_C_Link_Message_Link_Action_request2String(&message_wrapperP->_union_message.link_get_parameters_request, g_msg_print_buffer);
                        //DEBUG("%s", g_msg_print_buffer);

                        if (message_wrapperP->_union_message.link_action_request.primitive.LinkAction.link_ac_type == MIH_C_LINK_AC_TYPE_LINK_POWER_UP) {
                            g_mih_fsm_state = MIH_FSM_STATE_SEND_LINK_UP_INDICATION;
                        } else if (message_wrapperP->_union_message.link_action_request.primitive.LinkAction.link_ac_type == MIH_C_LINK_AC_TYPE_LINK_POWER_DOWN) {
                            g_mih_fsm_state = MIH_FSM_STATE_SEND_LINK_DOWN_INDICATION;
                        }


                        mRALlte_action_request(&message_wrapperP->_union_message.link_action_request);
                    }
                    break;

                default:
                    DEBUG("%s IGNORING UNKNOWN MESSAGE ID %d \n", __FUNCTION__, message_wrapperP->message_id);
                    transaction_id = (MIH_C_TRANSACTION_ID_T)0;
                    mRALlte_send_link_register_indication(&transaction_id);
            }
            break;


        case MIH_FSM_STATE_SEND_LINK_DETECTED_INDICATION:

            if (message_wrapperP == NULL) {
                g_mih_fsm_state = MIH_FSM_STATE_DISCOVERED;

                link_detected_info.link_tuple_id.link_id.link_type = MIH_C_WIRELESS_UMTS;
                link_detected_info.link_tuple_id.link_id.link_addr.choice = (MIH_C_CHOICE_T)MIH_C_CHOICE_3GPP_ADDR;
                MIH_C_3GPP_ADDR_load_3gpp_str_address(&link_detected_info.link_tuple_id.link_id.link_addr._union._3gpp_addr,
                                                      (u_int8_t*)DEFAULT_ADDRESS_3GPP);
                link_detected_info.link_tuple_id.choice = MIH_C_LINK_TUPLE_ID_CHOICE_NULL;
                //link_detected_info.link_tuple_id._union.link_addr._union._3gpp_addr = ;

                MIH_C_NETWORK_ID_set(&link_detected_info.network_id , (u_int8_t*)PREDEFINED_MIH_NETWORK_ID, strlen(PREDEFINED_MIH_NETWORK_ID));

                MIH_C_NET_AUX_ID_set(&link_detected_info.net_aux_id , (u_int8_t*)PREDEFINED_MIH_NETAUX_ID, strlen(PREDEFINED_MIH_NETAUX_ID));

                link_detected_info.sig_strength.choice     = MIH_C_SIG_STRENGTH_CHOICE_DBM;
                link_detected_info.sig_strength._union.dbm = PREDEFINED_LINK_DETECTED_INDICATION_SIG_STRENGTH;

                link_detected_info.sinr                    = (MIH_C_UNSIGNED_INT2_T)PREDEFINED_LINK_DETECTED_INDICATION_SINR;

                link_detected_info.link_data_rate          = (MIH_C_LINK_DATA_RATE_T)PREDEFINED_LINK_DETECTED_INDICATION_LINK_DATA_RATE;

                link_detected_info.link_mihcap_flag        = (MIH_C_LINK_MIHCAP_FLAG_T)(MIH_C_BIT_EVENT_SERVICE_SUPPORTED   |
                                                                MIH_C_BIT_COMMAND_SERVICE_SUPPORTED |
                                                                MIH_C_BIT_INFORMATION_SERVICE_SUPPORTED);

                link_detected_info.net_caps                = (MIH_C_NET_CAPS_T)(MIH_C_BIT_NET_CAPS_QOS_CLASS0 |
                                                               MIH_C_BIT_NET_CAPS_QOS_CLASS1 |
                                                               MIH_C_BIT_NET_CAPS_INTERNET_ACCESS);

                mRALlte_send_link_detected_indication(&transaction_id, &link_detected_info);
            }
            break;

        case MIH_FSM_STATE_SEND_LINK_UP_INDICATION:
            if (message_wrapperP == NULL) {
                g_mih_fsm_state = MIH_FSM_STATE_SEND_LINK_PARAMETERS_REPORT_INDICATION;

                link_identifier.link_id.link_type        = MIH_C_WIRELESS_UMTS;
                link_identifier.link_id.link_addr.choice = (MIH_C_CHOICE_T)MIH_C_CHOICE_3GPP_ADDR;
                MIH_C_3GPP_ADDR_load_3gpp_str_address(&link_identifier.link_id.link_addr._union._3gpp_addr, (u_int8_t*)DEFAULT_ADDRESS_3GPP);
                link_identifier.choice                   = MIH_C_LINK_TUPLE_ID_CHOICE_NULL;
                transaction_id                           = MIH_C_get_new_transaction_id();
                mRALlte_send_link_up_indication(&transaction_id, &link_identifier, NULL, NULL, NULL, NULL);
            }
            break;

        case MIH_FSM_STATE_SEND_LINK_PARAMETERS_REPORT_INDICATION:
            if (message_wrapperP == NULL) {
                g_mih_fsm_state = MIH_FSM_STATE_SEND_LINK_GOING_DOWN_INDICATION;

                link_identifier.link_id.link_type        = MIH_C_WIRELESS_UMTS;
                link_identifier.link_id.link_addr.choice = (MIH_C_CHOICE_T)MIH_C_CHOICE_3GPP_ADDR;
                MIH_C_3GPP_ADDR_load_3gpp_str_address(&link_identifier.link_id.link_addr._union._3gpp_addr, (u_int8_t*)DEFAULT_ADDRESS_3GPP);
                link_identifier.choice                   = MIH_C_LINK_TUPLE_ID_CHOICE_NULL;

                link_parameters_report_list.val[0].link_param.link_param_type.choice                = MIH_C_LINK_PARAM_TYPE_CHOICE_GEN;
                link_parameters_report_list.val[0].link_param.link_param_type._union.link_param_gen = MIH_C_LINK_PARAM_GEN_DATA_RATE;
                link_parameters_report_list.val[0].link_param.choice                                = MIH_C_LINK_PARAM_CHOICE_LINK_PARAM_VAL;
                link_parameters_report_list.val[0].link_param._union.link_param_val                 = 100;

                link_parameters_report_list.val[0].choice                          = MIH_C_LINK_PARAM_RPT_CHOICE_NULL;
                //link_parameters_report_list.val[0]._union.threshold.threshold_val  = 100;
                //link_parameters_report_list.val[0]._union.threshold.threshold_xdir = MIH_C_BELOW_THRESHOLD;
                link_parameters_report_list.length                                 = 1;

                transaction_id                           = MIH_C_get_new_transaction_id();
                mRALlte_send_link_parameters_report_indication(&transaction_id, &link_identifier, &link_parameters_report_list);
            }
            break;

        case MIH_FSM_STATE_SEND_LINK_GOING_DOWN_INDICATION:
            if (message_wrapperP == NULL) {
                g_mih_fsm_state = MIH_FSM_STATE_DISCOVERED;

                link_identifier.link_id.link_type        = MIH_C_WIRELESS_UMTS;
                link_identifier.link_id.link_addr.choice = (MIH_C_CHOICE_T)MIH_C_CHOICE_3GPP_ADDR;
                MIH_C_3GPP_ADDR_load_3gpp_str_address(&link_identifier.link_id.link_addr._union._3gpp_addr, (u_int8_t*)DEFAULT_ADDRESS_3GPP);
                link_identifier.choice                   = MIH_C_LINK_TUPLE_ID_CHOICE_NULL;

                time_interval = 50; // ms

                link_going_down_reason = MIH_C_LINK_GOING_DOWN_REASON_LINK_PARAMETER_DEGRADING;

                transaction_id                           = MIH_C_get_new_transaction_id();
                mRALlte_send_link_going_down_indication(&transaction_id, &link_identifier, &time_interval, &link_going_down_reason);
            }
            break;

        case MIH_FSM_STATE_SEND_LINK_DOWN_INDICATION:
            if (message_wrapperP == NULL) {
                g_mih_fsm_state = MIH_FSM_STATE_DISCOVERED;

                link_identifier.link_id.link_type        = MIH_C_WIRELESS_UMTS;
                link_identifier.link_id.link_addr.choice = (MIH_C_CHOICE_T)MIH_C_CHOICE_3GPP_ADDR;
                MIH_C_3GPP_ADDR_load_3gpp_str_address(&link_identifier.link_id.link_addr._union._3gpp_addr, (u_int8_t*)DEFAULT_ADDRESS_3GPP);
                link_identifier.choice                   = MIH_C_LINK_TUPLE_ID_CHOICE_NULL;

                transaction_id                           = MIH_C_get_new_transaction_id();

                reason_code                              = MIH_C_LINK_DOWN_REASON_AUTHENTICATION_FAILURE;

                mRALlte_send_link_down_indication(&transaction_id, &link_identifier, NULL, &reason_code);
            }
            break;


        default:
            if (message_wrapperP == NULL) return;
    }
}