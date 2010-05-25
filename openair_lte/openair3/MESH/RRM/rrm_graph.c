/*!
*******************************************************************************

\file       rrm_graph.c

\brief      RRM (Radio Ressource Manager )

            Cette application a pour objet
                - de gérer la ressource radio du cluster
                - de commander le RRC pour l'ouverture de RB
                - de recevoir des commandes du CMM
                - de gérer le voisinage

\author     IACOBELLI Lorenzo

\date       20/04/2010


\par     Historique:
        

*******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <pthread.h>

#include "debug.h"

#include "L3_rrc_defs.h"
#include "L3_rrc_interface.h"
#include "cmm_rrm_interface.h"
#include "rrm_sock.h"
#include "rrc_rrm_msg.h"
#include "ip_msg.h" //mod_lor_10_04_27
#include "sensing_rrm_msg.h"
#include "cmm_msg.h"
#include "pusu_msg.h"
#include "msg_mngt.h"
#include "neighbor_db.h"
#include "rb_db.h"
#include "sens_db.h"
#include "channels_db.h"
#include "transact.h"
#include "rrm_constant.h"
#include "rrm_util.h"
#include "rrm.h"


/*
** ----------------------------------------------------------------------------
** DEFINE LOCAL
** ----------------------------------------------------------------------------
*/
//mod_lor_10_05_18++
/*!
*******************************************************************************
\brief Definition of IP @ in main entities. i.e. they have to correspond 
        to the ones in node_info vector in emul_interface.c
*/
 static unsigned char FC_L3id [4]={0x0A,0x00,0x01,0x01};
 static unsigned char BTS_L3id [4]={0x0A,0x00,0x02,0x02};
 static unsigned char CH_COLL_L3id [4]={0x0A,0x00,0x02,0x02};
//mod_lor_10_05_18--
/*
** ----------------------------------------------------------------------------
** DECLARATION DE NOUVEAU TYPE
** ----------------------------------------------------------------------------
*/

/*!
*******************************************************************************
\brief Structure de data passe en parametre au threads
*/
struct data_thread {
    char *name              ; ///< Nom du thread
    char *sock_path_local   ; ///< fichier du "rrm->..." pour le socket Unix
    char *sock_path_dest    ; ///< fichier du "...->rrm " pour le socket Unix
    sock_rrm_t  s           ; ///< Descripteur du socket
}  ;

//mod_lor_10_01_25++
struct data_thread_int {
    char *name              ; ///< Nom du thread
    unsigned char *sock_path_local  ; ///< local IP address for internet socket
    int local_port          ; ///< local IP port for internet socket
    unsigned char *sock_path_dest   ; ///< dest IP address for internet socket
    int dest_port           ; ///< dest IP port for internet socket
    sock_rrm_int_t  s       ; ///< Descripteur du socket
    int instance            ; ///<instance rrm 
}  ;
//mod_lor_10_01_25--

#ifdef RRC_KERNEL_MODE

#define RRC2RRM_FIFO 14
#define RRM2RRC_FIFO 15

#define RX_MSG_STARTED 0; //mod_lor_10_01_25
/*!
*******************************************************************************
\brief Structure regroupant les handles des fifos pour la communication en
       mode KERNEL
*/
typedef struct{
    int rrc_2_rrm_fifo;
    int rrm_2_rrc_fifo;
}RRM_FIFOS; 

#endif /* RRC_KERNEL_MODE */

/*
** ----------------------------------------------------------------------------
** DECLARATION DES VARIABLES GLOBALES PUBLIQUES
** ----------------------------------------------------------------------------
*/
rrm_t rrm_inst[MAX_RRM] ;
int   nb_inst = -1 ;

/*
** ----------------------------------------------------------------------------
** DECLARATION DES VARIABLES GLOBALES PRIVEES
** ----------------------------------------------------------------------------
*/
#ifdef RRC_KERNEL_MODE
static RRM_FIFOS Rrm_fifos;
#endif

static int flag_not_exit = 1 ;
static pthread_t pthread_recv_rrc_msg_hnd,
                 pthread_recv_cmm_msg_hnd ,

                 pthread_send_rrc_msg_hnd ,

                 pthread_send_cmm_msg_hnd ,

                 pthread_recv_pusu_msg_hnd ,
                 pthread_recv_sensing_msg_hnd ,
                 pthread_send_sensing_msg_hnd ,
                 
                 pthread_recv_int_msg_hnd ,
                 pthread_send_ip_msg_hnd ,

                 pthread_ttl_hnd ;
static unsigned int cnt_timer = 0;

#ifdef TRACE
static FILE *cmm2rrm_fd  = NULL ;
static FILE *rrc2rrm_fd  = NULL ;
static FILE *pusu2rrm_fd = NULL ;
static FILE *sensing2rrm_fd = NULL ;
static FILE *ip2rrm_fd = NULL ;
#endif
static FILE *output_2 = NULL; //mod_lor_10_04_20

/*
** ----------------------------------------------------------------------------
** DECLARATION DES FONCTIONS
** ----------------------------------------------------------------------------
*/
//mod_lor_10_04_20++
static msg_t *msg_graph_resp(
    Instance_t    inst     , //!< instance ID
    int msg_type  //!< type of message
        )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ;

    if ( msg != NULL )
    {
        msg->head.start    = START_MSG ; 
        msg->head.msg_type = 0xFF & msg_type ;
        msg->head.inst     = inst  ;
        msg->head.Trans_id = 0  ;
        msg->head.size     = 0;
        msg->data = NULL ;
    }
    return msg ;
}

//mod_lor_10_04_20--
//mod_lor_10_04_21++
typedef struct {
    unsigned int        NB_chan              ; //!< Number of channels 
    unsigned int        NB_val               ; //!< Number of values 
    unsigned int        channels[NB_SENS_MAX]; //!< Vector of channels
    unsigned int        val[NB_SENS_MAX]    ; //!< Vector of values

} gen_sens_info_t ;

static msg_t *msg_generic_sens_resp(
    Instance_t   inst     , //!< instance ID
    int          msg_type,  //!< type of message
    unsigned int NB_chan,//!< Number of channels 
    unsigned int NB_val , //!< Number of values
    unsigned int *channels, //!< Vector of channels
    unsigned int *val    , //!< Vector of values
    Transaction_t trans
    )
{
    msg_t *msg = RRM_CALLOC(msg_t , 1 ) ;

    if ( msg != NULL )
    {
        unsigned int size = sizeof( gen_sens_info_t );
        gen_sens_info_t *p = RRM_CALLOC(gen_sens_info_t  ,1 ) ;
        if ( p != NULL )
        {
            msg->head.start    = START_MSG ; 
            msg->head.msg_type = 0xFF & msg_type ;
            msg->head.inst     = inst  ;
            msg->head.Trans_id = trans  ;
            msg->head.size     = size;
 
            p->NB_chan = NB_chan;
            if ( NB_chan != 0 )
                memcpy( p->channels, channels, NB_chan*sizeof(unsigned int) );
            p->NB_val = NB_val;
            if ( NB_chan != 0 )
                memcpy( p->val, val, NB_val*sizeof(unsigned int) );
       
        }
        msg->data = (char *) p ;
        
    }
    return msg ;
        
    
}

//mod_lor_10_04_21--

/*!
*******************************************************************************
\brief  thread de traitement des ttl des transactions (rrc ou cmm).

\return NULL
*/
static void * thread_processing_ttl (
    void * p_data /**< parametre du pthread */
    )
{
    int ii ;
    fprintf(stderr,"TTL :starting ... \n"); fflush(stdout);

    while ( flag_not_exit)
    {
        for ( ii = 0 ; ii<nb_inst ; ii++ )
        {
            rrm_t *rrm = &rrm_inst[ii] ;

            pthread_mutex_lock(   &( rrm->cmm.exclu )  ) ;
            dec_all_ttl_transact( rrm->cmm.transaction ) ;
            // Trop simpliste et pas fonctionnel , il faut faire une gestion des erreurs de transaction
            //if (rrm->cmm.transaction!=NULL)
              //  fprintf(stderr,"delete on cmm of %d\n", ii); //dbg
            del_all_obseleted_transact( &(rrm->cmm.transaction));
            pthread_mutex_unlock( &( rrm->cmm.exclu )  ) ;

            pthread_mutex_lock(   &( rrm->rrc.exclu )  ) ;
            dec_all_ttl_transact( rrm->rrc.transaction ) ;
            // idem :commentaire ci-dessus
            //if (rrm->rrc.transaction!=NULL)
              //  fprintf(stderr,"delete on rrc of %d\n", ii); //dbg
            del_all_obseleted_transact( &(rrm->rrc.transaction));
            pthread_mutex_unlock( &( rrm->rrc.exclu )  ) ;

            pthread_mutex_lock(   &( rrm->pusu.exclu )  ) ;
            dec_all_ttl_transact( rrm->pusu.transaction ) ;
            // idem :commentaire ci-dessus
            del_all_obseleted_transact( &(rrm->pusu.transaction));
            pthread_mutex_unlock( &( rrm->pusu.exclu )  ) ;

            pthread_mutex_lock(   &( rrm->sensing.exclu )  ) ;
            dec_all_ttl_transact( rrm->sensing.transaction ) ;
            // idem :commentaire ci-dessus
            del_all_obseleted_transact( &(rrm->sensing.transaction));
            pthread_mutex_unlock( &( rrm->sensing.exclu )  ) ;
            
            //mod_lor_10_01_25++
            pthread_mutex_lock(   &( rrm->ip.exclu )  ) ;
            dec_all_ttl_transact( rrm->ip.transaction ) ;
            // idem :commentaire ci-dessus
            del_all_obseleted_transact( &(rrm->ip.transaction));
            pthread_mutex_unlock( &( rrm->ip.exclu )  ) ;
            //mod_lor_10_01_25--*/
            
        }
        cnt_timer++;
        usleep( 2000*1000 ) ;//mod_lor_10_03_01: incrementing timeout
    }
    fprintf(stderr,"... stopped TTL\n"); fflush(stdout);
    return NULL;

}

