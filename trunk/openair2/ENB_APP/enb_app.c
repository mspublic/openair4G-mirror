/*******************************************************************************

 Eurecom OpenAirInterface
 Copyright(c) 1999 - 2012 Eurecom

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

#include <string.h>

#include "enb_app.h"
#include "enb_config.h"
#include "assertions.h"

#include "log.h"
#if defined(OAI_EMU)
# include "OCG.h"
# include "OCG_extern.h"
#endif

#if defined(ENABLE_ITTI)
# include "intertask_interface.h"
# include "timer.h"
# if defined(ENABLE_USE_MME)
#   include "s1ap_eNB.h"
#   include "sctp_eNB_task.h"
# endif

extern unsigned char NB_eNB_INST;
extern char         *g_conf_config_file_name;
#endif

#if defined(ENABLE_ITTI)

/*------------------------------------------------------------------------------*/
# if defined(ENABLE_USE_MME)
#   define ENB_REGISTER_RETRY_DELAY 10
# endif

/*------------------------------------------------------------------------------*/
static Enb_properties_t    *enb_properties[MAX_ENB];
static int                  enb_nb_properties;

/*------------------------------------------------------------------------------*/
static void configure_phy(uint32_t enb_id)
{
    MessageDef *msg_p;

    msg_p = itti_alloc_new_message (TASK_ENB_APP, PHY_CONFIGURATION_REQ);

    PHY_CONFIGURATION_REQ (msg_p).frame_type =              enb_properties[enb_id]->frame_type;
    PHY_CONFIGURATION_REQ (msg_p).prefix_type =             enb_properties[enb_id]->prefix_type;
    PHY_CONFIGURATION_REQ (msg_p).downlink_frequency =      enb_properties[enb_id]->downlink_frequency;
    PHY_CONFIGURATION_REQ (msg_p).uplink_frequency_offset = enb_properties[enb_id]->uplink_frequency_offset;

    itti_send_msg_to_task (TASK_PHY_ENB, enb_id, msg_p);
}

/*------------------------------------------------------------------------------*/
static void configure_rrc(uint32_t enb_id)
{
    MessageDef *msg_p;

    msg_p = itti_alloc_new_message (TASK_ENB_APP, RRC_CONFIGURATION_REQ);

    RRC_CONFIGURATION_REQ (msg_p).cell_identity =   enb_properties[enb_id]->eNB_id;
    RRC_CONFIGURATION_REQ (msg_p).tac =             enb_properties[enb_id]->tac;
    RRC_CONFIGURATION_REQ (msg_p).mcc =             enb_properties[enb_id]->mcc;
    RRC_CONFIGURATION_REQ (msg_p).mnc =             enb_properties[enb_id]->mnc;

    itti_send_msg_to_task (TASK_RRC_ENB, enb_id, msg_p);
}

