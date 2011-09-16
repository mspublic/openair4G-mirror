#    ifndef __RLC_U_TEST_H__
#        define __RLC_UM_TEST_H__
#        ifdef RLC_UM_TEST_C
#            define private_rlc_um_test(x)    x
#            define protected_rlc_um_test(x)  x
#            define public_rlc_um_test(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_test(x)
#                define protected_rlc_um_test(x)  extern x
#                define public_rlc_um_test(x)     extern x
#            else
#                define private_rlc_um_test(x)
#                define protected_rlc_um_test(x)
#                define public_rlc_um_test(x)     extern x
#            endif
#        endif
public_rlc_um_test(rlc_um_entity_t       um_tx;)
public_rlc_um_test(rlc_um_entity_t       um_rx;)

protected_rlc_um_test(void rlc_um_v9_3_0_test_windows();)
protected_rlc_um_test(void rlc_um_v9_3_0_test_data_conf(module_id_t module_idP, rb_id_t rb_idP, mui_t muiP, rlc_tx_status_t statusP);)
protected_rlc_um_test(void rlc_um_v9_3_0_test_send_sdu(rlc_um_entity_t *um_txP, int sdu_indexP);)
protected_rlc_um_test(void rlc_um_v9_3_0_test_exchange_pdus(rlc_um_entity_t *um_txP,rlc_um_entity_t *um_RxP,
                                                            u16_t bytes_txP,u16_t bytes_rxP);)
protected_rlc_um_test(void rlc_um_v9_3_0_test_mac_rlc_loop (struct mac_data_ind *data_indP,  struct mac_data_req *data_requestP, int* drop_countP, int *tx_packetsP, int* dropped_tx_packetsP);)
protected_rlc_um_test(void rlc_um_v9_3_0_test_data_ind (module_id_t module_idP, rb_id_t rb_idP, sdu_size_t sizeP,
mem_block_t *sduP);)
protected_rlc_um_test(void rlc_um_v9_3_0_test_tx_rx();)
public_rlc_um_test(   void rlc_um_v9_3_0_test_print_trace (void);)
public_rlc_um_test(   void rlc_um_v9_3_0_test());
#    endif
