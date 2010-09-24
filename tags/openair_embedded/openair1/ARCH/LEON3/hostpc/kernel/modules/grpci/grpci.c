
/*
 * References      [LnxDrv3]       Linux Device Drivers, 3rd Edition,
 *                                 by Jonathan Corbet, Alessandro Rubini, and Greg Kroah-Hartman.
 *                                 Copyright 2005 O'Reilly Media, Inc., ISBN 0-596-00590-3
 *                 [Grip]          GRLIB IP Core User's Manual (<grip.pdf>)
 *                                 Version 1.0.7, February 2006.
 *                                 See in particular chapters 37 (GRPCI bridge) & 39 (PCIDMA engine).
 *                                 (latest version of <grip.pdf> available at www.gaisler.com)
 *                 [ADF4108]       Component data sheet, Analog Devices (Rev. 0)
 *                                 PLL Frequency Synthesizer ADF4108
 *                 [LFSW190410]    Component datasheet, Synergy Microwave
 *                                 Interactive Synthesizer, LFSW190410-50
 *                 [AN7100A]       Application Note, Synergy Microwave
 *                                 Pinout & Programming Functions for Interactive Synthesizer
 *                 [GnuCPP]        The C Preprocessor (R. M. Stallman, Z. Weinberg)
 *                                 Last revised April 2001 for GCC version 3 */

#include <linux/init.h>     /* for use of module_init() and module_exit() */
#include <linux/module.h>
#include <linux/kdev_t.h>   /* for use of macros MAJOR(dev_t) & MINOR(dev_t) */
#include <linux/fs.h>       /* for use of alloc_chrdev_region() & unregister_chrdev_region() */
#include <linux/cdev.h>     /* for use of cdev, char driver structure */
#include <linux/pci.h>
#include <asm/pci.h>        /* for use of ioremap_nocache() */
#include <asm/system.h>     /* for use of wmb() & other memory barriers */
#include <asm/delay.h>      /* for use of udelay() */
#include <linux/timer.h>    /* for use of timers facilities */
#include <linux/jiffies.h>  /* for use of jiffies variable */
#include <linux/ioctl.h>    /* for use of all ioctl related help functions & macros */

#include "grpci.h"

#define uclong unsigned int
#define grpci_writel(val,port)     {writel((uclong)(val),(ulong)(port)); mb();}
#define grpci_readl(port)          readl(port)

/* To avoid warning messages related to the type of printk's arguments */
#define UINT(x)  ((unsigned int)x)

static struct pci_device_id gaisler_ids[] = {
  { PCI_DEVICE(FROM_GRLIB_CFG_PCIVID, FROM_GRLIB_CFG_PCIDID) },
};

MODULE_DEVICE_TABLE(pci, gaisler_ids);

static struct pci_driver grpci_driver = {
  .name = GRPCI_DEVICE_NAME"_pci",   /* The name of the driver, "it must be unique among all PCI drivers in the kernel
                                        and is normally set to the same name as the module name of the driver.
                                        It shows in sysfs under /sys/bus/pci/drivers" [LnxDrv3] p.311 */
  .id_table = gaisler_ids,
  .probe = grpci_probe,        /* "This function is called by the PCI core when it has struct pci_dev that it thinks
                                  this driver wants to control." [ibid] p312 */
  .remove = grpci_remove,      /* This function is called by the PCI core when the struct pci_dev is being removed from the
                                  system, or when the PCI driver is being unloaded (upon any call to pci_unregister_driver(),
                                  any PCI device that were bound to this driver are removed, and the remove() function
                                  for this PCI driver is called before pci_unregister_driver() returns). [ibid] pp312 & 313*/
  /* .suspend & .resume fields are optional [ibid] */
};

/* Private data of one card */
static struct grpci_device grpci_dev = {
  .fops = {
    .owner =    THIS_MODULE,
    .llseek =   no_llseek,
    //.write =    grpci_write,
    //.read =     grpci_read,
    .ioctl =    grpci_ioctl,
    .open =     grpci_open,
    .release =  grpci_release,
  },
};

