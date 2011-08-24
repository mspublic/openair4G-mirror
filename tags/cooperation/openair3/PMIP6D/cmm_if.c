/*! \file cmm_if.c
* \brief Unix socket communication skeleton for interaction with CMM
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
/*****************************************************************
* C Implementation: cmm_if.c
* Description: Unix socket communication skeleton for interaction with CMM
* Author:
*   Huu-Nghia Nguyen
*   Christian Bonnet
* Contributor:
*   Lionel Gauthier
* Copyright: Eurecom Institute,(C) 2008
******************************************************************/
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <unistd.h>
#define MAX_CMM_CLIENT 15
#define MAX_SOCK_BUFFER_SIZE 1500
/*******************************************************************************************
*                             CMM SERVER IMPLEMENTATION                                   *
*******************************************************************************************/
typedef void (*cmm_handler_t) (const int fd, const void *data, ssize_t len);
int cmm_server_fd;
char *cmm_socket = "/tmp/cmm_server.ctl";
cmm_handler_t cmm_server_handler = NULL;
pthread_t cmm_server_listener;
//---------------------------------------------------------------
void *cmm_server_listen(void *arg)
//---------------------------------------------------------------
{
    struct sockaddr addr;
    unsigned int len, cfd;
    printf("cmm server thread started\n");
    while (1 == 1) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (listen(cmm_server_fd, MAX_CMM_CLIENT) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    len = sizeof(addr);
    cfd = accept(cmm_server_fd, &addr, &len);
//Create new thread/process for this new connection
    if (fork() == 0) {
#ifndef S_SPLINT_S
        printf("New child process for a new client pid=%d\n", getpid());
#endif
        char buffer[MAX_SOCK_BUFFER_SIZE];
        size_t len;
        close(cmm_server_fd);   //we don't need this socket in the child process
        while (1) {
//Read msg
        len = recv(cfd, buffer, MAX_SOCK_BUFFER_SIZE, 0);
        if (cmm_server_handler)
            cmm_server_handler(cfd, buffer, len);
        }
    }
    }
    pthread_exit(NULL);
}

//---------------------------------------------------------------
int cmm_server_init(cmm_handler_t handler)  /*@globals cmm_server_handler;@ */
//---------------------------------------------------------------
{
    int one = 1;
    struct sockaddr_un sun;
    cmm_server_handler = handler;
    if ((cmm_server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return -1;
    }
    if (setsockopt(cmm_server_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) < 0) {
    perror("setsockopt");
    return -1;
    }
//
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, cmm_socket, sizeof(sun.sun_path));
    if (bind(cmm_server_fd, (struct sockaddr *) &sun, sizeof(sun)) < 0) {
    perror("bind");
    return -1;
    }
/* create CMM listener thread */
    if (pthread_create(&cmm_server_listener, NULL, cmm_server_listen, NULL)) {
    perror("thread");
    return -1;
    }
    return 0;
}

//---------------------------------------------------------------
void cmm_server_cleanup(void)
//---------------------------------------------------------------
{
    if (unlink(cmm_socket) < 0) {
    printf("Couldn't remove control socket '%s' : ", cmm_socket);
    perror("");
    }
    close(cmm_server_fd);
    pthread_cancel(cmm_server_listener);
    pthread_join(cmm_server_listener, NULL);
}

/*******************************************************************************************
*                             CMM CLIENT IMPLEMENTATION                                   *
*******************************************************************************************/
//Client Side
int cmm_client_fd;
pthread_t cmm_client_listener;
cmm_handler_t cmm_client_handler = NULL;
//---------------------------------------------------------------
int cmm_client_connect(struct sockaddr_un *cmm_addr, struct sockaddr_un *local_addr)
//---------------------------------------------------------------
{
    int fd;
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
    perror("socket");
    return -1;
    }
    if (bind(fd, (struct sockaddr *) local_addr, sizeof(*local_addr)) < 0) {
    perror("bind");
    return -1;
    }
    if (connect(fd, (struct sockaddr *) cmm_addr, sizeof(*cmm_addr)) < 0) {
    perror("connect");
    return -1;
    }
    return fd;
}

