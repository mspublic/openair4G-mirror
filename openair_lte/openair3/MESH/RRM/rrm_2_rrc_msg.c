/*!
*******************************************************************************

\file    	rrm_2_rrc_msg.c

\brief   	Fonctions permettant le formattage des donnees pour l'envoi d'un
            message sur le socket entre le  RRC et le RRM 

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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "rrm_sock.h"
#include "L3_rrc_defs.h"
#include "L3_rrc_interface.h"
#include "rrc_rrm_msg.h"
#include "rrm_util.h"

#ifdef TRACE
//! Macro creant la chaine a partir du nom de la variable
#define STRINGIZER(x) #x
//! Tableau pour le mode trace faisant la translation entre le numero et le nom du message
const char *Str_msg_rrc_rrm[NB_MSG_RRC_RRM] = { 
	STRINGIZER(RRM_RB_ESTABLISH_REQ 	),
	STRINGIZER(RRC_RB_ESTABLISH_RESP	),
	STRINGIZER(RRC_RB_ESTABLISH_CFM 	),
	STRINGIZER(RRM_RB_MODIFY_REQ  		),
	STRINGIZER(RRC_RB_MODIFY_RESP 		),
	STRINGIZER(RRC_RB_MODIFY_CFM  		),
	STRINGIZER(RRM_RB_RELEASE_REQ 		),
	STRINGIZER(RRC_RB_RELEASE_RESP		), 
	STRINGIZER(RRC_MR_ATTACH_IND		),
	STRINGIZER(RRM_SENSING_MEAS_REQ		),
	STRINGIZER(RRC_SENSING_MEAS_RESP	),
	STRINGIZER(RRC_CX_ESTABLISH_IND 	),
	STRINGIZER(RRC_PHY_SYNCH_TO_MR_IND	),
	STRINGIZER(RRC_PHY_SYNCH_TO_CH_IND	),
	STRINGIZER(RRCI_CX_ESTABLISH_RESP	),
	STRINGIZER(RRC_SENSING_MEAS_IND		),
	STRINGIZER(RRM_SENSING_MEAS_RESP	),
	STRINGIZER(RRC_RB_MEAS_IND	       	),
	STRINGIZER(RRM_RB_MEAS_RESP	       	),	
	STRINGIZER(RRM_INIT_CH_REQ              ), 
	STRINGIZER(RRM_INIT_MR_REQ              )	
} ;
#endif

/*!
*******************************************************************************
\brief  This function initialize the message header 
\return any return value 
*/
static void init_rrc_msg_head( 
	msg_head_t   *msg_head , ///< message header to initialize
	unsigned char inst, 
	MSG_RRC_RRM_T msg_type , ///< type of message to initialize
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
		msg_head->size     = size;
	}
}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_rb_establish_req().
\return message formate
*/
msg_t *msg_rrm_rb_establish_req(
	unsigned char inst, 
	const LCHAN_DESC *Lchan_desc,       		//!< Logical Channel Descriptor Array
	const MAC_RLC_MEAS_DESC *Mac_rlc_meas_desc, //!< MAC/RLC Measurement descriptors for RB 
	L2_ID *L2_id,                 				//!< Layer 2 (MAC) IDs for link
	unsigned int Trans_id,        				//!< Transaction ID
	unsigned char *L3_info,       				//!< Optional L3 Information
	L3_INFO_T L3_info_t         				//!< Type of L3 Information
	)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_rb_establish_req_t *p = RRM_CALLOC(rrm_rb_establish_req_t ,1 ) ;

		if ( p != NULL )
		{
			init_rrc_msg_head(&(msg->head),inst,RRM_RB_ESTABLISH_REQ, sizeof( rrm_rb_establish_req_t) ,Trans_id);
			
			memcpy(&(p->Lchan_desc), Lchan_desc, sizeof(LCHAN_DESC) );
			memcpy(&(p->Mac_rlc_meas_desc), Mac_rlc_meas_desc, sizeof(MAC_RLC_MEAS_DESC) );
			memcpy(&(p->L2_id[0]), L2_id, 2*sizeof(L2_ID) ); // SRC+DST

			p->Trans_id 	= Trans_id ;
			
			p->L3_info_t 	= L3_info_t ;	

			if ( L3_info_t == IPv4_ADDR ) 
				memcpy( p->L3_info, L3_info, 4 );
			else
				if ( L3_info_t == IPv6_ADDR ) 
					memcpy( p->L3_info, L3_info, 16 );

			//printf("=>rrm_rb_establish_req(%d):size %d , %d, L3_info_t %d, L3_addr %d \n",
			//       inst,msg->head.size,sizeof( rrm_rb_establish_req_t),
			//       p->L3_info_t,p->L3_info[0]); 

			//			print_L2_id(&p->L2_id[0] ); printf("=>rrm_rb_establish_req(%d):L2_id src\n",inst); 
			//print_L2_id(&p->L2_id[1] ); printf("=>rrm_rb_establish_req(%d):L2_id dst\n",inst); 
		}		
		msg->data = (char *) p ;			
	}
	return msg ;
}			  



