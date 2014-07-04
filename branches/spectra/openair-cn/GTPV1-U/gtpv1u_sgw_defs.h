#ifndef GTPV1U_SGW_DEFS_H_
#define GTPV1U_SGW_DEFS_H_

#include "mme_config.h"
#include "NwGtpv1u.h"
#include "gtpv1u.h"
#include "hashtable.h"
#include "common_types.h"

#define GTPV1U_UDP_PORT (2152)

#define MAX_BEARERS_PER_UE (11)

typedef enum {
    BEARER_DOWN = 0,
    BEARER_IN_CONFIG,
    BEARER_UP,
    BEARER_DL_HANDOVER,
    BEARER_UL_HANDOVER,
    BEARER_MAX,
} bearer_state_t;

#define BUFFER_TO_NwU32T(buf, x) \
do {                            \
    x = ((NwU32T)((buf)[0]) ) |      \
        ((NwU32T)((buf)[1]) << 8) |      \
        ((NwU32T)((buf)[2]) << 16)  |      \
        ((NwU32T)((buf)[3]) << 24);             \
} while(0)



typedef struct gtpv1u_teid2enb_info_s {
    /* TEID used in dl and ul */
    uint32_t       teid_enb;         ///< Remote eNB TEID
    ip_address_t   enb_ip_addr;
    bearer_state_t state;
    uint16_t       port; /// LG ???
} gtpv1u_teid2enb_info_t;

/*struct gtpv1u_ue_data_s {
    // UE identifier for oaisim stack
    uint8_t  ue_id;

    // Unique identifier used between PDCP and GTP-U to distinguish UEs
    uint32_t instance_id;

    // Bearer related data.
     // Note that the first LCID available for data is 3 and we fixed the maximum
     // number of e-rab per UE to be 11. The real rb id will 3 + rab_id (0..10).
     //
    struct gtpv1u_bearer_s bearers[MAX_BEARERS_PER_UE];

    RB_ENTRY(gtpv1u_ue_data_s) gtpv1u_ue_node;
};*/

typedef struct {
    /* nwgtpv1u stack internal data */
    NwGtpv1uStackHandleT  gtpv1u_stack;
    /* RB tree of UEs */
    //RB_HEAD(gtpv1u_ue_map, gtpv1u_ue_data_s) gtpv1u_ue_map_head;
    /* Local IP address to use */
    uint32_t  sgw_ip_address_for_S1u_S12_S4_up;
    char                 *ip_addr;

    uint16_t              seq_num;
    uint8_t               restart_counter;
    //gtpv1u_teid2enb_info_t* teid2enb_mapping[];
    hash_table_t         *S1U_mapping;

} gtpv1u_data_t;


int gtpv1u_init(const mme_config_t *mme_config);

#endif /* GTPV1U_SGW_DEFS_H_ */
