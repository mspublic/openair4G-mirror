/***************************************************************************
                          NASUEdummy.c  -  description
                             -------------------
    copyright            : (C) 2005 by Eurecom
    email                : michelle.wetterwald@eurecom.fr
 ***************************************************************************
 Dummy NAS for MT - Test interface with Dummy A21_MT_RAL_UMTS
 ***************************************************************************/
#include <stdio.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "nas_ue_netlink.h"

#define CONF_NASUE_DUMMY
#define CONF_UNKNOWN_CELL_ID 0

#include "nasUE_config.h"

int state, cell_id;

int NAS_IAL_sock_connect(void)
{
    struct sockaddr_un remote;
    int len,s;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("NAS_IALconnect - socket");
        exit(1);
    }

    printf("Trying to connect...\n");
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_NAS_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        perror("NAS_IALconnect - connect");
        return -1;
    }

    printf("Connected.\n");
    return s;
}

int NAS_IALreceive(int s)
{
  char str1[NAS_UE_NETL_MAXLEN];
  char str2[NAS_UE_NETL_MAXLEN];
  int i, t, done;
  struct nas_ue_netl_request *msgToRcve;
  struct nas_ue_netl_reply *msgToSend;

    done =0;
    t=recv(s, str1, NAS_UE_NETL_MAXLEN, 0);
    if (t <= 0) {
        if (t < 0) perror("IAL_process_command : recv");
        done = 1;
    }
    printf("\nmessage from RAL, length:  %d\n", t);

    msgToRcve = (struct nas_ue_netl_request *) str1;
    msgToSend = (struct nas_ue_netl_reply *) str2;
    memset(str2,0,NAS_UE_NETL_MAXLEN);
    switch (msgToRcve->type){
       case NAS_UE_MSG_CNX_ESTABLISH_REQUEST:
           printf("NAS_UE_MSG_CNX_ESTABLISH_REQUEST received\n");
					 msgToSend->type = NAS_UE_MSG_CNX_ESTABLISH_REPLY;
					 msgToSend->length = sizeof(struct nas_ue_netl_hdr)+sizeof(struct nas_ue_msg_cnx_establish_reply);
           msgToSend->ialNASPrimitive.cnx_est_rep.status = NAS_CONNECTED;
           state = NAS_CONNECTED;
           cell_id = msgToRcve->ialNASPrimitive.cnx_req.cellid;
           break;
       case NAS_UE_MSG_CNX_RELEASE_REQUEST:
           printf("NAS_UE_MSG_CNX_RELEASE_REQUEST received\n");
					 msgToSend->type = NAS_UE_MSG_CNX_RELEASE_REPLY;
					 msgToSend->length = sizeof(struct nas_ue_netl_hdr)+sizeof(struct nas_ue_msg_cnx_release_reply);
           msgToSend->ialNASPrimitive.cnx_rel_rep.status = NAS_DISCONNECTED;
           state = NAS_DISCONNECTED;
           cell_id = CONF_UNKNOWN_CELL_ID;
           break;
       case NAS_UE_MSG_CNX_LIST_REQUEST:
           printf("NAS_UE_MSG_CNX_LIST_REQUEST received\n");
					 msgToSend->type = NAS_UE_MSG_CNX_LIST_REPLY;
					 msgToSend->length = sizeof(struct nas_ue_netl_hdr)+sizeof(struct nas_ue_msg_cnx_list_reply);
           msgToSend->ialNASPrimitive.cnx_list_rep.state = state;
           msgToSend->ialNASPrimitive.cnx_list_rep.cellid = cell_id;
           msgToSend->ialNASPrimitive.cnx_list_rep.iid4 = CONF_iid4;
           msgToSend->ialNASPrimitive.cnx_list_rep.iid6[0] = conf_iid6_0[CONF_MT0];
           msgToSend->ialNASPrimitive.cnx_list_rep.iid6[1] = conf_iid6_1[CONF_MT0];
           if (state == NAS_CONNECTED){
               msgToSend->ialNASPrimitive.cnx_list_rep.num_rb = CONF_num_rb;
               msgToSend->ialNASPrimitive.cnx_list_rep.nsclassifier = CONF_num_rb;
           }
           break;
       case NAS_UE_MSG_CNX_STATUS_REQUEST:
           printf("NAS_UE_MSG_CNX_STATUS_REQUEST received\n");
					 msgToSend->type = NAS_UE_MSG_CNX_STATUS_REPLY;
					 msgToSend->length = sizeof(struct nas_ue_netl_hdr)+sizeof(struct nas_ue_msg_cnx_status_reply);
           msgToSend->ialNASPrimitive.cnx_stat_rep.status = state;
           msgToSend->ialNASPrimitive.cnx_stat_rep.cellid = cell_id;
           if (state == NAS_CONNECTED){
               msgToSend->ialNASPrimitive.cnx_stat_rep.num_rb = CONF_num_rb;
               msgToSend->ialNASPrimitive.cnx_stat_rep.signal_level = conf_level[0];
           }
           break;
       case NAS_UE_MSG_RB_LIST_REQUEST:
           printf("NAS_UE_MSG_RB_LIST_REQUEST received\n");
					 msgToSend->type = NAS_UE_MSG_RB_LIST_REPLY;
					 msgToSend->length = sizeof(struct nas_ue_netl_hdr)+sizeof(struct nas_ue_msg_rb_list_reply);
           if (state == NAS_CONNECTED){
              msgToSend->ialNASPrimitive.rb_list_rep.num_rb = CONF_num_rb;
              for (i=0; i<CONF_num_rb; i++){
                 msgToSend->ialNASPrimitive.rb_list_rep.RBList[i].rbId = conf_rbId[i];
                 msgToSend->ialNASPrimitive.rb_list_rep.RBList[i].QoSclass = conf_qoSclass[i];
                 msgToSend->ialNASPrimitive.rb_list_rep.RBList[i].dscp = conf_dscp[i];
              }
           }
           break;
       case NAS_UE_MSG_STATISTIC_REQUEST:
           printf("NAS_UE_MSG_STATISTIC_REQUEST received\n");
					 msgToSend->type = NAS_UE_MSG_STATISTIC_REPLY;
					 msgToSend->length = sizeof(struct nas_ue_netl_hdr)+sizeof(struct nas_ue_msg_statistic_reply);
           if (state == NAS_CONNECTED){
               msgToSend->ialNASPrimitive.statistics_rep.rx_packets = CONF_rx_packets;
               msgToSend->ialNASPrimitive.statistics_rep.tx_packets = CONF_tx_packets;
               msgToSend->ialNASPrimitive.statistics_rep.rx_bytes   = CONF_rx_bytes;
               msgToSend->ialNASPrimitive.statistics_rep.tx_bytes   = CONF_tx_bytes;
               msgToSend->ialNASPrimitive.statistics_rep.rx_errors  = CONF_rx_errors;
               msgToSend->ialNASPrimitive.statistics_rep.tx_errors  = CONF_tx_errors;
               msgToSend->ialNASPrimitive.statistics_rep.rx_dropped = CONF_rx_dropped;
               msgToSend->ialNASPrimitive.statistics_rep.tx_dropped = CONF_tx_dropped;
           }
           break;
       case NAS_UE_MSG_MEAS_REQUEST:
           printf("NAS_UE_MSG_MEAS_REQUEST received\n");
					 msgToSend->type = NAS_UE_MSG_MEAS_REPLY;
					 msgToSend->length = sizeof(struct nas_ue_netl_hdr)+sizeof(struct nas_ue_msg_measure_reply);
           msgToSend->ialNASPrimitive.meas_rep.num_cells = CONF_num_cells;
           for (i=0; i<CONF_num_rb; i++){
              msgToSend->ialNASPrimitive.meas_rep.measures[i].cell_id = conf_cell_id[i];
              msgToSend->ialNASPrimitive.meas_rep.measures[i].level = conf_level[i];
              msgToSend->ialNASPrimitive.meas_rep.measures[i].provider_id = conf_provider_id[i];
           }
           break;
       case NAS_UE_MSG_IMEI_REQUEST:
           printf("NAS_UE_MSG_IMEI_REQUEST received\n");
					 msgToSend->type = NAS_UE_MSG_IMEI_REPLY;
					 msgToSend->length = sizeof(struct nas_ue_netl_hdr)+sizeof(struct nas_ue_l2id_reply);
           msgToSend->ialNASPrimitive.l2id_rep.l2id[0] = conf_iid6_0[CONF_MT0];
           msgToSend->ialNASPrimitive.l2id_rep.l2id[1] = conf_iid6_1[CONF_MT0];
           state = NAS_DISCONNECTED;
           cell_id = CONF_UNKNOWN_CELL_ID;
           break;
       default:
         printf ("Invalid message Type %d\n",msgToRcve->type);
    }

    if (send(s, str2, msgToSend->length, 0) < 0) {
        perror("IAL_process_command : send");
        done = 1;
    }

    printf ("message sent to IAL %d\n",msgToSend->length);

    return done;

}

int main(void)
{
    int s = 0;
    int rc, done;
    fd_set readfds;
    struct timeval tv;

    do {
        s= NAS_IAL_sock_connect();
	if (s <= 0) {
		sleep(2);
        }
    } while (s < 0);

    done = 0;
    do {
        // Create fd_set and wait for input
        FD_ZERO(&readfds);
        FD_SET (s, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // timeout select for 100ms and read FIFOs

        rc = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);
        if (rc ==-1){
            perror("select");
            done = 1;
        }
        // something received !
        if (rc>=0){
            if (FD_ISSET(s,&readfds)){
                done = NAS_IALreceive(s);
            }
        }

    } while (!done);


    close(s);

    return 0;
}


