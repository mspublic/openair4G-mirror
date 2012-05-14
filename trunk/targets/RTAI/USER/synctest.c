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

#include "PHY/types.h"
#include "PHY/defs.h"

#include "ARCH/COMMON/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_pci.h"
#include "SIMULATION/LTE_PHY/openair_hw.h"

#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#include "SCHED/vars.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"
#include "LAYER2/MAC/vars.h"

#include "../../SIMU/USER/init_lte.h"

#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/vars.h"
#include "RRC/LITE/vars.h"
#include "PHY_INTERFACE/vars.h"
#endif


#ifdef XFORMS
#include <forms.h>
#include "USERSPACE_TOOLS/SCOPE/lte_scope.h"
FD_lte_scope *form_dl=NULL;
#endif //XFORMS

#define FRAME_PERIOD 100000000ULL

#undef MALLOC //there are two conflicting definitions, so we better make sure we don't use it at all

static SEM *mutex;
//static CND *cond;

static int thread0;
static int thread1;
pthread_t  thread2;

static int instance_cnt=-1; //0 means worker is busy, -1 means its free
int instance_cnt_ptr_kern,*instance_cnt_ptr_user;
int pci_interface_ptr_kern;

extern unsigned int bigphys_top;
extern unsigned int mem_base;

int openair_fd = 0;

int oai_exit = 0;

//PCI_interface_t *pci_interface[3];

void signal_handler(int sig)
{
  void *array[10];
  size_t size;

  oai_exit=1;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(-1);
}

void exit_fun(const char* s)
{
  void *array[10];
  size_t size;

  oai_exit=1;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: %s. Exiting!\n",s);
  backtrace_symbols_fd(array, size, 2);
  exit (-1);
}

