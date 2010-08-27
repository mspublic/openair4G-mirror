/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "../ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER`
 */

#ifndef	_RF_Parameters_H_
#define	_RF_Parameters_H_


#include <asn_application.h>

/* Including external dependencies */
#include "SupportedBandListEUTRA.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RF-Parameters */
typedef struct RF_Parameters {
	SupportedBandListEUTRA_t	 supportedBandListEUTRA;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RF_Parameters_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RF_Parameters;

#ifdef __cplusplus
}
#endif

#endif	/* _RF_Parameters_H_ */
