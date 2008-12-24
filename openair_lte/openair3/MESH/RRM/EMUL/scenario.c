/*!
*******************************************************************************

\file       scenario.c

\brief      Emulation d'un scenario de test  sur les interfaces du RRM 

            Cette application d'envoyer des stimuli sur les interfaces RRM:
                - RRC -> RRM
                - CMM -> RRM

\author     BURLOT Pascal

\date       10/07/08

   
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
#include <ctype.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <pthread.h>
#include <time.h>

#include "debug.h"

#include "L3_rrc_defs.h"
#include "cmm_rrm_interface.h"

#include "rrm_sock.h"
#include "cmm_msg.h"
#include "rrc_rrm_msg.h"
#include "pusu_msg.h"

#include "transact.h"
#include "actdiff.h"
#include "rrm_util.h"
#include "rrm_constant.h"
#include "emul_interface.h"

extern msg_t *msg_rrc_rb_meas_ind(Instance_t inst, RB_ID Rb_id, L2_ID L2_id, MEAS_MODE Meas_mode, MAC_RLC_MEAS_T *Mac_rlc_meas_t, Transaction_t Trans_id );
extern msg_t *msg_rrc_sensing_meas_ind( Instance_t inst, L2_ID L2_id, unsigned int NB_meas, SENSING_MEAS_T *Sensing_meas, Transaction_t Trans_id );
extern msg_t *msg_rrc_sensing_meas_resp( Instance_t inst, Transaction_t Trans_id )  ;
extern msg_t *msg_rrc_cx_establish_ind( Instance_t inst, L2_ID L2_id, Transaction_t Trans_id,unsigned char *L3_info, L3_INFO_T L3_info_t,
                                    RB_ID DTCH_B_id, RB_ID DTCH_id );
extern msg_t *msg_rrc_phy_synch_to_MR_ind( Instance_t inst, L2_ID L2_id);
extern msg_t *msg_rrc_phy_synch_to_CH_ind( Instance_t inst, unsigned int Ch_index,L2_ID L2_id );
extern msg_t *msg_rrc_rb_establish_resp( Instance_t inst, Transaction_t Trans_id  );
extern msg_t *msg_rrc_rb_establish_cfm( Instance_t inst, RB_ID Rb_id, RB_TYPE RB_type, Transaction_t Trans_id );
extern msg_t *msg_rrc_rb_modify_resp( Instance_t inst, Transaction_t Trans_id );
extern msg_t *msg_rrc_rb_modify_cfm(Instance_t inst, RB_ID Rb_id, Transaction_t Trans_id  );
extern msg_t *msg_rrc_rb_release_resp( Instance_t inst, Transaction_t Trans_id );
extern msg_t *msg_rrc_MR_attach_ind( Instance_t inst, L2_ID L2_id );


L2_ID L2_id_ch  ={{0xAA,0xCC,0x33,0x55,0x00,0x11,0x00,0x00}};
L2_ID L2_id_mr  ={{0xAA,0xCC,0x33,0x55,0x00,0x00,0x22,0x00}};
L2_ID L2_id_mr2 ={{0xAA,0xCC,0x33,0x55,0x00,0x00,0x33,0x00}};
L2_ID L2_id_mr3 ={{0xAA,0xCC,0x33,0x55,0x00,0x00,0x44,0x00}};

unsigned char L3_info_mr3[MAX_L3_INFO] = { 0xBB, 0xDD, 0, 1, 0, 0, 0, 0, 0xFF, 3, 3, 3, 0, 0, 0, 1 } ; 
unsigned char L3_info_mr2[MAX_L3_INFO] = { 0xBB, 0xDD, 0, 1, 0, 0, 0, 0, 0xFF, 2, 2, 2, 0, 0, 0, 1 } ; 
unsigned char L3_info_mr[MAX_L3_INFO]  = { 0xBB, 0xDD, 0, 1, 0, 0, 0, 0, 0xFF, 1, 1, 1, 0, 0, 0, 1 } ; 
unsigned char L3_info_ch[MAX_L3_INFO]  = { 0xAA, 0xCC, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0XFF, 0, 2 } ; 


static void prg_opening_RB( sock_rrm_t *s_cmm, double date, L2_ID *src, L2_ID *dst, QOS_CLASS_T qos )
{
    cmm_transaction++;
    pthread_mutex_lock( &actdiff_exclu  ) ;
    add_actdiff(&list_actdiff,date, cnt_actdiff++, s_cmm,
                msg_cmm_cx_setup_req(0,*src,*dst, qos, cmm_transaction ) ) ;
    pthread_mutex_unlock( &actdiff_exclu ) ;
                
    pthread_mutex_lock( &cmm_transact_exclu ) ;
    add_item_transact( &cmm_transact_list, cmm_transaction, INT_CMM,CMM_CX_SETUP_REQ,0,NO_PARENT);
    pthread_mutex_unlock( &cmm_transact_exclu ) ;
}

static void prg_modifying_RB( sock_rrm_t *s_cmm, double date, RB_ID Rb_id, QOS_CLASS_T qos )
{
    cmm_transaction++;
    pthread_mutex_lock( &actdiff_exclu  ) ;
    add_actdiff(&list_actdiff,date, cnt_actdiff++, s_cmm,
                msg_cmm_cx_modify_req(0,Rb_id, qos, cmm_transaction ) ) ;
    pthread_mutex_unlock( &actdiff_exclu ) ;
                
    pthread_mutex_lock( &cmm_transact_exclu ) ;
    add_item_transact( &cmm_transact_list, cmm_transaction, INT_CMM,CMM_CX_MODIFY_REQ,0,NO_PARENT);
    pthread_mutex_unlock( &cmm_transact_exclu ) ;
}

static void prg_releasing_RB( sock_rrm_t *s_cmm, double date, RB_ID Rb_id )
{
    cmm_transaction++;
    pthread_mutex_lock( &actdiff_exclu  ) ;
    add_actdiff(&list_actdiff,date, cnt_actdiff++, s_cmm,
                msg_cmm_cx_release_req(0,Rb_id, cmm_transaction ) ) ;
    pthread_mutex_unlock( &actdiff_exclu ) ;
                
    pthread_mutex_lock( &cmm_transact_exclu ) ;
    add_item_transact( &cmm_transact_list, cmm_transaction, INT_CMM,CMM_CX_RELEASE_REQ,0,NO_PARENT);
    pthread_mutex_unlock( &cmm_transact_exclu ) ;
}

static void prg_phy_synch_to_MR( sock_rrm_t *s_rrc, double date )
{
    rrc_transaction++;
    pthread_mutex_lock( &actdiff_exclu  ) ;
    add_actdiff(&list_actdiff,date, cnt_actdiff++, s_rrc, msg_rrc_phy_synch_to_MR_ind(0, L2_id_ch) ) ;
    pthread_mutex_unlock( &actdiff_exclu ) ;
                
    pthread_mutex_lock( &rrc_transact_exclu ) ;
    add_item_transact( &rrc_transact_list, rrc_transaction, INT_RRC,RRC_PHY_SYNCH_TO_MR_IND,0,NO_PARENT);
    pthread_mutex_unlock( &rrc_transact_exclu ) ;
}

static void prg_rrc_MR_attach_ind( sock_rrm_t *s_rrc, double date, L2_ID *L2_id_mr  )
{
    rrc_transaction++;
    pthread_mutex_lock( &actdiff_exclu  ) ;
    add_actdiff(&list_actdiff,date, cnt_actdiff++, s_rrc,
                msg_rrc_MR_attach_ind(0,*L2_id_mr ) ) ;
    pthread_mutex_unlock( &actdiff_exclu ) ;
                
    pthread_mutex_lock( &rrc_transact_exclu ) ;
    add_item_transact( &rrc_transact_list, rrc_transaction, INT_RRC,RRC_MR_ATTACH_IND,0,NO_PARENT);
    pthread_mutex_unlock( &rrc_transact_exclu ) ;
}

static void prg_rrc_cx_establish_ind( 
        sock_rrm_t *s_rrc, double date, 
        L2_ID *L2_id,
        unsigned char *L3_info,
        L3_INFO_T L3_info_t,
        RB_ID dtch_b_id,
        RB_ID dtch_id  
        )
{
    rrc_transaction++;
    pthread_mutex_lock( &actdiff_exclu  ) ;
    add_actdiff(&list_actdiff,date, cnt_actdiff++, s_rrc,
                msg_rrc_cx_establish_ind(0,*L2_id,rrc_transaction, L3_info, L3_info_t, dtch_b_id, dtch_id ) ) ;
    pthread_mutex_unlock( &actdiff_exclu ) ;
                
    pthread_mutex_lock( &rrc_transact_exclu ) ;
    add_item_transact( &rrc_transact_list, rrc_transaction, INT_RRC,RRC_CX_ESTABLISH_IND,0,NO_PARENT);
    pthread_mutex_unlock( &rrc_transact_exclu ) ;
}

static void prg_rrc_sensing_meas_ind( sock_rrm_t *s_rrc, double date, L2_ID *L2_id_mr,
                                    int nb_meas, SENSING_MEAS_T *Sensing_meas )
{
    rrc_transaction++;
    pthread_mutex_lock( &actdiff_exclu  ) ;
    add_actdiff(&list_actdiff,date, cnt_actdiff++, s_rrc,
                msg_rrc_sensing_meas_ind(0,*L2_id_mr,nb_meas,Sensing_meas, rrc_transaction ) ) ;
    pthread_mutex_unlock( &actdiff_exclu ) ;
                
    pthread_mutex_lock( &rrc_transact_exclu ) ;
    add_item_transact( &rrc_transact_list, rrc_transaction, INT_RRC,RRC_SENSING_MEAS_IND,0,NO_PARENT);
    pthread_mutex_unlock( &rrc_transact_exclu ) ;
}

static void prg_rrc_rb_meas_ind( sock_rrm_t *s_rrc, double date, RB_ID Rb_id, L2_ID *L2_id,MEAS_MODE Meas_mode,
                                    MAC_RLC_MEAS_T *Mac_rlc_meas )
{
    rrc_transaction++;
    pthread_mutex_lock( &actdiff_exclu  ) ;
    add_actdiff(&list_actdiff,date, cnt_actdiff++, s_rrc,
                msg_rrc_rb_meas_ind(0, Rb_id, *L2_id, Meas_mode, Mac_rlc_meas,  rrc_transaction ) ) ;
    pthread_mutex_unlock( &actdiff_exclu ) ;
                
    pthread_mutex_lock( &rrc_transact_exclu ) ;
    add_item_transact( &rrc_transact_list, rrc_transaction, INT_RRC,RRC_RB_MEAS_IND,0,NO_PARENT);
    pthread_mutex_unlock( &rrc_transact_exclu ) ;
}   


/* =========================================================================== *
 *                              SCENARII                                       *
 * =========================================================================== */

