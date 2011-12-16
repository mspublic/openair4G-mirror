#ifndef __RLC_AM_RX_LIST_H__
#    define __RLC_AM_RX_LIST_H__

#    include "UTIL/MEM/mem_block.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_RX_LIST_C
#            define private_rlc_am_rx_list(x)    x
#            define protected_rlc_am_rx_list(x)  x
#            define public_rlc_am_rx_list(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_rx_list(x)
#                define protected_rlc_am_rx_list(x)  extern x
#                define public_rlc_am_rx_list(x)     extern x
#            else
#                define private_rlc_am_rx_list(x)
#                define protected_rlc_am_rx_list(x)
#                define public_rlc_am_rx_list(x)     extern x
#            endif
#        endif
//-----------------------------------------------------------------------------
#include "platform_types.h"
#include "platform_constants.h"
#include "PHY/defs.h"

//-----------------------------------------------------------------------------
protected_rlc_am_rx_list( signed int rlc_am_rx_list_insert_pdu(rlc_am_entity_t* rlcP, mem_block_t* tbP);)
protected_rlc_am_rx_list( void rlc_am_rx_check_all_byte_segments(rlc_am_entity_t* rlcP, mem_block_t* tbP);)
protected_rlc_am_rx_list( void rlc_am_rx_mark_all_segments_received(rlc_am_entity_t* rlcP, mem_block_t* fisrt_segment_tbP);)
protected_rlc_am_rx_list( void rlc_am_rx_list_reassemble_rlc_sdus(rlc_am_entity_t* rlcP);)

public_rlc_am_rx_list( mem_block_t* list2_insert_before_element (mem_block_t * element_to_insertP, mem_block_t * elementP, list2_t * listP);)
public_rlc_am_rx_list( mem_block_t* list2_insert_after_element (mem_block_t * element_to_insertP, mem_block_t * elementP, list2_t * listP);)
protected_rlc_am_rx_list( void rlc_am_rx_list_display (rlc_am_entity_t* rlcP, char* messageP);)
#endif
