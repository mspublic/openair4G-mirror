/***************************************************************************
                          mRALlte_msg.c  -  description
                             -------------------
    copyright            : (C) 2005, 2007 by Eurecom
    email                : michelle.wetterwald@eurecom.fr
 ***************************************************************************
 Dummy A21_MT_RAL_UMTS / Messages processing
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
//add
//#include <sys/time.h>
#include <ctype.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//add end

#include "MIH_C.h"
#include "mRALlte_variables.h"
#include "mRALlte_proto.h"
#include "nas_ue_ioctl.h"

extern int sd_graal, s_nas;
extern struct sockaddr_unralu_socket; //mihf_socket,
extern struct sockaddr_in udp_socket;
extern int wait_start_mihf;
extern int sockaddr_len;

// ADD mRALInt
int listen_mih, la_req=1, lpd=0, link_detec=1, link_scan=0;
u_int8_t rcv_evt_list;
u_int16_t trans_id;
MIH_C_Link_Cfg_Param_List *rcv_Link_Cfg_Param_List;
u_int8_t val[8];

//added for link_get_Parameters and link_action_confirm

MIH_C_Link_Action_Rsp_List *rcv_Link_Action_Rsp_List;

#ifdef UPDATE
MIH_C_Link_Cfg_Param_List *rcv_Link_Cfg_Param_List;

MIH_C_Link_Param_Type_List rcv_Link_Param_Type_List;
MIH_C_Link_States_Req rcv_Link_States_Req;
MIH_C_Link_Desc_Req rcv_Link_Desc_Req;

MIH_C_Link_Param_Type_List *rcv_Link_Param_Type_List_p;
MIH_C_Link_States_Req *rcv_Link_States_Req_p;
MIH_C_Link_Desc_Req *rcv_Link_Desc_Req_p;
#endif

MIH_C_Get_Parameters_Req Get_Parameters_Req;
MIH_C_Get_Parameters_Req *Get_Parameters_Req_msg;

//---------------------------------------------------------------------------
int send_to_mih(char * str, int prim_length){
//---------------------------------------------------------------------------
   int rc;

   udp_socket.sin_family = AF_INET;
   udp_socket.sin_port = htons(PORT_ODTONE);
   udp_socket.sin_addr.s_addr = inet_addr(SOCKET_ADDRESS);
   bzero(&(udp_socket.sin_zero),8);

   rc =sendto(sd_graal, str,prim_length, 0, (struct sockaddr *)&udp_socket, sockaddr_len);

   return rc;
}

//-----------------------------------------------------------------------------
// Print the content of a buffer in hexadecimal and ascii
void RAL_print_buffer(char * bufferP, int lengthP) {
//-----------------------------------------------------------------------------
    char          c;
    unsigned int  index;
    unsigned long octet_index = 0;
    unsigned long char_index  = 0;

    if (bufferP == NULL) {
        return;
    }


    DEBUG("------+-------------------------------------------------+------------------+\n");
    DEBUG("      |  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f | 0123456789abcdef |\n");
    DEBUG("------+-------------------------------------------------+------------------+\n");
    for (octet_index = 0; octet_index < lengthP; octet_index++) {
        if ((octet_index % 16) == 0){
            if (octet_index != 0) {
                DEBUG(" | ");
                for (char_index = octet_index - 16; char_index < octet_index; char_index++) {
                    c = (char) bufferP[char_index] & 0177;
                    if (iscntrl(c) || isspace(c)) {
                        DEBUG(" ");
                    } else {
                        DEBUG("%c", c);
                    }
                }
                DEBUG(" |\n");
            }
            DEBUG(" %04d |", octet_index);
        }
        /*
        * Print every single octet in hexadecimal form
        */
        DEBUG(" %02x", (u_int8_t)(bufferP[octet_index] & 0x00FF));
        /*
        * Align newline and pipes according to the octets in groups of 2
        */
    }

    /*
    * Append enough spaces and put final pipe
    */
    if ((lengthP % 16) > 0) {
        for (index = (octet_index % 16); index < 16; ++index) {
            DEBUG("   ");
        }
    }
    DEBUG(" | ");
    for (char_index = ((octet_index - 1)/ 16) * 16; char_index < lengthP; char_index++) {
        c = (char) bufferP[char_index] & 0177;
        if (iscntrl(c) || isspace(c)) {
            DEBUG(" ");
        } else {
            DEBUG("%c", c);
        }
    }
    if ((lengthP % 16) > 0) {
        for (index = (octet_index % 16); index < 16; ++index) {
            DEBUG(" ");
        }
    }
    DEBUG(" |\n");
    DEBUG("------+-------------------------------------------------+------------------+\n");
}

/***************************************************************************
     Transmission side
 ***************************************************************************/

//-----------------------------------------------------------------------------
void print_header(char* str)
//-----------------------------------------------------------------------------
{
	MIH_C_Header *p;
	p = (MIH_C_Header *) str;

    DEBUG ("[mRAL]:   MID (Message ID) = %hhx0%hhx \n", p->Message_ID, p->Message_ID >>8);
    DEBUG ("[mRAL]:      > Service Identifier (sid) =");
    switch(((p->Message_ID)& SID_PRINT) >> 4)
    {
    	case(SID_SERVICE_MANAGEMENT): DEBUG (" Service Management (1)\n"); break;
    	case(SID_EVENT_SERVICE): DEBUG (" Event Service (2)\n"); break;
    	case(SID_COMMAND_SERVICE): DEBUG (" Command Service (3)\n"); break;
    	case(SID_INFORMATION_SERVICE): DEBUG (" Information Service (4)\n"); break;
    	default: DEBUG (" Not recognized! \n"); break;
    }
    DEBUG ("[mRAL]:      > Operation Code (opcode)  =");
    switch(((p->Message_ID)& OPCODE_PRINT) >> 2)
    {
    	case(OP_CONFIRM): DEBUG (" Confirm (0)\n"); break;
    	case(OP_REQUEST): DEBUG (" Request (1)\n"); break;
    	case(OP_RESPONCE): DEBUG (" Responce (2)\n"); break;
    	case(OP_INDICATION): DEBUG (" Indication (4)\n"); break;
    	default: DEBUG (" Not recognized! \n"); break;
    }

    DEBUG ("[mRAL]:      > Action Identifier (AID)  =");
    if(((p->Message_ID)& SID_PRINT) >> 4 == SID_SERVICE_MANAGEMENT)
    {
		switch(p->Message_ID >> 8)
		{
			case(AID_CAPABILITY_DISCOVER ): DEBUG (" Capability Discover (1)\n"); break;
			case(AID_EVENT_SUBSCRIBE): DEBUG (" Event Subscribe (4)\n"); break;
			case(AID_EVENT_UNSUBSCRIBE): DEBUG (" Event Unsubscribe (5)\n"); break;
			case(AID_LINK_REGISTER_INDICATION): DEBUG (" Link Register Ind (6)\n"); break;
			default: DEBUG (" Not recognized! \n"); break;
		}
    }
	if(((p->Message_ID)& SID_PRINT) >> 4 == SID_EVENT_SERVICE){
		switch(p->Message_ID >> 8)
		{
		    case(AID_LINK_DETECTED): DEBUG (" Link Detected (1)\n"); break;
			case(AID_LINK_UP): DEBUG (" Link Up (2)\n"); break;
			case(AID_LINK_DOWN): DEBUG (" Link Down (3)\n"); break;
			case(AID_LINK_PARAMETERS_REPORT): DEBUG (" Link Parameters Report (5)\n"); break;
			case(AID_LINK_GOING_DOWN): DEBUG(" Link Going Down (6)\n"); break;
			default: DEBUG (" Not recognized! \n"); break;
		}
	}
	if(((p->Message_ID)& SID_PRINT) >> 4 == SID_COMMAND_SERVICE){
		switch(p->Message_ID >> 8)
		{
			case(AID_LINK_GET_PARAMETERS): DEBUG (" Link Get Parameters (1)\n"); break;
			case(AID_LINK_CONFIGURE_THRESHOLDS): DEBUG (" Link Configure Thresholds (2)\n"); break;
			case(AID_LINK_ACTIONS): DEBUG (" Link Actions (3)\n"); break;
			default: DEBUG (" Not recognized! \n"); break;
		}
	}
	if(((p->Message_ID)& SID_PRINT) >> 4 == SID_INFORMATION_SERVICE){
		//switch(p->Message_ID >> 8)
		//{
		    DEBUG (" Add AI \n");
		//}
	}

    DEBUG ("[mRAL]:   Transition ID (Tid) = %d ", ((p->Transaction_ID >> 8) & TID_PRINT));
    DEBUG (" - Payload Length = %d \n\n", p->Payload_Length >> 8);
}

//-----------------------------------------------------------------------------
void print_mihf_id(char* str)
//-----------------------------------------------------------------------------
{
    int i=0, end=0;
    str = str+2;
    end = *(str);
    str = str + 1;
    for(i=0;i<end;i++)
    {
        DEBUG ("%c", *(str++));
    }
    DEBUG ("\n");
}

//-----------------------------------------------------------------------------
void get_mihf_id(char* str, char* output)
//-----------------------------------------------------------------------------
{
    int id_index = 0;
    int i=0, end=0;
    str = str+2;
    end = *(str);
    str = str + 1;
    for(i=0;i<end;i++)
    {
        output[id_index++] = *(str++);
    }
    output[id_index] = 0;
}

//-----------------------------------------------------------------------------
void print_link_id_3GPP(char* str)
//-----------------------------------------------------------------------------
{
	MIH_C_Link_ID_3GPP *p;
	p = (MIH_C_Link_ID_3GPP *) str;
	int i=0;
    DEBUG ("[mRAL]:		- Link Type: ");
    switch(p->LinkType)
    {
    	case(LINK_TYPE_3GPP): DEBUG ("UMTS \n"); break;
    	default: DEBUG ("Link Type not recognized \n \n"); break;
    }

    DEBUG ("[mRAL]:		- 3GPP Address: ");
    	 for(i=0; i<MIH_3GPP_ADDRESS_LENGTH; i++ ) {
    	    DEBUG ("%hhx", p->Address_3GPP.Addr_Val_3GPP[i]);
    	    if(i != MIH_3GPP_ADDRESS_LENGTH-1) DEBUG ("--");
    	 }
    DEBUG ("\n\n");
}

//-----------------------------------------------------------------------------
void print_link_tuple_id_3GPP(char* str)
//-----------------------------------------------------------------------------
{
	MIH_C_Link_Tuple_Id_3GPP *p;
	p = (MIH_C_Link_Tuple_Id_3GPP *) str;

	print_link_id_3GPP((char *) &(p->Link_ID_3GPP));
}

//-----------------------------------------------------------------------------
void print_status(char* str)
//-----------------------------------------------------------------------------
{
	MIH_C_Status *p;
	p = (MIH_C_Status *) str;
	DEBUG ("[mRAL]:		- Status: ");
    switch(p->Status_Value)
    {
    	case(STATUS_SUCCESS): DEBUG ("Success (0) \n"); break;
    	case(STATUS_UNSPECIFIED_FAILURE): DEBUG ("Unspecified Failure (1) \n"); break;
    	case(STATUS_REJECTED): DEBUG ("Rejected (2) \n"); break;
    	case(STATUS_AUTHORIZATION_FAILURE): DEBUG ("Authorization Failure (3) \n"); break;
    	case(STATUS_NETWORK_ERROR): DEBUG ("Network Error (4) \n"); break;
    	default: DEBUG ("Status not recognized \n"); break;
    }
    DEBUG ("\n");
}

