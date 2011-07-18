/*! \file pmip_tunnel.c
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
#define PMIP_TUNNEL_C
#include "pmip_tunnel.h"
//We apply some trick here to advoid create/delete of tunnel too frepquently.
/*!
*  Add a tunnel
* \param local
* \param remote
* \param link
* \return > 0 for success and <= 0 if error
*/
int pmip_tunnel_add(struct in6_addr *local, struct in6_addr *remote, int link)
{
    if (conf.tunneling_enabled) {
    dbg("Creating IP-in-IP tunnel link %d from %x:%x:%x:%x:%x:%x:%x:%x to %x:%x:%x:%x:%x:%x:%x:%x...\n", link, NIP6ADDR(local), NIP6ADDR(remote));
    int tunnel = tunnel_add(local, remote, link, 0, 0); // -1 if error
    return tunnel;
    } else {
    dbg("IP-in-IP tunneling is disabled, no tunnel is created\n");
    return 0;
    }
}

/*!
*  Delete a tunnel
* \param ifindex
* \return 0 for success and -1 if error
*/
int pmip_tunnel_del(int ifindex)
{
    int res = 0;
    if (conf.tunneling_enabled) {
    dbg("Decrease reference number of tunnel %d\n", ifindex);
    if (ifindex > 0) {
        int usercount = tunnel_getusers(ifindex);
        if (usercount > 1 || conf.dtun_enabled)
        res = tunnel_del(ifindex, 0, 0);
        else if (usercount == 1) {
        dbg("Completely delete the tunnel.... \n");
	    res = tunnel_del(ifindex, 0, 0);
//TODO: Put the tunnel  in to the pool
//TODO: Set timer to delete the tunnel after a long stalled period
        }
    } else
        res = -1;
    } else
    dbg("IP-in-IP tunneling is disabled, no tunnel is deleted\n");
    return res;
}
