/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "../ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER`
 */

#ifndef	_NeighCellsPerBandclassListCDMA2000_H_
#define	_NeighCellsPerBandclassListCDMA2000_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct NeighCellsPerBandclassCDMA2000;

/* NeighCellsPerBandclassListCDMA2000 */
typedef struct NeighCellsPerBandclassListCDMA2000 {
	A_SEQUENCE_OF(struct NeighCellsPerBandclassCDMA2000) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NeighCellsPerBandclassListCDMA2000_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NeighCellsPerBandclassListCDMA2000;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "NeighCellsPerBandclassCDMA2000.h"

#endif	/* _NeighCellsPerBandclassListCDMA2000_H_ */
