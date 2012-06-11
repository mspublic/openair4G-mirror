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
/*! \file mRALlteInt_get.h
 * \brief
 * \author BRIZZOLA Davide, GAUTHIER Lionel, MAUREL Frederic, WETTERWALD Michelle
 * \date 2012
 * \version
 * \note
 * \bug
 * \warning
 */

#ifndef __MRALLTEINT_GET_H__
#    define __MRALLTEINT_GET_H__
//-----------------------------------------------------------------------------
#        ifdef MRALLTEINT_GET_C
#            define private_mRALlteInt_get(x)    x
#            define protected_mRALlteInt_get(x)  x
#            define public_mRALlteInt_get(x)     x
#        else
#            ifdef MRAL_MODULE
#                define private_mRALlteInt_get(x)
#                define protected_mRALlteInt_get(x)  extern x
#                define public_mRALlteInt_get(x)     extern x
#            else
#                define private_mRALlteInt_get(x)
#                define protected_mRALlteInt_get(x)
#                define public_mRALlteInt_get(x)     extern x
#            endif
#        endif
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
//-----------------------------------------------------------------------------
#include "MIH_C.h"
//-----------------------------------------------------------------------------
public_mRALlteInt_get(void MIH_C_3GPP_ADDR_load_3gpp_str_address(MIH_C_3GPP_ADDR_T* _3gpp_addrP, u_int8_t* strP);)

#endif
