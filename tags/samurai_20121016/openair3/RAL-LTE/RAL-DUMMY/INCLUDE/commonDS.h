/* -*- Mode: C; c-file-style: "linux"; -*- */
#ifndef COMMONDS_H_
#define COMMONDS_H_

#include <sys/types.h>

#pragma pack(1)

/* Maximum number of QoS flows MIH can handle per transation. */
#define MIH_MAX_FLOWS 16

// Parameter Types
typedef enum {
        MIH_TYPE_MMPL = 2, // list of supportet mobility protocols
        MIH_TYPE_QOS_INFO = 27, // QoS flows information
        MIH_TYPE_MULTICAST_INFO = 30,  // Multicast Info. QoS flow Id
        MIH_TYPE_LINK_CHANNEL = 220, // the channel where the RAL should connect
        MIH_TYPE_LINK_ESSID = 221, // the essid where the link should connect
        MIH_TYPE_STATUS = 245,
        MIH_TYPE_IP_ADDRESS = 247, // the Layer 3 address of a AR
        MIH_TYPE_MAC_POA = 250, // the Layer 2 address or ID of a PoA
        MIH_TYPE_LINK_ID = 243, // the ID of the link VMAC
        MIH_TYPE_EVENTS_MAP = 255, // a list of supported events where 1 means supported an 0 unsupported
        MIH_TYPE_REASON_CODE = 3,
        MIH_TYPE_LINK_TYPE = 248,
        MIH_TYPE_UNIX_SOCKET = 133, // the name of the UnixSocket use for communication vina LINK_SAP
        MIH_TYPE_LQPT = 8, // Link Quality Parameter Type
        MIH_TYPE_LQPOV = 9, // Link Quality Parameter Old Value
        MIH_TYPE_LQPNV = 10, // Link Quality Parameter New Value
        MIH_TYPE_LQPV = 22, // Link Quality Parameter Value
        MIH_TYPE_LINK_PARAMETER_THRESHOLD = 16,
        MIH_TYPE_FLOW_ID_LIST = 28, // List of Flow IDs (for QoS)
        MIH_TYPE_LINK_ACTION = 13,
        MIH_TYPE_INTERFACE_ID = 251,
        MIH_TYPE_VID =180,
        MIH_TYPE_INTERFACE_NAME = 200,
        MIH_TYPE_LINK_LOCAL_ADDRESS = 190,
} MIH_DataTypeCode;

// Primitive types
typedef enum {
		MIH_TYPE_MIH_EVENT_REGISTER = 1,
        MIH_TYPE_LINK_CONFIGURE_THRESHOLDS = 3,
        MIH_TYPE_LINK_SWITCH = 20,
        MIH_TYPE_LINK_GET_PARAMETERS = 21,
//        MIH_TYPE_LINK_RESOURCE_DEACTIVATE = 26,
//        MIH_TYPE_LINK_RESOURCE_ACTIVATE = 40,
        MIH_TYPE_LINK_MULTICAST_JOIN = 25,
        MIH_TYPE_LINK_MULTICAST_LEAVE = 24,
        MIH_TYPE_LINK_ADVERTISMENT = 22,

        MIH_TYPE_LINK_UP = 4,
        MIH_TYPE_LINK_DOWN = 5,
        MIH_TYPE_LINK_PARAMETERS_REPORT = 9,

		MIH_TYPE_MIH_SWITCH = 20,
		MIH_TYPE_MIH_MULTICAST_JOIN = 25,
		MIH_TYPE_MIH_MULTICAST_LEAVE = 24,
		MIH_TYPE_MIH_USER_REGISTRATION = 30,
		MIH_TYPE_MIH_HANDOVER_PREPARE = 32,
		MIH_TYPE_MIH_HANDOVER_INITIATE = 13,
		MIH_TYPE_MIH_HANDOVER_COMMIT = 14,
		MIH_TYPE_MIH_HANDOVER_COMPLETE = 12,
		MIH_TYPE_MIH_SCAN = 31,
		MIH_TYPE_MIH_LINK_RESOURCE_ACTIVATE = 33,
		MIH_TYPE_MIH_LINK_RESOURCE_DEACTIVATE = 34,
} MIH_PrimitiveTypeCode;

//Events codes; to build an event map (32 bits long) just add the required values from the ones below.
typedef enum {
	MIH_EVENT_Link_AdvertisementIndication = 1, 		//00000001 0 0 0
	MIH_EVENT_Link_UpIndication = 128, 					//10000000 0 0 0
	MIH_EVENT_Link_DownIndication = 64, 				//01000000 0 0 0
	MIH_EVENT_Link_Parameters_ReportIndication = 32,	//00100000 0 0 0
	MIH_EVENT_MIH_User_RegistrationIndication = 32768, 	//0 10000000 0 0	
	MIH_EVENT_MIH_Handover_InitiateIndication = 16384, 	//0 01000000 0 0
	MIH_EVENT_MIH_Handover_CommitIndication = 8192, 	//0 00100000 0 0
	MIH_EVENT_MIH_Handover_CompleteIndication = 4096, 	//0 00010000 0 0
} MIH_Events;

typedef enum {
	MIH_LINK_TYPE_GENERIC = 0,
	MIH_LINK_TYPE_ETHERNET = 15,
	MIH_LINK_TYPE_WLAN = 19,
	MIH_LINK_TYPE_ADHOC = 20,
	MIH_LINK_TYPE_UMTS = 23,
	MIH_LINK_TYPE_DVB = 30,
	MIH_LINK_TYPE_WiMAX = 31,
} MIH_LinkTypes;

//Basic types in TLV form

typedef struct {
        u_int8_t type;		//3
        u_int16_t length; 	//1
        u_int8_t reserved;
        u_int8_t value;
        u_int8_t reserved2[3];
}MIH_ReasonCode;

typedef struct {
        u_int8_t type;		//248
        u_int16_t length; 	//4
        u_int8_t reserved;
        u_int32_t value;
}MIH_LinkType;
 
typedef struct {
        u_int8_t type;		//245
        u_int16_t length; 	//1
        u_int8_t reserved;
        u_int8_t value;		//0-Success; 1-Failure; 2-Rejected
        u_int8_t reserved2[3];
}MIH_Status;

typedef struct {
        u_int8_t type;		//251
        u_int16_t length;	//8
        u_int8_t reserved;
        u_int8_t value[8];
}MIH_InterfaceID;

typedef struct {
        u_int8_t type;		//250
        u_int16_t length;	//8
        u_int8_t reserved;
        u_int8_t value[8];
}MIH_MACPoA;

typedef struct {
        u_int8_t type;		//243
        u_int16_t length;	//24
        u_int8_t reserved; 
		MIH_InterfaceID interfaceL2ID;
        MIH_MACPoA poaL2ID;
}MIH_LinkID; //28 bytes

