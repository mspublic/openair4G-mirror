/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "../ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER`
 */

#ifndef	_IRAT_ParametersUTRA_TDD768_H_
#define	_IRAT_ParametersUTRA_TDD768_H_


#include <asn_application.h>

/* Including external dependencies */
#include "SupportedBandListUTRA-TDD768.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IRAT-ParametersUTRA-TDD768 */
typedef struct IRAT_ParametersUTRA_TDD768 {
	SupportedBandListUTRA_TDD768_t	 supportedBandListUTRA_TDD768;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} IRAT_ParametersUTRA_TDD768_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IRAT_ParametersUTRA_TDD768;

#ifdef __cplusplus
}
#endif

#endif	/* _IRAT_ParametersUTRA_TDD768_H_ */
