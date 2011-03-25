/*________________________mac_defs.h________________________
  
 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
 ________________________________________________________________*/

#ifndef __MAC_INTERFACE_DEFS_H__
#define __MAC_INTERFACE_DEFS_H__


#include "PHY/types.h"
#include "PHY/defs.h"


/*! \fn int mac_init(void)
* \brief 
* \return 0 on success, otherwise -1 
* @ingroup  tch
*/
int mac_init(void);

/*! \fn void mac_cleanup(void)
*  \brief freeing the allocated memory 
* @ingroup  tch
*/
void mac_cleanup(void);

/*
  \fn void mac_resynch(void)
*  \brief Clean up MAC after resynchronization procedure.  Called by low-level scheduler during resynch.
*/
void mac_resynch(void);

//void l2_init(PHY_VARS_eNB *phy_vars_eNb);

#ifdef OPENAIR2
//#include "LAYER2/MAC/defs.h"
#include "PHY_INTERFACE/defs.h"

/*@}*/

#endif

#endif
