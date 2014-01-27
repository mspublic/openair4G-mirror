/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2013 Eurecom

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
  Forums       : http://forums.eurecom.fr/openairinterface
  Address      : EURECOM, Campus SophiaTech, 450 Route des Chappes
                 06410 Biot FRANCE

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "intertask_interface.h"
#include "mme_app_itti_messaging.h"

#include "mme_config.h"

#include "mme_app_ue_context.h"
#include "mme_app_defs.h"

#include "assertions.h"

static
int mme_app_request_authentication_info(const mme_app_imsi_t imsi,
                                        const uint8_t nb_of_vectors,
                                        const plmn_t *plmn,
                                        const uint8_t *auts);

static
int mme_app_request_authentication_info(const mme_app_imsi_t imsi,
                                        const uint8_t nb_of_vectors,
                                        const plmn_t *plmn,
                                        const uint8_t *auts)
{
    s6a_auth_info_req_t *auth_info_req;
    MessageDef          *message_p;

    DevAssert(plmn != NULL);

    message_p = itti_alloc_new_message(TASK_MME_APP, S6A_AUTH_INFO_REQ);

    auth_info_req = &message_p->ittiMsg.s6a_auth_info_req;
    MME_APP_IMSI_TO_STRING(imsi, auth_info_req->imsi);
    memcpy(&auth_info_req->visited_plmn, plmn, sizeof(plmn_t));
    auth_info_req->nb_of_vectors = nb_of_vectors;
    if (auts != NULL) {
        auth_info_req->re_synchronization = 1;
        memcpy(auth_info_req->auts, auts, sizeof(auth_info_req->auts));
    } else {
        auth_info_req->re_synchronization = 0;
        memset(auth_info_req->auts, 0, sizeof(auth_info_req->auts));
    }

    return itti_send_msg_to_task(TASK_S6A, INSTANCE_DEFAULT, message_p);
}

int mme_app_handle_nas_auth_resp(nas_auth_resp_t *nas_auth_resp_p)
{
    struct ue_context_s *ue_context;
    uint64_t imsi;

    DevAssert(nas_auth_resp_p != NULL);

    MME_APP_STRING_TO_IMSI((char *)nas_auth_resp_p->imsi, &imsi);

    MME_APP_DEBUG("Handling imsi %"IMSI_FORMAT"\n", imsi);

    if ((ue_context = mme_ue_context_exists_imsi(&mme_app_desc.mme_ue_contexts,
                      imsi)) == NULL) {
        MME_APP_ERROR("That's embarrassing as we don't know this IMSI\n");
        return -1;
    }

    /* Consider the UE authenticated */
    ue_context->imsi_auth = IMSI_AUTHENTICATED;

    /* TODO: Get keys... */

    /* Now generate S6A ULR */
    {
        MessageDef                *message_p;
        s6a_update_location_req_t *s6a_ulr;

        message_p = itti_alloc_new_message(TASK_MME_APP, S6A_UPDATE_LOCATION_REQ);

        if (message_p == NULL) {
            return -1;
        }

        s6a_ulr = &message_p->ittiMsg.s6a_update_location_req;

        memcpy(s6a_ulr->imsi, nas_auth_resp_p->imsi, 16);
        s6a_ulr->initial_attach = INITIAL_ATTACH;
        s6a_ulr->rat_type = RAT_EUTRAN;
        /* Check if we already have UE data */
        s6a_ulr->skip_subsriber_data = 0;

        return itti_send_msg_to_task(TASK_S6A, INSTANCE_DEFAULT, message_p);
    }
    return -1;
}

