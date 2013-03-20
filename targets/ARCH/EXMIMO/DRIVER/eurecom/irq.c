/** irq.c
      - IRQ Handler: IRQ from Leon to PCIe/Kernel: exmimo_irq_handler
      - sends received packets to userspace

      - send command from PC to Leon and trigger IRQ on Leon using CONTROL1 register
      - commands are defined in $OPENAIR0/express-mimo/software/pcie_interface.h

      - added: pass card_id as parameter to tasklet and irq handler

   Authors: 
       Raymond Knopp <raymond.knopp@eurecom.fr>
       Matthias Ihmig <matthias.ihmig@mytum.de>, 2011, 2013
   */

#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/swab.h>

#include "openair_device.h"
#include "extern.h"

unsigned int openair_bh_cnt;

int get_frame_done = 0;

void pcie_printk(int card_id);

void openair_do_tasklet (unsigned long card_id);
DECLARE_TASKLET (openair_tasklet, openair_do_tasklet, 0);

irqreturn_t openair_irq_handler(int irq, void *cookie)
{
    unsigned int irqval;
    unsigned int irqcmd = EXMIMO_NOP;
    unsigned long card_id; // = (unsigned long) cookie;
    unsigned int pcie_control = PCIE_CONTROL1;

    // check interrupt status register
    //pci_read_config_word(pdev[0],6 , &irqval);
    
    // find card_id
    for (card_id=0; card_id<MAX_CARDS; card_id++)
        if ( pdev[card_id] == cookie )
            break;

    if (card_id == MAX_CARDS) // seems this is not for us. assume card0 and let the "else" branch handle it
        card_id = 0;

    if (exmimo_pci_kvirt[card_id].exmimo_id_ptr->board_swrev == BOARD_SWREV_LEGACY)
        pcie_control = PCIE_CONTROL1;
    else if (exmimo_pci_kvirt[card_id].exmimo_id_ptr->board_swrev == BOARD_SWREV_CMDREGISTERS)
        pcie_control = PCIE_CONTROL2;


    //printk("irq hndl called: card_id=%i, irqval=%i\n", card_id, irqval);
    if (exmimo_pci_kvirt[card_id].exmimo_id_ptr->board_swrev == BOARD_SWREV_LEGACY) {
        // get AHBPCIE interrupt line
        irqval = ioread32(bar[card_id]+PCIE_CONTROL0);
        // clear PCIE interrupt bit (bit 7 of register 0x0)
        if ( (irqval&0x80) != 0)
            iowrite32(irqval&0xffffff7f,bar[card_id]+PCIE_CONTROL0);
    }

    irqcmd = ioread32(bar[card_id]+pcie_control);   // On ExMIMO2, this ioread is sufficient to clear the IRQ

    //printk("IRQ: ctrl0: %08x, ctrl1: %08x, ctrl2: %08x, status: %08x\n", ioread32(bar[card_id]+PCIE_CONTROL0), ioread32(bar[card_id]+PCIE_CONTROL1), ioread32(bar[card_id]+PCIE_CONTROL2), ioread32(bar[card_id]+PCIE_STATUS));
    
    if ( irqcmd != EXMIMO_NOP )
    {
        if (irqcmd == GET_FRAME_DONE)
        {
            get_frame_done = 1;
            //iowrite32(EXMIMO_NOP, bar[card_id]+pcie_control);
        }
        else if (irqcmd == PCI_PRINTK)
        {
            // TODO: copy printk_buffer into FIFO to allow several consecutive printks and ACK pcie_printk
            //iowrite32(EXMIMO_NOP, bar[card_id]+pcie_control);
        }
        
        openair_tasklet.data = card_id;
        tasklet_schedule(&openair_tasklet);
        openair_bh_cnt++;
        //printk("PCIE IRQ: openair_irq_handler(irqval=%i): irqcmd = %i (0x%X) (printkbuf: 0x %02ux %02ux\n", irqval, irqcmd, irqcmd, exmimo_pci_kvirt[card_id].printk_buffer_ptr[0], exmimo_pci_kvirt[card_id].printk_buffer_ptr[1]);
    }
    else {
        // other IRQ
    }
    return IRQ_HANDLED;
}

void openair_do_tasklet (unsigned long card_id)
{
    int save_irq_cnt = openair_bh_cnt;
    unsigned int irqcmd;
    unsigned int pcie_control = PCIE_CONTROL1;
    openair_bh_cnt = 0;
    
    if (exmimo_pci_kvirt[card_id].exmimo_id_ptr->board_swrev == BOARD_SWREV_LEGACY)
        pcie_control = PCIE_CONTROL1;
    else if (exmimo_pci_kvirt[card_id].exmimo_id_ptr->board_swrev == BOARD_SWREV_CMDREGISTERS)
        pcie_control = PCIE_CONTROL2;
        
    irqcmd = ioread32(bar[card_id]+pcie_control);
    
    if (save_irq_cnt > 1)
        printk("[openair][IRQ tasklet(%ld)]: Warning: received more than 1 PCIE IRQ (openair_bh_cnt=%i), only the last one is acted upon(irqcmd= %i (0x%X)\n", card_id, openair_bh_cnt, irqcmd, irqcmd);
    
    switch( irqcmd )
    {
        case PCI_PRINTK:
            // printk("Got PCIe interrupt for printk ...\n");
            pcie_printk((int) card_id);
            break;
            
        case GET_FRAME_DONE:
            printk("[openair][IRQ tasklet] : Got PCIe interrupt for GET_FRAME_DONE ...\n");
            break;
            
        case EXMIMO_NOP:
            break;
            
        default:
            printk("[openair][IRQ tasklet] : Got unknown PCIe cmd: card_id = %li, irqcmd(CONTROL1) = %i (0x%X)\n", card_id, irqcmd, irqcmd);
    }
    
    iowrite32(EXMIMO_NOP, bar[card_id]+pcie_control);
}   

void pcie_printk(int card_id)
{
    char *buffer = exmimo_pci_kvirt[card_id].printk_buffer_ptr;
    unsigned int len = ((unsigned int *)buffer)[0];
    unsigned int off=0,i;
    unsigned char *dword;
    unsigned char tmp;

    //printk("In pci_fifo_printk : buffer %p, len %d: \n",buffer,len);
    printk("[LEON card%d]: ", card_id);

    if (len<256)
    {
        if ( (len&3) >0 )
            off=1;
    
        for (i=0; i<(off+(len>>2)); i++)
        {
            dword = &((unsigned char *)buffer)[(1+i)<<2];
            tmp = dword[3];
            dword[3] = dword[0];
            dword[0] = tmp;
            tmp = dword[2];
            dword[2] = dword[1];
            dword[1] = tmp;
        }
        for (i=0; i<len; i++)
            printk("%c",((char*)&buffer[4])[i]);
    }
}