//-----------------------------------------------------------------------------
void print_link_evt_list(char* str)
//-----------------------------------------------------------------------------
{
	MIH_C_Link_Evt_List *p;
	p = (MIH_C_Link_Evt_List *) str;
	DEBUG ("[mRAL]:		- ");
    if((p->Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_DETECTED){ DEBUG (" Link_Detected -");}
    if((p->Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_UP){ DEBUG (" Link_Up -");}
    if((p->Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_DOWN){DEBUG (" Link_Down -");}
    if((p->Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_PARAMETERS_REPORT){DEBUG (" Link_Param_Reports -");}
    if((p->Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_GOING_DOWN){DEBUG (" Link_Going_Down -");}
    if((p->Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_HANDOVER_IMMINENT){DEBUG (" Link_Handover_Imminent -");}
    if((p->Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_HANDOVER_COMPLETE){DEBUG (" Link_Handover_Complete -");}
    if((p->Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_PDU_TRANSMIT_STATUS){DEBUG (" Link_PDU_Transmit_Status -");}
    DEBUG ("\n\n");
}

//-----------------------------------------------------------------------------
void print_link_cmd_list(char* str)
//-----------------------------------------------------------------------------
{
	MIH_C_Link_Cmd_List *p;
	p = (MIH_C_Link_Cmd_List *) str;
	DEBUG ("[mRAL]:		- Command List Values: ");
    if(p->Cmd_List_Data.Cmd_List_Oct[3] & CMD_LIST_LINK_EVENT_SUBSCRIBE){DEBUG ("Link_Event_Subscribe -");}
    if(p->Cmd_List_Data.Cmd_List_Oct[3] & CMD_LIST_LINK_EVENT_UNSUBSCRIBE){DEBUG ("Link_Event_Unsubscribe -\n");}
    if(p->Cmd_List_Data.Cmd_List_Oct[3] & CMD_LIST_LINK_GET_PARAMETERS){DEBUG ("[mRAL]:		   - Link_Get_Parameters - ");}
    if(p->Cmd_List_Data.Cmd_List_Oct[3] & CMD_LIST_LINK_CONFIGURE_THRESHOLDS){DEBUG ("Link_Configure_Thresholds - ");}
    if(p->Cmd_List_Data.Cmd_List_Oct[3] & CMD_LIST_LINK_ACTION){DEBUG (" Link_Action");}
    DEBUG ("\n\n");
}


//-----------------------------------------------------------------------------
void print_link_cfg_param(char* str)
//-----------------------------------------------------------------------------
{
	MIH_C_Link_Cfg_Param *p;
	p = (MIH_C_Link_Cfg_Param *) str;
	int i=0;
    switch(p->Link_Param_Type_Choice)
    {
    	case(0): DEBUG ("[mRAL]:		- Generic Link Parameter (0): ");
    			switch(p->Link_Param_Type_Num)
    			{
    				case(0): DEBUG ("Data Rate (0) \n"); break;
    				case(1): DEBUG ("Signal Strength (1) \n"); break;
    				case(2): DEBUG ("SINR (2) \n"); break;
    				case(3): DEBUG ("Throughput (3) \n"); break;
    				case(4): DEBUG ("Packet Error Rate (4) \n"); break;
    				default: DEBUG ("Not recognized \n"); break;
    			}
    	break;
    	case(1): DEBUG ("[mRAL]:		- QoS List Parameters (1): "); break;
				switch(p->Link_Param_Type_Num)
				{
					case(0): DEBUG ("Max Num CoS Supported (0)\n"); break;
					case(1): DEBUG ("Minimum packet transfer delay (1)\n"); break;
					case(2): DEBUG ("Average packet transfer delay (2)\n"); break;
					case(3): DEBUG ("Maximum packet transfer delay (3)\n"); break;
					case(4): DEBUG ("Packet transfer delay jitter (4)\n"); break;
					default: DEBUG ("Reserved \n"); break;
				}
    	default: DEBUG (" Parameter not recognized \n"); break;
    }
	DEBUG ("[mRAL]:		- Link Th_Action: ");
    switch(p->Th_Action)
    {
    	case(LINK_TH_ACTION_SET_NORMAL): DEBUG ("Set normal threshold (0) \n"); break;
    	case(LINK_TH_ACTION_SET_ONE_SHOT): DEBUG ("Set one-shot threshold (1) \n"); break;
    	case(LINK_TH_ACTION_CANCEL): DEBUG ("Cancel threshold (2) \n"); break;
    	default: DEBUG ("Not recognized \n"); break;
    }

	DEBUG ("[mRAL]:		- Link Threshold List Length: ");
	DEBUG (" %d \n", p->Threshold_List.Length);

	for(i=0; i<p->Threshold_List.Length; i++)
	{
		DEBUG ("[mRAL]:		- List Element: %d  Link Threshold_Value: %d \n", (i+1), (p->Threshold_List.Threshold[i].Threshold_Val)/CONVERSION);
		DEBUG ("[mRAL]:		- List Element: %d  Link Threshold_X_Dir: ",(i+1));
	    switch(p->Threshold_List.Threshold[i].Threshold_X_Dir)
	    {
	    	case(ABOVE_THRESHOLD): DEBUG ("ABOVE THRESHOLD (0) \n"); break;
	    	case(BELOW_THRESHOLD): DEBUG ("BELOW THRESHOLD (1) \n"); break;
	    	default: DEBUG ("Reserved \n"); break;
	    }
	}
}

//-----------------------------------------------------------------------------
void print_link_cfg_status(char* str)
//-----------------------------------------------------------------------------
{
	MIH_C_Link_Cfg_Status *p;
	p = (MIH_C_Link_Cfg_Status *) str;

	DEBUG ("[mRAL]:		- Link Cfg Status Param - Threshold_Val: %d \n", p->Threshold.Threshold_Val);
	DEBUG ("[mRAL]:		- Link Cfg Status Param - Threshold_X_Dir: ");
    switch(p->Threshold.Threshold_X_Dir)
    {
    	case(ABOVE_THRESHOLD): DEBUG ("ABOVE THRESHOLD (0) \n"); break;
    	case(BELOW_THRESHOLD): DEBUG ("BELOW THRESHOLD (1) \n"); break;
    	default: DEBUG ("Reserved \n"); break;
    }

	DEBUG ("[mRAL]:		- Link Cfg Status Param - Config Status: ");
    switch(p->Config_Status)
    {
    	case(CONFIG_STATUS_SUCCESS): DEBUG ("SUCCESS (0) \n"); break;
    	case(CONFIG_STATUS_ERROR): DEBUG ("ERROR (1) \n"); break;
    	default: DEBUG ("Wrong Value! \n"); break;
    }
}

//-----------------------------------------------------------------------------
void load_3gpp_address(char* str)
//-----------------------------------------------------------------------------
{
	int i; //, val, l;
	//char address_3gpp[16], buf[3];
	char *buffer;
	//strcpy(address_3gpp, ADDRESS_3GPP);

	MIH_C_Address_3GPP *p;
	p = (MIH_C_Address_3GPP *) str;

	/*for(l=0; l<8; l++)
	{
		i=l*2;
		buf[0]= address_3gpp[i];
		buf[1]= address_3gpp[i+1];
		buf[2]= 0;
		sscanf(buf,"%02x", &val);
		p->Addr_Val_3GPP[l] = val;
	}*/

    buffer = (char *)(&ralupriv->ipv6_l2id[0]);
    //DEBUG ("LOAD IMEI VALUE : = ");
    for (i = 0; i < 8; i++)
    {
      //DEBUG ("-%hhx-", (unsigned char) buffer[i]);
      p->Addr_Val_3GPP[i] = (unsigned char) buffer[i];
    }
    //DEBUG ("\n");
}

//-----------------------------------------------------------------------------
void load_3gpp_address_1(void)
//-----------------------------------------------------------------------------
{
	int i; //, val, l;
	//char address_3gpp[16], buf[3];
	char *buffer;
	//strcpy(address_3gpp, ADDRESS_3GPP);

	//MIH_C_Address_3GPP *p;
	//p = (MIH_C_Address_3GPP *) str;

	/*for(l=0; l<8; l++)
	{
		i=l*2;
		buf[0]= address_3gpp[i];
		buf[1]= address_3gpp[i+1];
		buf[2]= 0;
		sscanf(buf,"%02x", &val);
		p->Addr_Val_3GPP[l] = val;
	}*/

    buffer = (char *)(&ralupriv->ipv6_l2id[0]);
    //DEBUG ("LOAD IMEI VALUE : = ");
    for (i = 0; i < 8; i++)
    {
      //DEBUG ("-%hhx-", (unsigned char) buffer[i]);
    	val[i] = (unsigned char) buffer[i];
    }
}

//---------------------------------------------------------------------------
int mRALte_send_link_register_indication(void){
//---------------------------------------------------------------------------
	int done=0, i=0;
    char str[MIHLink_MAX_LENGTH];
    int prim_length;

    MIH_C_Link_Register_Indication *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Link_Register_Indication);
    msgToBuild = (MIH_C_Link_Register_Indication *) str;

    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_ES_Ind;
    msgToBuild->Header.Transaction_ID = MIH_LINK_HEADER_4_ES_Ind;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Link_Register_Indication)-sizeof(MIH_C_Header)) << 8;

    // TLV  Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    //TLV link register Indication
    msgToBuild->LinkRegister.Type = TLV_LINK_REGISTER_TYPE;
    msgToBuild->LinkRegister.Length = sizeof(MIH_C_Link_ID_3GPP);
    msgToBuild->LinkRegister.Link_ID_3GPP.LinkType = LINK_TYPE_3GPP;
    msgToBuild->LinkRegister.Link_ID_3GPP.Choice_Address = CHOICE_ADDRESS_3GPP;
    msgToBuild->LinkRegister.Link_ID_3GPP.Address_3GPP.Addr_Val_List_Length = sizeof(MIH_C_Address_3GPP)-1;
    load_3gpp_address_1();
    for(i=0; i<8; i++) {msgToBuild->LinkRegister.Link_ID_3GPP.Address_3GPP.Addr_Val_3GPP[i] = val[i];}
    //load_3gpp_address((char*) &(msgToBuild->LinkRegister.Link_ID_3GPP.Address_3GPP));

    if (send_to_mih(str,prim_length)<0){
            perror("[mRALt]: Link_Register_Indication");
            done = 1;
    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: Link_Register_Indication - SENT - Begin \n\n");

	#ifdef OUTPUT_LEVEL_1
    print_header((char *) msgToBuild);
	#endif

    #ifdef OUTPUT_LEVEL_2
    DEBUG ("[mRAL]: Link_Register_Indication - Source: ");
    print_mihf_id((char *) &(msgToBuild->MIHF_Source));
    DEBUG ("[mRAL]: Link_Register_Indication - Destination: ");
    print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
    DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
    DEBUG ("[mRAL]:   LINK_ID - Type: %d \n", msgToBuild->LinkRegister.Type);
    print_link_id_3GPP((char *) &(msgToBuild->LinkRegister.Link_ID_3GPP));
	#endif

    #ifdef OUTPUT_LEVEL_3
    DEBUG ("[mRAL]: 		- Message content: \n");
    mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

    DEBUG ("[mRAL]: Link_Register_Indication - End \n\n");

    return done;
}

//---------------------------------------------------------------------------
int mRALte_send_link_up_indication(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    int prim_length,i;

    MIH_C_Link_Up_Indication *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Link_Up_Indication);
    msgToBuild = (MIH_C_Link_Up_Indication *) str;

    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Link_Up_Ind;
    msgToBuild->Header.Transaction_ID = MIH_LINK_HEADER_4_Link_Up_Ind;
    msgToBuild->Header.Payload_Length =(sizeof(MIH_C_Link_Up_Indication)-sizeof(MIH_C_Header)) << 8;

    // TLV build source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV build destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    //TLV link register
    msgToBuild->Link_Tuple_Id_3GPP.Type = TLV_LINK_TUPLE_ID;
    msgToBuild->Link_Tuple_Id_3GPP.Length = sizeof(MIH_C_Link_ID_3GPP)+sizeof(MIH_C_Choice);
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.LinkType = LINK_TYPE_3GPP;
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Choice_Address = CHOICE_ADDRESS_3GPP;
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP.Addr_Val_List_Length = sizeof(MIH_C_Address_3GPP)-1;
    for(i=0; i<8; i++) {msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP.Addr_Val_3GPP[i] = val[i];}
    //load_3gpp_address((char*) &(msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP));
    msgToBuild->Link_Tuple_Id_3GPP.Choice = CHOICE_NULL;

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL] mRALte_send_link_up_indication");
            wait_start_mihf = 1;
    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: Link_Up_Indication - SENT \n\n");

	#ifdef OUTPUT_LEVEL_1
    print_header((char *) msgToBuild);
    #endif

	#ifdef OUTPUT_LEVEL_2
    DEBUG ("[mRAL]: Link_Up_Indication - Source: ");
    print_mihf_id((char *) &(msgToBuild->MIHF_Source));
    DEBUG ("[mRAL]: Link_Up_Indication - Destination: ");
    print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
    DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
    DEBUG ("[mRAL]:   LINK_TUPLE_ID - Length: %d \n", msgToBuild->Link_Tuple_Id_3GPP.Type, msgToBuild->Link_Tuple_Id_3GPP.Length);
    print_link_tuple_id_3GPP((char *) &(msgToBuild->Link_Tuple_Id_3GPP));
    DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_3
    DEBUG ("[mRAL]: Link_Register_Indication - Message content: \n");
    mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif
    return 0;
}

//---------------------------------------------------------------------------
int mRALte_send_link_down_indication(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    int prim_length=0, done=0, i=0;

    MIH_C_Link_Down_Indication *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Link_Down_Indication);
    msgToBuild = (MIH_C_Link_Down_Indication *) str;

    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Link_Down_Ind;
    msgToBuild->Header.Transaction_ID = MIH_LINK_HEADER_4_Link_Down_Ind;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Link_Down_Indication)-sizeof(MIH_C_Header)) << 8;

    // TLV Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    //TLV Link
    msgToBuild->Link_Tuple_Id_3GPP.Type = TLV_LINK_TUPLE_ID;
    msgToBuild->Link_Tuple_Id_3GPP.Length = sizeof(MIH_C_Link_ID_3GPP)+sizeof(MIH_C_Choice);
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.LinkType = LINK_TYPE_3GPP;
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Choice_Address = CHOICE_ADDRESS_3GPP;
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP.Addr_Val_List_Length = sizeof(MIH_C_Address_3GPP)-1;
    for(i=0; i<8; i++) {msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP.Addr_Val_3GPP[i] = val[i];}
    //load_3gpp_address((char*) &(msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP));
    msgToBuild->Link_Tuple_Id_3GPP.Choice = CHOICE_NULL;

    //TLV Reason Code
    msgToBuild->Reason_Code.Type = TLV_LINK_DOWN_REASON_CODE;
    msgToBuild->Reason_Code.Length = REASON_CODE_LENGTH;
    msgToBuild->Reason_Code.Reason = LINK_DOWN_RC_NO_RESOURCE;

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL] mRALte_send_link_down_indication");
            wait_start_mihf = 1;

    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: Link_Down_Indication - SENT \n\n");

	#ifdef OUTPUT_LEVEL_2
	print_header((char *) msgToBuild);
	DEBUG ("[mRAL]: Link_Down_Indication - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Link_Down_Indication - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   LINK_TUPLE_ID - Length: %d \n", msgToBuild->Link_Tuple_Id_3GPP.Length);
	print_link_tuple_id_3GPP((char *) &(msgToBuild->Link_Tuple_Id_3GPP));
	DEBUG ("[mRAL]:   REASON CODE - Type: %d \n", msgToBuild->Reason_Code.Type);
	DEBUG ("[mRAL]:		- Reason: ");
    switch(msgToBuild->Reason_Code.Reason)
    {
    	case(LINK_DOWN_RC_EXPLICIT_DISCONNECT): DEBUG ("Explicit Disconnect (0) \n"); break;
    	case(LINK_DOWN_RC_PACKET_TIMEOUT): DEBUG ("Packer Timeout (1) \n"); break;
    	case(LINK_DOWN_RC_NO_RESOURCE): DEBUG ("No Resource (2) \n"); break;
    	case(LINK_DOWN_RC_NO_BROADCAST): DEBUG ("No Broadcast (3) \n"); break;
    	case(LINK_DOWN_RC_AUTHENTICATION_FAILURE): DEBUG ("Authentication Failure (4) \n"); break;
    	case(LINK_DOWN_RC_BILLING_FAILURE): DEBUG ("Billing Failure (5) \n"); break;
    	default: DEBUG ("Reason Code Not Recognised \n"); break;
    }
    DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Link_Down_Indication - Message content: \n");
	mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

    return done;
}

//---------------------------------------------------------------------------
int mRALte_send_capability_discover_confirm(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    int prim_length=0, i=0, done=0;
    u_int8_t evt_list=0, cmd_list=0;

    MIH_C_Capability_Discover_Conf *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Capability_Discover_Conf);
    msgToBuild = (MIH_C_Capability_Discover_Conf *) str;

    // Header
    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Cap_Disc_Conf;
    msgToBuild->Header.Transaction_ID = trans_id;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Capability_Discover_Conf)-sizeof(MIH_C_Header)) << 8;

    // TLV Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    // TLV Status
    msgToBuild->Status.Type = TLV_STATUS;
    msgToBuild->Status.Length = sizeof(MIH_C_Status_Value);
    msgToBuild->Status.Status_Value = STATUS_SUCCESS;

    // load the event for Event Subscribe Confirm
    // each bit corresponding to an event is set to 1
    evt_list = evt_list | EVT_LIST_LINK_DETECTED;
    evt_list = evt_list | EVT_LIST_LINK_UP;
    evt_list = evt_list | EVT_LIST_LINK_DOWN;
    evt_list = evt_list | EVT_LIST_LINK_GOING_DOWN;
    evt_list = evt_list | EVT_LIST_LINK_PARAMETERS_REPORT;

    // TLV
    msgToBuild->Link_Evt_List.Type = TLV_LINK_EVENT_LIST;
    msgToBuild->Link_Evt_List.Length = sizeof(MIH_C_Evt_List_Data);
    msgToBuild->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3] = evt_list;
    for(i=0;i<3;i++){msgToBuild->Link_Evt_List.Evt_List_Data.Evt_List_Oct[i] = VALUE_NULL;}

    // load the commands for Event Subscribe Confirm
    // each bit corresponding to a command is set to 1
    cmd_list = cmd_list | CMD_LIST_LINK_EVENT_SUBSCRIBE;
    cmd_list = cmd_list | CMD_LIST_LINK_EVENT_UNSUBSCRIBE;
    cmd_list = cmd_list | CMD_LIST_LINK_GET_PARAMETERS;
    cmd_list = cmd_list | CMD_LIST_LINK_CONFIGURE_THRESHOLDS;
    cmd_list = cmd_list | CMD_LIST_LINK_ACTION;

    //TLV link register
    msgToBuild->Link_Cmd_List.Type = TLV_LINK_CMD_LIST;
    msgToBuild->Link_Cmd_List.Length = sizeof(MIH_C_Cmd_List_Data);
    msgToBuild->Link_Cmd_List.Cmd_List_Data.Cmd_List_Oct[3] = cmd_list;
    for(i=0;i<3;i++){msgToBuild->Link_Cmd_List.Cmd_List_Data.Cmd_List_Oct[i] = VALUE_NULL;}

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL]: mRALu_send_capability_discover_confirm");
            wait_start_mihf = 1;
    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: Link_Capability_Discover_Confirm - SENT - Begin:\n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link_Capability_Discover_Confirm - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Link_Capability_Discover_Confirm - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   STATUS - Type: %d \n", msgToBuild->Status.Type);
	print_status((char *) &(msgToBuild->Status));
	DEBUG ("[mRAL]:   LINK_EVT_LIST - Type: %d \n", msgToBuild->Link_Evt_List.Type);
	print_link_evt_list((char *)&(msgToBuild->Link_Evt_List));
	DEBUG ("[mRAL]:   LINK_CMD_LIST - Type: %d \n", msgToBuild->Link_Cmd_List.Type);
	print_link_cmd_list((char *)&(msgToBuild->Link_Cmd_List));
	#endif

	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Link_Capability_Discover_Confirm - Message content: \n");
	mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

    return done;
}

