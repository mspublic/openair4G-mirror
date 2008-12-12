/*!
*******************************************************************************

\file    	cmm_msg.c

\brief   	Fonctions permettant le formattage des donnees pour l'envoi d'un
            message sur le socket entre le CMM et le RRM 

\author  	BURLOT Pascal

\date    	16/07/08

   
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
#include "cmm_msg.h"
#include "rrm_util.h"

#ifdef TRACE
//! Macro creant la chaine a partir du nom de la variable
#define STRINGIZER(x) #x
//! Tableau pour le mode trace faisant la translation entre le numero et le nom du message
const char *Str_msg_cmm_rrm[NB_MSG_CMM_RRM] =  { 
	STRINGIZER(CMM_CX_SETUP_REQ  ) ,
	STRINGIZER(RRM_CX_SETUP_CNF  ),
	STRINGIZER(CMM_CX_MODIFY_REQ ),
	STRINGIZER(RRM_CX_MODIFY_CNF ),
	STRINGIZER(CMM_CX_RELEASE_REQ),
	STRINGIZER(RRM_CX_RELEASE_CNF),
	STRINGIZER(CMM_CX_RELEASE_ALL_REQ),
	STRINGIZER(RRM_CX_RELEASE_ALL_CNF),
	STRINGIZER(RRCI_ATTACH_REQ),
	STRINGIZER(RRM_ATTACH_IND),
	STRINGIZER(CMM_ATTACH_CNF),
	STRINGIZER(RRM_MR_ATTACH_IND),
	STRINGIZER(ROUTER_IS_CH_IND),
	STRINGIZER(RRCI_CH_SYNCH_IND),
	STRINGIZER(CMM_INIT_MR_REQ),
	STRINGIZER(RRM_MR_SYNCH_IND),
	STRINGIZER(RRM_NO_SYNCH_IND),
	STRINGIZER(CMM_INIT_CH_REQ)	
};
#endif

/*!
*******************************************************************************
\brief  This function initialize the message header 
\return any return value 
*/
static void init_cmm_msg_head( 
	msg_head_t   *msg_head , ///< message header to initialize 
	unsigned char inst, 
	MSG_CMM_RRM_T msg_type , ///< type of message to initialize
	unsigned int  size     , ///< size of message
	unsigned int  Trans_id   ///< transaction id associated to this message
	) 
{
	if ( msg_head != NULL )
	{
		msg_head->start    = START_MSG ; 
		msg_head->msg_type = 0xFF & msg_type ;
		msg_head->inst     = inst  ;
		msg_head->Trans_id = Trans_id  ;
		msg_head->size     = size ;
	}
}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        cmm_cx_setup_req().
\return message formate
*/
msg_t * msg_cmm_cx_setup_req(
	unsigned char inst, 
	 L2_ID Src,             //!< L2 source MAC address
	 L2_ID Dst,             //!< L2 destination MAC address
	 QOS_CLASS_T QoS_class, //!< QOS class index
	 unsigned int Trans_id  //!< Transaction ID
	 )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		cmm_cx_setup_req_t *p = RRM_CALLOC(cmm_cx_setup_req_t , 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,CMM_CX_SETUP_REQ, sizeof( cmm_cx_setup_req_t) ,Trans_id);

			memcpy( p->Src.L2_id, Src.L2_id, sizeof(L2_ID) )  ;
			memcpy( p->Dst.L2_id, Dst.L2_id, sizeof(L2_ID) )  ;
			p->QoS_class 	= QoS_class;
			p->Trans_id 	= Trans_id ;
		}
		
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_cx_setup_cnf().
\return message formate
*/
msg_t * msg_rrm_cx_setup_cnf(
			unsigned char inst, 
			RB_ID Rb_id,           //!< L2 Rb_id
		    unsigned int Trans_id  //!< Transaction ID
		    )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_cx_setup_cnf_t *p = RRM_CALLOC(rrm_cx_setup_cnf_t , 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,RRM_CX_SETUP_CNF, sizeof( rrm_cx_setup_cnf_t) ,Trans_id);
			
			p->Rb_id 		= Rb_id ;
			p->Trans_id 	= Trans_id ;
		}	
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        cmm_cx_modify_req().
\return message formate
*/
msg_t * msg_cmm_cx_modify_req(
			unsigned char inst, 
			RB_ID Rb_id ,          //!< L2 Rb_id
		    QOS_CLASS_T QoS_class, //!< QOS class index
		    unsigned int Trans_id  //!< Transaction ID
		    )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		
		cmm_cx_modify_req_t *p = RRM_CALLOC(cmm_cx_modify_req_t, 1 ) ;

		if ( p != NULL )
		{
		    init_cmm_msg_head(&(msg->head),inst,CMM_CX_MODIFY_REQ, sizeof( cmm_cx_modify_req_t ) ,Trans_id);
			p->Rb_id 		= Rb_id  ;
			p->QoS_class 	= QoS_class  ;
			p->Trans_id 	= Trans_id ;
		}
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_cx_modify_cnf().
\return message formate
*/
msg_t * msg_rrm_cx_modify_cnf(
			unsigned char inst, 
			unsigned int Trans_id  //!< Transaction ID
		    )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_cx_modify_cnf_t *p = RRM_CALLOC(rrm_cx_modify_cnf_t , 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,RRM_CX_MODIFY_CNF, sizeof( rrm_cx_modify_cnf_t) ,Trans_id);			
			p->Trans_id 	= Trans_id ;
		}	
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
         cmm_cx_release_req().
