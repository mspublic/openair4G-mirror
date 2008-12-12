/*!
*******************************************************************************

\file    	rb_mngt.c

\brief   	Fonctions permettant la gestion des radio bearers du cluster par 
			le cluster head

\author  	BURLOT Pascal

\date    	29/08/08

   
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

#include <pthread.h>

#include "debug.h"
#include "L3_rrc_defs.h"
#include "L3_rrc_interface.h"
#include "cmm_rrm_interface.h"
#include "rrm_sock.h"
#include "rrc_rrm_msg.h"
#include "cmm_msg.h"
#include "msg_mngt.h"
#include "rb_db.h"
#include "neighbor_db.h"
#include "rrm_util.h"
#include "transact.h"
#include "rrm_constant.h"
#include "rrm.h"
#include "rb_mngt.h"

//! Met un message dans la file des messages a envoyer
#define PUT_MSG(s,m)  put_msg( 	&(rrm->file_send_msg),s,m) 
/*
 *  =========================================================================
 *  OUVERTURE D'UN RADIO BEARER
 *  =========================================================================
 */

/*!
*******************************************************************************
 \brief CMM connection setup request.  Only in CH.
*/
int cmm_cx_setup_req(
			unsigned char  inst,    //!< Identification de l'instance
			L2_ID Src,             //!< L2 source MAC address
		    L2_ID Dst,             //!< L2 destination MAC address
		    QOS_CLASS_T QoS_class, //!< QOS class index
		    unsigned int Trans_id  //!< Transaction ID
		    )
{
	int ret = -1 ;
	rrm_t *rrm = &rrm_inst[inst] ; /// \todo rrm à passer en parametre
	
	if ( rrm->state == CLUSTERHEAD )
	{
		L2_ID src_dst[2] ;
		
		pthread_mutex_lock( &( rrm->cmm.exclu ) ) ;
		add_item_transact( &(rrm->cmm.transaction), Trans_id,INT_CMM,CMM_CX_SETUP_REQ, 0,NO_PARENT);
		pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
		
		/** \todo Evaluer si le RB peut etre cree avant d'envoyer la commande au RRC */
		
		pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
		rrm->rrc.trans_cnt++ ;
		add_item_transact( &(rrm->rrc.transaction), rrm->rrc.trans_cnt ,INT_RRC,RRM_RB_ESTABLISH_REQ,Trans_id,PARENT);
		add_rb( &(rrm->rrc.pRbEntry), rrm->rrc.trans_cnt, QoS_class, &src_dst[0] ) ;
		pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
		
		memcpy(&src_dst[0], &Src, sizeof(L2_ID)) ;
		memcpy(&src_dst[1], &Dst, sizeof(L2_ID)) ;
		
		PUT_MSG( 	rrm->rrc.s, 
					msg_rrm_rb_establish_req(inst,				 
						&Lchan_desc[QoS_class], 
						&Mac_rlc_meas_desc[QoS_class],
						&src_dst[0] ,
						rrm->rrc.trans_cnt,
						NULL,NONE_L3)	
					) ;

		if(QoS_class == QOS_DTCH_D){//faire le srb2 seulement a l'attachement (ouverture du DTCH IP par Defaut)
		  pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
		  rrm->rrc.trans_cnt++ ;
		  add_item_transact( &(rrm->rrc.transaction), rrm->rrc.trans_cnt ,INT_RRC,RRM_RB_ESTABLISH_REQ,0,NO_PARENT);
		  add_rb( &(rrm->rrc.pRbEntry), rrm->rrc.trans_cnt, QoS_class, &src_dst[0] ) ;
		  pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
		  
		  
		  PUT_MSG( 	rrm->rrc.s, 
				msg_rrm_rb_establish_req(inst,				 
							 &Lchan_desc[QOS_SRB2], 
							 &Mac_rlc_meas_desc[QOS_SRB2],
							 &src_dst[0] ,
							 rrm->rrc.trans_cnt,
							 rrm->L3_info,rrm->L3_info_t)	
				) ;
		}
		  ret = 0 ;
	}
	
	else
	{
		if ( rrm->state == CLUSTERHEAD_INIT)	
		{
			L2_ID src_dst[2] ;
			
			pthread_mutex_lock( &( rrm->cmm.exclu ) ) ;
			add_item_transact( &(rrm->cmm.transaction), Trans_id, INT_CMM, CMM_CX_SETUP_REQ,0,NO_PARENT);
			pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
			
			/** \todo Evaluer si le RB peut etre cree avant de solliciter le RRC */
			
			pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;

			memcpy(&src_dst[0], &Src, sizeof(L2_ID)) ;
			memcpy(&src_dst[1], &Dst, sizeof(L2_ID)) ;

			src_dst[1].L2_id[0]=0;
			src_dst[1].L2_id[1]=0;			//memset( &src_dst[0], 0, 2*sizeof(L2_ID)) ;

			rrm->rrc.trans_cnt++ ;
			PUT_MSG(    rrm->rrc.s, 
						msg_rrm_rb_establish_req(inst,					 
							&Lchan_desc[QOS_DTCH_B], 
							&Mac_rlc_meas_desc[QOS_DTCH_B],
							&src_dst[0] ,
							rrm->rrc.trans_cnt,
							rrm->L3_info,rrm->L3_info_t)
						) ;
				
			add_item_transact( &(rrm->rrc.transaction), rrm->rrc.trans_cnt ,INT_RRC,RRM_RB_ESTABLISH_REQ,Trans_id,PARENT);

			add_rb( &(rrm->rrc.pRbEntry), rrm->rrc.trans_cnt, QoS_class, &src_dst[0] ) ;

			pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
			
			ret = 0 ;
		}
		else
		fprintf(stderr,"[RRM] CMM_CX_SETUP_REQ (%d) is not allowed (Only CH):etat=%d\n", Trans_id, rrm->state);	
	}
		
	return ret ;
}

