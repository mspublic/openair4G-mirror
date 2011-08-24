/*! \file pmip_handler.c
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
#define PMIP
#define PMIP_HANDLER_C
#include "pmip_handler.h"

#ifdef PMIP_HANDLER_DEBUG
#    define dbg(...) dbgprint(__FUNCTION__, __VA_ARGS__)
#else
#    define dbg(...)
#endif
uint16_t seqno_pbreq = 0;
/*!
*
* \param
* \return
*/
struct in6_addr *link_local_addr(struct in6_addr *id)
{
    static struct in6_addr ADDR;
    ADDR = in6addr_any;
    ADDR.s6_addr32[0] = htonl(0xfe800000);
//copy the MN_ID.
    memcpy(&ADDR.s6_addr32[2], &id->s6_addr32[2], sizeof(ip6mnid_t));
    return &ADDR;
}

/*!
*  converts an ID & a prefix into an address.
* \param result returned pointer on address
* \param prefix
* \param id
* \return pointer on address
*/
struct in6_addr *CONVERT_ID2ADDR(struct in6_addr *result, struct in6_addr *prefix, struct in6_addr *id)
{
    *result = in6addr_any;
    memcpy(&result->s6_addr32[0], &prefix->s6_addr32[0], sizeof(ip6mnid_t));
    memcpy(&result->s6_addr32[2], &id->s6_addr32[2], sizeof(ip6mnid_t));
    return result;
}

/*!
*  compute an address from a binding cache entry
* \param bce binding cache entry
* \return address
*/
struct in6_addr *get_mn_addr(pmip_entry_t * bce)
{
    CONVERT_ID2ADDR(&bce->mn_addr, &bce->mn_prefix, &bce->mn_suffix);
    return &bce->mn_addr;
}

/*!
*  converts an ID into a Multicast Address for NS Unreachability
* \param id mobile node id
* \return address
*/
struct in6_addr *solicited_mcast(struct in6_addr *id)
{
//NUD_ADDR converts an ID into a Multicast Address for NS Unreachability!
    static struct in6_addr ADDR2;
    ADDR2 = in6addr_any;
    ADDR2.s6_addr32[0] = htonl(0xff020000);
    ADDR2.s6_addr32[1] = htonl(0x00000000);
    ADDR2.s6_addr32[2] = htonl(0x00000001);
    ADDR2.s6_addr[12] = 0xff;
//copy the least 24 bits from the MN_ID.
    memcpy(&ADDR2.s6_addr[13], &id->s6_addr[13], 3 * sizeof(ADDR2.s6_addr));
    return &ADDR2;
}

/*!
*  Handler triggered by add_task_abs for entry expiry and deletion, retransmit PBU
* \param tqe
*/
void pmip_timer_retrans_pbu_handler(struct tq_elem *tqe)
{
    pthread_rwlock_wrlock(&pmip_lock);
    printf("-------------------------------------\n");
    if (!task_interrupted()) {
    pmip_entry_t *e = tq_data(tqe, pmip_entry_t, tqe);
    pthread_rwlock_wrlock(&e->lock);
    dbg("Retransmissions counter : %d\n", e->n_rets_counter);
    if (e->n_rets_counter == 0) {
        pthread_rwlock_unlock(&e->lock);
        free_iov_data((struct iovec *) &e->mh_vec, e->iovlen);
        dbg("No PBA received from LMA....\n");
        dbg("Abort Trasmitting the PBU....\n");
        pmip_cache_delete(&e->our_addr, &e->mn_hw_address);
        return;
    } else {
//Decrement the N trasnmissions counter.
        e->n_rets_counter--;
        struct in6_addr_bundle addrs;
        addrs.src = &conf.our_addr;
        addrs.dst = &conf.lma_addr;
//sends a PBU
        dbg("Send PBU again....\n");
        pmip_mh_send(&addrs, e->mh_vec, e->iovlen, e->link);
//add a new task for PBU retransmission.
        struct timespec expires;
        clock_gettime(CLOCK_REALTIME, &e->add_time);
        tsadd(e->add_time, conf.N_RetsTime, expires);
        add_task_abs(&expires, &e->tqe, pmip_timer_retrans_pbu_handler);
        dbg("PBU Retransmissions timer is triggered again....\n");
        pthread_rwlock_unlock(&e->lock);
    }
    }
    pthread_rwlock_unlock(&pmip_lock);
}

/*!
*  Handler triggered by add_task_abs for entry expiry and deletion, expire PMIP binding cache entry and NS on MAG
* \param tqe
*/
void pmip_timer_bce_expired_handler(struct tq_elem *tqe)
{
    pthread_rwlock_wrlock(&pmip_lock);
    printf("-------------------------------------\n");
    if (!task_interrupted()) {
    pmip_entry_t *e = tq_data(tqe, pmip_entry_t, tqe);
    pthread_rwlock_wrlock(&e->lock);
    dbg("Retransmissions counter : %d\n", e->n_rets_counter);
    if (e->n_rets_counter == 0) {
        free_iov_data((struct iovec *) &e->mh_vec, e->iovlen);
        if (is_mag()) {
        ++e->seqno_out;
        mag_dereg(e, 1);
        }
//Delete existing route for the deleted MN
        if (is_lma()) {
        lma_dereg(e, 0, 0);
        pmipcache_release_entry(e);
        pmip_bce_delete(e);
        }
        return;
    }
    if (is_mag()) {
        dbg("Send NS for Neighbour Reachability for:%x:%x:%x:%x:%x:%x:%x:%x iif=%d\n", NIP6ADDR(&e->mn_hw_address), e->link);
//Create NS for Reachability test!
        ndisc_send_ns(e->link, &conf.mag_addr_ingress, solicited_mcast(&e->mn_suffix), get_mn_addr(e));
    }
    if (is_lma()) {
		lma_dereg(e, 0, 0);
        pmipcache_release_entry(e);
        pmip_bce_delete(e);
    }
    struct timespec expires;
    clock_gettime(CLOCK_REALTIME, &e->add_time);
    tsadd(e->add_time, conf.N_RetsTime, expires);
// Add a new task for deletion of entry if No Na is received.
    add_task_abs(&expires, &e->tqe, pmip_timer_bce_expired_handler);
    dbg("Start the Timer for Retransmission/Deletion ....\n");
//Decrements the Retransmissions counter.
    e->n_rets_counter--;
    pthread_rwlock_unlock(&e->lock);
    }
    pthread_rwlock_unlock(&pmip_lock);
}