/**
 * \brief Cette fonction simule le passage de IN en CH (TIMEOUT) ,
 *        ensuite  l'ouverture d'un RB, la modification et finalement la
 *        libération.
 */
static void scenario0(sock_rrm_t *s_rrc,  sock_rrm_t *s_cmm )
{
    printf("\nSCENARIO 0: ...\n\n" ) ;
// ========================= ISOLATED NODE to CLUSTERHEAD 
    prg_phy_synch_to_MR( s_rrc, 0.1 );

// ========================= Ouverture d'un RB
    prg_opening_RB( s_cmm, 2.0, &L2_id_ch,&L2_id_mr,QOS_DTCH_USER1 );
// ========================= Modification d'un RB
    prg_modifying_RB( s_cmm, 2.1 , 7, QOS_DTCH_USER2 );
// ========================= Fermeture d'un RB
    prg_releasing_RB( s_cmm, 2.5, 7 );
}

/**
 * \brief Cette fonction simule le passage de IN en CH par la reception du 
 *        SYNCH d'un MR, et ensuite  l'ouverture d'un RB
 */
static void scenario1(sock_rrm_t *s_rrc,  sock_rrm_t *s_cmm )
{
    printf("\nSCENARIO 1: ...\n\n" ) ;
// ========================= ISOLATED NODE to CLUSTERHEAD : RRC_PHY_SYNCH_TO_MR_IND
    prg_phy_synch_to_MR( s_rrc, 0.2 );

// ========================= Ouverture d'un RB
    prg_opening_RB( s_cmm, 2.0, &L2_id_ch,&L2_id_mr,QOS_DTCH_USER1 );
}

