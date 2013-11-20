/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		nas_message.h

Version		0.1

Date		2012/26/09

Product		NAS stack

Subsystem	Application Programming Interface

Author		Frederic Maurel

Description	Defines the layer 3 messages supported by the NAS sublayer
		protocol and functions used to encode and decode

*****************************************************************************/
#ifndef __NAS_MESSAGE_H__
#define __NAS_MESSAGE_H__

#include "commonDef.h"
#include "emm_msg.h"
#include "esm_msg.h"

/****************************************************************************/
/*********************  G L O B A L    C O N S T A N T S  *******************/
/****************************************************************************/

#define NAS_MESSAGE_SECURITY_HEADER_SIZE	6

/****************************************************************************/
/************************  G L O B A L    T Y P E S  ************************/
/****************************************************************************/

/* Structure of security protected header */
typedef struct {
#ifdef __LITTLE_ENDIAN_BITFIELD
    UInt8_t protocol_discriminator:4;
    UInt8_t security_header_type:4;
#endif
#ifdef __BIG_ENDIAN_BITFIELD
    UInt8_t security_header_type:4;
    UInt8_t protocol_discriminator:4;
#endif
    UInt32_t message_authentication_code;
    UInt8_t sequence_number;
} nas_message_security_header_t;

/* Structure of plain NAS message */
typedef union {
    EMM_msg emm;	/* EPS Mobility Management messages	*/    
    ESM_msg esm;	/* EPS Session Management messages	*/
} nas_message_plain_t;

/* Structure of security protected NAS message */
typedef struct {
    nas_message_security_header_t header;
    nas_message_plain_t plain;
} nas_message_security_protected_t;

/*
 * Structure of a layer 3 NAS message
 */
typedef union {
    nas_message_security_header_t header;
    nas_message_security_protected_t protected;
    nas_message_plain_t plain;
} nas_message_t;

/****************************************************************************/
/********************  G L O B A L    V A R I A B L E S  ********************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

int nas_message_encrypt(const char* inbuf, char* outbuf, const nas_message_security_header_t* header, int length);

int nas_message_decrypt(const char* inbuf, char* outbuf, nas_message_security_header_t* header, int length);

int nas_message_decode(const char* buffer, nas_message_t* msg, int length);

int nas_message_encode(char* buffer, const nas_message_t* msg, int length);

#endif /* __NAS_MESSAGE_H__*/