//---------------------------------------------------------------------------
int mRALte_send_event_subscribe_confirm(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    int prim_length=0, i=0, done=0;

    MIH_C_Event_Subscribe_Conf *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Event_Subscribe_Conf);
    msgToBuild = (MIH_C_Event_Subscribe_Conf *) str;

    // Header
    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Evt_Sub_Conf;
    msgToBuild->Header.Transaction_ID = trans_id;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Event_Subscribe_Conf)-sizeof(MIH_C_Header)) << 8;

    // TLV build source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV build destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    // TLV Status
    msgToBuild->Status.Type = TLV_STATUS;
    msgToBuild->Status.Length = sizeof(MIH_C_Status_Value);
    msgToBuild->Status.Status_Value = STATUS_SUCCESS;

    // TLV Even
    msgToBuild->Link_Evt_List.Type = TLV_LINK_EVENT_LIST;
    msgToBuild->Link_Evt_List.Length = sizeof(MIH_C_Evt_List_Data);
    msgToBuild->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3] = rcv_evt_list;
    for(i=0;i<3;i++){msgToBuild->Link_Evt_List.Evt_List_Data.Evt_List_Oct[i] = VALUE_NULL;}

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL]: mRALu_send_event_subscribe_confirm");
            wait_start_mihf = 1;
    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: Event Subscribe Confirm - SENT \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Event Subscribe Confirm - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Event Subscribe Confirm - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   MIH_EVT_LIST - Type: %d \n", msgToBuild->Link_Evt_List.Type);
	print_link_evt_list((char *)&(msgToBuild->Link_Evt_List));
	#endif

	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Event Subscribe Confirm - Message content: \n");
	mRALu_print_buffer((char *) msgToBuild, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

    return done;
}

//---------------------------------------------------------------------------
int mRALte_send_event_unsubscribe_confirm(void){
//---------------------------------------------------------------------------
    //char * buffer;
    char str[MIHLink_MAX_LENGTH];
    int prim_length=0, done=0, i=0;

    MIH_C_Event_Unsubscribe_Conf *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Event_Unsubscribe_Conf);
    msgToBuild = (MIH_C_Event_Unsubscribe_Conf *) str;

    // Header
    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Evt_Unsub_Conf;
    msgToBuild->Header.Transaction_ID = trans_id;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Event_Unsubscribe_Conf)-sizeof(MIH_C_Header)) << 8;;

    // TLV build source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV build destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    // TLV Status
    msgToBuild->Status.Type = TLV_STATUS;
    msgToBuild->Status.Length = sizeof(MIH_C_Status_Value);
    msgToBuild->Status.Status_Value = STATUS_SUCCESS;

    // TLV
    msgToBuild->Responce_MIH_Evt_List.Type = TLV_EVENT_LIST;
    msgToBuild->Responce_MIH_Evt_List.Length = sizeof(MIH_C_Evt_List_Data);
    msgToBuild->Responce_MIH_Evt_List.Evt_List_Data.Evt_List_Oct[3] = rcv_evt_list;
    for(i=0;i<3;i++){msgToBuild->Responce_MIH_Evt_List.Evt_List_Data.Evt_List_Oct[i] = VALUE_NULL;}

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL]: mRALte_send_event_unsubscribe_confirm");
            wait_start_mihf = 1;
    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: Event Unsubscribe Confirm - SENT \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Event Unsubscribe Confirm - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Event Unsubscribe Confirm - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   STATUS - Type: %d \n", msgToBuild->Status.Type);
	print_status((char *) &(msgToBuild->Status));
	DEBUG ("[mRAL]:   EVT_LIST - Type: %d \n", msgToBuild->Responce_MIH_Evt_List.Type);
	print_link_evt_list((char *)&(msgToBuild->Responce_MIH_Evt_List));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_3

	DEBUG ("[mRAL]: Event Unsubscribe Confirm - Message content: \n");
	mRALu_print_buffer((char *) msgToBuild, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

    return done;
}