/**
 * \brief Cette fonction simule le passage de IN en CH ,
 *        puis l'attachement d'un MR et finalement l'ouverture d'un RB.
 */
static void scenario2(sock_rrm_t *s_rrc,  sock_rrm_t *s_cmm)
{
    printf("\nSCENARIO 2: ...\n\n" ) ;
// ========================= ISOLATED NODE to CLUSTERHEAD 
    prg_phy_synch_to_MR( s_rrc, 0.1 );

// ========================= Attachement d'un MR
    prg_rrc_MR_attach_ind( s_rrc, 2.0, &L2_id_mr );

// ========================= Indicateur d'une connexion établie
    prg_rrc_cx_establish_ind( s_rrc, 5.0, &L2_id_mr, L3_info_mr,IPv6_ADDR,0,0);
}

/**
 * \brief Cette fonction simule le passage de IN en MR par la reception du 
 *        SYNCH d'un CH
 */
static void scenario3(sock_rrm_t *s_rrc,  sock_rrm_t *s_cmm)
{
    printf("\nSCENARIO 3: ...\n\n" ) ;

// ========================= Attachement d'un MR

    rrc_transaction++;
    pthread_mutex_lock( &actdiff_exclu  ) ;
    add_actdiff(&list_actdiff,0.2, cnt_actdiff++, s_rrc,msg_rrc_phy_synch_to_CH_ind(0, 1, L2_id_mr ) ) ;
    pthread_mutex_unlock( &actdiff_exclu ) ;
                
    pthread_mutex_lock( &rrc_transact_exclu ) ;
    add_item_transact( &rrc_transact_list, rrc_transaction, INT_RRC,RRC_PHY_SYNCH_TO_CH_IND,0,NO_PARENT);
    pthread_mutex_unlock( &rrc_transact_exclu ) ;

// ========================= Connexion etablit du MR au CH
    prg_rrc_cx_establish_ind( s_rrc, 1.0, &L2_id_ch, L3_info_ch,IPv6_ADDR, 10, 20 ) ;
}