static unsigned int cards_found = 0;

static int __init grpci_init(void) {
  return pci_register_driver(&grpci_driver);
}

/*************************************************************************************
 *************************************************************************************
 **                                                                                 **
 **                              P R O B E   F U N C T I O N                        **
 **                                                                                 ** 
 *************************************************************************************
 *************************************************************************************/
/* "This function is called by the PCI core when it has struct pci_dev that it thinks
   this driver wants to control." [LnxDrv3] p312 ... */
static int grpci_probe(struct pci_dev* dev, const struct pci_device_id *id) {
  int ret = -EIO;
  unsigned int i, tmp;

  /***********************************************************************************
   *                               V E R I F Y   C A R D                             *        
   ***********************************************************************************/
  /* ... So let's check that we are called on the proper PCI card. */
  if ( dev->vendor != FROM_GRLIB_CFG_PCIVID || dev->device != FROM_GRLIB_CFG_PCIDID ) {
    printk(KERN_ERR GRPCI_PFX "** ERROR driver is asked for claiming of a PCI card with unexpected vendor and/or device id (%04x:%04x)\n",
           dev->vendor, dev->device);
    return -ENODEV;
  }
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG ok, probed PCI card %04x:%04x\n", dev->vendor, dev->device);
#endif /* GRPCI_DEBUG */
  cards_found++;
  /* For now, we only support ONE card */ /* TODO support multiple cards at a time */
  if (cards_found > 1) {
    printk(KERN_ERR GRPCI_PFX "** ERROR This driver only supports 1 card for now\n");
    return -ENODEV;
  }
  /***********************************************************************************
   *                               E N A B L E   C A R D                             *        
   ***********************************************************************************/
  /* Enable the card: "In the probe function for the PCI driver, before the driver can access
     any device ressource of the PCI device, the driver must call pci_enable_device()" [ibid] p314 */
  if (pci_enable_device(dev)) {
    printk(KERN_ERR GRPCI_PFX "** ERROR Could not enable card\n");
    return -ENODEV;
  }
  /* Get the card's resource start & end addresses, and flags */
  grpci_dev.resstart = pci_resource_start(dev, 0);
  if (!grpci_dev.resstart) {
    printk(KERN_ERR GRPCI_PFX "** ERROR No I/O-Address for card detected\n");
    ret = -ENODEV;
    goto err_out_disable_device;
  }
  grpci_dev.pcidev = dev;
  grpci_dev.resend = pci_resource_end(dev, 0);
  grpci_dev.reslen = pci_resource_len(dev, 0);
  grpci_dev.resflags = pci_resource_flags(dev, 0);
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG ok, enabled card (BAR%d=0x%08x-0x%08x, %s, %s)\n", 0, UINT(grpci_dev.resstart), UINT(grpci_dev.resend),
         grpci_dev.resflags & IORESOURCE_IO ? "I/O" : "mem", grpci_dev.resflags & IORESOURCE_PREFETCH ? "prefetch" : "no prefetch");
