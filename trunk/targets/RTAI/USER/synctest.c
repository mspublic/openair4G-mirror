#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>

#include <rtai_lxrt.h>
#include <rtai_sem.h>
#include <rtai_msg.h>

#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "SIMULATION/LTE_PHY/openair_hw.h"

#define PERIOD 100000000
#define THRESHOLD 100

static SEM *mutex;
static CND *cond;

static int thread0;
static int thread1;

static int instance_cnt=-1; //0 means worker is busy, -1 means its free
int instance_cnt_ptr_kern,*instance_cnt_ptr_user;

extern unsigned int bigphys_top;
extern unsigned int mem_base;

int oai_exit = 0;

void handler(int sig) {
  void *array[10];
  size_t size;

  oai_exit=1;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}

static void *fun0(void *arg)
{
    RT_TASK *task;
    int done1=0, cnt1=0;
    RTIME right_now;
    int ret;
    unsigned int msg;

    task = rt_task_init_schmod(nam2num("TASK0"), 0, 0, 0, SCHED_FIFO, 0xF);
    mlockall(MCL_CURRENT | MCL_FUTURE);

    rt_printk("fun0: task %p\n",task);

    rt_make_hard_real_time();

    while (!oai_exit)  {
      rt_sem_wait(mutex);
      if ((cnt1%2000)<10) 
	rt_printk("fun0: Hello World %d, instance_cnt %d!\n",cnt1,*instance_cnt_ptr_user);
      while (*instance_cnt_ptr_user<0) {
	rt_sem_signal(mutex);
	rt_receive(0,&msg);
	rt_sem_wait(mutex);
	if ((cnt1%2000)<10) 
	  rt_printk("fun0: instance_cnt %d, msg %d!\n",*instance_cnt_ptr_user,msg);
      }
      
      rt_sem_signal(mutex);
      
      // pretend to do some work here
      if ((cnt1%2000)==0) 
	rt_sleep(PERIOD);
      
      if ((cnt1%2000)<10) 
	rt_printk("fun0: doing very hard work\n");
      cnt1++;
      
      rt_sem_wait(mutex);
      (*instance_cnt_ptr_user)--; 
      //rt_printk("fun0: instance_cnt %d!\n",*instance_cnt_ptr_user);
      rt_sem_signal(mutex);
      
    }
    rt_printk("fun0: finished, ran %d times.\n",cnt1);
    
    rt_make_soft_real_time();

    // clean task
    rt_task_delete(task);
    rt_printk("Task deleted. returning\n");
    return 0;
}


int main(void)
{
    RT_TASK *task;
    RTIME now,last,diff;
    int i,cnt=0,ret=0;

    int openair_fd;
    LTE_DL_FRAME_PARMS *frame_parms;
    u32 carrier_freq[4]={1907600000,1907600000,1907600000,1907600000};
    u32 rxgain[4]={30,30,30,30};

    signal(SIGSEGV, handler); 

    frame_parms = (LTE_DL_FRAME_PARMS*) malloc(sizeof(LTE_DL_FRAME_PARMS));
    frame_parms->N_RB_DL            = 25;
    frame_parms->N_RB_UL            = 25;
    frame_parms->Ncp                = 1;
    frame_parms->Nid_cell           = 0;
    frame_parms->nushift            = 0;
    frame_parms->nb_antennas_tx     = NB_ANTENNAS_TX;
    frame_parms->nb_antennas_rx     = NB_ANTENNAS_RX;
    frame_parms->mode1_flag         = 1; //default == SISO
    frame_parms->tdd_config         = 3;
    frame_parms->dual_tx            = 0;
    frame_parms->frame_type         = 1;
    frame_parms->freq_idx           = 1; //for CBMIMO
    // for Express MIMO
    for (i=0;i<4;i++) {
      frame_parms->carrier_freq[i] = carrier_freq[i];
      frame_parms->rxgain[i]       = rxgain[i];
    }
  
    init_frame_parms(frame_parms,1);
    dump_frame_parms(frame_parms);

    openair_fd=setup_oai_hw(frame_parms);

    // make main thread LXRT soft realtime
    task = rt_task_init_schmod(nam2num("MYTASK"), 9, 0, 0, SCHED_FIFO, 0xF);
    mlockall(MCL_CURRENT | MCL_FUTURE);

    // start realtime timer and scheduler
    //rt_set_oneshot_mode();
    rt_set_periodic_mode();
    start_rt_timer(0);

    //now = rt_get_time() + 10*PERIOD;
    //rt_task_make_periodic(task, now, PERIOD);

    printf("Init mutex and cond.\n");
    //mutex = rt_get_adr(nam2num("MUTEX"));
    mutex = rt_sem_init(nam2num("MUTEX"), 1);
    if (mutex==0)
      printf("Error init mutex\n");

    cond = rt_cond_init(nam2num("CONDITION"));
    //cond = rt_get_adr(nam2num("CONDITION"));
    if (cond==0)
      printf("Error init cond\n");

    printf("mutex=%p, cond=%p\n",mutex,cond);

    instance_cnt_ptr_user = &instance_cnt;
    thread0 = rt_thread_create(fun0, NULL, 10000);
    //thread1 = rt_thread_create(fun1, NULL, 20000);

    printf("thread created\n");

    rt_sleep(PERIOD);

    printf("Sending ioctl\n");
    ioctl(openair_fd,openair_START_LXRT,&instance_cnt_ptr_kern);
    instance_cnt_ptr_user = (unsigned int*) (instance_cnt_ptr_kern -bigphys_top+mem_base);
    *instance_cnt_ptr_user = -1;
    printf("instance_cnt_ptr_kern %p, instance_cnt_ptr_user %p, *instance_cnt_ptr_user %d\n", instance_cnt_ptr_kern, instance_cnt_ptr_user,*instance_cnt_ptr_user);

    // wait for end of program
    printf("TYPE <ENTER> TO TERMINATE\n");
    getchar();

    // stop thread
    oai_exit=1;
    rt_sleep(PERIOD);

    // cleanup
    stop_rt_timer();
    return 0;
}
