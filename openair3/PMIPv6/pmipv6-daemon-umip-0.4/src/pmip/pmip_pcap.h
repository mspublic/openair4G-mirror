/*! \file pmip_pcap.h
* \brief
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** \defgroup PACKET_CAPTURE PACKET CAPTURE
 * \ingroup PMIP6D
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
#    include <netinet/ip6.h>
//-----------------------------------------------------------------------------
#    define PCAPMAXBYTES2CAPTURE 65535
#    define PCAPTIMEDELAYKERNEL2USERSPACE 1000
#    define PCAP_CAPTURE_SYSLOG_MESSAGE_FRAME_OFFSET   42
//-PROTOTYPES----------------------------------------------------------------------------
/*! \var pcap_t * pcap_descr
\brief PCAP descriptor for capturing packets on MAG ingress interface.
*/
public_pmip_pcap(pcap_t * pcap_descr);
/*! \fn void pmip_pcap_loop(char *, int )
* \brief
* \param[in]  devname The name of the device (ex "eth1") that will be listened for capturing packets.
* \param[in]  iif     The interface identifier that will be listened for capturing packets.
*/
public_pmip_pcap(void pmip_pcap_loop(char *devname, int iif);)
/*! \fn void pmip_pcap_msg_handler_associate(struct in6_addr , int )
* \brief  Construct amessage event telling that a mobile node is associated with the radio technology of the MAG, and send it to the MAG FSM.
* \param[in]  mn_iidP  The MAC address of the mobile node.
* \param[in]  iifP     The interface identifier that is listened for capturing packets.
*/
private_pmip_pcap(void pmip_pcap_msg_handler_associate(struct in6_addr mn_iidP, int iifP);)
/*! \fn void pmip_pcap_msg_handler_deassociate(struct in6_addr , int )
* \brief  Construct amessage event telling that a mobile node is de-associated with the radio technology of the MAG, and send it to the MAG FSM.
* \param[in]  mn_iidP  The MAC address of the mobile node.
* \param[in]  iifP     The interface identifier that is listened for capturing packets.
*/
private_pmip_pcap(void pmip_pcap_msg_handler_deassociate(struct in6_addr mn_iidP, int iifP);)
#endif
/** @}*/