#endif /* GRPCI_DEBUG */
  /* Request PCI regions */
  if (pci_request_regions(dev, GRPCI_DEVICE_NAME)) {
    printk(KERN_ERR GRPCI_PFX "** ERROR I/O address 0x%04x already in use\n", (int) grpci_dev.resstart);
    ret = -EIO;
    goto err_out_disable_device;
  }
  /* Probe value of MIN_GNT */
  pci_read_config_dword(grpci_dev.pcidev, GRPCI_PCICONFIG_MAXLAT_MINGNT_INT, &tmp);
  rmb();
  printk(KERN_INFO GRPCI_PFX "DEBUG Value of MIN_GNT register: 0x%08x\n", tmp);
  /***********************************************************************************
   *                               M A J O R   N U M B E R                           *        
   ***********************************************************************************/
  /* dynamically allocate a major number */
  grpci_dev.major = alloc_chrdev_region(&grpci_dev.devt, GRPCI_FIRST_MINOR, GRPCI_DEV_COUNT, GRPCI_DEVICE_NAME);
  if (grpci_dev.major<0) {
    printk(KERN_ERR GRPCI_PFX "** ERROR could not dynamically allocate a major number\n");
    ret = -EIO;
    goto err_out_release_regions;
  }
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG ok dynamically got major number %d\n", MAJOR(grpci_dev.devt));
#endif /* GRPCI_DEBUG */
  /***********************************************************************************
   *                         R E G I S T E R   C H A R   D R I V E R                 *        
   ***********************************************************************************/
  /* register a char driver */
  grpci_dev.cdev.ops = &grpci_dev.fops; /* binds char driver & file operations [LnxDrv3] p56 */
  grpci_dev.cdev.owner = THIS_MODULE;
  cdev_init(&grpci_dev.cdev, &grpci_dev.fops); /* [ibid] */
  if (cdev_add(&grpci_dev.cdev, grpci_dev.devt, GRPCI_DEV_COUNT)<0) { /* "As soon as cdev_add() returns, your device
																		 is 'live' and its operations can be called by
																		 the kernel" [ibid] p56 */
    printk(KERN_ERR GRPCI_PFX "** ERROR Could not register char driver\n");
    ret = -EIO;
    goto err_out_free_major_number;
  }
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG ok registered char driver\n");
#endif /* GRPCI_DEBUG */
  /***********************************************************************************
   *                               I O R E M A P   C A R D                           *        
   ***********************************************************************************/
  /* ioremap the device */
  grpci_dev.bar0 = (unsigned int*)ioremap_nocache(grpci_dev.resstart, grpci_dev.reslen);
  if (!grpci_dev.bar0) {
    printk(KERN_ERR GRPCI_PFX "** ERROR Failed to ioremap PCI card memory (start=0x%08x, len=0x%08x)\n", UINT(grpci_dev.resstart), UINT(grpci_dev.reslen));
    ret = -EIO;
    goto err_out_remove_char_driver;
  }
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG ok ioremapped PCI card memory: bar0 (start=0x%08x, len=0x%08x) remapped to 0x%08x\n",
    UINT(grpci_dev.resstart), UINT(grpci_dev.reslen), UINT(grpci_dev.bar0));
#endif /* GRPCI_DEBUG */
  /***********************************************************************************
   *                                   P O L L   G O K                               *        
   ***********************************************************************************/
  /* Start PCI handshake with Cardbus MIMO 1 board
   * Nothing must be transmitted to the board before its firmware set the GOK bit.
   * So poll on this bit, but before ARM A TIMER to handle a time-out security. */
  init_timer(&grpci_dev.timeout_timer_gok);
  grpci_dev.timeout_timer_gok.expires = jiffies + GRPCI_TIMEOUT_GOK_SECONDS*HZ;
  grpci_dev.timeout_timer_gok.data = (unsigned long)&grpci_dev;
  grpci_dev.timeout_timer_gok.function = grpci_timeout_gok;
  grpci_dev.gok = 0;
  grpci_dev.timeout_expired_gok = 0;
  add_timer(&grpci_dev.timeout_timer_gok);
  /* poll GOK bit actually */
  do
    pci_read_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL, &grpci_dev.gok);
  while (!(grpci_dev.gok & FROM_GRLIB_BOOT_GOK) && !grpci_dev.timeout_expired_gok);
  if (!(grpci_dev.gok & FROM_GRLIB_BOOT_GOK)) { /* we left the loop because of the time-out timer */
    printk(KERN_INFO GRPCI_PFX "** ERROR Time-out: card's ready-bit (GOK) not set. Driver installation cancelled.\n");
    ret = -EIO;
    goto err_out_iounmap;
  }
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG ok, card set its ready-bit (GOK).\n");
#endif /* GRPCI_DEBUG */
  /***********************************************************************************
   *                           G E T  G R P C I   C O N F I G                        *        
   ***********************************************************************************/
  /* PCI card booted A-Ok, we can get informations from registers settled inside the config space */
  pci_read_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_EXPPAGE0,    &grpci_dev.exppage0   );
  pci_read_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_SHARED0OFF,  &grpci_dev.shared0off );
  pci_read_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_SHARED0SIZE, &grpci_dev.shared0size);
  /* Write back the value of EXPPAGE0 to register PAGE0 of GRPCI (the PAGE0 register is accessed
   * through upper half of the PCI address space defined by BAR0, see [Grip] pp 295 & 295) */
  grpci_writel(grpci_dev.exppage0, (unsigned int*)(((unsigned char*)grpci_dev.bar0)+0x100000));
  wmb();
  printk(KERN_INFO GRPCI_PFX "PAGE0 set to 0x%08x\n",
    grpci_readl((unsigned int*)(((unsigned char*)grpci_dev.bar0)+0x100000))); /* max 0x200000 */
  /* debug printkies */
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG grpci_dev.gok         = 0x%08x\n", grpci_dev.gok        );
  printk(KERN_INFO GRPCI_PFX "DEBUG grpci_dev.exppage0    = 0x%08x\n", grpci_dev.exppage0   );
  printk(KERN_INFO GRPCI_PFX "DEBUG grpci_dev.shared0off  = 0x%08x\n", grpci_dev.shared0off );
  printk(KERN_INFO GRPCI_PFX "DEBUG grpci_dev.shared0size = 0x%08x\n", grpci_dev.shared0size);
