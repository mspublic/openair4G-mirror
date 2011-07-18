#ifndef __DEVICE_DRIVER_DEFS_H__
#define __DEVICE_DRIVER_DEFS_H__
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

#include "linux/moduleparam.h"


#endif //user_mode

//#include "SIMULATION/PHY_EMULATION/defs.h"
//#include "emul_device.h"
#include "PHY_INTERFACE/defs.h"


#define openair_MAJOR 127



/*------------------------------------------------*/
/*   Prototypes                                   */
/*------------------------------------------------*/

#ifndef USER_MODE
int emul_device_open    (struct inode *inode,struct file *filp);
int emul_device_release (struct inode *inode,struct file *filp);
int emul_device_ioctl   (struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg);
#endif

void dump_config(void);

/*! * @ingroup  _PHY_MAC_INTERFACE_
 * @{
 */



/*! \fn void macphy_scheduler(u8 slot)
*  \brief TTI MAC entry point for TrCH channel scheduling
*  \param last_slot
* @ingroup  tch
*/ 
//void macphy_scheduler(u8);

/*! \fn int emul_init(void)
* \brief 
* \return 0 on success, otherwise -1 
* @ingroup  tch
*/

/*@}*/

int mac_init(void);

/*! \fn void emul_cleanup(void)
*  \brief freeing the allocated memory 
* @ingroup  tch
*/
void mac_cleanup(void);

/*
  \fn void emul_resynch(void)
*  \brief Clean up MAC after resynchronization procedure.  Called by low-level scheduler during resynch.
*/
void mac_resynch(void);






/** @} */

#endif