typedef struct {
        u_int8_t type;		//133
        u_int16_t length;	//28
        u_int8_t reserved;
        u_int8_t value[28];
}MIH_UnixSocket;

typedef struct {
        u_int8_t type;		//255
        u_int16_t length;	//4
        u_int8_t reserved;
        u_int32_t value;
}MIH_EventsMap;

typedef struct {
        u_int8_t type;		//247
        u_int16_t length;	//16
        u_int8_t reserved;
        u_int8_t value[16];
}MIH_IPAddress;

typedef struct {
        u_int8_t type;		//190
        u_int16_t length;	//48
        u_int8_t reserved;
        u_int8_t value[48];
}MIH_LinkLocalAddress;



typedef struct {	//Mobility Management Protocol List
        u_int8_t type;		//2
        u_int16_t length;	//1
        u_int8_t reserved;
        u_int8_t value;		// see 802.21 doc
        u_int8_t reserved2[3];
}MIH_MMPL;

typedef struct {	//Link Quality Parameter Type
        u_int8_t type;		//8
        u_int16_t length;	//4
        u_int8_t reserved;
        u_int32_t value;		
}MIH_LQPT;

typedef struct {	//Link Quality Parameter Old Value
        u_int8_t type;		//9
        u_int16_t length;	//4
        u_int8_t reserved;
        u_int32_t value;		
}MIH_LQPOV;

typedef struct {	//Link Quality Parameter New Value
        u_int8_t type;		//10
        u_int16_t length;	//4
        u_int8_t reserved;
        u_int32_t value;		
}MIH_LQPNV;

typedef struct {	//Link Quality Parameter Value
        u_int8_t type;		//22

        u_int16_t length;	//4
        u_int8_t reserved;
        u_int32_t value;		
}MIH_LQPV;

typedef struct {	//Block unit for a report
        MIH_LQPT parameterType;
        MIH_LQPOV oldValue;
        MIH_LQPNV newValue;
}MIH_ReportInfo;

typedef struct {	
        u_int8_t type;		//16
        u_int16_t length;	//4
        u_int8_t reserved;
        u_int32_t value;		
}MIH_LinkParameterThreshold;

typedef struct {
        u_int8_t type;		//220
        u_int16_t length;	//4
        u_int8_t reserved;
        u_int32_t value;		
}MIH_LinkChannel;

typedef struct {
        u_int8_t type;		//221
        u_int16_t length;	//28
        u_int8_t reserved;
        u_int8_t value[28];		
}MIH_LinkESSID;

typedef struct {	//Block unit for ConfigureThresholdsRequest
        MIH_LQPT parameterType;
        MIH_LinkParameterThreshold initiateTh;
        MIH_LinkParameterThreshold rollbackTh;
        MIH_LinkParameterThreshold executeTh;
}MIH_ConfigureThresholdInfoRequestBlock;

typedef struct {	//Block unit for ConfigureThresholdsConfirm
        MIH_LQPT parameterType;
        MIH_Status status;
}MIH_ConfigureThresholdInfoConfirmBlock;

typedef struct {	//BlockUnit for MIH_ScanResponse and Link_Get_ParametersConfirm
	MIH_MACPoA PoA;
	MIH_LQPT lqpt;
	MIH_LQPV lqpv;
	MIH_LinkType linkType;
	MIH_LinkChannel channel;
	MIH_LinkESSID essid;
}MIH_ScanResponseInfoBlock;

typedef struct {
        u_int8_t type;		//13
        u_int16_t length;	//4
        u_int8_t reserved;
        u_int32_t value;		
}MIH_LinkAction;

typedef enum {
        MIH_UPLINK,
        MIH_DOWNLINK,
}MIH_FlowDirection;

typedef struct {
        u_int8_t classIdentifier;
        float reservedBitrate;
}MIH_RSpec;

typedef struct {
        float meanBitrate;
        float bucketDepth;
        float peakBitrate;
        float maximumTransmissionUnit;
}MIH_TSpec;

typedef struct {
        u_int16_t flowIdentifier;
        MIH_FlowDirection direction:8;
        MIH_RSpec rspec;
        MIH_TSpec tspec;
}MIH_FlowSpec;

typedef struct {
        u_int8_t type;		//27
        u_int16_t length;	//384
        u_int8_t reserved;
        MIH_FlowSpec value[MIH_MAX_FLOWS]; /* value[length/sizeof(MIH_FlowSpec)] */
}MIH_QoSInfo; //388

typedef struct {
        u_int8_t type;		//28
        u_int16_t length;	//64
        u_int8_t reserved;
        u_int32_t flowIDs[MIH_MAX_FLOWS];
}MIH_FlowIDList; //68

/* typedef struct { */
/*         u_int8_t type;		//30 */
/*         u_int16_t length;	//4 */
/*         u_int8_t reserved; */
/*         u_int32_t value; */
/* }MIH_MulticastInfo; //8 */

typedef struct {
        u_int8_t type;		//180
        u_int16_t length;	//8
        u_int8_t reserved;
        u_int8_t value[8];
}MIH_VID;	//12

typedef struct {
        u_int8_t type;		//100
        u_int16_t length;	//20
        u_int8_t reserved;
        u_int8_t value[20];				
}MIHF_ID;

typedef struct {
        u_int8_t type;		//101
        u_int16_t length;	//20
        u_int8_t reserved;
        u_int8_t value[20];				
}MIH_USER_ID;

typedef union {
        MIH_LinkLocalAddress remoteAddress;
        MIH_UnixSocket localAddress;
}MIH_User_Address;

typedef struct {
    u_int8_t type;		//200
    u_int16_t length;	//8
    u_int8_t reserved;
    u_int8_t value[8];				
}MIH_InterfaceName;

