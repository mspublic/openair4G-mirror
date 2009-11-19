#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#ifdef RTAI_ENABLED
#include <rtai.h>
//#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif // RTAI_ENABLED

#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/delay.h>
#include <asm/param.h>

#include <linux/init.h>
#include <linux/module.h>
#include <asm/ioctl.h>
//#include <linux/malloc.h>
#endif //

#include "cbmimo1_device.h"
#include "defs.h"
#include "extern.h"

#ifdef RTAI_ENABLED
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/INIT/defs.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#endif // RTAI_ENABLED

#include "from_grlib_softconfig.h"
#include "from_grlib_softregs.h"
#include "cbmimo1_pci.h"

int dummy_cnt = 0;

#ifdef BIGPHYSAREA
extern int bigphys_ptr;
#endif

struct struct_NEWRF openair_NEWRF_RFctrl;

//-----------------------------------------------------------------------------
int openair_device_open (struct inode *inode,struct file *filp) {
  //-----------------------------------------------------------------------------
  printk("[openair][MODULE]  openair_open()\n");
#ifdef KERNEL2_4
 MOD_INC_USE_COUNT;
#endif //
  return 0;
}
//-----------------------------------------------------------------------------
int openair_device_release (struct inode *inode,struct file *filp) {
  //-----------------------------------------------------------------------------
  printk("[openair][MODULE]  openair_release(), MODE = %d\n",openair_daq_vars.mode);
#ifdef KERNEL2_4
 MOD_DEC_USE_COUNT;
#endif // KERNEL2_4
  return 0;
}
//-----------------------------------------------------------------------------
int openair_device_mmap(struct file *filp, struct vm_area_struct *vma) {
  //-----------------------------------------------------------------------------
  
  unsigned long phys,pos;
  unsigned long start = (unsigned long)vma->vm_start; 
  unsigned long size = (unsigned long)(vma->vm_end-vma->vm_start); 


  /*
  printk("[openair][MMAP]  called (%x,%x,%x) prot %x\n", 
	 vma->vm_start, 
	 vma->vm_end, 
	 size,
	 vma->vm_page_prot);
  */

#ifdef BIGPHYSAREA  
  
  vma->vm_flags |= VM_RESERVED;

  /* if userspace tries to mmap beyond end of our buffer, fail */ 

  if (size>4096*PAGE_SIZE) {
    printk("[openair][MMAP][ERROR] Trying to map more than %d bytes (%d)\n",
	   (unsigned int)(4096*PAGE_SIZE),
	   (unsigned int)size);
    return -EINVAL;
  }
  /* start off at the PCI BAR0 */


  pos = (unsigned long) PHY_vars->tx_vars[0].TX_DMA_BUFFER;
  phys = virt_to_phys((void *)pos);
  
  //  printk("[openair][MMAP]  WILL START MAPPING AT %p (%p) \n", (void*)pos,virt_to_phys(pos));
  
  /* loop through all the physical pages in the buffer */ 
  /* Remember this won't work for vmalloc()d memory ! */

  if (remap_pfn_range(vma, 
		      start, 
		      phys>>PAGE_SHIFT, 
		      vma->vm_end-vma->vm_start, 
		      vma->vm_page_prot)) {
    
    printk("[openair][MMAP] ERROR EAGAIN\n");
    return -EAGAIN;
  }

  //  for (i=0;i<16;i++)
  //    printk("[openair][MMAP] rxsig %d = %x\n",i,PHY_vars->rx_vars[0].RX_DMA_BUFFER[i]);


  /*
  while (size > 0) {
    

    
    printk("[openair][MMAP] Mapping phys %x,virt %x\n",phys,start);


    if (remap_pfn_range(vma, 
			start, 
			phys>>PAGE_SHIFT, 
			PAGE_SIZE, 
			vma->vm_page_prot)) {
      
      printk("[openair][MMAP] ERROR EAGAIN\n");
      return -EAGAIN;
    }
    
    start+=PAGE_SIZE;
    pos+=PAGE_SIZE;
    
    if (size > PAGE_SIZE)
      size-=PAGE_SIZE;
    else {
      size = 0;
    }
    }
  */


#endif //BIGPHYSAREA
  return 0; 
}