int mme_app_handle_authentication_info_answer(s6a_auth_info_ans_t
        *s6a_auth_info_ans_p)
{
    struct ue_context_s *ue_context;
    uint64_t imsi;

    DevAssert(s6a_auth_info_ans_p != NULL);

    MME_APP_STRING_TO_IMSI((char *)s6a_auth_info_ans_p->imsi, &imsi);

    MME_APP_DEBUG("Handling imsi %"IMSI_FORMAT"\n", imsi);

    if ((ue_context = mme_ue_context_exists_imsi(&mme_app_desc.mme_ue_contexts,
                      imsi)) == NULL) {
        MME_APP_ERROR("That's embarrassing as we don't know this IMSI\n");
        return -1;
    }

    if ((s6a_auth_info_ans_p->result.present == S6A_RESULT_BASE) &&
            (s6a_auth_info_ans_p->result.choice.base == DIAMETER_SUCCESS)) {
        /* S6A procedure has succeeded.
         * We have to request UE authentication.
         */

        /* Check that list is not empty and contain only one element */
        DevCheck(s6a_auth_info_ans_p->auth_info.nb_of_vectors == 1,
                 s6a_auth_info_ans_p->auth_info.nb_of_vectors, 1, 0);

        if (ue_context->vector_list == NULL) {
            ue_context->vector_list = malloc(sizeof(eutran_vector_t));
            DevAssert(ue_context->vector_list != NULL);
        } else {
            /* Some vector already exist */
            ue_context->vector_list = realloc(ue_context->vector_list,
                                              (ue_context->nb_of_vectors + 1) * sizeof(eutran_vector_t));
            DevAssert(ue_context->vector_list != NULL);
        }
        memcpy(&ue_context->vector_list[ue_context->nb_of_vectors],
               &s6a_auth_info_ans_p->auth_info.eutran_vector, sizeof(eutran_vector_t));

        ue_context->vector_in_use = &ue_context->vector_list[ue_context->nb_of_vectors];

        ue_context->nb_of_vectors += s6a_auth_info_ans_p->auth_info.nb_of_vectors;

        mme_app_itti_auth_rsp(ue_context->ue_id, 1,
                              &s6a_auth_info_ans_p->auth_info.eutran_vector);
    } else {
        /* Inform NAS layer with the right failure */
        if (s6a_auth_info_ans_p->result.present == S6A_RESULT_BASE) {
            mme_app_itti_auth_fail(ue_context->ue_id, s6a_error_2_nas_cause(
                s6a_auth_info_ans_p->result.choice.base, 0));
        } else {
            mme_app_itti_auth_fail(ue_context->ue_id, s6a_error_2_nas_cause(
                s6a_auth_info_ans_p->result.choice.experimental, 1));
        }
    }

    return 0;
}

