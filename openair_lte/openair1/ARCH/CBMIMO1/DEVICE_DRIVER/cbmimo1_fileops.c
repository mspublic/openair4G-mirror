#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#endif

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

#define UL_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,9,6)
#define CCCH_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,3)
#define BCCH_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,3)
#define RA_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,3)
#define DLSCH_RB_ALLOC 0x1fbf  // skip DC RB (total 23/25 RBs)
#define DLSCH_RB_ALLOC_12 0x0aaa  // skip DC RB (total 23/25 RBs)

//-----------------------------------------------------------------------------
int openair_device_ioctl(struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg) {
  /* arg is not meaningful if no arg is passed in user space */
  //-----------------------------------------------------------------------------
  int ret=-1;
  int i,aa,eNb_id;
  
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

  scale = &scale_mem;

  printk("[openair][IOCTL]:  : In ioctl(), ioctl = %x (%x,%x)\n",cmd,openair_START_1ARY_CLUSTERHEAD,openair_START_NODE);
  
  switch(cmd) {
    

  case openair_TEST_FPGA:

      break;


    //----------------------
  case openair_DUMP_CONFIG:
    //----------------------
    printk("[openair][IOCTL]     openair_DUMP_CONFIG\n");

    openair_daq_vars.mac_registered=1; // Fix this - should only be set if MAC is really registered
    
#ifdef RTAI_ENABLED
    if (openair_daq_vars.node_configured > 0) {
      printk("[openair][IOCTL] NODE ALREADY CONFIGURED (%d), DYNAMIC RECONFIGURATION NOT SUPPORTED YET!!!!!!!\n",openair_daq_vars.node_configured);
    }
    else {
      if (openair_daq_vars.mac_registered == 1) {
	copy_from_user((char *)PHY_config,(char *)arg,sizeof(PHY_CONFIG));
	dump_config();
	printk("[openair][IOCTL] Allocating PHY variables\n");

#ifdef OPENAIR_LTE
	lte_frame_parms = &PHY_config->lte_frame_parms;

	lte_ue_common_vars = &PHY_vars->lte_ue_common_vars;
	lte_ue_dlsch_vars  = &PHY_vars->lte_ue_dlsch_vars[0];
	lte_ue_pbch_vars   = &PHY_vars->lte_ue_pbch_vars[0];
	lte_ue_pdcch_vars  = &PHY_vars->lte_ue_pdcch_vars[0];
	lte_ue_dlsch_vars_cntl = &PHY_vars->lte_ue_dlsch_vars_cntl[0];
	lte_ue_dlsch_vars_ra = &PHY_vars->lte_ue_dlsch_vars_ra[0];
	lte_ue_dlsch_vars_1A = &PHY_vars->lte_ue_dlsch_vars_1A[0];

	lte_eNB_common_vars = &PHY_vars->lte_eNB_common_vars;
	lte_eNB_ulsch_vars  = &PHY_vars->lte_eNB_ulsch_vars[0];

	openair_daq_vars.node_configured = phy_init_top(NB_ANTENNAS_TX);
	msg("[openair][IOCTL] phy_init_top done: %d\n",openair_daq_vars.node_configured);

	lte_frame_parms->twiddle_fft      = twiddle_fft;
	lte_frame_parms->twiddle_ifft     = twiddle_ifft;
	lte_frame_parms->rev              = rev;

	lte_gold(lte_frame_parms);
	lte_sync_time_init(lte_frame_parms);

	generate_64qam_table();
	generate_16qam_table();
	generate_RIV_tables();

	set_taus_seed();

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
       
	openair_daq_vars.dual_tx = PHY_config->dual_tx;
	openair_daq_vars.tdd     = PHY_config->tdd;
	openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL;  //unused for FDD

#ifdef OPENAIR_LTE
	if (openair_daq_vars.tdd)
		openair_daq_vars.freq = 0; //TX: 1902600kHz RX: 1902600kHz
	else
		openair_daq_vars.freq = 4; //UE TX: 1917600kHz UE RX: 1902600kHz
	printk("[openair][IOCTL] Configuring for frequency TX 1917600kHz RX 1902600kHz (%d)\n",openair_daq_vars.freq);
#else
	openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
	printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#endif

	openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);
	openair_daq_vars.rx_gain_val = 0;

	// PUT the card in calibrated frequency mode by putting a value > 255 in tcxo register
	openair_daq_vars.tcxo_dac = 256;

	openair_daq_vars.node_id = NODE;
	openair_daq_vars.mode    = openair_NOT_SYNCHED;
	openair_daq_vars.node_running = 0;

	openair_daq_vars.timing_advance = 19;
	openair_daq_vars.dlsch_transmission_mode = openair_daq_vars.dlsch_transmission_mode; //ALAMOUTI
	openair_daq_vars.target_ue_dl_mcs = 0;
	openair_daq_vars.target_ue_ul_mcs = 0;
	openair_daq_vars.dlsch_rate_adaptation = 0;
	openair_daq_vars.ue_ul_nb_rb = 2;
	openair_daq_vars.ulsch_allocation_mode = 0;
	lte_frame_parms->mode1_flag = (openair_daq_vars.dlsch_transmission_mode==1);

	mac_xface->is_cluster_head = 0;

	for (i=0;i<number_of_cards;i++) { 
	  ret = setup_regs(i);

	// Start LED dance with proper period
	  openair_dma(i,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
	}

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
	   (*((unsigned int *)arg_ptr)>>1)&7,
	   (*((unsigned int *)arg_ptr)>>4)&7,
	   (*((unsigned int *)arg_ptr)>>7)&0xFF);


    if ( (openair_daq_vars.node_configured > 0) && 
	 (openair_daq_vars.node_running == 0) && 
	 (openair_daq_vars.mac_registered == 1)) {

#ifdef OPENAIR_LTE
      if ((openair_daq_vars.node_configured&4)==0) { 
	if (phy_init_lte_eNB(lte_frame_parms, lte_eNB_common_vars,lte_eNB_ulsch_vars)) {
	  printk("[openair][IOCTL] phy_init_lte_eNB error\n");
	  break;
	}

	dlsch_eNb = (LTE_eNb_DLSCH_t**) malloc16(2*sizeof(LTE_eNb_DLSCH_t*));
	ulsch_eNb = (LTE_eNb_ULSCH_t**) malloc16(2*sizeof(LTE_eNb_ULSCH_t*));

	for (i=0;i<2;i++) {
	  dlsch_eNb[i] = new_eNb_dlsch(1,8);
	  if (dlsch_eNb[i]) {
	    msg("[openair][IOCTL] eNb dlsch structure %d created \n",i);
	  }
	  else {
	    printk("[openair][IOCTL] Can't get eNb dlsch structures\n");
	    break;
	  }
	}

	ulsch_eNb[0] = new_eNb_ulsch(3);
	if (ulsch_eNb[0]) 
	  msg("[openair][IOCTL] eNb ulsch structures created \n");
	else {
	  msg("[openair][IOCTL] Can't get eNb ulsch structures\n");
	  break;
	}
	
	dlsch_eNb_cntl = new_eNb_dlsch(1,1);
	msg("[openair][IOCTL] eNb dlsch cntl structures created \n");

	dlsch_eNb_ra = new_eNb_dlsch(1,1);
	msg("[openair][IOCTL] eNb dlsch ra structures created \n");
	
	dlsch_eNb_1A = new_eNb_dlsch(1,1);
	msg("[openair][IOCTL] eNb dlsch 1A structures created \n");
	//#ifndef OPENAIR2


	// init DCI structures for testing
	openair_daq_vars.target_ue_ul_mcs    = 1;
	UL_alloc_pdu.type    = 0;
	UL_alloc_pdu.hopping = 0;
	UL_alloc_pdu.rballoc = UL_RB_ALLOC;
	UL_alloc_pdu.mcs     = openair_daq_vars.target_ue_ul_mcs;
	UL_alloc_pdu.ndi     = 1;
	UL_alloc_pdu.TPC     = 0;
	UL_alloc_pdu.cqi_req = 1;
	
	CCCH_alloc_pdu.type               = 1;
	CCCH_alloc_pdu.vrb_type           = 0;
	CCCH_alloc_pdu.rballoc            = CCCH_RB_ALLOC;
	CCCH_alloc_pdu.ndi      = 1;
	CCCH_alloc_pdu.mcs      = 1;
	CCCH_alloc_pdu.harq_pid = 0;

	BCCH_alloc_pdu.type               = 0;
	BCCH_alloc_pdu.vrb_type           = 0;
	BCCH_alloc_pdu.rballoc            = BCCH_RB_ALLOC;
	BCCH_alloc_pdu.ndi      = 1;
	BCCH_alloc_pdu.mcs      = 1;
	BCCH_alloc_pdu.harq_pid = 0;

	RA_alloc_pdu.type               = 0;
	RA_alloc_pdu.vrb_type           = 0;
	RA_alloc_pdu.rballoc            = RA_RB_ALLOC;
	RA_alloc_pdu.ndi      = 1;
	RA_alloc_pdu.mcs      = 1;
	RA_alloc_pdu.harq_pid = 0;
	
	openair_daq_vars.target_ue_dl_mcs    = 1;

	DLSCH_alloc_pdu2.rah              = 0;
	DLSCH_alloc_pdu2.rballoc          = DLSCH_RB_ALLOC;
	DLSCH_alloc_pdu2.TPC              = 0;
	DLSCH_alloc_pdu2.dai              = 0;
	DLSCH_alloc_pdu2.harq_pid         = 0;
	DLSCH_alloc_pdu2.tb_swap          = 0;
	DLSCH_alloc_pdu2.mcs1             = openair_daq_vars.target_ue_dl_mcs;
	DLSCH_alloc_pdu2.ndi1             = 1;
	DLSCH_alloc_pdu2.rv1              = 0;
	// Forget second codeword
	if (openair_daq_vars.dlsch_transmission_mode == 6)
	  DLSCH_alloc_pdu2.tpmi           = PUSCH_PRECODING0;
	else
	  DLSCH_alloc_pdu2.tpmi             = 0;

	DLSCH_alloc_pdu1A.type               = 1;
	DLSCH_alloc_pdu1A.vrb_type           = 0;
	DLSCH_alloc_pdu1A.rballoc            = CCCH_RB_ALLOC;
	DLSCH_alloc_pdu1A.ndi      = 1;
	DLSCH_alloc_pdu1A.rv       = 1;
	DLSCH_alloc_pdu1A.mcs      = 0;
	DLSCH_alloc_pdu1A.harq_pid = 0;
	DLSCH_alloc_pdu1A.TPC      = 1;   // set to 3 PRB

#endif

	openair_daq_vars.node_configured += 4;
	msg("[openair][IOCTL] phy_init_lte_eNB done: %d\n",openair_daq_vars.node_configured);
    

      //#endif

	for (aa=0;aa<NB_ANTENNAS_TX; aa++)
	  Zero_Buffer(TX_DMA_BUFFER[0][aa],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
	udelay(10000);
	
	mac_xface->is_cluster_head = 1;
	mac_xface->is_primary_cluster_head = 1;
	mac_xface->is_secondary_cluster_head = 0;
	mac_xface->cluster_head_index = 0;
	NODE_ID[0] = ((*((unsigned int *)arg_ptr))>>7)&0xFF;
	
	mac_xface->slots_per_frame = SLOTS_PER_FRAME;
	
	// Initialize MAC layer

#ifdef OPENAIR2
	l2_init();
	mac_xface->mrbch_phy_sync_failure(0,0);
#endif
      } // eNB Configuration check

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
	ret = setup_regs(i);

      PHY_vars->rx_total_gain_eNB_dB = 138;

      if (ret == 0) {
#ifdef OPENAIR_LTE
	openair_daq_vars.mode = openair_SYNCHED;
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


    if ( (openair_daq_vars.node_configured > 0) && (openair_daq_vars.node_running == 0)) {

#ifdef OPENAIR_LTE
      if ( (openair_daq_vars.node_configured&2) == 0) {
	if (phy_init_lte_ue(lte_frame_parms, 
			    lte_ue_common_vars, 
			    lte_ue_dlsch_vars, 
			    lte_ue_dlsch_vars_cntl, 
			    lte_ue_dlsch_vars_ra,
			    lte_ue_dlsch_vars_1A,
			    lte_ue_pbch_vars, 
			    lte_ue_pdcch_vars)) {
	    msg("[openair][IOCTL] phy_init_lte_ue error\n");
	    break;
	  }
	  
	  dlsch_ue = (LTE_UE_DLSCH_t**) malloc16(2*sizeof(LTE_UE_DLSCH_t*));
	  ulsch_ue = (LTE_UE_ULSCH_t**) malloc16(2*sizeof(LTE_UE_ULSCH_t*));

	  for (i=0;i<2;i++) {
	    dlsch_ue[i]  = new_ue_dlsch(1,8);
	    if (dlsch_ue) {
	      msg("[openair][IOCTL] UE dlsch structure %d created\n",i);
	    }
	    else {
	      msg("[openair][IOCTL] Can't get ue dlsch structures\n");
	      break;
	    }
	  }
	  ulsch_ue[0]  = new_ue_ulsch(3);
	  if (ulsch_ue[0]) {
	    msg("[openair][IOCTL] ue ulsch structure %d created\n",i);
	  }
	  else {
	    msg("[openair][IOCTL] Can't get ue ulsch structures\n");
	    break;
	  }
	  
	  dlsch_ue_cntl  = new_ue_dlsch(1,1);
	  dlsch_ue_ra  = new_ue_dlsch(1,1);
	  dlsch_ue_1A  = new_ue_dlsch(1,1);

	  openair_daq_vars.node_configured += 2;
	  msg("[openair][IOCTL] phy_init_lte_ue done: %d\n",openair_daq_vars.node_configured);

	  for (i=0;i<NUMBER_OF_eNB_MAX;i++) {
	    lte_ue_pbch_vars[i]->pdu_errors_conseq=0;
	    lte_ue_pbch_vars[i]->pdu_errors=0;
	    
	    lte_ue_pdcch_vars[i]->dci_errors = 0;
	    lte_ue_pdcch_vars[i]->dci_missed = 0;
	    lte_ue_pdcch_vars[i]->dci_false  = 0;    
	    lte_ue_pdcch_vars[i]->dci_received = 0;    
	  } 
	
#endif 

	  mac_xface->is_cluster_head = 0;
	  mac_xface->is_primary_cluster_head = 0;
	  mac_xface->is_secondary_cluster_head = 0;
	  mac_xface->cluster_head_index = 0;
	  NODE_ID[0] = ((*((unsigned int *)arg_ptr))>>7)&0xFF;
	  UE_mode = PRACH;

	  for (eNb_id=0;eNb_id<3;eNb_id++)
	    lte_ue_pdcch_vars[eNb_id]->crnti = 0x1234;

#ifdef OPENAIR2	
	  l2_init();
#endif
      }  
      openair_daq_vars.node_id = NODE;
      //openair_daq_vars.dual_tx = 0;
      
#ifdef OPENAIR_LTE
      openair_daq_vars.freq = ((*((unsigned int *)arg_ptr))>>1)&7;
      printk("[openair][IOCTL] Configuring for frequency %d\n",openair_daq_vars.freq);
#else
      openair_daq_vars.freq = ((int)(PHY_config->PHY_framing.fc_khz - 1902600)/5000)&3;
      printk("[openair][IOCTL] Configuring for frequency %d kHz (%d)\n",(unsigned int)PHY_config->PHY_framing.fc_khz,openair_daq_vars.freq);
#endif
      
      openair_daq_vars.tx_rx_switch_point = TX_RX_SWITCH_SYMBOL;
      openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);
      
      // turn on AGC
      openair_daq_vars.rx_gain_mode = DAQ_AGC_ON;
      
      msg("[openair][START_NODE] RX_DMA_BUFFER[0] = %p = %p RX_DMA_BUFFER[1] = %p = %p\n",
	  RX_DMA_BUFFER[0],
	  lte_ue_common_vars->rxdata[0],
	  RX_DMA_BUFFER[1],
	  lte_ue_common_vars->rxdata[1]);
      
      udelay(10000);
      
      ret = setup_regs(0);
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




    //----------------------
  case openair_STOP:
    //----------------------
    printk("[openair][IOCTL]     openair_STOP, NODE_CONFIGURED %d\n",openair_daq_vars.node_configured);

    
#ifdef RTAI_ENABLED
    if (openair_daq_vars.node_configured > 0) {
      openair_daq_vars.node_running = 0;
#ifndef NOCARD_TEST

      for (aa=0;aa<NB_ANTENNAS_TX; aa++)
	Zero_Buffer(TX_DMA_BUFFER[0][aa],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
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
	setup_regs(i);
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

    openair_daq_vars.tx_rx_switch_point = NUMBER_OF_SYMBOLS_PER_FRAME; //this puts the node into RX mode only for TDD, its ignored in FDD mode
    openair_daq_vars.freq_info = 1 + (openair_daq_vars.freq<<1) + (openair_daq_vars.freq<<4);

#ifdef RTAI_ENABLED
    if (openair_daq_vars.node_configured > 0) {

      openair_daq_vars.node_id = NODE;      
      
      for (i=0;i<number_of_cards;i++)
	ret = setup_regs(i);

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

  case openair_GET_BIGPHYSTOP:

#ifdef RTAI_ENABLED
    printk("[openair][IOCTL]     openair_GET_BIGPHYSTOP ...(%p)\n",(void *)arg);
    copy_to_user((char *)arg,&bigphys_ptr,sizeof(char *));
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

    openair_set_rx_gain_cal_openair(0,((unsigned int *)arg)[0]);
    PHY_vars->rx_total_gain_dB = ((unsigned int *)arg)[0];
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
      ret = setup_regs(i);
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
      ret = setup_regs(i);
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

    openair_set_tcxo_dac(0,((unsigned int *)arg)[0]);
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
    ret = setup_regs(0);

    openair_dma(0,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);

    Zero_Buffer((void*)TX_DMA_BUFFER[0][0],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
    Zero_Buffer((void*)TX_DMA_BUFFER[0][1],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
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
    ret = setup_regs(0);

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
    openair_daq_vars.timing_advance = ((unsigned int *)arg)[0]; 

    msg("[openair][IOCTL] openair_daq_vars.timing_advance = %d\n",openair_daq_vars.timing_advance);

    for (i=0;i<number_of_cards;i++)
      ret = setup_regs(i);

    if (ret != 0)
      msg("[openair][IOCTL] Failed to set timing advance\n");
    
    break;

  case openair_SET_FREQ_OFFSET:

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
	 ((((unsigned int *)arg)[0]) < 6) )
      openair_daq_vars.dlsch_transmission_mode = (unsigned char)(((unsigned int *)arg)[0]);
    lte_frame_parms->mode1_flag = (openair_daq_vars.dlsch_transmission_mode==1);
    break;

  case openair_SET_ULSCH_ALLOCATION_MODE:

    if ( ((((unsigned int *)arg)[0]) >= 0) && 
	 ((((unsigned int *)arg)[0]) <2) )
      openair_daq_vars.ulsch_allocation_mode = (unsigned char)(((unsigned int *)arg)[0]);
    break;

  default:
    //----------------------
    return -EPERM;
    break;
  }
  return 0;
}