//-----------------------------------------------------------------------------
int openair_device_ioctl(struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg) {
  /* arg is not meaningful if no arg is passed in user space */
  //-----------------------------------------------------------------------------
   int ret=-1;
   int i;


  void *arg_ptr = (void *)arg;




  unsigned char *scale;
  unsigned char scale_mem;
  int tmp;
  unsigned int ltmp;
#define invert4(x)        {ltmp=x; x=((ltmp & 0xff)<<24) | ((ltmp & 0xff00)<<8) | \
                       ((ltmp & 0xff0000)>>8) | ((ltmp & 0xff000000)>>24); }

  static unsigned int fmw_off;
  static unsigned int update_firmware_command;
  static unsigned int update_firmware_address;
  static unsigned int update_firmware_length;
  static unsigned int* update_firmware_kbuffer;
  static unsigned int* __user update_firmware_ubuffer;
  static unsigned int update_firmware_start_address;
  static unsigned int update_firmware_stack_pointer;
  static unsigned int update_firmware_bss_address;
  static unsigned int update_firmware_bss_size;
  unsigned int sparc_tmp_0;
  unsigned int sparc_tmp_1;
  static unsigned int lendian_length;
  static unsigned int bendian_fmw_off;
  unsigned int ioctl_ack_cnt = 0;

  TX_VARS dummy_tx_vars[NB_ANTENNAS_TX];

  scale = &scale_mem;

  printk("[openair][IOCTL]:  : In ioctl(), ioctl = %x (%x,%x)\n",cmd,openair_START_1ARY_CLUSTERHEAD,openair_START_NODE);
  
  switch(cmd) {
    

  case openair_TEST_FPGA:

      break;


    //----------------------
  case openair_DUMP_CONFIG:
    //----------------------
    printk("[openair][IOCTL]     openair_DUMP_CONFIG\n");


#ifdef EMOS
	openair_daq_vars.mac_registered=1;
#endif
#ifndef OPENAIR2 
	openair_daq_vars.mac_registered=1;
#endif

#ifdef RTAI_ENABLED
    if (openair_daq_vars.node_configured == 1) {
      printk("[openair][IOCTL] NODE ALREADY CONFIGURED, DYNAMIC RECONFIGURATION NOT SUPPORTED YET!!!!!!!\n");
    }
    else {
      if (openair_daq_vars.mac_registered == 1) {
	copy_from_user((char *)PHY_config,(char *)arg,sizeof(PHY_CONFIG));
	dump_config();
	printk("[openair][IOCTL] Allocating PHY variables\n");

#ifdef OPENAIR_LTE
	lte_frame_parms = &PHY_config->lte_frame_parms;
	lte_ue_common_vars = &PHY_vars->lte_ue_common_vars;
	lte_ue_dlsch_vars = &PHY_vars->lte_ue_dlsch_vars;
	lte_ue_pbch_vars = &PHY_vars->lte_ue_pbch_vars;
	  
	openair_daq_vars.node_configured = phy_init_top(NB_ANTENNAS_TX);
	msg("[openair][IOCTL] phy_init_top done: %d\n",openair_daq_vars.node_configured);

	lte_frame_parms->twiddle_fft      = twiddle_fft;
	lte_frame_parms->twiddle_ifft     = twiddle_ifft;
	lte_frame_parms->rev              = rev;

	openair_daq_vars.node_configured += phy_init_lte_ue(lte_frame_parms, lte_ue_common_vars,lte_ue_dlsch_vars, lte_ue_pbch_vars);
	msg("[openair][IOCTL] phy_init_lte done: %d\n",openair_daq_vars.node_configured);
#else
	openair_daq_vars.node_configured = phy_init(NB_ANTENNAS_TX);
#endif
	if (openair_daq_vars.node_configured < 0) {
	  printk("[openair][IOCTL] Error in configuring PHY\n");
	  break;
	}
	
	else {
	  printk("[openair][IOCTL] PHY Configuration successful\n");


#ifndef EMOS	  
	  openair_daq_vars.node_configured = mac_init();
	  if (openair_daq_vars.node_configured != 1)
	    printk("[openair][IOCTL] Error in configuring MAC\n");
	  else 
	    printk("[openair][IOCTL] MAC Configuration successful\n");
#endif	
	}

#ifndef NOCARD_TEST
      // Initialize FPGA PCI registers
       
	openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
	printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);

	openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<3);
	openair_daq_vars.rx_gain_val = 0;

	// PUT the card in calibrated frequency mode by putting a value > 255 in tcxo register
	openair_daq_vars.tcxo_dac = 256;

	openair_daq_vars.node_id = NODE;
	openair_daq_vars.mode    = openair_NOT_SYNCHED;
	openair_daq_vars.node_running = 0;

	openair_daq_vars.timing_advance = 19;

	openair_daq_vars.dual_tx = PHY_config->dual_tx;
#ifdef OPENAIR_LTE
	openair_daq_vars.tdd = 0; //FDD
#endif

	mac_xface->is_cluster_head = 0;

	ret = setup_regs();

	// Start LED dance with proper period
	openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
	
	//	usleep(10);
	ret = openair_sched_init();
	//ret = -1;
	if (ret != 0)
	  printk("[openair][DUMP][CONFIG] Error in starting scheduler\n");
	
	// add Layer 1 stats in /proc/openair	
	add_openair1_stats();
	// rt_preempt_always(1);
#endif //NOCARD_TEST
      }
      else {
	printk("[openair][IOCTL] Add a MAC module first !!!!\n");
      }
    }
#endif // RTAI_ENABLED
    break;

    //----------------------
  case openair_START_1ARY_CLUSTERHEAD:
    //----------------------

#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_1ARY_START_CLUSTERHEAD\n");
    printk("[openair][IOCTL]     Freq corr = %d, Freq0 = %d, Freq1 = %d, NODE_ID = %d\n",*((unsigned int *)arg_ptr)&1,
	   (*((unsigned int *)arg_ptr)>>1)&3,
	   (*((unsigned int *)arg_ptr)>>3)&3,
	   (*((unsigned int *)arg_ptr)>>5)&0xFF);


    if ( (openair_daq_vars.node_configured == 1) && 
	 (openair_daq_vars.node_running == 0) && 
	 (openair_daq_vars.mac_registered == 1)) {



      mac_xface->is_cluster_head = 1;
      mac_xface->is_primary_cluster_head = 1;
      mac_xface->is_secondary_cluster_head = 0;
      mac_xface->cluster_head_index = 0;
      NODE_ID[0] = ((*((unsigned int *)arg_ptr))>>5)&0xFF;

      mac_xface->slots_per_frame = SLOTS_PER_FRAME;

      // Initialize MAC layer

      printk("[OPENAIR][IOCTL] MAC Init, is_cluster_head = %d (%p).slots_per_frame = %d (mac_xface %p)\n",mac_xface->is_cluster_head,&mac_xface->is_cluster_head,mac_xface->slots_per_frame,mac_xface);
       mac_xface->macphy_init();

#ifndef OPENAIR_LTE
      openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL;
#endif //OPENAIR_LTE

      openair_daq_vars.node_id = PRIMARY_CH;
      openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
      printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
	
      openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<3);
      //ret = setup_regs();

      /*
      if (ret == 0) {
	openair_daq_vars.mode = openair_SYNCHED_TO_MRSCH;
	openair_daq_vars.node_running = 1;
	openair_daq_vars.sync_state = 0;
	printk("[openair][IOCTL] Process initialization return code %d\n",ret);
      }
      */

    }
    else {
      printk("[openair][IOCTL] Radio (%d) or Mac (%d) not configured\n",openair_daq_vars.node_configured,openair_daq_vars.mac_registered);
    }
  


#endif // RTAI_ENABLED
    break;

    //----------------------
  case openair_START_1ARY_CLUSTERHEAD_COGNITIVE:
    //----------------------