/**
 * \brief Cette fonction simule le passage de IN en CH (TIMEOUT),
 *        puis l'attachement d'un MR ,
 *        puis l'ouverture d'un RB.
 *        puis la remontee de mesures du MR au CH.
 */
static void scenario4(sock_rrm_t *s_rrc,  sock_rrm_t *s_cmm)
{
    SENSING_MEAS_T Sensing_meas[3]={
        { 15, {{0xAA,0xCC,0x33,0x55,0x11,0x00,0x22,0x00}} },
        { 20, {{0xAA,0xCC,0x33,0x55,0x22,0x00,0x22,0x00}} },
        { 10, {{0xAA,0xCC,0x33,0x55,0x33,0x00,0x22,0x00}} }
    };
    printf("\nSCENARIO 4: ...\n\n" ) ;
// ========================= ISOLATED NODE to CLUSTERHEAD 
    prg_phy_synch_to_MR( s_rrc, 0.1 );

// ========================= Attachement d'un MR
    prg_rrc_MR_attach_ind( s_rrc, 2.0 , &L2_id_mr  ) ;

// ========================= Indicateur d'une connexion établie
    prg_rrc_cx_establish_ind( s_rrc, 5.0, &L2_id_mr, L3_info_mr, IPv6_ADDR, 0, 0 ) ;

// ========================= Remontée de mesure par le RRC

    // Meas 1
    prg_rrc_sensing_meas_ind( s_rrc, 5.00, &L2_id_mr, 1, Sensing_meas );
    
    // Meas 2
    prg_rrc_sensing_meas_ind( s_rrc, 5.25, &L2_id_mr, 3, Sensing_meas );
    
    // Meas 3
    prg_rrc_sensing_meas_ind( s_rrc, 5.50, &L2_id_mr, 2, Sensing_meas );
    
    // Meas 4
    prg_rrc_sensing_meas_ind( s_rrc, 5.75, &L2_id_mr, 0, Sensing_meas );
}

