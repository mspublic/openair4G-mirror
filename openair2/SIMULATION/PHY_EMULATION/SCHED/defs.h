/*!\brief SCHED function prototypes */

#ifndef __SCHED_PROTO_H__
#define __SCHED_PROTO_H__


/*
#ifndef USER_MODE
#ifdef RTAI
static void PHY_thread(int param);
static void PHY_scheduler(int param);
#endif
#else
static void *PHY_thread(void *param);
static void *PHY_scheduler(void *param);
#endif

#ifndef USER_MODE
#ifdef RTAI
int openair_sched_init(void);
int openair_sched_cleanup(void);
#endif
#else
*/

typedef struct {
  unsigned char mode;
  unsigned char node_configured;
  unsigned char node_running;
  unsigned char mac_registered;
  unsigned int sched_cnt;
  unsigned int instance_cnt;
  unsigned int slot_count;
  unsigned int scheduler_interval_ns;
} OPENAIR_SCHED_VARS;

int emul_sched_init(void);
void emul_sched_cleanup(void);
void emul_sched_exit(void);


enum THREAD_INDEX { OPENAIR_THREAD_INDEX = 0,
		    TOP_LEVEL_SCHEDULER_THREAD_INDEX,
                    openair_SCHED_NB_THREADS}; // do not modify this line

#define OPENAIR_THREAD_PRIORITY        255


#define OPENAIR_THREAD_STACK_SIZE    4096//RTL_PTHREAD_STACK_MIN*6


enum openair_SYNCH_STATUS {
      openair_NOT_SYNCHED=1,
      openair_SYNCHED_TO_CHSCH,
      openair_SYNCHED_TO_MRSCH};


#endif /*__SCHED_PROTO_H__ */
