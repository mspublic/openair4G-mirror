#ifndef __PMIP_INIT_H__
#    define __PMIP_INIT_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_C
#        define private_pmip_init(x) x
#        define protected_pmip_init(x) x
#        define public_pmip_init(x) x
#    else
#        ifdef PMIP
#            define private_pmip_init(x)
#            define protected_pmip_init(x) extern x
#            define public_pmip_init(x) extern x
#        else
#            define private_pmip_init(x)
#            define protected_pmip_init(x)
#            define public_pmip_init(x) extern x
#        endif
#    endif
//-----------------------------------------------------------------------------
//-PROTOTYPES----------------------------------------------------------------------------
/*! \fn void  init_mag_icmp_sock(void)
* \brief Set necessary option on the icmpv6 socket.
* @ingroup  PMIP6D
*/
private_pmip_init(void  init_mag_icmp_sock(void);)
/*! \fn void  pmip_cleanup      (void)
* \brief Release all resources handled by the LMA or MAG entity.
* @ingroup  PMIP6D
*/
public_pmip_init( void  pmip_cleanup      (void);)
/*! \fn int   pmip_common_init  (void)
* \brief Initialization common to LMA and MAGs.
* \return   Status of the initialization, zero if success, else -1.
* @ingroup  PMIP6D
*/
private_pmip_init(int   pmip_common_init  (void);)
/*! \fn int   pmip_mag_init  (void)
* \brief Initialization of the MAG.
* \return   Status of the initialization, zero if success, else -1.
* \note   This function has to be called after pmip_common_init().
* @ingroup  PMIP6D
*/
public_pmip_init( int   pmip_mag_init     (void);)
/*! \fn int   pmip_lma_init  (void)
* \brief Initialization of the LMA.
* \return   Status of the initialization, zero if success, else -1.
* \note   This function has to be called after pmip_common_init().
* @ingroup  PMIP6D
*/
public_pmip_init( int   pmip_lma_init     (void);)
#endif
