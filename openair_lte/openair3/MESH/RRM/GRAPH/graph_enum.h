/*!
*******************************************************************************

\file       graph_enum.h

\brief      Fichier d'entete contenant les declarations des types, des defines ,
et des fonctions relatives aux messages RRC-RRM ou RRC-RRCI. 
            
Les fonctions servent à créer le buffer de message, remplir 
l'entete et copier les parametres de fonction. Chaque fonction 
retourne le message qui pourra être envoye sur le socket entre le 
CMM et le RRM ou RRCI.

\author     BURLOT Pascal

\date       17/07/08

\par     Historique:
L.IACOBELLI 2009-10-19
+ sensing messages 

*******************************************************************************
*/

#ifndef __GRAPH_ENUM_H
#define __GRAPH_ENUM_H

#define BTS_ID 1
#define FC_ID 0

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\brief  Enumeration des messages entre RRM/RRCI et le RRC
*/
typedef enum { 
SNS_UPDATE_SENS             , ///< Message SENSING->RRM : update of the sensing information measured by the nodes
RRM_SCAN_ORD                , ///< Message RRM->SENSING : order to scann indicated channels
RRM_END_SCAN_ORD            , ///< Message RRM->SENSING : end of a scanning process in sensors
SNS_END_SCAN_CONF           , ///< Message SENSING->RRC : end of a scanning process in sensors
NB_MSG_SNS_RRM                ///< Nombre de message RRM-SENSING
} MSG_SENSING_RRM_T ;


/*!
*******************************************************************************
\brief  Enumeration des messages entre RRM/RRCI et le RRC
*/
typedef enum { 
    RRM_RB_ESTABLISH_REQ = 0    , ///< Message RRM->RRC : requete d'etablissement d'un RB
    RRC_RB_ESTABLISH_RESP       , ///< Message RRC->RRM : reponse d'etablissement d'un RB
    RRC_RB_ESTABLISH_CFM        , ///< Message RRC->RRM : confirmation d'etablissement d'un RB
    RRM_RB_MODIFY_REQ           , ///< Message RRM->RRC : requete de modification d'un RB
    RRC_RB_MODIFY_RESP          , ///< Message RRC->RRM : reponse de modification d'un RB
    RRC_RB_MODIFY_CFM           , ///< Message RRC->RRM : confirmation de modification d'un RB  
    RRM_RB_RELEASE_REQ          , ///< Message RRM->RRC : requete de liberation d'un RB
    RRC_RB_RELEASE_RESP         , ///< Message RRC->RRM : reponse de liberation d'un RB
    RRC_MR_ATTACH_IND           , ///< Message RRC->RRM : indication d'attachement d'un MR
    RRM_SENSING_MEAS_REQ        , ///< Message RRM->RRC : requete de configuration de mesure sensing
    RRC_SENSING_MEAS_RESP       , ///< Message RRC->RRM : reponse de configuration de mesure sensing
    RRC_CX_ESTABLISH_IND        , ///< Message RRC->RRM : indication de connexion etablie
    RRC_PHY_SYNCH_TO_MR_IND     , ///< Message RRC->RRM : indication de synchronisation physique a un MR
    RRC_PHY_SYNCH_TO_CH_IND     , ///< Message RRC->RRM : indication de synchronisation physique a un CH
    RRCI_CX_ESTABLISH_RESP      , ///< Message RRCI->RRC : reponse de connexion etablie
    RRC_SENSING_MEAS_IND        , ///< Message RRC->RRM : indication de nouvel mesure de sensing 
    RRM_SENSING_MEAS_RESP       , ///< Message RRM->RRC : reponse a l'indication de nouvel mesure de sensing
    RRC_RB_MEAS_IND             , ///< Message RRC->RRM : indication de nouvel mesure sur un RB 
    RRM_RB_MEAS_RESP            , ///< Message RRM->RRC : reponse a l'indication de nouvel mesure sur un RB
    RRM_INIT_CH_REQ             , ///< Message RRM->RRC : init d'un CH
    RRCI_INIT_MR_REQ            , ///< Message RRM->RRC : init d'un MR
    RRM_INIT_MON_REQ            , ///< Message RRM->RRC : initiation of a scanning monitoring
    RRM_INIT_SCAN_REQ           , ///< Message RRM->RRC : initiation of a scanning process
    RRC_INIT_SCAN_REQ           , ///< Message RRC->RRM : initiation of a scanning process
    //UPDATE_SENS_RESULTS_3       , ///< Message IP       : update to send to CH/FC //mod_lor_10_01_25
    RRM_END_SCAN_REQ            , ///< Message RRM->RRC : end of a scanning process
    RRC_END_SCAN_REQ            , ///< Message RRC->RRM : end of a scanning process
    RRC_END_SCAN_CONF           , ///< Message RRC->RRM : end of a scanning process ack
    RRC_INIT_MON_REQ            , ///< Message IP       : initiation of a scanning monitoring
   // OPEN_FREQ_QUERY_4           , ///< Message RRM->RRC : BTS to ask free frequencies to FC
   // UPDATE_OPEN_FREQ_7          , ///< Message IP       : list of frequencies usable by the secondary network
    //UPDATE_SN_OCC_FREQ_5        , ///< Message IP       : BTS sends used freq. to FC
    RRM_UP_FREQ_ASS             , ///< Message RRM->RRC : BTS assigns channels to SUs
    RRM_END_SCAN_CONF           , ///< Message RRM->RRC : end of a scanning process
    NB_MSG_RRC_RRM                ///< Nombre de message RRM-RRC

} MSG_RRC_RRM_T ;