/**
 * \brief Cette fonction simule le passage de IN en CH (TIMEOUT),
 *        puis l'attachement de 3 MR ,
 *        puis l'ouverture d'un RB.
 *        puis la remontee de mesures du MR au CH.
 */
static void scenario5(sock_rrm_t *s_rrc,  sock_rrm_t *s_cmm)
{
    static SENSING_MEAS_T Sensing_meas[3]={
        { 15, {{0xAA,0xCC,0x33,0x55,0x00,0x11,0x00,0x00}} },
        { 20, {{0xAA,0xCC,0x33,0x55,0x00,0x00,0x44,0x00}} },
        { 10, {{0xAA,0xCC,0x33,0x55,0x00,0x00,0x33,0x00}} }
    };
    static SENSING_MEAS_T Sensing_meas2[3]={
        { 16, {{0xAA,0xCC,0x33,0x55,0x00,0x11,0x00,0x00}} },
        { 25, {{0xAA,0xCC,0x33,0x55,0x00,0x00,0x22,0x00}} },
        { 30, {{0xAA,0xCC,0x33,0x55,0x00,0x00,0x44,0x00}} }
    };
    static SENSING_MEAS_T Sensing_meas3[3]={
        { 14, {{0xAA,0xCC,0x33,0x55,0x00,0x11,0x00,0x00}} },
        { 17, {{0xAA,0xCC,0x33,0x55,0x00,0x00,0x22,0x00}} },
        { 29, {{0xAA,0xCC,0x33,0x55,0x00,0x00,0x33,0x00}} }
    };
    
    printf("\nSCENARIO 5: ...\n\n" ) ;
// ========================= ISOLATED NODE to CLUSTERHEAD :
    prg_phy_synch_to_MR( s_rrc, 0.1 );

// ========================= Attachement d'un MR
    prg_rrc_MR_attach_ind( s_rrc, 2.0 , &L2_id_mr  ) ;
    prg_rrc_MR_attach_ind( s_rrc, 2.0 , &L2_id_mr2  ) ;
    prg_rrc_MR_attach_ind( s_rrc, 2.0 , &L2_id_mr3  ) ;

// =========================  Indicateur d'une connexion établie
    prg_rrc_cx_establish_ind( s_rrc, 5.0, &L2_id_mr, L3_info_mr, IPv6_ADDR, 0, 0 ) ;
    prg_rrc_cx_establish_ind( s_rrc, 5.0, &L2_id_mr2, L3_info_mr, IPv6_ADDR, 0, 0 ) ;
    prg_rrc_cx_establish_ind( s_rrc, 5.0, &L2_id_mr3, L3_info_mr, IPv6_ADDR, 0, 0 ) ;

// ========================= Remontée de mesure par le RRC

    // Meas 1
    prg_rrc_sensing_meas_ind( s_rrc, 5.10, &L2_id_mr, 1, Sensing_meas );
    prg_rrc_sensing_meas_ind( s_rrc, 5.10, &L2_id_mr2, 1, Sensing_meas2 );
    prg_rrc_sensing_meas_ind( s_rrc, 5.10, &L2_id_mr3, 1, Sensing_meas3 );
    
    // Meas 2
    prg_rrc_sensing_meas_ind( s_rrc, 5.25, &L2_id_mr, 2, Sensing_meas );
    prg_rrc_sensing_meas_ind( s_rrc, 5.25, &L2_id_mr2, 2, Sensing_meas2 );
    prg_rrc_sensing_meas_ind( s_rrc, 5.25, &L2_id_mr3, 2, Sensing_meas3 );
    
    // Meas 3
    prg_rrc_sensing_meas_ind( s_rrc, 5.50, &L2_id_mr, 3, Sensing_meas );
    prg_rrc_sensing_meas_ind( s_rrc, 5.50, &L2_id_mr2, 3, Sensing_meas2 );
    prg_rrc_sensing_meas_ind( s_rrc, 5.50, &L2_id_mr3, 3, Sensing_meas3 );
    
}

