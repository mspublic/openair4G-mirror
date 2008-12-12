/*!
*******************************************************************************

\file    	ch_init.c

\brief   	Fonctions permettant la gestion de la phase d'initialisation du 
			cluster head.

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
#include "ch_init.h"


//! Met un message dans la file des messages a envoyer
#define PUT_MSG(s,m)  put_msg( 	&(rrm->file_send_msg),s,m) 

/*!
*******************************************************************************
\brief  Request to initialize the Cluster Head with L3 Information 
*/
void cmm_init_ch_req(
			unsigned char  inst,    //!< Identification de l'instance
			L3_INFO_T L3_info_t, 	//!< Type of L3 Information
		    void *L3_info			//!< L3 addressing Information
		    )
{
	rrm_t *rrm = &rrm_inst[inst] ; /// \todo rrm à passer en parametre

	if ( rrm->state == CLUSTERHEAD_INIT )
	{
		if ( L3_info != NULL ) 
		{
			if ( L3_info_t == IPv4_ADDR ) 
				memcpy( rrm->L3_info, L3_info, 4 );
			else
				if ( L3_info_t == IPv6_ADDR ) 
					memcpy( rrm->L3_info, L3_info, MAX_L3_INFO );

			rrm->L3_info_t = L3_info_t ;	
		}

		pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
		rrm->rrc.trans_cnt++ ;

		PUT_MSG(    rrm->rrc.s, 
			      	msg_rrm_init_ch_req( inst,
							 rrm->rrc.trans_cnt,
							&Lchan_desc[QOS_SRB0], 
							&Lchan_desc[QOS_SRB1], 
							 rrm->L2_id
							 )
						) ;
				
		// add_item_transact( &(rrm->rrc.transaction), rrm->rrc.trans_cnt ,INT_RRC,RRM_INIT_CH_REQ,0,NO_PARENT);
		// add_rb( &(rrm->rrc.pRbEntry), rrm->rrc.trans_cnt, QoS_class, &src_dst[0] ) ;

		pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
	}
}

/*!
*******************************************************************************
\brief Mesh router PHY-Synch Indication
*/
void rrc_phy_synch_to_MR_ind(unsigned char  inst,L2_ID L2_id )
{
	rrm_t *rrm = &rrm_inst[inst] ; /// \todo rrm à passer en parametre

	if ( rrm->state == ISOLATEDNODE )
	{
	    /* Memorisation du L2_id du noeud ( c'est le niveau RRC qui a l'info ) */
	    memcpy( &rrm->L2_id,  &L2_id, sizeof(L2_ID));
	  
		//--------------------------------------------
		pthread_mutex_lock(   &( rrm->rrc.exclu )  ) ;
		
		if ( rrm->rrc.pNeighborEntry  != NULL ) // Reset Neighborhood
			del_all_neighbor( &(rrm->rrc.pNeighborEntry) );
			
		pthread_mutex_unlock( &( rrm->rrc.exclu )  ) ;

		//--------------------------------------------
		put_msg( &(rrm->file_send_msg), rrm->cmm.s, msg_router_is_CH_ind( inst,rrm->L2_id) ) ;

		rrm->state = CLUSTERHEAD_INIT ; 
	}
	else
		fprintf(stderr,"[RRM] RRC_PHY_SYNCH_TO_MR_IND/TIMEOUT_IN  is not allowed (Only IN):etat=%d\n",rrm->state);
}        


