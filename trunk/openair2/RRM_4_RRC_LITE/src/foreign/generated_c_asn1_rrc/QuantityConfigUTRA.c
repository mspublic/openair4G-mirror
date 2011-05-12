/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "./ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER -fcompound-names -fnative-types`
 */

#include "QuantityConfigUTRA.h"

static int
measQuantityUTRA_FDD_2_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	/* Replace with underlying type checker */
	td->check_constraints = asn_DEF_NativeEnumerated.check_constraints;
	return td->check_constraints(td, sptr, ctfailcb, app_key);
}

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static void
measQuantityUTRA_FDD_2_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_NativeEnumerated.free_struct;
	td->print_struct   = asn_DEF_NativeEnumerated.print_struct;
	td->ber_decoder    = asn_DEF_NativeEnumerated.ber_decoder;
	td->der_encoder    = asn_DEF_NativeEnumerated.der_encoder;
	td->xer_decoder    = asn_DEF_NativeEnumerated.xer_decoder;
	td->xer_encoder    = asn_DEF_NativeEnumerated.xer_encoder;
	td->uper_decoder   = asn_DEF_NativeEnumerated.uper_decoder;
	td->uper_encoder   = asn_DEF_NativeEnumerated.uper_encoder;
	if(!td->per_constraints)
		td->per_constraints = asn_DEF_NativeEnumerated.per_constraints;
	td->elements       = asn_DEF_NativeEnumerated.elements;
	td->elements_count = asn_DEF_NativeEnumerated.elements_count;
     /* td->specifics      = asn_DEF_NativeEnumerated.specifics;	// Defined explicitly */
}

static void
measQuantityUTRA_FDD_2_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	measQuantityUTRA_FDD_2_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

static int
measQuantityUTRA_FDD_2_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	measQuantityUTRA_FDD_2_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

static asn_dec_rval_t
measQuantityUTRA_FDD_2_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	measQuantityUTRA_FDD_2_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

static asn_enc_rval_t
measQuantityUTRA_FDD_2_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	measQuantityUTRA_FDD_2_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

static asn_dec_rval_t
measQuantityUTRA_FDD_2_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	measQuantityUTRA_FDD_2_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

static asn_enc_rval_t
measQuantityUTRA_FDD_2_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	measQuantityUTRA_FDD_2_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

static asn_dec_rval_t
measQuantityUTRA_FDD_2_decode_uper(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		asn_per_constraints_t *constraints, void **structure, asn_per_data_t *per_data) {
	measQuantityUTRA_FDD_2_inherit_TYPE_descriptor(td);
	return td->uper_decoder(opt_codec_ctx, td, constraints, structure, per_data);
}

static asn_enc_rval_t
measQuantityUTRA_FDD_2_encode_uper(asn_TYPE_descriptor_t *td,
		asn_per_constraints_t *constraints,
		void *structure, asn_per_outp_t *per_out) {
	measQuantityUTRA_FDD_2_inherit_TYPE_descriptor(td);
	return td->uper_encoder(td, constraints, structure, per_out);
}

static int
measQuantityUTRA_TDD_5_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	/* Replace with underlying type checker */
	td->check_constraints = asn_DEF_NativeEnumerated.check_constraints;
	return td->check_constraints(td, sptr, ctfailcb, app_key);
}

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static void
measQuantityUTRA_TDD_5_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_NativeEnumerated.free_struct;
	td->print_struct   = asn_DEF_NativeEnumerated.print_struct;
	td->ber_decoder    = asn_DEF_NativeEnumerated.ber_decoder;
	td->der_encoder    = asn_DEF_NativeEnumerated.der_encoder;
	td->xer_decoder    = asn_DEF_NativeEnumerated.xer_decoder;
	td->xer_encoder    = asn_DEF_NativeEnumerated.xer_encoder;
	td->uper_decoder   = asn_DEF_NativeEnumerated.uper_decoder;
	td->uper_encoder   = asn_DEF_NativeEnumerated.uper_encoder;
	if(!td->per_constraints)
		td->per_constraints = asn_DEF_NativeEnumerated.per_constraints;
	td->elements       = asn_DEF_NativeEnumerated.elements;
	td->elements_count = asn_DEF_NativeEnumerated.elements_count;
     /* td->specifics      = asn_DEF_NativeEnumerated.specifics;	// Defined explicitly */
}

