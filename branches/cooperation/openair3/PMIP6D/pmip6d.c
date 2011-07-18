/*! \file pmip6d.c 
* \brief The main PMIP6D file 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
#define PMIP
#define PMIP6D_C
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#include <netinet/icmp6.h>
#include <sys/socket.h>
#include <stdio.h>
#include <linux/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "icmp6.h"
#include <arpa/inet.h>
#include "debug.h"
#include "conf.h"
#include "time.h"
#include "rtnl.h"
#include "tunnelctl.h"
#include "pmip_types.h"
#include "pmip_cache.h"
#include "pmip_extern.h"
#include "pmip_tunnel.h"
#include "pmip_mag_proc.h"
#include "pmip_lma_proc.h"
#include "pmip_pcap.h"
#include "unistd.h"
#ifdef ENABLE_VT
#    include "vt.h"
#endif
#define IPV6_ALL_SOLICITED_MCAST_ADDR 68
typedef __u16 in_port_t;
typedef __u32 in_addr_t;
struct sockaddr_in6 nsaddr;
int flag = 0;
extern struct sock icmp6_sock;
volatile int sighup_received = 0;
volatile int sigterm_received = 0;
volatile int sigint_received = 0;
void sighup_handler(int sig);
void sigterm_handler(int sig);
void sigint_handler(int sig);
void *thr_ciscolog();
void sighup_handler(int sig)
{
/* Linux has "one-shot" signals, reinstall the signal handler */
    signal(SIGHUP, sighup_handler);
    dbg("sighup_handler called");
    sighup_received = 1;
}
void sigterm_handler(int sig)
{
/* Linux has "one-shot" signals, reinstall the signal handler */
    signal(SIGTERM, sigterm_handler);
    dbg("sigterm_handler called");
    sigterm_received = 1;
}
void sigint_handler(int sig)
{
/* Linux has "one-shot" signals, reinstall the signal handler */
    signal(SIGINT, sigint_handler);
    dbg("sigint_handler called");
    sigint_received = 1;
}
void usage(void)
{
    fprintf(stderr, "Usage: pmip6d [-m -L LMA@ -N MAG_IN@ -E MAG_E@] [-c -L LMA@] [-i] [-d]");
    fprintf(stderr,
        "\n\t-m: Run as MAG"
        "\n\t-c: Run as LMA" 
		"\n\t-L: LMA address"
        "\n\t-N: MAG ingress address (towards MN)"
        "\n\t-E: MAG egress address (towards LMA)"
        "\n\t-i: With IPv6-in-IPv6 tunneling"
        "\n\t-d: Dynamically create/delete tunnels"
        "\nExamples: " "\n\tRun as LMA with IP-in-IP: ./pmip6d -c -i -L 2001:100::1" "\n\tRun as MAG with IP-in-IP: ./pmip6d -m -i -L 2001:100::1 -E 2001:100::2 -N 2001:1::1\n");
    exit(0);
}
int get_options(int argc, char *argv[])
{
    int ch;
    if (argc == 1)
    usage();
    while ((ch = getopt(argc, argv, "nodpiscmL:N:E:A:")) != EOF) {
    switch (ch) {
    case 'L':
        if (strchr(optarg, ':')) {
        if (inet_pton(AF_INET6, optarg, (char *) &conf.lma_addr) <= 0) {
            fprintf(stderr, "invalid  address %s\n", optarg);
            exit(2);
        }
        }
        break;
    case 'N':
        if (strchr(optarg, ':')) {
        if (inet_pton(AF_INET6, optarg, (char *) &conf.mag_addr_ingress) <= 0) {
            fprintf(stderr, "invalid  address %s\n", optarg);
            exit(2);
        }
        }
        break;
    case 'E':
        if (strchr(optarg, ':')) {
        if (inet_pton(AF_INET6, optarg, (char *) &conf.mag_addr_egress) <= 0) {
            fprintf(stderr, "invalid  address %s\n", optarg);
            exit(2);
        }
        }
        break;
    case 'm':
        conf.mip6_entity = MIP6_ENTITY_MAG;
        break;
    case 'c':
        conf.mip6_entity = MIP6_ENTITY_LMA;
        break;
    case 'i':
        conf.tunneling_enabled = 1;
        break;
    case 'd':
        conf.dtun_enabled = 1;
        break;
    default:
        usage();
    }
    }
    argc -= optind;
    argv += optind;
//Ignore the rest
    while (argc > 1) {
    argv++;
    argc--;
    }
    return 0;
}

