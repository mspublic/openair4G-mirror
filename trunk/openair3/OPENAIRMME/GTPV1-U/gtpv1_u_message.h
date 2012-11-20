#include <stdint.h>

#ifndef GTPV1_U_MESSAGE_H_
#define GTPV1_U_MESSAGE_H_

#pragma pack(1)
typedef struct gtpv1_u_header_s {
    uint8_t  pn:1;          ///< N-PDU number flag
    uint8_t  s:1;           ///< Sequence number flag
    uint8_t  e:1;           ///< Extension header flag
    uint8_t  spare:1;       ///< Spare bit, read as 0
    uint8_t  pt:1;          ///< Protocol type
    uint8_t  version:3;     ///< Protocol version
    uint8_t  message_type;  ///< Type of GTP-U message as defined in enum gtpv1_u_message_types_s
    uint16_t length;        ///< Payload length
    uint32_t teid;          ///< Tunnel Endpoint Identifier
} gtpv1_u_header_t;

#define GTPV1_U_HEADER_LENGTH       (sizeof(gtpv1_u_header_t))
#define GTPV1_U_LENGTH_FIELD_OFFSET (2)

typedef struct gtpv1_u_header_optional_fields_s {
    uint16_t sequence_number;
    uint8_t  n_pdu_number;
    uint8_t  next_extension_header_type;
} gtpv1_u_header_optional_fields_t;

#define GTPV1_U_HEADER_OPTIONAL_FIELDS_LENGTH \
    (sizeof(gtpv1_u_header_optional_fields_t))

typedef struct gtpv1_u_udp_port_s {
    uint8_t  spare;                 ///< Spare bits: read as 0x01
    uint16_t udp_port_number;       ///< PDCP PDU number
    uint8_t  next_extension_header_type;
} gtpv1_u_udp_port_t;

#define GTPV1_U_UDP_EXTENSION_LENGTH (sizeof(gtpv1_u_udp_port_t))

typedef struct gtpv1_u_ppn_s {
    uint8_t  spare;    ///< Spare bits: read as 0x01
    uint16_t ppn;      ///< PDCP PDU number
    uint8_t  next_extension_header_type;
} gtpv1_u_ppn_t;

#define GTPV1_U_PPN_EXTENSION_LENGTH (sizeof(gtpv1_u_ppn_t))

typedef struct gtpv1_u_recovery_s {
    uint8_t type;
    uint8_t restart_counter;
} gtpv1_u_recovery_t;

#define GTPV1_U_RECOVERY_IE_LENGTH (sizeof(gtpv1_u_recovery_t))

typedef struct gtpv1_u_teid_data_s {
    uint8_t  type;
    uint32_t teid_data;
} gtpv1_u_teid_data_t;

#define GTPV1_U_TEID_DATA_IE_LENGTH (sizeof(gtpv1_u_teid_data_t))

typedef struct gtpv1_u_tlv_s {
    uint8_t  type;
    uint16_t length;
} gtpv1_u_tlv_t;

#define GTPV1_U_TLV_HEADER_LENGTH (sizeof(gtpv1_u_tlv_t))

#pragma pack()

#define GTPV1_U_VERSION         (0x1)

#define GTPV1_U_TYPE_GTP_PRIME  (0x0)
#define GTPV1_U_TYPE_GTP        (0x1)

#define GTPV1_U_NO_EXTENSION    (0x0)
#define GTPV1_U_HAS_EXTENSION   (0x1)

#define GTPV1_U_SN_ABSENT       (0x0)
#define GTPV1_U_SN_PRESENT      (0x1)

#define GTPV1_U_N_PDU_ABSENT    (0x0)
#define GTPV1_U_N_PDU_PRESENT   (0x1)

#define GTPV1_U_NEXT_EXTENSION_LAST     (0x00)
#define GTPV1_U_NEXT_EXTENSION_UDP_PORT (0x40)
#define GTPV1_U_NEXT_EXTENSION_PDCP_PDU (0xC0)

enum gtpv1_u_message_types_s {
    GTPV1_U_ECHO_REQUEST                             = 1,
    GTPV1_U_ECHO_RESPONSE                            = 2,
    /* 3 to 25 reserved in 29.060 and 32.295 */
    GTPV1_U_ERROR_INDICATION                         = 26,
    /* 27 to 30 reserved in 29.060 */
    GTPV1_U_SUPPORTED_EXTENSION_HEADERS_NOTIFICATION = 31,
    /* 32 to 253 reserved in 29.060 */
    GTPV1_U_END_MARKER                               = 254,
    GTPV1_U_G_PDU                                    = 255,
};

enum gtpv1_u_ies_types_s {
    /* 0 to 13 = reserved in 29.060 */
    GTPV1_U_RECOVERY            = 14,
    /* 15 = reserved in 29.060 */
    GTPV1_U_TEID_DATA_1         = 16,
    /* 17 to 132 = reserved in 29.060 */
    GTPV1_U_PEER_ADDRESS        = 133,
    /* 134 to 140 = reserved in 29.060 */
    GTPV1_U_EHT_LIST            = 141,
    /* 142 to 254 = reserved in 29.060 */
    GTPV1_U_PRIVATE_EXTENSION   = 255,
};

int gtpv1_u_new_gpdu_message(
    const uint8_t   *gpdu,
    const uint8_t    gpdu_length,
    const uint32_t   teid,
    uint8_t        **buffer,
    uint32_t        *buffer_length);

int gtpv1_u_new_echo_request_message(
    const uint16_t   seq_number,
    uint8_t        **buffer,
    uint32_t        *buffer_length);

int gtpv1_u_new_echo_response_message(
    const uint16_t   seq_number,
    uint8_t        **buffer,
    uint32_t        *buffer_length);

int gtpv1_u_new_error_indication_message(
    const uint32_t   teid,
    const uint32_t   teid_data,
    const uint8_t    address_length,
    const uint8_t   *address,
    uint8_t        **buffer,
    uint32_t        *buffer_length);

int gtpv1_u_new_end_marker_message(
    const uint32_t   teid,
    uint8_t        **buffer,
    uint32_t        *buffer_length);

#endif /* GTPV1_U_MESSAGE_H_ */