//#ifdef UPDATE
//---------------------------------------------------------------------------
int mRALte_send_link_get_parameters_confirm_test(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    char buffer[MIHLink_MAX_LENGTH];
    int prim_length, length_const, length_param, length_states, length_ldr;
    int i,j, index_buff_ldr_init, index_buff_lsr_init;

    MIH_C_Get_Parameters_Confirm_test *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    bzero(buffer,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Get_Parameters_Confirm_test);
    msgToBuild = (MIH_C_Get_Parameters_Confirm_test *) str;

    // Header
    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Get_Param_Conf;
    msgToBuild->Header.Transaction_ID = trans_id;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Get_Parameters_Confirm)-sizeof(MIH_C_Header)) << 8;

    // TLV Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    // TLV Status
    msgToBuild->Status.Type = TLV_STATUS;
    msgToBuild->Status.Length = sizeof(MIH_C_Status_Value);
    msgToBuild->Status.Status_Value = STATUS_SUCCESS;

    // TLV Link Parameters Status List
    msgToBuild->Link_Param_List_test.Type = TLV_LINK_PARAMETERS_STATUS_LIST;
    msgToBuild->Link_Param_List_test.Length = LINK_PARAMETERS_STATUS_LENGTH; //((sizeof(MIH_C_Link_Param_test)*2)+1);
    msgToBuild->Link_Param_List_test.List_Length = LINK_PARAMETERS_STATUS_LIST_LENGTH;
    msgToBuild->Link_Param_List_test.Link_Param[0].Param_Type_Choice = LINK_PARAM_GEN;
    msgToBuild->Link_Param_List_test.Link_Param[0].Link_Param_Type_Num = LINK_PARAM_GEN_VAL;
    msgToBuild->Link_Param_List_test.Link_Param[0].Choice = CHOICE_NULL;
    msgToBuild->Link_Param_List_test.Link_Param[0].Param_Value.Link_Param_Value = LINK_PARAM_GEN_VAL_1;
    msgToBuild->Link_Param_List_test.Link_Param[1].Param_Type_Choice = OP_MODE;
    msgToBuild->Link_Param_List_test.Link_Param[1].Link_Param_Type_Num = 1;
    msgToBuild->Link_Param_List_test.Link_Param[1].Choice = OP_MODE;
    msgToBuild->Link_Param_List_test.Link_Param[1].Param_Value.Qos_Param.Choice_Val = 2;
    msgToBuild->Link_Param_List_test.Link_Param[1].Param_Value.Qos_Param.Qos_Param_Value.Qos_Param_Val_List.List_Length = 2;
    msgToBuild->Link_Param_List_test.Link_Param[1].Param_Value.Qos_Param.Qos_Param_Value.Qos_Param_Val_List.Qos_Param_Val[0].Cos_id = COS_ID;
    msgToBuild->Link_Param_List_test.Link_Param[1].Param_Value.Qos_Param.Qos_Param_Value.Qos_Param_Val_List.Qos_Param_Val[0].Qos_Param_Value = LINK_PARAM_GEN_VAL_1;
    msgToBuild->Link_Param_List_test.Link_Param[1].Param_Value.Qos_Param.Qos_Param_Value.Qos_Param_Val_List.Qos_Param_Val[1].Cos_id = COS_ID;
    msgToBuild->Link_Param_List_test.Link_Param[1].Param_Value.Qos_Param.Qos_Param_Value.Qos_Param_Val_List.Qos_Param_Val[1].Qos_Param_Value = LINK_PARAM_GEN_VAL_1;

    // TLV Link States Response
    msgToBuild->Link_States_Rsp_List.Type = TLV_LINK_STATES_RSP;
    msgToBuild->Link_States_Rsp_List.Length = (sizeof(MIH_C_Link_States_Rsp_List)-2);
    msgToBuild->Link_States_Rsp_List.List_Length = LINK_STATES_RSP_LIST_LENGTH_TEST;
    msgToBuild->Link_States_Rsp_List.Link_States_Rsp[0].Choice= CHOICE_OP_MODE;
    msgToBuild->Link_States_Rsp_List.Link_States_Rsp[0].Rsp_Value.Link_States_Rsp_Value_Op_Mode = OP_MODE_NORMAL_MODE;
    msgToBuild->Link_States_Rsp_List.Link_States_Rsp[1].Choice = 1;
    msgToBuild->Link_States_Rsp_List.Link_States_Rsp[1].Rsp_Value.Link_States_Rsp_Value__Channel_Id = 22;

    // TLV Link Descriptors Response
    msgToBuild->Link_Desc_Rsp_List.Type = TLV_LINK_DESCRIPTOR_RSP;
    msgToBuild->Link_Desc_Rsp_List.Length = (sizeof(MIH_C_Link_Desc_Rsp_List)-2);
    msgToBuild->Link_Desc_Rsp_List.List_Length = LINK_GET_PARAMETERS_CONFIRM_LINK_DESC_LIST_LENGTH;
    msgToBuild->Link_Desc_Rsp_List.Link_Desc_Rsp[0].Choice = CHOICE_NUM_COS;
    msgToBuild->Link_Desc_Rsp_List.Link_Desc_Rsp[0].Link_Desc_Rsp_Value = NUM_COS_VALUE;
    msgToBuild->Link_Desc_Rsp_List.Link_Desc_Rsp[1].Choice = 1;
    msgToBuild->Link_Desc_Rsp_List.Link_Desc_Rsp[1].Link_Desc_Rsp_Value = 25;

    length_const=sizeof(MIH_C_Header)+sizeof(MIH_C_Mihf_ID_Link)+sizeof(MIH_C_Mihf_ID_Mihf)+sizeof(MIH_C_Status);

    for(i=0; i<length_const+3; i++) {buffer[i] = str[i];}

    // copied up to length
    int link_param_length, link_param_list_length;
    int index_lpl_init, index_lsr_init, index_ldr_init, index;
    int index_buff, lpl, l, lsr, ldr;

    link_param_length = 5+3*(QOS_PARAM_VALUE_LIST_LENGTH);
    link_param_list_length = 3+link_param_length*(LINK_PARAM_LIST_LENGTH_TEST);

    index_lpl_init= length_const;
    index_lsr_init= length_const+link_param_list_length;
    index_ldr_init= length_const+link_param_list_length+3+3*(LINK_STATES_RSP_LIST_LENGTH_TEST);

    //DEBUG ("link_param_length %d \n", link_param_length);
    //DEBUG ("link_param_list_length %d \n", link_param_list_length);

    index_buff = index = index_lpl_init+3;
    lpl=str[index-1];

    DEBUG ("[mRAL]: Link_Get_Parameters_Confirm - SENT - Begin \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link_Get_Parameters_Confirm - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Link_Get_Parameters_Confirm - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   STATUS - Type: %d \n", msgToBuild->Status.Type);
	print_status((char *) &(msgToBuild->Status));
	#endif

    DEBUG ("[mRAL]:   LINK PARAM LIST - Type: %d \n", buffer[index_buff-3]);
    DEBUG ("[mRAL]:   LINK PARAM LIST - List Element %d \n", lpl);

    //copy LIST(LINK_PARAM) to buffer
    for(i=0; i<lpl; i++) //for(i=0; i<msgToBuild->Link_Param_List_test.List_Length; i++)
    {
    	index=index_lpl_init+3+link_param_length*i;
    	//DEBUG ("index LINK PARAM %d \n", index);
    	for(j=0; j<2; j++){ buffer[index_buff]=str[index]; index++; index_buff++;}  //LINK_PARAM_TYPE
    	DEBUG ("[mRAL]:		- Link Param Type: ");
        switch(buffer[index_buff-2])
        {
    		case LINK_PARAM_GEN: DEBUG ("- Generic Link Parameters (%hhx) ", buffer[index_buff-2]);
				switch(buffer[index_buff-1])
				{
					case DATA_RATE: DEBUG (" - DATA RATE \n"); break;
					case SIG_STRENGTH: DEBUG (" - SIG_STRENGTH \n"); break;
					case SINR: DEBUG (" - SINR \n"); break;
					case THROUGHPUT: DEBUG (" - THROUGHPUT \n"); break;
					case PACKET_ERROR_RATE: DEBUG (" - PACKET ERROR RATE \n"); break;
					default: DEBUG (" - Add the specific Parameter Type Value"); break;
				} break;
			case LINK_PARAM_QOS: DEBUG (" - Quality of Service Parameters (%hhx) ", buffer[index_buff-2]);
			switch(buffer[index_buff-1])
				{
					case MAX_NUM_COS: DEBUG (" - MAX NUM COS \n"); break;
					case MIN_PACKET_TRANSFER_RATE: DEBUG (" - MIN PACKET TRANSFER RATE \n"); break;
					case AVERAGE_PACKET_TRANSFER_DELAY: DEBUG (" - AVERAGE PACKET TRANSFER DELAY \n"); break;
					case MAXIMUM_PACKET_TRASFER_DELAY: DEBUG (" - MAXIMUM PACKET TRASFER DELAY \n"); break;
					case PACKET_TRASFER_DELAY_JITTER: DEBUG (" - PACKET TRASFER DELAY JITTER \n"); break;
					case PACKET_LOSS_RATE: DEBUG (" - PACKET LOSS RATE \n"); break;
					default: DEBUG ("	- Add the specific Parameter Type Value"); break;
				}break;
        	default: DEBUG ("Not recognized \n"); break;
        }
        //DEBUG ("[mRAL]:		- Link Param Value: ");
        /*switch(buffer[index_buff-1])
        {
        	case(LINK_PARAM_GEN_VAL): DEBUG (" Data Rate\n"); break;
        	default: DEBUG ("Not recognized \n"); break;
        }*/
    	//DEBUG ("index switch %d, %d \n", index, str[index]);
    	buffer[index_buff]=str[index];
    	switch(str[index])   // CHOICE(LINK_PARAM_VAL, QOS_PARAM_VAL)
    	{
    	   case(0): index++; index_buff++;
    	   	   	    for(j=0; j<2; j++){ buffer[index_buff]=str[index]; index++; index_buff++;}
    	   	        switch(buffer[index_buff-2])
    	   	        {
    	   	        	case(0): DEBUG (" [mRAL]:		- Link Param val \n"); break;
    	   	        	case(1): DEBUG (" [mRAL]:		- QoS Param val \n"); break;
    	   	        	default: DEBUG ("Not recognized \n"); break;
    	   	        } break;
    	   case(1): index++; index_buff++;
					//DEBUG ("index switch case(1) %d, %d\n", index, str[index]);
					buffer[index_buff]=str[index];
					switch(str[index])
					{
					   case(0): 	index++; index_buff++;
									for(j=0; j<2; j++){buffer[index_buff]=str[index]; index++; index_buff++;}
									//DEBUG ("index case (1)(0)  %d, %d \n", index, str[index]);
									break;
					   default: 	index++; index_buff++;
									//DEBUG ("index case (1) def %d, %d \n", index, str[index]);
									l= str[index]*3+1;
									//DEBUG ("l value: %d \n", l);
									for(j=0; j<l; j++)
									{
										//DEBUG ("index %d \n", index);
										buffer[index_buff]=str[index];
										index++; index_buff++;
									}
									break;
					}
					//DEBUG ("index %d \n", index);
					break;
    	   default: DEBUG ("Not recognized \n"); break;
    	}
    }

    length_param = index_buff-index_lpl_init-2;
    //printf("Length_param: %d \n", length_param);
    buffer[index_lpl_init+1]=length_param;

    //end LIST(LINK_PARAM) to buffer
    index=index_lsr_init;
    index_buff_lsr_init = index_buff;

    DEBUG ("[mRAL]:   LINK STATES RSP LIST - Type: %d \n", buffer[index_lpl_init]);

    //printf(" LIST(LINK_STATES_RSP) \n lsr: %d, index: %d \n", str[index_lsr_init], index_lsr_init);
    //copy LIST(LINK_STATES_RSP) to buffer
    for(j=0; j<2; j++){buffer[index_buff]=str[index]; index++; index_buff++;}

    lsr= str[index];
    DEBUG ("[mRAL]:   LINK PARAM LIST - List Element %d \n", lsr);
    //printf("lsr: %d, index: %d \n", lsr, index);

    buffer[index_buff]=str[index]; // List_length
    index++; index_buff++;

    for(i=0; i<lsr; i++)
    {
    	buffer[index_buff]=str[index]; //choice
    	switch(str[index])
    	{
    		//printf("str[index]: %d \n", str[index]);
    		case(0): index++; index_buff++; buffer[index_buff]=str[index];
    	   	   	   	DEBUG ("[mRAL]:   		Operational Mode: ");
    	   	        switch(buffer[index_buff])
    	   	        {
    	   	        	case(OP_MODE_NORMAL_MODE): DEBUG (" Normal Mode \n"); break;
    	   	        	case(OP_MODE_POWER_SAVING_MODE): DEBUG (" Power Saving Mode \n"); break;
    	   	        	case(OP_MODE_POWERED_DOWN): DEBUG (" Powered Down \n"); break;
    	   	        	default: DEBUG ("Not recognized \n"); break;
    	   	        }
    	   	        // only 1 byte in buffer when OP_MODE
    	   	   	   	index=index+2; index_buff++;
    	   	   	   	//DEBUG ("index case(0) %d \n", index);
    	   	   	   	break;
    	   case(1): index++; index_buff++;
    	   	   	   	DEBUG ("[mRAL]:   		Channel ID: ");
    	   	   	   	// only 2 byte in buffer when
    	            for(j=0; j<2; j++){ buffer[index_buff]=str[index]; index++; index_buff++;}
    	            //DEBUG (" %d;", buffer[index_buff-2]);
					//DEBUG ("index switch case(1) %d, %d \n", index, str[index]);
					break;
					//buffer[index_buff]=str[index];
    	   default: DEBUG ("value not available \n"); break;
    	}
    	//printf("\n FOR LIST(LINK_PARAM) \n");
    }
    // end LIST(LINK_STATES_RSP)

    //printf("index_buff: %d \n", index);
    //printf("index_lsr_init: %d \n", index_lsr_init);
    length_states = index_buff-index_buff_lsr_init-2;
    //printf("Length_states: %d \n", length_states);
    buffer[index_buff_lsr_init+1]=length_states;

    //printf(" LIST(LINK_STATES_RSP) \n lsr: %d, index: %d \n", str[index], index_ldr_init);

    // LIST(LINK_DESC_RSP)
    index=index_ldr_init;
    index_buff_ldr_init = index_buff;

    for(j=0; j<2; j++){buffer[index_buff]=str[index]; index++; index_buff++;}
    DEBUG ("[mRAL]:   LINK STATES RSP LIST - Type: %d \n", buffer[index_buff-3]);
    DEBUG ("[mRAL]:   LINK STATES RSP LIST - List Element %d \n", buffer[index_buff-2]);
    ldr = str[index]; //list_length * number of int per element
    buffer[index_buff]=str[index];
    index++; index_buff++;

    for(i=0; i<ldr; i++)
    {
    	buffer[index_buff]=str[index];
    	buffer[index_buff+1]=str[index+1];
        switch(buffer[index_buff])
        {
        	case(CHOICE_NUM_COS): 	DEBUG ("[mRAL]:       - Max Num Class of Service: ");
        							DEBUG (" %d \n", buffer[index_buff+1]); break;
        	case(CHOICE_NUM_QUEUE): DEBUG ("[mRAL]:       - Num of transmit queues supported: ");
        							DEBUG (" %d \n", buffer[index_buff+1]); break;
        	default: DEBUG ("Not recognized \n"); break;
        }
    	index=index+2; index_buff=index_buff+2;
    	break;
    }

    length_ldr=index_buff-index_buff_ldr_init-2;
    //printf("LIST(LINK_DESC_RSP): %d \n", length_ldr);
    buffer[index_buff_ldr_init+1]=length_ldr;
    // end LIST(LINK_DESC_RSP)

    msgToBuild->Header.Payload_Length = (index_buff-sizeof(MIH_C_Header)) << 8;
    //printf("Size of MIH_C_Header: %d \n", sizeof(MIH_C_Header));
    buffer[7]= index_buff-sizeof(MIH_C_Header);

    /*DEBUG ("[mRAL]: Str \n");
    mRALu_print_buffer(str, 200);

    DEBUG ("[mRAL]: buffer \n");
    mRALu_print_buffer(buffer, index_buff);*/

    if (send_to_mih(buffer, index_buff)<0){
            perror("[mRAL]: mRALte_send_link_get_parameters_confirm");
            wait_start_mihf = 1;
    }

    DEBUG ("[mRAL]: Link_Get_Parameters_Confirm - SENT - Begin \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link_Get_Parameters_Confirm - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Link_Get_Parameters_Confirm - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   STATUS - Type: %d \n", msgToBuild->Status.Type);
	print_status((char *) &(msgToBuild->Status));

	/*DEBUG ("[mRAL]:   LINK PARAM LIST - Type: %d \n", msgToBuild->Link_Param_List.Type);
	DEBUG ("[mRAL]:   LINK PARAM LIST - List Element %d \n", msgToBuild->Link_Param_List.List_Length);

	// print_link_param
	DEBUG ("[mRAL]:		- Link Param Type: ");
    switch(msgToBuild->Link_Param_List.Link_Param[0].Param_Type_Choice)
    {
    	case(LINK_PARAM_802_11): DEBUG ("802.11 \n"); break;
    	case(LINK_PARAM_GEN): DEBUG (" Generic Link Type \n"); break;
    	default: DEBUG ("Not recognized \n"); break;
    }

    DEBUG ("[mRAL]:		- Link Param Type: ");
    switch(msgToBuild->Link_Param_List.Link_Param[0].Link_Param_Type_Num)
    {
    	case(LINK_PARAM_GEN_VAL): DEBUG (" Data Rate\n"); break;
    	default: DEBUG ("Not recognized \n"); break;
    }

    DEBUG ("[mRAL]:		- Link Param Type (Value): %d \n", msgToBuild->Link_Param_List.Link_Param[0].Link_Param_Value);*/

	//DEBUG ("[mRAL]:   LINK STATES RSP LIST - Type: %d \n", msgToBuild->Link_States_Rsp_List.Type);
	//DEBUG ("[mRAL]:   LINK STATES RSP LIST - List Element %d \n", msgToBuild->Link_States_Rsp_List.List_Length);
	// print_link_states_rsp
	//DEBUG ("[mRAL]:		- Link States requested: ");
    /*switch(msgToBuild->Link_States_Rsp_List.Link_States_Rsp[0].Choice)
    {
    	case(CHOICE_OP_MODE): DEBUG ("Operational Mode \n"); break;
    	case(CHOICE_CHANNEL_ID): DEBUG ("Channel ID \n"); break;
    	default: DEBUG ("Not recognized \n"); break;
    }*/
	/*DEBUG ("[mRAL]:		- Link States Rsp (Value):");
    switch(msgToBuild->Link_States_Rsp_List.Link_States_Rsp[0].Link_States_Rsp_Value)
    {
    	case(OP_MODE_NORMAL_MODE): DEBUG ("Normal Mode \n"); break;
    	case(OP_MODE_POWERED_DOWN): DEBUG ("Powered Down \n"); break;
    	default: DEBUG ("Not recognized \n"); break;
    }*/
	DEBUG ("[mRAL]:   LINK DESC RSP LIST - Type: %d \n", msgToBuild->Link_Desc_Rsp_List.Type);
	DEBUG ("[mRAL]:   LINK DESC RSP LIST - List Length %d \n", msgToBuild->Link_Desc_Rsp_List.List_Length);
	//print_link_desc_rsp
	DEBUG ("[mRAL]:		- Link Desc Rsp: ");
    switch(msgToBuild->Link_Desc_Rsp_List.Link_Desc_Rsp[0].Choice)
    {
    	case(CHOICE_NUM_COS): DEBUG ("NUM_COS - differentiable classes of service supported \n"); break;
    	case(CHOICE_NUM_QUEUE): DEBUG ("NUM_QUEUE - number of transmit queues supported \n"); break;
    	default: DEBUG ("Not recognized \n"); break;
    }

    /* NUM_COS: The maximum number of differentiable classes of service supported.
       NUM_QUEUE: The number of transmit queues supported. Valid Range: 0..255 */

	DEBUG ("[mRAL]:		- Link Desc Rsp (Value):");
	DEBUG ("%d \n\n", msgToBuild->Link_Desc_Rsp_List.Link_Desc_Rsp[0].Link_Desc_Rsp_Value);
	#endif


	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Link_Get_Parameters_Confirm - Message content: \n\n");
	mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

	DEBUG ("[mRAL]: Link_Get_Parameters_Confirm - End \n\n");

    return 0;
}
//#endif

//---------------------------------------------------------------------------
int mRALte_send_link_configure_thresholds_confirm(void){
//---------------------------------------------------------------------------
    //char * buffer;
    char str[MIHLink_MAX_LENGTH];
    int prim_length,i,l,link_cfg_status_length;

    MIH_C_Link_Configure_Thresholds_Confirm *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Link_Configure_Thresholds_Confirm);
    msgToBuild = (MIH_C_Link_Configure_Thresholds_Confirm*) str;

    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Link_Conf_Thres_Conf;
    msgToBuild->Header.Transaction_ID = trans_id;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Link_Configure_Thresholds_Confirm)-sizeof(MIH_C_Header)) << 8;

    // TLV Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    // TLV Status
    msgToBuild->Status.Type = TLV_STATUS;
    msgToBuild->Status.Length = sizeof(MIH_C_Status_Value);
    msgToBuild->Status.Status_Value = STATUS_SUCCESS;

    //TLV Link Configure Status List
    msgToBuild->Link_Cfg_Status_List.Type = TLV_LINK_CFG_STATUS_LIST;

    link_cfg_status_length = 0;

    for(i=0; i<rcv_Link_Cfg_Param_List->List_Length; i++) link_cfg_status_length += rcv_Link_Cfg_Param_List->Link_Cfg_Param[i].Threshold_List.Length;

    msgToBuild->Link_Cfg_Status_List.Length = link_cfg_status_length*sizeof(MIH_C_Link_Cfg_Status);
    msgToBuild->Link_Cfg_Status_List.List_Length = link_cfg_status_length;

    for(i=0; i<link_cfg_status_length; i++)
    {
    	for(l=0; l<LINK_CONF_THRES_CONF_LINK_CFG_STATUS_LIST_LENGTH; l++)
    	{
    		msgToBuild->Link_Cfg_Status_List.Link_Cfg_Status[i].Param_Type_Choice = rcv_Link_Cfg_Param_List->Link_Cfg_Param[i].Link_Param_Type_Choice;
    		msgToBuild->Link_Cfg_Status_List.Link_Cfg_Status[i].Link_Param_Type_Num = rcv_Link_Cfg_Param_List->Link_Cfg_Param[i].Link_Param_Type_Num;
    		msgToBuild->Link_Cfg_Status_List.Link_Cfg_Status[i].Threshold.Threshold_Val = rcv_Link_Cfg_Param_List->Link_Cfg_Param[i].Threshold_List.Threshold[l].Threshold_Val;
    		msgToBuild->Link_Cfg_Status_List.Link_Cfg_Status[i].Threshold.Threshold_X_Dir = rcv_Link_Cfg_Param_List->Link_Cfg_Param[i].Threshold_List.Threshold[l].Threshold_X_Dir;
    		msgToBuild->Link_Cfg_Status_List.Link_Cfg_Status[i].Config_Status = CONFIG_STATUS_TRUE;
    	}
    }

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL]: mRALte_send_link_configure_thresholds_confirm");
            wait_start_mihf = 1;

    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: Link_Configure_Thresholds_Confirm - SENT \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link_Configure_Thresholds_Confirm - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Link_Configure_Thresholds_Confirm - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   STATUS - Type: %d \n", msgToBuild->Status.Type);
	print_status((char *) &(msgToBuild->Status));

	DEBUG ("[mRAL]:   LINK CFG STATUS LIST - Type: %d \n", msgToBuild->Link_Cfg_Status_List.Type);
	DEBUG ("[mRAL]:   LINK CFG STATUS LIST - List Length: %d \n", msgToBuild->Link_Cfg_Status_List.List_Length);
	for(i=0; i<msgToBuild->Link_Cfg_Status_List.List_Length; i++)
	{
		DEBUG ("[mRAL]:   LINK CFG STATUS - %d \n", (i+1));
		print_link_cfg_status((char *) &(msgToBuild->Link_Cfg_Status_List.Link_Cfg_Status[i]));
	}
    DEBUG ("\n");
	#endif


	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Link_Configure_Thresholds_Confirm - Message content: \n\n");
	mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

	DEBUG ("[mRAL]: Link_Configure_Thresholds_Confirm - End \n\n");

    return 0;
}

