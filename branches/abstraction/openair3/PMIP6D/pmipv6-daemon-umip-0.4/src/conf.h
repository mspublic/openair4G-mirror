/* $Id: conf.h 1.39 06/05/12 11:48:36+03:00 vnuorval@tcs.hut.fi $ */

#ifndef __CONF_H__
#define __CONF_H__ 1

#include <time.h>
#include <net/if.h>
#include "list.h"
#include "pmgr.h"

struct mip6_config {
	/* Common options */
	char *config_file;
#ifdef ENABLE_VT
	char *vt_hostname;
	char *vt_service;
#endif
	unsigned int mip6_entity;
	unsigned int debug_level;
	char *debug_log_file;
	struct pmgr_cb pmgr;
	struct list_head net_ifaces;
	struct list_head bind_acl;
	uint8_t DefaultBindingAclPolicy;
	char NonVolatileBindingCache;

	/* IPsec options */
	char KeyMngMobCapability;
	char UseMnHaIPsec;
	struct list_head ipsec_policies;

	/* MN options */
	unsigned int MnMaxHaBindingLife;
	unsigned int MnMaxCnBindingLife;
	unsigned int MnRouterProbes;
	struct timespec MnRouterProbeTimeout_ts;
	struct timespec InitialBindackTimeoutFirstReg_ts;
	struct timespec InitialBindackTimeoutReReg_ts;
	struct list_head home_addrs;
	char *MoveModulePath;
	uint16_t CnBuAck;
	char DoRouteOptimizationMN;
	char MnUseAllInterfaces;
	char MnDiscardHaParamProb;
	char SendMobPfxSols;
	char OptimisticHandoff;

	/* HA options */
	char SendMobPfxAdvs;
	char SendUnsolMobPfxAdvs;
	unsigned int MaxMobPfxAdvInterval;
	unsigned int MinMobPfxAdvInterval;
	unsigned int HaMaxBindingLife;

	/* CN options */
	char DoRouteOptimizationCN;

    /* PMIP MAG options */
    struct in6_addr AllLmaMulticastAddress;    // All-LMA Multicast Address (Eurecom' Extension for SPMIPv6)
    struct in6_addr LmaAddress;                // address of LMA
    struct in6_addr MagAddressIngress;         //ingress address of MAG
    struct in6_addr MagAddressEgress;          //egress address of entity "Either CH OR MR ".
    struct in6_addr OurAddress;
    struct in6_addr HomeNetworkPrefix;         // home network address common for domain!
    struct timespec PBULifeTime;               // Life time CH side.
    struct timespec PBALifeTime;               // Life time MR side.
    struct timespec NRetransmissionTime;       //N_Retransmissions times before Final Deletion of the entry task.
    int MaxMessageRetransmissions;             //indicates the maximum number of message retransmissions
    char TunnelingEnabled;
    char DynamicTunnelingEnabled;
	char* RadiusClientConfigFile;
	char* RadiusPassword;
};

struct net_iface {
	struct list_head list;
	char name[IF_NAMESIZE];
	int ifindex;
	int is_rtr;
	int mip6_if_entity;
	int mn_if_preference;
};

extern struct mip6_config conf;

#define MIP6_ENTITY_NO -1
#define MIP6_ENTITY_CN 0
#define MIP6_ENTITY_MN 1
#define MIP6_ENTITY_HA 2
#define MIP6_ENTITY_MAG 3
#define MIP6_ENTITY_LMA 4

static inline int is_cn(void)
{
	return conf.mip6_entity == MIP6_ENTITY_CN;
}

static inline int is_mn(void)
{
	return conf.mip6_entity == MIP6_ENTITY_MN;
}

static inline int is_ha(void)
{
	return conf.mip6_entity == MIP6_ENTITY_HA;
}

static inline int is_mag(void)
{
    return conf.mip6_entity == MIP6_ENTITY_MAG;
}

static inline int is_lma(void)
{
    return conf.mip6_entity == MIP6_ENTITY_LMA;
}

static inline int is_if_entity_set(struct net_iface *i)
{
	return i->mip6_if_entity != MIP6_ENTITY_NO;

}

static inline int is_if_cn(struct net_iface *i)
{
	return (is_cn() &&
		(!is_if_entity_set(i) || i->mip6_if_entity == MIP6_ENTITY_CN));

}

static inline int is_if_mn(struct net_iface *i)
{
	return (is_mn() &&
		(!is_if_entity_set(i) || i->mip6_if_entity == MIP6_ENTITY_MN));
}

static inline int is_if_ha(struct net_iface *i)
{
	return (is_ha() &&
		(!is_if_entity_set(i) || i->mip6_if_entity == MIP6_ENTITY_HA));
}

static inline int is_if_lma(struct net_iface *i)
{
	return (is_lma() &&
		(!is_if_entity_set(i) || i->mip6_if_entity == MIP6_ENTITY_LMA));
}

static inline int is_if_mag(struct net_iface *i)
{
	return (is_mag() &&
		(!is_if_entity_set(i) || i->mip6_if_entity == MIP6_ENTITY_MAG));
}

int conf_parse(struct mip6_config *c, int argc, char **argv);

void conf_show(struct mip6_config *c);

int yyparse(void);

int yylex(void);

#endif
