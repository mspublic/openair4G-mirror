/** main_module.c
 * 
 *  Main Kernel module functions for load/init and cleanup of the kernel driver
 * 
 *  Detects one or more ExpressMIMO 1 & 2 cards
 * 
 *  Authors: Matthias Ihmig <matthias.ihmig@mytum.de>, 2012
 *           Riadh Ghaddab <riadh.ghaddab@eurecom.fr>
 *           Raymond Knopp <raymond.knopp@eurecom.fr>
 * 
 *  Changelog:
 *  14.01.2013: removed remaining of BIGPHYS stuff and replaced with pci_alloc_consistent
 */

#ifndef USER_MODE
#define __NO_VERSION__
#endif

#include "device.h"
#include "exmimo_fw.h"
#include "defs.h"
#include "vars.h"


#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/aer.h>
#include <linux/pci_regs.h>


static int  openair_init_module(void);
static void openair_cleanup_module(void);
static void openair_cleanup(void);



extern int intr_in;
/*------------------------------------------------*/

static struct file_operations openair_fops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
    unlocked_ioctl:openair_device_ioctl
#else
    ioctl:  openair_device_ioctl
#endif
    ,
    open:   openair_device_open,
    release:openair_device_release,
    mmap:   openair_device_mmap
};


//extern int pci_enable_pcie_error_reporting(struct pci_dev *dev);
//extern int pci_cleanup_aer_uncorrect_error_status(struct pci_dev *dev);