#endif /* GRPCI_DEBUG */
  /***********************************************************************************
   *                                   S E T   H O K                                 *        
   ***********************************************************************************/
  /* Ok, we can set the HOK bit */
  pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL, FROM_GRLIB_BOOT_HOK);
  wmb();
  for (i=0;i<10;i++) udelay(10000); /* this waits for 100 milliseconds */
  /***********************************************************************************
   * A u t h o r i z e   c a r d   t o   p e r f o r m   P C I   M S T   C y c l e s *
   ***********************************************************************************/
  pci_read_config_dword(grpci_dev.pcidev, GRPCI_PCICONFIG_STS_CMD, &tmp);
  rmb();
  pci_write_config_dword(grpci_dev.pcidev, GRPCI_PCICONFIG_STS_CMD, tmp | GRPCI_PCIMASTER_ENABLE);
  wmb();
  /***********************************************************************************
   *                             P R O B E   F I N I S H E D                         *        
   *                                  ( r e t u r n   0 )                            *        
   ***********************************************************************************/
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG ok, probe() function of %s driver finished A-OK\n", GRPCI_DEVICE_NAME);
#endif /* GRPCI_DEBUG */
  return 0; /* success */

err_out_iounmap:
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG passed label err_out_iounmap\n");
#endif /* GRPCI_DEBUG */
  iounmap(grpci_dev.bar0);
err_out_remove_char_driver:
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG passed label err_out_remove_char_driver\n");
#endif /* GRPCI_DEBUG */
  cdev_del(&grpci_dev.cdev); /* no access of cdev field anymore ([LnxDrv3] p56) */
err_out_free_major_number:
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG passed label err_out_free_major_number\n");
#endif /* GRPCI_DEBUG */
  unregister_chrdev_region(grpci_dev.devt, GRPCI_DEV_COUNT);
err_out_release_regions:
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG passed label err_out_release_regions\n");
#endif /* GRPCI_DEBUG */
  pci_release_regions(grpci_dev.pcidev);
err_out_disable_device:
#ifdef GRPCI_DEBUG
  printk(KERN_INFO GRPCI_PFX "DEBUG passed label err_out_disable_device\n");
#endif /* GRPCI_DEBUG */
  pci_disable_device(grpci_dev.pcidev);

  cards_found--;                                             /* WEIRD : why did I had to do that here ?
                                                                when grpci_remove() exactly is called ???!!!
                                                                (it is the one that should do that) */
  return ret;
}
/*************************************************************************************
 *************************************************************************************
 **                                                                                 **
 **                   E N D   O F   P R O B E   F U N C T I O N                     **
 **                                                                                 ** 
 *************************************************************************************
 *************************************************************************************/

