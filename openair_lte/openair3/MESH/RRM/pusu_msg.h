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
\brief 	Definition de la valeur encode
*/
typedef struct encoded_value{
	unsigned char encoded_type ;  ///< type de la valeur
	void        * value        ;  ///< pointeur sur la valeur encode               
} encoded_value_t ;

/*!
*******************************************************************************
\brief 	Definition de la metrique de QoS
*/
typedef struct QoS_metric{
	unsigned char 		type			;  ///< Type de metrique      
	unsigned int 		length			;  ///< Nombre de metrique     
	encoded_value_t *	encoded_value	;  ///< Metriques  
} QoS_metric_t ;

/*!
*******************************************************************************
\brief 	Definition de la structure de donnee remontee au PUSU
*/
typedef struct neighbor_entry_RRM_to_CMM{
	char mac1[IEEE80211_MAC_ADDR_LEN+1]	; ///< Adresse MAC1	
	char mac2[IEEE80211_MAC_ADDR_LEN+1] ; ///< Adresse MAC2 	
	int number_metric_uplink			; ///< Nombre de metrique pour le lien montant
	int number_metric_downlink			; ///< Nombre de metrique pour le lien descendant
	QoS_metric_t **  metric_uplink		; ///< metriques QoS pour le lien montant
	QoS_metric_t **  metric_downlink	; ///< metriques QoS pour le lien descendant
} neighbor_entry_RRM_to_CMM_t ;

/*
 *  **************************************************************************
 */

//! \brief encoded_type pour la metrique RRSI
#define RSSI_ENCODED_TYPE    0x11
//! \brief type pour la metrique de voisinage
#define NEIGHBOR_METRIC_TYPE 0x22

/*!
*******************************************************************************
\brief 	Definition de la metrique RSSI
*/
typedef struct {
	unsigned char encoded_type;  ///< Type de metrique (=RSSI_ENCODED_TYPE)
	unsigned char rssi 		  ;  ///< la metrique RSSI              
} rssi_value_t ;

/*!
*******************************************************************************
\brief 	Definition de la metrique de voisinage
*/
typedef struct {
	unsigned char 	type	; ///< type de metrique  (=NEIGHBOR_METRIC_TYPE)    
	unsigned int 	length	; ///< Nombre de metriques     
	rssi_value_t 	value   ; ///< la metrique RSSI
} neighbor_metric_t ;

/*!
*******************************************************************************
\brief 	Definition du message de mesure de voisinage entre 2 MR envoye au PUSU
*/
typedef struct {
		char mac1[IEEE80211_MAC_ADDR_LEN+1]  ; ///< Adresse MAC1	 	
		char mac2[IEEE80211_MAC_ADDR_LEN+1]  ; ///< Adresse MAC2 	 	
		int number_metric_uplink             ; ///< Nombre de metrique pour le lien montant
		int number_metric_downlink           ; ///< Nombre de metrique pour le lien descendant (=0)
		neighbor_metric_t neighbor_metric[2] ; ///< metriques QoS pour le lien montant
} rrm_neighbor_meas_ind_t ;                  

typedef struct {
		char mac1[IEEE80211_MAC_ADDR_LEN+1]  ; ///< Adresse MAC1 	
		char mac2[IEEE80211_MAC_ADDR_LEN+1]  ; ///< Adresse MAC2  	
		int number_metric_uplink             ; ///< Nombre de metrique pour le lien montant (=0)
		int number_metric_downlink           ; ///< Nombre de metrique pour le lien descendant (=0:attach/1=detach)
} rrm_mr_attach_ind_t ;

msg_pusu_t *msg_rrm_neighbor_meas_ind(unsigned char inst, L2_ID L2_id1,unsigned char rssi1, L2_ID L2_id2, unsigned char rssi2 );
msg_pusu_t *msg_rrm_mr_attach_ind(unsigned char inst,L2_ID L2_id_ch, L2_ID L2_id_mr, int flag_attach  );

#ifdef __cplusplus
}
#endif

#endif /* PUSU_MSG_H */
