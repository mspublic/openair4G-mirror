/*******************************************************************************

Eurecom OpenAirInterface 2
Copyright(c) 1999 - 2010 Eurecom

This program is free software; you can redistribute it and/or modify it
under the terms and conditions of the GNU General Public License,
version 2, as published by the Free Software Foundation.

This program is distributed in the hope it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

The full GNU General Public License is included in this distribution in
the file called "COPYING".

Contact Information
Openair Admin: openair_admin@eurecom.fr
Openair Tech : openair_tech@eurecom.fr
Forums       : http://forums.eurecom.fsr/openairinterface
Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/
/*! \file rlc_am_test.h
* \brief This file defines the prototypes of RLC AM test functions.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
#    ifndef __RLC_AM_TEST_H__
#        define __RLC_AM_TEST_H__
#        ifdef RLC_AM_TEST_C
#            define private_rlc_am_test(x)    x
#            define protected_rlc_am_test(x)  x
#            define public_rlc_am_test(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_test(x)
#                define protected_rlc_am_test(x)  extern x
#                define public_rlc_am_test(x)     extern x
#            else
#                define private_rlc_am_test(x)
#                define protected_rlc_am_test(x)
#                define public_rlc_am_test(x)     extern x
#            endif
#        endif
public_rlc_am_test(rlc_am_entity_t       g_am_tx;)
public_rlc_am_test(rlc_am_entity_t       g_am_rx;)

protected_rlc_am_test(void rlc_am_v9_3_0_test_windows();)
protected_rlc_am_test(void rlc_am_v9_3_0_test_read_write_bit_field();)
protected_rlc_am_test(void rlc_am_v9_3_0_test_data_conf(module_id_t module_idP, rb_id_t rb_idP, mui_t muiP, rlc_tx_status_t statusP);)
protected_rlc_am_test(void rlc_am_v9_3_0_test_send_sdu(rlc_am_entity_t *am_txP, int sdu_indexP);)
protected_rlc_am_test(void rlc_am_v9_3_0_test_exchange_pdus(rlc_am_entity_t *am_txP,rlc_am_entity_t *am_RxP,
                                                            uint16_t bytes_txP,uint16_t bytes_rxP);)
protected_rlc_am_test(void rlc_am_v9_3_0_test_mac_rlc_loop (struct mac_data_ind *data_indP,  struct mac_data_req *data_requestP, int* drop_countP, int *tx_packetsP, int* dropped_tx_packetsP);)
protected_rlc_am_test(void rlc_am_v9_3_0_test_data_ind (module_id_t module_idP, rb_id_t rb_idP, sdu_size_t sizeP,
mem_block_t *sduP);)
protected_rlc_am_test(void rlc_am_v9_3_0_test_tx_rx();)
public_rlc_am_test(   void rlc_am_v9_3_0_test_print_trace (void);)
public_rlc_am_test(   void rlc_am_v9_3_0_test());
#    endif
