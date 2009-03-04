/*****************************************************************
 * C header: pmip_consts.h
 * Description: Describe all constants for pmip
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute, (C) 2008
 ******************************************************************/

#ifndef __pmip_consts_h
#define __pmip_consts_h

#include <netinet/in.h>

#define CONN_PER_DEST      16;
/*
 * Mobility Header Message Option Types
 * new mobility header options types defined
*/
#define IP6_MHOPT_HOM_NET_PREX   0x07	/* Home Network Prefix */
#define IP6_MHOPT_MOB_IDENTIFIER 0x08	/* MOBILE NODE IDENTIFIER */
#define IP6_MHOPT_TIME_STAMP	 0x09	/* Timestamp */
#define IP6_MHOPT_LINK_ADDR	 0x0A	/* link local address*/

/*
 * Mobility Header Message Option Types
 * Options for Route Optimization and Flow Binding
*/
#define IP6_MHOPT_DST_MN_ADDR	 0x0B	/* Destination CN's address*/
#define IP6_MHOPT_SERV_MAG_ADDR	 0x0C	/* Destination CN's Serving MAG address*/
#define IP6_MHOPT_SERV_LMA_ADDR	 0x0D	/* Destination CN's Serving LMA address*/
#define IP6_MHOPT_SRC_MN_ADDR	 0x0E	/* Source MN's address*/
#define IP6_MHOPT_SRC_MAG_ADDR	 0x0F	/* Source MN's Serving MAG address*/
#define IP6_MHOPT_PMIP_MAX 		 IP6_MHOPT_SRC_MAG_ADDR		

//must change in mh.h 
//#define IP6_MHOPT_MAX 		IP6_MHOPT_PMIP_MAX 



/*
 * New Mobility Header Message Types
 * new mobility header  types defined
*/
#define IP6_MH_TYPE_PBREQ	8	/* Proxy Binding Request */
#define IP6_MH_TYPE_PBRES	9	/* Proxy Binding Response */
//#define IP6_MH_TYPE_MAX		IP6_MH_TYPE_PBRE //Defined in mh.h


/* PBU_flags */
#if BYTE_ORDER == BIG_ENDIAN
#define IP6_MH_PBU		0x0200	/* Proxy Binding Update */
#else				/* BYTE_ORDER == LITTLE_ENDIAN */
#define IP6_MH_PBU		0x0020	/* Proxy Binding Update */
#endif

/* PBA_flags */
#if BYTE_ORDER == BIG_ENDIAN
#define IP6_MH_PBA		0x20	/* Proxy Binding Ack */
#else				/* BYTE_ORDER == LITTLE_ENDIAN */
#define IP6_MH_PBA		0x20	/* Proxy Binding Ack */
#endif

/* MN_ID option_flags */
#if BYTE_ORDER == BIG_ENDIAN
#define IP6_MH_MNID		0x8000	/* MN_ID flag */
#else				/* BYTE_ORDER == LITTLE_ENDIAN */
#define IP6_MH_MNID		0x0080	/* MN_ID flag */
#endif

/* NDP NA_flags */

/* Router flag */
#if BYTE_ORDER == BIG_ENDIAN
#define NDP_NA_ROUTER		0x80000000	/* Router flag */	
#else				/* BYTE_ORDER == LITTLE_ENDIAN */
#define NDP_NA_ROUTER		0x00000080	/* Router flag */	
#endif
/* Solicited flag */
#if BYTE_ORDER == BIG_ENDIAN
#define NDP_NA_SOLICITED	0x40000000	/* Solicited flag */
#else				/* BYTE_ORDER == LITTLE_ENDIAN */
#define NDP_NA_SOLICITED	0x00000040	/* Solicited flag */
#endif
/* Override flag */
#if BYTE_ORDER == BIG_ENDIAN
#define NDP_NA_OVERRIDE		0x20000000	/* Override flag */
#else				/* BYTE_ORDER == LITTLE_ENDIAN */
#define NDP_NA_OVERRIDE		0x00000020	/* Override flag */
#endif

//Define STATUS FLAGS for FSM.
#define hasNS			0x00000010	/* Has a PBU */
#define hasNA			0x00000020	/* Has a PBU */
#define hasPBU			0x00000008	/* Has a PBU */	
#define hasPBA			0x00000004	/* Has a PBA */	
#define hasPBREQ		0x00000002	/* Has a PBRR */	
#define hasPBRES		0x00000001	/* Has a PBRE */
#define PLEN	64

#define PBREQ_LOCATE					1 
#define PBREQ_RO_TEST_INIT				2
#define PBREQ_RO_INIT 					4
 	
#define PBRES_OK						0
#define PBRES_INTER_CLUSTER_MOBILITY	1
#define PBRES_REASON_UNSPECIFIED		128
					
#endif
