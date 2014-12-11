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
  
  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/

#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sys/epoll.h>
#include <sys/eventfd.h>

#ifdef RTAI
# include <rtai_shm.h>
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

#include "liblfds611.h"

#include "assertions.h"
#include "intertask_interface.h"
#include "intertask_interface_dump.h"

#if defined(OAI_EMU) || defined(RTAI)
# include "memory_pools.h"
# include "vcd_signal_dumper.h"
#endif

/* Includes "intertask_interface_init.h" to check prototype coherence, but
 * disable threads and messages information generation.
 */
#define CHECK_PROTOTYPE_ONLY
#include "intertask_interface_init.h"
#undef CHECK_PROTOTYPE_ONLY

#include "signals.h"
#include "timer.h"

#ifdef RTAI
# include <rtai.h>
# include <rtai_fifos.h>
#    define FIFO_PRINTF_MAX_STRING_SIZE 1000
#    define FIFO_PRINTF_NO              62
#    define FIFO_PRINTF_SIZE            65536
#endif

/* ITTI DEBUG groups */
#define ITTI_DEBUG_POLL             (1<<0)
#define ITTI_DEBUG_SEND             (1<<1)
#define ITTI_DEBUG_EVEN_FD          (1<<2)
#define ITTI_DEBUG_INIT             (1<<3)
#define ITTI_DEBUG_EXIT             (1<<4)
#define ITTI_DEBUG_ISSUES           (1<<5)
#define ITTI_DEBUG_MP_STATISTICS    (1<<6)

const int itti_debug = ITTI_DEBUG_ISSUES | ITTI_DEBUG_MP_STATISTICS;

/* Don't flush if using RTAI */
#ifdef RTAI
# define ITTI_DEBUG(m, x, args...)  do { if ((m) & itti_debug) rt_log_debug (x, ##args); } while(0);
#else
# define ITTI_DEBUG(m, x, args...)  do { if ((m) & itti_debug) fprintf(stdout, "[ITTI][D]"x, ##args); fflush (stdout); } while(0);
#endif
#define ITTI_ERROR(x, args...) 	    do { fprintf(stdout, "[ITTI][E]"x, ##args); fflush (stdout); } while(0);

/* Global message size */
#define MESSAGE_SIZE(mESSAGEiD) (sizeof(MessageHeader) + itti_desc.messages_info[mESSAGEiD].size)

#ifdef RTAI
# define ITTI_MEM_PAGE_SIZE (1024)
# define ITTI_MEM_SIZE      (16 * 1024 * 1024)
#endif

typedef enum task_state_s {
    TASK_STATE_NOT_CONFIGURED, TASK_STATE_STARTING, TASK_STATE_READY, TASK_STATE_ENDED, TASK_STATE_MAX,
} task_state_t;

/* This list acts as a FIFO of messages received by tasks (RRC, NAS, ...) */
typedef struct message_list_s {
    MessageDef *msg; ///< Pointer to the message

    message_number_t message_number; ///< Unique message number
    uint32_t message_priority; ///< Message priority
} message_list_t;

typedef struct thread_desc_s {
    /* pthread associated with the thread */
    pthread_t task_thread;

    /* State of the thread */
    volatile task_state_t task_state;

    /* This fd is used internally by ITTI. */
    int epoll_fd;

    /* The thread fd */
    int task_event_fd;

    /* Number of events to monitor */
    uint16_t nb_events;


    /* Array of events monitored by the task.
     * By default only one fd is monitored (the one used to received messages
     * from other tasks).
     * More events can be suscribed later by the task itself.
     */
    struct epoll_event *events;

    int epoll_nb_events;

//#ifdef RTAI
    /* Flag to mark real time thread */
    unsigned real_time;

    /* Counter to indicate from RTAI threads that messages are pending for the thread */
    unsigned messages_pending;
//#endif
} thread_desc_t;

typedef struct task_desc_s {
    /* Queue of messages belonging to the task */
    struct lfds611_queue_state *message_queue;
} task_desc_t;

typedef struct itti_desc_s {
    thread_desc_t *threads;
    task_desc_t   *tasks;

    /* Current message number. Incremented every call to send_msg_to_task */
    message_number_t message_number __attribute__((aligned(8)));

    thread_id_t thread_max;
    task_id_t task_max;
    MessagesIds messages_id_max;

    pthread_t thread_handling_signals;

    const task_info_t *tasks_info;
    const message_info_t *messages_info;

    itti_lte_time_t lte_time;

    int running;

    volatile uint32_t created_tasks;
    volatile uint32_t ready_tasks;
    volatile int      wait_tasks;
#ifdef RTAI
    pthread_t rt_relay_thread;
#endif

#if defined(OAI_EMU) || defined(RTAI)
    memory_pools_handle_t memory_pools_handle;

    uint64_t vcd_poll_msg;
    uint64_t vcd_receive_msg;
    uint64_t vcd_send_msg;
#endif
} itti_desc_t;

