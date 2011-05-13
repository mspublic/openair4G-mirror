/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "RRM-RRC-Definitions"
 * 	found in "/homes/gauthier/PROJETS/OPENAIR4G/openair2/RRM_4_RRC_LITE/src/com/asn1/rrc-rrm.asn"
 * 	`asn1c -gen-PER -fcompound-names -fnative-types`
 */

#include "RRM-RRC-Message.h"

static asn_TYPE_member_t asn_MBR_RRM_RRC_Message_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct RRM_RRC_Message, message),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_RRM_RRC_MessageType,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"message"
		},
};
static ber_tlv_tag_t asn_DEF_RRM_RRC_Message_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_RRM_RRC_Message_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* message at 103 */
};
static asn_SEQUENCE_specifics_t asn_SPC_RRM_RRC_Message_specs_1 = {
	sizeof(struct RRM_RRC_Message),
	offsetof(struct RRM_RRC_Message, _asn_ctx),
	asn_MAP_RRM_RRC_Message_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_RRM_RRC_Message = {
	"RRM-RRC-Message",
	"RRM-RRC-Message",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	SEQUENCE_decode_uper,
	SEQUENCE_encode_uper,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_RRM_RRC_Message_tags_1,
	sizeof(asn_DEF_RRM_RRC_Message_tags_1)
		/sizeof(asn_DEF_RRM_RRC_Message_tags_1[0]), /* 1 */
	asn_DEF_RRM_RRC_Message_tags_1,	/* Same as above */
	sizeof(asn_DEF_RRM_RRC_Message_tags_1)
		/sizeof(asn_DEF_RRM_RRC_Message_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_RRM_RRC_Message_1,
	1,	/* Elements count */
	&asn_SPC_RRM_RRC_Message_specs_1	/* Additional specs */
};

