/*!
*******************************************************************************

\file    	pusu_msg.c

\brief   	Fonctions permettant le formattage des donnees pour l'envoi d'un
            message sur le socket entre le RRM et PuSu (intra/inter Routing)

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

#include "rrm_sock.h"
#include "L3_rrc_defs.h"
#include "cmm_rrm_interface.h"
#include "pusu_msg.h"
#include "rrm_util.h"
/*!
*******************************************************************************
\brief 	La fonction formate en un message de mesure de voisinage pret a etre 
        envoyer sur le socket du PUSU.
\return message formate
*/
msg_pusu_t *msg_rrm_neighbor_meas_ind(
	unsigned char inst  , ///< Identification de l'instance RRM
	L2_ID L2_id1		, ///< Id du voisin No 1
	unsigned char rssi1	, ///< RSSI du voisin No 2 mesure par le voisin No 1
	L2_ID L2_id2		, ///< Id du voisin No 2
	unsigned char rssi2   ///< RSSI du voisin No 1 mesure par le voisin No 2
	)
{
	msg_pusu_t *msg = RRM_CALLOC(msg_pusu_t , 1 ) ; 

	if ( msg != NULL )
	{
		rrm_neighbor_meas_ind_t *p = RRM_CALLOC(rrm_neighbor_meas_ind_t , 1 );
		
		if ( p != NULL )
		{
			// Header 
			msg->head.type = START_MSG_PUSU ;
			msg->head.inst = inst ;
			msg->head.length = sizeof( rrm_neighbor_meas_ind_t )  ; // + sizeof(msg_pusu_head_t) ;
			
			memcpy( p->mac1, L2_id1.L2_id, sizeof(L2_ID) )  ;
			memcpy( p->mac2, L2_id2.L2_id, sizeof(L2_ID) )  ;
			
			p->number_metric_uplink   = 1 ;
			p->number_metric_downlink = 1 ;
			
			// uplink
			p->neighbor_metric[0].type   = NEIGHBOR_METRIC_TYPE ;
			p->neighbor_metric[0].length = sizeof(neighbor_metric_t)  ;
			p->neighbor_metric[0].value.encoded_type = RSSI_ENCODED_TYPE ;
			p->neighbor_metric[0].value.rssi = rssi1  ;
			
			// downlink
			p->neighbor_metric[1].type   = NEIGHBOR_METRIC_TYPE ;
			p->neighbor_metric[1].length = sizeof(neighbor_metric_t)  ;
			p->neighbor_metric[1].value.encoded_type = RSSI_ENCODED_TYPE ;
			p->neighbor_metric[1].value.rssi = rssi2  ;
		}
		
		msg->data = (char *) p ;
	}
	
	return msg ;
}
/*!
*******************************************************************************
\brief 	La fonction formate en un message d'attachement ou detachement d'un MR
 		pret a etre envoyer sur le socket du PUSU.
\return message formate
*/
msg_pusu_t *msg_rrm_mr_attach_ind(
	unsigned char inst  ,///< Identification de l'instance RRM
	L2_ID L2_id_ch, ///< Id du cluster head 
	L2_ID L2_id_mr, ///< Id du mesh router  
	int flag_attach ///< flag d'attachement : 0=attach / 1=detach 
	)
{
	msg_pusu_t *msg = RRM_CALLOC(msg_pusu_t , 1 ) ; 

	if ( msg != NULL )
	{
		rrm_mr_attach_ind_t *p = RRM_CALLOC(rrm_mr_attach_ind_t , 1 );
		
		if ( p != NULL )
		{
			// Header 
			msg->head.type = START_MSG_PUSU ;
			msg->head.inst = inst ;
			msg->head.length = sizeof( rrm_mr_attach_ind_t )  ; // + sizeof(msg_pusu_head_t) ;
			
			memcpy( p->mac1, L2_id_ch.L2_id, sizeof(L2_ID) )  ;
			memcpy( p->mac2, L2_id_mr.L2_id, sizeof(L2_ID) )  ;
			
			p->number_metric_uplink   = 0 ;
			p->number_metric_downlink = flag_attach ;	 // 0:attach / 1:detach	
		}
		
		msg->data = (char *) p ;
	}
	
	return msg ;
}


