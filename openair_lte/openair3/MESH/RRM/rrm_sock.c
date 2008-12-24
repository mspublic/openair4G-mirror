/*!
*******************************************************************************

\file       rrm_sock.c

\brief      RRM (Radio Ressource Manager ) Socket

            Ceux sont les fonctions relatives à la communication avec les 
            autres entites: 
                - RRC , 
                - CMM , 
                - PUSU

\author     BURLOT Pascal

\date       10/07/08

   
\par     Historique:
            $Author$  $Date$  $Revision$
            $Id$
            $Log$

*******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "debug.h"
#include "L3_rrc_defs.h"
#include "rrm_util.h"
#include "rrm_sock.h"

//! \brief  Taille maximale de la charge utile
#define SIZE_MAX_PAYLOAD    2048

/*!
*******************************************************************************
\brief  This function opens a unix socket for the rrm communication
        ( no-connected mode / UDP DATAGRAM )
\return  The return value is a socket handle
*/
int open_socket( 
    sock_rrm_t *s   ,   ///< socket descriptor
    char *path_local,   ///< local socket path if unix socket
    char *path_dest ,   ///< host  Socket path if unix socket
    int rrm_inst        ///< instance of the rrm entity
    ) 
{ /* Unix socket */
    int     socket_fd ;
    int     len ;
        
    if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) 
    {
        perror("unix socket");
        return -1 ;
    }
        
    memset(&(s->un_local_addr), 0, sizeof(struct    sockaddr_un));
    s->un_local_addr.sun_family = AF_UNIX;
    sprintf(s->un_local_addr.sun_path,"%s%d", path_local, rrm_inst );
    unlink(s->un_local_addr.sun_path);
        
    len = strlen((s->un_local_addr).sun_path) + sizeof((s->un_local_addr).sun_family);
        
    if (bind(socket_fd, (struct sockaddr *)&(s->un_local_addr), len) == -1) 
    {
        perror("bind");
        return -1 ;
    }
        
    memset(&(s->un_dest_addr), 0, sizeof(struct     sockaddr_un));
    s->un_dest_addr.sun_family = AF_UNIX;
    sprintf(s->un_dest_addr.sun_path,"%s%d", path_dest, rrm_inst );
    
    s->s = socket_fd ;
    return socket_fd ; 
}

/*!
*******************************************************************************
\brief  This function closes a RRM socket 
\return none
*/
void close_socket( 
    sock_rrm_t *sock  ///< the socket handle 
    ) 
{
    shutdown(sock->s, SHUT_RDWR);
    close(sock->s);
}

/*!
*******************************************************************************
\brief  This function send a buffer message to the unix socket  
\return if OK then "0" is returned else "-1"
*/
int send_msg( 
    sock_rrm_t *s   ,///< socket descriptor
    msg_t *msg       ///< the message to send   
    ) 
{ /* Unix socket */
    int                 ret     = 0 ;
    char                *buf    = NULL;
    struct  msghdr      msghd ;
    struct  iovec       iov;
    int                 taille  = sizeof(msg_head_t)  ;
    
    if ( msg == NULL )
        return -1 ;
    
    if ( msg->data != NULL ) 
        taille += msg->head.size ;
        
    buf = RRM_MALLOC(char, taille);
    if (buf ==NULL) 
        ret =  -1 ; 
    else
    {
        memcpy( buf , &(msg->head) , sizeof(msg_head_t) ) ;
        memcpy( buf+sizeof(msg_head_t), msg->data, msg->head.size ) ;
        
        iov.iov_base      = (void *)buf;
        iov.iov_len       = taille ;
        
        msghd.msg_name          = (void *)&(s->un_dest_addr);
        msghd.msg_namelen       = sizeof(s->un_dest_addr);
        msghd.msg_iov           = &iov;
        msghd.msg_iovlen        = 1;            
        msghd.msg_control       = NULL ;
        msghd.msg_controllen    =   0 ;
                
        if ( sendmsg(s->s, &msghd, 0) < 0 )
        {
            ret = -1; 
            perror("sendmsg:unix socket");
        }
    }
    
    RRM_FREE(buf) ; 
    RRM_FREE(msg->data) ;
    RRM_FREE(msg) ;
    
    return ret ;
}

/*!
*******************************************************************************
\brief  This function read a buffer from a unix socket 
\return the function returns a message pointer. If the pointeur is NULL, a error
         is happened. 
*/
char *recv_msg( 
    sock_rrm_t *s   ///< socket descriptor
    ) 
{ /* Unix socket */
    char                *buf = NULL;
    char                *msg = NULL;
    struct  msghdr      msghd ;
    struct  iovec       iov;
    int                 size_msg ;
    msg_head_t          *head  ;
    int                 ret ;
    
    int taille =  SIZE_MAX_PAYLOAD ;

    buf                 = RRM_CALLOC( char,taille);
    if ( buf == NULL ) 
        return NULL ;
        
    iov.iov_base        = (void *)buf;
    iov.iov_len         = taille ;
    msghd.msg_name      = (void *)&(s->un_dest_addr);
    msghd.msg_namelen   = sizeof(s->un_dest_addr);
    msghd.msg_iov       = &iov;
    msghd.msg_iovlen    = 1;    
    msghd.msg_control   = NULL ;
    msghd.msg_controllen= 0 ;       

    ret = recvmsg(s->s, &msghd , 0 ) ; 
    if ( ret <= 0  )
    {
        perror("PB recvmsg_un");
        RRM_FREE(buf);
        return NULL ;
    }
    
    if (msghd.msg_flags != 0 )
    {
        fprintf(stderr,"error recvmsg_un: 0x%02x\n", msghd.msg_flags) ;
        RRM_FREE(buf);          
        return NULL ;       
    }

    head        = (msg_head_t *) buf  ;
    size_msg    = sizeof(msg_head_t) + head->size ;
    
    msg         = RRM_CALLOC(char , size_msg ) ;
    if ( msg != NULL )
        memcpy( msg , buf , size_msg ) ;
        
    RRM_FREE( buf ) ;
    
    return msg ;
}
