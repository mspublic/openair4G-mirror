#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "intertask_interface.h"
#include "udp_primitives_server.h"

#define IPV4_ADDR    "%u.%u.%u.%u"
#define IPV4_ADDR_FORMAT(aDDRESS)               \
    (uint8_t)((aDDRESS)  & 0x000000ff),         \
    (uint8_t)(((aDDRESS) & 0x0000ff00) >> 8 ),  \
    (uint8_t)(((aDDRESS) & 0x00ff0000) >> 16),  \
    (uint8_t)(((aDDRESS) & 0xff000000) >> 24)

#ifndef UDP_DEBUG
# define UDP_DEBUG(x, args...) do { fprintf(stdout, "[UDP][D]"x, ##args); } while(0)
# define UDP_ERROR(x, args...) do { fprintf(stderr, "[UDP][E]"x, ##args); } while(0)
#endif

/* Reader thread: reads messages from network */
static pthread_t udp_recv_thread;
/* UDP task thread: read messages from other tasks */
static pthread_t udp_task_thread;

void *udp_receiver_thread(void *args);
static int udp_create_socket(int port, char *address);

static int udp_create_socket(int port, char *address) {
    struct sockaddr_in addr;
    int fd;

    UDP_DEBUG("Creating new listen socket on address "IPV4_ADDR" and port %d\n",
              IPV4_ADDR_FORMAT(inet_addr(address)), port);

    /* Create UDP socket */
    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        /* Socket creation has failed... */
        UDP_ERROR("Socket creation failed (%s)\n", strerror(errno));
        return fd;
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr(address);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
        /* Bind failed */
        UDP_ERROR("Socket bind failed (%s)\n", strerror(errno));
        close(fd);
        return -1;
    }

    if (pthread_create(&udp_recv_thread, NULL, &udp_receiver_thread, &fd) < 0) {
        UDP_ERROR("Pthred_create failed (%s)\n", strerror(errno));
        return -1;
    }
    return fd;
}

void *udp_receiver_thread(void *args) {
    int *fd_p = (int*)args;
    int  fd = *fd_p;

    while (1) {
        uint8_t buffer[1024];
        struct sockaddr cli_addr;
        socklen_t from_len = (socklen_t)sizeof(struct sockaddr_in);

        if (recvfrom(fd, buffer, sizeof(buffer), 0, &cli_addr, &from_len) < 0) {
            UDP_ERROR("Recvfrom failed %s\n", strerror(errno));
            break;
        }
        UDP_DEBUG("Received new message on UDP\n");
    }
    return NULL;
}

static void *udp_intertask_interface(void *args) {
    while(1) {
        MessageDef *receivedMessage;
        receive_msg(TASK_UDP, &receivedMessage);
        switch(receivedMessage->messageId) {
            case UDP_INIT:
            {
                UdpInit *init;
                init = &receivedMessage->msg.udpInit;
                udp_create_socket(init->port, init->address);
            } break;
            default:
            {
                UDP_DEBUG("Unknown message ID %d\n", receivedMessage->messageId);
            } break;
        }
        free(receivedMessage);
        receivedMessage = NULL;
    }
    return NULL;
}

int udp_init(const mme_config_t *mme_config) {
    UDP_DEBUG("Initializing UDP task interface\n");
    if (pthread_create(&udp_task_thread, NULL, &udp_intertask_interface, NULL) < 0) {
        UDP_ERROR("udp pthread_create (%s)\n", strerror(errno));
        return -1;
    }
    UDP_DEBUG("Initializing UDP task interface: DONE\n");
    return 0;
}
