/*!
*******************************************************************************

\file       freq_ass_op.c

\brief      Fonctions permettant la gestion des frequences

\author     IACOBELLI Lorenzo

\date       21/10/09

   
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
#include "pusu_msg.h"
#include "rb_db.h"
#include "neighbor_db.h"
#include "sens_db.h"
#include "channels_db.h"
#include "rrm_util.h"
#include "transact.h"
#include "rrm_constant.h"
#include "rrm.h"
#include "freq_ass_op.h"




//! Met un message dans la file des messages a envoyer
#define PUT_CMM_MSG(m)  put_msg(  &(rrm->file_send_cmm_msg), 0, rrm->cmm.s,m ) //mod_lor_10_01_25
#define PUT_PUSU_MSG(m) put_msg(  &(rrm->file_send_cmm_msg), 0, rrm->pusu.s,m) //mod_lor_10_01_25
#define PUT_RRC_MSG(m)  put_msg(  &(rrm->file_send_rrc_msg), 0, rrm->rrc.s,m ) //mod_lor_10_01_25
#define PUT_IP_MSG(m)   put_msg(  &(rrm->file_send_ip_msg) , 1, rrm->ip.s,m  ) //mod_lor_10_01_25

/*!
*******************************************************************************
\brief  Comparaison de deux ID de niveau 2

\return si 0 alors les IDs sont identiques
*/
static int L2_ID_cmp(
    L2_ID *L2_id1, ///< ID de niveau 2
    L2_ID *L2_id2  ///< ID de niveau 2
    )
{
    return memcmp( L2_id1, L2_id2, sizeof(L2_ID) ) ;
}

/*!
*******************************************************************************
 \brief CMM frequency query request.  This message has also the scope to 
 * initialize the RRM in BTS state. 
*/
void cmm_ask_freq( 
    Instance_t inst            //!< identification de l'instance
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    //rrm->role = BTS;
           
    pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
    rrm->ip.trans_cnt++ ;
    PUT_IP_MSG(msg_open_freq_query_4( inst, rrm->L2_id_FC, 0, rrm->ip.trans_cnt));
    pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;

    
}


/*!
*******************************************************************************
 \brief RRC ask for frequency
*/
void open_freq_query( 
    Instance_t    inst            , //!< identification de l'instance
    L2_ID         L2_id           , //!< L2_id of the BTS/SU
    QOS_CLASS_T   QoS             , //!< quality of service required (0 means all available QoS) 
    Transaction_t Trans_id          //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    unsigned int NB_free_ch=0;
    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    CHANNELS_DB_T *pChannelsEntry=rrm->rrc.pChannelsEntry;
    CHANNELS_DB_T *pChannels = pChannelsEntry;
    //TO DO: here should be implemented the method to determine if a frequency is free
    
    while (pChannels!=NULL){
        if (pChannels->is_free)
            NB_free_ch++;
        pChannels = pChannels->next;
    }
    //fprintf(stdout,"cp3 : NB_free_chan %d\n", NB_free_ch); //dbg
    
    CHANNEL_T free_channels_hd[NB_free_ch];
    int j = 0;
    pChannels = pChannelsEntry;
    while (pChannels!=NULL){//mod_lor_10_03_08
        if (pChannels->is_free){

            memcpy(&(free_channels_hd[j]) , &(pChannels->channel), sizeof(CHANNEL_T));
            //fprintf(stdout,"open_freq_query: channel %d\n", free_channels_hd[j].Ch_id); //dbg
            j++;
        }
        pChannels = pChannels->next;
    }
   pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    
    //fprintf(stdout,"NB_free_chan %d\n", NB_free_ch); //dbg
    //for (int i = 0; i<NB_free_ch; i++)//dbg
       // fprintf(stdout,"channel %d meas %f\n", free_channels_hd[i].Ch_id, free_channels_hd[i].meas); //dbg
                        
            
             
    pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
    rrm->ip.trans_cnt++ ;
    PUT_IP_MSG(msg_update_open_freq_7( inst, L2_id, NB_free_ch, free_channels_hd, rrm->ip.trans_cnt));
    pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;

    
}

