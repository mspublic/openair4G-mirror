#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#endif

#include "cbmimo1_device.h"
#include "defs.h"
#include "../../COMMON/defs.h"
#include "extern.h"

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/INIT/defs.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "RRC/LITE/defs.h"
#include "RRC/LITE/extern.h"

#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#endif

#include "from_grlib_softconfig.h"
#include "from_grlib_softregs.h"
#include "cbmimo1_pci.h"

void set_taus_seed(void);

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

  if (size>BIGPHYS_NUMPAGES*PAGE_SIZE) {
    printk("[openair][MMAP][ERROR] Trying to map more than %d bytes (%d)\n",
	   (unsigned int)(BIGPHYS_NUMPAGES*PAGE_SIZE),
	   (unsigned int)size);
    return -EINVAL;
  }
  /* start off at the PCI BAR0 */


  pos = (unsigned long) bigphys_ptr;
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
  //    printk("[openair][MMAP] rxsig %d = %x\n",i,RX_DMA_BUFFER[0][i]);


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
  int i,j,aa;
  int ue,eNb;
  
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

  TX_VARS dummy_tx_vars;
  LTE_DL_FRAME_PARMS *frame_parms = lte_frame_parms_g;
  unsigned short node_id;

  u8 buffer[100];
  u8 size;

  scale = &scale_mem;

  printk("[openair][IOCTL] In ioctl(), ioctl = %x (%x,%x)\n",cmd,openair_START_1ARY_CLUSTERHEAD,openair_START_NODE);
  
  switch(cmd) {
    

  case openair_TEST_FPGA:

      break;


    //----------------------
  case openair_DUMP_CONFIG:
    //----------------------
    printk("[openair][IOCTL]     openair_DUMP_CONFIG\n");
    printk("[openair][IOCTL] sizeof(mod_sym_t)=%d\n",sizeof(mod_sym_t));

    set_taus_seed();

#ifdef RTAI_ENABLED
    if (openair_daq_vars.node_configured > 0) {
      printk("[openair][IOCTL] NODE ALREADY CONFIGURED (%d), DYNAMIC RECONFIGURATION NOT SUPPORTED YET!!!!!!!\n",openair_daq_vars.node_configured);
    }
    else {
      copy_from_user((void*)frame_parms,arg_ptr,sizeof(LTE_DL_FRAME_PARMS));
      dump_frame_parms(frame_parms);
      printk("[openair][IOCTL] Allocating frame_parms\n");

#ifdef OPENAIR_LTE
      openair_daq_vars.node_configured = phy_init_top(frame_parms);
      msg("[openair][IOCTL] phy_init_top done: %d\n",openair_daq_vars.node_configured);
      
      frame_parms->twiddle_fft      = twiddle_fft;
      frame_parms->twiddle_ifft     = twiddle_ifft;
      frame_parms->rev              = rev;
      
      phy_init_lte_top(frame_parms);
      msg("[openair][IOCTL] phy_init_lte_top done: %d\n",openair_daq_vars.node_configured);
#else
      openair_daq_vars.node_configured = phy_init(NB_ANTENNAS_TX);
#endif
      if (openair_daq_vars.node_configured < 0) {
	printk("[openair][IOCTL] Error in configuring PHY\n");
	break;
      }
      
      else {
	printk("[openair][IOCTL] PHY Configuration successful\n");

#ifndef OPENAIR2	  
	openair_daq_vars.mac_registered = mac_init();
	if (openair_daq_vars.mac_registered != 1)
	  printk("[openair][IOCTL] Error in configuring MAC\n");
	else 
	  printk("[openair][IOCTL] MAC Configuration successful\n");
#endif	
      }
#ifndef NOCARD_TEST
      // Initialize FPGA PCI registers
      
      openair_daq_vars.dual_tx = frame_parms->dual_tx;
      openair_daq_vars.tdd     = frame_parms->frame_type;
      openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL;  //unused for FDD
      
#ifdef OPENAIR_LTE
      openair_daq_vars.freq = frame_parms->freq_idx;
      printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#else
      openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
      printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#endif
      openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);
      openair_daq_vars.rx_gain_val = 0;
      openair_daq_vars.tcxo_dac = 256;       // PUT the card in calibrated frequency mode by putting a value > 255 in tcxo register
      openair_daq_vars.node_id = NODE;
      openair_daq_vars.mode    = openair_NOT_SYNCHED;
      openair_daq_vars.node_running = 0;
      
      openair_daq_vars.auto_freq_correction = 1;
      openair_daq_vars.manual_timing_advance = 0;
      openair_daq_vars.timing_advance = 19;
      if (frame_parms->mode1_flag)
	openair_daq_vars.dlsch_transmission_mode = 1;
      else
	openair_daq_vars.dlsch_transmission_mode = 2;
      openair_daq_vars.target_ue_dl_mcs = 4;
      openair_daq_vars.target_ue_ul_mcs = 4;
      openair_daq_vars.dlsch_rate_adaptation = 0;
      openair_daq_vars.ue_ul_nb_rb = 4;
      openair_daq_vars.ulsch_allocation_mode = 0;

      //mac_xface->slots_per_frame = SLOTS_PER_FRAME;
      mac_xface->is_cluster_head = 0;
      mac_xface->is_primary_cluster_head = 0;
      mac_xface->is_secondary_cluster_head = 0;
      mac_xface->cluster_head_index = 0;


      printk("[openair][IOCTL] Setting up registers\n");
      for (i=0;i<number_of_cards;i++) { 
	ret = setup_regs(i,frame_parms);
	
	// Start LED dance with proper period
	openair_dma(i,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
      }
      
      // usleep(10);
      ret = openair_sched_init();
      if (ret != 0)
	printk("[openair][IOCTL] Error in starting scheduler\n");
      else
	printk("[openair][IOCTL] Scheduler started\n");

      // add Layer 1 stats in /proc/openair	
      add_openair1_stats();
      // rt_preempt_always(1);

#endif //NOCARD_TEST
    }
#endif // RTAI_ENABLED
    break;

    //----------------------
  case openair_START_1ARY_CLUSTERHEAD:
    //----------------------

#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_1ARY_START_CLUSTERHEAD\n");
    printk("[openair][IOCTL]     Freq corr = %d, Freq0 = %d, Freq1 = %d, NODE_ID = %d\n",*((unsigned int *)arg_ptr)&1,
	   (*((unsigned int *)arg_ptr)>>1)&7,
	   (*((unsigned int *)arg_ptr)>>4)&7,
	   (*((unsigned int *)arg_ptr)>>7)&0xFF);


    if ( (openair_daq_vars.node_configured > 0) && 
	 (openair_daq_vars.node_running == 0)) {

#ifdef OPENAIR_LTE
      if (openair_daq_vars.node_configured==1) {

	// allocate memory for PHY
	PHY_vars_eNB_g = (PHY_VARS_eNB**) malloc16(sizeof(PHY_VARS_eNB*));
	if (PHY_vars_eNB_g == NULL) {
	  printk("[openair][IOCTL] Cannot allocate PHY_vars_eNb\n");
	  break;
	}
	PHY_vars_eNB_g[0] = (PHY_VARS_eNB*) malloc16(sizeof(PHY_VARS_eNB));
	if (PHY_vars_eNB_g[0] == NULL) {
	  printk("[openair][IOCTL] Cannot allocate PHY_vars_eNb\n");
	  break;
	}
	bzero(PHY_vars_eNB_g[0],sizeof(PHY_VARS_eNB));

	//copy frame parms
	memcpy((void*) &PHY_vars_eNB_g[0]->lte_frame_parms, (void*) frame_parms, sizeof(LTE_DL_FRAME_PARMS));

	dump_frame_parms(&PHY_vars_eNB_g[0]->lte_frame_parms);
 
	if (  phy_init_lte_eNB(&PHY_vars_eNB_g[0]->lte_frame_parms,
			       &PHY_vars_eNB_g[0]->lte_eNB_common_vars,
			       PHY_vars_eNB_g[0]->lte_eNB_ulsch_vars,
			       0,
			       PHY_vars_eNB_g[0],
			       2, //this will allocate memory for cooperation
			       0)) {
	  printk("[openair][IOCTL] phy_init_lte_eNB error\n");
	  break;
	}
	else
	  printk("[openair][IOCTL] phy_init_lte_eNB successful\n");

	PHY_vars_eNB_g[0]->Mod_id = 0;

	// allocate DLSCH structures
	PHY_vars_eNB_g[0]->dlsch_eNB_SI  = new_eNB_dlsch(1,1,0);
	if (!PHY_vars_eNB_g[0]->dlsch_eNB_SI) {
	  msg("Can't get eNb dlsch SI structures\n");
	  break;
	}
	else {
	  msg("dlsch_eNB_SI => %p\n",PHY_vars_eNB_g[0]->dlsch_eNB_SI);
	  PHY_vars_eNB_g[0]->dlsch_eNB_SI->rnti  = SI_RNTI;
	}
	PHY_vars_eNB_g[0]->dlsch_eNB_ra  = new_eNB_dlsch(1,1,0);
	if (!PHY_vars_eNB_g[0]->dlsch_eNB_ra) {
	  msg("Can't get eNb dlsch RA structures\n");
	  break;
	}
	else {
	  msg("dlsch_eNB_ra => %p\n",PHY_vars_eNB_g[0]->dlsch_eNB_ra);
	  PHY_vars_eNB_g[0]->dlsch_eNB_ra->rnti  = RA_RNTI;
	}

	for (i=0; i<NUMBER_OF_UE_MAX;i++){ 
	  for (j=0;j<2;j++) {
	    PHY_vars_eNB_g[0]->dlsch_eNB[i][j] = new_eNB_dlsch(1,8,0);
	    if (!PHY_vars_eNB_g[0]->dlsch_eNB[i][j]) {
	      msg("Can't get eNb dlsch structures\n");
	      break;
	    }
	    else {
	      msg("dlsch_eNB[%d][%d] => %p\n",i,j,PHY_vars_eNB_g[0]->dlsch_eNB[i][j]);
	      PHY_vars_eNB_g[0]->dlsch_eNB[i][j]->rnti=0;
	    }
	  }
	}

	for (i=0; i<NUMBER_OF_UE_MAX+1;i++){ //+1 because 0 is reserved for RA
	  PHY_vars_eNB_g[0]->ulsch_eNB[i] = new_eNB_ulsch(3,0);
	  if (!PHY_vars_eNB_g[0]->ulsch_eNB[i]) {
	    msg("Can't get eNb ulsch structures\n");
	    break;
	  }
	  else {
	    msg("ulsch_eNB[%d] => %p\n",i,PHY_vars_eNB_g[0]->ulsch_eNB[i]);
	  }
	}

	//init_transport_channels(openair_daq_vars.dlsch_transmission_mode);

#endif

	openair_daq_vars.node_configured = 5;
	msg("[openair][IOCTL] phy_init_lte_eNB done: %d\n",openair_daq_vars.node_configured);
    

	for (aa=0;aa<NB_ANTENNAS_TX; aa++)
	  bzero((void*) TX_DMA_BUFFER[0][aa],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
	udelay(10000);
		
	// Initialize MAC layer

#ifdef OPENAIR2
	//NODE_ID[0] = ((*((unsigned int *)arg_ptr))>>7)&0xFF;
	NB_eNB_INST=1;
	NB_UE_INST=0;
	openair_daq_vars.mac_registered = 
	  l2_init(&PHY_vars_eNB_g[0]->lte_frame_parms);
	if (openair_daq_vars.mac_registered != 1) {
	  printk("[openair][IOCTL] Error in configuring MAC\n");
	  break;
	}
	else 
	  printk("[openair][IOCTL] MAC Configuration successful\n");
	
	//mac_xface->mrbch_phy_sync_failure(0,0);

	Mac_rlc_xface->Is_cluster_head[0] = 1;
#endif

	/*
	// configure SRS parameters statically
	for (ue=0;ue<NUMBER_OF_UE_MAX;ue++) {
	  PHY_vars_eNB_g[0]->eNB_UE_stats[ue].SRS_parameters.Csrs = 2;
	  PHY_vars_eNB_g[0]->eNB_UE_stats[ue].SRS_parameters.Bsrs = 0;
	  PHY_vars_eNB_g[0]->eNB_UE_stats[ue].SRS_parameters.kTC = 0;
	  PHY_vars_eNB_g[0]->eNB_UE_stats[ue].SRS_parameters.n_RRC = 0;
	  if (ue>=3) {
	    msg("This SRS config will only work for 3 users! \n");
	    break;
	  }
	  PHY_vars_eNB_g[0]->eNB_UE_stats[ue].SRS_parameters.Ssrs = ue+1;
	}
	*/
      } // eNB Configuration check

      for (i=0;i<NUMBER_OF_UE_MAX;i++) {
	clean_eNb_dlsch(PHY_vars_eNB_g[0]->dlsch_eNB[i][0],0);
	clean_eNb_dlsch(PHY_vars_eNB_g[0]->dlsch_eNB[i][1],0);
	clean_eNb_ulsch(PHY_vars_eNB_g[0]->ulsch_eNB[i],0);
	memset(&(PHY_vars_eNB_g[0]->eNB_UE_stats[i]),0,sizeof(LTE_eNB_UE_stats));
      }
      clean_eNb_dlsch(PHY_vars_eNB_g[0]->dlsch_eNB_SI,0);
      clean_eNb_dlsch(PHY_vars_eNB_g[0]->dlsch_eNB_ra,0);


      mac_xface->is_cluster_head = 1;
      mac_xface->is_primary_cluster_head = 1;
      mac_xface->is_secondary_cluster_head = 0;
      mac_xface->cluster_head_index = 0;

      openair_daq_vars.node_id = PRIMARY_CH;
      //openair_daq_vars.dual_tx = 1;
      
#ifdef OPENAIR_LTE
      openair_daq_vars.freq = ((*((unsigned int *)arg_ptr))>>1)&7;
      printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#else
      openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
      printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#endif
      
      openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);
      openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL;
      
      for (i=0;i<number_of_cards;i++) 
	ret = setup_regs(i,frame_parms);

      PHY_vars_eNB_g[0]->rx_total_gain_eNB_dB = 138;
      for (i=0;i<number_of_cards;i++)
	openair_set_rx_gain_cal_openair(i,PHY_vars_eNB_g[0]->rx_total_gain_eNB_dB);

      if (ret == 0) {
#ifdef OPENAIR_LTE
	openair_daq_vars.mode = openair_SYNCHED;
	for (ue=0;ue<NUMBER_OF_UE_MAX;ue++)
	  PHY_vars_eNB_g[0]->eNB_UE_stats[ue].mode = PRACH;// NOT_SYNCHED
#else
	openair_daq_vars.mode = openair_SYNCHED_TO_MRSCH;
#endif
	openair_daq_vars.node_running = 1;
	openair_daq_vars.sync_state = 0;
	printk("[openair][IOCTL] Process initialization return code %d\n",ret);
      }

    }
    else {
      printk("[openair][IOCTL] Radio (%d) or Mac (%d) not configured\n",openair_daq_vars.node_configured,openair_daq_vars.mac_registered);
    }


#endif // RTAI_ENABLED
    break;

    /*
    //----------------------
  case openair_START_1ARY_CLUSTERHEAD_COGNITIVE:
    //----------------------

#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_1ARY_START_CLUSTERHEAD_COGNITIVE\n");
    printk("[openair][IOCTL]     Freq corr = %d, Freq0 = %d, Freq1 = %d, NODE_ID = %d\n",*((unsigned int *)arg_ptr)&1,
	   (*((unsigned int *)arg_ptr)>>1)&7,
	   (*((unsigned int *)arg_ptr)>>4)&7,
	   (*((unsigned int *)arg_ptr)>>7)&0xFF);


    if ( (openair_daq_vars.node_configured == 1) && 
	 (openair_daq_vars.node_running == 0) && 
	 (openair_daq_vars.mac_registered == 1)) {



      mac_xface->is_cluster_head = 1;
      mac_xface->is_primary_cluster_head = 1;
      mac_xface->is_secondary_cluster_head = 0;
      mac_xface->cluster_head_index = 0;
      NODE_ID[0] = ((*((unsigned int *)arg_ptr))>>7)&0xFF;

      mac_xface->slots_per_frame = SLOTS_PER_FRAME;

      // Initialize MAC layer

      printk("[OPENAIR][IOCTL] MAC Init, is_cluster_head = %d (%p).slots_per_frame = %d (mac_xface %p)\n",mac_xface->is_cluster_head,&mac_xface->is_cluster_head,mac_xface->slots_per_frame,mac_xface);
      mac_xface->macphy_init();

      openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL;
      openair_daq_vars.node_id = PRIMARY_CH;
      openair_daq_vars.freq = 0; //this is an initial value for the sensing
      openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);

    }
    else {
      printk("[openair][START_CLUSTERHEAD] Radio (%d) or Mac (%d) not configured\n",openair_daq_vars.node_configured,openair_daq_vars.mac_registered);
    }
  


#endif // RTAI_ENABLED
    break;
    */

    //----------------------
  case openair_START_NODE:
    //----------------------

#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_START_NODE\n");
    printk("[openair][IOCTL]     Freq corr = %d, Freq0 = %d, Freq1 = %d, NODE_ID = %d\n",
	   *((unsigned int *)arg_ptr)&1,
	   (*((unsigned int *)arg_ptr)>>1)&7,
	   (*((unsigned int *)arg_ptr)>>4)&7,
	   (*((unsigned int *)arg_ptr)>>7)&0xFF);


    if ( (openair_daq_vars.node_configured > 0) && 
	 (openair_daq_vars.node_running == 0)) {

#ifdef OPENAIR_LTE
      if (openair_daq_vars.node_configured == 1) {

	// allocate memory for PHY
	PHY_vars_UE_g = (PHY_VARS_UE**) malloc16(sizeof(PHY_VARS_UE*));
	if (PHY_vars_UE_g == NULL) {
	  printk("[openair][IOCTL] Cannot allocate PHY_vars_UE\n");
	  break;
	}
	PHY_vars_UE_g[0] = (PHY_VARS_UE*) malloc16(sizeof(PHY_VARS_UE));
	if (PHY_vars_UE_g[0] == NULL) {
	  printk("[openair][IOCTL] Cannot allocate PHY_vars_UE\n");
	  break;
	}
	bzero(PHY_vars_UE_g[0],sizeof(PHY_VARS_UE));

	//copy frame parms
	memcpy((void*) &PHY_vars_UE_g[0]->lte_frame_parms, (void*) frame_parms, sizeof(LTE_DL_FRAME_PARMS));

	dump_frame_parms(&PHY_vars_UE_g[0]->lte_frame_parms);

	if (phy_init_lte_ue(&PHY_vars_UE_g[0]->lte_frame_parms, 
			    &PHY_vars_UE_g[0]->lte_ue_common_vars, 
			    PHY_vars_UE_g[0]->lte_ue_dlsch_vars, 
			    PHY_vars_UE_g[0]->lte_ue_dlsch_vars_SI, 
			    PHY_vars_UE_g[0]->lte_ue_dlsch_vars_ra,
			    PHY_vars_UE_g[0]->lte_ue_pbch_vars, 
			    PHY_vars_UE_g[0]->lte_ue_pdcch_vars,
			    PHY_vars_UE_g[0],
			    0)) {
	    msg("[openair][IOCTL] phy_init_lte_ue error\n");
	    break;
	}
	else
	  msg("[openair][IOCTL] phy_init_lte_ue successful\n");

	PHY_vars_UE_g[0]->Mod_id = 0;
  
	// allocate dlsch structures
	for (i=0; i<NUMBER_OF_eNB_MAX;i++){ 
	  for (j=0;j<2;j++) {
	    PHY_vars_UE_g[0]->dlsch_ue[i][j]  = new_ue_dlsch(1,8,0);
	    if (PHY_vars_UE_g[0]->dlsch_ue[i][j]) {
	      msg("[openair][IOCTL] UE dlsch structure eNb %d layer %d created\n",i,j);
	    }
	    else {
	      msg("[openair][IOCTL] Can't get ue dlsch structures\n");
	      break;
	    }
	  }
	  PHY_vars_UE_g[0]->ulsch_ue[i]  = new_ue_ulsch(3,0);
	  if (PHY_vars_UE_g[0]->ulsch_ue[i]) {
	    msg("[openair][IOCTL] ue ulsch structure %d created\n",i);
	  }
	  else {
	    msg("[openair][IOCTL] Can't get ue ulsch structures\n");
	    break;
	  }
	  
	  PHY_vars_UE_g[0]->dlsch_ue_SI[i]  = new_ue_dlsch(1,1,0);
	  if (PHY_vars_UE_g[0]->dlsch_ue_SI[i]) {
	    msg("[openair][IOCTL] ue dlsch (SI) structure %d created\n",i);
	  }
	  else {
	    msg("[openair][IOCTL] Can't get ue dlsch (SI) structures\n");
	    break;
	  }

	  PHY_vars_UE_g[0]->dlsch_ue_ra[i]  = new_ue_dlsch(1,1,0);
	  if (PHY_vars_UE_g[0]->dlsch_ue_SI[i]) {
	    msg("[openair][IOCTL] ue dlsch (RA) structure %d created\n",i);
	  }
	  else {
	    msg("[openair][IOCTL] Can't get ue dlsch (RA) structures\n");
	    break;
	  }
	}

	openair_daq_vars.node_configured = 3;
	msg("[openair][IOCTL] phy_init_lte_ue done: %d\n",openair_daq_vars.node_configured);

#endif 

#ifdef OPENAIR2	
	//NODE_ID[0] = ((*((unsigned int *)arg_ptr))>>7)&0xFF;
	NB_eNB_INST=0;
	NB_UE_INST=1;
	openair_daq_vars.mac_registered =
	  l2_init(&PHY_vars_UE_g[0]->lte_frame_parms); 
	if (openair_daq_vars.mac_registered != 1) {
	  printk("[openair][IOCTL] Error in configuring MAC\n");
	  break;
	}
	else 
	  printk("[openair][IOCTL] MAC Configuration successful\n");

	Mac_rlc_xface->Is_cluster_head[0] = 0;
#endif

	/*
	// configure SRS parameters (this will only work for one UE)
	PHY_vars_UE_g[0]->SRS_parameters.Csrs = 2;
	PHY_vars_UE_g[0]->SRS_parameters.Bsrs = 0;
	PHY_vars_UE_g[0]->SRS_parameters.kTC = 0;
	PHY_vars_UE_g[0]->SRS_parameters.n_RRC = 0;
	PHY_vars_UE_g[0]->SRS_parameters.Ssrs = 1;
	*/
      }  

	for (i=0;i<NUMBER_OF_eNB_MAX;i++) {
	  PHY_vars_UE_g[0]->lte_ue_pbch_vars[i]->pdu_errors_conseq=0;
	  PHY_vars_UE_g[0]->lte_ue_pbch_vars[i]->pdu_errors=0;
	  
	  PHY_vars_UE_g[0]->lte_ue_pdcch_vars[i]->dci_errors = 0;
	  PHY_vars_UE_g[0]->lte_ue_pdcch_vars[i]->dci_missed = 0;
	  PHY_vars_UE_g[0]->lte_ue_pdcch_vars[i]->dci_false  = 0;    
	  PHY_vars_UE_g[0]->lte_ue_pdcch_vars[i]->dci_received = 0;    

	  node_id = ((*((unsigned int *)arg_ptr))>>7)&0xFF;
	  PHY_vars_UE_g[0]->lte_ue_pdcch_vars[i]->crnti = (node_id>0 ? 0x1236 : 0x1235);
	  printk("[openair][IOCTL] Setting crnti to %x\n",PHY_vars_UE_g[0]->lte_ue_pdcch_vars[i]->crnti);
	  PHY_vars_UE_g[0]->UE_mode[i] = NOT_SYNCHED;

	  /*
	  clean_ue_dlsch(PHY_vars_UE_g[0]->lte_ue_dlsch[i][0],0);
	  clean_ue_dlsch(PHY_vars_UE_g[0]->lte_ue_dlsch[i][1],0);
	  clean_ue_dlsch(PHY_vars_UE_g[0]->lte_ue_dlsch_SI[i],0);
	  clean_ue_dlsch(PHY_vars_UE_g[0]->lte_ue_dlsch_ra[i],0);
	  clean_ue_ulsch(PHY_vars_UE_g[0]->lte_ue_ulsch[i],0);
	  */
	} 
	
      mac_xface->is_cluster_head = 0;
      mac_xface->is_primary_cluster_head = 0;
      mac_xface->is_secondary_cluster_head = 0;
      mac_xface->cluster_head_index = 0;

      openair_daq_vars.node_id = NODE;

#ifdef OPENAIR2
      RRC_CONNECTION_FLAG = 0;
#endif
      
#ifdef OPENAIR_LTE
      openair_daq_vars.freq = ((*((unsigned int *)arg_ptr))>>1)&7;
      printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#else
      openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
      printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#endif
      
      openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL;
      openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);
      
      PHY_vars_UE_g[0]->rx_total_gain_dB = MIN_RF_GAIN;
      openair_daq_vars.rx_total_gain_dB = MIN_RF_GAIN;
      openair_daq_vars.rx_gain_mode = DAQ_AGC_ON;
      
      msg("[openair][IOCTL] RX_DMA_BUFFER[0] = %p = %p RX_DMA_BUFFER[1] = %p = %p\n",
	  RX_DMA_BUFFER[0],
	  PHY_vars_UE_g[0]->lte_ue_common_vars.rxdata[0],
	  RX_DMA_BUFFER[1],
	  PHY_vars_UE_g[0]->lte_ue_common_vars.rxdata[1]);

#ifdef DLSCH_THREAD
      ret = init_dlsch_threads();
      if (ret != 0)
	printk("[openair][IOCTL] Error in starting DLSCH thread\n");
      else
	printk("[openair][IOCTL] DLSCH thread started\n");
#endif
      
      udelay(10000);
      
      ret = setup_regs(0,frame_parms);
      if (ret == 0) {
	openair_daq_vars.node_running = 1;
	printk("[openair][IOCTL] Process initialization return code %d\n",ret);
      }
    }
    else {
      printk("[openair][IOCTL] Radio not configured\n");
    }
