/*!
*******************************************************************************

\file       sens_op.c

\brief      Fonctions permettant la gestion des informations de sensing des 
            differents noeuds

\author     IACOBELLI Lorenzo

\date       21/10/09

   
\par     Historique:
            L.IACOBELLI 2010-03-19
            + "take_decision" function added

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
#include "sens_op.h"


//! Met un message dans la file des messages a envoyer
#define PUT_CMM_MSG(m)  put_msg(  &(rrm->file_send_cmm_msg), 0, rrm->cmm.s,m )  //mod_lor_10_01_25
#define PUT_PUSU_MSG(m) put_msg(  &(rrm->file_send_cmm_msg), 0, rrm->pusu.s,m)  //mod_lor_10_01_25
#define PUT_RRC_MSG(m)  put_msg(  &(rrm->file_send_rrc_msg), 0, rrm->rrc.s,m )  //mod_lor_10_01_25
#define PUT_IP_MSG(m)   put_msg(  &(rrm->file_send_ip_msg) , 1, rrm->ip.s,m  )  //mod_lor_10_01_25
#define PUT_SENS_MSG(m) put_msg(  &(rrm->file_send_sensing_msg), 0, rrm->sensing.s,m )  //mod_lor_10_04_01

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
int take_decision( 
	Sens_node_t *SensDB     , //!< pointer to the sensing database
	unsigned int Ch_id        //!< channel ID
	
	)
{
    Sens_node_t *pSensNode = SensDB;
    Sens_ch_t *pSensChann;
    int is_free = 0;
    while (pSensNode!= NULL){
        if((pSensChann = get_chann_info(  pSensNode->info_hd , Ch_id)) != NULL)    {
            if (pSensChann->is_free==1)
                is_free++;
            else  if (pSensChann->is_free==0)
                is_free--;
        }
        pSensNode = pSensNode->next;
    }
        
    if (is_free>0)
        return 1;
    else
        return 0;
    
}
//mod_lor_10_03_19--
/*!
*******************************************************************************
\brief  Updating of the sensing measures 
*/
void rrc_update_sens( 
	Instance_t inst         , //!< Identification de l'instance
	L2_ID L2_id             , //!< Adresse L2 of the source of information 
	unsigned int NB_info    , //!< Number of channel info
	Sens_ch_t *Sens_meas    , //!< Pointer to the sensing information
	double info_time
	)
{
    rrm_t *rrm = &rrm_inst[inst] ; 

   
    int i;
    
    /*fprintf(stderr,"NB_info = %d\n",NB_info);//dbg
    Sens_ch_t *p;//dbg
    for ( i=0; i<NB_info; i++)//dbg
        fprintf(stderr," %d     ",Sens_meas[i].Ch_id);//dbg
    fprintf(stderr," \nrrm_database     ");//dbg
    if (rrm->rrc.pSensEntry != NULL)//dbg
        for ( p=rrm->rrc.pSensEntry->info_hd; p!=NULL; p=p->next)//dbg
            fprintf(stderr," %d     ",p->Ch_id);//dbg
    else//dbg
        fprintf(stderr," empty     ");//dbg
    fprintf(stderr,"\n1 update\n");//dbg*/
    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    update_node_info( &(rrm->rrc.pSensEntry), &L2_id, NB_info, Sens_meas, info_time);
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    
    
    //AAA: for the moment the channel db is reserved for CHs and SUs only in S2D 
    
    
    if ( SCEN_2_DISTR) 
    {
        
        //fprintf(stderr,"cluster_head\n");//dbg
        
        CHANNEL_T channel ;
        CHANNELS_DB_T *canal;
        int is_free;
        for (i=0; i<NB_info; i++){
            
            channel.Start_f = Sens_meas[i].Start_f;
            channel.Final_f = Sens_meas[i].Final_f;
            channel.Ch_id   = Sens_meas[i].Ch_id;
            channel.QoS     = 0;
            is_free     = Sens_meas[i].is_free;
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
            canal = up_chann_db( &(rrm->rrc.pChannelsEntry), channel, is_free, info_time);
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
            //fprintf(stderr,"inst %d, channel %d, is_free %d\n", inst,Sens_meas[i].Ch_id,Sens_meas[i].is_free);//dbg
            //fprintf(stderr,"chann %d updated\n", Sens_meas[i].Ch_id);//dbg 
            
        }
       
        //AAA: just to save the right L2_id for the simulation
        if ( rrm->state != CLUSTERHEAD && SCEN_2_DISTR)
            memcpy( rrm->L2_id.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
    
    }
    
    // mod_lor_10_01_25: monitoring will be launched after an Update SN occupied frequencies message from BTS
    /*if ( rrm->state == CLUSTERHEAD && !SCEN_2_DISTR)
    {
        unsigned int ch_to_scan[NB_info];
        for (i=0; i<NB_info; i++)
            ch_to_scan[i]=Sens_meas[i].Ch_id;
        if (SCEN_2_CENTR && (L2_ID_cmp(&(rrm->L2_id_FC),  &L2_id))==0){
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            rrm->rrc.trans_cnt++ ;
            //To send via IP: PUT_RRC_MSG(msg_rrm_clust_mon_req( inst, L2_id, ch_to_scan, NB_info, 0.5, rrm->rrc.trans_cnt));
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        }
        else{
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            rrm->rrc.trans_cnt++ ;
            PUT_RRC_MSG(msg_rrm_init_mon_req( inst, L2_id,  NB_info, 0.5, ch_to_scan, rrm->rrc.trans_cnt));
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        }
    }*/
#ifndef    RRC_EMUL   
    else if (!SCEN_2_DISTR && rrm->state != CLUSTERHEAD) ///< Case in which a sensor have to inform the FC via IP about its sensing results
    {
        //fprintf (stdout,"msg IP to send from inst %d\n",rrm->id);//dbg
        pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
        rrm->ip.trans_cnt++ ;
        PUT_IP_MSG(msg_update_sens_results_3( inst, rrm->L2_id, NB_info, Sens_meas, rrm->ip.trans_cnt)); 
        pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;

    }
#endif
    //fprintf(stderr,"end funct rrc_update_sens\n");//dbg
}


/*!
*******************************************************************************
 \brief CMM init sensing request.  Only in CH/FC. 
*/
void cmm_init_sensing( 
    Instance_t       inst,            //!< identification de l'instance
    unsigned int     Start_fr,
    unsigned int     Stop_fr,
    unsigned int     Meas_band,
    unsigned int     Meas_tpf,
    unsigned int     Nb_channels,
    unsigned int     Overlap,
    unsigned int     Sampl_freq
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    if ( (rrm->state == CLUSTERHEAD_INIT1 ) || (rrm->state == CLUSTERHEAD ) )
    {
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
        rrm->rrc.trans_cnt++ ;
        //fprintf(stderr,"rrc counter %d in rrm_init_scan_req  \n",rrm->rrc.trans_cnt);//dbg
        PUT_RRC_MSG(msg_rrm_init_scan_req( inst, Start_fr ,Stop_fr,Meas_band, Meas_tpf,
                         Nb_channels,  Overlap, Sampl_freq, rrm->rrc.trans_cnt));
        pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        
        
       /* if (SCEN_2_CENTR){
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            rrm->rrc.trans_cnt++ ;
            PUT_RRC_MSG(msg_rrm_clust_scan_req( inst, rrm->L2_id_FC, interv, 1, rrm->rrc.trans_cnt));
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        }*/
        
    }
    else
    {
        fprintf(stderr,"It is not a cluster head!!!"); 
    }
        
    
    
}

/*!
*******************************************************************************
 \brief rrc transmits order to start sensing
*/
//mod_lor_10_03_13++
void rrc_init_scan_req( 
    Instance_t inst           , //!< identification de l'instance
    L2_ID     L2_id           , //!< FC/CH address
    unsigned int     Start_fr,
    unsigned int     Stop_fr,
    unsigned int     Meas_band,
    unsigned int     Meas_tpf,
    unsigned int     Nb_channels,
    unsigned int     Overlap,
    unsigned int     Sampl_freq,
    Transaction_t  Trans_id     //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ;
    Sens_ch_t ch_info_init[Nb_channels];
    unsigned int Sampl_nb = ((Start_fr - Stop_fr)/Sampl_freq)/Nb_channels; //mod_lor_10_04_01: number of samples per sub-band
    unsigned int     act_start_fr = Start_fr;
    for (int i = 0; i<Nb_channels; i++){
        ch_info_init[i].Start_f = act_start_fr   ; 
        act_start_fr+=Meas_band;
        ch_info_init[i].Final_f = act_start_fr  ; ///< frequence final du canal
        ch_info_init[i].Ch_id  = i + 1    ; ///< ID du canal
        ch_info_init[i].meas   = 0    ; ///< Sensing results 
        ch_info_init[i].is_free  = 2  ; ///< Decision about the channel
    }
    
    
    //fprintf(stderr,"2 update\n");//dbg
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_13
    update_node_par( &(rrm->rrc.pSensEntry), &(rrm->L2_id), Nb_channels, ch_info_init, 0,Meas_tpf,Overlap,Sampl_freq); //mod_lor_10_02_19 
    //update_node_info( &(rrm->rrc.pSensEntry), &(rrm->L2_id), Nb_channels, ch_info_init, 0); 
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_13
    

    memcpy( rrm->L2_id_FC.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;

    pthread_mutex_lock( &( rrm->sensing.exclu ) ) ;
    rrm->sensing.trans_cnt++ ;
    //fprintf(stderr,"sensing counter %d in msg_rrm_scan_ord on socket %d \n",rrm->sensing.trans_cnt,rrm->sensing.s->s);//dbg
    PUT_SENS_MSG(msg_rrm_scan_ord( inst,  Nb_channels, Meas_tpf, Overlap, Sampl_nb, ch_info_init, Trans_id )); //mod_lor_10_04_01: Sampl_nb instead of Sampl_freq
    pthread_mutex_unlock( &( rrm->sensing.exclu ) ) ;
    
    
/*//mod_lor -> AAA: to remove when rrc_update_sens message is active.
    unsigned int length_info = 3;
    Sens_ch_t Sensing_meas[length_info];
    act_start_fr = Start_fr;
    for (int i = 0; i<length_info; i++){
        Sensing_meas[i].Start_f = act_start_fr   ; 
        act_start_fr+=Meas_band;
        Sensing_meas[i].Final_f = act_start_fr  ; ///< frequence final du canal
        Sensing_meas[i].Ch_id  = i + 1    ; ///< ID du canal
        Sensing_meas[i].meas   = 10+i   ; ///< Sensing results 
        Sensing_meas[i].is_free  = ((inst%2  )+((i+1)%2  ))%2; ///< Decision about the channel
        if (inst==4)
             Sensing_meas[i].is_free  = ((inst%2  )+((i)%2  ))%2; ///< Decision about the channel
        fprintf(stderr,"inst %d, channel %d, is_free %d\n", inst,Sensing_meas[i].Ch_id,Sensing_meas[i].is_free);//dbg
    }   
    
#ifdef    RRC_EMUL 
    rrc_update_sens( inst,rrm->L2_id, length_info, Sensing_meas,0);

#else
    
    pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
    rrm->ip.trans_cnt++ ;
    PUT_IP_MSG(msg_update_sens_results_3( inst, rrm->L2_id, 3, Sensing_meas, rrm->ip.trans_cnt));
    pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;
#endif    
//mod_lor -< */

    
}//mod_lor_10_03_13--

/*!
*******************************************************************************
 \brief CMM stop sensing request.  Only in CH/FC. With this function the node
        sends an order to stop sensing to all sensing nodes that were regeistered
        in sensing database. 
*/
void cmm_stop_sensing( 
    Instance_t inst            //!< identification de l'instance
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    //fprintf(stderr,"1 end\n");//dbg
    if ( (rrm->state == CLUSTERHEAD_INIT1 ) || (rrm->state == CLUSTERHEAD ) )
    {
        //fprintf(stderr,"2 end\n");//dbg
        Sens_node_t     *p = rrm->rrc.pSensEntry;
        if (p == NULL)
            fprintf(stderr,"no sensor node information saved\n");
        while (p!=NULL){
            //for ( int i=0;i<8;i++)
            //    fprintf(stderr,"cmm_stop_sens: to send on %X end\n",p->L2_id.L2_id[i]);//dbg
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            rrm->rrc.trans_cnt++ ;
            //fprintf(stderr,"rrc counter %d in msg_rrm_end_scan_req  \n",rrm->rrc.trans_cnt);//dbg
            PUT_RRC_MSG(msg_rrm_end_scan_req( inst, p->L2_id, rrm->rrc.trans_cnt));
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
            p = p->next;
        }
        if (SCEN_2_CENTR){ //!< To inform the CH that is collaborating in sensing to stop sensing
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            rrm->rrc.trans_cnt++ ;
            //fprintf(stderr,"rrc counter %d in msg_rrm_end_scan_req  \n",rrm->rrc.trans_cnt);//dbg
            PUT_RRC_MSG(msg_rrm_end_scan_req( inst, rrm->L2_id_FC, rrm->rrc.trans_cnt));
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        }
        
    }
    
        
    else
    {
        fprintf(stderr,"It is not a cluster head!!!"); 
    }
        
    
    
}


/*!
\brief RRC ending sensing confirmation 
 */
void rrc_end_scan_conf(
    Instance_t        inst            , //!< instance ID
    L2_ID             L2_id           , //!< FC/CH address
    Transaction_t     Trans_id          //!< Transaction ID
    
    )
{
    rrm_t *rrm = &rrm_inst[inst] ;     
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    del_node( &(rrm->rrc.pSensEntry), &L2_id ) ;
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    
    if ((rrm->role == CH_COLL) && (rrm->rrc.pSensEntry == NULL)){
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
        rrm->rrc.trans_cnt++ ;
        //fprintf(stderr,"rrc counter %d in end_scan_conf->to do  \n",rrm->rrc.trans_cnt);//dbg
        //To send via IP: PUT_RRC_MSG(msg_rrm_end_scan_conf( inst, rrm->L2_id_FC, rrm->rrc.trans_cnt));
        pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    }
        
    
}

/*!
\brief RRC ending sensing request 
 */
void rrc_end_scan_req(
    Instance_t        inst            , //!< instance ID
    L2_ID             L2_id           , //!< FC/CH address
    Transaction_t     Trans_id          //!< Transaction ID
    
    )
{
   
    //fprintf(stdout,"rrc_end_scan_req() cp1 %d\n",inst); //dbg
    rrm_t *rrm = &rrm_inst[inst] ; 
    Sens_node_t *pNode = rrm->rrc.pSensEntry;
    if (pNode==NULL)
        fprintf(stderr,"Database empty \n");
        
    else{
        if (rrm->role == CH_COLL && (L2_ID_cmp(&(rrm->L2_id_FC),  &L2_id))==0 )
        {
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;//mod_lor_10_03_08
            pNode = rrm->rrc.pSensEntry;//mod_lor_10_03_08
            while (pNode!=NULL){
                
                rrm->rrc.trans_cnt++ ;
                //fprintf(stderr,"rrc counter %d msg_rrm_end_scan_req  \n",rrm->rrc.trans_cnt);//dbg
                PUT_RRC_MSG(msg_rrm_end_scan_req( inst, pNode->L2_id, rrm->rrc.trans_cnt));
                
                pNode = pNode->next;
            }
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;    //mod_lor_10_03_08
            
        }
        else 
        {
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;//mod_lor_10_03_08
            pNode = rrm->rrc.pSensEntry;//mod_lor_10_03_08
            unsigned int i=0, Nb_chan = rrm->rrc.pSensEntry->Nb_chan;
            Sens_ch_t *ch_point = rrm->rrc.pSensEntry->info_hd;
            unsigned int channels[Nb_chan];

            ch_point = rrm->rrc.pSensEntry->info_hd;
            while  (ch_point!=NULL){
                channels[i]=ch_point->Ch_id;
                ch_point = ch_point->next;
                i++;
            }
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;//mod_lor_10_03_08
            
            if ((L2_ID_cmp(&(rrm->L2_id_FC),  &L2_id))==0) {
                
                pthread_mutex_lock( &( rrm->sensing.exclu ) ) ;
                rrm->sensing.trans_cnt++ ;
                //fprintf(stderr,"sensing counter %d msg_rrm_end_scan_ord  \n",rrm->sensing.trans_cnt);//dbg
                PUT_SENS_MSG(msg_rrm_end_scan_ord(inst, Nb_chan, channels, Trans_id ));
                pthread_mutex_unlock( &( rrm->sensing.exclu ) ) ;
                /*//mod_lor_10_04_14++ provvisorio
                ///< TO DO: This order is also sent to the RRC that have to send the confirmation
                pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
                rrm->rrc.trans_cnt++ ;
                fprintf(stderr,"rrc counter %d msg_rrm_end_scan_ord  \n",rrm->rrc.trans_cnt);//dbg
                PUT_RRC_MSG(msg_rrm_end_scan_ord(inst, Nb_chan, channels, Trans_id ));
                pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
                //mod_lor_10_04_14--*/

            }
            else {
                fprintf(stderr,"The message received is not from the right FC \n");
            }
        }
    }
    //dbg: test end_scan_ack
    /*
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    
    PUT_RRC_MSG(msg_rrm_end_scan_ord(inst, rrm->L2_id_FC, 0, NULL, Trans_id ));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;*/
    
    
}

/*!
*******************************************************************************
 \brief rrc transmits order to monitor channels
*/
void rrc_init_mon_req( 
    Instance_t inst           , //!< identification de l'instance
    L2_ID     L2_id           , //!< FC address
    unsigned int  *ch_to_scan , //!< vector of identifiers of the channels to monitor
    unsigned int  NB_chan     , //!< Number of channels to monitor
    unsigned int     interv          , //!< time between two sensing operation
    Transaction_t  Trans_id     //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    if ((L2_ID_cmp(&(rrm->L2_id_FC),  &L2_id))==0) {
   
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
        rrm->rrc.trans_cnt++ ;
        //memcpy( rrm->L2_id_FC.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ;
        //fprintf(stderr,"rrc counter %d msg_rrm_scan_ord  \n",rrm->rrc.trans_cnt);//dbg
        //PUT_RRC_MSG(msg_rrm_scan_ord( inst,  NB_chan, ch_to_scan, Trans_id ));
        pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    }
    else {
        fprintf(stderr,"The message received is not from the right FC \n");
    }
    
}

//To send via IP: 

/*!
*******************************************************************************
 \brief rrc transmits order to monitor channels
*/
/*
void rrc_clust_scan_req( 
    Instance_t inst             , //!< instance ID 
    L2_ID L2_id                 , //!< Layer 2 (MAC) ID of CH1
    float interv                , //!< time between two sensing operation
    COOPERATION_T coop          , //!< Cooperation mode
    Transaction_t Trans_id        //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 

    memcpy( rrm->L2_id_FC.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ; //AAA: how to save the CH1 address? -> to evaluate
    rrm->role = CH_COLL; 
    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    PUT_RRC_MSG(msg_rrm_init_scan_req( inst, interv, rrm->rrc.trans_cnt));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    PUT_RRC_MSG(msg_rrm_clust_scan_conf( inst, L2_id, coop, Trans_id));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    

    
}*/

/*!
*******************************************************************************
 \brief rrc transmits order from CH1 to monitor channels
*/
/*
void rrc_clust_mon_req( 
    Instance_t inst           , //!< identification de l'instance
    L2_ID     L2_id           , //!< Layer 2 (MAC) ID of CH1
    unsigned int  *ch_to_scan , //!< vector of identifiers of the channels to monitor
    unsigned int  NB_chan     , //!< Number of channels to monitor
    float     interv          , //!< time between two sensing operation
    Transaction_t  Trans_id     //!< Transaction ID
    )
{
    rrm_t *rrm = &rrm_inst[inst] ; 
    
    if ((L2_ID_cmp(&(rrm->L2_id_FC),  &L2_id))==0) {
   
        Sens_node_t *pNode = rrm->rrc.pSensEntry;
        while (pNode!=NULL){
             pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            rrm->rrc.trans_cnt++ ;
            PUT_RRC_MSG(msg_rrm_init_mon_req( inst,  pNode->L2_id, NB_chan, interv, ch_to_scan, Trans_id ));
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
            pNode = pNode->next;
        }
       
        
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
        rrm->rrc.trans_cnt++ ;
        PUT_RRC_MSG(msg_rrm_clust_mon_conf( inst,  L2_id, Trans_id ));
        pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        
        
    }
    else {
        fprintf(stderr,"The message received is not from the right CH \n");
    }
}*/

/*!
*******************************************************************************
\brief  Updating of the sensing measures received via IP from another node
*/
void update_sens_results( 
	Instance_t inst         , //!< Identification de l'instance
	L2_ID L2_id             , //!< Adresse L2 of the source of information 
	unsigned int NB_info    , //!< Number of channel info
	Sens_ch_t *Sens_meas    , //!< Pointer to the sensing information
	double info_time
	)
{
    rrm_t *rrm = &rrm_inst[inst] ; 

   
    int i;

    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    //fprintf(stderr,"inst update %d\n", rrm->state);//dbg
    update_node_info( &(rrm->rrc.pSensEntry), &L2_id, NB_info, Sens_meas, info_time);
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    //fprintf(stderr,"node entry  @%p \n", rrm->rrc.pSensEntry);//dbg
    //fprintf(stderr,"2 cluster_head\n");//dbg
    //AAA: for the moment the channel db is reserved for CHs and SUs only in S2D 
    
    
  
    if ( rrm->role == FUSIONCENTER || SCEN_2_DISTR) //mod_lor_10_03_08: role instead of status -> to check
    {
        
        //fprintf(stderr,"cluster_head\n");//dbg
        CHANNEL_T channel ;
        CHANNELS_DB_T *canal;
        int is_free;
        for (i=0; i<NB_info; i++){
            
            channel.Start_f = Sens_meas[i].Start_f;
            channel.Final_f = Sens_meas[i].Final_f;
            channel.Ch_id   = Sens_meas[i].Ch_id;
            channel.QoS     = 0;
            
            //mod_lor_10_03_19++
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            is_free = take_decision(rrm->rrc.pSensEntry, channel.Ch_id);
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
            //mod_lor_10_03_19--
            
            //fprintf(stdout,"Channel %d is %d \n", channel.Ch_id,is_free); //dbg ou LOG
            
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            canal = up_chann_db( &(rrm->rrc.pChannelsEntry), channel, is_free, info_time);
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
            //fprintf(stderr,"chann %d updated\n", Sens_meas[i].Ch_id);//dbg
            
        }

    
    }else   
        fprintf(stderr,"error!!! Cannot update channels \n");

}

//mod_lor_10_04_14++
/*!
*******************************************************************************
\brief  SENSING unit end scan confirmation 
*/
void sns_end_scan_conf( 
	Instance_t inst          //!< Identification de l'instance
	)
{
    rrm_t *rrm = &rrm_inst[inst] ; 

    ///< Next three lines delete the local sensing information database
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    del_node( &(rrm->rrc.pSensEntry), &(rrm->L2_id));
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    
    ///< AAA TO DO: Confirmation sent via RRC to the fusion centre
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    fprintf(stderr, "before put RRM_end_scan_confirm\n");//dbg
    PUT_RRC_MSG(msg_rrm_end_scan_conf( inst, rrm->rrc.trans_cnt));
    fprintf(stderr, "after put RRM_end_scan_confirm\n"); //dbg
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;

}
//mod_lor_10_04_14--
