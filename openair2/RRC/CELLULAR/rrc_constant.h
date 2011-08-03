/***************************************************************************
                          rrc_constant.h  -  description
                             -------------------
    begin             : Someday 2001
    copyright         : (C) 2001, 2008 by Eurecom
	  created by			  : Lionel.Gauthier@eurecom.fr	
    modified by       : Michelle.Wetterwald@eurecom.fr
 **************************************************************************
  RRC protocol constants
 ***************************************************************************/
#ifndef __RRC_CONSTANT_H__
#define __RRC_CONSTANT_H__

/* ***Debug flags*** */
#define DEBUG_RRC_DETAILS

#define DEBUG_RRC_STATE
//#define DEBUG_RRC_BROADCAST_NAS
//#ifndef DEBUG_RRC_BROADCAST
//#define DEBUG_RRC_BROADCAST
//#endif

#define DEBUG_NAS_CONTROL_SIMU

// Measurement Control Message
//#define DEBUG_RRC_MEASURE_CONTROL
// Measurement Report Message
#ifdef NODE_RG
//#define DEBUG_RRC_MEASURE_REPORT
//#define DEBUG_RRC_FORWARD_MT_MEASURE_REPORT
#endif

//FLAGS DEFINED FOR MBMS DEFINITION
//#define ALLOW_MBMS_ACCESS_INFO
#define ALLOW_MBMS_PROTOCOL  //Activate or deactivate the MBMS Protocol

//#define MBMS_TEST_MODE	//Used in stand-alone mode (MBMS simulator)
#define MBMS_INTEGRATION_MODE	//Used in platform mode - simulation or realtime mode

/* *** Debug flags for MBMS *** */
#define DEBUG_RRC_MBMS_BASIC      //basic level of trace for MBMS operation
//#define DEBUG_RRC_MBMS            //1st level of trace for MBMS operation
//#define DEBUG_RRC_MBMS_DETAIL   //2nd level of trace for MBMS operation
//#define DEBUG_UE_MBMS_FSM       //??
//#define DEBUG_UE_MBMS_FSM_TEST  //??
//#define DEBUG_RRC_MBMS_MSG_CONTENT  // content fo received/sent message buffer
//#define DEBUG_RRC_MBMS_SFN  // MBMS frame counter
//#define DEBUG_RRC_MBMS_STATUS  // current status of services in UE

#ifdef MBMS_INTEGRATION_MODE
#undef MBMS_TEST_MODE
#endif
/* ***End Debug flags*** */

//max number of UE per BS
// Warning : This must be coordinated with the number of FIFOs in the RG (rrc_sap.h)
#define maxUsers 3
// max number of cells or RGs
// to be coordinated with NAS list of RGs
#define maxCells 2

//Values to be used in UE for quality measurement - to be adjusted
//Formula : NAS_measure = (RSSI0 / RATIO_RSSI0) + SHIFT_RSSI0
//Result must be a positive integer [0-90]
#define RATIO_RSSI0 100
#define SHIFT_RSSI0 110
//#define NB_ANTENNAS 2   // A Revoir
// #define numANTENNAS   NB_ANTENNAS

#ifdef NODE_RG
#define RRC_RG_MAX_RB   maxRB*maxUsers
// RG_CONN_TIMEOUT will clean the MT resources from the RG if it failed
// between CONN_SETUP and CONN_SETUP_COMPLETE - 3000 ==> 30s.
#define RG_CONN_TIMEOUT 3000
#endif

/* CONSTANTS for ESTEREL */
#define SUCCESS 0
#define FAILURE 1
#define ALREADY_C 2
//
#define FALSE 0
#ifndef TRUE
#define TRUE 1
#endif

// Actions for computing the configuration
#define E_ADD_MT 1
#define E_REL_MT 3
#define E_ADD_RB 5
#define E_REL_RB 7

// UE States
#define IDLE      RRC_UE_IDLE                //0x01
#define CONNECTED RRC_UE_CONNECTED           //0x02
#define CELL_FACH RRC_UE_CONNECTED_CELL_FACH //0x06
#define CELL_DCH  RRC_UE_CONNECTED_CELL_DCH  //0x0A
#define CELL_PCH  16+CONNECTED
// RG States
#define Conn_Received        0x10
#define CELL_FACH_Connecting 0x14
#define CELL_DCH_Connecting  0x18
#define CELL_FACH_Connected  RRC_RG_CONNECTED_CELL_FACH  //0x06
#define CELL_DCH_Connected   RRC_RG_CONNECTED_CELL_DCH   //0x0A
#define CELL_FACH_Releasing  0x24
#define CELL_DCH_Releasing   0x28