/*!
*******************************************************************************
\brief  thread de traitement des messages sortants sur les sockets (rrc ou cmm).

\return NULL
*/
static void * thread_send_msg_cmm (
    void * p_data /**< parametre du pthread */
    )
{
    int ii ;
    int no_msg ;
    fprintf(stderr,"Thread Send Message: starting ... \n");
    fflush(stderr);
    file_msg_t *pItem ;

    while ( flag_not_exit)
    {
        no_msg = 0  ;
        for ( ii = 0 ; ii<nb_inst ; ii++ )
        {
            rrm_t      *rrm = &rrm_inst[ii] ;

            pItem = get_msg( &(rrm->file_send_cmm_msg) ) ;
            

            if ( pItem == NULL )
                no_msg++;
            else
            {
                int r =  send_msg( pItem->s, pItem->msg );
                WARNING(r!=0);
            }
            RRM_FREE( pItem ) ;
        }

        if ( no_msg==nb_inst ) // Pas de message
            usleep(1000);
    }
    fprintf(stderr,"... stopped Thread Send Message\n"); fflush(stderr);
    return NULL;
}

/*!
*******************************************************************************
\brief  thread de traitement des messages sortants sur les sockets (rrc ou cmm).

\return NULL
*/
static void * thread_send_msg_rrc (
    void * p_data /**< parametre du pthread */
    )
{
    int ii ;

    int no_msg ;
    fprintf(stderr,"Thread Send Message To RRC: starting ... \n");
    fflush(stderr);
    file_msg_t *pItem ;
    while ( flag_not_exit)
    {
        no_msg = 0  ;
        for ( ii = 0 ; ii<nb_inst ; ii++ )
        {
            rrm_t      *rrm = &rrm_inst[ii] ;

            pItem = get_msg( &(rrm->file_send_rrc_msg) ) ;
            

            if ( pItem == NULL )
                no_msg++;
            else
            {
            
#ifdef RRC_KERNEL_MODE
                // envoi du header
                status = write (Rrm_fifos.rrm_2_rrc_fifo,(char*) pItem->msg, sizeof(msg_head_t) );
                if ( pItem->msg->head.size >0)
                    status = write (Rrm_fifos.rrm_2_rrc_fifo,(pItem->msg)->data, pItem->msg->head.size);
                //printf( "status write: %d \n",status);
                //printf("send msg to rrc [%d] id msg: %d \n", (pItem->msg)->head.inst, (pItem->msg)->head.msg_type );
#else
                int r =  send_msg( pItem->s, pItem->msg );
                WARNING(r!=0);
#endif
            }
            RRM_FREE( pItem ) ;
        }

        if ( no_msg==nb_inst ) // Pas de message
            usleep(1000);
    }
    fprintf(stderr,"... stopped Thread Send Message\n"); fflush(stderr);
    return NULL;
}

//mod_lor_10_01_25++
/*!
*******************************************************************************
\brief  thread de traitement des messages sortants sur les sockets (rrc ou cmm).

\return NULL
*/
static void * thread_send_msg_ip (
    void * p_data /**< parametre du pthread */
    )
{
    int ii ;
    int no_msg ;
    fprintf(stderr,"Thread Send Message IP: starting ... \n");
    fflush(stderr);
    file_msg_t *pItem ;

    while ( flag_not_exit)
    {
        no_msg = 0  ;
        for ( ii = 0 ; ii<nb_inst ; ii++ )
        {
            rrm_t      *rrm = &rrm_inst[ii] ;

            pItem = get_msg( &(rrm->file_send_ip_msg) ) ;


            if ( pItem == NULL )
                no_msg++;
            else
            {
                 //fprintf(stderr,"Thread Send Message inst %d socket %d\n", ii, rrm->ip.s->s); //dbg
               // if (pItem->msg->head.msg_type == 26)
                 //   msg_fct( "IP -> UPDATE_SENSING_RESULTS_3 inst: %d sockid %d\n", ii, rrm->ip.s->s);//dbg
    
                int r =  send_msg_int( rrm->ip.s, pItem->msg );
           
                WARNING(r!=0);
            }
            RRM_FREE( pItem ) ;
        }

        if ( no_msg==nb_inst ) // Pas de message
            usleep(1000);
    }
    fprintf(stderr,"... stopped Thread Send Message\n"); fflush(stderr);
    return NULL;
}

/*!
*******************************************************************************
\brief  thread de traitement des messages sortants sur les sockets (rrc ou cmm).

\return NULL
*/
static void * thread_send_msg_sensing (
				       void * p_data /**< parametre du pthread */
				       )
{
  int ii ;
  
  int no_msg ;
  fprintf(stderr,"Thread Send Message To Sensing Unit: starting ... \n");
  fflush(stderr);
  file_msg_t *pItem ;
  while ( flag_not_exit)
    {
      no_msg = 0  ;
      for ( ii = 0 ; ii<nb_inst ; ii++ )
        {
	  rrm_t      *rrm = &rrm_inst[ii] ;
	  
	  pItem = get_msg( &(rrm->file_send_sensing_msg) ) ;
	  
	  
	  if ( pItem == NULL )
	    no_msg++;
	  else
            {
                int r =  send_msg( pItem->s, pItem->msg );
                WARNING(r!=0);
	      
            }
	  RRM_FREE( pItem ) ;
        }
      
      if ( no_msg==nb_inst ) // Pas de message
	usleep(1000);
    }
  fprintf(stderr,"... stopped Thread Send Message\n"); fflush(stderr);
  return NULL;
}

//mod_lor_10_01_25--*/



/*!
*******************************************************************************
\brief  thread de traitement des messages entrant sur une interface (rrc, cmm ou sensing).

\return NULL
*/

static void * thread_recv_msg (
    void * p_data /**< parametre du pthread */
    )
{
    msg_t *msg ;
    struct data_thread *data = (struct data_thread *) p_data;
    int sock ;

    fprintf(stderr,"%s interfaces :starting ... %s %s\n",data->name , data->sock_path_local, data->sock_path_dest);
    fflush(stderr);

    /* ouverture des liens de communications */
    sock = open_socket( &data->s,  data->sock_path_local, data->sock_path_dest ,0 );

    if ( sock != -1 )
    {
        fprintf(stderr,"   %s -> socket =  %d\n",data->name , sock );
        fflush(stderr);

        while (flag_not_exit)
        {
            msg = (msg_t *) recv_msg(&data->s) ;
            if (msg == NULL )
            {
                fprintf(stderr,"Server closed connection\n");
                flag_not_exit = 0;
            }
            else
            {
                int inst = msg->head.inst ;
                rrm_t      *rrm = &rrm_inst[inst];

                put_msg( &(rrm->file_recv_msg), 0, &data->s, msg) ;//mod_lor_10_01_25
            }
        }
        close_socket(&data->s) ;
    }

    fprintf(stderr,"... stopped %s interfaces\n",data->name);
    return NULL;
}

//mod_lor_10_01_25++
/*!
*******************************************************************************
\brief  thread de traitement des messages entrant via ip.

\return NULL
*/

static void * thread_recv_msg_int (
    void * p_data /**< parametre du pthread */
    )
{
    msg_t *msg ;
    struct data_thread_int *data = (struct data_thread_int *) p_data; 
    rrm_t      *rrm = &rrm_inst[data->instance]; 
    int sock ;

    fprintf(stderr,"%s interfaces :starting on inst. %d ... ",data->name, data->instance  );
    fprintf(stderr,"\n");//dbg
               
    fflush(stderr);

    /* ouverture des liens de communications */
    sock = open_socket_int( &data->s,  data->sock_path_local, data->local_port, data->sock_path_dest, data->dest_port,0 );
    data->s.s = sock;
    memcpy(rrm->ip.s, &(data->s), sizeof(sock_rrm_int_t));
 
    if ( sock != -1 )
    {
        fprintf(stderr,"   %s -> socket =  %d\n",data->name , sock );
        fflush(stderr);

        while (flag_not_exit)
        {
            
            msg = (msg_t *) recv_msg_int(rrm->ip.s) ;

            if (msg == NULL )
            {
                fprintf(stderr,"Server closed connection\n");
                flag_not_exit = 0;
            }
            else
            {
                //fprintf(stderr,"msg received from %X \n", rrm->ip.s->in_dest_addr.sin_addr.s_addr);
                put_msg( &(rrm->file_recv_msg), 1, &data->s, msg) ;
                
            }
        }
        close_socket_int(&data->s) ;
    }

    fprintf(stderr,"... stopped %s interfaces\n",data->name);
    return NULL;
}
//mod_lor_10_01_25--

/*******************************************************************************/
#ifdef RRC_KERNEL_MODE
char Header_buf[sizeof(msg_head_t)];
char Data[2400];
unsigned short  Header_read_idx=0,
                Data_read_idx=0, 
                Data_to_read=0,
                Header_size=sizeof(msg_head_t),
                READ_OK=1;
  
