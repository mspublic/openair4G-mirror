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
#include <execinfo.h>

#include <rtai_lxrt.h>
#include <rtai_sem.h>
#include <rtai_msg.h>

#include "ARCH/COMMON/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "SIMULATION/LTE_PHY/openair_hw.h"

// includes for softmodem
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#include "SCHED/vars.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"
#include "LAYER2/MAC/vars.h"

#include "../../SIMU/USER/init_lte.h"

#define PERIOD 100000000
#define THRESHOLD 100

#undef MALLOC //there are two conflicting definitions, so we better make sure we don't use it at all

static SEM *mutex;
static CND *cond;

static int thread0;
static int thread1;

static int instance_cnt=-1; //0 means worker is busy, -1 means its free
int instance_cnt_ptr_kern,*instance_cnt_ptr_user;

extern unsigned int bigphys_top;
extern unsigned int mem_base;

int oai_exit = 0;

void signal_handler(int sig) {
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

void exit_fun(const char* s) {
  rt_printk("%s\n",s);
  
  exit (-1);
}

/* This is the main UE thread. It gets woken up by the kernel driver using the RTAI message mechanism (rt_send and rt_receive). */ 
static void *UE_thread(void *arg)
{
    RT_TASK *task;
    int slot=0,last_slot, next_slot,frame=0;
    unsigned int msg;
    unsigned int aa,slot_offset, slot_offset_F;

    task = rt_task_init_schmod(nam2num("TASK0"), 0, 0, 0, SCHED_FIFO, 0xF);
    mlockall(MCL_CURRENT | MCL_FUTURE);

    rt_printk("fun0: task %p\n",task);

    rt_make_hard_real_time();

    while (!oai_exit)  {
      rt_sem_wait(mutex);
      if ((slot%2000)<10) 
	rt_printk("fun0: Hello World %d, instance_cnt %d!\n",slot,*instance_cnt_ptr_user);
      while (*instance_cnt_ptr_user<0) {
	rt_sem_signal(mutex);
	rt_receive(0,&msg);
	rt_sem_wait(mutex);
	if ((slot%2000)<10) 
	  rt_printk("fun0: instance_cnt %d, msg %d!\n",*instance_cnt_ptr_user,msg);
      }
      
      rt_sem_signal(mutex);
      
      slot = msg % LTE_SLOTS_PER_FRAME;
      last_slot = (slot - 1)%20;
      if (last_slot <0)
	last_slot+=20;
      next_slot = (slot + 1)%20;

      PHY_vars_eNB_g[0]->frame = frame;
      phy_procedures_eNB_lte (last_slot, next_slot, PHY_vars_eNB_g[0], 0);
 #ifndef IFFT_FPGA
	  slot_offset_F = (next_slot)*
	    (PHY_vars_eNB_g[0]->lte_frame_parms.ofdm_symbol_size)*
	    ((PHY_vars_eNB_g[0]->lte_frame_parms.Ncp==1) ? 6 : 7);
	  slot_offset = (next_slot)*
	    (PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti>>1);

	  for (aa=0; aa<PHY_vars_eNB_g[0]->lte_frame_parms.nb_antennas_tx; aa++) {
	    if (PHY_vars_eNB_g[0]->lte_frame_parms.Ncp == 1) {
	      PHY_ofdm_mod(&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][aa][slot_offset_F],
#ifdef BIT8_TX
			   &PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][slot_offset>>1],
#else
			   &PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][slot_offset],
#endif
			   PHY_vars_eNB_g[0]->lte_frame_parms.log2_symbol_size,
			   6,
			   PHY_vars_eNB_g[0]->lte_frame_parms.nb_prefix_samples,
			   PHY_vars_eNB_g[0]->lte_frame_parms.twiddle_ifft,
			   PHY_vars_eNB_g[0]->lte_frame_parms.rev,
			   CYCLIC_PREFIX);
	    }
	    else {
	      normal_prefix_mod(&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][aa][slot_offset_F],
#ifdef BIT8_TX
				&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][slot_offset>>1],
#else
				&PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][aa][slot_offset],
#endif
				7,
				&(PHY_vars_eNB_g[0]->lte_frame_parms));
	    }
	  }
#endif //IFFT_FPGA
     
      if ((slot%2000)<10) 
	rt_printk("fun0: doing very hard work\n");

      //slot++;
      if ((slot%20)==0)
	frame++;
      
      rt_sem_wait(mutex);
      (*instance_cnt_ptr_user)--; 
      //rt_printk("fun0: instance_cnt %d!\n",*instance_cnt_ptr_user);
      rt_sem_signal(mutex);
      
    }
    rt_printk("fun0: finished, ran %d times.\n",slot);
    
    rt_make_soft_real_time();

    // clean task
    rt_task_delete(task);
    rt_printk("Task deleted. returning\n");
    return 0;
}

