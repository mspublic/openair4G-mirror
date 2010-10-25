/*! \file pmip_lma_proc.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup LMA_Processing
 * @ingroup PMIP6D
 *  PMIP Processing for LMA 
 *  @{
 */

#ifndef __PMIP_LMA_PROC_H__
#    define __PMIP_LMA_PROC_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_LMA_PROC_C
#        define private_pmip_lma_proc(x) x
#        define protected_pmip_lma_proc(x) x
#        define public_pmip_lma_proc(x) x
#    else
#        ifdef PMIP
#            define private_pmip_lma_proc(x)
#            define protected_pmip_lma_proc(x) extern x
#            define public_pmip_lma_proc(x) extern x
#        else
#            define private_pmip_lma_proc(x)
#            define protected_pmip_lma_proc(x)
#            define public_pmip_lma_proc(x) extern x
#        endif
#    endif
//-----------------------------------------------------------------------------
#    include "prefix.h"
#    include "mh.h"
#    include "debug.h"
#    include "conf.h"
#    include "pmip_cache.h"
#    include "pmip_consts.h"
#    include "rtnl.h"
#    include "tunnelctl.h"
#    include <pthread.h>
//#    include "pmip_ro_cache.h"
#    include "pmip_extern.h"
#    include "pmip_tunnel.h"
#    include "pmip_msgs.h"
#    include "arpa/inet.h"
//-----------------------------------------------------------------------------
private_pmip_lma_proc(int lma_setup_route(struct in6_addr *pmip6_addr, int tunnel));
protected_pmip_lma_proc(int lma_remove_route(struct in6_addr *pmip6_addr, int tunnel));
protected_pmip_lma_proc(int lma_reg(pmip_entry_t * bce));
protected_pmip_lma_proc(int lma_dereg(pmip_entry_t * bce, msg_info_t * info, int propagate));
protected_pmip_lma_proc(int lma_update_binding_entry(pmip_entry_t * bce, msg_info_t * info));
#endif
