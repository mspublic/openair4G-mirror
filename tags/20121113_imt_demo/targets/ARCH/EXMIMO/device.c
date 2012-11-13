#ifndef USER_MODE
#define __NO_VERSION__



#endif //USER_MODE

#include "device.h"
#include "defs.h"
#include "vars.h"


#include "linux/moduleparam.h"
#include <linux/interrupt.h>

/*------------------------------------------------*/
/*   Prototypes                                   */
/*------------------------------------------------*/
static int   openair_init_module( void );
static void  openair_cleanup_module(void);
extern irqreturn_t openair_irq_handler(int irq, void *cookie);

static void openair_cleanup(void);

#ifdef BIGPHYSAREA
extern char *bigphys_current,*bigphys_ptr;
#endif

extern int intr_in;
/*------------------------------------------------*/

static struct file_operations openair_fops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
unlocked_ioctl:openair_device_ioctl
#else
ioctl:openair_device_ioctl
#endif
,
open: openair_device_open,
release:openair_device_release,
mmap: openair_device_mmap
};

extern int pci_enable_pcie_error_reporting(struct pci_dev *dev);
extern int pci_cleanup_aer_uncorrect_error_status(struct pci_dev *dev);


static int __init openair_init_module( void ) {
  //-----------------------------------------------------------------------------
  int res = 0;
  // unsigned long i;
  
#ifdef BIGPHYSAREA  
  char *adr;
  int32_t temp_size;
#endif
  unsigned int readback;  


#ifndef NOCARD_TEST     
  //------------------------------------------------
  // Look for GRPCI
  //------------------------------------------------
  unsigned int i=0;  

   
  pdev[0] = pci_get_device(XILINX_VENDOR, XILINX_ID, NULL);
  if(pdev[0]) {
    printk("[openair][INIT_MODULE][INFO]:  openair card (ExpressMIMO) %d found, bus %x, primary %x, secondary %x\n",i,
	   pdev[i]->bus->number,pdev[i]->bus->primary,pdev[i]->bus->secondary);
    i=1;
    vid = XILINX_VENDOR;
    did = XILINX_ID;
      
  }
  else {
    printk("[openair][INIT_MODULE][INFO]:  no card found, stopping.\n");
    return -ENODEV;
  }


  // Now look for more cards on the same bus
  while (i<3) {
    pdev[i] = pci_get_device(vid,did, pdev[i-1]);
    if(pdev[i]) {
      printk("[openair][INIT_MODULE][INFO]:  openair card %d found, bus %x, primary %x, secondary %x\n",i,
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
      printk("[openair][INIT_MODULE][INFO]: Could not enable device %d\n",i);

      return -ENODEV;
    }
    else {
      //      pci_read_config_byte(pdev[i], PCI_INTERRUPT_PIN, &pdev[i]->pin);
      //      if (pdev[i]->pin)
      //	pci_read_config_byte(pdev[i], PCI_INTERRUPT_LINE, &pdev[i]->irq);

      printk("[openair][INIT_MODULE][INFO]: Device %d (%p)enabled, irq %d\n",i,pdev[i],pdev[i]->irq);
    }
      
      
    // Make the FPGA to a PCI master
    pci_set_master(pdev[i]);
      
      


  }


  if (pci_enable_pcie_error_reporting(pdev[0]) > 0)
    printk("[openair][INIT_MODULE][INFO]: Enabled PCIe error reporting\n");
  else
    printk("[openair][INIT_MODULE][INFO]: Failed to enable PCIe error reporting\n");

  pci_cleanup_aer_uncorrect_error_status(pdev[0]);

    
  mmio_start = pci_resource_start(pdev[0], 0);

  mmio_length = pci_resource_len(pdev[0], 0);
  mmio_flags = pci_resource_flags(pdev[0], 0);

  if (check_mem_region(mmio_start,256) < 0) {
    printk("[openair][INIT_MODULE][FATAL] : Cannot get memory region 0, aborting\n");
      return(-1);
  }
  request_mem_region(mmio_start,256,"openair_rf");
    

  bar[0] = pci_iomap(pdev[0],0,mmio_length);
    //    bar_len[i] = mm_len;
  printk("[openair][INIT_MODULE][INFO]: BAR0 card %d = %p\n",i,bar[0]);

  printk("[openair][INIT_MODULE][INFO]: Writing %x to BAR0+0x1c (PCIBASE)\n",0x12345678);

  iowrite32(0x12345678,(bar[0]+0x1c));
  udelay(100);
  readback = ioread32(bar[0]+0x1c);
  if (readback != 0x12345678) {
    printk("[openair][INIT_MODULE][INFO]: Readback of FPGA register failed (%x)\n",readback);
    release_mem_region(mmio_start,256);
    return(-1);
  }
  iowrite32((1<<8) | (1<<9) | (1<<10),bar[0]);
  udelay(1000);
  readback = ioread32(bar[0]);
  printk("CONTROL0 readback %x\n",readback);


#endif //NOCARD_TEST

  //------------------------------------------------
  // Register the device
  //------------------------------------------------
  
  major = openair_MAJOR;

  if((res = register_chrdev(major, "openair", 
			    &openair_fops
			    )) < 0){
    printk("[openair][INIT_MODULE][ERROR]:  can't register char device driver, major : %d, error: %d\n", major, res);
    release_mem_region(mmio_start,256);
    return -EIO;
  } else {
    printk("[openair][INIT_MODULE][INFO]:  char device driver registered major : %d\n", major);
  }

  

 


#ifdef BIGPHYSAREA
  printk("[openair][module] calling Bigphys_alloc_page for %d ...\n", BIGPHYS_NUMPAGES);
  bigphys_ptr = (char *)bigphysarea_alloc_pages(BIGPHYS_NUMPAGES,0,GFP_KERNEL);
  //bigphys_ptr = (char *)alloc_pages_exact(BIGPHYS_NUMPAGES*4096,GFP_KERNEL);
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
  printk("[OPENAIR][INIT_MODULE][INIT] bigphys_ptr =%p ,bigphys_current =%p\n",bigphys_ptr,bigphys_current);
#endif //BIGPHYSAREA

  if (vid == XILINX_VENDOR)  // This is ExpressMIMO
    exmimo_firmware_init();


#ifdef RTAI_ENABLED
  rt_set_oneshot_mode();
  start_rt_timer(0);  //in oneshot mode the argument (period) is ignored


  openair_daq_vars.mac_registered  = 0;
  openair_daq_vars.node_configured = 0;
  openair_daq_vars.node_running    = 0;
  printk("[OPENAIR][INIT_MODULE][INIT] openair_daq_vars set\n");
#endif //RTAI_ENABLED

  printk("[OPENAIR][SCHED][INIT] Trying to get IRQ %d\n",pdev[0]->irq);

  openair_irq_enabled = 0;
  
  if (request_irq(pdev[0]->irq, 
		  openair_irq_handler, 
		  IRQF_SHARED , "openair_rf", pdev[0]) == 0) {
    openair_irq_enabled=1;
  } else {
    printk("[EXMIMO][SCHED][INIT] Cannot get IRQ %d for HW\n",pdev[0]->irq);
    release_mem_region((resource_size_t)mmio_start,256);
    openair_cleanup();
    return(-1);
  }
  
 
  printk("[openair][MODULE][INFO] Done init\n");
  return 0;
}

  
static void __exit openair_cleanup_module(void) {
  printk("[openair][CLEANUP MODULE]\n");


  release_mem_region(mmio_start,256);


  openair_cleanup();


  
}
static void  openair_cleanup(void) {


  int i;


  unregister_chrdev(major,"openair");
  exmimo_firmware_cleanup();

  for (i=0;i<number_of_cards;i++) {
    if (bar[i])
      iounmap((void *)bar[i]);
  }

  // unregister interrupt
  if (openair_irq_enabled == 1) {
    printk("[openair][CLEANUP] disabling interrupt\n");
    free_irq(pdev[0]->irq,pdev[0]);
  }
  openair_irq_enabled=0;


#ifdef BIGPHYSAREA
  if (bigphys_ptr != (char *)NULL) {
    printk("[openair][MODULE][INFO] Freeing BigPhys buffer\n");
    bigphysarea_free_pages((void *)bigphys_ptr);
    //free_pages_exact((void *)bigphys_ptr,BIGPHYS_NUMPAGES*4096);
  }
#endif //BIGPHYSAREA


}



MODULE_AUTHOR
  ("Raymond KNOPP <raymond.knopp@eurecom.fr>, Florian KALTENBERGER <florian.kaltenberger@eurecom.fr>");
MODULE_DESCRIPTION ("openair ExpressMIMO/ExpressMIMO2 driver");
MODULE_LICENSE ("GPL");
module_init (openair_init_module);
module_exit (openair_cleanup_module);
