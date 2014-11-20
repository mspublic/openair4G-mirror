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
#include <string.h>

#if defined(ENABLE_ITTI)

#include "intertask_interface.h"
#include "nas_itti_messaging.h"

#if ((defined(EPC_BUILD) && defined(NAS_MME)) || (defined(UE_BUILD) && defined(NAS_UE)))
# if (defined(EPC_BUILD) && defined(NAS_MME))
#   define TASK_ORIGIN  TASK_NAS_MME
# else
#   define TASK_ORIGIN  TASK_NAS_UE
# endif
#endif

#if ((defined(EPC_BUILD) && defined(NAS_MME)) || (defined(ENABLE_NAS_UE_LOGGING) && defined(UE_BUILD) && defined(NAS_UE)))
static const UInt8_t emm_message_ids[] =
{
    ATTACH_REQUEST,
    ATTACH_ACCEPT,
    ATTACH_COMPLETE,
    ATTACH_REJECT,
    DETACH_REQUEST,
    DETACH_ACCEPT,
    TRACKING_AREA_UPDATE_REQUEST,
    TRACKING_AREA_UPDATE_ACCEPT,
    TRACKING_AREA_UPDATE_COMPLETE,
    TRACKING_AREA_UPDATE_REJECT,
    EXTENDED_SERVICE_REQUEST,
    SERVICE_REQUEST,
    SERVICE_REJECT,
    GUTI_REALLOCATION_COMMAND,
    GUTI_REALLOCATION_COMPLETE,
    AUTHENTICATION_REQUEST,
    AUTHENTICATION_RESPONSE,
    AUTHENTICATION_REJECT,
    AUTHENTICATION_FAILURE,
    IDENTITY_REQUEST,
    IDENTITY_RESPONSE,
    SECURITY_MODE_COMMAND,
    SECURITY_MODE_COMPLETE,
    SECURITY_MODE_REJECT,
    EMM_STATUS,
    EMM_INFORMATION,
    DOWNLINK_NAS_TRANSPORT,
    UPLINK_NAS_TRANSPORT,
    CS_SERVICE_NOTIFICATION,
};

static const UInt8_t esm_message_ids[] =
{
    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST,
    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT,
    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REJECT,
    ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REQUEST,
    ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_ACCEPT,
    ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REJECT,
    MODIFY_EPS_BEARER_CONTEXT_REQUEST,
    MODIFY_EPS_BEARER_CONTEXT_ACCEPT,
    MODIFY_EPS_BEARER_CONTEXT_REJECT,
    DEACTIVATE_EPS_BEARER_CONTEXT_REQUEST,
    DEACTIVATE_EPS_BEARER_CONTEXT_ACCEPT,
    PDN_CONNECTIVITY_REQUEST,
    PDN_CONNECTIVITY_REJECT,
    PDN_DISCONNECT_REQUEST,
    PDN_DISCONNECT_REJECT,
    BEARER_RESOURCE_ALLOCATION_REQUEST,
    BEARER_RESOURCE_ALLOCATION_REJECT,
    BEARER_RESOURCE_MODIFICATION_REQUEST,
    BEARER_RESOURCE_MODIFICATION_REJECT,
    ESM_INFORMATION_REQUEST,
    ESM_INFORMATION_RESPONSE,
    ESM_STATUS,
};

static int _nas_find_message_index(const UInt8_t message_id, const UInt8_t *message_ids, const int ids_number)
{
    int i;

    for(i = 0; i < ids_number; i ++)
    {
        if (message_id == message_ids[i])
        {
            return (2 + i);
        }
    }

    return (1);
}

