/*******************************************************************************
 *
 * Eurecom OpenAirInterface 3
 * Copyright(c) 2012 Eurecom
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information
 * Openair Admin: openair_admin@eurecom.fr
 * Openair Tech : openair_tech@eurecom.fr
 * Forums       : http://forums.eurecom.fsr/openairinterface
 * Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France
 *
 *******************************************************************************/
/*! \file lteRALenb_process.h
 * \brief
 * \author GAUTHIER Lionel, MAUREL Frederic, WETTERWALD Michelle
 * \date 2012
 * \version
 * \note
 * \bug
 * \warning
 */

#ifndef __LTE_RAL_ENB_PROCESS_H__
#define __LTE_RAL_ENB_PROCESS_H__
//-----------------------------------------------------------------------------
#        ifdef LTE_RAL_ENB_PROCESS_C
#            define private_lteralenb_process(x)    x
#            define protected_lteralenb_process(x)  x
#            define public_lteralenb_process(x)     x
#        else
#            ifdef LTE_RAL_ENB
#                define private_lteralenb_process(x)
#                define protected_lteralenb_process(x)  extern x
#                define public_lteralenb_process(x)     extern x
#            else
#                define private_lteralenb_process(x)
#                define protected_lteralenb_process(x)
#                define public_lteralenb_process(x)     extern x
#            endif
#        endif
//-----------------------------------------------------------------------------
#include "lteRALenb.h"

protected_lteralenb_process(int   eRAL_process_find_channel            (ral_enb_instance_t instanceP, unsigned int cnxid, int* mt_ix, int* ch_ix);)
protected_lteralenb_process(int   eRAL_process_find_new_channel        (ral_enb_instance_t instanceP, int mt_ix);)
private_lteralenb_process(  void  eRAL_process_clean_channel           (struct ral_lte_channel* channel);)
protected_lteralenb_process(char* eRAL_process_mt_addr_to_string       (const unsigned char* ip_addr);)
protected_lteralenb_process(void  eRAL_process_mt_addr_to_l2id         (const unsigned char* mt_addr, unsigned int* l2id);)
private_lteralenb_process(  int   eRAL_process_cmp_mt_addr             (const char* mt_addr, const char* l2id);)
protected_lteralenb_process(int   eRAL_process_find_mt_by_addr         (ral_enb_instance_t instanceP, const char* mt_addr);)
private_lteralenb_process(  void  eRAL_process_verify_pending_mt_status(ral_enb_instance_t instanceP);)
protected_lteralenb_process(int   eRAL_process_map_qos                 (ral_enb_instance_t instanceP, int mt_ix, int ch_ix);)
private_lteralenb_process(  void  eRAL_process_waiting_RB              (ral_enb_instance_t instanceP, int mt_ix);)
private_lteralenb_process(  void  eRAL_process_clean_pending_mt        (ral_enb_instance_t instanceP);)
protected_lteralenb_process(void  RAL_printInitStatus                     (ral_enb_instance_t instanceP);)
private_lteralenb_process(  void  RAL_NAS_measures_polling                (ral_enb_instance_t instanceP);)
private_lteralenb_process(  void  RAL_NAS_report_congestion               (ral_enb_instance_t instanceP, int ix);)
private_lteralenb_process(  void  RAL_NAS_measures_analyze                (ral_enb_instance_t instanceP);)
#endif