#endif // RTAI_ENABLED
    break;

    /*
    //----------------------
  case openair_START_2ARY_CLUSTERHEAD:

#ifdef RTAI_ENABLED
    //----------------------
    printk("[openair][IOCTL]     openair_START_2ARY_CLUSTERHEAD\n");
    printk("[openair][IOCTL]     Freq corr = %d, Freq0 = %d, Freq1 = %d, NODE_ID = %d\n",*((unsigned int *)arg_ptr)&1,
	   (*((unsigned int *)arg_ptr)>>1)&7,
	   (*((unsigned int *)arg_ptr)>>4)&7,
	   (*((unsigned int *)arg_ptr)>>7)&0xFF);

    if ( (openair_daq_vars.node_configured == 1) && (openair_daq_vars.node_running == 0)) {
      mac_xface->is_cluster_head = 1;
      mac_xface->is_primary_cluster_head = 0;
      mac_xface->is_secondary_cluster_head = 1;
      mac_xface->cluster_head_index = 0;
   
      NODE_ID[0] = ((*((unsigned int *)arg_ptr))>>7)&0xFF;
      mac_xface->macphy_init(); ///////H.A

      openair_daq_vars.node_id = SECONDARY_CH;
#ifdef OPENAIR_LTE
      openair_daq_vars.freq = ((*((unsigned int *)arg_ptr))>>1)&7;
      printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#else
      openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
      printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#endif

      openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL;
      openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);

      ret = setup_regs(0);
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
    */


    //----------------------
  case openair_STOP:
    //----------------------
    printk("[openair][IOCTL]     openair_STOP, NODE_CONFIGURED %d\n",openair_daq_vars.node_configured);

    
