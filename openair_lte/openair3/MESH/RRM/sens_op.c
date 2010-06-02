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
#include "ip_msg.h" //mod_lor_10_04_27
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



//mod_lor_10_05_26++
/*!
*******************************************************************************
\brief  Function that decides locally if a channel is free using mu0 and mu1 datas: 
        
\return 
*/
void take_local_decision(
    Sens_ch_t *Sens_info
    )
{
    unsigned int i;
    for (i=0; i < NUM_SB; i++){
        if (Sens_info->mu0[i]>LAMBDA0 && Sens_info->mu1[i]>LAMBDA1)
            Sens_info->is_free[i]=0; // primary system is present
        else
            Sens_info->is_free[i]=1; // primary system is not present
    }
} 
//mod_lor_10_05_26--

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

   
    int i,j;
    
    //fprintf(stderr,"rrc_update_sens NB_info = %d\n",NB_info);//dbg
   // Sens_ch_t *p;//dbg
    /*for ( i=0; i<NB_info; i++){//dbg
        fprintf(stderr," Ch_id %d     \n",Sens_meas[i].Ch_id);//dbg
        for (j=0;j<MAX_NUM_SB;j++)
            fprintf(stderr,"    SB %d  is %d   \n",j,Sens_meas[i].is_free[j]);//dbg
    }*/
    /*fprintf(stderr," \nrrm_database     ");//dbg
    if (rrm->rrc.pSensEntry != NULL)//dbg
        for ( p=rrm->rrc.pSensEntry->info_hd; p!=NULL; p=p->next)//dbg
            fprintf(stderr," %d     ",p->Ch_id);//dbg
    else//dbg
        fprintf(stderr," empty     ");//dbg
    fprintf(stderr,"\n1 update\n");//dbg*/
    for (i=0; i<NB_info;i++){
        take_local_decision(&Sens_meas[i]); //mod_lor_10_05_26
        //for (int j=0; j<NUM_SB; j++)//dbg
        //    fprintf(stderr,"sns_update: channel %d is %d\n",Sens_meas[i].Ch_id, Sens_meas[i].is_free[j]);//dbg*/
    }
    
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
    update_node_info( &(rrm->rrc.pSensEntry), &L2_id, NB_info, Sens_meas, info_time);
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08

    
    //AAA: for the moment the channel db is reserved for CHs and SUs only in SCEN_2_DISTR 
    
    
    if ( SCEN_2_DISTR) 
    {
        
        //fprintf(stderr,"cluster_head\n");//dbg
        
        CHANNEL_T channel ;
        CHANNELS_DB_T *canal;
        unsigned int *is_free; //mod_eure_lor //mod_lor_10_05_28 ->char instead of int
        for (i=0; i<NB_info; i++){
            
            channel.Start_f = Sens_meas[i].Start_f;
            channel.Final_f = Sens_meas[i].Final_f;
            channel.Ch_id   = Sens_meas[i].Ch_id;
            channel.QoS     = 0;
            is_free     = Sens_meas[i].is_free;
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
            canal = up_chann_db( &(rrm->rrc.pChannelsEntry), channel, is_free[0], info_time);//TO DO fix it!
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ; //mod_lor_10_03_08
            //fprintf(stderr,"inst %d, channel %d, is_free %d\n", inst,Sens_meas[i].Ch_id,Sens_meas[i].is_free);//dbg
            //fprintf(stderr,"chann %d updated\n", Sens_meas[i].Ch_id);//dbg 
            
        }
       
        //AAA: just to save the right L2_id in SCEN_2_DISTR
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
        
        //mod_lor_10_04_22++
        /*int r =  send_msg_int( rrm->ip.s, msg_update_sens_results_3( inst, rrm->L2_id, NB_info, Sens_meas, rrm->ip.trans_cnt));
                    WARNING(r!=0);*/
        
        PUT_IP_MSG(msg_update_sens_results_3( inst, rrm->L2_id, NB_info, Sens_meas, rrm->ip.trans_cnt)); 
        //mod_lor_10_04_22--
        pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;
        
        //mod_lor_10_04_21++ TO DO -> to remove when sensing remont automatically info
        //sleep(10);
        /*if(rrm->sensing.sens_active){
            sleep(10);
            pthread_mutex_lock( &( rrm->sensing.exclu ) ) ;
            rrm->sensing.trans_cnt++ ;
            //fprintf(stderr,"sensing counter %d in msg_rrm_scan_ord on socket %d \n",rrm->sensing.trans_cnt,rrm->sensing.s->s);//dbg
            PUT_SENS_MSG(msg_rrm_scan_ord( inst,  NB_info, 0, 0, 0, Sens_meas, rrm->sensing.trans_cnt )); //mod_lor_10_04_01: Sampl_nb instead of Sampl_freq
            pthread_mutex_unlock( &( rrm->sensing.exclu ) ) ;
        }*/
        //mod_lor_10_04_21--

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
    
    //if ( (rrm->state == CLUSTERHEAD_INIT1 ) || (rrm->state == CLUSTERHEAD ) )
    if (rrm->role == FUSIONCENTER || rrm->role == CH_COLL) //mod_lor_10_05_05
    {
        
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
        rrm->rrc.trans_cnt++ ;
        //fprintf(stderr,"rrc counter %d in rrm_init_scan_req  \n",rrm->rrc.trans_cnt);//dbg
        PUT_RRC_MSG(msg_rrm_init_scan_req( inst, Start_fr ,Stop_fr,Meas_band, Meas_tpf,
                         Nb_channels,  Overlap, Sampl_freq, rrm->rrc.trans_cnt));
        pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        
       //mod_lor_10_05_05++ 
       if (SCEN_2_CENTR && rrm->role == FUSIONCENTER){
            pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
            rrm->ip.trans_cnt++ ;
            PUT_IP_MSG(msg_init_coll_sens_req( inst, rrm->L2_id, Start_fr, Stop_fr,Meas_band, Meas_tpf,
                         Nb_channels,  Overlap, Sampl_freq, rrm->ip.trans_cnt));
            pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;
        }
        //mod_lor_10_05_05--
        
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
        //ch_info_init[i].meas   = 0    ; ///< Sensing results 
        //mod_eure_lor++
        for (int j=0;j<MAX_NUM_SB;j++)
            ch_info_init[i].is_free[j]  = 2  ; ///< Decision about the channel
        //mod_eure_lor--
    }
    
    

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
 //   fprintf(stderr,"stop sensing %d role %d\n\n\n\n\n\n\n\n\n", rrm->id, rrm->role);//dbg
   // if ( (rrm->state == CLUSTERHEAD_INIT1 ) || (rrm->state == CLUSTERHEAD ) )
    if ( (rrm->role == FUSIONCENTER ) || (rrm->role == CH_COLL ) ) //mod_lor_10_05_06
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
        //mod_lor_10_05_06++
        if (SCEN_2_CENTR){ //TO DO: need to add control to know if collaboration is active
            if (rrm->role == FUSIONCENTER){
                sleep(2);
                pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
                rrm->ip.trans_cnt++ ;
                //fprintf(stderr,"rrc counter %d in msg_rrm_end_scan_req  \n",rrm->rrc.trans_cnt);//dbg
                PUT_IP_MSG(msg_stop_coll_sens( inst));
                pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;
            }else{
                pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
                rrm->ip.trans_cnt++ ;
                //fprintf(stderr,"rrc counter %d in msg_rrm_end_scan_req  \n",rrm->rrc.trans_cnt);//dbg
                //PUT_IP_MSG(msg_stop_coll_sens_conf( inst));
                pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;
            }
        }
        
        /*if (SCEN_2_CENTR ){ //!< To inform the CH that is collaborating in sensing to stop sensing
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            rrm->rrc.trans_cnt++ ;
            //fprintf(stderr,"rrc counter %d in msg_rrm_end_scan_req  \n",rrm->rrc.trans_cnt);//dbg
            PUT_RRC_MSG(msg_rrm_end_scan_req( inst, rrm->L2_id_FC, rrm->rrc.trans_cnt));
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        }*/
        //mod_lor_10_05_06--
        
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
        pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
        PUT_IP_MSG(msg_stop_coll_sens_conf( inst, rrm->L2_id)); //mod_lor_10_05_12
        pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;
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
        if (rrm->role == CH_COLL && (L2_ID_cmp(&(rrm->L2_id_FC),  &L2_id))==0 ) ///< case SCEN_2_CENTR
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
                PUT_SENS_MSG(msg_rrm_end_scan_ord(inst, Nb_chan, channels, Trans_id ));
                pthread_mutex_unlock( &( rrm->sensing.exclu ) ) ;
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

    memcpy( rrm->L2_id_FC.L2_id, L2_id.L2_id, sizeof(L2_ID) )  ; 
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
unsigned int update_sens_results( 
	Instance_t inst         , //!< Identification de l'instance
	L2_ID L2_id             , //!< Adresse L2 of the source of information 
	unsigned int NB_info    , //!< Number of channel info
	Sens_ch_t *Sens_meas    , //!< Pointer to the sensing information
	double info_time
	)
{
    rrm_t *rrm = &rrm_inst[inst] ; 

    CHANNELS_DB_T *channel;
    int i,j,k, send_up_to_SN=0;
    
   /* printf("Update from node\n");
    for (i=0;i<NB_info;i++){
        printf("channel %d st: %d end: %d\n", Sens_meas[i].Ch_id,Sens_meas[i].Start_f,Sens_meas[i].Final_f);
        for (k=0;k<NUM_SB;k++)
            printf ("bk= %d, mu0=%d; mu1=%d; I0 = %d; is_free = %d\n",k,Sens_meas[i].mu0[k],Sens_meas[i].mu1[k],Sens_meas[i].I0[k], Sens_meas[i].is_free[k]);
    }*/
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    update_node_info( &(rrm->rrc.pSensEntry), &L2_id, NB_info, Sens_meas, info_time);
    //print_sens_db( rrm->rrc.pSensEntry   );//dbg
    //sleep (20);
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
    
    //mod_lor_10_05_28
    if (!(rrm->ip.waiting_SN_update) ){
        pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
        channel = rrm->rrc.pChannelsEntry;
        while (channel!=NULL){
            //printf("Channel!=NULL\n");
            if (channel->is_ass)
                if((send_up_to_SN = evalaute_sens_info(rrm->rrc.pSensEntry,channel->channel.Start_f,channel->channel.Final_f)))
                    break;
            channel = channel->next;
        }
        pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
        if (send_up_to_SN && rrm->role == FUSIONCENTER){
            return 1;
            //open_freq_query(inst, L2_id, 0, 0);
        }
    }
    return 0;
    
    
    //fprintf(stderr,"node entry  @%p \n", rrm->rrc.pSensEntry);//dbg
    //fprintf(stderr,"2 cluster_head\n");//dbg
    //AAA: for the moment the channel db is reserved for CHs and SUs only in SCEN_2_DISTR
    /*Sens_node_t *pn = rrm->rrc.pSensEntry;
    Sens_ch_t *pc;
    while (pn!=NULL){
        pc = pn->info_hd;
        while (pc!=NULL){
            for (int j=0; j<NUM_SB; j++)//dbg
                fprintf(stderr,"sns_update: channel %d sb %d is %d\n",pc->Ch_id, j, pc->is_free[j]);//dbg
            pc=pc->next;
        }
        pn=pn->next;
    }*/
    
  
    /*if ( rrm->role == FUSIONCENTER || SCEN_2_DISTR || rrm->role == CH_COLL ) //mod_lor_10_03_08: role instead of status -> to check
    //mod_lor_10_05_06 -> 2nd option of if changed (before SCEN_2_DISTR)
    {
        
        //fprintf(stderr,"cluster_head\n");//dbg
        CHANNEL_T channel ;
        CHANNELS_DB_T *canal;
        unsigned int is_free[MAX_NUM_SB];//mod_lor_10_05_28 ->char instead of int
        for (i=0;i<MAX_NUM_SB;i++)
            is_free[i]=0;
        int decision;
        unsigned int send_up_to_SN =0; //mod_lor_10_05_12
        for (i=0; i<NB_info; i++){
            
            channel.Start_f = Sens_meas[i].Start_f;
            channel.Final_f = Sens_meas[i].Final_f;
            channel.Ch_id   = Sens_meas[i].Ch_id;
            channel.QoS     = 0;
            printf(stdout,"Channel in msg %d : \n", channel.Ch_id); //dbg ou LOG
            for (decision = 0; decision<MAX_NUM_SB; decision++)
                fprintf(stdout,"SB %d : is %d\n", decision,Sens_meas[i].is_free[decision]); //dbg ou LOG
            */
            //mod_lor_10_03_19++
       /*     pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            take_decision(rrm->rrc.pSensEntry, channel.Ch_id,is_free);//mod_eure_lor
         //   pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
            //mod_lor_10_03_19--
            
            //mod_lor_10_05_07++
            if (decision>0)
                is_free = 1;
            else
                is_free = 0;*/
                
            
           
                //Sens_meas[i].meas = decision;
            
       /*     if(rrm->role == CH_COLL){
                memcpy(Sens_meas[i].is_free, is_free, MAX_NUM_SB*sizeof(unsigned int));//mod_lor_10_05_28 ->char instead of int
                //Sens_meas[i].meas = decision;
            }
            //mod_lor_10_05_07--
            
            fprintf(stdout,"Channel %d : \n", channel.Ch_id); //dbg ou LOG
            for (decision = 0; decision<MAX_NUM_SB; decision++)
                fprintf(stdout,"SB %d : is %d\n", decision,is_free[decision]); //dbg ou LOG
            */
         //   pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    /*        canal = up_chann_db( &(rrm->rrc.pChannelsEntry), channel, is_free[0], info_time);//TO DO: fix it!
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
            //mod_lor_10_05_12++
            if (!(rrm->ip.waiting_SN_update) && canal->is_ass && !(canal->is_free)){//mod_lor_10_05_18
                //fprintf(stderr,"send_up_to_SN =1\n");//dbg
                send_up_to_SN =1;//mod_lor_10_05_12--
            }
            //fprintf(stderr,"chann %d updated\n", Sens_meas[i].Ch_id);//dbg
            
        }
        //mod_lor_10_05_07++
        if(rrm->role == CH_COLL){
            pthread_mutex_lock( &( rrm->ip.exclu ) ) ;
            rrm->ip.trans_cnt++ ;
            PUT_IP_MSG(msg_up_clust_sens_results( inst, rrm->L2_id, NB_info, decision, Sens_meas, rrm->ip.trans_cnt));
            pthread_mutex_unlock( &( rrm->ip.exclu ) ) ;

        }//mod_lor_10_05_07--
        //mod_lor_10_05_12++
        if (send_up_to_SN && rrm->role == FUSIONCENTER){
            open_freq_query(inst, L2_id, 0, 0);
        }

    
    }else   
        fprintf(stderr,"error!!! Cannot update channels \n");*/

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
    
    // AAA TO DO: Confirmation sent via RRC to the fusion centre in case FC id != 0
    pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
    rrm->rrc.trans_cnt++ ;
    //fprintf(stderr, "before put RRM_end_scan_confirm\n");//dbg
    PUT_RRC_MSG(msg_rrm_end_scan_conf( inst, rrm->rrc.trans_cnt));
    //fprintf(stderr, "after put RRM_end_scan_confirm\n"); //dbg
    pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;

}
//mod_lor_10_04_14--

