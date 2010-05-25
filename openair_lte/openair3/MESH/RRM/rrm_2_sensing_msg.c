/*!
*******************************************************************************

\file       rrm_2_rrc_msg.c

\brief      Fonctions permettant le formattage des donnees pour l'envoi d'un
            message sur le socket entre le  RRC et le RRM

\author     BURLOT Pascal

\date       16/07/08


\par     Historique:
        P.BURLOT 2009-01-20 
            + L2_id: ajout de memcpy() dans msg_rrci_cx_establish_resp()

        L. IACOBELLI 2010-01-05
            + new messages for sensing
            + messages sent via ip

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
//#include "rrm_sock.h"
//#include "L3_rrc_interface.h"
#include "sensing_rrm_msg.h"
#include "rrm_util.h"

#ifdef TRACE
//! Macro creant la chaine a partir du nom de la variable
#define STRINGIZER(x) #x
//! Tableau pour le mode trace faisant la translation entre le numero et le nom du message
const char *Str_msg_sns_rrm[NB_MSG_SNS_RRM] = {
    STRINGIZER(SNS_UPDATE_SENS          ),
    STRINGIZER(RRM_SCAN_ORD             ),
    STRINGIZER(RRM_END_SCAN_ORD         ),
    STRINGIZER(SNS_END_SCAN_CONF        )
} ;
#endif

/*!
*******************************************************************************
\brief  This function initialize the message header
\return any return value
*/
static void init_sensing_msg_head(
    msg_head_t    *msg_head , //!< message header to initialize
    Instance_t     inst     , //!< instance ID
    MSG_SENSING_RRM_T  msg_type , //!< type of message to initialize
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
\brief  La fonction formate en un message les parametres de la fonction
        rrm_scan_ord().
\return message formate
*/

msg_t *msg_rrm_scan_ord( 
			Instance_t inst             , ///< instance ID 
			unsigned int NB_chan        , ///< Number of channels to sens
			unsigned int Meas_tpf       , ///< time on each carrier           //mod_lor_10_02_19
			unsigned int Overlap        , ///< overlap factor (percentage)    //mod_lor_10_02_19
			unsigned int Sampl_nb       , ///< number of samples per sub-band //mod_lor_10_04_01
			Sens_ch_t    *ch_to_scan    , ///< Vector of channels to scan     //mod_lor_10_02_19 
			Transaction_t Trans_id        ///< Transaction ID
			
        )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ;

    if ( msg != NULL )
    {
        unsigned int size;
        /*if ( NB_chan != 0 ){
            size = sizeof( rrm_scan_ord_t ) + (NB_chan-1) * sizeof(unsigned int) ;
            
        }else*/
            size =  sizeof( rrm_scan_ord_t );
        rrm_scan_ord_t *p = RRM_CALLOC2(rrm_scan_ord_t , size ) ;
       

        if ( p != NULL )
        {
            init_sensing_msg_head(&(msg->head),inst,RRM_SCAN_ORD, size ,Trans_id);

            p->Meas_tpf = Meas_tpf;     //mod_lor_10_02_19
            p->Overlap = Overlap;       //mod_lor_10_02_19
            p->Sampl_nb = Sampl_nb; //mod_lor_10_02_19
            p->NB_chan = NB_chan;
            
          
            if ( NB_chan != 0 ){
                
                memcpy( p->ch_to_scan, ch_to_scan, NB_chan*sizeof(Sens_ch_t) );
                
            }
       
        }
        msg->data = (char *) p ;
    }
    return msg ;
}



/*!
*******************************************************************************
\brief  La fonction formate en un message les parametres de la fonction 
        rrm_end_scan_ord().
\return message formate
*/

msg_t *msg_rrm_end_scan_ord(
        Instance_t inst             , //!< instance ID 
        unsigned int NB_chan        ,
        unsigned int *channels      , 
        Transaction_t Trans_id        //!< Transaction ID
        )
            
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ;
    //fprintf(stdout,"rrc_end_scan_ord() cp1\n"); //dbg

    if ( msg != NULL )
    {
        unsigned int size = sizeof( rrm_end_scan_ord_t ); //+ (NB_chan-1) * sizeof(unsigned int) ;
        rrm_end_scan_ord_t *p = RRM_CALLOC2(rrm_end_scan_ord_t , size ) ;

        if ( p != NULL )
        {
        
            
            init_sensing_msg_head(&(msg->head),inst,RRM_END_SCAN_ORD, size,Trans_id);
            
        
            p->NB_chan = NB_chan;
            if ( NB_chan != 0 ){
                
                memcpy( p->channels, channels, NB_chan*sizeof(unsigned int) );
                
            }
  
       
        }
        msg->data = (char *) p ;
        
    }
    return msg ;
}