#ifdef XFORMS
void do_forms2(FD_lte_scope *form, 
	       LTE_DL_FRAME_PARMS *frame_parms, 
	       int pdcch_symbols,
	       s16 **channel, 
	       s16 **channel_f, 
	       s16 **rx_sig, 
	       s16 **rx_sig_f, 
	       s16 *pdcch_comp, 
	       s16 *dlsch_comp, 
	       s16 *dlsch_comp_i, 
	       s16 *dlsch_llr, 
	       s16 *pbch_comp, 
	       s8 *pbch_llr, 
	       int coded_bits_per_codeword)
{

  int i,j,k,s;

  float Re,Im;
  float mag_sig[NB_ANTENNAS_RX*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT],
    sig_time[NB_ANTENNAS_RX*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT],
    sig2[FRAME_LENGTH_COMPLEX_SAMPLES],
    time2[FRAME_LENGTH_COMPLEX_SAMPLES],
    I[25*12*11*4], Q[25*12*11*4],
    *llr,*llr_time;
  int ind;
  float avg, cum_avg;

  extern int* sync_corr_ue0;
  
  //  u16 nsymb = (frame_parms->Ncp == 0) ? 14 : 12;

  llr = malloc(coded_bits_per_codeword*sizeof(float));
  llr_time = malloc(coded_bits_per_codeword*sizeof(float));


  // Channel frequency response
  if (channel_f[0] != NULL) {
    cum_avg = 0;
    ind = 0;
    for (j=0; j<2; j++) { 
      for (i=0;i<frame_parms->nb_antennas_rx;i++) {
	for (k=0;k<(13*frame_parms->N_RB_DL);k++){
	  sig_time[ind] = (float)ind;
	  Re = (float)(channel_f[(j<<1)+i][(2*k)]);
	  Im = (float)(channel_f[(j<<1)+i][(2*k)+1]);
	  //mag_sig[ind] = (short) rand(); 
	  mag_sig[ind] = (short)10*log10(1.0+((double)Re*Re + (double)Im*Im)); 
	  cum_avg += (short)sqrt((double)Re*Re + (double)Im*Im) ;
	  ind++;
	}
	//      ind+=NUMBER_OF_OFDM_CARRIERS/4; // spacing for visualization
      }
    }

    avg = cum_avg/NUMBER_OF_USEFUL_CARRIERS;

    //fl_set_xyplot_ybounds(form->channel_f,30,70);
    fl_set_xyplot_data(form->channel_f,sig_time,mag_sig,ind,"","","");
  }

  // sync_corr
  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++){
    time2[i] = (float) i;
    sig2[i] = (float) sync_corr_ue0[i];
  }
  fl_set_xyplot_data(form->channel_t_im,time2,sig2,FRAME_LENGTH_COMPLEX_SAMPLES,"","","");
  fl_set_xyplot_ybounds(form->channel_t_im,0,1e6);

  // rx sig 0
  for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES; i++)  {
    //for (i=0; i<NUMBER_OF_OFDM_CARRIERS*frame_parms->symbols_per_tti/2; i++)  {
    sig2[i] = 10*log10(1.0+(double) ((rx_sig[0][2*i])*(rx_sig[0][2*i])+(rx_sig[0][2*i+1])*(rx_sig[0][2*i+1])));
    time2[i] = (float) i;
  }
  fl_set_xyplot_ybounds(form->channel_t_re,30,60);
  //fl_set_xyplot_data(form->channel_t_re,&time2[640*12*6],&sig2[640*12*6],640*12,"","","");
  fl_set_xyplot_data(form->channel_t_re,time2,sig2,FRAME_LENGTH_COMPLEX_SAMPLES,"","","");

  /*
  // rx sig 1
  for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES; i++)  {
    //for (i=0; i<NUMBER_OF_OFDM_CARRIERS*frame_parms->symbols_per_tti/2; i++)  {
    sig2[i] = 10*log10(1.0+(double) ((rx_sig[1][2*i])*(rx_sig[1][2*i])+(rx_sig[1][2*i+1])*(rx_sig[1][2*i+1])));
    time2[i] = (float) i;
  }
  fl_set_xyplot_ybounds(form->channel_t_im,30,60);
  //fl_set_xyplot_data(form->channel_t_im,&time2[640*12*6],&sig2[640*12*6],640*12,"","","");
  fl_set_xyplot_data(form->channel_t_im,time2,sig2,FRAME_LENGTH_COMPLEX_SAMPLES,"","","");
  */

  // PBCH LLR
  if (pbch_llr!=NULL) {
    j=0;
    for(i=0;i<1920;i++) {
      llr[j] = (float) pbch_llr[i];
      llr_time[j] = (float) j;
      //if (i==63)
      //  i=127;
      //else if (i==191)
      //  i=319;
      j++;
    }
    
    fl_set_xyplot_data(form->decoder_input,llr_time,llr,1920,"","","");
    //fl_set_xyplot_ybounds(form->decoder_input,-100,100);
  }

  // PBCH I/Q
  if (pbch_comp!=NULL) {
    j=0;
    for(i=0;i<12*12;i++) {
      I[j] = pbch_comp[2*i];
      Q[j] = pbch_comp[2*i+1];
      j++;
      //if (i==47)
      //  i=96;
      //else if (i==191)
      //  i=239;
    }

    fl_set_xyplot_data(form->scatter_plot,I,Q,12*12,"","","");
    //fl_set_xyplot_xbounds(form->scatter_plot,-100,100);
    //fl_set_xyplot_ybounds(form->scatter_plot,-100,100);
  }

  
  // PDCCH I/Q
  j=0;
  for(i=0;i<12*25*1;i++) {
    I[j] = pdcch_comp[2*i];
    Q[j] = pdcch_comp[2*i+1];
    j++;
  }

  fl_set_xyplot_data(form->scatter_plot1,I,Q,12*25*1,"","","");
  //fl_set_xyplot_xbounds(form->scatter_plot,-100,100);
  //fl_set_xyplot_ybounds(form->scatter_plot,-100,100);
  

  // DLSCH LLR
  if (dlsch_llr != NULL) {
    for(i=0;i<coded_bits_per_codeword;i++) {
      llr[i] = (float) dlsch_llr[i];
      llr_time[i] = (float) i;
    }

    fl_set_xyplot_data(form->demod_out,llr_time,llr,coded_bits_per_codeword,"","","");
    //    fl_set_xyplot_ybounds(form->demod_out,-1000,1000);
  }

  // DLSCH I/Q
  if (dlsch_comp!=NULL) {
    j=0;
    for (s=pdcch_symbols;s<frame_parms->symbols_per_tti;s++) {
      for(i=0;i<12*25;i++) {
	I[j] = dlsch_comp[(2*25*12*s)+2*i];
	Q[j] = dlsch_comp[(2*25*12*s)+2*i+1];
	j++;
      }
      //if (s==2)
      //  s=3;
      //else if (s==5)
      //  s=6;
      //else if (s==8)
      //  s=9;
    }
    
    fl_set_xyplot_data(form->scatter_plot2,I,Q,j,"","","");
    //fl_set_xyplot_xbounds(form->scatter_plot,-2000,2000);
    //fl_set_xyplot_ybounds(form->scatter_plot,-2000,2000);
  }


  free(llr);
  free(llr_time);

}  

