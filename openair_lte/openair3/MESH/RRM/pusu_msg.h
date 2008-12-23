/*!
*******************************************************************************

\file    	pusu_msg.h

\brief   	Fichier d'entete contenant les declarations des types, des defines ,
			et des fonctions relatives aux messages RRM->PUSU. 
			
			Les fonctions servent à créer le buffer de message, remplir l'entete 
			et copier les parametres de fonction. Chaque fonction retourne le 
			message qui pourra être envoye sur le socket.

\author  	BURLOT Pascal

\date    	29/08/08

   
\par     Historique:
			$Author$  $Date$  $Revision$
			$Id$
			$Log$

*******************************************************************************
*/

#ifndef PUSU_MSG_H
#define PUSU_MSG_H

#ifdef __cplusplus
extern "C" {
#endif

//! Definition de la taille de l'identification niveau 2
#define IEEE80211_MAC_ADDR_LEN 8


/*!
*******************************************************************************
\brief  Enumeration des messages entre le RRM et PUSU 
*/
typedef enum { 
	RRM_PUBLISH_IND = 0 	, ///< Annonce de nouveau service
	PUSU_PUBLISH_RESP 		, ///< Reponse de PUSU_PUBLISH_IND
	RRM_UNPUBLISH_IND  	    , ///< Annonce de retrait d'un service
	PUSU_UNPUBLISH_RESP  	, ///< Reponse de PUSU_UNPUBLISH_IND 
	RRM_LINK_INFO_IND   	, ///< Info sur un lien (oriente)
	PUSU_LINK_INFO_RESP   	, ///< Reponse de PUSU_LINK_INFO_IND 
	RRM_SENSING_INFO_IND   	, ///< Info sur un voisin (oriente)
	PUSU_SENSING_INFO_RESP  , ///< Reponse de RRM_SENSING_INFO_IND 
	RRM_CH_LOAD_IND   		, ///< Info sur la charge d'un cluster 
	PUSU_CH_LOAD_RESP   	, ///< Reponse de PUSU_CH_LOAD_IND
	NB_MSG_RRM_PUSU			  ///< Nombre de message de l'interface
} MSG_RRM_PUSU_T ;

/*!
*******************************************************************************
\brief  Enumeration des services entre le RRM et PUSU 
*/
typedef enum { 
	PUSU_RRM_SERVICE = 0xAA 	 ///< service d'un information RRM vers le TRM
} SERVICE_RRM_PUSU_T ;

/*!
*******************************************************************************
\brief 	Definition de publish/unpublish
*/
typedef struct {
	int service_type ;  ///< type de service
} pusu_publish_t ,  pusu_unpublish_t ;

/*!
*******************************************************************************
\brief 	Definition des informations remontees au TRM relatives a un lien
*/
typedef struct {
  L2_ID         noeud1		 ;  ///< L2_ID du noeud 1
  L2_ID         noeud2	     ;  ///< L2_ID du noeud 2
  RB_ID         rb_id        ;  ///< identification du lien              
  unsigned char rssi 		 ;  ///< metrique RSSI              
  unsigned char spec_eff     ;  ///< metrique Efficacite spectrale
} pusu_link_info_t;

/*!
*******************************************************************************
\brief 	Definition des informations remontees au TRM relatives a un voisin
        sans notion de lien etabli obligatoirement
*/
typedef struct {
  L2_ID         noeud1		 ;  ///< L2_ID du noeud 1
  L2_ID         noeud2	     ;  ///< L2_ID du noeud 2
  unsigned char rssi 		 ;  ///< metrique RSSI              
} pusu_sensing_info_t;

/*!
*******************************************************************************
\brief 	Definition de la structure d'information de charge du cluster remontee
        au TRM  
*/
typedef struct {
  unsigned char load 		 ;  ///< la charge du cluster              
} pusu_ch_load_t;

#ifdef TRACE
extern const char *Str_msg_pusu_rrm[NB_MSG_RRM_PUSU] ;
#endif

msg_t *msg_rrm_publish_ind(unsigned char inst, int service, unsigned int Trans_id  );
msg_t *msg_rrm_unpublish_ind(unsigned char inst, int service, unsigned int Trans_id  );
msg_t *msg_rrm_link_info_ind(unsigned char inst, L2_ID noeud1, L2_ID noeud2, RB_ID rb_id, unsigned char rssi ,unsigned char spec_eff, unsigned int Trans_id ) ;
msg_t *msg_rrm_sensing_info_ind(unsigned char inst, L2_ID noeud1, L2_ID noeud2, unsigned char rssi, unsigned int Trans_id ) ;
msg_t *msg_rrm_ch_load_ind(unsigned char inst, unsigned char load, unsigned int Trans_id  );
msg_t *msg_pusu_resp(unsigned char inst, MSG_RRM_PUSU_T response, unsigned int Trans_id );

#ifdef __cplusplus
}
#endif

#endif /* PUSU_MSG_H */
