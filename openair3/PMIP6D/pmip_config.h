/*! \file pmip_config.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup CONFIG
 * @ingroup PMIP6D
 *  RADIUS SERVER PMIP CONFIG FOR THE LMA 
 *  @{
 */

#ifndef __PMIP_CONFIG_H__
#    define __PMIP_CONFIG_H__
// USED BY LMA
// radius is used to grant access to mobiles upon their MAC address.
// If radius is not used, the access control is done by reading allowed
// MAC addresses in a text file called "match".
#    ifdef USE_RADIUS       // defined in top makefile
// defines if the response of the radius server can be cached in memory in the LMA
#        define CACHE_RADIUS
#        define RC_CONFIG_FILE "/usr/local/etc/radiusclient/radiusclient.conf"
#        define RADIUS_PASSWORD "pmiphip"
#    endif
#endif
