/** fileops.c
 * 
 *  Device IOCTL File Operations on character device /dev/openair0
 * 
 *  Authors: Matthias Ihmig <matthias.ihmig@mytum.de>, 2012, 2013
 *           Riadh Ghaddab <riadh.ghaddab@eurecom.fr>
 *           Raymond Knopp <raymond.knopp@eurecom.fr>
 * 
 *  Changelog:
 *  14.01.2013: removed remaining of BIGPHYS stuff and replaced with pci_alloc_consistent
 */
 
#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#endif

#include "openair_device.h"
#include "defs.h"
#include "extern.h"

#include "pcie_interface.h"

#define invert4(x)  { \
    unsigned int ltmp; \
    ltmp=x; x=((ltmp & 0xff)<<24) | ((ltmp & 0xff00)<<8) | \
    ((ltmp & 0xff0000)>>8) | ((ltmp & 0xff000000)>>24); \
}

extern int get_frame_done;

//-----------------------------------------------------------------------------
int openair_device_open (struct inode *inode,struct file *filp)
{
    printk("[openair][MODULE]  openair_open()\n");
    return 0;
}

//-----------------------------------------------------------------------------
int openair_device_release (struct inode *inode,struct file *filp)
{
  //  printk("[openair][MODULE]  openair_release(), MODE = %d\n",openair_daq_vars.mode);
  return 0;
}

//-----------------------------------------------------------------------------
int openair_device_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long phys;
    unsigned long start = (unsigned long)vma->vm_start; 
    unsigned long size = (unsigned long)(vma->vm_end-vma->vm_start);
    unsigned long maxsize;
    unsigned long openair_mmap_ind = vma->vm_pgoff;
    
    unsigned card = 0;

    printk("[openair][MMAP]  called (start %lx, end %lx, pg_off %lx, size %lx)\n", 
        vma->vm_start, 
        vma->vm_end,
        vma->vm_pgoff,
        size);

    vma->vm_pgoff = 0;
    vma->vm_flags |= VM_RESERVED;

    if (openair_mmap_ind == openair_mmap_BIGSHM)
    {
        // map a buffer from bigshm
        maxsize = BIGSHM_SIZE_PAGES<<PAGE_SHIFT;
        if ( size > maxsize) {
            printk("[openair][MMAP][ERROR] Trying to map more than %d bytes (req size=%d)\n",
                (unsigned int)(BIGSHM_SIZE_PAGES<<PAGE_SHIFT), (unsigned int)size);
            return -EINVAL;
        }
        phys = bigshm_head_phys[card];
    }
    else if ( (openair_mmap_ind & 1) == 1) 
    {
        // mmap a RX buffer
        maxsize = ADAC_BUFFERSZ_PERCHAN_B;
        if ( size > maxsize) {
            printk("[openair][MMAP][ERROR] Trying to map more than %d bytes (req size=%d)\n",
                (unsigned int)(ADAC_BUFFERSZ_PERCHAN_B), (unsigned int)size);
            return -EINVAL;
        }
        phys = p_exmimo_pci_phys[card]->adc_head[ openair_mmap_getAntRX(openair_mmap_ind) ];
    }
    else   
    {
        // mmap a TX buffer
        maxsize = ADAC_BUFFERSZ_PERCHAN_B;
        if ( size > maxsize) {
            printk("[openair][MMAP][ERROR] Trying to map more than %d bytes (%d)\n",
                (unsigned int)(ADAC_BUFFERSZ_PERCHAN_B), (unsigned int)size);
            return -EINVAL;
        }
        phys = p_exmimo_pci_phys[card]->adc_head[ openair_mmap_getAntTX(openair_mmap_ind) ];
    }
    
    printk("[openair][MMAP]  Will mappi phys (%08lx) at %08lx \n", phys, start);
  
    /* loop through all the physical pages in the buffer */ 
    /* Remember this won't work for vmalloc()d memory ! */
    if (remap_pfn_range(vma, 
                      start, 
                      phys>>PAGE_SHIFT, 
                      vma->vm_end-vma->vm_start, 
                      vma->vm_page_prot))
    {
        printk("[openair][MMAP] ERROR EAGAIN\n");
        return -EAGAIN;
    }

    return 0; 
}

