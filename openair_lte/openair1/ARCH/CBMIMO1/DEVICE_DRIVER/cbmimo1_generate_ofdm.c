#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#ifdef RTAI_ENABLED
#include <rtai.h>
//#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif


#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/delay.h>

#include <linux/init.h>
#include <linux/module.h>
//#include <linux/malloc.h>
#endif


#include "defs.h"
#include "extern.h"
#include "cbmimo1_pci.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#ifndef OPENAIR_LTE
#include "PHY/TRANSPORT/defs.h"
#endif

#include "from_grlib_softconfig.h"
#include "from_grlib_softregs.h"


void openair_generate_ofdm() {

  unsigned char dummy_mac_pdu[1024];

#ifndef OPENAIR_LTE

  phy_generate_chbch(0,0,NB_ANTENNAS_TX,dummy_mac_pdu);

#endif 

}
