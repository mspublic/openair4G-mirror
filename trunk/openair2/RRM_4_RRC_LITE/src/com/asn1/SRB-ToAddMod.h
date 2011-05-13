/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "/homes/gauthier/PROJETS/OPENAIR4G/openair2/RRM_4_RRC_LITE/src/foreign/generated_c_asn1_rrc/ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER -fcompound-names -fnative-types`
 */

#ifndef	_SRB_ToAddMod_H_
#define	_SRB_ToAddMod_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include "RLC-Config.h"
#include <NULL.h>
#include <constr_CHOICE.h>
#include "LogicalChannelConfig.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SRB_ToAddMod__rlc_Config_PR {
	SRB_ToAddMod__rlc_Config_PR_NOTHING,	/* No components present */
	SRB_ToAddMod__rlc_Config_PR_explicitValue,
	SRB_ToAddMod__rlc_Config_PR_defaultValue
} SRB_ToAddMod__rlc_Config_PR;
typedef enum SRB_ToAddMod__logicalChannelConfig_PR {
	SRB_ToAddMod__logicalChannelConfig_PR_NOTHING,	/* No components present */
	SRB_ToAddMod__logicalChannelConfig_PR_explicitValue,
	SRB_ToAddMod__logicalChannelConfig_PR_defaultValue
} SRB_ToAddMod__logicalChannelConfig_PR;

/* SRB-ToAddMod */
typedef struct SRB_ToAddMod {
	long	 srb_Identity;
	struct SRB_ToAddMod__rlc_Config {
		SRB_ToAddMod__rlc_Config_PR present;
		union SRB_ToAddMod__rlc_Config_u {
			RLC_Config_t	 explicitValue;
			NULL_t	 defaultValue;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *rlc_Config;
	struct SRB_ToAddMod__logicalChannelConfig {
		SRB_ToAddMod__logicalChannelConfig_PR present;
		union SRB_ToAddMod__logicalChannelConfig_u {
			LogicalChannelConfig_t	 explicitValue;
			NULL_t	 defaultValue;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *logicalChannelConfig;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SRB_ToAddMod_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SRB_ToAddMod;

#ifdef __cplusplus
}
#endif

#endif	/* _SRB_ToAddMod_H_ */
#include <asn_internal.h>
