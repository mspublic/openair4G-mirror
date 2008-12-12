/*!
*******************************************************************************

\file    	rrm_sock.h

\brief   	Fichier d'entete contenant les declarations des types, des defines ,
			et des fonctions relatives aux fonctions de communication RRM 
			(Radio Resource Management ) vec les autres entites RRC/CMM/PUSU.

\author  	BURLOT Pascal

\date    	15/07/08

   
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
\brief	 Entete des messages de RRM/CMM/RRC
*/
typedef struct {
	unsigned short start    ; ///< Identification du debut de message
	unsigned char  inst     ; ///< Identification de l'instance RRM
	unsigned char  msg_type ; ///< Identification du type message
	unsigned int   size     ; ///< taille du message
	unsigned int   Trans_id ; ///< Identification de la transaction
} msg_head_t ;

/*!
*******************************************************************************
\brief	 Definition de la structure d'un message a envoyer sur un socket:
         	- RRM->RRC
         	- RRC->RRM
         	- RRCI->RRC
         	- RRC->RRCI
         	- CMM->RRM
         	- RRM->CMM
*/
typedef struct {
	msg_head_t 	head  ; ///< entete du message
	char 		*data ; ///< message
} msg_t ;

/*!
*******************************************************************************
\brief  entete du message vers le PuSu
*/
typedef struct   { 
	unsigned char type	; ///< type du message
	unsigned char inst  ; ///< Identification de l'instance RRM
	unsigned int  length; ///< taille du message
} msg_pusu_head_t ;

/*!
*******************************************************************************
\brief  Definition de la structure d'un message a envoyer sur le socket vers le
    	PuSu
*/
typedef struct {
	msg_pusu_head_t  head ; ///< entete du message
	void 			*data ; ///< message
} msg_pusu_t;

/*!
*******************************************************************************
\brief  Definition de la structure definissant le socket pour envoyer les messages
*/
typedef struct {
	int s 									; ///< identification du socket	
	struct 	sockaddr_un un_local_addr 		; ///< Adresse local si unix socket
	struct 	sockaddr_un un_dest_addr 		; ///< Adresse destinataire si unix socket
} sock_rrm_t ;

/* *** Fonctions relatives aux interfaces CMM ou RRC *** */
int open_socket( sock_rrm_t *s 	,char *path_local, char *path_dest , int rrm_inst ) ;
void close_socket(sock_rrm_t *sock ) ;
int send_msg(sock_rrm_t *s 	,msg_t *msg ) ;
char *recv_msg( sock_rrm_t *s ) ;	
	
/* *** Fonctions relatives a l'interface PUSU *** */


int open_pusu_sock( char *path,int rrm_inst ) ;
void close_pusu_sock( int sock  ) ;

int connect_to_pusu_sock( char *path, int rrm_inst) ;	
void disconnect_to_pusu_sock(int sock ) ;
int send_msg_pusu( int sock , msg_pusu_t *msg ) ;
	
#ifdef __cplusplus
}
#endif

#endif /* RRM_SOCK_H */