/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_rb_modify_req().
\return message formate
*/
msg_t *msg_rrm_rb_modify_req(
		unsigned char inst, 
		const LCHAN_DESC 		*Lchan_desc,    //!< Logical Channel Descriptor Array
		const MAC_RLC_MEAS_DESC *Mac_meas_desc, //!< MAC/RLC Measurement descriptors for RB 
		RB_ID 					Rb_id,         	//!< Radio Bearer ID
		unsigned int 			Trans_id       	//!< Transaction ID
		)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_rb_modify_req_t *p = RRM_CALLOC(rrm_rb_modify_req_t , 1) ;

		if ( p != NULL )
		{
			init_rrc_msg_head(&(msg->head),inst,RRM_RB_MODIFY_REQ, sizeof( rrm_rb_modify_req_t) ,Trans_id);
			
			memcpy(&(p->Lchan_desc), Lchan_desc, sizeof(LCHAN_DESC) );
			memcpy(&(p->Mac_meas_desc), Mac_meas_desc, sizeof(MAC_RLC_MEAS_DESC) );

			p->Rb_id 	= Rb_id ;
			p->Trans_id 	= Trans_id ;
		}		
		msg->data = (char *) p ;
	}
	return msg ;
}			  


/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_rb_release_req().
\return message formate
*/
msg_t *msg_rrm_rb_release_req(
		unsigned char inst, 
		RB_ID 			Rb_id, 		//!< Radio Bearer ID
		unsigned int 	Trans_id    //!< Transaction ID
	)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_rb_release_req_t *p = RRM_CALLOC(rrm_rb_release_req_t , 1 ) ;

		if ( p != NULL )
		{
			init_rrc_msg_head(&(msg->head),inst,RRM_RB_RELEASE_REQ, sizeof( rrm_rb_release_req_t) ,Trans_id);
			
			p->Rb_id 	= Rb_id ;
			p->Trans_id = Trans_id ;
			
		}		
		msg->data = (char *) p ;
	}
	return msg ;
}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_sensing_meas_req().
\return message formate
*/
msg_t *msg_rrm_sensing_meas_req(
	unsigned char inst, 
	L2_ID 				L2_id,              //!< Layer 2 (MAC) ID
	SENSING_MEAS_DESC 	Sensing_meas_desc,  //!< Sensing Measurement Descriptor
	unsigned int 		Trans_id            //!< Transaction ID
	)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_sensing_meas_req_t *p = RRM_CALLOC(rrm_sensing_meas_req_t , 1 ) ;

		if ( p != NULL )
		{
			init_rrc_msg_head(&(msg->head),inst,RRM_SENSING_MEAS_REQ, sizeof( rrm_sensing_meas_req_t) ,Trans_id);
			
			memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
			memcpy(&(p->Sensing_meas_desc), &Sensing_meas_desc, sizeof(SENSING_MEAS_DESC)) ;
			p->Trans_id = Trans_id ;			
		}		
		msg->data = (char *) p ;
	}
	return msg ;
	
}


/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
          rrci_cx_establish_resp().
