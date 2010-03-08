/*!
*******************************************************************************

\file       rrm.c

\brief      RRM (Radio Ressource Manager )

            Cette application a pour objet
                - de gérer la ressource radio du cluster
                - de commander le RRC pour l'ouverture de RB
                - de recevoir des commandes du CMM
                - de gérer le voisinage

\author     BURLOT Pascal

\date       10/07/08


\par     Historique:
        P.BURLOT 2009-01-20 
            + separation de la file de message CMM/RRM a envoyer en 2 files 
              distinctes ( file_send_cmm_msg, file_send_rrc_msg)
            + l'envoi de message via la fifo:
                - envoi du header
                - puis des donnees s'il y en a
            + reception des donnees de la fifo:
                - copie du message dans la file d'attente des messages
                - traitement du cas du message n'ayant pas de data (ex: response )
        L.IACOBELLI 2009-10-19
            + sensing database
            + channels database
            + new cases 

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
                 
                 pthread_recv_int_msg_hnd ,
                 pthread_send_ip_msg_hnd ,

                 pthread_ttl_hnd ;
static unsigned int cnt_timer = 0;

#ifdef TRACE
static FILE *cmm2rrm_fd  = NULL ;
static FILE *rrc2rrm_fd  = NULL ;
static FILE *pusu2rrm_fd = NULL ;
static FILE *ip2rrm_fd = NULL ;
#endif
/*
** ----------------------------------------------------------------------------
** DECLARATION DES FONCTIONS
** ----------------------------------------------------------------------------
*/
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

//mod_lor_10_01_25--*/