#ifdef RTAI_ENABLED
    if (openair_daq_vars.node_configured > 0) {
      openair_daq_vars.node_running = 0;
#ifndef NOCARD_TEST

      for (aa=0;aa<NB_ANTENNAS_TX; aa++)
	bzero((void*) TX_DMA_BUFFER[0][aa],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
      udelay(1000);


      openair_daq_vars.node_id = NODE;
#ifdef OPENAIR_LTE
      openair_daq_vars.freq = ((*((unsigned int *)arg_ptr))>>1)&7;
      printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#else
      openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
      printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#endif

      openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL; 
      openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);

      for (i=0;i<number_of_cards;i++) {
	setup_regs(i,frame_parms);
	openair_dma(i,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
      }

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
 #endif // NOCARD_TEST
 
      // for (i=0;i<16;i++)
      //printk("TX_DMA_BUFFER[0][%d] = %x\n",i,((unsigned int *)TX_DMA_BUFFER[0])[i]);
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
#ifndef OPENAIR_LTE
    openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
    printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#else
    openair_daq_vars.freq = ((*((unsigned int *)arg_ptr))>>1)&7;
    printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#endif

    //openair_daq_vars.tx_rx_switch_point = NUMBER_OF_SYMBOLS_PER_FRAME; //this puts the node into RX mode only for TDD, its ignored in FDD mode
    openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);

#ifdef RTAI_ENABLED
    if (openair_daq_vars.node_configured > 0) {

      openair_daq_vars.node_id = NODE;      
      
      for (i=0;i<number_of_cards;i++)
	ret = setup_regs(i,frame_parms);

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
    copy_to_user((char *)arg,lte_frame_parms_g,sizeof(LTE_DL_FRAME_PARMS));
#endif // RTAI_ENABLED

    break;

  case openair_GET_BIGPHYSTOP:

#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_GET_BIGPHYSTOP ...(%p)\n",(void *)arg);
    printk("[openair][IOCTL]     bigphys_ptr = %x\n",bigphys_ptr);
    copy_to_user((char *)arg,&bigphys_ptr,sizeof(char *));
#endif // RTAI_ENABLED
    break;

  case openair_GET_VARS:

#ifdef PC_TARGET
#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_GET_VARS ...(%p)\n",(void *)arg);
    if (openair_daq_vars.node_configured == 3){    
      printk("[openair][IOCTL]  ... for UE (%d bytes) \n",sizeof(PHY_VARS_UE));
      copy_to_user((char *)arg,PHY_vars_UE_g[0],sizeof(PHY_VARS_UE));
    }
    else if (openair_daq_vars.node_configured == 5) {
      printk("[openair][IOCTL]  ... for eNB (%d bytes)\n",sizeof(PHY_VARS_eNB));
      copy_to_user((char *)arg,PHY_vars_eNB_g[0],sizeof(PHY_VARS_eNB));
    }
    else 
      printk("[openair][IOCTL] neither UE or eNb configured yet (%d)\n",openair_daq_vars.node_configured);
#endif // RTAI_ENABLED
#endif // PC_TARGET
    break;

  case openair_SET_TX_GAIN:

    printk("[openair][IOCTL]     openair_SET_TX_GAIN ...(%p)\n",(void *)arg);
    for (i=0;i<number_of_cards;i++)
      openair_set_tx_gain_openair(i,((unsigned char *)arg)[0],((unsigned char *)arg)[1],((unsigned char *)arg)[2],((unsigned char *)arg)[3]
);

    break;

  case openair_SET_RX_GAIN:

    printk("[openair][IOCTL]     openair_SET_RX_GAIN ...(%p)\n",(void *)arg);

    for (i=0;i<number_of_cards;i++)
      openair_set_rx_gain_openair(i,((unsigned char *)arg)[0],((unsigned char *)arg)[1],((unsigned char *)arg)[2],((unsigned char *)arg)[3]);
    openair_daq_vars.rx_gain_mode = DAQ_AGC_OFF; // ((unsigned int *)arg)[0] & 0x1; 
    break;

  case openair_SET_CALIBRATED_RX_GAIN:

    printk("[openair][IOCTL]     openair_SET_CALIBRATED_RX_GAIN ...(%p)\n",(void *)arg);

    for (i=0;i<number_of_cards;i++)
      openair_set_rx_gain_cal_openair(i,((unsigned int *)arg)[0]);

    //PHY_vars->rx_total_gain_dB = ((unsigned int *)arg)[0];
    //PHY_vars->rx_total_gain_eNB_dB = ((unsigned int *)arg)[0];
    openair_daq_vars.rx_gain_mode = DAQ_AGC_OFF; // ((unsigned int *)arg)[0] & 0x1; 
    break;

  case openair_START_FS4_TEST:
    
    printk("[openair][IOCTL]     openair_START_FS4_TEST ...(%p)\n",(void *)arg);
    openair_daq_vars.node_id = NODE;

#ifndef OPENAIR_LTE
    openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
    printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#else
    openair_daq_vars.freq = ((*((unsigned int *)arg_ptr))>>1)&7;
    printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#endif
    openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);

    openair_daq_vars.tx_rx_switch_point = NUMBER_OF_SYMBOLS_PER_FRAME-2;
    
    openair_daq_vars.tx_test=1;
    for (i=0;i<number_of_cards;i++) {
      ret = setup_regs(i,frame_parms);
      openair_dma(i,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
      udelay(1000);
      openair_dma(i,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_GEN_FS4);
    }

    break;

  case openair_START_REAL_FS4_WITH_DC_TEST:

    printk("[openair][IOCTL]     openair_START_REAL_FS4_WITH_DC_TEST ...(%p)\n",(void *)arg);


    break;

  case openair_START_OFDM_TEST:
    printk("[openair][IOCTL]     openair_START_OFDM_TEST ...(%p)\n",(void *)arg);

    openair_daq_vars.node_id = NODE;

#ifndef OPENAIR_LTE
    openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
    printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#else
    openair_daq_vars.freq = ((*((unsigned int *)arg_ptr))>>1)&7;
    printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#endif
    openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);

    openair_daq_vars.tx_rx_switch_point = NUMBER_OF_SYMBOLS_PER_FRAME-2;
    
    openair_daq_vars.tx_test=1;
    printk("[openair][IOCTL] OFDM: first rb %d, nb_rb %d\n",
	   ((*((unsigned *)arg_ptr))>>7)&0x1f,
	   ((*((unsigned *)arg_ptr))>>12)&0x1f);

    for (i=0;i<number_of_cards;i++) {
      ret = setup_regs(i,frame_parms);
      pci_interface[i]->first_rb = ((*((unsigned *)arg_ptr))>>7)&0x1f;
      pci_interface[i]->nb_rb = ((*((unsigned *)arg_ptr))>>12)&0x1f;
    //    start_rt_timer(0);  //in oneshot mode the argument (period) is ignored
      openair_dma(i,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
      udelay(1000);
    //    openair_dma(0,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_START_RT_ACQUISITION);
      openair_dma(i,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_GEN_OFDM);
      udelay(1000);
    }
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
    printk("[openair][IOCTL]     openair_RX_RF_MODE ...(%p), setting to %d\n",(void *)arg,((unsigned int *)arg)[0]);

    for (i=0;i<number_of_cards;i++)
      openair_set_rx_rf_mode(i,((unsigned int *)arg)[0]);
    break;

  case openair_SET_TCXO_DAC:
    printk("[openair][IOCTL]     openair_set_tcxo_dac ...(%p)\n",(void *)arg);

    for (i=0;i<number_of_cards;i++)
      openair_set_tcxo_dac(i,((unsigned int *)arg)[0]);
    break;


  case openair_START_TX_SIG:


    openair_daq_vars.node_id = NODE;

#ifndef OPENAIR_LTE
    openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
    printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#else
    printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#endif
    
    openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);
    openair_daq_vars.tx_rx_switch_point = NUMBER_OF_SYMBOLS_PER_FRAME-2;

    openair_daq_vars.tx_test=1;
    ret = setup_regs(0,frame_parms);

    openair_dma(0,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);

    bzero((void*)TX_DMA_BUFFER[0][0],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
    bzero((void*)TX_DMA_BUFFER[0][1],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
    copy_from_user((unsigned char*)&dummy_tx_vars,
		   (unsigned char*)arg,
		   sizeof(TX_VARS));
    
    copy_from_user((unsigned char*)TX_DMA_BUFFER[0][0],
		   (unsigned char*)dummy_tx_vars.TX_DMA_BUFFER[0],
		   FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
    copy_from_user((unsigned char*)TX_DMA_BUFFER[0][1],
		   (unsigned char*)dummy_tx_vars.TX_DMA_BUFFER[1],
		   FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));

    printk("TX_DMA_BUFFER[0] = %p, arg = %p, FRAMELENGTH_BYTES = %x\n",(void *)TX_DMA_BUFFER[0],(void *)arg,FRAME_LENGTH_BYTES);
    /*
    for (i=0;i<256;i++) {
      printk("TX_DMA_BUFFER[0][%d] = %x\n",i,((unsigned int *)TX_DMA_BUFFER[0])[i]);
      printk("TX_DMA_BUFFER[1][%d] = %x\n",i,((unsigned int *)TX_DMA_BUFFER[1])[i]);
    }
    */


    openair_dma(0,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_START_RT_ACQUISITION);
		


    break;

  case openair_START_TX_SIG_NO_OFFSET:
    openair_dma(0,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
    openair_daq_vars.tx_test=1;    
    copy_from_user((unsigned char*)TX_DMA_BUFFER[0][0],
		   (unsigned char*)arg,
		   FRAME_LENGTH_BYTES);
    printk("TX_DMA_BUFFER[0] = %p, arg = %p, FRAMELENGTH_BYTES = %x\n",(void *)TX_DMA_BUFFER[0],(void *)arg,FRAME_LENGTH_BYTES);

    //    for (i=0;i<16;i++)
    //      printk("TX_DMA_BUFFER[0][%d] = %x\n",i,((unsigned int *)TX_DMA_BUFFER[0])[i]);

    openair_daq_vars.node_id = PRIMARY_CH;
#ifndef OPENAIR_LTE
    openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
    printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#else
    printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#endif
    
    openair_daq_vars.freq_info = 0 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);
    openair_daq_vars.tx_rx_switch_point = NUMBER_OF_SYMBOLS_PER_FRAME-2;
    ret = setup_regs(0,frame_parms);

    openair_dma(0,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_START_RT_ACQUISITION);
		
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
    if (!pci_interface[0]) {
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
    pci_interface[0]->ADF4108_Func0   = openair_NEWRF_RFctrl.ADF4108_Func0;
    pci_interface[0]->ADF4108_Ref_Cnt = openair_NEWRF_RFctrl.ADF4108_Ref_Cnt;
    pci_interface[0]->ADF4108_N_Cnt   = openair_NEWRF_RFctrl.ADF4108_N_Cnt;
    pci_interface[0]->nb_posted_rfctl_ADF4108 += 1;
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
    if (!pci_interface[0]) {
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
    /* Only write those values in the pci_interface[0]_t shared memory. */
    pci_interface[0]->LFSW190410_CharCmd = 'K';
    pci_interface[0]->LFSW190410_KHZ_0   = *((unsigned int*)openair_NEWRF_RFctrl.LFSW190410_KHZ);
    pci_interface[0]->LFSW190410_KHZ_1   = *((unsigned int*)(openair_NEWRF_RFctrl.LFSW190410_KHZ+4));
    pci_interface[0]->nb_posted_rfctl_LFSW += 1;
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
    if (!pci_interface[0]) {
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
    pci_interface[0]->RFswitches_onoff   = openair_NEWRF_RFctrl.RFswitches_onoff;
    pci_interface[0]->RFswitches_mask    = openair_NEWRF_RFctrl.RFswitches_mask;
    pci_interface[0]->nb_posted_rfctl_RFSW += 1;
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
    if (!pci_interface[0]) {
      printk("[openair][IOCTL]     Impossible to post SETTX config to card: pci_interface NOT yet allocated\n");
      return -1;
    }
    /***************************************************
     * Configuring both RF switches & gain of Tx chain * (POSTED mode)
     ***************************************************/
    /* Get the 32bit raw word containing info of both TX gains & TX switches */
    openair_NEWRF_RFctrl.settx_raw_word = *((unsigned int*)arg);
    /* Only write those values in the PCI_interface_t shared memory. */
    pci_interface[0]->settx_raw_word = openair_NEWRF_RFctrl.settx_raw_word;
    pci_interface[0]->nb_posted_rfctl_SETTX += 1; // |= PENDING_POSTED_RFCTL_SETTX;
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
    if (!pci_interface[0]) {
      printk("[openair][IOCTL]     Impossible to post SETRX config to card: pci_interface NOT yet allocated\n");
      return -1;
    }
    /***************************************************
     * Configuring both RF switches & gain of Rx chain * (POSTED mode)
     ***************************************************/
    /* Get the 32bit raw word containing info of both RX gains & RX switches */
    openair_NEWRF_RFctrl.setrx_raw_word = *((unsigned int*)arg);
    /* Only write those values in the PCI_interface_t shared memory. */
    pci_interface[0]->setrx_raw_word = openair_NEWRF_RFctrl.setrx_raw_word;
         // /* Test if flag is CLEAR */
         // ltmp = pci_interface->pending_posted_rfctl;
         // if (ltmp & PENDING_POSTED_RFCTL_SETRX)
         //   printk("[openair][IOCTL]       NO GOOD: RF ctl FLAG of SETRX is NOT RESET! (flags=0x%08x)\n", ltmp);
    /* ... and raise a flag so that firmware knows that we have posted some RF ctl for RX switches & gains*/
    pci_interface[0]->nb_posted_rfctl_SETRX += 1; // |= PENDING_POSTED_RFCTL_SETRX;
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
    openair_daq_vars.manual_timing_advance = 1;
    openair_daq_vars.timing_advance = ((unsigned int *)arg)[0]; 

    msg("[openair][IOCTL] openair_daq_vars.timing_advance = %d\n",openair_daq_vars.timing_advance);

    for (i=0;i<number_of_cards;i++)
      ret = setup_regs(i,frame_parms);

    if (ret != 0)
      msg("[openair][IOCTL] Failed to set timing advance\n");
    
    break;

  case openair_SET_FREQ_OFFSET:

    openair_daq_vars.freq_offset = ((int *)arg)[0];
    openair_daq_vars.auto_freq_correction = 0;
    if (openair_set_freq_offset(0,((int *)arg)[0]) == 0)
      msg("[openair][IOCTL] Set frequency offset to %d\n",((int *)arg)[0]);
    else 
      msg("[openair][IOCTL] Problem setting frequency offset\n");

  case openair_SET_UE_DL_MCS:

    if ( ((((unsigned int *)arg)[0]) >= 0) && 
	 ((((unsigned int *)arg)[0]) <32) )
      openair_daq_vars.target_ue_dl_mcs = (unsigned char)(((unsigned int *)arg)[0]);
    break;

  case openair_SET_UE_UL_MCS:

    if ( ((((unsigned int *)arg)[0]) >= 0) && 
	 ((((unsigned int *)arg)[0]) <32) )
      openair_daq_vars.target_ue_ul_mcs = (unsigned char)(((unsigned int *)arg)[0]);
    break;

  case openair_SET_UE_UL_NB_RB:

    if ( ((((unsigned int *)arg)[0]) >= 0) && 
	 ((((unsigned int *)arg)[0]) <10) )
      openair_daq_vars.ue_ul_nb_rb = (unsigned char)(((unsigned int *)arg)[0]);
    break;

  case openair_SET_DLSCH_RATE_ADAPTATION:

    if ( ((((unsigned int *)arg)[0]) >= 0) && 
	 ((((unsigned int *)arg)[0]) <2) )
      openair_daq_vars.dlsch_rate_adaptation = (unsigned char)(((unsigned int *)arg)[0]);
    break;

  case openair_SET_DLSCH_TRANSMISSION_MODE:

    if ( ((((unsigned int *)arg)[0]) > 0) && 
	 ((((unsigned int *)arg)[0]) < 7) )
      openair_daq_vars.dlsch_transmission_mode = (unsigned char)(((unsigned int *)arg)[0]);
    if  ((PHY_vars_eNB_g != NULL) && (PHY_vars_eNB_g[0] != NULL))
      // if eNb is already configured, frame parms are local to it
      PHY_vars_eNB_g[0]->lte_frame_parms.mode1_flag = (openair_daq_vars.dlsch_transmission_mode==1);
    else
      // global frame parms have not been copied yet to eNB vars
      frame_parms->mode1_flag = (openair_daq_vars.dlsch_transmission_mode==1);
    break;

  case openair_SET_ULSCH_ALLOCATION_MODE:

    if ( ((((unsigned int *)arg)[0]) >= 0) && 
	 ((((unsigned int *)arg)[0]) <2) )
      openair_daq_vars.ulsch_allocation_mode = (unsigned char)(((unsigned int *)arg)[0]);
    break;

  case openair_SET_RRC_CONN_SETUP:
#ifdef OPENAIR2
    RRC_CONNECTION_FLAG = 1;
    printk("[IOCTL] Setting RRC_CONNECTION_FLAG\n");
#endif
  break;

  case openair_SET_COOPERATION_FLAG:
    if (PHY_vars_eNB_g && PHY_vars_eNB_g[0]) {
      PHY_vars_eNB_g[0]->cooperation_flag = ((unsigned int *)arg)[0];
      printk("[IOCTL] Setting cooperation flag to %d\n",PHY_vars_eNB_g[0]->cooperation_flag);
    }
    else
      printk("[IOCTL] Cooperation flag not set, PHY_vars_eNB_g not allocated!!!\n");
    break;


  default:
    //----------------------
    return -EPERM;
    break;
  }
  return 0;
}