//Link Primitives

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - LinkID Block
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - end Block
|   Type-Sok    |          Length-Sok           |  Reserved-Sok |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-Sok                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-EM     |          Length-EM            |  Reserved-EM  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-EM                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-VID    |          Length-VID           |  Reserved-VID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                          Value-VID                            +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-IfN    |          Length-IfN           |  Reserved-IfN |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                            Value-IfN                          +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        
		LT - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
		LID	- LinkID
				Type-LID	- 243
				Length-LID	- 24
				Reserved-LID- 0
				Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		Sok	- UnixDomainSocket used for IPC
				Type-Sok	- 133
				Length-Sok	- 28
				Reserved-Sok- 0
				Value-Sok	- the name of UnixDomainSocket (e.g. RAL_UMTS, VIP, ...)
		EM	- EventsMap supported by the RAL Each bit represents an event(1-Supported, 0-Unsupported)
				Type-EM		- 255
				Length-EM	- 4
				Reserved-EM	- 0
				Value-EM	- the events bitmap	//TBD 
		VID	- VID
				Type-VID	- 180
				Length-LID	- 8
				Reserved-LID- 0
				Value-LID	- 8 bytes of value for the VID
		IfN - Interface's Name (eth1, ath0, ...)
				Type-LT		- 200
				Length-LT	- 8
				Reserved-LT	- 0
				Value-LT	- 8 bytes for value
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_UnixSocket ralUnixSocket;
        MIH_EventsMap eventsMap;
        MIH_VID vid;
        MIH_InterfaceName interfaceName;
}Link_AdvertisementIndicationDS; // 108

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |	
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - end Block
|   Type-AR     |          Length-AR            |  Reserved-AR  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-AR                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-MM     |          Length-MM            |  Reserved-MM  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Value-MM    |                Reserved-MM                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        LT - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		AR - The Layer 3 ID of the Access Router
                Type-AR		- 247
                Length-AR	- 16
                Reserved-AR	- 0
                Value-AR	- 16 bytes IPv6 address
        MM - Mobility Management Protocol List
                Type-MM		- 2
                Length-MM	- 1
                Reserved-MM	- 0
                Value-MM	- bitmap for supported protocols (1-supported 0-unsupported)
                Reserved-MM	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_IPAddress addressAR;
        MIH_MMPL mmpl;
}Link_UpIndicationDS; //72

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - end Block
|   Type-AR     |          Length-AR            |  Reserved-AR  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-AR                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-RC     |          Length-RC            |  Reserved-RC  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Value-RC    |                Reserved-RC                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        LT - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        AR - The Layer 3 ID of the Access Router
                Type-AR		- 247
                Length-AR	- 16
                Reserved-AR	- 0
                Value-AR	- 16 bytes IPv6 address
        RC - Reason Code
                Type-RC		- 3
                Length-RC	- 1
                Reserved-RC	- 0
                Value-RC	- see 802.21 doc for values
                Reserved-RC	- 0
