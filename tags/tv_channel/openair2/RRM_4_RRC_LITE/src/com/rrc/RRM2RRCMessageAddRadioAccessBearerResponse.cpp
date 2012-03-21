#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "RRM2RRCMessageAddRadioAccessBearerResponse.h"
#include "RadioResources.h"
#include "Exceptions.h"
#include "TestScenario.h"
//----------------------------------------------------------------------------
RRM2RRCMessageAddRadioAccessBearerResponse::RRM2RRCMessageAddRadioAccessBearerResponse(tcpip::Storage& in_messageP, msg_length_t msg_lengthP, frame_t msg_frameP, struct sockaddr *sa_fromP, socklen_t sa_lenP):Message(in_messageP)
//----------------------------------------------------------------------------
{
    ParseIpParameters(sa_fromP, sa_lenP);
    m_status         = m_message_storage.readChar();
    m_reason         = m_message_storage.readChar();
    m_cell_id        = m_message_storage.readChar();
    m_mobile_id      = m_message_storage.readChar();
    m_radio_bearer_id= m_message_storage.readChar();
    m_transaction_id = m_message_storage.readChar();
    m_transaction_result = NULL;
    cout << "----------------------------------------------------------------------------------------------------------" << endl;
    cout << "RRC\t<-------ADD DATA RADIO BEARER RESPONSE-------\tRRM  STATUS: "<< Message::StatusToString(m_status).c_str() << " REASON: "<< Message::ReasonToString(m_reason).c_str() << endl;
    cout << "----------------------------------------------------------------------------------------------------------" << endl;
    cout << "cell id = " << m_cell_id << " mobile id = " << m_mobile_id << " radio bearer id = "<< m_radio_bearer_id << " transaction id = "<< m_transaction_id << endl;
    if ((msg_lengthP - 6) > 0 ) {
        m_transaction_result = new Transaction(m_transaction_id, m_message_storage, msg_lengthP - 6);
        cout << m_transaction_result->ToString().c_str() << endl;
    }
}
//----------------------------------------------------------------------------
RRM2RRCMessageAddRadioAccessBearerResponse::RRM2RRCMessageAddRadioAccessBearerResponse(msg_response_status_t statusP,
                                                                                       msg_response_reason_t reasonP,
                                                                                       cell_id_t cell_idP,
                                                                                       mobile_id_t mobile_idP,
                                                                                       rb_id_t            radio_bearer_idP,
                                                                                       transaction_id_t transaction_idP)
//----------------------------------------------------------------------------
{
    Message();
    m_status           = statusP;
    m_reason           = reasonP;
    m_cell_id          = cell_idP;
    m_mobile_id        = mobile_idP;
    m_radio_bearer_id  = radio_bearer_idP;
    m_transaction_id   = transaction_idP;
}
//----------------------------------------------------------------------------
RRM2RRCMessageAddRadioAccessBearerResponse::RRM2RRCMessageAddRadioAccessBearerResponse(msg_response_status_t statusP,
                                                                                       msg_response_reason_t reasonP,
                                                                                       cell_id_t cell_idP,
                                                                                       mobile_id_t mobile_idP,
                                                                                       rb_id_t            radio_bearer_idP,
                                                                                       transaction_id_t transaction_idP,
                                                                                       Transaction* transactionP)
//----------------------------------------------------------------------------
{
    Message();
    m_status           = statusP;
    m_reason           = reasonP;
    m_cell_id          = cell_idP;
    m_mobile_id        = mobile_idP;
    m_radio_bearer_id  = radio_bearer_idP;
    m_transaction_id   = transaction_idP;
    m_transaction_result = transactionP;
}
//----------------------------------------------------------------------------
void RRM2RRCMessageAddRadioAccessBearerResponse::Serialize ()
//----------------------------------------------------------------------------
{
    //printf("[RRM][RRM2RRCMessageAddRadioAccessBearerResponse] Serialize () status = %d reason = %d cell id = %d mobile id = %d radio bearer id = %d transaction id = %d \n",  m_status, m_reason, m_cell_id, m_mobile_id, m_radio_bearer_id, m_transaction_id);
    int tx_packet_size = 0;
    if (m_transaction_result != NULL) {
        tx_packet_size = m_transaction_result->Size();
    }
    m_message_storage.reset();
    m_message_storage.writeChar(Message::MESSAGE_ADD_RADIO_ACCESS_BEARER_RESPONSE);
    m_message_storage.writeShort(9u+tx_packet_size);
    m_message_storage.writeChar(m_status);
    m_message_storage.writeChar(m_reason);
    m_message_storage.writeChar(m_cell_id);
    m_message_storage.writeChar(m_mobile_id);
    m_message_storage.writeChar(m_radio_bearer_id);
    m_message_storage.writeChar(m_transaction_id);
    if (m_transaction_result != NULL) {
        unsigned char packet[tx_packet_size];
        m_transaction_result->Serialize(&packet[0]);
        m_message_storage.writePacket(&packet[0], tx_packet_size);
    }
}
//----------------------------------------------------------------------------
void RRM2RRCMessageAddRadioAccessBearerResponse::Forward()
//----------------------------------------------------------------------------
{
    #ifdef TEST_SCENARIO
    TestScenario::notifyRxMessage(this);
    #endif
}
//----------------------------------------------------------------------------
RRM2RRCMessageAddRadioAccessBearerResponse::~RRM2RRCMessageAddRadioAccessBearerResponse()
//----------------------------------------------------------------------------
{
    if (m_transaction_result != NULL) {
        delete m_transaction_result;
    }
}
