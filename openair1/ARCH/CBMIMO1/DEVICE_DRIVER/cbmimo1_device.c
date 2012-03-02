#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#endif //USER_MODE

#include "cbmimo1_device.h"
#include "defs.h"
#include "vars.h"

#include "ARCH/COMMON/defs.h"

#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"

#include "SCHED/defs.h"
#include "SCHED/vars.h"

#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/vars.h"
#include "LAYER2/MAC/vars.h"
#ifdef OPENAIR2
#include "RRC/LITE/vars.h"
#include "UTIL/LOG/log.h"
#endif
//#ifndef PHY_EMUL
#include "from_grlib_softconfig.h"
#include "from_grlib_softregs.h"
//#endif //PHY_EMUL
#include "linux/moduleparam.h"

#include "SIMULATION/ETH_TRANSPORT/vars.h"


/*------------------------------------------------*/
/*   Prototypes                                   */
/*------------------------------------------------*/
#ifdef KERNEL2_4
static int   init_module( void );
static void  cleanup_module(void);
#else
static int   openair_init_module( void );
static void  openair_cleanup_module(void);

/*------------------------------------------------*/
/*   Prototypes                                   */
/*------------------------------------------------*/
int openair_device_open    (struct inode *inode,struct file *filp);
int openair_device_release (struct inode *inode,struct file *filp);
int openair_device_mmap    (struct file *filp, struct vm_area_struct *vma);
int openair_device_ioctl   (struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg)
;

#endif

/* The variable 'updatefirmware' defined below is used by the driver at insmod time
 * to decide if whether or not it must jump directly to user firmware settled in Scratch Pad
 * Rams of the Carbus-MIMO-1 SoC (updatefirmware == 1) OR if it must wait for update of the
 * firmware (updatefirmware == 0, the default). The update of the firmware is handled through
 * a specific ioctl code.
 * The value of updatefirmware can be changed at insmod time this very simple way:
 *   $ insmod openair_rf_softmodem.ko updatefirmware=1
 * (For more information on how to transmit parameter to modules at insmod time,
 * refer to [LinuxDeviceDrivers, 3rd edition, by Corbet/Rubini/Kroah-Hartman] pp 35-36). */
static int updatefirmware = 0;
module_param(updatefirmware, bool, S_IRUGO); /* permission mask S_IRUGO is for sysfs possible entry
                                                and means "readable to all" (from include/linux/stat.h) */

extern void dummy_macphy_scheduler(unsigned char last_slot);
extern void dummy_macphy_setparams(void *params);
extern void dummy_macphy_init(void );

static void openair_cleanup(void);

#ifdef BIGPHYSAREA
extern char *bigphys_current,*bigphys_ptr;
#endif

/*------------------------------------------------*/

#ifdef KERNEL2_4
static struct file_operations openair_fops[] = {{
  THIS_MODULE,
  NULL,               //llseek
  NULL,               //read
  NULL,               //write
  NULL,               //readdir
  NULL,               //poll
  openair_device_ioctl,       //ioctl
  openair_device_mmap,        //mmap
  openair_device_open,        //open
  NULL,               //flush
  openair_device_release,     //release
  NULL,               //fsync
  NULL,               //fasync
  NULL,               //check_media_change
  NULL,               //revalidate
  NULL}};             //lock
//-----------------------------------------------------------------------------
#else
static struct file_operations openair_fops = {
ioctl:openair_device_ioctl,
open: openair_device_open,
release:openair_device_release,
mmap: openair_device_mmap
};
#endif

int oai_trap_handler (int vec, int signo, struct pt_regs *regs, void *dummy) {

  RT_TASK *rt_task;
  
  rt_task = rt_smp_current[rtai_cpuid()];

  printk("[openair][TRAP_HANDLER] vec %d, signo %d, task %p, ip %04x (%04x), frame %d, slot %d\n", 
	 vec, signo, rt_task, regs->ip, regs->ip - (unsigned int) &bigphys_malloc, mac_xface->frame, openair_daq_vars.slot_count);

  openair_sched_exit("[openair][TRAP_HANDLER] Exiting!");

  rt_task_suspend(rt_task);
  
  return 1;

}


#ifdef KERNEL2_6 
static int __init openair_init_module( void ) 
#else 
  int init_module( void ) 
