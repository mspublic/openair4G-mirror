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

/*! \file s1ap_eNB_nnsf.c
 * \brief s1ap NAS node selection functions
 * \author Sebastien ROUX <sebastien.roux@eurecom.fr>
 * \date 2012
 * \version 0.1
 */

#include <stdio.h>
#include <stdlib.h>

#include "intertask_interface.h"

#include "s1ap_common.h"

#include "s1ap_eNB_defs.h"
#include "s1ap_eNB_nnsf.h"

struct s1ap_eNB_mme_data_s *
s1ap_eNB_nnsf_select_mme(s1ap_eNB_instance_t       *instance_p,
                         rrc_establishment_cause_t  cause)
{
    struct s1ap_eNB_mme_data_s *mme_data_p = NULL;
    struct s1ap_eNB_mme_data_s *mme_highest_capacity_p = NULL;
    uint8_t current_capacity = 0;

    RB_FOREACH(mme_data_p, s1ap_mme_map, &instance_p->s1ap_mme_head)
    {
        if (mme_data_p->state != S1AP_ENB_STATE_CONNECTED) {
            /* The association between MME and eNB is not ready for the moment,
             * go to the next known MME.
             */
            if (mme_data_p->state == S1AP_ENB_OVERLOAD) {
                /* MME is overloaded. We have to check the RRC establishment
                 * cause and take decision to the select this MME depending on
                 * the overload state.
                 */
                if ((cause == RRC_CAUSE_MO_DATA)
                        && (mme_data_p->overload_state == S1AP_OVERLOAD_REJECT_MO_DATA)) {
                    continue;
                }
                if ((mme_data_p->overload_state == S1AP_OVERLOAD_REJECT_ALL_SIGNALLING)
                        && ((cause == RRC_CAUSE_MO_SIGNALLING) || (cause == RRC_CAUSE_MO_DATA))) {
                    continue;
                }
                if ((mme_data_p->overload_state == S1AP_OVERLOAD_ONLY_EMERGENCY_AND_MT)
                        && ((cause == RRC_CAUSE_MO_SIGNALLING) || (cause == RRC_CAUSE_MO_DATA)
                            || (cause == RRC_CAUSE_HIGH_PRIO_ACCESS))) {
                    continue;
                }
                /* At this point, the RRC establishment can be handled by the MME
                 * even if it is in overload state.
                 */
            } else {
                /* The MME is not overloaded, association is simply not ready. */
                continue;
            }
        }
        if (current_capacity < mme_data_p->relative_mme_capacity) {
            /* We find a better MME, keep a reference to it */
            current_capacity = mme_data_p->relative_mme_capacity;
            mme_highest_capacity_p = mme_data_p;
        }
    }

    return mme_highest_capacity_p;
}

struct s1ap_eNB_mme_data_s *
s1ap_eNB_nnsf_select_mme_by_mme_code(s1ap_eNB_instance_t       *instance_p,
                                     rrc_establishment_cause_t  cause,
                                     uint8_t                    mme_code)
{
    struct s1ap_eNB_mme_data_s *mme_data_p = NULL;
    struct s1ap_eNB_mme_data_s *mme_highest_capacity_p = NULL;
    uint8_t current_capacity = 0;

    RB_FOREACH(mme_data_p, s1ap_mme_map, &instance_p->s1ap_mme_head) {
        struct served_gummei_s *gummei_p = NULL;

        if (mme_data_p->state != S1AP_ENB_STATE_CONNECTED) {
            /* The association between MME and eNB is not ready for the moment,
             * go to the next known MME.
             */
            if (mme_data_p->state == S1AP_ENB_OVERLOAD) {
                /* MME is overloaded. We have to check the RRC establishment
                 * cause and take decision to the select this MME depending on
                 * the overload state.
                 */
                if ((cause == RRC_CAUSE_MO_DATA)
                        && (mme_data_p->overload_state == S1AP_OVERLOAD_REJECT_MO_DATA)) {
                    continue;
                }
                if ((mme_data_p->overload_state == S1AP_OVERLOAD_REJECT_ALL_SIGNALLING)
                        && ((cause == RRC_CAUSE_MO_SIGNALLING) || (cause == RRC_CAUSE_MO_DATA))) {
                    continue;
                }
                if ((mme_data_p->overload_state == S1AP_OVERLOAD_ONLY_EMERGENCY_AND_MT)
                        && ((cause == RRC_CAUSE_MO_SIGNALLING) || (cause == RRC_CAUSE_MO_DATA)
                            || (cause == RRC_CAUSE_HIGH_PRIO_ACCESS))) {
                    continue;
                }
                /* At this point, the RRC establishment can be handled by the MME
                 * even if it is in overload state.
                 */
            } else {
                /* The MME is not overloaded, association is simply not ready. */
                continue;
            }
        }
        if (current_capacity < mme_data_p->relative_mme_capacity) {
            /* We find a better MME, keep a reference to it */
            current_capacity = mme_data_p->relative_mme_capacity;
            mme_highest_capacity_p = mme_data_p;
        }

        /* Looking for MME code matching the one provided by NAS */
        STAILQ_FOREACH(gummei_p, &mme_data_p->served_gummei, next) {
            struct mme_code_s *mme_code_p = NULL;

            STAILQ_FOREACH(mme_code_p, &gummei_p->mme_codes, next) {
                if (mme_code_p->mme_code == mme_code) {
                    return mme_data_p;
                }
            }
        }
    }

    /* At this point no MME matches the provided GUMMEI. Select the one with the
     * highest relative capacity.
     * In case the list of known MME is empty, simply return NULL, that way the RRC
     * layer should know about it and reject RRC connectivity.
     */
    return mme_highest_capacity_p;
}

