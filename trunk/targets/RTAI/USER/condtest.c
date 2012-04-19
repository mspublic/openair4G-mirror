#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>


#include <rtai_lxrt.h>
#include <rtai_sem.h>
#include <rtai_msg.h>

#define PERIOD 100000
#define THRESHOLD 10

static SEM *mutex;
static CND *cond;

static int thread0;
static int thread1;

static int instance_cnt=0;

static void *fun0(void *arg)
{
    RT_TASK *task;
    int done1=0, cnt1;
    RTIME right_now;

    task = rt_task_init_schmod(nam2num("TASK0"), 0, 0, 0, SCHED_FIFO, 0xF);
    mlockall(MCL_CURRENT | MCL_FUTURE);
    //set_periodic_mode();

    //rt_make_hard_real_time();
    // makes task hard real time (default: soft)
    // uncomment the next line when developing : develop in soft real time mode 
    right_now = rt_get_time() + 10*PERIOD;
    rt_task_make_periodic(task, right_now, PERIOD);

    
    //void rt_sem_init(SEM *sem, int value)   

    while (cnt1 < THRESHOLD)
	 {
		rt_sem_wait(mutex);
		rt_cond_wait(cond, mutex); 
    		rt_printk("Hello World!\n");
		instance_cnt++;
		cnt1++;
		rt_sem_signal(mutex);
	}
    // makes task soft real time
    rt_make_soft_real_time();

    printf("Back to soft RT\n");
    // clean task
    rt_task_delete(task);
    printf("Task deleted. returning\n");
    return 0;
}


int main(void)
{
    RT_TASK *task;
    RTIME now;
    int cnt=0;

    // make main thread LXRT soft realtime
    task = rt_task_init_schmod(nam2num("MYTASK"), 9, 0, 0, SCHED_FIFO, 0xF);
    mlockall(MCL_CURRENT | MCL_FUTURE);

    // start realtime timer and scheduler
    //rt_set_oneshot_mode();
    rt_set_periodic_mode();
    start_rt_timer(0);

    now = rt_get_time() + 10*PERIOD;
    rt_task_make_periodic(task, now, PERIOD);

    printf("Init mutex and cond.\n");
    mutex = rt_typed_sem_init(nam2num("MUTEX"), 0, BIN_SEM);
    cond = rt_cond_init(nam2num("CONDITION"));

    while (cnt < THRESHOLD)
         {
                rt_task_wait_period();
		rt_sem_wait(mutex);
		thread0 = rt_thread_create(fun0, NULL, 10000);
    		//thread1 = rt_thread_create(fun1, NULL, 20000);

                rt_printk("Hello World!\n");
		rt_sem_signal(mutex);
		rt_cond_signal(cond);
        }
    // wait for end of program
    printf("TYPE <ENTER> TO TERMINATE\n");
    getchar();

    // cleanup
    stop_rt_timer();
    return 0;
}
