#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#endif


#include "device.h"
#include "defs.h"
#include "extern.h"
#include "pci.h"

int openair_dma(unsigned char card_id,unsigned int cmd) {


  unsigned int val;

  //    printk("Sending command to ExpressMIMO : %x\n",cmd);
  //write cmd to be executed by
  iowrite32(cmd,(bar[0]+0x04));
  //    printk("Readback of control1 %x\n",ioread32(bar[0]+0x4));
  val = ioread32(bar[0]);
  // set interrupt bit to trigger LEON interrupt
  iowrite32(val|0x1,bar[0]);
  //    printk("Readback of control0 %x\n",ioread32(bar[0]));

  return(0);
}
