#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mme_default_values.h"

#include "gtpv1_u.h"
#include "gtpv1_u_message.h"

uint8_t packet_sample[] = {
    0x45, 0x00, 0x00, 0x4B, 0x57, 0x49, 0x40, 0x00, 0xFA,
    0x06, 0x85, 0x77, 0xC7, 0xB6, 0x78, 0x0E, 0xCE, 0xD6,
    0x95, 0x50, 0x00, 0x6E, 0x04, 0x9F, 0x74, 0x5B, 0xEE,
    0xA2, 0x59, 0x9A, 0x00, 0x0E, 0x50, 0x18, 0x24, 0x00,
    0xE3, 0x2A, 0x00, 0x00, 0x2B, 0x4F, 0x4B, 0x20, 0x50,
    0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64, 0x20, 0x72,
    0x65, 0x71, 0x75, 0x69, 0x72, 0x65, 0x64, 0x20, 0x66,
    0x6F, 0x72, 0x20, 0x61, 0x6C, 0x65, 0x78, 0x75, 0x72,
    0x2E, 0x0D, 0x0A, 0x67, 0xB2, 0x7E,
};

uint8_t ipv4_address[] = { 127, 0, 0, 1 };
uint8_t ipv6_address[] = {
    0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x32,
    0xE4, 0xBF, 0xFE, 0x1A, 0x83, 0x24
};

uint32_t teid = 0x00230105;

int main(void) {
    int fd;
    uint8_t  *buff = NULL;
    uint32_t  len;
    struct sockaddr_in servaddr;
    int return_code = 0;
    uint32_t seq_number = 0;

    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        return -1;
    }

    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(GTPV1_U_PORT_NUMBER);

    /* Sending a T-PDU */
    if (gtpv1_u_new_gpdu_message(packet_sample, sizeof(packet_sample), teid, &buff, &len) < 0) {
        fprintf(stderr, "gtpv1_u_new_gpdu_message failed\n");
        close(fd);
        return -1;
    }

    if (sendto(fd, buff, len, 0, (struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        perror("sendto");
        return -1;
    }

    free(buff);
    buff = NULL;

    /* Sending an echo request */
    if (gtpv1_u_new_echo_request_message(0x00000000, &buff, &len) < 0) {
        fprintf(stderr, "gtpv1_u_new_echo_request_message failed\n");
        close(fd);
        return -1;
    }

    if (sendto(fd, buff, len, 0, (struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        perror("sendto");
        return_code = -1;
        return -1;
    }

    free(buff);
    buff = NULL;

    /* Sending an echo response */
    if (gtpv1_u_new_echo_response_message(seq_number, &buff, &len) < 0) {
        fprintf(stderr, "gtpv1_u_new_echo_response_message failed\n");
        close(fd);
        return -1;
    }

    if (sendto(fd, buff, len, 0, (struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        perror("sendto");
        return_code = -1;
    }

    free(buff);
    buff = NULL;

    /* Sending an echo response */
    if (gtpv1_u_new_error_indication_message(teid, 0x00000000,
        4, ipv4_address, &buff, &len) < 0) {
        fprintf(stderr, "gtpv1_u_new_echo_response_message failed\n");
        close(fd);
        return -1;
    }

    if (sendto(fd, buff, len, 0, (struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        perror("sendto");
        return_code = -1;
    }

    free(buff);
    buff = NULL;

    /* Sending an echo response */
    if (gtpv1_u_new_error_indication_message(teid, 0x00000000,
        16, ipv6_address, &buff, &len) < 0) {
        fprintf(stderr, "gtpv1_u_new_echo_response_message failed\n");
        close(fd);
        return -1;
    }

    if (sendto(fd, buff, len, 0, (struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        perror("sendto");
        return_code = -1;
    }

        free(buff);
    buff = NULL;

    /* Sending end marker response */
    if (gtpv1_u_new_end_marker_message(teid, &buff, &len) < 0) {
        fprintf(stderr, "gtpv1_u_new_end_marker_message failed\n");
        close(fd);
        return -1;
    }

    if (sendto(fd, buff, len, 0, (struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        perror("sendto");
        return_code = -1;
    }

    free(buff);
    return return_code;
}
