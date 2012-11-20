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

/** @defgroup _intertask_interface_impl_ Intertask Interface Mechanisms
 * Implementation
 * @ingroup _ref_implementation_
 * @{
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mme_config.h"

#include "gtpv1_u_messages_types.h"
#include "sctp_messages_types.h"
#include "s1ap_messages_types.h"
#include "timer_messages_types.h"
#include "udp_messages_types.h"

#ifndef INTERTASK_INTERFACE_H_
#define INTERTASK_INTERFACE_H_

enum task_priorities {
    TASK_PRIORITY_MAX       = 100,
    TASK_PRIORITY_MAX_LEAST = 85,
    TASK_PRIORITY_MED_PLUS  = 70,
    TASK_PRIORITY_MED       = 55,
    TASK_PRIORITY_MED_LEAST = 40,
    TASK_PRIORITY_MIN_PLUS  = 25,
    TASK_PRIORITY_MIN       = 10,
};

// #define MESSAGE_ID(x) x,
// #define MESSAGE_DEF(x) x

/* This enum defines messages ids. Each one is unique. */
typedef enum {

    #define MESSAGE_DEF(iD, pRIO, sTRUCT) iD,

//     #include "gtpv1_u_messages_def.h"
    #include "sctp_messages_def.h"
    #include "s1ap_messages_def.h"
    #include "timer_messages_def.h"
    #include "udp_messages_def.h"

    #undef MESSAGE_DEF

    MESSAGES_ID_MAX,
    MESSAGES_ID_END = MESSAGES_ID_MAX,
} MessagesIds;

struct message_priority_s {
    uint32_t id;
    uint32_t priority;
};

//! Tasks id of each task
typedef enum {
    /// SCTP network task
    TASK_SCTP = 0,
    /// S1AP message task
    TASK_S1AP,
    /// GTP-U message task
    TASK_GTPV1_U,
    /// NAS message task
    TASK_UDP,
    /// TIMERS message task
    TASK_TIMER,
    TASK_MAX,
    TASK_END = TASK_MAX,
} TaskId;

/** @struct MessageDef
 *  @brief Message structure for inter thread communication.
 */
typedef struct MessageDef_s {
    uint32_t messageId;         /**< Unique message id as referenced in enum MessagesIds */

    uint32_t originTaskId;      /**< ID of the sender task */
    uint32_t destinationTaskId; /**< ID of the destination task */

    union msg {
        #define MESSAGE_DEF(iD, pRIO, sTRUCT) sTRUCT;

        #include "gtpv1_u_messages_def.h"
        #include "sctp_messages_def.h"
        #include "s1ap_messages_def.h"
        #include "timer_messages_def.h"
        #include "udp_messages_def.h"

        #undef MESSAGE_DEF
    } msg; /**< Union of payloads as defined in x_messages_def.h headers */
} MessageDef;

/** \brief Send a message to a task (could be itself)
 \param task_id Task ID
 \param message Pointer to the message to send
 @returns -1 on failure, 0 otherwise
 **/
int send_msg_to_task(TaskId task_id, MessageDef *message);

/** \brief Retrieves a messsage in queue.
 * If the queue is empty, the thread is blocked till a new message arrives.
 \param task_id Task ID of the receiving task
 \param received_msg Pointer to the allocated message
 **/
void receive_msg(TaskId task_id, MessageDef **received_msg);

/** \brief Init function for the intertask interface. Init queues, Mutexes and Cond vars.
 * If the queue is empty, the thread is blocked till a new message arrives.
 **/
void intertask_interface_init(const mme_config_t *mme_config);

#endif /* INTERTASK_INTERFACE_H_ */
/* @} */
