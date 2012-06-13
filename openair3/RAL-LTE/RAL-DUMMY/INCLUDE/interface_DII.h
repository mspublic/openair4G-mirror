

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

/*
typedef struct {	//Link Quality Parameter Type
        u_int8_t type;		//8
        u_int16_t length;	//2
        u_int8_t reserved;
        u_int8_t linkType;		
        u_int8_t paramType;
        u_int16_t reserved2;		
}MIH_LQPT;
*/

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

/*
typedef struct {	//BlockUnit for MIH_ScanResponse and Link_Get_ParametersConfirm
	MIH_MACPoA PoA;
	MIH_LQPT lqpt1;
	MIH_LQPV lqpv1;
	MIH_LQPT lqpt2;
	MIH_LQPV lqpv2;
}MIH_ScanResponseInfoBlock;
*/

typedef struct {	//BlockUnit for MIH_ScanResponse and Link_Get_ParametersConfirm
	MIH_MACPoA PoA;
	MIH_LQPT lqpt;
	MIH_LQPV lqpv;
	MIH_LinkType linkType;
	MIH_LinkChannel channel;
	MIH_LinkESSID essid;
}MIH_ScanResponseInfoBlock;

/************************/

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
    u_int8_t type;		//200
    u_int16_t length;	//8
    u_int8_t reserved;
    u_int8_t value[8];				
}MIH_InterfaceName;

/************************/
/************************/
/*
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_UnixSocket ralUnixSocket;
        MIH_EventsMap eventsMap;
}Link_AdvertismentIndicationDS; //68
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

/************************/

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_ReportInfo info[4];
//Link_Parameters_ReportIndicationDS; //124
}Link_Parameters_ReportIndicationDS; //140

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_IPAddress addressAR;
        MIH_MMPL mmpl;
//}Link_UpIndicationDS; //68
}Link_UpIndicationDS; //72

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkType linkType;
        MIH_LinkID linkID;
        MIH_IPAddress addressAR;
        MIH_ReasonCode reason;
//}Link_DownIndicationDS; //68
}Link_DownIndicationDS; //72

/************************/
//typedef struct {
//        u_int8_t header[4];
//        u_int16_t transactionID;
//        u_int16_t valiableLoadLength;
//        MIH_LinkID linkID;
//        MIH_MACPoA PoAs[3];	
//}Link_Get_ParametersRequestDS;//28

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_MACPoA PoAs[3];
        MIH_LinkChannel channels[3];
}Link_Get_ParametersRequestDS;//96
/************************/
//typedef struct {
//        u_int8_t header[4];
//        u_int16_t transactionID;
//        u_int16_t valiableLoadLength;
//        MIH_LinkID linkID;
//        MIH_ScanResponseInfoBlock PoAs[3];
//}Link_Get_ParametersConfirmDS;//28
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_ScanResponseInfoBlock info[3];
}Link_Get_ParametersConfirmDS;//264
/************************/

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_MACPoA newPoA;
        MIH_LinkChannel channel;
        MIH_LinkESSID essid;
}Link_SwitchRequestDS;//72
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
/************************/

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_Status status;
//}Link_SwitchConfirmDS;//28
}Link_SwitchConfirmDS;//44
/************************/
/*
typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_IPAddress destination;
        MIH_QoSInfo qos;
}Link_Resource_PrepareRequestDS;//444
*/

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_IPAddress nCoA;
        MIH_QoSInfo qos;
}Link_Resource_ActivateRequestDS;	//444

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_Status status;
}Link_Resource_ActivateConfirmDS;//44

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_FlowIDList flows;
}Link_Resource_DeactivateRequestDS;//104

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_Status status;
}Link_Resource_DeactivateConfirmDS;//44
//********************

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_IPAddress MT_address;
        MIH_FlowIDList flows;
//}Link_Multicast_JoinRequestDS;	//??
}Link_Multicast_JoinRequestDS;	//124

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_Status status;
}Link_Multicast_JoinConfirmDS;//44

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_IPAddress MT_address;
        MIH_FlowIDList flows;
//}Link_Multicast_LeaveRequestDS;//??
}Link_Multicast_LeaveRequestDS;	//124

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_Status status;
}Link_Multicast_LeaveConfirmDS;//44

//********************

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_ConfigureThresholdInfoRequestBlock thresholdsInfo[4]; //MAX 4 Thresholds!!!
//}Link_Configure_ThresholdsRequestDS; //148
}Link_Configure_ThresholdsRequestDS; //164

typedef struct {
        u_int8_t header[4];
        u_int16_t transactionID;
        u_int16_t valiableLoadLength;
        MIH_LinkID linkID;
        MIH_ConfigureThresholdInfoConfirmBlock configurationStatus[4]; //MAX 4 Thresholds!!!
//}Link_Configure_ThresholdsConfirmDS; //84
}Link_Configure_ThresholdsConfirmDS; //100

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
