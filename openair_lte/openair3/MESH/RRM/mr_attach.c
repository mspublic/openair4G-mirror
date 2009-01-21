/*!
*******************************************************************************

\file       mr_attach.c

\brief      Fonctions permettant la gestion de l'attachement d'un Mesh Router 
            a un cluster.

\author     BURLOT Pascal

\date       29/08/08

   
\par     Historique:
        P.BURLOT 2009-01-20 
            + separation de la file de message CMM/RRM a envoyer en 2 files 
              distinctes ( file_send_cmm_msg, file_send_rrc_msg)

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
#include "pusu_msg.h"
#include "msg_mngt.h"
#include "rb_db.h"
#include "neighbor_db.h"
#include "rrm_util.h"
#include "transact.h"
#include "rrm_constant.h"
#include "rrm.h"
#include "mr_attach.h"

//! Met un message dans la file des messages a envoyer
#define PUT_CMM_MSG(m)  put_msg(  &(rrm->file_send_cmm_msg),rrm->cmm.s,m ) 
#define PUT_PUSU_MSG(m) put_msg(  &(rrm->file_send_cmm_msg),rrm->pusu.s,m) 
#define PUT_RRC_MSG(m)  put_msg(  &(rrm->file_send_rrc_msg),rrm->rrc.s,m ) 

/*!
*******************************************************************************
\brief MR attachement indication. Sent by RRC to RRM to indicate the MAC ID of 
       a new MR attached to CH at layer 2 
*/
void rrc_MR_attach_ind(
    Instance_t inst  , //!< Identification de l'instance
    L2_ID      L2_id   //!< Layer 2 (MAC) ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    if ( (rrm->state == CLUSTERHEAD_INIT )|| (rrm->state == CLUSTERHEAD ))
    {
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
        add_neighbor( &(rrm->rrc.pNeighborEntry), &L2_id ) ;
        rrm->rrc.trans_cnt++ ;
        add_item_transact( &(rrm->rrc.transaction), rrm->rrc.trans_cnt ,INT_RRC,RRM_SENSING_MEAS_REQ,0,NO_PARENT );
        pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;

        PUT_RRC_MSG( msg_rrm_sensing_meas_req( inst, L2_id ,Sensing_meas_desc, rrm->rrc.trans_cnt));
        
        PUT_CMM_MSG( msg_rrm_MR_attach_ind(inst,L2_id ));
    }
    else
        fprintf(stderr,"[RRM] RRC_MR_ATTACH_IND  is not allowed (Only CH):etat=%d\n",rrm->state);   
        
}

/*!
*******************************************************************************
\brief RRC response to sensing_meas_req
*/
void rrc_sensing_meas_resp(
    Instance_t    inst     , //!< Identification de l'instance
    Transaction_t Trans_id   //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    if ( rrm->state == CLUSTERHEAD )
    {
        //fprintf(stderr, "[RRM] RRC_SENSING_MEAS_RESP  is not coded %s %d \n",__FILE__ , __LINE__ );   
        
        transact_t *pTransact ;
        
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
        pTransact = get_item_transact(rrm->rrc.transaction,Trans_id ) ;
        if ( pTransact == NULL )
        {
            fprintf(stderr,"[RRM] rrc_sensing_meas_resp (%d) unknown transaction\n",Trans_id);
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        }
        else
        {
            del_item_transact( &(rrm->rrc.transaction),Trans_id ) ;
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        }       
    }
    else
        fprintf(stderr,"[RRM] RRC_SENSING_MEAS_RESP (%d) is not allowed (Only CH):etat=%d\n",Trans_id,rrm->state);          
}
    
/*!
*******************************************************************************
\brief RRC Connection Establishment indication.  Message received by RRM in CH 
        at completion of attachment phase of a new MR (after configuration MR IPAddr). 
        Here L3_info contains MR IPAddr. Message received by RRCI in MR after 
        configuration of initial RBs and reception of CH IPAddr.  Here L3_info 
        contains CH IPAddr.  For MR the RBID's of basic IP services are also required.
*/
void rrc_cx_establish_ind(
    Instance_t     inst      , //!< Identification de l'instance
    L2_ID          L2_id     , //!< Layer 2 (MAC) ID
    Transaction_t  Trans_id  , //!< Transaction ID
    unsigned char *L3_info   , //!< Optional L3 Information
    L3_INFO_T      L3_info_t , //!< Type of L3 Information 
    RB_ID          DTCH_B_id , //!< RBID of broadcast IP service (MR only)
    RB_ID          DTCH_id     //!< RBID of default IP service (MR only)
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    if ( rrm->state == CLUSTERHEAD )
    {
        PUT_CMM_MSG( msg_rrm_attach_ind(inst,L2_id,L3_info_t,L3_info, 0 )) ;
    }
    else if ( rrm->state == MESHROUTER )
    {
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
        add_item_transact( &(rrm->rrc.transaction), Trans_id,INT_RRC,RRC_CX_ESTABLISH_IND,0,NO_PARENT);
        pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;

        pthread_mutex_lock( &( rrm->cmm.exclu ) ) ;
        rrm->cmm.trans_cnt++ ;
        add_item_transact( &(rrm->cmm.transaction), rrm->cmm.trans_cnt,INT_CMM,RRCI_ATTACH_REQ,Trans_id,PARENT);
        pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;       

        PUT_CMM_MSG( msg_rrci_attach_req(inst,L2_id,L3_info_t,L3_info, DTCH_B_id, DTCH_id ,rrm->cmm.trans_cnt)) ;
    }
    else
        fprintf(stderr,"[RRM] RRC_CX_ESTABLISH_IND (%d) is not allowed (Only CH):etat=%d\n",Trans_id,rrm->state);   
    
}
/*!
*******************************************************************************
\brief Clusterhead PHY-Synch Indication
 */
