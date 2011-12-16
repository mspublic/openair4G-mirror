/*
 * $Id: conf.c 1.50 06/05/12 11:48:36+03:00 vnuorval@tcs.hut.fi $
 *
 * This file is part of the MIPL Mobile IPv6 for Linux.
 *
 * Authors: Antti Tuominen <anttit@tcs.hut.fi>
 *          Ville Nuorvala <vnuorval@tcs.hut.fi>
 *
 * Copyright 2003-2005 Go-Core Project
 * Copyright 2003-2006 Helsinki University of Technology
 *
 * MIPL Mobile IPv6 for Linux is free software; you can redistribute
 * it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; version 2 of
 * the License.
 *
 * MIPL Mobile IPv6 for Linux is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MIPL Mobile IPv6 for Linux; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.
 */
/*
 * This file is part of the PMIP, Proxy Mobile IPv6 for Linux.
 *
 * Authors: OPENAIR3 <openair_tech@eurecom.fr>
 *
 * Copyright 2010-2011 EURECOM (Sophia-Antipolis, FRANCE)
 * 
 * Proxy Mobile IPv6 (or PMIPv6, or PMIP) is a network-based mobility 
 * management protocol standardized by IETF. It is a protocol for building 
 * a common and access technology independent of mobile core networks, 
 * accommodating various access technologies such as WiMAX, 3GPP, 3GPP2 
 * and WLAN based access architectures. Proxy Mobile IPv6 is the only 
 * network-based mobility management protocol standardized by IETF.
 * 
 * PMIP Proxy Mobile IPv6 for Linux has been built above MIPL free software;
 * which it involves that it is under the same terms of GNU General Public
 * License version 2. See MIPL terms condition if you need more details. 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <netinet/in.h>
#include <netinet/ip6mh.h>
#include <arpa/inet.h>
#include "defpath.h"
#include "conf.h"
#include "debug.h"
#include "util.h"
#include "mipv6.h"
#ifdef ENABLE_VT
#include "vt.h"
#endif

static void conf_usage(char *exec_name)
{
	fprintf(stderr,
		"Usage: %s [options]\nOptions:\n"
		"  -V, --version                Display version information and copyright\n"
		"  -?, -h, --help               Display this help text\n"
		"  -c <file>                    Read configuration from <file>\n"
#ifdef ENABLE_VT
		"      --vt-service <serv>      Set VT service (default=" VT_DEFAULT_SERVICE ")\n"
#endif
		"\n These options override values read from config file:\n"
		"  -d <number>                  Set debug level (0-10)\n"
		"  -l <file>                    Write debug log to <file> instead of stderr\n"
		"  -C, --correspondent-node     Node is CN\n"
		"  -H, --home-agent             Node is HA\n"
        "  -M, --mobile-node            Node is MN\n\n"
        "  -m, --mobile-access-gateway  Node is MAG (Proxy MIP architecture)\n"
        "  -a, --local-mobility-anchor  Node is LMA (Proxy MIP architecture)\n"
        "  -i, --pmip-tunneling         With IPv6-in-IPv6 tunneling      (Proxy MIP architecture)\n"
        "  -p, --pmip-dyn-tunneling     Dynamicaly create/delete tunnels (Proxy MIP architecture)\n"
        "  -L, --lma-address            LMA address exposed to MAGs\n"
        "  -N, --mag-ingress-address    MAG ingress address (towards MNs)\n"
        "  -E, --mag-egress-address     MAG egress address  (towards LMA)\n"
        "  -R, --radius-client-cfg-file RADIUS client config file location\n"
        "  -P, --radius-password        RADIUS passord for AAA server\n"
                "For bug reporting, see %s.\n", exec_name, PACKAGE_BUGREPORT);
}

static void conf_version(void)
{
	fprintf(stderr,
		"%s (%s) %s\n"
		"%s\n"
		"This is free software; see the source for copying conditions.  There is NO\n"
		"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
		PACKAGE, PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_COPYRIGHT);
}

static int conf_alt_file(char *filename, int argc, char **argv)
{
	int args_left = argc;
	char **cur_arg = argv;

	while (args_left--) {
		if (strcmp(*cur_arg, "-c") == 0 && args_left > 0) {
			cur_arg++;
			if (**cur_arg == '-')
				return -EINVAL;
			if (strlen(*cur_arg) >= MAXPATHLEN)
				return -ENAMETOOLONG;
			strcpy(filename, *cur_arg);
			return 0;
		}
		cur_arg++;
	}

	return 1;
}

static int conf_file(struct mip6_config *c, char *filename)
{
	extern FILE *yyin;
	int ret;

	yyin = fopen(filename, "r");
	if (yyin == NULL)
		return -ENOENT;

	c->config_file = malloc(strlen(filename) + 1);
	if (c->config_file == NULL)
		return -ENOMEM;
	strcpy(c->config_file, filename);

	ret = yyparse();

	fclose(yyin);
	if (ret) return -EINVAL;

	return 0;
}

static int conf_cmdline(struct mip6_config *cfg, int argc, char **argv)
{
	static struct option long_opts[] = {
		{"version", 0, 0, 'V'},
		{"help", 0, 0, 'h'},
		{"correspondent-node", 0, 0, 'C'},
		{"home-agent", 0, 0, 'H'},
		{"mobile-node", 0, 0, 'M'},
		{"mobile-access-gateway", optional_argument, 0, 'm'},
		{"local-mobility-anchor", optional_argument, 0, 'a'},
		{"pmip-tunneling", optional_argument, 0, 'i'},
		{"pmip-dyn-tunneling", optional_argument, 0, 'd'},
		{"lma-address", optional_argument, 0, 'L'},
		{"mag-ingress-address", optional_argument, 0, 'N'},
		{"mag-egress-address", optional_argument, 0, 'E'},
        {"show-config", 0, 0, 0},
        {"radius-client-cfg-file", optional_argument, 0, 'R'},
        {"radius-password", 0, 0, 'P'},
#ifdef ENABLE_VT
		{"vt-service", 1, 0, 0 },
#endif

		{0, 0, 0, 0}
	};

	/* parse all other cmd line parameters than -c */
	while (1) {
		int idx, c;
		c = getopt_long(argc, argv, "c:d:l:L:N:E:R:P:Vh?CMHmaip", long_opts, &idx);
		if (c == -1) break;

		switch (c) {
		case 0:
#ifdef ENABLE_VT
			if (strcmp(long_opts[idx].name, "vt-service") == 0) {
				cfg->vt_service = optarg;
				break;
			}
#endif
			if (idx == 5)
				conf_show(cfg);
			return -1;
		case 'V':
			conf_version();
			return -1;
		case '?':
		case 'h':
			conf_usage(basename(argv[0]));
			return -1;
		case 'd':
			cfg->debug_level = atoi(optarg);
			break;
		case 'l':
			cfg->debug_log_file = optarg;
			break;
		case 'C':
			cfg->mip6_entity = MIP6_ENTITY_CN;
			break;
		case 'H':
			cfg->mip6_entity = MIP6_ENTITY_HA;
			break;
		case 'M':
			cfg->mip6_entity = MIP6_ENTITY_MN;
			break;
        case 'L':
            if (strchr(optarg, ':')) {
                if (inet_pton(AF_INET6, optarg, (char *) &cfg->LmaAddress) <= 0) {
                    fprintf(stderr, "invalid  address %s\n", optarg);
                    exit(2);
                }
            }
            break;
        case 'N':
            if (strchr(optarg, ':')) {
                if (inet_pton(AF_INET6, optarg, (char *) &cfg->MagAddressIngress) <= 0) {
                    fprintf(stderr, "invalid  address %s\n", optarg);
                    exit(2);
                }
            }
            break;
        case 'E':
            if (strchr(optarg, ':')) {
                if (inet_pton(AF_INET6, optarg, (char *) &cfg->MagAddressEgress) <= 0) {
                    fprintf(stderr, "invalid  address %s\n", optarg);
                    exit(2);
                }
            }
            break;
        case 'm':
            cfg->mip6_entity = MIP6_ENTITY_MAG;
            break;
        case 'a':
            cfg->mip6_entity = MIP6_ENTITY_LMA;
            break;
        case 'i':
            cfg->TunnelingEnabled = 1;
            break;
        case 'p':
            cfg->DynamicTunnelingEnabled = 1;
            break;
        case 'R':
            cfg->RadiusClientConfigFile = optarg;
            break;
        case 'P':
            cfg->RadiusPassword = optarg;
            break;
		default:
			break;
		};
	}
	return 0;
}