int nas_itti_plain_msg(const char *buffer, const nas_message_t *msg, const int length, const int down_link)
{
    MessageDef *message_p;
    int data_length = length < NAS_DATA_LENGHT_MAX ? length : NAS_DATA_LENGHT_MAX;
    int message_type = -1;
    MessagesIds messageId_raw = -1;
    MessagesIds messageId_plain = -1;

    /* Define message ids */
    if (msg->header.protocol_discriminator == EPS_MOBILITY_MANAGEMENT_MESSAGE)
    {
        message_type    = 0;
        messageId_raw   = down_link ? NAS_DL_EMM_RAW_MSG : NAS_UL_EMM_RAW_MSG;
        messageId_plain = down_link ? NAS_DL_EMM_PLAIN_MSG : NAS_UL_EMM_PLAIN_MSG;
    }
    else
    {
        if (msg->header.protocol_discriminator == EPS_SESSION_MANAGEMENT_MESSAGE)
        {
            message_type    = 1;
            messageId_raw   = down_link ? NAS_DL_ESM_RAW_MSG : NAS_UL_ESM_RAW_MSG;
            messageId_plain = down_link ? NAS_DL_ESM_PLAIN_MSG : NAS_UL_ESM_PLAIN_MSG;
        }
    }

    if (message_type >= 0)
    {
        /* Create and send the RAW message */
        message_p = itti_alloc_new_message(TASK_ORIGIN, messageId_raw);

        NAS_DL_EMM_RAW_MSG(message_p).lenght = length;
        memset ((void *) &(NAS_DL_EMM_RAW_MSG(message_p).data), 0, NAS_DATA_LENGHT_MAX);
        memcpy ((void *) &(NAS_DL_EMM_RAW_MSG(message_p).data), buffer, data_length);

        itti_send_msg_to_task(TASK_UNKNOWN, INSTANCE_DEFAULT, message_p);

        /* Create and send the plain message */
        if (message_type == 0)
        {
            message_p = itti_alloc_new_message(TASK_ORIGIN, messageId_plain);

            NAS_DL_EMM_PLAIN_MSG(message_p).present = _nas_find_message_index(msg->plain.emm.header.message_type, emm_message_ids, sizeof(emm_message_ids) / sizeof(emm_message_ids[0]));
            memcpy ((void *) &(NAS_DL_EMM_PLAIN_MSG(message_p).choice), &msg->plain.emm, sizeof (EMM_msg));
        }
        else {
            message_p = itti_alloc_new_message(TASK_ORIGIN, messageId_plain);

            NAS_DL_ESM_PLAIN_MSG(message_p).present = _nas_find_message_index(msg->plain.esm.header.message_type, esm_message_ids, sizeof(esm_message_ids) / sizeof(esm_message_ids[0]));
            memcpy ((void *) &(NAS_DL_ESM_PLAIN_MSG(message_p).choice), &msg->plain.esm, sizeof (ESM_msg));
        }

        return itti_send_msg_to_task(TASK_UNKNOWN, INSTANCE_DEFAULT, message_p);
    }

    return EXIT_FAILURE;
}

int nas_itti_protected_msg(const char *buffer, const nas_message_t *msg, const int length, const int down_link)
{
    MessageDef *message_p = NULL;

    if (msg->header.protocol_discriminator == EPS_MOBILITY_MANAGEMENT_MESSAGE)
    {
        message_p = itti_alloc_new_message(TASK_ORIGIN, down_link ? NAS_DL_EMM_PROTECTED_MSG : NAS_UL_EMM_PROTECTED_MSG);

        memcpy ((void *) &(NAS_DL_EMM_PROTECTED_MSG(message_p).header), &msg->header, sizeof (nas_message_security_header_t));
        NAS_DL_EMM_PROTECTED_MSG(message_p).present = _nas_find_message_index(msg->security_protected.plain.emm.header.message_type, emm_message_ids, sizeof(emm_message_ids) / sizeof(emm_message_ids[0]));
        memcpy ((void *) &(NAS_DL_EMM_PROTECTED_MSG(message_p).choice), &msg->security_protected.plain.emm, sizeof (EMM_msg));
    }
    else {
        if (msg->header.protocol_discriminator == EPS_SESSION_MANAGEMENT_MESSAGE)
        {
            message_p = itti_alloc_new_message(TASK_ORIGIN, down_link ? NAS_DL_ESM_PROTECTED_MSG : NAS_UL_ESM_PROTECTED_MSG);

            memcpy ((void *) &(NAS_DL_ESM_PROTECTED_MSG(message_p).header), &msg->header, sizeof (nas_message_security_header_t));
            NAS_DL_ESM_PROTECTED_MSG(message_p).present =  _nas_find_message_index(msg->security_protected.plain.esm.header.message_type, esm_message_ids, sizeof(esm_message_ids) / sizeof(esm_message_ids[0]));
            memcpy ((void *) &(NAS_DL_ESM_PROTECTED_MSG(message_p).choice), &msg->security_protected.plain.esm, sizeof (ESM_msg));
        }
    }

    if (message_p != NULL) {
        return itti_send_msg_to_task(TASK_UNKNOWN, INSTANCE_DEFAULT, message_p);
    }

    return EXIT_FAILURE;
}
#endif