#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_1ARY_START_CLUSTERHEAD_COGNITIVE\n");
    printk("[openair][IOCTL]     Freq corr = %d, Freq0 = %d, Freq1 = %d, NODE_ID = %d\n",*((unsigned int *)arg_ptr)&1,
	   (*((unsigned int *)arg_ptr)>>1)&3,
	   (*((unsigned int *)arg_ptr)>>3)&3,
	   (*((unsigned int *)arg_ptr)>>5)&0xFF);


    if ( (openair_daq_vars.node_configured == 1) && 
	 (openair_daq_vars.node_running == 0) && 
	 (openair_daq_vars.mac_registered == 1)) {



      mac_xface->is_cluster_head = 1;
      mac_xface->is_primary_cluster_head = 1;
      mac_xface->is_secondary_cluster_head = 0;
      mac_xface->cluster_head_index = 0;
      NODE_ID[0] = ((*((unsigned int *)arg_ptr))>>5)&0xFF;

      mac_xface->slots_per_frame = SLOTS_PER_FRAME;

      // Initialize MAC layer

      printk("[OPENAIR][IOCTL] MAC Init, is_cluster_head = %d (%p).slots_per_frame = %d (mac_xface %p)\n",mac_xface->is_cluster_head,&mac_xface->is_cluster_head,mac_xface->slots_per_frame,mac_xface);
       mac_xface->macphy_init();

#ifndef OPENAIR_LTE
      openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL;
#endif //OPENAIR_LTE
      openair_daq_vars.node_id = PRIMARY_CH;
      openair_daq_vars.freq = 0; //this is an initial value for the sensing
      openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<3);

    }
    else {
      printk("[openair][START_CLUSTERHEAD] Radio (%d) or Mac (%d) not configured\n",openair_daq_vars.node_configured,openair_daq_vars.mac_registered);
    }
  


#endif // RTAI_ENABLED
    break;


    //----------------------
  case openair_START_NODE:
    //----------------------

#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_START_NODE\n");
    printk("[openair][IOCTL]     Freq corr = %d, Freq0 = %d, Freq1 = %d, NODE_ID = %d\n",
	   *((unsigned int *)arg_ptr)&1,
	   (*((unsigned int *)arg_ptr)>>1)&3,
	   (*((unsigned int *)arg_ptr)>>3)&3,
	   (*((unsigned int *)arg_ptr)>>5)&0xFF);


    if ( (openair_daq_vars.node_configured == 1) && (openair_daq_vars.node_running == 0)) {
      mac_xface->is_cluster_head = 0;
      mac_xface->is_primary_cluster_head = 0;
      mac_xface->is_secondary_cluster_head = 0;
      mac_xface->cluster_head_index = 0;
      NODE_ID[0] = ((*((unsigned int *)arg_ptr))>>5)&0xFF;
      mac_xface->macphy_init(); ///////H.A

      openair_daq_vars.node_id = NODE;
      ret = setup_regs();
      if (ret == 0) {
	openair_daq_vars.node_running = 1;
	printk("[openair][START_NODE] Process initialization return code %d\n",ret);
      }
    }
    else {
      printk("[openair][START_CLUSTERHEAD] Radio not configured\n");
    }
#endif // RTAI_ENABLED
    break;

    //----------------------
  case openair_START_2ARY_CLUSTERHEAD:

#ifdef RTAI_ENABLED
    //----------------------
    printk("[openair][IOCTL]     openair_START_2ARY_CLUSTERHEAD\n");
    printk("[openair][IOCTL]     Freq corr = %d, Freq0 = %d, Freq1 = %d, NODE_ID = %d\n",*((unsigned int *)arg_ptr)&1,
	   (*((unsigned int *)arg_ptr)>>1)&3,
	   (*((unsigned int *)arg_ptr)>>3)&3,
	   (*((unsigned int *)arg_ptr)>>5)&0xFF);

    if ( (openair_daq_vars.node_configured == 1) && (openair_daq_vars.node_running == 0)) {
      mac_xface->is_cluster_head = 1;
      mac_xface->is_primary_cluster_head = 0;
      mac_xface->is_secondary_cluster_head = 1;
      mac_xface->cluster_head_index = 0;
   
      NODE_ID[0] = ((*((unsigned int *)arg_ptr))>>5)&0xFF;
      mac_xface->macphy_init(); ///////H.A

      openair_daq_vars.node_id = SECONDARY_CH;
      ret = setup_regs();
      if (ret == 0) {
	openair_daq_vars.node_running = 1;
	printk("[openair][START_2ARYCLUSTERHEAD] Process initialization return code %d\n",ret);
      }
    }

    else {
      printk("[openair][START_2ARY_CLUSTERHEAD] Radio not configured\n");
    }

#endif // RTAI_ENABLED
    break;
    //----------------------




    //----------------------
  case openair_STOP:
    //----------------------
    printk("[openair][IOCTL]     openair_STOP, NODE_CONFIGURED %d\n",openair_daq_vars.node_configured);

    
#ifdef RTAI_ENABLED
    if (openair_daq_vars.node_configured == 1) {
      openair_daq_vars.node_running = 0;
#ifndef NOCARD_TEST

      openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
      printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
      
      openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<3);
      openair_daq_vars.node_id = NODE;
      setup_regs();
      openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
      openair_daq_vars.tx_test=0;
      openair_daq_vars.mode = openair_NOT_SYNCHED;
      openair_daq_vars.sync_state = 0;
      mac_xface->frame = 0;
      mac_xface->is_cluster_head = 0;
      /*
	for (j=0;j<NB_ANTENNAS;j++) 
	for (i=0;i<FRAME_LENGTH_BYTES;i+=OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*4)
	copy_to_user(&((unsigned char *)arg)[i+(j*FRAME_LENGTH_BYTES)],&((unsigned char *)RX_DMA_BUFFER[j])[i],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*4);
      */
      
      
      udelay(1000);
      //#ifndef PC_TARGET

      //#endif // PC_TARGET
#endif // NOCARD_TEST
      
      
      //      openair_sched_cleanup();
      


     for (i=0;i<16;i++)
    printk("TX_DMA_BUFFER[0][%d] = %x\n",i,((unsigned int *)TX_DMA_BUFFER[0])[i]);
    }
    else {
      printk("[openair][STOP][ERROR] Cannot stop, radio is not configured ...\n");
      return -1;
    }
#endif // RTAI_ENABLED

#ifndef OPENAIR_LTE
    for (i=0;i<4;i++) {
      PHY_vars->PHY_measurements.chbch_detection_count[i]= 0;
    }
    PHY_vars->PHY_measurements.mrbch_detection_count= 0;
    PHY_vars->PHY_measurements.chbch_search_count= 0;
    PHY_vars->PHY_measurements.mrbch_search_count= 0;
#endif //OPENAIR_LTE    
    break;
      
  case openair_GET_BUFFER:

    printk("[openair][IOCTL]     openair_GET_BUFFER (%p)\n",(void *)RX_DMA_BUFFER[0]);
