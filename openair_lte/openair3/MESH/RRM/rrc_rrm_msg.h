/*!
*******************************************************************************

\file    	rrc_msg.h

\brief   	Fichier d'entete contenant les declarations des types, des defines ,
			et des fonctions relatives aux messages RRC-RRM ou RRC-RRCI. 
			
			Les fonctions servent à créer le buffer de message, remplir 
			l'entete et copier les parametres de fonction. Chaque fonction 
			retourne le message qui pourra être envoye sur le socket entre le 
			CMM et le RRM ou RRCI.

\author  	BURLOT Pascal

\date    	17/07/08

   
\par     Historique:
			$Author$  $Date$  $Revision$
			$Id$
			$Log$

*******************************************************************************
*/
#ifndef __RRC_MSG_H
#define __RRC_MSG_H


#include "L3_rrc_defs.h"
#include "rrm_sock.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\brief  Enumeration des messages entre RRM/RRCI et le RRC
*/
typedef enum { 
	RRM_RB_ESTABLISH_REQ = 0 	, ///< Message RRM->RRC : requete d'etablissement d'un RB
	RRC_RB_ESTABLISH_RESP		, ///< Message RRC->RRM : reponse d'etablissement d'un RB
	RRC_RB_ESTABLISH_CFM		, ///< Message RRC->RRM : confirmation d'etablissement d'un RB
	RRM_RB_MODIFY_REQ		, ///< Message RRM->RRC : requete de modification d'un RB
	RRC_RB_MODIFY_RESP		, ///< Message RRC->RRM : reponse de modification d'un RB
	RRC_RB_MODIFY_CFM		, ///< Message RRC->RRM : confirmation de modification d'un RB	
	RRM_RB_RELEASE_REQ		, ///< Message RRM->RRC : requete de liberation d'un RB
	RRC_RB_RELEASE_RESP		, ///< Message RRC->RRM : reponse de liberation d'un RB
	RRC_MR_ATTACH_IND		, ///< Message RRC->RRM : indication d'attachement d'un MR
	RRM_SENSING_MEAS_REQ		, ///< Message RRM->RRC : requete de configuration de mesure sensing
	RRC_SENSING_MEAS_RESP		, ///< Message RRC->RRM : reponse de configuration de mesure sensing
	RRC_CX_ESTABLISH_IND		, ///< Message RRC->RRM : indication de connexion etablie
	RRC_PHY_SYNCH_TO_MR_IND		, ///< Message RRC->RRM : indication de synchronisation physique a un MR
	RRC_PHY_SYNCH_TO_CH_IND		, ///< Message RRC->RRM : indication de synchronisation physique a un CH
	RRCI_CX_ESTABLISH_RESP		, ///< Message RRCI->RRC : reponse de connexion etablie
	RRC_SENSING_MEAS_IND		, ///< Message RRC->RRM : indication de nouvel mesure de sensing 
	RRM_SENSING_MEAS_RESP		, ///< Message RRM->RRC : reponse a l'indication de nouvel mesure de sensing
	RRC_RB_MEAS_IND			, ///< Message RRC->RRM : indication de nouvel mesure sur un RB 
	RRM_RB_MEAS_RESP		, ///< Message RRM->RRC : reponse a l'indication de nouvel mesure sur un RB
	RRM_INIT_CH_REQ                 , ///<Message RRM->RRC : init d'un CH
	RRCI_INIT_MR_REQ                 , ///<Message RRM->RRC : init d'un MR
	NB_MSG_RRC_RRM 				  ///< Nombre de message RRM-RRC

} MSG_RRC_RRM_T ;


/*!
*******************************************************************************
\brief 	Definition des parametres de la fonction rrm_init_ch_req() dans 
        une structure permettant le passage des parametres via un socket
*/

typedef struct   { 
	unsigned int 		Trans_id			; //!< Transaction ID
	LCHAN_DESC 		Lchan_desc_srb0			; //!< Logical Channel Descriptor Array
	LCHAN_DESC 		Lchan_desc_srb1			; //!< Logical Channel Descriptor Array
	L2_ID 			L2_id   				; //!< Layer 2 (MAC) IDs for link
} rrm_init_ch_req_t ;