/*!
*******************************************************************************
\brief RRC response to rb_establish_req.  RRC Acknowledgement of reception of 
       rrc_rb_establishment_req.
*/
void rrc_rb_establish_resp(
		unsigned char  inst,    //!< Identification de l'instance
		unsigned int Trans_id       			//!< Transaction ID
		)
{
	rrm_t *rrm = &rrm_inst[inst] ; /// \todo rrm à passer en parametre
	
	if ( (rrm->state == CLUSTERHEAD) 
		|| (rrm->state == CLUSTERHEAD_INIT)
		)
	{
		pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
		set_ttl_transact(rrm->rrc.transaction, Trans_id, TTL_DEFAULT_VALUE ) ;
		pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;		
	}
	else
		fprintf(stderr,"[RRM] RRC_RB_ESTABLISH_RESP (%d) is not allowed (Only CH):etat=%d\n",Trans_id,rrm->state);	
}
/*!
*******************************************************************************
\brief Liberation d'un radio bearer
 */
static void rb_release_req( 
	unsigned char  inst         , ///< Identification de l'instance
	rrm_t *rrm 					, ///< pointeur sur l'instance du RRM
	RB_ID Rb_id					, ///< Id du RB a liberer
	unsigned int parentTransact	, ///< Transaction parent  ( a l'origne de la requete )
	unsigned int status_parent    ///< Status du parent
	)
{
	rrm->rrc.trans_cnt++ ;					
	PUT_MSG( rrm->rrc.s, msg_rrm_rb_release_req(inst,Rb_id,	rrm->rrc.trans_cnt) );
	add_item_transact( &(rrm->rrc.transaction), rrm->rrc.trans_cnt, INT_RRC, RRM_RB_RELEASE_REQ, parentTransact,status_parent);
}