#ifdef RTAI_ENABLED
    if (openair_daq_vars.node_configured == 1) {
      printk("[openair][IOCTL]     Freq corr = %d, Freq0 = %d, Freq1 = %d\n",*((unsigned int *)arg_ptr)&1,
	  (*((unsigned int *)arg_ptr)>>1)&3,
	  (*((unsigned int *)arg_ptr)>>3)&3);

      openair_daq_vars.node_id = NODE;      
      ret = setup_regs();


      openair_daq_vars.one_shot_get_frame=1;

    }
    else {
      printk("[openair][GET_BUFFER][ERROR]  Radio not configured\n");
      return -1;
    }

#else


#endif // RTAI_ENABLED
    break;

    //----------------------

  case openair_GET_CONFIG:

#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_GET_CONFIG ...(%p)\n",(void *)arg);
    copy_to_user((char *)arg,PHY_config,sizeof(PHY_CONFIG));
#endif // RTAI_ENABLED

    break;

  case openair_GET_VARS:

#ifdef PC_TARGET
#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_GET_VARS ...(%p)\n",(void *)arg);
    copy_to_user((char *)arg,PHY_vars,sizeof(PHY_VARS));
#endif // RTAI_ENABLED
#endif // PC_TARGET
    break;

  case openair_SET_TX_GAIN:

    printk("[openair][IOCTL]     openair_SET_TX_GAIN ...(%p)\n",(void *)arg);
    openair_set_tx_gain_openair(((unsigned char *)arg)[0],((unsigned char *)arg)[1],((unsigned char *)arg)[2],((unsigned char *)arg)[3]
);

    break;

  case openair_SET_RX_GAIN:

    printk("[openair][IOCTL]     openair_SET_RX_GAIN ...(%p)\n",(void *)arg);

    openair_set_rx_gain_openair(((unsigned char *)arg)[0],((unsigned char *)arg)[1],((unsigned char *)arg)[2],((unsigned char *)arg)[3]);
    openair_daq_vars.rx_gain_mode = DAQ_AGC_OFF; // ((unsigned int *)arg)[0] & 0x1; 
    break;

  case openair_SET_CALIBRATED_RX_GAIN:

    printk("[openair][IOCTL]     openair_SET_CALIBRATED_RX_GAIN ...(%p)\n",(void *)arg);

    openair_set_rx_gain_cal_openair(((unsigned int *)arg)[0]);
    PHY_vars->rx_vars[0].rx_total_gain_dB = ((unsigned int *)arg)[0];
    openair_daq_vars.rx_gain_mode = DAQ_AGC_OFF; // ((unsigned int *)arg)[0] & 0x1; 
    break;

  case openair_START_FS4_TEST:

    printk("[openair][IOCTL]     openair_START_FS4_TEST ...(%p)\n",(void *)arg);
    openair_daq_vars.node_id = PRIMARY_CH;
    openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
    printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
    
    openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<3);

    openair_daq_vars.tx_rx_switch_point = NUMBER_OF_SYMBOLS_PER_FRAME;
    openair_daq_vars.tx_test=1;
    ret = setup_regs();

    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
    openair_generate_fs4(0);//*((unsigned char *)arg));

    for (i=0;i<256;i++)
      printk("TX_DMA_BUFFER[0][%d] = %x (%p)\n",i,((unsigned int *)TX_DMA_BUFFER[0])[i],&((unsigned int *)TX_DMA_BUFFER[0])[i] );


    
    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_START_RT_ACQUISITION);
    break;

  case openair_START_REAL_FS4_WITH_DC_TEST:

    printk("[openair][IOCTL]     openair_START_REAL_FS4_WITH_DC_TEST ...(%p)\n",(void *)arg);


    break;

  case openair_START_OFDM_TEST:
    printk("[openair][IOCTL]     openair_START_OFDM_TEST ...(%p)\n",(void *)arg);
    openair_daq_vars.node_id = PRIMARY_CH;
    openair_daq_vars.tx_test=1;
    ret = setup_regs();
    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);

    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_START_RT_ACQUISITION);
    break;

  case openair_START_QAM16_TEST:

    printk("[openair][IOCTL]     openair_START_QAM16_TEST ...(%p)\n",(void *)arg);

    break;

  case openair_START_QPSK_TEST:

    printk("[openair][IOCTL]     openair_START_QPSK_TEST ...(%p)\n",(void *)arg);


    break;

  case openair_START_IQ_IMPULSES_TEST:

    printk("[openair][IOCTL]     openair_START_IQ_IMPULSES_TEST ...(%p)\n",(void *)arg);

    break;

  case openair_RX_RF_MODE:
    printk("[openair][IOCTL]     openair_RX_RF_MODE ...(%p)\n",(void *)arg);

    openair_set_rx_rf_mode(((unsigned int *)arg)[0]);
    break;

  case openair_SET_TCXO_DAC:
    printk("[openair][IOCTL]     openair_set_tcxo_dac ...(%p)\n",(void *)arg);

    openair_set_tcxo_dac(((unsigned int *)arg)[0]);
    break;


  case openair_START_TX_SIG:
    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
    openair_daq_vars.tx_test=1;

    copy_from_user((unsigned char*)dummy_tx_vars,
		   (unsigned char*)arg,
		   NB_ANTENNAS_TX*sizeof(TX_VARS));
    
    copy_from_user((unsigned char*)TX_DMA_BUFFER[0],
		   (unsigned char*)dummy_tx_vars[0].TX_DMA_BUFFER,
		   FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
    copy_from_user((unsigned char*)TX_DMA_BUFFER[1],
		   (unsigned char*)dummy_tx_vars[1].TX_DMA_BUFFER,
		   FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));

    printk("TX_DMA_BUFFER[0] = %p, arg = %p, FRAMELENGTH_BYTES = %x\n",(void *)TX_DMA_BUFFER[0],(void *)arg,FRAME_LENGTH_BYTES);
    for (i=0;i<256;i++) {
      printk("TX_DMA_BUFFER[0][%d] = %x\n",i,((unsigned int *)TX_DMA_BUFFER[0])[i]);
      printk("TX_DMA_BUFFER[1][%d] = %x\n",i,((unsigned int *)TX_DMA_BUFFER[1])[i]);
    }

    openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
    printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
    
    openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<3);
    openair_daq_vars.node_id = PRIMARY_CH;
    openair_daq_vars.tx_rx_switch_point = NUMBER_OF_SYMBOLS_PER_FRAME-2;
    ret = setup_regs();

    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_START_RT_ACQUISITION);
		


    break;

  case openair_START_TX_SIG_NO_OFFSET:
    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
    openair_daq_vars.tx_test=1;    
    copy_from_user((unsigned char*)TX_DMA_BUFFER[0],
		   (unsigned char*)arg,
		   FRAME_LENGTH_BYTES);
    printk("TX_DMA_BUFFER[0] = %p, arg = %p, FRAMELENGTH_BYTES = %x, TX_RX_SWITCH = %d\n",(void *)TX_DMA_BUFFER[0],(void *)arg,FRAME_LENGTH_BYTES,(*((unsigned int *)arg_ptr)>>8)&0xff);
    for (i=0;i<16;i++)
      printk("TX_DMA_BUFFER[0][%d] = %x\n",i,((unsigned int *)TX_DMA_BUFFER[0])[i]);

    openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
    printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
    
    openair_daq_vars.freq_info = 0 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<3);
    openair_daq_vars.node_id = PRIMARY_CH;
    openair_daq_vars.tx_rx_switch_point = NUMBER_OF_SYMBOLS_PER_FRAME-2;
    ret = setup_regs();

    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_START_RT_ACQUISITION);
		


    break;


    /* Ioctls to control new (Idromel & E2R2) RF prototype chain:
         o openair_NEWRF_ADF4108_WRITE_REG
         o openair_NEWRF_ADF4108_INIT
         o openair_NEWRF_LFSW190410_WRITE_KHZ
         o openair_NEWRF_RF_SWITCH_CTRL
         o openair_NEWRF_SETTX_SWITCH_GAIN
         o and some POSTED eqauivalents.
       (K. Khalfallah, May 10th, 2007) */
  
  case openair_NEWRF_ADF4108_WRITE_REG:
    printk("[openair][IOCTL]     openair_NEWRF_ADF4108_WRITE_REG\n");
    /********************************
     * Writing registers of ADF4108 *
     ********************************/
    /* Get the values to write in the registers of ADF4108 frequency synthesizer
       (see [ADF4108] pp 11-12) */
    openair_NEWRF_RFctrl.ADF4108_Func0   = *((unsigned int*)arg);
    openair_NEWRF_RFctrl.ADF4108_Ref_Cnt = *(((unsigned int*)arg)+1);
    openair_NEWRF_RFctrl.ADF4108_N_Cnt   = *(((unsigned int*)arg)+2);
    /* Same claptrap as for previous ioctl: not testing consistency of the value in arg */
    /* Transmit the values in the CTRL0-2 registers. */
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL0_OFFSET, openair_NEWRF_RFctrl.ADF4108_Func0);
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL1_OFFSET, openair_NEWRF_RFctrl.ADF4108_Ref_Cnt);
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL2_OFFSET, openair_NEWRF_RFctrl.ADF4108_N_Cnt);
    wmb();
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_SET_ADF4108_REG | FROM_GRLIB_IRQ_FROM_PCI);
    wmb();
    /* We wait for acknowledge of the irq by the Cardbus MIMO board firmware
       (even if it may be dangerous for now, because we are in development phase,
       we are obliged to do so, unless we may perform several writings without the
       Cardbus MIMO board firmware having time to actually perform them... */

    /* So poll the IRQ bit */
    do {
      openair_readl(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, &tmp);
      rmb();
    } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
    break;

  case openair_NEWRF_ADF4108_INIT:
    printk("[openair][IOCTL]     openair_NEWRF_ADF4108_INIT\n");
    /************************************
     * Writing INIT register of ADF4108 *
     ************************************/
    /* Get the value to write in the Initialization register of ADF4108 frequency synthesizer
       (see [ADF4108] pp 11 & 15) */
    openair_NEWRF_RFctrl.ADF4108_Init = (unsigned int)arg;
    /* Same claptrap as for previous ioctl: not testing consistency of the value in arg */
    /* Transmit the value in the CTRL0 register */
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL3_OFFSET, openair_NEWRF_RFctrl.ADF4108_Init);
    wmb();
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_INIT_ADF4108 | FROM_GRLIB_IRQ_FROM_PCI);
    wmb();
    /* We wait for acknowledge of the irq by the Cardbus MIMO board firmware
       (even if it may be dangerous for now, because we are in development phase,
       we are obliged to do so, unless we may perform several writings without the
       Cardbus MIMO board firmware having time to actually perform them... */

    /* So poll the IRQ bit */
    do {
      openair_readl(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, &tmp);
      rmb();
    } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
    break;

  case openair_NEWRF_ADF4108_WRITE_REG_POSTED:
    printk("[openair][IOCTL]     openair_NEWRF_ADF4108_WRITE_REG_POSTED\n");
    if (!pci_interface) {
      printk("[openair][IOCTL]       Impossible to post ADF4108 config to card: pci_interface NOT yet allocated\n");
      return -1;
    }
    /********************************
     * Writing registers of ADF4108 *  (POSTED mode)
     ********************************/
    /* Get the values to write in the registers of ADF4108 frequency synthesizer
       (see [ADF4108] pp 11-12) */
    openair_NEWRF_RFctrl.ADF4108_Func0   = *((unsigned int*)arg);
    openair_NEWRF_RFctrl.ADF4108_Ref_Cnt = *(((unsigned int*)arg)+1);
    openair_NEWRF_RFctrl.ADF4108_N_Cnt   = *(((unsigned int*)arg)+2);
    /* Only write those values in the PCI_interface_t shared memory. */
    pci_interface->ADF4108_Func0   = openair_NEWRF_RFctrl.ADF4108_Func0;
    pci_interface->ADF4108_Ref_Cnt = openair_NEWRF_RFctrl.ADF4108_Ref_Cnt;
    pci_interface->ADF4108_N_Cnt   = openair_NEWRF_RFctrl.ADF4108_N_Cnt;
    pci_interface->nb_posted_rfctl_ADF4108 += 1;
    break;

  case openair_NEWRF_LFSW190410_WRITE_KHZ:
    printk("[openair][IOCTL]     openair_NEWRF_LFSW190410_WRITE_KHZ\n");
    /*****************************************
     * Writing KHZ Register of LFSW190410-50 *
     *****************************************/
    /* Get the value to write in KHZ register of LFSW190410-50 frequency synthesizer
       (see [LFSW190410] & [AN7100A] p4) */
    openair_NEWRF_RFctrl.LFSW190410_KHZ = (char*)arg;
    /* Same remark as for R counter reg of ADF4108 (see above): we don't verify correctness
       of the value passed in the arg parameter. */
    /* Transmit the ASCII value in the CTRL0 & CTRL1 registers */
    invert4(*((unsigned int*)openair_NEWRF_RFctrl.LFSW190410_KHZ));     /* because Sparc is big endian */
    invert4(*((unsigned int*)(openair_NEWRF_RFctrl.LFSW190410_KHZ+4))); /* because Sparc is big endian */
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL0_OFFSET, 'K');
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL1_OFFSET, *((unsigned int*)openair_NEWRF_RFctrl.LFSW190410_KHZ));
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL2_OFFSET, *((unsigned int*)(openair_NEWRF_RFctrl.LFSW190410_KHZ+4)));
    wmb();
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_SET_LFSW190410_KHZ | FROM_GRLIB_IRQ_FROM_PCI);
    wmb();
    /* Same remark as for R & N counter regs of ADF4108 (see above): we poll irq bit
       (possibly blocking local machine!...) */

    /* So poll the IRQ bit */
    do {
      openair_readl(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, &tmp);
      rmb();
    } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
    break;

  case openair_NEWRF_LFSW190410_WRITE_KHZ_POSTED:
    printk("[openair][IOCTL]     openair_NEWRF_LFSW190410_WRITE_KHZ_POSTED\n");
    if (!pci_interface) {
      printk("[openair][IOCTL]       Impossible to post LFSW109410 config to card: pci_interface NOT yet allocated\n");
      return -1;
    }
    /*****************************************
     * Writing KHZ Register of LFSW190410-50 * (POSTED mode)
     *****************************************/
    /* Get the value to write in KHZ register of LFSW190410-50 frequency synthesizer
       (see [LFSW190410] & [AN7100A] p4) */
    openair_NEWRF_RFctrl.LFSW190410_KHZ = (char*)arg;
    /* Same remark as for R counter reg of ADF4108 (see above): we don't verify correctness
       of the value passed in the arg parameter. */
    /* Transmit the ASCII value in the CTRL0 & CTRL1 registers */
    invert4(*((unsigned int*)openair_NEWRF_RFctrl.LFSW190410_KHZ));     /* because Sparc is big endian */
    invert4(*((unsigned int*)(openair_NEWRF_RFctrl.LFSW190410_KHZ+4))); /* because Sparc is big endian */
    /* Only write those values in the PCI_interface_t shared memory. */
    pci_interface->LFSW190410_CharCmd = 'K';
    pci_interface->LFSW190410_KHZ_0   = *((unsigned int*)openair_NEWRF_RFctrl.LFSW190410_KHZ);
    pci_interface->LFSW190410_KHZ_1   = *((unsigned int*)(openair_NEWRF_RFctrl.LFSW190410_KHZ+4));
    pci_interface->nb_posted_rfctl_LFSW += 1;
    break;

  case openair_NEWRF_RF_SWITCH_CTRL:
    printk("[openair][IOCTL]     openair_NEWRF_RF_SWITCH_CTRL\n");
    /***************************
     * Configuring RF switches *
     ***************************/
    /* Get the values to write in the registers of ADF4108 frequency synthesizer
       (see [ADF4108] pp 11-12) */
    openair_NEWRF_RFctrl.RFswitches_onoff = *((unsigned int*)arg);
    openair_NEWRF_RFctrl.RFswitches_mask = *(((unsigned int*)arg)+1);
    /* Same claptrap as for previous ioctl: not testing consistency of the value in arg */
    /* Transmit the value in the CTRL0 register */
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL0_OFFSET, openair_NEWRF_RFctrl.RFswitches_onoff);
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL1_OFFSET, openair_NEWRF_RFctrl.RFswitches_mask);
    wmb();
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_SET_RF_SWITCH | FROM_GRLIB_IRQ_FROM_PCI);
    wmb();
    /* We wait for acknowledge of the irq by the Cardbus MIMO board firmware
       (even if it may be dangerous for now, because we are in development phase,
       we are obliged to do so, unless we may perform several writings without the
       Cardbus MIMO board firmware having time to actually perform them... */

    /* So poll the IRQ bit */
    do {
      openair_readl(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, &tmp);
      rmb();
    } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
    break;

  case openair_NEWRF_RF_SWITCH_CTRL_POSTED:
    printk("[openair][IOCTL]     openair_NEWRF_RF_SWITCH_CTRL_POSTED\n");
    if (!pci_interface) {
      printk("[openair][IOCTL]     Impossible to post RF switches config to card: pci_interface NOT yet allocated\n");
      return -1;
    }
    /***************************
     * Configuring RF switches * (POSTED mode)
     ***************************/
    /* Get the values to write in the registers of ADF4108 frequency synthesizer
       (see [ADF4108] pp 11-12) */
    openair_NEWRF_RFctrl.RFswitches_onoff = *((unsigned int*)arg);
    openair_NEWRF_RFctrl.RFswitches_mask = *(((unsigned int*)arg)+1);
    /* Only write those values in the PCI_interface_t shared memory. */
    pci_interface->RFswitches_onoff   = openair_NEWRF_RFctrl.RFswitches_onoff;
    pci_interface->RFswitches_mask    = openair_NEWRF_RFctrl.RFswitches_mask;
    pci_interface->nb_posted_rfctl_RFSW += 1;
    break;

  case openair_NEWRF_SETTX_SWITCH_GAIN:
    printk("[openair][IOCTL]     openair_NEWRF_SETTX_SWITCH_GAIN\n");
    /***************************************************
     * Configuring both RF switches & gain of Tx chain *
     ***************************************************/
    /* Get the 32bit raw word containing info of both TX gains & TX switches */
    openair_NEWRF_RFctrl.settx_raw_word = *((unsigned int*)arg);
    /* Same claptrap as for previous ioctl: not testing consistency of the value in arg */
    /* Transmit the value in the CTRL0 register */
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL0_OFFSET, openair_NEWRF_RFctrl.settx_raw_word);
    wmb();
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_SETTX_GAIN_SWITCH | FROM_GRLIB_IRQ_FROM_PCI);
    wmb();
    /* We wait for acknowledge of the irq by the Cardbus MIMO board firmware
       (even if it may be dangerous for now, because we are in development phase,
       we are obliged to do so, unless we may perform several writings without the
       Cardbus MIMO board firmware having time to actually perform them... */

    /* So poll the IRQ bit */
    do {
      openair_readl(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, &tmp);
      rmb();
    } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
    break;

  case openair_NEWRF_SETTX_SWITCH_GAIN_POSTED:
    printk("[openair][IOCTL]     openair_NEWRF_SETTX_SWITCH_GAIN_POSTED\n");
    if (!pci_interface) {
      printk("[openair][IOCTL]     Impossible to post SETTX config to card: pci_interface NOT yet allocated\n");
      return -1;
    }
    /***************************************************
     * Configuring both RF switches & gain of Tx chain * (POSTED mode)
     ***************************************************/
    /* Get the 32bit raw word containing info of both TX gains & TX switches */
    openair_NEWRF_RFctrl.settx_raw_word = *((unsigned int*)arg);
    /* Only write those values in the PCI_interface_t shared memory. */
    pci_interface->settx_raw_word = openair_NEWRF_RFctrl.settx_raw_word;
    pci_interface->nb_posted_rfctl_SETTX += 1; // |= PENDING_POSTED_RFCTL_SETTX;
    break;

  case openair_NEWRF_SETRX_SWITCH_GAIN:
    printk("[openair][IOCTL]     openair_NEWRF_SETRX_SWITCH_GAIN\n");
    /***************************************************
     * Configuring both RF switches & gain of Rx chain *
     ***************************************************/
    /* Get the 32bit raw word containing info of both RX gains & RX switches */
    openair_NEWRF_RFctrl.setrx_raw_word = *((unsigned int*)arg);
    /* Same claptrap as for previous ioctl: not testing consistency of the value in arg */
    /* Transmit the value in the CTRL0 register */
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL0_OFFSET, openair_NEWRF_RFctrl.setrx_raw_word);
    wmb();
    openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_SETRX_GAIN_SWITCH | FROM_GRLIB_IRQ_FROM_PCI);
    wmb();
    /* We wait for acknowledge of the irq by the Cardbus MIMO board firmware
       (even if it may be dangerous for now, because we are in development phase,
       we are obliged to do so, unless we may perform several writings without the
       Cardbus MIMO board firmware having time to actually perform them... */

    /* So poll the IRQ bit */
    do {
      openair_readl(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, &tmp);
      rmb();
    } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
    break;

  case openair_NEWRF_SETRX_SWITCH_GAIN_POSTED:
    printk("[openair][IOCTL]     openair_NEWRF_SETRX_SWITCH_GAIN_POSTED\n");
    if (!pci_interface) {
      printk("[openair][IOCTL]     Impossible to post SETRX config to card: pci_interface NOT yet allocated\n");
      return -1;
    }
    /***************************************************
     * Configuring both RF switches & gain of Rx chain * (POSTED mode)
     ***************************************************/
    /* Get the 32bit raw word containing info of both RX gains & RX switches */
    openair_NEWRF_RFctrl.setrx_raw_word = *((unsigned int*)arg);
    /* Only write those values in the PCI_interface_t shared memory. */
    pci_interface->setrx_raw_word = openair_NEWRF_RFctrl.setrx_raw_word;
         // /* Test if flag is CLEAR */
         // ltmp = pci_interface->pending_posted_rfctl;
         // if (ltmp & PENDING_POSTED_RFCTL_SETRX)
         //   printk("[openair][IOCTL]       NO GOOD: RF ctl FLAG of SETRX is NOT RESET! (flags=0x%08x)\n", ltmp);
    /* ... and raise a flag so that firmware knows that we have posted some RF ctl for RX switches & gains*/
    pci_interface->nb_posted_rfctl_SETRX += 1; // |= PENDING_POSTED_RFCTL_SETRX;
    break;


  case openair_UPDATE_FIRMWARE:
    printk("[openair][IOCTL]     openair_UPDATE_FIRMWARE\n");
    /***************************************************
     *   Updating the firmware of Cardbus-MIMO-1 SoC   *
     ***************************************************/
    /* 1st argument of this ioctl indicates the action to perform among these:
         - Transfer a block of data at a specified address (given as the 2nd argument)
           and for a specified length (given as the 3rd argument, in number of 32-bit words).
           The USER-SPACE address where to find the block of data is given as the 4th
           argument.
         - Ask the Leon processor to clear the .bss section. In this case, the base
           address of section .bss is given as the 2nd argument, and its size is
           given as the 3rd one.
         - Ask the Leon processor to jump at a specified address (given as the 2nd
           argument, most oftenly expected to be the top address of Ins, Scratch Pad
           Ram), after having set the stack pointer (given as the 3rd argument).
       For the openair_UPDATE_FIRMWARE ioctl, we perform a partial infinite loop
       while acknowledging the PCI irq from Leon software: the max number of loop
       is yielded by preprocessor constant MAX_IOCTL_ACK_CNT. This avoids handing
       the kernel with an infinite polling loop. An exception is the case of clearing
       the bss: it takes time to Leon3 to perform this operation, so we poll te
       acknowledge with no limit */
