/*! \file pmip_dummy.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup DUMMY
 * @ingroup PMIP6D
 *  PMIP DUMMY 
 *  @{
 */

#ifndef __pmip_dummy_h
#    define __pmip_dummy_h
int yyparse(void);      //gram.c
void rr_mn_calc_Kbm(uint8_t * keygen_hoa, uint8_t * keygen_coa, uint8_t * kbm); //keygen.c
int bce_type(const struct in6_addr *our_addr, const struct in6_addr *peer_addr);    //bcache.c
void rr_mn_calc_Kbm(uint8_t * keygen_hoa, uint8_t * keygen_coa, uint8_t * kbm);
int rr_cn_calc_Kbm(uint16_t home_nonce_ind, uint16_t coa_nonce_ind, struct in6_addr *hoa, struct in6_addr *coa, uint8_t * kbm);
long xfrm_last_used(const struct in6_addr *daddr, const struct in6_addr *saddr, int proto, const struct timespec *now); //xfrm.c
#endif              //__pmip_dummy_h