/*!
*******************************************************************************
 \brief RRC open frequency.  
 *
*/
void update_open_freq( 
    Instance_t inst             , //!< identification de l'instance
    L2_ID L2_id                 , //!< L2_id of the FC/CH
    unsigned int NB_chan        , //!< Number of available channels
    CHANNEL_T *fr_channels      , //!< List of available channels
    Transaction_t Trans_id        //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ;
    unsigned int NB_occ_chan = 0      ;
    unsigned int occ_channels[NB_chan];
    CHANNEL_T ass_channels[NB_chan];
    for (int i=0; i<NB_chan; i++)
    {
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
        if(up_chann_db( &(rrm->rrc.pChannelsEntry), fr_channels[i], 1, 0)==NULL) //info_time still to evaluate
            fprintf(stderr, "error in updating free channels in BTS \n");
        pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
        //fprintf(stderr,"chann db pointer  @%p \n", canal);//dbg 
        //rrm_t *rrm = &rrm_inst[inst] ;
        //TO DO: next 3 lines have to be substituted by the process that allocate frequencies to SUs 
        ass_channels[i]=fr_channels[i] ;
        //fprintf(stderr,"update_open_freq -> ass_channel  %d \n", ass_channels[i].Ch_id);//dbg
        occ_channels[i]=fr_channels[i].Ch_id ;
        //fprintf(stderr,"update_open_freq -> occ_channel  %d \n", occ_channels[i]);//dbg
        NB_occ_chan++;
    }
    
    // TO DO -> part in which the free channels are assigned to secondary users.         
    pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
    rrm->ip.trans_cnt++ ;
    PUT_IP_MSG(msg_update_SN_occ_freq_5( inst, rrm->L2_id, NB_occ_chan, occ_channels, rrm->ip.trans_cnt));
    pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;

    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    PUT_RRC_MSG(msg_rrm_up_freq_ass( inst, rrm->L2_id,  NB_occ_chan, ass_channels));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    
}

/*!
\brief RRC update secondary network frequencies in use  (SENDORA first scenario)
 */ 
void update_SN_occ_freq(
    Instance_t inst             , //!< instance ID 
    L2_ID L2_id                 , //!< Layer 2 (MAC) ID of BTS
    unsigned int NB_chan        , //!< number of channels used by secondary network
    unsigned int *occ_channels  , //!< channels used by secondary network
    Transaction_t Trans_id        //!< Transaction ID
    )
        
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    CHANNELS_DB_T *pChannelsEntry=rrm->rrc.pChannelsEntry;
    CHANNELS_DB_T *pChannels;
    Sens_node_t *nodes_db = rrm->rrc.pSensEntry;
    //fprintf(stderr,"update_SN_occ_freq  %d \n", inst);//dbg 
    //for (pChannels = rrm->rrc.pChannelsEntry; pChannels!=NULL; pChannels=pChannels->next)//dbg
    //    fprintf(stderr,"channel   %d  in db\n", pChannels->channel.Ch_id);//dbg
    for (int i=0; i<NB_chan; i++){
        //fprintf(stderr,"occ_channels  %d  val %d\n", i,occ_channels[i]);//dbg
        pChannels = up_chann_ass( pChannelsEntry  , occ_channels[i], 1, L2_id, L2_id );
        
        
    }
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    // TO DO -> start monitoring on the SN channels
    /*while (nodes_db!=NULL){
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
        rrm->rrc.trans_cnt++ ;
        PUT_RRC_MSG(msg_rrm_init_mon_req( inst, nodes_db->L2_id,  NB_chan, 1, occ_channels, rrm->rrc.trans_cnt));
        pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        nodes_db = nodes_db->next;
    }*/
    
    
    
}
    
/*!
\brief CMM of a SU asks to have a channel in SENDORA scenario 2 centralized  
 */ 
/*void cmm_need_to_tx(
    Instance_t inst             , //!< instance ID 
    QOS_CLASS_T QoS_class         //!< quality of service required 
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 

    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    PUT_RRC_MSG(msg_rrm_ask_for_freq( inst, rrm->L2_id_FC, QoS_class, rrm->rrc.trans_cnt));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
}*/

/*!
\brief CMM of a SU asks to start a connection with another SU in SENDORA scenario 2 distributed  
 */ 
/*void cmm_init_trans_req(
    Instance_t    inst      , //!< identification de l'instance 
    L2_ID L2_id             , //!< ID of SU with which it wants to communicate
    unsigned int Session_id , //!< Session identifier
    QOS_CLASS_T   QoS_class , //!< QOS class index
    Transaction_t Trans_id    //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 

    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    PUT_RRC_MSG(msg_rrm_init_conn_req( inst, L2_id, Session_id, QoS_class, rrm->rrc.trans_cnt));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
}*/


