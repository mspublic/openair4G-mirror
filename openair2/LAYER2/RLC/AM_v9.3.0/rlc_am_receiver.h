#    ifndef __RLC_AM_RECEIVER_PROTO_EXTERN_H__
#        define __RLC_AM_RECEIVER_PROTO_EXTERN_H__
#        ifdef RLC_AM_RECEIVER_C
#            define private_rlc_am_receiver(x)    x
#            define protected_rlc_am_receiver(x)  x
#            define public_rlc_am_receiver(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_receiver(x)
#                define protected_rlc_am_receiver(x)  extern x
#                define public_rlc_am_receiver(x)     extern x
#            else
#                define private_rlc_am_receiver(x)
#                define protected_rlc_am_receiver(x)
#                define public_rlc_am_receiver(x)     extern x
#            endif
#        endif
//protected_rlc_am_receiver( signed int rlc_am_sn_in_window(u8_t snP, u8_t lower_boundP, u8_t window_sizeP));
protected_rlc_am_receiver( signed int rlc_am_get_data_pdu_infos(rlc_am_pdu_sn_10_t* headerP, s16_t sizeP, rlc_am_pdu_info_t* pdu_infoP));
protected_rlc_am_receiver( void rlc_am_display_data_pdu_infos(rlc_am_entity_t *rlcP, rlc_am_pdu_info_t* pdu_infoP);)
protected_rlc_am_receiver( void rlc_am_rx_update_vr_ms(rlc_am_entity_t *rlcP,mem_block_t* tbP);)
protected_rlc_am_receiver( void rlc_am_rx_update_vr_r (rlc_am_entity_t *rlcP,mem_block_t* tbP);)
protected_rlc_am_receiver( void rlc_am_receive_routing (rlc_am_entity_t *rlcP, struct mac_data_ind data_indP));
private_rlc_am_receiver( void rlc_am_receive_process_data_pdu (rlc_am_entity_t *rlcP, mem_block_t* tbP, u8_t* first_byteP, u16_t tb_size_in_bytesP));
#    endif