#define MAX_IOCTL_ACK_CNT    500
    update_firmware_command = *((unsigned int*)arg);

    switch (update_firmware_command) {

      case UPDATE_FIRMWARE_TRANSFER_BLOCK:
        update_firmware_address   = ((unsigned int*)arg)[1];
        invert4(update_firmware_address); /* because Sparc is big endian */
        update_firmware_length    = ((unsigned int*)arg)[2];
        invert4(update_firmware_length); /* because Sparc is big endian */
        update_firmware_ubuffer   = (unsigned int*)((unsigned int*)arg)[3];
        /* Alloc some space from kernel to copy the user data block into */
        lendian_length = update_firmware_length;
        invert4(lendian_length); /* because Sparc is big endian */
        update_firmware_kbuffer = (unsigned int*)kmalloc(lendian_length * 4 /* 4 because kmalloc expects bytes */,
                                                         GFP_KERNEL);
        if (!update_firmware_kbuffer) {
          printk("[openair][IOCTL]  Could NOT allocate %u bytes from kernel memory (kmalloc failed).\n", lendian_length * 4);
          return -1; 
          break;
        }
        /* Copy the data block from user space */
        tmp = copy_from_user(
                             update_firmware_kbuffer, /* to   */
                             update_firmware_ubuffer, /* from */
                             lendian_length * 4       /* in bytes */
                            );
        if (tmp) {
          printk("[openair][IOCTL] Could NOT copy all data from user-space to kernel-space (%d bytes remained uncopied).\n", tmp);
          if (update_firmware_kbuffer)
            kfree(update_firmware_kbuffer);
          return -1;
          break;
        }
        for (fmw_off = 0 ; fmw_off < (lendian_length * 4) ; fmw_off += 4) {
          bendian_fmw_off = fmw_off; invert4(bendian_fmw_off);
          sparc_tmp_0 = update_firmware_address + bendian_fmw_off;
          invert4(sparc_tmp_0); /* because Sparc is big endian */
          sparc_tmp_1 = update_firmware_kbuffer[fmw_off/4];
          invert4(sparc_tmp_1); /* because Sparc is big endian */
          openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL0_OFFSET, sparc_tmp_0);
          openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL1_OFFSET, sparc_tmp_1);
          wmb();
          openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET,
                         FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_SINGLE_WRITE | FROM_GRLIB_IRQ_FROM_PCI);
          wmb();
          /* Poll the IRQ bit */
          ioctl_ack_cnt = 0;
          do {
            openair_readl(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, &tmp);
            rmb();
          } while ((tmp & FROM_GRLIB_IRQ_FROM_PCI) && (ioctl_ack_cnt++ < MAX_IOCTL_ACK_CNT));
          if (tmp & FROM_GRLIB_IRQ_FROM_PCI) {
            printk("[openair][IOCTL] ERROR: Leon did not acknowledge 'SINGLE_WRITE' irq (after a %u polling loop).\n", MAX_IOCTL_ACK_CNT);
            kfree(update_firmware_kbuffer);
            return -1;
            break;
          }
        }
        kfree(update_firmware_kbuffer);
        sparc_tmp_0 = update_firmware_address; sparc_tmp_1 = update_firmware_length;
        invert4(sparc_tmp_0); invert4(sparc_tmp_1);
        printk("[openair][IOCTL] ok %u words copied at address 0x%08x (Leon ack after %u polling loops)\n",
            sparc_tmp_1, sparc_tmp_0, ioctl_ack_cnt);
        break;

      case UPDATE_FIRMWARE_CLEAR_BSS:
        update_firmware_bss_address   = ((unsigned int*)arg)[1];
        update_firmware_bss_size      = ((unsigned int*)arg)[2];
        sparc_tmp_0 = update_firmware_bss_address;
        sparc_tmp_1 = update_firmware_bss_size;
        //printk("[openair][IOCTL]  BSS address passed to Leon3 = 0x%08x\n", sparc_tmp_0);
        //printk("[openair][IOCTL]  BSS  size   passed to Leon3 = 0x%08x\n", sparc_tmp_1);
        openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL0_OFFSET, sparc_tmp_0);
        openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL1_OFFSET, sparc_tmp_1);
        wmb();
        openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET,
                       FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_CLEAR_BSS | FROM_GRLIB_IRQ_FROM_PCI);
        wmb();
        /* Poll the IRQ bit */
        do {
          openair_readl(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, &tmp);
          rmb();
        } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
        printk("[openair][IOCTL] ok asked Leon to clear .bss (addr 0x%08x, size %d bytes)\n", sparc_tmp_0, sparc_tmp_1);
        break;

      case UPDATE_FIRMWARE_START_EXECUTION:
        update_firmware_start_address = ((unsigned int*)arg)[1];
        update_firmware_stack_pointer = ((unsigned int*)arg)[2];
        sparc_tmp_0 = update_firmware_start_address;
        sparc_tmp_1 = update_firmware_stack_pointer;
        //printk("[openair][IOCTL]  Entry point   passed to Leon3 = 0x%08x\n", sparc_tmp_0);
        //printk("[openair][IOCTL]  Stack pointer passed to Leon3 = 0x%08x\n", sparc_tmp_1);
        openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL0_OFFSET, sparc_tmp_0);
        openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL1_OFFSET, sparc_tmp_1);
        wmb();
        openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET,
                       FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_JUMP_USER_ENTRY | FROM_GRLIB_IRQ_FROM_PCI);
        wmb();
        /* Poll the IRQ bit */
        ioctl_ack_cnt = 0;
        do {
          openair_readl(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, &tmp);
          rmb();
        } while ((tmp & FROM_GRLIB_IRQ_FROM_PCI) && (ioctl_ack_cnt++ < MAX_IOCTL_ACK_CNT));
        if (tmp & FROM_GRLIB_IRQ_FROM_PCI) {
          printk("[openair][IOCTL] ERROR: Leon did not acknowledge 'START_EXECUTION' irq (after a %u polling loop).\n", MAX_IOCTL_ACK_CNT);
          return -1;
          break;
        }
        printk("[openair][IOCTL] ok asked Leon to run firmware (ep = 0x%08x, sp = 0x%08x, Leon ack after %u polling loops)\n",
            sparc_tmp_0, sparc_tmp_1, ioctl_ack_cnt);
        break;

      case UPDATE_FIRMWARE_FORCE_REBOOT:
        openair_writel(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET,
                       /*FROM_GRLIB_BOOT_HOK |*/ FROM_GRLIB_IRQ_FROM_PCI_IS_FORCE_REBOOT | FROM_GRLIB_IRQ_FROM_PCI);
        wmb();
        /* We don't wait for any acknowledge from Leon, because it can't acknowledge upon reboot */
        printk("[openair][IOCTL] ok asked Leon to reboot.\n");
        break;

      case UPDATE_FIRMWARE_TEST_GOK:

        /* No loop, just a single test (the polling loop should better be placed in user-space code). */
        openair_readl(pdev[0], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, &tmp);
        rmb();
        if (tmp & FROM_GRLIB_BOOT_GOK)
          return 0;
        else
          return -1;
        break;

      default:
        return -1;
        break;
    }    break;


  case openair_SET_TIMING_ADVANCE:
    openair_daq_vars.timing_advance = ((unsigned int *)arg)[0]; 

    msg("[openair][IOCTL] openair_daq_vars.timing_advance = %d\n",openair_daq_vars.timing_advance);

    ret = setup_regs();

    if (ret != 0)
      msg("[openair][IOCTL] Failed to set timing advance\n");
    
    break;

  case openair_SET_RX_MODE:

    //    rx_mode = ((unsigned int *)arg)[0]; 
    //    msg("[openair][IOCTL] Set RX_MODE to %d\n",rx_mode);

    break;


  default:
    //----------------------
    return -EPERM;
    break;
  }
  return 0;
}

