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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "queue.h"

#include "intertask_interface.h"
#include "intertask_interface_dump.h"

#define ITTI_DEBUG(x, args...) do { fprintf(stdout, "[ITTI][D]"x, ##args); } \
    while(0)

/* This list acts as a FIFO of messages received by tasks (RRC, NAS, ...) */
struct message_list_s {
    STAILQ_ENTRY(message_list_s) next_element;
//     struct message_list_s *next_element; ///< Next element in the list

    MessageDef         *msg;          ///< Pointer to the message

    uint32_t message_number;          ///< Unique message number
    uint32_t message_priority;        ///< Message priority
};

// /* Reference to the first element of the list */
// static struct message_list_s *head[TASK_MAX];
// /* Reference to the last element of the list */
// static struct message_list_s *tail[TASK_MAX];
static STAILQ_HEAD(message_queue_head, message_list_s) message_queue[TASK_MAX];
/* Number of messages in the queue */
static uint32_t message_in_queue[TASK_MAX];
/* Mutex for the message queue */
static pthread_mutex_t message_queue_mutex[TASK_MAX];
/* Conditinal Var for message queue and task synchro */
static pthread_cond_t  mssage_queue_cond_var[TASK_MAX];

/* Current message number. Increment every call to send_msg_to_task */
static uint32_t message_number = 0;

/* Map message priority to message id */
const struct message_priority_s messages_priorities[MESSAGES_ID_MAX] = {
    #define MESSAGE_DEF(iD, pRIO, sTRUCT) { iD, pRIO },

    //     #include "gtpv1_u_messages_def.h"
    #include "sctp_messages_def.h"
    #include "s1ap_messages_def.h"
    #include "timer_messages_def.h"
    #include "udp_messages_def.h"

    #undef MESSAGE_DEF
};

static inline uint32_t get_message_priority(TaskId task_id) {
    uint32_t i;

    for (i = 0; i < TASK_MAX; i++) {
        if ((messages_priorities[i]).id == task_id)
            return ((messages_priorities[i]).priority);
    }
    return 0;
}

int send_msg_to_task(TaskId task_id, MessageDef *message)
{
    struct message_list_s *new;
    struct message_list_s *temp;

    uint32_t priority;

    assert(message != NULL);
    assert(task_id < TASK_MAX);

//     priority = messages_priorities[task_id];
    priority = get_message_priority(task_id);

    // Lock the mutex to get exclusive access to the list
    pthread_mutex_lock(&message_queue_mutex[task_id]);
    // Allocate new list element
    new = (struct message_list_s *)malloc(sizeof(struct message_list_s));

    //Increment message number
    message_number++;

    // Fill in members
    new->msg = message;
    new->message_number = message_number;

    if (STAILQ_EMPTY(&message_queue[task_id])) {
        STAILQ_INSERT_HEAD(&message_queue[task_id], new, next_element);
    } else {
        STAILQ_FOREACH(temp, &message_queue[task_id], next_element) {
            struct message_list_s *next;
            next = STAILQ_NEXT(temp, next_element);
            if (next == NULL) {
                STAILQ_INSERT_TAIL(&message_queue[task_id], new, next_element);
                break;
            }
            if (next->message_priority < priority) {
                STAILQ_INSERT_AFTER(&message_queue[task_id], temp, new, next_element);
                break;
            }
        }
    }

    // Update the number of messages in the queue
    message_in_queue[task_id]++;
    if (message_in_queue[task_id] == 1) {
        // Emit a signal
        pthread_cond_signal(&mssage_queue_cond_var[task_id]);
    }
    // Release the mutex
    pthread_mutex_unlock(&message_queue_mutex[task_id]);
    ITTI_DEBUG("Message %d succesfully sent to task_id %d\n", message_number, task_id);
    return 0;
}

void receive_msg(TaskId task_id, MessageDef **received_msg)
{
    assert(task_id < TASK_MAX);
    assert(received_msg != NULL);

    // Lock the mutex to get exclusive access to the list
    pthread_mutex_lock(&message_queue_mutex[task_id]);

    if (message_in_queue[task_id] == 0) {
        ITTI_DEBUG("Message in queue[%u] == 0, waiting\n", task_id);
        // Wait while list == 0
        pthread_cond_wait(&mssage_queue_cond_var[task_id], &message_queue_mutex[task_id]);
        ITTI_DEBUG("Receiver thread queue[%u] got new message notification\n", task_id);
    }

    if (!STAILQ_EMPTY(&message_queue[task_id])) {
        struct message_list_s *temp = STAILQ_FIRST(&message_queue[task_id]);

        // Update received_msg reference
        *received_msg = temp->msg;

        /* Remove message from queue */
        STAILQ_REMOVE_HEAD(&message_queue[task_id], next_element);
        free(temp);
        message_in_queue[task_id]--;
    }
    // Release the mutex
    pthread_mutex_unlock(&message_queue_mutex[task_id]);
}

void intertask_interface_init(const mme_config_t *mme_config)
{
    int i;
    for (i = 0; i < TASK_MAX; i++) {
//         head[i] = NULL;
//         tail[i] = NULL;
        STAILQ_INIT(&message_queue[i]);
        message_in_queue[i] = 0;
        // Initialize mutexes
        pthread_mutex_init(&message_queue_mutex[i], NULL);
        // Initialize Cond vars
        pthread_cond_init(&mssage_queue_cond_var[i], NULL);
    }
    itti_init();
}