void init_mag_icmp_sock()
{
    int on = 1;
    if (flag) {
    dbg("Set SOLRAW, IPV6_ALL_SOLICTED_MCAST_ADDR = %d\n", IPV6_ALL_SOLICITED_MCAST_ADDR);
    if (setsockopt(icmp6_sock.fd, SOL_RAW, IPV6_ALL_SOLICITED_MCAST_ADDR, &on, sizeof(on)) < 0)
        perror("allow all solicited mcast address\n");
    }
}
static int pmip_cache_delete_each(void *data, void *arg)
{
    pmip_entry_t *bce = (pmip_entry_t *) data;
    if (is_mag()) {
//Delete existing route & rule for the deleted MN
    mag_remove_route(&bce->mn_addr, bce->link);
    int usercount = tunnel_getusers(bce->tunnel);
    dbg("# of binding entries %d \n", usercount);
    if (usercount == 1) {
        route_del(bce->tunnel, RT6_TABLE_PMIP, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, &in6addr_any, 0, NULL);
    }
//decrement users of old tunnel.
    pmip_tunnel_del(bce->tunnel);
    }
//Delete existing route for the deleted MN
    if (is_lma()) {
    lma_remove_route(&bce->mn_addr, bce->tunnel);
//decrement users of old tunnel.
    pmip_tunnel_del(bce->tunnel);
    }
//Delete the Entry.
    free_iov_data((struct iovec *) &bce->mh_vec, bce->iovlen);
    pmip_bce_delete(bce);
    return 0;
}
static void terminate(void)
{
//Release the pmip cache ==> deletes the routes and rules and "default route on PMIP" and tunnels created.
    dbg("Release all occupied resources...\n");
//delete the default rule.
    dbg("Remove default rule...\n");
    rule_del(NULL, RT6_TABLE_MIP6, IP6_RULE_PRIO_MIP6_FWD, RTN_UNICAST, &in6addr_any, 0, &in6addr_any, 0);
    icmp6_cleanup();
    mh_cleanup();
    tunnelctl_cleanup();
//dbg ("Release ro_cache...\n");
//pmip_ro_cache_iterate (pmip_ro_cache_delete_each, NULL);
    dbg("Release pmip_cache...\n");
    pmip_cache_iterate(pmip_cache_delete_each, NULL);
    taskqueue_destroy();
#ifdef ENABLE_VT
    vt_fini();
#endif
//#undef HAVE_PCAP_BREAKLOOP
#define HAVE_PCAP_BREAKLOOP
#ifdef HAVE_PCAP_BREAKLOOP
/*
* We have "pcap_breakloop()"; use it, so that we do as little
* as possible in the signal handler (it's probably not safe
* to do anything with standard I/O streams in a signal handler -
* the ANSI C standard doesn't say it is).
*/
    if (is_mag()) {
        pcap_breakloop(pcap_descr);
    }
#endif
    // LG exit(0);
    pthread_exit(NULL);
}
static void *sigh(void *arg)
{
    int signum;
    sigset_t sigcatch;
    pthread_dbg("thread started");
    sigemptyset(&sigcatch);
    sigaddset(&sigcatch, SIGHUP);
    sigaddset(&sigcatch, SIGINT);
    sigaddset(&sigcatch, SIGTERM);
#ifdef ENABLE_VT
    sigaddset(&sigcatch, SIGPIPE);
#endif
    for (;;) {
    sigwait(&sigcatch, &signum);
    switch (signum) {
    case SIGHUP:
    case SIGINT:
    case SIGTERM:
        terminate();
#ifdef ENABLE_VT
    case SIGPIPE:
#endif
    default:
        break;
    }
    }
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
//Default Values for variables.
    conf.Home_Network_Prefix = in6addr_any;
    conf.mag_addr_ingress = in6addr_loopback;
    conf.mag_addr_egress = in6addr_loopback;
    conf.lma_addr = in6addr_loopback;
    conf.our_addr = in6addr_loopback;
//Lifetime for PB entry
    struct timespec lifetime1;  //15 sec
   //lifetime1.tv_sec = 60;  //60;
    lifetime1.tv_sec = 1000;  //1000;
    lifetime1.tv_nsec = 0;
    conf.PBU_LifeTime = lifetime1;
    struct timespec lifetime2;  //15 sec
    //lifetime2.tv_sec = 30;
    lifetime2.tv_sec = 1000;
    lifetime2.tv_nsec = 0;
    conf.PBA_LifeTime = lifetime2;
//Time for N_Retransmissions
    struct timespec lifetime3;  // 0.5 sec
    lifetime3.tv_sec = 5;
    lifetime3.tv_nsec = 0;
    conf.N_RetsTime = lifetime3;
//Define the maximum # of message retransmissions.
    int Max_rets = 5;
    conf.Max_Rets = Max_rets;
//Get input Configuration parameters.
    get_options(argc, argv);
    pthread_t sigth;
// For DEREG
    pthread_t sigthdereg;

//---------------------------------------------------------------------------
    if (is_mn()) {
    sigset_t oset, nset;
    sigemptyset(&nset);
    sigaddset(&nset, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &nset, &oset);
    if (sigismember(&oset, SIGALRM))
        dbg("SIGALRM has been unblocked. Your startup environment might be wrong.");
    signal(SIGHUP, sighup_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigint_handler);
    } else {
//pthread_t sigth;
    sigset_t sigblock;
    sigemptyset(&sigblock);
    sigaddset(&sigblock, SIGHUP);
    sigaddset(&sigblock, SIGINT);
    sigaddset(&sigblock, SIGTERM);
#ifdef ENABLE_VT
    sigaddset(&sigblock, SIGPIPE);
#endif
    pthread_sigmask(SIG_BLOCK, &sigblock, NULL);
    }
//---------------------------------------------------------------------------
//Probe for the local address
    int probe_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (probe_fd < 0) {
    perror("socket");
    exit(2);
    }
    unsigned int alen;
    struct sockaddr_in6 host;
    struct sockaddr_in6 firsthop;
    firsthop.sin6_port = htons(1025);
    memset(&firsthop, 0, sizeof(firsthop));
    firsthop.sin6_family = AF_INET6;
    if (connect(probe_fd, (struct sockaddr *) &firsthop, sizeof(firsthop))
    == -1) {
    perror("connect");
    exit(2);
    }
    alen = sizeof(host);
    if (getsockname(probe_fd, (struct sockaddr *) &host, &alen) == -1) {
    perror("probe getsockname");
    exit(2);
    }
    close(probe_fd);
/**
* Initializes the VT.
**/
#ifdef ENABLE_VT
    if (vt_init() < 0) {
    dbg("vt initialization failed! \n");
    return -1;
    } else
    dbg("vt is initialized!\n");
    if (vt_start(VT_DEFAULT_HOSTNAME, VT_DEFAULT_SERVICE) < 0) {
    dbg("vt is NOT started! \n");
    return -1;
    }
#endif
/**
* Initializes PMIP cache.
**/
    dbg("Starting....\n");
/**
* Initializes task queue and creates a task runner thread.
**/
    if (taskqueue_init() < 0) {
    dbg("Task Queue Initialization failed....\n");
    return -1;
    } else
    dbg("Task Queue Initialized....\n");
    if (pmip_cache_init() < 0) {
    dbg("PMIP Binding Cache initialization failed! \n");
    return -1;
    } else
    dbg("PMIP Binding Cache is initialized!\n");
/**
* Initializes Tunnelcntl.
**/
    if (tunnelctl_init() < 0) {
    dbg("Tunnelcntl initialization failed! \n");
    return -1;
    } else
    dbg("Tunnelcntl is initialized!\n");
/**
* Adds a default rule for RT6_TABLE_MIP6.
*/
    dbg("Add default rule for RT6_TABLE_MIP6\n");
    if (rule_add(NULL, RT6_TABLE_MIP6, IP6_RULE_PRIO_MIP6_FWD, RTN_UNICAST, &in6addr_any, 0, &in6addr_any, 0) < 0) {
    dbg("Add default rule for RT6_TABLE_MIP6 failed, insufficient privilege/kernel options missing!\n");
    return -1;
    }
// Removed for kernel 2.6.28.10
    if (mh_init() < 0) {
    dbg("Mobility Header initialization failed! \n");
    return -1;
    } else
    dbg("Mobility Header is successfully initialized! \n");
    if (icmp6_init() < 0) {
    dbg("ICMP socket initialization failed! \n");
    return -1;
    } else
    dbg("ICMP socket is successfully initialized! \n");
//Run the MAG Code
    if (is_mag()) {
    conf.our_addr = conf.mag_addr_egress;
    conf.Home_Network_Prefix = get_node_prefix(&conf.mag_addr_ingress); //copy Home network prefix.
    dbg("Running as MAG entity\n");
    dbg("Entity Egress Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.our_addr));
    dbg("Entity Ingress Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.mag_addr_ingress));
    dbg("Home Network Prefix Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.Home_Network_Prefix));
    if (mag_init_fsm() < 0) {
        dbg("Initialization of FSM failed...exit\n");
        exit(-1);
    }
//hip
    init_iface_ra();
    init_mag_icmp_sock();
    dbg("Initializing the NA handler\n");
// to capture NA message
    icmp6_handler_reg(ND_NEIGHBOR_ADVERT, &pmip_mag_recv_na_handler);
// hip
    dbg("Initializing the RS handler\n");
// to capture RS message
    icmp6_handler_reg(ND_ROUTER_SOLICIT, &pmip_mag_rs_handler);
//end
    dbg("Initializing the PBA handler\n");
//To capture PBA message.
    mh_handler_reg(IP6_MH_TYPE_BACK, &pmip_mag_pba_handler);

/**
* Deletes the default route for MN prefix so routing is per unicast MN address!
**/
    route_del((int) NULL, RT6_TABLE_MAIN, IP6_RT_PRIO_ADDRCONF, &in6addr_any, 0, &conf.Home_Network_Prefix, 64, NULL);
    dbg("Initializing the HNP cache\n");
    if (pmip_mn_to_hnp_cache_init() < 0) {
        return -1;
    }
    }
//Run the LMA Code
    else if (is_lma()) {
        dbg("Running as LMA entity\n");
        pmip_lma_mn_to_hnp_cache_init();
//conf.lma_addr = host.sin6_addr;
        conf.our_addr = conf.lma_addr;
        dbg("Entity Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.our_addr));
        dbg("Initializing the PBU handler\n");
//To capture PBU message.
        mh_handler_reg(IP6_MH_TYPE_BU, &pmip_lma_pbu_handler);
    }
    if (!is_mn()) {
        if (pthread_create(&sigth, NULL, sigh, NULL)) {
            dbg("Pthread for tracing Signals is not created!\n");
            return -1;
        } else {
            dbg("Pthread for tracing Signals is created!\n");
        }
#define WLCCP_CAPTURE
#ifdef WLCCP_CAPTURE
        if (is_mag()) {
            char devname[32];
            int iif;
            dbg("Getting ingress informations\n");
            mag_get_ingress_info(&iif, devname);		
            /*dbg("Starting capturing DEREG messages from CISCO log\n");
	    if (pthread_create(&sigthdereg, NULL, thr_ciscolog, NULL)) {
            	dbg("Pthread for tracing DEREG is not created!\n");
          	  return -1;
          	  } else {
           	 dbg("Pthread for tracing DEREG is created!\n");
                   }
	    sleep (1);*/
	    dbg("Starting capturing AP messages for incoming MNs detection\n");
            pmip_pcap_loop(devname, iif);
        }
#endif
	pthread_join(sigth, NULL);
	//pthread_join(sigthdereg, NULL);
    }
    dbg("The Entity is Ready to function...\n");
    return 0;
}
