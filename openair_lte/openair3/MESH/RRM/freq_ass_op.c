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
#include "sensing_rrm_msg.h"
#include "ip_msg.h" //mod_lor_10_04_27
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
//mod_lor_10_03_19++
/*!
*******************************************************************************
\brief  Function that decides if a channel is free: 
        applyed rule: strict majority
\return 1 if channel is free, 0 otherwise
*/
//mod_eure_lor
void take_decision( 
	Sens_node_t *SensDB     , //!< pointer to the sensing database
	unsigned int Ch_id      , //!< channel ID
	unsigned int *is_free  //mod_lor_10_05_28 ->char instead of int
	
	)
{
    Sens_node_t *pSensNode = SensDB;
    Sens_ch_t *pSensChann;
    //char is_free = 0;
    int i;
    while (pSensNode!= NULL){
        if((pSensChann = get_chann_info(  pSensNode->info_hd , Ch_id)) != NULL)    {
            for (i = 0; i<NUM_SB; i++){
                if (pSensChann->is_free[i]==1)
                    is_free[i]++;
                else  if (pSensChann->is_free[i]==0)
                    is_free[i]--;
            }
        }
        pSensNode = pSensNode->next;
    }
    //mod_lor_10_05_07++
    for (i = 0; i<NUM_SB; i++){
        if (is_free[i]>0)
            is_free[i]=1;
        else  
            is_free[i]=0;
    }
    
    //return pSensChann;
    /*if (is_free>0)
        return 1;
    else
        return 0;*/
    //return is_free;
    //mod_lor_10_05_07--
    
}
//mod_lor_10_03_19--
//mod_lor_10_05_26++
/*!
*******************************************************************************
\brief  Function that decides if a channel is free: 
        applyed rule: strict majority
\return 0 if operation ended correctly, 1 if info on sensing channel does not exist 
*/

unsigned int take_decision_sens( 
	Sens_node_t *SensDB     , //!< pointer to the sensing database
	Sens_ch_t *finalSensChann, //!< info that will be returned after decision
	unsigned int Ch_id       //!< channel ID
	
	)
{
    Sens_node_t *pSensNode = SensDB;
    Sens_ch_t *pSensChann;
    int i, flag=0;
    for (i = 0; i<NUM_SB; i++) // initialization of is_free vector
        finalSensChann->is_free[i]=10; //AAA: to change when is_free becomes char instead of unsigned int
    while (pSensNode!= NULL ){

        if((pSensChann = get_chann_info(  pSensNode->info_hd , Ch_id)) != NULL){
            flag++;    
            for (i = 0; i<NUM_SB; i++){
                //printf (" ch %d is_free %d", i, pSensChann->is_free[i]);
                if (pSensChann->is_free[i]==1)
                    finalSensChann->is_free[i]++;
                else  if (pSensChann->is_free[i]==0)
                    finalSensChann->is_free[i]--;
            }
        }
        pSensNode = pSensNode->next;
    }
    if (flag==0)  // case in which information on the channel requested is not in the database
        return 1;

    for (i = 0; i<NUM_SB; i++){
        if (finalSensChann->is_free[i]>10)
            finalSensChann->is_free[i]=1;
        else  
            finalSensChann->is_free[i]=0;
        //printf ("ch %d is_free %d \n", i, finalSensChann->is_free[i]);
    }
    finalSensChann->Start_f = pSensChann->Start_f;
    finalSensChann->Final_f = pSensChann->Final_f;
    finalSensChann->Ch_id   = pSensChann->Ch_id;
    return 0;
    
}
/*!
*******************************************************************************
\brief  Function that identify free block of SB_NEEDED_FOR_SN subbands analysing the 
* sensing information present in the sensing database 
        
\return 
*/
unsigned int find_available_channels(
    Sens_node_t *Sens_db,
    CHANNELS_DB_T **channels_db
    )
{
    int sens_ch_id=1, sb, sb_f;
    unsigned int curr_fr;
    CHANNEL_T channel;
    Sens_ch_t curr_sens_ch;
    
    del_all_channels( channels_db ); //to remove all old entries
    //printf("channels eliminated\n");//dbg
    channel.Ch_id = 0;
    channel.QoS   = 0;
    
    sb_f = 0;
    // printf("beginning of find_available_channels\n\n\n");//dbg
    while(take_decision_sens( Sens_db,&curr_sens_ch,sens_ch_id)==0)
    {
    //    printf("\nsens_ch_id in while :%d found channel id %d\n\n\n", sens_ch_id, curr_sens_ch.Ch_id);//dbg
        curr_fr = curr_sens_ch.Start_f;
        for (sb=0;sb<NUM_SB;sb++){ //loop over all the subbands of a sensing channel
         //   printf("        sb %d is_free %d", sb, curr_sens_ch.is_free[sb]);//dbg
            if (curr_sens_ch.is_free[sb]==0){
                sb_f = 0;
                curr_fr+=SB_BANDWIDTH;
            }
            else{
                sb_f++;
                if (sb_f == BG+1)  {
                    //printf("\nFirst free sb : %d start_freq_select %d\n",sb,curr_fr);//dbg
                    channel.Start_f = curr_fr;
                }
                if (sb_f == SB_NEEDED_FOR_SN+BG)
                    channel.Final_f = curr_fr+SB_BANDWIDTH;
                if (sb_f == (SB_NEEDED_FOR_SN+(2*BG))){
                    //curr_fr += SB_BANDWIDTH;
                    //printf("last free sb : %d end_freq_select %d\n",sb,channel.Final_f);//dbg
                    channel.Ch_id++;
                    add_chann_db(channels_db,channel,1,0);
                    sb_f = BG; //to avoid to compute two times the BG
                    //printf("added channel %d start %d end %d\n", channel.Ch_id, channel.Start_f, channel.Final_f);//dbg
                }
                curr_fr+=SB_BANDWIDTH;
            }
        }
        sens_ch_id++;
            
    }
    return (channel.Ch_id); //the last Ch_id memorized in channels database corresponds to the total number of free channels found
} 


