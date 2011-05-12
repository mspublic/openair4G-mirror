/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "./ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER -fcompound-names -fnative-types`
 */

#ifndef	_RRCConnectionSetupComplete_H_
#define	_RRCConnectionSetupComplete_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RRC-TransactionIdentifier.h"
#include "RRCConnectionSetupComplete-r8-IEs.h"
#include <NULL.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RRCConnectionSetupComplete__criticalExtensions_PR {
	RRCConnectionSetupComplete__criticalExtensions_PR_NOTHING,	/* No components present */
	RRCConnectionSetupComplete__criticalExtensions_PR_c1,
	RRCConnectionSetupComplete__criticalExtensions_PR_criticalExtensionsFuture
} RRCConnectionSetupComplete__criticalExtensions_PR;
typedef enum RRCConnectionSetupComplete__criticalExtensions__c1_PR {
	RRCConnectionSetupComplete__criticalExtensions__c1_PR_NOTHING,	/* No components present */
	RRCConnectionSetupComplete__criticalExtensions__c1_PR_rrcConnectionSetupComplete_r8,
	RRCConnectionSetupComplete__criticalExtensions__c1_PR_spare3,
	RRCConnectionSetupComplete__criticalExtensions__c1_PR_spare2,
	RRCConnectionSetupComplete__criticalExtensions__c1_PR_spare1
} RRCConnectionSetupComplete__criticalExtensions__c1_PR;

/* RRCConnectionSetupComplete */
typedef struct RRCConnectionSetupComplete {
	RRC_TransactionIdentifier_t	 rrc_TransactionIdentifier;
	struct RRCConnectionSetupComplete__criticalExtensions {
		RRCConnectionSetupComplete__criticalExtensions_PR present;
		union RRCConnectionSetupComplete__criticalExtensions_u {
			struct RRCConnectionSetupComplete__criticalExtensions__c1 {
				RRCConnectionSetupComplete__criticalExtensions__c1_PR present;
				union RRCConnectionSetupComplete__criticalExtensions__c1_u {
					RRCConnectionSetupComplete_r8_IEs_t	 rrcConnectionSetupComplete_r8;
					NULL_t	 spare3;
					NULL_t	 spare2;
					NULL_t	 spare1;
				} choice;
				
				/* Context for parsing across buffer boundaries */
				asn_struct_ctx_t _asn_ctx;
			} c1;
			struct RRCConnectionSetupComplete__criticalExtensions__criticalExtensionsFuture {
				
				/* Context for parsing across buffer boundaries */
				asn_struct_ctx_t _asn_ctx;
			} criticalExtensionsFuture;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} criticalExtensions;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RRCConnectionSetupComplete_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RRCConnectionSetupComplete;

#ifdef __cplusplus
}
#endif

#endif	/* _RRCConnectionSetupComplete_H_ */
#include <asn_internal.h>
