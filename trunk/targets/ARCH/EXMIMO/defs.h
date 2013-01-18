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

#include "device.h"

#include "linux/moduleparam.h"

#define MAX_CARDS   4
#define MAX_ANTENNA 4
#define INIT_ZEROS {0, 0, 0, 0}

/*------------------------------------------------*/
/*   Prototypes                                   */
/*------------------------------------------------*/
int openair_device_open    (struct inode *inode,struct file *filp);
int openair_device_release (struct inode *inode,struct file *filp);
int openair_device_mmap    (struct file *filp, struct vm_area_struct *vma);

irqreturn_t openair_irq_handler(int irq, void *cookie);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
int openair_device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg); 
#else
int openair_device_ioctl(struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg); 
#endif

//void openair_get_frame(unsigned char card_id);

int openair_send_pccmd(int card_id, unsigned int cmd);

int exmimo_assign_shm_vars(int card_id);
int exmimo_firmware_init(int card_id);
void exmimo_firmware_cleanup(int card_id);

#endif
#endif
