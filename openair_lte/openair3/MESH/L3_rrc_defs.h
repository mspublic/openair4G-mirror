/** @addtogroup _mesh_layer3_ 
  * @{ This page describes the interface between the RRC and RRM/RRCI modules for OpenAirInterface Mesh.
  */


#ifndef __L3_RRC_DEFS_H__
#define __L3_RRC_DEFS_H__

#define MAX_L3_INFO    16
#define LENGTH_L2_ID   8
#define IEEE_ADDR_LEN 6


#include "COMMON/mac_rrc_primitives.h"

/*! \brief SENDORA scenario active: flags to set at the beginning of the simulation
*/
#define WSN            1  ///if wsn = 0 -> secondary network, else sensor network 
#define SCEN_1         1
#define SCEN_2_CENTR   0
#define SCEN_2_DISTR   0

//mod_lor_10_05_05++
/*!
*******************************************************************************
\brief Id of different CRRM entities in case of multiple entities on same machine       
  */
#define BTS_ID 1
#define FC_ID 0
#define CH_COLL_ID -1
//mod_lor_10_05_05--

/*! 
 *******************************************************************************
 \brief Parameters about channels:
 * 
*/
#define CH_NEEDED_FOR_SN 2 //!< Number of channels needed by secondary network//mod_lor_10_05_17
#define NB_SENS_MAX    20  //!< Maximum number of channels accepted by the system

/*! \brief Transaction ID descriptor
*/
typedef unsigned int Transaction_t;

/*! \brief Instance ID descriptor
 */
typedef unsigned char Instance_t;

/*!\brief Radio Bearer ID descriptor
 */
typedef unsigned short RB_ID;


typedef unsigned short QOS_CLASS_T;

#define LCHAN_BCCH      0
#define LCHAN_CCCH      1
#define LCHAN_DCCH      2
#define LCHAN_DTCH_B    3
#define LCHAN_DTCH      4
#define LCHAN_MRBCH     5

/*!\brief Layer 2 Identifier
 */

typedef struct {
  unsigned char L2_id[LENGTH_L2_ID];
} L2_ID;



/*!\brief Measurement Mode
 */
typedef enum {
  PERIODIC=0,   /*!< Periodic measurement*/
  EVENT_DRIVEN  /*!< Event-driven measurement*/
} MEAS_MODE;

/*!\brief Sensing measurement descriptor
 */
typedef struct {
  unsigned char RSSI_Threshold;    /*!< Threshold (minus in dBm) for neighbour RSSI measurement*/
  unsigned char RSSI_F_Factor;    /*!< Forgetting factor for RSSI averaging*/
  unsigned short Rep_interval;  /*!< \brief Reporting interval between successive measurement reports in this process*/
} SENSING_MEAS_DESC;

/*!\brief Sensing measurement information
 */
typedef struct {
  unsigned char Rssi;    /*!< RSSI (minus in dBm) for neighbour*/
  L2_ID L2_id;           /*!< Layer 2 ID for neighbour*/
} SENSING_MEAS_T;

/*!
*******************************************************************************
\brief Structure of sensing information database        
*/
typedef struct  Sens_ch_s { 
    unsigned int        Start_f    ; ///< frequence initial du canal //mod_lor_10_03_17: intxflot
    unsigned int        Final_f    ; ///< frequence final du canal   //mod_lor_10_03_17: intxflot
    unsigned int        Ch_id      ; ///< ID du canal
    float               meas       ; ///< Sensing results 
    unsigned int        is_free    ; ///< Decision about the channel
    struct  Sens_ch_s   *next      ; ///< pointeur sur le prochain canal 
} Sens_ch_t ;

/*!
*******************************************************************************
\brief Structure that describes the channels        
*/
typedef struct { 
    unsigned int        Start_f    ; ///< frequence initial du canal //mod_lor_10_03_17: intxflot
    unsigned int        Final_f    ; ///< frequence final du canal   //mod_lor_10_03_17: intxflot
    unsigned int        Ch_id      ; ///< ID du canal               //mod_lor_10_03_17: intxflot
    QOS_CLASS_T         QoS        ; ///< Max QoS possible on the channel
} CHANNEL_T ;

/*!\brief cooperation type between CHs
 */
typedef enum {
  NO_COOP     = 0, //!< No cooperation
  AMPL_FORW   = 1, //!< amplify and forward collaboration
  DECO_FORW   = 2, //!< decode and forward collaboration
}COOPERATION_T;
/*!
*******************************************************************************
\brief Structure that describes the channels        
*/
typedef struct { 
    //float               Start_f    ; ///< frequence initial du canal
    //float               Final_f    ; ///< frequence final du canal
    int                 Ch_id      ; ///< ID du canal
    //QOS_CLASS_T         QoS        ; ///< Max QoS possible on the channel
} MAC_INFO_T ;


/*!\brief Layer 3 Info types for RRC messages
 */
typedef enum {
  NONE_L3     = 0, //!< No information
  IPv4_ADDR   = 4, //!< IPv4 Address = size Info
  IPv6_ADDR   =16, //!< IPv6 Address = size Info
  MAC_ADDR    = 8  //!< MAC Id       = size Info
} L3_INFO_T;

/*!\brief Layer 3 Info types for RRC messages
 */
typedef enum {
  BROADCAST=0,        /*!< Broadcast bearer*/
  UNICAST,            /*!< Unicast bearer*/
  MULTICAST           /*!< Multicast bearer*/
} RB_TYPE;


/*!
*******************************************************************************
\brief Entete de la file des messages reçus ou a envoyer		
*/
typedef struct channels_db_s { 
	double               info_time ; ///< information age 
	unsigned int         is_free   ; ///< channel availability  
	unsigned int         priority  ; ///< channel priority
	unsigned int         is_ass    ; ///< channel used by secondary network
	L2_ID                source_id ; ///< SU using channel (source)
    L2_ID                dest_id   ; ///< SU using channel (dest)
	CHANNEL_T            channel   ; ///< channel description
	struct channels_db_s *next     ; ///< next node pointer
} CHANNELS_DB_T ;

#endif //__L3_RRC_DEFS_H__
/** @} */
