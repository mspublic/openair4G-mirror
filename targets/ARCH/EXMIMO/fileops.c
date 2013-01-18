/** fileops.c
 * 
 *  Device IOCTL file Operations on character device /dev/openair0
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

//#include "rt_compat.h"

#endif

#include "device.h"
#include "defs.h"
#include "extern.h"

#include "pci.h"

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
  //  printk("[openair][MODULE]  openair_release(), MODE = %d\n",openair_daq_vars.mode);
#ifdef KERNEL2_4
 MOD_DEC_USE_COUNT;
#endif // KERNEL2_4
  return 0;
}
//-----------------------------------------------------------------------------
int openair_device_mmap(struct file *filp, struct vm_area_struct *vma) {
  //-----------------------------------------------------------------------------

#ifdef BIGPHYSAREA  
  unsigned long phys,pos;
  unsigned long start = (unsigned long)vma->vm_start; 
  int i;
#endif
  unsigned long size = (unsigned long)(vma->vm_end-vma->vm_start); 

  
  printk("[openair][MMAP]  called (%lx,%lx,%lx)\n", 
	 vma->vm_start, 
	 vma->vm_end, 
	 size);
  

#ifdef BIGPHYSAREA  
  
  vma->vm_flags |= VM_RESERVED;

  /* if userspace tries to mmap beyond end of our buffer, fail */ 

  if (size>BIGPHYS_NUMPAGES*PAGE_SIZE) {
    printk("[openair][MMAP][ERROR] Trying to map more than %d bytes (%d)\n",
	   (unsigned int)(BIGPHYS_NUMPAGES*PAGE_SIZE),
	   (unsigned int)size);
    return -EINVAL;
  }


  pos = (unsigned long) bigphys_ptr;
  phys = virt_to_phys((void *)pos);
  
  printk("[openair][MMAP]  WILL START MAPPING AT %p (%p) \n", (void*)pos,virt_to_phys(pos));
  
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

  /*
  for (i=0;i<16;i++)
    printk("[openair][MMAP] rxsig %d = %x\n",i,((unsigned int*)RX_DMA_BUFFER[0][0])[i]);
  */

  for (i=0;i<16;i++)
    ((unsigned int*)RX_DMA_BUFFER[0][0])[i] = i;

  for (i=0;i<16;i++)
    ((unsigned int*)TX_DMA_BUFFER[0][0])[i] = i;


#endif //BIGPHYSAREA
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
  //  int ret=-1;
  int i;//,j,aa;
  //  int ue,eNb;
  
  // void *arg_ptr = (void *)arg;

  unsigned char *scale;
  unsigned char scale_mem;
  int tmp;
  unsigned int ltmp;