void rrc_phy_synch_to_CH_ind(
        Instance_t   inst     , //!< Identification de l'instance
        unsigned int Ch_index , //!< Clusterhead index
        L2_ID        L2_id 
        )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    if ( (rrm->state == ISOLATEDNODE)  || (rrm->state == MESHROUTER) )
    {
        rrm->state = MESHROUTER ; 
        
        /* Memorisation du L2_id du noeud ( c'est le niveau RRC qui a l'info ) */
        memcpy( &rrm->L2_id,  &L2_id, sizeof(L2_ID));

        PUT_RRC_MSG( 
                        msg_rrci_init_mr_req( inst, 
                                rrm->rrc.trans_cnt,
                                &Lchan_desc[QOS_SRB0], 
                                &Lchan_desc[QOS_SRB1], 
                                Ch_index) 
                );  
                    

    }
    else
        fprintf(stderr,"[RRM] RRC_PHY_SYNCH_TO_CH_IND is not allowed (Only IN):etat=%d\n",rrm->state);
}

/*!
*******************************************************************************
\brief L3 Connection Attachment confirmation.  Message sent by CMM in MR at 
       completion of L3 attachment phase of a new MR Here L3_info contains 
       MR IPAddr. 
*/
void cmm_attach_cnf(
    Instance_t     inst      , //!< Identification de l'instance
    L2_ID          L2_id     , //!< L2_id of CH ( Mesh Router can see 2 CH )
    L3_INFO_T      L3_info_t , //!< Type of L3 Information
    unsigned char *L3_info   , //!< L3 addressing Information
    Transaction_t  Trans_id    //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    if ( rrm->state == MESHROUTER )
    {
        transact_t *pTransact ;
        
        pthread_mutex_lock( &( rrm->cmm.exclu ) ) ;
        pTransact = get_item_transact(rrm->cmm.transaction,Trans_id ) ;
        if ( pTransact == NULL )
        {
            fprintf(stderr,"[RRM] CMM_ATTACH_CNF (%d): unknown transaction\n",Trans_id);
            pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
        }
        else
        {
            unsigned int parent_id     = pTransact->parent_id ;
            unsigned int status_parent = pTransact->parent_status ;

            del_item_transact( &(rrm->cmm.transaction),Trans_id ) ;
            pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
            
            if ( status_parent )
            {
                transact_t *pTransactParent ;
                            
                pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
                pTransactParent =get_item_transact(rrm->rrc.transaction,parent_id ) ;
                if ( pTransactParent != NULL )
                {
                    PUT_RRC_MSG( msg_rrci_cx_establish_resp(inst,pTransactParent->id,L2_id,L3_info,L3_info_t ));
                    del_item_transact( &(rrm->rrc.transaction),pTransactParent->id ) ;
                }
                else // la transaction parent est inconnue, on ne fait rien
                    fprintf(stderr,"[RRM] CMM_ATTACH_CNF (%d) : the parent transaction (%d) is unknown\n",Trans_id,parent_id);  
                    
                pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;

            }
        }           
    }
    else
        fprintf(stderr,"[RRM] CMM_ATTACH_CNF (%d) is not allowed (Only MR):etat=%d\n",Trans_id,rrm->state);     
}
/*!
*******************************************************************************
\brief RRC sensing measurement indication 
*/
void rrc_sensing_meas_ind(
	Instance_t      inst         , //!< Identification de l'instance
	L2_ID           L2_id        , //!< Layer 2 ID (MAC) of sensing node
	unsigned int    NB_meas      , //!< Layer 2 ID (MAC) of sensing node
	SENSING_MEAS_T *Sensing_meas , //!< Sensing Information
	Transaction_t   Trans_id       //!< Transaction ID
    )
{
    int i ;
    
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    if ( rrm->state == CLUSTERHEAD )
    {
        pthread_mutex_lock(   &( rrm->rrc.exclu )  ) ;
        
        // update database of neighbor
        set_Sensing_meas_neighbor( rrm->rrc.pNeighborEntry, &L2_id , NB_meas, Sensing_meas );   
            
        pthread_mutex_unlock( &( rrm->rrc.exclu )  ) ;
        // send the response to rrc 
        PUT_RRC_MSG( msg_rrm_sensing_meas_resp(inst,Trans_id) );

        for (  i = 0 ; i< NB_meas ; i++)
        {
            pthread_mutex_lock( &( rrm->pusu.exclu ) ) ;
            rrm->pusu.trans_cnt++ ;
            add_item_transact( &(rrm->pusu.transaction), rrm->pusu.trans_cnt,INT_PUSU,RRM_SENSING_INFO_IND,0,NO_PARENT);
            pthread_mutex_unlock( &( rrm->pusu.exclu ) ) ;  
                
            PUT_PUSU_MSG( msg_rrm_sensing_info_ind(inst, L2_id,Sensing_meas[i].L2_id, Sensing_meas[i].Rssi, rrm->pusu.trans_cnt ) ) ;
        }

    }
    else
        fprintf(stderr,"[RRM] RRC_SENSING_MEAS_IND is not allowed (Only CH):etat=%d\n",rrm->state);
}



