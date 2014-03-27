#ifndef GTPV1_U_MESSAGES_TYPES_H_
#define GTPV1_U_MESSAGES_TYPES_H_

#define GTPV1U_MAX_BEARERS_PER_UE max_val_DRB_Identity

#define GTPV1U_ENB_CREATE_TUNNEL_REQ(mSGpTR)  (mSGpTR)->ittiMsg.Gtpv1uCreateTunnelReq
#define GTPV1U_ENB_CREATE_TUNNEL_RESP(mSGpTR) (mSGpTR)->ittiMsg.Gtpv1uCreateTunnelResp
#define GTPV1U_ENB_UPDATE_TUNNEL_REQ(mSGpTR)  (mSGpTR)->ittiMsg.Gtpv1uUpdateTunnelReq
#define GTPV1U_ENB_UPDATE_TUNNEL_RESP(mSGpTR) (mSGpTR)->ittiMsg.Gtpv1uUpdateTunnelResp
#define GTPV1U_ENB_DELETE_TUNNEL_REQ(mSGpTR)  (mSGpTR)->ittiMsg.Gtpv1uDeleteTunnelReq
#define GTPV1U_ENB_DELETE_TUNNEL_RESP(mSGpTR) (mSGpTR)->ittiMsg.Gtpv1uDeleteTunnelResp
#define GTPV1U_ENB_TUNNEL_DATA_IND(mSGpTR)    (mSGpTR)->ittiMsg.Gtpv1uTunnelDataInd
#define GTPV1U_ENB_TUNNEL_DATA_REQ(mSGpTR)    (mSGpTR)->ittiMsg.Gtpv1uTunnelDataReq

typedef struct gtpv1u_enb_create_tunnel_req_s {
    module_id_t            ue_index;
    int                    num_tunnels;
    teid_t                 sgw_S1u_teid[GTPV1U_MAX_BEARERS_PER_UE];  ///< Tunnel Endpoint Identifier
    ebi_t                  eps_bearer_id[GTPV1U_MAX_BEARERS_PER_UE];
    transport_layer_addr_t sgw_addr[GTPV1U_MAX_BEARERS_PER_UE];
} gtpv1u_enb_create_tunnel_req_t;

typedef struct gtpv1u_enb_create_tunnel_resp_s {
    uint8_t                status;               ///< Status of S1U endpoint creation (Failed = 0xFF or Success = 0x0)
    uint8_t                ue_index;
    int                    num_tunnels;
    teid_t                 enb_S1u_teid[GTPV1U_MAX_BEARERS_PER_UE];  ///< Tunnel Endpoint Identifier
    ebi_t                  eps_bearer_id[GTPV1U_MAX_BEARERS_PER_UE];
    transport_layer_addr_t enb_addr;
} gtpv1u_enb_create_tunnel_resp_t;

typedef struct gtpv1u_enb_update_tunnel_req_s {
    uint8_t                ue_index;
    teid_t                 enb_S1u_teid;         ///< eNB S1U Tunnel Endpoint Identifier
    teid_t                 sgw_S1u_teid;         ///< SGW S1U local Tunnel Endpoint Identifier
    transport_layer_addr_t sgw_addr;
    ebi_t                  eps_bearer_id;
} gtpv1u_enb_update_tunnel_req_t;

typedef struct gtpv1u_enb_update_tunnel_resp_s {
    uint8_t                ue_index;
    uint8_t                status;               ///< Status (Failed = 0xFF or Success = 0x0)
    teid_t                 enb_S1u_teid;         ///< eNB S1U Tunnel Endpoint Identifier
    teid_t                 sgw_S1u_teid;         ///< SGW S1U local Tunnel Endpoint Identifier
    ebi_t                  eps_bearer_id;
} gtpv1u_enb_update_tunnel_resp_t;

typedef struct gtpv1u_enb_delete_tunnel_req_s {
    module_id_t            ue_index;
    ebi_t                  eps_bearer_id;
    teid_t                 enb_S1u_teid;         ///< local SGW S11 Tunnel Endpoint Identifier
} gtpv1u_enb_delete_tunnel_req_t;

typedef struct gtpv1u_enb_delete_tunnel_resp_s {
    uint8_t                ue_index;
    uint8_t                status;               ///< Status of S1U endpoint deleteion (Failed = 0xFF or Success = 0x0)
    teid_t                 enb_S1u_teid;         ///< local S1U Tunnel Endpoint Identifier to be deleted
} gtpv1u_enb_delete_tunnel_resp_t;

typedef struct gtpv1u_enb_tunnel_data_ind_s {
    uint8_t                ue_index;
    uint8_t               *buffer;
    uint32_t               length;
    teid_t                 enb_S1u_teid;         ///< Tunnel Endpoint Identifier
} gtpv1u_enb_tunnel_data_ind_t;

typedef struct gtpv1u_enb_tunnel_data_req_s {
    uint8_t               *buffer;
    uint32_t               length;
    uint8_t                ue_index;
    rb_id_t                rab_id;
} gtpv1u_enb_tunnel_data_req_t;

#endif /* GTPV1_U_MESSAGES_TYPES_H_ */
