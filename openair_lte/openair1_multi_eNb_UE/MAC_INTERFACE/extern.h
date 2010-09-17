/*________________________mac_extern.h________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


/*! \file mac_extern.h 
* \brief Generic mac interface external variable definitions
* \author R. Knopp
* \date March 2006
* \note
* \warning
* @ingroup macxface
*/

#ifndef __MAC_INTERFACE_EXTERN_H__
#define __MAC_INTERFACE_EXTERN_H__

#include "PHY/types.h"
#include "PHY/extern.h"

#include "defs.h"

//#ifndef USER_MODE
#include "SCHED/extern.h"
//#endif

#ifdef RTAI_ENABLED
#include <rtai.h>
#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif //

#ifndef USER_MODE
extern unsigned int fifo_bypass_phy_kern2user; 
extern unsigned int fifo_bypass_phy_user2kern;
extern unsigned int fifo_bypass_phy_kern2user_control; 
extern unsigned int fifo_bypass_mac; 
extern unsigned int fifo_mac_bypass;
#endif //USER_MODE


extern unsigned short  sach_data_rate[16];

extern unsigned int frame;

extern unsigned int mac_registered;

extern MAC_xface *mac_xface;
/*
#ifdef USER_MODE
extern MAC_xface *mac_xface;
#else
#ifndef MAC_CONTEXT
extern MAC_xface *mac_xface;
#endif

#endif
*/




#endif /*__MAC_INTERFACE_EXTERN_H__ */
