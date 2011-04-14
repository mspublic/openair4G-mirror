/*! \file pmip_radius.c
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
#define PMIP
#define PMIP_RADIUS_C
#include "pmip_radius.h"

static int mn_count = 0;
static rc_handle *rh;
static char username[256];
static char passwd[AUTH_PASS_LEN + 1];
static char *default_realm;
static char username_realm[256];
/*!
 *  translate a MAC address (6 bytes) into an interface id (ipv6 suffix of 8 bytes)
 *
 * \param  macaddr a MAC address
 * \return the corresponding IPV6 interface id.
 */
struct in6_addr macid2iid(struct in6_addr macaddr)
{
  struct in6_addr iid;
  iid.s6_addr16[0] = 0;
  iid.s6_addr16[1] = 0;
  iid.s6_addr16[2] = 0;
  iid.s6_addr16[3] = 0;
  iid.s6_addr16[4] = 0;
  iid.s6_addr16[5] = macaddr.s6_addr16[4] & ntohs(0x00FF);    // remove 0x02 part of 0x02xx
  iid.s6_addr16[6] = (macaddr.s6_addr16[6] & ntohs(0x00FF)) | (macaddr.s6_addr16[5] & ntohs(0xFF00)); // remove fffe part of xxff.fexx
  iid.s6_addr16[7] = macaddr.s6_addr16[7];
  return iid;
}
