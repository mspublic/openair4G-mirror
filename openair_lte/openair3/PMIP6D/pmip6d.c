/*****************************************************************
 * C Implementation: pmip6d.c
 * Description: 
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute,(C) 2008
 ******************************************************************/
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
#include "pmip_ro_cache.h"

#ifdef ENABLE_VT
#include "vt.h"
#endif

#define IPV6_ALL_SOLICITED_MCAST_ADDR 68

typedef __u16 in_port_t;
typedef __u32 in_addr_t;
struct sockaddr_in6 nsaddr;
int flag=0;
extern struct sock icmp6_sock;
char servicename[64];

void usage(void)
{
	fprintf(stderr, "Usage: pmip6d [-s -m -L LMA@ -N MAG_IN@ -E MAG_E@] [-c -L LMA@ [-A All-LMA@]] [-i] [-p] [-d] [-o] [-P port]");
	fprintf(stderr, "\n\t-s: allow capturing NS(DAD)"\
		"\n\t-m: Run as Mobile Router (MAG)"\
		"\n\t-c: Run as Cluster Head (LMA)"\
		"\n\t-L: LMA address"\
		"\n\t-N: MAG ingress address (toward MN)"\
		"\n\t-E: MAG egress address (toward LMA)"
		"\n\t-i: With IPv6-in-IPv6 tunneling"\
		"\n\t-p: Do proxy arp for other MAGs"
		"\n\t-d: Dynamically create/delete tunnels"\
		"\n\t-o: Enable Route Optimization"\
		"\n\t-A: All-LMA Multicast Address"\
		"\n\t-P: Virtual Terminal Port"\ 
	"\nNotes:"\
	"\n\t[-c] and [-m] are exclusive"\
	"\n\t[-s] must always appears with [-m]"\
	"\nExamples: "\
	"\n\tRun as CH with IP-in-IP: ./pmip6d -c -i -L 2001:100::1"\
        "\n\tRun as MR with IP-in-IP: ./pmip6d -m -s -i -L 2001:100::1 -E 2001:100::2 -N 2001:1::1\n"
	);	
	exit(0);
}

