/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "../ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER`
 */

#ifndef	_DRB_CountInfo_H_
#define	_DRB_CountInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include "DRB-Identity.h"
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DRB-CountInfo */
typedef struct DRB_CountInfo {
	DRB_Identity_t	 drb_Identity;
	unsigned long	 count_Uplink;
	unsigned long	 count_Downlink;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DRB_CountInfo_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_count_Uplink_3;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_count_Downlink_4;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_DRB_CountInfo;

#ifdef __cplusplus
}
#endif

#endif	/* _DRB_CountInfo_H_ */