#endif //KERNEL2_6
{
  //-----------------------------------------------------------------------------
  int res = 0;
  // unsigned long i;
  
  
  char *adr;
  int32_t temp_size;
  

#ifndef PHY_EMUL
#ifndef NOCARD_TEST     
  //------------------------------------------------
  // Look for GRPCI
  //------------------------------------------------
  unsigned long i=0;  
  printk("[openair][INIT_MODULE][INFO]: Looking for GRLIB (%x,%x)\n",
	 FROM_GRLIB_CFG_PCIVID,
	 FROM_GRLIB_CFG_PCIDID);

  pdev[0] = pci_get_device(FROM_GRLIB_CFG_PCIVID, FROM_GRLIB_CFG_PCIDID, NULL);

  if(pdev[0]) {
    printk("[openair][INIT_MODULE][INFO]:  openair card %ld found, bus %x, primary %x, secondate %x\n",i,
	     pdev[i]->bus->number,pdev[i]->bus->primary,pdev[i]->bus->secondary);
    i=1;
  }
  else {
    printk("[openair][INIT_MODULE][INFO]:  no card found:\n");
    
    return -ENODEV;
  }

  // Now look for more cards on the same bus
  while (i<3) {
    pdev[i] = pci_get_device(FROM_GRLIB_CFG_PCIVID, FROM_GRLIB_CFG_PCIDID, pdev[i-1]);
    if(pdev[i]) {
      printk("[openair][INIT_MODULE][INFO]:  openair card %ld found, bus %x, primary %x, secondate %x\n",i,
	     pdev[i]->bus->number,pdev[i]->bus->primary,pdev[i]->bus->secondary);
      i++;
    }
    else {
      break;
    }
  }

  // at least one device found, enable it
  number_of_cards = i;

  for (i=0;i<number_of_cards;i++) {
    if(pci_enable_device(pdev[i])) {
      printk("[openair][INIT_MODULE][INFO]: Could not enable device %ld\n",i);

      return -ENODEV;
    }
    else {
	
      printk("[openair][INIT_MODULE][INFO]: Device %ld (%p)enabled\n",i,pdev[i]);
    }
      
      
    // Make the FPGA to a PCI master
    pci_set_master(pdev[i]);
      
      
    //pci_module_init(pdev);
      
    // Now map the openair Memory Spaces
      
    // get PCI memory mapped I/O space base address from BARs
  
    //    bar_index = 0;
    /*
    mm_start = pci_resource_start(pdev[i], bar_index);
    mm_end = pci_resource_end(pdev[i], bar_index);
    mm_len = pci_resource_len(pdev[i], bar_index);
    mm_flags = pci_resource_flags(pdev[i], bar_index);

    bar[i] = (unsigned int)ioremap_nocache(mm_start,mm_len);
    bar_len[i] = mm_len;
    printk("[openair][INIT_MODULE][INFO]: BAR%d card %d = %p (%p), length %x bytes\n",bar_index,i,bar[i],mm_start,bar_len[i]);
    */

  }


  for (i=0;i<number_of_cards;i++) {
    openair_readl(pdev[i], 
		  FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, 
		  &res);
    if ((res & FROM_GRLIB_BOOT_GOK) != 0)
      printk("[openair][INIT_MODULE][INFO]: LEON3 on card %ld is ok!\n",i);
    else {
      printk("[openair][INIT_MODULE][INFO]: Readback from LEON CMD %x\n",res);
      return -ENODEV;
    }
  }
  /* The boot strap of Leon is waiting for us, polling the HOK bit and 
   * waiting for us to assert it high.
   * If we also set the flag IRQ_FROM_PCI_IS_JUMP_USER_ENTRY in the PCI Irq field,
   * then it will automatically:
   *   1) set the stack pointer to the top of Data Scratch Pad Ram
   *   2) jump to Ins. Scratch Pad Ram.
   * So if the user performing the insmod of openair does not want to use
   * the default firmware, it must inform the driver by setting the boolean
   * variable 'updatefirmware' to 1/TRUE (by default, this variable is statically
   * equal to 0/FALSE.
   * In the latter case (that is, updatefirmware == 1) we only set the HOK bit,
   * without asking for an auto. jump to user firmware. This way, the user can
   * later call the driver with an ioctl to ask for firmware download & jump to it.
   * In the former case (that is, updatefirmware == 0), which is the default,
   * we ask for auto. jump to user firmware.
   * (for more information on how to transmit parameter to modules at insmod time,
   * refer to [LinuxDeviceDrivers, 3rd edition, by Corbet/Rubini/Kroah-Hartman] pp 35-36). */
  for (i=0;i<number_of_cards;i++) {
    if (!updatefirmware) {
      printk("[openair][INIT_MODULE][INFO]: Card %ld Setting HOK bit with auto jump to user firmware.\n",i);
      openair_writel(pdev[i], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_JUMP_USER_ENTRY);
    } else {
      printk("[openair][INIT_MODULE][INFO]: Setting HOK bit WITHOUT auto jump to user firmware.\n");
      openair_writel(pdev[i], FROM_GRLIB_CFG_GRPCI_EUR_CTRL_OFFSET, FROM_GRLIB_BOOT_HOK);
    }
  }
#endif //NOCARD_TEST
#endif //PHY_EMUL

  //------------------------------------------------
  // Register the device
  //------------------------------------------------
  
  major = openair_MAJOR;

  if((res = register_chrdev(major, "openair", 
#ifdef KERNEL2_4
			    openair_fops
#else
			    &openair_fops
#endif
			    )) < 0){
    printk("[openair][INIT_MODULE][ERROR]:  can't register char device driver, major : %d, error: %d\n", major, res);
    return -EIO;
  } else {
    printk("[openair][INIT_MODULE][INFO]:  char device driver registered major : %d\n", major);
  }

  

 


#ifdef BIGPHYSAREA
  printk("[openair][module] calling Bigphys_alloc_page for %d ...\n", BIGPHYS_NUMPAGES);
  bigphys_ptr = (char *)bigphysarea_alloc_pages(BIGPHYS_NUMPAGES,0,GFP_KERNEL);
  if (bigphys_ptr == (char *)NULL) {
    printk("[openair][MODULE][ERROR] Cannot Allocate Memory for shared data\n");
    openair_cleanup();
    return -ENODEV;
  }
  else {
    printk("[openair][MODULE][INFO] Bigphys at %p\n",(void *)bigphys_ptr);

    adr = (char *) bigphys_ptr;
    temp_size = BIGPHYS_NUMPAGES*PAGE_SIZE;
    while (temp_size > 0) {
      SetPageReserved(virt_to_page(adr));
      adr += PAGE_SIZE;
      temp_size -= PAGE_SIZE;
    }
      
    bigphys_current = bigphys_ptr;
    memset(bigphys_ptr,0,BIGPHYS_NUMPAGES*PAGE_SIZE);
  }

#endif //BIGPHYSAREA

#ifdef RTAI_ENABLED

  /*
#ifdef PC_TARGET
  // Allocate memory for PHY low-level data structures
  PHY_vars = kmalloc(sizeof(PHY_VARS),GFP_KERNEL);
  memset(PHY_vars,0,sizeof(PHY_VARS));
#endif // PC_TARGET

  PHY_config = kmalloc(sizeof(PHY_CONFIG),GFP_KERNEL);  
  memset(PHY_config,0,sizeof(PHY_CONFIG));

#ifdef PC_TARGET
  if (PHY_vars)
    printk("[openair][MODULE][INFO] Allocated %d bytes for PHY_vars at %p\n",
	sizeof(PHY_VARS),PHY_vars);
  else {
    printk("[openair][MODULE][ERROR] Could not allocate memory for PHY_vars\n");
    openair_cleanup();
    return -ENODEV;
  }
#endif //PC_TARGET

  if (PHY_config)
    printk("[openair][MODULE][INFO] Allocated %d bytes for PHY_config at %p\n",
	sizeof(PHY_CONFIG),PHY_config);
  else {
    printk("[openair][MODULE][ERROR] Could not allocate memory for PHY_config\n");
    openair_cleanup();
    return -ENODEV;
  }
  */

  rt_set_oneshot_mode();

  start_rt_timer(0);  //in oneshot mode the argument (period) is ignored
#endif //RTAI_ENABLED

  openair_daq_vars.mac_registered  = 0;
  openair_daq_vars.node_configured = 0;
  openair_daq_vars.node_running    = 0;

  printk("[OPENAIR][INIT_MODULE][INIT] openair_daq_vars set\n");
  printk("[OPENAIR][INIT_MODULE][INIT] bigphys_ptr =%p ,bigphys_current =%p\n",bigphys_ptr,bigphys_current);

  mac_xface = malloc16(sizeof(MAC_xface));
  if (mac_xface) {
    /*
    mac_xface->macphy_scheduler = dummy_macphy_scheduler;
    mac_xface->macphy_setparams = dummy_macphy_setparams;
    mac_xface->macphy_init      = dummy_macphy_init;
    */

    printk("[OPENAIR][INIT_MODULE][INIT] mac_xface @ %p\n",mac_xface);
  }
  else {
    printk("[OPENAIR][INIT_MODULE][INIT] mac_xface cannot be allocated\n");
    openair_cleanup();
    return -1;
  }

#ifdef OPENAIR_LTE
  lte_frame_parms_g = malloc16(sizeof(LTE_DL_FRAME_PARMS));

  if (lte_frame_parms_g) {
    printk("[OPENAIR][INIT_MODULE][INIT] lte_frame_parms allocated @ %p\n",lte_frame_parms_g);
  }
  else {
    printk("[OPENAIR][INIT_MODULE][INIT] lte_frame_parms cannot be allocated\n");
    openair_cleanup();
    return -1;
  }
#endif

  printk("[openair][MODULE][INFO] OPENAIR_CONFIG %x, OPENAIR_START_1ARY_CLUSTERHEAD %x,OPENAIR_START_NODE %x\n", openair_GET_CONFIG, openair_START_1ARY_CLUSTERHEAD, _IOR('o',3,long));

  //  for (i=0;i<10;i++)
  //printk("[openair][MODULE][INFO] IOCTL %d : %x\n",i,_IOR('o',i,long));

 		
  fifo_printf_init();

#ifdef OPENAIR2
  logInit(LOG_DEBUG);
#endif

  printk("[openair][MODULE][INFO] &rtai_global_heap = %p\n",&rtai_global_heap);


  // set default trap handler
  rt_set_trap_handler(oai_trap_handler);

  printk("[openair][MODULE][INFO] &bigphys_malloc = %p\n",&bigphys_malloc);

  printk("[openair][MODULE][INFO] Done init\n");
  return 0;
}

  
#ifdef KERNEL2_6 
static void __exit openair_cleanup_module(void)
#else 
  void cleanup_module(void)