\return message formate
*/
msg_t * msg_cmm_cx_release_req(
			unsigned char inst, 
			RB_ID Rb_id ,           //!< L2 Rb_id
		    unsigned int Trans_id   //!< Transaction ID
		    )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		cmm_cx_release_req_t *p = RRM_CALLOC(cmm_cx_release_req_t , 1) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,CMM_CX_RELEASE_REQ, sizeof( cmm_cx_release_req_t ) ,Trans_id);
			p->Trans_id 	= Trans_id ;
			p->Rb_id	 	= Rb_id ;
		}
		
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_cx_release_cnf().
\return message formate
*/
msg_t * msg_rrm_cx_release_cnf(
			unsigned char inst, 
			unsigned int Trans_id  //!< Transaction ID
			)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_cx_release_cnf_t *p = RRM_CALLOC(rrm_cx_release_cnf_t , 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,RRM_CX_RELEASE_CNF, sizeof( rrm_cx_release_cnf_t) ,Trans_id);
			p->Trans_id 	= Trans_id ;
		}	
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        cmm_cx_release_all_req().
\return message formate
*/
msg_t * msg_cmm_cx_release_all_req(
			unsigned char inst, 
			L2_ID L2_id ,           //!< L2 Rb_id
			unsigned int Trans_id  //!< Transaction ID
			)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		cmm_cx_release_all_req_t *p = RRM_CALLOC(cmm_cx_release_all_req_t , 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,CMM_CX_RELEASE_ALL_REQ, sizeof( cmm_cx_release_all_req_t) ,Trans_id);			
			p->Trans_id 	= Trans_id ;
			memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
		}	
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_cx_release_all_cnf().
\return message formate
*/
msg_t * msg_rrm_cx_release_all_cnf(
			unsigned char inst, 
			unsigned int Trans_id  //!< Transaction ID
			)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_cx_release_all_cnf_t *p = RRM_CALLOC(rrm_cx_release_all_cnf_t , 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,RRM_CX_RELEASE_ALL_CNF, sizeof( rrm_cx_release_all_cnf_t) ,Trans_id);
			p->Trans_id 	= Trans_id ;
		}	
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrci_attach_req().
\return message formate
*/
msg_t * msg_rrci_attach_req(
			unsigned char inst, 
			L2_ID L2_id,               //!< Layer 2 (MAC) ID
		    L3_INFO_T L3_info_t,       //!< Type of L3 Information
		    unsigned char *L3_info,    //!< L3 addressing Information
		    RB_ID DTCH_B_id,           //!< RBID of broadcast IP service (MR only)
		    RB_ID DTCH_id,             //!< RBID of default IP service (MR only)
		    unsigned int Trans_id      //!< Transaction ID
		    )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrci_attach_req_t *p = RRM_CALLOC(rrci_attach_req_t , 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,RRCI_ATTACH_REQ, sizeof( rrci_attach_req_t) ,Trans_id);
			
			p->Trans_id 	= Trans_id ;
			memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
			p->L3_info_t 	= L3_info_t ;
			
			if ( L3_info_t == IPv4_ADDR ) 
				memcpy( p->L3_info, L3_info, 4 );
			else
				if ( L3_info_t == IPv6_ADDR ) 
					memcpy( p->L3_info, L3_info, 16 );
					
			p->DTCH_B_id 	= DTCH_B_id ;
			p->DTCH_id 		= DTCH_id ;
		}	
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_attach_ind().
\return message formate
*/
msg_t * msg_rrm_attach_ind(
			unsigned char inst, 
			L2_ID L2_id,               //!< Layer 2 (MAC) ID
		    L3_INFO_T L3_info_t,       //!< Type of L3 Information
		    unsigned char *L3_info,    //!< L3 addressing Information
		    RB_ID DTCH_id              //!< RBID of default IP service (MR only)
		    )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_attach_ind_t *p = RRM_CALLOC(rrm_attach_ind_t , 1) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,RRM_ATTACH_IND, sizeof( rrm_attach_ind_t) ,0);
			
			memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
			p->L3_info_t 	= L3_info_t ;
			
			if ( L3_info_t == IPv4_ADDR ) 
				memcpy( p->L3_info, L3_info, 4 );
			else
				if ( L3_info_t == IPv6_ADDR ) 
					memcpy( p->L3_info, L3_info, 16 );
					
			p->DTCH_id 		= DTCH_id ;
		}
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        cmm_attach_cnf().
\return message formate
*/
msg_t * msg_cmm_attach_cnf(
			unsigned char inst,
			L2_ID L2_id, 
			L3_INFO_T L3_info_t,       //!< Type of L3 Information
			unsigned char *L3_info,    //!< L3 addressing Information
			unsigned int Trans_id      //!< Transaction ID
			   )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		cmm_attach_cnf_t *p = RRM_CALLOC(cmm_attach_cnf_t, 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,CMM_ATTACH_CNF, sizeof( cmm_attach_cnf_t) ,Trans_id);
			
			p->L3_info_t 	= L3_info_t ;
			
			if ( L3_info_t == IPv4_ADDR ) 
				memcpy( p->L3_info, L3_info, 4 );
			else
				if ( L3_info_t == IPv6_ADDR ) 
					memcpy( p->L3_info, L3_info, 16 );
			memcpy( &p->L2_id, &L2_id, sizeof(L2_ID) );
			p->Trans_id 	= Trans_id ;
		}	
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_MR_attach_ind().
\return message formate
*/
msg_t * msg_rrm_MR_attach_ind(
			unsigned char inst, 
			L2_ID L2_id      //!< MR Layer 2 (MAC) ID
			)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_MR_attach_ind_t *p = RRM_CALLOC( rrm_MR_attach_ind_t , 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,RRM_MR_ATTACH_IND, sizeof( rrm_MR_attach_ind_t) ,0);			
			memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
		}				
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        router_is_CH_ind().
\return message formate
*/
msg_t * msg_router_is_CH_ind(
			unsigned char inst, 
			L2_ID L2_id      //!< CH Layer 2 (MAC) ID
			)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		router_is_CH_ind_t *p = RRM_CALLOC(router_is_CH_ind_t , 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst,ROUTER_IS_CH_IND, sizeof( router_is_CH_ind_t) ,0);	
			memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
		}
		msg->data = (char *) p ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrci_CH_synch_ind().