/*!
\brief RRC reported confirmation about the connection (SENDORA 2 scenario distr.)
    A selection of the potential channels to use is then performed
    and a msg_rrm_freq_all_prop is sent.
 */ 
/*void rrc_init_conn_conf(
    Instance_t    inst      , //!< identification de l'instance 
    L2_ID L2_id             , //!< ID of SU with which the connection will be established
    unsigned int Session_id , //!< Session identifier
    Transaction_t Trans_id    //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    CHANNELS_DB_T *pChann_db = rrm->rrc.pChannelsEntry;
    //CHANNEL_T *fr_channels;//CLC
    unsigned int NB_free_ch = 0;
    unsigned int i = 0;
    while ( pChann_db!=NULL)
    {
        //fprintf(stdout,"cp1: loop pChann_db!=NULL\n");//dbg
        //fprintf(stdout,"channel: %d is_free: %d is_ass: %d\n", pChann_db->channel.Ch_id, pChann_db->is_free, pChann_db->is_ass); //dbg
        if (pChann_db->is_free && !(pChann_db->is_ass))
            NB_free_ch++;
        pChann_db = pChann_db->next;
    }
    
    CHANNEL_T fr_channels[NB_free_ch];
    //fr_channels = RRM_CALLOC(CHANNEL_T, NB_free_ch ) ;//CLC
    pChann_db = rrm->rrc.pChannelsEntry;
    while ( pChann_db!=NULL)
    {
        if (pChann_db->is_free && !(pChann_db->is_ass)){
            fr_channels[i] = pChann_db->channel;
            i++;
        }
        pChann_db = pChann_db->next;
    }
    //fprintf(stdout,"freq prop NB_chan in rrc_init_conn_conf() cp2: %d\n", NB_free_ch);//dbg
    //for (int i = 0; i<NB_free_ch; i++) //dbg
    //    fprintf(stdout,"channels in rrc_init_conn_conf(): %d\n", fr_channels[i].Ch_id); //dbg
    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    PUT_RRC_MSG(msg_rrm_freq_all_prop( inst, L2_id, Session_id, NB_free_ch, fr_channels, Trans_id));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
}*/


/*!
\brief RRC reported confirmation from SU2 on the proposed channels (SENDORA 2 scenario distr.)
 */    
/*void rrc_freq_all_prop_conf( 
    Instance_t    inst              , //!< identification de l'instance
    L2_ID         L2_id             , //!< ID of the other SU involved in connection establishing
    unsigned int Session_id         , //!< Session identifier
    unsigned int NB_free_ch         , //!< Number of proposed channels
    CHANNEL_T   *fr_channels        , //!< List of proposed channels
    Transaction_t  Trans_id           //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    //CHANNELS_DB_T *pChann_db = rrm->rrc.pChannelsEntry;
    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    PUT_RRC_MSG(msg_rrm_rep_freq_all( inst, rrm->L2_id_FC, rrm->L2_id, L2_id, Session_id, NB_free_ch, fr_channels, Trans_id));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
}*/


/*!
\brief RRC reportes a message from CH communicating the assignement of a channel (SENDORA scenario 2 distr) 
 */
/*void rrc_rep_freq_ack( 
    Instance_t    inst              , //!< identification de l'instance
    L2_ID         L2_id_ch          , //!< Cluster head ID
    L2_ID         L2_id_source      , //!< ID of the first SU involved in connection establishing
    L2_ID         L2_id_dest        , //!< ID of the second SU involved in connection establishing
    unsigned int  Session_id        , //!< Session identifier
    CHANNEL_T     all_channel       , //!< Channel assigned to the two secondary users
    Transaction_t Trans_id            //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    up_chann_ass( rrm->rrc.pChannelsEntry, all_channel.Ch_id, 1, L2_id_source, L2_id_dest);
    //TO DO: add Session_id field in CHANNELS_DB or maybe a "active_session" variable in rrm.h
    if (L2_ID_cmp(&(L2_id_source), &(rrm->L2_id)) == 0 || L2_ID_cmp(&(L2_id_dest), &(rrm->L2_id)) == 0 ){
        pthread_mutex_lock( &( rrm->cmm.exclu ) ) ;
        rrm->cmm.trans_cnt++ ;
        PUT_CMM_MSG(msg_rrm_init_trans_conf( inst, Session_id, all_channel, Trans_id));
        pthread_mutex_unlock( &( rrm->cmm.exclu ) ) ;
    }
}*/
    