int get_options(int argc, char *argv[])
{
	int ch;
	if (argc == 1)  usage();
	while ((ch = getopt(argc, argv, "odpiscmL:N:E:A:P:")) != EOF) {
	  
	switch(ch) {
	case 'L':
		if (strchr(optarg, ':')) {
			if (inet_pton(AF_INET6, optarg, (char*)&conf.lma_addr) <= 0) {
				fprintf(stderr, "invalid  address %s\n", optarg);
				exit(2);
			
			}
		}			
		break;
	case 'N':
		if (strchr(optarg, ':')) {
			if (inet_pton(AF_INET6, optarg, (char*)&conf.mag_addr_ingress) <= 0) {
				fprintf(stderr, "invalid  address %s\n", optarg);
				exit(2);
			
			}
		}			
		break;
	case 'E':
		if (strchr(optarg, ':')) {
			if (inet_pton(AF_INET6, optarg, (char*)&conf.mag_addr_egress) <= 0) {
				fprintf(stderr, "invalid  address %s\n", optarg);
				exit(2);
			
			}
		}			
		break;

	case 'A':
		if (strchr(optarg, ':')) {
			if (inet_pton(AF_INET6, optarg, (char*)&conf.all_lma_addr) <= 0) {
				fprintf(stderr, "invalid  address %s\n", optarg);
				exit(2);
			
			}
		}			
		break;

	case 'P':
		strcpy(servicename, optarg);		
		break;

	case 'm':
		conf.mip6_entity = MIP6_ENTITY_MAG;
		break;
	case 'c':
		conf.mip6_entity = MIP6_ENTITY_LMA;
		break;
	case 's':
		flag=1;
		break;
	case 'i':
		conf.tunneling_enabled=1;
		break;
	case 'p':
		conf.pndisc_enabled=1;
		break;
	case 'd':
		conf.dtun_enabled=1;
		break;
	case 'o':
		conf.ro_enabled=1;
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
	int on=1;
	if (flag)
	{
		dbg("Set SOLRAW, IPV6_ALL_SOLICTED_MCAST_ADDR = %d\n", IPV6_ALL_SOLICITED_MCAST_ADDR);
		if (setsockopt(icmp6_sock.fd, SOL_RAW, IPV6_ALL_SOLICITED_MCAST_ADDR, &on, sizeof(on))<0)
		  perror("allow all solicited mcast address\n");	
	}
}

void testcache()
{
	struct pmip_entry info;
	info.mn_iid.in6_u.u6_addr32[2] = 10;
	pmip_cache_add(&info);
	struct in6_addr addr1= in6addr_any;
	dbg("exists an entry of type: %d\n", pmip_cache_exists(&addr1,&info.mn_iid));


	struct pmip_ro_entry* de = pmip_ro_cache_alloc();	
	inet_pton(AF_INET6, "2001::700", &de->src_addr);
	inet_pton(AF_INET6, "2001::800", &de->dst_addr);
	pmip_ro_cache_add(de);	
	pmip_ro_cache_exists(&de->src_addr, &de->dst_addr);

	de = pmip_ro_cache_alloc();
	inet_pton(AF_INET6, "2001::600", &de->src_addr);
	inet_pton(AF_INET6, "2001::800", &de->dst_addr);
	pmip_ro_cache_add(de);	
	pmip_ro_cache_exists(&de->src_addr, &de->dst_addr);
	pmip_ro_cache_exists(&de->src_addr, &de->dst_addr);
	pmip_ro_cache_exists(&de->src_addr, &de->dst_addr);

	struct in6_addr src, dst;
	inet_pton(AF_INET6, "2001::600", &src);
	inet_pton(AF_INET6, "2001::800", &dst);
	pmip_ro_cache_exists(&src, &dst);

	inet_pton(AF_INET6, "2001::700", &src);
	inet_pton(AF_INET6, "2001::800", &dst);
	pmip_ro_cache_exists(&src, &dst);

	pmip_ro_cache_delete(&src, &dst);
	pmip_ro_cache_exists(&src, &dst);
	dbg("Cache test finished!\n");
	exit(0);
}

static int pmip_cache_delete_each(void *data, void *arg)
{
	struct pmip_entry *bce = (struct pmip_entry *)data;

	if(is_mag())
	{
		//Delete existing route & rule for the deleted MN		
		mag_remove_route( &bce->mn_addr, bce->link);

/*		int usercount = tunnel_getusers(bce->tunnel);
		dbg("# of binding entries %d \n", usercount);
		if (usercount == 1) {*/
			route_del(bce->tunnel, RT6_TABLE_PMIP, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0,&in6addr_any, 0,NULL);
// 		}
		//decrement users of old tunnel.
		pmip_tunnel_del(bce->tunnel);
	}
	//Delete existing route for the deleted MN
	if(is_lma())
	{
		lma_remove_route(&bce->mn_addr, bce->tunnel); 

		//decrement users of old tunnel.
		pmip_tunnel_del(bce->tunnel);
	}

	//Delete the Entry.
	free_iov_data((struct iovec *)&bce->mh_vec,bce->iovlen);
	pmip_bce_delete(bce);
	return 0;	
}


static int pmip_ro_cache_delete_each(void *data, void *arg)
{
	struct pmip_ro_entry *de = (struct pmip_ro_entry *)data;

	if(is_mag())
	{
		//Delete existing route & rule for the deleted ro association		
		if (conf.ro_enabled && conf.tunneling_enabled)
		{
			dbg("Delete route for ro {%x:%x:%x:%x:%x:%x:%x:%x - %x:%x:%x:%x:%x:%x:%x:%x} in table %d\n", NIP6ADDR(&de->src_addr), NIP6ADDR(&de->dst_addr), RT6_TABLE_PMIP);
			route_del(de->tunnel, RT6_TABLE_PMIP, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, &de->dst_addr, 128, NULL);
/*			if (de->tunnel > 0)
			{		
				int usercount = tunnel_getusers(de->tunnel);			
				pmip_tunnel_del(de->tunnel); //decrement users of old tunnel.
				dbg("Last #ro entries %d, decremented #ro entries %d \n", usercount, tunnel_getusers(de->tunnel));
			}
			else dbg("No tunnel need being deleted\n");*/
		}
	}

	if(is_lma())
	{
		//Delete existing route & rule for the deleted ro association		
		if (conf.tunneling_enabled)
		{
			dbg("Delete route for ro {%x:%x:%x:%x:%x:%x:%x:%x - %x:%x:%x:%x:%x:%x:%x:%x} in table %d\n", NIP6ADDR(&de->src_addr), NIP6ADDR(&de->dst_addr), RT6_TABLE_MIP6);
			route_del(de->tunnel, RT6_TABLE_MIP6, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, &de->dst_addr, 128, NULL);			
/*			if (de->tunnel > 0)
			{
				int usercount = tunnel_getusers(de->tunnel);			
				pmip_tunnel_del(de->tunnel); //decrement users of old tunnel.
				dbg("Last #ro entries %d, decremented #ro entries %d \n", usercount, tunnel_getusers(de->tunnel));
			}
			else dbg("No tunnel need being deleted\n"); 			*/
		}

	}

	//Delete the Entry.
	pmip_ro_delete(de);
	return 0;	
}

static void terminate(void)
{
	//Release the pmip cache ==> deletes the routes and rules and "default route on PMIP" and tunnels created.
	dbg("Release all occupied resources...\n");

	icmp6_cleanup();
	mh_cleanup();

	dbg("Release ro_cache...\n");
	pmip_ro_cache_iterate(pmip_ro_cache_delete_each,NULL);

	dbg("Release pmip_cache...\n");
	pmip_cache_iterate(pmip_cache_delete_each,NULL);

	//delete the default rule.
    dbg("Remove default rule referring to table 252 (MIP6 table) ...\n");
    rule_del(NULL,RT6_TABLE_MIP6,IP6_RULE_PRIO_MIP6_FWD, RTN_UNICAST,&in6addr_any, 0, &in6addr_any, 0);

	tunnelctl_cleanup();

	taskqueue_destroy();

#ifdef ENABLE_VT
	vt_fini();
#endif
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

	for ( ;; ) {
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
	pthread_t sigth;
	sigset_t sigblock;

	dbg("Starting Proxy Mobile IPv6 version " PMIP6D_VERSION ", compiled on " __DATE__ " at " __TIME__ " ....\n");


	sigemptyset(&sigblock);
	sigaddset(&sigblock, SIGHUP);
	sigaddset(&sigblock, SIGINT);
	sigaddset(&sigblock, SIGTERM);
#ifdef ENABLE_VT
	sigaddset(&sigblock, SIGPIPE);
#endif
	pthread_sigmask(SIG_BLOCK, &sigblock, NULL);

	//Default Values for variables.
	conf.Home_Network_Prefix = in6addr_any;
	conf.mag_addr_ingress = in6addr_loopback;
	conf.mag_addr_egress = in6addr_loopback;
	conf.lma_addr = in6addr_loopback;
	conf.our_addr = in6addr_loopback;
	conf.all_lma_addr = in6addr_loopback;

	//Lifetime for PB entry
	struct timespec lifetime1; //15 sec
	lifetime1.tv_sec = 60;
	lifetime1.tv_nsec = 0;
	conf.PBU_LifeTime = lifetime1;
	struct timespec lifetime2; //15 sec
	lifetime2.tv_sec = 30;
	lifetime2.tv_nsec = 0;
	conf.PBA_LifeTime = lifetime2;
	//Time for N_Retransmissions
	struct timespec lifetime3; // 0.5 sec
	lifetime3.tv_sec = 5;
	lifetime3.tv_nsec = 0;
	conf.N_RetsTime = lifetime3;
	
	//Define the maximum # of message retransmissions.
	int Max_rets = 5;
	conf.Max_Rets = Max_rets;

	//Get input Configuration parameters.
	conf.vt_hostname = VT_DEFAULT_HOSTNAME;
	strcpy(&servicename, VT_DEFAULT_SERVICE);	
	conf.vt_service = &servicename;
	get_options(argc, argv);

	//Probe for the local address
	int probe_fd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (probe_fd < 0) {
		perror("socket");
		exit(2);
	}
	int alen;
	struct sockaddr_in6 host;	
	struct sockaddr_in6 firsthop;
	firsthop.sin6_port = htons(1025);
	memset(&firsthop, 0, sizeof(firsthop));
	firsthop.sin6_family = AF_INET6;
	if (connect(probe_fd, (struct sockaddr*)&firsthop, sizeof(firsthop)) == -1) {
			perror("connect");
			exit(2);
	}
	alen = sizeof(host);
	if (getsockname(probe_fd, (struct sockaddr*)&host, &alen) == -1) {
		perror("probe getsockname");
		exit(2);
	}
	close(probe_fd);


	/**
 	* Initializes the VT.
 	**/

#ifdef ENABLE_VT

	if (vt_init() < 0)
	{
		dbg("vt initialization failed! \n");
		return -1;
	}
	else dbg("vt is initialized!\n");

 	if (vt_start(conf.vt_hostname, conf.vt_service) < 0)
 	{
 		dbg("vt can not start on port %s \n", conf.vt_service);
 		return -1;
 	}
	else dbg("vt is listening on port %s\n", conf.vt_service);
#endif

	/**
 	* Initializes task queue and creates a task runner thread.
 	**/
	
	if(taskqueue_init() <0)
	{
		dbg("Task Queue Initialization failed....\n");
		return -1;
	}
	else dbg("Task Queue Initialized....\n");

	/**
 	* Initializes PMIP cache.
 	**/
	if (pmip_cache_init() < 0) 
	{
		dbg("PMIP Binding Cache initialization failed! \n");
		return -1;
	}
	else dbg("PMIP Binding Cache is initialized!\n");

	if (pmip_ro_cache_init() < 0) 
	{
		dbg("PMIP RO Cache initialization failed! \n");
		return -1;
	}
	else dbg("PMIP RO Cache is initialized!\n");
	//testcache(); //check the cache.
	
	/**
 	* Initializes Tunnelcntl.
 	**/
	if (tunnelctl_init() < 0) 
	{
		dbg("Tunnelcntl initialization failed! \n");
		return -1;
	}
	else dbg("Tunnelcntl is initialized!\n");

	/**
 	* Adds a default rule for RT6_TABLE_MIP6.
 	**/
	if (rule_add(NULL, RT6_TABLE_MIP6,
		     IP6_RULE_PRIO_MIP6_FWD, RTN_UNICAST,
		     &in6addr_any, 0, &in6addr_any, 0) < 0)
	{
		dbg("Add default rule for RT6_TABLE_MIP6 failed, insufficient privilege/kernel options missing!\n");
		return -1;
	}

	if (mh_init() < 0)
	{
		dbg("Mobility Header initialization failed! \n");
		return -1;
	}
  	else dbg("Mobility Header is sucsessfully initialized! \n");
		
	if (icmp6_init() < 0)
	{
		dbg("ICMP socket initialization failed! \n");
		return -1;
	}
	else dbg("ICMP socket is sucsessfully initialized! \n");

	//Run the MAG Code
	if (is_mag())
	{
		conf.our_addr = conf.mag_addr_egress;
		conf.Home_Network_Prefix = get_node_prefix(&conf.mag_addr_ingress); //copy Home network prefix.

		dbg("Running as MAG entity\n");
		dbg("Entity Egress Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.our_addr));
		dbg("Entity Ingress Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.mag_addr_ingress));
		dbg("Home Network Prefix Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.Home_Network_Prefix));

		
		init_mag_icmp_sock(); 
		
		dbg("Initializing the NS handler\n");

		//To capture NS message
		icmp6_handler_reg(ND_NEIGHBOR_SOLICIT, &pmip_mag_ns_handler);

		dbg("Initializing the NA handler\n");

		// to capture NA message
		icmp6_handler_reg(ND_NEIGHBOR_ADVERT, &pmip_mag_recv_na_handler);

		dbg("Initializing the PBA handler\n");

		//To capture PBU message.
		mh_handler_reg(IP6_MH_TYPE_BU, &pmip_mag_pbu_handler);

		//To capture PBA message.
		mh_handler_reg(IP6_MH_TYPE_BACK, &pmip_mag_pba_handler);

		dbg("Initializing the PBREQ handler\n");

		//To capture PBRR message.
		mh_handler_reg(IP6_MH_TYPE_PBREQ, &pmip_pbreq_handler);

		dbg("Initializing the PBRES handler\n");

		//To capture PBRE message.
		mh_handler_reg(IP6_MH_TYPE_PBRES, &pmip_pbres_handler);


		/**
		* Deletes the default route for MN prefix so routing is per unicast MN address!
		**/
		route_del((int)NULL, RT6_TABLE_MAIN, IP6_RT_PRIO_ADDRCONF,&in6addr_any, 0,&conf.Home_Network_Prefix, 64,NULL);
	}

	//Run the LMA Code
	else if (is_lma())
	{	   
		dbg("Running as LMA entity\n");
		//conf.lma_addr = host.sin6_addr;
		conf.our_addr = conf.lma_addr;
		dbg("Entity Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.our_addr));
	
		dbg("Initializing the PBU handler\n");
		
		//To capture PBU message.
		mh_handler_reg(IP6_MH_TYPE_BU, &pmip_lma_pbu_handler);
		
		dbg("Initializing the PBREQ handler\n");

		//To capture PBRR message.
		mh_handler_reg(IP6_MH_TYPE_PBREQ, &pmip_pbreq_handler);

		dbg("Initializing the PBRES handler\n");

		//To capture PBRE message.
		mh_handler_reg(IP6_MH_TYPE_PBRES, &pmip_pbres_handler);
	 }

	if (pthread_create(&sigth, NULL, sigh, NULL)){
		dbg("Pthread for tracing Signals is not created!\n");
		return -1;
	}
	dbg("The Entity is Ready to function...\n");
	pthread_join(sigth, NULL);
	
	return 0;
}