*/	
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_IPAddress addressAR;
        MIH_ReasonCode reason;
}Link_DownIndicationDS; //72

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPO   |          Length-LQPO          | Reserved-LQPO |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPO                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPN   |          Length-LQPN          | Reserved-LQPN |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPN                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPO   |          Length-LQPO          | Reserved-LQPO |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPO                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPN   |          Length-LQPN          | Reserved-LQPN |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPN                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPO   |          Length-LQPO          | Reserved-LQPO |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPO                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPN   |          Length-LQPN          | Reserved-LQPN |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPN                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPO   |          Length-LQPO          | Reserved-LQPO |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPO                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPN   |          Length-LQPN          | Reserved-LQPN |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPN                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LT - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        LQPT - Link Quality Parameter Type
                Type-LQPT	- 8
                Length-LQPT	- 4
                Reserved-LQPT	- 0
                Value-LQPT	- 0-speed, 1-signal strength, 2-BER, ...
        LQPO - Link Quality Parameter Old Value
                Type-LQPO	- 9
                Length-LQPO	- 4
                Reserved-LQPO	- 0
                Value-LQPO	- the value
        LQPN - Link Quality Parameter New Value
                Type-LQPN	- 10
                Length-LQPN	- 4
                Reserved-LQPN	- 0
                Value-LQPN	- the value
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_ReportInfo info[4];
}Link_Parameters_ReportIndicationDS; //140

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-ITH    |          Length-ITH           | Reserved-ITH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-ITH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-RTH    |          Length-RTH           | Reserved-RTH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-RTH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-ETH    |          Length-ETH           | Reserved-ETH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-ETH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-ITH    |          Length-ITH           | Reserved-ITH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-ITH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-RTH    |          Length-RTH           | Reserved-RTH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-RTH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-ETH    |          Length-ETH           | Reserved-ETH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-ETH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-ITH    |          Length-ITH           | Reserved-ITH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-ITH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-RTH    |          Length-RTH           | Reserved-RTH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-RTH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-ETH    |          Length-ETH           | Reserved-ETH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-ETH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-ITH    |          Length-ITH           | Reserved-ITH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-ITH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-RTH    |          Length-RTH           | Reserved-RTH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-RTH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-ETH    |          Length-ETH           | Reserved-ETH  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-ETH                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        LQPT - Link Quality Parameter Type
                Type-LQPT	- 8
                Length-LQPT	- 4
                Reserved-LQPT	- 0
                Value-LQPT	- 0-speed, 1-signal strength, 2-BER, ...
        ITH	- Initiate Threshold
                Type-ITH	- 16
                Length-ITH	- 4
                Reserved-ITH- 0
                Value-ITH	- 4 bytes value for the threshold
        RTH	- Rollback Threshold
                Type-RTH	- 16
                Length-RTH	- 4
                Reserved-RTH- 0
                Value-RTH	- 4 bytes value for the threshold
        ETH	- Execute Threshold
                Type-ETH	- 16
                Length-ETH	- 4
                Reserved-ETH- 0
                Value-ETH	- 4 bytes value for the threshold
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_ConfigureThresholdInfoRequestBlock thresholdsInfo[4]; //MAX 4 Thresholds!!!
}Link_Configure_ThresholdsRequestDS; //164

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        LQPT - Link Quality Parameter Type
                Type-LQPT	- 8
                Length-LQPT	- 4
                Reserved-LQPT	- 0
                Value-LQPT	- 0-speed, 1-signal strength, 2-BER, ...
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/	
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_ConfigureThresholdInfoConfirmBlock configurationStatus[4]; //MAX 4 Thresholds!!! 
}Link_Configure_ThresholdsConfirmDS; //100

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch    |           Length-Ch           |  Reserved-Ch  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SSID   |          Length-SSID           | Reserved-SSID|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SSID                           |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        PoA	- The Layer 2 ID of the Point of Attachment (AP or Basis Station)
                Type-PoA	- 250
                Length-PoA	- 8
                Reserved-PoA- 0
                Value-PoA	- 8 bytes value for Layers 2 ID
		LT - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
		Ch - Channel
                Type-Ch		- 220
                Length-Ch	- 4
                Reserved-Ch	- 0
                Value-Ch	- 4 bytes value of the channel (or first parameter used in Handover; 2 B used by other techs than WLAN)
		SSID - ESSID
                Type-SSID	- 221
                Length-SSID	- 28
                Reserved-SSID	- 0
                Value-SSID	- 28 bytes value of the ESSID (or second parameter used in Handover; 2 B used by other techs than WLAN)
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_MACPoA newPoA;
        MIH_LinkType linkType;
        MIH_LinkChannel channel;
        MIH_LinkESSID essid;
}Link_SwitchRequestDS;//104

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_Status status;
}Link_SwitchConfirmDS;//44

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA1   |          Length-PoA1          | Reserved-PoA1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA1                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch1   |           Length-Ch1          |  Reserved-Ch1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch1                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA2   |          Length-PoA2          | Reserved-PoA2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA2                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch2   |           Length-Ch2          |  Reserved-Ch2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch2                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA3   |          Length-PoA3          | Reserved-PoA3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA3                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch3   |           Length-Ch3          |  Reserved-Ch3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch3                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		PoA1/2/3 - The PoAs for which scanning is desired... all bytes 0 and a general scan will be performed
				Type-PoA1/2/3	- 250
				Length-PoA1/2/3	- 8
				Reserved-PoA1/2/3	- 0
				Value-PoA1/2/3	- L2ID of the PoA1/2/3
		Ch1/2/3 - Channel for each of the abouve PoAs
                Type-Ch		- 220
                Length-Ch	- 4
                Reserved-Ch	- 0
                Value-Ch	- 4 bytes value of the channel (or first parameter used in Handover; 2 B used by other techs than WLAN)
		
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_MACPoA PoAs[3];
        MIH_LinkChannel channels[3];
}Link_Get_ParametersRequestDS;//96
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA1   |          Length-PoA1          | Reserved-PoA1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA1                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPV   |          Length-LQPV          | Reserved-LQPV |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPV                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT1    |          Length-LT1           |  Reserved-LT1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT1                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch1   |           Length-Ch1          |  Reserved-Ch1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch1                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SSID1  |          Length-SSID1          |Reserved-SSID1|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SSID1                          |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA2   |          Length-PoA2          | Reserved-PoA2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA2                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPV   |          Length-LQPV          | Reserved-LQPV |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPV                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT2    |          Length-LT2           |  Reserved-LT2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT2                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch2   |           Length-Ch2          |  Reserved-Ch2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch2                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SSID2  |          Length-SSID2          |Reserved-SSID2|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SSID2                          |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA3   |          Length-PoA3          | Reserved-PoA3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA3                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPV   |          Length-LQPV          | Reserved-LQPV |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPV                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT3    |          Length-LT3           |  Reserved-LT3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT3                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch3   |           Length-Ch3          |  Reserved-Ch3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch3                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SSID3  |          Length-SSID3          |Reserved-SSID3|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SSID3                          |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		PoA1/2/3 - The PoA for which scanning was done
				Type-PoA1	- 250
				Length-PoA1	- 8
				Reserved-PoA1	- 0
				Value-PoA1	- L2ID of the PoA1
		LQPT - Link Quality Parameter Type
                Type-LQPT	- 8
                Length-LQPT	- 4
                Reserved-LQPT	- 0
                Value-LQPT	- 0-speed, 1-signal strength, 2-BER, ...
		LQPV - Link Quality Parameter Value 
                Type-LQPV	- 22
                Length-LQPV	- 4
                Reserved-LQPV	- 0
                Value-LQPV	- Value (1 for signal strength)
		LT1/2/3 - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
		Ch1/2/3 - Channel of each of the above PoAs
                Type-Ch		- 220
                Length-Ch	- 4
                Reserved-Ch	- 0
                Value-Ch	- 4 bytes value of the channel (or first parameter used in Handover; 2 B used by other techs than WLAN)
		SSID1/2/3 - ESSID of each of the above PoAs
                Type-SSID	- 221
                Length-SSID	- 28
                Reserved-SSID	- 0
                Value-SSID	- 28 bytes value of the ESSID (or second parameter used in Handover; 2 B used by other techs than WLAN)
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_ScanResponseInfoBlock info[3];
}Link_Get_ParametersConfirmDS;//264 

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-IP     |          Length-IP            |  Reserved-IP  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-IP                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-QoS    |          Length-QoS           |  Reserved-QoS |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
~                          QoS-Value                            ~ - 384 bytes
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        FID - Flow ID List
                Type-FID	- 28
                Length-FID	-  64
                Reserved-FID- 0
                Value-FID	- 
		IP - The MT's CoA... needed only by the UMTS RAL... the other will ignore it
                Type-IP		- 247
                Length-IP	- 16
                Reserved-IP	- 0
                Value-IP	- 16 bytes IPv6 address
		QoS - QoS related info
                Type-QoS	- 27
                Length-QoS	- 384
                Reserved-QoS- 0
                Value-QoS	- 384 bytes of QoS Value
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_IPAddress nCoA;
        MIH_QoSInfo qos;
}Link_Resource_ActivateRequestDS;	//444

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_Status status;
}Link_Resource_ActivateConfirmDS;//44

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-FID    |          Length-FID           |  Reserved-FID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
~                          FID-Value                            ~ - 64 bytes
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        FID - Flow ID List
                Type-FID	- 28
                Length-FID	-  64
                Reserved-FID- 0
                Value-FID	- 
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_FlowIDList flows;
}Link_Resource_DeactivateRequestDS;//104

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_Status status;
}Link_Resource_DeactivateConfirmDS;//44
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-IP     |          Length-IP            |  Reserved-IP  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-IP                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-FID   |           Length-FID          |  Reserved-FID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
~                          FID-Value                            ~ - 64 bytes
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        IP - The MT's PoA... needed only by the UMTS RAL... the other will ignore it
                Type-IP		- 247
                Length-IP	- 16
                Reserved-IP	- 0
                Value-IP	- 16 bytes IPv6 address
        FID - Flow ID List
                Type-FID	- 28
                Length-FID	-  64
                Reserved-FID- 0
                Value-FID	-
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_IPAddress MT_address;
        MIH_FlowIDList flows;
}Link_Multicast_JoinRequestDS;	//124
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_Status status;
}Link_Multicast_JoinConfirmDS;//44
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-IP     |          Length-IP            |  Reserved-IP  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-IP                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-FID   |           Length-FID          |  Reserved-FID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
~                          FID-Value                            ~ - 64 bytes
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        IP - The MT's PoA... needed only by the UMTS RAL... the other will ignore it
                Type-IP		- 247
                Length-IP	- 16
                Reserved-IP	- 0
                Value-IP	- 16 bytes IPv6 address
        FID - Flow ID List
                Type-FID	- 28
                Length-FID	-  64
                Reserved-FID- 0
                Value-FID	-
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_IPAddress MT_address;
        MIH_FlowIDList flows;
}Link_Multicast_LeaveRequestDS;	//124
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_Status status;
}Link_Multicast_LeaveConfirmDS;//44


