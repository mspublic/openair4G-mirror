/*! \file pmip_lma_proc.c
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
#define PMIP
#define PMIP_LMA_PROC_C
#include "pmip_lma_proc.h"

/*!
*  set a route by establishing a tunnel
* \param pmip6_addr
* \param tunnel
* \return status of the setup of the route
*/
int lma_setup_route(struct in6_addr *pmip6_addr, int tunnel)
{
    int res = 0;
    if (conf.tunneling_enabled) {
    dbg("Forward: Add new route for %x:%x:%x:%x:%x:%x:%x:%x in table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);
    res = route_add(tunnel, RT6_TABLE_MIP6, RTPROT_MIP, 0, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, pmip6_addr, 128, NULL);
    }
    return res;
}

/*!
*  remove a route established by a tunnel
* \param pmip6_addr
* \param tunnel
* \return status of the removal of the route
*/
int lma_remove_route(struct in6_addr *pmip6_addr, int tunnel)
{
    int res = 0;
    if (conf.tunneling_enabled) {
//Delete existing rule for the deleted MN
    dbg("Delete old route for: %x:%x:%x:%x:%x:%x:%x:%x from table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);
    res = route_del(tunnel, RT6_TABLE_MIP6, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, pmip6_addr, 128, NULL);
    }
    return res;
}

/*!
*  register a bce then create a tunnel between MAG and LMA for this MN && add a route for peer address.
* \param bce
* \return always 0
*/
int lma_reg(pmip_entry_t * bce)
{
//create a tunnel between MAG and LMA && add a route for peer address.
    bce->tunnel = pmip_tunnel_add(&conf.our_addr, &bce->mn_serv_mag_addr, bce->link);
    lma_setup_route(get_mn_addr(bce), bce->tunnel);
    bce->status = 0;        //PBU was Accepted!
//Add task for entry expiry.
    pmip_cache_start(bce);
//Send a PBA to ack new serving MAG
    dbg("Create PBA to new Serving MAG...\n");
    struct in6_addr_bundle addrs;
    addrs.src = &conf.our_addr;
    addrs.dst = &bce->mn_serv_mag_addr;
    mh_send_pba(&addrs, bce, &conf.PBA_LifeTime, 0);
    return 0;
}



/*!
*  deregister a bce
* \param bce       binding cache entry
* \param info
* \param propagate deregistration to MAG
* \return always 0
*/
int lma_dereg(pmip_entry_t * bce, msg_info_t * info, int propagate)
{
    if (propagate) {
		      //Delete the Task
                      del_task(&bce->tqe);
                      //delete old route to old tunnel.
                      lma_remove_route(get_mn_addr(bce), bce->tunnel);
                      //decrement users of old tunnel.
                      pmip_tunnel_del(bce->tunnel);			 
                      dbg("Create PBA for deregistration for MAG (%x:%x:%x:%x:%x:%x:%x:%x)\n", NIP6ADDR(&bce->mn_serv_mag_addr));
                      struct in6_addr_bundle addrs;
                      struct timespec lifetime = { 0, 0 };
                      addrs.src = &conf.lma_addr;
                      addrs.dst = &bce->mn_serv_mag_addr;
                      mh_send_pba(&addrs, bce, &lifetime, 0);
	              bce->type = BCE_NO_ENTRY;
    } else {
	     dbg("Doing nothing....\n");
            }
    return 0;
}



/*!
*  update a binding cache entry with received message informations
* \param bce
* \param info
* \param flagMNHNP
* \return always 0
*/
int lma_update_binding_entry(pmip_entry_t * bce, msg_info_t * info)
{
    int result;
    struct in6_addr r_tmp, r_tmp1;
    memset(&r_tmp1, 0, sizeof(struct in6_addr));
    dbg("Store Binding Entry\n");
    bce->our_addr = conf.our_addr;
    bce->mn_suffix = info->mn_iid;
    bce->mn_hw_address = eth_address2hw_address(info->mn_iid);
    dbg("searching for the prefix for a new BCE entry...\n");
    r_tmp = lma_mnid_hnp_map(bce->mn_hw_address, &result);
    if (result >= 0) {
            if (!(IN6_ARE_ADDR_EQUAL(&r_tmp, &r_tmp1))) {
                bce->mn_prefix = r_tmp;
                dbg("found the prefix  %x:%x:%x:%x:%x:%x:%x:%x in lma_update_binding entry \n", NIP6ADDR(&bce->mn_prefix));
                //delete old route to old tunnel.
                lma_remove_route(get_mn_addr(bce), bce->tunnel);
                //decrement users of old tunnel.
                pmip_tunnel_del(bce->tunnel);
                dbg("Deleting the old tunnel \n");
            } else {
                dbg("special case doing nothing");
            }
    }
    bce->mn_addr = info->mn_addr;
    bce->mn_link_local_addr = info->mn_link_local_addr;
    bce->mn_serv_mag_addr = info->src;
    bce->lifetime = info->lifetime;
    bce->n_rets_counter = conf.Max_Rets;
    bce->seqno_in = info->seqno;
    bce->link = info->iif;
    dbg("Finished updating the binding cache\n");
    return 0;
}