static itti_desc_t itti_desc;

void *itti_malloc(task_id_t origin_task_id, task_id_t destination_task_id, ssize_t size)
{
    void *ptr = NULL;

#if defined(OAI_EMU) || defined(RTAI)
    ptr = memory_pools_allocate (itti_desc.memory_pools_handle, size, origin_task_id, destination_task_id);
    if (ptr == NULL)
    {
        char *statistics = memory_pools_statistics (itti_desc.memory_pools_handle);

        ITTI_ERROR (" Memory pools statistics:\n%s", statistics);
        free (statistics);
    }
#else
    ptr = malloc (size);
#endif

    AssertFatal (ptr != NULL, "Memory allocation of %d bytes failed (%d -> %d)!\n", (int) size, origin_task_id, destination_task_id);

    return ptr;
}

int itti_free(task_id_t task_id, void *ptr)
{
    int result = EXIT_SUCCESS;
    AssertFatal (ptr != NULL, "Trying to free a NULL pointer (%d)!\n", task_id);

#if defined(OAI_EMU) || defined(RTAI)
    result = memory_pools_free (itti_desc.memory_pools_handle, ptr, task_id);

    AssertError (result == EXIT_SUCCESS, {}, "Failed to free memory at %p (%d)!\n", ptr, task_id);
#else
    free (ptr);
#endif

    return (result);
}

static inline message_number_t itti_increment_message_number(void) {
    /* Atomic operation supported by GCC: returns the current message number
     * and then increment it by 1.
     * This can be done without mutex.
     */
    return __sync_fetch_and_add (&itti_desc.message_number, 1);
}

static inline uint32_t itti_get_message_priority(MessagesIds message_id) {
    AssertFatal (message_id < itti_desc.messages_id_max, "Message id (%d) is out of range (%d)!\n", message_id, itti_desc.messages_id_max);

    return (itti_desc.messages_info[message_id].priority);
}

const char *itti_get_message_name(MessagesIds message_id) {
    AssertFatal (message_id < itti_desc.messages_id_max, "Message id (%d) is out of range (%d)!\n", message_id, itti_desc.messages_id_max);

    return (itti_desc.messages_info[message_id].name);
}

const char *itti_get_task_name(task_id_t task_id)
{
    if (itti_desc.task_max > 0)
    {
        AssertFatal (task_id < itti_desc.task_max, "Task id (%d) is out of range (%d)!\n", task_id, itti_desc.task_max);
    }
    else
    {
        return ("ITTI NOT INITIALIZED !!!");
    }

    return (itti_desc.tasks_info[task_id].name);
}

static task_id_t itti_get_current_task_id(void)
{
    task_id_t task_id;
    thread_id_t thread_id;
    pthread_t thread = pthread_self ();

    for (task_id = TASK_FIRST; task_id < itti_desc.task_max; task_id++)
    {
        thread_id = TASK_GET_THREAD_ID(task_id);
        if (itti_desc.threads[thread_id].task_thread == thread)
        {
            return task_id;
        }
    }

    return TASK_UNKNOWN;
}

#ifdef RTAI
static void rt_log_debug(char *format, ...)
{
    task_id_t   task_id;
    va_list     args;
    char        log_buffer[FIFO_PRINTF_MAX_STRING_SIZE];
    int         len;

    task_id = itti_get_current_task_id ();
    len = snprintf(log_buffer, FIFO_PRINTF_MAX_STRING_SIZE-1, "[ITTI][D][%s]", itti_get_task_name(task_id));
    va_start(args, format);
    len += vsnprintf(&log_buffer[len], FIFO_PRINTF_MAX_STRING_SIZE-1-len, format, args);
    va_end (args);

    if (task_id != TASK_UNKNOWN)
        fwrite(log_buffer, len, 1, stdout);
    else
        rtf_put (FIFO_PRINTF_NO, log_buffer, len);
}
#endif

void itti_update_lte_time(uint32_t frame, uint8_t slot)
{
    itti_desc.lte_time.frame = frame;
    itti_desc.lte_time.slot = slot;
}

