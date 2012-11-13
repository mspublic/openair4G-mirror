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

#include "intertask_interface.h"
#include "intertask_interface_dump.h"

#define ITTI_DEBUG(x, args...) do { fprintf(stdout, "[ITTI][D]"x, ##args); } \
    while(0)

/* This list acts as a FIFO of messages received by tasks (RRC, NAS, ...) */
struct linked_list {
    struct linked_list *next_element; ///< Next element in the list

    MessageDef         *msg;          ///< Pointer to the message

    uint32_t messageNumber;           ///< Unique message number
};

/* Reference to the first element of the list */
static struct linked_list *head[TASK_MAX];
/* Reference to the last element of the list */
static struct linked_list *tail[TASK_MAX];
/* Number of messages in the queue */
static uint32_t messageInQueue[TASK_MAX];
/* Mutex for the message queue */
static pthread_mutex_t messageQueueMutex[TASK_MAX];
/* Conditinal Var for message queue and task synchro */
static pthread_cond_t  messageQueueCondVar[TASK_MAX];

/* Current message number. Increment every call to send_msg_to_task */
static uint32_t messageNumber = 0;

int send_msg_to_task(TaskId taskId, MessageDef *message)
{
    struct linked_list *temp;

    assert(message != NULL);
    assert(taskId < TASK_MAX);

    // Lock the mutex to get exclusive access to the list
    pthread_mutex_lock(&messageQueueMutex[taskId]);
    // Allocate new list element
    temp = (struct linked_list *)malloc(sizeof(struct linked_list));

    //Increment message number
    messageNumber++;

    // Fill in members
    temp->next_element = NULL;
    temp->msg = message;
    temp->messageNumber = messageNumber;

    // First element in the list ?
    if (tail[taskId] == NULL) {
        tail[taskId] = temp;
        head[taskId] = tail[taskId];
    }
    else {
        tail[taskId]->next_element = temp;
        tail[taskId] = temp;
    }

    // Update the number of messages in the queue
    messageInQueue[taskId]++;
    if (messageInQueue[taskId] == 1) {
        // Emit a signal
        pthread_cond_signal(&messageQueueCondVar[taskId]);
    }
    // Release the mutex
    pthread_mutex_unlock(&messageQueueMutex[taskId]);
    ITTI_DEBUG("Message %d succesfully sent to taskId %d\n", messageNumber, taskId);
    return 0;
}

void receive_msg(TaskId taskId, MessageDef **receivedMsg)
{
    assert(taskId < TASK_MAX);
    assert(receivedMsg != NULL);

    // Lock the mutex to get exclusive access to the list
    pthread_mutex_lock(&messageQueueMutex[taskId]);

    if (messageInQueue[taskId] == 0) {
        ITTI_DEBUG("Message in queue[%u] == 0, waiting\n", taskId);
        // Wait while list == 0
        pthread_cond_wait(&messageQueueCondVar[taskId], &messageQueueMutex[taskId]);
        ITTI_DEBUG("Receiver thread queue[%u] got new message notification\n", taskId);
    }

    if ((head[taskId] != NULL) && (messageInQueue[taskId] > 0)) {
        struct linked_list *temp = head[taskId];

        // Update receivedMsg reference
        *receivedMsg = temp->msg;

        // More than one element in the list ?
        if (temp->next_element != NULL) {
            head[taskId] = temp->next_element;
        }
        else {
            head[taskId] = NULL;
            tail[taskId] = NULL;
        }
        free(temp);
        messageInQueue[taskId]--;
    }
    // Release the mutex
    pthread_mutex_unlock(&messageQueueMutex[taskId]);
}

void intertask_interface_init(void)
{
    int i;
    for (i = 0; i < TASK_MAX; i++) {
        head[i] = NULL;
        tail[i] = NULL;
        messageInQueue[i] = 0;
        // Initialize mutexes
        pthread_mutex_init(&messageQueueMutex[i], NULL);
        // Initialize Cond vars
        pthread_cond_init(&messageQueueCondVar[i], NULL);
    }
    itti_init();
}
