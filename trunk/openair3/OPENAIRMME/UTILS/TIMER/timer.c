#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "intertask_interface.h"
#include "timer.h"

#include "queue.h"

#define SIG SIGRTMIN

#ifndef TIMER_DEBUG
# define TIMER_DEBUG(x, args...) do { fprintf(stdout, "[TMR][D] "x, ##args); } while(0)
#endif
#ifndef TIMER_ERROR
# define TIMER_ERROR(x, args...) do { fprintf(stdout, "[TMR][D] "x, ##args); } while(0)
#endif

/* TIMER task thread: read messages from other tasks */
static pthread_t timer_task_thread;

static timer_t timer_id;
static STAILQ_HEAD(timer_list_head, timer_elm) timer_queue;
/* Ticks from beginning (in ms) */
static uint32_t tick = 0;
/* Root timer resolution in ms */
static uint32_t timer_resolution = 0;
static int timer_setup(int interval_ms);

struct timer_elm {
    uint32_t expiry;        ///< Expiry after
    uint32_t task_id;
    uint32_t timer_id;
    STAILQ_ENTRY(timer_elm) entries;
};

#define TIMER_SEARCH(var, timerfield, timervalue, taskfield, taskvalue) \
do {        \
    STAILQ_FOREACH(var, &timer_queue, entries) {    \
        if (((var)->timerfield == timervalue) &&    \
            ((var)->taskfield == taskvalue))        \
            break;                                  \
    }                                               \
} while(0)

static void timer_expiry_handler(int sig, siginfo_t *si_p, void *arg_p) {
    struct timer_elm *timer;
    tick++;
    STAILQ_FOREACH(timer, &timer_queue, entries) {
        if (timer->expiry == tick) {
            /* TODO: notify task
             * Remove timer from list of active timers
             */
            STAILQ_REMOVE(&timer_queue, timer, timer_elm, entries);
        }
    }
}

static int timer_setup(int interval_ms) {
    struct sigevent   se;
    struct itimerspec its;
    struct sigaction  sa;

    timer_resolution = interval_ms;

    /* Setting up signal handler */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_expiry_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) < 0) {
        TIMER_ERROR("Failed to setup signal handler (%s)\n", strerror(errno));
        return -1;
    }

    /* Setting up alarm */
    /* Set and enable alarm */
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIG;
    se.sigev_value.sival_ptr = &timer_id;
    timer_create(CLOCK_REALTIME, &se, &timer_id);

    its.it_interval.tv_sec = interval_ms / 1000;
    its.it_interval.tv_nsec = (interval_ms % 1000) * 1000000;
    its.it_value.tv_sec = interval_ms / 1000;
    its.it_value.tv_nsec = (interval_ms % 1000) * 1000000;
    timer_settime(timer_id, 0, &its, NULL);
    return 0;
}

static void *timer_intertask_interface(void *args_p) {
    while(1) {
        MessageDef *received_message_p;
        receive_msg(TASK_TIMER, &received_message_p);
        switch(received_message_p->messageId) {
            case TIMER_NEW_REQUEST:
            {
                TimerNewRequest  *timer_req_p;
                struct timer_elm *timer_p;

                timer_req_p = &received_message_p->msg.timerNewRequest;
                timer_p = malloc(sizeof(struct timer_elm));
                if (timer_p == NULL) {
                    TIMER_ERROR("Failed to create new timer element\n");
                    break;
                }
                timer_p->task_id = received_message_p->originTaskId;
                timer_p->expiry = tick + (timer_req_p->expiry_ms / timer_resolution);
                timer_p->timer_id = timer_req_p->timer_id;
                STAILQ_INSERT_TAIL(&timer_queue, timer_p, entries);
            } break;
            case TIMER_REMOVE:
            {
                TimerRemove *timer_removed_p;
                struct timer_elm *timer_p;

                timer_removed_p = &received_message_p->msg.timerRemove;
                TIMER_SEARCH(timer_p, timer_id, timer_removed_p->timer_id,
                             task_id, received_message_p->originTaskId);
                
            } break;
            default:
            {
                TIMER_DEBUG("Unknown message ID %d\n", received_message_p->messageId);
            } break;
        }
        free(received_message_p);
        received_message_p = NULL;
    }
    return NULL;
}

int timer_init(const mme_config_t *mme_config) {
    TIMER_DEBUG("Initializing TIMER task interface\n");
    STAILQ_INIT(&timer_queue);
    if (pthread_create(&timer_task_thread, NULL, &timer_intertask_interface, NULL) < 0) {
        TIMER_ERROR("udp pthread_create (%s)\n", strerror(errno));
        return -1;
    }
    if (timer_setup(mme_config->timer_config.root_timer_value_ms) < 0) {
        TIMER_ERROR("setup_timer failed\n");
        return -1;
    }
    TIMER_DEBUG("Initializing TIMER task interface: DONE\n");
    return 0;
}