int itti_send_broadcast_message(MessageDef *message_p) {
    task_id_t destination_task_id;
    task_id_t origin_task_id;
    thread_id_t origin_thread_id;
    uint32_t thread_id;
    int ret = 0;
    int result;

    AssertFatal (message_p != NULL, "Trying to broadcast a NULL message!\n");

    origin_task_id = message_p->ittiMsgHeader.originTaskId;
    origin_thread_id = TASK_GET_THREAD_ID(origin_task_id);

    destination_task_id = TASK_FIRST;
    for (thread_id = THREAD_FIRST; thread_id < itti_desc.thread_max; thread_id++) {
        MessageDef *new_message_p;

        while (thread_id != TASK_GET_THREAD_ID(destination_task_id))
        {
            destination_task_id++;
        }
        /* Skip task that broadcast the message */
        if (thread_id != origin_thread_id) {
            /* Skip tasks which are not running */
            if (itti_desc.threads[thread_id].task_state == TASK_STATE_READY) {
                new_message_p = itti_malloc (origin_task_id, destination_task_id, sizeof(MessageDef));
                AssertFatal (new_message_p != NULL, "New message allocation failed!\n");

                memcpy (new_message_p, message_p, sizeof(MessageDef));
                result = itti_send_msg_to_task (destination_task_id, INSTANCE_DEFAULT, new_message_p);
                AssertFatal (result >= 0, "Failed to send message %d to thread %d (task %d)!\n", message_p->ittiMsgHeader.messageId, thread_id, destination_task_id);
            }
        }
    }
    result = itti_free (ITTI_MSG_ORIGIN_ID(message_p), message_p);
    AssertFatal (result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);

    return ret;
}

inline MessageDef *itti_alloc_new_message_sized(task_id_t origin_task_id, MessagesIds message_id, MessageHeaderSize size)
{
    MessageDef *temp = NULL;

    AssertFatal (message_id < itti_desc.messages_id_max, "Message id (%d) is out of range (%d)!\n", message_id, itti_desc.messages_id_max);

#if defined(OAI_EMU) || defined(RTAI)
    vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLE_ITTI_ALLOC_MSG, size);
#endif

    if (origin_task_id == TASK_UNKNOWN)
    {
        /* Try to identify real origin task ID */
        origin_task_id = itti_get_current_task_id();
    }

    temp = itti_malloc (origin_task_id, TASK_UNKNOWN, sizeof(MessageHeader) + size);

    temp->ittiMsgHeader.messageId = message_id;
    temp->ittiMsgHeader.originTaskId = origin_task_id;
    temp->ittiMsgHeader.ittiMsgSize = size;

#if defined(OAI_EMU) || defined(RTAI)
    vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLE_ITTI_ALLOC_MSG, 0);
#endif

    return temp;
}

inline MessageDef *itti_alloc_new_message(task_id_t origin_task_id, MessagesIds message_id)
{
    return itti_alloc_new_message_sized(origin_task_id, message_id, itti_desc.messages_info[message_id].size);
}

int itti_send_msg_to_task(task_id_t destination_task_id, instance_t instance, MessageDef *message)
{
    thread_id_t destination_thread_id;
    thread_id_t origin_task_id;
    message_list_t *new;
    uint32_t priority;
    message_number_t message_number;
    uint32_t message_id;

#if defined(OAI_EMU) || defined(RTAI)
    vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLE_ITTI_SEND_MSG,
                                            __sync_or_and_fetch (&itti_desc.vcd_send_msg, 1L << destination_task_id));
#endif

    AssertFatal (message != NULL, "Message is NULL!\n");
    AssertFatal (destination_task_id < itti_desc.task_max, "Destination task id (%d) is out of range (%d)\n", destination_task_id, itti_desc.task_max);

    destination_thread_id = TASK_GET_THREAD_ID(destination_task_id);
    message->ittiMsgHeader.destinationTaskId = destination_task_id;
    message->ittiMsgHeader.instance = instance;
    message->ittiMsgHeader.lte_time.frame = itti_desc.lte_time.frame;
    message->ittiMsgHeader.lte_time.slot = itti_desc.lte_time.slot;
    message_id = message->ittiMsgHeader.messageId;
    AssertFatal (message_id < itti_desc.messages_id_max, "Message id (%d) is out of range (%d)!\n", message_id, itti_desc.messages_id_max);

    origin_task_id = ITTI_MSG_ORIGIN_ID(message);

    priority = itti_get_message_priority (message_id);

    /* Increment the global message number */
    message_number = itti_increment_message_number ();

    itti_dump_queue_message (origin_task_id, message_number, message, itti_desc.messages_info[message_id].name,
                             sizeof(MessageHeader) + message->ittiMsgHeader.ittiMsgSize);

    if (destination_task_id != TASK_UNKNOWN)
    {
#if defined(OAI_EMU) || defined(RTAI)
        vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_ENQUEUE_MESSAGE, VCD_FUNCTION_IN);

        memory_pools_set_info (itti_desc.memory_pools_handle, message, 1, destination_task_id);
