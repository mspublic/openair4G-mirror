#ifndef USER_MODE
#define __NO_VERSION__


#include <rtai.h>
#include <rtai_fifos.h>


#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/delay.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/mman.h>

#include <linux/slab.h>
//#include <linux/config.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include <linux/errno.h>

#ifdef KERNEL2_6
//#include <linux/config.h>
#include <linux/slab.h>
#endif

#ifdef KERNEL2_4
#include <linux/malloc.h>
#include <linux/wrapper.h>
#endif

#endif


#include <linux/bigphysarea.h>

#include "emul_device.h"
#include "defs.h"
#include "vars.h"


#include "linux/moduleparam.h"
#include "UTIL/BIGPHYS/defs.h"
#include "SIMULATION/simulation_defs.h"
#include "SIMULATION/PHY_EMULATION/SCHED/defs.h"
#include "SIMULATION/PHY_EMULATION/SCHED/vars.h"
#include "PHY_INTERFACE/vars.h"

#define BIGPHYS_NUMPAGES 1920

#ifdef BIGPHYSAREA
extern char *bigphys_current,*bigphys_ptr;
#endif

/*------------------------------------------------*/
/*   Prototypes                                   */
/*------------------------------------------------*/
#ifdef KERNEL2_4
static int   init_module( void );
static void  cleanup_module(void);
#else
static int   emul_init_module( void );
static void  emul_cleanup_module(void);/*------------------------------------------------*/
/*   Prototypes                                   */
/*------------------------------------------------*/
int emul_device_open    (struct inode *inode,struct file *filp);
int emul_device_release (struct inode *inode,struct file *filp);
int emul_device_mmap    (struct file *filp, struct vm_area_struct *vma);
int emul_device_ioctl   (struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg)
;

#endif


extern void dummy_macphy_scheduler(unsigned char last_slot);
extern void dummy_macphy_setparams(void *params);
extern void dummy_macphy_init(void );


/* The variable 'updatefirmware' defined below is used by the driver at insmod time
 * to decide if whether or not it must jump directly to user firmware settled in Scratch Pad
 * Rams of the Carbus-MIMO-1 SoC (updatefirmware == 1) OR if it must wait for update of the
 * firmware (updatefirmware == 0, the default). The update of the firmware is handled through
 * a specific ioctl code.
 * The value of updatefirmware can be changed at insmod time this very simple way:
 *   $ insmod openair_rf_softmodem.ko updatefirmware=1
 * (For more information on how to transmit parameter to modules at insmod time,
 * refer to [LinuxDeviceDrivers, 3rd edition, by Corbet/Rubini/Kroah-Hartman] pp 35-36). */

static void emul_cleanup(void);

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
  emul_device_ioctl,  //ioctl
  NULL,               //mmap
  emul_device_open,   //open
  NULL,               //flush
  emul_device_release,//release
  NULL,               //fsync
  NULL,               //fasync
  NULL,               //check_media_change
  NULL,               //revalidate
  NULL}};             //lock
//-----------------------------------------------------------------------------
#else
static struct file_operations openair_fops = {
ioctl:emul_device_ioctl,
open: emul_device_open,
release:emul_device_release,
};
#endif



#ifdef KERNEL2_6 
static int __init emul_init_module( void ) 
#else 
     int init_module( void ) 
#endif //KERNEL2_6
{
  //-----------------------------------------------------------------------------
  int res = 0;
  unsigned long i;
  
  
  char *adr;
  int32_t temp_size;
  

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
    printk("[EMUL][INIT_MODULE][ERROR]:  can't register char device driver, major : %d, error: %d\n", major, res);
    return -EIO;
  } else {
    printk("[EMUL][INIT_MODULE][INFO]:  char device driver registered major : %d\n", major);
  }

  

 


#ifdef BIGPHYSAREA
  printk("[openair][module] calling Bigphys_alloc_page...\n");
  bigphys_ptr = (char *)bigphysarea_alloc_pages(BIGPHYS_NUMPAGES,0,GFP_KERNEL);
  if (bigphys_ptr == (char *)NULL) {
    printk("[openair][MODULE][ERROR] Cannot Allocate Memory for shared data\n");
    emul_cleanup();
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
  }

#endif //BIGPHYSAREA




  PHY_config = kmalloc(sizeof(PHY_CONFIG),GFP_KERNEL);  
  memset(PHY_config,0,sizeof(PHY_CONFIG));


  if (PHY_config)
    printk("[openair][MODULE][INFO] Allocated %d bytes for PHY_config at %p\n",
	   sizeof(PHY_CONFIG),PHY_config);
  else {
    printk("[openair][MODULE][ERROR] Could not allocate memory for PHY_config\n");
    emul_cleanup();
    return -ENODEV;
  }

  Emul_vars = malloc16(sizeof(EMULATION_VARS));
  if (Emul_vars)
    printk("[openair][MODULE][INFO] Allocated %d bytes for Emul_vars at %p\n",
	   sizeof(EMULATION_VARS),Emul_vars);
  else {
    printk("[openair][MODULE][ERROR] Could not allocate memory for Emul_vars\n");
    emul_cleanup();
    return -ENODEV;
  }

  rt_set_oneshot_mode();
  start_rt_timer(0);  //in oneshot mode the argument (period) is ignored



  openair_emul_vars.mac_registered  = 0;
  openair_emul_vars.node_configured = -1;
  openair_emul_vars.node_running    = 0;

  mac_xface = malloc16(sizeof(MAC_xface));
  mac_xface->macphy_scheduler = dummy_macphy_scheduler;
  mac_xface->macphy_init      = dummy_macphy_init;

  printk("[OPENAIR][INIT_MODULE][INIT] mac_xface @ %p\n",mac_xface);

  printk("[openair][MODULE][INFO] Done init\n");

  return 0;
}

  
#ifdef KERNEL2_6 
static void __exit emul_cleanup_module(void)
#else 
  void cleanup_module(void)
#endif //KERNEL2_6
{
  printk("[openair][CLEANUP MODULE]\n");

  emul_cleanup();

 
  
}

static void  emul_cleanup(void) {


  int i;

  unregister_chrdev(major,"openair");


  printk("[openair][CLEANUP] Cleaning PHY Variables\n");


    if (PHY_config)
      kfree(PHY_config);


  if (bigphys_ptr != (char *)NULL) {
    printk("[openair][MODULE][INFO] Freeing BigPhys buffer\n");
    bigphysarea_free_pages((void *)bigphys_ptr);
  }


  stop_rt_timer ();             //stop the timer
  printk("[openair][MODULE][INFO] RTAI Timer stopped\n");
}





// Dump PHY Framing configuration

void dump_config(void) {

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



MODULE_AUTHOR
  ("Lionel GAUTHIER <lionel.gauthier@eurecom.fr>, Raymond KNOPP <raymond.knopp@eurecom.fr>, Aawatif MENOUNI <aawatif.menouni@eurecom.fr>,Dominique NUSSBAUM <dominique.nussbaum@eurecom.fr>, Michelle WETTERWALD <michelle.wetterwald@eurecom.fr>");
MODULE_DESCRIPTION ("openair Emulation driver");
MODULE_LICENSE ("GPL");
module_init (emul_init_module);
module_exit (emul_cleanup_module);
