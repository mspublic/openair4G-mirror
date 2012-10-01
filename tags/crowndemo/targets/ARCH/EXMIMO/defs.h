#ifndef __DEFS_H__
#define __DEFS_H__
#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

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

#ifdef BIGPHYSAREA
#include <linux/bigphysarea.h>
#endif 

#include "device.h"

#include "linux/moduleparam.h"


/*------------------------------------------------*/
/*   Prototypes                                   */
/*------------------------------------------------*/
int openair_device_open    (struct inode *inode,struct file *filp);
int openair_device_release (struct inode *inode,struct file *filp);
int openair_device_mmap    (struct file *filp, struct vm_area_struct *vma);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
int openair_device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg); 
#else
int openair_device_ioctl(struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg); 
#endif


void openair_get_frame(unsigned char card_id);

int openair_dma(unsigned char card_id, unsigned int cmd);

void exmimo_firmware_init(void);
void exmimo_firmware_cleanup(void);

#endif
#endif
