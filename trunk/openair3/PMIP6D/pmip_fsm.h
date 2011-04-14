/*! \file pmip_fsm.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup FINITE_STATE_MACHINE
 * @ingroup PMIP6D
 *  PMIP Finite State Machine (FSM) 
 *  @{
 */

#ifndef __PMIP_FSM_H__
#    define __PMIP_FSM_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_FSM_C
#        define private_pmip_fsm(x) x
#        define protected_pmip_fsm(x) x
#        define public_pmip_fsm(x) x
#    else
#        ifdef PMIP
#            define private_pmip_fsm(x)
#            define protected_pmip_fsm(x) extern x
#            define public_pmip_fsm(x) extern x
#        else
#            define private_pmip_fsm(x)
#            define protected_pmip_fsm(x)
#            define public_pmip_fsm(x) extern x
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
#    include "pmip_msgs.h"
#    include "pmip_mag_proc.h"
#    include "pmip_lma_proc.h"
#    include "pmip_tunnel.h"
#    include "pmip_extern.h"
#    ifdef FSM_DEBUG
#        define dbg(...) dbgprint(__FUNCTION__, __VA_ARGS__)
#    else
#        define dbg(...)
#    endif
//-----------------------------------------------------------------------------
protected_pmip_fsm(int mag_init_fsm(void));
protected_pmip_fsm(int mag_fsm(msg_info_t * info));
protected_pmip_fsm(int lma_fsm(msg_info_t * info));
private_pmip_fsm(pthread_rwlock_t fsm_lock);
#endif