/**
* Handlers defined for MH and ICMP messages.
**/
/*!
* check if address is solicited multicast
* \param addr
* \return value <> 0 if true
*/
static inline int ipv6_addr_is_solicited_mcast(const struct in6_addr *addr)
{
    return (addr->s6_addr32[0] == htonl(0xff020000)
        && addr->s6_addr32[1] == htonl(0x00000000)
        && addr->s6_addr32[2] == htonl(0x00000001)
        && addr->s6_addr[12] == 0xff);
}

/*!
* check if address is multicast
* \param addr
* \return value <> 0 if true
*/
static inline int ipv6_addr_is_multicast(const struct in6_addr *addr)
{
    return (addr->s6_addr32[0] & htonl(0xFF000000)) == htonl(0xFF000000);
}

/*!
* check if address is linklocal
* \param addr
* \return value <> 0 if true
*/
static inline int ipv6_addr_is_linklocal(const struct in6_addr *addr)
{
    return IN6_IS_ADDR_LINKLOCAL(addr);
}


/*!
* handler called when receiving a router solicitation
*/
//hip
static void pmip_mag_recv_rs(const struct icmp6_hdr *ih, ssize_t len, const struct in6_addr *saddr, const struct in6_addr *daddr, int iif, int hoplimit)
{
    dbg("\n");
    dbg("Router Solicitation received \n");
    printf("-------------------------------------\n");
    dbg("Router Solicitation (RS) Received iif %d\n", iif);
    dbg("Received RS Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(saddr));
    dbg("Received RS Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(daddr));
    msg_info_t rs_info;
    bzero(&rs_info, sizeof(rs_info));
    icmp_rs_parse(&rs_info, (struct nd_router_solicit *) ih, saddr, daddr, iif, hoplimit);
    mag_fsm(&rs_info);
}

/*!
* handler called when receiving a proxy binding acknowledgment
*/
static void pmip_mag_recv_pba(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
    printf("=====================================\n");
    dbg("Proxy Binding Acknowledgement (PBA) Received\n");
    dbg("Received PBA Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
    dbg("Received PBA Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));
//define the values for calling the parsing function
//call the parsing function
    struct ip6_mh_binding_ack *pba;
//call the fsm function.
    msg_info_t info;
    pba = (const struct ip6_mh_binding_ack *) ((void *) mh);
    mh_pba_parse(&info, pba, len, in_addrs, iif);
    mag_fsm(&info);
}


/*!
* handler called when receiving a proxy binding update
*/
static void pmip_lma_recv_pbu(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
    printf("=====================================\n");
    dbg("Proxy Binding Update (PBU) Received\n");
    dbg("Received PBU Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
    dbg("Received PBU Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));
//define the values for the parsing function
//call the parsing function
    struct ip6_mh_binding_update *pbu = (struct ip6_mh_binding_update *) mh;
//call the fsm function.
    msg_info_t info;
    bzero(&info, sizeof(info));
    mh_pbu_parse(&info, pbu, len, in_addrs, iif);
    lma_fsm(&info);
}

/*!
* handler called when MAG receive a neighbor advertisement
*/
static void pmip_mag_recv_na(const struct icmp6_hdr *ih, ssize_t len, const struct in6_addr *saddr, const struct in6_addr *daddr, int iif, int hoplimit)
{
// define the MN identifier
//struct in6_addr id = in6addr_any;
    struct nd_neighbor_advert *msg = (struct nd_neighbor_advert *) ih;
//Check target is not link local address.
    if (ipv6_addr_is_linklocal(&msg->nd_na_target)) {
    return;
    }
//Check target is not multicast.
    if (ipv6_addr_is_multicast(&msg->nd_na_target)) {
    return;
    }
    if (len - sizeof(struct nd_neighbor_advert) > 0) {
    printf("-------------------------------------\n");
    dbg("Neighbor Advertisement (NA) Received\n");
    dbg("Received NA Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(saddr));
    dbg("Received NA Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(daddr));
    msg_info_t na_info;
    bzero(&na_info, sizeof(na_info));
    icmp_na_parse(&na_info, (struct nd_neighbor_advert *) ih, saddr, daddr, iif, hoplimit);
    mag_fsm(&na_info);
    }
    return;
}


// hip
struct icmp6_handler pmip_mag_rs_handler = {
    .recv = pmip_mag_recv_rs
};

struct mh_handler pmip_mag_pba_handler = {
    .recv = pmip_mag_recv_pba
};
struct mh_handler pmip_lma_pbu_handler = {
    .recv = pmip_lma_recv_pbu
};
struct icmp6_handler pmip_mag_recv_na_handler = {
    .recv = pmip_mag_recv_na
};
