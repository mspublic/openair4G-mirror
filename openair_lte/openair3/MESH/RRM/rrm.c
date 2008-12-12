/*!
*******************************************************************************

\file    	rrm.c

\brief   	RRM (Radio Ressource Manager )

			Cette application a pour objet 
			    - de gérer la ressource radio du cluster
			    - de commander le RRC pour l'ouverture de RB
			    - de recevoir des commandes du CMM
			    - de gérer le voisinage

\author  	BURLOT Pascal

\date    	10/07/08

   
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
#include "neighbor_db.h"
#include "rb_db.h"
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
	char *name      		; ///< Nom du thread
	char *sock_path_local 	; ///< fichier du "rrm->..." pour le socket Unix
	char *sock_path_dest  	; ///< fichier du "...->rrm " pour le socket Unix
	sock_rrm_t  s           ; ///< Descripteur du socket 
}  ;

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

static int flag_not_exit = 1 ;
static pthread_t pthread_recv_rrc_msg_hnd, 
                 pthread_recv_cmm_msg_hnd , 
                 pthread_send_msg_hnd , 
                 pthread_ttl_hnd ;
static unsigned int cnt_timer = 0;

#ifdef TRACE    
static FILE *cmm2rrm_fd = NULL ;
static FILE *rrc2rrm_fd = NULL ; 
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
		
			if ( *(rrm->pusu.s) != -1 )
			{		
				pthread_mutex_lock(   &( rrm->cmm.exclu )  ) ;
				dec_all_ttl_transact( rrm->cmm.transaction ) ;
				// Trop simpliste et pas fonctionnel , il faut faire une gestion des erreurs de transaction
				del_all_obseleted_transact( &(rrm->cmm.transaction));
				pthread_mutex_unlock( &( rrm->cmm.exclu )  ) ;
				
				pthread_mutex_lock(   &( rrm->rrc.exclu )  ) ;
				dec_all_ttl_transact( rrm->rrc.transaction ) ;
				// Trop simpliste et pas fonctionnel , il faut faire une gestion des erreurs de transaction
				del_all_obseleted_transact( &(rrm->rrc.transaction));
				pthread_mutex_unlock( &( rrm->rrc.exclu )  ) ;
			}
		}

		cnt_timer++;
		usleep( 200*1000 ) ;
	}
	fprintf(stderr,"... stopped TTL\n"); fflush(stdout);
	return NULL;
	
}

/*!
*******************************************************************************
\brief  thread de traitement des messages sortants sur les sockets (rrc ou cmm). 

\return NULL
*/
static void * thread_send_msg (
	void * p_data /**< parametre du pthread */
	)
{
	int ii ;
	int no_msg ;
	fprintf(stderr,"Thread Send Message: starting ... \n"); 
	fflush(stderr);

	while ( flag_not_exit)
	{
		no_msg = 0  ;
		for ( ii = 0 ; ii<nb_inst ; ii++ )
		{
			rrm_t      *rrm = &rrm_inst[ii] ; 
			file_msg_t *pItem ;
		
			pItem = get_msg( &(rrm->file_send_msg) ) ;
		
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
			usleep(100000);
	
	}
	fprintf(stderr,"... stopped Thread Send Message\n"); fflush(stderr);
	return NULL;
}

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
				
				put_msg( &(rrm->file_recv_msg), &data->s, msg) ;
			}
		}
		close_socket(&data->s) ;
	}

	fprintf(stderr,"... stopped %s interfaces\n",data->name);
	return NULL;
}