#define invert4(x)        {ltmp=x; x=((ltmp & 0xff)<<24) | ((ltmp & 0xff00)<<8) | \
				     ((ltmp & 0xff0000)>>8) | ((ltmp & 0xff000000)>>24); }

  //  static unsigned int fmw_off;
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
  //  static unsigned int bendian_fmw_off;
  //  unsigned int ioctl_ack_cnt = 0;

  //  u8 buffer[100];
  //  u8 size;
  unsigned int *fw_block;

  unsigned int get_frame_cnt=0;

  scale = &scale_mem;
  
  printk("[openair][IOCTL] In ioctl(), ioctl = %x (%x,%x)\n",cmd,openair_STOP,openair_GET_BUFFER);
  
  switch(cmd) {
    


  case openair_STOP:
    //----------------------
    //    printk("[openair][IOCTL]     openair_STOP, NODE_CONFIGURED %d\n",openair_daq_vars.node_configured);

    

    //    setup_regs(i);
    openair_send_pccmd(0,EXMIMO_STOP);

    break;
  
  case openair_GET_BUFFER:

    //    openair_daq_vars.get_frame_done = 0;
    //    setup_regs(0,frame_parms);
    get_frame_cnt=0;
    printk("calling openair_send_pccmd(0,EXMIMO_GET_FRAME);\n");
    openair_send_pccmd(0,EXMIMO_GET_FRAME);
      
    while (get_frame_cnt<30) {
      udelay(1000);
      get_frame_cnt++;
    }
    if (get_frame_cnt==30)
      printk("Get frame error\n");

    pci_dma_sync_single_for_cpu(pdev[0], 
				exmimo_pci_interface_ptr[0]->rf.adc_head[0],
				76800*4, 
				PCI_DMA_FROMDEVICE);
    break;
    
    //----------------------

  case openair_GET_BIGPHYSTOP:

    printk("[openair][IOCTL]     openair_GET_BIGPHYSTOP ...(%p)\n",(void *)arg);
#ifdef BIGPHYSAREA
    printk("[openair][IOCTL]     bigphys_ptr = %x\n",bigphys_ptr);
    copy_to_user((char *)arg,&bigphys_ptr,sizeof(char *));
#else

#endif
    break;


  case openair_START_TX_SIG:

    openair_send_pccmd(0,EXMIMO_START_RT_ACQUISITION);
    

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
    
    
    switch (update_firmware_command) {
      
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
      fw_block = (unsigned int *)phys_to_virt(exmimo_pci_bot_ptr[0]->firmware_block_ptr);
      /* Copy the data block from user space */
      fw_block[0] = update_firmware_address;
      fw_block[1] = update_firmware_length;
      //	printk("copy_from_user %p => %p (pci) => %p (ahb) length %d\n",update_firmware_ubuffer,&fw_block[16],update_firmware_address,update_firmware_length);
      tmp = copy_from_user(update_firmware_kbuffer,
			   update_firmware_ubuffer, /* from */
			   update_firmware_length * 4       /* in bytes */
			   );
      //	pci_map_single(pdev[0],(void*)fw_block, update_firmware_length*4,PCI_DMA_BIDIRECTIONAL);
      for (i=0;i<update_firmware_length;i++) {
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
      
      openair_send_pccmd(0,EXMIMO_FW_INIT);
	
      printk("[openair][IOCTL] ok %u words copied at address 0x%08x (fw_block %p)\n",
	     ((unsigned int*)arg)[2],((unsigned int*)arg)[1],fw_block);
      
	
    
      break;

    case UPDATE_FIRMWARE_CLEAR_BSS:

      update_firmware_bss_address   = ((unsigned int*)arg)[1];
      update_firmware_bss_size      = ((unsigned int*)arg)[2];
      sparc_tmp_0 = update_firmware_bss_address;
      sparc_tmp_1 = update_firmware_bss_size;

      printk("[openair][IOCTL] ok asked Leon to clear .bss (addr 0x%08x, size %d bytes)\n", sparc_tmp_0, sparc_tmp_1);
      fw_block = (unsigned int *)phys_to_virt(exmimo_pci_bot_ptr[0]->firmware_block_ptr);
      /* Copy the data block from user space */
      fw_block[0] = update_firmware_bss_address;
      fw_block[1] = update_firmware_bss_size;
      
      openair_send_pccmd(0,EXMIMO_CLEAR_BSS);
	
	
      break;
        
    case UPDATE_FIRMWARE_START_EXECUTION:

      update_firmware_start_address = ((unsigned int*)arg)[1];
      update_firmware_stack_pointer = ((unsigned int*)arg)[2];
      sparc_tmp_0 = update_firmware_start_address;
      sparc_tmp_1 = update_firmware_stack_pointer;

      printk("[openair][IOCTL] ok asked Leon to set stack and start execution (addr 0x%08x, size %d bytes)\n", sparc_tmp_0, sparc_tmp_1);
      fw_block = (unsigned int *)phys_to_virt(exmimo_pci_bot_ptr[0]->firmware_block_ptr);
      /* Copy the data block from user space */
      fw_block[0] = update_firmware_start_address;
      fw_block[1] = update_firmware_stack_pointer;
      
      openair_send_pccmd(0,EXMIMO_START_EXEC);
      
      udelay(1000);
      
      exmimo_firmware_init(0);
      
    break;
          
    case UPDATE_FIRMWARE_FORCE_REBOOT:

      printk("[openair][IOCTL] ok asked Leon to reboot.\n");
      openair_send_pccmd(0,EXMIMO_REBOOT);
      
      break;

    default:
      return -1;
      break;
      
    }
    break;
  
  case openair_GET_PCI_INTERFACE:
    copy_to_user((void *)arg,&exmimo_pci_interface_ptr[0],sizeof(exmimo_pci_interface_t*));
    printk("[IOCTL] copying exmimo_pci_interface=%x to %lx\n", (unsigned int)exmimo_pci_interface_ptr[0],arg);
    
    break;
    

  default:
    //----------------------
    printk("[IOCTL] openair_IOCTL unknown: cmd = %i\n", cmd);
    return -EPERM;
    break;
  }
  return 0;
}


