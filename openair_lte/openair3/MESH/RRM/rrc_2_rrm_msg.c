/*!
*******************************************************************************

\file       rrc_2_rrm_msg.c

\brief      Fonctions permettant le formattage des donnees pour l'envoi d'un
            message sur le socket entre le  RRC et le RRM 

\author     BURLOT Pascal

\date       16/07/08

   
\par     Historique:
        P.BURLOT 2009-01-20 
            Correction de bug (debordement memoire) remplacement de la macro 
            RRM_CALLOC() par RRM_CALLOC2() dans la fonction msg_rrc_sensing_meas_ind()

*******************************************************************************
*/
#ifdef RRC_EMUL

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "L3_rrc_defs.h"
#include "rrm_sock.h"
#include "L3_rrc_interface.h"
#include "rrc_rrm_msg.h"
#include "rrm_util.h"

#else

#include "defs.h"

#endif

#ifdef TRACE
//! Macro creant la chaine a partir du nom de la variable
#define STRINGIZER(x) #x
//! Tableau pour le mode trace faisant la translation entre le numero et le nom du message
const char *Str_msg_rrc_rrm[NB_MSG_RRC_RRM] = { 
    STRINGIZER(RRM_RB_ESTABLISH_REQ     ),
    STRINGIZER(RRC_RB_ESTABLISH_RESP    ),
    STRINGIZER(RRC_RB_ESTABLISH_CFM     ),
    STRINGIZER(RRM_RB_MODIFY_REQ        ),
    STRINGIZER(RRC_RB_MODIFY_RESP       ),
    STRINGIZER(RRC_RB_MODIFY_CFM        ),
    STRINGIZER(RRM_RB_RELEASE_REQ       ),
    STRINGIZER(RRC_RB_RELEASE_RESP      ), 
    STRINGIZER(RRC_MR_ATTACH_IND        ),
    STRINGIZER(RRM_SENSING_MEAS_REQ     ),
    STRINGIZER(RRC_SENSING_MEAS_RESP    ),
    STRINGIZER(RRC_CX_ESTABLISH_IND     ),
    STRINGIZER(RRC_PHY_SYNCH_TO_MR_IND  ),
    STRINGIZER(RRC_PHY_SYNCH_TO_CH_IND  ),
    STRINGIZER(RRCI_CX_ESTABLISH_RESP   ),
    STRINGIZER(RRC_SENSING_MEAS_IND     ),
    STRINGIZER(RRM_SENSING_MEAS_RESP    ),
    STRINGIZER(RRC_RB_MEAS_IND          ),
    STRINGIZER(RRM_RB_MEAS_RESP         ),
    STRINGIZER(RRM_INIT_CH_REQ          ), 
    STRINGIZER(RRCI_INIT_MR_REQ         )   
} ;
#endif

/*!
*******************************************************************************
\brief  This function initialize the message header 
\return any return value 
*/
static void init_rrc_msg_head( 
    msg_head_t    *msg_head , //!< message header to initialize
    Instance_t     inst     , //!< Instance ID
    MSG_RRC_RRM_T  msg_type , //!< type of message to initialize
    unsigned int   size     , //!< size of message
    Transaction_t  Trans_id   //!< transaction id associated to this message
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
\brief  La fonction formate en un message generique de reponse pour les 
          fonctions :
            - rrc_rb_establish_resp(),rrc_rb_modify_resp(),rrc_rb_modify_resp(),
            - rrc_rb_release_resp() et rrc_sensing_meas_resp() .
\return message formate
*/
static msg_t *msg_rrc_generic_resp(
    Instance_t    inst     , //!< Instance ID
	MSG_RRC_RRM_T msg_type , //!< type of message
	Transaction_t Trans_id   //!< Transaction ID
	)
{

    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
    
    if ( msg != NULL )
    {
        init_rrc_msg_head(&(msg->head),inst, msg_type, 0 , Trans_id);
        msg->data = NULL ;
    }
    return msg ;
}   

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrc_rb_establish_resp().
\return message formate
*/
msg_t *msg_rrc_rb_establish_resp(
    Instance_t    inst     , //!< Instance ID
	Transaction_t Trans_id   //!< Transaction ID
    )
{
    return msg_rrc_generic_resp( inst, RRC_RB_ESTABLISH_RESP, Trans_id) ;
}   
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrc_rb_establish_cfm().
\return message formate
*/
msg_t *msg_rrc_rb_establish_cfm(
    Instance_t    inst     , //!< Instance ID
    RB_ID         Rb_id    , //!< Radio Bearer ID used by RRC
    RB_TYPE       RB_type  , //!< Radio Bearer Type
	Transaction_t Trans_id   //!< Transaction ID
    )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
    
    if ( msg != NULL )
    {
        rrc_rb_establish_cfm_t *p = RRM_CALLOC(rrc_rb_establish_cfm_t , 1 ) ;

        if ( p != NULL )
        {
            init_rrc_msg_head(&(msg->head),inst,RRC_RB_ESTABLISH_CFM, sizeof( rrc_rb_establish_cfm_t ) ,Trans_id);


            p->Rb_id        = Rb_id ;
            p->RB_type      = RB_type ;
        }
        msg->data = (char *) p ;
    }   
    return msg ;

}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrc_rb_modify_resp().
\return message formate
*/
msg_t *msg_rrc_rb_modify_resp(
    Instance_t    inst     , //!< Instance ID
	Transaction_t Trans_id   //!< Transaction ID
    )
{
    return msg_rrc_generic_resp( inst,RRC_RB_MODIFY_RESP, Trans_id) ;
}   
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrc_rb_modify_cfm().
\return message formate
*/
msg_t *msg_rrc_rb_modify_cfm(
    Instance_t    inst     , //!< Instance ID
    RB_ID         Rb_id    , //!< Radio Bearer ID used by RRC
	Transaction_t Trans_id   //!< Transaction ID
    )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
    
    if ( msg != NULL )
    {
        rrc_rb_modify_cfm_t *p = RRM_CALLOC(rrc_rb_modify_cfm_t , 1 ) ;

        if ( p != NULL )
        {
            init_rrc_msg_head(&(msg->head),inst,RRC_RB_MODIFY_CFM, sizeof( rrc_rb_modify_cfm_t ) ,Trans_id);


            p->Rb_id        = Rb_id ;
        }       
        msg->data = (char *) p ;
    }   
    return msg ;

}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
          rrc_rb_release_resp().