#endif

        if (itti_desc.threads[destination_thread_id].task_state == TASK_STATE_ENDED)
        {
            ITTI_DEBUG(ITTI_DEBUG_ISSUES, " Message %s, number %lu with priority %d can not be sent from %s to queue (%u:%s), ended destination task!\n",
                       itti_desc.messages_info[message_id].name,
                       message_number,
                       priority,
                       itti_get_task_name(origin_task_id),
                       destination_task_id,
                       itti_get_task_name(destination_task_id));
        }
        else
        {
            /* We cannot send a message if the task is not running */
            AssertFatal (itti_desc.threads[destination_thread_id].task_state == TASK_STATE_READY,
                    "Task %s Cannot send message %s (%d) to thread %d, it is not in ready state (%d)!\n",
                    itti_get_task_name(origin_task_id),
                    itti_desc.messages_info[message_id].name,
                    message_id,
                    destination_thread_id,
                    itti_desc.threads[destination_thread_id].task_state);

            /* Allocate new list element */
            new = (message_list_t *) itti_malloc (origin_task_id, destination_task_id, sizeof(struct message_list_s));

            /* Fill in members */
            new->msg = message;
            new->message_number = message_number;
            new->message_priority = priority;

            /* Enqueue message in destination task queue */
            lfds611_queue_enqueue(itti_desc.tasks[destination_task_id].message_queue, new);

#if defined(OAI_EMU) || defined(RTAI)
            vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_ENQUEUE_MESSAGE, VCD_FUNCTION_OUT);
#endif

#ifdef RTAI
            if (itti_desc.threads[TASK_GET_THREAD_ID(origin_task_id)].real_time)
            {
                /* This is a RT task, increase destination task messages pending counter */
                __sync_fetch_and_add (&itti_desc.threads[destination_thread_id].messages_pending, 1);
            }
            else
#endif
            {
                /* Only use event fd for tasks, subtasks will pool the queue */
                if (TASK_GET_PARENT_TASK_ID(destination_task_id) == TASK_UNKNOWN)
                {
                    ssize_t write_ret;
                    eventfd_t sem_counter = 1;

                    /* Call to write for an event fd must be of 8 bytes */
                    write_ret = write (itti_desc.threads[destination_thread_id].task_event_fd, &sem_counter, sizeof(sem_counter));
                    AssertFatal (write_ret == sizeof(sem_counter), "Write to task message FD (%d) failed (%d/%d)\n",
                                 destination_thread_id, (int) write_ret, (int) sizeof(sem_counter));
                }
            }

            ITTI_DEBUG(ITTI_DEBUG_SEND, " Message %s, number %lu with priority %d successfully sent from %s to queue (%u:%s)\n",
                       itti_desc.messages_info[message_id].name,
                       message_number,
                       priority,
                       itti_get_task_name(origin_task_id),
                       destination_task_id,
                       itti_get_task_name(destination_task_id));
        }
    } else {
        /* This is a debug message to TASK_UNKNOWN, we can release safely release it */
        int result = itti_free(origin_task_id, message);
        AssertFatal (result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
    }

#if defined(OAI_EMU) || defined(RTAI)
    vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLE_ITTI_SEND_MSG,
                                            __sync_and_and_fetch (&itti_desc.vcd_send_msg, ~(1L << destination_task_id)));
#endif

    return 0;
}

void itti_subscribe_event_fd(task_id_t task_id, int fd)
{
    thread_id_t thread_id;
    struct epoll_event event;

    AssertFatal (task_id < itti_desc.task_max, "Task id (%d) is out of range (%d)!\n", task_id, itti_desc.task_max);

    thread_id = TASK_GET_THREAD_ID(task_id);
    itti_desc.threads[thread_id].nb_events++;

    /* Reallocate the events */
    itti_desc.threads[thread_id].events = realloc(
        itti_desc.threads[thread_id].events,
        itti_desc.threads[thread_id].nb_events * sizeof(struct epoll_event));

    event.events  = EPOLLIN | EPOLLERR;
    event.data.u64 = 0;
    event.data.fd  = fd;

    /* Add the event fd to the list of monitored events */
    if (epoll_ctl(itti_desc.threads[thread_id].epoll_fd, EPOLL_CTL_ADD, fd,
        &event) != 0)
    {
        /* Always assert on this condition */
        AssertFatal (0, "epoll_ctl (EPOLL_CTL_ADD) failed for task %s, fd %d: %s!\n",
                     itti_get_task_name(task_id), fd, strerror(errno));
    }

    ITTI_DEBUG(ITTI_DEBUG_EVEN_FD, " Successfully subscribed fd %d for task %s\n", fd, itti_get_task_name(task_id));
}

void itti_unsubscribe_event_fd(task_id_t task_id, int fd)
{
    thread_id_t thread_id;

    AssertFatal (task_id < itti_desc.task_max, "Task id (%d) is out of range (%d)!\n", task_id, itti_desc.task_max);
    AssertFatal (fd >= 0, "File descriptor (%d) is invalid!\n", fd);

    thread_id = TASK_GET_THREAD_ID(task_id);
    /* Add the event fd to the list of monitored events */
    if (epoll_ctl(itti_desc.threads[thread_id].epoll_fd, EPOLL_CTL_DEL, fd, NULL) != 0)
    {
        /* Always assert on this condition */
        AssertFatal (0, "epoll_ctl (EPOLL_CTL_DEL) failed for task %s, fd %d: %s!\n",
                     itti_get_task_name(task_id), fd, strerror(errno));
    }

    itti_desc.threads[thread_id].nb_events--;
    itti_desc.threads[thread_id].events = realloc(
        itti_desc.threads[thread_id].events,
        itti_desc.threads[thread_id].nb_events * sizeof(struct epoll_event));
}

