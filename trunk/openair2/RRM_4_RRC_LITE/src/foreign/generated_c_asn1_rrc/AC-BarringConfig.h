/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "./ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER -fcompound-names -fnative-types`
 */

#ifndef	_AC_BarringConfig_H_
#define	_AC_BarringConfig_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <BIT_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AC_BarringConfig__ac_BarringFactor {
	AC_BarringConfig__ac_BarringFactor_p00	= 0,
	AC_BarringConfig__ac_BarringFactor_p05	= 1,
	AC_BarringConfig__ac_BarringFactor_p10	= 2,
	AC_BarringConfig__ac_BarringFactor_p15	= 3,
	AC_BarringConfig__ac_BarringFactor_p20	= 4,
	AC_BarringConfig__ac_BarringFactor_p25	= 5,
	AC_BarringConfig__ac_BarringFactor_p30	= 6,
	AC_BarringConfig__ac_BarringFactor_p40	= 7,
	AC_BarringConfig__ac_BarringFactor_p50	= 8,
	AC_BarringConfig__ac_BarringFactor_p60	= 9,
	AC_BarringConfig__ac_BarringFactor_p70	= 10,
	AC_BarringConfig__ac_BarringFactor_p75	= 11,
	AC_BarringConfig__ac_BarringFactor_p80	= 12,
	AC_BarringConfig__ac_BarringFactor_p85	= 13,
	AC_BarringConfig__ac_BarringFactor_p90	= 14,
	AC_BarringConfig__ac_BarringFactor_p95	= 15
} e_AC_BarringConfig__ac_BarringFactor;
typedef enum AC_BarringConfig__ac_BarringTime {
	AC_BarringConfig__ac_BarringTime_s4	= 0,
	AC_BarringConfig__ac_BarringTime_s8	= 1,
	AC_BarringConfig__ac_BarringTime_s16	= 2,
	AC_BarringConfig__ac_BarringTime_s32	= 3,
	AC_BarringConfig__ac_BarringTime_s64	= 4,
	AC_BarringConfig__ac_BarringTime_s128	= 5,
	AC_BarringConfig__ac_BarringTime_s256	= 6,
	AC_BarringConfig__ac_BarringTime_s512	= 7
} e_AC_BarringConfig__ac_BarringTime;

/* AC-BarringConfig */
typedef struct AC_BarringConfig {
	long	 ac_BarringFactor;
	long	 ac_BarringTime;
	BIT_STRING_t	 ac_BarringForSpecialAC;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AC_BarringConfig_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_ac_BarringFactor_2;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_ac_BarringTime_19;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_AC_BarringConfig;

#ifdef __cplusplus
}
#endif

#endif	/* _AC_BarringConfig_H_ */
#include <asn_internal.h>
