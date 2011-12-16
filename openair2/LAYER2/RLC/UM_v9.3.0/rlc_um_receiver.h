#    ifndef __RLC_UM_RECEIVER_PROTO_EXTERN_H__
#        define __RLC_UM_RECEIVER_PROTO_EXTERN_H__
#        ifdef RLC_UM_RECEIVER_C
#            define private_rlc_um_receiver(x)    x
#            define protected_rlc_um_receiver(x)  x
#            define public_rlc_um_receiver(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_receiver(x)
#                define protected_rlc_um_receiver(x)  extern x
#                define public_rlc_um_receiver(x)     extern x
#            else
#                define private_rlc_um_receiver(x)
#                define protected_rlc_um_receiver(x)
#                define public_rlc_um_receiver(x)     extern x
#            endif
#        endif

#        include "rlc_um_entity.h"
#        include "mac_primitives.h"
protected_rlc_um_receiver( signed int rlc_um_sn_in_window(u8_t snP, u8_t lower_boundP, u8_t window_sizeP));
protected_rlc_um_receiver( int rlc_um_get_length_indicators_7 (u16_t * li_arrayP, u8_t * li_array_in_pduP));
protected_rlc_um_receiver( int rlc_um_get_length_indicators_15 (u16_t * li_arrayP, u16_t * li_array_in_pduP));
protected_rlc_um_receiver( void rlc_um_receive_process_pdu_in_sequence_delivery (struct rlc_um_entity *rlcP, struct rlc_um_rx_pdu_management *pdu_mngtP, struct rlc_um_rx_data_pdu_struct *dataP, u16_t tb_sizeP, u16 bad_crc_l1P));
protected_rlc_um_receiver( void rlc_um_receive (struct rlc_um_entity *rlcP, struct mac_data_ind data_indP));
#    endif
