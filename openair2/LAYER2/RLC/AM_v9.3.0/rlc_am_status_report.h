#ifndef __RLC_AM_STATUS_REPORT_H__
#    define __RLC_AM_STATUS_REPORT_H__

#    include "UTIL/MEM/mem_block.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_STATUS_REPORT_C
#            define private_rlc_am_status_report(x)    x
#            define protected_rlc_am_status_report(x)  x
#            define public_rlc_am_status_report(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_status_report(x)
#                define protected_rlc_am_status_report(x)  extern x
#                define public_rlc_am_status_report(x)     extern x
#            else
#                define private_rlc_am_status_report(x)
#                define protected_rlc_am_status_report(x)
#                define public_rlc_am_status_report(x)     extern x
#            endif
#        endif
//-----------------------------------------------------------------------------
#include "platform_types.h"
#include "platform_constants.h"
#include "PHY/defs.h"

//-----------------------------------------------------------------------------
protected_rlc_am_status_report( u16_t      rlc_am_read_bit_field             (u8_t** dataP, unsigned int* bit_posP, signed int
bits_to_readP);)
protected_rlc_am_status_report(void        rlc_am_write8_bit_field(u8_t** dataP, unsigned int* bit_posP, signed int
bits_to_writeP, u8_t valueP);)
protected_rlc_am_status_report(void        rlc_am_write16_bit_field(u8_t** dataP, unsigned int* bit_posP, signed int
bits_to_writeP, u16_t valueP);)

protected_rlc_am_status_report( signed int rlc_am_get_control_pdu_infos      (rlc_am_pdu_sn_10_t* headerP, s16_t total_sizeP,
rlc_am_control_pdu_info_t* pdu_infoP);)
protected_rlc_am_status_report( void       rlc_am_display_control_pdu_infos(rlc_am_control_pdu_info_t* pdu_infoP);)
protected_rlc_am_status_report( void       rlc_am_receive_process_control_pdu(rlc_am_entity_t* rlcP, mem_block_t*  tbP, u8_t*
first_byte, u16_t tb_size_in_bytes);)
protected_rlc_am_status_report(int         rlc_am_write_status_pdu(rlc_am_pdu_sn_10_t* rlc_am_pdu_sn_10P,
rlc_am_control_pdu_info_t* pdu_infoP);)
protected_rlc_am_status_report(void        rlc_am_send_status_pdu(rlc_am_entity_t* rlcP);)

#endif
