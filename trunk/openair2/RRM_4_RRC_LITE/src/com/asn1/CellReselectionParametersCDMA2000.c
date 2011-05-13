/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "/homes/gauthier/PROJETS/OPENAIR4G/openair2/RRM_4_RRC_LITE/src/foreign/generated_c_asn1_rrc/ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER -fcompound-names -fnative-types`
 */

#include "CellReselectionParametersCDMA2000.h"

static asn_TYPE_member_t asn_MBR_CellReselectionParametersCDMA2000_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CellReselectionParametersCDMA2000, bandClassList),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BandClassListCDMA2000,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"bandClassList"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CellReselectionParametersCDMA2000, neighCellList),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NeighCellListCDMA2000,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"neighCellList"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CellReselectionParametersCDMA2000, t_ReselectionCDMA2000),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_Reselection,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"t-ReselectionCDMA2000"
		},
	{ ATF_POINTER, 1, offsetof(struct CellReselectionParametersCDMA2000, t_ReselectionCDMA2000_SF),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SpeedStateScaleFactors,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"t-ReselectionCDMA2000-SF"
		},
};
static int asn_MAP_CellReselectionParametersCDMA2000_oms_1[] = { 3 };
static ber_tlv_tag_t asn_DEF_CellReselectionParametersCDMA2000_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_CellReselectionParametersCDMA2000_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* bandClassList at 1023 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* neighCellList at 1024 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* t-ReselectionCDMA2000 at 1025 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* t-ReselectionCDMA2000-SF at 1026 */
};
static asn_SEQUENCE_specifics_t asn_SPC_CellReselectionParametersCDMA2000_specs_1 = {
	sizeof(struct CellReselectionParametersCDMA2000),
	offsetof(struct CellReselectionParametersCDMA2000, _asn_ctx),
	asn_MAP_CellReselectionParametersCDMA2000_tag2el_1,
	4,	/* Count of tags in the map */
	asn_MAP_CellReselectionParametersCDMA2000_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CellReselectionParametersCDMA2000 = {
	"CellReselectionParametersCDMA2000",
	"CellReselectionParametersCDMA2000",
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
	asn_DEF_CellReselectionParametersCDMA2000_tags_1,
	sizeof(asn_DEF_CellReselectionParametersCDMA2000_tags_1)
		/sizeof(asn_DEF_CellReselectionParametersCDMA2000_tags_1[0]), /* 1 */
	asn_DEF_CellReselectionParametersCDMA2000_tags_1,	/* Same as above */
	sizeof(asn_DEF_CellReselectionParametersCDMA2000_tags_1)
		/sizeof(asn_DEF_CellReselectionParametersCDMA2000_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_CellReselectionParametersCDMA2000_1,
	4,	/* Elements count */
	&asn_SPC_CellReselectionParametersCDMA2000_specs_1	/* Additional specs */
};