//MIH Primitives

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - LinkID Block
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - end Block
|   Type-Sok    |          Length-Sok           |  Reserved-Sok |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-Sok                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-EM     |          Length-EM            |  Reserved-EM  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-EM                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-VID    |          Length-VID           |  Reserved-VID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                          Value-VID                            +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        
        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
        LT - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
        LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        Sok	- UnixDomainSocket used for IPC
                Type-Sok	- 133
                Length-Sok	- 28
                Reserved-Sok- 0
                Value-Sok	- the name of UnixDomainSocket (e.g. RAL_UMTS, VIP, ...)
        EM	- EventsMap supported by the RAL Each bit represents an event(1-Supported, 0-Unsupported)
                Type-EM		- 255
                Length-EM	- 4
                Reserved-EM	- 0
                Value-EM	- the events bitmap	//TBD 
        VID	- VID
                Type-VID	- 180
                Length-LID	- 8
                Reserved-LID- 0
                Value-LID	- 8 bytes of value for the VID
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_UnixSocket ralUnixSocket;
        MIH_EventsMap eventsMap;
        MIH_VID vid;
}MIH_Link_AdvertisementIndicationDS; //144

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-AR     |          Length-AR            |  Reserved-AR  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-AR                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-MM     |          Length-MM            |  Reserved-MM  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Value-MM    |                Reserved-MM                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LT - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD

		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		AR - The Layer 3 ID of the Access Router
                Type-AR		- 247
                Length-AR	- 16
                Reserved-AR	- 0
                Value-AR	- 16 bytes IPv6 address
        MM - Mobility Management Protocol List
                Type-MM		- 2
                Length-MM	- 1
                Reserved-MM	- 0
                Value-MM	- bitmap for supported protocols (1-supported 0-unsupported)
                Reserved-MM	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkType linkType;
        MIH_LinkID linkID; 
        MIH_IPAddress addressAR;
        MIH_MMPL mmpl;
}MIH_Link_UpIndicationDS; //116 bytes

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-AR     |          Length-AR            |  Reserved-AR  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-AR                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-RC     |          Length-RC            |  Reserved-RC  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Value-RC    |                Reserved-RC                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
        LT - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		AR - The Layer 3 ID of the Access Router
                Type-AR		- 247
                Length-AR	- 16
                Reserved-AR	- 0
                Value-AR	- 16 bytes IPv6 address
        RC - Reason Code
                Type-RC		- 3
                Length-RC	- 1
                Reserved-RC	- 0
                Value-RC	- see 802.21 doc for values
                Reserved-RC	- 0

*/	
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkType linkType;
        MIH_LinkID linkID; 
        MIH_IPAddress addressAR;
        MIH_ReasonCode reason;
}MIH_Link_DownIndicationDS; //116 bytes
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPO   |          Length-LQPO          | Reserved-LQPO |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPO                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPN   |          Length-LQPN          | Reserved-LQPN |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPN                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPO   |          Length-LQPO          | Reserved-LQPO |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPO                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPN   |          Length-LQPN          | Reserved-LQPN |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPN                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPO   |          Length-LQPO          | Reserved-LQPO |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPO                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPN   |          Length-LQPN          | Reserved-LQPN |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPN                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ - block
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPO   |          Length-LQPO          | Reserved-LQPO |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPO                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPN   |          Length-LQPN          | Reserved-LQPN |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPN                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
        LT - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        LQPT - Link Quality Parameter Type
                Type-LQPT	- 8
                Length-LQPT	- 4
                Reserved-LQPT	- 0
                Value-LQPT	- 0-speed, 1-signal strength, 2-BER, ...
        LQPO - Link Quality Parameter Old Value
                Type-LQPO	- 9
                Length-LQPO	- 4
                Reserved-LQPO	- 0
                Value-LQPO	- the value
        LQPN - Link Quality Parameter New Value
                Type-LQPN	- 10
                Length-LQPN	- 4
                Reserved-LQPN	- 0
                Value-LQPN	- the value

*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_ReportInfo info[4];
}MIH_Link_Parameters_ReportIndicationDS; //172
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-CLID   |          Length-CLID          | Reserved-CLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID1   |          Length-LID1          | Reserved-LID1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If1    |          Length-If1           | Reserved-If1  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If1                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA1   |          Length-PoA1          | Reserved-PoA1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA1                           |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID2   |          Length-LID2          | Reserved-LID2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If2    |          Length-If2           | Reserved-If2  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If2                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA2   |          Length-PoA2          | Reserved-PoA2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA2                           |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID3   |          Length-LID3          | Reserved-LID3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If3    |          Length-If3           | Reserved-If3  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If3                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA3   |          Length-PoA3          | Reserved-PoA3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA3                           |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-LA    |           Length-LA           |  Reserved-LA  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LA                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		CLID - Curent LinkID
                Type-CLID	- 243
                Length-CLID	- 24
                Reserved-CLID- 0
                Value-CLID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		LID1/2/3 - LinkID of the first/second/third option
                Type-LID1	- 243
                Length-LID1	- 24
                Reserved-LID1	- 0
                Value-LID1	- 2 TLV values for the L2ID of the terminal's interface and the L2ID of the current PoA
				-	Type-If1	- 251
					Length-If1	- 8
					Reserved-If1- 0
					Value-If1	- L2ID of the interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA1	- 250
					Length-PoA1	- 8
					Reserved-PoA1	- 0
					Value-PoA1	- L2ID of the PoA
		LA - Link Action
                Type-LA		- 13
                Length-LA	- 4
                Reserved-LA	- 0
                Value-LA	- value 
        
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID currentLinkID;
        MIH_LinkID newLinkIDs[3]; //in case of DVB the first will be for DVB and the second for the return technology
        MIH_LinkAction oldLinkAction;
}MIH_Handover_InitiateIndicationDS; //176
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-CLID   |          Length-CLID          | Reserved-CLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end LinkID block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-LA    |           Length-LA           |  Reserved-LA  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LA                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		CLID - Curent LinkID
                Type-CLID	- 243
                Length-CLID	- 24
                Reserved-CLID- 0
                Value-CLID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		LID	- the new LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		LA - Link Action
                Type-LA		- 13
                Length-LA	- 4
                Reserved-LA	- 0
                Value-LA	- value 
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
		MIH_LinkID currentLinkID;
        MIH_LinkID newLinkID;
        MIH_LinkAction oldLinkAction;
}MIH_Handover_CommitIndicationDS;//120
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-CLID   |          Length-CLID          | Reserved-CLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-OLID   |          Length-OLID          | Reserved-OLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		CLID - Curent LinkID
                Type-CLID	- 243
                Length-CLID	- 24
                Reserved-CLID- 0
                Value-CLID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		OLID - Old LinkID (from where the HO started)
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID currentLinkID;
		MIH_LinkID oldLinkID;
}MIH_Handover_CompleteIndicationDS;//112
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-UID    |          Length-UID           |  Reserved-UID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-UID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-UA     |          Length-UA            |  Reserved-UA  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-UA                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		UID - User_ID
                Type-SID	- 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        UA - User Address
                Type-UA		- 133 or 247
                Length-UA	- 28 or 16
                Reserved-UA	- 0
                Value-UA	- 28 bytes for the UX Domain Socket or 16 bytes for the IPv6 Address 
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_USER_ID userID;
        MIH_User_Address userAddress;	//IP address of where the user can be reached or the local UNIX domain socket
}MIH_User_RegistrationIndicationDS; //112

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-UID    |          Length-UID           |  Reserved-UID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-UID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-UA     |          Length-UA            |  Reserved-UA  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-UA                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-IP     |          Length-IP            |  Reserved-IP  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-IP                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-AR     |          Length-AR            |  Reserved-AR  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-AR                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-VID    |          Length-VID           |  Reserved-VID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                          Value-VID                            +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		UID - User_ID
                Type-SID	- 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        UA - User Address
                Type-UA		- 133 or 247
                Length-UA	- 28 or 16
                Reserved-UA	- 0
                Value-UA	- 28 bytes for the UX Domain Socket or 16 bytes for the IPv6 Address 
        IP - The MT's CoA... needed to register it at the AR
                Type-IP		- 247
                Length-IP	- 16
                Reserved-IP	- 0
                Value-IP	- 16 bytes IPv6 address
        AR - The AR's Link Local IP address as string with the interface... needed to register at the AR
                Type-IP		- 190
                Length-IP	- 48
                Reserved-IP	- 0
                Value-IP	- 48 bytes IPv6 address and interface name
 
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_USER_ID userID;
        MIH_User_Address userAddress;	//IP address of where the user can be reached or the local UNIX domain socket
		MIH_IPAddress coaMT;
		MIH_LinkLocalAddress addressAR;
		MIH_VID vid;
} MIH_User_RegistrationRequestDS;	//216
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-VID    |          Length-VID           |  Reserved-VID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                          Value-VID                            +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 

        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
		MIH_Status status;
		MIH_VID vid;
} MIH_User_RegistrationConfirmDS;	//78
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-EM     |          Length-EM            |  Reserved-EM  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-EM                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        EM	- EventsMap supported by the RAL Each bit represents an event(1-Supported, 0-Unsupported)
                Type-EM		- 255
                Length-EM	- 4
                Reserved-EM	- 0
                Value-EM	- the events bitmap	//TBD 

