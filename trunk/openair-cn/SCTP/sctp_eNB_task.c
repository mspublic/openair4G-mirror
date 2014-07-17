/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/

#include <pthread.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <netinet/in.h>
#include <netinet/sctp.h>

#include <arpa/inet.h>

#include "assertions.h"
#include "queue.h"

#include "intertask_interface.h"

#include "sctp_default_values.h"
#include "sctp_common.h"
#include "sctp_eNB_itti_messaging.h"

/* Used to format an uint32_t containing an ipv4 address */
#define IPV4_ADDR    "%u.%u.%u.%u"
#define IPV4_ADDR_FORMAT(aDDRESS)               \
    (uint8_t)((aDDRESS)  & 0x000000ff),         \
    (uint8_t)(((aDDRESS) & 0x0000ff00) >> 8 ),  \
    (uint8_t)(((aDDRESS) & 0x00ff0000) >> 16),  \
    (uint8_t)(((aDDRESS) & 0xff000000) >> 24)


enum sctp_connection_type_e {
    SCTP_TYPE_CLIENT,
    SCTP_TYPE_SERVER,
    SCTP_TYPE_MAX
};

typedef struct sctp_cnx_list_elm_s {
    STAILQ_ENTRY(sctp_cnx_list_elm_s) entries;

    /* Type of this association
     */
    enum sctp_connection_type_e connection_type;

    int        sd;              ///< Socket descriptor of connection */
    uint16_t   local_port;
    uint16_t   in_streams;      ///< Number of input streams negociated for this connection
    uint16_t   out_streams;     ///< Number of output streams negotiated for this connection
    uint16_t   ppid;            ///< Payload protocol Identifier
    int32_t    assoc_id;        ///< SCTP association id for the connection
    uint32_t   messages_recv;   ///< Number of messages received on this connection
    uint32_t   messages_sent;   ///< Number of messages sent on this connection
    task_id_t  task_id;         ///< Task id of the task who asked for this connection
    instance_t instance;        ///< Instance
    uint16_t   cnx_id;          ///< Upper layer identifier

    struct   sockaddr *peer_addresses;///< A list of peer addresses for server socket
    int      nb_peer_addresses; ///< For server socket
} sctp_cnx_list_elm_t;


static STAILQ_HEAD(sctp_cnx_list_head, sctp_cnx_list_elm_s) sctp_cnx_list;
static uint16_t sctp_nb_cnx = 0;


struct sctp_cnx_list_elm_s *sctp_get_cnx(int32_t assoc_id, int sd)
{
    struct sctp_cnx_list_elm_s *elm;

    STAILQ_FOREACH(elm, &sctp_cnx_list, entries)
    {
        if (assoc_id != -1) {
            if (elm->assoc_id == assoc_id) {
                return elm;
            }
        } else {
            if (elm->sd == sd) {
                return elm;
            }
        }
    }

    return NULL;
}

