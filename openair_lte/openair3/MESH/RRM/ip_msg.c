/*!
*******************************************************************************

\file       ip_msg.c

\brief      Fonctions permettant le formattage des donnees pour l'envoi d'un
            message sur le socket entre le  RRC et le RRM

\author     IACOBELLI Lorenzo

\date       27/04/10


\par     Historique:
        

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

#include "L3_rrc_defs.h"
#include "rrm_sock.h"
#include "L3_rrc_interface.h"
#include "ip_msg.h"
#include "rrm_util.h"

#ifdef TRACE
//! Macro creant la chaine a partir du nom de la variable
#define STRINGIZER(x) #x
//! Tableau pour le mode trace faisant la translation entre le numero et le nom du message
const char *Str_msg_ip[NB_MSG_IP] = {
    STRINGIZER(UPDATE_SENS_RESULTS_3    ), 
    STRINGIZER(OPEN_FREQ_QUERY_4        ),
    STRINGIZER(UPDATE_OPEN_FREQ_7       ),
    STRINGIZER(UPDATE_SN_OCC_FREQ_5     )/*,
    STRINGIZER(RRM_OPEN_FREQ            ),
    STRINGIZER(RRM_UPDATE_SN_FREQ       ),
    STRINGIZER(RRC_UPDATE_SN_FREQ       ),
    STRINGIZER(RRM_CLUST_SCAN_REQ       ),
    STRINGIZER(RRC_CLUST_SCAN_REQ       ),
    STRINGIZER(RRM_CLUST_SCAN_CONF      ),
    STRINGIZER(RRM_CLUST_MON_REQ        ),
    STRINGIZER(RRC_CLUST_MON_REQ        ),
    STRINGIZER(RRM_CLUST_MON_CONF       ),
    STRINGIZER(RRM_END_SCAN_CONF        ),
    STRINGIZER(RRM_INIT_CONN_REQ        ),
    STRINGIZER(RRC_INIT_CONN_CONF       ),
    STRINGIZER(RRM_FREQ_ALL_PROP        ),
    STRINGIZER(RRC_FREQ_ALL_PROP_CONF   ),
    STRINGIZER(RRM_REP_FREQ_ALL         ),
    STRINGIZER(RRC_REP_FREQ_ACK         ),
    STRINGIZER(RRC_INIT_CONN_REQ        ),
    STRINGIZER(RRM_CONN_SET             ),
    STRINGIZER(RRC_FREQ_ALL_PROP        ),
    STRINGIZER(RRM_FREQ_ALL_PROP_CONF   ),
    STRINGIZER(RRC_REP_FREQ_ALL         ),
    STRINGIZER(RRM_REP_FREQ_ACK         )  */

} ;
#endif