#if defined(EPC_BUILD) && defined(NAS_MME)
int nas_itti_dl_data_req(const uint32_t ue_id, void *const data,
                         const uint32_t length)
{
    MessageDef *message_p;

    message_p = itti_alloc_new_message(TASK_NAS_MME, NAS_DOWNLINK_DATA_REQ);

    NAS_DL_DATA_REQ(message_p).UEid          = ue_id;
    NAS_DL_DATA_REQ(message_p).nasMsg.data   = data;
    NAS_DL_DATA_REQ(message_p).nasMsg.length = length;

    return itti_send_msg_to_task(TASK_S1AP, INSTANCE_DEFAULT, message_p);
}

#endif

#if defined(UE_BUILD) && defined(NAS_UE)
extern unsigned char NB_eNB_INST;

int nas_itti_cell_info_req(const plmn_t plmnID, const Byte_t rat)
{
    MessageDef *message_p;

    message_p = itti_alloc_new_message(TASK_NAS_UE, NAS_CELL_SELECTION_REQ);

    NAS_CELL_SELECTION_REQ(message_p).plmnID    = plmnID;
    NAS_CELL_SELECTION_REQ(message_p).rat       = rat;

    return itti_send_msg_to_task(TASK_RRC_UE, NB_eNB_INST + 0 /* TODO to be virtualized */, message_p);
}

int nas_itti_nas_establish_req(as_cause_t cause, as_call_type_t type, as_stmsi_t s_tmsi, plmn_t plmnID, Byte_t *data, UInt32_t length)
{
    MessageDef *message_p;

    message_p = itti_alloc_new_message(TASK_NAS_UE, NAS_CONN_ESTABLI_REQ);

    NAS_CONN_ESTABLI_REQ(message_p).cause                       = cause;
    NAS_CONN_ESTABLI_REQ(message_p).type                        = type;
    NAS_CONN_ESTABLI_REQ(message_p).s_tmsi                      = s_tmsi;
    NAS_CONN_ESTABLI_REQ(message_p).plmnID                      = plmnID;
    NAS_CONN_ESTABLI_REQ(message_p).initialNasMsg.data          = data;
    NAS_CONN_ESTABLI_REQ(message_p).initialNasMsg.length        = length;

    return itti_send_msg_to_task(TASK_RRC_UE, NB_eNB_INST + 0 /* TODO to be virtualized */, message_p);
}

int nas_itti_ul_data_req(const uint32_t ue_id, void *const data, const uint32_t length)
{
    MessageDef *message_p;

    message_p = itti_alloc_new_message(TASK_NAS_UE, NAS_UPLINK_DATA_REQ);

    NAS_UPLINK_DATA_REQ(message_p).UEid          = ue_id;
    NAS_UPLINK_DATA_REQ(message_p).nasMsg.data   = data;
    NAS_UPLINK_DATA_REQ(message_p).nasMsg.length = length;

    return itti_send_msg_to_task(TASK_RRC_UE, NB_eNB_INST + 0 /* TODO to be virtualized */, message_p);
}

int nas_itti_rab_establish_rsp(const as_stmsi_t s_tmsi, const as_rab_id_t rabID, const nas_error_code_t errCode)
{
    MessageDef *message_p;

    message_p = itti_alloc_new_message(TASK_NAS_UE, NAS_RAB_ESTABLI_RSP);

    NAS_RAB_ESTABLI_RSP(message_p).s_tmsi       = s_tmsi;
    NAS_RAB_ESTABLI_RSP(message_p).rabID        = rabID;
    NAS_RAB_ESTABLI_RSP(message_p).errCode      = errCode;

    return itti_send_msg_to_task(TASK_RRC_UE, NB_eNB_INST + 0 /* TODO to be virtualized */, message_p);
}
#endif

#endif