/*!
*******************************************************************************
\brief  traitement des messages entrant sur l'interface CMM
          
\return Auncune valeur
*/
static void processing_msg_cmm( 
	rrm_t 		*rrm		, ///< Donnee relative a une instance du RRM
	msg_head_t 	*header		, ///< Entete du message
	char 		*msg		, ///< Message recu	
	int 		len_msg 	  ///< Longueur du message
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
				if ( cmm_cx_setup_req(header->inst,p->Src,p->Dst,p->QoS_class,p->Trans_id ) )
				{ /* RB_ID = 0xFFFF => RB error */
					put_msg( &(rrm->file_send_msg), 
								rrm->cmm.s, msg_rrm_cx_setup_cnf(header->inst,0xFFFF , p->Trans_id )) ;
				} 
			}
			break ;
		case CMM_CX_MODIFY_REQ:
			{
				cmm_cx_modify_req_t *p = (cmm_cx_modify_req_t *) msg ;
				msg_fct( "[CMM]>[RRM]:%d:CMM_CX_MODIFY_REQ\n",header->inst);
				cmm_cx_modify_req(header->inst,p->Rb_id,p->QoS_class,p->Trans_id )  ;
			}
			break ;
		case CMM_CX_RELEASE_REQ :
			{
				cmm_cx_release_req_t *p = (cmm_cx_release_req_t *) msg ;
				msg_fct( "[CMM]>[RRM]:%d:CMM_CX_RELEASE_REQ\n",header->inst);
				cmm_cx_release_req(header->inst,p->Rb_id,p->Trans_id )  ;
			}
			break ;
		case CMM_CX_RELEASE_ALL_REQ :
			{
				//cmm_cx_release_all_req_t *p = (cmm_cx_release_all_req_t *) msg ;
				msg_fct( "[CMM]>[RRM]:%d:CMM_CX_RELEASE_ALL_REQ\n",header->inst);
			}
			break ;
		case CMM_ATTACH_CNF :
			{
				cmm_attach_cnf_t *p = (cmm_attach_cnf_t *) msg ;
				msg_fct( "[CMM]>[RRM]:%d:CMM_ATTACH_CNF\n",header->inst);
				
				cmm_attach_cnf( header->inst, p->L2_id, p->L3_info_t, p->L3_info, p->Trans_id ) ;
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
				cmm_init_ch_req(header->inst,p->L3_info_t,&(p->L3_info[0]));
				msg_fct( "[CMM]>[RRM]:%d:CMM_INIT_CH_REQ\n",header->inst);
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
	rrm_t *rrm			, ///< Donnee relative a une instance du RRM
	msg_head_t *header	, ///< Entete du message
	char *msg			, ///< Message reçu	
	int len_msg 		  ///< Longueur du message
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
				rrc_generic_resp_t *p = (rrc_generic_resp_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_RB_ESTABLISH_RESP\n",header->inst );
				rrc_rb_establish_resp(header->inst,p->Trans_id) ;
			}
			break ;
		case RRC_RB_ESTABLISH_CFM:
			{
				rrc_rb_establish_cfm_t *p = (rrc_rb_establish_cfm_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_RB_ESTABLISH_CFM\n",header->inst);
				rrc_rb_establish_cfm(header->inst,p->Rb_id,p->RB_type,p->Trans_id) ;
			}
			break ;
			
		case RRC_RB_MODIFY_RESP:
			{
				rrc_generic_resp_t *p = (rrc_generic_resp_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_RB_MODIFY_RESP\n",header->inst);
				rrc_rb_modify_resp(header->inst,p->Trans_id) ;
			}
			break ;
		case RRC_RB_MODIFY_CFM:
			{
				rrc_rb_modify_cfm_t *p = (rrc_rb_modify_cfm_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_RB_MODIFY_CFM\n",header->inst);
				rrc_rb_modify_cfm(header->inst,p->Rb_id,p->Trans_id) ;
			}
			break ;
			
		case RRC_RB_RELEASE_RESP:
			{
				rrc_generic_resp_t *p = (rrc_generic_resp_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_RB_RELEASE_RESP\n",header->inst);
				rrc_rb_release_resp(header->inst,p->Trans_id) ;
			}
			break ;
		case RRC_MR_ATTACH_IND :
			{
				rrc_MR_attach_ind_t *p = (rrc_MR_attach_ind_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_MR_ATTACH_IND\n",header->inst);
				rrc_MR_attach_ind(header->inst,p->L2_id) ;
			}
			break ;
		case RRC_SENSING_MEAS_RESP:
			{
				rrc_generic_resp_t *p = (rrc_generic_resp_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_SENSING_MEAS_RESP\n",header->inst);
				rrc_sensing_meas_resp(header->inst,p->Trans_id) ;
			}
			break ;			
		case RRC_CX_ESTABLISH_IND:
			{
				rrc_cx_establish_ind_t *p = (rrc_cx_establish_ind_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_CX_ESTABLISH_IND\n",header->inst);
				rrc_cx_establish_ind(header->inst,p->L2_id,p->Trans_id,
									p->L3_info,p->L3_info_t,
									p->DTCH_B_id,p->DTCH_id) ;	
			}
			break ;			
		case RRC_PHY_SYNCH_TO_MR_IND :
			{
				rrc_phy_synch_to_MR_ind_t *p = (rrc_phy_synch_to_MR_ind_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_MR_IND\n",header->inst);
				rrc_phy_synch_to_MR_ind(header->inst,p->L2_id) ;				
			}
			break ;
		case RRC_PHY_SYNCH_TO_CH_IND :
			{
				rrc_phy_synch_to_CH_ind_t *p = (rrc_phy_synch_to_CH_ind_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_CH_IND\n",header->inst);
				rrc_phy_synch_to_CH_ind(header->inst,p->Ch_index,p->L2_id ) ;
								
			}
			break ;
		case RRC_SENSING_MEAS_IND :
			{
				rrc_sensing_meas_ind_t *p  = (rrc_sensing_meas_ind_t *) msg ;
				msg_fct( "[RRC]>[RRM]:%d:RRC_SENSING_MEAS_IND\n",header->inst);
				rrc_sensing_meas_ind( header->inst,p->L2_id, p->NB_meas, p->Sensing_meas, p->Trans_id );			
			}
			break ;
		default :
			fprintf(stderr,"RRC:\n") ;
			printHex(msg,len_msg,1) ;
	}
	
}

/*!
*******************************************************************************
\brief  thread de traitement des messages sortants sur les sockets
        (rrc ou cmm). 
\return none
*/
static void rrm_scheduler (	)
{
	int ii ;
	int no_msg ;
	fprintf(stderr,"RRM Scheduler: starting ... \n"); 
	fflush(stderr);

	while ( flag_not_exit)
	{
		for ( ii = 0 ; ii<nb_inst ; ii++ )
		{
			rrm_t      *rrm = &rrm_inst[ii] ; 
			file_msg_t *pItem ;
		
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
					
					if ( pItem->s->s == rrm->cmm.s->s )
						processing_msg_cmm( rrm , header , msg , header->size ) ;
					else
						processing_msg_rrc( rrm , header , msg , header->size ) ;
						
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
		
 		rrm_inst[ii].id              	= ii ; 
 		rrm_inst[ii].L2_id.L2_id[0]	 	= adresse[0] &  0xFF ;
 		rrm_inst[ii].L2_id.L2_id[1]	 	= adresse[1] &  0xFF;
 		rrm_inst[ii].L2_id.L2_id[2]	 	= adresse[2] &  0xFF;
 		rrm_inst[ii].L2_id.L2_id[3]	 	= adresse[3] &  0xFF;
 		rrm_inst[ii].L2_id.L2_id[4]	 	= adresse[4] &  0xFF;
 		rrm_inst[ii].L2_id.L2_id[5]	 	= adresse[5] &  0xFF;
 		rrm_inst[ii].L2_id.L2_id[6]	 	= adresse[6] &  0xFF;
 		rrm_inst[ii].L2_id.L2_id[7]	 	= adresse[7] &  0xFF;
		
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
	int c 			=  0;
	int ret 		=  0;
	int flag_cfg    =  0 ;
	struct data_thread DataRrc;
 	struct data_thread DataCmm;
 	int             sockPusu;
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
	
	/* ***** MUTEX ***** */ 
	// initialise les attributs des threads
	pthread_attr_init( &attr ) ;
	pthread_attr_setschedpolicy( &attr, SCHED_RR ) ;

	DataRrc.name     	= "RRC" ;
	DataRrc.sock_path_local	= RRM_RRC_SOCK_PATH ;
	DataRrc.sock_path_dest	= RRC_RRM_SOCK_PATH ;
	DataRrc.s.s	 			= -1  ; 
	
	DataCmm.name     	= "CMM" ;
	DataCmm.sock_path_local	= RRM_CMM_SOCK_PATH ;
	DataCmm.sock_path_dest	= CMM_RRM_SOCK_PATH ;
	DataCmm.s.s  			= -1 ;

	sockPusu  			    = -1 ;

#ifdef TRACE    
	cmm_fd = fopen( "vcd/cmm2rrm.txt" , "w") ;
	PNULL(cmm_fd) ;

	rrc_fd = fopen( "vcd/rrc2rrm.txt", "w") ;
	PNULL(rrc_fd) ;
#endif
	for ( ii = 0 ; ii < nb_inst ; ii++ )
	{
 		if ( !flag_cfg ) 
 		{
 			rrm_inst[ii].id              	= ii ; 
			rrm_inst[ii].L2_id.L2_id[0]	 	= ii;
			rrm_inst[ii].L2_id.L2_id[1]	 	= 0x00;
			rrm_inst[ii].L2_id.L2_id[2]	 	= 0x00;
			rrm_inst[ii].L2_id.L2_id[3]	 	= 0xDE;
			rrm_inst[ii].L2_id.L2_id[4]	 	= 0xAD;
			rrm_inst[ii].L2_id.L2_id[5]	 	= 0xBE;
			rrm_inst[ii].L2_id.L2_id[6]	 	= 0xAF;
			rrm_inst[ii].L2_id.L2_id[7]	 	= 0x00;
		}

		pthread_mutex_init( &( rrm_inst[ii].rrc.exclu ), NULL ) ;
 		pthread_mutex_init( &( rrm_inst[ii].cmm.exclu ), NULL ) ;
 		
  		init_file_msg( &(rrm_inst[ii].file_recv_msg), 1 ) ;
		init_file_msg( &(rrm_inst[ii].file_send_msg), 2 ) ;
	
 		rrm_inst[ii].state           	= ISOLATEDNODE ; 
 		rrm_inst[ii].cmm.trans_cnt	 	=  1;
 		rrm_inst[ii].rrc.trans_cnt	 	=  1;
 		rrm_inst[ii].rrc.s		 		= &DataRrc.s;
 		rrm_inst[ii].cmm.s		 		= &DataCmm.s;
 		rrm_inst[ii].pusu.s		 		= &sockPusu;
		rrm_inst[ii].rrc.transaction 	= NULL ;
 		rrm_inst[ii].cmm.transaction 	= NULL ;
 		rrm_inst[ii].rrc.pNeighborEntry	= NULL ;
	}

	
	/* Creation du thread de reception des messages RRC*/
	fprintf(stderr,"Creation du thread RRC : %d\n", nb_inst);
	ret = pthread_create ( &pthread_recv_rrc_msg_hnd, NULL, thread_recv_msg , &DataRrc );
	if (ret)
	{
		fprintf (stderr, "%s", strerror (ret));
		exit(-1) ;
	}
	
	/* Creation du thread de reception des messages CMM */
	ret = pthread_create (&pthread_recv_cmm_msg_hnd , NULL, thread_recv_msg, &DataCmm );
	if (ret)
	{
		fprintf (stderr, "%s", strerror (ret));
		exit(-1) ;
	}
		
	/* Creation du thread CMM d'envoi des messages */
	ret = pthread_create (&pthread_send_msg_hnd, NULL, thread_send_msg, NULL );
	if (ret)
	{
		fprintf (stderr, "%s", strerror (ret));
		exit(-1) ;
	}

	/* Creation du thread TTL */
	ret = pthread_create (&pthread_ttl_hnd , NULL, thread_processing_ttl, NULL);
	if (ret)
	{
		fprintf (stderr, "%s", strerror (ret));
		exit(-1) ;
	}

	
	{
		int timeout_pusu = 10000 ;
		while ( timeout_pusu!=0 )
		{
			sockPusu	 = connect_to_pusu_sock(RRM_PUSU_SOCK_PATH ,0 ) ;
			if ( sockPusu != -1) 
				break ;
			usleep( 1000000);
			timeout_pusu--;
		}
		
		if ( timeout_pusu == 0 )
		{
			fprintf(stderr,"Abort: Timeout of PuSu socket\n");
			exit(1) ;
		}
    }
    /* main loop */
	rrm_scheduler( ) ;
	
	/* Attente de la fin des threads. */
	pthread_join (pthread_recv_cmm_msg_hnd, NULL);
	pthread_join (pthread_recv_rrc_msg_hnd, NULL);
	pthread_join (pthread_send_msg_hnd, NULL);
	pthread_join (pthread_ttl_hnd, NULL);
	
#ifdef TRACE   
	  fclose(cmm_fd ) ;
	  fclose(rrc_fd ) ;
#endif
	
	return 0 ;	
}