\return message formate
*/
msg_t * msg_rrci_CH_synch_ind( unsigned char inst)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		init_cmm_msg_head(&(msg->head),inst,RRCI_CH_SYNCH_IND, 0 ,0);			
		msg->data = NULL ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        cmm_init_mr_req().
\return message formate 
*/
msg_t * msg_cmm_init_mr_req( unsigned char inst )
{
	msg_t *msg = RRM_CALLOC( msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		init_cmm_msg_head(&(msg->head),inst,RRCI_CH_SYNCH_IND, 0 ,0);
			
		msg->data = NULL ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_MR_synch_ind().
\return message formate
*/
msg_t * msg_rrm_MR_synch_ind(  unsigned char inst)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		init_cmm_msg_head(&(msg->head),inst, RRM_MR_SYNCH_IND, 0 ,0);	
		msg->data = NULL ;
	}
	return msg ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rm_no_synch_ind().
\return message formate
*/
msg_t * msg_rrm_no_synch_ind(  unsigned char inst)
{
	msg_t *msg = RRM_CALLOC(msg_t ,1 ) ; 
	
	if ( msg != NULL )
	{
		init_cmm_msg_head(&(msg->head),inst,RRM_NO_SYNCH_IND, 0 ,0);			
		msg->data = NULL ;
	}
	return msg  ;
}
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        cmm_init_ch_req.
\return message formate
*/
msg_t * msg_cmm_init_ch_req(
			unsigned char inst,
			L3_INFO_T L3_info_t, //!< Type of L3 Information
		    void *L3_info		 //!< L3 addressing Information
		    )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		cmm_init_ch_req_t *p = RRM_CALLOC(cmm_init_ch_req_t , 1 ) ;

		if ( p != NULL )
		{
			init_cmm_msg_head(&(msg->head),inst, CMM_INIT_CH_REQ, sizeof( cmm_init_ch_req_t) ,0);

			p->L3_info_t	= L3_info_t ;
			
			if ( L3_info_t == IPv4_ADDR ) 
				memcpy( p->L3_info, L3_info, 4 );
			else
				if ( L3_info_t == IPv6_ADDR ) 
					memcpy( p->L3_info, L3_info, 16 );
		}		
		msg->data = (char *) p ;
	}
	return msg ;
}
