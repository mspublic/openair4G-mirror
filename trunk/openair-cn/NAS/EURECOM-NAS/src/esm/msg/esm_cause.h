/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
/*****************************************************************************
Source		esm_cause.h

Version		0.1

Date		2013/02/06

Product		NAS stack

Subsystem	EPS Session Management

Author		Frederic Maurel

Description	Defines error cause code returned upon receiving unknown,
		unforeseen, and erroneous EPS session management protocol
		data.

*****************************************************************************/
#ifndef __ESM_CAUSE_H__
#define __ESM_CAUSE_H__

/****************************************************************************/
/*********************  G L O B A L    C O N S T A N T S  *******************/
/****************************************************************************/

/*
 * Cause code used to notify that the EPS session management procedure
 * has been successfully processed
 */
#define ESM_CAUSE_SUCCESS			(-1)

/*
 * Causes related to nature of request (TS 24.301 - Annex B1)
 */
#define ESM_CAUSE_OPERATOR_DETERMINED_BARRING			8
#define ESM_CAUSE_INSUFFICIENT_RESOURCES			26
#define ESM_CAUSE_UNKNOWN_ACCESS_POINT_NAME			27
#define ESM_CAUSE_UNKNOWN_PDN_TYPE				28
#define ESM_CAUSE_USER_AUTHENTICATION_FAILED			29
#define ESM_CAUSE_REQUEST_REJECTED_BY_GW			30
#define ESM_CAUSE_REQUEST_REJECTED_UNSPECIFIED			31
#define ESM_CAUSE_SERVICE_OPTION_NOT_SUPPORTED			32
#define ESM_CAUSE_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED	33
#define ESM_CAUSE_SERVICE_OPTION_TEMPORARILY_OUT_OF_ORDER	34
#define ESM_CAUSE_PTI_ALREADY_IN_USE				35
#define ESM_CAUSE_REGULAR_DEACTIVATION				36
#define ESM_CAUSE_EPS_QOS_NOT_ACCEPTED				37
#define ESM_CAUSE_NETWORK_FAILURE				38
#define ESM_CAUSE_REACTIVATION_REQUESTED			39
#define ESM_CAUSE_SEMANTIC_ERROR_IN_THE_TFT_OPERATION		41
#define ESM_CAUSE_SYNTACTICAL_ERROR_IN_THE_TFT_OPERATION	42
#define ESM_CAUSE_INVALID_EPS_BEARER_IDENTITY			43
#define ESM_CAUSE_SEMANTIC_ERRORS_IN_PACKET_FILTER		44
#define ESM_CAUSE_SYNTACTICAL_ERROR_IN_PACKET_FILTER		45
#define ESM_CAUSE_PTI_MISMATCH					47
#define ESM_CAUSE_LAST_PDN_DISCONNECTION_NOT_ALLOWED		49
#define ESM_CAUSE_PDN_TYPE_IPV4_ONLY_ALLOWED			50
#define ESM_CAUSE_PDN_TYPE_IPV6_ONLY_ALLOWED			51
#define ESM_CAUSE_SINGLE_ADDRESS_BEARERS_ONLY_ALLOWED		52
#define ESM_CAUSE_ESM_INFORMATION_NOT_RECEIVED			53
#define ESM_CAUSE_PDN_CONNECTION_DOES_NOT_EXIST			54
#define ESM_CAUSE_MULTIPLE_PDN_CONNECTIONS_NOT_ALLOWED		55
#define ESM_CAUSE_COLLISION_WITH_NETWORK_INITIATED_REQUEST	56
#define ESM_CAUSE_UNSUPPORTED_QCI_VALUE				59
#define ESM_CAUSE_BEARER_HANDLING_NOT_SUPPORTED			60
#define ESM_CAUSE_INVALID_PTI_VALUE				81
#define ESM_CAUSE_APN_RESTRICTION_VALUE_NOT_COMPATIBLE		112

/*
 * Protocol errors (e.g., unknown message) class (TS 24.301 - Annex B2)
 */
#define ESM_CAUSE_SEMANTICALLY_INCORRECT			95
#define ESM_CAUSE_INVALID_MANDATORY_INFO			96
#define ESM_CAUSE_MESSAGE_TYPE_NOT_IMPLEMENTED			97
#define ESM_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE			98
#define ESM_CAUSE_IE_NOT_IMPLEMENTED				99
#define ESM_CAUSE_CONDITIONAL_IE_ERROR				100
#define ESM_CAUSE_MESSAGE_NOT_COMPATIBLE			101
#define ESM_CAUSE_PROTOCOL_ERROR				111

/*
 * TS 24.301 - Table 9.9.4.4.1
 * Any other value received by the UE shall be treated as cause code #34,
 * "service option temporarily out of order".
 * Any other value received by the network shall be treated as cause code #111
 * "protocol error, unspecified".
 */

/****************************************************************************/
/************************  G L O B A L    T Y P E S  ************************/
/****************************************************************************/

/****************************************************************************/
/********************  G L O B A L    V A R I A B L E S  ********************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

#endif /* __ESM_CAUSE_H__*/
