#define MRAL_MODULE
#define MRALLTEINT_ACTION_C
#include <assert.h>
#include "mRALlteInt_action.h"

//-----------------------------------------------------------------------------
void mRALlte_action_request(MIH_C_Message_Link_Action_request_t* messageP) {
//-----------------------------------------------------------------------------
    MIH_C_STATUS_T                      status;
    LIST(MIH_C_LINK_SCAN_RSP,           scan_response_set);
    MIH_C_LINK_AC_RESULT_T              link_action_result;


    memcpy(&g_link_action, &messageP->primitive.LinkAction, sizeof(MIH_C_LINK_ACTION_T));

    status             = MIH_C_STATUS_SUCCESS;
    link_action_result = MIH_C_LINK_AC_RESULT_SUCCESS;

    if ( messageP->primitive.LinkAction.link_ac_attr & MIH_C_BIT_LINK_AC_ATTR_LINK_SCAN) {
    }
    if ( messageP->primitive.LinkAction.link_ac_attr & MIH_C_BIT_LINK_AC_ATTR_LINK_RES_RETAIN) {
    }
    if ( messageP->primitive.LinkAction.link_ac_attr & MIH_C_BIT_LINK_AC_ATTR_DATA_FWD_REQ) {
    }


    switch (messageP->primitive.LinkAction.link_ac_type) {
        case MIH_C_LINK_AC_TYPE_NONE:
            ERR("%s ACTION REQUESTED: MIH_C_LINK_AC_TYPE_NONE\n", __FUNCTION__);

            break;

        case MIH_C_LINK_AC_TYPE_LINK_DISCONNECT:
            ERR("%s ACTION REQUESTED: MIH_C_LINK_AC_TYPE_LINK_DISCONNECT\n", __FUNCTION__);
            mRALlte_send_link_action_confirm(&messageP->header.transaction_id, &status, &scan_response_set_list, &link_action_result);
            break;

        case MIH_C_LINK_AC_TYPE_LINK_LOW_POWER:
            ERR("%s ACTION REQUESTED: MIH_C_LINK_AC_TYPE_LINK_LOW_POWER\n", __FUNCTION__);
            mRALlte_send_link_action_confirm(&messageP->header.transaction_id, &status, &scan_response_set_list, &link_action_result);
            break;

        case MIH_C_LINK_AC_TYPE_LINK_POWER_DOWN:
            ERR("%s ACTION REQUESTED: MIH_C_LINK_AC_TYPE_LINK_POWER_DOWN\n", __FUNCTION__);
            mRALlte_send_link_action_confirm(&messageP->header.transaction_id, &status, &scan_response_set_list, &link_action_result);
            break;

        case MIH_C_LINK_AC_TYPE_LINK_POWER_UP:
            ERR("%s ACTION REQUESTED: MIH_C_LINK_AC_TYPE_LINK_POWER_UP\n", __FUNCTION__);
            mRALlte_send_link_action_confirm(&messageP->header.transaction_id, &status, &scan_response_set_list, &link_action_result);
            break;

        case MIH_C_LINK_AC_TYPE_LINK_FLOW_ATTR:
            ERR("%s ACTION REQUESTED: MIH_C_LINK_AC_TYPE_LINK_FLOW_ATTR\n", __FUNCTION__);
            mRALlte_send_link_action_confirm(&messageP->header.transaction_id, &status, &scan_response_set_list, &link_action_result);
            break;

        case MIH_C_LINK_AC_TYPE_LINK_ACTIVATE_RESOURCES:
            ERR("%s ACTION REQUESTED: MIH_C_LINK_AC_TYPE_LINK_ACTIVATE_RESOURCES\n", __FUNCTION__);
            mRALlte_send_link_action_confirm(&messageP->header.transaction_id, &status, &scan_response_set_list, &link_action_result);
            break;

        case MIH_C_LINK_AC_TYPE_LINK_DEACTIVATE_RESOURCES:
            ERR("%s ACTION REQUESTED: MIH_C_LINK_AC_TYPE_LINK_DEACTIVATE_RESOURCES\n", __FUNCTION__);
            mRALlte_send_link_action_confirm(&messageP->header.transaction_id, &status, &scan_response_set_list, &link_action_result);
            break;

        default:
            ERR("%s Invalid LinkAction.link_ac_type in MIH_C_Message_Link_Action_request\n", __FUNCTION__);
            status = MIH_C_STATUS_UNSPECIFIED_FAILURE;
            mRALlte_send_link_action_confirm(&messageP->header.transaction_id, &status, &scan_response_set_list, &link_action_result);
    }
}