/*!
*******************************************************************************
\brief RRC confirmation of rb_establish_req.  RRC confirmation of 
		rrc_rb_establishment_req after transactions are complete.  
		Essentially for CH only (except SRB0/1)
*/
void rrc_rb_establish_cfm(
	unsigned char  inst,    //!< Identification de l'instance
	RB_ID Rb_id,                  //!< Radio Bearer ID used by RRC
	RB_TYPE RB_type,              //!< Radio Bearer Type
	unsigned int Trans_id         //!< Transaction ID
	)
{
	rrm_t *rrm = &rrm_inst[inst] ; /// \todo rrm à passer en parametre
	
	if ( (rrm->state == CLUSTERHEAD) 
		|| (rrm->state == CLUSTERHEAD_INIT)
		)
	{
		transaction_t *pTransact ;

		pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
		pTransact = get_item_transact(rrm->rrc.transaction,Trans_id ) ;
		if ( pTransact == NULL )
		{
			fprintf(stderr,"[RRM] rrc_rb_establish_cfm (%d) unknown transaction\n",Trans_id);
			// comme la transaction est inconnue, on libere immediatement le 
			// tuyau nouvellement  cree (au niveau du RRC uniquement)
			rb_release_req( inst, rrm,Rb_id,0,NO_PARENT );
			pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
		}
		else
		{
			unsigned int parent_id = pTransact->parent_id ;
			unsigned int status_parent = pTransact->parent_status ;
			
			update_rb_desc_(rrm->rrc.pRbEntry, Trans_id, Rb_id, RB_type );
			del_item_transact( &(rrm->rrc.transaction),Trans_id ) ;
			pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
			
			
			if ( status_parent )
			{
				transaction_t *pTransactParent;
				
				pthread_mutex_lock( &( rrm->cmm.exclu ) ) ;
				pTransactParent = get_item_transact(rrm->cmm.transaction,parent_id ) ;
				if ( pTransactParent != NULL )
				{
					PUT_MSG( rrm->cmm.s, msg_rrm_cx_setup_cnf(inst,Rb_id,pTransactParent->id ));
					
					if ( rrm->state == CLUSTERHEAD_INIT ) rrm->state = CLUSTERHEAD ;
					
					del_item_transact( &(rrm->cmm.transaction),pTransactParent->id ) ;
					pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
				}
				else
				{	// comme la transaction parent est inconnue, on libere 
					// immediatement le  tuyau nouvellement  cree (au niveau 
					// du RRC uniquement)
					pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
					
					pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
					rb_release_req( inst, rrm,Rb_id, 0, NO_PARENT );
					del_rb_by_rbid( &(rrm->rrc.pRbEntry), Rb_id ) ;
					pthread_mutex_unlock( &( rrm->rrc.exclu ) );
				}
			}
			pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
		}	
	}
	else
		fprintf(stderr,"[RRM] RRC_RB_ESTABLISH_CFM (%d) is not allowed (Only CH):etat=%d\n",Trans_id,rrm->state);		
}

/*
 *  =========================================================================
 *  MODICATION D'UN RADIO BEARER
 *  =========================================================================
 */