\return message formate
*/
msg_t *msg_rrc_rb_release_resp(
    Instance_t    inst     , //!< Instance ID
	Transaction_t Trans_id   //!< Transaction ID
    )
{
    return msg_rrc_generic_resp( inst,RRC_RB_RELEASE_RESP, Trans_id) ;
}   
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrc_MR_attach_ind().
\return message formate
*/
msg_t * msg_rrc_MR_attach_ind(
    Instance_t    inst     , //!< Instance ID
    L2_ID         L2_id 
    )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
    
    if ( msg != NULL )
    {
        rrc_MR_attach_ind_t *p = RRM_CALLOC(rrc_MR_attach_ind_t , 1 ) ;

        if ( p != NULL )
        {
            init_rrc_msg_head(&(msg->head),inst,RRC_MR_ATTACH_IND, sizeof( rrc_MR_attach_ind_t ) ,0);

            memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
        }
        
        msg->data = (char *) p ;
    }
    
    return msg ;
}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrc_sensing_meas_resp().
\return message formate
*/
msg_t *msg_rrc_sensing_meas_resp(
    Instance_t    inst     , //!< Instance ID
	Transaction_t Trans_id   //!< Transaction ID
    )
{
    return msg_rrc_generic_resp( inst,RRC_SENSING_MEAS_RESP, Trans_id) ;
}   
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrc_cx_establish_ind().
\return message formate
*/
msg_t * msg_rrc_cx_establish_ind(
    Instance_t     inst      , //!< Instance ID
    L2_ID          L2_id     , //!< Layer 2 (MAC) ID
	Transaction_t  Trans_id  , //!< Transaction ID
    unsigned char *L3_info   , //!< Optional L3 Information
    L3_INFO_T      L3_info_t , //!< Type of L3 Information
    RB_ID          DTCH_B_id , //!< RBID of broadcast IP service (MR only)
    RB_ID          DTCH_id     //!< RBID of default IP service (MR only)
    )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
    
    if ( msg != NULL )
    {
        rrc_cx_establish_ind_t *p = RRM_CALLOC(rrc_cx_establish_ind_t , 1 ) ;

        if ( p != NULL )
        {
            init_rrc_msg_head(&(msg->head),inst,RRC_CX_ESTABLISH_IND, sizeof( rrc_cx_establish_ind_t ) ,Trans_id);


            memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
            
            p->DTCH_B_id    = DTCH_B_id ;
            p->DTCH_id      = DTCH_id   ;
            
            p->L3_info_t    = L3_info_t ;       
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
\brief  La fonction formate en un message les parametres de la fonction 
        rrc_phy_synch_to_MR_ind().
\return message formate
*/
msg_t * msg_rrc_phy_synch_to_MR_ind(
    Instance_t     inst  , //!< Instance ID
    L2_ID          L2_id
)
{

    msg_t *smsg = RRM_CALLOC(msg_t , 1 ) ; 
    
    if ( smsg != NULL )
    {
      rrc_phy_synch_to_MR_ind_t *p = RRM_CALLOC(rrc_phy_synch_to_MR_ind_t , 1) ;
      if ( p != NULL ){
        init_rrc_msg_head(&(smsg->head),inst,RRC_PHY_SYNCH_TO_MR_IND, sizeof( rrc_phy_synch_to_MR_ind_t ) ,0);
        memcpy(&p->L2_id,(L2_ID*)&L2_id,sizeof(L2_ID));
      }
      smsg->data = (char *)p ;
      msg("[msg_rrc_phy_synch_to_MR_ind] from Inst :%d\n",smsg->head.inst);
    }
    return smsg ;
} 
/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
          rrc_phy_synch_to_CH_ind().
\return message formate
*/
msg_t * msg_rrc_phy_synch_to_CH_ind(
    Instance_t   inst      , //!< Instance ID
    unsigned int Ch_index  , //!< Clusterhead index
    L2_ID        L2_id
    )
{
    msg_t *smsg = RRM_CALLOC(msg_t , 1 ) ; 
    
    if ( smsg != NULL )
    {
        rrc_phy_synch_to_CH_ind_t *p = RRM_CALLOC(rrc_phy_synch_to_CH_ind_t , 1) ;

        if ( p != NULL )
        {
            init_rrc_msg_head(&(smsg->head),inst, RRC_PHY_SYNCH_TO_CH_IND, sizeof( rrc_phy_synch_to_CH_ind_t ) ,0);

            p->Ch_index     = Ch_index  ;
            memcpy(&p->L2_id,(L2_ID*)&L2_id,sizeof(L2_ID));
        }       
        smsg->data = (char *) p ;
        msg("[msg_rrc_phy_synch_to_CH_ind] from Inst :%d\n",smsg->head.inst);
    }   
    return smsg ;
}   

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
          rrc_sensing_meas_ind().
\return message formate
*/
msg_t * msg_rrc_sensing_meas_ind(
    Instance_t      inst         , //!< Instance ID
    L2_ID           L2_id        , //!< Layer 2 ID (MAC) of sensing node
    unsigned int    NB_meas      , //!< Layer 2 ID (MAC) of sensing node
    SENSING_MEAS_T *Sensing_meas , //!< Sensing Information
    Transaction_t   Trans_id       //!< Transaction ID
    )
{

    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
        
    if ( msg != NULL )
    {
        unsigned int size = sizeof( rrc_sensing_meas_ind_t ) + (NB_meas-1) * sizeof(SENSING_MEAS_T) ;
                    // Note : (NB_meas-1) car la première est incorporé dans  rrc_sensing_meas_ind_t 
        rrc_sensing_meas_ind_t *p   = RRM_CALLOC2(rrc_sensing_meas_ind_t,size ) ;

        if ( p != NULL )
        {
            init_rrc_msg_head(&(msg->head),inst, RRC_SENSING_MEAS_IND, size ,Trans_id);


            memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
            
            if ( NB_meas > 0 )
            {
                memcpy( p->Sensing_meas , Sensing_meas, NB_meas * sizeof(SENSING_MEAS_T) )  ;
            }
            
            p->NB_meas      = NB_meas   ;
        }
        
        msg->data = (char *) p ;
    }
    
    return msg ;

}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrc_rb_meas_ind().
\return message formate
*/
msg_t * msg_rrc_rb_meas_ind(
    Instance_t      inst         , //!< Instance ID
	RB_ID           Rb_id        , //!< Radio Bearer ID
	L2_ID           L2_id        , //!< Layer 2 (MAC) IDs for link
	MEAS_MODE       Meas_mode    , //!< Measurement mode (periodic or event-driven)
	MAC_RLC_MEAS_T *Mac_rlc_meas , //!< MAC/RLC measurements
	Transaction_t   Trans_id       //!< Transaction ID
    )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
        
    if ( msg != NULL )
    {
        rrc_rb_meas_ind_t *p = RRM_CALLOC(rrc_rb_meas_ind_t , 1 ) ;

        if ( p != NULL )
        {
            init_rrc_msg_head(&(msg->head),inst, RRC_RB_MEAS_IND, sizeof( rrc_rb_meas_ind_t ) ,Trans_id);


            memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
            memcpy( &(p->Mac_rlc_meas), Mac_rlc_meas, sizeof(L2_ID) )  ;

            p->Rb_id        = Rb_id     ;
            p->Meas_mode    = Meas_mode ;
        }
        
        msg->data = (char *) p ;
    }
    
    return msg ;

}

 