/*!
*******************************************************************************
\brief 	Definition des parametres de la fonction rrm_init_mr_req() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct   { 
	unsigned int 		Trans_id			; //!< Transaction ID
	LCHAN_DESC 		Lchan_desc_srb0			; //!< Logical Channel Descriptor Array
	LCHAN_DESC 		Lchan_desc_srb1			; //!< Logical Channel Descriptor Array
	unsigned char 		CH_index 			; //!< Layer 2 (MAC) IDs for CH
} rrci_init_mr_req_t ;



/*!
*******************************************************************************
\brief 	Definition des parametres de la fonction rrm_rb_establish_req() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct   { 
	unsigned int 		Trans_id			; //!< Transaction ID
        L2_ID 				L2_id[2] 			; //!< Layer 2 (MAC) IDs for link
	LCHAN_DESC 			Lchan_desc			; //!< Logical Channel Descriptor Array
	MAC_RLC_MEAS_DESC 	Mac_rlc_meas_desc	; //!< MAC/RLC Measurement descriptors for RB
	L3_INFO_T 			L3_info_t			; //!< Optional L3 Information
	unsigned char   	L3_info[MAX_L3_INFO]; //!< Type of L3 Information
} rrm_rb_establish_req_t ;

/*! 
*******************************************************************************
\brief 	Definition des parametres des fonctions suivantes dans une structure
        permettant le passage des parametres via un socket
			- rrc_rb_establish_resp(),
			- rrc_rb_modify_resp(),
			- rrc_rb_modify_resp(),
          	- rrc_rb_release_resp() 
          	- rrc_sensing_meas_resp()
*/
typedef struct {
	unsigned int 	Trans_id  	; //!< Transaction ID
} rrc_generic_resp_t  ;

/*! 
*******************************************************************************
\brief 	Definition des parametres de la fonction rrc_rb_establish_cfm() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
	unsigned int 	Trans_id  	; //!< Transaction ID
	RB_ID 			Rb_id		; //!< Radio Bearer ID used by RRC
	RB_TYPE			RB_type		; //!< Radio Bearer Type
} rrc_rb_establish_cfm_t 		;

/*! 
*******************************************************************************
\brief 	Definition des parametres de la fonction rrm_rb_modify_req() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct   { 
	unsigned int 		Trans_id		; //!< Transaction ID
	LCHAN_DESC 			Lchan_desc		; //!< Logical Channel Descriptor Array
	MAC_RLC_MEAS_DESC 	Mac_meas_desc	; //!< MAC/RLC Measurement descriptors for RB
	RB_ID 				Rb_id			; //!< Radio Bearer ID
} rrm_rb_modify_req_t ;

/*! 
*******************************************************************************
\brief 	Definition des parametres des fonctions rrc_rb_modify_cfm() et rrm_rb_release_req() 
        dans une structure  permettant le passage des parametres via un socket
*/
typedef struct {
	unsigned int 	Trans_id  	; //!< Transaction ID
	RB_ID 			Rb_id		; //!< Radio Bearer ID
} rrc_rb_modify_cfm_t 	,
  rrm_rb_release_req_t	;
  
