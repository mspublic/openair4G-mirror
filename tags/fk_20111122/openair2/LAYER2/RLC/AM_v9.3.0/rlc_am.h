#    ifndef __RLC_AM_PROTO_EXTERN_H__
#        define __RLC_AM_PROTO_EXTERN_H__
#        ifdef RLC_AM_C
#            define private_rlc_am(x)
#            define protected_rlc_am(x)
#            define public_rlc_am(x)
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am(x)
#                define protected_rlc_am(x)  extern x
#                define public_rlc_am(x)     extern x
#            else
#                define private_rlc_am(x)
#                define protected_rlc_am(x)
#                define public_rlc_am(x)     extern x
#            endif
#        endif
#        include "platform_types.h"
#        include "rlc_def.h"
#        include "rlc_def_lte.h"
#        include "rlc_am_constants.h"
#        include "rlc_am_structs.h"
#        include "rlc_am_entity.h"
#        include "rlc_am_windows.h"
#        include "mem_block.h"
#        include "rlc_am_in_sdu.h"
#        include "rlc_am_segment.h"
#        include "rlc_am_segments_holes.h"
#        include "rlc_am_timer_poll_retransmit.h"
#        include "rlc_am_timer_reordering.h"
#        include "rlc_am_timer_status_prohibit.h"
#        include "rlc_am_retransmit.h"
#        include "rlc_am_receiver.h"
#        include "rlc_am_status_report.h"
#        include "rlc_am_rx_list.h"
#        include "rlc_am_reassembly.h"
#        include "rlc_am_init.h"
//#        include "rlc_am_test.h"

#ifdef USER_MODE
//#        include "rlc_am_very_simple_test.h"
#endif

typedef volatile struct {
	u16_t max_retx_threshold;
	u16_t poll_pdu;
	u16_t poll_byte;
	u32_t t_poll_retransmit;
	u32_t t_reordering;
	u32_t t_status_prohibit;
} rlc_am_info_t;


public_rlc_am(void     rlc_am_release (rlc_am_entity_t *rlcP);)
public_rlc_am(void     config_req_rlc_am (rlc_am_entity_t *rlcP, module_id_t module_idP, rlc_am_info_t * config_amP, u8_t rb_idP, rb_type_t rb_typeP);)
public_rlc_am(void     rlc_am_stat_req     (rlc_am_entity_t *rlcP,
                              unsigned int* tx_pdcp_sdu,
                              unsigned int* tx_pdcp_sdu_discarded,
                              unsigned int* tx_retransmit_pdu_unblock,
                              unsigned int* tx_retransmit_pdu_by_status,
                              unsigned int* tx_retransmit_pdu,
                              unsigned int* tx_data_pdu,
                              unsigned int* tx_control_pdu,
                              unsigned int* rx_sdu,
                              unsigned int* rx_error_pdu,
                              unsigned int* rx_data_pdu,
                              unsigned int* rx_data_pdu_out_of_window,
                              unsigned int* rx_control_pdu);)
private_rlc_am(   void     rlc_am_get_pdus (void *argP);)
protected_rlc_am( void     rlc_am_rx (void *, struct mac_data_ind);)
public_rlc_am(    struct mac_status_resp rlc_am_mac_status_indication (void *rlcP, u16_t tbs_sizeP, struct mac_status_ind tx_statusP);)
public_rlc_am(    struct mac_data_req rlc_am_mac_data_request (void *rlcP);)
public_rlc_am(    void     rlc_am_mac_data_indication (void *rlcP, struct mac_data_ind data_indP);)
public_rlc_am(    void     rlc_am_data_req (void *rlcP, mem_block_t *sduP);)
#    endif