//---------------------------------------------------------------------------
int mRALte_send_link_action_confirm(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    int prim_length=0, done=0;

    MIH_C_Link_Action_Confirm *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);

    prim_length =  sizeof(MIH_C_Link_Action_Confirm);
    msgToBuild = (MIH_C_Link_Action_Confirm *) str;

    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Link_Act_Conf;
    msgToBuild->Header.Transaction_ID = trans_id;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Link_Action_Confirm)-sizeof(MIH_C_Header)) << 8;

    // TLV Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    // TLV Status
    msgToBuild->Status.Type = TLV_STATUS;
    msgToBuild->Status.Length = sizeof(MIH_C_Status_Value);
    msgToBuild->Status.Status_Value = STATUS_SUCCESS;

    //TLV link action result
    msgToBuild->Link_Ac_Result.Type = TLV_LINK_AC_RESULT;
    msgToBuild->Link_Ac_Result.Length = sizeof(MIH_C_Link_Ac_Result_Value);
    msgToBuild->Link_Ac_Result.Link_Ac_Result_Value = ralupriv->pending_req_status;
    //msgToBuild->Link_Ac_Result.Link_Ac_Result_Value = STATUS_SUCCESS;

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL]: mRALte_send_link_action_confirm");
            wait_start_mihf = 1;
    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: Link_Action_Confirm - SENT \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

    #ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link_Action_Confirm - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Link_Action_Confirm - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   STATUS - Length: %d \n", msgToBuild->Status.Length);
	print_status((char *) &(msgToBuild->Status));

	DEBUG ("[mRAL]:   LINK ACTION RESULT - Length: %d \n", msgToBuild->Link_Ac_Result.Length);
	DEBUG ("[mRAL]:   LINK ACTION RESULT - Action: ");
	switch(msgToBuild->Link_Ac_Result.Link_Ac_Result_Value)
	{
		case(STATUS_SUCCESS): DEBUG("Link Action Success \n"); break;
		default: DEBUG("Link Action not Success \n"); break;
	}
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Link_Action_Confirm - Message content: \n\n");
	mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

	DEBUG ("[mRAL]: Link_Action_Confirm - End\n\n");

	if(la_req == 1){
		sleep(2);
		mRALte_send_link_up_indication();
		sleep(3);
	    mRALte_send_link_parameters_report_indication();
	    sleep(2);
		mRALte_send_link_going_down_indication();
		sleep(4);
	}
	if(la_req == 2)
	{
		sleep(3);
		mRALte_send_link_down_indication();
		sleep(1);
	}
	la_req = 2;

    return done;
}

//---------------------------------------------------------------------------
int mRALte_send_link_action_confirm_lsr(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    int prim_length=0, done=0, i=0;

    MIH_C_Link_Action_Confirm_Lsr *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);

    prim_length =  sizeof(MIH_C_Link_Action_Confirm_Lsr);
    msgToBuild = (MIH_C_Link_Action_Confirm_Lsr *) str;

    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Link_Act_Conf;
    msgToBuild->Header.Transaction_ID = trans_id;

    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Link_Action_Confirm_Lsr)-sizeof(MIH_C_Header)) << 8;

    // TLV Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    // TLV Status
    msgToBuild->Status.Type = TLV_STATUS;
    msgToBuild->Status.Length = sizeof(MIH_C_Status_Value);
    msgToBuild->Status.Status_Value = ralupriv->pending_req_status;
    //msgToBuild->Status.Status_Value = STATUS_SUCCESS;

    // TLV Link Scan Rsp List
	msgToBuild->Link_Action_Rsp_List.Type = TLV_LINK_SCAN_RSP_LIST;
	msgToBuild->Link_Action_Rsp_List.Length = sizeof(MIH_C_Link_Action_Rsp_List)-2;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.List_Length = LINK_STATES_RSP_LIST_LENGTH;

	//added
	for(i=0; i<LINK_STATES_RSP_LIST_LENGTH; i++) //to update when the parameters used to set LIST_LENGTH will be fixed;
	{
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[i].Choice = rcv_Link_Action_Rsp_List->Link_Scan_Rsp_List.Link_Scan_Rsp[0].Choice;
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[i].Address_3GPP_CellId.PLMN_ID[1] = ralupriv->provider_id[i] << 16;
		//msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[i].Address_3GPP_CellId.PLMN_ID[2] = ralupriv->provider[i];
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[i].Address_3GPP_CellId.Address_3GPP_CellId_Value = ralupriv->meas_cell_id[i];
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[i].Network_Id.Network_Id_Length = rcv_Link_Action_Rsp_List->Link_Scan_Rsp_List.Link_Scan_Rsp[i].Network_Id.Network_Id_Length;
		strcpy(msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[i].Network_Id.Network_Id_Value, rcv_Link_Action_Rsp_List->Link_Scan_Rsp_List.Link_Scan_Rsp[i].Network_Id.Network_Id_Value);
		//strcpy(msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[i].Network_Id.Network_Id_Value , MIH_NETWORK_ID);
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[i].Link_Sig_Strength.Choice = rcv_Link_Action_Rsp_List->Link_Scan_Rsp_List.Link_Scan_Rsp[i].Link_Sig_Strength.Choice;
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[i].Link_Sig_Strength.Link_Sig_Strength_Value = ralupriv->last_meas_level[i];
	}

	/* before 16/03/12

	for(i=0; i<1; i++) //to update when the parameters used to set LIST_LENGTH will be fixed;
	{
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Choice = LINK_ADDR_3GPP_3G_CELL_ID;
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.PLMN_ID[1] = MCC;
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.PLMN_ID[2] = MNC;
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.Address_3GPP_CellId_Value = CELL_ID;
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.Address_3GPP_CellId_Value = ralupriv->meas_cell_id[i];
		//msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.Address_3GPP_CellId_Value = ralupriv->cell_id;
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Network_Id.Network_Id_Length = MIH_NETWORK_ID_LENGTH;
		strcpy(msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Network_Id.Network_Id_Value , MIH_NETWORK_ID);
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Choice = CHOICE_NULL;
		msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Link_Sig_Strength_Value = ralupriv->last_meas_level[i];
		//msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Link_Sig_Strength_Value = SIG_STRENGTH_VALUE;
		//msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Link_Sig_Strength_Value = ralupriv->curr_signal_level;
	}*/
	/*msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Choice = LINK_ADDR_3GPP_3G_CELL_ID;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.PLMN_ID[1] = MCC;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.PLMN_ID[2] = MNC;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.Address_3GPP_CellId_Value = CELL_ID;
	//msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.Address_3GPP_CellId_Value = ralupriv->cell_id;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Network_Id.Network_Id_Length = MIH_NETWORK_ID_LENGTH;
	strcpy(msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Network_Id.Network_Id_Value , MIH_NETWORK_ID);
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Choice = CHOICE_NULL;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Link_Sig_Strength_Value = SIG_STRENGTH_VALUE;
	//msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Link_Sig_Strength_Value = ralupriv->curr_signal_level;*/

    //TLV link action result
    msgToBuild->Link_Ac_Result.Type = TLV_LINK_AC_RESULT;
    msgToBuild->Link_Ac_Result.Length = sizeof(MIH_C_Link_Ac_Result_Value);
    msgToBuild->Link_Ac_Result.Link_Ac_Result_Value = STATUS_SUCCESS;

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL]: mRALte_send_link_action_confirm_lsr");
            wait_start_mihf = 1;
    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: Link_Action_Confirm - SENT \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

    #ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link_Action_Confirm - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Link_Action_Confirm - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   STATUS - Length: %d \n", msgToBuild->Status.Length);
	print_status((char *) &(msgToBuild->Status));

	DEBUG ("[mRAL]:   LINK ACTION RESULT - Type: %d \n", msgToBuild->Link_Ac_Result.Type);
	DEBUG ("[mRAL]:   LINK ACTION RESULT - ");
	switch(msgToBuild->Link_Ac_Result.Link_Ac_Result_Value)
	{
		case(STATUS_SUCCESS): DEBUG("Link Action Success \n"); break;
		default: DEBUG("Link Action not Success \n"); break;
	}
	DEBUG ("\n");

	if(link_scan == LINK_SCAN)
	{
		DEBUG ("[mRAL]:   LINK SCAN RESPONSE LIST: ");
		DEBUG ("  Type: %d - List Length: %d \n", msgToBuild->Link_Action_Rsp_List.Type, msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.List_Length);

		DEBUG ("[mRAL]:   	LINK ADDR: ");
		//link_addr Cell_ID 3GPP
		switch(msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Choice)
		{
			case(LINK_ADDR_3GPP_3G_CELL_ID): DEBUG("3GPP 3G Cell ID - Value: %d \n", msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.Address_3GPP_CellId_Value); break;
			default: DEBUG(" Add Link Addr Type \n"); break;
		}
		//network_id
		DEBUG ("[mRAL]:   	NETWORK_ID: ");
		for(i=0; i<MIH_NETWORK_ID_LENGTH; i++)
		{
			DEBUG ("%c", msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Network_Id.Network_Id_Value[i]);
		}
		DEBUG ("\n");
		//sig_strength
		DEBUG ("[mRAL]:   	SIG STRENGTH: %d", msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Link_Sig_Strength_Value);
		DEBUG ("\n");
	}
	#endif

	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Link_Action_Confirm - Message content: \n\n");
	mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

	DEBUG ("[mRAL]: Link_Action_Confirm - End\n\n");

	if(la_req == 1){
		sleep(2);
		mRALte_send_link_up_indication();
		sleep(3);
		#ifdef UPDATE
			if(ralu_priv->event | EVT_LIST_LINK_PARAMETERS_REPORT)
			{
				mRALte_send_link_parameters_report_indication();
			}
			else
			{DEBUG ("[mRAL]: Event not subscribed \n\n");}
		#endif
	    mRALte_send_link_parameters_report_indication(); 	//to comment id UPDATE defined
	    sleep(2);
		#ifdef UPDATE
			if(ralu_priv->event | EVT_LIST_LINK_PARAMETERS_REPORT)
			{
				mRALte_send_link_going_down_indication();
			}
			else
			{DEBUG ("[mRAL]: Event not subscribed \n\n");}
		#endif
		mRALte_send_link_going_down_indication();			//to comment id UPDATE defined
		sleep(4);
	}
	if(la_req == 2)
	{
		sleep(3);
		#ifdef UPDATE
			if(ralu_priv->event | EVT_LIST_LINK_DOWN)
			{
				mRALte_send_link_down_indication();
			}
			else
			{DEBUG ("[mRAL]: Event not subscribed \n\n");}
		#endif
		mRALte_send_link_down_indication();
		sleep(1);
	}
	la_req = 2;

    return done;
}

#ifdef UPDATE
//---------------------------------------------------------------------------
int mRALte_send_link_action_confirm_lsr(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    char buffer[MIHLink_MAX_LENGTH];
    int prim_length=0, done=0, i=0, length_scan_rsp, last_index, size_rsp_list, size_lar, size_all, size_send;

    MIH_C_Link_Action_Confirm_Lsr *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);

    prim_length =  sizeof(MIH_C_Link_Action_Confirm_Lsr);
    msgToBuild = (MIH_C_Link_Action_Confirm_Lsr *) str;

    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Link_Act_Conf;
    msgToBuild->Header.Transaction_ID = trans_id;
    if(list_scan_resp == 1) { msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Link_Action_Confirm_Lsr)-sizeof(MIH_C_Header)) << 8;}
    else { msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Link_Action_Confirm_Lsr)-sizeof(MIH_C_Header)-sizeof(MIH_C_Link_Action_Rsp_List)) << 8;}

    // TLV Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    // TLV Status
    msgToBuild->Status.Type = TLV_STATUS;
    msgToBuild->Status.Length = sizeof(MIH_C_Status_Value);
    msgToBuild->Status.Status_Value = STATUS_SUCCESS;

    // TLV Link Scan Rsp List
	msgToBuild->Link_Action_Rsp_List.Type = TLV_LINK_SCAN_RSP_LIST;
	msgToBuild->Link_Action_Rsp_List.Length = sizeof(MIH_C_Link_Action_Rsp_List)-2;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.List_Length = LINK_STATES_RSP_LIST_LENGTH;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Choice = LINK_ADDR_3GPP_3G_CELL_ID;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.PLMN_ID[1] = MCC;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.PLMN_ID[2] = MNC;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.Address_3GPP_CellId_Value = CELL_ID;
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Network_Id.Network_Id_Length = MIH_NETWORK_ID_LENGTH;
	strcpy(msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Network_Id.Network_Id_Value , MIH_NETWORK_ID);
	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Choice = CHOICE_NULL;

	msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Link_Sig_Strength_Value = SIG_STRENGTH_VALUE;

    //TLV link action result
    msgToBuild->Link_Ac_Result.Type = TLV_LINK_AC_RESULT;
    msgToBuild->Link_Ac_Result.Length = sizeof(MIH_C_Link_Ac_Result_Value);
    msgToBuild->Link_Ac_Result.Link_Ac_Result_Value = STATUS_SUCCESS;

    length_scan_rsp = sizeof(MIH_C_Header)+sizeof(MIH_C_Mihf_ID_Link)+sizeof(MIH_C_Mihf_ID_Mihf)+sizeof(MIH_C_Status);

    for(i=0;i<length_scan_rsp; i++)
    {
    	buffer[i]=str[i];
    	last_index = i;
    }

    //mRALu_print_buffer(str, length_scan_rsp);

    //mRALu_print_buffer(buffer, length_scan_rsp);

    last_index++;

    size_all = sizeof(MIH_C_Link_Action_Rsp_List)+sizeof(MIH_C_Link_Ac_Result);
    size_rsp_list = sizeof(MIH_C_Link_Action_Rsp_List);
    size_lar = sizeof(MIH_C_Link_Ac_Result);

    //printf("size_all: %d \n", size_all);

    //list_scan_resp = 1;
    //if List Scan Rsp
    if(list_scan_resp ==1){
    	for(i=0;i<size_all; i++)
    	{
    		buffer[last_index+i]=str[last_index+i];
    		size_send=last_index+i;
    	}
    }
    else
    {
    	for(i=0;i<size_lar; i++)
    	{
    		buffer[last_index+i]=str[last_index+size_rsp_list+i];
    		size_send=last_index+i;
    	}
    }

    //mRALu_print_buffer(str, 100);

    //mRALu_print_buffer(buffer, 100);

    size_send++;

    if (send_to_mih(buffer,size_send)<0){
            perror("[mRAL]: mRALte_send_link_action_confirm_lsr");
            wait_start_mihf = 1;
    }

    DEBUG ("[mRAL]: Link_Action_Confirm - SENT \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

    #ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link_Action_Confirm - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Link_Action_Confirm - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   STATUS - Length: %d \n", msgToBuild->Status.Length);
	print_status((char *) &(msgToBuild->Status));

	DEBUG ("[mRAL]:   LINK ACTION RESULT - Type: %d \n", msgToBuild->Link_Ac_Result.Type);
	DEBUG ("[mRAL]:   LINK ACTION RESULT - ");
	switch(msgToBuild->Link_Ac_Result.Link_Ac_Result_Value)
	{
		case(STATUS_SUCCESS): DEBUG("Link Action Success \n"); break;
		default: DEBUG("Link Action not Success \n"); break;
	}
	DEBUG ("\n");

	if(link_scan == LINK_SCAN)
	{
		DEBUG ("[mRAL]:   LINK SCAN RESPONSE LIST: ");
		DEBUG ("  Type: %d - List Length: %d \n", msgToBuild->Link_Action_Rsp_List.Type, msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.List_Length);

		DEBUG ("[mRAL]:   	LINK ADDR: ");
		//link_addr Cell_ID 3GPP
		switch(msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Choice)
		{
			case(LINK_ADDR_3GPP_3G_CELL_ID): DEBUG("3GPP 3G Cell ID - Value: %d \n", msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Address_3GPP_CellId.Address_3GPP_CellId_Value); break;
			default: DEBUG(" Add Link Addr Type \n"); break;
		}
		//network_id
		DEBUG ("[mRAL]:   	NETWORK_ID: ");
		for(i=0; i<MIH_NETWORK_ID_LENGTH; i++)
		{
			DEBUG ("%c", msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Network_Id.Network_Id_Value[i]);
		}
		DEBUG ("\n");
		//sig_strength
		DEBUG ("[mRAL]:   	SIG STRENGTH: %d", msgToBuild->Link_Action_Rsp_List.Link_Scan_Rsp_List.Link_Scan_Rsp[0].Link_Sig_Strength.Link_Sig_Strength_Value);
		DEBUG ("\n");
	}
	#endif

	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Link_Action_Confirm - Message content: \n\n");
	mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

	DEBUG ("[mRAL]: Link_Action_Confirm - End\n\n");

    return done;
}
#endif


