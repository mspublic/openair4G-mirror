/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "/homes/gauthier/PROJETS/OPENAIR4G/openair2/RRM_4_RRC_LITE/src/foreign/generated_c_asn1_rrc/ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER -fcompound-names -fnative-types`
 */

#ifndef	_MeasIdToAddModList_H_
#define	_MeasIdToAddModList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct MeasIdToAddMod;

/* MeasIdToAddModList */
typedef struct MeasIdToAddModList {
	A_SEQUENCE_OF(struct MeasIdToAddMod) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MeasIdToAddModList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MeasIdToAddModList;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "MeasIdToAddMod.h"

#endif	/* _MeasIdToAddModList_H_ */
#include <asn_internal.h>