/*!
*******************************************************************************
\brief  thread de traitement des messages entrant sur une interface (rrc ou cmm).

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
    struct data_thread_int *data = (struct data_thread_int *) p_data; // mod_lor AAA: memory problem?
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
            }
            break ;
        case CMM_CX_MODIFY_REQ:
            {
                cmm_cx_modify_req_t *p = (cmm_cx_modify_req_t *) msg ;
                msg_fct( "[CMM]>[RRM]:%d:CMM_CX_MODIFY_REQ\n",header->inst);
                cmm_cx_modify_req(header->inst,p->Rb_id,p->QoS_class,header->Trans_id )  ;
            }
            break ;
        case CMM_CX_RELEASE_REQ :
            {
                cmm_cx_release_req_t *p = (cmm_cx_release_req_t *) msg ;
                msg_fct( "[CMM]>[RRM]:%d:CMM_CX_RELEASE_REQ\n",header->inst);
                cmm_cx_release_req(header->inst,p->Rb_id,header->Trans_id )  ;
            }
            break ;
        case CMM_CX_RELEASE_ALL_REQ :
            {
                //cmm_cx_release_all_req_t *p = (cmm_cx_release_all_req_t *) msg ;
                msg_fct( "[CMM]>[RRM]:%d:CMM_CX_RELEASE_ALL_REQ\n",header->inst);
            }
            break ;
        case CMM_ATTACH_CNF : ///< The thread that allows 
            {
                cmm_attach_cnf_t *p = (cmm_attach_cnf_t *) msg ;
                msg_fct( "[CMM]>[RRM]:%d:CMM_ATTACH_CNF\n",header->inst);
              
               //mod_lor_10_01_25++
                if (rrm->ip.s->s == -1){ 
                    unsigned char tmp [4]={0x0A,0x00,0x01,0x01};
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
                //mod_lor_10_01_25--*/
                
                cmm_attach_cnf( header->inst, p->L2_id, p->L3_info_t, p->L3_info, header->Trans_id ) ;
            }
            break ;
        case CMM_INIT_MR_REQ :
            {
                msg_fct( "[CMM]>[RRM]:%d:CMM_INIT_MR_REQ ????\n",header->inst);
            }
            break ;
        case CMM_INIT_CH_REQ :
            {
                cmm_init_ch_req_t *p = (cmm_init_ch_req_t *) msg ;
                //mod_lor_10_03_01++
            
                struct data_thread_int DataIp;
                
                DataIp.name = "IP"             ; ///< Nom du thread
                DataIp.sock_path_local=p->L3_info;///< local IP address for internet socket
                DataIp.local_port = 7000          ; ///< local IP port for internet socket
                //mod_lor_10_03_02++: setting for topology with FC and BTS on instances 0 and 1
                if (rrm->role == FUSIONCENTER){
                    unsigned char tmp [4]={0x0A,0x00,0x02,0x02};
                    DataIp.sock_path_dest = tmp ; ///< dest IP address for internet socket
                }else if (rrm->role == BTS){
                    unsigned char tmp [4]={0x0A,0x00,0x01,0x01};
                    DataIp.sock_path_dest = tmp  ; ///< dest IP address for internet socket
                }else 
                    fprintf (stderr, "wrong node role %d \n", rrm->role);
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
                cmm_init_sensing(header->inst,p->interv);
                //fprintf(stderr," End of CMM_INIT_CH_REQ\n");*/
            }
            break ;
        case CMM_STOP_SENSING :
            {
                msg_fct( "[CMM]>[RRM]:%d:CMM_STOP_SENSING\n",rrm->id);
                //print_sens_db(rrm->rrc.pSensEntry);//dbg
                cmm_stop_sensing(header->inst);
            }
            break ;
        case CMM_ASK_FREQ :
            {
                msg_fct( "[CMM]>[RRM]:%d:CMM_ASK_FREQ\n",header->inst);
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
                rrc_rb_establish_resp(header->inst,header->Trans_id) ;
            }
            break ;
        case RRC_RB_ESTABLISH_CFM:
            {
                rrc_rb_establish_cfm_t *p = (rrc_rb_establish_cfm_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_RB_ESTABLISH_CFM (%d)  %d \n",header->inst,p->Rb_id, header->Trans_id);
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
                rrc_rb_modify_cfm(header->inst,p->Rb_id,header->Trans_id) ;
            }
            break ;

        case RRC_RB_RELEASE_RESP:
            {
                msg_fct( "[RRC]>[RRM]:%d:RRC_RB_RELEASE_RESP %d \n",header->inst, header->Trans_id);
                rrc_rb_release_resp(header->inst,header->Trans_id) ;
            }
            break ;
        case RRC_MR_ATTACH_IND :
            {
                rrc_MR_attach_ind_t *p = (rrc_MR_attach_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_MR_ATTACH_IND (Node %02d) %d \n",header->inst, p->L2_id.L2_id[0], header->Trans_id);
                rrc_MR_attach_ind(header->inst,p->L2_id) ;
            }
            break ;
        case RRC_SENSING_MEAS_RESP:
            {
                msg_fct( "[RRC]>[RRM]:%d:RRC_SENSING_MEAS_RESP %d \n",header->inst, header->Trans_id);
                rrc_sensing_meas_resp(header->inst,header->Trans_id) ;
            }
            break ;
        case RRC_CX_ESTABLISH_IND:
            {
                rrc_cx_establish_ind_t *p = (rrc_cx_establish_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_CX_ESTABLISH_IND (Node %02d) %d \n",header->inst, p->L2_id.L2_id[0], header->Trans_id);

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
                rrc_phy_synch_to_MR_ind(header->inst,p->L2_id) ;
                //msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_MR_IND Done\n",header->inst);
            }
            break ;
        case RRC_PHY_SYNCH_TO_CH_IND :
            {
                rrc_phy_synch_to_CH_ind_t *p = (rrc_phy_synch_to_CH_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_CH_IND.... %d (Node %02d) %d \n",header->inst, p->Ch_index, p->L2_id.L2_id[0], header->Trans_id);
                rrc_phy_synch_to_CH_ind(header->inst,p->Ch_index,p->L2_id ) ;
                //msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_CH_IND Done\n",header->inst);

            }
            break ;
        case RRC_SENSING_MEAS_IND :
            {
                rrc_sensing_meas_ind_t *p  = (rrc_sensing_meas_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_SENSING_MEAS_IND (Node %02d) %d \n",header->inst, p->L2_id.L2_id[0], header->Trans_id);
                rrc_sensing_meas_ind( header->inst,p->L2_id, p->NB_meas, p->Sensing_meas, header->Trans_id );
            }
            break ;
        case RRC_RB_MEAS_IND :
            {
                rrc_rb_meas_ind_t *p  = (rrc_rb_meas_ind_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_RB_MEAS_IND (Noede %02d) %d \n",header->inst, p->L2_id.L2_id[0], header->Trans_id);
                rrc_rb_meas_ind( header->inst, p->Rb_id, p->L2_id, p->Meas_mode, p->Mac_rlc_meas, header->Trans_id );
            }
            break ;

        case RRC_UPDATE_SENS :
            {
                rrc_update_sens_t *p  = (rrc_update_sens_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_UPDATE_SENS %d\n ",header->inst, header->Trans_id);
                /*for ( int i=0;i<8;i++)
                    msg_fct("%02X", p->L2_id.L2_id[i]);
                msg_fct( ")\n");*/
                //update_sens_results( 0, p->L2_id, p->NB_info, p->Sens_meas, p->info_time); //dbg -> test_emul
                rrc_update_sens( header->inst, p->L2_id, p->NB_info, p->Sens_meas, p->info_time );  //fix info_time & understand trans_id
                //fprintf(stderr,"end case RRC_UPDATE_SENS\n");//dbg
            }
            break ;
        case RRC_INIT_SCAN_REQ :
            {
                rrc_init_scan_req_t *p  = (rrc_init_scan_req_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_INIT_SCAN_REQ  %d \n",header->inst, header->Trans_id);
                /*for ( int i=0;i<8;i++)
                    msg_fct("%02X", p->L2_id.L2_id[i]);
                msg_fct( ")\n");*/
                
                rrc_init_scan_req( header->inst, p->L2_id, p->interv, header->Trans_id ); 
                
            }
            break ;
        case RRC_END_SCAN_CONF :
            {
                rrc_end_scan_conf_t *p  = (rrc_end_scan_conf_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_END_SCAN_CONF  %d (Node ",header->inst, header->Trans_id);
                for ( int i=0;i<8;i++)
                    msg_fct("%02X", p->L2_id.L2_id[i]);
                msg_fct( ")\n");
                rrc_end_scan_conf( header->inst, p->L2_id, header->Trans_id );  
            }
            break ;
        case RRC_END_SCAN_REQ :
            {
                rrc_end_scan_req_t *p  = (rrc_end_scan_req_t *) msg ;
                msg_fct( "[RRC]>[RRM]:%d:RRC_END_SCAN_REQ %d \n",header->inst, header->Trans_id);
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
                rrc_init_mon_req( header->inst, p->L2_id, p->ch_to_scan, p->NB_chan, p->interval, header->Trans_id );  
            }
            break ;

        default :
            fprintf(stderr,"RRC:\n") ;
            printHex(msg,len_msg,1) ;
    }

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
    if ( header->msg_type < NB_MSG_RRC_RRM )
    fprintf(ip2rrm_fd,"%lf IP->RRM %d %-30s %d %d\n",get_currentclock(),header->inst,Str_msg_rrc_rrm[header->msg_type], header->msg_type,header->Trans_id);
    else
    fprintf(ip2rrm_fd,"%lf CMM->RRM %-30s %d %d\n",get_currentclock(),"inconnu", header->msg_type,header->Trans_id);
    fflush(ip2rrm_fd);
#endif

    switch ( header->msg_type )
    {
        case UPDATE_SENS_RESULTS_3 :
            {
                //fprintf(stderr,"1node entry  @%p \n", rrm->rrc.pSensEntry);//dbg
                rrm_update_sens_t *p = (rrm_update_sens_t *) msg ;
                msg_fct( "[IP]>[RRM]:%d:UPDATE_SENS_RESULTS_3 from %d \n",rrm->id, header->inst);
                update_sens_results( rrm->id, p->L2_id, p->NB_info, p->Sens_meas, p->info_time); 
                //fprintf(stderr,"2node entry  @%p \n", rrm->rrc.pSensEntry);//dbg
                
            }
            break ;
        case OPEN_FREQ_QUERY_4 :
            {
                open_freq_query_t *p = (open_freq_query_t *) msg ;
                msg_fct( "[IP]>[RRM]:%d:OPEN_FREQ_QUERY_4 from %d\n",rrm->id, header->inst);
                open_freq_query( rrm->id, p->L2_id, p->QoS, header->Trans_id ); 
                
            }
            break ;
        case UPDATE_OPEN_FREQ_7 :
            {
                update_open_freq_t *p = (update_open_freq_t *) msg ;
                msg_fct( "[IP]>[RRM]:%d:UPDATE_OPEN_FREQ_7 from %d\n",rrm->id, header->inst);
                update_open_freq( rrm->id, p->L2_id, p->NB_chan, p->fr_channels, header->Trans_id ); 
                
            }
            break ;
        case UPDATE_SN_OCC_FREQ_5 :
            {
                update_SN_occ_freq_t *p = (update_SN_occ_freq_t *) msg ;
                msg_fct( "[IP]>[RRM]:%d:UPDATE_SN_OCC_FREQ_5 from %d\n",rrm->id, header->inst);
                update_SN_occ_freq( rrm->id, p->L2_id, p->NB_chan, p->occ_channels, header->Trans_id );  
                
            }
            break ;
       
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
    while ( flag_not_exit)
    {
        no_msg = 0  ;
        for ( ii = 0 ; ii<nb_inst ; ii++ )
        {
            rrm_t      *rrm = &rrm_inst[ii] ;
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

    int ii;
    int c           =  0;
    int ret         =  0;
    int flag_cfg    =  0 ;
    struct data_thread DataRrc;
    struct data_thread DataCmm;
    struct data_thread DataPusu;
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
    
    

#ifdef TRACE
    cmm2rrm_fd  = fopen( "VCD/cmm2rrm.txt" , "w") ;
    PNULL(cmm2rrm_fd) ;

    rrc2rrm_fd  = fopen( "VCD/rrc2rrm.txt", "w") ;
    PNULL(rrc2rrm_fd) ;

    pusu2rrm_fd = fopen( "VCD/pusu2rrm.txt", "w") ;
    PNULL(pusu2rrm_fd) ;
    
    ip2rrm_fd = fopen( "VCD/ip2rrm.txt", "w") ;
    PNULL(ip2rrm_fd) ;
#endif

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
        pthread_mutex_init( &( rrm_inst[ii].ip.exclu ), NULL ) ; //mod_lor_10_01_25

        init_file_msg( &(rrm_inst[ii].file_recv_msg), 1 ) ;
        init_file_msg( &(rrm_inst[ii].file_send_cmm_msg), 2 ) ;
        init_file_msg( &(rrm_inst[ii].file_send_rrc_msg), 3 ) ;
        init_file_msg( &(rrm_inst[ii].file_send_ip_msg), 4 ) ; //mod_lor_10_01_25

        rrm_inst[ii].state              = ISOLATEDNODE ;
        rrm_inst[ii].role               = NOROLE ;
        rrm_inst[ii].cmm.trans_cnt      =  1024;
        rrm_inst[ii].rrc.trans_cnt      =  2048;
        rrm_inst[ii].pusu.trans_cnt     =  3072;
        rrm_inst[ii].ip.trans_cnt       =  4096; //mod_lor_10_01_25

        rrm_inst[ii].rrc.s              = &DataRrc.s;
        rrm_inst[ii].cmm.s              = &DataCmm.s;
        rrm_inst[ii].pusu.s             = &DataPusu.s;
        rrm_inst[ii].ip.s               = &DataIpS[ii]; //mod_lor_10_01_25

        rrm_inst[ii].rrc.transaction    = NULL ;
        rrm_inst[ii].cmm.transaction    = NULL ;
        rrm_inst[ii].pusu.transaction   = NULL ;
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
    pthread_join (pthread_send_cmm_msg_hnd, NULL);
    pthread_join (pthread_send_rrc_msg_hnd, NULL);
    pthread_join (pthread_send_ip_msg_hnd, NULL);
    pthread_join (pthread_ttl_hnd, NULL);
    
 
#ifdef TRACE
    fclose(cmm2rrm_fd ) ;
    fclose(rrc2rrm_fd ) ;
    fclose(pusu2rrm_fd ) ;
#endif

    return 0 ;
}


                
               
                //fprintf(stderr,"L3_local in CMM ");//dbg
                //print_L3_id( IPv4_ADDR,  rrm->L3_info_corr );
                //fprintf(stderr,"\n");//dbg
