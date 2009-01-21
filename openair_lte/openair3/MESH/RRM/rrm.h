/*!
*******************************************************************************

\file       rrm.h

\brief      Fichier d'entete contenant les declarations des types, des defines ,
            et des fonctions relatives aux fonctions du RRM (Radio Resource Management ).

\author     BURLOT Pascal

\date       15/07/08

   
\par     Historique:
        P.BURLOT 2009-01-20 
            + separation de la file de message CMM/RRM a envoyer en 2 files 
              distinctes ( file_send_cmm_msg, file_send_rrc_msg)

*******************************************************************************
*/

#ifndef RRM_H
#define RRM_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\brief Nombre max. d'instance de RRM (Emulation)        
*/
#define MAX_RRM     10

/*!
*******************************************************************************
\brief Structure definissant une instance RRM       
*/


typedef struct {
    int  id                                 ; ///< identification de l'instance RRM
    
    enum { 
        ISOLATEDNODE=0  , ///< Node is in a isolated State
        CLUSTERHEAD_INIT, ///< Node is in a Cluster Head initialization State 
        CLUSTERHEAD     , ///< Node is in a Cluster Head State 
        MESHROUTER        ///< Node is in a Mesh Router State
        } state                             ; ///< etat de l'instance
    
    L2_ID               L2_id               ; ///< identification de niveau L2            
    L3_INFO_T           L3_info_t           ; ///< type de l'identification de niveau L3       

    unsigned char       L3_info[MAX_L3_INFO]; ///< identification de niveau L3   
    
    file_head_t         file_send_cmm_msg   ; ///< File des messages en emission
    file_head_t         file_send_rrc_msg   ; ///< File des messages en emission
    file_head_t         file_recv_msg       ; ///< File des messages en reception
    
    struct {
        sock_rrm_t      *s                  ; ///< Socket associé a l'interface CMM
        unsigned int    trans_cnt           ; ///< Compteur de transaction avec l'interface CMM
        transact_t     *transaction         ; ///< liste des transactions non terminees
        pthread_mutex_t exclu               ; ///< mutex pour le partage de structure


	} cmm 									; ///<  info relatif a l'interface CMM
	
	struct {
#ifdef TRACE    
		FILE *fd 							; ///< Fichier pour trace de debug : action RRM->RRC
#endif


		sock_rrm_t  	*s 					; ///< Socket associé a l'interface RRC
		unsigned int 	trans_cnt 			; ///< Compteur de transaction avec l'interface RRC
		transact_t 	    *transaction		; ///< liste des transactions non terminees
		pthread_mutex_t exclu				; ///< mutex pour le partage de structure

		neighbor_desc_t *pNeighborEntry 	; ///< Descripteur sur le voisinage
		RB_desc_t 		*pRbEntry 			; ///< Descripteur sur les RB (radio bearer) ouverts
	} rrc 									; ///<  info relatif a l'interface rrc
	
	struct {
        sock_rrm_t      *s                  ; ///< Socket associé a l'interface PUSU
        unsigned int    trans_cnt           ; ///< Compteur de transaction avec l'interface PUSU
        transact_t     *transaction        ; ///< liste des transactions non terminees
        pthread_mutex_t exclu               ; ///< mutex pour le partage de structure
	} pusu 									; ///<  info relatif a l'interface pusu
	

} rrm_t ;

extern rrm_t rrm_inst[MAX_RRM] ;
extern int   nb_inst ;

#ifdef __cplusplus
}
#endif

#endif /* RRM_H */