*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_EventsMap eventsMap;
}MIH_Event_RegisterRequestDS;//100
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-EM     |          Length-EM            |  Reserved-EM  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-EM                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        EM	- EventsMap supported by the RAL Each bit represents an event(1-Supported, 0-Unsupported)
                Type-EM		- 255
                Length-EM	- 4
                Reserved-EM	- 0
                Value-EM	- the events bitmap	//TBD 
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_EventsMap eventsMap;
}MIH_Event_RegisterConfirmDS;//100
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-EM     |          Length-EM            |  Reserved-EM  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-EM                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        EM	- EventsMap supported by the RAL Each bit represents an event(1-Supported, 0-Unsupported)
                Type-EM		- 255
                Length-EM	- 4
                Reserved-EM	- 0
                Value-EM	- the events bitmap	//TBD 
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_EventsMap eventsMap;
}MIH_Event_DeregisterRequestDS;	//100
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-EM     |          Length-EM            |  Reserved-EM  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-EM                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        EM	- EventsMap supported by the RAL Each bit represents an event(1-Supported, 0-Unsupported)
                Type-EM		- 255
                Length-EM	- 4
                Reserved-EM	- 0
                Value-EM	- the events bitmap	//TBD 
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_EventsMap eventsMap;
}MIH_Event_DeregisterConfirmDS;	//100

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA1   |          Length-PoA1          | Reserved-PoA1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA1                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch1   |           Length-Ch1          |  Reserved-Ch1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch1                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA2   |          Length-PoA2          | Reserved-PoA2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA2                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch2   |           Length-Ch2          |  Reserved-Ch2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch2                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA3   |          Length-PoA3          | Reserved-PoA3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA3                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch3   |           Length-Ch3          |  Reserved-Ch3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch3                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		PoA1/2/3 - The PoAs for which scanning is desired... all bytes 0 and a general scan will be performed
				Type-PoA1/2/3	- 250
				Length-PoA1/2/3	- 8
				Reserved-PoA1/2/3	- 0
				Value-PoA1/2/3	- L2ID of the PoA1/2/3
		Ch1/2/3 - Channel for each of the abouve PoAs
                Type-Ch		- 220
                Length-Ch	- 4
                Reserved-Ch	- 0
                Value-Ch	- 4 bytes value of the channel (or first parameter used in Handover; 2 B used by other techs than WLAN)
