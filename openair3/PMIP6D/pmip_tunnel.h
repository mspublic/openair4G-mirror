/*! \file pmip_tunnel.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup TUNNEL_Management
 * @ingroup PMIP6D
 *  PMIP Tunnel management (Creation/Deletion) 
 *  @{
 */

#ifndef __PMIP_TUNNEL_H__
#    define __PMIP_TUNNEL_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_TUNNEL_C
#        define private_pmip_tunnel(x) x
#        define protected_pmip_tunnel(x) x
#        define public_pmip_tunnel(x) x
#    else
#        ifdef PMIP
#            define private_pmip_tunnel(x)
#            define protected_pmip_tunnel(x) extern x
#            define public_pmip_tunnel(x) extern x
#        else
#            define private_pmip_tunnel(x)
#            define protected_pmip_tunnel(x)
#            define public_pmip_tunnel(x) extern x
#        endif
#    endif
//-----------------------------------------------------------------------------
#    include <netinet/in.h>
#    include <netinet/ip6.h>
#    include "tunnelctl.h"
#    include "debug.h"
#    include "conf.h"
#    include "util.h"
//-----------------------------------------------------------------------------
protected_pmip_tunnel(int pmip_tunnel_add(struct in6_addr *local, struct in6_addr *remote, int link));
protected_pmip_tunnel(int pmip_tunnel_del(int ifindex));
#endif