int main(void)
{
    RT_TASK *task;
    int i,j;

    int openair_fd;
    LTE_DL_FRAME_PARMS *frame_parms;
    u32 carrier_freq[4]={1907600000,1907600000,1907600000,1907600000};
    u32 rxgain[4]={30,30,30,30};

    u8  eNB_id=0;
    u16 Nid_cell = 0;
    u8  cooperation_flag=0, transmission_mode=1, abstraction_flag=0;

    // initialize the log (see log.h for details)
    logInit();

    // to make a graceful exit when ctrl-c is pressed
    signal(SIGSEGV, signal_handler); 

    // init the parameters
    frame_parms = (LTE_DL_FRAME_PARMS*) malloc(sizeof(LTE_DL_FRAME_PARMS));
    frame_parms->N_RB_DL            = 25;
    frame_parms->N_RB_UL            = 25;
    frame_parms->Ncp                = 1;
    frame_parms->Ncp_UL             = 1;
    frame_parms->Nid_cell           = Nid_cell;
    frame_parms->nushift            = 0;
    frame_parms->nb_antennas_tx     = NB_ANTENNAS_TX;
    frame_parms->nb_antennas_rx     = NB_ANTENNAS_RX;
    frame_parms->mode1_flag         = 1; //default == SISO
    frame_parms->frame_type         = 1;
    frame_parms->tdd_config         = 3;
    frame_parms->tdd_config_S       = 0;
    frame_parms->phich_config_common.phich_resource = oneSixth;
    frame_parms->phich_config_common.phich_duration = normal;
    frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift = 0;//n_DMRS1 set to 0

    // hardware specific parameters
    // for CBMIMO
    frame_parms->dual_tx            = 0;
    frame_parms->freq_idx           = 1; 
    // for Express MIMO
    for (i=0;i<4;i++) {
      frame_parms->carrier_freq[i] = carrier_freq[i];
      frame_parms->rxgain[i]       = rxgain[i];
    }
  
    init_frame_parms(frame_parms,1);
    dump_frame_parms(frame_parms);

    phy_init_top(frame_parms);
    phy_init_lte_top(frame_parms);

    PHY_vars_eNB_g = malloc(sizeof(PHY_VARS_eNB*));
    PHY_vars_eNB_g[0] = init_lte_eNB(frame_parms,eNB_id,Nid_cell,cooperation_flag,transmission_mode,abstraction_flag);
    
    /*
    PHY_vars_UE_g = malloc(sizeof(PHY_VARS_UE*));
    PHY_vars_UE_g[0] = init_lte_UE(frame_parms, UE_id,abstraction_flag,transmission_mode);
    */

    mac_xface = malloc(sizeof(MAC_xface));
    mac_xface->macphy_exit = &exit_fun;

    // start up the hardware
    openair_fd=setup_oai_hw(frame_parms);

    number_of_cards = 1;

    // connect the TX/RX buffers 
    setup_eNB_buffers(PHY_vars_eNB_g[0],frame_parms);
    //setup_ue_buffers(PHY_vars_UE_g[0],frame_parms,0);

    // test signal
    for (j=0;j<PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti*10;j+=4) {
      ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j] = 0;
      ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+2] = 0x7f;
      ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+4] = 0;
      ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+6] = 0x80;
      ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+1] = 0x7f;
      ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+3] = 0;
      ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+5] = 0x80;
      ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+7] = 0;
    }
    
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

    // initialize the instance cnt before starting the thread
    instance_cnt_ptr_user = &instance_cnt;

    // start the main thread
    thread0 = rt_thread_create(UE_thread, NULL, 10000);
    //thread1 = rt_thread_create(fun1, NULL, 20000);

    rt_sleep(PERIOD);
    printf("thread created\n");

    // signal the driver to set up for user-space operation
    // this will initialize the semaphore and the task pointers in the kernel
    // further we receive back the pointer to the shared instance counter which is used to signal if the thread is busy or not. This pointer needs to be mapped to user space.
    ioctl(openair_fd,openair_START_LXRT,&instance_cnt_ptr_kern);
    instance_cnt_ptr_user = (int*) (instance_cnt_ptr_kern -bigphys_top+mem_base);
    *instance_cnt_ptr_user = -1;
    printf("instance_cnt_ptr_kern %p, instance_cnt_ptr_user %p, *instance_cnt_ptr_user %d\n", (void*) instance_cnt_ptr_kern, (void*) instance_cnt_ptr_user,*instance_cnt_ptr_user);

    // this starts the DMA transfers 
    ioctl(openair_fd,openair_START_TX_SIG,NULL);

    // wait for end of program
    printf("TYPE <ENTER> TO TERMINATE\n");
    getchar();

    // stop thread
    oai_exit=1;
    rt_sleep(PERIOD);
    
    // cleanup
    stop_rt_timer();
    
    ioctl(openair_fd,openair_STOP,NULL);
    munmap((void*)mem_base, BIGPHYS_NUMPAGES*4096);

    return 0;
}
