/*________________________mac_vars.h________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


/*! \file mac_vars.h
* \brief generic mac interface variable definitions
* \author R. Knopp 
* \version 1 
* \date March 2006
*  @ingroup macxface
*/

#ifndef __MAC_VARS_H__
#define __MAC_VARS_H__

#ifdef OPENAIR2
#include "PHY_INTERFACE/defs.h"
#endif //USER_MODE

#include "defs.h"

#ifdef RTAI_ENABLED
#include <rtai.h>
#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif //

unsigned int frame;

MAC_xface *mac_xface;

unsigned int mac_registered;





//#endif

#ifdef OPENAIR2
MACPHY_DATA_REQ_TABLE Macphy_req_table[1];
#endif //OPENAIR2



#endif //__MAC_VARS_H__
