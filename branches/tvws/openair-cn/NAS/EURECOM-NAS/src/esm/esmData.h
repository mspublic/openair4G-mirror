/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      esmData.h

Version     0.1

Date        2012/12/04

Product     NAS stack

Subsystem   EPS Session Management

Author      Frederic Maurel

Description Defines internal private data handled by EPS Session
        Management sublayer.

*****************************************************************************/
#include <stdio.h>  // sprintf

#include "networkDef.h"
#include "OctetString.h"
#include "EpsBearerIdentity.h"

#ifdef NAS_MME
#include "mme_api.h"
#endif

#if defined(EPC_BUILD)
# include "tree.h"
#endif

#ifndef __ESMDATA_H__
#define __ESMDATA_H__

/****************************************************************************/
/*********************  G L O B A L    C O N S T A N T S  *******************/
/****************************************************************************/

/* Total number of active EPS bearers */
#define ESM_DATA_EPS_BEARER_TOTAL   11

/****************************************************************************/
/************************  G L O B A L    T Y P E S  ************************/
/****************************************************************************/

/*
 * Minimal and maximal value of an EPS bearer identity:
 * The EPS Bearer Identity (EBI) identifies a message flow
 */
#define ESM_EBI_MIN     (EPS_BEARER_IDENTITY_FIRST)
#define ESM_EBI_MAX     (EPS_BEARER_IDENTITY_LAST)

/* EPS bearer context states */
typedef enum {
    ESM_EBR_INACTIVE,       /* No EPS bearer context exists     */
    ESM_EBR_ACTIVE,     /* The EPS bearer context is active,
                 * in the UE, in the network        */
#ifdef NAS_MME
    ESM_EBR_INACTIVE_PENDING,   /* The network has initiated an EPS bearer
                 * context deactivation towards the UE  */
    ESM_EBR_MODIFY_PENDING, /* The network has initiated an EPS bearer
                 * context modification towards the UE  */
    ESM_EBR_ACTIVE_PENDING, /* The network has initiated an EPS bearer
                 * context activation towards the UE    */
#endif
    ESM_EBR_STATE_MAX
} esm_ebr_state;

#ifdef NAS_MME
/* ESM message timer retransmission data */
typedef struct {
    void        *ctx;
    unsigned int ueid;      /* Lower layers UE identifier       */
    unsigned int ebi;       /* EPS bearer identity          */
    unsigned int count;     /* Retransmission counter       */
    OctetString  msg;        /* Encoded ESM message to re-transmit   */
} esm_ebr_timer_data_t;
#endif

/*
 * -----------------------
 * EPS bearer context data
 * -----------------------
 */
typedef struct {
    unsigned char ebi;      /* EPS bearer identity          */
    esm_ebr_state status;   /* EPS bearer context status        */
#ifdef NAS_UE
    int is_default_ebr;     /* TRUE if the bearer context is associated
                 * to a default EPS bearer      */
    char cid;           /* Identifier of the PDN context the EPS
                 * bearer context has been assigned to  */
#endif
#ifdef NAS_MME
    struct nas_timer_t timer;   /* Retransmission timer         */
    esm_ebr_timer_data_t *args; /* Retransmission timer parameters data */
#endif
} esm_ebr_context_t;

typedef struct {
    unsigned char index;    /* Index of the next EPS bearer context
                 * identity to be used */
#define ESM_EBR_DATA_SIZE (ESM_EBI_MAX - ESM_EBI_MIN + 1)
    esm_ebr_context_t *context[ESM_EBR_DATA_SIZE + 1];
} esm_ebr_data_t;

/*
 * --------------------------------------------------------------------------
 * Structure of data handled by EPS Session Management sublayer in the UE
 * and in the MME
 * --------------------------------------------------------------------------
 */

/*
 * Structure of an EPS bearer
 * --------------------------
 * An EPS bearer is a logical concept which applies to the connection
 * between two endpoints (UE and PDN Gateway) with specific QoS attri-
 * butes. An EPS bearer corresponds to one Quality of Service policy
 * applied within the EPC and E-UTRAN.
 */
typedef struct {
    int bid;        /* Identifier of the EPS bearer         */
    unsigned int ebi;   /* EPS bearer identity              */
    network_qos_t qos;  /* EPS bearer level QoS parameters      */
    network_tft_t tft;  /* Traffic Flow Template for packet filtering   */
} esm_bearer_t;

/*
 * Structure of a PDN connection
 * -----------------------------
 * A PDN connection is the association between a UE represented by
 * one IPv4 address and/or one IPv6 prefix and a PDN represented by
 * an Access Point Name (APN).
 */