/*!
*******************************************************************************
\brief CMM connection modify request.  Only in CH.

*/
int cmm_cx_modify_req(
			unsigned char  inst,    //!< Identification de l'instance
			RB_ID Rb_id ,           //!< L2 Rb_id
		    QOS_CLASS_T QoS_class, //!< QOS class index
		    unsigned int Trans_id  //!< Transaction ID
		    )
{
	int r ;
	int ret = -1 ;
	rrm_t *rrm = &rrm_inst[inst] ; /// \todo rrm à passer en parametre
	
	if ( rrm->state == CLUSTERHEAD )
	{
		RB_desc_t *pRb ;
		
		pthread_mutex_lock( &( rrm->cmm.exclu ) ) ;
		add_item_transact( &(rrm->cmm.transaction), Trans_id,INT_CMM,CMM_CX_MODIFY_REQ,0,NO_PARENT);
		pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
		
		/** \todo Evaluer si le RB peut etre modifier avant de solliciter le RRC */
		pRb = (RB_desc_t *) get_rb_desc_by_rbid( rrm->rrc.pRbEntry, Rb_id ) ;
		if ( pRb != NULL )
		{
			pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
			rrm->rrc.trans_cnt++ ;
					
			PUT_MSG( rrm->rrc.s, 
						msg_rrm_rb_modify_req( inst,
							&Lchan_desc[QoS_class], 
							&Mac_rlc_meas_desc[QoS_class],
							Rb_id,
							rrm->rrc.trans_cnt)
						) ;
			WARNING(r!=0);
				
			add_item_transact( &(rrm->rrc.transaction), rrm->rrc.trans_cnt ,INT_RRC,RRM_RB_MODIFY_REQ,Trans_id,PARENT);
			
			add_rb( &(rrm->rrc.pRbEntry), rrm->rrc.trans_cnt, QoS_class, pRb->L2_id ) ;
		}
					
		pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
		
		ret = 0 ;
	}
	else
		fprintf(stderr,"[RRM] CMM_CX_MODIFY_REQ (%d) is not allowed (Only CH):etat=%d\n",Trans_id,rrm->state);	
		
	return ret ;
}
/*!
*******************************************************************************
\brief RRC response to rb_modify_req
*/
void rrc_rb_modify_resp(
		unsigned char  inst,    //!< Identification de l'instance
		unsigned int Trans_id       			//!< Transaction ID
		)
{
	rrm_t *rrm = &rrm_inst[inst] ; /// \todo rrm à passer en parametre
	
	if ( (rrm->state == CLUSTERHEAD) || (rrm->state == CLUSTERHEAD_INIT) )
	{
		pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
		set_ttl_transact(rrm->rrc.transaction,Trans_id, TTL_DEFAULT_VALUE) ;
		pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;		
	}
	else
		fprintf(stderr,"[RRM] RRC_RB_MODIFY_RESP (%d) is not allowed (Only CH):etat=%d\n",Trans_id,rrm->state);	
}
/*!
*******************************************************************************
\brief RRC confirmation of rb_modify_req
*/
void rrc_rb_modify_cfm(
	unsigned char  inst,    //!< Identification de l'instance
	RB_ID Rb_id,                  //!< Radio Bearer ID used by RRC
	unsigned int Trans_id         //!< Transaction ID
	)
{
	rrm_t *rrm = &rrm_inst[inst] ; /// \todo rrm à passer en parametre
	
	if ( rrm->state == CLUSTERHEAD )
	{
		transaction_t *pTransact ;
		pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
		pTransact = get_item_transact(rrm->rrc.transaction,Trans_id ) ;
		if ( pTransact == NULL )
		{
			fprintf(stderr,"[RRM] rrc_rb_modify_cfm (%d) unknown transaction\n",Trans_id);
			pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
		}
		else
		{
			unsigned int parent_id = pTransact->parent_id ;
			unsigned int status_parent = pTransact->parent_status ;
			
			RB_desc_t * pRb = (RB_desc_t *) get_rb_desc_by_rbid( rrm->rrc.pRbEntry, Rb_id ) ;
			if ( pRb != NULL )
			{
				RB_TYPE RB_type = pRb->RB_type ;
				// détruit l'ancienne description
				del_rb_by_rbid( &(rrm->rrc.pRbEntry), Rb_id ) ;
				// mise à jour de la nouvelle
				update_rb_desc_(rrm->rrc.pRbEntry, Trans_id, Rb_id, RB_type );
			}
			
			del_item_transact( &(rrm->rrc.transaction),Trans_id ) ;
			pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
			
			pthread_mutex_lock( &( rrm->cmm.exclu ) ) ;
			if ( status_parent )
			{
				transaction_t *pTransactParent = get_item_transact(rrm->cmm.transaction,parent_id ) ;
				if ( pTransactParent != NULL )
				{
					PUT_MSG( rrm->cmm.s, msg_rrm_cx_modify_cnf(inst,pTransactParent->id ));
					del_item_transact( &(rrm->cmm.transaction),pTransactParent->id ) ;
				}
				else // la transaction parent est inconnue, on ne fait rien
					fprintf(stderr, "[RRM] RRC_RB_MODIFY_CFM (%d) : the parent transaction (%d) is unknown\n",Trans_id,parent_id);	
			}
			pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
		}			
	}
	else
		fprintf(stderr,"[RRM] RRC_RB_MODIFY_CFM (%d) is not allowed (Only CH):etat=%d\n",Trans_id,rrm->state);	
}