//mod_lor_10_05_26--
//mod_lor_10_05_28++

 /*!
*******************************************************************************
\brief  Function to check if the channels used by SN are still free 
        
\return 
*/
unsigned int evalaute_sens_info(
    Sens_node_t *Sens_db,
    unsigned int Start_f,
    unsigned int Final_f
    )
{
    unsigned int i=0, sb, curr_freq;
    int is_free[MIN_NB_SB_CH];
    Sens_node_t *ac_node = Sens_db;
    Sens_ch_t *ac_chann;
    for (i=0; i<MIN_NB_SB_CH; i++)
        is_free[i]=0;
    while (ac_node != NULL){
        ac_chann = ac_node->info_hd;
        
        while (ac_chann!= NULL ){
           // printf("actual ch_id %d actual start_f %d final_f %d\n",ac_chann->Ch_id, ac_chann->Start_f,ac_chann->Final_f);//dbg
            if (ac_chann->Start_f < Final_f && ac_chann->Final_f > Start_f){
                sb = 0;
                curr_freq = ac_chann->Start_f;
                while (sb < NUM_SB && curr_freq<Start_f){
                    sb++;
                    curr_freq+=SB_BANDWIDTH;
                }
                //printf ("sb after first loop %d curr_fr %d\n", sb, curr_freq);//dbg
                while (sb < NUM_SB && curr_freq<Final_f){
                  //  printf ("curr_fr %d\n", curr_freq);//dbg
                    i = (curr_freq - Start_f)/SB_BANDWIDTH;
                  //  printf("index for is_free vector %d, freq %d, sb %d\n",i, curr_freq, sb);//dbg
                    if (i >= MIN_NB_SB_CH){
                        printf("ERROR!\n");
                        return 2;
                    }
                    if (ac_chann->is_free[sb]==1)
                        is_free[i]++;
                    if (ac_chann->is_free[sb]==0)
                        is_free[i]--;
                    sb++;
                    curr_freq+=SB_BANDWIDTH;
                }
                    
                
            }
            
            ac_chann=ac_chann->next;
        }
        
        ac_node = ac_node->next;
    }
    for (i=0; i<MIN_NB_SB_CH; i++)
        if (is_free[i]<=0)
            return 1;
    return 0;
    
}
/*!
*******************************************************************************
\brief  Function to check if the channels used by SN are still free 
        
\return 
*/
unsigned int check_SN_channels(
    Sens_node_t *Sens_db,
    CHANNELS_DB_T *channels_db,
    unsigned int *used_channels,
    unsigned int nb_used_ch
    
    )
{
    int i, need_to_update=0;
    CHANNELS_DB_T *channel_2_check;
    for (i=0; i<nb_used_ch; i++){
        channel_2_check = get_chann_db_info( channels_db  , used_channels[i] );
        //printf("check channel: %d, start_fr: %d, final_f: %d\n",used_channels[i], channel_2_check->channel.Start_f, channel_2_check->channel.Final_f);//dbg
        if( evalaute_sens_info(Sens_db, channel_2_check->channel.Start_f, channel_2_check->channel.Final_f))
            need_to_update = 1;
        //printf("need_to_update: %d\n",need_to_update);//dbg
    }
    return need_to_update;

}
//mod_lor_10_05_28--


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

    pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
    rrm->ip.trans_cnt++ ;
    PUT_IP_MSG(msg_open_freq_query_4( inst, rrm->L2_id_FC, 0, rrm->ip.trans_cnt));
    pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;

    
}


