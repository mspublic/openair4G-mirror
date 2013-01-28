/** irq.c
      - IRQ Handler: IRQ from Leon to PCIe/Kernel: exmimo_irq_handler
      - sends received packets to userspace

      - send command from PC to Leon and trigger IRQ on Leon
      - CONTROL1 commands are defined in ../pcie_defs.h

   Authors: 
       Raymond Knopp <raymond.knopp@eurecom.fr>
       Matthias Ihmig <matthias.ihmig@mytum.de>, 2011 
   */

#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/swab.h>

#include "device.h"
#include "extern.h"

unsigned int openair_bh_cnt;

void pcie_printk(void);

void openair_do_tasklet (unsigned long);
DECLARE_TASKLET (openair_tasklet, openair_do_tasklet, 0);

irqreturn_t openair_irq_handler(int irq, void *cookie)
{
    unsigned int irqval;
    unsigned int irqcmd;

    // check interrupt status register
    //pci_read_config_word(pdev[0],6 , &irqval);

    // get AHBPCIE interrupt line
    irqval = ioread32(bar[0]);
    //  printk("irq hndl called:%i\n", irqval);

    if ((irqval&0x80) != 0)
    {
        // clear PCIE interrupt bit (bit 7 of register 0x0)
        iowrite32(irqval&0xffffff7f,bar[0]);
        irqcmd = ioread32(bar[0]+0x4);
    
        tasklet_schedule(&openair_tasklet);
        openair_bh_cnt++;
        //printk("PCIE IRQ: openair_irq_handler(irqval=%i): irqcmd = %i (0x%X)\n", irqval, irqcmd, irqcmd);
    }
    else {
        // other IRQ
    }
    return IRQ_HANDLED;
}

void openair_do_tasklet (unsigned long unused)
{
    int save_irq_cnt = openair_bh_cnt;
    unsigned int irqcmd;
    openair_bh_cnt = 0;
    
    irqcmd = ioread32(bar[0]+0x4);
    
    if (save_irq_cnt > 1)
        printk("openair_do_tasklet(): Warning: received more than 1 PCIE IRQ (openair_bh_cnt=%i), only the last one is acted upon(irqcmd= %i (0x%X)\n", openair_bh_cnt, irqcmd, irqcmd);
    
    switch( irqcmd )
    {
        case SLOT_INTERRUPT:
            // process_slot_interrupt();
            printk("Got PCIe interrupt for SLOT_INTERRUPT\n");
            //intr_cnt++;
            break;
            
        case PCI_PRINTK:
            // printk("Got PCIe interrupt for printk ...\n");
            pcie_printk();
            break;
            
        case GET_FRAME_DONE:
            printk("Got PCIe interrupt for GET_FRAME_DONE ...\n");
            break;
            
        default:
            printk("Got unknown PCIe cmd: irqcmd(CONTROL1) = %i (0x%X)\n", irqcmd, irqcmd);
    }
}   
