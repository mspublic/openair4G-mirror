#    ifndef __RLC_AM_WINDOWS_H__
#        define __RLC_AM_WINDOWS_H__
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_WINDOWS_C
#            define private_rlc_am_windows(x)    x
#            define protected_rlc_am_windows(x)  x
#            define public_rlc_am_windows(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_windows(x)
#                define protected_rlc_am_windows(x)  extern x
#                define public_rlc_am_windows(x)     extern x
#            else
#                define private_rlc_am_windows(x)
#                define protected_rlc_am_windows(x)
#                define public_rlc_am_windows(x)     extern x
#            endif
#        endif

protected_rlc_am_windows(signed int rlc_am_in_tx_window(rlc_am_entity_t* rlcP, u16_t snP);)
protected_rlc_am_windows(signed int rlc_am_in_rx_window(rlc_am_entity_t* rlcP, u16_t snP);)
protected_rlc_am_windows(signed int rlc_am_sn_gte_vr_h (rlc_am_entity_t* rlcP, u16_t snP);)
protected_rlc_am_windows(signed int rlc_am_sn_gte_vr_x (rlc_am_entity_t* rlcP, u16_t snP);)
protected_rlc_am_windows(signed int rlc_am_sn_gt_vr_ms(rlc_am_entity_t* rlcP, u16_t snP);)
protected_rlc_am_windows(signed int rlc_am_tx_sn1_gt_sn2(rlc_am_entity_t* rlcP, u16_t sn1P, u16_t sn2P);)
protected_rlc_am_windows(signed int rlc_am_rx_sn1_gt_sn2(rlc_am_entity_t* rlcP, u16_t sn1P, u16_t sn2P);)

#    endif