static int __init openair_init_module( void )
{
    int res = 0;
    unsigned int readback;
    unsigned int i, j;
    
    char *adr;
    int temp_size;
    unsigned int vid,did;
    unsigned short subid;
    

    bigshm_size_pages = ( (2*MAX_ANTENNA*( TRX_CNT_SIZE_B + ADAC_BUFFERSZ_PERCHAN_B )) >> PAGE_SHIFT ) +1;

#ifdef NOCARD_TEST
#error FIXME: Implement this
#endif
    
    //------------------------------------------------
    // Find and enable ExpressMIMO PCI cards
    //------------------------------------------------
    //
    i = 0;
    pdev[i] = pci_get_device(XILINX_VENDOR, XILINX_ID, NULL);
    if( pdev[i] )
    {
        printk("[openair][INIT_MODULE][INFO]:  openair card (ExpressMIMO) %d found, bus 0x%x, primary 0x%x, secondary 0x%x\n",i,
                 pdev[i]->bus->number, pdev[i]->bus->primary,pdev[i]->bus->secondary);

        pci_read_config_word(pdev[i], PCI_SUBSYSTEM_ID, &subid);
        pci_read_config_word(pdev[i], PCI_SUBSYSTEM_VENDOR_ID, &exmimo_id[i].board_vendor);
        if ( exmimo_id[i].board_vendor == XILINX_VENDOR )
            exmimo_id[i].board_vendor = EURECOM_VENDOR; // set default to EURECOM
            
        exmimo_id[i].board_exmimoversion = (subid >> 12) & 0x0F;
            
        exmimo_id[i].board_hwrev  = (subid >>  8) & 0x0F;
        exmimo_id[i].board_swrev  = (subid      ) & 0xFF;
        printk("[openair][INIT_MODULE][INFO]: card %d: ExpressMIMO-%i (HW Rev %i), Bitstream: %s, SW/Protocol Revision: 0x%02X\n", i,
                 exmimo_id[i].board_exmimoversion, exmimo_id[i].board_hwrev, ( (exmimo_id[i].board_vendor == EURECOM_VENDOR) ? "Eurecom" : "Telecom Paristech"), exmimo_id[i].board_swrev );
        
        i++;
        vid = XILINX_VENDOR;
        did = XILINX_ID;
    }
    else {
        printk("[openair][INIT_MODULE][INFO]:  no card found, stopping.\n");
        return -ENODEV;
    }

    // Now look for more cards on the same bus
    while (i<MAX_CARDS)
    {
        pdev[i] = pci_get_device(vid,did, pdev[i-1]);
        if(pdev[i])
        {
            printk("[openair][INIT_MODULE][INFO]: openair card %d found, bus 0x%x, primary 0x%x, secondary 0x%x\n",i,
                    pdev[i]->bus->number,pdev[i]->bus->primary,pdev[i]->bus->secondary);

            pci_read_config_word(pdev[i], PCI_SUBSYSTEM_ID, &subid);
            pci_read_config_word(pdev[i], PCI_SUBSYSTEM_VENDOR_ID, &(exmimo_id[i].board_vendor));
            if ( exmimo_id[i].board_vendor == XILINX_VENDOR )
                exmimo_id[i].board_vendor = EURECOM_VENDOR;
            
            exmimo_id[i].board_exmimoversion = (subid >> 12) & 0x0F;
            if (exmimo_id[i].board_exmimoversion == 0) 
                exmimo_id[i].board_exmimoversion = 1;
            
            exmimo_id[i].board_hwrev  = (subid >>  8) & 0x0F;
            exmimo_id[i].board_swrev  = (subid      ) & 0xFF;
            printk("[openair][INIT_MODULE][INFO]: card %d: ExpressMIMO-%i (HW Rev %i), Bitstream: %s, SW/Protocol Revision: 0x%02X\n", i,
                     exmimo_id[i].board_exmimoversion, exmimo_id[i].board_hwrev, ( (exmimo_id[i].board_vendor == EURECOM_VENDOR) ? "Eurecom" : "Telecom Paristech"), exmimo_id[i].board_swrev );

            i++;
        }
        else
            break;
    }

    // at least one device found, enable it
    number_of_cards = i;

    for (i=0; i<number_of_cards; i++)
    {
        if( pci_enable_device(pdev[i]) )
        {
            printk("[openair][INIT_MODULE][INFO]: Could not enable PCI card device %d\n",i);
            openair_cleanup();
            return -ENODEV;
        }
        else {
            printk("[openair][INIT_MODULE][INFO]: *** CARD DEVICE %d (pdev=%p) ENABLED, irq %d\n",i,pdev[i],pdev[i]->irq);
            openair_pci_device_enabled[i] = 1;
        }
        // Make the FPGA to a PCI master
        pci_set_master(pdev[i]);

        if (pci_enable_pcie_error_reporting(pdev[i]) > 0)
            printk("[openair][INIT_MODULE][INFO]: Enabled PCIe error reporting\n");
        else
            printk("[openair][INIT_MODULE][INFO]: Failed to enable PCIe error reporting\n");

        pci_cleanup_aer_uncorrect_error_status(pdev[i]);
        
        mmio_start[i]  = pci_resource_start(pdev[i], 0); // get start of BAR0
        mmio_length[i] = pci_resource_len  (pdev[i], 0);
        mmio_flags[i]  = pci_resource_flags(pdev[i], 0);

        if (check_mem_region(mmio_start[i],256) < 0)
        {
            printk("[openair][INIT_MODULE][FATAL] : Cannot get memory region 0, aborting\n");
            mmio_start[i] = 0;
            openair_cleanup();
            return(-1);
        }
        else 
            printk("[openair][INIT_MODULE][INFO] : Reserving memory region 0 : mmio_start = 0x%x\n",mmio_start[i]);

        request_mem_region(mmio_start[i],256,"openair_rf");

        bar[i] = pci_iomap( pdev[i], 0, mmio_length[i] );   // get virtual kernel address for BAR0
        
        printk("[openair][INIT_MODULE][INFO]: BAR0 card %d = 0x%p\n", i, bar[i]);

        printk("[openair][INIT_MODULE][INFO]: Writing 0x%x to BAR0+0x1c (PCIBASEL)\n", 0x12345678);

        iowrite32( 0x12345678, (bar[i]+0x1c) );
        udelay(100);
        readback = ioread32( bar[i]+0x1c );
        if (readback != 0x12345678)
        {
            printk("[openair][INIT_MODULE][INFO]: Readback of FPGA register failed (%x)\n",readback);
            openair_cleanup();
            return(-1);
        }
        iowrite32((1<<8) | (1<<9) | (1<<10),bar[i]); // bit8=AHBPCIE_CTL0_SOFTRESET, but what is bit9 and bit10?
        udelay(1000);
        readback = ioread32(bar[i]);
        printk("CONTROL0 readback %x\n",readback);


        // Allocating large shared memory for DMA data exchange 
        //
        printk("[openair][module] calling pci_alloc_consistent for card %d, bigshm (size: %lu*%lu bytes)...\n", i, bigshm_size_pages, PAGE_SIZE);
        
        bigshm_head[i] = pci_alloc_consistent( pdev[i], bigshm_size_pages<<PAGE_SHIFT, &bigshm_head_phys[i] );
        
        if (bigshm_head[i] == NULL) {
            printk("[openair][MODULE][ERROR] Cannot Allocate Memory for shared data (bigshm)\n");
            openair_cleanup();
            return -ENODEV;
        }
        else {
            printk("[openair][MODULE][INFO] Bigshm at %p (phys %x)\n", bigshm_head[i], bigshm_head_phys[i]);
            
            bigshm_currentptr[i] = bigshm_head[i];

            adr = (char *) bigshm_head[i];
            temp_size = bigshm_size_pages << PAGE_SHIFT;
            while (temp_size > 0) {
                SetPageReserved( virt_to_page(adr) );
                adr += PAGE_SIZE;
                temp_size -= PAGE_SIZE;
            } 
            memset(bigshm_head[i], 0, bigshm_size_pages << PAGE_SHIFT);
            
        }
        
        if ( exmimo_firmware_init( i ) ) {
            printk("[openair][MODULE][ERROR] pci_alloc_consistent failed for pci_interface_bot content!\n");
            openair_cleanup();
            return(-1);
        }

        if ( exmimo_assign_shm_vars( i ) ) {
            printk("[openair][MODULE][ERROR] Not enough shared memory was allocated for all variables!\n");
            openair_cleanup();
            return(-1);
        }

        printk("[OPENAIR][SCHED][INIT] Trying to get IRQ %d\n",pdev[i]->irq);

        openair_irq_enabled[i] = 0;
      
        if (request_irq(pdev[i]->irq, openair_irq_handler, 
                        IRQF_SHARED /* was:IRQF_SHARED, try:0*/ , "openair_rf", pdev[i]) == 0)
        {
            openair_irq_enabled[i] = 1;
        }
        else {
            printk("[EXMIMO][SCHED][INIT] Cannot get IRQ %d for HW\n",pdev[i]->irq);
            openair_cleanup();
            return(-1);
        }
    } // for (i=0; i<number_of_cards; i++)
    
    //------------------------------------------------
    // Register the device in /dev
    //------------------------------------------------
    //
    major = openair_MAJOR;

    if( (res = register_chrdev(major, "openair", &openair_fops )) < 0)
    {
        printk("[openair][INIT_MODULE][ERROR]:  can't register char device driver, major : %d, error: %d\n", major, res);
        for (j=0; j<=number_of_cards; j++)
            release_mem_region(mmio_start[j],256);
        return -EIO;
    } else {
        printk("[openair][INIT_MODULE][INFO]:  char device driver registered major : %d\n", major);
        openair_chrdev_registered = 1;
    }

    printk("[openair][MODULE][INFO] Done init\n");
    return 0;
}

  
static void __exit openair_cleanup_module(void)
{
    printk("[openair][CLEANUP MODULE]\n");
    openair_cleanup();
}