// RRC Messages Types (sent by)-- ESTEREL values
#define RRC_CONN_REQ            11 //UE
#define RRC_CONN_SETUP          12 //RG
#define RRC_CONN_SETUP_COMPLETE 13 //UE
#define RRC_CONN_REJECT         14 //RG
#define RRC_CONN_RELEASE        15 //RG - UE
#define RRC_CONN_REL_COMPLETE   16 //UE
#define RB_SETUP                17 //RG
#define RB_SETUP_COMPLETE       18 //UE
#define RB_SETUP_FAILURE        19 //UE
#define RB_RELEASE              21 //RG
#define RB_RELEASE_COMPLETE     22 //UE
#define RB_RELEASE_FAILURE      23 //UE
#define CELL_UPDATE             25 //UE
#define CELL_UPDATE_CONFIRM     26 //RG
//HNN : Added on 04/06 for UE Capability Information
#define UE_CAPABILITY_INFO      27 //UE
#define UE_CAPABILITY_INFO_CONF 28 //RG

//
#define REL_COUNT 3         // Nb RESEND RRC_CONNECTION_RELEASE by RG
//#define N300 5000  // NB RETRY RRC_CONNECTION_REQUEST
#define N300 500000         // L1 TEST
#define N308 2              // NB RETRY RRC_CONNECTION_RELEASE_REQUEST


#define T300_DURATION 2000  //in millisec

// Timers ID
#define RRC_T300 1

// protocol state
#define  RRC_UE_IDLE                   0x01
#define  RRC_UE_CONNECTED              0x02
#define  RRC_UE_CONNECTED_CELL_FACH    0x04 + RRC_UE_CONNECTED
#define  RRC_UE_CONNECTED_CELL_DCH     0x08 + RRC_UE_CONNECTED
#define  RRC_UE_REQUESTING             0x10

#define  RRC_RG_IDLE                   0x01
#define  RRC_RG_CONNECTED              0x02
#define  RRC_RG_CONNECTED_CELL_FACH    0x04 + RRC_RG_CONNECTED
#define  RRC_RG_CONNECTED_CELL_DCH     0x08 + RRC_RG_CONNECTED

// protocol events
#define  RRC_CONNECTION_REQUEST_EVENT           0x01
#define  RRC_CONNECTION_REQUEST_TIME_OUT_EVENT  0x02
#define  RRC_CONNECTION_SETUP_EVENT             0x04
#define  RRC_CONNECTION_RELEASE_EVENT           0x08
#define  RRC_CONNECTION_LOSS_EVENT              0x10

// RADIO BEARERS
#define RRC_SRB0_ID  0  // traffic on CCCH
#define RRC_SRB1_ID  1  // any traffic on DCCH using RLC-UM
#define RRC_SRB2_ID  2  // AS traffic using RLC-AM
#define RRC_SRB3_ID  3  // high priority NAS traffic using RLC-AM
#define RRC_SRB4_ID  4  // low  priority NAS traffic using RLC-AM

// RB ESTABLISHMENT
#define RB_STARTED  1
#define RB_STOPPED  0
//For PDCP
#define RRC_RB_NO_USE_PDCP           0
#define RRC_RB_USE_PDCP              1

// PROTOCOL ALLOC
#define RRC_PROTOCOL_ENTITY_FREE         0x00
#define RRC_PROTOCOL_ENTITY_ALLOCATED    0xAA

// RB Release Error cause
#define  RB_FAIL_IDLE_MOBILE         1
#define  RB_FAIL_INVALID_QOS_VALUE   2
#define  RB_FAIL_INVALID_CONFIG      3
#define  RB_FAIL_RLC_FAILURE         4
#define  RB_FAIL_PHY_CHANNEL_FAILURE 5
#define  RB_FAIL_UE_SETUP_FAILURE    6


// extension to 25.331
#define maxCCTrCH_rg                32

/*HNN : Added 04/06 for UE Capability Information messages */
#define ACCESS_STRATUM_RELEASE_INDICATOR_DEFAULT  ACCESS_STRATUM_RELEASE_INDICATOR_REL_6 //The real value is defined in rrc_msg_constant.h
#define EURECOM_KERNEL_RELEASE_INDICATOR_DEFAULT  EURECOM_KERNEL_RELEASE_INDICATOR_REL_26

////25.331
#define maxCCTrCH                    8

#endif