void grpci_timeout_gok(unsigned long data) {
  struct grpci_device* grpcidev = (struct grpci_device*)data;
  grpcidev->timeout_expired_gok = 1;
}

static int grpci_open(struct inode* inode, struct file* filp) {
#ifdef GRPCI_DEBUG
  printk(KERN_DEBUG GRPCI_PFX "DEBUG 'been in grpci_open()\n");
#endif /* GRPCI_DEBUG */
  /* No use of private_data for now ... */
      //struct grpci_device* grpcidev; /* device information */
      //grpcidev = container_of(inode->i_cdev, struct grpci_device, cdev);
      //filp->private_data = grpcidev; /* for other methods */
  /* ...that's why I put this in comment (see [LnxDrv3] p59 for more info) */
  return 0;
}

/*************************************************************************************
 *************************************************************************************
 **                                                                                 **
 **                              I O C T L   F U N C T I O N                        **
 **                                                                                 ** 
 *************************************************************************************
 *************************************************************************************/
int grpci_ioctl(struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg) {
  /* If the invoking program doesn't pass a third argument to the ioctl() system call,
     the arg value received by the driver operation is undefined [LnxDrv3] p136 */
  int retval = 0;
  unsigned int tmp = 0;
  unsigned int ltmp;
  int i; /* good old fellow */

#ifdef GRPCI_DEBUG
  printk(KERN_EMERG GRPCI_PFX "DEBUG grpci_ioctl() was called with cmd = 0x%08x,arg = 0x%08x\n", cmd, UINT(arg));
#endif /* GRPCI_DEBUG */

  /* Verify correctness of cmd value */
  if (_IOC_TYPE(cmd) != GRPCI_IOCTRL_MAGIC) {
#ifdef GRPCI_DEBUG
    printk(KERN_EMERG GRPCI_PFX "ERROR Called grpci_ioctl() with invalid cmd (0x%08x)\n", cmd);
#endif /* GRPCI_DEBUG */
    return -ENOTTY;\
  }

  switch (cmd) {

    /********************************
     * Writing registers of ADF4108 *
     ********************************/
    case GRPCI_IOCTRL_ADF4108_WRITE_REG:
#ifdef GRPCI_DEBUG
      printk(KERN_EMERG GRPCI_PFX "DEBUG Ok recognized GRPCI_IOCTRL_ADF4108_WRITE_REG ioctl.\n");
#endif /* GRPCI_DEBUG */
      /* Get the values to write in the registers of ADF4108 frequency synthesizer
         (see [ADF4108] pp 11-12) */
      grpci_dev.RFctrl.ADF4108_Func0   = *((unsigned int*)arg);
      grpci_dev.RFctrl.ADF4108_Ref_Cnt = *(((unsigned int*)arg)+1);
      grpci_dev.RFctrl.ADF4108_N_Cnt   = *(((unsigned int*)arg)+2);
#ifdef GRPCI_DEBUG
	  printk(KERN_EMERG GRPCI_PFX "DEBUG grpci_dev.RFctrl.ADF4108_Func0 = 0x%08x\n", grpci_dev.RFctrl.ADF4108_Func0);
	  printk(KERN_EMERG GRPCI_PFX "DEBUG grpci_dev.RFctrl.ADF4108_Ref_Cnt = 0x%08x\n", grpci_dev.RFctrl.ADF4108_Ref_Cnt);
	  printk(KERN_EMERG GRPCI_PFX "DEBUG grpci_dev.RFctrl.ADF4108_N_Cnt = 0x%08x\n", grpci_dev.RFctrl.ADF4108_N_Cnt);
#endif /* GRPCI_DEBUG */
      /* It may seem better to verify at this point the consistency of the values, passed through use of the
         arg parameter, that are to be written in the registers of ADF Freq. synthesizer.
         Indeed, that would be a pity to transfer a spurious value to the Cardbus MIMO board firmware
         and get a returned error response because the value is not coherent.
         ...
         ANYWAY, we DON'T perform any test, because we don't want to introduce policy in kernel code
		 (besides, not making this test cannot be harmful to the op. system).
         In turn, the Cardbus MIMO board firmware won't check the value.
         ...
         So the user code IS RESPONSIBLE for providing proper value. */
      /* Transmit the values in the CTRL0-3 registers. */
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL0, grpci_dev.RFctrl.ADF4108_Func0);
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL1, grpci_dev.RFctrl.ADF4108_Ref_Cnt);
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL2, grpci_dev.RFctrl.ADF4108_N_Cnt);
      wmb();
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_SET_ADF4108_REG | FROM_GRLIB_IRQ_FROM_PCI);
      wmb();
      /* We wait for acknowledge of the irq by the Cardbus MIMO board firmware
         (even if it may be dangerous for now, because we are in development phase,
         we are obliged to do so, unless we may perform several writings without the
         Cardbus MIMO board firmware having time to actually perform them... */

      /* So poll the IRQ bit */
      do {
        pci_read_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL, &tmp);
        rmb();