/**
 * \brief Cette fonction simule le passage de IN en CH (TIMEOUT) ,
 *        ensuite  l'ouverture d'un RB, la modification et finalement la
 *        libération.
 */
static void scenario6(sock_rrm_t *s_rrc,  sock_rrm_t *s_cmm )
{
    
    static MAC_RLC_MEAS_T Meas1_CH= { .Rssi=25 , .Sinr={ 1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15,16}, .Harq_delay =45,
                              .Bler=1234,  .rlc_sdu_buffer_occ=13,.rlc_sdu_loss_indicator=25000};
    static MAC_RLC_MEAS_T Meas2_CH= { .Rssi=15 , .Sinr={ 1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15,16}, .Harq_delay =35,
                              .Bler=4,  .rlc_sdu_buffer_occ=13,.rlc_sdu_loss_indicator=35000};
                              
    static MAC_RLC_MEAS_T Meas1_MR= { .Rssi=35 , .Sinr={ 11,12,13,14, 15,16,17,18, 19,20,21,22, 23,24,25,26}, .Harq_delay =55,
                              .Bler=2134,  .rlc_sdu_buffer_occ=23,.rlc_sdu_loss_indicator=15000};

    static MAC_RLC_MEAS_T Meas2_MR= { .Rssi=45 , .Sinr={ 11,12,13,14, 15,16,17,18, 19,20,21,22, 23,24,25,26}, .Harq_delay =25,
                              .Bler=3000,  .rlc_sdu_buffer_occ=11,.rlc_sdu_loss_indicator=300};
    printf("\nSCENARIO 6: ...\n\n" ) ;
// ========================= ISOLATED NODE to CLUSTERHEAD 
    prg_phy_synch_to_MR( s_rrc, 0.1 );

// ========================= Ouverture d'un RB
    prg_opening_RB( s_cmm, 2.0, &L2_id_ch,&L2_id_mr,QOS_DTCH_USER1 );
    

    prg_rrc_rb_meas_ind( s_rrc, 2.5, 4 , &L2_id_mr,PERIODIC,&Meas1_MR )  ;
    prg_rrc_rb_meas_ind( s_rrc, 2.5, 4 , &L2_id_ch,PERIODIC,&Meas1_CH )  ;

    prg_rrc_rb_meas_ind( s_rrc, 2.7, 4 , &L2_id_ch,PERIODIC,&Meas2_CH )  ;
    prg_rrc_rb_meas_ind( s_rrc, 2.7, 4 , &L2_id_mr,PERIODIC,&Meas2_MR )  ;

}

void scenario(int num , sock_rrm_t *s_rrc,  sock_rrm_t *s_cmm )
{
    switch ( num )
    {
        case 0 : scenario0(s_rrc,  s_cmm ) ; break ;
        case 1 : scenario1(s_rrc,  s_cmm ) ; break ;
        case 2 : scenario2(s_rrc,  s_cmm ) ; break ;
        case 3 : scenario3(s_rrc,  s_cmm ) ; break ;
        case 4 : scenario4(s_rrc,  s_cmm ) ; break ;
        case 5 : scenario5(s_rrc,  s_cmm ) ; break ;
        case 6 : scenario6(s_rrc,  s_cmm ) ; break ;
        default:
            fprintf( stderr,"Erreur : '%d' => Numero de test inconnu\n" , num ) ;
    }
}