/*
static void conf_default(struct mip6_config *c)
{
    memset(c, 0, sizeof(*c));

    // Common options
#ifdef ENABLE_VT
    c->vt_hostname = VT_DEFAULT_HOSTNAME;
    c->vt_service = VT_DEFAULT_SERVICE;
#endif
    c->mip6_entity = MIP6_ENTITY_CN;
    pmgr_init(NULL, &conf.pmgr);
    INIT_LIST_HEAD(&c->net_ifaces);
    INIT_LIST_HEAD(&c->bind_acl);
    c->DefaultBindingAclPolicy = IP6_MH_BAS_ACCEPTED;

    // IPsec options
    c->UseMnHaIPsec = 1;
    INIT_LIST_HEAD(&c->ipsec_policies);

    // MN options
    c->MnMaxHaBindingLife = MAX_BINDING_LIFETIME;
    c->MnMaxCnBindingLife = MAX_RR_BINDING_LIFETIME;
    tssetdsec(c->InitialBindackTimeoutFirstReg_ts, 1.5);//seconds
    tssetsec(c->InitialBindackTimeoutReReg_ts, INITIAL_BINDACK_TIMEOUT);//seconds
    INIT_LIST_HEAD(&c->home_addrs);
    c->MoveModulePath = NULL; // internal
    c->DoRouteOptimizationMN = 1;
    c->SendMobPfxSols = 1;
    c->OptimisticHandoff = 0;

    // HA options
    c->SendMobPfxAdvs = 1;
    c->SendUnsolMobPfxAdvs = 1;
    c->MaxMobPfxAdvInterval = 86400; // seconds
    c->MinMobPfxAdvInterval = 600; // seconds
    c->HaMaxBindingLife = MAX_BINDING_LIFETIME;

    // CN bindings
    c->DoRouteOptimizationCN = 1;
}
*/
static void conf_default(struct mip6_config *c)
{
    memset(c, 0, sizeof(*c));

    // Common options
#ifdef ENABLE_VT
    c->vt_hostname = VT_DEFAULT_HOSTNAME;
    c->vt_service = VT_DEFAULT_SERVICE;
#endif
    c->mip6_entity = MIP6_ENTITY_CN;
    pmgr_init(NULL, &conf.pmgr);
    INIT_LIST_HEAD(&c->net_ifaces);
    INIT_LIST_HEAD(&c->bind_acl);
    c->DefaultBindingAclPolicy = IP6_MH_BAS_ACCEPTED;

    // IPsec options
    c->UseMnHaIPsec = 0;
    INIT_LIST_HEAD(&c->ipsec_policies);

    // MN options
    c->MnMaxHaBindingLife = MAX_BINDING_LIFETIME;
    c->MnMaxCnBindingLife = MAX_RR_BINDING_LIFETIME;
    tssetdsec(c->InitialBindackTimeoutFirstReg_ts, 1.5);//seconds
    tssetsec(c->InitialBindackTimeoutReReg_ts, INITIAL_BINDACK_TIMEOUT);//seconds
    INIT_LIST_HEAD(&c->home_addrs);
    c->MoveModulePath = NULL; // internal
    c->DoRouteOptimizationMN = 1;
    c->SendMobPfxSols = 1;
    c->OptimisticHandoff = 0;

    // HA options
    c->SendMobPfxAdvs = 1;
    c->SendUnsolMobPfxAdvs = 1;
    c->MaxMobPfxAdvInterval = 86400; // seconds
    c->MinMobPfxAdvInterval = 600; // seconds
    c->HaMaxBindingLife = MAX_BINDING_LIFETIME;

    // CN bindings
    c->DoRouteOptimizationCN = 1;

    //Default Values for variables.
    c->HomeNetworkPrefix = in6addr_any;
    c->MagAddressIngress = in6addr_loopback;
    c->MagAddressEgress  = in6addr_loopback;
    c->LmaAddress        = in6addr_loopback;
    c->OurAddress        = in6addr_loopback;
    //Lifetime for PB entry
    struct timespec lifetime1;  //15 sec
    //lifetime1.tv_sec = 60;
    lifetime1.tv_sec = 1000;
    lifetime1.tv_nsec = 0;
    c->PBULifeTime = lifetime1;
    struct timespec lifetime2;  //15 sec
    //lifetime2.tv_sec = 30;
    lifetime2.tv_sec = 1000;
    lifetime2.tv_nsec = 0;
    c->PBALifeTime = lifetime2;
    //Time for N_Retransmissions
    struct timespec lifetime3;  // 0.5 sec
    lifetime3.tv_sec = 5;
    lifetime3.tv_nsec = 0;
    c->NRetransmissionTime = lifetime3;
    //Define the maximum # of message retransmissions.
    int Max_rets = 5;
    c->MaxMessageRetransmissions = Max_rets;
    c->TunnelingEnabled          = 0;
    c->DynamicTunnelingEnabled   = 0;
    c->RadiusClientConfigFile    = "";
    c->RadiusPassword            = "";

}