#endif //KERNEL2_6
{
  printk("[openair][CLEANUP MODULE]\n");

  openair_cleanup();

  fifo_printf_clean_up();

#ifdef OPENAIR2
  //logClean();
#endif

  
}
static void  openair_cleanup(void) {

#ifndef PHY_EMUL
  int i;
#endif //PHY_EMUL

  unregister_chrdev(major,"openair");

#ifdef RTAI_ENABLED
  printk("[openair][CLEANUP] Cleaning PHY Variables\n");
#ifndef PHY_EMUL

  openair_sched_cleanup();

#ifdef DLSCH_THREAD
  cleanup_dlsch_threads();
#endif

  udelay(1000);

  phy_cleanup();

  remove_openair_stats();

#ifdef OPENAIR
  mac_top_cleanup();
#endif


#endif //PHY_EMUL
#endif //RTAI_ENABLED

#ifndef PHY_EMUL
  for (i=0;i<number_of_cards;i++) {
    if (bar[i])
      iounmap((void *)bar[i]);
  }
#endif //PHY_EMUL

  /*
#ifdef RTAI_ENABLED
#ifdef PC_TARGET
    if (PHY_vars)
      kfree(PHY_vars);
#endif    
    if (PHY_config)
      kfree(PHY_config);
#endif //RTAI_ENABLED
  */

#ifdef BIGPHYSAREA
  if (bigphys_ptr != (char *)NULL) {
    printk("[openair][MODULE][INFO] Freeing BigPhys buffer\n");
    bigphysarea_free_pages((void *)bigphys_ptr);
  }
#endif //BIGPHYSAREA

#ifdef RTAI_ENABLED
  stop_rt_timer ();             //stop the timer
  printk("[openair][MODULE][INFO] RTAI Timer stopped\n");
#endif //RTAI_ENABLED

}