/*!
\brief RRC init connection request from another SU (SENDORA scenario 2 distr) 
 */
/*void rrc_init_conn_req( 
    Instance_t    inst            , //!< identification de l'instance
    L2_ID         L2_id           , //!< ID of an SU that wants to establish a connection
    unsigned int Session_id       , //!< Session identifier
    QOS_CLASS_T QoS_class         , //!< QoS required on the link
    Transaction_t Trans_id          //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    PUT_RRC_MSG(msg_rrm_conn_set( inst, L2_id, Session_id, QoS_class, Trans_id));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
}*/

/*!
\brief RRC reported proposed channels from SU1
 */    
/*void rrc_freq_all_prop( 
    Instance_t    inst              , //!< identification de l'instance
    L2_ID         L2_id             , //!< ID of the other SU involved in connection establishing
    unsigned int Session_id         , //!< Session identifier
    unsigned int NB_free_ch         , //!< Number of proposed channels
    CHANNEL_T   *fr_channels        , //!< List of proposed channels
    Transaction_t Trans_id            //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ;
    CHANNELS_DB_T *pChann_db ;
    //CHANNEL_T *pr_channels = RRM_CALLOC(CHANNEL_T, NB_free_ch );//CLC
    CHANNEL_T pr_channels[NB_free_ch];
    unsigned int NB_prop_ch;
    unsigned int j = 0;

    for ( int i = 0; i<NB_free_ch; i++)
    {
        pChann_db = get_chann_db_info( rrm->rrc.pChannelsEntry  , fr_channels[i].Ch_id );
        if (pChann_db == NULL || (pChann_db->is_free && !(pChann_db->is_ass))){
            pr_channels[j] = pChann_db->channel;
            j++;
        }
    }
    NB_prop_ch = j;
    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    PUT_RRC_MSG(msg_rrm_freq_all_prop_conf( inst, L2_id, Session_id, NB_prop_ch, pr_channels, Trans_id));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    
    
}
*/

/*!
\brief RRC reported proposed channels from SU1 in CH
 */    
/*void rrc_rep_freq_all( 
    Instance_t    inst              , //!< identification de l'instance
    L2_ID         L2_id_source      , //!< ID of the first SU involved in connection establishing
    L2_ID         L2_id_dest        , //!< ID of the second SU involved in connection establishing
    unsigned int  Session_id        , //!< Session identifier
    unsigned int  NB_prop_ch        , //!< Number of proposed channels
    CHANNEL_T     *pr_channels      , //!< List of proposed channels
    Transaction_t Trans_id            //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ;
    CHANNELS_DB_T *pChann_db ;
    CHANNEL_T all_channel ;

    for ( int i = 0; i<NB_prop_ch; i++)
    {
        pChann_db = get_chann_db_info( rrm->rrc.pChannelsEntry  , pr_channels[i].Ch_id );
        if (pChann_db != NULL && (pChann_db->is_free) && !(pChann_db->is_ass)){
            all_channel = pChann_db->channel;
            up_chann_ass( rrm->rrc.pChannelsEntry, pr_channels[i].Ch_id, 1, L2_id_source, L2_id_dest);
            break;
        }
    }
    fprintf(stdout,"channel: %d is_free: %d is_ass: %d\n source: ", pChann_db->channel.Ch_id, pChann_db->is_free, pChann_db->is_ass); //dbg
    for ( int i=0;i<8;i++)
        fprintf(stdout, "%02X", pChann_db->source_id.L2_id[i]);
    fprintf(stdout, "\n"); //dbg
    fprintf(stdout,"dest: ");
    for ( int i=0;i<8;i++)
        fprintf(stdout, "%02X", pChann_db->dest_id.L2_id[i]);
    fprintf(stdout, "\n"); //dbg
    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    PUT_RRC_MSG(msg_rrm_rep_freq_ack( inst, L2_id_source, L2_id_dest, Session_id, all_channel, Trans_id));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    
    
}*/