//mod_lor_10_05_10++
/*!
*******************************************************************************
\brief  Updating of the sensing measures received via IP from another node
*/
void up_coll_sens_results( 
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
    //AAA: for the moment the channel db is reserved for CHs and SUs only in SCEN_2_DISTR 
    
    
  
    if ( rrm->role == FUSIONCENTER || SCEN_2_DISTR || rrm->role == CH_COLL ) //mod_lor_10_03_08: role instead of status -> to check
    //mod_lor_10_05_06 -> 2nd option of if changed (before SCEN_2_DISTR)
    {
        
        //fprintf(stderr,"cluster_head\n");//dbg
        CHANNEL_T channel ;
        CHANNELS_DB_T *canal;
        unsigned int is_free[MAX_NUM_SB];//mod_lor_10_05_28 ->char instead of int
        for (i=0;i<MAX_NUM_SB;i++)
            is_free[i]=0;
        int decision;
        for (i=0; i<NB_info; i++){
            
            channel.Start_f = Sens_meas[i].Start_f;
            channel.Final_f = Sens_meas[i].Final_f;
            channel.Ch_id   = Sens_meas[i].Ch_id;
            channel.QoS     = 0;
            
            //mod_lor_10_03_19++
            /*pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            decision = take_decision(rrm->rrc.pSensEntry, channel.Ch_id);
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
            //mod_lor_10_03_19--
            
            //mod_lor_10_05_07++
            if (decision>0)
                is_free = 1;
            else
                is_free = 0;
                
            if(rrm->role == CH_COLL){
                Sens_meas[i].is_free = is_free;
                //Sens_meas[i].meas = decision;
            }*/
            //mod_lor_10_05_07--
            //mod_lor_10_03_19++
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            take_decision(rrm->rrc.pSensEntry, channel.Ch_id,is_free);//mod_eure_lor
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
            //mod_lor_10_03_19--
            
            //mod_lor_10_05_07++
            /*if (decision>0)
                is_free = 1;
            else
                is_free = 0;*/
                
            if(rrm->role == CH_COLL){
                memcpy(Sens_meas[i].is_free, is_free, MAX_NUM_SB*sizeof(unsigned int)); //mod_lor_10_05_28 ->char instead of int
                //Sens_meas[i].meas = decision;
            }
            
            fprintf(stdout,"Channel %d is %d:\n", channel.Ch_id,is_free[0]); //dbg ou LOG
            
            pthread_mutex_lock( &( rrm->rrc.exclu ) ) ;
            canal = up_chann_db( &(rrm->rrc.pChannelsEntry), channel, is_free[0], info_time);//TO DO: fix it!
            pthread_mutex_unlock( &( rrm->rrc.exclu ) ) ;
            //fprintf(stderr,"chann %d updated\n", Sens_meas[i].Ch_id);//dbg
            
        }

    
    }else   
        fprintf(stderr,"error!!! Cannot update channels \n");

}
//mod_lor_10_05_10--
