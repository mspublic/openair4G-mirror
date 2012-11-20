#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#include "gtpv1_u_message.h"

#define GTPV1_U_ENCODE_FAILED   \
    {   \
        if (temp_buffer) free(temp_buffer); \
        buffer = NULL;  \
        *buffer_length = 0; \
        fprintf(stderr, "Failed at line %d in file %s\n", __LINE__, __FILE__);  \
        return -1;  \
    }

static int gtpv1_u_append_optionals(
    const uint16_t   seq_number,
    const uint8_t    n_pdu_number,
    const uint8_t    next_extension,
    uint8_t        **buffer,
    uint32_t        *buffer_length);
static int gtpv1_u_append_teid_data(
    const uint32_t   teid_data_param,
    uint8_t        **buffer,
    uint32_t        *buffer_length);
static int gtpv1_u_append_recovery(
    const uint8_t   restart_counter,
    uint8_t       **buffer,
    uint32_t       *buffer_length);
static int gtpv1_u_append_peer_address(
    const uint8_t    address_length,
    const uint8_t   *address,
    uint8_t        **buffer,
    uint32_t        *buffer_length);
static int gtpv1_u_append_udp_port(
    const uint16_t   udp_port,
    const uint8_t    next_extension,
    uint8_t        **buffer,
    uint32_t        *buffer_length);
static int gtpv1_u_append_pdcp_pdu_number(
    const uint16_t   ppn,
    const uint8_t    next_extension,
    uint8_t        **buffer,
    uint32_t        *buffer_length);

int gtpv1_u_new_gpdu_message(
    const uint8_t   *gpdu,
    const uint8_t    gpdu_length,
    const uint32_t   teid,
    uint8_t        **buffer,
    uint32_t        *buffer_length) {

    gtpv1_u_header_t header;
    uint8_t *temp_buffer;

    memset(&header, 0, sizeof(gtpv1_u_header_t));

    if (gpdu == NULL && gpdu_length != 0)
        return -1;

    header.version      = GTPV1_U_VERSION;
    header.pt           = GTPV1_U_TYPE_GTP;
    header.message_type = GTPV1_U_G_PDU;
    header.length       = htons(gpdu_length);
    header.teid         = htonl(teid);

    temp_buffer = malloc(sizeof(uint8_t) * (gpdu_length + GTPV1_U_HEADER_LENGTH));
    memcpy(temp_buffer, &header, GTPV1_U_HEADER_LENGTH);
    memcpy(&temp_buffer[GTPV1_U_HEADER_LENGTH], gpdu, gpdu_length);

    *buffer = temp_buffer;
    *buffer_length = GTPV1_U_HEADER_LENGTH + gpdu_length;

    return 0;
}

int gtpv1_u_new_echo_request_message(
    const uint16_t   seq_number,
    uint8_t        **buffer,
    uint32_t        *buffer_length) {

    gtpv1_u_header_t header;
    uint16_t *payload_length;
    uint8_t  *temp_buffer;
    uint32_t  message_size = 0;

    memset(&header, 0, sizeof(gtpv1_u_header_t));

    header.version      = GTPV1_U_VERSION;
    header.pt           = GTPV1_U_TYPE_GTP;
    header.s            = GTPV1_U_SN_PRESENT;
    header.message_type = GTPV1_U_ECHO_REQUEST;
    header.length       = htons(GTPV1_U_HEADER_OPTIONAL_FIELDS_LENGTH);
    header.teid         = htonl(0x00000000UL);

    temp_buffer = malloc(sizeof(uint8_t) * GTPV1_U_HEADER_LENGTH);

    payload_length = (uint16_t*)&temp_buffer[GTPV1_U_LENGTH_FIELD_OFFSET];

    memcpy(&temp_buffer[0], &header, GTPV1_U_HEADER_LENGTH);
    message_size += GTPV1_U_HEADER_LENGTH;

    if (gtpv1_u_append_optionals(seq_number, 0, GTPV1_U_NEXT_EXTENSION_LAST,
        &temp_buffer, &message_size) < 0)
        GTPV1_U_ENCODE_FAILED;

    *payload_length = htons(message_size - GTPV1_U_HEADER_LENGTH);

    *buffer = temp_buffer;
    *buffer_length = message_size;

    return 0;
}