#ifdef GRPCI_DEBUG
        printk(KERN_INFO GRPCI_PFX "DEBUG tmp = 0x%08x\n", tmp);
#endif /* GRPCI_DEBUG */
      } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
#ifdef GRPCI_DEBUG
      printk(KERN_INFO GRPCI_PFX "DEBUG last tmp = 0x%08x\n", tmp);
#endif /* GRPCI_DEBUG */
      break;

    /************************************
     * Writing INIT register of ADF4108 *
     ************************************/
    case GRPCI_IOCTRL_ADF4108_INIT:
#ifdef GRPCI_DEBUG
      printk(KERN_EMERG GRPCI_PFX "DEBUG Ok recognized GRPCI_IOCTRL_ADF4108_INIT ioctl.\n");
#endif /* GRPCI_DEBUG */
      /* Get the values to write in the Initialization register of ADF4108 frequency synthesizer
         (see [ADF4108] pp 11 & 15) */
      grpci_dev.RFctrl.ADF4108_Init = (unsigned int)arg;
#ifdef GRPCI_DEBUG
	  printk(KERN_EMERG GRPCI_PFX "DEBUG grpci_dev.RFctrl.ADF4108_Init = 0x%08x\n", grpci_dev.RFctrl.ADF4108_Init);
#endif /* GRPCI_DEBUG */
      /* It may seem better to verify at this point the consistency of the value, passed in the arg parameter,
         that is to be written in the Initialization register of ADF Freq. synthesizer.
         Indeed, that would be a pity to transfer a spurious value to the Cardbus MIMO board firmware
         and get a returned error response because the value is not coherent.
         ...
         ANYWAY, we DON'T perform any test, because we don't want to introduce policy in kernel code
		 (besides, not making this test cannot be harmful to the op. system).
         In turn, the Cardbus MIMO board firmware won't check the value.
         ...
         So the user code IS RESPONSIBLE for providing proper value. */
      /* Transmit the value in the CTRL0 register */
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL3, grpci_dev.RFctrl.ADF4108_Init);
      wmb();
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_INIT_ADF4108 | FROM_GRLIB_IRQ_FROM_PCI);
      wmb();
      /* We wait for acknowledge of the irq by the Cardbus MIMO board firmware
         (even if it may be dangerous for now, because we are in development phase,
         we are obliged to do so, unless we may perform several writings without the
         Cardbus MIMO board firmware having time to actually perform them... */

      /* So poll the IRQ bit */
      do {
        pci_read_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL, &tmp);
        rmb();
#ifdef GRPCI_DEBUG
        printk(KERN_INFO GRPCI_PFX "DEBUG tmp = 0x%08x\n", tmp);
#endif /* GRPCI_DEBUG */
      } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
#ifdef GRPCI_DEBUG
      printk(KERN_INFO GRPCI_PFX "DEBUG last tmp = 0x%08x\n", tmp);
#endif /* GRPCI_DEBUG */
      break;

    /*****************************************
     * Writing KHZ Register of LFSW190410-50 *
     *****************************************/
	case GRPCI_IOCTRL_LFSW190410_WRITE_KHZ:
      /* Get the value to write in KHZ register of LFSW190410-50 frequency synthesizer
         (see [LFSW190410] & [AN7100A] p4) */