//---------------------------------------------------------------------------
int mRALte_send_link_detected_indication(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    int prim_length=0, done=0, i=0;

    u_int8_t link_mihcap_flag=0;
    u_int32_t link_net_caps = 0;

    MIH_C_Link_Detected_Indication *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Link_Detected_Indication);
    msgToBuild = (MIH_C_Link_Detected_Indication *) str;

    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Link_Detected_Ind;
    msgToBuild->Header.Transaction_ID = MIH_LINK_HEADER_4_Link_Detected_Ind;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Link_Detected_Indication)-sizeof(MIH_C_Header)) << 8;

    // TLV Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    // TLV Link_Det_Info_List
    msgToBuild->Link_Det_Info.Type = TLV_LINK_DET_INFO;
    msgToBuild->Link_Det_Info.Length = sizeof(MIH_C_Link_Det_Info)-2;
    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_ID_3GPP.LinkType = LINK_TYPE_3GPP;
    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_ID_3GPP.Choice_Address = CHOICE_ADDRESS_3GPP;
    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_ID_3GPP.Address_3GPP.Addr_Val_List_Length = sizeof(MIH_C_Address_3GPP)-1;
    for(i=0; i<8; i++) {msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_ID_3GPP.Address_3GPP.Addr_Val_3GPP[i] = val[i];}
    //load_3gpp_address((char*) &(msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_ID_3GPP.Address_3GPP));
    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Choice = CHOICE_NULL;
    // Network ID
    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Network_Id.Network_Id_Length = MIH_NETWORK_ID_LENGTH;
    strcpy(msgToBuild->Link_Det_Info.Link_Det_Info_Value.Network_Id.Network_Id_Value, MIH_NETWORK_ID);
    // Netaux
    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Net_Aux_Id.Net_Aux_Id_Length = MIH_NET_AUX_ID_LENGTH;
    strcpy(msgToBuild->Link_Det_Info.Link_Det_Info_Value.Net_Aux_Id.Net_Aux_Id_Value, MIH_NETAUX);

    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Sig_Strength.Choice = CHOICE_NULL;
	#ifdef UPDATE
    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Sig_Strength.Sig_Strength_Value = LINK_DETECTED_INDICATION_SIG_STRENGTH;
	#endif
    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Sig_Strength.Sig_Strength_Value = LINK_DETECTED_INDICATION_SIG_STRENGTH;
    //msgToBuild->Link_Det_Info.Link_Det_Info_Value.Sig_Strength.Sig_Strength_Value = ralupriv->curr_signal_level;

    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Sinr = LINK_DETECTED_INDICATION_SINR;

    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_Data_Rate = LINK_DETECTED_INDICATION_LINK_DATA_RATE;

    link_mihcap_flag = link_mihcap_flag | EVENT_SERVICE_SUPPORTED_ON;
    link_mihcap_flag = link_mihcap_flag | COMMAND_SERVICE_SUPPORTED_ON;
    link_mihcap_flag = link_mihcap_flag | INFORMATION_SERVICE_SUPPORTED_ON;

    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_Mihcap_Flag = link_mihcap_flag;

    link_net_caps = link_net_caps | NET_CAPS_QOS_CLASS_0;
    link_net_caps = link_net_caps | NET_CAPS_QOS_CLASS_1;
    link_net_caps = link_net_caps | NET_CAPS_INTERNET_ACCESS;

    msgToBuild->Link_Det_Info.Link_Det_Info_Value.Net_Caps = link_net_caps;

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL]: mRALte_send_link_detected_indication");
            wait_start_mihf = 1;
    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: Link_Detected_Indication - SENT - Begin \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link_Detected_Indication - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Link_Detected_Indication - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:     - Link Det Info - Type: %d \n", msgToBuild->Link_Det_Info.Type);
	print_link_id_3GPP((char *) &(msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_ID_3GPP));
	DEBUG ("[mRAL]:         - Network identifier: ");
	for(i=0;i<MIH_NETWORK_ID_LENGTH ;i++)
	{
		DEBUG("%c", msgToBuild->Link_Det_Info.Link_Det_Info_Value.Network_Id.Network_Id_Value[i]);
	}
	DEBUG ("\n");
	DEBUG ("[mRAL]:         - Auxiliary access network identifier: ");
	for(i=0; i<MIH_NET_AUX_ID_LENGTH; i++)
	{
		DEBUG("%c", msgToBuild->Link_Det_Info.Link_Det_Info_Value.Net_Aux_Id.Net_Aux_Id_Value[i]);
	}
	DEBUG ("\n");

	DEBUG ("[mRAL]:         - Signal Strength: %d \n", msgToBuild->Link_Det_Info.Link_Det_Info_Value.Sig_Strength.Sig_Strength_Value);
    DEBUG ("[mRAL]:         - SINR Value: %d \n", msgToBuild->Link_Det_Info.Link_Det_Info_Value.Sinr);
    DEBUG ("[mRAL]:         - Link Data Rate: %d \n", msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_Data_Rate);
    DEBUG ("[mRAL]:         - MIH Capabilities Supported: \n");
    DEBUG ("[mRAL]:          ");
    if((msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_Mihcap_Flag) & EVENT_SERVICE_SUPPORTED_ON){ DEBUG (" Evt Service (ES) -");}
    if((msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_Mihcap_Flag) & COMMAND_SERVICE_SUPPORTED_ON){ DEBUG (" Cmd Service (CS) -");}
    if((msgToBuild->Link_Det_Info.Link_Det_Info_Value.Link_Mihcap_Flag) & INFORMATION_SERVICE_SUPPORTED_ON){DEBUG (" Inf Service (IS) -");}
    DEBUG ("\n");
    DEBUG ("[mRAL]:         - High Level Capabilities Supported: \n");
    DEBUG ("[mRAL]:          ");
    if((msgToBuild->Link_Det_Info.Link_Det_Info_Value.Net_Caps) & NET_CAPS_QOS_CLASS_0){ DEBUG (" QoS Class 0 -");}
    if((msgToBuild->Link_Det_Info.Link_Det_Info_Value.Net_Caps) & NET_CAPS_QOS_CLASS_1){ DEBUG (" QoS Class 1 -");}
    if((msgToBuild->Link_Det_Info.Link_Det_Info_Value.Net_Caps) & NET_CAPS_INTERNET_ACCESS){DEBUG (" Internet Access -");}
    DEBUG ("\n\n");
	#endif

	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Link_Action_Confirm - Message content: \n\n");
	mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

    DEBUG ("[mRAL]: Link_Detected_Indication - End \n\n");

    link_detec = 0;

    return done;
}

//---------------------------------------------------------------------------
int mRALte_send_link_going_down_indication(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    int prim_length=0, done=0, i=0;

    MIH_C_Link_Going_Down_Indication *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Link_Going_Down_Indication);
    msgToBuild = (MIH_C_Link_Going_Down_Indication *) str;

    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Link_Going_Down;
    msgToBuild->Header.Transaction_ID = MIH_LINK_HEADER_4_Link_Detected_Ind;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Link_Going_Down_Indication)-sizeof(MIH_C_Header)) << 8;

    // TLV Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    //TLV Link
    msgToBuild->Link_Tuple_Id_3GPP.Type = TLV_LINK_TUPLE_ID;
    msgToBuild->Link_Tuple_Id_3GPP.Length = sizeof(MIH_C_Link_ID_3GPP)+sizeof(MIH_C_Choice);
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.LinkType = LINK_TYPE_3GPP;
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Choice_Address = CHOICE_ADDRESS_3GPP;
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP.Addr_Val_List_Length = sizeof(MIH_C_Address_3GPP)-1;
    for(i=0; i<8; i++) {msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP.Addr_Val_3GPP[i] = val[i];}
    //load_3gpp_address((char*) &(msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP));
    msgToBuild->Link_Tuple_Id_3GPP.Choice = CHOICE_NULL;

    //TLV Time Interval
    msgToBuild->Time_Interval.Type = TLV_LINK_TIME_INTERVAL;
    msgToBuild->Time_Interval.Length = sizeof(MIH_C_Time_Interval_Value);
    msgToBuild->Time_Interval.Time_Interval_Value = LINK_DETECTED_TIME_INTERVAL;

    //TLV Link Going Down Reason
    msgToBuild->Link_Going_Down_Reason.Type = TLV_LINK_GOING_DOWN_REASON;
    msgToBuild->Link_Going_Down_Reason.Length = sizeof(MIH_C_Link_Going_Down_Reason_Value);
    msgToBuild->Link_Going_Down_Reason.Link_Going_Down_Reason_Value =  LINK_GOING_DOWN_REASON_LINK_PARAM_DEGRADING;

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL]: mRALte_send_link_going_down_indication");
            wait_start_mihf = 1;
    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: mRALte_send_link_going_down - SENT - Begin \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link_Going_Down - Source: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Source));
	DEBUG ("[mRAL]: Link_Going_Down - Destination: ");
	print_mihf_id((char *) &(msgToBuild->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
    DEBUG ("[mRAL]:   LINK_TUPLE_ID - Type: %d \n", msgToBuild->Link_Tuple_Id_3GPP.Type);
    print_link_tuple_id_3GPP((char *) &(msgToBuild->Link_Tuple_Id_3GPP));
    DEBUG ("[mRAL]:   TIME INTERVAL - Type: %d - Value: %d\n", msgToBuild->Time_Interval.Type, msgToBuild->Time_Interval.Time_Interval_Value);
    DEBUG ("[mRAL]:   GOING DOWN REASON - Type %d -  ", msgToBuild->Link_Going_Down_Reason.Type);
    switch(msgToBuild->Link_Going_Down_Reason.Link_Going_Down_Reason_Value)
    {
    	case LINK_GOING_DOWN_REASON_EXPLICIT_DISCONNECT: DEBUG (" Explicit Disconnect \n"); break;
    	case LINK_GOING_DOWN_REASON_LINK_PARAM_DEGRADING: DEBUG (" Link Parameter degrading \n"); break;
    	case LINK_GOING_DOWN_REASON_LOW_POWER: DEBUG(" Low Power \n"); break;
    	case LINK_GOING_DOWN_REASON_NO_RESOURCE: DEBUG(" No Resource \n"); break;
    	default: DEBUG (" Not recognized \n"); break;
    }
	#endif
    DEBUG ("\n");
    DEBUG ("[mRAL]: mRALte_send_link_going_down - End \n\n");
    return done;
}

//---------------------------------------------------------------------------
int mRALte_send_link_parameters_report_indication(void){
//---------------------------------------------------------------------------
    char str[MIHLink_MAX_LENGTH];
    int prim_length=0, done=0, i=0;

    MIH_C_Link_Parameters_Report_Indication *msgToBuild;

    bzero(str,MIHLink_MAX_LENGTH);
    prim_length =  sizeof(MIH_C_Link_Parameters_Report_Indication);
    msgToBuild = (MIH_C_Link_Parameters_Report_Indication *) str;

    msgToBuild->Header.Header_Vers = PROTOCOL_VERSION;
    msgToBuild->Header.Header_FN = MIH_LINK_HEADER_2;
    msgToBuild->Header.Message_ID = MIH_LINK_HEADER_3_Link_Param_Report;
    msgToBuild->Header.Transaction_ID = trans_id;
    msgToBuild->Header.Payload_Length = (sizeof(MIH_C_Link_Parameters_Report_Indication)-sizeof(MIH_C_Header)) << 8;

    // TLV Source
    msgToBuild->MIHF_Source.Type = TLV_MIHF_ID_SOURCE;
    msgToBuild->MIHF_Source.Length = sizeof(MIH_C_Link_ID_Name_Link);
    msgToBuild->MIHF_Source.Link_ID_Name_Link.Length= sizeof(MIH_C_Link_ID_Name_Link)-1;
    strcpy(msgToBuild->MIHF_Source.Link_ID_Name_Link.Link_ID_Char, MIH_SOURCE_ID);

    // TLV Destination
    msgToBuild->MIHF_Destination.Type = TLV_MIHF_ID_DESTINATION;
    msgToBuild->MIHF_Destination.Length = sizeof(MIH_C_Link_ID_Name_Mihf);
    msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Length= sizeof(MIH_C_Link_ID_Name_Mihf)-1;
    strcpy(msgToBuild->MIHF_Destination.Link_ID_Name_Mihf.Link_ID_Char, MIH_DESTINATION_ID);

    //TLV Link
    msgToBuild->Link_Tuple_Id_3GPP.Type = TLV_LINK_TUPLE_ID;
    msgToBuild->Link_Tuple_Id_3GPP.Length = sizeof(MIH_C_Link_ID_3GPP)+sizeof(MIH_C_Choice);
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.LinkType = LINK_TYPE_3GPP;
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Choice_Address = CHOICE_ADDRESS_3GPP;
    msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP.Addr_Val_List_Length = sizeof(MIH_C_Address_3GPP)-1;
    for(i=0; i<8; i++) {msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP.Addr_Val_3GPP[i] = val[i];}
    //load_3gpp_address((char*) &(msgToBuild->Link_Tuple_Id_3GPP.Link_ID_3GPP.Address_3GPP));
    msgToBuild->Link_Tuple_Id_3GPP.Choice = CHOICE_NULL;

    //TLV Link Param RPT List
    msgToBuild->Link_Param_Rpt_List.Type = TLV_LINK_PARAM_RPT_LIST;
    msgToBuild->Link_Param_Rpt_List.Length = sizeof(MIH_C_Link_Param_Rpt_List)-2;
    msgToBuild->Link_Param_Rpt_List.Link_Param_Rpt_List_Value.List_Length = LIST_PARAM_RPT_LENGTH_VALUE;
    msgToBuild->Link_Param_Rpt_List.Link_Param_Rpt_List_Value.Link_Param_Rpt[0].Link_Param.Param_Type_Choice = LINK_PARAM_GEN;
    //msgToBuild->Link_Param_Rpt_List.Link_Param_Rpt_List_Value.Link_Param_Rpt[0].Link_Param.Link_Param_Type_Num = LINK_PARAM_GEN_VAL;
    msgToBuild->Link_Param_Rpt_List.Link_Param_Rpt_List_Value.Link_Param_Rpt[0].Link_Param.Link_Param_Type_Num = ralupriv->integrated_meas_level[0];
    msgToBuild->Link_Param_Rpt_List.Link_Param_Rpt_List_Value.Link_Param_Rpt[0].Link_Param.Choice = CHOICE_LINK_PARAM_VAL;
    msgToBuild->Link_Param_Rpt_List.Link_Param_Rpt_List_Value.Link_Param_Rpt[0].Link_Param.Link_Param_Value = LINK_PARAM_VAL_VALUE;
    msgToBuild->Link_Param_Rpt_List.Link_Param_Rpt_List_Value.Link_Param_Rpt[0].Link_Param.Choice = CHOICE_NULL;

    if (send_to_mih(str,prim_length)<0){
            perror("[mRAL]: mRALte_send_link_parameters_report_indication");
            wait_start_mihf = 1;
    }

    ralupriv->pending_req_transaction_id = 0;
    ralupriv->pending_req_status = 0;

    DEBUG ("[mRAL]: mRALte send link parameters report indication - SENT \n\n");

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) msgToBuild);
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   LINK_TUPLE_ID - Type: %d \n", msgToBuild->Link_Tuple_Id_3GPP.Type);
	print_link_tuple_id_3GPP((char *) &(msgToBuild->Link_Tuple_Id_3GPP));
	#endif

	DEBUG ("[mRAL]: mRALte send link parameters report indication - End \n\n");
    return done;
}

