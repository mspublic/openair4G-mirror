#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "RRC2RRMMessageAddUserRequest.h"
#include "RRM2RRCMessageAddUserResponse.h"
#include "RRCMessageHandler.h"
#include "RadioResources.h"
#include "Exceptions.h"

//----------------------------------------------------------------------------
RRC2RRMMessageAddUserRequest::RRC2RRMMessageAddUserRequest(struct sockaddr *sa_fromP, socklen_t sa_lenP, RRC_RRM_Message_t* asn1_messageP)
//----------------------------------------------------------------------------
{
    Message();
    ParseIpParameters(sa_fromP, sa_lenP);
    m_asn1_message   = asn1_messageP;
    m_transaction_id = m_asn1_message->message.choice.c1.choice.rrcAddUserRequest.transactionId;
    m_cell_id        = m_asn1_message->message.choice.c1.choice.rrcAddUserRequest.e_NodeB_Identity.physCellId;
    m_mobile_id      = m_asn1_message->message.choice.c1.choice.rrcAddUserRequest.c_RNTI;
}
//----------------------------------------------------------------------------
RRC2RRMMessageAddUserRequest::RRC2RRMMessageAddUserRequest(std::string ip_dest_strP, int port_destP, cell_id_t cell_idP, mobile_id_t mobile_idP, transaction_id_t transaction_idP)
//----------------------------------------------------------------------------
{
    Message();
    m_asn1_message = static_cast<RRC_RRM_Message_t*>(CALLOC(1,sizeof(RRC_RRM_Message_t)));
    memset(m_asn1_message, 0, sizeof(RRC_RRM_Message_t));
    m_asn1_message->message.present                                                        = RRC_RRM_MessageType_PR_c1;
    m_asn1_message->message.choice.c1.present                                              = RRC_RRM_MessageType__c1_PR_rrcAddUserRequest;
    m_asn1_message->message.choice.c1.choice.rrcAddUserRequest.transactionId               = transaction_idP;
    m_asn1_message->message.choice.c1.choice.rrcAddUserRequest.e_NodeB_Identity.physCellId = cell_idP;
    m_asn1_message->message.choice.c1.choice.rrcAddUserRequest.c_RNTI                      = mobile_idP;


    m_transaction_id = transaction_idP;
    m_cell_id        = cell_idP;
    m_mobile_id      = mobile_idP;
    m_is_ipv6        = Utility::isipv6(ip_dest_strP);
    m_msg_length     = 0;
    Serialize();
}
//----------------------------------------------------------------------------
void RRC2RRMMessageAddUserRequest::Forward()
//----------------------------------------------------------------------------
{
    try {
        RRM2RRCMessageAddUserResponse* add_user_response = RadioResources::Instance()->Request(*this);
        RRCMessageHandler::Instance()->Send2Peer(this->m_ip_str_src, this->m_port_src, add_user_response->GetSerializedMessageBuffer(), add_user_response->GetSerializedMessageSize());
        delete add_user_response;
    } catch (mobile_already_connected_error & x ) {
        RRM2RRCMessageAddUserResponse  response(OpenAir_RRM_Response_Status_failure, OpenAir_RRM_Response_Reason_protocolError, m_cell_id, m_mobile_id, m_transaction_id);
        RRCMessageHandler::Instance()->Send2Peer(this->m_ip_str_src, this->m_port_src, response.GetSerializedMessageBuffer(), response.GetSerializedMessageSize());
    } catch (std::exception const& e ) {
        RRM2RRCMessageAddUserResponse  response(OpenAir_RRM_Response_Status_failure, OpenAir_RRM_Response_Reason_internalError, m_cell_id, m_mobile_id, m_transaction_id);
        RRCMessageHandler::Instance()->Send2Peer(this->m_ip_str_src, this->m_port_src, response.GetSerializedMessageBuffer(), response.GetSerializedMessageSize());
    }
}
//----------------------------------------------------------------------------
void RRC2RRMMessageAddUserRequest::Serialize ()
//----------------------------------------------------------------------------
{
    if (m_asn1_message != NULL) {
        asn_enc_rval_t enc_rval;
        memset(&m_message_serialize_buffer[0], 0, MESSAGE_SERIALIZE_BUFFER_SIZE);
        enc_rval = uper_encode_to_buffer(&asn_DEF_RRC_RRM_Message,
                    (void*)m_asn1_message,
                    &m_message_serialize_buffer[0],
                    MESSAGE_SERIALIZE_BUFFER_SIZE);
        printf("[RRM][RRC2RRMMessageConnectionRequest] asn_DEF_RRC_RRM_Message Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);

        m_msg_length = (enc_rval.encoded+7)/8;

        if (enc_rval.encoded==-1) {
            printf("[RRM][RRC2RRMMessageConnectionRequest] ASN1 : asn_DEF_RRC_RRM_Message encoding FAILED, EXITING\n");
            throw asn1_encoding_error();
        }
    }
}
//----------------------------------------------------------------------------
RRC2RRMMessageAddUserRequest::~RRC2RRMMessageAddUserRequest()
//----------------------------------------------------------------------------
{
    if (m_asn1_message != NULL) {
        ASN_STRUCT_FREE( asn_DEF_RRC_RRM_Message, m_asn1_message);
    }
}

