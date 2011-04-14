/*! \file pmip_consts.h
* \brief Describe all constants for pmip
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup CONSTANTS
 * @ingroup PMIP6D
 *  PMIP CONSTANTS 
 *  @{
 */

#ifndef __pmip_consts_h
#    define __pmip_consts_h
#    include <netinet/in.h>
/*
* Mobility Header Message Option Types
* new mobility header options types defined
*/
#    define IP6_MHOPT_HOM_NET_PREX   0x07   /* Home Network Prefix */
#    define IP6_MHOPT_MOB_IDENTIFIER 0x08   /* MOBILE NODE IDENTIFIER */
#    define IP6_MHOPT_TIME_STAMP     0x09   /* Timestamp */
#    define IP6_MHOPT_LINK_ADDR  0x0A   /* link local address */
/*
* Mobility Header Message Option Types
* Options for Route Optimization and Flow Binding
*/
#    define IP6_MHOPT_DST_MN_ADDR    0x0B   /* Source Mobile Node address */
#    define IP6_MHOPT_SERV_MAG_ADDR  0x0C   /* Serving MAG address */
#    define IP6_MHOPT_SERV_LMA_ADDR  0x0D   /* Source Mobile Node address */
#    define IP6_MHOPT_SRC_MN_ADDR    0x0E   /* Source Mobile Node address */
#    define IP6_MHOPT_SRC_MAG_ADDR   0x0F   /* Serving MAG address */
#    define IP6_MHOPT_PMIP_MAX       IP6_MHOPT_SRC_MAG_ADDR
//must change in mh.h
//#define IP6_MHOPT_MAX                 IP6_MHOPT_PMIP_MAX
/*
* New Mobility Header Message Types
* new mobility header  types defined
*/
#    define IP6_MH_TYPE_PBREQ   8   /* Proxy Binding Request */
#    define IP6_MH_TYPE_PBRES   9   /* Proxy Binding Response */
//#define IP6_MH_TYPE_MAX               IP6_MH_TYPE_PBRE //Defined in mh.h
/* PBU_flags */
#    if BYTE_ORDER == BIG_ENDIAN
#        define IP6_MH_PBU      0x0200  /* Proxy Binding Update */
#    else           /* BYTE_ORDER == LITTLE_ENDIAN */
#        define IP6_MH_PBU      0x0020  /* Proxy Binding Update */
#    endif
/* PBA_flags */
#    if BYTE_ORDER == BIG_ENDIAN
#        define IP6_MH_PBA      0x20    /* Proxy Binding Ack */
#    else           /* BYTE_ORDER == LITTLE_ENDIAN */
#        define IP6_MH_PBA      0x20    /* Proxy Binding Ack */
#    endif
/* MN_ID option_flags */
#    if BYTE_ORDER == BIG_ENDIAN
#        define IP6_MH_MNID     0x8000  /* MN_ID flag */
#    else           /* BYTE_ORDER == LITTLE_ENDIAN */
#        define IP6_MH_MNID     0x0080  /* MN_ID flag */
#    endif
/* NDP NA_flags */
/* Router flag */
#    if BYTE_ORDER == BIG_ENDIAN
#        define NDP_NA_ROUTER       0x80000000  /* Router flag */
#    else           /* BYTE_ORDER == LITTLE_ENDIAN */
#        define NDP_NA_ROUTER       0x00000080  /* Router flag */
#    endif
/* Solicited flag */
#    if BYTE_ORDER == BIG_ENDIAN
#        define NDP_NA_SOLICITED    0x40000000  /* Solicited flag */
#    else           /* BYTE_ORDER == LITTLE_ENDIAN */
#        define NDP_NA_SOLICITED    0x00000040  /* Solicited flag */
#    endif
/* Override flag */
#    if BYTE_ORDER == BIG_ENDIAN
#        define NDP_NA_OVERRIDE     0x20000000  /* Override flag */
#    else           /* BYTE_ORDER == LITTLE_ENDIAN */
#        define NDP_NA_OVERRIDE     0x00000020  /* Override flag */
#    endif
//Define STATUS FLAGS for FSM.
#    define hasDEREG        0x00000050  /* Has a DEREG */
#    define hasWLCCP        0x00000040  /* Has a WLCCP CISCO protocol */
#    define hasRS           0x00000030  /* Has a RS */
#    define hasNA           0x00000020  /* Has a NA */
#    define hasNS           0x00000010  /* Has a NS */
#    define hasPBU          0x00000008  /* Has a PBU */
#    define hasPBA          0x00000004  /* Has a PBA */
#    define hasPBREQ        0x00000002  /* Has a PBRR */
#    define hasPBRES        0x00000001  /* Has a PBRE */
#    define PLEN    64
#    define PBREQ_LOCATE                    0
#    define PBREQ_RO_TEST_INIT              1
#    define PBREQ_RO_INIT                   2
#    define PBRES_OK                        0
#    define PBRES_REASON_UNSPECIFIED        128
#endif
