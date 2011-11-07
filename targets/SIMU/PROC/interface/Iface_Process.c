
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <err.h>
#include <getopt.h>
#include <paths.h>

#include "gm_update.h"
#include "../interface.h"
#include "definitions.h"

void Interface_init(int port,int node_id){
        s_t *st = calloc(1,sizeof(s_t));
	  	memset(st,0,sizeof(st));
	  	st->pointer = st;
	  	obj_ref_t2 *this_ref = calloc(1, sizeof(obj_ref_t2)); // generic obj
	  	this_ref->mem_ref.pointer          = st;
	  	Instance[1].gm=this_ref;
	  	s_t* st2=(s_t*)(Instance[1].gm->mem_ref.pointer);
		st2->Exec_Msg_Flag=0;
		st2->port=port;
		st2->node_id=node_id;
}

void send_exec_msg(int frame,int slot,int port){
	obj_ref_t          peer;
	int sock;
	msg_t2 msg;
	msg=Execute_Msg(frame,slot);
	CreatSocket(&peer,sock, "127.0.0.1",port);
	SEND_MESSAGE(&peer,"127.0.0.1", msg);
	close(peer.obj_ref.sock_inet_ref.sock_fd);
}

void send_exec_complete(int port){
	obj_ref_t          peer;
	int sock;
	msg_t2 msg;
	msg=Execute_Msg_Response();
	CreatSocket(&peer,sock, "127.0.0.1",port);
	SEND_MESSAGE(&peer,"127.0.0.1", msg);
	close(peer.obj_ref.sock_inet_ref.sock_fd);
}


