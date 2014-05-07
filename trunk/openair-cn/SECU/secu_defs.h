#include "security_types.h"

#ifndef SECU_DEFS_H_
#define SECU_DEFS_H_

#define EIA0_ALG_ID     0x00
#define EIA1_128_ALG_ID 0x01
#define EIA2_128_ALG_ID 0x02

#define EEA0_ALG_ID     0x00
#define EEA1_128_ALG_ID 0x01
#define EEA2_128_ALG_ID 0x02

#define SECU_DIRECTION_UPLINK   0
#define SECU_DIRECTION_DOWNLINK 1

inline
void kdf(const uint8_t *s, const uint32_t s_length, const uint8_t *key,
         const uint32_t key_length, uint8_t **out, uint32_t out_length);

int derive_keNB(const uint8_t kasme[32], const uint32_t nas_count, uint8_t **keNB);

int derive_key_nas(algorithm_type_dist_t nas_alg_type, uint8_t nas_enc_alg_id,
                   const uint8_t kasme[32], uint8_t **knas);

#define derive_key_nas_enc(aLGiD, kASME, kNAS)  \
    derive_key_nas(NAS_ENC_ALG, aLGiD, kASME, kNAS)

#define derive_key_nas_int(aLGiD, kASME, kNAS)  \
    derive_key_nas(NAS_INT_ALG, aLGiD, kASME, kNAS)

#define derive_key_rrc_enc(aLGiD, kASME, kNAS)  \
    derive_key_nas(RRC_ENC_ALG, aLGiD, kASME, kNAS)

#define derive_key_rrc_int(aLGiD, kASME, kNAS)  \
    derive_key_nas(RRC_INT_ALG, aLGiD, kASME, kNAS)

#define derive_key_up_enc(aLGiD, kASME, kNAS)  \
    derive_key_nas(UP_ENC_ALG, aLGiD, kASME, kNAS)

#define derive_key_up_int(aLGiD, kASME, kNAS)  \
    derive_key_nas(UP_INT_ALG, aLGiD, kASME, kNAS)

#define SECU_DIRECTION_UPLINK   0
#define SECU_DIRECTION_DOWNLINK 1

typedef struct {
    uint8_t *key;
    uint32_t key_length;
    uint32_t count;
    uint8_t  bearer;
    uint8_t  direction;
    uint8_t  *message;
    /* length in bits */
    uint32_t  blength;
} nas_stream_cipher_t;

int nas_stream_encrypt_eea2(nas_stream_cipher_t *stream_cipher, uint8_t **out);

int nas_stream_encrypt_eia2(nas_stream_cipher_t *stream_cipher, uint8_t out[4]);

#undef SECU_DEBUG

#endif /* SECU_DEFS_H_ */
