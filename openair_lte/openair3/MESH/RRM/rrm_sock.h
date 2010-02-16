/*!
*******************************************************************************

\file       rrm_sock.h

\brief      Fichier d'entete contenant les declarations des types, des defines ,
            et des fonctions relatives aux fonctions de communication RRM 
            (Radio Resource Management ) avec les autres entites RRC/CMM/PUSU.

\author     BURLOT Pascal

\date       15/07/08

   
\par     Historique:
            $Author$  $Date$  $Revision$
            $Id$
            $Log$

*******************************************************************************
*/

#ifndef RRM_SOCK_H
#define RRM_SOCK_H





#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>   //mod_lor_10_01_25
#include <netdb.h>   //mod_lor_10_01_25


#ifdef __cplusplus
extern "C" {
#endif

//! \brief Socket path associated to RRM-CMM interface      
#define RRM_CMM_SOCK_PATH "/tmp/rrm_cmm_socket"
//! \brief Socket path associated to CMM-RRM interface      
#define CMM_RRM_SOCK_PATH "/tmp/cmm_rrm_socket"

//! \brief Socket path associated to RRM-RRC interface  
#define RRM_RRC_SOCK_PATH "/tmp/rrm_rrc_socket"
//! \brief Socket path associated to RRC-RRM interface      
#define RRC_RRM_SOCK_PATH "/tmp/rrc_rrm_socket"

//! \brief Socket path associated to RRM-PUSU interface     
#define RRM_PUSU_SOCK_PATH "/tmp/rrm_pusu_socket"
//! \brief Socket path associated to PUSU-RRM interface     
#define PUSU_RRM_SOCK_PATH "/tmp/pusu_rrm_socket"


//! \brief Identification of the RRM/CMM/RRC message begin      
#define START_MSG      0xA533
//! \brief Identification of the PUSU message begin         
#define START_MSG_PUSU 0xCC



/*!
*******************************************************************************
\brief   Entete des messages de RRM/CMM/RRC
*/
typedef struct {
    unsigned short start    ; ///< Identification du debut de message
    Instance_t     inst     ; ///< Identification de l'instance RRM
    unsigned char  msg_type ; ///< Identification du type message
    unsigned int   size     ; ///< Taille du message
    Transaction_t  Trans_id ; ///< Identification de la transaction
} msg_head_t ;

/*!
*******************************************************************************
\brief   Definition de la structure d'un message a envoyer sur un socket:
            - RRM->RRC
            - RRC->RRM
            - RRCI->RRC
            - RRC->RRCI
            - CMM->RRM
            - RRM->CMM
*/
typedef struct {
    msg_head_t  head  ; ///< entete du message
    char        *data ; ///< message
} msg_t ;


/*!
*******************************************************************************
\brief  Definition de la structure definissant le socket pour envoyer les messages
*/
typedef struct {
    int s                                   ; ///< identification du socket 
    struct  sockaddr_un un_local_addr       ; ///< Adresse local si unix socket
    struct  sockaddr_un un_dest_addr        ; ///< Adresse destinataire si unix socket
} sock_rrm_t ;

//mod_lor_10_01_25++
typedef struct {
    int s                                   ; ///< identification du socket 
    struct  sockaddr_in in_local_addr       ; ///< Adresse local si internet socket
    struct  sockaddr_in in_dest_addr        ; ///< Adresse destinataire si internet socket
} sock_rrm_int_t ;
//mod_lor_10_01_25--


int   open_socket( sock_rrm_t *s  ,char *path_local, char *path_dest , int rrm_inst ) ;
void  close_socket(sock_rrm_t *sock ) ;
int   send_msg(sock_rrm_t *s  ,msg_t *msg ) ;
char *recv_msg( sock_rrm_t *s ) ;   

int   open_socket_int( sock_rrm_int_t *s  , unsigned char *path_local, int local_port, unsigned char *path_dest , int dest_port, int rrm_inst ) ;   //mod_lor_10_01_25
int   send_msg_int(sock_rrm_int_t *s  ,msg_t *msg  ) ; //mod_lor_10_01_25
char *recv_msg_int( sock_rrm_int_t *s ) ;  //mod_lor_10_01_25
void  close_socket_int(sock_rrm_int_t *sock ) ;  //mod_lor_10_01_25
        
#ifdef __cplusplus
}
#endif


#endif /* RRM_SOCK_H */
