#include "secu_defs.h"

/*!
 * Derive the kNASenc from kasme and perform truncate on the generated key to
 * reduce his size to 128 bits. Definition of the derivation function can
 * be found in 3GPP TS.33401 #A.7
 * @param[in] nas_alg_type NAS algorithm distinguisher
 * @param[in] nas_enc_alg_id NAS encryption/integrity algorithm identifier.
 * Possible values are:
 *      - 0 for EIA0 algorithm (Null Integrity Protection algorithm)
 *      - 1 for 128-EIA1 SNOW 3G
 *      - 2 for 128-EIA2 AES
 * @param[in] kasme Key for MME as provided by AUC
 * @param[out] knas Truncated ouput key as derived by KDF
 */
int derive_key_nas(algorithm_distinguisher_t nas_alg_type, uint8_t nas_enc_alg_id,
                   uint8_t kasme[32], uint8_t knas[16])
{
    uint8_t s[7];
    uint8_t knas_temp[32];

    /* FC */
    s[0] = 0x15;

    /* P0 = algorithm type distinguisher */
    s[1] = nas_alg_type & 0xFF;

    /* L0 = length(P0) = 1 */
    s[2] = 0x00;
    s[3] = 0x01;

    /* P1 */
    s[4] = nas_enc_alg_id;

    /* L1 = length(P1) = 1 */
    s[5] = 0x00;
    s[6] = 0x01;

    kdf(s, 7, kasme, 32, knas_temp, 32);

    /* Truncate the generate key to 128 bits */
    memcpy(knas, knas_temp, 16);

    return 0;
}