static void
measQuantityUTRA_TDD_5_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	measQuantityUTRA_TDD_5_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

static int
measQuantityUTRA_TDD_5_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	measQuantityUTRA_TDD_5_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

static asn_dec_rval_t
measQuantityUTRA_TDD_5_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	measQuantityUTRA_TDD_5_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

static asn_enc_rval_t
measQuantityUTRA_TDD_5_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	measQuantityUTRA_TDD_5_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

static asn_dec_rval_t
measQuantityUTRA_TDD_5_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	measQuantityUTRA_TDD_5_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

static asn_enc_rval_t
measQuantityUTRA_TDD_5_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	measQuantityUTRA_TDD_5_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

static asn_dec_rval_t
measQuantityUTRA_TDD_5_decode_uper(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		asn_per_constraints_t *constraints, void **structure, asn_per_data_t *per_data) {
	measQuantityUTRA_TDD_5_inherit_TYPE_descriptor(td);
	return td->uper_decoder(opt_codec_ctx, td, constraints, structure, per_data);
}

static asn_enc_rval_t
measQuantityUTRA_TDD_5_encode_uper(asn_TYPE_descriptor_t *td,
		asn_per_constraints_t *constraints,
		void *structure, asn_per_outp_t *per_out) {
	measQuantityUTRA_TDD_5_inherit_TYPE_descriptor(td);
	return td->uper_encoder(td, constraints, structure, per_out);
}

