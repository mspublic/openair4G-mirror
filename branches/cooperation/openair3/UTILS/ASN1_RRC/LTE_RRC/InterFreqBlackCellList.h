/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "../ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER`
 */

#ifndef	_InterFreqBlackCellList_H_
#define	_InterFreqBlackCellList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct PhysCellIdRange;

/* InterFreqBlackCellList */
typedef struct InterFreqBlackCellList {
	A_SEQUENCE_OF(struct PhysCellIdRange) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} InterFreqBlackCellList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InterFreqBlackCellList;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "PhysCellIdRange.h"

#endif	/* _InterFreqBlackCellList_H_ */