/*!
*******************************************************************************
\brief  This function initialize the message header 
\return any return value 
*/
static void init_ip_msg_head( 
    msg_head_t    *msg_head , //!< message header to initialize
    Instance_t     inst     , //!< Instance ID
    MSG_IP_T  msg_type , //!< type of message to initialize
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
///! MESSAGES SENT VIA IP

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        update_sens_results_3().
\return message formate
*/

msg_t *msg_update_sens_results_3( 
    Instance_t inst, 
    L2_ID L2_id,                //!< FC L2_id
    unsigned int NB_info,
    Sens_ch_t *Sens_meas, 
    Transaction_t Trans_id
    )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ; 
        
    if ( msg != NULL )
    {
        unsigned int size = sizeof( rrm_update_sens_t );// + (NB_info-1) * sizeof(Sens_ch_t) ; //mod_lor_10_04_23
        
        rrm_update_sens_t *p = RRM_CALLOC2(rrm_update_sens_t , size ) ;

        if ( p != NULL )
        {
            //fprintf(stderr,"rrmUSR 1 \n");//dbg
            init_ip_msg_head(&(msg->head),inst, UPDATE_SENS_RESULTS_3, size ,Trans_id);

            memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
        
            p->NB_info       = NB_info    ;
            p->info_time     = 0 ;
            
            if ( NB_info > 0 )
            {
                memcpy( p->Sens_meas , Sens_meas, NB_info * sizeof(Sens_ch_t) )  ;
            }
            //fprintf(stdout,"msg_rrm_update_sens()2 : NB_chan %d\n", p->NB_info);//dbg
            //fprintf(stdout,"NB_chan %d\n", p->NB_info);
              //  for (int i=0; i<NB_info; i++)
               // Sens_ch_t *ch = p->Sens_meas; ch!=NULL; ch=ch->next)
            //fprintf(stdout,"channel in msg arr: %d\n", ch->Ch_id); //dbg
            
        }
        //fprintf(stderr,"rrmUSR end \n");//dbg
        msg->data = (char *) p ;
    }
    
    return msg ;

}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        open_freq_query_4().
\return message formate
*/
msg_t *msg_open_freq_query_4( 
    Instance_t    inst, 
    L2_ID         L2_id           ,
    QOS_CLASS_T   QoS             ,
    Transaction_t Trans_id 
    )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ;

    if ( msg != NULL )
    {
        open_freq_query_t *p = RRM_CALLOC(open_freq_query_t , 1 ) ;

        if ( p != NULL )
        {
            init_ip_msg_head(&(msg->head),inst,OPEN_FREQ_QUERY_4, sizeof( open_freq_query_t ) ,Trans_id);
            memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
            p->QoS = QoS;
        }
        msg->data = (char *) p ;
    }
    return msg ;
}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_open_freq().
\return message formate
*/
msg_t *msg_update_open_freq_7( 
    Instance_t    inst, 
    L2_ID         L2_id           ,
    unsigned int NB_free_ch,
    CHANNEL_T *fr_channels,
    Transaction_t Trans_id 
    )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ;

    if ( msg != NULL )
    {
        unsigned int size = sizeof( update_open_freq_t );// + (NB_free_ch-1) * sizeof(CHANNEL_T) ;//mod_lor_10_04_23
        
        update_open_freq_t *p = RRM_CALLOC2(update_open_freq_t , size ) ;

        if ( p != NULL )
        {
            init_ip_msg_head(&(msg->head),inst,UPDATE_OPEN_FREQ_7, size ,Trans_id);
            memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
            p->NB_chan       = NB_free_ch    ;
            
            if ( NB_free_ch > 0 )
            {
                memcpy( p->fr_channels , fr_channels, NB_free_ch * sizeof(CHANNEL_T) )  ;
            }
        }
        msg->data = (char *) p ;
    }
    return msg ;
}

/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_update_SN_freq().
\return message formate
*/

msg_t *msg_update_SN_occ_freq_5(
        Instance_t inst             , //!< instance ID 
        L2_ID L2_id                 , //!< Layer 2 (MAC) ID of FC
        unsigned int NB_chan        ,
        unsigned int *occ_channels      , 
        Transaction_t Trans_id        //!< Transaction ID
        )
            
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ;
    //fprintf(stdout,"rrc_end_scan_ord() cp1\n"); //dbg

    if ( msg != NULL )
    {
        unsigned int size = sizeof( update_SN_occ_freq_t );// + (NB_chan-1) * sizeof(unsigned int) ;//mod_lor_10_04_23
        update_SN_occ_freq_t *p = RRM_CALLOC2(update_SN_occ_freq_t , size ) ;

        if ( p != NULL )
        {
        
            
            init_ip_msg_head(&(msg->head),inst,UPDATE_SN_OCC_FREQ_5, size,Trans_id);
            
            memcpy( p->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
            
        
            p->NB_chan = NB_chan;
            if ( NB_chan != 0 ){
                
                memcpy( p->occ_channels, occ_channels, NB_chan*sizeof(unsigned int) );
                
            }
  
       
        }
        msg->data = (char *) p ;
        
    }
    return msg ;
}