int itti_get_events(task_id_t task_id, struct epoll_event **events)
{
    thread_id_t thread_id;

    AssertFatal (task_id < itti_desc.task_max, "Task id (%d) is out of range (%d)\n", task_id, itti_desc.task_max);

    thread_id = TASK_GET_THREAD_ID(task_id);
    *events = itti_desc.threads[thread_id].events;

    return itti_desc.threads[thread_id].epoll_nb_events;
}

static inline void itti_receive_msg_internal_event_fd(task_id_t task_id, uint8_t polling, MessageDef **received_msg)
{
    thread_id_t thread_id;
    int epoll_ret = 0;
    int epoll_timeout = 0;
    int i;

    AssertFatal (task_id < itti_desc.task_max, "Task id (%d) is out of range (%d)!\n", task_id, itti_desc.task_max);
    AssertFatal (received_msg != NULL, "Received message is NULL!\n");

    thread_id = TASK_GET_THREAD_ID(task_id);
    *received_msg = NULL;

    if (polling) {
        /* In polling mode we set the timeout to 0 causing epoll_wait to return
         * immediately.
         */
        epoll_timeout = 0;
    } else {
        /* timeout = -1 causes the epoll_wait to wait indefinitely.
         */
        epoll_timeout = -1;
    }

    do {
        epoll_ret = epoll_wait(itti_desc.threads[thread_id].epoll_fd,
                               itti_desc.threads[thread_id].events,
                               itti_desc.threads[thread_id].nb_events,
                               epoll_timeout);
    } while (epoll_ret < 0 && errno == EINTR);

    if (epoll_ret < 0) {
        AssertFatal (0, "epoll_wait failed for task %s: %s!\n", itti_get_task_name(task_id), strerror(errno));
    }
    if (epoll_ret == 0 && polling) {
        /* No data to read -> return */
        return;
    }

    itti_desc.threads[thread_id].epoll_nb_events = epoll_ret;

    for (i = 0; i < epoll_ret; i++) {
        /* Check if there is an event for ITTI for the event fd */
        if ((itti_desc.threads[thread_id].events[i].events & EPOLLIN) &&
            (itti_desc.threads[thread_id].events[i].data.fd == itti_desc.threads[thread_id].task_event_fd))
        {
            struct message_list_s *message = NULL;
            eventfd_t   sem_counter;
            ssize_t     read_ret;
            int         result;

            /* Read will always return 1 */
            read_ret = read (itti_desc.threads[thread_id].task_event_fd, &sem_counter, sizeof(sem_counter));
            AssertFatal (read_ret == sizeof(sem_counter), "Read from task message FD (%d) failed (%d/%d)!\n", thread_id, (int) read_ret, (int) sizeof(sem_counter));


            if (lfds611_queue_dequeue (itti_desc.tasks[task_id].message_queue, (void **) &message) == 0) {
                /* No element in list -> this should not happen */
                AssertFatal (0, "No message in queue for task %d while there are %d events and some for the messages queue!\n", task_id, epoll_ret);
            }
            AssertFatal(message != NULL, "Message from message queue is NULL!\n");
            *received_msg = message->msg;
            result = itti_free (ITTI_MSG_ORIGIN_ID(*received_msg), message);
            AssertFatal (result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);

            /* Mark that the event has been processed */
            itti_desc.threads[thread_id].events[i].events &= ~EPOLLIN;
            return;
        }
    }
}

void itti_receive_msg(task_id_t task_id, MessageDef **received_msg)
{
#if defined(OAI_EMU) || defined(RTAI)
    vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLE_ITTI_RECV_MSG,
                                            __sync_and_and_fetch (&itti_desc.vcd_receive_msg, ~(1L << task_id)));
#endif

    itti_receive_msg_internal_event_fd(task_id, 0, received_msg);

#if defined(OAI_EMU) || defined(RTAI)
    vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLE_ITTI_RECV_MSG,
                                            __sync_or_and_fetch (&itti_desc.vcd_receive_msg, 1L << task_id));
#endif
}

void itti_poll_msg(task_id_t task_id, MessageDef **received_msg) {
    AssertFatal (task_id < itti_desc.task_max, "Task id (%d) is out of range (%d)!\n", task_id, itti_desc.task_max);

    *received_msg = NULL;

#if defined(OAI_EMU) || defined(RTAI)
    vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLE_ITTI_POLL_MSG,
                                            __sync_or_and_fetch (&itti_desc.vcd_poll_msg, 1L << task_id));