\return message formate
*/
msg_t * msg_rrci_cx_establish_resp(
		unsigned char inst, 
		unsigned int Trans_id,        //!< Transaction ID
		L2_ID L2_id,
		unsigned char *L3_info,       //!< Optional L3 Information
		L3_INFO_T L3_info_t           //!< Type of L3 Information
		)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrci_cx_establish_resp_t *p = RRM_CALLOC(rrci_cx_establish_resp_t , 1 ) ;

		if ( p != NULL )
		{
			init_rrc_msg_head(&(msg->head),inst,RRCI_CX_ESTABLISH_RESP, sizeof( rrci_cx_establish_resp_t ) ,Trans_id);

			p->Trans_id 	= Trans_id	 	;
			p->L3_info_t 	= L3_info_t 	;
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
/*!
*******************************************************************************
\brief  La fonction formate en un message generique de reponse pour les 
		  fonctions :
          	- msg_rrm_sensing_meas_resp(),  msg_rrm_rb_meas_resp().
\return message formate
*/
static msg_t *msg_rrm_generic_resp(
		unsigned char inst, 
		MSG_RRC_RRM_T 	msg_type	, 			//!< type of message
		unsigned int Trans_id       			//!< Transaction ID
		)
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrc_generic_resp_t *p = RRM_CALLOC(rrc_generic_resp_t , 1 ) ;

		if ( p != NULL )
		{
			init_rrc_msg_head(&(msg->head),inst,msg_type, sizeof( rrc_generic_resp_t), Trans_id);
			p->Trans_id 	= Trans_id ;
		}		
		msg->data = (char *) p ;
	}
	return msg ;
}	

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_sensing_meas_resp().
\return message formate
*/
msg_t * msg_rrm_sensing_meas_resp(
	unsigned char inst, 
	unsigned int Trans_id       //!< Transaction ID
	)
{
	return msg_rrm_generic_resp( inst,RRM_SENSING_MEAS_RESP, Trans_id) ;
}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_rb_meas_resp().
\return message formate
*/
msg_t * msg_rrm_rb_meas_resp(
		unsigned char inst, 
		unsigned int Trans_id       //!< Transaction ID
		      )
{
	return msg_rrm_generic_resp( inst,RRM_RB_MEAS_RESP, Trans_id) ;
}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_init_ch_req().
\return message formate
*/
msg_t *msg_rrm_init_ch_req( 
	unsigned char inst, 
	unsigned int Trans_id, 				//!< Transaction ID
	const LCHAN_DESC *Lchan_desc_srb0,  //!< Logical Channel Descriptor - SRB0
	const LCHAN_DESC *Lchan_desc_srb1,  //!< Logical Channel Descriptor - SRB1
	L2_ID 				L2_id    
    )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrm_init_ch_req_t *p = RRM_CALLOC(rrm_init_ch_req_t , 1 ) ;

		if ( p != NULL )
		{
		  init_rrc_msg_head(&(msg->head),inst,RRM_INIT_CH_REQ, sizeof( rrm_init_ch_req_t ) ,Trans_id);

			p->Trans_id 	= Trans_id	 	;
			memcpy(&(p->Lchan_desc_srb0), Lchan_desc_srb0, sizeof(LCHAN_DESC) );
			memcpy(&(p->Lchan_desc_srb1), Lchan_desc_srb1, sizeof(LCHAN_DESC) );
			memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
		        print_L2_id(&p->L2_id ); printf("=>rrm_init_ch_req(%d):L2_id\n",inst); 	
		}		
		msg->data = (char *) p ;
	}	
	return msg ;

}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_init_mr_req().
\return message formate
*/

msg_t *msg_rrci_init_mr_req( 
	unsigned char inst, 
	unsigned int 		Trans_id,		  //!< Transaction ID 
	const LCHAN_DESC 	*Lchan_desc_srb0,  //!< Logical Channel Descriptor - SRB0 
	const LCHAN_DESC 	*Lchan_desc_srb1,  //!< Logical Channel Descriptor - SRB1 
	unsigned char 		CH_index 		  //!< index to identify the CH 
    )
{
	msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
	
	if ( msg != NULL )
	{
		rrci_init_mr_req_t *p = RRM_CALLOC(rrci_init_mr_req_t , 1 ) ;

		if ( p != NULL )
		{
		  init_rrc_msg_head(&(msg->head),inst,RRCI_INIT_MR_REQ, sizeof( rrci_init_mr_req_t ) ,Trans_id);

			p->Trans_id 	= Trans_id	 	;
			memcpy(&(p->Lchan_desc_srb0), Lchan_desc_srb0, sizeof(LCHAN_DESC) );
			memcpy(&(p->Lchan_desc_srb1), Lchan_desc_srb1, sizeof(LCHAN_DESC) );
			p->CH_index = CH_index ; 
			
		}		
		msg->data = (char *) p ;
	}	
	return msg ;
} 

 
