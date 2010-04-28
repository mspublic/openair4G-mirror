/*!
*******************************************************************************

\file       ip_msg.h

\brief      Fichier d'entete contenant les declarations des types, des defines ,
            et des fonctions relatives aux messages RRC-RRM ou RRC-RRCI. 
            
            Les fonctions servent à créer le buffer de message, remplir 
            l'entete et copier les parametres de fonction. Chaque fonction 
            retourne le message qui pourra être envoye sur le socket entre le 
            CMM et le RRM ou RRCI.

\author     IACOBELLI Lorenzo

\date       27/04/10

\par     Historique:
        

*******************************************************************************
*/
#ifndef __IP_MSG_H
#define __IP_MSG_H

#include "L3_rrc_defs.h"
#include "COMMON/mac_rrc_primitives.h"

#ifdef OPENAIR2_IN
#include "rrm_sock.h"
#else
#include "RRC/MESH/rrc_rrm_interface.h" 
#endif
//#include "../../../openair2/RRC/MESH/rrc_rrm_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\brief  Enumeration des messages entre RRM/RRCI et le RRC
*/
typedef enum { 
    UPDATE_SENS_RESULTS_3       , ///< Message IP       : update to send to CH/FC //mod_lor_10_01_25
    OPEN_FREQ_QUERY_4           , ///< Message IP       : BTS to ask free frequencies to FC
    UPDATE_OPEN_FREQ_7          , ///< Message IP       : list of frequencies usable by the secondary network
    UPDATE_SN_OCC_FREQ_5        , ///< Message IP       : BTS sends used freq. to FC
    /*RRC_ASK_FOR_FREQ            , ///< Message RRC->RRM : in FC/CH to report a frequency query
    RRM_OPEN_FREQ               , ///< Message RRM->RRC : FC communicates open frequencies 
    RRM_UPDATE_SN_FREQ          , ///< Message RRM->RRC : BTS sends used freq. to FC
    RRC_UPDATE_SN_FREQ          , ///< Message RRC->RRM : FC receives used freq. from BTS
    RRM_CLUST_SCAN_REQ          , ///< Message RRM->RRC : CH1 contacts CH2 to collaborate for the sensing process 
    RRC_CLUST_SCAN_REQ          , ///< Message RRC->RRM : CH2 receive request to collaborate for the sensing process from CH1
    RRM_CLUST_SCAN_CONF         , ///< Message RRM->RRC : CH2 confirmrs the beginning of a collaboration process
    RRM_CLUST_MON_REQ           , ///< Message RRM->RRC : CH1 contacts CH2 to collaborate for the monitoring process 
    RRC_CLUST_MON_REQ           , ///< Message RRC->RRM : CH2 receive request to collaborate for the monitoring process from CH1
    RRM_CLUST_MON_CONF          , ///< Message RRM->RRC : CH2 confirmrs the beginning of a monitoring process
    RRM_END_SCAN_CONF           , ///< Message RRM->RRC : CH2 confirmrs the end of the collaborative sensing process at CH1
    RRM_INIT_CONN_REQ           , ///< Message RRM->RRC : SU 1 requests a connection to SU 2
    RRC_INIT_CONN_CONF          , ///< Message RRM->RRC : Confirm from SU2 about the requested connection
    RRM_FREQ_ALL_PROP           , ///< Message RRM->RRC : SU1 sends a proposition of teh frequencies to use
    RRC_FREQ_ALL_PROP_CONF      , ///< Message RRC->RRM : SU1 receives a confirm on the usable channels
    RRM_REP_FREQ_ALL            , ///< Message RRM->RRC : SU1 reports to CH the possible channels
    RRC_REP_FREQ_ACK            , ///< Message RRC->RRM : CH informs all SUs about a channel allocated to 2 SUs
    RRC_INIT_CONN_REQ           , ///< Message RRC->RRM : Request from SU1 to establish a connection
    RRM_CONN_SET                , ///< Message RRC->RRM : Response to SU1 and connection settings communicated to rcc
    RRC_FREQ_ALL_PROP           , ///< Message RRC->RRM : SU1 proposition of frequencies to use
    RRM_FREQ_ALL_PROP_CONF      , ///< Message RRC->RRM : SU2 choise of frequencies to use
    RRC_REP_FREQ_ALL            , ///< Message RRM->RRC : SU1 reports to CH the possible channels
    RRM_REP_FREQ_ACK            , ///< Message RRC->RRM : CH informs all SUs about a channel allocated to 2 SUs
    */NB_MSG_IP                   ///< Nombre de message RRM-RRC

} MSG_IP_T ;