//-----------------------------------------------------------------------------
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
int openair_device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) 
#else
int openair_device_ioctl(struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg) 
#endif
{
    /* arg is not meaningful if no arg is passed in user space */
    //-----------------------------------------------------------------------------

    int i;
  
    int tmp;
    
    static unsigned int update_firmware_command;
    static unsigned int update_firmware_address;
    static unsigned int update_firmware_length;
    static unsigned int* update_firmware_kbuffer;
    static unsigned int* __user update_firmware_ubuffer;
    static unsigned int update_firmware_start_address;
    static unsigned int update_firmware_stack_pointer;
    static unsigned int update_firmware_bss_address;
    static unsigned int update_firmware_bss_size;
    unsigned int *fw_block;
    unsigned int sparc_tmp_0;
    unsigned int sparc_tmp_1;
    static unsigned int lendian_length;

    unsigned int get_frame_cnt=0;
    
    unsigned card = 0;
 
    switch(cmd)
    {
    case openair_STOP:
        //----------------------
        printk("[openair][IOCTL]     openair_STOP, NODE_CONFIGURED  (TODO!)\n"); //,openair_daq_vars.node_configured);
        exmimo_send_pccmd(card, EXMIMO_STOP);

        break;
  
    case openair_GET_FRAME:

        get_frame_cnt=0;
        printk("calling exmimo_send_pccmd(0,EXMIMO_GET_FRAME);\n");
        exmimo_send_pccmd(card, EXMIMO_GET_FRAME);
        
        *(exmimo_pci_kvirt[card].adc_head[0]) = 0;
      
        while (get_frame_cnt<20 && !get_frame_done) {
            mdelay(5);
            get_frame_cnt++;
        }
        if (get_frame_cnt==20)
            printk("Get frame error: no IRQ received within 100ms.\n");
        
        get_frame_done = 0;

        break;


    case openair_GET_BIGSHMTOP_KVIRT:

        printk("[IOCTL] card%i:  openair_GET_BIGSHMTOP  *(0x%p) = %p (bigshm_head) \n",card, (void *)arg, bigshm_head[card]);
        copy_to_user((char *)arg,&bigshm_head[card],sizeof(char *));

        break;
        
        
    case openair_GET_PCI_INTERFACE_BOT_KVIRT:
  
        printk("[IOCTL] card%i:  openair_GET_PCI_INTERFACE_BOT_KVIRST: copying exmimo_pci_kvirt(@%8p) to %lx\n", card, &exmimo_pci_kvirt[card],arg);
        copy_to_user((void *)arg,&exmimo_pci_kvirt[card],sizeof(exmimo_pci_interface_bot_virtual_t));
    
        break;

    case openair_DUMP_CONFIG:

        printk("[openair][IOCTL]     openair_DUMP_CONFIG\n");
        
        printk("exmimo_pci_kvirt[%d].exmimo_config_ptr = %p (phys %08x)\n",
            card, exmimo_pci_kvirt[card].exmimo_config_ptr, p_exmimo_pci_phys[card]->exmimo_config_ptr);
            
        /*printk("EXMIMO_CONFIG: freq0 %d Hz, freq1 %d Hz, freqtx0 %d Hz, freqtx1 %d Hz, \nRX gain0 %d dB, RX Gain1 %d dB\n",  
                    exmimo_pci_kvirt[card].exmimo_config_ptr->rf.rf_freq_rx[0],
                    exmimo_pci_kvirt[card].exmimo_config_ptr->rf.rf_freq_rx[1],
                    exmimo_pci_kvirt[card].exmimo_config_ptr->rf.rf_freq_tx[0],
                    exmimo_pci_kvirt[card].exmimo_config_ptr->rf.rf_freq_tx[1],
                    exmimo_pci_kvirt[card].exmimo_config_ptr->rf.rx_gain[0][0],
                    exmimo_pci_kvirt[card].exmimo_config_ptr->rf.rx_gain[1][0]);        
        */
        exmimo_send_pccmd(card, EXMIMO_CONFIG);
    
        break;

    case openair_START_TX_SIG:

        exmimo_send_pccmd(card, EXMIMO_START_RT_ACQUISITION);

        break;


    case openair_UPDATE_FIRMWARE:

        printk("[openair][IOCTL]     openair_UPDATE_FIRMWARE\n");
        /***************************************************
        *   Updating the firmware of Cardbus-MIMO-1 or ExpressMIMO SoC   *
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
    
    
        switch (update_firmware_command)
        {
            case UPDATE_FIRMWARE_TRANSFER_BLOCK:
                update_firmware_address   = ((unsigned int*)arg)[1];
                update_firmware_length    = ((unsigned int*)arg)[2];
      
                update_firmware_ubuffer   = (unsigned int*)((unsigned int*)arg)[3];
                update_firmware_kbuffer = (unsigned int*)kmalloc(update_firmware_length * 4 /* 4 because kmalloc expects bytes */,
                                            GFP_KERNEL);
                if (!update_firmware_kbuffer) {
                    printk("[openair][IOCTL]  Could NOT allocate %u bytes from kernel memory (kmalloc failed).\n", lendian_length * 4);
                    return -1; 
                    break;
                }
                fw_block = (unsigned int *)exmimo_pci_kvirt[card].firmware_block_ptr;
                /* Copy the data block from user space */
                fw_block[0] = update_firmware_address;
                fw_block[1] = update_firmware_length;
                // printk("copy_from_user %p => %p (pci) => %p (ahb) length %d\n",update_firmware_ubuffer,&fw_block[16],update_firmware_address,update_firmware_length);
                tmp = copy_from_user(update_firmware_kbuffer,
                        update_firmware_ubuffer, /* from */
                        update_firmware_length * 4       /* in bytes */
                        );
                // pci_map_single(pdev[0],(void*)fw_block, update_firmware_length*4,PCI_DMA_BIDIRECTIONAL);
                for (i=0;i<update_firmware_length;i++)
                {
                    fw_block[32+i] = ((unsigned int *)update_firmware_kbuffer)[i];
                    // Endian flipping is done in user-space so undo it
                    invert4(fw_block[32+i]);
                }
      
                kfree(update_firmware_kbuffer);
      
                if (tmp) {
                    printk("[openair][IOCTL] Could NOT copy all data from user-space to kernel-space (%d bytes remained uncopied).\n", tmp);
                    return -1;
                    break;
                }
                
                /*
                for (i=0; i<update_firmware_length;i++) {
                    printk("%08x ", fw_block[32+i]);
                    if ((i % 8) == 7)
                        printk("\n");
                }*/
                
                exmimo_send_pccmd(card, EXMIMO_FW_INIT);

                printk("[openair][IOCTL] ok %u words copied at address 0x%08x (fw_block %p)\n",
                    ((unsigned int*)arg)[2],((unsigned int*)arg)[1],fw_block);
                    
            break;

        case UPDATE_FIRMWARE_CLEAR_BSS:

            update_firmware_bss_address   = ((unsigned int*)arg)[1];
            update_firmware_bss_size      = ((unsigned int*)arg)[2];
            sparc_tmp_0 = update_firmware_bss_address;
            sparc_tmp_1 = update_firmware_bss_size;

            printk("[openair][IOCTL] ok asked Leon to clear .bss (addr 0x%08x, size %d bytes)\n", sparc_tmp_0, sparc_tmp_1);
            fw_block = (unsigned int *)exmimo_pci_kvirt[card].firmware_block_ptr;
            /* Copy the data block from user space */
            fw_block[0] = update_firmware_bss_address;
            fw_block[1] = update_firmware_bss_size;

            exmimo_send_pccmd(card, EXMIMO_FW_CLEAR_BSS);

            break;
        
        
        case UPDATE_FIRMWARE_START_EXECUTION:

            update_firmware_start_address = ((unsigned int*)arg)[1];
            update_firmware_stack_pointer = ((unsigned int*)arg)[2];
            sparc_tmp_0 = update_firmware_start_address;
            sparc_tmp_1 = update_firmware_stack_pointer;

            printk("[openair][IOCTL] ok asked Leon to set stack and start execution (addr 0x%08x, stackptr %08x)\n", sparc_tmp_0, sparc_tmp_1);
            fw_block = (unsigned int *)exmimo_pci_kvirt[card].firmware_block_ptr;
            /* Copy the data block from user space */
            fw_block[0] = update_firmware_start_address;
            fw_block[1] = update_firmware_stack_pointer;
      
            exmimo_send_pccmd(card, EXMIMO_FW_START_EXEC);
      
            mdelay(100);
      
            exmimo_firmware_init(card);
            break;
          
          
        case UPDATE_FIRMWARE_FORCE_REBOOT:

            printk("[openair][IOCTL] ok asked Leon to reboot.\n");
            exmimo_send_pccmd(card, EXMIMO_REBOOT);
            mdelay(100);
            exmimo_firmware_init(card);

            break;
      
        case UPDATE_FIRMWARE_TEST_GOK:
            printk("[openair][IOCTL] TEST_GOK command doesn't work with ExpressMIMO!\n");
            break;

        default:
            return -1;
            break;
        }
        
        break;
 
 
    default:
        //----------------------
        printk("[IOCTL] openair_IOCTL unknown: cmd = %i, basecmd = %i\n", cmd, _IOC_NR(cmd) );
        return -EPERM;
        break;
    }
    return 0;
}