#endif

    {
        struct message_list_s *message;

        if (lfds611_queue_dequeue (itti_desc.tasks[task_id].message_queue, (void **) &message) == 1)
        {
            int result;

            *received_msg = message->msg;
            result = itti_free (ITTI_MSG_ORIGIN_ID(*received_msg), message);
            AssertFatal (result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
        }
    }

    if (*received_msg == NULL) {
        ITTI_DEBUG(ITTI_DEBUG_POLL, " No message in queue[(%u:%s)]\n", task_id, itti_get_task_name(task_id));
    }

#if defined(OAI_EMU) || defined(RTAI)
    vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLE_ITTI_POLL_MSG,
                                            __sync_and_and_fetch (&itti_desc.vcd_poll_msg, ~(1L << task_id)));
#endif
}

int itti_create_task(task_id_t task_id, void *(*start_routine)(void *), void *args_p) {
    thread_id_t thread_id = TASK_GET_THREAD_ID(task_id);
    int result;

    AssertFatal (start_routine != NULL, "Start routine is NULL!\n");
    AssertFatal (thread_id < itti_desc.thread_max, "Thread id (%d) is out of range (%d)!\n", thread_id, itti_desc.thread_max);
    AssertFatal (itti_desc.threads[thread_id].task_state == TASK_STATE_NOT_CONFIGURED, "Task %d, thread %d state is not correct (%d)!\n",
                 task_id, thread_id, itti_desc.threads[thread_id].task_state);

    itti_desc.threads[thread_id].task_state = TASK_STATE_STARTING;

    ITTI_DEBUG(ITTI_DEBUG_INIT, " Creating thread for task %s ...\n", itti_get_task_name(task_id));

    result = pthread_create (&itti_desc.threads[thread_id].task_thread, NULL, start_routine, args_p);
    AssertFatal (result >= 0, "Thread creation for task %d, thread %d failed (%d)!\n", task_id, thread_id, result);

    itti_desc.created_tasks ++;

    /* Wait till the thread is completely ready */
    while (itti_desc.threads[thread_id].task_state != TASK_STATE_READY)
        usleep (1000);

    return 0;
}

//#ifdef RTAI 
void itti_set_task_real_time(task_id_t task_id)
{
    thread_id_t thread_id = TASK_GET_THREAD_ID(task_id);

    DevCheck(thread_id < itti_desc.thread_max, thread_id, itti_desc.thread_max, 0);

    itti_desc.threads[thread_id].real_time = TRUE;
}
//#endif

void itti_wait_ready(int wait_tasks)
{
    itti_desc.wait_tasks = wait_tasks;

    ITTI_DEBUG(ITTI_DEBUG_INIT,
            " wait for tasks: %s, created tasks %d, ready tasks %d\n",
            itti_desc.wait_tasks ? "yes" : "no",
            itti_desc.created_tasks,
            itti_desc.ready_tasks);

    AssertFatal (itti_desc.created_tasks == itti_desc.ready_tasks, "Number of created tasks (%d) does not match ready tasks (%d), wait task %d!\n",
                 itti_desc.created_tasks, itti_desc.ready_tasks, itti_desc.wait_tasks);
}

void itti_mark_task_ready(task_id_t task_id)
{
    thread_id_t thread_id = TASK_GET_THREAD_ID(task_id);

    AssertFatal (thread_id < itti_desc.thread_max, "Thread id (%d) is out of range (%d)!\n", thread_id, itti_desc.thread_max);

    /* Register the thread in itti dump */
    itti_dump_thread_use_ring_buffer();

    /* Mark the thread as using LFDS queue */
    lfds611_queue_use(itti_desc.tasks[task_id].message_queue);

#ifdef RTAI
    /* Assign low priority to created threads */
    {
        struct sched_param sched_param;
        sched_param.sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;
        sched_setscheduler(0, SCHED_FIFO, &sched_param);
    }
#endif

    itti_desc.threads[thread_id].task_state = TASK_STATE_READY;
    itti_desc.ready_tasks ++;

    while (itti_desc.wait_tasks != 0)
    {
        usleep (10000);
    }

    ITTI_DEBUG(ITTI_DEBUG_INIT, " task %s started\n", itti_get_task_name(task_id));
}

void itti_exit_task(void) {
#if defined(OAI_EMU) || defined(RTAI)
    task_id_t task_id = itti_get_current_task_id();

    if (task_id > TASK_UNKNOWN)
    {
        vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLE_ITTI_RECV_MSG,
                                                __sync_and_and_fetch (&itti_desc.vcd_receive_msg, ~(1L << task_id)));
    }
#endif
    pthread_exit (NULL);
}

void itti_terminate_tasks(task_id_t task_id) {
    // Sends Terminate signals to all tasks.
    itti_send_terminate_message (task_id);

    if (itti_desc.thread_handling_signals >= 0) {
        pthread_kill (itti_desc.thread_handling_signals, SIGUSR1);
    }

    pthread_exit (NULL);
}