static void  openair_cleanup(void)
{
    int i;

    if ( openair_chrdev_registered )
        unregister_chrdev(major,"openair");
    openair_chrdev_registered = 0;

    for (i=0; i<number_of_cards; i++)
    {
        // unregister interrupt
        if ( openair_irq_enabled[i] ) {
            printk("[openair][CLEANUP] disabling interrupt card %d\n", i);
            free_irq( pdev[i]->irq, pdev[i] );
            openair_irq_enabled[i] = 0;
        }

        exmimo_firmware_cleanup( i );

        if ( bigshm_head[i] )
        {
            char *adr;
            int temp_size;
            
            printk("free bigshm_head[%d] pdev %p, size %lu, head %p, phys %x\n", i, pdev[i], bigshm_size_pages<<PAGE_SHIFT, bigshm_head[i], bigshm_head_phys[i]);
            adr = (char *) bigshm_head[i];
            temp_size = bigshm_size_pages << PAGE_SHIFT;
            while (temp_size > 0) {
                ClearPageReserved( virt_to_page(adr) );
                adr += PAGE_SIZE;
                temp_size -= PAGE_SIZE;
            } 
            pci_free_consistent(pdev[i], bigshm_size_pages<<PAGE_SHIFT, bigshm_head[i], bigshm_head_phys[i]);
        }

        if ( bar[i] ) {
            printk("unmap bar[%d] %p\n", i, bar[i]);
            iounmap((void *)bar[i]);
        }

        if ( mmio_start[i] ) {
            printk("release mem[%d] %x\n", i, mmio_start[i]);
            release_mem_region(mmio_start[i],256);
        }
        
        if ( openair_pci_device_enabled[i] ) {
            printk("pci_disable_device %i\n", i);
            pci_disable_device( pdev[i] ); 
        }
    }
}

MODULE_AUTHOR
  ("Raymond KNOPP <raymond.knopp@eurecom.fr>, Florian KALTENBERGER <florian.kaltenberger@eurecom.fr>, Matthias IHMIG <matthias.ihmig@eurecom.fr>");
MODULE_DESCRIPTION ("openair ExpressMIMO/ExpressMIMO2 driver");
MODULE_LICENSE ("GPL");
module_init (openair_init_module);
module_exit (openair_cleanup_module);
