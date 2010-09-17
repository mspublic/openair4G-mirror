/*________________________mac_init.c________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/

/*!\brief Initilization and reconfiguration routines for generic MAC interface */
#ifndef USER_MODE
#define __NO_VERSION__


#ifdef RTAI_ENABLED
#include <asm/io.h>
#include <asm/rtai.h>
#include <rtai.h>
#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif //


#else  // USER_MODE
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#endif // USER_MODE

#include "PHY/types.h"
#include "PHY/extern.h"
#include "PHY/defs.h"
#include "SCHED/defs.h"
#include "defs.h"
#include "extern.h"

#ifdef OPENAIR2
#include "PHY_INTERFACE/defs.h"
#endif


int mac_init(void)
{



  int i;


  
#ifndef USER_MODE
  // mac_xface->macphy_init();
  mac_xface->macphy_exit = openair_sched_exit;
#else
  mac_xface->macphy_exit=(void (*)(void)) exit;
#endif

  /* this is done in cbmimo1_fileops
#ifdef OPENAIR2
  mac_xface->macphy_init();
#endif //OPENAIR2
  */

  return(1);
}

void mac_cleanup(void)
{

}

#ifdef OPENAIR2
void mac_resynch(void) {

}
#endif //OPENAIR2

#ifdef OPENAIR2
EXPORT_SYMBOL(frame);

#endif //OPENAIR2