/*!
*******************************************************************************
\brief  thread de traitement des messages entrant sur l'interface fifo du RRC
        en mode KERNEL

\return NULL
*/
static void * thread_recv_msg_fifo (void * p_data )
{
    msg_t      *msg_cpy   ;
    rrm_t      *rrm       ;
    msg_head_t *Header    ;
    int         taille    ;
    int         inst      ;
    int         bytes_read;
    
    msg_fifo("[RRM]: RX MSG_FIFOS %d handler starting....\n",RRC2RRM_FIFO);
    
    while (flag_not_exit)
    {
        if(Header_read_idx < Header_size)
        {
            bytes_read = read(Rrm_fifos.rrc_2_rrm_fifo,&Data[Header_read_idx],Header_size-Header_read_idx);
            if(bytes_read <0) 
                continue;
            Header_read_idx+=bytes_read;
            msg_fifo("[RRM]: RX MSG ON FIFOS %d: Header size %d, bytes_read %d\n",RRC2RRM_FIFO,Header_read_idx,bytes_read);
            if(Header_read_idx == Header_size)
            {
                Header=(msg_head_t*)Data;
                Data_to_read=Header->size;
                Data_read_idx=Header_read_idx;
                msg_fifo("[RRM]: RX MSG ON FIFOS %d: Header read completed, Data size %d\n",RRC2RRM_FIFO,Data_to_read);
                Header = (msg_head_t *) Data;
            }
            else
                continue;
        }
        
        if (Data_to_read > 0 )
        {
            bytes_read = read (Rrm_fifos.rrc_2_rrm_fifo,&Data[Data_read_idx],Data_to_read);
            if(bytes_read <0) 
                continue;
            Data_to_read-=bytes_read;
            Data_read_idx+=bytes_read;
            msg_fifo("[RRM]: RX MSG ON FIFOS %d: data size %d\n",RRC2RRM_FIFO,Data_read_idx-Header_read_idx);
            if(Data_to_read > 0 )
                continue;
            Header_read_idx= 0 ;
            Data_read_idx  = 0 ;
            Data_to_read   = 0 ;
            taille         = Header->size + sizeof(msg_head_t) ;
            msg_cpy        = RRM_MALLOC( msg_t, taille ) ;
            inst           = Header->inst ;
            rrm            = &rrm_inst[inst];
 
            memcpy( msg_cpy, Data , taille ) ;
            msg_fifo("[RRM]: RX MSG ON FIFOS %d: data read completed, Proccess on inst .... %d\n",RRC2RRM_FIFO,inst);
            put_msg( &(rrm->file_recv_msg), 0, rrm->rrc.s, msg_cpy) ; //mod_lor_10_01_25
            msg_fifo("[RRM]: RX MSG ON FIFOS %d: data read completed, Proccess on inst done %d\n",RRC2RRM_FIFO,inst);
        }
        else
        { /* Seulement le header */
            Header_read_idx= 0 ;
            Data_read_idx  = 0 ;
            Data_to_read   = 0 ;
            taille         = sizeof(msg_head_t) ;
            msg_cpy        = RRM_MALLOC( msg_t, taille ) ;
            inst           = Header->inst ;
            rrm            = &rrm_inst[inst];

            memcpy( msg_cpy, Data , taille ) ;
            put_msg( &(rrm->file_recv_msg), 0, rrm->rrc.s, msg_cpy) ;//mod_lor_10_01_25
        }
    }
    return NULL;
}
#endif
/*!
*******************************************************************************
\brief  traitement des messages entrant sur l'interface CMM

\return Auncune valeur
*/
static void processing_msg_cmm(
    rrm_t       *rrm        , ///< Donnee relative a une instance du RRM
    msg_head_t  *header     , ///< Entete du message
    char        *msg        , ///< Message recu
    int         len_msg       ///< Longueur du message
    )
{
#ifdef TRACE
    if ( header->msg_type < NB_MSG_CMM_RRM )
    fprintf(cmm2rrm_fd,"%lf CMM->RRM %d %-30s %d %d\n",get_currentclock(),header->inst,Str_msg_cmm_rrm[header->msg_type], header->msg_type,header->Trans_id);
    else
    fprintf(cmm2rrm_fd,"%lf CMM->RRM %-30s %d %d\n",get_currentclock(),"inconnu", header->msg_type,header->Trans_id);
    fflush(cmm2rrm_fd);
#endif

    switch ( header->msg_type )
    {
        case CMM_CX_SETUP_REQ:
            {
                cmm_cx_setup_req_t *p = (cmm_cx_setup_req_t *) msg ;
                msg_fct( "[CMM]>[RRM]:%d:CMM_CX_SETUP_REQ\n",header->inst);
                if ( cmm_cx_setup_req(header->inst,p->Src,p->Dst,p->QoS_class,header->Trans_id ) )
                { /* RB_ID = 0xFFFF => RB error */
                    put_msg( &(rrm->file_send_cmm_msg), 0,
                                rrm->cmm.s, msg_rrm_cx_setup_cnf(header->inst,0xFFFF , header->Trans_id )) ;//mod_lor_10_01_25
                }
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM +NB_MSG_RRC_RRM;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                }
            break ;
        case CMM_CX_MODIFY_REQ:
            {
                cmm_cx_modify_req_t *p = (cmm_cx_modify_req_t *) msg ;
                msg_fct( "[CMM]>[RRM]:%d:CMM_CX_MODIFY_REQ\n",header->inst);
                cmm_cx_modify_req(header->inst,p->Rb_id,p->QoS_class,header->Trans_id )  ;
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM +NB_MSG_RRC_RRM;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                
            }
            break ;
        case CMM_CX_RELEASE_REQ :
            {
                cmm_cx_release_req_t *p = (cmm_cx_release_req_t *) msg ;
                msg_fct( "[CMM]>[RRM]:%d:CMM_CX_RELEASE_REQ\n",header->inst);
                cmm_cx_release_req(header->inst,p->Rb_id,header->Trans_id )  ;
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM +NB_MSG_RRC_RRM;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                
            }
            break ;
        case CMM_CX_RELEASE_ALL_REQ :
            {
                //cmm_cx_release_all_req_t *p = (cmm_cx_release_all_req_t *) msg ;
                msg_fct( "[CMM]>[RRM]:%d:CMM_CX_RELEASE_ALL_REQ\n",header->inst);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM +NB_MSG_RRC_RRM;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                
            }
            break ;
        case CMM_ATTACH_CNF : ///< The thread that allows 
            {
                cmm_attach_cnf_t *p = (cmm_attach_cnf_t *) msg ;
                msg_fct( "[CMM]>[RRM]:%d:CMM_ATTACH_CNF\n",header->inst);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM +NB_MSG_RRC_RRM;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                
                
              
               //mod_lor_10_01_25++
#ifndef    RRC_EMUL          
                
                if (rrm->ip.s->s == -1){ 
                    //mod_lor_10_05_06++
                    unsigned char tmp [4];
                    /*for (int i=0; i<10;i++ )
                        if (memcmp( &(node_info[i].L2_id), &(p->L2_id), sizeof(L2_ID) )){
                            fprintf(stderr,"Inst. to connect with %d\n",i);
                            break;
                        }//memcpy()*/
                    //mod_lor_10_05_18++: destination addresses depends on 
                    //scenario and on role, they are declared at beginning of file
                    if (SCEN_2_CENTR && rrm->id == 4){
                        tmp[0]=CH_COLL_L3id[0];
                        tmp[1]=CH_COLL_L3id[1];
                        tmp[2]=CH_COLL_L3id[2];
                        tmp[3]=CH_COLL_L3id[3];
                    }
                   else {
                        tmp[0]=FC_L3id[0];
                        tmp[1]=FC_L3id[1];
                        tmp[2]=FC_L3id[2];
                        tmp[3]=FC_L3id[3];
                    }//mod_lor_10_05_18--
                       //unsigned char tmp [4]={0x0A,0x00,0x01,0x01};
                    /*fprintf(stderr,"IP_addr :");//dbg //mod_lor_10_05_06
                    for (int i=0;i<4;i++)//dbg
                        fprintf(stderr," %X",tmp[i]);//dbg
                    fprintf(stderr,"\n");//dbg*/
                    
                    //mod_lor_10_05_06--
                    fprintf(stderr,"IP interface starting inst. %d\n",rrm->id); 
                    int sock = open_socket_int(rrm->ip.s, p->L3_info, 0, tmp, 0, header->inst);
                    if ( sock != -1 )
                    {
                        fprintf(stderr,"   Ip -> socket =  %d\n", rrm->ip.s->s );
                        fflush(stderr);
                    }else
                        fprintf(stderr," Error in IP socket opening \n");
                }else
                        fprintf(stderr," Socket IP for inst %d already opened %d \n",rrm->id,rrm->ip.s->s);
#endif            //mod_lor_10_01_25--*/
                
                cmm_attach_cnf( header->inst, p->L2_id, p->L3_info_t, p->L3_info, header->Trans_id ) ;
                
            }
            break ;
        case CMM_INIT_MR_REQ :
            {
                msg_fct( "[CMM]>[RRM]:%d:CMM_INIT_MR_REQ ????\n",header->inst);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM +NB_MSG_RRC_RRM;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                
            }
            break ;
        case CMM_INIT_CH_REQ :
            {
                cmm_init_ch_req_t *p = (cmm_init_ch_req_t *) msg ;
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM +NB_MSG_RRC_RRM;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                
                //mod_lor_10_03_01++
            
                struct data_thread_int DataIp;
                
                DataIp.name = "IP"             ; ///< Nom du thread
                DataIp.sock_path_local=p->L3_info;///< local IP address for internet socket
                DataIp.local_port = 7000          ; ///< local IP port for internet socket
                //mod_lor_10_03_02++: setting for topology with FC and BTS on instances 0 and 1
                //mod_lor_10_05_18++
                if (rrm->role == FUSIONCENTER){
                    if (SCEN_1)
                        DataIp.sock_path_dest = BTS_L3id ; ///< dest IP address for internet socket
                    else if (SCEN_2_CENTR)
                        DataIp.sock_path_dest = CH_COLL_L3id ; ///< dest IP address for internet socket
                }else if (rrm->role == BTS ||rrm->role == CH_COLL){ //mod_lor_10_04_27
                    DataIp.sock_path_dest = FC_L3id  ; ///< dest IP address for internet socket
                }else 
                    fprintf (stderr, "wrong node role %d \n", rrm->role);
                //mod_lor_10_05_18--
                //mod_lor_10_03_02--
                DataIp.dest_port = 0          ; ///< dest IP port for internet socket
                DataIp.s.s = -1      ; 
                DataIp.instance = rrm->id;
                //fprintf(stderr,"L3_local ");//dbg
                //print_L3_id( IPv4_ADDR,  rrm->L3_info );
                //fprintf(stderr,"\n");//dbg
                
                int ret = pthread_create ( &pthread_recv_int_msg_hnd, NULL, thread_recv_msg_int , &DataIp );
                if (ret)
                {
                    fprintf (stderr, "%s", strerror (ret));
                    exit(-1) ;
                }
                    
                sleep(5);
                //mod_lor_10_03_01--
                cmm_init_ch_req(header->inst,p->L3_info_t,&(p->L3_info[0]));
                msg_fct( "[CMM]>[RRM]:%d:CMM_INIT_CH_REQ\n",header->inst);
                
            }
            break ;

        case CMM_INIT_SENSING :
            {
                cmm_init_sensing_t *p = (cmm_init_sensing_t *) msg ;                
                msg_fct( "[CMM]>[RRM]:%d:CMM_INIT_SENSING\n",header->inst);
                rrm->sensing.sens_active=1;//mod_lor_10_05_07
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM +NB_MSG_RRC_RRM;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                
                cmm_init_sensing(header->inst,p->Start_fr ,p->Stop_fr,p->Meas_band,p->Meas_tpf,
                        p->Nb_channels, p->Overlap,p->Sampl_freq);
                fprintf(output_2,"PROVA\n"); //mod_lor_10_04_20
                
           
            }
            break ;
        case CMM_STOP_SENSING :
            {
                msg_fct( "[CMM]>[RRM]:%d:CMM_STOP_SENSING\n",rrm->id);
                rrm->sensing.sens_active=0;//mod_lor_10_05_07
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM +NB_MSG_RRC_RRM;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                //print_sens_db(rrm->rrc.pSensEntry);//dbg
                cmm_stop_sensing(header->inst);
            }
            break ;
        case CMM_ASK_FREQ :
            {
                msg_fct( "[CMM]>[RRM]:%d:CMM_ASK_FREQ\n",header->inst);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM +NB_MSG_RRC_RRM;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                cmm_ask_freq(header->inst);
            }
            break ;

        default :
            fprintf(stderr,"CMM:\n") ;
            printHex(msg,len_msg,1) ;
    }
}
/*!
*******************************************************************************
\brief  traitement des messages entrant sur l'interface RRC

\return Aucune valeur
*/
static void processing_msg_rrc(
    rrm_t *rrm          , ///< Donnee relative a une instance du RRM
    msg_head_t *header  , ///< Entete du message
    char *msg           , ///< Message recu
    int len_msg           ///< Longueur du message
    )
{
#ifdef TRACE
    if ( header->msg_type < NB_MSG_RRC_RRM )
      fprintf(rrc2rrm_fd,"%lf RRC->RRM %d %-30s %d %d\n",get_currentclock(),header->inst,Str_msg_rrc_rrm[header->msg_type],header->msg_type,header->Trans_id);
    else
    fprintf(rrc2rrm_fd,"%lf RRC->RRM %-30s %d %d\n",get_currentclock(),"inconnu",header->msg_type,header->Trans_id);
    fflush(rrc2rrm_fd);
#endif

    switch ( header->msg_type )
    {
        case RRC_RB_ESTABLISH_RESP:
            {
                msg_fct( "[RRC]>[RRM]:%d:RRC_RB_ESTABLISH_RESP %d \n",header->inst, header->Trans_id );
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_rb_establish_resp(header->inst,header->Trans_id) ;
            }
            break ;
        case RRC_RB_ESTABLISH_CFM:
            {
                rrc_rb_establish_cfm_t *p = (rrc_rb_establish_cfm_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_RB_ESTABLISH_CFM (%d)  %d \n",header->inst,p->Rb_id, header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_rb_establish_cfm(header->inst,p->Rb_id,p->RB_type,header->Trans_id) ;
            }
            break ;

        case RRC_RB_MODIFY_RESP:
            {
                msg_fct( "[RRC]>[RRM]:%d:RRC_RB_MODIFY_RESP  %d \n",header->inst, header->Trans_id);
                rrc_rb_modify_resp(header->inst,header->Trans_id) ;
            }
            break ;
        case RRC_RB_MODIFY_CFM:
            {
                rrc_rb_modify_cfm_t *p = (rrc_rb_modify_cfm_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_RB_MODIFY_CFM (%d) %d \n",header->inst,p->Rb_id, header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_rb_modify_cfm(header->inst,p->Rb_id,header->Trans_id) ;
            }
            break ;

        case RRC_RB_RELEASE_RESP:
            {
                msg_fct( "[RRC]>[RRM]:%d:RRC_RB_RELEASE_RESP %d \n",header->inst, header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_rb_release_resp(header->inst,header->Trans_id) ;
            }
            break ;
        case RRC_MR_ATTACH_IND :
            {
                rrc_MR_attach_ind_t *p = (rrc_MR_attach_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_MR_ATTACH_IND (Node %02d) %d \n",header->inst, p->L2_id.L2_id[0], header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_MR_attach_ind(header->inst,p->L2_id) ;
            }
            break ;
        case RRC_SENSING_MEAS_RESP:
            {
                msg_fct( "[RRC]>[RRM]:%d:RRC_SENSING_MEAS_RESP %d \n",header->inst, header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_sensing_meas_resp(header->inst,header->Trans_id) ;
            }
            break ;
        case RRC_CX_ESTABLISH_IND:
            {
                rrc_cx_establish_ind_t *p = (rrc_cx_establish_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_CX_ESTABLISH_IND (Node %02d) %d \n",header->inst, p->L2_id.L2_id[0], header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                
                rrc_cx_establish_ind(header->inst,p->L2_id,header->Trans_id,
                                    p->L3_info,p->L3_info_t,
                                    p->DTCH_B_id,p->DTCH_id) ;
                //mod_lor_10_01_25++
                if (rrm->state == MESHROUTER)
                    memcpy(rrm->L3_info_corr,p->L3_info, IPv4_ADDR);
               
                    //mod_lor_10_01_25--*/
            }
            break ;
        case RRC_PHY_SYNCH_TO_MR_IND :
            {
                rrc_phy_synch_to_MR_ind_t *p = (rrc_phy_synch_to_MR_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_MR_IND.... (Node %02d) %d \n",header->inst, p->L2_id.L2_id[0], header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_phy_synch_to_MR_ind(header->inst,p->L2_id) ;
                //msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_MR_IND Done\n",header->inst);
            }
            break ;
        case RRC_PHY_SYNCH_TO_CH_IND :
            {
                rrc_phy_synch_to_CH_ind_t *p = (rrc_phy_synch_to_CH_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_CH_IND.... %d (Node %02d) %d \n",header->inst, p->Ch_index, p->L2_id.L2_id[0], header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_phy_synch_to_CH_ind(header->inst,p->Ch_index,p->L2_id ) ;
                //msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_CH_IND Done\n",header->inst);

            }
            break ;
        case RRC_SENSING_MEAS_IND :
            {
                rrc_sensing_meas_ind_t *p  = (rrc_sensing_meas_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_SENSING_MEAS_IND (Node %02d) %d \n",header->inst, p->L2_id.L2_id[0], header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_sensing_meas_ind( header->inst,p->L2_id, p->NB_meas, p->Sensing_meas, header->Trans_id );
            }
            break ;
        case RRC_RB_MEAS_IND :
            {
                rrc_rb_meas_ind_t *p  = (rrc_rb_meas_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_RB_MEAS_IND (Noede %02d) %d \n",header->inst, p->L2_id.L2_id[0], header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_rb_meas_ind( header->inst, p->Rb_id, p->L2_id, p->Meas_mode, p->Mac_rlc_meas, header->Trans_id );
            }
            break ;


        case RRC_INIT_SCAN_REQ :
            {
                rrc_init_scan_req_t *p  = (rrc_init_scan_req_t *) msg ;
              //  fprintf(stdout,"sens_database before:\n");//dbg
              //  print_sens_db( rrm->rrc.pSensEntry );//dbg
                msg_fct( "[RRC]>[RRM]:%d:RRC_INIT_SCAN_REQ  %d \n",header->inst, header->Trans_id);
                rrm->sensing.sens_active=1;//mod_lor_10_04_21
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_init_scan_req( header->inst, p->L2_id, p->Start_fr ,p->Stop_fr,p->Meas_band,p->Meas_tpf,
                        p->Nb_channels, p->Overlap,p->Sampl_freq, header->Trans_id ); 
            //    fprintf(stdout,"sens_database:\n");//dbg
            //    print_sens_db( rrm->rrc.pSensEntry );//dbg
                
            }
            break ;
        case RRC_END_SCAN_CONF :
            {
                rrc_end_scan_conf_t *p  = (rrc_end_scan_conf_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_END_SCAN_CONF  %d (Node ",header->inst, header->Trans_id);
                int inst_sens;
                for ( int i=0;i<8;i++)
                    msg_fct("%02X", p->L2_id.L2_id[i]);
                msg_fct( ")\n");
                //mod_lor_10_04_20++
                //mod_lor_10_04_22++
                for ( inst_sens=0;inst_sens<nb_inst;inst_sens++){
                    if (memcmp(&(rrm_inst[inst_sens].L2_id),  &(p->L2_id),sizeof(L2_ID))==0)
                        break;
                }
                
                //mod_lor_10_04_22--
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(inst_sens,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_end_scan_conf( header->inst, p->L2_id, header->Trans_id );  
            }
            break ;
        case RRC_END_SCAN_REQ :
            {
                rrc_end_scan_req_t *p  = (rrc_end_scan_req_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_END_SCAN_REQ %d \n",header->inst, header->Trans_id);
                rrm->sensing.sens_active=0;//mod_lor_10_04_21
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_end_scan_req( header->inst, p->L2_id, header->Trans_id );  
            }
            break ;
        case RRC_INIT_MON_REQ :
            {
                rrc_init_mon_req_t *p  = (rrc_init_mon_req_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_INIT_MON_REQ %d (Node ",header->inst, header->Trans_id);
                for ( int i=0;i<8;i++)
                    msg_fct("%02X", p->L2_id.L2_id[i]);
                msg_fct( ")\n");
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                rrc_init_mon_req( header->inst, p->L2_id, p->ch_to_scan, p->NB_chan, p->interval, header->Trans_id );  
            }
            break ;

        default :
            fprintf(stderr,"RRC:\n") ;
            printHex(msg,len_msg,1) ;
    }

}

static void processing_msg_sensing(
    rrm_t       *rrm        , ///< Donnee relative a une instance du RRM
    msg_head_t  *header     , ///< Entete du message
    char        *msg        , ///< Message recu
    int         len_msg       ///< Longueur du message
    )
{

#ifdef TRACE
  if ( header->msg_type < NB_MSG_SENSING_RRM )
    fprintf(sensing2rrm_fd,"%lf SENSING->RRM %d %-30s %d %d\n",get_currentclock(),header->inst,Str_msg_sensing_rrm[header->msg_type],header->msg_type,header->Trans_id);
  else
    fprintf(sensing2rrm_fd,"%lf SENSING->RRM %-30s %d %d\n",get_currentclock(),"inconnu",header->msg_type,header->Trans_id);
  fflush(sensing2rrm_fd);
#endif
  
  switch ( (MSG_SENSING_RRM_T)header->msg_type )
    {

        case SNS_UPDATE_SENS :
            {
                rrc_update_sens_t *p  = (rrc_update_sens_t *) msg ;
               
                msg_fct( "[SENSING]>[RRM]:%d:SNS_UPDATE_SENS trans %d\n",header->inst, header->Trans_id);
                if(rrm->sensing.sens_active == 0)//mod_lor_10_04_23
                    break;
               /* for (int i=0; i< p->NB_info; i++){
                    printf("SNS before rrc_update-> CH: %d \n",p->Sens_meas[i].Ch_id);//dbg
                    for (int j=0; j< MAX_NUM_SB; j++)
                        printf("SB %d is_free %d\n",j,p->Sens_meas[i].is_free[j]);
                }*/
                //for (int i =0; i <p->NB_info; i++)//dbg
                //    msg_fct("ch_id: %d\n",p->Sens_meas[i].Ch_id);//dbg
                rrc_update_sens( header->inst, rrm->L2_id, p->NB_info, p->Sens_meas, p->info_time );
                //mod_lor_10_04_20++
                int msg_type = header->msg_type ;
                //mod_lor_10_04_21++
                unsigned int        channels[NB_SENS_MAX]; //!< Vector of channels
                unsigned int        free[NB_SENS_MAX]    ; //!< Vector of values
                unsigned int    Trans = rrm->ip.trans_cnt;
            
                for (int i=0; i< p->NB_info; i++){
                    channels[i]=p->Sens_meas[i].Ch_id;
                    free[i]=p->Sens_meas[i].is_free[0];//TO DO 
                    //printf("SNS -> CH: %d is_free %d\n",channels[i],free[i]);//dbg
                }
                /*for (int i=0; i< p->NB_info; i++){
                    printf("SNS after rrc_update-> CH: %d \n",p->Sens_meas[i].Ch_id);//dbg
                    for (int j=0; j< MAX_NUM_SB; j++)
                        printf("SB %d is_free %d\n",j,p->Sens_meas[i].is_free[j]);
                }*/
                int r =  send_msg( rrm->graph.s,msg_generic_sens_resp(header->inst,msg_type,p->NB_info,p->NB_info,channels,free, Trans) );
                    WARNING(r!=0);
                //mod_lor_10_04_21--
                //mod_lor_10_04_20--
                
                
            
            }
            break ;
    //mod_lor_10_04_14++
         case SNS_END_SCAN_CONF :
            {
                
                msg_fct( "[SENSING]>[RRM]:%d:SNS_END_SCAN_CONF\n",header->inst);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type ;
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                
                sns_end_scan_conf( header->inst );  
            }
            break ;
        default :
            fprintf(stderr,"SENSING:%d:\n",header->msg_type) ;
            printHex(msg,len_msg,1) ;
    }//mod_lor_10_04_14--
}


/*!
*******************************************************************************
\brief  traitement des messages entrant sur l'interface PUSU

\return Aucune valeur
*/
static void processing_msg_pusu(
    rrm_t *rrm          , ///< Donnee relative a une instance du RRM
    msg_head_t *header  , ///< Entete du message
    char *msg           , ///< Message recu
    int len_msg           ///< Longueur du message
    )
{
    transact_t *pTransact ;

    pthread_mutex_lock( &( rrm->pusu.exclu ) ) ;
    pTransact = get_item_transact(rrm->pusu.transaction,header->Trans_id ) ;
    if ( pTransact == NULL )
    {
        fprintf(stderr,"[RRM] %d PUSU Response (%d): unknown transaction\n",header->msg_type,header->Trans_id);
    }
    else
    {
        del_item_transact( &(rrm->pusu.transaction),header->Trans_id ) ;
    }
    pthread_mutex_unlock( &( rrm->pusu.exclu ) ) ;

#ifdef TRACE
    if ( header->msg_type < NB_MSG_RRM_PUSU )
      fprintf(pusu2rrm_fd,"%lf PUSU->RRM %d %-30s %d %d\n",get_currentclock(),header->inst,Str_msg_pusu_rrm[header->msg_type],header->msg_type,header->Trans_id);
    else
    fprintf(pusu2rrm_fd,"%lf PUSU->RRM %-30s %d %d\n",get_currentclock(),"inconnu",header->msg_type,header->Trans_id);
    fflush(pusu2rrm_fd);
#endif

    switch ( header->msg_type )
    {
        case PUSU_PUBLISH_RESP:
            {
                msg_fct( "[PUSU]>[RRM]:%d:PUSU_PUBLISH_RESP\n",header->inst );
            }
            break ;
        case PUSU_UNPUBLISH_RESP:
            {
                msg_fct( "[PUSU]>[RRM]:%d:PUSU_UNPUBLISH_RESP\n",header->inst );
            }
            break ;
        case PUSU_LINK_INFO_RESP:
            {
                msg_fct( "[PUSU]>[RRM]:%d:PUSU_LINK_INFO_RESP\n",header->inst );
            }
            break ;
        case PUSU_SENSING_INFO_RESP:
            {
                msg_fct( "[PUSU]>[RRM]:%d:PUSU_SENSING_INFO_RESP\n",header->inst );
            }
            break ;
        case PUSU_CH_LOAD_RESP:
            {
                msg_fct( "[PUSU]>[RRM]:%d:PUSU_CH_LOAD_RESP\n",header->inst );
            }
            break ;
        default :
            fprintf(stderr,"PUSU:%d:\n",header->msg_type) ;
            printHex(msg,len_msg,1) ;
    }

}

//mod_lor_10_01_25++
/*!
*******************************************************************************
\brief  traitement des messages entrant via IP

\return Auncune valeur
*/
static void processing_msg_ip(
    rrm_t       *rrm        , ///< Donnee relative a une instance du RRM
    msg_head_t  *header     , ///< Entete du message
    char        *msg        , ///< Message recu
    int         len_msg       ///< Longueur du message
    )
{
#ifdef TRACE
   //mod_lor_10_04_27++
    /*if ( header->msg_type < NB_MSG_RRC_RRM )
    fprintf(ip2rrm_fd,"%lf IP->RRM %d %-30s %d %d\n",get_currentclock(),header->inst,Str_msg_rrc_rrm[header->msg_type], header->msg_type,header->Trans_id);
    else
    fprintf(ip2rrm_fd,"%lf CMM->RRM %-30s %d %d\n",get_currentclock(),"inconnu", header->msg_type,header->Trans_id);
    fflush(ip2rrm_fd);*/
    if ( header->msg_type < NB_MSG_IP )//mod_lor_10_04_27
    fprintf(ip2rrm_fd,"%lf IP->RRM %d %-30s %d %d\n",get_currentclock(),header->inst,Str_msg_ip[header->msg_type], header->msg_type,header->Trans_id);
    else
    fprintf(ip2rrm_fd,"%lf IP->RRM %-30s %d %d\n",get_currentclock(),"inconnu", header->msg_type,header->Trans_id);
    fflush(ip2rrm_fd);
    //mod_lor_10_04_27--
#endif

    switch ( header->msg_type )
    {
        case UPDATE_SENS_RESULTS_3 :
            {
                //fprintf(stderr,"1node entry  @%p \n", rrm->rrc.pSensEntry);//dbg
                if(rrm->sensing.sens_active == 0)//mod_lor_10_05_07
                    break;
                rrm_update_sens_t *p = (rrm_update_sens_t *) msg ;
                msg_fct( "[IP]>[RRM]:%d:UPDATE_SENS_RESULTS_3 from %d trans %d\n",rrm->id, header->inst, header->Trans_id);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM + NB_MSG_RRC_RRM + NB_MSG_CMM_RRM ; //mod_lor_10_04_27
                //mod_lor_10_04_20--
                update_sens_results( rrm->id, p->L2_id, p->NB_info, p->Sens_meas, p->info_time); 
                //mod_lor_10_04_21++
                unsigned int        channels[NB_SENS_MAX]; //!< Vector of channels
                unsigned int        free[NB_SENS_MAX]    ; //!< Vector of values
                int i;
                pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
                CHANNELS_DB_T *channel=rrm->rrc.pChannelsEntry;
                for (i=0;channel!=NULL && i<NB_SENS_MAX;i++){
                    channels[i]=channel->channel.Ch_id;
                    if (!(rrm->ip.waiting_SN_update) && !(channel->is_free) && channel->is_ass) //mod_lor_10_05_18
                        free[i] = 3;
                    else
                        free[i]=channel->is_free;
                    channel=channel->next;
                  //  printf("  ->channel %d is %d\n",channels[i],free[i]);//dbg
                }
                pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
                int r =  send_msg( rrm->graph.s, msg_generic_sens_resp(header->inst,msg_type,i,i,channels,free, header->Trans_id));
                    WARNING(r!=0);
                
                //mod_lor_10_04_21--
                
            }
            break ;
        //mod_lor_10_05_07++    
        case UP_CLUST_SENS_RESULTS :
            {
                if(rrm->sensing.sens_active == 0)//mod_lor_10_05_07
                    break;
                update_coll_sens_t *p = (update_coll_sens_t *) msg ;
                msg_fct( "[IP]>[RRM]:%d:UP_CLUST_SENS_RESULTS from %d \n",rrm->id, header->inst);
                up_coll_sens_results( rrm->id, p->L2_id, p->NB_info, p->Sens_meas, p->info_time); 
                //mod_lor_10_04_21++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM + NB_MSG_RRC_RRM + NB_MSG_CMM_RRM ; //mod_lor_10_04_27
                unsigned int        channels[NB_SENS_MAX]; //!< Vector of channels
                unsigned int        free[NB_SENS_MAX]    ; //!< Vector of values
                int i;
                pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
                CHANNELS_DB_T *channel=rrm->rrc.pChannelsEntry;
                for (i=0;channel!=NULL && i<NB_SENS_MAX;i++){
                    
                    channels[i]=channel->channel.Ch_id;
                    free[i]=channel->is_free;
                    channel=channel->next;
                }
                pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
                int r =  send_msg( rrm->graph.s, msg_generic_sens_resp(header->inst,msg_type,i,i,channels,free, header->Trans_id));
                    WARNING(r!=0);
                
                //mod_lor_10_04_21--
                
            }
            break ;
        //mod_lor_10_05_07--
        case OPEN_FREQ_QUERY_4 :
            {
                open_freq_query_t *p = (open_freq_query_t *) msg ;
                msg_fct( "[IP]>[RRM]:%d:OPEN_FREQ_QUERY_4 from %d\n",rrm->id, header->inst);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM  + NB_MSG_RRC_RRM + NB_MSG_CMM_RRM ; //mod_lor_10_04_27
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                open_freq_query( rrm->id, p->L2_id, p->QoS, header->Trans_id ); 
                
            }
            break ;
        case UPDATE_OPEN_FREQ_7 :
            {
                update_open_freq_t *p = (update_open_freq_t *) msg ; 
                msg_fct( "[IP]>[RRM]:%d:UPDATE_OPEN_FREQ_7 from %d\n",rrm->id, header->inst);
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM + NB_MSG_RRC_RRM + NB_MSG_CMM_RRM ; //mod_lor_10_04_27
                //mod_lor_10_05_18++: occ_channels passed as parameter to update open freq.
                //channels vector incremented by one to have place to save the lenght of the vector
                unsigned int        channels[NB_SENS_MAX]; //!< Vector of channels
                unsigned int        occ_channels[NB_SENS_MAX]    ; //!< Vector of values
                unsigned int        occ_ch_NB; //!< Number of occupied channels
                int i, j=0;
                for (i=0;i<p->NB_chan;i++){
                    if (p->channels[i].is_free){
                        channels[j]=p->channels[i].channel.Ch_id;
                        j++;
                    }
                }            
                occ_ch_NB = update_open_freq( rrm->id, p->L2_id, p->NB_chan, occ_channels, p->channels, header->Trans_id ); //mod_lor_10_05_18
                //mod_lor_10_05_18--
                pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
                int r =  send_msg( rrm->graph.s,msg_generic_sens_resp(header->inst,msg_type,j,occ_ch_NB,channels,occ_channels, header->Trans_id)); //mod_lor_10_05_17
                    WARNING(r!=0);
                //mod_lor_10_04_20--
                
                
            }
            break ;
        case UPDATE_SN_OCC_FREQ_5 :
            {
                update_SN_occ_freq_t *p = (update_SN_occ_freq_t *) msg ;

                msg_fct( "[IP]>[RRM]:%d:UPDATE_SN_OCC_FREQ_5 from %d\n",rrm->id, header->inst);
                msg_fct( "Channels used by SN:\n");//dbg
                for (int i=0; i<p->NB_chan; i++)//dbg
                    msg_fct( "  %d  ", p->occ_channels[i]);//dbg
                msg_fct( "\n");//dbg
                //mod_lor_10_04_20++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM + NB_MSG_RRC_RRM + NB_MSG_CMM_RRM ; //mod_lor_10_04_27
                //int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                
                //mod_lor_10_04_20--
                //mod_lor_10_05_18++
                unsigned int alarm[2];
                if (update_SN_occ_freq( rrm->id, p->L2_id, p->NB_chan, p->occ_channels, header->Trans_id )){
                    alarm[0]=1;
                    int r =  send_msg( rrm->graph.s,msg_generic_sens_resp(header->inst,msg_type,p->NB_chan,2,p->occ_channels,alarm, header->Trans_id)); //mod_lor_10_05_17
                    WARNING(r!=0);  
                    open_freq_query( rrm->id, p->L2_id, 0, header->Trans_id );//mod_lor_10_05_17
                }
                else{
                    alarm[0]=0;
                    int r =  send_msg( rrm->graph.s,msg_generic_sens_resp(header->inst,msg_type,p->NB_chan,2,p->occ_channels,alarm, header->Trans_id)); //mod_lor_10_05_17
                    WARNING(r!=0);
                }//mod_lor_10_05_18--
            }
            break ;
            
        //mod_lor_10_05_05++
        case INIT_COLL_SENS_REQ :
            {
                init_coll_sens_req_t *p = (init_coll_sens_req_t *) msg ;
                msg_fct( "[IP]>[RRM]:%d:INIT_COLL_SENS_REQ from %d\n",rrm->id, header->inst);
                rrm->sensing.sens_active=1;//mod_lor_10_05_07
                //mod_lor_10_05_10++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM  + NB_MSG_RRC_RRM + NB_MSG_CMM_RRM ; //mod_lor_10_04_27
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_05_10--
                memcpy( rrm->L2_id_FC.L2_id, p->L2_id.L2_id, sizeof(L2_ID) );
                cmm_init_sensing(rrm->id,p->Start_fr ,p->Stop_fr,p->Meas_band,p->Meas_tpf,
                        p->Nb_channels, p->Overlap,p->Sampl_freq);
                
            }
            break ;
        //mod_lor_10_05_05--
        //mod_lor_10_05_06++
        case STOP_COLL_SENS :
            {
                //init_coll_sens_req_t *p = (init_coll_sens_req_t *) msg ;
                msg_fct( "[IP]>[RRM]:%d:STOP_COLL_SENS from %d\n",rrm->id, header->inst);
                //mod_lor_10_05_10++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM  + NB_MSG_RRC_RRM + NB_MSG_CMM_RRM ; //mod_lor_10_04_27
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_05_10--
                //memcpy( rrm->L2_id_FC.L2_id, p->L2_id.L2_id, sizeof(L2_ID) );
                rrm->sensing.sens_active=0;//mod_lor_10_05_07
                cmm_stop_sensing(rrm->id);
                
            }
            break ;
        //mod_lor_10_05_06--
        //mod_lor_10_05_12++
        case STOP_COLL_SENS_CONF :
            {
                stop_coll_sens_conf_t *p = (stop_coll_sens_conf_t *) msg ;
                msg_fct( "[IP]>[RRM]:%d:STOP_COLL_SENS_CONF from %d\n",rrm->id, header->inst);
                //mod_lor_10_05_10++
                int msg_type = header->msg_type + NB_MSG_SNS_RRM  + NB_MSG_RRC_RRM + NB_MSG_CMM_RRM ; //mod_lor_10_04_27
                int r =  send_msg( rrm->graph.s, msg_graph_resp(header->inst,msg_type) );
                    WARNING(r!=0);
                //mod_lor_10_05_10--
                //memcpy( rrm->L2_id_FC.L2_id, p->L2_id.L2_id, sizeof(L2_ID) );
                rrc_end_scan_conf( header->inst, p->L2_id, header->Trans_id );
                
            }
            break ;
        //mod_lor_10_05_12--
       
        default :
            fprintf(stderr,"IP:\n") ;
            printHex(msg,len_msg,1) ;
    }
}
//mod_lor_10_01_25--

/*!
*******************************************************************************
\brief  thread de traitement des messages sortants sur les sockets
        (rrc ou cmm).
\return none
*/
static void rrm_scheduler ( )
{
    int ii ;
    int no_msg ;
    fprintf(stderr,"RRM Scheduler: starting ... \n");
    fflush(stderr);
    file_msg_t *pItem ;
    //mod_lor_10_04_22++
    unsigned int priority = 0; ///< to guarantee priority to one rrm (fusion center) during sensing period
    unsigned int pr_ii = 0; ///< id of the rrm with priority
    rrm_t      *rrm ;
    //mod_lor_10_04_22--
    while ( flag_not_exit)
    {
        no_msg = 0  ;
        for ( ii = 0 ; ii<nb_inst ; ii++ )
        {
            //mod_lor_10_04_22++
            if (priority){
                rrm = &rrm_inst[pr_ii] ;
                ii--;
                priority = 0;
            }
            
            else{
                rrm = &rrm_inst[ii] ;
                if (ii > 0 && rrm->sensing.sens_active)
                    priority = 1;
            }
            //mod_lor_10_04_22--
            pItem=NULL;

            pItem = get_msg( &(rrm->file_recv_msg)) ;

            if ( pItem == NULL )
            no_msg++;
            else
            {
                msg_head_t *header = (msg_head_t *) pItem->msg;
                char *msg = NULL ;

                if ( header != NULL )
                {
                    if ( header->size > 0 )
                    {
                        msg = (char *) (header +1) ;
                    }
                     //mod_lor_10_01_25
                    if (pItem->s_type==0){
                        if ( pItem->s->s == rrm->cmm.s->s )
                            processing_msg_cmm( rrm , header , msg , header->size ) ;
                        else if ( pItem->s->s == rrm->rrc.s->s )
                        {
                            processing_msg_rrc( rrm , header , msg , header->size ) ;
                        }
                        else if ( pItem->s->s == rrm->sensing.s->s) {
                          processing_msg_sensing( rrm , header , msg , header->size ) ;
                          //fprintf(stderr,"RRM Scheduler: sensing message ... \n"); //dbg
                        }
                        else
                            processing_msg_pusu( rrm , header , msg , header->size ) ;
                    }
                    else{
                        //fprintf(stderr,"RRM Scheduler: ip message ... \n"); //dbg
                        processing_msg_ip( rrm , header , msg , header->size ) ;
                        
                    }

                RRM_FREE( pItem->msg) ;
            }
            RRM_FREE( pItem ) ;
          }
      }
        if ( no_msg == nb_inst )
      usleep(1000);
      }
    fprintf(stderr,"... stopped RRM Scheduler\n"); fflush(stderr);
}
/*!
*******************************************************************************
\brief This function reads the configuration node file
*/
static void get_config_file(char *filename )
{
    FILE *fd = fopen( filename , "r" );
    char buf_line[128] ;
    int adresse[LENGTH_L2_ID] ;
    int ii = 0 ;


    if ( fd == NULL )
        return ;

    while ( 1 )
    {
        fgets( buf_line, 127, fd ) ;
        if (feof(fd))
            break ;

        if ( buf_line[0] == '#' )
            continue ;
        if ( buf_line[0] == ' ' )
            continue ;
        if ( buf_line[0] == '\t' )
            continue ;
        if ( buf_line[0] == '\n' )
            continue ;

        sscanf( buf_line, "%x %x %x %x %x %x %x %x",
                          &adresse[0],&adresse[1],&adresse[2],&adresse[3],
                          &adresse[4],&adresse[5],&adresse[6],&adresse[7]);

        rrm_inst[ii].id                 = ii ;
        rrm_inst[ii].L2_id.L2_id[0]     = adresse[0] &  0xFF ;
        rrm_inst[ii].L2_id.L2_id[1]     = adresse[1] &  0xFF;
        rrm_inst[ii].L2_id.L2_id[2]     = adresse[2] &  0xFF;
        rrm_inst[ii].L2_id.L2_id[3]     = adresse[3] &  0xFF;
        rrm_inst[ii].L2_id.L2_id[4]     = adresse[4] &  0xFF;
        rrm_inst[ii].L2_id.L2_id[5]     = adresse[5] &  0xFF;
        rrm_inst[ii].L2_id.L2_id[6]     = adresse[6] &  0xFF;
        rrm_inst[ii].L2_id.L2_id[7]     = adresse[7] &  0xFF;

        print_L2_id( &rrm_inst[ii].L2_id ) ;
        fprintf(stderr," (%d) %d \n", ii, buf_line[0] );

        ii++ ;

        adresse[0]=adresse[1]=adresse[2]=adresse[3]=adresse[4]=adresse[5]=adresse[6]=adresse[7]=0;
        buf_line[0] = 0;
    }

    nb_inst = ii ;

    fclose(fd) ;
}
/*!
*******************************************************************************
\brief This function displays the help message (syntax)
*/
static void help()
{
    fprintf(stderr,"syntax: rrm -i <nombre instance> \n" );
    fprintf(stderr,"        rrm -f <config file> \n" );
}

/*!
*******************************************************************************
\brief programme principale du RRM
*/
int main( int argc , char **argv )
{
    fprintf(stderr,"\n\nVersion of RRM with graphic interface\n\n\n\n" );//mod_lor_10_04_20

    int ii;
    int c           =  0;
    int ret         =  0;
    int flag_cfg    =  0 ;
    struct data_thread DataRrc;
    struct data_thread DataCmm;
    struct data_thread DataPusu;
    struct data_thread DataSensing;
    struct data_thread DataGraph;
    sock_rrm_t  s_gr[MAX_RRM]         ;

    sock_rrm_int_t  DataIpS[MAX_RRM]; //mod_lor_10_01_25
    pthread_attr_t attr ;

    /* Vérification des arguments */
    while ((c = getopt(argc,argv,"i:f:h")) != -1)
        switch (c)
        {
            case 'i':
                nb_inst=atoi(optarg);
            break;
            case 'f':
                get_config_file(optarg);
                flag_cfg = 1 ;
            break;
            case 'h':
                help();
                exit(0);
            break;
            default:
                help();
                exit(0);
        }

    if (nb_inst <= 0 )
    {
        fprintf(stderr,"[RRM] Provide a node id\n");
        exit(-1);
    }
    if (nb_inst >= MAX_RRM)
    {
        fprintf(stderr,"[RRM] the instance number (%d) is upper than MAX_RRM (%d)\n", nb_inst, MAX_RRM);
        exit(-1);
    }

#ifdef RRC_KERNEL_MODE
    msg("RRM INIT :open fifos\n");
    while (( Rrm_fifos.rrc_2_rrm_fifo= open ("/dev/rtf14", O_RDONLY )) < 0) 
    {
        printf("[RRM][INIT] open fifo  /dev/rtf14 returned %d\n", Rrm_fifos.rrc_2_rrm_fifo);
        usleep(100);
    }
    printf ("[RRM][INIT] open fifo  /dev/rtf14 returned %d\n", Rrm_fifos.rrc_2_rrm_fifo);
    
    while (( Rrm_fifos.rrm_2_rrc_fifo= open ("/dev/rtf15", O_WRONLY |O_NONBLOCK  | O_NDELAY)) < 0) 
    {//| O_BLOCK
        printf("[RRM][INIT] open fifo  /dev/rtf15 returned %d\n", Rrm_fifos.rrm_2_rrc_fifo);
        usleep(100);
    }
    printf("[RRM][INIT] open fifo  /dev/rtf15 returned %d\n", Rrm_fifos.rrm_2_rrc_fifo);
#endif /* RRC_KERNEL_MODE */

    /* ***** MUTEX ***** */
    // initialise les attributs des threads
    pthread_attr_init( &attr ) ;
    pthread_attr_setschedpolicy( &attr, SCHED_RR ) ;

    DataRrc.name        = "RRC" ;
    DataRrc.sock_path_local = RRM_RRC_SOCK_PATH ;
    DataRrc.sock_path_dest  = RRC_RRM_SOCK_PATH ;
    DataRrc.s.s             = -1 ;

    DataCmm.name        = "CMM" ;
    DataCmm.sock_path_local = RRM_CMM_SOCK_PATH ;
    DataCmm.sock_path_dest  = CMM_RRM_SOCK_PATH ;
    DataCmm.s.s             = -1 ;

    DataPusu.name           = "PUSU" ;
    DataPusu.sock_path_local= RRM_PUSU_SOCK_PATH ;
    DataPusu.sock_path_dest = PUSU_RRM_SOCK_PATH ;
    DataPusu.s.s            = -1 ;
    
    DataSensing.name           = "SENSING" ;
    DataSensing.sock_path_local= RRM_SENSING_SOCK_PATH ;
    DataSensing.sock_path_dest = SENSING_RRM_SOCK_PATH ;
    DataSensing.s.s            = -1 ;    
    
    //mod_lor_10_04_20++
    DataGraph.name           = "Graph" ;
    DataGraph.sock_path_local= "/tmp/rrm_socket" ;
    DataGraph.sock_path_dest = "/tmp/rrm_socket" ;
    DataGraph.s.s            = -1 ;    
    //mod_lor_10_04_20--

#ifdef TRACE
    cmm2rrm_fd  = fopen( "VCD/cmm2rrm.txt" , "w") ;
    PNULL(cmm2rrm_fd) ;

    rrc2rrm_fd  = fopen( "VCD/rrc2rrm.txt", "w") ;
    PNULL(rrc2rrm_fd) ;

    pusu2rrm_fd = fopen( "VCD/pusu2rrm.txt", "w") ;
    PNULL(pusu2rrm_fd) ;
    
    ip2rrm_fd = fopen( "VCD/ip2rrm.txt", "w") ;
    PNULL(ip2rrm_fd) ;

    sensing2rrm_fd = fopen( "VCD/sensing2rrm.txt", "w") ;
    PNULL(sensing2rrm_fd) ;    
    
#endif
    output_2 = fopen( "VCD/output_2.txt", "w") ; //mod_lor_10_04_20
    PNULL(output_2) ; //mod_lor_10_04_20

    for ( ii = 0 ; ii < nb_inst ; ii++ ){
        DataIpS[ii].s               = -1 ;    //mod_lor_10_01_25
        if ( !flag_cfg )
        {
            rrm_inst[ii].id                 = ii ;
            rrm_inst[ii].L2_id.L2_id[0]     = ii;
            rrm_inst[ii].L2_id.L2_id[1]     = 0x00;
            rrm_inst[ii].L2_id.L2_id[2]     = 0x00;
            rrm_inst[ii].L2_id.L2_id[3]     = 0xDE;
            rrm_inst[ii].L2_id.L2_id[4]     = 0xAD;
            rrm_inst[ii].L2_id.L2_id[5]     = 0xBE;
            rrm_inst[ii].L2_id.L2_id[6]     = 0xAF;
            rrm_inst[ii].L2_id.L2_id[7]     = 0x00;
        }

        pthread_mutex_init( &( rrm_inst[ii].rrc.exclu ), NULL ) ;
        pthread_mutex_init( &( rrm_inst[ii].cmm.exclu ), NULL ) ;
        pthread_mutex_init( &( rrm_inst[ii].pusu.exclu ), NULL ) ;
        pthread_mutex_init( &( rrm_inst[ii].sensing.exclu ), NULL ) ;
        pthread_mutex_init( &( rrm_inst[ii].ip.exclu ), NULL ) ; //mod_lor_10_01_25

        init_file_msg( &(rrm_inst[ii].file_recv_msg), 1 ) ;
        init_file_msg( &(rrm_inst[ii].file_send_cmm_msg), 2 ) ;
        init_file_msg( &(rrm_inst[ii].file_send_rrc_msg), 3 ) ;
        init_file_msg( &(rrm_inst[ii].file_send_ip_msg), 4 ) ; //mod_lor_10_01_25
        init_file_msg( &(rrm_inst[ii].file_send_sensing_msg), 5 ) ;


        rrm_inst[ii].state              = ISOLATEDNODE ;
        rrm_inst[ii].role               = NOROLE ;
        rrm_inst[ii].cmm.trans_cnt      =  1024;
        rrm_inst[ii].rrc.trans_cnt      =  2048;
        rrm_inst[ii].pusu.trans_cnt     =  3072;
        rrm_inst[ii].ip.trans_cnt       =  4096; //mod_lor_10_01_25
        rrm_inst[ii].sensing.trans_cnt  =  5120; //mod_lor_10_01_25
        
        
        
        rrm_inst[ii].rrc.s              = &DataRrc.s;
        rrm_inst[ii].cmm.s              = &DataCmm.s;
        rrm_inst[ii].pusu.s             = &DataPusu.s;
        rrm_inst[ii].sensing.s          = &DataSensing.s;
        rrm_inst[ii].ip.s               = &DataIpS[ii]; //mod_lor_10_01_25
        rrm_inst[ii].graph.s            = &s_gr[ii];    //mod_lor_10_04_20
        
        //mod_lor_10_04_20++
        int sock = open_socket( rrm_inst[ii].graph.s  ,"/tmp/rrm_socket", "/tmp/rrm_socket", ii ) ;  
        if ( sock != -1 )
        {
            fprintf(stderr,"  RRM %d graphic interface -> socket =  %d\n",ii , sock );
            fflush(stderr);
        }
        //mod_lor_10_04_20--



        rrm_inst[ii].rrc.transaction    = NULL ;
        rrm_inst[ii].cmm.transaction    = NULL ;
        rrm_inst[ii].pusu.transaction   = NULL ;
        rrm_inst[ii].sensing.transaction= NULL ;
        
        rrm_inst[ii].sensing.sens_active= 0 ;       //mod_lor_10_04_21
        rrm_inst[ii].rrc.pNeighborEntry = NULL ;
        rrm_inst[ii].rrc.pRbEntry       = NULL ;
        rrm_inst[ii].rrc.pSensEntry     = NULL ;
        rrm_inst[ii].rrc.pChannelsEntry = NULL ;
        
    }
    
    
    
    //open_socket( &DataRrc.s,  DataRrc.sock_path_local, DataRrc.sock_path_dest ,0 );

    /* Creation du thread de reception des messages RRC*/
    fprintf(stderr,"Creation du thread RRC : %d\n", nb_inst);
#ifdef RRC_KERNEL_MODE
    ret = pthread_create ( &pthread_recv_rrc_msg_hnd, NULL, thread_recv_msg_fifo , &DataRrc );
    if (ret)
    {
        fprintf (stderr, "%s", strerror (ret));
        exit(-1) ;
    }
#else
    ret = pthread_create ( &pthread_recv_rrc_msg_hnd, NULL, thread_recv_msg , &DataRrc );
    if (ret)
    {
        fprintf (stderr, "%s", strerror (ret));
        exit(-1) ;
    }
#endif
    /* Creation du thread de reception des messages CMM */
    ret = pthread_create (&pthread_recv_cmm_msg_hnd , NULL, thread_recv_msg, &DataCmm );
    if (ret)
    {
        fprintf (stderr, "%s", strerror (ret));
        exit(-1) ;
    }

    /* Creation du thread CMM d'envoi des messages */
    ret = pthread_create (&pthread_send_cmm_msg_hnd, NULL, thread_send_msg_cmm, NULL );
    if (ret)
    {
        fprintf (stderr, "%s", strerror (ret));
        exit(-1) ;
    }

    /* Creation du thread de reception des messages PUSU */
    ret = pthread_create (&pthread_recv_pusu_msg_hnd , NULL, thread_recv_msg, &DataPusu );
    if (ret)
      {
        fprintf (stderr, "%s", strerror (ret));
        exit(-1) ;
      }

    /* Creation du thread de reception des messages SENSING */
    ret = pthread_create (&pthread_recv_sensing_msg_hnd , NULL, thread_recv_msg, &DataSensing );
    if (ret)
      {
        fprintf (stderr, "%s", strerror (ret));
        exit(-1) ;
      }
    /* Creation du thread SENSING d'envoi des messages */
    ret = pthread_create (&pthread_send_sensing_msg_hnd, NULL, thread_send_msg_sensing, NULL );
    if (ret)
    {
        fprintf (stderr, "%s", strerror (ret));
        exit(-1) ;
    }
 
    /* Creation du thread RRC d'envoi des messages */
    ret = pthread_create (&pthread_send_rrc_msg_hnd, NULL, thread_send_msg_rrc, NULL );
    if (ret)
    {
        fprintf (stderr, "%s", strerror (ret));
        exit(-1) ;
    }
    
    //mod_lor_10_01_25++
    
    /* Creation du thread RRC d'envoi des messages */
    ret = pthread_create (&pthread_send_ip_msg_hnd, NULL, thread_send_msg_ip, NULL );
    if (ret)
    {
        fprintf (stderr, "%s", strerror (ret));
        exit(-1) ;
    }
    //mod_lor_10_01_25--*/

    /* Creation du thread TTL */
    ret = pthread_create (&pthread_ttl_hnd , NULL, thread_processing_ttl, NULL);
    if (ret)
    {
        fprintf (stderr, "%s", strerror (ret));
        exit(-1) ;
    }

    /* main loop */
    rrm_scheduler( ) ;

    /* Attente de la fin des threads. */
    pthread_join (pthread_recv_cmm_msg_hnd, NULL);
    pthread_join (pthread_recv_rrc_msg_hnd, NULL);
    pthread_join (pthread_recv_pusu_msg_hnd, NULL);
    pthread_join (pthread_recv_sensing_msg_hnd, NULL);
    pthread_join (pthread_send_cmm_msg_hnd, NULL);
    pthread_join (pthread_send_rrc_msg_hnd, NULL);
    pthread_join (pthread_send_sensing_msg_hnd, NULL);
    pthread_join (pthread_send_ip_msg_hnd, NULL);
    pthread_join (pthread_ttl_hnd, NULL);
    
 
#ifdef TRACE
    fclose(cmm2rrm_fd ) ;
    fclose(rrc2rrm_fd ) ;
    fclose(pusu2rrm_fd ) ;
    fclose(sensing2rrm_fd ) ;

#endif
    fclose(output_2 ) ; //mod_lor_10_04_20

    return 0 ;
}

