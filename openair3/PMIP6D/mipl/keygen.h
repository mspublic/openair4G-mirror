/* $Id: keygen.h,v 1.1.1.1 2008/04/23 13:21:04 nguyenhn Exp $ */

#ifndef __KEYGEN_H__
#define __KEYGEN_H__ 1

int rr_cn_calc_Kbm(uint16_t home_nonce_ind, uint16_t coa_nonce_ind, 
		   struct in6_addr *hoa, struct in6_addr *coa, uint8_t *kbm);

void rr_mn_calc_Kbm(uint8_t *nonce_hoa, uint8_t *nonce_coa, uint8_t *kbm);

uint16_t rr_cn_keygen_token(struct in6_addr *addr, uint8_t bit, uint8_t *kgt);

int rr_cn_nonce_lft(uint16_t index, struct timespec *lft);

static inline void cookiecpy(void *cookie_a, const void *cookie_b)
{
	memcpy(cookie_a, cookie_b, 8);
}

int rr_cn_init(void);

#endif