static asn_per_constraints_t asn_PER_type_measQuantityUTRA_FDD_constr_2 = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_type_measQuantityUTRA_TDD_constr_5 = {
	{ APC_CONSTRAINED,	 0,  0,  0,  0 }	/* (0..0) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_INTEGER_enum_map_t asn_MAP_measQuantityUTRA_FDD_value2enum_2[] = {
	{ 0,	10,	"cpich-RSCP" },
	{ 1,	10,	"cpich-EcN0" }
};
static unsigned int asn_MAP_measQuantityUTRA_FDD_enum2value_2[] = {
	1,	/* cpich-EcN0(1) */
	0	/* cpich-RSCP(0) */
};
static asn_INTEGER_specifics_t asn_SPC_measQuantityUTRA_FDD_specs_2 = {
	asn_MAP_measQuantityUTRA_FDD_value2enum_2,	/* "tag" => N; sorted by tag */
	asn_MAP_measQuantityUTRA_FDD_enum2value_2,	/* N => "tag"; sorted by N */
	2,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static ber_tlv_tag_t asn_DEF_measQuantityUTRA_FDD_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_measQuantityUTRA_FDD_2 = {
	"measQuantityUTRA-FDD",
	"measQuantityUTRA-FDD",
	measQuantityUTRA_FDD_2_free,
	measQuantityUTRA_FDD_2_print,
	measQuantityUTRA_FDD_2_constraint,
	measQuantityUTRA_FDD_2_decode_ber,
	measQuantityUTRA_FDD_2_encode_der,
	measQuantityUTRA_FDD_2_decode_xer,
	measQuantityUTRA_FDD_2_encode_xer,
	measQuantityUTRA_FDD_2_decode_uper,
	measQuantityUTRA_FDD_2_encode_uper,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_measQuantityUTRA_FDD_tags_2,
	sizeof(asn_DEF_measQuantityUTRA_FDD_tags_2)
		/sizeof(asn_DEF_measQuantityUTRA_FDD_tags_2[0]) - 1, /* 1 */
	asn_DEF_measQuantityUTRA_FDD_tags_2,	/* Same as above */
	sizeof(asn_DEF_measQuantityUTRA_FDD_tags_2)
		/sizeof(asn_DEF_measQuantityUTRA_FDD_tags_2[0]), /* 2 */
	&asn_PER_type_measQuantityUTRA_FDD_constr_2,
	0, 0,	/* Defined elsewhere */
	&asn_SPC_measQuantityUTRA_FDD_specs_2	/* Additional specs */
};

static asn_INTEGER_enum_map_t asn_MAP_measQuantityUTRA_TDD_value2enum_5[] = {
	{ 0,	11,	"pccpch-RSCP" }
};
static unsigned int asn_MAP_measQuantityUTRA_TDD_enum2value_5[] = {
	0	/* pccpch-RSCP(0) */
};
static asn_INTEGER_specifics_t asn_SPC_measQuantityUTRA_TDD_specs_5 = {
	asn_MAP_measQuantityUTRA_TDD_value2enum_5,	/* "tag" => N; sorted by tag */
	asn_MAP_measQuantityUTRA_TDD_enum2value_5,	/* N => "tag"; sorted by N */
	1,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static ber_tlv_tag_t asn_DEF_measQuantityUTRA_TDD_tags_5[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_measQuantityUTRA_TDD_5 = {
	"measQuantityUTRA-TDD",
	"measQuantityUTRA-TDD",
	measQuantityUTRA_TDD_5_free,
	measQuantityUTRA_TDD_5_print,
	measQuantityUTRA_TDD_5_constraint,
	measQuantityUTRA_TDD_5_decode_ber,
	measQuantityUTRA_TDD_5_encode_der,
	measQuantityUTRA_TDD_5_decode_xer,
	measQuantityUTRA_TDD_5_encode_xer,
	measQuantityUTRA_TDD_5_decode_uper,
	measQuantityUTRA_TDD_5_encode_uper,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_measQuantityUTRA_TDD_tags_5,
	sizeof(asn_DEF_measQuantityUTRA_TDD_tags_5)
		/sizeof(asn_DEF_measQuantityUTRA_TDD_tags_5[0]) - 1, /* 1 */
	asn_DEF_measQuantityUTRA_TDD_tags_5,	/* Same as above */
	sizeof(asn_DEF_measQuantityUTRA_TDD_tags_5)
		/sizeof(asn_DEF_measQuantityUTRA_TDD_tags_5[0]), /* 2 */
	&asn_PER_type_measQuantityUTRA_TDD_constr_5,
	0, 0,	/* Defined elsewhere */
	&asn_SPC_measQuantityUTRA_TDD_specs_5	/* Additional specs */
};

static int asn_DFL_7_set_4(int set_value, void **sptr) {
	FilterCoefficient_t *st = *sptr;
	
	if(!st) {
		if(!set_value) return -1;	/* Not a default value */
		st = (*sptr = CALLOC(1, sizeof(*st)));
		if(!st) return -1;
	}
	
	if(set_value) {
		/* Install default value 4 */
		*st = 4;
		return 0;
	} else {
		/* Test default value 4 */
		return (*st == 4);
	}
}
static asn_TYPE_member_t asn_MBR_QuantityConfigUTRA_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct QuantityConfigUTRA, measQuantityUTRA_FDD),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_measQuantityUTRA_FDD_2,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"measQuantityUTRA-FDD"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct QuantityConfigUTRA, measQuantityUTRA_TDD),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_measQuantityUTRA_TDD_5,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"measQuantityUTRA-TDD"
		},
	{ ATF_POINTER, 1, offsetof(struct QuantityConfigUTRA, filterCoefficient),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_FilterCoefficient,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		asn_DFL_7_set_4,	/* DEFAULT 4 */
		"filterCoefficient"
		},
};
static int asn_MAP_QuantityConfigUTRA_oms_1[] = { 2 };
static ber_tlv_tag_t asn_DEF_QuantityConfigUTRA_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_QuantityConfigUTRA_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* measQuantityUTRA-FDD at 2247 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* measQuantityUTRA-TDD at 2248 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* filterCoefficient at 2249 */
};
static asn_SEQUENCE_specifics_t asn_SPC_QuantityConfigUTRA_specs_1 = {
	sizeof(struct QuantityConfigUTRA),
	offsetof(struct QuantityConfigUTRA, _asn_ctx),
	asn_MAP_QuantityConfigUTRA_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_QuantityConfigUTRA_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_QuantityConfigUTRA = {
	"QuantityConfigUTRA",
	"QuantityConfigUTRA",
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
	asn_DEF_QuantityConfigUTRA_tags_1,
	sizeof(asn_DEF_QuantityConfigUTRA_tags_1)
		/sizeof(asn_DEF_QuantityConfigUTRA_tags_1[0]), /* 1 */
	asn_DEF_QuantityConfigUTRA_tags_1,	/* Same as above */
	sizeof(asn_DEF_QuantityConfigUTRA_tags_1)
		/sizeof(asn_DEF_QuantityConfigUTRA_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_QuantityConfigUTRA_1,
	3,	/* Elements count */
	&asn_SPC_QuantityConfigUTRA_specs_1	/* Additional specs */
};

