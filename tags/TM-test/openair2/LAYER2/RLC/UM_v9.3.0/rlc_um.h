#    ifndef __RLC_UM_H__
#        define __RLC_UM_H__
#        ifdef RLC_UM_C
#            define private_rlc_um(x)
#            define protected_rlc_um(x)
#            define public_rlc_um(x)
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um(x)
#                define protected_rlc_um(x)  extern x
#                define public_rlc_um(x)     extern x
#            else
#                define private_rlc_um(x)
#                define protected_rlc_um(x)
#                define public_rlc_um(x)     extern x
#            endif
#        endif
#        include "platform_types.h"
#        include "rlc_def.h"
#        include "rlc_def_lte.h"
#        include "rlc_um_constants.h"
#        include "rlc_um_structs.h"
#        include "rlc_um_entity.h"
#        include "mem_block.h"
//#        include "rlc_um_control_primitives.h"
#        include "rlc_um_dar.h"
#        include "rlc_um_fsm.h"
#        include "rlc_um_reassembly.h"
#        include "rlc_um_receiver.h"
#        include "rlc_um_segment.h"
#        include "rlc_um_test.h"
#ifdef USER_MODE
//#        include "rlc_um_very_simple_test.h"
#endif

typedef volatile struct {
    u32_t             timer_reordering;
    u32_t             sn_field_length; // 5 or 10
    u32_t             is_mXch; // boolean, true if configured for MTCH or MCCH
} rlc_um_info_t;

public_rlc_um( void     rlc_um_stat_req     (struct rlc_um_entity *rlcP,
							  unsigned int* tx_pdcp_sdu,
							  unsigned int* tx_pdcp_sdu_discarded,
							  unsigned int* tx_data_pdu,
							  unsigned int* rx_sdu,
							  unsigned int* rx_error_pdu,
							  unsigned int* rx_data_pdu,
							  unsigned int* rx_data_pdu_out_of_window);)
private_rlc_um(   void     rlc_um_get_pdus (void *argP);)
protected_rlc_um( void     rlc_um_rx (void *, struct mac_data_ind);)
public_rlc_um(    struct mac_status_resp rlc_um_mac_status_indication (void *rlcP, u16_t tbs_sizeP, struct mac_status_ind tx_statusP);)
public_rlc_um(    struct mac_data_req rlc_um_mac_data_request (void *rlcP);)
public_rlc_um(   void     rlc_um_mac_data_indication (void *rlcP, struct mac_data_ind data_indP);)
public_rlc_um(    void     rlc_um_data_req (void *rlcP, mem_block_t *sduP);)
#    endif
