/*! \file pmip_fsm.h
* \brief
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup FINITE_STATE_MACHINE FINITE STATE MACHINE
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
#    include <pthread.h>
#    include "pmip_msgs.h"
//-----------------------------------------------------------------------------
/*! \fn int mag_init_fsm(void)
* \brief Initialization of the Finite state machine of the MAG.
* \return   The status of the initialization, 0 for success, else -1.
* \note  Initialization of the mutex only.
*/
protected_pmip_fsm(int mag_init_fsm(void);)

/*! \fn int mag_fsm(msg_info_t *info)
* \brief Finite state machine of the MAG.
* \param[in]  info All informations about the event received.
* \return   0 for success and -1 if error
*/
protected_pmip_fsm(int mag_fsm(msg_info_t *info);)
/*! \fn int lma_fsm(msg_info_t *info)
* \brief Finite state machine of the LMA.
* \param[in]  info All informations about the event received.
* \return   0 for success and -1 if error
*/
protected_pmip_fsm(int lma_fsm(msg_info_t *info);)
/*! \var pthread_rwlock_t fsm_lock
\brief Global var mutex on the MAG finite state machine.
*/
private_pmip_fsm(pthread_rwlock_t fsm_lock;)
#endif
/** @}*/