int conf_parse(struct mip6_config *c, int argc, char **argv)
{
	char cfile[MAXPATHLEN];
	int ret;

	/* set config defaults */
	conf_default(c);

	if ((ret = conf_alt_file(cfile, argc, argv)) != 0) {
		if (ret == -EINVAL) {
			fprintf(stderr,
				"%s: option requires an argument -- c\n",
				argv[0]);
			conf_usage(basename(argv[0]));
			return -1;
		} else if (ret == -ENAMETOOLONG) {
			fprintf(stderr,
				"%s: argument too long -- c <file>\n",
				argv[0]);
			return -1;
		}
		strcpy(cfile, DEFAULT_CONFIG_FILE);
	}

	if (conf_file(c, cfile) < 0 && ret == 0)
		return -1;

	if (conf_cmdline(c, argc, argv) < 0)
		return -1;

	return 0;
}

#define CONF_BOOL_STR(x) ((x) ? "enabled" : "disabled")

void conf_show(struct mip6_config *c)
{
	/* Common options */
	dbg("config_file = %s\n", c->config_file);
#ifdef ENABLE_VT
	dbg("vt_hostname = %s\n", c->vt_hostname);
	dbg("vt_service = %s\n", c->vt_service);
#endif
	dbg("mip6_entity = %u\n", c->mip6_entity);
	dbg("debug_level = %u\n", c->debug_level);
	dbg("debug_log_file = %s\n", (c->debug_log_file ? c->debug_log_file :
				      "stderr"));
	if (c->pmgr.so_path)
		dbg("PolicyModulePath = %s\n", c->pmgr.so_path);
	dbg("DefaultBindingAclPolicy = %u\n", c->DefaultBindingAclPolicy);
	dbg("NonVolatileBindingCache = %s\n",
	    CONF_BOOL_STR(c->NonVolatileBindingCache));

	/* IPsec options */
	dbg("KeyMngMobCapability = %s\n",
	    CONF_BOOL_STR(c->KeyMngMobCapability));
	dbg("UseMnHaIPsec = %s\n", CONF_BOOL_STR(c->UseMnHaIPsec));

	/* MN options */
	dbg("MnMaxHaBindingLife = %u\n", c->MnMaxHaBindingLife);
	dbg("MnMaxCnBindingLife = %u\n", c->MnMaxCnBindingLife);
	dbg("MnRouterProbes = %u\n", c->MnRouterProbes);
	dbg("MnRouterProbeTimeout = %f\n",
	    tstodsec(c->MnRouterProbeTimeout_ts));
	dbg("InitialBindackTimeoutFirstReg = %f\n",
	    tstodsec(c->InitialBindackTimeoutFirstReg_ts));
	dbg("InitialBindackTimeoutReReg = %f\n",
	    tstodsec(c->InitialBindackTimeoutReReg_ts));
	if (c->MoveModulePath)
		dbg("MoveModulePath = %s\n", c->MoveModulePath);
	dbg("UseCnBuAck = %s\n", CONF_BOOL_STR(c->CnBuAck));
	dbg("DoRouteOptimizationMN = %s\n",
	    CONF_BOOL_STR(c->DoRouteOptimizationMN));
	dbg("MnUseAllInterfaces = %s\n", CONF_BOOL_STR(c->MnUseAllInterfaces));
	dbg("MnDiscardHaParamProb = %s\n",
	    CONF_BOOL_STR(c->MnDiscardHaParamProb));
	dbg("SendMobPfxSols = %s\n", CONF_BOOL_STR(c->SendMobPfxSols));
	dbg("OptimisticHandoff = %s\n", CONF_BOOL_STR(c->OptimisticHandoff));

	/* HA options */
	dbg("SendMobPfxAdvs = %s\n", CONF_BOOL_STR(c->SendMobPfxAdvs));
	dbg("SendUnsolMobPfxAdvs = %s\n",
	    CONF_BOOL_STR(c->SendUnsolMobPfxAdvs));
	dbg("MaxMobPfxAdvInterval = %u\n", c->MaxMobPfxAdvInterval);
	dbg("MinMobPfxAdvInterval = %u\n", c->MinMobPfxAdvInterval);
	dbg("HaMaxBindingLife = %u\n", c->HaMaxBindingLife);

    /* CN options */
    dbg("DoRouteOptimizationCN = %s\n",
        CONF_BOOL_STR(c->DoRouteOptimizationCN));


    /* PMIP options */
    dbg("AllLmaMulticastAddress    = %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&c->AllLmaMulticastAddress));
    dbg("LmaAddress                = %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&c->LmaAddress));
    if (is_mag()) {
    dbg("MagAddressIngress         = %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&c->MagAddressIngress));
    dbg("MagAddressEgress          = %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&c->MagAddressEgress));
	}
    dbg("OurAddress                = %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&c->OurAddress));
    dbg("HomeNetworkPrefix         = %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&c->HomeNetworkPrefix));

    dbg("PBULifeTime               = %u seconds\n",c->PBULifeTime.tv_sec);
    dbg("PBALifeTime               = %u seconds\n",c->PBALifeTime.tv_sec);
    dbg("NRetransmissionTime       = %u seconds\n",c->NRetransmissionTime.tv_sec);
    dbg("MaxMessageRetransmissions = %u\n", c->MaxMessageRetransmissions);
    dbg("TunnelingEnabled          = %s\n", CONF_BOOL_STR(c->TunnelingEnabled));
    dbg("DynamicTunnelingEnabled   = %s\n", CONF_BOOL_STR(c->DynamicTunnelingEnabled));
	dbg("RadiusClientConfigFile    = %s\n", (c->RadiusClientConfigFile ? c->RadiusClientConfigFile :
	"No Config file"));
	dbg("RadiusPassword            = %s\n", (c->RadiusPassword ? c->RadiusPassword :
	"No password"));

}
