/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "RRM-RRC-Definitions"
 * 	found in "/homes/gauthier/PROJETS/OPENAIR4G/openair2/RRM_4_RRC_LITE/src/com/asn1/rrc-rrm.asn"
 * 	`asn1c -gen-PER -fcompound-names -fnative-types`
 */

#ifndef	_RRC_RRM_Message_H_
#define	_RRC_RRM_Message_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RRC-RRM-MessageType.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RRC-RRM-Message */
typedef struct RRC_RRM_Message {
	RRC_RRM_MessageType_t	 message;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RRC_RRM_Message_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RRC_RRM_Message;

#ifdef __cplusplus
}
#endif

#endif	/* _RRC_RRM_Message_H_ */
#include <asn_internal.h>