void *scope_thread(void *arg) {
  while (!oai_exit) {
    do_forms2(form_dl,
	      &(PHY_vars_UE_g[0]->lte_frame_parms),
	      PHY_vars_UE_g[0]->lte_ue_pdcch_vars[0]->num_pdcch_symbols,    
	      (s16**)PHY_vars_UE_g[0]->lte_ue_common_vars.dl_ch_estimates_time,
	      (s16**)PHY_vars_UE_g[0]->lte_ue_common_vars.dl_ch_estimates[0],
	      (s16**)PHY_vars_UE_g[0]->lte_ue_common_vars.rxdata,
	      (s16**)PHY_vars_UE_g[0]->lte_ue_common_vars.rxdataF,
	      (s16*)PHY_vars_UE_g[0]->lte_ue_pdcch_vars[0]->rxdataF_comp[0],
	      (s16*)PHY_vars_UE_g[0]->lte_ue_pdsch_vars[0]->rxdataF_comp[0],
	      (s16*)PHY_vars_UE_g[0]->lte_ue_pdsch_vars[3]->rxdataF_comp[0],
	      (s16*)PHY_vars_UE_g[0]->lte_ue_pdsch_vars[0]->llr[0],
	      (s16*)PHY_vars_UE_g[0]->lte_ue_pbch_vars[0]->rxdataF_comp[0],
	      (s8*)PHY_vars_UE_g[0]->lte_ue_pbch_vars[0]->llr,
	      1920);
    printf("doing forms\n");
    sleep(1);
  }
  return (void*)(1);
}
#endif