/*------------------------------------------------------------------------------*/
# if defined(ENABLE_USE_MME)
static uint32_t eNB_app_register(uint32_t enb_id_start, uint32_t enb_id_end)
{
    uint32_t enb_id;
    uint32_t mme_id;
    MessageDef *msg_p;
    uint32_t register_enb_pending = 0;

#   if defined(OAI_EMU)

#   endif

    for (enb_id = enb_id_start; (enb_id < enb_id_end) ; enb_id++)
    {
#   if defined(OAI_EMU)
        if (oai_emulation.info.cli_start_enb[enb_id] == 1)
#   endif
        {
            s1ap_register_enb_req_t *s1ap_register_eNB;
            uint32_t hash;

            /* Overwrite default eNB ID */
            hash = s1ap_generate_eNB_id ();
            enb_properties[enb_id]->eNB_id = enb_id + (hash & 0xFFFF8);

            /* note:  there is an implicit relationship between the data structure and the message name */
            msg_p = itti_alloc_new_message (TASK_ENB_APP, S1AP_REGISTER_ENB_REQ);

            s1ap_register_eNB = &S1AP_REGISTER_ENB_REQ(msg_p);

            /* Some default/random parameters */
            s1ap_register_eNB->eNB_id = enb_properties[enb_id]->eNB_id;
            s1ap_register_eNB->cell_type = enb_properties[enb_id]->cell_type;
            s1ap_register_eNB->eNB_name = enb_properties[enb_id]->eNB_name;
            s1ap_register_eNB->tac = enb_properties[enb_id]->tac;
            s1ap_register_eNB->mcc = enb_properties[enb_id]->mcc;
            s1ap_register_eNB->mnc = enb_properties[enb_id]->mnc;
            s1ap_register_eNB->default_drx = enb_properties[enb_id]->default_drx;

            s1ap_register_eNB->nb_mme = enb_properties[enb_id]->nb_mme;
            AssertFatal (s1ap_register_eNB->nb_mme <= S1AP_MAX_NB_MME_IP_ADDRESS, "Too many MME for eNB %d (%d/%d)!", enb_id, s1ap_register_eNB->nb_mme, S1AP_MAX_NB_MME_IP_ADDRESS);

            for (mme_id = 0; mme_id < s1ap_register_eNB->nb_mme; mme_id++)
            {
                s1ap_register_eNB->mme_ip_address[mme_id].ipv4 = enb_properties[enb_id]->mme_ip_address[mme_id].ipv4;
                s1ap_register_eNB->mme_ip_address[mme_id].ipv6 = enb_properties[enb_id]->mme_ip_address[mme_id].ipv6;
                strncpy (s1ap_register_eNB->mme_ip_address[mme_id].ipv4_address,
                         enb_properties[enb_id]->mme_ip_address[mme_id].ipv4_address,
                         sizeof(s1ap_register_eNB->mme_ip_address[0].ipv4_address));
                strncpy (s1ap_register_eNB->mme_ip_address[mme_id].ipv6_address,
                         enb_properties[enb_id]->mme_ip_address[mme_id].ipv6_address,
                         sizeof(s1ap_register_eNB->mme_ip_address[0].ipv6_address));
            }

            itti_send_msg_to_task (TASK_S1AP, enb_id, msg_p);

            register_enb_pending++;
        }
    }

    return register_enb_pending;
}
# endif
#endif