//---------------------------------------------------------------
void *cmm_client_listen(void *arg)
//---------------------------------------------------------------
{
    printf("cmm client thread started\n");
    while (1) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
//Read msg
#ifndef S_SPLINT_S
    char buffer[MAX_SOCK_BUFFER_SIZE];
    int len = recv(cmm_client_fd, buffer, MAX_SOCK_BUFFER_SIZE, 0);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (cmm_client_handler)
        cmm_client_handler(cmm_client_fd, buffer, len);
#endif
    }
    pthread_exit(NULL);
}

//---------------------------------------------------------------
int cmm_client_init(cmm_handler_t handler)  /*@globals cmm_client_handler;@ */
//---------------------------------------------------------------
{
//Create servide unix address
    struct sockaddr_un server_sun, local_sun;
//Create client unix address
    struct timeval tv;
//Register the handler
    cmm_client_handler = handler;
    server_sun.sun_family = AF_UNIX;
    strncpy(server_sun.sun_path, cmm_socket, sizeof(server_sun.sun_path));
    gettimeofday(&tv, NULL);
    local_sun.sun_family = AF_UNIX;
    sprintf(local_sun.sun_path, "/tmp/cmm-client-%d-%d", getpid(), (int) tv.tv_usec);
    printf("Socket name %s\n", local_sun.sun_path);
//Connect to cmm_server
    cmm_client_fd = cmm_client_connect(&server_sun, &local_sun);
    if (cmm_client_fd < 0) {
    fprintf(stderr, "Unable to create cmm client side socket\n");
    return -1;
    }
/* create cmm listener thread */
    if (pthread_create(&cmm_client_listener, NULL, cmm_client_listen, NULL)) {
    perror("thread");
    return -1;
    }
    return 0;
}

//---------------------------------------------------------------
void cmm_client_cleanup(void)
//---------------------------------------------------------------
{
    close(cmm_client_fd);
    pthread_cancel(cmm_client_listener);
    pthread_join(cmm_client_listener, NULL);
}

#ifdef UNIT_TEST
void cmm_server_recv(const int fd, const void *data, ssize_t len)
{
    char buffer[MAX_SOCK_BUFFER_SIZE];
    char *msg = (char *) data;
//Multiplex to right handler
    msg[len] = 0;
    printf("CMM server (pid=%d) received: %s\n", getpid(), msg);
//Ack back to the sender
    sprintf(buffer, "%s_ACK", msg);
    len = send(fd, buffer, strlen(buffer), 0);
}
void cmm_pmip_recv(const int fd, const void *data, ssize_t len)
{
    char buffer[MAX_SOCK_BUFFER_SIZE];
    char *msg = (char *) data;
//Multiplex to right handler
    msg[len] = 0;
    printf("PMIP (pid=%d) received: %s\n", getpid(), msg);
}
void cmm_mpls_recv(const int fd, const void *data, ssize_t len)
{
    char buffer[MAX_SOCK_BUFFER_SIZE];
    char *msg = (char *) data;
//Multiplex to right handler
    msg[len] = 0;
    printf("MPLS (pid=%d) received: %s\n", getpid(), msg);
}

int main()
{
//Main process is the CMM server
    if (cmm_server_init(cmm_server_recv) < 0)
    exit(EXIT_FAILURE);
//Child process: First CMM client = PMIP+
    if (fork() == 0) {
    sleep(2);
    cmm_client_init(cmm_pmip_recv);
    char buffer[MAX_SOCK_BUFFER_SIZE];  //Read msg
    strcpy(buffer, "PMIP_RO_REQ");
    int len = send(cmm_client_fd, buffer, strlen("PMIP_RO_REQ"), 0);
    sleep(2);
    strcpy(buffer, "PMIP_RO_REQ");
    len = send(cmm_client_fd, buffer, strlen("PMIP_RO_REQ"), 0);
    while (1) {
    };
    }
//Child process Second CMM client = MPLS
    if (fork() == 0) {
    sleep(3);
    cmm_client_init(cmm_mpls_recv);
    char buffer[MAX_SOCK_BUFFER_SIZE];  //Read msg
    strcpy(buffer, "MPLS_SETUP_PATH_REQ");
    int len = send(cmm_client_fd, buffer, strlen("MPLS_SETUP_PATH_REQ"), 0);
    while (1) {
    };
    }
    printf("Press Ctrl+C to stop\n");
    while (1) {
    };
}
#endif              /*
                 */
