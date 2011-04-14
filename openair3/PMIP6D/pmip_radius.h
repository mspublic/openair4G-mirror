/*! \file pmip_radius.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup RADIUS CLIENT
 * @ingroup PMIP6D
 *  PMIP Radius 
 *  @{
 */

#ifndef __PMIP_RADIUS_H__
#    define __PMIP_RADIUS_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_RADIUS_C
#        define private_pmip_radius(x) x
#        define protected_pmip_radius(x) x
#        define public_pmip_radius(x) x
#    else
#        ifdef PMIP
#            define private_pmip_radius(x)
#            define protected_pmip_radius(x) extern x
#            define public_pmip_radius(x) extern x
#        else
#            define private_pmip_radius(x)
#            define protected_pmip_radius(x)
#            define public_pmip_radius(x) extern x
#        endif
#    endif
//-----------------------------------------------------------------------------
#    include <stdlib.h>
#    include <string.h>
#    include "netinet/in.h"
#    include "debug.h"
#    include <pthread.h>
#    include "pmip_types.h"
#    include "pmip_fsm.h"
public_pmip_radius(pcap_t * pcap_descr);
public_pmip_radius(void pmip_radius_loop(char *devname, int iif));
private_pmip_radius(void pmip_radius_msg_handler(struct in6_addr mn_iidP, int iifP));
#endif
