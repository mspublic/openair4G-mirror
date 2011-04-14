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

#else

/*! \brief MACPHY Interface */
typedef struct
  {

    void (*macphy_scheduler)(unsigned char); /*!<\brief Pointer to phy scheduling routine in MAC.  Used by the low-level hardware synchronized scheduler*/
    void (*macphy_setparams)(void *);     /*  Pointer function that reads params for the MAC interface - this function is called when an IOCTL passes parameters to the MAC */
    void (*macphy_init)(void);          /*  Pointer function that reads params for the MAC interface - this function is called when an IOCTL passes parameters to the MAC */
    void (*macphy_exit)(char *);          /*  Pointer function that stops the low-level scheduler due an exit condition */

    unsigned int frame;
    unsigned char is_cluster_head;
    unsigned char is_primary_cluster_head;
    unsigned char is_secondary_cluster_head;
    unsigned char cluster_head_index;
  } MAC_xface;

#endif

#endif