#ifdef RTAI
static void *itti_rt_relay_thread(void *arg)
{
    thread_id_t thread_id;
    unsigned pending_messages;

    while (itti_desc.running)
    {
        usleep (200); // Poll for messages a little more than 2 time by slot to get a small latency between RT and other tasks

#if defined(OAI_EMU) || defined(RTAI)
        vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_RELAY_THREAD, VCD_FUNCTION_IN);
#endif

        /* Checks for all non real time tasks if they have pending messages */
        for (thread_id = THREAD_FIRST; thread_id < itti_desc.thread_max; thread_id++)
        {
            if ((itti_desc.threads[thread_id].task_state == TASK_STATE_READY)
                    && (itti_desc.threads[thread_id].real_time == FALSE))
            {
                pending_messages = __sync_fetch_and_and (&itti_desc.threads[thread_id].messages_pending, 0);

                if (pending_messages > 0)
                {
                    ssize_t write_ret;
                    eventfd_t sem_counter = pending_messages;

                    /* Call to write for an event fd must be of 8 bytes */
                    write_ret = write (itti_desc.threads[thread_id].task_event_fd, &sem_counter, sizeof(sem_counter));
                    DevCheck(write_ret == sizeof(sem_counter), write_ret, sem_counter, thread_id);
                }
            }
        }

#if defined(OAI_EMU) || defined(RTAI)
        vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_RELAY_THREAD, VCD_FUNCTION_OUT);
#endif
    }
    return NULL;
}
#endif

int itti_init(task_id_t task_max, thread_id_t thread_max, MessagesIds messages_id_max, const task_info_t *tasks_info,
              const message_info_t *messages_info, const char * const messages_definition_xml, const char * const dump_file_name) {
    task_id_t task_id;
    thread_id_t thread_id;
    int ret;

    itti_desc.message_number = 1;

    ITTI_DEBUG(ITTI_DEBUG_INIT, " Init: %d tasks, %d threads, %d messages\n", task_max, thread_max, messages_id_max);

    CHECK_INIT_RETURN(signal_mask());

    /* Saves threads and messages max values */
    itti_desc.task_max = task_max;
    itti_desc.thread_max = thread_max;
    itti_desc.messages_id_max = messages_id_max;
    itti_desc.thread_handling_signals = -1;
    itti_desc.tasks_info = tasks_info;
    itti_desc.messages_info = messages_info;

    /* Allocates memory for tasks info */
    itti_desc.tasks = calloc (itti_desc.task_max, sizeof(task_desc_t));

    /* Allocates memory for threads info */
    itti_desc.threads = calloc (itti_desc.thread_max, sizeof(thread_desc_t));

    /* Initializing each queue and related stuff */
    for (task_id = TASK_FIRST; task_id < itti_desc.task_max; task_id++)
    {
        ITTI_DEBUG(ITTI_DEBUG_INIT, " Initializing %stask %s%s%s\n",
                   itti_desc.tasks_info[task_id].parent_task != TASK_UNKNOWN ? "sub-" : "",
                   itti_desc.tasks_info[task_id].name,
                   itti_desc.tasks_info[task_id].parent_task != TASK_UNKNOWN ? " with parent " : "",
                   itti_desc.tasks_info[task_id].parent_task != TASK_UNKNOWN ?
                   itti_get_task_name(itti_desc.tasks_info[task_id].parent_task) : "");

        ITTI_DEBUG(ITTI_DEBUG_INIT, " Creating queue of message of size %u\n", itti_desc.tasks_info[task_id].queue_size);

        ret = lfds611_queue_new(&itti_desc.tasks[task_id].message_queue, itti_desc.tasks_info[task_id].queue_size);
        if (ret < 0)
        {
            AssertFatal (0, "lfds611_queue_new failed for task %s!\n", itti_get_task_name(task_id));
        }
    }

    /* Initializing each thread */
    for (thread_id = THREAD_FIRST; thread_id < itti_desc.thread_max; thread_id++)
    {
        itti_desc.threads[thread_id].task_state = TASK_STATE_NOT_CONFIGURED;

        itti_desc.threads[thread_id].epoll_fd = epoll_create1(0);
        if (itti_desc.threads[thread_id].epoll_fd == -1) {
            /* Always assert on this condition */
            AssertFatal (0, "Failed to create new epoll fd: %s!\n", strerror(errno));
        }

        itti_desc.threads[thread_id].task_event_fd = eventfd(0, EFD_SEMAPHORE);
        if (itti_desc.threads[thread_id].task_event_fd == -1)
        {
            /* Always assert on this condition */
            AssertFatal (0, " eventfd failed: %s!\n", strerror(errno));
        }

        itti_desc.threads[thread_id].nb_events = 1;

        itti_desc.threads[thread_id].events = calloc(1, sizeof(struct epoll_event));

        itti_desc.threads[thread_id].events->events  = EPOLLIN | EPOLLERR;
        itti_desc.threads[thread_id].events->data.fd = itti_desc.threads[thread_id].task_event_fd;

        /* Add the event fd to the list of monitored events */
        if (epoll_ctl(itti_desc.threads[thread_id].epoll_fd, EPOLL_CTL_ADD,
            itti_desc.threads[thread_id].task_event_fd, itti_desc.threads[thread_id].events) != 0)
        {
            /* Always assert on this condition */
            AssertFatal (0, " epoll_ctl (EPOLL_CTL_ADD) failed: %s!\n", strerror(errno));
        }

        ITTI_DEBUG(ITTI_DEBUG_EVEN_FD, " Successfully subscribed fd %d for thread %d\n",
                   itti_desc.threads[thread_id].task_event_fd, thread_id);

#ifdef RTAI
        itti_desc.threads[thread_id].real_time = FALSE;
        itti_desc.threads[thread_id].messages_pending = 0;
#endif
    }

    itti_desc.running = 1;
    itti_desc.wait_tasks = 0;
    itti_desc.created_tasks = 0;
    itti_desc.ready_tasks = 0;
#ifdef RTAI
    /* Start RT relay thread */
    DevAssert(pthread_create (&itti_desc.rt_relay_thread, NULL, itti_rt_relay_thread, NULL) >= 0);

    rt_global_heap_open();
#endif

#if defined(OAI_EMU) || defined(RTAI)
    itti_desc.memory_pools_handle = memory_pools_create (5);
    memory_pools_add_pool (itti_desc.memory_pools_handle, 1000 + ITTI_QUEUE_MAX_ELEMENTS,       50);
    memory_pools_add_pool (itti_desc.memory_pools_handle, 1000 + (2 * ITTI_QUEUE_MAX_ELEMENTS), 100);
    memory_pools_add_pool (itti_desc.memory_pools_handle, 10000,                                1000);
    memory_pools_add_pool (itti_desc.memory_pools_handle,  400,                                 20050);
    memory_pools_add_pool (itti_desc.memory_pools_handle,  100,                                 30050);

    {
        char *statistics = memory_pools_statistics (itti_desc.memory_pools_handle);

        ITTI_DEBUG(ITTI_DEBUG_MP_STATISTICS, " Memory pools statistics:\n%s", statistics);
        free (statistics);
    }
#endif

#if defined(OAI_EMU) || defined(RTAI)
    itti_desc.vcd_poll_msg = 0;
    itti_desc.vcd_receive_msg = 0;
    itti_desc.vcd_send_msg = 0;
#endif

    itti_dump_init (messages_definition_xml, dump_file_name);

    CHECK_INIT_RETURN(timer_init ());

    return 0;
}

