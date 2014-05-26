#ifndef OSA_INTERNAL_H_
#define OSA_INTERNAL_H_

#define FC_KENB         (0x11)
#define FC_NH           (0x12)
#define FC_KENB_STAR    (0x13)
/* 33401 #A.7 Algorithm for key derivation function.
 * This FC should be used for:
 * - NAS Encryption algorithm
 * - NAS Integrity algorithm
 * - RRC Encryption algorithm
 * - RRC Integrity algorithm
 * - User Plane Encryption algorithm
 */
#define FC_ALG_KEY_DER  (0x15)
#define FC_KASME_TO_CK  (0x16)

#ifndef hton_int32
# define hton_int32(x)   \
(((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8) |  \
((x & 0x00FF0000) >> 8) | ((x & 0xFF000000) >> 24))
#endif

// #define SECU_DEBUG

#endif /* OSA_INTERNAL_H_ */