typedef struct {
    unsigned int pti;   /* Identity of the procedure transaction executed
             * to activate the PDN connection entry     */
    int is_emergency;   /* Emergency bearer services indicator      */
    OctetString apn;    /* Access Point Name currently in used      */
    int ambr;       /* Aggregate Maximum Bit Rate of this APN   */
    int type;       /* Address PDN type (IPv4, IPv6, IPv4v6)    */
#define ESM_DATA_IPV4_ADDRESS_SIZE  4
#define ESM_DATA_IPV6_ADDRESS_SIZE  8
#define ESM_DATA_IP_ADDRESS_SIZE    (ESM_DATA_IPV4_ADDRESS_SIZE + \
                                     ESM_DATA_IPV6_ADDRESS_SIZE)
    /* IPv4 PDN address and/or IPv6 prefix      */
    char ip_addr[ESM_DATA_IP_ADDRESS_SIZE+1];
    int addr_realloc;   /* Indicates whether the UE is allowed to subsequently
             * request another PDN connectivity to the same APN
             * using an address PDN type (IPv4 or IPv6) other
             * than the one already activated       */
    int n_bearers;  /* Number of allocated EPS bearers;
             * default EPS bearer is defined at index 0 */
#define ESM_DATA_EPS_BEARER_MAX 4
    esm_bearer_t *bearer[ESM_DATA_EPS_BEARER_MAX];
} esm_pdn_t;

/*
 * Structure of the ESM data
 * -------------------------
 * The EPS Session Management sublayer handles data related to PDN
 * connections and EPS bearers. Each active PDN connection has a de-
 * fault EPS bearer. Several dedicated EPS bearers may exist within
 * a PDN connection.
 */
typedef struct esm_data_context_s {
    unsigned int ue_id;

    int n_ebrs;     /* Total number of active EPS bearer contexts   */
    int n_pdns;     /* Number of active PDN connections     */
    int emergency;  /* Indicates whether a PDN connection for emergency
             * bearer services is established       */
#define ESM_DATA_PDN_MAX    4
    struct {
        int pid;     /* Identifier of the PDN connection        */
        int is_active;   /* TRUE/FALSE if the PDN connection is active/inactive
              * or the process to activate/deactivate the PDN
              * connection is in progress           */
        esm_pdn_t *data; /* Active PDN connection data          */
    } pdn[ESM_DATA_PDN_MAX+1];

    esm_ebr_data_t ebr;
} esm_data_context_t;

/*
 * --------------------------------------------------------------------------
 *  ESM internal data handled by EPS Session Management sublayer in the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/*
 * Structure of the ESM data
 * -------------------------
 */
typedef esm_data_context_t esm_data_t;
#endif // NAS_UE

/*
 * --------------------------------------------------------------------------
 *  ESM internal data handled by EPS Session Management sublayer in the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/*
 * Structure of the ESM data
 * -------------------------
 */
typedef struct {
    /*
     * MME configuration
     * -----------------
     */
    mme_api_esm_config_t conf;
    /*
     * ESM contexts
     * ------------
     */
# if defined(EPC_BUILD)
    /* Use a tree for ue data context within MME */
    RB_HEAD(esm_data_context_map, esm_data_context_s) ctx_map;
# else
#   define ESM_DATA_NB_UE_MAX  (MME_API_NB_UE_MAX + 1)
    esm_data_context_t *ctx[ESM_DATA_NB_UE_MAX];
# endif
} esm_data_t;

# if defined(EPC_BUILD)
struct esm_data_context_s *esm_data_context_get(
    esm_data_t *esm_data, unsigned int _ueid);

struct esm_data_context_s *esm_data_context_remove(
    esm_data_t *esm_data, struct esm_data_context_s *elm);

void esm_data_context_add(esm_data_t *esm_data, struct esm_data_context_s *elm);

# endif

#endif //NAS_MME

/****************************************************************************/
/********************  G L O B A L    V A R I A B L E S  ********************/
/****************************************************************************/

/*
 * ESM internal data (used within ESM only)
 * ----------------------------------------
 */
esm_data_t _esm_data;

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

extern char ip_addr_str[100];

extern inline char *esm_data_get_ipv4_addr(const OctetString *ip_addr);

extern inline char *esm_data_get_ipv6_addr(const OctetString *ip_addr);

extern inline char *esm_data_get_ipv4v6_addr(const OctetString *ip_addr);

#endif /* __ESMDATA_H__*/