#ifdef GRPCI_DEBUG
      printk(KERN_EMERG GRPCI_PFX "DEBUG Ok recognized GRPCI_IOCTRL_LFSW190410_WRITE_KHZ ioctl.\n");
#endif /* GRPCI_DEBUG */
      grpci_dev.RFctrl.LFSW190410_KHZ = (char*)arg;
#ifdef GRPCI_DEBUG
	  for (i=0;i<8;i++) printk(KERN_EMERG GRPCI_PFX "DEBUG grpci_dev.RFctrl.LFSW190410_KHZ[%d] = %c\n", i, grpci_dev.RFctrl.LFSW190410_KHZ[i]);
#endif /* GRPCI_DEBUG */
      /* Same remark as for R counter reg of ADF4108 (see above): we don't verify correctness
         of the value passed in the arg parameter. */
      /* Transmit the ASCII value in the CTRL0 & CTRL1 registers */
#define invert4(x)        {ltmp=x; x=((ltmp & 0xff)<<24) | ((ltmp & 0xff00)<<8) | \
                         ((ltmp & 0xff0000)>>8) | ((ltmp & 0xff000000)>>24); }
      invert4(*((unsigned int*)grpci_dev.RFctrl.LFSW190410_KHZ));  /* because Sparc is big endian */
      invert4(*((unsigned int*)(grpci_dev.RFctrl.LFSW190410_KHZ+4))); /* because Sparc is big endian */
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL0, 'K');
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL1, *((unsigned int*)grpci_dev.RFctrl.LFSW190410_KHZ));
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL2, *((unsigned int*)(grpci_dev.RFctrl.LFSW190410_KHZ+4)));
      wmb();
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_SET_LFSW190410_KHZ | FROM_GRLIB_IRQ_FROM_PCI);
      wmb();
      /* Same remark as for R & N counter regs of ADF4108 (see above): we poll irq bit
         (possibly blocking local machine!...) */

    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES          /* So poll the IRQ bit */
    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES          do {
    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES            pci_read_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL, &tmp);
    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES            rmb();
    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES    #ifdef GRPCI_DEBUG
    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES            printk(KERN_INFO GRPCI_PFX "DEBUG tmp = 0x%08x\n", tmp);
    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES    #endif /* GRPCI_DEBUG */
    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES          } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES    #ifdef GRPCI_DEBUG
    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES          printk(KERN_INFO GRPCI_PFX "DEBUG last tmp = 0x%08x\n", tmp);
    // JUST REMOVED BIT POLLING (FOR TEST): RESET THESE LINES    #endif /* GRPCI_DEBUG */
      break;

    /***************************
     * Configuring RF switches *
     ***************************/
    case GRPCI_IOCTRL_RF_SWITCH_CTRL:
#ifdef GRPCI_DEBUG
      printk(KERN_EMERG GRPCI_PFX "DEBUG Ok recognized GRPCI_IOCTRL_RF_SWITCH_CTRL ioctl.\n");
#endif /* GRPCI_DEBUG */
      /* Get the values to write in the registers of ADF4108 frequency synthesizer
         (see [ADF4108] pp 11-12) */
      grpci_dev.RFctrl.RFswitches_onoff = *((unsigned int*)arg);
      grpci_dev.RFctrl.RFswitches_mask = *(((unsigned int*)arg)+1);
#ifdef GRPCI_DEBUG
	  printk(KERN_EMERG GRPCI_PFX "DEBUG grpci_dev.RFctrl.RFswitches_onoff = 0x%08x\n", grpci_dev.RFctrl.RFswitches_onoff);
	  printk(KERN_EMERG GRPCI_PFX "DEBUG grpci_dev.RFctrl.RFswitches_mask = 0x%08x\n", grpci_dev.RFctrl.RFswitches_mask);