void sctp_handle_new_association_req(
    const instance_t instance,
    const task_id_t requestor,
    const sctp_new_association_req_t * const sctp_new_association_req_p)
{
    int                           sd;
    int32_t                       assoc_id;

    struct sctp_event_subscribe   events;

    struct sctp_cnx_list_elm_s   *sctp_cnx = NULL;
    enum sctp_connection_type_e   connection_type = SCTP_TYPE_CLIENT;

    struct ifreq                  ifr;
    struct ifaddrs               *ifaddr, *ifa;
    int                           family, s;
    struct in_addr                in;
    struct in6_addr               in6;
    /* Prepare a new SCTP association as requested by upper layer and try to connect
     * to remote host.
     */
    DevAssert(sctp_new_association_req_p != NULL);

    /* Create new socket with IPv6 affinity */
#ifdef NO_VIRTUAL_MACHINE
    // in init chunk appears a list of host addresses, IPv4 and IPv4 in an arbitrary (unsorted) order
    // SCTP hearbeats starts with first ipv4 addresses then stop triyng with other ipv4 addresses
    // if it encounters an IPv6 address in list, so we can force the building of IPv4 addresses only
    // with AF_INET (of cource the working IPv4 address is the last in the list...)
    if ((sd = socket(AF_INET6, SOCK_STREAM, IPPROTO_SCTP)) < 0) {
#else
    if ((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0) {
#endif
        SCTP_ERROR("Socket creation failed: %s\n", strerror(errno));
        return;
    }

    /* Add the socket to list of fd monitored by ITTI */
    itti_subscribe_event_fd(TASK_SCTP, sd);

    if (sctp_set_init_opt(sd, SCTP_IN_STREAMS, SCTP_OUT_STREAMS,
        SCTP_MAX_ATTEMPTS, SCTP_TIMEOUT) != 0)
    {
        SCTP_ERROR("Setsockopt IPPROTO_SCTP_INITMSG failed: %s\n",
                   strerror(errno));
        itti_unsubscribe_event_fd(TASK_SCTP, sd);
        close(sd);
        return;
    }

    /* Subscribe to all events */
    memset((void *)&events, 1, sizeof(struct sctp_event_subscribe));
    if (setsockopt(sd, IPPROTO_SCTP, SCTP_EVENTS, &events,
                   sizeof(struct sctp_event_subscribe)) < 0) {
        SCTP_ERROR("Setsockopt IPPROTO_SCTP_EVENTS failed: %s\n",
                   strerror(errno));
        close(sd);
        return;
    }

    // Bind to device ... or we could bind to address also
    if (getifaddrs(&ifaddr) == -1) {
        SCTP_ERROR("getifaddrs failed: %s\n", strerror(errno));
        close(sd);
    }
    /* Walk through linked list, maintaining head pointer so we
       can free list later */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        /* For an AF_INET* interface address, display the address */
        if (sctp_new_association_req_p->local_address.ipv4 && family == AF_INET) {
            // compare address
            s = inet_aton(sctp_new_association_req_p->local_address.ipv4_address,
                    &in);
            if (s > 0 ) {
                if (((struct sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr == in.s_addr) {
                    memset(&ifr, 0, sizeof(ifr));
                    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s",ifa->ifa_name);
                    if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
                        SCTP_ERROR("Setsockopt SOL_SOCKET failed: %s\n",
                                   strerror(errno));
                    } else {
                        SCTP_DEBUG("Setsockopt SOL_SOCKET socket bound to : %s\n",
                                ifa->ifa_name);
                    }
                    break;
                }
            }
        } else if (sctp_new_association_req_p->local_address.ipv6 && family == AF_INET6) {
            // compare address
            s = inet_pton(AF_INET6,
                    sctp_new_association_req_p->local_address.ipv6_address,
                    &in6);
            if (s == 1 ) {
                if (memcmp(&((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr,
                        &in6, sizeof(in6)) == 0) {
                    memset(&ifr, 0, sizeof(ifr));
                    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifa->ifa_name);
                    if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
                        SCTP_ERROR("Setsockopt SOL_SOCKET failed: %s\n",
                                   strerror(errno));
                    } else {
                        SCTP_DEBUG("Setsockopt SOL_SOCKET socket bound to : %s\n",
                                ifa->ifa_name);
                    }
                    break;
                }
            }
        }
    }

    freeifaddrs(ifaddr);

    /* Mark the socket as non-blocking */
    if (fcntl(sd, F_SETFL, O_NONBLOCK) < 0) {
        SCTP_ERROR("fcntl F_SETFL O_NONBLOCK failed: %s\n",
                   strerror(errno));
        close(sd);
        return;
    }

    /* SOCK_STREAM socket type requires an explicit connect to the remote host
     * address and port.
     * Only use IPv4 for the first connection attempt
     */
    if ((sctp_new_association_req_p->remote_address.ipv6 != 0) ||
        (sctp_new_association_req_p->remote_address.ipv4 != 0))
    {
        uint8_t address_index = 0;
        uint8_t used_address  = sctp_new_association_req_p->remote_address.ipv6 +
                                sctp_new_association_req_p->remote_address.ipv4;
        struct sockaddr_in addr[used_address];

        memset(addr, 0, used_address * sizeof(struct sockaddr_in));

        if (sctp_new_association_req_p->remote_address.ipv6 == 1) {
            if (inet_pton(AF_INET6, sctp_new_association_req_p->remote_address.ipv6_address,
                &addr[address_index].sin_addr.s_addr) != 1)
            {
                SCTP_ERROR("Failed to convert ipv6 address %*s to network type\n",
                           strlen(sctp_new_association_req_p->remote_address.ipv6_address),
                           sctp_new_association_req_p->remote_address.ipv6_address);
                close(sd);
                return;
            }

            SCTP_DEBUG("Converted ipv6 address %*s to network type\n",
                       strlen(sctp_new_association_req_p->remote_address.ipv6_address),
                       sctp_new_association_req_p->remote_address.ipv6_address);

            addr[address_index].sin_family = AF_INET6;
            addr[address_index].sin_port   = htons(sctp_new_association_req_p->port);
            address_index++;
        }
        if (sctp_new_association_req_p->remote_address.ipv4 == 1) {
            if (inet_pton(AF_INET, sctp_new_association_req_p->remote_address.ipv4_address,
                &addr[address_index].sin_addr.s_addr) != 1)
            {
                SCTP_ERROR("Failed to convert ipv4 address %*s to network type\n",
                           strlen(sctp_new_association_req_p->remote_address.ipv4_address),
                           sctp_new_association_req_p->remote_address.ipv4_address);
                close(sd);
                return;
            }

            SCTP_DEBUG("Converted ipv4 address %*s to network type\n",
                       strlen(sctp_new_association_req_p->remote_address.ipv4_address),
                       sctp_new_association_req_p->remote_address.ipv4_address);

            addr[address_index].sin_family = AF_INET;
            addr[address_index].sin_port   = htons(sctp_new_association_req_p->port);
            address_index++;
        }

        /* Connect to remote host and port */
        if (sctp_connectx(sd, (struct sockaddr *)addr, used_address, &assoc_id) < 0)
        {
            /* sctp_connectx on non-blocking socket return EINPROGRESS */
            if (errno != EINPROGRESS) {
                SCTP_ERROR("Connect failed: %s\n", strerror(errno));
                sctp_itti_send_association_resp(
                    requestor, instance, -1, sctp_new_association_req_p->ulp_cnx_id,
                    SCTP_STATE_UNREACHABLE, 0, 0);
                /* Add the socket to list of fd monitored by ITTI */
                itti_unsubscribe_event_fd(TASK_SCTP, sd);
                close(sd);
                return;
            } else {
                SCTP_DEBUG("connectx assoc_id  %d in progress..., used %d addresses\n",
                		assoc_id, used_address);
            }
        } else {
            SCTP_DEBUG("sctp_connectx SUCCESS, used %d addresses assoc_id %d\n",
                       used_address,
                       assoc_id);
        }
    } else {
        /* No remote address provided -> only bind the socket for now.
         * Connection will be accepted in the main event loop
         */
        struct sockaddr_in6 addr6;

        connection_type = SCTP_TYPE_SERVER;

        /* For now bind to any interface */
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr = in6addr_any;
        addr6.sin6_port = htons(sctp_new_association_req_p->port);

        if (bind(sd, (struct sockaddr*)&addr6, sizeof(addr6)) < 0) {
            SCTP_ERROR("Failed to bind the socket %d to address any (v4/v6): %s\n",
                       strerror(errno));
            close(sd);
            return;
        }
    }

    sctp_cnx = calloc(1, sizeof(*sctp_cnx));

    sctp_cnx->connection_type = connection_type;

    sctp_cnx->sd       = sd;
    sctp_cnx->task_id  = requestor;
    sctp_cnx->cnx_id   = sctp_new_association_req_p->ulp_cnx_id;
    sctp_cnx->ppid     = sctp_new_association_req_p->ppid;
    sctp_cnx->instance = instance;
    sctp_cnx->assoc_id = assoc_id;

    /* Insert new element at end of list */
    STAILQ_INSERT_TAIL(&sctp_cnx_list, sctp_cnx, entries);
    sctp_nb_cnx++;

    SCTP_DEBUG("Inserted new descriptor for sd %d in list, nb elements %u, assoc_id %d\n",
                sd, sctp_nb_cnx, assoc_id);
}

void sctp_send_data(
		instance_t       instance,
		task_id_t        task_id,
		sctp_data_req_t *sctp_data_req_p)
{
    struct sctp_cnx_list_elm_s *sctp_cnx = NULL;

    DevAssert(sctp_data_req_p != NULL);

    sctp_cnx = sctp_get_cnx(sctp_data_req_p->assoc_id, 0);

    if (sctp_cnx == NULL) {
        SCTP_ERROR("Failed to find SCTP description for assoc_id %d\n",
                   sctp_data_req_p->assoc_id);
        /* TODO: notify upper layer */
        return;
    }

    if (sctp_data_req_p->stream >= sctp_cnx->out_streams) {
        SCTP_ERROR("Requested stream (%u) >= nb out streams\n",
                   sctp_data_req_p->stream, sctp_cnx->out_streams);
        return;
    }

    /* Send message on specified stream of the sd association
     * NOTE: PPID should be defined in network order
     */
    if (sctp_sendmsg(sctp_cnx->sd, sctp_data_req_p->buffer,
                     sctp_data_req_p->buffer_length, NULL, 0,
                     htonl(sctp_cnx->ppid), 0, sctp_data_req_p->stream, 0, 0) < 0) {
        SCTP_ERROR("Sctp_sendmsg failed: %s\n", strerror(errno));
        /* TODO: notify upper layer */
        return;
    }

    SCTP_DEBUG("Successfully sent %u bytes on stream %d for assoc_id %u\n",
               sctp_data_req_p->buffer_length, sctp_data_req_p->stream,
               sctp_cnx->assoc_id);
}

static int sctp_create_new_listener(
		const instance_t instance,
	    const task_id_t  requestor,
	    sctp_init_t     *init_p)
{
    struct sctp_event_subscribe   event;
    struct sockaddr              *addr      = NULL;
    struct sctp_cnx_list_elm_s   *sctp_cnx  = NULL;
    uint16_t                      i  = 0, j = 0;
    int                           sd = 0;
    int                           used_addresses = 0;

    DevAssert(init_p != NULL);

    if (init_p->ipv4 == 0 && init_p->ipv6 == 0) {
        SCTP_ERROR("Illegal IP configuration upper layer should request at"
                   "least ipv4 and/or ipv6 config\n");
        return -1;
    }

    if ((used_addresses = init_p->nb_ipv4_addr + init_p->nb_ipv6_addr) == 0) {
        SCTP_WARN("No address provided...\n");
        return -1;
    }

    addr = calloc(used_addresses, sizeof(struct sockaddr));

    SCTP_DEBUG("Creating new listen socket on port %u with\n", init_p->port);
    if (init_p->ipv4 == 1) {
        struct sockaddr_in *ip4_addr;

        SCTP_DEBUG("ipv4 addresses:\n");
        for (i = 0; i < init_p->nb_ipv4_addr; i++) {
            SCTP_DEBUG("\t- "IPV4_ADDR"\n",
                       IPV4_ADDR_FORMAT(init_p->ipv4_address[i]));
            ip4_addr = (struct sockaddr_in *)&addr[i];
            ip4_addr->sin_family = AF_INET;
            ip4_addr->sin_port   = htons(init_p->port);
            ip4_addr->sin_addr.s_addr = init_p->ipv4_address[i];
        }
    }
    if (init_p->ipv6 == 1) {
        struct sockaddr_in6 *ip6_addr;

        SCTP_DEBUG("ipv6 addresses:\n");
        for (j = 0; j < init_p->nb_ipv6_addr; j++) {
            SCTP_DEBUG("\t- %s\n", init_p->ipv6_address[j]);
            ip6_addr = (struct sockaddr_in6 *)&addr[i + j];
            ip6_addr->sin6_family = AF_INET6;
            ip6_addr->sin6_port  = htons(init_p->port);
            if (inet_pton(AF_INET6, init_p->ipv6_address[j],
                          ip6_addr->sin6_addr.s6_addr) <= 0) {
                SCTP_WARN("Provided ipv6 address %s is not valid\n",
                             init_p->ipv6_address[j]);
            }
        }
    }

    if ((sd = socket(AF_INET6, SOCK_STREAM, IPPROTO_SCTP)) < 0) {
        SCTP_ERROR("socket: %s:%d\n", strerror(errno), errno);
        return -1;
    }

    memset((void *)&event, 1, sizeof(struct sctp_event_subscribe));
    if (setsockopt(sd, IPPROTO_SCTP, SCTP_EVENTS, &event,
                   sizeof(struct sctp_event_subscribe)) < 0) {
        SCTP_ERROR("setsockopt: %s:%d\n", strerror(errno), errno);
        return -1;
    }

    sctp_cnx = calloc(1, sizeof(*sctp_cnx));

    sctp_cnx->connection_type = SCTP_TYPE_SERVER;
    sctp_cnx->sd              = sd;
    sctp_cnx->local_port      = init_p->port;
    sctp_cnx->in_streams      = 32;
    sctp_cnx->out_streams     = 32;
    sctp_cnx->ppid            = init_p->ppid;
    sctp_cnx->task_id         = requestor;
    sctp_cnx->instance        = instance;

    /* Some pre-bind socket configuration */
    if (sctp_set_init_opt(sd,
    		sctp_cnx->in_streams,
    		sctp_cnx->out_streams,
            0,
            0) < 0)
    {
        goto err;
    }

    if (sctp_bindx(sd, addr, used_addresses, SCTP_BINDX_ADD_ADDR) != 0) {
        SCTP_ERROR("sctp_bindx: %s:%d\n", strerror(errno), errno);
        return -1;
    }
    if (listen(sd, 5) < 0) {
        SCTP_ERROR("listen: %s:%d\n", strerror(errno), errno);
        return -1;
    }

    /* Insert new element at end of list */
    STAILQ_INSERT_TAIL(&sctp_cnx_list, sctp_cnx, entries);
    sctp_nb_cnx++;

    /* Add the socket to list of fd monitored by ITTI */
    itti_subscribe_event_fd(TASK_SCTP, sd);

    return sd;
err:
    if (sd != -1) {
        close(sd);
        sd = -1;
    }
    return -1;
}

static
inline void sctp_eNB_accept_associations(struct sctp_cnx_list_elm_s *sctp_cnx)
{
    int client_sd;

    struct sockaddr saddr;
    socklen_t       saddr_size;

    DevAssert(sctp_cnx != NULL);

    saddr_size = sizeof(saddr);

    /* There is a new client connecting. Accept it...
     */
    if ((client_sd = accept(sctp_cnx->sd, &saddr, &saddr_size)) < 0) {
        SCTP_ERROR("[%d] accept failed: %s:%d\n", sctp_cnx->sd, strerror(errno), errno);
    } else {
        struct sctp_cnx_list_elm_s *new_cnx;
        uint16_t port;

        /* This is an ipv6 socket */
        port = ((struct sockaddr_in6*)&saddr)->sin6_port;

        /* Contrary to BSD, client socket does not inherit O_NONBLOCK option */
        if (fcntl(client_sd, F_SETFL, O_NONBLOCK) < 0) {
            SCTP_ERROR("fcntl F_SETFL O_NONBLOCK failed: %s\n",
                       strerror(errno));
            close(client_sd);
            return;
        }

        new_cnx = calloc(1, sizeof(*sctp_cnx));

        DevAssert(new_cnx != NULL);

        new_cnx->connection_type = SCTP_TYPE_CLIENT;

        new_cnx->sd         = client_sd;
        new_cnx->task_id    = sctp_cnx->task_id;
        new_cnx->cnx_id     = 0;
        new_cnx->ppid       = sctp_cnx->ppid;
        new_cnx->instance   = sctp_cnx->instance;
        new_cnx->local_port = sctp_cnx->local_port;

        if (sctp_get_sockinfo(client_sd, &new_cnx->in_streams, &new_cnx->out_streams,
            &new_cnx->assoc_id) != 0)
        {
            SCTP_ERROR("sctp_get_sockinfo failed\n");
            close(client_sd);
            free(new_cnx);
            return;
        }

        /* Insert new element at end of list */
        STAILQ_INSERT_TAIL(&sctp_cnx_list, new_cnx, entries);
        sctp_nb_cnx++;

        /* Add the socket to list of fd monitored by ITTI */
        itti_subscribe_event_fd(TASK_SCTP, client_sd);

        sctp_itti_send_association_ind(new_cnx->task_id, new_cnx->instance,
                                       new_cnx->assoc_id, port,
                                       new_cnx->out_streams, new_cnx->in_streams);
    }
}

static
inline void sctp_eNB_read_from_socket(struct sctp_cnx_list_elm_s *sctp_cnx)
{
    int flags = 0, n;
    socklen_t from_len;
    struct sctp_sndrcvinfo sinfo;

    struct sockaddr_in addr;
    uint8_t buffer[SCTP_RECV_BUFFER_SIZE];

    DevAssert(sctp_cnx != NULL);

    memset((void *)&addr, 0, sizeof(struct sockaddr_in));
    from_len = (socklen_t)sizeof(struct sockaddr_in);
    memset((void *)&sinfo, 0, sizeof(struct sctp_sndrcvinfo));

    n = sctp_recvmsg(sctp_cnx->sd, (void *)buffer, SCTP_RECV_BUFFER_SIZE,
                     (struct sockaddr *)&addr, &from_len,
                     &sinfo, &flags);
    if (n < 0) {
        if (errno == ENOTCONN) {
            itti_unsubscribe_event_fd(TASK_SCTP, sctp_cnx->sd);

            SCTP_DEBUG("Received not connected for sd %d\n", sctp_cnx->sd);

            sctp_itti_send_association_resp(
                sctp_cnx->task_id, sctp_cnx->instance, -1,
                sctp_cnx->cnx_id, SCTP_STATE_UNREACHABLE, 0, 0);

            close(sctp_cnx->sd);
            STAILQ_REMOVE(&sctp_cnx_list, sctp_cnx, sctp_cnx_list_elm_s, entries);
            sctp_nb_cnx--;
            free(sctp_cnx);
        } else {
            SCTP_DEBUG("An error occured during read\n");
            SCTP_ERROR("sctp_recvmsg: %s:%d\n", strerror(errno), errno);
        }
        return;
    } else if (n == 0) {
        SCTP_DEBUG("return of sctp_recvmsg is 0...\n");
        return;
    }
    if (flags & MSG_NOTIFICATION) {
        union sctp_notification *snp;
        snp = (union sctp_notification *)buffer;

        SCTP_DEBUG("Received notification for sd %d, type %u\n",
                   sctp_cnx->sd, snp->sn_header.sn_type);

        /* Client deconnection */
        if (SCTP_SHUTDOWN_EVENT == snp->sn_header.sn_type) {
            itti_unsubscribe_event_fd(TASK_SCTP, sctp_cnx->sd);

            close(sctp_cnx->sd);

            sctp_itti_send_association_resp(
                sctp_cnx->task_id, sctp_cnx->instance, sctp_cnx->assoc_id,
                sctp_cnx->cnx_id, SCTP_STATE_SHUTDOWN,
                0, 0);

            STAILQ_REMOVE(&sctp_cnx_list, sctp_cnx, sctp_cnx_list_elm_s, entries);
            sctp_nb_cnx--;

            free(sctp_cnx);
        }
        /* Association has changed. */
        else if (SCTP_ASSOC_CHANGE == snp->sn_header.sn_type) {
            struct sctp_assoc_change *sctp_assoc_changed;
            sctp_assoc_changed = &snp->sn_assoc_change;

            SCTP_DEBUG("Client association changed: %d\n", sctp_assoc_changed->sac_state);

            /* New physical association requested by a peer */
            switch (sctp_assoc_changed->sac_state) {
                case SCTP_COMM_UP: {
                    if (sctp_get_peeraddresses(sctp_cnx->sd, NULL, NULL) != 0)
                    {
                        /* TODO Failure -> notify upper layer */
                    } else {
                        sctp_get_sockinfo(sctp_cnx->sd, &sctp_cnx->in_streams,
                                          &sctp_cnx->out_streams, &sctp_cnx->assoc_id);
                    }

                    SCTP_DEBUG("Comm up notified for sd %d, assigned assoc_id %d\n",
                               sctp_cnx->sd, sctp_cnx->assoc_id);

                    sctp_itti_send_association_resp(
                        sctp_cnx->task_id, sctp_cnx->instance, sctp_cnx->assoc_id,
                        sctp_cnx->cnx_id, SCTP_STATE_ESTABLISHED,
                        sctp_cnx->out_streams, sctp_cnx->in_streams);

                } break;
                default:
                    break;
            }
        }
    } else {
        sctp_cnx->messages_recv++;

        if (ntohl(sinfo.sinfo_ppid) != sctp_cnx->ppid) {
            /* Mismatch in Payload Protocol Identifier,
             * may be we received unsollicited traffic from stack other than S1AP.
             */
            SCTP_ERROR("Received data from peer with unsollicited PPID %d, expecting %d\n",
                sinfo.sinfo_ppid, sctp_cnx->ppid);
        }

        SCTP_DEBUG("[%d][%d] Msg of length %d received from port %u, on stream %d, PPID %d\n",
                   sinfo.sinfo_assoc_id, sctp_cnx->sd, n, ntohs(addr.sin_port),
                   sinfo.sinfo_stream, ntohl(sinfo.sinfo_ppid));

        sctp_itti_send_new_message_ind(sctp_cnx->task_id,
                                       sinfo.sinfo_assoc_id,
                                       buffer, n, sinfo.sinfo_stream);
    }
}

void sctp_eNB_flush_sockets(struct epoll_event *events, int nb_events)
{
    int i;
    struct sctp_cnx_list_elm_s *sctp_cnx = NULL;

    if (events == NULL) {
        return;
    }

    for (i = 0; i < nb_events; i++) {
        sctp_cnx = sctp_get_cnx(-1, events[i].data.fd);
        if (sctp_cnx == NULL) {
            continue;
        }
        SCTP_DEBUG("Found data for descriptor %d\n", events[i].data.fd);
        if (sctp_cnx->connection_type == SCTP_TYPE_CLIENT) {
            sctp_eNB_read_from_socket(sctp_cnx);
        } else {
            sctp_eNB_accept_associations(sctp_cnx);
        }
    }
}


void *sctp_eNB_task(void *arg)
{
    int                 nb_events;
    struct epoll_event *events;
    MessageDef         *received_msg = NULL;
    int                 result;

    SCTP_DEBUG("Starting SCTP layer\n");

    STAILQ_INIT(&sctp_cnx_list);

    itti_mark_task_ready(TASK_SCTP);

    while (1) {
        itti_receive_msg(TASK_SCTP, &received_msg);

        /* Check if there is a packet to handle */
        if (received_msg != NULL) {
            switch (ITTI_MSG_ID(received_msg))
            {
                case SCTP_INIT_MSG: {
                    SCTP_DEBUG("Received SCTP_INIT_MSG\n");
                    /* We received a new connection request */
                    if (sctp_create_new_listener(
                    		ITTI_MESSAGE_GET_INSTANCE(received_msg),
                            ITTI_MSG_ORIGIN_ID(received_msg),
                            &received_msg->ittiMsg.sctp_init) < 0) {
                        /* SCTP socket creation or bind failed... */
                        SCTP_ERROR("Failed to create new SCTP listener\n");
                    }
                } break;
                case SCTP_CLOSE_ASSOCIATION:
                	break;

                case TERMINATE_MESSAGE:
                    itti_exit_task();
                    break;

                case SCTP_NEW_ASSOCIATION_REQ: {
                    sctp_handle_new_association_req(ITTI_MESSAGE_GET_INSTANCE(received_msg),
                                                    ITTI_MSG_ORIGIN_ID(received_msg),
                                                    &received_msg->ittiMsg.sctp_new_association_req);
                } break;
                case SCTP_DATA_REQ: {
                    sctp_send_data(ITTI_MESSAGE_GET_INSTANCE(received_msg),
                                   ITTI_MSG_ORIGIN_ID(received_msg),
                                   &received_msg->ittiMsg.sctp_data_req);
                } break;
                default:
                    SCTP_ERROR("Received unhandled message %d:%s\n",
                               ITTI_MSG_ID(received_msg), ITTI_MSG_NAME(received_msg));
                    break;
            }
            result = itti_free(ITTI_MSG_ORIGIN_ID(received_msg), received_msg);
            AssertFatal (result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
            received_msg = NULL;
        }

        nb_events = itti_get_events(TASK_SCTP, &events);
        /* Now handle notifications for other sockets */
        sctp_eNB_flush_sockets(events, nb_events);
    }
    return NULL;
}
