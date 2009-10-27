#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#ifdef RTAI_ENABLED
#include <rtai.h>
//#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif //RTAI_ENABLED

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

#include "cbmimo1_device.h"

#include "from_grlib_softconfig.h"
#include "from_grlib_softregs.h"
#include "linux/moduleparam.h"


/*------------------------------------------------*/
/*   Prototypes                                   */
/*------------------------------------------------*/
int openair_device_open    (struct inode *inode,struct file *filp);
int openair_device_release (struct inode *inode,struct file *filp);
int openair_device_mmap    (struct file *filp, struct vm_area_struct *vma);
int openair_device_ioctl   (struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg);



void openair_set_rx_rf_mode(unsigned int arg);
void openair_set_tx_gain_openair(unsigned char txgain00,unsigned char txgain10,unsigned char txgain01, unsigned char txgain11);
void openair_set_rx_gain_openair(unsigned char rxgain00,unsigned char rxgain10,unsigned char rxgain01,unsigned char rxgain11);
void openair_set_lo_freq_openair(char freq0,char freq1);
void openair_set_rx_gain_cal_openair(unsigned int gain_dB);

void openair_generate_ofdm(void);
void openair_generate_fs4(unsigned char);

void openair_set_tcxo_dac(unsigned int);

void openair_get_frame(void);

int openair_dma(unsigned int cmd);

int setup_regs(void );


void dump_config(void);

int add_chbch_stats(void);
void remove_chbch_stats(void);
void remove_openair_stats();
int add_openair1_stats();
#endif