#endif /* GRPCI_DEBUG */
      /* It may seem better to verify at this point the consistency of the value, passed in the arg parameter,
         that is to be passed to hardware to control the swtiches of the RF chain.
         Indeed, that would be a pity to transfer a spurious value to the Cardbus MIMO board firmware
         and get a returned error response because the value is not coherent.
         ...
         ANYWAY, we DON'T perform any test, because we don't want to introduce policy in kernel code
		 (besides, not making this test cannot be harmful to the op. system, nor to RF chain hardware).
         In turn, the Cardbus MIMO board firmware won't check the value.
         ...
         So the user code IS RESPONSIBLE for providing proper value. */
      /* Transmit the value in the CTRL0 register */
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL0, grpci_dev.RFctrl.RFswitches_onoff);
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL1, grpci_dev.RFctrl.RFswitches_mask);
      wmb();
      pci_write_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL, FROM_GRLIB_BOOT_HOK | FROM_GRLIB_IRQ_FROM_PCI_IS_SET_RF_SWITCH |
        FROM_GRLIB_IRQ_FROM_PCI);
      wmb();
      /* We wait for acknowledge of the irq by the Cardbus MIMO board firmware
         (even if it may be dangerous for now, because we are in development phase,
         we are obliged to do so, unless we may perform several writings without the
         Cardbus MIMO board firmware having time to actually perform them... */

      /* So poll the IRQ bit */
      do {
        pci_read_config_dword(grpci_dev.pcidev, GRPCI_IOCONFIG_CTRL, &tmp);
        rmb();
#ifdef GRPCI_DEBUG
        printk(KERN_INFO GRPCI_PFX "DEBUG tmp = 0x%08x\n", tmp);
#endif /* GRPCI_DEBUG */
      } while (tmp & FROM_GRLIB_IRQ_FROM_PCI);
#ifdef GRPCI_DEBUG
      printk(KERN_INFO GRPCI_PFX "DEBUG last tmp = 0x%08x\n", tmp);
#endif /* GRPCI_DEBUG */
      break;

    default:
      retval = -ENOTTY;
      break;
  }
  return retval;
}

static int grpci_release(struct inode* inode, struct file* file) {
#ifdef GRPCI_DEBUG
  printk(KERN_DEBUG GRPCI_PFX "DEBUG 'been in grpci_release()\n");
#endif /* GRPCI_DEBUG */
  return 0;
}

static void grpci_remove(struct pci_dev* dev) {
  /* This function is called by the PCI core when the struct pci_dev is being removed from the
     system, or when the PCI driver is being unloaded (upon any call to pci_unregister_driver(),
     any PCI device that were bound to this driver are removed, and the remove() function
     for this PCI driver is called before pci_unregister_driver() returns). [LnxDrv3] pp312 & 313 */
  if (dev == grpci_dev.pcidev) {
#ifdef GRPCI_DEBUG
    printk(KERN_DEBUG GRPCI_PFX "DEBUG 'been in grpci_remove()...\n");
#endif /* GRPCI_DEBUG */
    /* iounmap */
    iounmap(grpci_dev.bar0);
    /* unregister the char driver */
    cdev_del(&grpci_dev.cdev); /* should not access this cdev field anymore from now on */  /* UTILISER CONTAINER_OF() PLUTOT !!! */
    /* free major number */
    unregister_chrdev_region(grpci_dev.devt, GRPCI_DEV_COUNT); /* UTILISER CONTAINER_OF() PLUTOT !!! */
    pci_release_regions(dev);
    pci_disable_device(dev);
    cards_found--;
  }
}

static void __exit grpci_exit(void) {
  pci_unregister_driver(&grpci_driver); /* see [LnxDrv3] pp312 sq */
  printk(KERN_INFO GRPCI_PFX "module unloaded\n");
}

module_init(grpci_init);
module_exit(grpci_exit);

MODULE_AUTHOR("Karim.Khalfallah@eurecom.fr (based on code by A. Rubini & W. Van Sebroeck");
MODULE_DESCRIPTION("CardBus driver for Eurecom's Openair card");
//????MODULE_LICENSE("GPL");
//MODULE_ALIAS_MISCDEV(GRPCI_MINOR_0);