struct s1ap_eNB_mme_data_s *
s1ap_eNB_nnsf_select_mme_by_gummei(s1ap_eNB_instance_t       *instance_p,
                                   rrc_establishment_cause_t  cause,
                                   s1ap_gummei_t                   gummei)
{
    struct s1ap_eNB_mme_data_s *mme_data_p = NULL;
    struct s1ap_eNB_mme_data_s *mme_highest_capacity_p = NULL;
    uint8_t current_capacity = 0;

    RB_FOREACH(mme_data_p, s1ap_mme_map, &instance_p->s1ap_mme_head) {
        struct served_gummei_s *gummei_p = NULL;

        if (mme_data_p->state != S1AP_ENB_STATE_CONNECTED) {
            /* The association between MME and eNB is not ready for the moment,
             * go to the next known MME.
             */
            if (mme_data_p->state == S1AP_ENB_OVERLOAD) {
                /* MME is overloaded. We have to check the RRC establishment
                 * cause and take decision to the select this MME depending on
                 * the overload state.
                 */
                if ((cause == RRC_CAUSE_MO_DATA)
                        && (mme_data_p->overload_state == S1AP_OVERLOAD_REJECT_MO_DATA)) {
                    continue;
                }
                if ((mme_data_p->overload_state == S1AP_OVERLOAD_REJECT_ALL_SIGNALLING)
                        && ((cause == RRC_CAUSE_MO_SIGNALLING) || (cause == RRC_CAUSE_MO_DATA))) {
                    continue;
                }
                if ((mme_data_p->overload_state == S1AP_OVERLOAD_ONLY_EMERGENCY_AND_MT)
                        && ((cause == RRC_CAUSE_MO_SIGNALLING) || (cause == RRC_CAUSE_MO_DATA)
                            || (cause == RRC_CAUSE_HIGH_PRIO_ACCESS))) {
                    continue;
                }
                /* At this point, the RRC establishment can be handled by the MME
                 * even if it is in overload state.
                 */
            } else {
                /* The MME is not overloaded, association is simply not ready. */
                continue;
            }
        }
        if (current_capacity < mme_data_p->relative_mme_capacity) {
            /* We find a better MME, keep a reference to it */
            current_capacity = mme_data_p->relative_mme_capacity;
            mme_highest_capacity_p = mme_data_p;
        }

        /* Looking for MME gummei matching the one provided by NAS */
        STAILQ_FOREACH(gummei_p, &mme_data_p->served_gummei, next) {
            struct served_group_id_s *group_id_p = NULL;
            struct mme_code_s        *mme_code_p = NULL;
            struct plmn_identity_s   *served_plmn_p = NULL;

            STAILQ_FOREACH(served_plmn_p, &gummei_p->served_plmns, next) {
                if ((served_plmn_p->mcc == gummei.mcc) &&
                        (served_plmn_p->mnc == gummei.mnc)) {
                    break;
                }
            }
            STAILQ_FOREACH(mme_code_p, &gummei_p->mme_codes, next) {
                if (mme_code_p->mme_code == gummei.mme_code) {
                    break;
                }
            }
            STAILQ_FOREACH(group_id_p, &gummei_p->served_group_ids, next) {
                if (group_id_p->mme_group_id == gummei.mme_group_id) {
                    break;
                }
            }
            /* The MME matches the parameters provided by the NAS layer ->
            * the MME is knwown and the association is ready.
            * Return the reference to the MME to use it for this UE.
            */
            if ((group_id_p != NULL) &&
                    (mme_code_p != NULL) &&
                    (served_plmn_p != NULL)) {
                return mme_data_p;
            }
        }
    }

    /* At this point no MME matches the provided GUMMEI. Select the one with the
     * highest relative capacity.
     * In case the list of known MME is empty, simply return NULL, that way the RRC
     * layer should know about it and reject RRC connectivity.
     */
    return mme_highest_capacity_p;
}