/***************************************************************************
     Reception side
 ***************************************************************************/

//#ifdef UPDATE
//-----------------------------------------------------------------------------
void mRALte_decode_link_get_parameters_request(char* str, int message_length)
//-----------------------------------------------------------------------------
{
	MIH_C_ODTONE_Message  *p;
    char msg_src[32];
    char msg_dst[32];

	p = (MIH_C_ODTONE_Message *) str;

	Get_Parameters_Req_msg = &(Get_Parameters_Req);

    int i, last_val;
	trans_id = p->Header.Transaction_ID;

    get_mihf_id((char *) &(p->MIHF_Source), msg_src);
    get_mihf_id((char *) &(p->MIHF_Destination), msg_dst);

	#ifdef OUTPUT_LEVEL_1
	print_header((char *) p);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link Get Parameters Request - Source: %s\n", msg_src);
	print_mihf_id((char *) &(p->MIHF_Source));
	DEBUG ("[mRAL]: Link Get Parameters Request - Destination: %s\n", msg_dst);
	print_mihf_id((char *) &(p->MIHF_Destination));
	DEBUG ("\n");
	#endif

	Get_Parameters_Req_msg->Header = p->Header;
	Get_Parameters_Req_msg->MIHF_Source	= p->MIHF_Source;
	Get_Parameters_Req_msg->MIHF_Destination = p->MIHF_Destination;

	if(p->variablePayload[0] == TLV_LIST_LINK_PARAM_TYPE)
	{
		DEBUG ("[mRAL]:    LINK PARAMETER TYPE LIST - Type: %d \n", p->variablePayload[0]);
		Get_Parameters_Req_msg->Link_Param_Type_List.Type = p->variablePayload[0];
		Get_Parameters_Req_msg->Link_Param_Type_List.Length = p->variablePayload[1];
		DEBUG ("[mRAL]:        - Number of elements: %d \n", p->variablePayload[2]);
		Get_Parameters_Req_msg->Link_Param_Type_List.List_Length = p->variablePayload[2];
	}
	else
	{
		DEBUG ("[mRAL]:    TLV not recognized! \n");
	}

	last_val=0;
	for(i=0; i<p->variablePayload[2]; i++)
	{
		Get_Parameters_Req_msg->Link_Param_Type_List.Link_Param_Type[i].Choice = p->variablePayload[i*2+3];
		Get_Parameters_Req_msg->Link_Param_Type_List.Link_Param_Type[i].Link_Param_Type_Value = p->variablePayload[i*2+4];

		DEBUG ("[mRAL]:        ");
	    switch(Get_Parameters_Req_msg->Link_Param_Type_List.Link_Param_Type[i].Choice)
	    {
	    	case LINK_PARAM_GEN: DEBUG ("- Generic Link Parameters (%hhx) ", Get_Parameters_Req_msg->Link_Param_Type_List.Link_Param_Type[i].Choice);
					switch(Get_Parameters_Req_msg->Link_Param_Type_List.Link_Param_Type[i].Link_Param_Type_Value)
					{
						case DATA_RATE: DEBUG (" - DATA RATE \n"); break;
						case SIG_STRENGTH: DEBUG (" - SIG_STRENGTH \n"); break;
						case SINR: DEBUG (" - SINR \n"); break;
						case THROUGHPUT: DEBUG (" - THROUGHPUT \n"); break;
						case PACKET_ERROR_RATE: DEBUG (" - PACKET ERROR RATE \n"); break;
						default: DEBUG (" - Add the specific Parameter Type Value"); break;
					}
				break;
	    	case LINK_PARAM_QOS: DEBUG (" - Quality of Service Parameters (%hhx) ", Get_Parameters_Req_msg->Link_Param_Type_List.Link_Param_Type[i].Choice);
				switch(Get_Parameters_Req_msg->Link_Param_Type_List.Link_Param_Type[i].Link_Param_Type_Value)
					{
						case MAX_NUM_COS: DEBUG (" - MAX NUM COS \n"); break;
						case MIN_PACKET_TRANSFER_RATE: DEBUG (" - MIN PACKET TRANSFER RATE \n"); break;
						case AVERAGE_PACKET_TRANSFER_DELAY: DEBUG (" - AVERAGE PACKET TRANSFER DELAY \n"); break;
						case MAXIMUM_PACKET_TRASFER_DELAY: DEBUG (" - MAXIMUM PACKET TRASFER DELAY \n"); break;
						case PACKET_TRASFER_DELAY_JITTER: DEBUG (" - PACKET TRASFER DELAY JITTER \n"); break;
						case PACKET_LOSS_RATE: DEBUG (" - PACKET LOSS RATE \n"); break;
						default: DEBUG ("	- Add the specific Parameter Type Value"); break;
					}
			break;
	    	default: DEBUG ("	- Add the specific Parameter Type"); break;
	    }
	    last_val= i*2+4;
	}

	//mRALte_send_link_get_parameters_confirm();
	last_val++;

	//DEBUG ("VALORE last_val %d \n", last_val);

	//DEBUG ("variablePayload[last_val] %d \n", p->variablePayload[last_val]);

	if(p->variablePayload[last_val] == TLV_LINK_STATES_REQ) //{DEBUG (" TLV_LINK_STATES_REQ \n");}
	{
		//DEBUG ("variablePayload[last_val] %d \n", p->variablePayload[last_val]);
		Get_Parameters_Req_msg->Link_States_Req.Type = p->variablePayload[last_val]; //p->variablePayload[last_val];
		last_val++;
		//DEBUG ("TLV_LINK_STATES_REQ 1 \n");
		Get_Parameters_Req_msg->Link_States_Req.Length = p->variablePayload[last_val];
		Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value = 0;
		last_val=last_val+2;
		//DEBUG ("TLV_LINK_STATES_REQ 2 \n");
		Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value = p->variablePayload[last_val];
		//DEBUG ("TLV_LINK_STATES_REQ 3 \n");

		//DEBUG ("Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value %hhx \n", Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value);
		//DEBUG ("variablePayload[last_val] %hhx \n", p->variablePayload[last_val]);


		//DEBUG ("[mRAL]:    LINK STATES REQUEST - Type: %d\n", Get_Parameters_Req_msg->Link_States_Req.Type);
	    //DEBUG ("[mRAL]:		- Link States to be requested: ");

		DEBUG ("[mRAL]:    LINK STATES REQUEST - Type: %d \n", Get_Parameters_Req_msg->Link_States_Req.Type);
		DEBUG ("[mRAL]:        ");
	    switch(Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value)
	    {
	    	case 0: DEBUG ("None \n"); break;
	    	case 1: DEBUG ("Operational Mode (OP_MODE) \n"); break;
	    	case 2: DEBUG ("Channel ID (CHANNEL_ID) \n"); break;
	    	case 3: DEBUG ("Operational Mode (OP_MODE) - Channel ID (CHANNEL_ID)\n"); break;
	    	default: DEBUG ("Value not recognized!"); break;
	    }
	    //Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value = Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value >> 1;
	    //if(Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value | OP_MODE){DEBUG ("Operational Mode (OP_MODE) - ");}
	    //if(Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value | CHANNEL_ID){DEBUG ("Channel ID (CHANNEL_ID) - ");}
	    DEBUG ("\n");
	}
	else
	{
		DEBUG ("[mRAL]:    TLV not recognized! \n");
	}
    last_val++;

	if(p->variablePayload[last_val] == TLV_LINK_DESC_REQ)
	{
		Get_Parameters_Req_msg->Link_Desc_Req.Type = p->variablePayload[last_val];
		last_val++;
		Get_Parameters_Req_msg->Link_Desc_Req.Length = p->variablePayload[last_val];
		last_val=last_val+2;
		Get_Parameters_Req_msg->Link_Desc_Req.Link_Desc_Req_Value = p->variablePayload[last_val];

	    DEBUG ("[mRAL]:    LINK DESCRIPTORS REQUEST - Type: %d \n", Get_Parameters_Req_msg->Link_Desc_Req.Type);
	    DEBUG ("[mRAL]:		 - Link Descriptors Request : \n");

	    //DEBUG ("[mRAL]:    Get_Parameters_Req_msg->Link_Desc_Req.Link_Desc_Req_Value: %d \n", Get_Parameters_Req_msg->Link_Desc_Req.Link_Desc_Req_Value);

	    switch(Get_Parameters_Req_msg->Link_Desc_Req.Link_Desc_Req_Value)
	    {
	    	case 0: DEBUG ("[mRAL]:		      None \n"); break;
	    	case 1: DEBUG ("[mRAL]:		      Number of Classes of Service Supported  (CLASSES_SERVICE) \n"); break;
	    	case 2: DEBUG ("[mRAL]:		      Number of Queues Supported (QUEUES_SUPPORTED) \n"); break;
	    	case 3: DEBUG ("[mRAL]:		      Number of Classes of Service Supported  (CLASSES_SERVICE) \n"); DEBUG ("[mRAL]:		      Number of Queues Supported (QUEUES_SUPPORTED)\n"); break;
	    }
	    //Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value = Get_Parameters_Req_msg->Link_States_Req.Link_States_Req_Value << 8;
	}
	else
	{
		DEBUG ("[mRAL]:    TLV not recognized! \n");
	}

	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Link Get Parameters Request - Message content: \n");
	mRALu_print_buffer((char *) Get_Parameters_Req_msg, sizeof(MIH_C_Get_Parameters_Req));
    RAL_print_buffer((char *) Get_Parameters_Req_msg, sizeof(MIH_C_Get_Parameters_Req));
	#endif

    NOTICE("[MSC_MSG][%s][%s][--- Link_Get_Parameters.request --->][%s]\n",
           getTimeStamp4Log(),
           msg_src,
           msg_dst);

    DEBUG ("[mRAL]: Link Get Parameters Request - End \n\n");
	//mRALte_send_link_get_parameters_confirm_test();
	mRALte_send_link_get_parameters_confirm_test();
	sleep(1);
}
//#endif