/*------------------------------------------------------------------------------*/
void *eNB_app_task(void *args_p)
{
#if defined(ENABLE_ITTI)
    uint32_t    enb_nb = 1; /* Default number of eNB is 1 */
    uint32_t    enb_id_start = 0;
    uint32_t    enb_id_end = enb_id_start + enb_nb;
# if defined(ENABLE_USE_MME)
    uint32_t    register_enb_pending;
    uint32_t    registered_enb;
    long    enb_register_retry_timer_id;
# endif
    uint32_t    enb_id;
    MessageDef *msg_p;
    const char *msg_name;
    instance_t  instance;
    int         result;

    itti_mark_task_ready (TASK_ENB_APP);

# if defined(ENABLE_USE_MME)
#   if defined(OAI_EMU)
    enb_nb =        oai_emulation.info.nb_enb_local;
    enb_id_start =  oai_emulation.info.first_enb_local;
    enb_id_end =    oai_emulation.info.first_enb_local + enb_nb;

    AssertFatal (enb_id_end <= NUMBER_OF_eNB_MAX,
                 "Last eNB index is greater or equal to maximum eNB index (%d/%d)!",
                 enb_id_end, NUMBER_OF_eNB_MAX);
#   endif
# endif

    enb_nb_properties = enb_config_init(g_conf_config_file_name, enb_properties);

    AssertFatal (enb_nb <= enb_nb_properties,
                 "Number of eNB is greater than eNB defined in configuration file %s (%d/%d)!",
                 g_conf_config_file_name, enb_nb, enb_nb_properties);

    for (enb_id = enb_id_start; (enb_id < enb_id_end) ; enb_id++)
    {
        configure_phy(enb_id);
        configure_rrc(enb_id);
    }

# if defined(ENABLE_USE_MME)
    /* Try to register each eNB */
    registered_enb = 0;
    register_enb_pending = eNB_app_register (enb_id_start, enb_id_end);
# else
    /* Start L2L1 task */
    msg_p = itti_alloc_new_message(TASK_ENB_APP, INITIALIZE_MESSAGE);
    itti_send_msg_to_task(TASK_L2L1, INSTANCE_DEFAULT, msg_p);
# endif

    do
    {
        // Wait for a message
        itti_receive_msg (TASK_ENB_APP, &msg_p);

        msg_name = ITTI_MSG_NAME (msg_p);
        instance = ITTI_MSG_INSTANCE (msg_p);

        switch (ITTI_MSG_ID(msg_p))
        {
            case TERMINATE_MESSAGE:
                itti_exit_task ();
                break;

            case MESSAGE_TEST:
                LOG_I(ENB_APP, "Received %s\n", ITTI_MSG_NAME(msg_p));
                break;

# if defined(ENABLE_USE_MME)
            case S1AP_REGISTER_ENB_CNF:
                LOG_I(ENB_APP, "[eNB %d] Received %s: associated MME %d\n", instance, msg_name,
                      S1AP_REGISTER_ENB_CNF(msg_p).nb_mme);

                DevAssert(register_enb_pending > 0);
                register_enb_pending--;

                /* Check if at least eNB is registered with one MME */
                if (S1AP_REGISTER_ENB_CNF(msg_p).nb_mme > 0)
                {
                    registered_enb++;
                }

                /* Check if all register eNB requests have been processed */
                if (register_enb_pending == 0)
                {
                    if (registered_enb == enb_nb)
                    {
                        /* If all eNB are registered, start L2L1 task */
                        MessageDef *msg_init_p;

                        msg_init_p = itti_alloc_new_message (TASK_ENB_APP, INITIALIZE_MESSAGE);
                        itti_send_msg_to_task (TASK_L2L1, INSTANCE_DEFAULT, msg_init_p);

#   if defined(OAI_EMU)
                        /* Also inform all NAS UE tasks */
                        for (instance = NB_eNB_INST + oai_emulation.info.first_ue_local;
                            instance < (NB_eNB_INST + oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local); instance ++)
                        {
                            msg_init_p = itti_alloc_new_message (TASK_ENB_APP, INITIALIZE_MESSAGE);
                            itti_send_msg_to_task (TASK_NAS_UE, instance, msg_init_p);
                        }
#   endif
                    }
                    else
                    {
                        uint32_t not_associated = enb_nb - registered_enb;

                        LOG_W(ENB_APP, " %d eNB %s not associated with a MME, retrying registration in %d seconds ...\n",
                              not_associated, not_associated > 1 ? "are" : "is", ENB_REGISTER_RETRY_DELAY);

                        /* Restart the eNB registration process in ENB_REGISTER_RETRY_DELAY seconds */
                        if (timer_setup (ENB_REGISTER_RETRY_DELAY, 0, TASK_ENB_APP, INSTANCE_DEFAULT, TIMER_ONE_SHOT,
                                         NULL, &enb_register_retry_timer_id) < 0)
                        {
                            LOG_E(ENB_APP, " Can not start eNB register retry timer, use \"sleep\" instead!\n");

                            sleep(ENB_REGISTER_RETRY_DELAY);
                            /* Restart the registration process */
                            registered_enb = 0;
                            register_enb_pending = eNB_app_register (enb_id_start, enb_id_end);
                        }
                    }
                }
                break;

            case S1AP_DEREGISTERED_ENB_IND:
                LOG_W(ENB_APP, "[eNB %d] Received %s: associated MME %d\n", instance, msg_name,
                      S1AP_DEREGISTERED_ENB_IND(msg_p).nb_mme);

                /* TODO handle recovering of registration */
                break;

            case TIMER_HAS_EXPIRED:
                LOG_I(ENB_APP, " Received %s: timer_id %d\n", msg_name, TIMER_HAS_EXPIRED(msg_p).timer_id);

                if (TIMER_HAS_EXPIRED (msg_p).timer_id == enb_register_retry_timer_id)
                {
                    /* Restart the registration process */
                    registered_enb = 0;
                    register_enb_pending = eNB_app_register (enb_id_start, enb_id_end);
                }
                break;
# endif

            default:
                LOG_E(ENB_APP, "Received unexpected message %s\n", msg_name);
                break;
        }

        result = itti_free (ITTI_MSG_ORIGIN_ID(msg_p), msg_p);
        AssertFatal (result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
    } while (1);
#endif

    return NULL;
}