/*
#ifdef RTAI_ENABLED
// Dump PHY Framing configuration

void dump_config() {

  printk("[openair][CONFIG][INFO] PHY_config = %p\n",PHY_config);
  printk("[openair][CONFIG][INFO] PHY_framing.fc_khz = %d\n",(unsigned int)PHY_config->PHY_framing.fc_khz);
  printk("[openair][CONFIG][INFO] PHY_framing.fs_khz = %d\n",(unsigned int)PHY_config->PHY_framing.fs_khz);
  printk("[openair][CONFIG][INFO] PHY_framing.Nsymb = %d\n",PHY_config->PHY_framing.Nsymb);
  printk("[openair][CONFIG][INFO] PHY_framing.Nd = %d\n",PHY_config->PHY_framing.Nd);
  printk("[openair][CONFIG][INFO] PHY_framing.log2Nd = %d\n",PHY_config->PHY_framing.log2Nd);
  printk("[openair][CONFIG][INFO] PHY_framing.Nc = %d\n",PHY_config->PHY_framing.Nc);
  printk("[openair][CONFIG][INFO] PHY_framing.Nz = %d\n",PHY_config->PHY_framing.Nz);
  printk("[openair][CONFIG][INFO] PHY_framing.Nf = %d\n",PHY_config->PHY_framing.Nf);

} 

#endif //RTAI_ENABLED
*/

MODULE_AUTHOR
  ("Lionel GAUTHIER <lionel.gauthier@eurecom.fr>, Raymond KNOPP <raymond.knopp@eurecom.fr>, Aawatif MENOUNI <aawatif.menouni@eurecom.fr>,Dominique NUSSBAUM <dominique.nussbaum@eurecom.fr>, Michelle WETTERWALD <michelle.wetterwald@eurecom.fr>, Florian KALTENBERGER <florian.kaltenberger@eurecom.fr>");
MODULE_DESCRIPTION ("openair CardBus driver");
MODULE_LICENSE ("GPL");
module_init (openair_init_module);
module_exit (openair_cleanup_module);