int gtpv1_u_new_echo_response_message(
    const uint16_t   seq_number,
    uint8_t        **buffer,
    uint32_t        *buffer_length) {

    gtpv1_u_header_t                 header;

    uint16_t *payload_length;
    uint8_t  *temp_buffer;
    uint32_t  message_size = 0;

    memset(&header,    0, sizeof(gtpv1_u_header_t));

    header.version      = GTPV1_U_VERSION;
    header.pt           = GTPV1_U_TYPE_GTP;
    header.s            = GTPV1_U_SN_PRESENT;
    header.message_type = GTPV1_U_ECHO_RESPONSE;
    header.teid         = htonl(0x00000000UL);

    temp_buffer = malloc(sizeof(uint8_t) * GTPV1_U_HEADER_LENGTH);
    if (temp_buffer == NULL)
        GTPV1_U_ENCODE_FAILED;

    payload_length = (uint16_t*)&temp_buffer[GTPV1_U_LENGTH_FIELD_OFFSET];

    /* Copying header to buffer */
    memcpy(&temp_buffer[0], &header, GTPV1_U_HEADER_LENGTH);
    message_size += GTPV1_U_HEADER_LENGTH;

    if (gtpv1_u_append_optionals(seq_number, 0, GTPV1_U_NEXT_EXTENSION_LAST,
        &temp_buffer, &message_size) < 0)
        GTPV1_U_ENCODE_FAILED;

    /* Recovery IE is mandatory for compatilibity reasons.
     * 3GPP TS39.281 #7.2.2.
     */
    if (gtpv1_u_append_recovery(0, &temp_buffer, &message_size) < 0)
        GTPV1_U_ENCODE_FAILED;

    *payload_length = htons(message_size - GTPV1_U_HEADER_LENGTH);

    *buffer = temp_buffer;
    *buffer_length = message_size;

    return 0;
}

int gtpv1_u_new_error_indication_message(
    const uint32_t   teid,
    const uint32_t   teid_data,
    const uint8_t    address_length,
    const uint8_t   *address,
    uint8_t        **buffer,
    uint32_t        *buffer_length) {

    gtpv1_u_header_t  header;
    uint16_t         *payload_length;
    uint8_t          *temp_buffer;
    uint32_t          message_size = 0;

    memset(&header,    0, sizeof(gtpv1_u_header_t));

    header.version      = GTPV1_U_VERSION;
    header.pt           = GTPV1_U_TYPE_GTP;
    header.message_type = GTPV1_U_ERROR_INDICATION;
    header.teid         = htonl(teid);

    temp_buffer = malloc(sizeof(uint8_t) * GTPV1_U_HEADER_LENGTH);
    if (temp_buffer == NULL)
        GTPV1_U_ENCODE_FAILED;

    payload_length = (uint16_t*)&temp_buffer[GTPV1_U_LENGTH_FIELD_OFFSET];

    /* Copying header to buffer */
    memcpy(&temp_buffer[0], &header, GTPV1_U_HEADER_LENGTH);
    message_size += GTPV1_U_HEADER_LENGTH;

    if (gtpv1_u_append_teid_data(teid_data, &temp_buffer, &message_size) < 0)
        GTPV1_U_ENCODE_FAILED;

    if (gtpv1_u_append_peer_address(address_length, address, &temp_buffer, &message_size) < 0)
        GTPV1_U_ENCODE_FAILED;

    *payload_length = htons(message_size - GTPV1_U_HEADER_LENGTH);

    *buffer = temp_buffer;
    *buffer_length = message_size;

    return 0;
}

int gtpv1_u_new_end_marker_message(
    const uint32_t   teid,
    uint8_t        **buffer,
    uint32_t        *buffer_length) {

    gtpv1_u_header_t  header;
    uint16_t         *payload_length;
    uint8_t          *temp_buffer;
    uint32_t          message_size = 0;

    memset(&header,    0, sizeof(gtpv1_u_header_t));

    header.version      = GTPV1_U_VERSION;
    header.pt           = GTPV1_U_TYPE_GTP;
    header.message_type = GTPV1_U_END_MARKER;
    header.teid         = htonl(teid);

    temp_buffer = malloc(sizeof(uint8_t) * GTPV1_U_HEADER_LENGTH);
    if (temp_buffer == NULL)
        GTPV1_U_ENCODE_FAILED;

    payload_length = (uint16_t*)&temp_buffer[GTPV1_U_LENGTH_FIELD_OFFSET];

    /* Copying header to buffer */
    memcpy(&temp_buffer[0], &header, GTPV1_U_HEADER_LENGTH);
    message_size += GTPV1_U_HEADER_LENGTH;

    *payload_length = htons(message_size - GTPV1_U_HEADER_LENGTH);

    *buffer = temp_buffer;
    *buffer_length = message_size;

    return 0;
}

