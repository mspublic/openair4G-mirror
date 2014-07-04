/*******************************************************************************
Eurecom OpenAirInterface Core Network
Copyright(c) 1999 - 2014 Eurecom

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
Forums       : http://forums.eurecom.fsr/openairinterface
Address      : EURECOM,
               Campus SophiaTech,
               450 Route des Chappes,
               CS 50193
               06904 Biot Sophia Antipolis cedex,
               FRANCE
*******************************************************************************/

#ifndef MME_APP_ITTI_MESSAGING_H_
#define MME_APP_ITTI_MESSAGING_H_

static inline void
mme_app_itti_auth_fail(
        const uint32_t ue_id,
        const nas_cause_t cause)
{
    MessageDef *message_p;

    message_p = itti_alloc_new_message(TASK_MME_APP, NAS_AUTHENTICATION_PARAM_FAIL);

    NAS_AUTHENTICATION_PARAM_FAIL(message_p).ue_id = ue_id;
    NAS_AUTHENTICATION_PARAM_FAIL(message_p).cause = cause;

    itti_send_msg_to_task(TASK_NAS_MME, INSTANCE_DEFAULT, message_p);
}



static inline void mme_app_itti_auth_rsp(
        const uint32_t                 ue_id,
        const uint8_t                  nb_vectors,
        const  eutran_vector_t * const vector)
{
    MessageDef *message_p;

    message_p = itti_alloc_new_message(TASK_MME_APP, NAS_AUTHENTICATION_PARAM_RSP);

    NAS_AUTHENTICATION_PARAM_RSP(message_p).ue_id       = ue_id;
    NAS_AUTHENTICATION_PARAM_RSP(message_p).nb_vectors  = nb_vectors;
    memcpy(&NAS_AUTHENTICATION_PARAM_RSP(message_p).vector, vector, sizeof(*vector));

    itti_send_msg_to_task(TASK_NAS_MME, INSTANCE_DEFAULT, message_p);
}

#endif /* MME_APP_ITTI_MESSAGING_H_ */