/*!
*******************************************************************************
 \brief BTS ask for frequency
*/
unsigned int open_freq_query( //mod_lor_10_06_02
    Instance_t    inst            , //!< identification de l'instance
    L2_ID         L2_id           , //!< L2_id of the BTS/SU
    QOS_CLASS_T   QoS             , //!< quality of service required (0 means all available QoS) 
    Transaction_t Trans_id          //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    unsigned int NB_chan=0, ind = 0;
    CHANNELS_DB_T *pChannels;
    CHANNELS_DB_T channels_hd[NB_SENS_MAX];
    //mod_lor_10_05_26++
    /*pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    CHANNELS_DB_T *pChannelsEntry=rrm->rrc.pChannelsEntry;
    CHANNELS_DB_T *pChannels = pChannelsEntry;

    //fprintf(stdout,"cp3 : NB_free_chan %d\n", NB_free_ch); //dbg
    //mod_lor_10_05_17++: send vector with all frequencies (not only free ones)
    CHANNELS_DB_T channels_hd[NB_SENS_MAX];
    pChannels = pChannelsEntry;
    while (pChannels!=NULL){//mod_lor_10_03_08
        memcpy(&(channels_hd[NB_chan]) , pChannels, sizeof(CHANNELS_DB_T));
        pChannels = pChannels->next;
        NB_chan++;
    }
    //mod_lor_10_05_17--
   pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    
    //fprintf(stdout,"NB_free_chan %d\n", NB_free_ch); //dbg
    //for (int i = 0; i<NB_free_ch; i++)//dbg
       // fprintf(stdout,"channel %d meas %f\n", free_channels_hd[i].Ch_id, free_channels_hd[i].meas); //dbg
    
    //printf("channels in open_freq_query_funct\n");//dbg
    //for (int i=0; i<NB_chan;i++)//dbg
    //    printf("  %d  ",channels_hd[i].channel.Ch_id);//dbg
    //printf("\n");//dbg         */   
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;        
    NB_chan = find_available_channels(rrm->rrc.pSensEntry,&(rrm->rrc.pChannelsEntry));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    
    pChannels = rrm->rrc.pChannelsEntry;
    while (pChannels!=NULL){//mod_lor_10_03_08
        memcpy(&(channels_hd[ind]) , pChannels, sizeof(CHANNELS_DB_T));
        //printf ("copied channel: %d start %d end %d\n",channels_hd[ind].channel.Ch_id,channels_hd[ind].channel.Start_f,channels_hd[ind].channel.Final_f);//dbg
        pChannels = pChannels->next;
        ind++;
    }
    if (ind!=NB_chan)    
        printf("Error!!! Found free channels: %d, copyed channels: %d\n",NB_chan, ind);
    //mod_lor_10_05_26--   
    if (Trans_id!=1 || NB_chan!=0) {  //mod_lor_10_06_02
        pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
        rrm->ip.trans_cnt++ ;
        PUT_IP_MSG(msg_update_open_freq_7( inst, L2_id, NB_chan, channels_hd, rrm->ip.trans_cnt));
        rrm->ip.waiting_SN_update=1;
        pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;
    }
    return (NB_chan);//mod_lor_10_06_02
}

