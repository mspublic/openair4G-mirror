/*! \file pmip_pcap.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup PACKET CAPTURE
 * @ingroup PMIP6D
 *  PMIP PACKet CAPture (PCAP) 
 *  @{
 */

#ifndef __PMIP_PCAP_H__
#    define __PMIP_PCAP_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_PCAP_C
#        define private_pmip_pcap(x) x
#        define protected_pmip_pcap(x) x
#        define public_pmip_pcap(x) x
#    else
#        ifdef PMIP
#            define private_pmip_pcap(x)
#            define protected_pmip_pcap(x) extern x
#            define public_pmip_pcap(x) extern x
#        else
#            define private_pmip_pcap(x)
#            define protected_pmip_pcap(x)
#            define public_pmip_pcap(x) extern x
#        endif
#    endif
//-----------------------------------------------------------------------------
#    include <pcap.h>
#    include <stdlib.h>
#    include <string.h>
#    include "netinet/in.h"
#    include "debug.h"
#    include <pthread.h>
#    include "pmip_types.h"
#    include "pmip_fsm.h"
#    define PCAPMAXBYTES2CAPTURE 1024
#    define PCAPTIMEDELAYKERNEL2USERSPACE 1000
typedef struct wlccp {
    u_char version;
    u_char length;
    u_int16_t message_type;
    u_char dest_hw_address[6];
    u_char src_hw_address[6];
} wlccp_t;
public_pmip_pcap(pcap_t * pcap_descr);
public_pmip_pcap(void pmip_pcap_loop(char *devname, int iif));
private_pmip_pcap(void pmip_pcap_msg_handler(struct in6_addr mn_iidP, int iifP));
#endif