/*! 
*******************************************************************************
\brief  Definition des parametres de la fonction rrc_init_scan_req() dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
    L2_ID      L2_id              ; //!< Layer 2 (MAC) ID of Fusion Centre
    unsigned int     Start_fr;      //!< Lowest frequency considered (MHz)
    unsigned int     Stop_fr;       //!< Highest frequency considered (MHz)
    unsigned int     Meas_band;     //!< Measurement bandwidth (KHz)
    unsigned int     Meas_tpf;      //!< Measurement time per sub-band
    unsigned int     Nb_channels;   //!< Number of sub-bands
    unsigned int     Overlap;       //!< Overlap factor (%)
    unsigned int     Sampl_freq;    //!< Sampling frequency (Ms/s)
} init_coll_sens_req_t;  

/*! 
*******************************************************************************
\brief  Definition des parametres de les fonctions  
        rrm_update_sens()dans une structure permettant le passage 
        des parametres via un socket
*/
typedef struct {
    L2_ID               L2_id                  ; //!< Layer 2 ID (MAC) of sensing node
    unsigned int        NB_info                ; //!< Number of sensed channels
    Sens_ch_t           Sens_meas[NB_SENS_MAX] ; //!< Sensing information
    double              info_time              ; //!< Data of the information
} rrm_update_sens_t ; 


/*! 
*******************************************************************************
\brief  Definition des parametres de les fonctions rrm_ask_for_freq() et rrc_ask_for_freq()dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
    L2_ID      L2_id              ; //!< Layer 2 (MAC) ID of Fusion Centre
    QOS_CLASS_T      QoS          ; //!< QoS required; if 0 all QoS at disposition
} open_freq_query_t;

/*! 
*******************************************************************************
\brief  Definition des parametres de les fonctions  
        rrc_open_freq() et rrm_open_freq() dans une structure permettant le passage 
        des parametres via un socket
*/
typedef struct {
    double              date;
    L2_ID               L2_id                  ; //!< Layer 2 ID (MAC) of FC/CH
    unsigned int        NB_chan                ; //!< number of free channels
    CHANNEL_T           fr_channels[NB_SENS_MAX]; //!< description of free channelS
} update_open_freq_t;

/*! 
*******************************************************************************
\brief  Definition des parametres des fonctions rrm_update_SN_freq() et 
        rrm_update_SN_freq()dans 
        une structure permettant le passage des parametres via un socket
*/
typedef struct {
    L2_ID               L2_id                ; //!< Layer 2 (MAC) ID of FC/BTS
    unsigned int        NB_chan              ; //!< Number of channels 
    unsigned int        occ_channels[NB_SENS_MAX]; //!< Vector of channels
} update_SN_occ_freq_t;

#ifdef TRACE
extern const char *Str_msg_ip[NB_MSG_IP] ; 
#endif

/* Rappel : les paramètres sont identiques aux fonctions 
 *          (sans le prefixe msg_ ) du fichier: L3_rrc_interface.h
 */          

             
///MESSAGES VIA IP
msg_t *msg_update_sens_results_3( Instance_t inst, L2_ID L2_id, unsigned int NB_chan, Sens_ch_t *Sens_meas, Transaction_t Trans_id ); 
msg_t *msg_open_freq_query_4( Instance_t inst, L2_ID L2_id, QOS_CLASS_T QoS, Transaction_t Trans_id );
msg_t *msg_update_open_freq_7( Instance_t inst, L2_ID L2_id, unsigned int NB_free_ch, CHANNEL_T *fr_channels, Transaction_t Trans_id);
msg_t *msg_update_SN_occ_freq_5( Instance_t inst, L2_ID L2_id, unsigned int NB_chan, unsigned int *occ_channels, Transaction_t Trans_id);


#ifdef __cplusplus
}
#endif

#endif /* RRC_MSG_H */
