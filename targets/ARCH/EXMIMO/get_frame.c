q0
#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#endif

#include "defs.h"
#include "extern.h"
#include "pci.h"



void openair_get_frame(unsigned char card_id) {


  openair_dma(card_id,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_GET_FRAME);
}