//-----------------------------------------------------------------------------
void mRALte_decode_link_capability_discover_request(char* str, int message_length)
//-----------------------------------------------------------------------------
{
	MIH_C_Capability_Discover_Req *p;
    char msg_src[32];
    char msg_dst[32];
	p = (MIH_C_Capability_Discover_Req *) str;

	trans_id = p->Header.Transaction_ID;

	#ifdef OUTPUT_LEVEL_3
	int prim_length;
	#endif

	DEBUG ("[mRAL]: Capability_Discover_Request - RECEIVED - Begin \n\n");

    get_mihf_id((char *) &(p->MIHF_Source), msg_src);
    get_mihf_id((char *) &(p->MIHF_Destination), msg_dst);
	#ifdef OUTPUT_LEVEL_1
	print_header((char *) p);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Capability_Discover_Request - Source: %s\n", msg_src);
	print_mihf_id((char *) &(p->MIHF_Source));
	DEBUG ("[mRAL]: Capability_Discover_Request - Destination: %s\n", msg_dst);
	print_mihf_id((char *) &(p->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_3
	prim_length = p->Header.Payload_Length/CONVERSION;
	DEBUG ("[mRAL]: Capability_Discover_Request - Message content: \n");
	mRALu_print_buffer(str, prim_length);
    RAL_print_buffer(str, message_length);
	#endif

    NOTICE("[MSC_MSG][%s][%s][--- Link_Capability_Discover.request --->][%s]\n",
           getTimeStamp4Log(),
           msg_src,
           msg_dst);
	DEBUG ("[mRAL]: Capability_Discover_Request - End \n\n");

	mRALte_send_capability_discover_confirm();
}

//-----------------------------------------------------------------------------
void mRALte_decode_link_configure_threshold_request(char* str, int message_length)
//-----------------------------------------------------------------------------
{
	int i;
    char msg_src[32];
    char msg_dst[32];
	MIH_C_Link_Configure_Thresholds_Req *p;
	p = (MIH_C_Link_Configure_Thresholds_Req *) str;

	trans_id = p->Header.Transaction_ID;
	rcv_Link_Cfg_Param_List = &(p->Link_Cfg_Param_List);

	#ifdef OUTPUT_LEVEL_3
	int prim_length;
	#endif

	DEBUG ("[mRAL]: Link Configure Thresholds Req - RECEIVED: \n\n");

    get_mihf_id((char *) &(p->MIHF_Source), msg_src);
    get_mihf_id((char *) &(p->MIHF_Destination), msg_dst);
	#ifdef OUTPUT_LEVEL_1
	print_header((char *) p);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link Configure Thresholds Req - Source: %s\n", msg_src);
	print_mihf_id((char *) &(p->MIHF_Source));
	DEBUG ("[mRAL]: Link Configure Thresholds Req - Destination: %s\n", msg_dst);
	print_mihf_id((char *) &(p->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:    LINK CFG PARAMETERS LIST - TLV: %d \n", p->Link_Cfg_Param_List.Type);

	for(i=0; i<p->Link_Cfg_Param_List.List_Length; i++)
	{
		DEBUG ("[mRAL]: 	LINK CFG PARAM - %d \n", (i+1));
		print_link_cfg_param((char *) &(p->Link_Cfg_Param_List.Link_Cfg_Param[i]));
	}
	#endif

	DEBUG ("\n");
	#ifdef OUTPUT_LEVEL_3
	prim_length = sizeof(MIH_C_Link_Configure_Thresholds_Req);
	DEBUG ("[mRAL]: Link Configure Thresholds Req - Message content: \n");
	mRALu_print_buffer((char *) p, prim_length);
    RAL_print_buffer((char *) p, prim_length);
	#endif

    NOTICE("[MSC_MSG][%s][%s][--- Link_Configure_Thresholds.request --->][%s]\n",
           getTimeStamp4Log(),
           msg_src,
           msg_dst);
	DEBUG ("[mRAL]: Link Configure Thresholds Req - End \n\n");

	mRALte_send_link_configure_thresholds_confirm();

	/*
	if(ralu_priv->event |)
	{
		mRALte_send_link_configure_thresholds_confirm();
	}
	else
	{DEBUG ("[mRAL]: Event not subscribed \n\n");}

	*/
}

//-----------------------------------------------------------------------------
void mRALte_decode_event_subscribe_request(char * str, int message_length)
//-----------------------------------------------------------------------------
{
    char msg_src[32];
    char msg_dst[32];
    char evt_list[128];
	MIH_C_Event_Subscribe_Req *p;
	p = (MIH_C_Event_Subscribe_Req *) str;

	trans_id = p->Header.Transaction_ID;

	#ifdef OUTPUT_LEVEL_3
	int prim_length;
	#endif


    get_mihf_id((char *) &(p->MIHF_Source), msg_src);
    get_mihf_id((char *) &(p->MIHF_Destination), msg_dst);
	#ifdef OUTPUT_LEVEL_1
	print_header((char *) p);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Event Subscribe Request - Source: ");
	print_mihf_id((char *) &(p->MIHF_Source));
	DEBUG ("[mRAL]: Event Subscribe Request - Destination: ");
	print_mihf_id((char *) &(p->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:		MIH_EVT_LIST - Type %d \n", p->Link_Evt_List.Type);
	print_link_evt_list((char *)&(p->Link_Evt_List));
	#endif

	#ifdef OUTPUT_LEVEL_3
	prim_length = sizeof(MIH_C_Event_Subscribe_Req);
	DEBUG ("[mRAL]: Event Subscribe Request - Message content: \n");
	mRALu_print_buffer((char *) p, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

    evt_list[0] = 0;
    if((p->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_DETECTED){ strcat(evt_list," Link_Detected");}
    if((p->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_UP){ strcat(evt_list," Link_Up");}
    if((p->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_DOWN){strcat(evt_list," Link_Down");}
    if((p->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_PARAMETERS_REPORT){strcat(evt_list," Link_Param_Reports");}
    if((p->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_GOING_DOWN){strcat(evt_list," Link_Going_Down");}
    if((p->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_HANDOVER_IMMINENT){strcat(evt_list," Link_Handover_Imminent");}
    if((p->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_LINK_HANDOVER_COMPLETE){strcat(evt_list," Link_Handover_Complete");}
    if((p->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3]) & EVT_LIST_PDU_TRANSMIT_STATUS){strcat(evt_list," Link_PDU_Transmit_Status");}
    DEBUG ("\n\n");

    NOTICE("[MSC_MSG][%s][%s][--- Link_Event_Subscribe.request\\n%s --->][%s]\n",
           getTimeStamp4Log(),
           msg_src,evt_list,
           msg_dst);
	DEBUG ("[mRAL]: Event Subscribe Request - End \n\n");

	ralupriv->event = p->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3];

	rcv_evt_list = p->Link_Evt_List.Evt_List_Data.Evt_List_Oct[3];

	mRALte_send_event_subscribe_confirm();
}

//-----------------------------------------------------------------------------
void mRALte_decode_link_action_request(char * str, int message_length)
//-----------------------------------------------------------------------------
{
    char msg_src[32];
    char msg_dst[32];
    char action [64];
    action[0] = 0;
	MIH_C_Link_Action_Request *p;
	p = (MIH_C_Link_Action_Request *) str;

	trans_id = p->Header.Transaction_ID;

    get_mihf_id((char *) &(p->MIHF_Source), msg_src);
    get_mihf_id((char *) &(p->MIHF_Destination), msg_dst);
	#ifdef OUTPUT_LEVEL_1
	print_header((char *) p);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link Action Request - Source: ");
	print_mihf_id((char *) &(p->MIHF_Source));
	DEBUG ("[mRAL]: Link Action Request - Destination: ");
	print_mihf_id((char *) &(p->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:    LINK ACTION TYPE: ");
	switch(p->Link_Action.Link_Action_Value.Link_Ac_Type)
	{
		case(LINK_AC_TYPE_NONE): DEBUG (" None (0)\n"); strcat(action, "Action None ");break;
		case(LINK_AC_TYPE_LINK_DISCONNECT): DEBUG (" Link_Disconnect (1) \n"); strcat(action, "Link_Disconnect ");break;
		case(LINK_AC_TYPE_POWER_DOWN):
			#ifdef UPDATE
			if(ralu_priv->command |LINK_AC_TYPE_POWER_DOWN)
			{
				lpd = LINK_AC_TYPE_POWER_DOWN; DEBUG (" Link_Power_Down (3)\n");
                 strcat(action, "Link_Power_Down ");
			}
			else {DEBUG ("[mRAL]: command POWER DOWN not available \n\n");}
			#endif
			lpd = LINK_AC_TYPE_POWER_DOWN; DEBUG (" Link_Power_Down (3)\n"); break; 	//to comment when update defined
		case(LINK_AC_TYPE_POWER_UP):
			#ifdef UPDATE
			if(ralu_priv->command | LINK_AC_TYPE_POWER_DOWN)
			{
				lpd = LINK_AC_TYPE_POWER_UP; DEBUG (" Link_Power_Up (4)\n");
                strcat(action, "Link_Power_Up ");
			}
			else {DEBUG ("[mRAL]: command POWER UP not available \n\n");}
			#endif
		    lpd = LINK_AC_TYPE_POWER_UP; DEBUG (" Link_Power_Up (4)\n"); break;      //to comment when UPDATE defined
		default: DEBUG (" Not recognized! \n");strcat(action, "Action Not recognized "); break;
	}
	DEBUG ("[mRAL]:    LINK ACTION ATTRIBUTE: ");
	switch(p->Link_Action.Link_Action_Value.Link_Ac_Attr)
	{
		case(LINK_SCAN): link_scan = LINK_SCAN; DEBUG (" Link Scan (1)\n\n"); break;
		default: link_scan = 0; DEBUG (" None \n\n"); break;
	}
	#endif

	#ifdef OUTPUT_LEVEL_3
	DEBUG ("[mRAL]: Link Action Request - Message content: \n");
	mRALu_print_buffer((char *) p, sizeof(MIH_C_Link_Action_Request)+30);
    RAL_print_buffer((char *) p, sizeof(MIH_C_Link_Action_Request)+30);
	#endif
    NOTICE("[MSC_MSG][%s][%s][--- Link_Action.request\\n%s --->][%s]\n",
           getTimeStamp4Log(),
           msg_src, action,
           msg_dst);

	DEBUG ("[mRAL]: Link Action Request - End \n\n");

	#ifdef UPDATE
	for(i=0; i<LINK_STATES_RSP_LIST_LENGTH; i++) //to update when the parameters used to set LIST_LENGTH will be fixed;
	{
		rcv_Link_Action_Rsp_List->Link_Scan_Rsp_List.Link_Scan_Rsp[i].Choice = LINK_ADDR_3GPP_3G_CELL_ID;
		rcv_Link_Action_Rsp_List->Link_Scan_Rsp_List.Link_Scan_Rsp[i].Network_Id.Network_Id_Length = MIH_NETWORK_ID_LENGTH;
		strcpy(rcv_Link_Action_Rsp_List->Link_Scan_Rsp_List.Link_Scan_Rsp[i].Network_Id.Network_Id_Value , MIH_NETWORK_ID);
		rcv_Link_Action_Rsp_List->Link_Scan_Rsp_List.Link_Scan_Rsp[i].Link_Sig_Strength.Choice = CHOICE_NULL;
	}

	// to set confirm directly
	switch(p->Link_Action.Link_Action_Value.Link_Ac_Attr)
	{
		case(LINK_SCAN): mRALte_send_link_action_confirm_lsr(); break;
		default: mRALte_send_link_action_confirm(); break;
	}
	#endif
	//

    //get l2-ID and check if activation or deactivation
    //ralupriv->cell_id = p->newPoA.value[0];
    //DEBUG("requested cell_id %d , current state %d\n", ralupriv->cell_id , ralupriv->state);


	switch(p->Link_Action.Link_Action_Value.Link_Ac_Type)
	{
		case(LINK_AC_TYPE_NONE): DEBUG (" No action \n"); break;
		case(LINK_AC_TYPE_LINK_DISCONNECT): DEBUG (" no action \n"); break;
		case(LINK_AC_TYPE_POWER_DOWN):
			  #ifdef UPDATE
					if(ralu_priv->event| LINK_AC_TYPE_POWER_DOWN)
					{
			    	      DEBUG("Cell_ID = 0, Deactivation requested to NAS interface\n");
			    	      IAL_process_DNAS_message(IO_OBJ_CNX, IO_CMD_DEL, ralupriv->cell_id);
			    	      lpd = LINK_AC_TYPE_POWER_DOWN;
					}
					else {DEBUG ("[mRAL]: command POWER DOWN not available \n\n");}
			  #endif
    	      DEBUG("Cell_ID = 0, Deactivation requested to NAS interface\n");
    	      IAL_process_DNAS_message(IO_OBJ_CNX, IO_CMD_DEL, ralupriv->cell_id);
    	      lpd = LINK_AC_TYPE_POWER_DOWN;
    	      break;
		case(LINK_AC_TYPE_POWER_UP):
    	      // Activation requested - check it is not already active
				#ifdef UPDATE
						if(ralu_priv->event| LINK_AC_TYPE_POWER_DOWN)
						{
				    	      if (ralupriv->state == CONNECTED){
				    	        DEBUG("Cell_ID != 0, Activation requested, but interface already active ==> NO OP\n");
				    	        ralupriv->pending_req_status = 0;
				    	        mRALte_send_link_action_confirm();
				    	      }else{
				    	        DEBUG("Cell_ID != 0, Activation requested to NAS interface\n");
				    	        IAL_process_DNAS_message(IO_OBJ_CNX, IO_CMD_ADD, ralupriv->cell_id);
				    	      }
							lpd = LINK_AC_TYPE_POWER_UP;
						}
						else {DEBUG ("[mRAL]: command POWER DOWN not available \n\n");}
				#endif
    	      if (ralupriv->state == CONNECTED){
    	        DEBUG("Cell_ID != 0, Activation requested, but interface already active ==> NO OP\n");
    	        ralupriv->pending_req_status = 0;
    	        mRALte_send_link_action_confirm();
    	      }else{
    	        DEBUG("Cell_ID != 0, Activation requested to NAS interface\n");
    	        IAL_process_DNAS_message(IO_OBJ_CNX, IO_CMD_ADD, ralupriv->cell_id);
    	      }
			lpd = LINK_AC_TYPE_POWER_UP;
		break;
		default: DEBUG (" Not recognized! \n"); break;
	}
}

//---------------------------------------------------------------------------
// Decode_event_unsubscribe_request
void mRALte_decode_event_unsubscribe_request(char * str, int message_length)
//---------------------------------------------------------------------------
{
    char msg_src[32];
    char msg_dst[32];
	MIH_C_Event_Unsubscribe_Req *p;
	p = (MIH_C_Event_Unsubscribe_Req *) str;

	trans_id = p->Header.Transaction_ID;

	#ifdef OUTPUT_LEVEL_3
	int prim_length=0;
	#endif
    get_mihf_id((char *) &(p->MIHF_Source), msg_src);
    get_mihf_id((char *) &(p->MIHF_Destination), msg_dst);
	#ifdef OUTPUT_LEVEL_1
	print_header((char *) p);
	#endif

	#ifdef OUTPUT_LEVEL_2
	DEBUG ("[mRAL]: Link Event Unsubscribe Request - Source: %s\n", msg_src);
	print_mihf_id((char *) &(p->MIHF_Source));
	DEBUG ("[mRAL]: Link Event Unsubscribe Request - Destination: %s\n", msg_dst);
	print_mihf_id((char *) &(p->MIHF_Destination));
	DEBUG ("\n");
	#endif

	#ifdef OUTPUT_LEVEL_1
	DEBUG ("[mRAL]:   EVT_LIST - Type: %d \n", p->Requested_MIH_Evt_List.Type);
	print_link_evt_list((char *) &(p->Requested_MIH_Evt_List));
	#endif

	#ifdef OUTPUT_LEVEL_3
	prim_length = sizeof(MIH_C_Event_Unsubscribe_Req);
	DEBUG ("[mRAL]: Link Event Unsubscribe Request - Message content: \n");
	mRALu_print_buffer((char *) p, prim_length);
    RAL_print_buffer(str, prim_length);
	#endif

    NOTICE("[MSC_MSG][%s][%s][--- Link_Event_Unsubscribe.request --->][%s]\n",
           getTimeStamp4Log(),
           msg_src,
           msg_dst);
	DEBUG ("[mRAL]: Link Event Unsubscribe Request - End \n\n");

	rcv_evt_list = p->Requested_MIH_Evt_List.Evt_List_Data.Evt_List_Oct[3];

	mRALte_send_event_unsubscribe_confirm();
}

/*-----------------------------------------------------------------------------
// Process incoming messages from MIH
int process_mih_command(void){
//-----------------------------------------------------------------------------
	MIH_C_ODTONE_Message *ODTONE_Message;

	int done=0, n=0, prim_length;

	char str[MIHLink_MAX_LENGTH];
	bzero(str,MIHLink_MAX_LENGTH);

	prim_length =  sizeof(MIH_C_ODTONE_Message);


	n = recvfrom(sd_graal, (void *)str, prim_length, 0, (struct sockaddr *) &udp_socket, &sockaddr_len);

    ODTONE_Message = (MIH_C_ODTONE_Message *) str;

    switch(ODTONE_Message->Header.Message_ID){
    	case MIH_LINK_HEADER_3_Cap_Disc_Req:
            DEBUG ("[mRAL]: Event Capability Dicover Request (%d Bytes)- RECEIVED - Begin \n\n", n);
    		mRALte_decode_link_capability_discover_request(str, n);
    		break;
    	case MIH_LINK_HEADER_3_Evt_Sub_Req:

    		DEBUG ("[mRAL]: Event Subscribe Request (%d Bytes)- RECEIVED - Begin \n\n", n);
    		mRALte_decode_event_subscribe_request(str, n);
    		break;
    	case MIH_LINK_HEADER_3_Evt_Unsub_Req:
    		DEBUG ("[mRAL]: Event Unsubscribe Request (%d Bytes)- RECEIVED - Begin \n\n", n);
    		mRALte_decode_event_unsubscribe_request(str, n);
    		break;
    	case MIH_LINK_HEADER_3_Link_Conf_Thres_Req:
            DEBUG ("[mRAL]: Event Link configure Threshold Request (%d Bytes)- RECEIVED - Begin \n\n", n);
    		mRALte_decode_link_configure_threshold_request(str, n);
    		break;
    	case MIH_LINK_HEADER_3_Get_Param_Req:
    		DEBUG ("[mRAL]: Link Get Parameters Request (%d Bytes)- RECEIVED - Begin \n\n", n);
    		mRALte_decode_link_get_parameters_request(str, n);
    		sleep(2);
    		if(link_detec == 1)
    		{
    			mRALte_send_link_detected_indication();
    		}
    		break;
    	case MIH_LINK_HEADER_3_Link_Act_Req:
    		DEBUG ("[mRAL]: Link Action Request (%d Bytes)- RECEIVED - Begin \n\n", n);
    		prim_length= sizeof(MIH_C_Link_Action_Request);
    		mRALte_decode_link_action_request(str, n);
    		DEBUG (" in mRALuD_msg after mRALtle_decode \n\n");
    		break;
    	default: DEBUG ("Message not recognized \n");
    }
    return done;
}*/