*/

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_MACPoA PoAs[3];
        MIH_LinkChannel channels[3];
}MIH_ScanRequestDS; //144

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA1   |          Length-PoA1          | Reserved-PoA1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA1                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPV   |          Length-LQPV          | Reserved-LQPV |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPV                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT1    |          Length-LT1           |  Reserved-LT1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT1                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch1   |           Length-Ch1          |  Reserved-Ch1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch1                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SSID1  |          Length-SSID1          |Reserved-SSID1|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SSID1                          |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA2   |          Length-PoA2          | Reserved-PoA2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA2                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPV   |          Length-LQPV          | Reserved-LQPV |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPV                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT2    |          Length-LT2           |  Reserved-LT2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT2                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch2   |           Length-Ch2          |  Reserved-Ch2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch2                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SSID2  |          Length-SSID2          |Reserved-SSID2|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SSID2                          |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA3   |          Length-PoA3          | Reserved-PoA3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA3                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPT   |          Length-LQPT          | Reserved-LQPT |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPT                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LQPV   |          Length-LQPV          | Reserved-LQPV |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-LQPV                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT3    |          Length-LT3           |  Reserved-LT3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT3                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch3   |           Length-Ch3          |  Reserved-Ch3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch3                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SSID3  |          Length-SSID3          |Reserved-SSID3|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SSID3                          |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		PoA1/2/3 - The PoA for which scanning was done
				Type-PoA1	- 250
				Length-PoA1	- 8
				Reserved-PoA1	- 0
				Value-PoA1	- L2ID of the PoA1
		LQPT - Link Quality Parameter Type
                Type-LQPT	- 8
                Length-LQPT	- 4
                Reserved-LQPT	- 0
                Value-LQPT	- 0-speed, 1-signal strength, 2-BER, ...
		LQPV - Link Quality Parameter Value 
                Type-LQPV	- 22
                Length-LQPV	- 4
                Reserved-LQPV	- 0
                Value-LQPV	- Value (1 for signal strength)
		LT1/2/3 - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
		Ch1/2/3 - Channel of each of the above PoAs
                Type-Ch		- 220
                Length-Ch	- 4
                Reserved-Ch	- 0
                Value-Ch	- 4 bytes value of the channel (or first parameter used in Handover; 2 B used by other techs than WLAN)
		SSID1/2/3 - ESSID of each of the above PoAs
                Type-SSID	- 221
                Length-SSID	- 28
                Reserved-SSID	- 0
                Value-SSID	- 28 bytes value of the ESSID (or second parameter used in Handover; 2 B used by other techs than WLAN)
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
		MIH_ScanResponseInfoBlock info[3];
}MIH_ScanResponseDS; //312

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LT     |          Length-LT            |  Reserved-LT  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LT                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-Ch    |           Length-Ch           |  Reserved-Ch  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Value-Ch                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SSID   |          Length-SSID           | Reserved-SSID|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SSID                           |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        PoA	- The Layer 2 ID of the Point of Attachment (AP or Basis Station)
                Type-PoA	- 250
                Length-PoA	- 8
                Reserved-PoA- 0
                Value-PoA	- 8 bytes value for Layers 2 ID
		LT - LinkType
                Type-LT		- 248
                Length-LT	- 4
                Reserved-LT	- 0
                Value-LT	- TBD
        Ch - Channel
                Type-Ch		- 220
                Length-Ch	- 4
                Reserved-Ch	- 0
                Value-Ch	- 4 bytes value of the channel (or first parameter used in Handover; 2 B used by other techs than WLAN)
        SSID - ESSID
                Type-SSID	- 221
                Length-SSID	- 28
                Reserved-SSID	- 0
                Value-SSID	- 28 bytes value of the ESSID (or second parameter used in Handover; 2 B used by other techs than WLAN)
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_MACPoA newPoA;
        MIH_LinkType linkType;
        MIH_LinkChannel channel;
        MIH_LinkESSID essid;
}MIH_SwitchRequestDS;	//144
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_Status status;
}MIH_SwitchConfirmDS;	//92
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-CLID   |          Length-CLID          | Reserved-CLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID1   |          Length-LID1          | Reserved-LID1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If1    |          Length-If1           | Reserved-If1  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If1                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA1   |          Length-PoA1          | Reserved-PoA1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA1                           |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID2   |          Length-LID2          | Reserved-LID2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If2    |          Length-If2           | Reserved-If2  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If2                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA2   |          Length-PoA2          | Reserved-PoA2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA2                           |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID3   |          Length-LID3          | Reserved-LID3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If3    |          Length-If3           | Reserved-If3  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If3                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA3   |          Length-PoA3          | Reserved-PoA3 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA3                           |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-LA    |           Length-LA           |  Reserved-LA  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LA                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-VID    |          Length-VID           |  Reserved-VID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                          Value-VID                            +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		CLID - Curent LinkID
                Type-CLID	- 243
                Length-CLID	- 24
                Reserved-CLID- 0
                Value-CLID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		LID1/2/3 - LinkID of the first/second/third option
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID	- 0
                Value-LID	- 2 TLV values for the L2ID of the terminal's interface and the L2ID of the current PoA
				-	Type-If	- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA	- 0
					Value-PoA	- L2ID of the PoA
		LA - Link Action
                Type-LA		- 13
                Length-LA	- 4
                Reserved-LA	- 0
                Value-LA	- value 
		VID	- VID
                Type-VID	- 180
                Length-LID	- 8
                Reserved-LID- 0
                Value-LID	- 8 bytes of value for the VID