/*
 *  =========================================================================
 *  LIBERATION D'UN RADIO BEARER
 *  =========================================================================
 */

/*!
*******************************************************************************
\brief CMM connection release request.  Only in CH.
 */
int cmm_cx_release_req(
			unsigned char  inst,    //!< Identification de l'instance
			RB_ID Rb_id ,           //!< L2 Rb_id
		    unsigned int Trans_id  //!< Transaction ID
		    )
{
	int ret = -1 ;
	rrm_t *rrm = &rrm_inst[inst] ; /// \todo rrm à passer en parametre
	
	if ( rrm->state == CLUSTERHEAD )
	{
		pthread_mutex_lock( &( rrm->cmm.exclu ) ) ;
		add_item_transact( &(rrm->cmm.transaction), Trans_id,INT_CMM,CMM_CX_RELEASE_REQ,0,NO_PARENT);
		pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
		
		pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
		rb_release_req( inst, rrm,Rb_id,Trans_id, PARENT );
		del_rb_by_rbid( &(rrm->rrc.pRbEntry), Rb_id ) ;
		pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
		
		ret = 0 ;
	}
	else
		fprintf(stderr,"[RRM] CMM_CX_RELEASE_REQ (%d) is not allowed (Only CH):etat=%d\n",Trans_id,rrm->state);	
		
	return ret ;
}

/*!
*******************************************************************************
\brief RRC response to rb_release_req
*/
void rrc_rb_release_resp(
	unsigned char  inst,    //!< Identification de l'instance
	unsigned int Trans_id         //!< Transaction ID
	)
{
	rrm_t *rrm = &rrm_inst[inst] ; /// \todo rrm à passer en parametre
	
	if ( rrm->state == CLUSTERHEAD )
	{
		transaction_t *pTransact ;

		pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
		pTransact = get_item_transact(rrm->rrc.transaction,Trans_id ) ;
		if ( pTransact == NULL )
		{
			fprintf(stderr,"[RRM] rrc_rb_release_resp (%d) unknown transaction\n",Trans_id);
			pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
		}
		else
		{
			unsigned int parent_id = pTransact->parent_id ;
			unsigned int status_parent = pTransact->parent_status ;

			del_item_transact( &(rrm->rrc.transaction),Trans_id ) ;
			pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
			
			pthread_mutex_lock( &( rrm->cmm.exclu ) ) ;
			if ( status_parent )
			{
				transaction_t *pTransactParent = get_item_transact(rrm->cmm.transaction,parent_id ) ;
				if ( pTransactParent != NULL )
				{
					PUT_MSG( rrm->cmm.s, msg_rrm_cx_release_cnf(inst,pTransactParent->id ) );
					del_item_transact( &(rrm->cmm.transaction),pTransactParent->id ) ;
				}	
				else // la transaction parent est inconnue, on ne fait rien
					fprintf(stderr,"[RRM] RRC_RB_RELEASE_RESP (%d) : the parent transaction (%d) is unknown\n",Trans_id,parent_id);	
			}
			pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
		}		
	}
	else
		fprintf(stderr,"[RRM] RRC_RB_RELEASE_RESP (%d) is not allowed (Only CH):etat=%d\n",Trans_id,rrm->state);	
}