/*!
*******************************************************************************
 \brief RRC open frequency.  
 *
*/
unsigned int update_open_freq(   //mod_lor_10_05_18
    Instance_t inst             , //!< identification de l'instance
    L2_ID L2_id                 , //!< L2_id of the FC/CH
    unsigned int NB_chan        , //!< Number of channels
    unsigned int *occ_channels  , //!< vector on wich the selected frequencies will be saved //mod_lor_10_05_18
    CHANNELS_DB_T *channels     , //!< List of channels
    Transaction_t Trans_id        //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ;
    unsigned int NB_occ_chan = 0 ;
    //unsigned int occ_channels[NB_chan];
    CHANNELS_DB_T *chann_checked;
    CHANNEL_T ass_channels[NB_chan];
    //printf("In update_open_freq: NB_chan: %d\n",NB_chan);
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    del_all_channels( &(rrm->rrc.pChannelsEntry) ) ; //mod_lor_10_06_01
    for (int i=0; i<NB_chan; i++)
    {
         //mod_lor_10_05_17++
        // fprintf(stderr, "update channel : %d that is %d\n",channels[i].channel.Ch_id, channels[i].is_free);//dbg
        chann_checked = up_chann_db( &(rrm->rrc.pChannelsEntry), channels[i].channel, channels[i].is_free, 0);
        if(chann_checked == NULL) //info_time still to evaluate
            fprintf(stderr, "error in updating free channels in BTS \n");
       /* else if (chann_checked->is_ass && !(chann_checked->is_free)){
            fprintf(stderr, "  -> Channel %d in use not free anymore! \n",channels[i].channel.Ch_id);
            chann_checked->is_ass = 0;
        }else if (chann_checked->is_ass && chann_checked->is_free){
            ass_channels[NB_occ_chan]=chann_checked->channel;
            occ_channels[NB_occ_chan]=chann_checked->channel.Ch_id ;
            NB_occ_chan++;
        }*/
    }
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    
    while (NB_occ_chan<CH_NEEDED_FOR_SN){
        chann_checked=select_new_channel( rrm->rrc.pChannelsEntry, rrm->L2_id, rrm->L2_id);
        if (chann_checked == NULL){
            //fprintf(stderr, "Channel is null \n"); //dbg
            break;
        }
        ass_channels[NB_occ_chan]=chann_checked->channel;
        occ_channels[NB_occ_chan]=chann_checked->channel.Ch_id ;
        NB_occ_chan++;
    }
    

    
    /*fprintf(stderr, "Channels for SN selected by BTS: \n"); //dbg
    for (int i=0; i<NB_occ_chan;i++)//dbg
        fprintf(stderr, "   %d   ", occ_channels[i]);//dbg
    fprintf(stderr, "\n");//dbg*/
    
    //mod_lor_10_05_17--
    
        
    pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
    rrm->ip.trans_cnt++ ;
    PUT_IP_MSG(msg_update_SN_occ_freq_5( inst, rrm->L2_id, NB_occ_chan, occ_channels, rrm->ip.trans_cnt));
    pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;

    //AAA: BTS sends a vector containing the channels that have to be used by secondary users
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    //printf("In update_open_freq: NB_chan passed: %d\n",NB_occ_chan);
    PUT_RRC_MSG(msg_rrm_up_freq_ass( inst, rrm->L2_id,  NB_occ_chan, ass_channels));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;

    return (NB_occ_chan);//mod_lor_10_05_18
}

/*!
\brief RRC update secondary network frequencies in use  (SENDORA first scenario)
 */ 
unsigned int update_SN_occ_freq( //mod_lor_10_05_18
    Instance_t inst             , //!< instance ID 
    L2_ID L2_id                 , //!< Layer 2 (MAC) ID of BTS
    unsigned int NB_chan        , //!< number of channels used by secondary network
    unsigned int *occ_channels  , //!< channels used by secondary network
    Transaction_t Trans_id        //!< Transaction ID
    )
        
{
    rrm_t *rrm = &rrm_inst[inst] ;
    pthread_mutex_lock( &( rrm->ip.exclu ) ) ;  //mod_lor_10_05_18
    rrm->ip.waiting_SN_update=0;                //mod_lor_10_05_18
    pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;//mod_lor_10_05_18
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    CHANNELS_DB_T *pChannelsEntry=rrm->rrc.pChannelsEntry;
    CHANNELS_DB_T *pChannels;
    unsigned int need_to_update = 0;
    //Sens_node_t *nodes_db = rrm->rrc.pSensEntry;
    //fprintf(stderr,"update_SN_occ_freq  %d \n", inst);//dbg 
    //for (pChannels = rrm->rrc.pChannelsEntry; pChannels!=NULL; pChannels=pChannels->next)//dbg
    //    fprintf(stderr,"channel   %d  in db\n", pChannels->channel.Ch_id);//dbg
    for (int i=0; i<NB_chan ; i++){//&& !need_to_update
        //mod_lor_10_05_28++
        //fprintf(stderr,"occ_channels  %d  val %d\n", i,occ_channels[i]);//dbg
        need_to_update = check_SN_channels(rrm->rrc.pSensEntry,rrm->rrc.pChannelsEntry,occ_channels,NB_chan);
        if (need_to_update==0){
            pChannels = up_chann_ass( pChannelsEntry  , occ_channels[i], 1, L2_id, L2_id );
            if (pChannels==NULL)
                printf ("ERROR! Channel not updated\n");
        }
       //mod_lor_10_05_28--
      
    }
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08

    return (need_to_update); //mod_lor_10_05_17
    
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
    //TO DO SCEN_2_DISTR: add Session_id field in CHANNELS_DB or maybe a "active_session" variable in rrm.h
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