*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID currentLinkID;
        MIH_LinkID newLinkIDs[3]; //in case of DVB the first will be for DVB and the second for the return technology
        MIH_LinkAction oldLinkAction;
        MIH_VID vid;
}MIH_Handover_InitiateRequestDS;	//188
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-CLID   |          Length-CLID          | Reserved-CLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID1   |          Length-LID1          | Reserved-LID1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If1    |          Length-If1           | Reserved-If1  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If1                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA1   |          Length-PoA1          | Reserved-PoA1 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA1                           |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID2   |          Length-LID2          | Reserved-LID2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If2    |          Length-If2           | Reserved-If2  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If2                            |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA2   |          Length-PoA2          | Reserved-PoA2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA2                           |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		CLID - Curent LinkID
                Type-CLID	- 243
                Length-CLID	- 24
                Reserved-CLID- 0
                Value-CLID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0		
		LID1 - LinkID of the first option
                Type-LID1	- 243
                Length-LID1	- 24
                Reserved-LID1	- 0
                Value-LID1	- 2 TLV values for the L2ID of the terminal's interface and the L2ID of the current PoA
				-	Type-If1	- 251
					Length-If1	- 8
					Reserved-If1- 0
					Value-If1	- L2ID of the interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA1	- 250
					Length-PoA1	- 8
					Reserved-PoA1	- 0
					Value-PoA1	- L2ID of the PoA
		LID2 - LinkID of the second option
                Type-LID2	- 243
                Length-LID2	- 24
                Reserved-LID2	- 0
                Value-LID2	- 2 TLV values for the L2ID of the terminal's interface and the L2ID of the current PoA
				-	Type-If2	- 251
					Length-If2	- 8

					Reserved-If2- 0
					Value-If2	- L2ID of the interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA2	- 250
					Length-PoA2	- 8
					Reserved-PoA2	- 0
					Value-PoA2	- L2ID of the PoA
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID currentLinkID;
        MIH_Status ack;
        MIH_LinkID newLinkIDs[2]; //in case of DVB the first will be for DVB and the second for the return technology
}MIH_Handover_InitiateResponseDS;	//148

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-IP     |          Length-IP            |  Reserved-IP  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-IP                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-QoS    |          Length-QoS           |  Reserved-QoS |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
~                          QoS-Value                            ~ - 384 bytes
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        IP - The destination CoA or the multicast group address... needed only by the UMTS RAL... the other will ignore it
                Type-IP		- 247
                Length-IP	- 16
                Reserved-IP	- 0
                Value-IP	- 16 bytes IPv6 address
        QoS - QoS related info
                Type-QoS	- 27
                Length-QoS	- 384
                Reserved-QoS- 0
                Value-QoS	- 384 bytes of QoS Value
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_IPAddress nCoA;
        MIH_QoSInfo qos;
}MIH_Link_Resource_ActivateRequestDS;	// 492

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        
        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_Status status;
}MIH_Link_Resource_ActivateResponseDS;	//92

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-FID    |          Length-FID           |  Reserved-FID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
~                          FID-Value                            ~ - 64 bytes
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        FID - Flow IDs List
                Type-FID	- 28
                Length-FID	- 64
                Reserved-FID- 0
                Value-FID	- 64 bytes of QoS Value
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_FlowIDList flows;
}MIH_Link_Resource_DeactivateRequestDS;	//132

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        
        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_Status status;
}MIH_Link_Resource_DeactivateResponseDS;	//92

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-CLID   |          Length-CLID          | Reserved-CLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-LA    |           Length-LA           |  Reserved-LA  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LA                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-IP     |          Length-IP            |  Reserved-IP  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-IP                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-VID    |          Length-VID           |  Reserved-VID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                          Value-VID                            +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		CLID - Curent LinkID
                Type-CLID	- 243
                Length-CLID	- 24
                Reserved-CLID- 0
                Value-CLID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		LA - Link Action
                Type-LA		- ???
                Length-LA	- 4
                Reserved-LA	- 0
                Value-LA	- value 
        IP - The new CoA or the multicast group address... needed only by the UMTS RAL... the other will ignore it
                Type-IP		- 247
                Length-IP	- 16
                Reserved-IP	- 0
                Value-IP	- 16 bytes IPv6 address
		VID	- VID
                Type-VID	- 180
                Length-LID	- 8
                Reserved-LID- 0
                Value-LID	- 8 bytes of value for the VID
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
		MIH_LinkID currentLinkID;
        MIH_LinkID newLinkID;
        MIH_LinkAction oldLinkAction;
        MIH_IPAddress nCoA;
        MIH_VID vid;
}MIH_Handover_CommitRequestDS;	//152
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-CLID   |          Length-CLID          | Reserved-CLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-LA    |           Length-LA           |  Reserved-LA  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Value-LA                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		CLID - Curent LinkID
                Type-CLID	- 243
                Length-CLID	- 24
                Reserved-CLID- 0
                Value-CLID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		LA - Link Action
                Type-LA		- ???
                Length-LA	- 4
                Reserved-LA	- 0
                Value-LA	- value 
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
		MIH_LinkID currentLinkID;
        MIH_LinkID newLinkID;
        MIH_LinkAction oldLinkAction;
        MIH_Status ack;
}MIH_Handover_CommitResponseDS;	//128
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-CLID   |          Length-CLID          | Reserved-CLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-OLID   |          Length-OLID          | Reserved-OLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-VID    |          Length-VID           |  Reserved-VID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                          Value-VID                            +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		CLID - Curent LinkID
                Type-CLID	- 243
                Length-CLID	- 24
                Reserved-CLID- 0
                Value-CLID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		OLID - Old LinkID (from where the HO started)
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
		VID	- VID
                Type-VID	- 180
                Length-LID	- 8
                Reserved-LID- 0
                Value-LID	- 8 bytes of value for the VID
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID currentLinkID;
        MIH_LinkID oldLinkID;
		MIH_VID vid;
}MIH_Handover_CompleteRequestDS;//124
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-OLID   |          Length-OLID          | Reserved-OLID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA

        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID currentLinkID;
		MIH_LinkID oldLinkID;
		MIH_Status status;
}MIH_Handover_CompleteResponseDS;	//120
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-IP     |          Length-IP            |  Reserved-IP  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-IP                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-FID   |           Length-FID          | Reserved-FID  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
~                          FID-Value                            ~ - 64 bytes
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        IP - The MT's PoA... needed only by the UMTS RAL... the other will ignore it
                Type-IP		- 247
                Length-IP	- 16
                Reserved-IP	- 0
                Value-IP	- 16 bytes IPv6 address
        FID - Flow ID List
                Type-FID	- 28
                Length-FID	-  64
                Reserved-FID- 0
                Value-FID	-
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_IPAddress MT_address;
        MIH_FlowIDList flows;
}MIH_Multicast_JoinRequestDS;	//??
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0

*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_Status status;
}MIH_Multicast_JoinResponseDS;	//92
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-IP     |          Length-IP            |  Reserved-IP  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                          Value-IP                             |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-FID   |           Length-FID          |  Reserved-FID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
~                          FID-Value                            ~ - 64 bytes
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        IP - The MT's PoA... needed only by the UMTS RAL... the other will ignore it
                Type-IP		- 247
                Length-IP	- 16
                Reserved-IP	- 0
                Value-IP	- 16 bytes IPv6 address
        FID - Flow ID
                Type-FID	- 28
                Length-FID	-  64
                Reserved-FID- 0
                Value-FID	-
*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_IPAddress MT_address;
        MIH_FlowIDList flows;
}MIH_Multicast_LeaveRequestDS;	//??
/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Vers.  |Q|P|     Reserved      |servID | op|  event/comm. ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Transaction ID         |      VariableLoadLength       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-SID    |          Length-SID           |  Reserved-SID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-SID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-DID    |          Length-DID           |  Reserved-DID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                                                               +
|                          Value-DID                            |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-LID    |          Length-LID           |  Reserved-LID |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-If     |          Length-If            |  Reserved-If  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-If                             |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type-PoA    |          Length-PoA           |  Reserved-PoA |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Value-PoA                            |
+                                                               +
|                                                               | - end block
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Type-S     |           Length-S            |   Reserved-S  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Value-S    |                 Reserved-S                    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        SID - Source ID (MIHF_ID or MIHFUser_ID)
                Type-SID	- 100 or 101
                Length-SID	- 20
                Reserved-SID- 0
                Value-SID	- 20 bytes value 
        DID - Destination ID (MIHF_ID or MIHFUser_ID)
                Type-DID	- 100 or 101
                Length-DID	- 20
                Reserved-DID- 0
                Value-DID	- 20 bytes value 
		LID	- LinkID
                Type-LID	- 243
                Length-LID	- 24
                Reserved-LID- 0
                Value-LID	- 2 TLV values for the L2ID of the current terminal's interface and the L2ID of the current PoA
				-	Type-If		- 251
					Length-If	- 8
					Reserved-If	- 0
					Value-If	- L2ID of the current interface or the VMAC of that interface as generated by the VIP Module
				-	Type-PoA	- 250
					Length-PoA	- 8
					Reserved-PoA- 0
					Value-PoA	- L2ID of the current PoA
        S - Status
                Type-S		- 245
                Length-S	- 4
                Reserved-S	- 0
                Value-S	- 0-Success, 1-Failure, 2-Rejected
                Reserved-S	- 0

*/
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        MIH_LinkID linkID;
        MIH_Status status;
}MIH_Multicast_LeaveResponseDS;	//92

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
}MIH_Get_InformationRequestDS;	//56

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIHF_ID source;
        MIHF_ID destination;
        u_int8_t* miisData;
}MIH_Get_InformationResponseDS;	//variable

#pragma pack()

#endif /*COMMONDS_H_*/
