#define MRAL_MODULE
#define MRALLTE_THRESHOLDS_C
#include <assert.h>
#include "mRALlte_thresholds.h"
#include "mRALlte_variables.h"

//-----------------------------------------------------------------------------
void mRALlte_configure_thresholds_request(MIH_C_Message_Link_Configure_Thresholds_request_t* messageP) {
//-----------------------------------------------------------------------------
    MIH_C_STATUS_T                      status;
    MIH_C_LINK_CFG_STATUS_LIST_T        link_cfg_status_list;
    unsigned int                        threshold_index;
    unsigned int                        link_index;
    unsigned int                        result_index;

    // SAVE REQUEST
    // MAY BE MERGE REQUESTS ?
    memcpy(&ralpriv->mih_link_cfg_param_thresholds_list, &messageP->primitive.LinkConfigureParameterList_list, sizeof(MIH_C_LINK_CFG_PARAM_LIST_T));

    status = MIH_C_STATUS_SUCCESS;

    result_index = 0;

    for (link_index = 0;
         link_index < messageP->primitive.LinkConfigureParameterList_list.length;
         link_index++) {

        for (threshold_index = 0;
            threshold_index < messageP->primitive.LinkConfigureParameterList_list.val[link_index].threshold_list.length;
            threshold_index ++) {

            memcpy(&link_cfg_status_list.val[result_index].link_param_type,
            &messageP->primitive.LinkConfigureParameterList_list.val[link_index].link_param_type,
            sizeof(MIH_C_LINK_PARAM_TYPE_T));

            memcpy(&link_cfg_status_list.val[result_index].threshold,
            &messageP->primitive.LinkConfigureParameterList_list.val[link_index].threshold_list.val[threshold_index],
            sizeof(MIH_C_THRESHOLD_T));

             // ALWAYS SAY OK FOR PARAMETERS
            link_cfg_status_list.val[result_index].config_status = MIH_C_CONFIG_STATUS_SUCCESS;

            result_index += 1;
        }
    }
    link_cfg_status_list.length = result_index;

    mRALte_send_configure_thresholds_confirm(&messageP->header.transaction_id,&status, &link_cfg_status_list);
}