/*! 
*******************************************************************************
\brief 	Definition des parametres de la fonction rrc_MR_attach_ind() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
	L2_ID 	L2_id  	   ; //!< Layer 2 (MAC) ID
} rrc_MR_attach_ind_t  ;

/*! 
*******************************************************************************
\brief 	Definition des parametres de la fonction rrm_sensing_meas_req() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
	L2_ID 				L2_id				; //!< Layer 2 (MAC) ID
	SENSING_MEAS_DESC 	Sensing_meas_desc	; //!< Sensing Measurement Descriptor
	unsigned int 		Trans_id 			; //!< Transaction ID          
} rrm_sensing_meas_req_t  ;

/*! 
*******************************************************************************
\brief 	Definition des parametres de la fonction rrc_cx_establish_ind() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
	L2_ID L2_id							; //!< Layer 2 (MAC) ID
	unsigned int Trans_id				; //!< Transaction ID
	L3_INFO_T L3_info_t					; //!< Type of L3 Information
	unsigned char L3_info[MAX_L3_INFO]	; //!< Optional L3 Information
	RB_ID DTCH_B_id						; //!< RBID of broadcast IP service (MR only)
	RB_ID DTCH_id						; //!< RBID of default IP service (MR only)
} rrc_cx_establish_ind_t ;	

/*! 
*******************************************************************************
\brief 	Definition des parametres de la fonction rrc_phy_synch_to_CH_ind() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
  unsigned int Ch_index 			; //!< Clusterhead index
  L2_ID L2_id 			; //!< L2_ID du MR
} rrc_phy_synch_to_CH_ind_t ;

/*! 
*******************************************************************************
\brief 	Definition des parametres de la fonction rrc_phy_synch_to_MR_ind() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
		L2_ID L2_id 			; //!< L2_ID du CH
} rrc_phy_synch_to_MR_ind_t ;


/*! 
*******************************************************************************
\brief 	Definition des parametres de la fonction rrci_cx_establish_resp() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
		unsigned int Trans_id 				; //!< Transaction ID
                L2_ID L2_id;
		unsigned char L3_info[MAX_L3_INFO]	; //!< Optional L3 Information
		L3_INFO_T L3_info_t   				; //!< Type of L3 Information
} rrci_cx_establish_resp_t ;

/*! 
*******************************************************************************
\brief 	Definition des parametres de la fonction rrc_sensing_meas_ind_t() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
	L2_ID 			L2_id			; //!< Layer 2 ID (MAC) of sensing node
	unsigned int 	NB_meas			; //!< Layer 2 ID (MAC) of sensing node
	unsigned int 	Trans_id		; //!< Transaction ID
	SENSING_MEAS_T  Sensing_meas[1]	; //!< first Sensing Information
} rrc_sensing_meas_ind_t ;	

/*! 
*******************************************************************************
\brief 	Definition des parametres de la fonction rrc_rb_meas_ind() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct { 
	unsigned int Trans_id			; //!< Transaction ID	
	RB_ID Rb_id						; //!< Radio Bearer ID
	L2_ID L2_id						; //!< Layer 2 (MAC) IDs for link
	MEAS_MODE Meas_mode				; //!< Measurement mode (periodic or event-driven)
	MAC_RLC_MEAS_T Mac_rlc_meas_t	; //!< MAC/RLC measurements
} rrc_rb_meas_ind_t;

#ifdef TRACE
extern const char *Str_msg_rrc_rrm[NB_MSG_RRC_RRM] ; 
#endif

/* Rappel : les paramètres sont identiques aux fonctions 
 *          (sans le prefixe msg_ ) du fichier: L3_rrc_interface.h
 */          

msg_t *msg_rrm_rb_establish_req( unsigned char inst, const LCHAN_DESC *Lchan_desc, const MAC_RLC_MEAS_DESC *Mac_rlc_meas_desc, 
								L2_ID *L2_id, unsigned int Trans_id, unsigned char *L3_info, L3_INFO_T L3_info_t );			  

msg_t *msg_rrm_rb_modify_req( unsigned char inst, const LCHAN_DESC 	*Lchan_desc, const MAC_RLC_MEAS_DESC *Mac_meas_desc, 
								RB_ID Rb_id, unsigned int Trans_id );

msg_t *msg_rrm_rb_release_req( unsigned char inst, RB_ID Rb_id, unsigned int Trans_id );

msg_t *msg_rrm_sensing_meas_req( unsigned char inst, L2_ID L2_id, SENSING_MEAS_DESC Sensing_meas_desc, unsigned int Trans_id ) ;

  msg_t *msg_rrci_cx_establish_resp( unsigned char inst, unsigned int Trans_id,L2_ID L2_id,unsigned char *L3_info, L3_INFO_T L3_info_t );

msg_t *msg_rrm_sensing_meas_resp( unsigned char inst, unsigned int Trans_id );


msg_t *msg_rrm_rb_meas_resp( unsigned char inst, unsigned int Trans_id );

msg_t *msg_rrm_init_ch_req( unsigned char inst, unsigned int Trans_id, const LCHAN_DESC *Lchan_desc_srb0, const LCHAN_DESC *Lchan_desc_srb1, 
                            L2_ID L2_id )  ;

msg_t *msg_rrci_init_mr_req( unsigned char inst, unsigned int Trans_id, const LCHAN_DESC *Lchan_desc_srb0, const LCHAN_DESC  *Lchan_desc_srb1, 
							unsigned char CH_index); 


#ifdef __cplusplus
}
#endif

#endif /* RRC_MSG_H */