static int gtpv1_u_append_optionals(
    const uint16_t   seq_number,
    const uint8_t    n_pdu_number,
    const uint8_t    next_extension,
    uint8_t        **buffer,
    uint32_t        *buffer_length) {

    gtpv1_u_header_optional_fields_t optionals;
    uint32_t offset;

    optionals.sequence_number = htons(seq_number);
    optionals.n_pdu_number = n_pdu_number;
    optionals.next_extension_header_type = next_extension;

    offset = *buffer_length;
    *buffer_length += GTPV1_U_HEADER_OPTIONAL_FIELDS_LENGTH;

    *buffer = realloc(*buffer, *buffer_length);
    memcpy(&(*buffer)[offset], &optionals, GTPV1_U_HEADER_OPTIONAL_FIELDS_LENGTH);

    return 0;
}

static int gtpv1_u_append_peer_address(
    const uint8_t    address_length,
    const uint8_t   *address,
    uint8_t        **buffer,
    uint32_t        *buffer_length) {

    gtpv1_u_tlv_t peer_address;
    uint32_t offset;

    if (address == NULL || !(address_length == 4 || address_length == 16))
        return -1;

    offset = *buffer_length;
    *buffer_length += GTPV1_U_TLV_HEADER_LENGTH + address_length;

    peer_address.type = GTPV1_U_PEER_ADDRESS;
    peer_address.length = htons(address_length);

    *buffer = realloc(*buffer, *buffer_length);
    memcpy(&(*buffer)[offset], &peer_address, GTPV1_U_TLV_HEADER_LENGTH);
    offset += GTPV1_U_TLV_HEADER_LENGTH;
    memcpy(&(*buffer)[offset], address, address_length);

    return 0;
}

static int gtpv1_u_append_recovery(
    const uint8_t   restart_counter,
    uint8_t       **buffer,
    uint32_t       *buffer_length) {

    gtpv1_u_recovery_t recovery;
    uint32_t offset;

    offset = *buffer_length;
    *buffer_length += GTPV1_U_RECOVERY_IE_LENGTH;

    recovery.type = GTPV1_U_RECOVERY;
    recovery.restart_counter = restart_counter;

    *buffer = realloc(*buffer, *buffer_length);
    memcpy(&(*buffer)[offset], &recovery, GTPV1_U_RECOVERY_IE_LENGTH);

    return 0;
}

static int gtpv1_u_append_teid_data(
    const uint32_t   teid_data_param,
    uint8_t        **buffer,
    uint32_t        *buffer_length) {

    gtpv1_u_teid_data_t teid_data;
    uint32_t offset;

    offset = *buffer_length;
    *buffer_length += GTPV1_U_TEID_DATA_IE_LENGTH;

    teid_data.type = GTPV1_U_TEID_DATA_1;
    teid_data.teid_data = teid_data_param;

    *buffer = realloc(*buffer, *buffer_length);
    memcpy(&(*buffer)[offset], &teid_data, GTPV1_U_TEID_DATA_IE_LENGTH);

    return 0;
}

static int gtpv1_u_append_udp_port(
    const uint16_t   udp_port,
    const uint8_t    next_extension,
    uint8_t        **buffer,
    uint32_t        *buffer_length) {

    gtpv1_u_udp_port_t udp_port_number;
    uint32_t offset;

    offset = *buffer_length;
    *buffer_length += GTPV1_U_UDP_EXTENSION_LENGTH;

    udp_port_number.next_extension_header_type = next_extension;
    udp_port_number.spare  = 0x01;
    udp_port_number.udp_port_number = htons(udp_port);

    *buffer = realloc(*buffer, *buffer_length);
    if (*buffer == NULL)
        return -1;

    memcpy(&(*buffer)[offset], &udp_port_number, GTPV1_U_UDP_EXTENSION_LENGTH);

    return 0;
}

static int gtpv1_u_append_pdcp_pdu_number(
    const uint16_t   ppn,
    const uint8_t    next_extension,
    uint8_t        **buffer,
    uint32_t        *buffer_length) {

    gtpv1_u_ppn_t pdcp_pdu_number;
    uint32_t offset;

    offset = *buffer_length;
    *buffer_length += GTPV1_U_PPN_EXTENSION_LENGTH;

    pdcp_pdu_number.next_extension_header_type = next_extension;
    pdcp_pdu_number.spare  = 0x01;
    pdcp_pdu_number.ppn = htons(ppn);

    *buffer = realloc(*buffer, *buffer_length);
    if (*buffer == NULL)
        return -1;

    memcpy(&(*buffer)[offset], &pdcp_pdu_number, GTPV1_U_PPN_EXTENSION_LENGTH);

    return 0;
}
