/*****************************************************************
 * C Implementation: pmip_dummy
 * Description: 
 * Author: Eurecom Institute <Huu-Nghia.Nguyen@eurecom.fr>, (C) 2008
 * Copyright: Eurecom Institute
 ******************************************************************/

#include <stdint.h>
#include <pthread.h>
#include "mipv6.h"
#include "debug.h"
#include "crypto.h"
#include "conf.h"

#define pmip_dummy(...) dbg("Dummy function\r\n")

struct mip6_config conf; 
FILE *yyin; 

// //----------------------------------------------------------
// int yyparse (void)
// //----------------------------------------------------------
// {
// 	pmip_dummy();	
// 	return 0;
// }

// //----------------------------------------------------------
// void rr_mn_calc_Kbm(uint8_t *keygen_hoa, uint8_t *keygen_coa, uint8_t *kbm)
// //----------------------------------------------------------
// {
// 	pmip_dummy();
// }

/** 
 * rr_cn_calc_Kbm - calculates the binding authorization key 
 * @home_nonce_ind: home nonce index
 * @coa_nonce_ind: care-of nonce index
 * @hoa: home address of MN
 * @coa: care-of address of MN
 * @kbm: buffer for storing the bu_key, must be at least 20 bytes
 *
 * Returns 0 on success and BA error code on error
 **/
//----------------------------------------------------------

/** 
 * xfrm_last_used - when was a binding  last used
 * @daddr: destination address (home address)
 * @saddr: source address (home address)
 * @proto: protocol. Either IPPROTO_ROUTING or IPPROTO_DSTOPTS 
 **/
//----------------------------------------------------------
// // long xfrm_last_used(const struct in6_addr *daddr, 
// // 		    const struct in6_addr *saddr, int proto,
// // 		    const struct timespec *now)
// // //----------------------------------------------------------
// // {
// // 	pmip_dummy();
// // 	long time_used = 1;
// // 	return time_used; 
// // }

/**
 * bce_type - get type of binding cache entry
 * @our_addr: our IPv6 address
 * @peer_addr: peer's IPv6 address
 *
 * Looks up entry from binding cache and returns its type.  If not
 * found, returns -%ENOENT.
 **/
//----------------------------------------------------------
// int bce_type(const struct in6_addr *our_addr, const struct in6_addr *peer_addr)
// //----------------------------------------------------------
// {
// 	pmip_dummy();
// 	return 0;
// }