/* This is the main eNB thread. It gets woken up by the kernel driver using the RTAI message mechanism (rt_send and rt_receive). */
static void *eNB_thread(void *arg)
{
  RT_TASK *task;
  int slot=0,last_slot, next_slot,frame=0;
  unsigned int msg;
  unsigned int aa,slot_offset, slot_offset_F;

  task = rt_task_init_schmod(nam2num("TASK0"), 0, 0, 0, SCHED_FIFO, 0xF);
  mlockall(MCL_CURRENT | MCL_FUTURE);

  rt_printk("fun0: task %p\n",task);

#ifdef HARD_RT
  rt_make_hard_real_time();
#endif

  while (!oai_exit)
    {
      rt_sem_wait(mutex);
      /*
      if ((slot%2000)<10)
        rt_printk("fun0: Hello World %d, instance_cnt %d!\n",slot,*instance_cnt_ptr_user);
      */
      while (*instance_cnt_ptr_user<0)
        {
          rt_sem_signal(mutex);
          rt_receive(0,&msg);
          rt_sem_wait(mutex);
	  /*
          if ((slot%2000)<10)
            rt_printk("fun0: instance_cnt %d, msg %d!\n",*instance_cnt_ptr_user,msg);
	  */
        }
      rt_sem_signal(mutex);

      slot = msg % LTE_SLOTS_PER_FRAME;
      last_slot = (slot - 1)%LTE_SLOTS_PER_FRAME;
      if (last_slot <0)
        last_slot+=20;
      next_slot = (slot + 1)%LTE_SLOTS_PER_FRAME;

      PHY_vars_eNB_g[0]->frame = frame;
      phy_procedures_eNB_lte (last_slot, next_slot, PHY_vars_eNB_g[0], 0);
#ifndef IFFT_FPGA
      slot_offset_F = (next_slot)*
                      (PHY_vars_eNB_g[0]->lte_frame_parms.ofdm_symbol_size)*
                      ((PHY_vars_eNB_g[0]->lte_frame_parms.Ncp==1) ? 6 : 7);
      slot_offset = (next_slot)*
                    (PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti>>1);

      for (aa=0; aa<PHY_vars_eNB_g[0]->lte_frame_parms.nb_antennas_tx; aa++)
        {
          if (PHY_vars_eNB_g[0]->lte_frame_parms.Ncp == 1)
            {
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
          else
            {
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

      /*
      if ((slot%2000)<10)
        rt_printk("fun0: doing very hard work\n");
      */

      //slot++;
      if ((slot%20)==0)
        frame++;

      rt_sem_wait(mutex);
      (*instance_cnt_ptr_user)--;
      //rt_printk("fun0: instance_cnt %d!\n",*instance_cnt_ptr_user);
      rt_sem_signal(mutex);

    }
  rt_printk("fun0: finished, ran %d times.\n",slot);

#ifdef HARD_RT
  rt_make_soft_real_time();
#endif

  // clean task
  rt_task_delete(task);
  rt_printk("Task deleted. returning\n");
  return 0;
}

/* This is the main UE thread. Initially it is doing a periodic get_frame. One synchronized it gets woken up by the kernel driver using the RTAI message mechanism (rt_send and rt_receive). */
static void *UE_thread(void *arg)
{
  RT_TASK *task;
  RTIME in, out, diff;
  int slot=0,last_slot, next_slot;
  unsigned int msg;
  unsigned int aa,slot_offset, slot_offset_F;
  static int is_synchronized = 0;
  static int received_slots = 0;
  static int slot0 = 0;

  task = rt_task_init_schmod(nam2num("TASK0"), 0, 0, 0, SCHED_FIFO, 0xF);
  mlockall(MCL_CURRENT | MCL_FUTURE);

  rt_printk("fun0: task %p\n",task);

#ifdef HARD_RT
  rt_make_hard_real_time();
#endif

  while (!oai_exit)
    {
      rt_sem_wait(mutex);
      /*
      if ((slot%2000)<10)
        rt_printk("fun0: Hello World %d, instance_cnt %d!\n",slot,*instance_cnt_ptr_user);
      */
      while (*instance_cnt_ptr_user<0)
        {
          rt_sem_signal(mutex);
          rt_receive(0,&msg);
          rt_sem_wait(mutex);
	  /*
          if ((slot%2000)<10)
            rt_printk("fun0: instance_cnt %d, msg %d!\n",*instance_cnt_ptr_user,msg);
	  */
        }

      rt_sem_signal(mutex);

      slot = (msg - slot0) % LTE_SLOTS_PER_FRAME;
      last_slot = (slot - 1)%LTE_SLOTS_PER_FRAME;
      if (last_slot <0)
        last_slot+=LTE_SLOTS_PER_FRAME;
      next_slot = (slot + 1)%LTE_SLOTS_PER_FRAME;


      if (is_synchronized)
        {
	  /*
	  if (PHY_vars_UE_g[0]->frame%100==0)
	    rt_printk("fun0: slot %d, next_slot %d, last_slot %d!\n",slot,last_slot,next_slot);
	  */

	  in = rt_get_time_ns();
          phy_procedures_UE_lte (last_slot, next_slot, PHY_vars_UE_g[0], 0, 0);

          #ifndef IFFT_FPGA
          slot_offset_F = (next_slot)*
            (PHY_vars_UE_g[0]->lte_frame_parms.ofdm_symbol_size)*
            ((PHY_vars_UE_g[0]->lte_frame_parms.Ncp==1) ? 6 : 7);
          slot_offset = (next_slot)*
            (PHY_vars_UE_g[0]->lte_frame_parms.samples_per_tti>>1);

          for (aa=0; aa<PHY_vars_UE_g[0]->lte_frame_parms.nb_antennas_tx; aa++) {
            if (PHY_vars_UE_g[0]->lte_frame_parms.Ncp == 1) {
              PHY_ofdm_mod(&PHY_vars_UE_g[0]->lte_ue_common_vars.txdataF[aa][slot_offset_F],
          #ifdef BIT8_TX
          		 &PHY_vars_UE_g[0]->lte_ue_common_vars.txdata[aa][slot_offset>>1],
          #else
          		 &PHY_vars_UE_g[0]->lte_ue_common_vars.txdata[aa][slot_offset],
          #endif
          		 PHY_vars_UE_g[0]->lte_frame_parms.log2_symbol_size,
          		 6,
          		 PHY_vars_UE_g[0]->lte_frame_parms.nb_prefix_samples,
          		 PHY_vars_UE_g[0]->lte_frame_parms.twiddle_ifft,
          		 PHY_vars_UE_g[0]->lte_frame_parms.rev,
          		 CYCLIC_PREFIX);
            }
            else {
              normal_prefix_mod(&PHY_vars_UE_g[0]->lte_ue_common_vars.txdataF[aa][slot_offset_F],
          #ifdef BIT8_TX
          		      &PHY_vars_UE_g[0]->lte_ue_common_vars.txdata[aa][slot_offset>>1],
          #else
          		      &PHY_vars_UE_g[0]->lte_ue_common_vars.txdata[aa][slot_offset],
          #endif
          		      7,
          		      &(PHY_vars_UE_g[0]->lte_frame_parms));
            }
          }
          #endif //IFFT_FPGA

	  out = rt_get_time_ns();
	  diff = out-in;

	  if (PHY_vars_UE_g[0]->frame % 100 == 0)
	    rt_printk("Frame %d: last_slot %d, phy_procedures_lte_ue time_in %llu, time_out %llu, diff %llu\n", 
		PHY_vars_UE_g[0]->frame, last_slot,
		in,out,diff);
        }
      else   // we are not yet synchronized
        {
	  if (received_slots==0) {
	    ioctl(openair_fd,openair_GET_BUFFER,NULL);
	  }
          if (received_slots==(100*LTE_SLOTS_PER_FRAME)-1)   //we got enough slots so we can do sync (the factor 100 is to wait some time longer)
            {
              rt_printk("fun0: slot %d: doing sync\n",slot);
              received_slots = -1; // will be increased below
              if (initial_sync(PHY_vars_UE_g[0])==0)
                {
                  ioctl(openair_fd,openair_SET_RX_OFFSET,&PHY_vars_UE_g[0]->rx_offset); //synchronize hardware
		  // here we should actually do another dump config with the parameters obtained from the sync. 
		  ioctl(openair_fd,openair_START_TX_SIG,NULL); //start the DMA transfers
		  //for better visualization afterwards
		  for (aa=0; aa<PHY_vars_UE_g[0]->lte_frame_parms.nb_antennas_rx; aa++)
		    memset(PHY_vars_UE_g[0]->lte_ue_common_vars.rxdata[aa],0,
			   PHY_vars_UE_g[0]->lte_frame_parms.samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*sizeof(int));
                  is_synchronized = 1;
                  slot0 = msg;
                }
            }
          received_slots++;
        }

      /*
      if ((slot%2000)<10)
        rt_printk("fun0: doing very hard work\n");
      */

      rt_sem_wait(mutex);
      (*instance_cnt_ptr_user)--;
      //rt_printk("fun0: instance_cnt %d!\n",*instance_cnt_ptr_user);
      rt_sem_signal(mutex);

    }
  rt_printk("fun0: finished, ran %d times.\n",slot);

#ifdef HARD_RT
  rt_make_soft_real_time();
#endif

  // clean task
  rt_task_delete(task);
  rt_printk("Task deleted. returning\n");
  return 0;
}


int main(int argc, char **argv)
{
  RT_TASK *task;
  int i,j;

  LTE_DL_FRAME_PARMS *frame_parms;
  u32 carrier_freq[4]={1907600000,1907600000,1907600000,1907600000};
  u32 rxgain[4]={30,30,30,30};

  u8  eNB_id=0,UE_id=0;
  u16 Nid_cell = 0;
  u8  cooperation_flag=0, transmission_mode=1, abstraction_flag=0;

  char c;
  char UE_flag=0,do_forms=0;
  unsigned int fd;
  unsigned int tcxo = 114;

#ifdef XFORMS
  char title[255];
#endif

  while ((c = getopt (argc, argv, "Ud")) != -1)
    {
      switch (c)
        {
	case 'd':
	  do_forms=1;
	  break;
        case 'U':
          printf("configuring for UE\n");
          UE_flag = 1;
          break;
        default:
          break;
        }
    }

  // initialize the log (see log.h for details)
  logInit();

  // to make a graceful exit when ctrl-c is pressed
  signal(SIGSEGV, signal_handler);

  // init the parameters
  frame_parms = (LTE_DL_FRAME_PARMS*) malloc(sizeof(LTE_DL_FRAME_PARMS));
  frame_parms->N_RB_DL            = 25;
  frame_parms->N_RB_UL            = 25;
  frame_parms->Ncp                = 0;
  frame_parms->Ncp_UL             = 0;
  frame_parms->Nid_cell           = Nid_cell;
  frame_parms->nushift            = 0;
  frame_parms->nb_antennas_tx     = 1;
  frame_parms->nb_antennas_rx     = 1;
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
  for (i=0;i<4;i++)
    {
      frame_parms->carrier_freq[i] = carrier_freq[i];
      frame_parms->rxgain[i]       = rxgain[i];
    }

  init_frame_parms(frame_parms,1);
  dump_frame_parms(frame_parms);

  phy_init_top(frame_parms);
  phy_init_lte_top(frame_parms);

  if (UE_flag==1)
    {
      frame_parms->node_id = NODE;
      PHY_vars_UE_g = malloc(sizeof(PHY_VARS_UE*));
      PHY_vars_UE_g[0] = init_lte_UE(frame_parms, UE_id,abstraction_flag,transmission_mode);
      PHY_vars_UE_g[0]->lte_ue_pdcch_vars[0]->crnti = 0x1234;
      NB_UE_INST=1;
      NB_INST=1;
    }
  else
    {
      frame_parms->node_id = PRIMARY_CH;
      PHY_vars_eNB_g = malloc(sizeof(PHY_VARS_eNB*));
      PHY_vars_eNB_g[0] = init_lte_eNB(frame_parms,eNB_id,Nid_cell,cooperation_flag,transmission_mode,abstraction_flag);
      NB_eNB_INST=1;
      NB_INST=1;
    }

  mac_xface = malloc(sizeof(MAC_xface));
  mac_xface->macphy_exit = &exit_fun;

#ifdef OPENAIR2
  l2_init(frame_parms);
  if (UE_flag == 1)
    mac_xface->dl_phy_sync_success (0, 0, 0, 1);
  else
    mac_xface->mrbch_phy_sync_failure (0, 0, 0);
#endif


  // start up the hardware
  openair_fd=setup_oai_hw(frame_parms);

  number_of_cards = 1;

  // connect the TX/RX buffers
  if (UE_flag==1)
    setup_ue_buffers(PHY_vars_UE_g[0],frame_parms,0);
  else
    {
      setup_eNB_buffers(PHY_vars_eNB_g[0],frame_parms);

      // test signal
      for (j=0;j<PHY_vars_eNB_g[0]->lte_frame_parms.samples_per_tti*10;j+=4)
        {
          ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j] = 0;
          ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+2] = 0x7f;
          ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+4] = 0;
          ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+6] = 0x80;
          ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+1] = 0x7f;
          ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+3] = 0;
          ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+5] = 0x80;
          ((char*)PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0])[2*j+7] = 0;
        }
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

  printf("Init mutex\n");
  //mutex = rt_get_adr(nam2num("MUTEX"));
  mutex = rt_sem_init(nam2num("MUTEX"), 1);
  if (mutex==0)
    {
      printf("Error init mutex\n");
      exit(-1);
    }
  else
    printf("mutex=%p\n",mutex);

  /*
  cond = rt_cond_init(nam2num("CONDITION"));
  //cond = rt_get_adr(nam2num("CONDITION"));
  if (cond==0)
    printf("Error init cond\n");
  */

  // initialize the instance cnt before starting the thread
  instance_cnt_ptr_user = &instance_cnt;

  // start the main thread
  if (UE_flag == 1)
    thread1 = rt_thread_create(UE_thread, NULL, 10000000);
  else
    thread0 = rt_thread_create(eNB_thread, NULL, 10000000);

  rt_sleep(FRAME_PERIOD);
  printf("thread created\n");

  // signal the driver to set up for user-space operation
  // this will initialize the semaphore and the task pointers in the kernel
  // further we receive back the pointer to the shared instance counter which is used to signal if the thread is busy or not. This pointer needs to be mapped to user space.
  ioctl(openair_fd,openair_START_LXRT,&instance_cnt_ptr_kern);
  instance_cnt_ptr_user = (int*) (instance_cnt_ptr_kern -bigphys_top+mem_base);
  *instance_cnt_ptr_user = -1;
  printf("instance_cnt_ptr_kern %p, instance_cnt_ptr_user %p, *instance_cnt_ptr_user %d\n", (void*) instance_cnt_ptr_kern, (void*) instance_cnt_ptr_user,*instance_cnt_ptr_user);

  rt_sleep(FRAME_PERIOD);

  ioctl(openair_fd,openair_SET_TCXO_DAC,(void *)&tcxo);

  ioctl(openair_fd,openair_GET_PCI_INTERFACE,&pci_interface_ptr_kern);
  pci_interface[0] = (PCI_interface_t*) (pci_interface_ptr_kern-bigphys_top+mem_base);
  printf("pci_interface_ptr_kern = %p, pci_interface = %p, tcxo_dac =%d\n", (void*) pci_interface_ptr_kern, pci_interface[0],pci_interface[0]->tcxo_dac);

  // this starts the DMA transfers
  if (UE_flag!=1)
    ioctl(openair_fd,openair_START_TX_SIG,NULL);


#ifdef XFORMS
  if ((do_forms==1) && (UE_flag==1)) {
    fl_initialize (&argc, argv, NULL, 0, 0);
    form_dl = create_form_lte_scope();
    sprintf (title, "LTE DL SCOPE UE");
    fl_show_form (form_dl->lte_scope, FL_PLACE_HOTSPOT, FL_FULLBORDER, title);
    thread2 = pthread_create(&thread2, NULL, scope_thread, NULL);
  }
#endif


  // wait for end of program
  printf("TYPE <ENTER> TO TERMINATE\n");
  getchar();

  // stop threads
  oai_exit=1;
  rt_sleep(FRAME_PERIOD);

  // cleanup
  stop_rt_timer();

  fd = 0;
  ioctl(openair_fd,openair_STOP,&fd);
  munmap((void*)mem_base, BIGPHYS_NUMPAGES*4096);

#ifdef XFORMS
  if ((do_forms==1) && (UE_flag==1)) {
    //pthread_join?
    fl_hide_form(form_dl->lte_scope);
    fl_free_form(form_dl->lte_scope);
  }
#endif

  return 0;
}
