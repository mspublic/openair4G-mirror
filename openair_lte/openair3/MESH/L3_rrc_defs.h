/** @addtogroup _mesh_layer3_ 
  * @{ This page describes the interface between the RRC and RRM/RRCI modules for OpenAirInterface Mesh.
  */


#ifndef __L3_RRC_DEFS_H__
#define __L3_RRC_DEFS_H__

#define MAX_L3_INFO    16
#define LENGTH_L2_ID   8
#define IEEE_ADDR_LEN 6


#include "COMMON/mac_rrc_primitives.h"

/*!\brief Radio Bearer ID descriptor
 */
typedef unsigned short RB_ID;


typedef unsigned short QOS_CLASS_T;

#define LCHAN_BCCH		0
#define LCHAN_CCCH		1
#define LCHAN_DCCH		2
#define LCHAN_DTCH_B    	3
#define LCHAN_DTCH		4
#define LCHAN_MRBCH		5

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

/*!\brief Layer 3 Info types for RRC messages
 */
typedef enum {
  NONE_L3=0,        /*!< No information*/
  IPv4_ADDR,   /*!< IPv4 Address*/
  IPv6_ADDR,    /*!< IPv6 Address*/
  MAC_ADDR
} L3_INFO_T;

/*!\brief Layer 3 Info types for RRC messages
 */
typedef enum {
  BROADCAST=0,        /*!< Broadcast bearer*/
  UNICAST,            /*!< Unicast bearer*/
  MULTICAST           /*!< Multicast bearer*/
} RB_TYPE;

#endif //__L3_RRC_DEFS_H__
/** @} */