#if defined(DISABLE_USE_NAS)
int mme_app_handle_attach_req(nas_attach_req_t *attach_req_p)
{
    /* An attach request has been received from NAS layer.
     * If the UE authentication vectors for the UE are known within MME then the
     * authentication procedure should be triggered only if the request is an
     * initial attach, otherwise an update location should be sent to the HSS
     * and default bearer should be established for the provided APN.
     * In case of initial attach procedure, the default APN retrieved from the
     * HSS will be used to establish the default bearer within EPC.
     * The default APN is the one that matches the context-identifier
     */
    struct ue_context_s *ue_context;
    uint64_t imsi = 0;

    DevAssert(attach_req_p != NULL);

    MME_APP_STRING_TO_IMSI((char *)attach_req_p->imsi, &imsi);

    MME_APP_DEBUG("Handling imsi %"IMSI_FORMAT"\n", imsi);

    ue_context = mme_ue_context_exists_imsi(&mme_app_desc.mme_ue_contexts, imsi);
    if (ue_context == NULL) {
        /* The MME doesn't know this IMSI.
         * Insert the UE to the list of known equipements and
         * Retrieve the authentication vector from HSS.
         */
        MME_APP_DEBUG("UE context doesn't exist -> create one\n");
        if ((ue_context = mme_create_new_ue_context()) == NULL) {
            /* Error during ue context malloc */
            /* TODO */
            DevMessage("mme_create_new_ue_context");
            return -1;
        }
        ue_context->imsi = imsi;

        ue_context->eNB_ue_s1ap_id = attach_req_p->transparent.eNB_ue_s1ap_id;
        ue_context->mme_ue_s1ap_id = attach_req_p->transparent.mme_ue_s1ap_id;

//         STAILQ_INIT(&ue_context->vector_list);
        DevAssert(mme_insert_ue_context(&mme_app_desc.mme_ue_contexts, ue_context) == 0);
        goto request_auth;
    } else {
        /* MME knows this IMSI, check if UE is authenticated and authentication
         * vectors are known.
         */
        MME_APP_DEBUG("UE context already exists, use it\n");

        /* Update mme ue s1ap id */
        ue_context->mme_ue_s1ap_id = attach_req_p->transparent.mme_ue_s1ap_id;

        if ((ue_context->imsi_auth == IMSI_AUTHENTICATED) &&
                (attach_req_p->initial != INITIAL_REQUEST)) {
            /* We have to send an update location request to the HSS */
            MME_APP_DEBUG("UE is authenticated\n");
        } else {
            MME_APP_DEBUG("UE is not authenticated\n");
            /* UE is not authenticated or an initial request */
//              if (STAILQ_EMPTY(&ue_context->vector_list))
            if (ue_context->nb_of_vectors == 0)
request_auth: {
                /* We have no vector for this UE, send an authentication request
                 * to the HSS.
                 */
                plmn_t plmn = {
                    .MCCdigit2 = 0,
                    .MCCdigit1 = 8,
                    .MCCdigit3 = 2,
                    .MNCdigit1 = 0,
                    .MNCdigit2 = 4,
                    .MNCdigit3 = 3,
                };

                memcpy(&ue_context->e_utran_cgi, &attach_req_p->transparent.e_utran_cgi,
                       sizeof(cgi_t));

                /* Acquire the current time */
                time(&ue_context->cell_age);

                /* Some random values for GUTI */
                ue_context->guti.m_tmsi         = 0x24568956;
                ue_context->guti.gummei.MMEcode = 0x01;
                ue_context->guti.gummei.MMEgid  = 0x5691;

                memcpy(&ue_context->guti.gummei.plmn, &plmn, sizeof(plmn_t));
                MME_APP_DEBUG("and we have no auth. vector for it, request"
                              " authentication information\n");
//                 mme_app_dump_ue_contexts();
                mme_app_request_authentication_info(imsi, 1, &plmn, NULL);
            } else {
                nas_auth_req_t *nas_auth_req_p;
                MessageDef     *message_p;
                /* We have a vector... USE it */
                MME_APP_DEBUG("but we have an auth. vector for it, request"
                              " authentication from NAS\n");
                message_p = itti_alloc_new_message(TASK_MME_APP, NAS_AUTHENTICATION_PARAM_FAIL);

                nas_auth_req_p = &message_p->ittiMsg.nas_auth_req;

                MME_APP_IMSI_TO_STRING(imsi, nas_auth_req_p->imsi);
                nas_auth_req_p->failure = NAS_FAILURE_OK;

                return itti_send_msg_to_task(TASK_NAS, INSTANCE_DEFAULT, message_p);
            }
        }
    }
    return 0;
}
#else
void mme_app_handle_nas_auth_param_req(nas_auth_param_req_t
                                       *nas_auth_param_req_p)
{
    struct ue_context_s *ue_context;
    uint64_t imsi = 0;
    plmn_t visited_plmn_dongle = {
        .MCCdigit3 = 2,
        .MCCdigit2 = 0,
        .MCCdigit1 = 8,
        .MNCdigit3 = 2,
        .MNCdigit2 = 9,
        .MNCdigit1 = 0xF,
    };

    DevAssert(nas_auth_param_req_p != NULL);

    MME_APP_STRING_TO_IMSI(nas_auth_param_req_p->imsi, &imsi);

    MME_APP_DEBUG("Handling imsi %"IMSI_FORMAT"\n", imsi);

    /* Fetch the context associated with this IMSI */
    ue_context = mme_ue_context_exists_imsi(&mme_app_desc.mme_ue_contexts, imsi);

    if (ue_context == NULL) {
        /* Currently no context available -> trigger an authentication request
         * to the HSS.
         */
        MME_APP_DEBUG("UE context doesn't exist -> create one\n");
        if ((ue_context = mme_create_new_ue_context()) == NULL) {
            /* Error during ue context malloc */
            /* TODO */
            DevMessage("mme_create_new_ue_context");
            return;
        }

        ue_context->imsi  = imsi;
        ue_context->ue_id = nas_auth_param_req_p->ue_id;

        DevAssert(mme_insert_ue_context(&mme_app_desc.mme_ue_contexts, ue_context) == 0);

        /* We have no vector for this UE, send an authentication request
         * to the HSS.
         */
        plmn_t visited_plmn_eur = {
            .MCCdigit3 = 2,
            .MCCdigit2 = 0,
            .MCCdigit1 = 8,
            .MNCdigit1 = 0,
            .MNCdigit2 = 4,
            .MNCdigit3 = 3,
        };

        /* Acquire the current time */
        time(&ue_context->cell_age);

        memcpy(&ue_context->guti.gummei.plmn, &visited_plmn_dongle, sizeof(plmn_t));
        MME_APP_DEBUG("and we have no auth. vector for it, request"
                      " authentication information\n");
        mme_app_request_authentication_info(imsi, 1, &visited_plmn_dongle, NULL);
    } else {
        memcpy(&ue_context->guti.gummei.plmn, &visited_plmn_dongle, sizeof(plmn_t));

        mme_app_request_authentication_info(imsi, 1, &visited_plmn_dongle, nas_auth_param_req_p->auts);
    }
}
#endif