void itti_wait_tasks_end(void) {
    int end = 0;
    int thread_id;
    task_id_t task_id;
    int ready_tasks;
    int result;
    int retries = 10;

    itti_desc.thread_handling_signals = pthread_self ();

    /* Handle signals here */
    while (end == 0) {
        signal_handle (&end);
    }

    printf("closing all tasks\n");

    do {
        ready_tasks = 0;

        task_id = TASK_FIRST;
        for (thread_id = THREAD_FIRST; thread_id < itti_desc.task_max; thread_id++) {
            /* Skip tasks which are not running */
            if (itti_desc.threads[thread_id].task_state == TASK_STATE_READY) {
                while (thread_id != TASK_GET_THREAD_ID(task_id))
                {
                    task_id++;
                }

                result = pthread_tryjoin_np (itti_desc.threads[thread_id].task_thread, NULL);

                ITTI_DEBUG(ITTI_DEBUG_EXIT, " Thread %s join status %d\n", itti_get_task_name(task_id), result);

                if (result == 0) {
                    /* Thread has terminated */
                    itti_desc.threads[thread_id].task_state = TASK_STATE_ENDED;
                }
                else {
                    /* Thread is still running, count it */
                    ready_tasks++;
                }
            }
        }
        if (ready_tasks > 0) {
            usleep (100 * 1000);
        }
    } while ((ready_tasks > 0) && (retries--));

    printf("ready_tasks %d\n",ready_tasks);

    itti_desc.running = 0;

#if defined(OAI_EMU) || defined(RTAI)
    {
        char *statistics = memory_pools_statistics (itti_desc.memory_pools_handle);

        ITTI_DEBUG(ITTI_DEBUG_MP_STATISTICS, " Memory pools statistics:\n%s", statistics);
        free (statistics);
    }
#endif

    if (ready_tasks > 0) {
        ITTI_DEBUG(ITTI_DEBUG_ISSUES, " Some threads are still running, force exit\n");
        exit (0);
    }

    itti_dump_exit();
}

void itti_send_terminate_message(task_id_t task_id) {
    MessageDef *terminate_message_p;

    terminate_message_p = itti_alloc_new_message (task_id, TERMINATE_MESSAGE);

    itti_send_broadcast_message (terminate_message_p);
}