/*!
*******************************************************************************
\brief  Enumeration des messages entre le RRM/RRCI et le CMM
*/
typedef enum { 
    CMM_CX_SETUP_REQ = 0    , ///< Message CMM->RRM : requete de configuration d'une connexion
    RRM_CX_SETUP_CNF        , ///< Message RRM->CMM : confirmation de configuration d'une connexion 
    CMM_CX_MODIFY_REQ       , ///< Message CMM->RRM : requete de modification d'une connexion
    RRM_CX_MODIFY_CNF       , ///< Message RRM->CMM : confirmation de modification d'une connexion
    CMM_CX_RELEASE_REQ      , ///< Message CMM->RRM : requete de liberation d'une connexion
    RRM_CX_RELEASE_CNF      , ///< Message RRM->CMM : confirmation de liberation d'une connexion
    CMM_CX_RELEASE_ALL_REQ  , ///< Message CMM->RRM : requete de liberation de toutes les connexions
    RRM_CX_RELEASE_ALL_CNF  , ///< Message RRM->CMM : confirmation de liberation de toutes les connexions 
    RRCI_ATTACH_REQ         , ///< Message RRCI->CMM : requete de d'attachement d'un MR
    RRM_ATTACH_IND          , ///< Message RRM->CMM : indicateur d'attachement d'un MR
    CMM_ATTACH_CNF          , ///< Message CMM->RRM : confirmation d'attachement d'un MR
    RRM_MR_ATTACH_IND       , ///< Message RRM->CMM : indicateur d'attachement d'un MR
    ROUTER_IS_CH_IND        , ///< Message RRM->CMM : indicateur que le noeud est un cluster head
    RRCI_CH_SYNCH_IND       , ///< Message RRCI->CMM : indicateur que le noeud est synchronise sur un CH
    CMM_INIT_MR_REQ         , ///< Message CMM->RRM : requete d'initialisation de l'attachement d'un MR
    RRM_MR_SYNCH_IND        , ///< Message RRM->CMM : indicateur que le noeud est synchronise sur un MR
    RRM_NO_SYNCH_IND        , ///< Message RRM->CMM : indicateur que le noeud n'est pas synchronise sur un CH ou MR
    CMM_INIT_CH_REQ         , ///< Message CMM->RRM : requete d'initialisation d'un CH
    CMM_INIT_SENSING        , ///< Message CMM->RRM : requete d'initialisation du sensing
    CMM_STOP_SENSING        , ///< Message CMM->RRM : requete de stop du sensing
    CMM_ASK_FREQ            , ///< Message CMM->RRM : in BTS, message to start an open freq. query
    NB_MSG_CMM_RRM            ///< Nombre de message de l'interface
} MSG_CMM_RRM_T ;
 //mod_lor_10_04_27++
typedef enum { 
    UPDATE_SENS_RESULTS_3       , ///< Message IP       : update to send to CH/FC //mod_lor_10_01_25
    OPEN_FREQ_QUERY_4           , ///< Message IP       : BTS to ask free frequencies to FC
    UPDATE_OPEN_FREQ_7          , ///< Message IP       : list of frequencies usable by the secondary network
    UPDATE_SN_OCC_FREQ_5        , ///< Message IP       : BTS sends used freq. to FC
NB_MSG_IP                   ///< Nombre de message IP
} MSG_IP_T ;
 //mod_lor_10_04_27--

typedef enum {
    SNS=0,
    RRC,
    CMM,
    IP,  //mod_lor_10_04_27
    NB_INTERF
}INTERF_T;


#endif /* GRAPH_ENUM_H */
