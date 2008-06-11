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

/*
 * Mobility Header Message Option Types
 * new mobility header options types defined
*/
#define IP6_MHOPT_HOM_NET_PREX   0x07	/* Home Network Prefix */
#define IP6_MHOPT_MOB_IDENTIFIER 0x08	/* MOBILE NODE IDENTIFIER */
#define IP6_MHOPT_TIME_STAMP	 0x09	/* Timestamp */
#define IP6_MHOPT_LINK_ADDR	 0x0A	/* link local address*/
#define IP6_MHOPT_SERV_MAG_ADDR	 0x0B	/* Serving MAG address*/

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
#if BYTE_ORDER == BIG_ENDIAN
#define hasPBU			0x80000000	/* Has a PBU */	
#define hasPBA			0x40000000	/* Has a PBA */	
#define hasPBREQ		0x20000000	/* Has a PBRR */	
#define hasPBRES		0x10000000	/* Has a PBRE */	

// 	 hasID;			//TODO to be defined later as flags to specify which options to create or not
// 	 hasTimeStamp;		
// 	 hasPrefix;
// 	 hasLinkLocal;

#else				/* BYTE_ORDER == LITTLE_ENDIAN */
#define hasPBU			0x00000080	/* Has a PBU */	
#define hasPBA			0x00000040	/* Has a PBA */	
#define hasPBREQ		0x00000020	/* Has a PBRR */	
#define hasPBRES		0x00000010	/* Has a PBRE */	
	
#endif

#endif
